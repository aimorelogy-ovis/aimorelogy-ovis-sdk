#include "sot.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <numeric>
#include "cv/target_search/color_segment.hpp"
#include "cv/target_search/grabcut_segment.hpp"
#include "utils/mot_box_helper.hpp"
#include "utils/tdl_log.hpp"

namespace {
constexpr int kTargetSearchGrabCut = 1;
constexpr int kTargetSearchColor = 2;
constexpr int kTargetSearchFastSAM = 3;
constexpr int kSotHysteresisHoldFrames = 10;
constexpr int kSotCoastingOutputFrames = 12;
constexpr int kSotRelockConfirmFrames = 2;

uint64_t sot_time_us() {
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());
}

bool sot_bbox_valid(const std::vector<float>& bbox) {
  return bbox.size() >= 4 && bbox[2] > 1.0f && bbox[3] > 1.0f;
}

void sot_set_tracker_output(TrackerInfo* tracker_info,
                            const std::vector<float>& bbox, float score,
                            TrackStatus status) {
  if (tracker_info == nullptr || !sot_bbox_valid(bbox)) {
    return;
  }
  tracker_info->box_info_.x1 = bbox[0];
  tracker_info->box_info_.y1 = bbox[1];
  tracker_info->box_info_.x2 = bbox[0] + bbox[2];
  tracker_info->box_info_.y2 = bbox[1] + bbox[3];
  tracker_info->box_info_.score = score;
  tracker_info->status_ = status;
}

void sot_clamp_bbox_position(std::vector<float>& bbox, float image_width,
                             float image_height) {
  if (!sot_bbox_valid(bbox) || image_width <= 1.0f || image_height <= 1.0f) {
    return;
  }
  bbox[2] = std::min(bbox[2], image_width);
  bbox[3] = std::min(bbox[3], image_height);
  bbox[0] = std::max(0.0f, std::min(bbox[0], image_width - bbox[2]));
  bbox[1] = std::max(0.0f, std::min(bbox[1], image_height - bbox[3]));
}

float sot_bbox_area(const std::vector<float>& bbox) {
  return sot_bbox_valid(bbox) ? bbox[2] * bbox[3] : 0.0f;
}

float sot_bbox_center_distance2(const std::vector<float>& lhs,
                                const std::vector<float>& rhs) {
  if (!sot_bbox_valid(lhs) || !sot_bbox_valid(rhs)) {
    return std::numeric_limits<float>::max();
  }
  const float dx = lhs[0] + lhs[2] * 0.5f -
                   (rhs[0] + rhs[2] * 0.5f);
  const float dy = lhs[1] + lhs[3] * 0.5f -
                   (rhs[1] + rhs[3] * 0.5f);
  return dx * dx + dy * dy;
}

bool sot_bbox_consistent(const std::vector<float>& candidate,
                         const std::vector<float>& reference, bool loose) {
  const float candidate_area = sot_bbox_area(candidate);
  const float reference_area = sot_bbox_area(reference);
  if (candidate_area <= 1.0f || reference_area <= 1.0f) {
    return false;
  }

  const float area_ratio = candidate_area / reference_area;
  if (area_ratio < (loose ? 0.25f : 0.45f) ||
      area_ratio > (loose ? 3.0f : 2.0f)) {
    return false;
  }
  const float max_side = std::max(reference[2], reference[3]);
  const float max_distance =
      std::max(loose ? 28.0f : 14.0f,
               max_side * (loose ? 5.0f : 2.0f));
  return sot_bbox_center_distance2(candidate, reference) <=
         max_distance * max_distance;
}

void sot_stabilize_bbox_size(std::vector<float>& candidate,
                             const std::vector<float>& reference,
                             const std::vector<float>& size_anchor,
                             float score, float image_width,
                             float image_height) {
  if (!sot_bbox_valid(candidate) || !sot_bbox_valid(reference) ||
      !sot_bbox_valid(size_anchor)) {
    return;
  }

  const float center_x = candidate[0] + candidate[2] * 0.5f;
  const float center_y = candidate[1] + candidate[3] * 0.5f;
  const float bounded_w =
      std::max(size_anchor[2] * 0.72f,
               std::min(candidate[2], size_anchor[2] * 1.38f));
  const float bounded_h =
      std::max(size_anchor[3] * 0.72f,
               std::min(candidate[3], size_anchor[3] * 1.38f));
  const float learning_rate =
      0.06f + std::max(0.0f, std::min(score, 1.0f)) * 0.08f;

  candidate[2] = reference[2] +
                 (bounded_w - reference[2]) * learning_rate;
  candidate[3] = reference[3] +
                 (bounded_h - reference[3]) * learning_rate;
  candidate[2] = std::max(size_anchor[2] * 0.72f,
                          std::min(candidate[2], size_anchor[2] * 1.38f));
  candidate[3] = std::max(size_anchor[3] * 0.72f,
                          std::min(candidate[3], size_anchor[3] * 1.38f));
  candidate[0] = center_x - candidate[2] * 0.5f;
  candidate[1] = center_y - candidate[3] * 0.5f;
  sot_clamp_bbox_position(candidate, image_width, image_height);
}

float sot_dynamic_search_offset(float base_offset, TrackStatus status,
                                int unstable_frames, int lost_frames,
                                bool have_shadow, float max_expand_ratio) {
  float offset = base_offset;
  const float base_crop_ratio = 1.0f + 2.0f * base_offset;
  const float max_crop_ratio =
      base_crop_ratio * std::max(1.0f, max_expand_ratio);
  const float max_offset =
      std::max(base_offset, (max_crop_ratio - 1.0f) * 0.5f);

  if (status == TrackStatus::LOST || lost_frames > 0) {
    offset += std::min(1.25f, 0.35f + lost_frames * 0.12f);
  } else if (unstable_frames > 0) {
    offset += std::min(0.8f, unstable_frames * 0.25f);
  }
  if (have_shadow) {
    offset += 0.25f;
  }
  return std::min(offset, max_offset);
}
}  // namespace

SOT::SOT() {
  preprocessor_ =
      PreprocessorFactory::createPreprocessor(InferencePlatform::AUTOMATIC);
  // 初始化FastSAMSegmentor为nullptr
  fastsam_segmentor_ = nullptr;
}

SOT::~SOT() {}

void SOT::getStatus(const std::vector<float>& bbox,
                    const std::vector<float>& kalman_bbox, float score,
                    float score_ratio, float iou, float size_ratio) {
  sot_info_.bbox = bbox;
  sot_info_.kalman_bbox = kalman_bbox;
  sot_info_.score = score;
  sot_info_.score_ratio = score_ratio;
  sot_info_.iou = iou;
  sot_info_.size_ratio = size_ratio;
  float size_ratio_abs = std::abs(size_ratio - size_ratio_threshold_);
  // 实际得分与丢失得分阈值的差值
  float score_abs = std::max(occluded_score_threshold_ - score, 0.0f);
  // 实际得分比率与丢失得分比率阈值的差值
  float score_ratio_abs =
      std::max(occluded_score_ratio_threshold_ - score_ratio, 0.0f);
  // 实际IoU与丢失IoU阈值的差值
  float iou_abs = std::max(occluded_iou_threshold_ - iou, 0.0f);
  // 实际得分与重现得分阈值的差值
  float reappear_score_abs = std::max(score - reappear_score_threshold_, 0.0f);
  // 实际得分比率与重现得分比率阈值的差值
  float reappear_score_ratio_abs =
      std::max(score_ratio - reappear_score_ratio_threshold_, 0.0f);
  // 实际IoU与重现IoU阈值的差值
  float reappear_iou_abs = iou - reappear_iou_threshold_;
  float confidence_of_occluded = 0.1 * score_abs + 0.7 * score_ratio_abs +
                                 0.5 * iou_abs + 0.5 * size_ratio_abs;
  float confidence_of_reappear = 2 * reappear_score_abs +
                                 2 * reappear_score_ratio_abs +
                                 0.1 * reappear_iou_abs - 0.2 * size_ratio_abs;
  sot_info_.confidence_of_occluded = confidence_of_occluded;
  sot_info_.confidence_of_reappear = confidence_of_reappear;
  sot_info_.is_occluded = confidence_of_occluded > occluded_threshold_;
  sot_info_.is_reappear = confidence_of_reappear > reappear_threshold_;
}

void SOT::ensureBBoxBoundaries(std::vector<float>& bbox,
                               const std::shared_ptr<BaseImage>& image) {
  float img_w = image->getWidth();
  float img_h = image->getHeight();
  float x1 = bbox[0];
  float y1 = bbox[1];
  float x2 = bbox[0] + bbox[2];
  float y2 = bbox[1] + bbox[3];
  x1 = std::min(std::max(0.0f, x1), img_w);
  y1 = std::min(std::max(0.0f, y1), img_h);
  x2 = std::min(std::max(0.0f, x2), img_w);
  y2 = std::min(std::max(0.0f, y2), img_h);
  bbox[0] = x1;
  bbox[1] = y1;
  bbox[2] = x2 - x1;
  bbox[3] = y2 - y1;
}

void SOT::clampBBox(std::vector<float>& bbox,
                    const std::shared_ptr<BaseImage>& image, int min_side) {
  ensureBBoxBoundaries(bbox, image);
  float img_w = image->getWidth();
  float img_h = image->getHeight();
  if (bbox[2] < min_side) {
    bbox[2] = min_side;
    bbox[0] -= std::max(0.0f, bbox[0] + bbox[2] - img_w);
  }
  if (bbox[3] < min_side) {
    bbox[3] = min_side;
    bbox[1] -= std::max(0.0f, bbox[1] + bbox[3] - img_h);
  }
}

std::shared_ptr<BaseImage> SOT::preprocessReuse(
    const std::shared_ptr<BaseImage>& image, const std::vector<float>& bbox,
    float offset, int crop_size, std::vector<int>& context,
    std::shared_ptr<BaseImage>& reuse_image) {
  if (!calculateContext(image, bbox, offset, context)) {
    LOGE("预处理输入无效");
    return nullptr;
  }

  PreprocessParams params;
  memset(&params, 0, sizeof(PreprocessParams));
  params.dst_image_format = image->getImageFormat();
  params.dst_pixdata_type = image->getPixDataType();
  params.dst_width = crop_size;
  params.dst_height = crop_size;
  params.crop_x = context[0];
  params.crop_y = context[1];
  params.crop_width = context[2];
  params.crop_height = context[3];
  params.mean[0] = 0;
  params.mean[1] = 0;
  params.mean[2] = 0;
  params.scale[0] = 1;
  params.scale[1] = 1;
  params.scale[2] = 1;
  params.keep_aspect_ratio = false;
  if (!reuse_image ||
      reuse_image->getWidth() != static_cast<uint32_t>(crop_size) ||
      reuse_image->getHeight() != static_cast<uint32_t>(crop_size) ||
      reuse_image->getImageFormat() != params.dst_image_format ||
      reuse_image->getPixDataType() != params.dst_pixdata_type ||
      !reuse_image->isInitialized()) {
    reuse_image = ImageFactory::createImage(
        crop_size, crop_size, params.dst_image_format, params.dst_pixdata_type,
        true);
    if (!reuse_image) {
      LOGE("create reusable SOT image failed");
      return nullptr;
    }
  }

  if (preprocessor_->preprocessToImage(image, params, reuse_image) != 0) {
    LOGE("SOT preprocessToImage failed");
    return nullptr;
  }
  return reuse_image;
}

bool SOT::calculateContext(const std::shared_ptr<BaseImage>& image,
                           const std::vector<float>& bbox, float offset,
                           std::vector<int>& context) const {
  if (!image || bbox.size() < 4 || context.size() < 4) {
    return false;
  }

  const int width = image->getWidth();
  const int height = image->getHeight();
  const float x = bbox[0];
  const float y = bbox[1];
  const float w = bbox[2];
  const float h = bbox[3];
  if (width <= 0 || height <= 0 || w <= 0.0f || h <= 0.0f) {
    return false;
  }

  context[0] = static_cast<int>(x - w * offset);
  context[1] = static_cast<int>(y - h * offset);
  context[2] = static_cast<int>(w * (1.0f + 2.0f * offset));
  context[3] = static_cast<int>(h * (1.0f + 2.0f * offset));
  context[0] = std::max(0, std::min(context[0], width));
  context[1] = std::max(0, std::min(context[1], height));
  context[2] = std::max(0, std::min(context[2], width - context[0]));
  context[3] = std::max(0, std::min(context[3], height - context[1]));
  return context[2] > 0 && context[3] > 0;
}

void SOT::updateScoreLst(float score) {
  if (score_lst_.size() >= 10) {
    float avg_score =
        std::accumulate(score_lst_.begin(), score_lst_.end(), 0.0f) /
        score_lst_.size();
    score_ratio_ = avg_score > 0 ? score / avg_score : 0;
    if (score_ratio_ > occluded_score_ratio_threshold_) {
      score_lst_.pop_front();
      score_lst_.push_back(score);
    }
  } else {
    score_lst_.push_back(score);
  }
}

int32_t SOT::setModel(std::shared_ptr<BaseModel> sot_model) {
  if (!sot_model) {
    LOGE("sot_model is null");
    return -1;
  }
  sot_model_ = sot_model;
  return 0;
}

int32_t SOT::prepareTargetSearch(int frame_type,
                                 const std::string& model_path) {
  if (frame_type != kTargetSearchFastSAM) {
    return 0;
  }
  if (model_path.empty()) {
    LOGE("FastSAM模型路径为空");
    return -1;
  }
  if (fastsam_segmentor_ && fastsam_segmentor_->isReady()) {
    return 0;
  }

  std::shared_ptr<FastSAMSegmentor> segmentor =
      std::make_shared<FastSAMSegmentor>(model_path);
  if (!segmentor->isReady()) {
    LOGE("FastSAM模型准备失败");
    return -1;
  }
  fastsam_segmentor_ = segmentor;
  return 0;
}

int32_t SOT::setScoreThreshold(float threshold) {
  if (threshold < 0.0f || threshold > 1.0f) {
    LOGE("跟踪得分阈值超出范围: %.3f", threshold);
    return -1;
  }
  tracking_score_threshold_ = threshold;
  return 0;
}

int32_t SOT::setSearchMotionHint(float dx, float dy, float confidence) {
  if (!std::isfinite(dx) || !std::isfinite(dy) ||
      !std::isfinite(confidence) || confidence < 0.0f || confidence > 1.0f) {
    return -1;
  }
  if (confidence < 0.05f || !sot_bbox_valid(current_bbox_)) {
    return 0;
  }
  if (!sot_bbox_valid(search_prior_bbox_)) {
    search_prior_bbox_ = current_bbox_;
  }
  const float max_dx = std::max(12.0f, current_bbox_[2]);
  const float max_dy = std::max(12.0f, current_bbox_[3]);
  search_prior_bbox_[0] += std::max(-max_dx, std::min(dx, max_dx));
  search_prior_bbox_[1] += std::max(-max_dy, std::min(dy, max_dy));
  search_prior_valid_ = true;
  return 0;
}

void SOT::recordPerformance(uint64_t context_us, uint64_t model_us,
                            uint64_t map_us, uint64_t kalman_us,
                            uint64_t state_us, uint64_t total_us) {
  uint64_t now_us = sot_time_us();

  if (perf_window_start_us_ == 0) {
    perf_window_start_us_ = now_us;
  }
  perf_frames_++;
  perf_context_us_ += context_us;
  perf_model_us_ += model_us;
  perf_map_us_ += map_us;
  perf_kalman_us_ += kalman_us;
  perf_state_us_ += state_us;
  perf_total_us_ += total_us;
  if (now_us - perf_window_start_us_ < 1000ULL * 1000ULL) {
    return;
  }

  const double frames = static_cast<double>(perf_frames_);
  const double seconds =
      static_cast<double>(now_us - perf_window_start_us_) / 1000000.0;
  LOGI("SOT PERF fps=%.2f context=%.3fms model=%.3fms map=%.3fms "
       "kalman_predict=%.3fms state=%.3fms total=%.3fms",
       frames / seconds, perf_context_us_ / frames / 1000.0,
       perf_model_us_ / frames / 1000.0,
       perf_map_us_ / frames / 1000.0,
       perf_kalman_us_ / frames / 1000.0,
       perf_state_us_ / frames / 1000.0,
       perf_total_us_ / frames / 1000.0);
  perf_window_start_us_ = now_us;
  perf_frames_ = 0;
  perf_context_us_ = 0;
  perf_model_us_ = 0;
  perf_map_us_ = 0;
  perf_kalman_us_ = 0;
  perf_state_us_ = 0;
  perf_total_us_ = 0;
}

int32_t SOT::initialize(const std::shared_ptr<BaseImage>& image,
                        const std::vector<ObjectBoxInfo>& detect_boxes,
                        const ObjectBoxInfo& bbox, uint64_t frame_id,
                        int frame_type, const std::string& model_path) {
  frame_id_ = frame_id;
  // 如果检测框为空，直接使用目标框选算法
  if (detect_boxes.empty()) {
    if (frame_type == 1) {
      cv::Point seed;
      cvtdl_grabcut_result_t result;
      GrabCutSegmentor segmentor;
      seed.x = (bbox.x2 + bbox.x1) / 2;
      seed.y = (bbox.y2 + bbox.y1) / 2;
      int ret = segmentor.segment(image, seed, &result);
      if (ret != 0) {
        initBBox(image, bbox);
        return 0;
      }
      ObjectBoxInfo area_bbox;
      area_bbox.x1 = static_cast<float>(result.bbox.x);
      area_bbox.y1 = static_cast<float>(result.bbox.y);
      area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
      area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
      initBBox(image, area_bbox);
    } else if (frame_type == 2) {
      cv::Point seed;
      cvtdl_color_result_t result;
      ColorSegmentor segmentor;
      seed.x = (bbox.x2 + bbox.x1) / 2;
      seed.y = (bbox.y2 + bbox.y1) / 2;
      int ret = segmentor.segment(image, seed, &result);
      if (ret != 0) {
        initBBox(image, bbox);
        return 0;
      }
      ObjectBoxInfo area_bbox;
      area_bbox.x1 = static_cast<float>(result.bbox.x);
      area_bbox.y1 = static_cast<float>(result.bbox.y);
      area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
      area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
      initBBox(image, area_bbox);
    } else if (frame_type == 3) {
      cv::Point seed;
      cvtdl_fastsam_result_t result;
      // 使用成员变量fastsam_segmentor_
      if (!fastsam_segmentor_) {
        fastsam_segmentor_ = std::make_shared<FastSAMSegmentor>(model_path);
      }
      seed.x = (bbox.x2 + bbox.x1) / 2;
      seed.y = (bbox.y2 + bbox.y1) / 2;
      cv::Rect hint_bbox(
          static_cast<int>(bbox.x1), static_cast<int>(bbox.y1),
          static_cast<int>(bbox.x2 - bbox.x1),
          static_cast<int>(bbox.y2 - bbox.y1));
      int ret = fastsam_segmentor_->segment(
          image, seed, &result, &hint_bbox);
      if (ret != 0) {
        initBBox(image, bbox);
        return 0;
      }
      ObjectBoxInfo area_bbox;
      area_bbox.x1 = static_cast<float>(result.bbox.x);
      area_bbox.y1 = static_cast<float>(result.bbox.y);
      area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
      area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
      initBBox(image, area_bbox);
    } else {
      initBBox(image, bbox);
    }
    return 0;
  }

  // 查找与输入框有重叠的检测框（检测框中心点在输入框内）
  float bbox_cx = (bbox.x1 + bbox.x2) / 2.0f;
  float bbox_cy = (bbox.y1 + bbox.y2) / 2.0f;
  float min_distance_sq =
      std::numeric_limits<float>::max();  // 最小距离平方（避免开方运算）
  ObjectBoxInfo closest_box;
  bool found_closest = false;

  for (auto& detect_box : detect_boxes) {
    // 计算检测框中心点
    float db_cx = (detect_box.x1 + detect_box.x2) / 2.0f;
    float db_cy = (detect_box.y1 + detect_box.y2) / 2.0f;

    // 检查检测框中心点是否落入目标框内
    if (db_cx >= bbox.x1 && db_cx <= bbox.x2 && db_cy >= bbox.y1 &&
        db_cy <= bbox.y2) {
      // 计算中心点距离平方（避免sqrt提升效率）
      float dx = db_cx - bbox_cx;
      float dy = db_cy - bbox_cy;
      float distance_sq = dx * dx + dy * dy;

      // 跟踪最小距离的检测框
      if (distance_sq < min_distance_sq) {
        min_distance_sq = distance_sq;
        closest_box = detect_box;
        found_closest = true;
      }
    }
  }

  // 如果找到重叠的检测框，使用最近的检测框初始化
  if (found_closest) {
    initBBox(image, closest_box);
    return 0;
  }

  // 如果没有重叠，使用目标框选算法
  if (frame_type == 1) {
    cv::Point seed;
    cvtdl_grabcut_result_t result;
    GrabCutSegmentor segmentor;
    seed.x = (bbox.x2 + bbox.x1) / 2;
    seed.y = (bbox.y2 + bbox.y1) / 2;
    int ret = segmentor.segment(image, seed, &result);
    if (ret != 0) {
      initBBox(image, bbox);
      return 0;
    }
    ObjectBoxInfo area_bbox;
    area_bbox.x1 = static_cast<float>(result.bbox.x);
    area_bbox.y1 = static_cast<float>(result.bbox.y);
    area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
    area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
    LOGI("FastSAM selected target seed=[%d,%d] bbox=[%.1f,%.1f,%.1f,%.1f]",
         seed.x, seed.y, area_bbox.x1, area_bbox.y1,
         area_bbox.x2, area_bbox.y2);
    initBBox(image, area_bbox);
  } else if (frame_type == 2) {
    cv::Point seed;
    cvtdl_color_result_t result;
    ColorSegmentor segmentor;
    seed.x = (bbox.x2 + bbox.x1) / 2;
    seed.y = (bbox.y2 + bbox.y1) / 2;
    int ret = segmentor.segment(image, seed, &result);
    if (ret != 0) {
      initBBox(image, bbox);
      return 0;
    }
    ObjectBoxInfo area_bbox;
    area_bbox.x1 = static_cast<float>(result.bbox.x);
    area_bbox.y1 = static_cast<float>(result.bbox.y);
    area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
    area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
    initBBox(image, area_bbox);
  } else if (frame_type == 3) {
    cv::Point seed;
    cvtdl_fastsam_result_t result;
    if (!fastsam_segmentor_) {
      fastsam_segmentor_ = std::make_shared<FastSAMSegmentor>(model_path);
    }
    seed.x = (bbox.x2 + bbox.x1) / 2;
    seed.y = (bbox.y2 + bbox.y1) / 2;
    cv::Rect hint_bbox(
        static_cast<int>(bbox.x1), static_cast<int>(bbox.y1),
        static_cast<int>(bbox.x2 - bbox.x1),
        static_cast<int>(bbox.y2 - bbox.y1));
    int ret = fastsam_segmentor_->segment(
        image, seed, &result, &hint_bbox);
    if (ret != 0) {
      initBBox(image, bbox);
      return 0;
    }
    ObjectBoxInfo area_bbox;
    area_bbox.x1 = static_cast<float>(result.bbox.x);
    area_bbox.y1 = static_cast<float>(result.bbox.y);
    area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
    area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
    initBBox(image, area_bbox);
  } else {
    initBBox(image, bbox);
  }
  return 0;
}

int32_t SOT::initialize(const std::shared_ptr<BaseImage>& image,
                        const std::vector<ObjectBoxInfo>& detect_boxes, float x,
                        float y, uint64_t frame_id, int frame_type,
                        const std::string& model_path) {
  return initializePoint(image, detect_boxes, x, y, nullptr, frame_id,
                         frame_type, model_path);
}

int32_t SOT::initializePoint(
    const std::shared_ptr<BaseImage>& image,
    const std::vector<ObjectBoxInfo>& detect_boxes, float x, float y,
    const ObjectBoxInfo* hint_bbox, uint64_t frame_id, int frame_type,
    const std::string& model_path) {
  if (!image || x < 0.0f || y < 0.0f || x >= image->getWidth() ||
      y >= image->getHeight()) {
    LOGE("目标点超出图像范围");
    return -1;
  }
  frame_id_ = frame_id;

  // 查找包含点 (x, y) 的检测框
  for (auto& detect_box : detect_boxes) {
    if (detect_box.x1 <= x && detect_box.x2 >= x && detect_box.y1 <= y &&
        detect_box.y2 >= y) {
      return initBBox(image, detect_box);
    }
  }

  cv::Point seed(static_cast<int>(x), static_cast<int>(y));
  ObjectBoxInfo area_bbox;
  if (frame_type == kTargetSearchGrabCut) {
    cvtdl_grabcut_result_t result;
    GrabCutSegmentor segmentor;
    if (segmentor.segment(image, seed, &result) != 0) {
      LOGE("该位置没有可分割目标");
      return -1;
    }
    area_bbox.x1 = static_cast<float>(result.bbox.x);
    area_bbox.y1 = static_cast<float>(result.bbox.y);
    area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
    area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
  } else if (frame_type == kTargetSearchColor) {
    cvtdl_color_result_t result;
    ColorSegmentor segmentor;
    if (segmentor.segment(image, seed, &result) != 0) {
      LOGE("该位置没有可分割目标");
      return -1;
    }
    area_bbox.x1 = static_cast<float>(result.bbox.x);
    area_bbox.y1 = static_cast<float>(result.bbox.y);
    area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
    area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
  } else if (frame_type == kTargetSearchFastSAM) {
    if (prepareTargetSearch(frame_type, model_path) != 0) {
      return -1;
    }
    cvtdl_fastsam_result_t result;
    cv::Rect hint_rect;
    const cv::Rect* hint_rect_ptr = nullptr;
    if (hint_bbox != nullptr && hint_bbox->x2 > hint_bbox->x1 &&
        hint_bbox->y2 > hint_bbox->y1) {
      hint_rect = cv::Rect(
          static_cast<int>(hint_bbox->x1),
          static_cast<int>(hint_bbox->y1),
          static_cast<int>(hint_bbox->x2 - hint_bbox->x1),
          static_cast<int>(hint_bbox->y2 - hint_bbox->y1));
      hint_rect &= cv::Rect(0, 0, image->getWidth(), image->getHeight());
      if (!hint_rect.empty()) {
        hint_rect_ptr = &hint_rect;
      }
    }
    if (fastsam_segmentor_->segment(image, seed, &result, hint_rect_ptr) != 0) {
      LOGE("中心点没有FastSAM可分割目标");
      return -1;
    }
    area_bbox.x1 = static_cast<float>(result.bbox.x);
    area_bbox.y1 = static_cast<float>(result.bbox.y);
    area_bbox.x2 = static_cast<float>(result.bbox.x + result.bbox.width);
    area_bbox.y2 = static_cast<float>(result.bbox.y + result.bbox.height);
    LOGI("FastSAM selected point target seed=[%d,%d] "
         "bbox=[%.1f,%.1f,%.1f,%.1f]",
         seed.x, seed.y, area_bbox.x1, area_bbox.y1,
         area_bbox.x2, area_bbox.y2);
  } else {
    LOGE("目标点需要有效的分割方法");
    return -1;
  }

  return initBBox(image, area_bbox);
}

int32_t SOT::initialize(const std::shared_ptr<BaseImage>& image,
                        const std::vector<ObjectBoxInfo>& detect_boxes,
                        int index, uint64_t frame_id,
                        const std::string& model_path) {
  frame_id_ = frame_id;
  if (detect_boxes.empty()) {
    LOGE("no detection boxes");
    return -1;
  }

  if (detect_boxes.size() <= index) {
    LOGE("index out of range");
    return -1;
  }

  initBBox(image, detect_boxes[index]);
  return 0;
}

int32_t SOT::initBBox(const std::shared_ptr<BaseImage>& image,
                      const ObjectBoxInfo& init_bbox) {
  is_initialized_ = false;
  status_ = TrackStatus::TRACKED;
  lost_frames_ = 0;
  score_lst_.clear();
  score_ratio_ = 1.0f;
  last_observed_score_ = 1.0f;
  last_template_update_frame_ = 0;
  template_update_count_ = 0;
  prev_w_h_ratio_ = 0.0f;
  last_reliable_template_bbox_.clear();
  size_anchor_bbox_.clear();
  search_prior_bbox_.clear();
  shadow_bbox_.clear();
  search_prior_valid_ = false;
  shadow_good_frames_ = 0;
  unstable_frames_ = 0;
  sot_info_ = SOTInfo{};
  perf_window_start_us_ = 0;
  perf_frames_ = 0;
  perf_context_us_ = 0;
  perf_model_us_ = 0;
  perf_map_us_ = 0;
  perf_kalman_us_ = 0;
  perf_state_us_ = 0;
  perf_total_us_ = 0;

  float x = init_bbox.x1;
  float y = init_bbox.y1;
  float w = init_bbox.x2 - init_bbox.x1;
  float h = init_bbox.y2 - init_bbox.y1;
  std::vector<float> init_bbox_xywh_format = {x, y, w, h};
  std::vector<int> context;
  context.resize(4);
  clampBBox(init_bbox_xywh_format, image);
  x = init_bbox_xywh_format[0];
  y = init_bbox_xywh_format[1];
  w = init_bbox_xywh_format[2];
  h = init_bbox_xywh_format[3];
  current_bbox_ = {x, y, w, h};
  last_reliable_template_bbox_ = current_bbox_;
  size_anchor_bbox_ = current_bbox_;
  search_prior_bbox_ = current_bbox_;
  sot_info_.template_bbox = current_bbox_;
  sot_info_.frame_id = frame_id_;
  if (use_kalman_filter_) {
    kalman_tracker_ = std::make_shared<KalmanBoxTracker>(current_bbox_);
  }
  template_image_ =
      preprocessReuse(image, current_bbox_, template_bbox_offset_,
                      template_size_, context, template_image_);
  if (!template_image_) {
    LOGE("模板提取失败");
    return -1;
  }
  sot_model_->invalidateInputCache();
  init_diagnostic_frames_ = 5;
  is_initialized_ = true;
  LOGI("跟踪器初始化成功 bbox=[%.1f,%.1f,%.1f,%.1f] "
       "template_context=[%d,%d,%d,%d]",
       x, y, w, h, context[0], context[1], context[2], context[3]);
  return 0;
}

int32_t SOT::track(const std::shared_ptr<BaseImage>& image, uint64_t frame_id,
                   TrackerInfo& tracker_info) {
  const uint64_t total_start_us = sot_time_us();
  uint64_t context_start_us = 0;
  uint64_t context_us = 0;
  uint64_t model_start_us = 0;
  uint64_t model_us = 0;
  uint64_t map_start_us = 0;
  uint64_t map_us = 0;
  uint64_t kalman_start_us = 0;
  uint64_t kalman_us = 0;
  uint64_t state_start_us = 0;
  tracker_info.status_ = TrackStatus::LOST;
  tracker_info.box_info_.x1 = 0.0f;
  tracker_info.box_info_.y1 = 0.0f;
  tracker_info.box_info_.x2 = 0.0f;
  tracker_info.box_info_.y2 = 0.0f;
  tracker_info.box_info_.score = 0.0f;
  tracker_info.box_info_.class_id = 0;
  tracker_info.matched_times_ = 0;
  tracker_info.obj_idx_ = -1;
  tracker_info.pair_track_idx_ = 0;
  tracker_info.track_id_ = 1;
  tracker_info.velocity_x_ = 0.0f;
  tracker_info.velocity_y_ = 0.0f;
  tracker_info.blurness = 0.0f;
  if (!image) {
    LOGE("输入图像为空");
    return -1;
  }
  if (!is_initialized_ || !sot_model_ || current_bbox_.size() < 4) {
    LOGE("跟踪器尚未初始化");
    return -1;
  }

  uint64_t frame_gap = 1;
  if (frame_id > frame_id_) {
    frame_gap = std::min<uint64_t>(frame_id - frame_id_, 8);
  }
  context_start_us = sot_time_us();
  std::vector<int> context;
  context.resize(4);
  const bool use_search_prior =
      search_prior_valid_ && sot_bbox_valid(search_prior_bbox_);
  std::vector<float> search_bbox =
      use_search_prior ? search_prior_bbox_ : current_bbox_;
  sot_clamp_bbox_position(search_bbox, image->getWidth(), image->getHeight());
  if (use_search_prior) {
    search_prior_bbox_ = search_bbox;
  }
  const float search_offset = sot_dynamic_search_offset(
      search_bbox_offset_, status_, unstable_frames_, lost_frames_,
      sot_bbox_valid(shadow_bbox_), max_expand_ratio_);
  if (!calculateContext(image, search_bbox,
                        search_offset, context)) {
    LOGE("搜索区域计算失败");
    return -1;
  }

  std::vector<std::vector<std::shared_ptr<BaseImage>>> input_images = {
      {template_image_, image}};
  std::vector<std::shared_ptr<ModelOutputInfo>> output_datas;
  std::map<std::string, float> inference_params = {
      {"search_crop_x", static_cast<float>(context[0])},
      {"search_crop_y", static_cast<float>(context[1])},
      {"search_crop_width", static_cast<float>(context[2])},
      {"search_crop_height", static_cast<float>(context[3])}};
  context_us = sot_time_us() - context_start_us;

  model_start_us = sot_time_us();
  int32_t ret =
      sot_model_->inference(input_images, output_datas, inference_params);
  model_us = sot_time_us() - model_start_us;
  if (ret != 0) {
    LOGE("跟踪模型推理失败: %#x", ret);
    return ret;
  }

  if (output_datas.empty()) {
    LOGE("跟踪结果为空");
    return -1;
  }

  map_start_us = sot_time_us();
  std::shared_ptr<ModelBoxInfo> track_result =
      std::dynamic_pointer_cast<ModelBoxInfo>(output_datas[0]);
  if (!track_result) {
    LOGE("跟踪结果转换失败");
    return -1;
  }

  if (track_result->bboxes.empty()) {
    if (init_diagnostic_frames_ > 0) {
      LOGW("SOT initial result is empty frame=%llu remaining=%d",
           static_cast<unsigned long long>(frame_id),
           init_diagnostic_frames_);
      init_diagnostic_frames_--;
    }
    unstable_frames_++;
    std::vector<float> coasting_bbox = search_bbox;
    kalman_start_us = sot_time_us();
    if (use_kalman_filter_ && kalman_tracker_) {
      std::vector<float> predicted_bbox = kalman_tracker_->predict();
      if (frame_gap > 1) {
        for (uint64_t i = 1; i < frame_gap; i++) {
          kalman_tracker_->update(predicted_bbox, false);
          predicted_bbox = kalman_tracker_->predict();
        }
      }
      kalman_tracker_->update(predicted_bbox, false);
      if (!use_search_prior &&
          sot_bbox_consistent(predicted_bbox, search_bbox, true)) {
        coasting_bbox = predicted_bbox;
      }
    }
    kalman_us = sot_time_us() - kalman_start_us;
    if (status_ == TrackStatus::TRACKED &&
        unstable_frames_ <= kSotHysteresisHoldFrames &&
        sot_bbox_valid(last_reliable_template_bbox_)) {
      current_bbox_ = last_reliable_template_bbox_;
    } else {
      status_ = TrackStatus::LOST;
      lost_frames_++;
      if (sot_bbox_valid(last_reliable_template_bbox_)) {
        current_bbox_ = last_reliable_template_bbox_;
      }
    }
    sot_info_.template_bbox = current_bbox_;
    frame_id_ = frame_id;
    sot_clamp_bbox_position(coasting_bbox, image->getWidth(),
                            image->getHeight());
    if (unstable_frames_ <= kSotCoastingOutputFrames) {
      sot_set_tracker_output(&tracker_info, coasting_bbox,
                             last_observed_score_, TrackStatus::NEW);
    }
    recordPerformance(context_us, model_us, sot_time_us() - map_start_us,
                      kalman_us,
                      0, sot_time_us() - total_start_us);
    return 0;
  }

  float x1 = track_result->bboxes[0].x1;
  float y1 = track_result->bboxes[0].y1;
  float w = track_result->bboxes[0].x2 - x1;
  float h = track_result->bboxes[0].y2 - y1;
  float score = track_result->bboxes[0].score;
  float w_scale = context[2] / static_cast<float>(instance_size_);
  float h_scale = context[3] / static_cast<float>(instance_size_);
  std::vector<float> scaled_bbox = {x1 * w_scale + context[0],
                                    y1 * h_scale + context[1], w * w_scale,
                                    h * h_scale};
  clampBBox(scaled_bbox, image);
  sot_stabilize_bbox_size(scaled_bbox, search_bbox, size_anchor_bbox_, score,
                          image->getWidth(), image->getHeight());

  // 几何异常检测：中心点位移、面积比、宽高比
  float prev_cx = search_bbox[0] + search_bbox[2] / 2;
  float prev_cy = search_bbox[1] + search_bbox[3] / 2;
  float new_cx = scaled_bbox[0] + scaled_bbox[2] / 2;
  float new_cy = scaled_bbox[1] + scaled_bbox[3] / 2;
  float center_displacement =
      std::sqrt((new_cx - prev_cx) * (new_cx - prev_cx) +
                (new_cy - prev_cy) * (new_cy - prev_cy));

  float prev_area = search_bbox[2] * search_bbox[3];
  float new_area = scaled_bbox[2] * scaled_bbox[3];
  float area_ratio = prev_area > 0 ? new_area / prev_area : 1.0f;

  float prev_aspect =
      search_bbox[3] > 0 ? search_bbox[2] / search_bbox[3] : 1.0f;
  float new_aspect =
      scaled_bbox[3] > 0 ? scaled_bbox[2] / scaled_bbox[3] : 1.0f;
  float aspect_ratio = prev_aspect > 0 ? new_aspect / prev_aspect : 1.0f;

  float displacement_limit = 2.5f * std::max(search_bbox[2], 1.0f);
  bool geom_anomaly = center_displacement > displacement_limit ||
                      area_ratio > 2.2f || area_ratio < 0.35f ||
                      aspect_ratio > 2.0f || aspect_ratio < 0.5f;
  map_us = sot_time_us() - map_start_us;

  std::vector<float> kalman_bbox;
  kalman_start_us = sot_time_us();
  if (use_kalman_filter_ && kalman_tracker_) {
    kalman_bbox = kalman_tracker_->predict();
    if (frame_gap > 1) {
      for (uint64_t i = 1; i < frame_gap; i++) {
        kalman_tracker_->update(kalman_bbox, false);
        kalman_bbox = kalman_tracker_->predict();
      }
    }
  }
  kalman_us = sot_time_us() - kalman_start_us;
  state_start_us = sot_time_us();

  if (init_diagnostic_frames_ > 0) {
    LOGI("SOT initial frame=%llu score=%.3f threshold=%.3f "
         "bbox=[%.1f,%.1f,%.1f,%.1f] displacement=%.1f/%.1f "
         "area_ratio=%.3f aspect_ratio=%.3f anomaly=%d",
         static_cast<unsigned long long>(frame_id), score,
         tracking_score_threshold_, scaled_bbox[0], scaled_bbox[1],
         scaled_bbox[2], scaled_bbox[3], center_displacement,
         displacement_limit, area_ratio, aspect_ratio,
         geom_anomaly ? 1 : 0);
    init_diagnostic_frames_--;
  }

  if (!use_kalman_filter_ || !kalman_tracker_) {
    if (!geom_anomaly && score >= tracking_score_threshold_) {
      current_bbox_ = scaled_bbox;
      last_reliable_template_bbox_ = scaled_bbox;
      last_observed_score_ = score;
      search_prior_bbox_ = current_bbox_;
      search_prior_valid_ = false;
      lost_frames_ = 0;
      status_ = TrackStatus::TRACKED;
      tracker_info.box_info_.x1 = scaled_bbox[0];
      tracker_info.box_info_.y1 = scaled_bbox[1];
      tracker_info.box_info_.x2 = scaled_bbox[0] + scaled_bbox[2];
      tracker_info.box_info_.y2 = scaled_bbox[1] + scaled_bbox[3];
      tracker_info.box_info_.score = score;
      tracker_info.status_ = TrackStatus::TRACKED;
    } else {
      unstable_frames_++;
      lost_frames_++;
      status_ = TrackStatus::LOST;
      if (unstable_frames_ <= kSotCoastingOutputFrames) {
        sot_set_tracker_output(&tracker_info, search_bbox,
                               last_observed_score_, TrackStatus::NEW);
      }
    }
    frame_id_ = frame_id;
    recordPerformance(context_us, model_us, map_us, kalman_us,
                      sot_time_us() - state_start_us,
                      sot_time_us() - total_start_us);
    return 0;
  }

  float iou;
  if (kalman_tracker_->update_count_ > kalman_update_count_) {
    ObjectBoxInfo bbox1, bbox2;
    bbox1.x1 = scaled_bbox[0];
    bbox1.y1 = scaled_bbox[1];
    bbox1.x2 = scaled_bbox[0] + scaled_bbox[2];
    bbox1.y2 = scaled_bbox[1] + scaled_bbox[3];
    bbox2.x1 = kalman_bbox[0];
    bbox2.y1 = kalman_bbox[1];
    bbox2.x2 = kalman_bbox[0] + kalman_bbox[2];
    bbox2.y2 = kalman_bbox[1] + kalman_bbox[3];
    iou = MotBoxHelper::calculateIOU(bbox1, bbox2);
  } else {
    iou = 1.0f;
    kalman_bbox = scaled_bbox;
  }

  updateScoreLst(score);
  float size_ratio;
  float current_w_h_ratio =
      scaled_bbox[2] / std::max(scaled_bbox[3], 1.0f);
  if (prev_w_h_ratio_ != 0) {
    size_ratio = current_w_h_ratio / prev_w_h_ratio_;
  } else {
    size_ratio = 1.0f;
  }

  getStatus(scaled_bbox, kalman_bbox, score, score_ratio_, iou, size_ratio);

  const std::vector<float>& reliable_reference =
      use_search_prior
          ? search_bbox
          : (sot_bbox_valid(last_reliable_template_bbox_)
                 ? last_reliable_template_bbox_
                 : current_bbox_);
  const bool lost_state = status_ == TrackStatus::LOST || lost_frames_ > 0;
  const bool consistent_with_reliable =
      !geom_anomaly &&
      sot_bbox_consistent(scaled_bbox, reliable_reference, lost_state);
  const bool response_reliable =
      score >= tracking_score_threshold_ &&
      (score_lst_.size() < 10 || score_ratio_ >= 0.45f);
  if (status_ == TrackStatus::TRACKED) {
    if (response_reliable && consistent_with_reliable) {
      status_ = TrackStatus::TRACKED;
      kalman_tracker_->update(scaled_bbox, true);
      last_reliable_template_bbox_ = scaled_bbox;
      last_observed_score_ = score;
      shadow_bbox_.clear();
      shadow_good_frames_ = 0;
      unstable_frames_ = 0;
      current_bbox_ = last_reliable_template_bbox_;
      sot_info_.template_bbox = last_reliable_template_bbox_;
      search_prior_bbox_ = current_bbox_;
      search_prior_valid_ = false;
      prev_w_h_ratio_ = current_w_h_ratio;
      lost_frames_ = 0;
      tracker_info.status_ = TrackStatus::TRACKED;
    } else {
      unstable_frames_++;
      kalman_tracker_->update(kalman_bbox, false);
      current_bbox_ = reliable_reference;
      sot_info_.template_bbox = current_bbox_;
      tracker_info.status_ = TrackStatus::LOST;
      if (unstable_frames_ > kSotHysteresisHoldFrames) {
        status_ = TrackStatus::LOST;
        shadow_bbox_ = scaled_bbox;
        shadow_good_frames_ = 0;
        lost_frames_ = 1;
      }
    }
  } else {
    const bool have_shadow = sot_bbox_valid(shadow_bbox_);
    const std::vector<float>& shadow_reference =
        have_shadow ? shadow_bbox_ : reliable_reference;
    const bool shadow_candidate =
        response_reliable && !geom_anomaly &&
        sot_bbox_consistent(scaled_bbox, shadow_reference, !have_shadow);

    tracker_info.status_ = TrackStatus::LOST;
    if (shadow_candidate) {
      shadow_bbox_ = scaled_bbox;
      shadow_good_frames_++;
    } else {
      shadow_good_frames_ = 0;
    }

    if (shadow_good_frames_ >= kSotRelockConfirmFrames) {
      status_ = TrackStatus::TRACKED;
      kalman_tracker_->update(scaled_bbox, true);
      last_reliable_template_bbox_ = scaled_bbox;
      last_observed_score_ = score;
      current_bbox_ = scaled_bbox;
      sot_info_.template_bbox = current_bbox_;
      search_prior_bbox_ = current_bbox_;
      search_prior_valid_ = false;
      prev_w_h_ratio_ = current_w_h_ratio;
      shadow_bbox_.clear();
      shadow_good_frames_ = 0;
      unstable_frames_ = 0;
      lost_frames_ = 0;
      tracker_info.status_ = TrackStatus::TRACKED;
    } else {
      kalman_tracker_->update(kalman_bbox, false);
      current_bbox_ = reliable_reference;
      sot_info_.template_bbox = current_bbox_;
      lost_frames_++;
    }
  }

  if (geom_anomaly) {
    LOGD("geom_anomaly: displacement=%.2f limit=%.2f area_ratio=%.2f "
         "aspect_ratio=%.2f frame_gap=%llu\n",
         center_displacement, displacement_limit, area_ratio, aspect_ratio,
         static_cast<unsigned long long>(frame_gap));
  }

  if (tracker_info.status_ == TrackStatus::TRACKED) {
    sot_set_tracker_output(&tracker_info, current_bbox_, score,
                           TrackStatus::TRACKED);
  } else if (unstable_frames_ + lost_frames_ <=
                 kSotCoastingOutputFrames) {
    std::vector<float> coasting_bbox = search_bbox;
    if (!use_search_prior && sot_bbox_valid(kalman_bbox) &&
        sot_bbox_consistent(kalman_bbox, search_bbox, true)) {
      coasting_bbox = kalman_bbox;
    }
    sot_clamp_bbox_position(coasting_bbox, image->getWidth(),
                            image->getHeight());
    sot_set_tracker_output(&tracker_info, coasting_bbox,
                           last_observed_score_, TrackStatus::NEW);
  }
  frame_id_ = frame_id;
  LOGD("tracker_info.status_: %d, lost_frames_: %d\n", tracker_info.status_,
       lost_frames_);
  recordPerformance(context_us, model_us, map_us, kalman_us,
                    sot_time_us() - state_start_us,
                    sot_time_us() - total_start_us);
  return 0;
}
