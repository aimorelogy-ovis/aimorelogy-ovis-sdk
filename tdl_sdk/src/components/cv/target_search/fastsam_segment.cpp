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
static const int kPromptTolerance = 4;
static const float kAmbiguousScoreDelta = 0.025f;
static const float kAmbiguousIouThreshold = 0.15f;

struct FastSAMMaskCandidate {
  int segment_index = -1;
  cv::Rect bbox;
  float score = -std::numeric_limits<float>::max();
  float model_score = 0.0f;
  float crop_ratio = 0.0f;
  float raw_bbox_ratio = 0.0f;
  float fill_ratio = 0.0f;
  int foreground_area = 0;
  float seed_distance = 0.0f;
  bool contains_prompt = false;
  bool touches_crop_border = false;
};

struct FastSAMRejectStats {
  int invalid = 0;
  int bbox_miss = 0;
  int no_component = 0;
  int prompt_miss = 0;
  int too_small = 0;
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

  if (hint_bbox != nullptr && hint_bbox->width > 0 && hint_bbox->height > 0) {
    int hint_extent = std::max(hint_bbox->width, hint_bbox->height);
    crop_size = std::max(crop_size,
                         static_cast<int>(hint_extent * kHintCropScale + 0.5f));
  }
  crop_size = std::min(crop_size, kMaximumDenseCropSize);
  return makeSquareCropAt(image_width, image_height, seed_point, crop_size);
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
    int seg_index, cv::Point prompt_point, FastSAMMaskCandidate* candidate,
    FastSAMRejectStats* reject_stats) {
  if (!obj_meta || candidate == nullptr || seg_index < 0 ||
      seg_index >= static_cast<int>(obj_meta->box_seg.size())) {
    if (reject_stats != nullptr) {
      reject_stats->invalid++;
    }
    return false;
  }

  const int proto_height = static_cast<int>(obj_meta->mask_height);
  const int proto_width = static_cast<int>(obj_meta->mask_width);
  const auto& segment = obj_meta->box_seg[seg_index];
  if (proto_width <= 0 || proto_height <= 0 || segment.mask == nullptr ||
      obj_meta->image_width == 0 || obj_meta->image_height == 0) {
    if (reject_stats != nullptr) {
      reject_stats->invalid++;
    }
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
    if (reject_stats != nullptr) {
      reject_stats->invalid++;
    }
    return false;
  }

  cv::Rect raw_bbox(
      static_cast<int>(std::floor(segment.x1)),
      static_cast<int>(std::floor(segment.y1)),
      static_cast<int>(std::ceil(segment.x2 - segment.x1)),
      static_cast<int>(std::ceil(segment.y2 - segment.y1)));
  raw_bbox &= cv::Rect(0, 0, model_mask.cols, model_mask.rows);
  if (raw_bbox.width <= 2 || raw_bbox.height <= 2) {
    if (reject_stats != nullptr) {
      reject_stats->invalid++;
    }
    return false;
  }
  cv::Rect prompt_region(prompt_point.x - kPromptTolerance,
                         prompt_point.y - kPromptTolerance,
                         kPromptTolerance * 2 + 1,
                         kPromptTolerance * 2 + 1);
  if ((raw_bbox & prompt_region).empty()) {
    if (reject_stats != nullptr) {
      reject_stats->bbox_miss++;
    }
    return false;
  }

  cv::Mat roi_mask = model_mask(raw_bbox).clone();

  cv::Mat labels;
  cv::Mat stats;
  cv::Mat centroids;
  int component_count = cv::connectedComponentsWithStats(
      roi_mask, labels, stats, centroids, 8, CV_32S);
  if (component_count <= 1) {
    if (reject_stats != nullptr) {
      reject_stats->no_component++;
    }
    return false;
  }

  cv::Point prompt_in_roi = prompt_point - raw_bbox.tl();
  float seed_distance = 0.0f;
  int seed_label = findPromptComponent(labels, prompt_in_roi,
                                       kPromptTolerance, &seed_distance);
  if (seed_label <= 0 || seed_label >= component_count ||
      seed_distance > kPromptTolerance) {
    if (reject_stats != nullptr) {
      reject_stats->prompt_miss++;
    }
    return false;
  }

  cv::Rect selected_bbox = componentRect(stats, seed_label);
  int selected_area = stats.at<int>(seed_label, cv::CC_STAT_AREA);
  if (selected_area < 12 || selected_bbox.width <= 2 ||
      selected_bbox.height <= 2) {
    if (reject_stats != nullptr) {
      reject_stats->too_small++;
    }
    return false;
  }

  float raw_bbox_ratio = static_cast<float>(selected_bbox.area()) /
                         std::max(1, raw_bbox.area());
  if (raw_bbox_ratio < 0.03f) {
    if (reject_stats != nullptr) {
      reject_stats->too_small++;
    }
    return false;
  }
  selected_bbox.x += raw_bbox.x;
  selected_bbox.y += raw_bbox.y;
  selected_bbox &= cv::Rect(0, 0, model_mask.cols, model_mask.rows);
  if (selected_bbox.width <= 2 || selected_bbox.height <= 2) {
    if (reject_stats != nullptr) {
      reject_stats->too_small++;
    }
    return false;
  }

  float fill_ratio = clampUnit(static_cast<float>(selected_area) /
                               std::max(1, selected_bbox.area()));
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
  candidate->fill_ratio = fill_ratio;
  candidate->foreground_area = selected_area;
  candidate->seed_distance = seed_distance;
  candidate->contains_prompt = seed_distance == 0.0f;
  candidate->touches_crop_border = touches_border;
  candidate->score = candidate->model_score + 0.20f * fill_ratio +
                     0.10f * specificity;
  if (candidate->contains_prompt) {
    candidate->score += 0.50f;
  } else {
    candidate->score -= 0.05f * candidate->seed_distance;
  }
  if (touches_border) {
    candidate->score -= 0.25f;
  }
  if (crop_ratio > 0.65f) {
    candidate->score -= 1.0f;
  } else if (crop_ratio < 0.001f) {
    candidate->score -= 0.5f;
  }
  return true;
}

static int findBestMaskBboxContainingPoint(
    const std::shared_ptr<ModelBoxSegmentationInfo>& obj_meta,
    cv::Point point, FastSAMMaskCandidate* best_candidate,
    int* candidate_count, bool* ambiguous) {
  if (!obj_meta || best_candidate == nullptr || obj_meta->box_seg.empty()) {
    return -1;
  }

  std::vector<FastSAMMaskCandidate> candidates;
  FastSAMRejectStats reject_stats;
  for (uint32_t i = 0; i < obj_meta->box_seg.size(); ++i) {
    FastSAMMaskCandidate candidate;
    if (!buildMaskCandidate(obj_meta, static_cast<int>(i), point, &candidate,
                            &reject_stats)) {
      continue;
    }
    candidates.push_back(candidate);
  }

  if (candidate_count != nullptr) {
    *candidate_count = static_cast<int>(candidates.size());
  }
  if (candidates.empty()) {
    LOGW("FastSAM candidates rejected: outputs=%zu invalid=%d bbox_miss=%d "
         "no_component=%d prompt_miss=%d too_small=%d",
         obj_meta->box_seg.size(), reject_stats.invalid,
         reject_stats.bbox_miss, reject_stats.no_component,
         reject_stats.prompt_miss, reject_stats.too_small);
    return -1;
  }

  std::sort(candidates.begin(), candidates.end(),
            [](const FastSAMMaskCandidate& lhs,
               const FastSAMMaskCandidate& rhs) {
              if (lhs.contains_prompt != rhs.contains_prompt) {
                return lhs.contains_prompt;
              }
              if (lhs.touches_crop_border != rhs.touches_crop_border) {
                return !lhs.touches_crop_border;
              }
              if (std::fabs(lhs.seed_distance - rhs.seed_distance) > 0.01f) {
                return lhs.seed_distance < rhs.seed_distance;
              }
              if (std::fabs(lhs.model_score - rhs.model_score) > 0.02f) {
                return lhs.model_score > rhs.model_score;
              }
              if (std::fabs(lhs.fill_ratio - rhs.fill_ratio) > 0.05f) {
                return lhs.fill_ratio > rhs.fill_ratio;
              }
              return lhs.bbox.area() < rhs.bbox.area();
            });
  *best_candidate = candidates.front();
  if (ambiguous != nullptr) {
    *ambiguous = false;
    if (candidates.size() > 1) {
      const FastSAMMaskCandidate& second = candidates[1];
      *ambiguous = best_candidate->contains_prompt == second.contains_prompt &&
                   best_candidate->touches_crop_border ==
                       second.touches_crop_border &&
                   std::fabs(best_candidate->seed_distance -
                             second.seed_distance) <= 1.0f &&
                   std::fabs(best_candidate->model_score -
                             second.model_score) <= kAmbiguousScoreDelta &&
                   rectIou(best_candidate->bbox, second.bbox) <
                       kAmbiguousIouThreshold;
    }
  }
  int log_count = std::min(3, static_cast<int>(candidates.size()));
  for (int i = 0; i < log_count; ++i) {
    const FastSAMMaskCandidate& candidate = candidates[i];
    LOGI("FastSAM candidate[%d] seg=%d score=%.3f model=%.3f "
         "crop_ratio=%.4f fill=%.3f seed_distance=%.1f exact=%d "
         "border=%d bbox=[%d,%d,%d,%d]",
         i, candidate.segment_index, candidate.score,
         candidate.model_score, candidate.crop_ratio, candidate.fill_ratio,
         candidate.seed_distance, candidate.contains_prompt ? 1 : 0,
         candidate.touches_crop_border ? 1 : 0, candidate.bbox.x,
         candidate.bbox.y, candidate.bbox.width, candidate.bbox.height);
  }
  if (ambiguous != nullptr && *ambiguous) {
    LOGW("FastSAM point is ambiguous between the top candidates");
  }
  return 0;
}

struct FastSAMPassResult {
  cv::Rect bbox;
  FastSAMMaskCandidate candidate;
  int candidate_count = 0;
  bool ambiguous = false;
};

static int runFastSAMPass(const std::shared_ptr<BaseModel>& model,
                          const std::shared_ptr<BaseImage>& image,
                          const cv::Rect& crop_rect, cv::Point seed_point,
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
    LOGW("FastSAM inference failed for crop=[%d,%d,%d,%d], ret=%d",
         crop_rect.x, crop_rect.y, crop_rect.width, crop_rect.height,
         inference_ret);
    return -1;
  }

  std::shared_ptr<ModelBoxSegmentationInfo> obj_meta =
      std::dynamic_pointer_cast<ModelBoxSegmentationInfo>(out_datas[0]);
  if (!obj_meta || obj_meta->box_seg.empty() || obj_meta->image_width == 0 ||
      obj_meta->image_height == 0) {
    LOGW("FastSAM produced no segments for crop=[%d,%d,%d,%d]",
         crop_rect.x, crop_rect.y, crop_rect.width, crop_rect.height);
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

  int ret = findBestMaskBboxContainingPoint(
      obj_meta, seed_in_output, &pass_result->candidate,
      &pass_result->candidate_count, &pass_result->ambiguous);
  if (ret != 0) {
    LOGW("FastSAM found no candidate at seed=[%d,%d] in crop=[%d,%d,%d,%d]",
         seed_point.x, seed_point.y, crop_rect.x, crop_rect.y,
         crop_rect.width, crop_rect.height);
    return ret;
  }

  const cv::Rect& bbox_output = pass_result->candidate.bbox;
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
  if (pass_result->bbox.width <= 2 || pass_result->bbox.height <= 2) {
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

  cv::Rect hint_crop = makeSquareCrop(img_w, img_h, seed_point, hint_bbox);
  if (hint_crop.empty() || !hint_crop.contains(seed_point)) {
    return -1;
  }

  int max_crop_size = std::min(kMaximumDenseCropSize,
                               std::min(img_w, img_h));
  std::vector<int> crop_sizes;
  crop_sizes.push_back(std::min(kMinimumCropSize, max_crop_size));
  crop_sizes.push_back(std::min(hint_crop.width, max_crop_size));
  crop_sizes.push_back(std::min(480, max_crop_size));
  crop_sizes.push_back(max_crop_size);
  std::sort(crop_sizes.begin(), crop_sizes.end());
  crop_sizes.erase(std::unique(crop_sizes.begin(), crop_sizes.end()),
                   crop_sizes.end());

  FastSAMPassResult selected_pass;
  cv::Rect selected_crop;
  bool has_candidate = false;
  bool retried = false;
  for (size_t i = 0; i < crop_sizes.size(); ++i) {
    cv::Rect crop_rect = makeSquareCropAt(
        img_w, img_h, seed_point, crop_sizes[i]);
    FastSAMPassResult pass;
    if (runFastSAMPass(model_od_, image, crop_rect, seed_point, &pass) != 0) {
      continue;
    }
    if (pass.ambiguous) {
      LOGW("FastSAM rejects ambiguous crop=[%d,%d,%d,%d]",
           crop_rect.x, crop_rect.y, crop_rect.width, crop_rect.height);
      continue;
    }
    if (pass.bbox.area() >= crop_rect.area() * 9 / 10) {
      LOGW("FastSAM target fills crop=[%d,%d,%d,%d], retrying wider view",
           crop_rect.x, crop_rect.y, crop_rect.width, crop_rect.height);
      continue;
    }

    selected_pass = pass;
    selected_crop = crop_rect;
    has_candidate = true;
    retried = i > 0;
    if (!pass.candidate.touches_crop_border) {
      break;
    }
  }

  if (!has_candidate) {
    LOGW("FastSAM has no unambiguous foreground mask at seed=(%d,%d)",
         seed_point.x, seed_point.y);
    return -1;
  }

  result->bbox = selected_pass.bbox;
  if (result->bbox.width <= 2 || result->bbox.height <= 2 ||
      result->bbox.area() >= selected_crop.area() * 9 / 10) {
    result->bbox = cv::Rect();
    LOGW("FastSAM rejected invalid result at seed=(%d,%d)",
         seed_point.x, seed_point.y);
    return -1;
  }
  LOGI("FastSAM crop=[%d,%d,%d,%d] seed=[%d,%d] "
       "candidates=%d score=%.3f seed_distance=%.1f exact=%d "
       "border=%d retry=%d "
       "result=[%d,%d,%d,%d]",
       selected_crop.x, selected_crop.y, selected_crop.width,
       selected_crop.height, seed_point.x, seed_point.y,
       selected_pass.candidate_count, selected_pass.candidate.score,
       selected_pass.candidate.seed_distance,
       selected_pass.candidate.contains_prompt ? 1 : 0,
       selected_pass.candidate.touches_crop_border ? 1 : 0,
       retried ? 1 : 0, result->bbox.x, result->bbox.y,
       result->bbox.width, result->bbox.height);
  result->success = true;
  return 0;
}
