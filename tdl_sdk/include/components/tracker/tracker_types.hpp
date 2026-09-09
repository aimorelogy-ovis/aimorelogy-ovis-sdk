#ifndef TDL_SDK_TRACKER_TYPES_HPP
#define TDL_SDK_TRACKER_TYPES_HPP

#include <cstdint>
#include <memory>
#include <string>
#include "common/model_output_types.hpp"
#include "common/object_type_def.hpp"
#include "image/base_image.hpp"
#include "model/base_model.hpp"

enum class TrackStatus {
  NEW = 0,
  TRACKED,
  LOST,
  REMOVED,
  CANDIDATE,
  FOLLOWING
};
class TrackerInfo {
 public:
  TrackerInfo() = default;
  ~TrackerInfo() = default;

  ObjectBoxInfo box_info_;
  TrackStatus status_;
  int matched_times_;
  // if -1, it is not matched with current detection
  int obj_idx_;
  uint64_t pair_track_idx_;
  uint64_t track_id_;
  float velocity_x_;
  float velocity_y_;
  float blurness;
};
class TrackerConfig {
 public:
  int max_unmatched_times_ = 15;
  int track_confirmed_frames_ = 3;
  int track_pair_update_missed_times_ = 2;
  float track_init_score_thresh_ = 0.6;
  float high_score_thresh_ = 0.5;
  float high_score_iou_dist_thresh_ = 0.7;
  float low_score_iou_dist_thresh_ = 0.5;
};

struct SingleObjectTrackerDiagnostics {
  uint32_t template_update_count = 0;
  uint32_t template_rollback_count = 0;
  uint32_t template_probation_frames = 0;
  uint64_t template_age_frames = 0;
  uint32_t template_refresh_votes = 0;
  uint32_t template_refresh_window = 0;
  uint32_t template_using_long_term = 0;
  uint32_t template_switch_count = 0;
  uint32_t template_recovery_stable_frames = 0;
  uint32_t kalman_search_horizon = 0;
  float kalman_search_dx = 0.0f;
  float kalman_search_dy = 0.0f;
  uint32_t relock_votes = 0;
  uint32_t relock_good_votes = 0;
  uint32_t relock_motion_rejects = 0;
  float relock_candidate_distance = 0.0f;
  float relock_motion_gate = 0.0f;
  float response_score = 0.0f;
  float response_psr = 0.0f;
  float response_peak_margin = 0.0f;
  float response_area_ratio = 0.0f;
  float response_aspect_ratio = 0.0f;
  uint32_t response_reject_mask = 0;
  uint32_t bootstrap_active = 0;
  uint32_t bootstrap_retry_count = 0;
  uint32_t weak_follow_count = 0;
  uint32_t hard_lost_deferred_count = 0;
  uint32_t low_detail_target = 0;
  uint32_t response_selected_candidate = 0;
  float response_psr_baseline = 0.0f;
  float response_peak_margin_baseline = 0.0f;
  float search_target_pixels = 0.0f;
  uint32_t template_recovery_attempted = 0;
  uint32_t handoff_identity_active = 0;
  uint32_t handoff_identity_confirmed = 0;
  uint32_t handoff_identity_frames = 0;
  uint32_t handoff_identity_good_frames = 0;
  uint32_t candidate_output_held = 0;
  uint32_t candidate_output_following = 0;
  float candidate_follow_dx = 0.0f;
  float candidate_follow_dy = 0.0f;
  float handoff_candidate_distance = 0.0f;
  float handoff_candidate_area_ratio = 0.0f;
  float handoff_candidate_aspect_ratio = 0.0f;
  uint32_t scale_update_count = 0;
  uint32_t scale_reject_count = 0;
  uint32_t scale_pending_frames = 0;
  uint32_t scale_stable_frames = 0;
  float scale_raw_ratio = 1.0f;
  float scale_raw_width_ratio = 1.0f;
  float scale_raw_height_ratio = 1.0f;
  uint32_t preserve_aspect_context = 0;
  uint32_t search_context_width = 0;
  uint32_t search_context_height = 0;
  float scale_raw_bias_width = 1.0f;
  float scale_raw_bias_height = 1.0f;
  uint32_t scale_raw_bias_samples = 0;
  float scale_trusted_width = 0.0f;
  float scale_trusted_height = 0.0f;
  uint32_t scale_approach_active = 0;
  uint32_t scale_approach_frames = 0;
};

enum class TrackerType {
  TDL_MOT_SORT = 0,
  TDL_SOT = 1,
};
class Tracker {
 public:
  Tracker() = default;
  ~Tracker() = default;

  virtual int32_t setModel(std::shared_ptr<BaseModel> sot_model);

  virtual int32_t initialize(const std::shared_ptr<BaseImage>& image,
                             const std::vector<ObjectBoxInfo>& detect_boxes,
                             const ObjectBoxInfo& bbox, uint64_t frame_id,
                             int frame_type,
                             const std::string& model_path = "");

  virtual int32_t initialize(const std::shared_ptr<BaseImage>& image,
                             const std::vector<ObjectBoxInfo>& detect_boxes,
                             float x, float y, uint64_t frame_id,
                             int frame_type,
                             const std::string& model_path = "");

  virtual int32_t initializePoint(
      const std::shared_ptr<BaseImage>& image,
      const std::vector<ObjectBoxInfo>& detect_boxes, float x, float y,
      const ObjectBoxInfo* hint_bbox, uint64_t frame_id, int frame_type,
      const std::string& model_path = "");

  virtual int32_t initialize(const std::shared_ptr<BaseImage>& image,
                             const std::vector<ObjectBoxInfo>& detect_boxes,
                             int index, uint64_t frame_id,
                             const std::string& model_path = "");

  virtual void setPairConfig(
      std::map<TDLObjectType, TDLObjectType> object_pair_config){};

  virtual void setUseKalmanFilter(bool use) {}
  virtual int32_t setScoreThreshold(float threshold);
  virtual int32_t setSearchMotionHint(float dx, float dy, float confidence) {
    (void)dx;
    (void)dy;
    (void)confidence;
    return -1;
  }
  virtual int32_t setSearchExpansionHint(float motion_pixels,
                                         float confidence) {
    (void)motion_pixels;
    (void)confidence;
    return -1;
  }

  virtual int32_t getTargetInfo(ObjectBoxInfo* bbox,
                                bool* refined_small_target);

  virtual int32_t getDiagnostics(
      SingleObjectTrackerDiagnostics* diagnostics);

  virtual int32_t prepareTargetSearch(int frame_type,
                                      const std::string& model_path);

  void setTrackConfig(const TrackerConfig& track_config);

  TrackerConfig getTrackConfig();

  virtual int32_t track(std::vector<ObjectBoxInfo>& boxes, uint64_t frame_id,
                        std::vector<TrackerInfo>& trackers);

  virtual int32_t track(const std::shared_ptr<BaseImage>& image,
                        uint64_t frame_id, TrackerInfo& tracker_info);

  void setImgSize(int width, int height);

 protected:
  int img_width_ = 0;
  int img_height_ = 0;
  TrackerConfig tracker_config_;
};

class TrackerFactory {
 public:
  static std::shared_ptr<Tracker> createTracker(TrackerType type);
};

#endif /* TDL_SDK_TRACKER_TYPES_HPP */
