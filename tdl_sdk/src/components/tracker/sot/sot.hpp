#include "cv/target_search/fastsam_segment.hpp"
#include "kalman_box_tracker.hpp"
#include "model/base_model.hpp"
#include "tdl_model_factory.hpp"
#include "tracker/tracker_types.hpp"

struct SOTInfo {
  std::vector<float> bbox;
  std::vector<float> kalman_bbox;
  std::vector<float> template_bbox;
  float score;
  float score_ratio;
  float iou;
  float size_ratio;
  float confidence_of_occluded;
  float confidence_of_reappear;
  uint64_t frame_id;
  bool is_occluded;
  bool is_reappear;
};

class SOT : public Tracker {
 public:
  SOT();
  ~SOT();

  int32_t setModel(std::shared_ptr<BaseModel> sot_model) override;

  int32_t initialize(const std::shared_ptr<BaseImage>& image,
                     const std::vector<ObjectBoxInfo>& detect_boxes,
                     const ObjectBoxInfo& bbox, uint64_t frame_id,
                     int frame_type,
                     const std::string& model_path = "") override;

  int32_t initialize(const std::shared_ptr<BaseImage>& image,
                     const std::vector<ObjectBoxInfo>& detect_boxes, float x,
                     float y, uint64_t frame_id, int frame_type,
                     const std::string& model_path = "") override;
  int32_t initializePoint(
      const std::shared_ptr<BaseImage>& image,
      const std::vector<ObjectBoxInfo>& detect_boxes, float x, float y,
      const ObjectBoxInfo* hint_bbox, uint64_t frame_id, int frame_type,
      const std::string& model_path = "") override;
  int32_t initialize(const std::shared_ptr<BaseImage>& image,
                     const std::vector<ObjectBoxInfo>& detect_boxes, int index,
                     uint64_t frame_id,
                     const std::string& model_path = "") override;
  int32_t track(const std::shared_ptr<BaseImage>& image, uint64_t frame_id,
                TrackerInfo& tracker_info) override;

  void setUseKalmanFilter(bool use) override { use_kalman_filter_ = use; }
  int32_t setScoreThreshold(float threshold) override;
  int32_t setSearchMotionHint(float dx, float dy, float confidence) override;
  int32_t setSearchExpansionHint(float motion_pixels,
                                 float confidence) override;
  int32_t getTargetInfo(ObjectBoxInfo* bbox,
                        bool* refined_small_target) override;
  int32_t getDiagnostics(
      SingleObjectTrackerDiagnostics* diagnostics) override;
  int32_t prepareTargetSearch(int frame_type,
                              const std::string& model_path) override;

 private:
  enum class TemplateProbationObservation {
    GOOD,
    NEUTRAL,
    BAD,
  };

  // 预处理图像，提取模板和搜索区域
  int32_t initBBox(const std::shared_ptr<BaseImage>& image,
                   const ObjectBoxInfo& init_bbox,
                   bool refined_small_target = false,
                   int initialization_search_type = 0);

  std::shared_ptr<BaseImage> preprocessReuse(
      const std::shared_ptr<BaseImage>& image, const std::vector<float>& bbox,
      float offset, int crop_size, std::vector<int>& context,
      std::shared_ptr<BaseImage>& reuse_image, int minimum_context_side = 0);

  bool calculateContext(const std::shared_ptr<BaseImage>& image,
                        const std::vector<float>& bbox, float offset,
                        std::vector<int>& context) const;

  bool calculateContextAtLeast(const std::shared_ptr<BaseImage>& image,
                               const std::vector<float>& bbox, float offset,
                               int minimum_context_side,
                               std::vector<int>& context) const;

  float calculateScoreRatio(float score) const;
  void commitReliableScore(float score);
  void updateReliableResponseBaseline(float score, float psr,
                                      float peak_margin);
  void resetReliableMotion();
  void observeReliableMotion(const std::vector<float>& bbox,
                             uint64_t frame_id);
  void resetRecoveryMotionValidation();
  void beginRecoveryMotionValidation(const std::vector<float>& bbox,
                                     uint64_t frame_id);
  bool observeRecoveryMotionCandidate(const std::vector<float>& bbox,
                                      uint64_t frame_id);
  void applyRecoveryTemplateQuarantine(uint64_t frame_id);
  bool candidateDirectionConsistent(
      const std::vector<float>& candidate,
      const std::vector<float>& predicted,
      float* longitudinal_error = nullptr,
      float* backward_gate = nullptr) const;
  void resetCandidateMotion();
  void observeCandidateMotion(const std::vector<float>& bbox,
                              uint64_t frame_id);
  bool buildCandidateMotionBridge(uint64_t frame_id, float image_width,
                                  float image_height,
                                  std::vector<float>* bbox);
  void resetRelockCandidate();
  void recordRelockVote(bool accepted);
  void recordRelockMiss();
  bool relockConfirmed(bool near_continuation) const;
  int templateRefreshFrames(float adaptive_target_factor) const;
  void recordTemplateRefreshVote(bool reliable);
  bool templateRefreshReady(float adaptive_target_factor) const;
  void recordTemplateEnvelopeSample(const std::vector<float>& bbox);
  bool buildTemplateRefreshEnvelope(
      const std::vector<float>& bbox,
      std::vector<float>* envelope) const;
  bool refreshTemplate(const std::shared_ptr<BaseImage>& image,
                       const std::vector<float>& bbox, uint64_t frame_id);
  void observeTemplateProbation(TemplateProbationObservation observation);
  void rollbackTemplate();
  void activateTemplate(bool use_long_term);
  void scheduleTemplateRecovery();
  void observeReliableTemplate(const std::shared_ptr<BaseImage>& image,
                               const std::vector<float>& bbox,
                               uint64_t frame_id,
                               float adaptive_target_factor,
                               bool refresh_quality_strong);
  void observeUnreliableTemplate(bool hard_lost = false);
  void resetScaleCandidate();
  void observeRawScaleBias(const std::vector<float>& raw_bbox,
                           const std::vector<float>& reference_bbox);
  std::vector<float> normalizeRawScaleBBox(
      const std::vector<float>& raw_bbox) const;
  bool observeTrustedScale(const std::vector<float>& raw_bbox,
                           bool reliable, float image_width,
                           float image_height);
  void resetApproachScale();
  void observeApproachScale(bool grow_observation);

  // 计算跟踪结果置信度
  void getStatus(const std::vector<float>& bbox,
                 const std::vector<float>& kalman_bbox, float score,
                 float score_ratio, float iou, float size_ratio);

  void ensureBBoxBoundaries(std::vector<float>& bbox,
                            const std::shared_ptr<BaseImage>& image);

  void clampBBox(std::vector<float>& bbox,
                 const std::shared_ptr<BaseImage>& image, int min_side = 3);

  // 模型
  std::shared_ptr<BaseModel> sot_model_;
  std::shared_ptr<FastSAMSegmentor> fastsam_segmentor_;  // FastSAM分割器实例
  // 卡尔曼滤波器
  std::shared_ptr<KalmanBoxTracker> kalman_tracker_;

  // 预处理器
  std::shared_ptr<BasePreprocessor> preprocessor_;

  // 模型参数
  int instance_size_ = 256;           // 实例大小
  int template_size_ = 128;           // 模板大小
  float template_bbox_offset_ = 0.2;  // 模板边界框偏移
  float search_bbox_offset_ = 2.0;    // 正常态搜索边界框偏移
  int kalman_update_count_ = 25;      // 卡尔曼开始更新的帧数
  float size_ratio_threshold_ = 1;    // 宽高比阈值
  float max_expand_ratio_ = 1.4;      // 目标丢失时最大外扩比例
  float tracking_score_threshold_ = 0.18f;  // FearTrack最低观测得分

  // 判断目标是否丢失相关参数
  float occluded_score_ratio_threshold_ = 0.9;  // 目标丢失时得分比率阈值
  float occluded_score_threshold_ = 0.6;        // 目标丢失时得分阈值
  float occluded_iou_threshold_ = 0.8;          // 目标丢失时IoU阈值
  float occluded_threshold_ = 0.3;              // 遮挡阈值

  // 判断目标是否重现相关参数
  float reappear_score_threshold_ = 0.3;      // 目标重现时得分阈值
  float reappear_score_ratio_threshold_ = 0.3;  // 目标重现时得分比率阈值
  float reappear_iou_threshold_ = 0.3;        // 目标重现时IoU阈值
  float reappear_threshold_ = 2;              // 重现阈值

  // 当前跟踪的目标边界框 [x, y, w, h]
  std::vector<float> current_bbox_;

  // 模板图像
  std::shared_ptr<BaseImage> template_image_;
  std::shared_ptr<BaseImage> long_term_template_image_;
  std::shared_ptr<BaseImage> adaptive_template_image_;
  std::shared_ptr<BaseImage> template_backup_image_;

  // 是否已初始化
  bool is_initialized_ = false;
  uint64_t frame_id_ = 0;
  uint64_t initialization_frame_id_ = 0;
  int initialization_search_type_ = 0;
  std::deque<float> score_lst_;
  float score_ratio_ = 1.0f;
  float last_observed_score_ = 1.0f;
  uint64_t last_template_update_frame_ = 0;
  uint64_t backup_template_update_frame_ = 0;
  uint32_t template_update_count_ = 0;
  uint32_t template_rollback_count_ = 0;
  int template_probation_frames_ = 0;
  int template_probation_good_frames_ = 0;
  int template_probation_bad_frames_ = 0;
  int template_probation_consecutive_bad_frames_ = 0;
  std::deque<bool> template_refresh_vote_window_;
  std::deque<std::vector<float>> template_bbox_history_;
  int template_refresh_good_votes_ = 0;
  int template_recovery_miss_frames_ = 0;
  int template_recovery_stable_frames_ = 0;
  int template_long_term_probe_frames_ = 0;
  int template_recovery_reset_frames_ = 0;
  bool template_recovery_long_term_tried_ = false;
  uint32_t template_switch_count_ = 0;
  bool using_long_term_template_ = false;
  uint64_t template_refresh_cooldown_until_frame_ = 0;
  bool handoff_identity_active_ = false;
  bool handoff_identity_confirmed_ = false;
  int handoff_identity_good_frames_ = 0;
  int handoff_identity_frames_ = 0;
  bool candidate_output_held_ = false;
  bool candidate_output_following_ = false;
  float handoff_candidate_distance_ = 0.0f;
  float handoff_candidate_area_ratio_ = 0.0f;
  float handoff_candidate_aspect_ratio_ = 0.0f;
  float prev_w_h_ratio_ = 0.0f;
  TrackStatus status_ = TrackStatus::TRACKED;
  std::vector<float> last_reliable_template_bbox_;
  std::vector<float> size_anchor_bbox_;
  std::vector<float> pending_scale_bbox_;
  std::vector<float> template_envelope_bbox_;
  std::vector<float> template_envelope_backup_bbox_;
  std::vector<float> search_prior_bbox_;
  std::vector<float> candidate_search_bbox_;
  std::vector<float> last_observed_candidate_bbox_;
  std::vector<float> jump_candidate_bbox_;
  std::vector<float> shadow_bbox_;
  std::deque<bool> shadow_vote_window_;
  bool search_prior_valid_ = false;
  float search_expansion_motion_ = 0.0f;
  int search_expansion_frames_ = 0;
  int candidate_search_frames_ = 0;
  int candidate_search_miss_frames_ = 0;
  int soft_follow_frames_ = 0;
  int soft_follow_recovery_frames_ = 0;
  int candidate_motion_bridge_frames_ = 0;
  float candidate_velocity_x_ = 0.0f;
  float candidate_velocity_y_ = 0.0f;
  uint64_t last_candidate_frame_id_ = 0;
  std::vector<float> last_reliable_motion_bbox_;
  float reliable_velocity_x_ = 0.0f;
  float reliable_velocity_y_ = 0.0f;
  uint64_t last_reliable_motion_frame_id_ = 0;
  int reliable_motion_stable_frames_ = 0;
  int stationary_reliable_frames_ = 0;
  std::vector<float> stationary_reference_bbox_;
  int camera_motion_guard_frames_ = 0;
  std::vector<float> recovery_motion_last_bbox_;
  uint64_t recovery_motion_last_frame_id_ = 0;
  float recovery_motion_reference_speed_ = 0.0f;
  float recovery_motion_candidate_velocity_x_ = 0.0f;
  float recovery_motion_candidate_velocity_y_ = 0.0f;
  int recovery_motion_observed_frames_ = 0;
  int recovery_motion_good_frames_ = 0;
  bool recovery_motion_validation_active_ = false;
  bool recovery_motion_stall_logged_ = false;
  bool template_recovery_quarantine_pending_ = false;
  int jump_candidate_frames_ = 0;
  int shadow_good_frames_ = 0;
  uint32_t relock_motion_reject_count_ = 0;
  float last_relock_candidate_distance_ = 0.0f;
  float last_relock_motion_gate_ = 0.0f;
  float last_response_score_ = 0.0f;
  float last_response_psr_ = 0.0f;
  float last_response_peak_margin_ = 0.0f;
  float last_response_area_ratio_ = 0.0f;
  float last_response_aspect_ratio_ = 0.0f;
  uint32_t last_response_reject_mask_ = 0;
  int pending_scale_mode_ = 0;
  int pending_scale_frames_ = 0;
  int scale_stable_frames_ = 0;
  uint32_t scale_update_count_ = 0;
  uint32_t scale_reject_count_ = 0;
  float last_raw_scale_ratio_ = 1.0f;
  float last_raw_scale_width_ratio_ = 1.0f;
  float last_raw_scale_height_ratio_ = 1.0f;
  float raw_scale_bias_width_ = 1.0f;
  float raw_scale_bias_height_ = 1.0f;
  uint32_t raw_scale_bias_samples_ = 0;
  bool scale_committed_this_frame_ = false;
  int approach_scale_frames_ = 0;
  int approach_scale_miss_frames_ = 0;
  bool approach_scale_active_ = false;
  bool last_grow_observation_ = false;
  bool last_approach_geometry_ok_ = false;
  bool last_approach_response_ok_ = false;
  float last_approach_raw_scale_ = 1.0f;
  float last_center_displacement_ = 0.0f;
  float last_scale_position_gate_ = 0.0f;
  uint64_t last_grow_commit_frame_ = 0;
  uint32_t bootstrap_active_ = 0;
  uint32_t bootstrap_retry_count_ = 0;
  uint32_t weak_follow_count_ = 0;
  uint32_t hard_lost_deferred_count_ = 0;
  uint32_t last_selected_candidate_ = 0;
  float last_search_target_pixels_ = 0.0f;
  uint32_t last_search_context_width_ = 0;
  uint32_t last_search_context_height_ = 0;
  bool preserve_aspect_context_ = false;
  bool lock_output_aspect_ = false;
  bool context_seed_ = false;
  bool last_low_detail_target_ = false;
  uint32_t kalman_search_horizon_ = 0;
  float kalman_search_dx_ = 0.0f;
  float kalman_search_dy_ = 0.0f;
  int unstable_frames_ = 0;
  int suspect_frames_ = 0;
  int lost_frames_ = 0;
  float reliable_score_ema_ = 0.0f;
  float reliable_psr_ema_ = 0.0f;
  float reliable_peak_margin_ema_ = 0.0f;
  uint32_t reliable_response_samples_ = 0;
  bool refined_small_target_ = false;

  std::vector<int> search_context_ = std::vector<int>(4, 0);
  std::vector<std::vector<std::shared_ptr<BaseImage>>> inference_images_ =
      std::vector<std::vector<std::shared_ptr<BaseImage>>>(
          1, std::vector<std::shared_ptr<BaseImage>>(2));
  std::vector<std::shared_ptr<ModelOutputInfo>> inference_outputs_;
  std::map<std::string, float> inference_params_ = {
      {"search_crop_x", 0.0f}, {"search_crop_y", 0.0f},
      {"search_crop_width", 0.0f}, {"search_crop_height", 0.0f}};

  // 中间结果
  SOTInfo sot_info_;
  bool use_kalman_filter_ = false;
};
