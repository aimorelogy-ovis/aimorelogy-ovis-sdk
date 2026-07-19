#include "cv/target_search/fastsam_segment.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <opencv2/opencv.hpp>
#include <vector>
#include "utils/tdl_log.hpp"

static const int kMinimumCropSize = 320;
static const int kMaximumDenseCropSize = 640;
static const float kHintCropScale = 1.2f;
static const float kLargeHintCropScale = 1.08f;
static const float kResultPaddingRatio = 0.08f;

struct FastSAMMaskCandidate {
  int segment_index = -1;
  cv::Rect bbox;
  float score = -std::numeric_limits<float>::max();
  float model_score = 0.0f;
  float crop_ratio = 0.0f;
  float raw_bbox_ratio = 0.0f;
  int foreground_area = 0;
  float seed_distance = 0.0f;
  bool touches_crop_border = false;
};

static float clampUnit(float value) {
  return std::max(0.0f, std::min(value, 1.0f));
}

static float rectIou(const cv::Rect& lhs, const cv::Rect& rhs) {
  cv::Rect intersection = lhs & rhs;
  int union_area = lhs.area() + rhs.area() - intersection.area();
  if (intersection.empty() || union_area <= 0) {
    return 0.0f;
  }
  return static_cast<float>(intersection.area()) / union_area;
}

static cv::Rect makeSquareCropAt(int image_width, int image_height,
                                 cv::Point center, int crop_size) {
  crop_size = std::min(crop_size, std::min(image_width, image_height));
  if (crop_size <= 0) {
    return cv::Rect();
  }

  int crop_x = center.x - crop_size / 2;
  int crop_y = center.y - crop_size / 2;
  crop_x = std::max(0, std::min(crop_x, image_width - crop_size));
  crop_y = std::max(0, std::min(crop_y, image_height - crop_size));
  return cv::Rect(crop_x, crop_y, crop_size, crop_size);
}

static cv::Rect makeSquareCrop(int image_width, int image_height,
                               cv::Point seed_point,
                               const cv::Rect* hint_bbox) {
  int crop_size = kMinimumCropSize;
  int center_x = seed_point.x;
  int center_y = seed_point.y;

  if (hint_bbox != nullptr && hint_bbox->width > 0 && hint_bbox->height > 0) {
    int hint_extent = std::max(hint_bbox->width, hint_bbox->height);
    float crop_scale = hint_extent <= kMaximumDenseCropSize
                           ? kHintCropScale
                           : kLargeHintCropScale;
    crop_size = std::max(crop_size,
                         static_cast<int>(hint_extent * crop_scale + 0.5f));
    if (hint_extent <= kMaximumDenseCropSize) {
      crop_size = std::min(crop_size, kMaximumDenseCropSize);
    }
    center_x = hint_bbox->x + hint_bbox->width / 2;
    center_y = hint_bbox->y + hint_bbox->height / 2;
  }
  return makeSquareCropAt(image_width, image_height,
                          cv::Point(center_x, center_y), crop_size);
}

static cv::Rect mapRectToOutputSpace(const cv::Rect& bbox,
                                     const cv::Rect& crop_rect,
                                     int output_width, int output_height) {
  cv::Rect mapped;
  mapped.x = static_cast<int>((bbox.x - crop_rect.x) *
                                  static_cast<float>(output_width) /
                                  crop_rect.width +
                              0.5f);
  mapped.y = static_cast<int>((bbox.y - crop_rect.y) *
                                  static_cast<float>(output_height) /
                                  crop_rect.height +
                              0.5f);
  mapped.width = static_cast<int>(bbox.width *
                                      static_cast<float>(output_width) /
                                      crop_rect.width +
                                  0.5f);
  mapped.height = static_cast<int>(bbox.height *
                                       static_cast<float>(output_height) /
                                       crop_rect.height +
                                   0.5f);
  return mapped & cv::Rect(0, 0, output_width, output_height);
}

static int findPromptComponent(const cv::Mat& labels, cv::Point point,
                               int search_radius, float* distance) {
  int best_label = 0;
  int best_distance_sq = std::numeric_limits<int>::max();
  int min_x = std::max(0, point.x - search_radius);
  int max_x = std::min(labels.cols - 1, point.x + search_radius);
  int min_y = std::max(0, point.y - search_radius);
  int max_y = std::min(labels.rows - 1, point.y + search_radius);

  for (int y = min_y; y <= max_y; ++y) {
    for (int x = min_x; x <= max_x; ++x) {
      int label = labels.at<int>(y, x);
      if (label <= 0) {
        continue;
      }
      int dx = x - point.x;
      int dy = y - point.y;
      int distance_sq = dx * dx + dy * dy;
      if (distance_sq < best_distance_sq) {
        best_distance_sq = distance_sq;
        best_label = label;
      }
    }
  }
  if (distance != nullptr) {
    *distance = best_label > 0 ? std::sqrt(static_cast<float>(best_distance_sq))
                               : std::numeric_limits<float>::max();
  }
  return best_label;
}

static cv::Rect componentRect(const cv::Mat& stats, int label) {
  return cv::Rect(stats.at<int>(label, cv::CC_STAT_LEFT),
                  stats.at<int>(label, cv::CC_STAT_TOP),
                  stats.at<int>(label, cv::CC_STAT_WIDTH),
                  stats.at<int>(label, cv::CC_STAT_HEIGHT));
}

static bool buildMaskCandidate(
    const std::shared_ptr<ModelBoxSegmentationInfo>& obj_meta,
    int seg_index, cv::Point prompt_point, const cv::Rect& hint_bbox,
    FastSAMMaskCandidate* candidate) {
  if (!obj_meta || candidate == nullptr || seg_index < 0 ||
      seg_index >= static_cast<int>(obj_meta->box_seg.size())) {
    return false;
  }

  const int proto_height = static_cast<int>(obj_meta->mask_height);
  const int proto_width = static_cast<int>(obj_meta->mask_width);
  const auto& segment = obj_meta->box_seg[seg_index];
  if (proto_width <= 0 || proto_height <= 0 || segment.mask == nullptr ||
      obj_meta->image_width == 0 || obj_meta->image_height == 0) {
    return false;
  }

  cv::Mat proto_mask(proto_height, proto_width, CV_8UC1, segment.mask,
                     proto_width * sizeof(uint8_t));
  cv::Mat model_mask;
  cv::resize(proto_mask, model_mask,
             cv::Size(obj_meta->image_width, obj_meta->image_height),
             0, 0, cv::INTER_NEAREST);
  cv::threshold(model_mask, model_mask, 0, 255, cv::THRESH_BINARY);
  if (prompt_point.x < 0 || prompt_point.x >= model_mask.cols ||
      prompt_point.y < 0 || prompt_point.y >= model_mask.rows) {
    return false;
  }

  cv::Rect raw_bbox(
      static_cast<int>(std::floor(segment.x1)),
      static_cast<int>(std::floor(segment.y1)),
      static_cast<int>(std::ceil(segment.x2 - segment.x1)),
      static_cast<int>(std::ceil(segment.y2 - segment.y1)));
  raw_bbox &= cv::Rect(0, 0, model_mask.cols, model_mask.rows);
  if (raw_bbox.width <= 2 || raw_bbox.height <= 2) {
    return false;
  }
  int search_radius = std::max(
      6, std::min(24, static_cast<int>(
                          std::max(raw_bbox.width, raw_bbox.height) * 0.06f)));
  cv::Rect prompt_region(prompt_point.x - search_radius,
                         prompt_point.y - search_radius,
                         search_radius * 2 + 1, search_radius * 2 + 1);
  if ((raw_bbox & prompt_region).empty()) {
    return false;
  }

  cv::Mat roi_mask = model_mask(raw_bbox).clone();
  cv::morphologyEx(
      roi_mask, roi_mask, cv::MORPH_CLOSE,
      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

  cv::Mat labels;
  cv::Mat stats;
  cv::Mat centroids;
  int component_count = cv::connectedComponentsWithStats(
      roi_mask, labels, stats, centroids, 8, CV_32S);
  if (component_count <= 1) {
    return false;
  }

  cv::Point prompt_in_roi = prompt_point - raw_bbox.tl();
  float seed_distance = 0.0f;
  int seed_label = findPromptComponent(
      labels, prompt_in_roi, search_radius, &seed_distance);
  if (seed_label <= 0 || seed_label >= component_count ||
      seed_distance > search_radius) {
    return false;
  }

  cv::Rect selected_bbox = componentRect(stats, seed_label);
  int selected_area = stats.at<int>(seed_label, cv::CC_STAT_AREA);
  if (selected_area < 12 || selected_bbox.width <= 2 ||
      selected_bbox.height <= 2) {
    return false;
  }

  float raw_bbox_ratio = static_cast<float>(selected_bbox.area()) /
                         std::max(1, raw_bbox.area());
  if (raw_bbox_ratio < 0.03f) {
    return false;
  }
  selected_bbox.x += raw_bbox.x;
  selected_bbox.y += raw_bbox.y;
  selected_bbox |= cv::Rect(prompt_point.x, prompt_point.y, 1, 1);
  selected_bbox &= cv::Rect(0, 0, model_mask.cols, model_mask.rows);
  if (selected_bbox.width <= 2 || selected_bbox.height <= 2) {
    return false;
  }

  float prompt_proximity =
      1.0f - clampUnit(seed_distance / std::max(1, search_radius));
  float hint_iou = hint_bbox.empty() ? 0.0f
                                     : rectIou(selected_bbox, hint_bbox);
  float fill_support = clampUnit(
      static_cast<float>(selected_area) /
      std::max(1.0f, selected_bbox.area() * 0.5f));
  bool touches_border = selected_bbox.x <= 2 || selected_bbox.y <= 2 ||
                        selected_bbox.x + selected_bbox.width >=
                            model_mask.cols - 2 ||
                        selected_bbox.y + selected_bbox.height >=
                            model_mask.rows - 2;
  float crop_ratio = static_cast<float>(selected_bbox.area()) /
                     (model_mask.cols * model_mask.rows);
  float specificity = 1.0f - clampUnit(std::sqrt(crop_ratio));

  candidate->segment_index = seg_index;
  candidate->bbox = selected_bbox;
  candidate->model_score = clampUnit(segment.score);
  candidate->crop_ratio = crop_ratio;
  candidate->raw_bbox_ratio = raw_bbox_ratio;
  candidate->foreground_area = selected_area;
  candidate->seed_distance = seed_distance;
  candidate->touches_crop_border = touches_border;
  candidate->score = 1.75f * candidate->model_score +
                     1.25f * prompt_proximity + 1.75f * specificity +
                     0.5f * fill_support + 2.0f * hint_iou;
  if (touches_border) {
    candidate->score -= 1.25f;
  }
  if (crop_ratio > 0.65f) {
    candidate->score -= 3.0f;
  } else if (crop_ratio < 0.001f) {
    candidate->score -= 2.0f;
  }
  return true;
}

static int findBestMaskBboxContainingPoint(
    const std::shared_ptr<ModelBoxSegmentationInfo>& obj_meta,
    cv::Point point, const cv::Rect& hint_bbox,
    FastSAMMaskCandidate* best_candidate, int* candidate_count) {
  if (!obj_meta || best_candidate == nullptr || obj_meta->box_seg.empty()) {
    return -1;
  }

  std::vector<FastSAMMaskCandidate> candidates;
  for (uint32_t i = 0; i < obj_meta->box_seg.size(); ++i) {
    FastSAMMaskCandidate candidate;
    if (!buildMaskCandidate(obj_meta, static_cast<int>(i), point,
                            hint_bbox, &candidate)) {
      continue;
    }
    candidates.push_back(candidate);
  }

  if (candidate_count != nullptr) {
    *candidate_count = static_cast<int>(candidates.size());
  }
  if (candidates.empty()) {
    return -1;
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const FastSAMMaskCandidate& lhs,
               const FastSAMMaskCandidate& rhs) {
              if (std::fabs(lhs.score - rhs.score) > 0.001f) {
                return lhs.score > rhs.score;
              }
              return lhs.bbox.area() < rhs.bbox.area();
            });
  *best_candidate = candidates.front();
  int log_count = std::min(3, static_cast<int>(candidates.size()));
  for (int i = 0; i < log_count; ++i) {
    const FastSAMMaskCandidate& candidate = candidates[i];
    LOGI("FastSAM candidate[%d] seg=%d score=%.3f model=%.3f "
         "crop_ratio=%.4f raw_ratio=%.3f seed_distance=%.1f "
         "border=%d bbox=[%d,%d,%d,%d]",
         i, candidate.segment_index, candidate.score,
         candidate.model_score, candidate.crop_ratio,
         candidate.raw_bbox_ratio, candidate.seed_distance,
         candidate.touches_crop_border ? 1 : 0, candidate.bbox.x,
         candidate.bbox.y, candidate.bbox.width, candidate.bbox.height);
  }
  return 0;
}

static cv::Rect expandAndClampBbox(const cv::Rect& bbox, int width,
                                   int height) {
  int padding_x = static_cast<int>(bbox.width * kResultPaddingRatio + 0.5f);
  int padding_y = static_cast<int>(bbox.height * kResultPaddingRatio + 0.5f);
  cv::Rect expanded(bbox.x - padding_x, bbox.y - padding_y,
                    bbox.width + padding_x * 2,
                    bbox.height + padding_y * 2);
  return expanded & cv::Rect(0, 0, width, height);
}

struct FastSAMPassResult {
  cv::Rect bbox;
  FastSAMMaskCandidate candidate;
  int candidate_count = 0;
};

static int runFastSAMPass(const std::shared_ptr<BaseModel>& model,
                          const std::shared_ptr<BaseImage>& image,
                          const cv::Rect& crop_rect, cv::Point seed_point,
                          const cv::Rect* hint_bbox,
                          FastSAMPassResult* pass_result) {
  if (!model || !image || crop_rect.empty() || pass_result == nullptr) {
    return -1;
  }

  std::shared_ptr<BasePreprocessor> preprocessor = model->getPreprocessor();
  if (!preprocessor) {
    return -1;
  }
  std::shared_ptr<BaseImage> crop_image =
      preprocessor->crop(image, crop_rect.x, crop_rect.y,
                         crop_rect.width, crop_rect.height);
  if (!crop_image) {
    return -1;
  }

  std::vector<std::shared_ptr<BaseImage>> input_images = {crop_image};
  std::vector<std::shared_ptr<ModelOutputInfo>> out_datas;
  int inference_ret = model->inference(input_images, out_datas);
  if (inference_ret != 0 || out_datas.empty()) {
    return -1;
  }

  std::shared_ptr<ModelBoxSegmentationInfo> obj_meta =
      std::dynamic_pointer_cast<ModelBoxSegmentationInfo>(out_datas[0]);
  if (!obj_meta || obj_meta->box_seg.empty() || obj_meta->image_width == 0 ||
      obj_meta->image_height == 0) {
    return -1;
  }

  int output_width = static_cast<int>(obj_meta->image_width);
  int output_height = static_cast<int>(obj_meta->image_height);
  cv::Point seed_in_output(
      static_cast<int>((seed_point.x - crop_rect.x) *
                           static_cast<float>(output_width) /
                           crop_rect.width +
                       0.5f),
      static_cast<int>((seed_point.y - crop_rect.y) *
                           static_cast<float>(output_height) /
                           crop_rect.height +
                       0.5f));
  seed_in_output.x =
      std::max(0, std::min(seed_in_output.x, output_width - 1));
  seed_in_output.y =
      std::max(0, std::min(seed_in_output.y, output_height - 1));

  cv::Rect hint_in_output;
  if (hint_bbox != nullptr && hint_bbox->width > 0 &&
      hint_bbox->height > 0) {
    hint_in_output = mapRectToOutputSpace(
        *hint_bbox, crop_rect, output_width, output_height);
  }

  int ret = findBestMaskBboxContainingPoint(
      obj_meta, seed_in_output, hint_in_output, &pass_result->candidate,
      &pass_result->candidate_count);
  if (ret != 0) {
    return ret;
  }

  cv::Rect bbox_output = expandAndClampBbox(
      pass_result->candidate.bbox, output_width, output_height);
  pass_result->bbox.x =
      static_cast<int>(bbox_output.x * static_cast<float>(crop_rect.width) /
                           output_width +
                       0.5f) +
      crop_rect.x;
  pass_result->bbox.y =
      static_cast<int>(bbox_output.y * static_cast<float>(crop_rect.height) /
                           output_height +
                       0.5f) +
      crop_rect.y;
  pass_result->bbox.width = static_cast<int>(
      bbox_output.width * static_cast<float>(crop_rect.width) /
          output_width +
      0.5f);
  pass_result->bbox.height = static_cast<int>(
      bbox_output.height * static_cast<float>(crop_rect.height) /
          output_height +
      0.5f);
  pass_result->bbox &= cv::Rect(
      0, 0, static_cast<int>(image->getWidth()),
      static_cast<int>(image->getHeight()));
  if (pass_result->bbox.width <= 2 || pass_result->bbox.height <= 2 ||
      !pass_result->bbox.contains(seed_point)) {
    pass_result->bbox = cv::Rect();
    return -1;
  }
  return 0;
}

FastSAMSegmentor::FastSAMSegmentor(const std::string& model_path)
    : model_path_(model_path) {
  if (model_path_.empty()) {
    LOGE("FastSAMSegmentor: model_path is empty");
    return;
  }

  TDLModelFactory& model_factory = TDLModelFactory::getInstance();
  model_od_ = model_factory.getModel(ModelType::FASTSAM_SEG, model_path_);
  if (!model_od_) {
    LOGE("FastSAMSegmentor: get model failed");
    return;
  }
}

int FastSAMSegmentor::segment(std::shared_ptr<BaseImage> image,
                              cv::Point seed_point,
                              cvtdl_fastsam_result_t* result,
                              const cv::Rect* hint_bbox) {
  if (result == nullptr) {
    return -1;
  }
  result->success = false;
  if (!image) {
    return -1;
  }

  if (!model_od_) {
    result->success = false;
    return -1;
  }

  const int img_w = static_cast<int>(image->getWidth());
  const int img_h = static_cast<int>(image->getHeight());

  cv::Rect crop_rect = makeSquareCrop(
      img_w, img_h, seed_point, hint_bbox);
  if (crop_rect.empty() || !crop_rect.contains(seed_point)) {
    return -1;
  }

  FastSAMPassResult selected_pass;
  if (runFastSAMPass(model_od_, image, crop_rect, seed_point, hint_bbox,
                     &selected_pass) != 0) {
    LOGW("FastSAM has no foreground mask at seed=(%d,%d)",
         seed_point.x, seed_point.y);
    return -1;
  }

  cv::Rect selected_crop = crop_rect;
  bool retried = false;
  if (selected_pass.candidate.touches_crop_border &&
      crop_rect.width < std::min(img_w, img_h)) {
    int retry_size = std::min(
        std::min(img_w, img_h),
        std::max(crop_rect.width + 160, crop_rect.width * 3 / 2));
    cv::Rect retry_crop = makeSquareCropAt(
        img_w, img_h, seed_point, retry_size);
    FastSAMPassResult retry_pass;
    if (retry_crop != crop_rect &&
        runFastSAMPass(model_od_, image, retry_crop, seed_point, hint_bbox,
                       &retry_pass) == 0) {
      retried = true;
      if (!retry_pass.candidate.touches_crop_border ||
          retry_pass.candidate.score > selected_pass.candidate.score) {
        selected_pass = retry_pass;
        selected_crop = retry_crop;
      }
    }
  }

  result->bbox = selected_pass.bbox;
  if (result->bbox.width <= 2 || result->bbox.height <= 2 ||
      !result->bbox.contains(seed_point) ||
      result->bbox.area() >= selected_crop.area() * 9 / 10) {
    result->bbox = cv::Rect();
    return -1;
  }
  LOGI("FastSAM crop=[%d,%d,%d,%d] seed=[%d,%d] "
       "candidates=%d score=%.3f seed_distance=%.1f border=%d retry=%d "
       "result=[%d,%d,%d,%d]",
       selected_crop.x, selected_crop.y, selected_crop.width,
       selected_crop.height, seed_point.x, seed_point.y,
       selected_pass.candidate_count, selected_pass.candidate.score,
       selected_pass.candidate.seed_distance,
       selected_pass.candidate.touches_crop_border ? 1 : 0,
       retried ? 1 : 0, result->bbox.x, result->bbox.y,
       result->bbox.width, result->bbox.height);
  result->success = true;
  return 0;
}
