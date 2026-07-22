#include "sot.hpp"
#include <cmath>
#include <limits>
#include "cv/target_search/color_segment.hpp"
#include "cv/target_search/grabcut_segment.hpp"
#include "utils/mot_box_helper.hpp"
#include "utils/tdl_log.hpp"

namespace {
constexpr int kTargetSearchGrabCut = 1;
constexpr int kTargetSearchColor = 2;
constexpr int kTargetSearchFastSAM = 3;
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
  last_template_update_frame_ = 0;
  template_update_count_ = 0;
  prev_w_h_ratio_ = 0.0f;
  last_reliable_template_bbox_.clear();
  sot_info_ = SOTInfo{};

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
  is_initialized_ = true;
  LOGI("跟踪器初始化成功");
  return 0;
}

int32_t SOT::track(const std::shared_ptr<BaseImage>& image, uint64_t frame_id,
                   TrackerInfo& tracker_info) {
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
  float lost_expand = std::min(
      1.0f + static_cast<float>(lost_frames_) / 20.0f,
      max_expand_ratio_);
  std::vector<int> context;
  context.resize(4);
  if (!calculateContext(image, current_bbox_,
                        search_bbox_offset_ * lost_expand, context)) {
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

  int32_t ret =
      sot_model_->inference(input_images, output_datas, inference_params);
  if (ret != 0) {
    LOGE("跟踪模型推理失败: %#x", ret);
    return ret;
  }

  if (output_datas.empty()) {
    LOGE("跟踪结果为空");
    return -1;
  }

  std::shared_ptr<ModelBoxInfo> track_result =
      std::dynamic_pointer_cast<ModelBoxInfo>(output_datas[0]);
  if (!track_result) {
    LOGE("跟踪结果转换失败");
    return -1;
  }

  if (track_result->bboxes.empty()) {
    LOGD("track_result->bboxes.empty()\n");
    status_ = TrackStatus::LOST;
    lost_frames_++;
    frame_id_ = frame_id;
    tracker_info.status_ = TrackStatus::LOST;
    return 0;
  }

  float x1 = track_result->bboxes[0].x1;
  float y1 = track_result->bboxes[0].y1;
  float w = track_result->bboxes[0].x2 - x1;
  float h = track_result->bboxes[0].y2 - y1;

  float score = track_result->bboxes[0].score;

  std::vector<float> kalman_bbox;
  if (use_kalman_filter_ && kalman_tracker_) {
    kalman_bbox = kalman_tracker_->predict();
    if (frame_gap > 1) {
      for (uint64_t i = 1; i < frame_gap; i++) {
        kalman_tracker_->update(kalman_bbox, false);
        kalman_bbox = kalman_tracker_->predict();
      }
    }
  }

  // 计算缩放比例
  float w_scale = context[2] / static_cast<float>(instance_size_);
  float h_scale = context[3] / static_cast<float>(instance_size_);

  // 创建边界框
  std::vector<float> scaled_bbox = {x1 * w_scale + context[0],
                                    y1 * h_scale + context[1], w * w_scale,
                                    h * h_scale};
  clampBBox(scaled_bbox, image);

  // 几何异常检测：中心点位移、面积比、宽高比
  float prev_cx = current_bbox_[0] + current_bbox_[2] / 2;
  float prev_cy = current_bbox_[1] + current_bbox_[3] / 2;
  float new_cx = scaled_bbox[0] + scaled_bbox[2] / 2;
  float new_cy = scaled_bbox[1] + scaled_bbox[3] / 2;
  float center_displacement =
      std::sqrt((new_cx - prev_cx) * (new_cx - prev_cx) +
                (new_cy - prev_cy) * (new_cy - prev_cy));

  float prev_area = current_bbox_[2] * current_bbox_[3];
  float new_area = scaled_bbox[2] * scaled_bbox[3];
  float area_ratio = prev_area > 0 ? new_area / prev_area : 1.0f;

  float prev_aspect =
      current_bbox_[3] > 0 ? current_bbox_[2] / current_bbox_[3] : 1.0f;
  float new_aspect =
      scaled_bbox[3] > 0 ? scaled_bbox[2] / scaled_bbox[3] : 1.0f;
  float aspect_ratio = prev_aspect > 0 ? new_aspect / prev_aspect : 1.0f;

  float scale_limit = std::pow(1.25f, static_cast<float>(frame_gap));
  float displacement_limit =
      2.0f * std::max(current_bbox_[2], current_bbox_[3]) *
      static_cast<float>(frame_gap);
  bool geom_anomaly = center_displacement > displacement_limit ||
                      area_ratio > scale_limit ||
                      area_ratio < 1.0f / scale_limit ||
                      aspect_ratio > scale_limit ||
                      aspect_ratio < 1.0f / scale_limit;

  if (!use_kalman_filter_ || !kalman_tracker_) {
    if (!geom_anomaly && score >= tracking_score_threshold_) {
      current_bbox_ = scaled_bbox;
      last_reliable_template_bbox_ = scaled_bbox;
      lost_frames_ = 0;
      status_ = TrackStatus::TRACKED;
      tracker_info.box_info_.x1 = scaled_bbox[0];
      tracker_info.box_info_.y1 = scaled_bbox[1];
      tracker_info.box_info_.x2 = scaled_bbox[0] + scaled_bbox[2];
      tracker_info.box_info_.y2 = scaled_bbox[1] + scaled_bbox[3];
      tracker_info.box_info_.score = score;
      tracker_info.status_ = TrackStatus::TRACKED;
    } else {
      lost_frames_++;
      status_ = TrackStatus::LOST;
    }
    frame_id_ = frame_id;
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
  float current_w_h_ratio = scaled_bbox[2] / scaled_bbox[3];
  if (prev_w_h_ratio_ != 0) {
    size_ratio = current_w_h_ratio / prev_w_h_ratio_;
  } else {
    size_ratio = 1.0f;
  }

  getStatus(scaled_bbox, kalman_bbox, score, score_ratio_, iou, size_ratio);

  bool assert_occluded = geom_anomaly || score < 0.1f || iou < 0.2f;
  bool measurement_reliable = !assert_occluded && !sot_info_.is_occluded &&
                              score >= tracking_score_threshold_;
  if (status_ == TrackStatus::TRACKED && measurement_reliable) {
    kalman_tracker_->update(scaled_bbox, true);
    last_reliable_template_bbox_ = scaled_bbox;
    current_bbox_ = last_reliable_template_bbox_;
    sot_info_.template_bbox = last_reliable_template_bbox_;
    prev_w_h_ratio_ = current_w_h_ratio;
    lost_frames_ = 0;
    tracker_info.status_ = TrackStatus::TRACKED;
  } else {
    if (status_ == TrackStatus::LOST && !assert_occluded &&
        score >= tracking_score_threshold_ &&
        sot_info_.is_reappear) {
      status_ = TrackStatus::TRACKED;
      kalman_tracker_->update(scaled_bbox, true);
      last_reliable_template_bbox_ = scaled_bbox;
      current_bbox_ = last_reliable_template_bbox_;
      sot_info_.template_bbox = last_reliable_template_bbox_;
      prev_w_h_ratio_ = current_w_h_ratio;
      lost_frames_ = 0;
      tracker_info.status_ = TrackStatus::TRACKED;
    } else {
      status_ = TrackStatus::LOST;
      kalman_tracker_->update(kalman_bbox, false);
      clampBBox(kalman_bbox, image);
      current_bbox_ = kalman_bbox;
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
    tracker_info.box_info_.x1 = current_bbox_[0];
    tracker_info.box_info_.y1 = current_bbox_[1];
    tracker_info.box_info_.x2 = current_bbox_[0] + current_bbox_[2];
    tracker_info.box_info_.y2 = current_bbox_[1] + current_bbox_[3];
    tracker_info.box_info_.score = score;
  }
  frame_id_ = frame_id;
  LOGD("tracker_info.status_: %d, lost_frames_: %d\n", tracker_info.status_,
       lost_frames_);
  return 0;
}
