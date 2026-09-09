#include "sot.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
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
constexpr int kSotCoastingOutputFrames = 12;
constexpr std::size_t kSotRelockVoteWindow = 4;
constexpr int kSotRelockVotesRequired = 3;
constexpr std::size_t kSotNearRelockVoteWindow = 3;
constexpr int kSotNearRelockVotesRequired = 2;
constexpr int kSotSuspectConfirmFrames = 2;
constexpr int kSotSmallTargetHardLostConfirmFrames = 4;
constexpr uint32_t kSotResponseBaselineMinSamples = 5;
constexpr float kSotMinResponsePsr = 2.0f;
constexpr float kSotMinPeakMargin = 0.02f;
constexpr float kSotRelockMinResponsePsr = 2.5f;
constexpr float kSotRelockMinPeakMargin = 0.03f;
constexpr float kSotRelockMinScoreRatio = 0.75f;
constexpr float kSotResponseBaselineAlpha = 0.1f;
constexpr float kSotResponseBaselineMaxPsr = 12.0f;
constexpr float kSotResponseBaselineMaxPeakMargin = 0.12f;
constexpr float kSotTrackedBaselineScoreRatio = 0.78f;
constexpr float kSotTrackedBaselinePsrRatio = 0.72f;
constexpr float kSotTrackedBaselineMarginRatio = 0.65f;
constexpr float kSotMotionBaselinePsrRatio = 0.55f;
constexpr float kSotMotionBaselineMarginRatio = 0.50f;
constexpr float kSotRelockBaselineScoreRatio = 0.78f;
constexpr float kSotRelockBaselinePsrRatio = 0.72f;
constexpr float kSotRelockBaselineMarginRatio = 0.65f;
constexpr float kSotSmallTargetRelockScoreRatio = 0.55f;
constexpr float kSotSmallTargetRelockBaselineScoreRatio = 0.60f;
constexpr float kSotSmallTargetRelockBaselinePsrRatio = 0.55f;
constexpr float kSotSmallTargetRelockBaselineMarginRatio = 0.50f;
constexpr float kSotTinyTargetSide = 16.0f;
constexpr float kSotAdaptiveTargetSide = 80.0f;
constexpr float kSotLowDetailTargetSide = 144.0f;
constexpr float kSotContextAmount = 0.5f;
constexpr float kSotMinimumSearchOffset = 1.5f;
constexpr float kSotTrackedMinimumTargetPixels = 28.0f;
constexpr float kSotRecoveryMinimumTargetPixels = 24.0f;
constexpr uint32_t kSotBootstrapFrames = 8;
constexpr float kSotBootstrapMinimumScore = 0.06f;
constexpr float kSotBootstrapRetryTargetPixels = 24.0f;
constexpr float kSotBootstrapWideTargetPixels = 16.0f;
constexpr uint32_t kSotRejectEmpty = 1u << 0;
constexpr uint32_t kSotRejectResponse = 1u << 1;
constexpr uint32_t kSotRejectAmbiguous = 1u << 2;
constexpr uint32_t kSotRejectGeometry = 1u << 3;
constexpr uint32_t kSotRejectConsistency = 1u << 4;
constexpr uint32_t kSotRejectRelockQuality = 1u << 5;
constexpr uint32_t kSotRejectRelockMotion = 1u << 6;
constexpr uint32_t kSotRejectRelockDirection = 1u << 7;
constexpr float kSotCandidateScoreFloorRatio = 0.65f;
constexpr float kSotCandidateMinimumScore = 0.08f;
constexpr float kSotCandidateMinimumPsr = 1.0f;
constexpr float kSotCandidateMinimumPeakMargin = 0.01f;
constexpr float kSotTopKMinimumScoreRatio = 0.45f;
constexpr float kSotTopKReplacementMargin = 0.12f;
constexpr int kSotRefinedSmallTargetTemplateContext = 32;
constexpr int kSotSmallTargetSuspectConfirmFrames = 5;
constexpr int kSotCandidateSearchMaxFrames = 3;
constexpr int kSotCandidateSearchBridgeFrames = 2;
constexpr int kSotSoftFollowMaxFrames = 30;
constexpr int kSotSoftFollowRecoveryFrames = 3;
constexpr int kSotCandidateMotionBridgeFrames = 2;
constexpr float kSotCandidateMotionMinPixels = 0.75f;
constexpr float kSotCandidateMotionMaxTargetRatio = 0.45f;
constexpr int kSotInitialTemplateRefreshFrames = 20;
constexpr int kSotMinimumTemplateRefreshFrames = 24;
constexpr int kSotMaximumTemplateRefreshFrames = 30;
constexpr float kSotTemplateRefreshVoteRatio = 0.75f;
constexpr std::size_t kSotTemplateRefreshVoteWindow = 32;
constexpr int kSotTemplateProbationFrames = 8;
constexpr int kSotTemplateProbationMaxFrames = 12;
constexpr int kSotTemplateProbationGoodFrames = 6;
constexpr int kSotTemplateProbationConsecutiveBadFrames = 3;
constexpr int kSotTemplateRecoverySwitchFrames = 4;
constexpr int kSotTemplateLongTermProbeFrames = 3;
constexpr int kSotTemplateRecoveryResetFrames = 15;
constexpr int kSotTemplateRecoveryStableFrames = 15;
constexpr std::size_t kSotTemplateEnvelopeHistory = 24;
constexpr std::size_t kSotTemplateEnvelopeMinimumSamples = 10;
constexpr float kSotTemplateEnvelopePercentile = 0.65f;
constexpr float kSotTemplateEnvelopeMinimumRatio = 0.78f;
constexpr float kSotTemplateEnvelopeMaximumRatio = 1.25f;
constexpr float kSotTemplateEnvelopeScaleStep = 0.12f;
constexpr uint64_t kSotTemplateRefreshCooldownFrames = 30;
constexpr uint64_t kSotLowDetailTemplateRefreshCooldownFrames = 45;
constexpr float kSotTemplateRefreshMinimumTargetSide = 24.0f;
constexpr float kSotTemplateRefreshMinimumScore = 0.30f;
constexpr float kSotScaleDeadband = 0.02f;
constexpr float kSotScaleMinimumRawRatio = 0.55f;
constexpr float kSotScaleMaximumRawRatio = 1.80f;
constexpr float kSotScalePendingConsistency = 0.12f;
constexpr int kSotScaleShrinkConfirmFrames = 2;
constexpr int kSotScaleGrowConfirmFrames = 2;
constexpr int kSotScaleReshapeConfirmFrames = 3;
constexpr float kSotScaleShrinkStep = 0.90f;
constexpr float kSotScaleGrowStep = 1.10f;
constexpr float kSotScaleAspectStep = 1.10f;
constexpr int kSotApproachConfirmFrames = 3;
constexpr int kSotApproachBreakFrames = 8;
constexpr int kSotApproachHoldFrames = 20;
constexpr int kSotApproachShrinkHoldFrames = 20;
constexpr float kSotApproachHoldRawRatio = 1.00f;
constexpr float kSotApproachMinimumRawRatio = 1.06f;
constexpr float kSotApproachMaximumRawRatio = 3.20f;
constexpr float kSotApproachGrowStep = 1.12f;
constexpr int kSotApproachGrowConfirmFrames = 2;
constexpr float kSotApproachGeomAreaMax = 6.0f;
constexpr float kSotApproachIdentityAreaMax = 5.50f;
constexpr float kSotApproachIdentityAspectMin = 0.22f;
constexpr float kSotApproachIdentityAspectMax = 4.50f;
constexpr float kSotInitMinimumSide = 16.0f;
constexpr float kSotInitMinimumAspect = 0.25f;
constexpr float kSotApproachTemplateEnvelopeScaleStep = 0.40f;
constexpr uint64_t kSotApproachTemplateRefreshCooldownFrames = 6;
constexpr int kSotApproachTemplateRefreshFrames = 4;
constexpr float kSotApproachTemplateRefreshVoteRatio = 0.50f;
constexpr float kSotApproachSearchExtraOffsetScale = 0.75f;
constexpr float kSotApproachSearchExtraOffsetMax = 0.80f;
constexpr float kSotRawScaleBiasAlpha = 0.20f;
constexpr int kSotHandoffIdentityConfirmFrames = 3;
constexpr int kSotHandoffIdentityMidConfirmFrames = 4;
constexpr int kSotHandoffIdentityFarConfirmFrames = 5;
constexpr float kSotHandoffIdentityVerifyDistanceRatio = 0.75f;
constexpr float kSotHandoffIdentityMidDistanceRatio = 0.75f;
constexpr float kSotHandoffIdentityFarDistanceRatio = 1.25f;
constexpr float kSotFarHandoffBaselineScoreRatio = 0.85f;
constexpr float kSotFarHandoffBaselinePsrRatio = 0.85f;
constexpr float kSotFarHandoffBaselineMarginRatio = 0.50f;
constexpr float kSotFarHandoffPsrBoost = 1.25f;
constexpr int kSotRefinedNearHandoffConfirmFrames = 3;
constexpr float kSotRefinedNearHandoffDistanceRatio = 0.35f;
constexpr float kSotRefinedNearHandoffMotionRatio = 0.04f;
constexpr float kSotRefinedNearHandoffMinimumPsr = 4.0f;
constexpr float kSotRefinedNearHandoffMinimumMargin = 0.04f;
constexpr int kSotStationaryReliableFrames = 20;
constexpr float kSotStationaryMotionRatio = 0.04f;
constexpr float kSotStationaryDriftRatio = 0.12f;
constexpr int kSotStationarySearchGuardFrames = 4;
constexpr float kSotStationarySearchExtraOffset = 0.45f;
constexpr int kSotRefinedSmallBootstrapFrames = 20;
constexpr float kSotHandoffIdentityMinAreaRatio = 0.50f;
constexpr float kSotHandoffIdentityMaxAreaRatio = 2.00f;
constexpr float kSotHandoffIdentityMinAspectRatio = 0.45f;
constexpr float kSotHandoffIdentityMaxAspectRatio = 2.20f;
constexpr uint32_t kSotKalmanSearchHorizonFrames = 8;
constexpr int kSotReliableMotionMinimumFrames = 8;
constexpr int kSotCameraMotionGuardFrames = 4;
constexpr float kSotReliableMotionMinimumPixels = 0.75f;
constexpr float kSotReliableMotionDirectionCosine = 0.50f;
constexpr float kSotRelockBackwardTargetRatio = 0.40f;
constexpr float kSotRelockBackwardMinimumPixels = 10.0f;
constexpr int kSotRecoveryMotionConfirmFrames = 3;
constexpr float kSotRecoveryMotionMinimumPixels = 0.50f;
constexpr float kSotRecoveryMotionSpeedRatio = 0.30f;
constexpr float kSotRecoveryMotionMaximumGate = 3.0f;
constexpr uint64_t kSotRecoveryTemplateQuarantineFrames = 90;

bool sot_bbox_valid(const std::vector<float>& bbox) {
  return bbox.size() >= 4 && bbox[2] > 1.0f && bbox[3] > 1.0f;
}

float sot_bbox_min_side(const std::vector<float>& bbox) {
  return sot_bbox_valid(bbox) ? std::min(bbox[2], bbox[3]) : 0.0f;
}

int sot_handoff_identity_confirm_frames(float distance,
                                        float target_side) {
  const float distance_ratio = distance / std::max(target_side, 1.0f);
  if (distance_ratio >= kSotHandoffIdentityFarDistanceRatio) {
    return kSotHandoffIdentityFarConfirmFrames;
  }
  if (distance_ratio >= kSotHandoffIdentityMidDistanceRatio) {
    return kSotHandoffIdentityMidConfirmFrames;
  }
  return kSotHandoffIdentityConfirmFrames;
}

float sot_small_target_factor(const std::vector<float>& bbox) {
  const float side = sot_bbox_min_side(bbox);
  if (side <= 0.0f || side >= kSotAdaptiveTargetSide) {
    return 0.0f;
  }
  if (side <= kSotTinyTargetSide) {
    return 1.0f;
  }
  return (kSotAdaptiveTargetSide - side) /
         (kSotAdaptiveTargetSide - kSotTinyTargetSide);
}

float sot_low_detail_target_factor(const std::vector<float>& bbox) {
  const float side = sot_bbox_min_side(bbox);
  if (side <= 0.0f || side >= kSotLowDetailTargetSide) {
    return 0.0f;
  }
  if (side <= kSotTinyTargetSide) {
    return 1.0f;
  }
  return (kSotLowDetailTargetSide - side) /
         (kSotLowDetailTargetSide - kSotTinyTargetSide);
}

bool sot_template_refresh_allowed(const std::vector<float>& bbox) {
  return sot_bbox_min_side(bbox) > kSotTemplateRefreshMinimumTargetSide;
}

uint64_t sot_template_refresh_cooldown(const std::vector<float>& bbox) {
  return sot_low_detail_target_factor(bbox) > 0.0f
             ? kSotLowDetailTemplateRefreshCooldownFrames
             : kSotTemplateRefreshCooldownFrames;
}

bool sot_template_response_strong(float score, float score_baseline,
                                  float psr, float psr_baseline,
                                  float peak_margin,
                                  float peak_margin_baseline) {
  if (!std::isfinite(score) || !std::isfinite(psr) ||
      !std::isfinite(peak_margin)) {
    return false;
  }

  const bool margin_baseline_available = peak_margin_baseline >= 0.005f;
  return score >= std::max(kSotTemplateRefreshMinimumScore,
                           score_baseline * 0.92f) &&
         psr >= std::max(3.5f, psr_baseline * 0.80f) &&
         (!margin_baseline_available ||
          peak_margin >= std::max(0.005f,
                                  peak_margin_baseline * 0.60f));
}

bool sot_identity_response_distinct(float psr, float peak_margin) {
  if (!std::isfinite(psr) || !std::isfinite(peak_margin)) {
    return false;
  }
  return (psr >= kSotMinResponsePsr &&
          peak_margin >= kSotMinPeakMargin) ||
         (psr >= kSotMinResponsePsr * 1.75f &&
          peak_margin >= kSotCandidateMinimumPeakMargin) ||
         (psr >= kSotCandidateMinimumPsr &&
          peak_margin >= kSotMinPeakMargin * 2.5f) ||
         psr >= kSotMinResponsePsr * 3.0f;
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

float sot_bbox_linear_scale(const std::vector<float>& bbox,
                            const std::vector<float>& reference) {
  const float bbox_area = sot_bbox_area(bbox);
  const float reference_area = sot_bbox_area(reference);
  if (bbox_area <= 1.0f || reference_area <= 1.0f) {
    return 0.0f;
  }
  return std::sqrt(bbox_area / reference_area);
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

bool sot_relock_motion_consistent(const std::vector<float>& candidate,
                                  const std::vector<float>& predicted,
                                  const std::vector<float>& identity_reference,
                                  int lost_frames, float* distance,
                                  float* gate) {
  if (!sot_bbox_valid(candidate) || !sot_bbox_valid(predicted) ||
      !sot_bbox_valid(identity_reference)) {
    return false;
  }

  const float candidate_area = sot_bbox_area(candidate);
  const float reference_area = sot_bbox_area(identity_reference);
  const float area_ratio = candidate_area / std::max(reference_area, 1.0f);
  const float target_side = std::max(identity_reference[2],
                                     identity_reference[3]);
  const float horizon = static_cast<float>(
      std::max(1, std::min(lost_frames, 10)));
  const float motion_gate = std::max(
      16.0f, target_side * (0.90f + 0.22f * horizon));
  const float candidate_distance = std::sqrt(
      sot_bbox_center_distance2(candidate, predicted));

  if (distance != nullptr) {
    *distance = candidate_distance;
  }
  if (gate != nullptr) {
    *gate = motion_gate;
  }
  return area_ratio >= 0.45f && area_ratio <= 2.2f &&
         candidate_distance <= motion_gate;
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

bool sot_relock_seed_consistent(const std::vector<float>& candidate,
                                const std::vector<float>& reference) {
  const float candidate_area = sot_bbox_area(candidate);
  const float reference_area = sot_bbox_area(reference);
  if (candidate_area <= 1.0f || reference_area <= 1.0f) {
    return false;
  }

  const float area_ratio = candidate_area / reference_area;
  if (area_ratio < 0.4f || area_ratio > 2.5f) {
    return false;
  }
  const float max_side = std::max(reference[2], reference[3]);
  const float max_distance = std::max(24.0f, max_side * 3.0f);
  return sot_bbox_center_distance2(candidate, reference) <=
         max_distance * max_distance;
}

bool sot_relock_temporally_consistent(
    const std::vector<float>& candidate,
    const std::vector<float>& reference, bool loose) {
  const float candidate_area = sot_bbox_area(candidate);
  const float reference_area = sot_bbox_area(reference);
  if (candidate_area <= 1.0f || reference_area <= 1.0f) {
    return false;
  }

  const float area_ratio = candidate_area / reference_area;
  if (area_ratio < (loose ? 0.5f : 0.6f) ||
      area_ratio > (loose ? 2.0f : 1.67f)) {
    return false;
  }
  const float max_side = std::max(reference[2], reference[3]);
  const float max_distance = std::max(
      loose ? 16.0f : 10.0f, max_side * (loose ? 1.25f : 0.9f));
  return sot_bbox_center_distance2(candidate, reference) <=
         max_distance * max_distance;
}

bool sot_candidate_search_consistent(
    const std::vector<float>& candidate,
    const std::vector<float>& reference) {
  const float candidate_area = sot_bbox_area(candidate);
  const float reference_area = sot_bbox_area(reference);
  if (candidate_area <= 1.0f || reference_area <= 1.0f) {
    return false;
  }

  const float area_ratio = candidate_area / reference_area;
  if (area_ratio < 0.4f || area_ratio > 2.5f) {
    return false;
  }
  const float max_side = std::max(reference[2], reference[3]);
  const float max_distance = std::max(18.0f, max_side * 2.5f);
  return sot_bbox_center_distance2(candidate, reference) <=
         max_distance * max_distance;
}

void sot_apply_bbox_size(std::vector<float>& candidate,
                         const std::vector<float>& size_reference,
                         float image_width, float image_height) {
  if (!sot_bbox_valid(candidate) || !sot_bbox_valid(size_reference)) {
    return;
  }

  const float center_x = candidate[0] + candidate[2] * 0.5f;
  const float center_y = candidate[1] + candidate[3] * 0.5f;
  candidate[2] = size_reference[2];
  candidate[3] = size_reference[3];
  candidate[0] = center_x - candidate[2] * 0.5f;
  candidate[1] = center_y - candidate[3] * 0.5f;
  sot_clamp_bbox_position(candidate, image_width, image_height);
}

float sot_dynamic_search_offset(float base_offset, TrackStatus status,
                                int unstable_frames, int lost_frames,
                                bool have_shadow, float max_expand_ratio,
                                float expansion_offset) {
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
  offset += std::max(0.0f, expansion_offset);
  return std::min(offset, max_offset);
}
}  // namespace

SOT::SOT() {
  preprocessor_ =
      PreprocessorFactory::createPreprocessor(InferencePlatform::AUTOMATIC);
  // 初始化FastSAMSegmentor为nullptr
  fastsam_segmentor_ = nullptr;
  inference_outputs_.reserve(1);
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
    std::shared_ptr<BaseImage>& reuse_image, int minimum_context_side) {
  bool context_valid = minimum_context_side > 0
                           ? calculateContextAtLeast(
                                 image, bbox, offset, minimum_context_side,
                                 context)
                           : calculateContext(image, bbox, offset, context);
  if (!context_valid) {
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

  if (preserve_aspect_context_) {
    // Keep template and search geometry identical for the whole track. A
    // per-frame switch at the 80px boundary changes the model representation.
    const float context_padding = kSotContextAmount * (w + h);
    const float template_context = std::sqrt(
        (w + context_padding) * (h + context_padding));
    const float context_scale = offset <= template_bbox_offset_ + 0.01f
                                    ? 1.0f
                                    : (1.0f + 2.0f * offset) /
                                          (1.0f +
                                           2.0f * template_bbox_offset_);
    int context_side = static_cast<int>(std::ceil(
        std::max(std::max(w, h), template_context * context_scale)));
    context_side = std::max(1, std::min(
        context_side, std::min(width, height)));
    const float center_x = x + w * 0.5f;
    const float center_y = y + h * 0.5f;
    context[0] = static_cast<int>(center_x - context_side * 0.5f);
    context[1] = static_cast<int>(center_y - context_side * 0.5f);
    context[0] = std::max(0, std::min(context[0], width - context_side));
    context[1] = std::max(0, std::min(context[1], height - context_side));
    context[2] = context_side;
    context[3] = context_side;
    return true;
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

bool SOT::calculateContextAtLeast(const std::shared_ptr<BaseImage>& image,
                                  const std::vector<float>& bbox,
                                  float offset, int minimum_context_side,
                                  std::vector<int>& context) const {
  if (!image || bbox.size() < 4 || context.size() < 4 ||
      minimum_context_side <= 0) {
    return false;
  }

  const int image_width = image->getWidth();
  const int image_height = image->getHeight();
  const float bbox_width = bbox[2];
  const float bbox_height = bbox[3];
  if (image_width <= 0 || image_height <= 0 || bbox_width <= 0.0f ||
      bbox_height <= 0.0f) {
    return false;
  }

  if (!calculateContext(image, bbox, offset, context)) {
    return false;
  }

  int context_width = context[2];
  int context_height = context[3];
  context_width = std::min(
      image_width, std::max(context_width, minimum_context_side));
  context_height = std::min(
      image_height, std::max(context_height, minimum_context_side));

  float center_x = bbox[0] + bbox_width * 0.5f;
  float center_y = bbox[1] + bbox_height * 0.5f;
  context[0] = static_cast<int>(center_x - context_width * 0.5f);
  context[1] = static_cast<int>(center_y - context_height * 0.5f);
  context[0] = std::max(0, std::min(context[0],
                                    image_width - context_width));
  context[1] = std::max(0, std::min(context[1],
                                    image_height - context_height));
  context[2] = context_width;
  context[3] = context_height;
  return context[2] > 0 && context[3] > 0;
}

float SOT::calculateScoreRatio(float score) const {
  if (score_lst_.size() < 10) {
    return 1.0f;
  }
  const float avg_score =
      std::accumulate(score_lst_.begin(), score_lst_.end(), 0.0f) /
      score_lst_.size();
  return avg_score > 0.0f ? score / avg_score : 0.0f;
}

void SOT::commitReliableScore(float score) {
  if (score_lst_.size() >= 10) {
    if (score_ratio_ <= occluded_score_ratio_threshold_) {
      return;
    }
    score_lst_.pop_front();
  }
  if (std::isfinite(score)) {
    score_lst_.push_back(score);
  }
}

void SOT::updateReliableResponseBaseline(float score, float psr,
                                         float peak_margin) {
  if (!std::isfinite(score) || !std::isfinite(psr) ||
      !std::isfinite(peak_margin)) {
    return;
  }
  const float bounded_psr = std::min(psr, kSotResponseBaselineMaxPsr);
  const float bounded_peak_margin = std::min(
      peak_margin, kSotResponseBaselineMaxPeakMargin);
  if (reliable_response_samples_ == 0) {
    reliable_score_ema_ = score;
    reliable_psr_ema_ = bounded_psr;
    reliable_peak_margin_ema_ = bounded_peak_margin;
  } else {
    reliable_score_ema_ +=
        (score - reliable_score_ema_) * kSotResponseBaselineAlpha;
    reliable_psr_ema_ +=
        (bounded_psr - reliable_psr_ema_) * kSotResponseBaselineAlpha;
    reliable_peak_margin_ema_ +=
        (bounded_peak_margin - reliable_peak_margin_ema_) *
        kSotResponseBaselineAlpha;
  }
  if (reliable_response_samples_ <
      std::numeric_limits<uint32_t>::max()) {
    reliable_response_samples_++;
  }
}

void SOT::resetReliableMotion() {
  last_reliable_motion_bbox_.clear();
  reliable_velocity_x_ = 0.0f;
  reliable_velocity_y_ = 0.0f;
  last_reliable_motion_frame_id_ = 0;
  reliable_motion_stable_frames_ = 0;
  stationary_reliable_frames_ = 0;
  stationary_reference_bbox_.clear();
}

void SOT::observeReliableMotion(const std::vector<float>& bbox,
                                uint64_t frame_id) {
  if (!sot_bbox_valid(bbox)) {
    resetReliableMotion();
    return;
  }
  if (!sot_bbox_valid(last_reliable_motion_bbox_) ||
      last_reliable_motion_frame_id_ == 0 ||
      frame_id <= last_reliable_motion_frame_id_ ||
      frame_id - last_reliable_motion_frame_id_ > 8) {
    last_reliable_motion_bbox_ = bbox;
    last_reliable_motion_frame_id_ = frame_id;
    reliable_velocity_x_ = 0.0f;
    reliable_velocity_y_ = 0.0f;
    reliable_motion_stable_frames_ = 0;
    stationary_reliable_frames_ = 0;
    stationary_reference_bbox_ = bbox;
    return;
  }

  const float frame_gap = static_cast<float>(
      frame_id - last_reliable_motion_frame_id_);
  const float previous_cx = last_reliable_motion_bbox_[0] +
      last_reliable_motion_bbox_[2] * 0.5f;
  const float previous_cy = last_reliable_motion_bbox_[1] +
      last_reliable_motion_bbox_[3] * 0.5f;
  const float current_cx = bbox[0] + bbox[2] * 0.5f;
  const float current_cy = bbox[1] + bbox[3] * 0.5f;
  const float velocity_x = (current_cx - previous_cx) / frame_gap;
  const float velocity_y = (current_cy - previous_cy) / frame_gap;
  const float velocity_norm = std::sqrt(
      velocity_x * velocity_x + velocity_y * velocity_y);
  const float reliable_norm = std::sqrt(
      reliable_velocity_x_ * reliable_velocity_x_ +
      reliable_velocity_y_ * reliable_velocity_y_);
  const float stationary_motion_gate = std::max(
      1.0f,
      sot_bbox_min_side(bbox) * kSotStationaryMotionRatio);
  if (!sot_bbox_valid(stationary_reference_bbox_)) {
    stationary_reference_bbox_ = bbox;
  }
  const float stationary_drift_gate = std::max(
      3.0f, sot_bbox_min_side(bbox) * kSotStationaryDriftRatio);
  const float stationary_drift = std::sqrt(
      sot_bbox_center_distance2(bbox, stationary_reference_bbox_));

  last_reliable_motion_bbox_ = bbox;
  last_reliable_motion_frame_id_ = frame_id;
  if (velocity_norm < stationary_motion_gate &&
      stationary_drift <= stationary_drift_gate) {
    reliable_velocity_x_ *= 0.80f;
    reliable_velocity_y_ *= 0.80f;
    reliable_motion_stable_frames_ = std::max(
        reliable_motion_stable_frames_ - 1, 0);
    if (camera_motion_guard_frames_ == 0) {
      stationary_reliable_frames_ = std::min(
          stationary_reliable_frames_ + 1, 1000000);
    } else {
      stationary_reliable_frames_ = 0;
    }
    return;
  }
  stationary_reliable_frames_ = 0;
  stationary_reference_bbox_ = bbox;
  if (reliable_norm < kSotReliableMotionMinimumPixels) {
    reliable_velocity_x_ = velocity_x;
    reliable_velocity_y_ = velocity_y;
    reliable_motion_stable_frames_ = 1;
    return;
  }

  const float direction_cosine =
      (reliable_velocity_x_ * velocity_x +
       reliable_velocity_y_ * velocity_y) /
      std::max(reliable_norm * velocity_norm, 1e-6f);
  if (direction_cosine < kSotReliableMotionDirectionCosine) {
    reliable_velocity_x_ = velocity_x;
    reliable_velocity_y_ = velocity_y;
    reliable_motion_stable_frames_ = 0;
    return;
  }

  constexpr float kReliableVelocityAlpha = 0.25f;
  reliable_velocity_x_ +=
      (velocity_x - reliable_velocity_x_) * kReliableVelocityAlpha;
  reliable_velocity_y_ +=
      (velocity_y - reliable_velocity_y_) * kReliableVelocityAlpha;
  reliable_motion_stable_frames_ = std::min(
      reliable_motion_stable_frames_ + 1, 1000000);
}

void SOT::resetRecoveryMotionValidation() {
  recovery_motion_last_bbox_.clear();
  recovery_motion_last_frame_id_ = 0;
  recovery_motion_reference_speed_ = 0.0f;
  recovery_motion_candidate_velocity_x_ = 0.0f;
  recovery_motion_candidate_velocity_y_ = 0.0f;
  recovery_motion_observed_frames_ = 0;
  recovery_motion_good_frames_ = 0;
  recovery_motion_validation_active_ = false;
  recovery_motion_stall_logged_ = false;
}

void SOT::beginRecoveryMotionValidation(const std::vector<float>& bbox,
                                        uint64_t frame_id) {
  resetRecoveryMotionValidation();
  const float reference_speed = std::sqrt(
      reliable_velocity_x_ * reliable_velocity_x_ +
      reliable_velocity_y_ * reliable_velocity_y_);
  if (camera_motion_guard_frames_ > 0 ||
      reliable_motion_stable_frames_ < kSotReliableMotionMinimumFrames ||
      reference_speed < kSotReliableMotionMinimumPixels ||
      !sot_bbox_valid(bbox)) {
    return;
  }

  recovery_motion_validation_active_ = true;
  recovery_motion_reference_speed_ = reference_speed;
  if (sot_bbox_valid(last_reliable_motion_bbox_) &&
      last_reliable_motion_frame_id_ > 0 &&
      last_reliable_motion_frame_id_ <= frame_id) {
    recovery_motion_last_bbox_ = last_reliable_motion_bbox_;
    recovery_motion_last_frame_id_ = last_reliable_motion_frame_id_;
  } else {
    recovery_motion_last_bbox_ = bbox;
    recovery_motion_last_frame_id_ = frame_id;
  }
}

bool SOT::observeRecoveryMotionCandidate(const std::vector<float>& bbox,
                                         uint64_t frame_id) {
  if (!recovery_motion_validation_active_) {
    return true;
  }
  if (camera_motion_guard_frames_ > 0) {
    resetRecoveryMotionValidation();
    return true;
  }
  if (!sot_bbox_valid(bbox) ||
      !sot_bbox_valid(recovery_motion_last_bbox_) ||
      recovery_motion_last_frame_id_ == 0 ||
      frame_id <= recovery_motion_last_frame_id_ ||
      frame_id - recovery_motion_last_frame_id_ > 4) {
    recovery_motion_last_bbox_ = bbox;
    recovery_motion_last_frame_id_ = frame_id;
    recovery_motion_candidate_velocity_x_ = 0.0f;
    recovery_motion_candidate_velocity_y_ = 0.0f;
    recovery_motion_good_frames_ = 0;
    recovery_motion_observed_frames_++;
    return false;
  }

  const float frame_gap = static_cast<float>(
      frame_id - recovery_motion_last_frame_id_);
  const float previous_cx = recovery_motion_last_bbox_[0] +
      recovery_motion_last_bbox_[2] * 0.5f;
  const float previous_cy = recovery_motion_last_bbox_[1] +
      recovery_motion_last_bbox_[3] * 0.5f;
  const float current_cx = bbox[0] + bbox[2] * 0.5f;
  const float current_cy = bbox[1] + bbox[3] * 0.5f;
  recovery_motion_candidate_velocity_x_ =
      (current_cx - previous_cx) / frame_gap;
  recovery_motion_candidate_velocity_y_ =
      (current_cy - previous_cy) / frame_gap;
  const float candidate_speed = std::sqrt(
      recovery_motion_candidate_velocity_x_ *
          recovery_motion_candidate_velocity_x_ +
      recovery_motion_candidate_velocity_y_ *
          recovery_motion_candidate_velocity_y_);
  const float motion_gate = std::max(
      kSotRecoveryMotionMinimumPixels,
      std::min(recovery_motion_reference_speed_ *
                   kSotRecoveryMotionSpeedRatio,
               kSotRecoveryMotionMaximumGate));

  recovery_motion_last_bbox_ = bbox;
  recovery_motion_last_frame_id_ = frame_id;
  recovery_motion_observed_frames_++;
  if (candidate_speed >= motion_gate) {
    recovery_motion_good_frames_++;
    recovery_motion_stall_logged_ = false;
  } else {
    recovery_motion_good_frames_ = 0;
    if (!recovery_motion_stall_logged_ &&
        recovery_motion_observed_frames_ >= 2) {
      recovery_motion_stall_logged_ = true;
    }
  }
  return recovery_motion_good_frames_ >= kSotRecoveryMotionConfirmFrames;
}

void SOT::applyRecoveryTemplateQuarantine(uint64_t frame_id) {
  if (!template_recovery_quarantine_pending_) {
    return;
  }
  if (approach_scale_active_) {
    template_recovery_quarantine_pending_ = false;
    return;
  }
  template_refresh_cooldown_until_frame_ = std::max(
      template_refresh_cooldown_until_frame_,
      frame_id + kSotRecoveryTemplateQuarantineFrames);
  template_refresh_vote_window_.clear();
  template_refresh_good_votes_ = 0;
  template_recovery_stable_frames_ = 0;
  template_recovery_quarantine_pending_ = false;
}

bool SOT::candidateDirectionConsistent(
    const std::vector<float>& candidate,
    const std::vector<float>& predicted,
    float* longitudinal_error, float* backward_gate) const {
  if (longitudinal_error != nullptr) {
    *longitudinal_error = 0.0f;
  }
  if (backward_gate != nullptr) {
    *backward_gate = 0.0f;
  }
  if (!sot_bbox_valid(candidate) || !sot_bbox_valid(predicted) ||
      camera_motion_guard_frames_ > 0 ||
      reliable_motion_stable_frames_ < kSotReliableMotionMinimumFrames) {
    return true;
  }

  const float velocity_norm = std::sqrt(
      reliable_velocity_x_ * reliable_velocity_x_ +
      reliable_velocity_y_ * reliable_velocity_y_);
  if (velocity_norm < kSotReliableMotionMinimumPixels) {
    return true;
  }
  const float unit_x = reliable_velocity_x_ / velocity_norm;
  const float unit_y = reliable_velocity_y_ / velocity_norm;
  const float candidate_cx = candidate[0] + candidate[2] * 0.5f;
  const float candidate_cy = candidate[1] + candidate[3] * 0.5f;
  const float predicted_cx = predicted[0] + predicted[2] * 0.5f;
  const float predicted_cy = predicted[1] + predicted[3] * 0.5f;
  const float along = (candidate_cx - predicted_cx) * unit_x +
      (candidate_cy - predicted_cy) * unit_y;
  const float gate = std::max(
      kSotRelockBackwardMinimumPixels,
      std::max(predicted[2], predicted[3]) *
          kSotRelockBackwardTargetRatio + velocity_norm * 2.0f);
  if (longitudinal_error != nullptr) {
    *longitudinal_error = along;
  }
  if (backward_gate != nullptr) {
    *backward_gate = gate;
  }
  return along >= -gate;
}

void SOT::resetCandidateMotion() {
  last_observed_candidate_bbox_.clear();
  candidate_velocity_x_ = 0.0f;
  candidate_velocity_y_ = 0.0f;
  candidate_motion_bridge_frames_ = 0;
  last_candidate_frame_id_ = 0;
}

void SOT::observeCandidateMotion(const std::vector<float>& bbox,
                                 uint64_t frame_id) {
  if (!sot_bbox_valid(bbox)) {
    return;
  }

  float velocity_x = 0.0f;
  float velocity_y = 0.0f;
  const uint64_t frame_gap = frame_id > last_candidate_frame_id_
      ? frame_id - last_candidate_frame_id_
      : 0;
  if (sot_bbox_valid(last_observed_candidate_bbox_) &&
      frame_gap > 0 && frame_gap <= 4) {
    const float frame_gap_f = static_cast<float>(frame_gap);
    const float previous_cx = last_observed_candidate_bbox_[0] +
        last_observed_candidate_bbox_[2] * 0.5f;
    const float previous_cy = last_observed_candidate_bbox_[1] +
        last_observed_candidate_bbox_[3] * 0.5f;
    const float current_cx = bbox[0] + bbox[2] * 0.5f;
    const float current_cy = bbox[1] + bbox[3] * 0.5f;
    velocity_x = (current_cx - previous_cx) / frame_gap_f;
    velocity_y = (current_cy - previous_cy) / frame_gap_f;

    const float speed = std::sqrt(
        velocity_x * velocity_x + velocity_y * velocity_y);
    const float maximum_speed = std::max(
        2.0f, sot_bbox_min_side(bbox) *
                  kSotCandidateMotionMaxTargetRatio);
    if (speed > maximum_speed) {
      const float scale = maximum_speed / speed;
      velocity_x *= scale;
      velocity_y *= scale;
    }

    const float previous_speed2 =
        candidate_velocity_x_ * candidate_velocity_x_ +
        candidate_velocity_y_ * candidate_velocity_y_;
    const float current_speed2 =
        velocity_x * velocity_x + velocity_y * velocity_y;
    const float direction_dot = candidate_velocity_x_ * velocity_x +
                                candidate_velocity_y_ * velocity_y;
    if (previous_speed2 >=
            kSotCandidateMotionMinPixels * kSotCandidateMotionMinPixels &&
        current_speed2 >=
            kSotCandidateMotionMinPixels * kSotCandidateMotionMinPixels &&
        direction_dot < 0.0f) {
      velocity_x = 0.0f;
      velocity_y = 0.0f;
    } else if (current_speed2 <
               kSotCandidateMotionMinPixels *
                   kSotCandidateMotionMinPixels) {
      velocity_x = 0.0f;
      velocity_y = 0.0f;
    }
  }

  last_observed_candidate_bbox_ = bbox;
  last_candidate_frame_id_ = frame_id;
  candidate_velocity_x_ = velocity_x;
  candidate_velocity_y_ = velocity_y;
  candidate_motion_bridge_frames_ = 0;
}

bool SOT::buildCandidateMotionBridge(uint64_t frame_id, float image_width,
                                     float image_height,
                                     std::vector<float>* bbox) {
  if (bbox == nullptr || !sot_bbox_valid(last_observed_candidate_bbox_) ||
      last_candidate_frame_id_ == 0 || frame_id <= last_candidate_frame_id_) {
    return false;
  }

  const uint64_t elapsed_frames = frame_id - last_candidate_frame_id_;
  if (elapsed_frames > kSotCandidateMotionBridgeFrames ||
      candidate_motion_bridge_frames_ >= kSotCandidateMotionBridgeFrames) {
    return false;
  }

  *bbox = last_observed_candidate_bbox_;
  const float elapsed_frames_f = static_cast<float>(elapsed_frames);
  (*bbox)[0] += candidate_velocity_x_ * elapsed_frames_f;
  (*bbox)[1] += candidate_velocity_y_ * elapsed_frames_f;
  sot_clamp_bbox_position(*bbox, image_width, image_height);
  candidate_motion_bridge_frames_ = static_cast<int>(elapsed_frames);
  return true;
}

void SOT::resetRelockCandidate() {
  shadow_bbox_.clear();
  shadow_vote_window_.clear();
  shadow_good_frames_ = 0;
}

void SOT::recordRelockVote(bool accepted) {
  if (shadow_vote_window_.size() >= kSotRelockVoteWindow) {
    if (shadow_vote_window_.front()) {
      shadow_good_frames_--;
    }
    shadow_vote_window_.pop_front();
  }
  shadow_vote_window_.push_back(accepted);
  if (accepted) {
    shadow_good_frames_++;
  }
}

void SOT::recordRelockMiss() {
  if (!sot_bbox_valid(shadow_bbox_)) {
    return;
  }
  recordRelockVote(false);
  const std::size_t vote_count = shadow_vote_window_.size();
  const std::size_t miss_limit = 2;
  bool consecutive_misses = vote_count >= miss_limit;
  for (std::size_t i = 0; consecutive_misses && i < miss_limit; i++) {
    consecutive_misses =
        !shadow_vote_window_[vote_count - 1 - i];
  }
  if (consecutive_misses) {
    resetRelockCandidate();
  }
}

bool SOT::relockConfirmed(bool near_continuation) const {
  if (near_continuation) {
    return shadow_vote_window_.size() >= kSotNearRelockVoteWindow &&
           shadow_good_frames_ >= kSotNearRelockVotesRequired;
  }
  return shadow_vote_window_.size() >= kSotRelockVoteWindow &&
         shadow_good_frames_ >= kSotRelockVotesRequired;
}

int SOT::templateRefreshFrames(float adaptive_target_factor) const {
  if (approach_scale_active_) {
    return kSotApproachTemplateRefreshFrames;
  }
  if (template_update_count_ == 0) {
    return kSotInitialTemplateRefreshFrames;
  }
  const float factor = std::max(
      0.0f, std::min(adaptive_target_factor, 1.0f));
  return kSotMinimumTemplateRefreshFrames + static_cast<int>(std::lround(
      (kSotMaximumTemplateRefreshFrames -
       kSotMinimumTemplateRefreshFrames) * factor));
}

void SOT::recordTemplateRefreshVote(bool reliable) {
  if (template_refresh_vote_window_.size() >=
      kSotTemplateRefreshVoteWindow) {
    if (template_refresh_vote_window_.front()) {
      template_refresh_good_votes_--;
    }
    template_refresh_vote_window_.pop_front();
  }
  template_refresh_vote_window_.push_back(reliable);
  if (reliable) {
    template_refresh_good_votes_++;
  }
}

bool SOT::templateRefreshReady(float adaptive_target_factor) const {
  const int required_frames = templateRefreshFrames(adaptive_target_factor);
  if (static_cast<int>(template_refresh_vote_window_.size()) <
      required_frames) {
    return false;
  }

  int good_votes = 0;
  const auto begin = template_refresh_vote_window_.end() - required_frames;
  for (auto it = begin; it != template_refresh_vote_window_.end(); ++it) {
    if (*it) {
      good_votes++;
    }
  }
  const float vote_ratio = approach_scale_active_
      ? kSotApproachTemplateRefreshVoteRatio
      : kSotTemplateRefreshVoteRatio;
  const int required_good_votes = static_cast<int>(std::ceil(
      required_frames * vote_ratio));
  return good_votes >= required_good_votes;
}

void SOT::recordTemplateEnvelopeSample(const std::vector<float>& bbox) {
  if (!sot_bbox_valid(bbox)) {
    return;
  }
  if (template_bbox_history_.size() >= kSotTemplateEnvelopeHistory) {
    template_bbox_history_.pop_front();
  }
  template_bbox_history_.push_back(bbox);
}

bool SOT::buildTemplateRefreshEnvelope(
    const std::vector<float>& bbox,
    std::vector<float>* envelope) const {
  const std::size_t envelope_minimum_samples = approach_scale_active_
      ? 3 : kSotTemplateEnvelopeMinimumSamples;
  if (envelope == nullptr || !sot_bbox_valid(bbox) ||
      template_bbox_history_.size() < envelope_minimum_samples) {
    return false;
  }

  std::vector<float> widths;
  std::vector<float> heights;
  widths.reserve(template_bbox_history_.size());
  heights.reserve(template_bbox_history_.size());
  for (const auto& sample : template_bbox_history_) {
    if (sot_bbox_valid(sample)) {
      widths.push_back(sample[2]);
      heights.push_back(sample[3]);
    }
  }
  if (widths.size() < envelope_minimum_samples) {
    return false;
  }
  std::sort(widths.begin(), widths.end());
  std::sort(heights.begin(), heights.end());
  const std::size_t percentile_index = std::min(
      widths.size() - 1,
      static_cast<std::size_t>(std::lround(
          (widths.size() - 1) * kSotTemplateEnvelopePercentile)));
  float envelope_width = widths[percentile_index];
  float envelope_height = heights[percentile_index];
  const float width_ratio = bbox[2] / envelope_width;
  const float height_ratio = bbox[3] / envelope_height;
  const float envelope_minimum_ratio = approach_scale_active_
      ? 0.55f : kSotTemplateEnvelopeMinimumRatio;
  const float envelope_maximum_ratio = approach_scale_active_
      ? 2.20f : kSotTemplateEnvelopeMaximumRatio;
  if (width_ratio < envelope_minimum_ratio ||
      width_ratio > envelope_maximum_ratio ||
      height_ratio < envelope_minimum_ratio ||
      height_ratio > envelope_maximum_ratio) {
    return false;
  }

  if (approach_scale_active_) {
    envelope_width = std::max(envelope_width, bbox[2]);
    envelope_height = std::max(envelope_height, bbox[3]);
  }

  if (sot_bbox_valid(template_envelope_bbox_)) {
    const float envelope_scale_step = approach_scale_active_
        ? kSotApproachTemplateEnvelopeScaleStep
        : kSotTemplateEnvelopeScaleStep;
    const float minimum_width = template_envelope_bbox_[2] *
        (1.0f - envelope_scale_step);
    const float maximum_width = template_envelope_bbox_[2] *
        (1.0f + envelope_scale_step);
    const float minimum_height = template_envelope_bbox_[3] *
        (1.0f - envelope_scale_step);
    const float maximum_height = template_envelope_bbox_[3] *
        (1.0f + envelope_scale_step);
    const bool previous_envelope_nearby =
        bbox[2] >= minimum_width * 0.80f &&
        bbox[2] <= maximum_width * 1.20f &&
        bbox[3] >= minimum_height * 0.80f &&
        bbox[3] <= maximum_height * 1.20f;

    if (previous_envelope_nearby) {
      envelope_width = std::max(
          minimum_width, std::min(envelope_width, maximum_width));
      envelope_height = std::max(
          minimum_height, std::min(envelope_height, maximum_height));
    }
  }

  const float center_x = bbox[0] + bbox[2] * 0.5f;
  const float center_y = bbox[1] + bbox[3] * 0.5f;
  *envelope = {center_x - envelope_width * 0.5f,
               center_y - envelope_height * 0.5f,
               envelope_width, envelope_height};
  return true;
}

bool SOT::refreshTemplate(const std::shared_ptr<BaseImage>& image,
                          const std::vector<float>& bbox,
                          uint64_t frame_id) {
  if (!image || !sot_bbox_valid(bbox) ||
      !sot_template_refresh_allowed(bbox)) {
    return false;
  }

  std::vector<int> template_context(4);
  std::vector<float> template_bbox;
  std::shared_ptr<BaseImage> refreshed_template;

  if (!buildTemplateRefreshEnvelope(bbox, &template_bbox)) {
    return false;
  }
  sot_clamp_bbox_position(template_bbox, image->getWidth(),
                          image->getHeight());
  refreshed_template = preprocessReuse(
      image, template_bbox, template_bbox_offset_, template_size_,
      template_context,
      refreshed_template,
      sot_low_detail_target_factor(template_bbox) > 0.0f
          ? kSotRefinedSmallTargetTemplateContext : 0);
  if (!refreshed_template) {
    return false;
  }

  if (!adaptive_template_image_) {
    adaptive_template_image_ = template_image_;
  }
  template_backup_image_ = adaptive_template_image_;
  template_envelope_backup_bbox_ = template_envelope_bbox_;
  backup_template_update_frame_ = last_template_update_frame_;
  adaptive_template_image_ = refreshed_template;
  template_envelope_bbox_ = template_bbox;
  template_image_ = adaptive_template_image_;
  using_long_term_template_ = false;
  sot_model_->invalidateInputCache();
  last_template_update_frame_ = frame_id;
  template_refresh_cooldown_until_frame_ =
      frame_id + (approach_scale_active_
                      ? kSotApproachTemplateRefreshCooldownFrames
                      : sot_template_refresh_cooldown(template_bbox));
  template_update_count_++;
  template_probation_frames_ = 0;
  template_probation_good_frames_ = 0;
  template_probation_bad_frames_ = 0;
  template_probation_consecutive_bad_frames_ = 0;
  template_refresh_vote_window_.clear();
  template_refresh_good_votes_ = 0;
  template_bbox_history_.clear();
  recordTemplateEnvelopeSample(template_bbox);
  template_recovery_miss_frames_ = 0;
  template_recovery_stable_frames_ = 0;
  template_long_term_probe_frames_ = 0;
  template_recovery_reset_frames_ = 0;
  template_recovery_long_term_tried_ = false;

  return true;
}

void SOT::rollbackTemplate() {
  if (!template_backup_image_) {
    return;
  }
  adaptive_template_image_ = template_backup_image_;
  template_image_ = adaptive_template_image_;
  if (sot_bbox_valid(template_envelope_backup_bbox_)) {
    template_envelope_bbox_ = template_envelope_backup_bbox_;
  }
  using_long_term_template_ = false;
  template_backup_image_.reset();
  template_envelope_backup_bbox_.clear();
  last_template_update_frame_ = backup_template_update_frame_;
  backup_template_update_frame_ = 0;
  template_rollback_count_++;

  template_probation_frames_ = 0;
  template_probation_good_frames_ = 0;
  template_probation_bad_frames_ = 0;
  template_probation_consecutive_bad_frames_ = 0;
  template_refresh_vote_window_.clear();
  template_refresh_good_votes_ = 0;
  template_bbox_history_.clear();
  template_recovery_miss_frames_ = 0;
  template_recovery_stable_frames_ = 0;
  template_long_term_probe_frames_ = 0;
  template_recovery_reset_frames_ = 0;
  template_recovery_long_term_tried_ = false;
  resetScaleCandidate();
  scale_stable_frames_ = 0;
  template_refresh_cooldown_until_frame_ =
      frame_id_ + kSotTemplateRefreshCooldownFrames;
  sot_model_->invalidateInputCache();
}

void SOT::observeTemplateProbation(
    TemplateProbationObservation observation) {
  if (!template_backup_image_) {
    return;
  }

  template_probation_frames_++;
  if (observation == TemplateProbationObservation::GOOD) {
    template_probation_good_frames_++;
    template_probation_consecutive_bad_frames_ = 0;
  } else if (observation == TemplateProbationObservation::BAD) {
    template_probation_bad_frames_++;
    template_probation_consecutive_bad_frames_++;
  } else {
    template_probation_consecutive_bad_frames_ = 0;
  }

  if (template_probation_consecutive_bad_frames_ >=
      kSotTemplateProbationConsecutiveBadFrames) {
    rollbackTemplate();
    return;
  }
  if (template_probation_frames_ < kSotTemplateProbationFrames) {
    return;
  }
  if (template_probation_good_frames_ >=
          kSotTemplateProbationGoodFrames &&
      template_probation_good_frames_ >=
          template_probation_bad_frames_ * 3) {
    template_backup_image_.reset();
    template_envelope_backup_bbox_.clear();
    backup_template_update_frame_ = 0;
    template_probation_frames_ = 0;
    template_probation_good_frames_ = 0;
    template_probation_bad_frames_ = 0;
    template_probation_consecutive_bad_frames_ = 0;
    return;
  }
  if (template_probation_frames_ >= kSotTemplateProbationMaxFrames) {
    rollbackTemplate();
  }
}

void SOT::activateTemplate(bool use_long_term) {
  const std::shared_ptr<BaseImage>& target = use_long_term
      ? long_term_template_image_ : adaptive_template_image_;
  if (!target || (template_image_ == target &&
                  using_long_term_template_ == use_long_term)) {
    return;
  }
  template_image_ = target;
  using_long_term_template_ = use_long_term;
  template_switch_count_++;
  sot_model_->invalidateInputCache();

}

void SOT::scheduleTemplateRecovery() {
  template_recovery_stable_frames_ = 0;
  template_recovery_reset_frames_ = 0;
  if (approach_scale_active_) {
    return;
  }
  if (handoff_identity_active_ && using_long_term_template_) {
    return;
  }
  if (template_backup_image_ || !long_term_template_image_ ||
      !adaptive_template_image_ ||
      long_term_template_image_ == adaptive_template_image_) {
    return;
  }

  if (using_long_term_template_) {
    template_long_term_probe_frames_++;
    if (template_long_term_probe_frames_ >=
        kSotTemplateLongTermProbeFrames) {
      activateTemplate(false);
      template_long_term_probe_frames_ = 0;
    }
    return;
  }

  if (template_recovery_long_term_tried_) {
    return;
  }
  template_recovery_miss_frames_++;
  if (template_recovery_miss_frames_ < kSotTemplateRecoverySwitchFrames) {
    return;
  }
  template_recovery_miss_frames_ = 0;
  template_long_term_probe_frames_ = 0;
  template_recovery_long_term_tried_ = true;
  activateTemplate(true);
}

void SOT::observeReliableTemplate(
    const std::shared_ptr<BaseImage>& image,
    const std::vector<float>& bbox, uint64_t frame_id,
    float adaptive_target_factor, bool refresh_quality_strong) {
  template_recovery_miss_frames_ = 0;
  if (approach_scale_active_ && using_long_term_template_) {
    activateTemplate(false);
  }
  if (refresh_quality_strong ||
      (approach_scale_active_ && scale_committed_this_frame_)) {
    recordTemplateEnvelopeSample(bbox);
  }
  std::vector<float> refresh_envelope;
  const bool refresh_geometry_ready =
      buildTemplateRefreshEnvelope(bbox, &refresh_envelope);
  if (using_long_term_template_ && !approach_scale_active_) {
    template_long_term_probe_frames_ = 0;
    template_recovery_reset_frames_ = 0;
    template_refresh_vote_window_.clear();
    template_refresh_good_votes_ = 0;
    observeTemplateProbation(TemplateProbationObservation::BAD);
    if (refresh_quality_strong && refresh_geometry_ready) {
      template_recovery_stable_frames_++;
    } else {
      template_recovery_stable_frames_ = 0;
    }
    if (template_recovery_stable_frames_ >=
            kSotTemplateRecoveryStableFrames &&
        sot_template_refresh_allowed(bbox) &&
        frame_id >= template_refresh_cooldown_until_frame_) {
      if (template_backup_image_) {
        rollbackTemplate();
      }
      refreshTemplate(image, bbox, frame_id);
    }
    return;
  }

  template_recovery_stable_frames_ = 0;
  template_long_term_probe_frames_ = 0;
  if (refresh_quality_strong) {
    template_recovery_reset_frames_++;
    if (template_recovery_reset_frames_ >=
        kSotTemplateRecoveryResetFrames) {
      template_recovery_long_term_tried_ = false;
      template_recovery_reset_frames_ = 0;
    }
  } else {
    template_recovery_reset_frames_ = 0;
  }
  const bool refresh_allowed = sot_template_refresh_allowed(bbox);
  const bool template_quality_strong = refresh_allowed &&
                                       refresh_quality_strong &&
                                       refresh_geometry_ready;
  observeTemplateProbation(TemplateProbationObservation::GOOD);
  recordTemplateRefreshVote(template_quality_strong);
  const bool approach_refresh_ready =
      refresh_quality_strong && !context_seed_ && approach_scale_active_ &&
      scale_committed_this_frame_ &&
      refresh_geometry_ready && refresh_allowed;
  const bool grow_refresh_ready =
      refresh_quality_strong && !context_seed_ &&
      scale_committed_this_frame_ &&
      last_raw_scale_ratio_ >= 1.0f &&
      refresh_geometry_ready && refresh_allowed &&
      (last_template_update_frame_ == 0 ||
       frame_id >= last_template_update_frame_ +
                       kSotApproachTemplateRefreshCooldownFrames);
  if (!template_backup_image_ &&
      (grow_refresh_ready ||
       (frame_id >= template_refresh_cooldown_until_frame_ &&
        ((template_quality_strong &&
          templateRefreshReady(adaptive_target_factor)) ||
         approach_refresh_ready)))) {
    refreshTemplate(image, bbox, frame_id);
  }
}

void SOT::observeUnreliableTemplate(bool hard_lost) {
  template_recovery_stable_frames_ = 0;
  template_recovery_reset_frames_ = 0;
  resetScaleCandidate();
  if (hard_lost) {
    scale_stable_frames_ = 0;
  } else {
    scale_stable_frames_ = std::max(scale_stable_frames_ - 1, 0);
  }
  scale_committed_this_frame_ = false;
  recordTemplateRefreshVote(false);
  observeTemplateProbation(hard_lost
      ? TemplateProbationObservation::BAD
      : TemplateProbationObservation::NEUTRAL);
  scheduleTemplateRecovery();
}

void SOT::resetScaleCandidate() {
  pending_scale_bbox_.clear();
  pending_scale_mode_ = 0;
  pending_scale_frames_ = 0;
}

void SOT::resetApproachScale() {
  approach_scale_frames_ = 0;
  approach_scale_miss_frames_ = 0;
  approach_scale_active_ = false;
}

void SOT::observeApproachScale(bool grow_observation) {
  if (context_seed_) {
    last_grow_observation_ = false;
    if (approach_scale_active_) {
      resetApproachScale();

    }
    return;
  }
  const bool was_active = approach_scale_active_;
  last_grow_observation_ = grow_observation;
  const bool recent_grow =
      last_grow_commit_frame_ > 0 && frame_id_ >= last_grow_commit_frame_ &&
      frame_id_ - last_grow_commit_frame_ <=
          static_cast<uint64_t>(kSotApproachHoldFrames);
  if (grow_observation) {
    approach_scale_miss_frames_ = 0;
    approach_scale_frames_ = std::min(approach_scale_frames_ + 1, 1000000);
    if (approach_scale_frames_ >= kSotApproachConfirmFrames) {
      approach_scale_active_ = true;
    }
    if (!was_active && approach_scale_active_) {
      template_refresh_cooldown_until_frame_ = std::min(
          template_refresh_cooldown_until_frame_, frame_id_);

    }
    return;
  }
  if (approach_scale_active_ &&
      (recent_grow ||
       last_approach_raw_scale_ >= kSotApproachHoldRawRatio)) {
    approach_scale_miss_frames_ = 0;
    return;
  }
  if (!approach_scale_active_ && approach_scale_frames_ == 0) {
    return;
  }
  approach_scale_miss_frames_++;
  if (approach_scale_miss_frames_ >= kSotApproachBreakFrames) {
    resetApproachScale();

  }
}

void SOT::observeRawScaleBias(
    const std::vector<float>& raw_bbox,
    const std::vector<float>& reference_bbox) {
  if (!sot_bbox_valid(raw_bbox) || !sot_bbox_valid(reference_bbox)) {
    return;
  }

  const float width_ratio = raw_bbox[2] / reference_bbox[2];
  const float height_ratio = raw_bbox[3] / reference_bbox[3];
  if (width_ratio < kSotScaleMinimumRawRatio ||
      width_ratio > kSotScaleMaximumRawRatio ||
      height_ratio < kSotScaleMinimumRawRatio ||
      height_ratio > kSotScaleMaximumRawRatio) {
    return;
  }

  if (raw_scale_bias_samples_ == 0) {
    raw_scale_bias_width_ = width_ratio;
    raw_scale_bias_height_ = height_ratio;
  } else {
    raw_scale_bias_width_ +=
        (width_ratio - raw_scale_bias_width_) * kSotRawScaleBiasAlpha;
    raw_scale_bias_height_ +=
        (height_ratio - raw_scale_bias_height_) * kSotRawScaleBiasAlpha;
  }
  if (raw_scale_bias_samples_ < std::numeric_limits<uint32_t>::max()) {
    raw_scale_bias_samples_++;
  }
}

std::vector<float> SOT::normalizeRawScaleBBox(
    const std::vector<float>& raw_bbox) const {
  if (!sot_bbox_valid(raw_bbox) || raw_scale_bias_samples_ == 0 ||
      raw_scale_bias_width_ <= 0.0f || raw_scale_bias_height_ <= 0.0f) {
    return raw_bbox;
  }

  const float center_x = raw_bbox[0] + raw_bbox[2] * 0.5f;
  const float center_y = raw_bbox[1] + raw_bbox[3] * 0.5f;
  const float width = raw_bbox[2] / raw_scale_bias_width_;
  const float height = raw_bbox[3] / raw_scale_bias_height_;
  return {center_x - width * 0.5f, center_y - height * 0.5f,
          width, height};
}

bool SOT::observeTrustedScale(const std::vector<float>& raw_bbox,
                              bool reliable, float image_width,
                              float image_height) {
  scale_committed_this_frame_ = false;
  if (!sot_bbox_valid(raw_bbox) || !sot_bbox_valid(size_anchor_bbox_)) {
    resetScaleCandidate();
    scale_stable_frames_ = 0;
    return false;
  }

  const float trusted_width = size_anchor_bbox_[2];
  const float trusted_height = size_anchor_bbox_[3];
  const float width_ratio = raw_bbox[2] / trusted_width;
  const float height_ratio = raw_bbox[3] / trusted_height;
  last_raw_scale_width_ratio_ = width_ratio;
  last_raw_scale_height_ratio_ = height_ratio;
  const float area_ratio = width_ratio * height_ratio;
  last_raw_scale_ratio_ = area_ratio > 0.0f
      ? std::sqrt(area_ratio) : 0.0f;

  if (!reliable) {
    resetScaleCandidate();
    scale_stable_frames_ = std::max(scale_stable_frames_ - 1, 0);
    if (approach_scale_active_ || last_grow_observation_ ||
        last_raw_scale_ratio_ >= kSotApproachMinimumRawRatio) {

    }
    return false;
  }
  const bool approach_like =
      !context_seed_ &&
      (approach_scale_active_ || last_grow_observation_);
  const float maximum_raw_ratio = approach_like
      ? kSotApproachMaximumRawRatio
      : kSotScaleMaximumRawRatio;
  if (width_ratio < kSotScaleMinimumRawRatio ||
      width_ratio > maximum_raw_ratio ||
      height_ratio < kSotScaleMinimumRawRatio ||
      height_ratio > maximum_raw_ratio) {
    scale_reject_count_++;
    resetScaleCandidate();
    scale_stable_frames_ = 0;

    return false;
  }

  int scale_mode = 0;
  const float raw_aspect_ratio = width_ratio / height_ratio;
  const float common_scale = std::sqrt(area_ratio);
  if (common_scale <= 1.0f - kSotScaleDeadband) {
    scale_mode = -1;
  } else if (common_scale >= 1.0f + kSotScaleDeadband) {
    scale_mode = 1;
  } else if (!lock_output_aspect_ &&
             std::fabs(std::log(raw_aspect_ratio)) >= 0.08f) {
    scale_mode = 2;
  }

  if (scale_mode == 0) {
    scale_stable_frames_ = std::min(scale_stable_frames_ + 1, 1000000);
    resetScaleCandidate();
    return false;
  }

  const bool recent_grow =
      last_grow_commit_frame_ > 0 && frame_id_ >= last_grow_commit_frame_ &&
      frame_id_ - last_grow_commit_frame_ <=
          static_cast<uint64_t>(kSotApproachShrinkHoldFrames);
  const bool strong_shrink = common_scale <= 0.90f;
  if (scale_mode < 0 && !strong_shrink && !context_seed_ &&
      (approach_scale_active_ || recent_grow || last_grow_observation_)) {
    resetScaleCandidate();
    return false;
  }

  // A consistent scale trend is not a stable geometry interval. Keeping this
  // counter alive while the anchor changes lets a partial response immediately
  // become the next template and creates a self-reinforcing size collapse.
  scale_stable_frames_ = 0;

  const float pending_consistency = approach_like
      ? 0.28f : kSotScalePendingConsistency;
  const bool pending_consistent = pending_scale_mode_ == scale_mode &&
      sot_bbox_valid(pending_scale_bbox_) &&
      std::fabs(std::log(raw_bbox[2] / pending_scale_bbox_[2])) <=
          pending_consistency &&
      std::fabs(std::log(raw_bbox[3] / pending_scale_bbox_[3])) <=
          pending_consistency;
  if (!pending_consistent) {
    if (pending_scale_frames_ > 1) {
      scale_reject_count_++;
    }
    pending_scale_bbox_ = raw_bbox;
    pending_scale_mode_ = scale_mode;
    pending_scale_frames_ = 1;
    return false;
  }

  constexpr float kPendingScaleAlpha = 0.35f;
  pending_scale_bbox_[2] +=
      (raw_bbox[2] - pending_scale_bbox_[2]) * kPendingScaleAlpha;
  pending_scale_bbox_[3] +=
      (raw_bbox[3] - pending_scale_bbox_[3]) * kPendingScaleAlpha;
  pending_scale_frames_++;

  const int required_frames = scale_mode < 0
      ? kSotScaleShrinkConfirmFrames
      : (scale_mode > 1 ? kSotScaleReshapeConfirmFrames
                        : (approach_like
                               ? kSotApproachGrowConfirmFrames
                               : kSotScaleGrowConfirmFrames));
  if (pending_scale_frames_ < required_frames) {
    return false;
  }

  float new_width = trusted_width;
  float new_height = trusted_height;
  const float pending_width_ratio = pending_scale_bbox_[2] / trusted_width;
  const float pending_height_ratio = pending_scale_bbox_[3] / trusted_height;
  const float target_scale = std::sqrt(
      pending_width_ratio * pending_height_ratio);
  const float grow_step_limit = approach_like
      ? kSotApproachGrowStep
      : kSotScaleGrowStep;
  const float scale_step = target_scale < 1.0f
      ? std::max(target_scale, kSotScaleShrinkStep)
      : std::min(target_scale, grow_step_limit);
  const float target_aspect_step =
      pending_width_ratio / pending_height_ratio;
  const float aspect_step = lock_output_aspect_
      ? 1.0f
      : std::max(1.0f / kSotScaleAspectStep,
                 std::min(target_aspect_step, kSotScaleAspectStep));
  const float dimension_step = std::sqrt(aspect_step);
  new_width *= scale_step * dimension_step;
  new_height *= scale_step / dimension_step;

  const float center_x = raw_bbox[0] + raw_bbox[2] * 0.5f;
  const float center_y = raw_bbox[1] + raw_bbox[3] * 0.5f;
  size_anchor_bbox_ = {center_x - new_width * 0.5f,
                       center_y - new_height * 0.5f,
                       new_width, new_height};
  sot_clamp_bbox_position(size_anchor_bbox_, image_width, image_height);
  scale_update_count_++;
  scale_committed_this_frame_ = true;
  scale_stable_frames_ = 0;
  resetScaleCandidate();
  if (sot_bbox_min_side(size_anchor_bbox_) >= kSotAdaptiveTargetSide) {
    preserve_aspect_context_ = false;
  }

  return true;
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
  const float motion = std::sqrt(dx * dx + dy * dy);
  if (confidence >= 0.08f && motion >= 3.0f) {
    camera_motion_guard_frames_ = std::max(
        camera_motion_guard_frames_, kSotCameraMotionGuardFrames);
    stationary_reliable_frames_ = 0;
    stationary_reference_bbox_.clear();
  }
  const float low_detail_factor =
      sot_low_detail_target_factor(current_bbox_);
  if (low_detail_factor > 0.0f) {
    if (motion >= 2.0f) {
      search_expansion_motion_ = std::max(search_expansion_motion_, motion);
      search_expansion_frames_ = std::max(search_expansion_frames_, 2);
    }
    return 0;
  }
  if (status_ == TrackStatus::LOST && use_kalman_filter_ &&
      kalman_tracker_) {
    const uint32_t horizon = std::min<uint32_t>(
        static_cast<uint32_t>(std::max(lost_frames_, 1)),
        kSotKalmanSearchHorizonFrames);
    const std::vector<float> predicted =
        kalman_tracker_->predictAhead(horizon);
    if (sot_bbox_valid(predicted)) {
      const float kalman_dx = predicted[0] + predicted[2] * 0.5f -
          (current_bbox_[0] + current_bbox_[2] * 0.5f);
      const float kalman_dy = predicted[1] + predicted[3] * 0.5f -
          (current_bbox_[1] + current_bbox_[3] * 0.5f);
      const float gmc_motion = std::sqrt(dx * dx + dy * dy);
      const float kalman_motion =
          std::sqrt(kalman_dx * kalman_dx + kalman_dy * kalman_dy);
      if (gmc_motion >= 2.0f && kalman_motion >= 2.0f &&
          dx * kalman_dx + dy * kalman_dy <= 0.0f) {
        search_expansion_motion_ = std::max(
            search_expansion_motion_, gmc_motion);
        search_expansion_frames_ = std::max(search_expansion_frames_, 2);
        return 0;
      }
    }
  }
  if (status_ == TrackStatus::TRACKED ||
      !sot_bbox_valid(search_prior_bbox_)) {
    search_prior_bbox_ = current_bbox_;
  }
  const float normal_max_dx = std::max(12.0f, current_bbox_[2]);
  const float normal_max_dy = std::max(12.0f, current_bbox_[3]);
  search_prior_bbox_[0] +=
      std::max(-normal_max_dx, std::min(dx, normal_max_dx));
  search_prior_bbox_[1] +=
      std::max(-normal_max_dy, std::min(dy, normal_max_dy));
  search_prior_valid_ = true;
  return 0;
}

int32_t SOT::setSearchExpansionHint(float motion_pixels, float confidence) {
  if (!std::isfinite(motion_pixels) || !std::isfinite(confidence) ||
      motion_pixels < 0.0f || confidence < 0.0f || confidence > 1.0f) {
    return -1;
  }
  if (motion_pixels <= 0.0f || confidence < 0.05f ||
      !sot_bbox_valid(current_bbox_)) {
    return 0;
  }
  if (confidence >= 0.08f && motion_pixels >= 3.0f) {
    camera_motion_guard_frames_ = std::max(
        camera_motion_guard_frames_, kSotCameraMotionGuardFrames);
    stationary_reliable_frames_ = 0;
    stationary_reference_bbox_.clear();
  }
  search_expansion_motion_ =
      std::max(search_expansion_motion_, motion_pixels);
  search_expansion_frames_ = 2;
  return 0;
}

int32_t SOT::getTargetInfo(ObjectBoxInfo* bbox,
                           bool* refined_small_target) {
  if (bbox == nullptr || refined_small_target == nullptr ||
      !sot_bbox_valid(current_bbox_)) {
    return -1;
  }
  bbox->x1 = current_bbox_[0];
  bbox->y1 = current_bbox_[1];
  bbox->x2 = current_bbox_[0] + current_bbox_[2];
  bbox->y2 = current_bbox_[1] + current_bbox_[3];
  bbox->score = last_observed_score_;
  *refined_small_target = refined_small_target_;
  return 0;
}

int32_t SOT::getDiagnostics(
    SingleObjectTrackerDiagnostics* diagnostics) {
  if (diagnostics == nullptr) {
    return -1;
  }
  diagnostics->template_update_count = template_update_count_;
  diagnostics->template_rollback_count = template_rollback_count_;
  diagnostics->template_probation_frames =
      static_cast<uint32_t>(std::max(template_probation_frames_, 0));
  diagnostics->template_age_frames = frame_id_ >= last_template_update_frame_
      ? frame_id_ - last_template_update_frame_ : 0;
  diagnostics->template_refresh_votes = static_cast<uint32_t>(
      std::max(template_refresh_good_votes_, 0));
  diagnostics->template_refresh_window = static_cast<uint32_t>(
      template_refresh_vote_window_.size());
  diagnostics->template_using_long_term =
      using_long_term_template_ ? 1u : 0u;
  diagnostics->template_switch_count = template_switch_count_;
  diagnostics->template_recovery_stable_frames = static_cast<uint32_t>(
      std::max(template_recovery_stable_frames_, 0));
  diagnostics->kalman_search_horizon = kalman_search_horizon_;
  diagnostics->kalman_search_dx = kalman_search_dx_;
  diagnostics->kalman_search_dy = kalman_search_dy_;
  diagnostics->relock_votes =
      static_cast<uint32_t>(shadow_vote_window_.size());
  diagnostics->relock_good_votes =
      static_cast<uint32_t>(std::max(shadow_good_frames_, 0));
  diagnostics->relock_motion_rejects = relock_motion_reject_count_;
  diagnostics->relock_candidate_distance = last_relock_candidate_distance_;
  diagnostics->relock_motion_gate = last_relock_motion_gate_;
  diagnostics->response_score = last_response_score_;
  diagnostics->response_psr = last_response_psr_;
  diagnostics->response_peak_margin = last_response_peak_margin_;
  diagnostics->response_area_ratio = last_response_area_ratio_;
  diagnostics->response_aspect_ratio = last_response_aspect_ratio_;
  diagnostics->response_reject_mask = last_response_reject_mask_;
  diagnostics->bootstrap_active = bootstrap_active_;
  diagnostics->bootstrap_retry_count = bootstrap_retry_count_;
  diagnostics->weak_follow_count = weak_follow_count_;
  diagnostics->hard_lost_deferred_count = hard_lost_deferred_count_;
  diagnostics->low_detail_target = last_low_detail_target_ ? 1u : 0u;
  diagnostics->response_selected_candidate = last_selected_candidate_;
  diagnostics->response_psr_baseline = reliable_psr_ema_;
  diagnostics->response_peak_margin_baseline =
      reliable_peak_margin_ema_;
  diagnostics->search_target_pixels = last_search_target_pixels_;
  diagnostics->template_recovery_attempted =
      template_recovery_long_term_tried_ ? 1u : 0u;
  diagnostics->handoff_identity_active =
      handoff_identity_active_ ? 1u : 0u;
  diagnostics->handoff_identity_confirmed =
      handoff_identity_confirmed_ ? 1u : 0u;
  diagnostics->handoff_identity_frames = static_cast<uint32_t>(
      std::max(handoff_identity_frames_, 0));
  diagnostics->handoff_identity_good_frames = static_cast<uint32_t>(
      std::max(handoff_identity_good_frames_, 0));
  diagnostics->candidate_output_held = candidate_output_held_ ? 1u : 0u;
  diagnostics->candidate_output_following =
      candidate_output_following_ ? 1u : 0u;
  diagnostics->candidate_follow_dx = recovery_motion_candidate_velocity_x_;
  diagnostics->candidate_follow_dy = recovery_motion_candidate_velocity_y_;
  diagnostics->handoff_candidate_distance = handoff_candidate_distance_;
  diagnostics->handoff_candidate_area_ratio =
      handoff_candidate_area_ratio_;
  diagnostics->handoff_candidate_aspect_ratio =
      handoff_candidate_aspect_ratio_;
  diagnostics->scale_update_count = scale_update_count_;
  diagnostics->scale_reject_count = scale_reject_count_;
  diagnostics->scale_pending_frames = static_cast<uint32_t>(
      std::max(pending_scale_frames_, 0));
  diagnostics->scale_stable_frames = static_cast<uint32_t>(
      std::max(scale_stable_frames_, 0));
  diagnostics->scale_raw_ratio = last_raw_scale_ratio_;
  diagnostics->scale_raw_width_ratio = last_raw_scale_width_ratio_;
  diagnostics->scale_raw_height_ratio = last_raw_scale_height_ratio_;
  diagnostics->preserve_aspect_context =
      preserve_aspect_context_ ? 1u : 0u;
  diagnostics->search_context_width = last_search_context_width_;
  diagnostics->search_context_height = last_search_context_height_;
  diagnostics->scale_raw_bias_width = raw_scale_bias_width_;
  diagnostics->scale_raw_bias_height = raw_scale_bias_height_;
  diagnostics->scale_raw_bias_samples = raw_scale_bias_samples_;
  diagnostics->scale_trusted_width = sot_bbox_valid(size_anchor_bbox_)
      ? size_anchor_bbox_[2] : 0.0f;
  diagnostics->scale_trusted_height = sot_bbox_valid(size_anchor_bbox_)
      ? size_anchor_bbox_[3] : 0.0f;
  diagnostics->scale_approach_active = approach_scale_active_ ? 1u : 0u;
  diagnostics->scale_approach_frames = static_cast<uint32_t>(
      std::max(approach_scale_frames_, 0));
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
      initBBox(image, area_bbox, result.refined_small_target,
               kTargetSearchFastSAM);
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
    initBBox(image, area_bbox, result.refined_small_target,
             kTargetSearchFastSAM);
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
  bool refined_small_target = false;
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
    refined_small_target = result.refined_small_target;
  } else {
    LOGE("目标点需要有效的分割方法");
    return -1;
  }

  return initBBox(image, area_bbox, refined_small_target,
                  frame_type == kTargetSearchFastSAM
                      ? kTargetSearchFastSAM : 0);
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
                      const ObjectBoxInfo& init_bbox,
                      bool refined_small_target,
                      int initialization_search_type) {
  is_initialized_ = false;
  status_ = TrackStatus::TRACKED;
  lost_frames_ = 0;
  score_lst_.clear();
  score_ratio_ = 1.0f;
  last_observed_score_ = 1.0f;
  last_template_update_frame_ = frame_id_;
  initialization_frame_id_ = frame_id_;
  initialization_search_type_ = initialization_search_type;
  backup_template_update_frame_ = 0;
  template_update_count_ = 0;
  template_rollback_count_ = 0;
  template_probation_frames_ = 0;
  template_probation_good_frames_ = 0;
  template_probation_bad_frames_ = 0;
  template_probation_consecutive_bad_frames_ = 0;
  template_refresh_vote_window_.clear();
  template_refresh_good_votes_ = 0;
  template_bbox_history_.clear();
  template_recovery_miss_frames_ = 0;
  template_recovery_stable_frames_ = 0;
  template_long_term_probe_frames_ = 0;
  template_recovery_reset_frames_ = 0;
  template_recovery_long_term_tried_ = false;
  template_switch_count_ = 0;
  using_long_term_template_ = false;
  template_refresh_cooldown_until_frame_ = 0;
  handoff_identity_active_ = false;
  handoff_identity_confirmed_ = false;
  handoff_identity_good_frames_ = 0;
  handoff_identity_frames_ = 0;
  candidate_output_held_ = false;
  candidate_output_following_ = false;
  handoff_candidate_distance_ = 0.0f;
  handoff_candidate_area_ratio_ = 0.0f;
  handoff_candidate_aspect_ratio_ = 0.0f;
  long_term_template_image_.reset();
  adaptive_template_image_.reset();
  template_backup_image_.reset();
  template_envelope_bbox_.clear();
  template_envelope_backup_bbox_.clear();
  prev_w_h_ratio_ = 0.0f;
  last_reliable_template_bbox_.clear();
  size_anchor_bbox_.clear();
  pending_scale_bbox_.clear();
  search_prior_bbox_.clear();
  candidate_search_bbox_.clear();
  resetCandidateMotion();
  resetReliableMotion();
  resetRecoveryMotionValidation();
  template_recovery_quarantine_pending_ = false;
  camera_motion_guard_frames_ = 0;
  jump_candidate_bbox_.clear();
  resetRelockCandidate();
  search_prior_valid_ = false;
  search_expansion_motion_ = 0.0f;
  search_expansion_frames_ = 0;
  candidate_search_frames_ = 0;
  candidate_search_miss_frames_ = 0;
  soft_follow_frames_ = 0;
  soft_follow_recovery_frames_ = 0;
  jump_candidate_frames_ = 0;
  relock_motion_reject_count_ = 0;
  last_relock_candidate_distance_ = 0.0f;
  last_relock_motion_gate_ = 0.0f;
  last_response_score_ = 0.0f;
  last_response_psr_ = 0.0f;
  last_response_peak_margin_ = 0.0f;
  last_response_area_ratio_ = 0.0f;
  last_response_aspect_ratio_ = 0.0f;
  last_response_reject_mask_ = 0;
  pending_scale_mode_ = 0;
  pending_scale_frames_ = 0;
  scale_stable_frames_ = 0;
  scale_update_count_ = 0;
  scale_reject_count_ = 0;
  last_raw_scale_ratio_ = 1.0f;
  last_raw_scale_width_ratio_ = 1.0f;
  last_raw_scale_height_ratio_ = 1.0f;
  raw_scale_bias_width_ = 1.0f;
  raw_scale_bias_height_ = 1.0f;
  raw_scale_bias_samples_ = 0;
  scale_committed_this_frame_ = false;
  resetApproachScale();
  last_grow_observation_ = false;
  last_approach_geometry_ok_ = false;
  last_approach_response_ok_ = false;
  last_approach_raw_scale_ = 1.0f;
  last_center_displacement_ = 0.0f;
  last_scale_position_gate_ = 0.0f;
  last_grow_commit_frame_ = 0;
  bootstrap_active_ = 0;
  bootstrap_retry_count_ = 0;
  weak_follow_count_ = 0;
  hard_lost_deferred_count_ = 0;
  last_selected_candidate_ = 0;
  last_search_target_pixels_ = 0.0f;
  last_search_context_width_ = 0;
  last_search_context_height_ = 0;
  preserve_aspect_context_ = false;
  lock_output_aspect_ = false;
  context_seed_ = false;
  last_low_detail_target_ = false;
  kalman_search_horizon_ = 0;
  kalman_search_dx_ = 0.0f;
  kalman_search_dy_ = 0.0f;
  unstable_frames_ = 0;
  suspect_frames_ = 0;
  reliable_score_ema_ = 0.0f;
  reliable_psr_ema_ = 0.0f;
  reliable_peak_margin_ema_ = 0.0f;
  reliable_response_samples_ = 0;
  refined_small_target_ = refined_small_target;
  sot_info_ = SOTInfo{};
  float x = init_bbox.x1;
  float y = init_bbox.y1;
  float w = init_bbox.x2 - init_bbox.x1;
  float h = init_bbox.y2 - init_bbox.y1;
  std::vector<float> init_bbox_xywh_format = {x, y, w, h};
  std::vector<int>& context = search_context_;
  clampBBox(init_bbox_xywh_format, image);
  x = init_bbox_xywh_format[0];
  y = init_bbox_xywh_format[1];
  w = init_bbox_xywh_format[2];
  h = init_bbox_xywh_format[3];
  {
    const float min_side = std::min(w, h);
    const float max_side = std::max(w, h);
    const float aspect = max_side > 1.0f ? min_side / max_side : 0.0f;
    if (min_side < kSotInitMinimumSide ||
        aspect < kSotInitMinimumAspect) {
      const float center_x = x + w * 0.5f;
      const float center_y = y + h * 0.5f;
      if (w <= h) {
        w = std::max(kSotInitMinimumSide,
                     h * kSotInitMinimumAspect);
      } else {
        h = std::max(kSotInitMinimumSide,
                     w * kSotInitMinimumAspect);
      }
      x = center_x - w * 0.5f;
      y = center_y - h * 0.5f;
      init_bbox_xywh_format = {x, y, w, h};
      sot_clamp_bbox_position(init_bbox_xywh_format, image->getWidth(),
                              image->getHeight());
      x = init_bbox_xywh_format[0];
      y = init_bbox_xywh_format[1];
      w = init_bbox_xywh_format[2];
      h = init_bbox_xywh_format[3];

    }
  }
  current_bbox_ = {x, y, w, h};
  preserve_aspect_context_ =
      sot_small_target_factor(current_bbox_) > 0.0f;
  const bool adaptive_fixed_seed =
      initialization_search_type_ == 0 &&
      std::fabs(w - kSotAdaptiveTargetSide) <= 1.0f &&
      std::fabs(h - kSotAdaptiveTargetSide) <= 1.0f;
  lock_output_aspect_ =
      !adaptive_fixed_seed && std::min(w, h) > 1.0f &&
      std::max(w, h) / std::min(w, h) <= 1.15f;
  context_seed_ =
      !adaptive_fixed_seed &&
      (lock_output_aspect_ ||
       sot_bbox_min_side(current_bbox_) >= kSotAdaptiveTargetSide);
  last_reliable_template_bbox_ = current_bbox_;
  last_reliable_motion_bbox_ = current_bbox_;
  last_reliable_motion_frame_id_ = frame_id_;
  size_anchor_bbox_ = current_bbox_;
  template_envelope_bbox_ = current_bbox_;
  search_prior_bbox_ = current_bbox_;
  sot_info_.template_bbox = current_bbox_;
  sot_info_.frame_id = frame_id_;
  if (use_kalman_filter_) {
    kalman_tracker_ = std::make_shared<KalmanBoxTracker>(current_bbox_);
  }
  template_image_ =
      preprocessReuse(image, current_bbox_, template_bbox_offset_,
                      template_size_, context, template_image_,
                      refined_small_target
                          ? kSotRefinedSmallTargetTemplateContext
                          : 0);
  if (!template_image_) {
    LOGE("模板提取失败");
    return -1;
  }
  long_term_template_image_ = template_image_;
  adaptive_template_image_ = template_image_;
  sot_model_->invalidateInputCache();
  is_initialized_ = true;

  return 0;
}

int32_t SOT::track(const std::shared_ptr<BaseImage>& image, uint64_t frame_id,
                   TrackerInfo& tracker_info) {
  std::vector<float> predicted_output_bbox;
  float predicted_output_score = last_observed_score_;
  bool predicted_output_observed = false;
  const bool inference_uses_identity_template =
      using_long_term_template_ ||
      (long_term_template_image_ &&
       long_term_template_image_ == adaptive_template_image_);

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
  if (camera_motion_guard_frames_ > 0) {
    camera_motion_guard_frames_--;
  }

  uint64_t frame_gap = 1;
  if (frame_id > frame_id_) {
    frame_gap = std::min<uint64_t>(frame_id - frame_id_, 8);
  }
  const std::vector<float>& reliable_search_reference =
      sot_bbox_valid(last_reliable_template_bbox_)
          ? last_reliable_template_bbox_
          : current_bbox_;
  const float low_detail_target_factor =
      sot_low_detail_target_factor(reliable_search_reference);
  const bool low_detail_target = low_detail_target_factor > 0.0f;
  last_low_detail_target_ = low_detail_target;
  last_selected_candidate_ = 0;
  last_search_target_pixels_ = 0.0f;
  const uint64_t initialization_age = frame_id >= initialization_frame_id_
      ? frame_id - initialization_frame_id_ : 0;
  const uint64_t initialization_bootstrap_frames =
      refined_small_target_ ? kSotRefinedSmallBootstrapFrames
                            : kSotBootstrapFrames;
  const bool initialization_bootstrap_active =
      initialization_age <= initialization_bootstrap_frames &&
      reliable_response_samples_ < kSotResponseBaselineMinSamples &&
      sot_small_target_factor(reliable_search_reference) > 0.0f;
  bootstrap_active_ = initialization_bootstrap_active ? 1u : 0u;
  last_response_score_ = 0.0f;
  last_response_psr_ = 0.0f;
  last_response_peak_margin_ = 0.0f;
  last_response_area_ratio_ = 0.0f;
  last_response_aspect_ratio_ = 0.0f;
  last_response_reject_mask_ = 0;
  candidate_output_held_ = false;
  candidate_output_following_ = false;
  std::vector<float> kalman_search_bbox;
  kalman_search_horizon_ = 0;
  kalman_search_dx_ = 0.0f;
  kalman_search_dy_ = 0.0f;
  if (use_kalman_filter_ && kalman_tracker_ &&
      status_ == TrackStatus::LOST &&
      sot_bbox_valid(reliable_search_reference)) {
    kalman_search_horizon_ = std::min<uint32_t>(
        static_cast<uint32_t>(std::max(lost_frames_, 1)),
        kSotKalmanSearchHorizonFrames);
    kalman_search_bbox = kalman_tracker_->predictAhead(
        kalman_search_horizon_);
    if (sot_bbox_valid(kalman_search_bbox)) {
      const float reference_cx = reliable_search_reference[0] +
          reliable_search_reference[2] * 0.5f;
      const float reference_cy = reliable_search_reference[1] +
          reliable_search_reference[3] * 0.5f;
      const float predicted_cx = kalman_search_bbox[0] +
          kalman_search_bbox[2] * 0.5f;
      const float predicted_cy = kalman_search_bbox[1] +
          kalman_search_bbox[3] * 0.5f;
      const float predicted_dx = predicted_cx - reference_cx;
      const float predicted_dy = predicted_cy - reference_cy;
      if (low_detail_target) {
        const float predicted_motion = std::sqrt(
            predicted_dx * predicted_dx + predicted_dy * predicted_dy);
        search_expansion_motion_ = std::max(
            search_expansion_motion_, predicted_motion);
        search_expansion_frames_ = std::max(search_expansion_frames_, 2);
        kalman_search_bbox.clear();
      } else {
        float dx = predicted_dx * 0.80f;
        float dy = predicted_dy * 0.80f;
        const float target_side = std::max(reliable_search_reference[2],
                                           reliable_search_reference[3]);
        const float max_motion = std::max(
            12.0f, target_side *
                (0.50f + 0.35f * kalman_search_horizon_));
        const float motion = std::sqrt(dx * dx + dy * dy);
        if (motion > max_motion && motion > 0.0f) {
          const float scale = max_motion / motion;
          dx *= scale;
          dy *= scale;
        }
        kalman_search_dx_ = dx;
        kalman_search_dy_ = dy;
        kalman_search_bbox = reliable_search_reference;
        kalman_search_bbox[0] += dx;
        kalman_search_bbox[1] += dy;
        sot_clamp_bbox_position(kalman_search_bbox, image->getWidth(),
                                image->getHeight());
      }
    }
  }
  std::vector<int>& context = search_context_;
  // Follow an uncommitted jump for one search without moving the reliable box.
  const bool use_jump_candidate_prior =
      status_ == TrackStatus::TRACKED && jump_candidate_frames_ > 0 &&
      sot_bbox_valid(jump_candidate_bbox_);
  // Follow a provisional candidate with the search crop while keeping the
  // externally visible box and reliable template at the trusted position.
  const bool use_candidate_search_prior =
      !use_jump_candidate_prior &&
      candidate_search_frames_ > 0 &&
      candidate_search_miss_frames_ < kSotCandidateSearchBridgeFrames &&
      sot_bbox_valid(candidate_search_bbox_);
  const bool use_search_prior =
      !use_jump_candidate_prior && !use_candidate_search_prior &&
      search_prior_valid_ &&
      sot_bbox_valid(search_prior_bbox_);
  const bool use_shadow_prior =
      !use_search_prior && status_ == TrackStatus::LOST &&
      sot_bbox_valid(shadow_bbox_);
  const bool use_kalman_search_prior =
      !use_search_prior && !use_shadow_prior &&
      status_ == TrackStatus::LOST &&
      sot_bbox_valid(kalman_search_bbox);
  const bool use_search_expansion =
      !use_jump_candidate_prior && !use_candidate_search_prior &&
      search_expansion_frames_ > 0;
  const float applied_search_expansion_motion = use_search_expansion ?
      search_expansion_motion_ : 0.0f;
  float search_expansion_offset = 0.0f;
  std::vector<float> search_bbox = use_jump_candidate_prior
                                       ? jump_candidate_bbox_
                                       : (use_candidate_search_prior
                                              ? candidate_search_bbox_
                                              : (use_search_prior
                                                     ? search_prior_bbox_
                                                     : (use_shadow_prior
                                                            ? shadow_bbox_
                                                            : (use_kalman_search_prior
                                                                   ? kalman_search_bbox
                                                                   : current_bbox_))));
  sot_clamp_bbox_position(search_bbox, image->getWidth(), image->getHeight());
  if (use_search_prior) {
    search_prior_bbox_ = search_bbox;
    search_prior_valid_ = false;
  }
  if (use_search_expansion) {
    const float target_side = std::max(
        1.0f, std::min(search_bbox[2], search_bbox[3]));
    const float small_factor = sot_small_target_factor(search_bbox);
    const float max_expansion_offset = 1.0f + small_factor * 0.5f;
    search_expansion_offset = std::min(
        max_expansion_offset,
        applied_search_expansion_motion / target_side);
    search_expansion_frames_--;
    if (search_expansion_frames_ == 0) {
      search_expansion_motion_ = 0.0f;
    }
  }
  if (approach_scale_active_ && last_raw_scale_ratio_ > 1.05f) {
    search_expansion_offset += std::min(
        kSotApproachSearchExtraOffsetMax,
        (last_raw_scale_ratio_ - 1.0f) * kSotApproachSearchExtraOffsetScale);
  }
  const float small_factor = sot_small_target_factor(search_bbox);
  const float adaptive_search_offset = std::max(
      kSotMinimumSearchOffset,
      search_bbox_offset_ -
          (search_bbox_offset_ - kSotMinimumSearchOffset) * small_factor);
  const float effective_max_expand_ratio = use_search_expansion
      ? max_expand_ratio_ + 0.2f * small_factor
      : max_expand_ratio_;
  float search_offset = sot_dynamic_search_offset(
      adaptive_search_offset, status_, unstable_frames_, lost_frames_,
      sot_bbox_valid(shadow_bbox_), effective_max_expand_ratio,
      search_expansion_offset);
  const bool stationary_small_search_guard =
      refined_small_target_ && small_factor > 0.0f &&
      stationary_reliable_frames_ >= kSotStationaryReliableFrames &&
      camera_motion_guard_frames_ == 0 &&
      applied_search_expansion_motion <= 0.0f &&
      std::max(lost_frames_, unstable_frames_) > 0 &&
      std::max(lost_frames_, unstable_frames_) <=
          kSotStationarySearchGuardFrames;
  if (stationary_small_search_guard) {
    search_offset = std::min(
        search_offset,
        adaptive_search_offset + kSotStationarySearchExtraOffset);
  }
  if (!calculateContext(image, search_bbox,
                        search_offset, context)) {
    LOGE("搜索区域计算失败");
    return -1;
  }
  const bool expanded_small_search = small_factor > 0.0f &&
      (search_offset > adaptive_search_offset + 0.01f ||
       use_search_expansion || status_ == TrackStatus::LOST);
  if (small_factor > 0.0f && preserve_aspect_context_) {
    const float target_short_side = std::max(
        1.0f, std::min(search_bbox[2], search_bbox[3]));
    const float target_long_side = std::max(search_bbox[2], search_bbox[3]);
    float minimum_target_pixels = expanded_small_search
        ? kSotRecoveryMinimumTargetPixels
        : kSotTrackedMinimumTargetPixels;
    // A small target may enter tracking with a stale or tight selection box.
    // Alternate detail and coverage until the response baseline settles.
    if (initialization_bootstrap_active && unstable_frames_ > 0) {
      if (bootstrap_retry_count_ <
          std::numeric_limits<uint32_t>::max()) {
        bootstrap_retry_count_++;
      }
      minimum_target_pixels = unstable_frames_ % 2 == 0
          ? kSotBootstrapRetryTargetPixels
          : kSotBootstrapWideTargetPixels;
    }
    const float projected_target_pixels = target_short_side *
        instance_size_ / std::max(context[2], context[3]);
    int maximum_context_side = static_cast<int>(std::floor(
        target_short_side * instance_size_ /
        minimum_target_pixels));
    maximum_context_side = std::max(
        maximum_context_side,
        static_cast<int>(std::ceil(target_long_side + 8.0f)));
    maximum_context_side = std::min(
        maximum_context_side,
        static_cast<int>(std::min(image->getWidth(), image->getHeight())));
    if (projected_target_pixels < minimum_target_pixels &&
        maximum_context_side > 0 &&
        (context[2] > maximum_context_side ||
         context[3] > maximum_context_side)) {
      const float center_x = search_bbox[0] + search_bbox[2] * 0.5f;
      const float center_y = search_bbox[1] + search_bbox[3] * 0.5f;
      context[0] = static_cast<int>(center_x - maximum_context_side * 0.5f);
      context[1] = static_cast<int>(center_y - maximum_context_side * 0.5f);
      context[0] = std::max(
          0, std::min(context[0],
                      static_cast<int>(image->getWidth()) -
                          maximum_context_side));
      context[1] = std::max(
          0, std::min(context[1],
                      static_cast<int>(image->getHeight()) -
                          maximum_context_side));
      context[2] = maximum_context_side;
      context[3] = maximum_context_side;
    }
  }
  last_search_target_pixels_ = std::min(search_bbox[2], search_bbox[3]) *
      instance_size_ / std::max(context[2], context[3]);
  last_search_context_width_ = static_cast<uint32_t>(context[2]);
  last_search_context_height_ = static_cast<uint32_t>(context[3]);

  inference_images_[0][0] = template_image_;
  inference_images_[0][1] = image;
  inference_outputs_.clear();
  inference_params_["search_crop_x"] = static_cast<float>(context[0]);
  inference_params_["search_crop_y"] = static_cast<float>(context[1]);
  inference_params_["search_crop_width"] = static_cast<float>(context[2]);
  inference_params_["search_crop_height"] = static_cast<float>(context[3]);
  int32_t ret = sot_model_->inference(
      inference_images_, inference_outputs_, inference_params_);
  if (ret != 0) {
    LOGE("跟踪模型推理失败: %#x", ret);
    return ret;
  }

  if (inference_outputs_.empty()) {
    LOGE("跟踪结果为空");
    return -1;
  }

  std::shared_ptr<ModelBoxInfo> track_result =
      std::dynamic_pointer_cast<ModelBoxInfo>(inference_outputs_[0]);
  if (!track_result) {
    LOGE("跟踪结果转换失败");
    return -1;
  }
  std::shared_ptr<ModelTrackInfo> track_response =
      std::dynamic_pointer_cast<ModelTrackInfo>(inference_outputs_[0]);

  if (track_result->bboxes.empty()) {
    last_response_reject_mask_ = kSotRejectEmpty;
    std::vector<float> candidate_bridge_bbox;
    const bool candidate_bridge_ready = small_factor > 0.0f &&
        buildCandidateMotionBridge(frame_id, image->getWidth(),
                                   image->getHeight(),
                                   &candidate_bridge_bbox);

    if (initialization_bootstrap_active) {
      unstable_frames_++;
      frame_id_ = frame_id;
      sot_set_tracker_output(&tracker_info, current_bbox_,
                             last_observed_score_, TrackStatus::NEW);
      return 0;
    }

    unstable_frames_++;
    observeApproachScale(false);
    observeUnreliableTemplate(false);
    suspect_frames_ = 0;
    jump_candidate_bbox_.clear();
    jump_candidate_frames_ = 0;
    handoff_identity_active_ = false;
    handoff_identity_confirmed_ = false;
    handoff_identity_good_frames_ = 0;
    handoff_identity_frames_ = 0;
    if (candidate_bridge_ready) {
      candidate_search_bbox_ = candidate_bridge_bbox;
      candidate_search_frames_ = std::max(candidate_search_frames_, 1);
      candidate_search_miss_frames_ = candidate_motion_bridge_frames_;
    } else {
      candidate_search_bbox_.clear();
      candidate_search_frames_ = 0;
      candidate_search_miss_frames_ = 0;
    }
    recordRelockMiss();
    std::vector<float> coasting_bbox = candidate_bridge_ready
                                           ? candidate_bridge_bbox
                                           : search_bbox;
    if (use_kalman_filter_ && kalman_tracker_) {
      std::vector<float> predicted_bbox = kalman_tracker_->predict();
      if (small_factor <= 0.0f &&
          !use_search_prior && !use_shadow_prior &&
          !use_candidate_search_prior &&
          sot_bbox_consistent(predicted_bbox, search_bbox, true)) {
        coasting_bbox = predicted_bbox;
      }
    }
    status_ = TrackStatus::LOST;
    template_recovery_quarantine_pending_ = true;
    resetRecoveryMotionValidation();
    lost_frames_++;
    if (sot_bbox_valid(last_reliable_template_bbox_)) {
      current_bbox_ = last_reliable_template_bbox_;
    }
    sot_info_.template_bbox = current_bbox_;
    frame_id_ = frame_id;
    sot_clamp_bbox_position(coasting_bbox, image->getWidth(),
                            image->getHeight());
    if (unstable_frames_ <= kSotCoastingOutputFrames) {
      sot_set_tracker_output(&tracker_info, coasting_bbox,
                             last_observed_score_, TrackStatus::NEW);
    }
    return 0;
  }

  float w_scale = context[2] / static_cast<float>(instance_size_);
  float h_scale = context[3] / static_cast<float>(instance_size_);
  auto scale_candidate = [&](const ObjectBoxInfo& candidate) {
    const float candidate_w = candidate.x2 - candidate.x1;
    const float candidate_h = candidate.y2 - candidate.y1;
    std::vector<float> bbox = {
        candidate.x1 * w_scale + context[0],
        candidate.y1 * h_scale + context[1],
        candidate_w * w_scale, candidate_h * h_scale};
    clampBBox(bbox, image);
    return bbox;
  };

  size_t selected_candidate = 0;
  const std::vector<float>& temporal_reference =
      (use_jump_candidate_prior || use_candidate_search_prior ||
       use_search_prior || use_shadow_prior || use_kalman_search_prior)
          ? search_bbox
          : (sot_bbox_valid(last_reliable_template_bbox_)
                 ? last_reliable_template_bbox_
                 : search_bbox);
  const bool primary_quality_available =
      track_response && track_response->response_candidate_count > 0 &&
      std::isfinite(track_response->candidate_peak_margins[0]) &&
      std::isfinite(track_response->candidate_psrs[0]);
  const bool primary_ambiguous = primary_quality_available &&
      (track_response->candidate_psrs[0] < kSotMinResponsePsr ||
       track_response->candidate_peak_margins[0] < kSotMinPeakMargin);
  const bool allow_temporal_topk = low_detail_target &&
      !initialization_bootstrap_active &&
      track_result->bboxes.size() > 1 &&
      (primary_ambiguous || status_ == TrackStatus::LOST ||
       unstable_frames_ > 0 ||
       track_result->bboxes[0].score < tracking_score_threshold_);

  if (allow_temporal_topk) {
    const float reference_area = std::max(
        1.0f, temporal_reference[2] * temporal_reference[3]);
    const float reference_aspect = temporal_reference[3] > 0.0f
        ? temporal_reference[2] / temporal_reference[3] : 1.0f;
    const float reference_cx = temporal_reference[0] +
        temporal_reference[2] * 0.5f;
    const float reference_cy = temporal_reference[1] +
        temporal_reference[3] * 0.5f;
    const float reference_side = std::max(
        8.0f, std::max(temporal_reference[2], temporal_reference[3]));
    const float maximum_distance = std::max(32.0f, reference_side * 4.0f);
    const float minimum_score = std::max(
        kSotCandidateMinimumScore,
        track_result->bboxes[0].score * kSotTopKMinimumScoreRatio);
    float primary_cost = std::numeric_limits<float>::max();
    float best_cost = std::numeric_limits<float>::max();

    for (size_t candidate_index = 0;
         candidate_index < track_result->bboxes.size(); candidate_index++) {
      const ObjectBoxInfo& candidate = track_result->bboxes[candidate_index];
      if (candidate.score < minimum_score) {
        continue;
      }
      std::vector<float> candidate_bbox = normalizeRawScaleBBox(
          scale_candidate(candidate));
      const float candidate_area = std::max(
          1.0f, candidate_bbox[2] * candidate_bbox[3]);
      const float area_ratio = candidate_area / reference_area;
      const float candidate_aspect = candidate_bbox[3] > 0.0f
          ? candidate_bbox[2] / candidate_bbox[3] : reference_aspect;
      const float aspect_ratio = reference_aspect > 0.0f
          ? candidate_aspect / reference_aspect : 1.0f;
      const float candidate_cx = candidate_bbox[0] +
          candidate_bbox[2] * 0.5f;
      const float candidate_cy = candidate_bbox[1] +
          candidate_bbox[3] * 0.5f;
      const float dx = candidate_cx - reference_cx;
      const float dy = candidate_cy - reference_cy;
      const float distance = std::sqrt(dx * dx + dy * dy);
      if (area_ratio < 0.35f || area_ratio > 2.8f ||
          aspect_ratio < 0.45f || aspect_ratio > 2.2f ||
          distance > maximum_distance) {
        continue;
      }

      const float distance_cost = distance / maximum_distance;
      const float area_cost = std::fabs(std::log(area_ratio));
      const float aspect_cost = std::fabs(std::log(aspect_ratio));
      const float score_cost = std::max(
          0.0f, track_result->bboxes[0].score - candidate.score);
      float response_cost = 0.0f;
      if (track_response &&
          candidate_index < track_response->response_candidate_count &&
          std::isfinite(
              track_response->candidate_peak_margins[candidate_index]) &&
          std::isfinite(track_response->candidate_psrs[candidate_index])) {
        const float candidate_psr =
            track_response->candidate_psrs[candidate_index];
        const float candidate_margin =
            track_response->candidate_peak_margins[candidate_index];
        response_cost += std::max(
            0.0f, (kSotMinResponsePsr - candidate_psr) /
                      kSotMinResponsePsr) * 0.20f;
        response_cost += std::max(
            0.0f, (kSotMinPeakMargin - candidate_margin) /
                      kSotMinPeakMargin) * 0.20f;
        if (area_ratio > 1.38f &&
            candidate_psr < kSotMinResponsePsr &&
            candidate_margin < kSotMinPeakMargin) {
          response_cost += 0.25f;
        }
      }
      const float candidate_cost = distance_cost * 0.50f +
          area_cost * 0.25f + aspect_cost * 0.15f + score_cost * 0.25f +
          response_cost;
      if (candidate_index == 0) {
        primary_cost = candidate_cost;
      }
      if (candidate_cost < best_cost) {
        best_cost = candidate_cost;
        selected_candidate = candidate_index;
      }
    }

    if (selected_candidate != 0 &&
        primary_cost < std::numeric_limits<float>::max() &&
        best_cost + kSotTopKReplacementMargin >= primary_cost) {
      selected_candidate = 0;
    }
  }

  const bool response_quality_available = track_response &&
      selected_candidate < track_response->response_candidate_count &&
      std::isfinite(
          track_response->candidate_peak_margins[selected_candidate]) &&
      std::isfinite(track_response->candidate_psrs[selected_candidate]);
  const float response_peak_margin = response_quality_available
      ? track_response->candidate_peak_margins[selected_candidate]
      : std::numeric_limits<float>::max();
  const float response_psr = response_quality_available
      ? track_response->candidate_psrs[selected_candidate]
      : std::numeric_limits<float>::max();
  const ObjectBoxInfo& selected_box =
      track_result->bboxes[selected_candidate];
  last_selected_candidate_ = static_cast<uint32_t>(selected_candidate);
  float score = selected_box.score;
  std::vector<float> scaled_bbox = scale_candidate(selected_box);
  const std::vector<float> raw_scaled_bbox = scaled_bbox;
  const bool bootstrap_bias_sample = initialization_bootstrap_active &&
      selected_candidate == 0 && score >= tracking_score_threshold_ &&
      (!response_quality_available ||
       response_psr >= kSotCandidateMinimumPsr ||
       response_peak_margin >= kSotCandidateMinimumPeakMargin);
  if (bootstrap_bias_sample) {
    observeRawScaleBias(raw_scaled_bbox, size_anchor_bbox_);
  }
  const std::vector<float> normalized_raw_bbox =
      normalizeRawScaleBBox(raw_scaled_bbox);
  sot_apply_bbox_size(scaled_bbox, size_anchor_bbox_, image->getWidth(),
                      image->getHeight());

  // 几何异常检测：中心点位移、面积比、宽高比
  float prev_cx = search_bbox[0] + search_bbox[2] / 2;
  float prev_cy = search_bbox[1] + search_bbox[3] / 2;
  float new_cx = normalized_raw_bbox[0] + normalized_raw_bbox[2] / 2;
  float new_cy = normalized_raw_bbox[1] + normalized_raw_bbox[3] / 2;
  float center_displacement =
      std::sqrt((new_cx - prev_cx) * (new_cx - prev_cx) +
                (new_cy - prev_cy) * (new_cy - prev_cy));

  float prev_area = search_bbox[2] * search_bbox[3];
  float new_area = normalized_raw_bbox[2] * normalized_raw_bbox[3];
  float area_ratio = prev_area > 0 ? new_area / prev_area : 1.0f;

  float prev_aspect =
      search_bbox[3] > 0 ? search_bbox[2] / search_bbox[3] : 1.0f;
  float new_aspect = normalized_raw_bbox[3] > 0
                         ? normalized_raw_bbox[2] / normalized_raw_bbox[3]
                         : 1.0f;
  float aspect_ratio = prev_aspect > 0 ? new_aspect / prev_aspect : 1.0f;
  last_response_score_ = score;
  last_response_psr_ = response_quality_available ? response_psr : 0.0f;
  last_response_peak_margin_ = response_quality_available
      ? response_peak_margin : 0.0f;
  last_response_area_ratio_ = area_ratio;
  last_response_aspect_ratio_ = aspect_ratio;

  float displacement_limit =
      2.5f * std::max(search_bbox[2], 1.0f) +
      applied_search_expansion_motion;
  const float scale_position_gate = std::max(
      12.0f, std::max(search_bbox[2], search_bbox[3]) * 1.25f);
  const float approach_raw_scale =
      sot_bbox_linear_scale(raw_scaled_bbox, size_anchor_bbox_);
  const bool approach_geometry_ok =
      selected_candidate == 0 &&
      !initialization_bootstrap_active &&
      center_displacement <= scale_position_gate &&
      area_ratio >= 0.20f &&
      aspect_ratio >= 0.20f && aspect_ratio <= 4.00f;
  const bool approach_response_ok =
      score >= kSotCandidateMinimumScore &&
      (!response_quality_available ||
       response_psr >= kSotCandidateMinimumPsr ||
       response_peak_margin >= kSotCandidateMinimumPeakMargin);
  const bool grow_observation =
      !context_seed_ &&
      approach_geometry_ok && approach_response_ok &&
      approach_raw_scale >= kSotApproachMinimumRawRatio &&
      approach_raw_scale <= kSotApproachMaximumRawRatio;
  last_approach_geometry_ok_ = approach_geometry_ok;
  last_approach_response_ok_ = approach_response_ok;
  last_approach_raw_scale_ = approach_raw_scale;
  last_center_displacement_ = center_displacement;
  last_scale_position_gate_ = scale_position_gate;
  observeApproachScale(grow_observation);

  const bool approach_grow_active =
      approach_scale_active_ || grow_observation;
  const float geom_area_max = approach_grow_active
      ? kSotApproachGeomAreaMax
      : 2.2f;
  const bool strict_geom_anomaly =
      center_displacement > displacement_limit ||
      area_ratio > geom_area_max || area_ratio < 0.35f ||
      aspect_ratio > 2.0f || aspect_ratio < 0.5f;
  const float bootstrap_displacement_limit = std::max(
      20.0f, std::max(search_bbox[2], search_bbox[3]) * 1.5f);
  const bool bootstrap_geometry_consistent =
      initialization_bootstrap_active &&
      center_displacement <= bootstrap_displacement_limit &&
      area_ratio >= 0.20f && area_ratio <= 4.0f &&
      aspect_ratio >= 0.25f && aspect_ratio <= 4.0f;
  bool geom_anomaly = strict_geom_anomaly &&
                      !bootstrap_geometry_consistent;
  std::vector<float> kalman_bbox;
  if (use_kalman_filter_ && kalman_tracker_) {
    kalman_bbox = kalman_tracker_->predict();
    if (small_factor <= 0.0f && frame_gap > 1) {
      for (uint64_t i = 1; i < frame_gap; i++) {
        kalman_tracker_->update(kalman_bbox, false);
        kalman_bbox = kalman_tracker_->predict();
      }
    }
  }
  score_ratio_ = calculateScoreRatio(score);
  const bool response_ambiguous = response_quality_available &&
      response_psr < kSotMinResponsePsr &&
      response_peak_margin < kSotMinPeakMargin;
  const bool response_identity_ambiguous = response_quality_available &&
      !sot_identity_response_distinct(response_psr,
                                      response_peak_margin);
  const bool bootstrap_response_reliable =
      initialization_bootstrap_active && bootstrap_geometry_consistent &&
      score >= kSotBootstrapMinimumScore;
  if (!use_kalman_filter_ || !kalman_tracker_) {
    const bool response_reliable = bootstrap_response_reliable ||
        (!response_ambiguous && score >= tracking_score_threshold_ &&
         (score_lst_.size() < 10 || score_ratio_ >= 0.45f));
    const bool scale_quality_strong =
        !geom_anomaly && response_reliable &&
        selected_candidate == 0 && !initialization_bootstrap_active &&
        !handoff_identity_active_ &&
        center_displacement <= scale_position_gate &&
        (!response_quality_available || !response_identity_ambiguous);
    const std::vector<float>& scale_observe_bbox =
        approach_grow_active ? raw_scaled_bbox : normalized_raw_bbox;
    observeTrustedScale(scale_observe_bbox,
                        scale_quality_strong || grow_observation,
                        image->getWidth(), image->getHeight());
    if (scale_committed_this_frame_ && last_raw_scale_ratio_ >= 1.0f) {
      last_grow_commit_frame_ = frame_id;
    }
    scaled_bbox = raw_scaled_bbox;
    sot_apply_bbox_size(scaled_bbox, size_anchor_bbox_, image->getWidth(),
                        image->getHeight());
    if (!geom_anomaly && response_reliable) {
      current_bbox_ = scaled_bbox;
      commitReliableScore(score);
      if (response_quality_available) {
        updateReliableResponseBaseline(score, response_psr,
                                       response_peak_margin);
      }
      last_reliable_template_bbox_ = scaled_bbox;
      observeReliableMotion(scaled_bbox, frame_id);
      last_observed_score_ = score;
      search_prior_bbox_ = current_bbox_;
      search_prior_valid_ = false;
      candidate_search_bbox_.clear();
      candidate_search_frames_ = 0;
      candidate_search_miss_frames_ = 0;
      soft_follow_frames_ = 0;
      lost_frames_ = 0;
      suspect_frames_ = 0;
      status_ = TrackStatus::TRACKED;
      tracker_info.box_info_.x1 = scaled_bbox[0];
      tracker_info.box_info_.y1 = scaled_bbox[1];
      tracker_info.box_info_.x2 = scaled_bbox[0] + scaled_bbox[2];
      tracker_info.box_info_.y2 = scaled_bbox[1] + scaled_bbox[3];
      tracker_info.box_info_.score = score;
      tracker_info.status_ = TrackStatus::TRACKED;
      const bool refresh_quality_strong =
          !initialization_bootstrap_active &&
          reliable_response_samples_ >= kSotResponseBaselineMinSamples &&
          response_quality_available &&
          !response_ambiguous &&
          (sot_template_response_strong(
               score, reliable_score_ema_, response_psr,
               reliable_psr_ema_, response_peak_margin,
               reliable_peak_margin_ema_) ||
           (approach_scale_active_ &&
            score >= kSotTemplateRefreshMinimumScore));
      observeReliableTemplate(
          image, current_bbox_, frame_id,
          sot_low_detail_target_factor(current_bbox_),
          refresh_quality_strong);
    } else {
      unstable_frames_++;
      observeUnreliableTemplate(true);
      lost_frames_++;
      suspect_frames_ = 0;
      candidate_search_bbox_.clear();
      candidate_search_frames_ = 0;
      candidate_search_miss_frames_ = 0;
      status_ = TrackStatus::LOST;
      template_recovery_quarantine_pending_ = true;
      resetRecoveryMotionValidation();
      if (!response_reliable) {
        last_response_reject_mask_ |= kSotRejectResponse;
      }
      if (response_ambiguous) {
        last_response_reject_mask_ |= kSotRejectAmbiguous;
      }
      if (geom_anomaly) {
        last_response_reject_mask_ |= kSotRejectGeometry;
      }
      if (unstable_frames_ <= kSotCoastingOutputFrames) {
        sot_set_tracker_output(&tracker_info, search_bbox,
                               last_observed_score_, TrackStatus::NEW);
      }
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

  float size_ratio;
  float current_w_h_ratio =
      scaled_bbox[2] / std::max(scaled_bbox[3], 1.0f);
  if (prev_w_h_ratio_ != 0) {
    size_ratio = current_w_h_ratio / prev_w_h_ratio_;
  } else {
    size_ratio = 1.0f;
  }

  getStatus(scaled_bbox, kalman_bbox, score, score_ratio_, iou, size_ratio);
  if (bootstrap_response_reliable && !geom_anomaly) {
    status_ = TrackStatus::TRACKED;
  }

  const std::vector<float>& reliable_reference =
      sot_bbox_valid(last_reliable_template_bbox_)
          ? last_reliable_template_bbox_
          : current_bbox_;
  const std::vector<float>& candidate_seed_reference =
      use_kalman_search_prior ? kalman_search_bbox
                              : (use_search_prior ? search_bbox
                                                  : reliable_reference);
  const bool lost_state = status_ == TrackStatus::LOST || lost_frames_ > 0;
  const bool consistent_with_reliable = bootstrap_response_reliable ||
      (!geom_anomaly &&
       sot_bbox_consistent(scaled_bbox, reliable_reference,
                           lost_state || use_search_prior ||
                               use_candidate_search_prior ||
                               use_search_expansion ||
                               use_jump_candidate_prior));
  const bool response_reliable =
      bootstrap_response_reliable ||
      (score >= tracking_score_threshold_ &&
       (score_lst_.size() < 10 || score_ratio_ >= 0.45f) &&
       !response_ambiguous);
  const bool response_baseline_ready =
      reliable_response_samples_ >= kSotResponseBaselineMinSamples;
  const float small_target_factor =
      sot_small_target_factor(reliable_reference);
  const bool small_target = small_target_factor > 0.0f;
  const float adaptive_target_factor =
      sot_low_detail_target_factor(reliable_reference);
  const bool adaptive_target = adaptive_target_factor > 0.0f;
  const bool low_detail_motion_search =
      adaptive_target && (use_search_expansion || unstable_frames_ > 0 ||
                          soft_follow_frames_ > 0);
  const float tracked_psr_ratio = low_detail_motion_search
      ? kSotMotionBaselinePsrRatio
      : kSotTrackedBaselinePsrRatio;
  const float tracked_margin_ratio = low_detail_motion_search
      ? kSotMotionBaselineMarginRatio
      : kSotTrackedBaselineMarginRatio;
  const bool tracked_score_degraded = response_baseline_ready &&
      score < std::max(tracking_score_threshold_,
                       reliable_score_ema_ *
                           kSotTrackedBaselineScoreRatio);
  const bool tracked_psr_degraded = response_baseline_ready &&
      response_quality_available &&
      reliable_psr_ema_ > kSotMinResponsePsr &&
      response_psr < reliable_psr_ema_ * tracked_psr_ratio;
  const bool tracked_margin_degraded = response_baseline_ready &&
      response_quality_available &&
      reliable_peak_margin_ema_ > kSotMinPeakMargin &&
      response_peak_margin <
          reliable_peak_margin_ema_ * tracked_margin_ratio;
  const int tracked_degradation_votes =
      (tracked_score_degraded ? 1 : 0) +
      (tracked_psr_degraded ? 1 : 0) +
      (tracked_margin_degraded ? 1 : 0);
  const bool identity_suspect =
      tracked_degradation_votes >= 2 && !approach_grow_active;
  const bool scale_quality_strong =
      status_ != TrackStatus::LOST && lost_frames_ == 0 &&
      selected_candidate == 0 && !initialization_bootstrap_active &&
      !handoff_identity_active_ &&
      !use_search_prior && !use_shadow_prior &&
      !use_jump_candidate_prior && !use_kalman_search_prior &&
      !geom_anomaly && consistent_with_reliable && response_reliable &&
      !identity_suspect && center_displacement <= scale_position_gate &&
      (!response_quality_available || !response_identity_ambiguous);
  const std::vector<float>& scale_observe_bbox =
      approach_grow_active ? raw_scaled_bbox : normalized_raw_bbox;
  observeTrustedScale(scale_observe_bbox,
                      scale_quality_strong || grow_observation,
                      image->getWidth(), image->getHeight());
  if (scale_committed_this_frame_ && last_raw_scale_ratio_ >= 1.0f) {
    last_grow_commit_frame_ = frame_id;
  }
  scaled_bbox = raw_scaled_bbox;
  sot_apply_bbox_size(scaled_bbox, size_anchor_bbox_, image->getWidth(),
                      image->getHeight());
  if (!response_reliable) {
    last_response_reject_mask_ |= kSotRejectResponse;
  }
  if (response_ambiguous) {
    last_response_reject_mask_ |= kSotRejectAmbiguous;
  }
  if (geom_anomaly) {
    last_response_reject_mask_ |= kSotRejectGeometry;
  }
  if (!consistent_with_reliable) {
    last_response_reject_mask_ |= kSotRejectConsistency;
  }
  const float candidate_score_floor = std::max(
      kSotCandidateMinimumScore,
      tracking_score_threshold_ * kSotCandidateScoreFloorRatio);
  const bool candidate_quality_floor =
      score >= candidate_score_floor &&
      (!response_quality_available ||
       response_psr >= kSotCandidateMinimumPsr ||
       response_peak_margin >= kSotCandidateMinimumPeakMargin);
  const bool have_candidate_position =
      candidate_search_frames_ > 0 &&
      sot_bbox_valid(candidate_search_bbox_);
  const bool candidate_position_consistent =
      have_candidate_position &&
      sot_candidate_search_consistent(scaled_bbox, candidate_search_bbox_) &&
      sot_bbox_consistent(scaled_bbox, candidate_search_bbox_, true);
  const bool candidate_position_strictly_consistent =
      have_candidate_position &&
      sot_relock_temporally_consistent(scaled_bbox,
                                       candidate_search_bbox_, false);
  const bool candidate_geometry_consistent =
      !geom_anomaly || (adaptive_target && candidate_position_consistent);
  const bool candidate_search_consistent =
      candidate_geometry_consistent &&
      sot_candidate_search_consistent(scaled_bbox, search_bbox) &&
      (candidate_position_consistent ||
       sot_bbox_consistent(scaled_bbox, reliable_reference, true));
  const bool initial_candidate_search_consistent =
      !have_candidate_position &&
      sot_candidate_search_consistent(scaled_bbox, search_bbox) &&
      sot_bbox_consistent(scaled_bbox, reliable_reference, true);
  const bool candidate_search_followable =
      adaptive_target && candidate_quality_floor &&
      (candidate_search_consistent ||
       initial_candidate_search_consistent);
  const float reliable_area = std::max(
      1.0f, reliable_reference[2] * reliable_reference[3]);
  const float reliable_aspect = reliable_reference[3] > 0.0f
      ? reliable_reference[2] / reliable_reference[3] : 1.0f;
  const float candidate_identity_area_ratio =
      std::max(1.0f, normalized_raw_bbox[2] * normalized_raw_bbox[3]) /
      reliable_area;
  const float candidate_identity_aspect = normalized_raw_bbox[3] > 0.0f
      ? normalized_raw_bbox[2] / normalized_raw_bbox[3]
      : reliable_aspect;
  const float candidate_identity_aspect_ratio = reliable_aspect > 0.0f
      ? candidate_identity_aspect / reliable_aspect : 1.0f;
  const float reliable_side = std::max(
      reliable_reference[2], reliable_reference[3]);
  const float handoff_distance_gate = std::max(24.0f,
                                                reliable_side * 1.35f);
  const float handoff_distance = std::sqrt(
      sot_bbox_center_distance2(normalized_raw_bbox, reliable_reference));
  const std::vector<float>& handoff_motion_reference =
      sot_bbox_valid(kalman_bbox) ? kalman_bbox : reliable_reference;
  float handoff_longitudinal_error = 0.0f;
  float handoff_backward_gate = 0.0f;
  const bool handoff_direction_consistent = candidateDirectionConsistent(
      normalized_raw_bbox, handoff_motion_reference,
      &handoff_longitudinal_error, &handoff_backward_gate);
  if (!handoff_direction_consistent) {
    last_response_reject_mask_ |= kSotRejectRelockDirection;
  }
  // Verify moderate jumps without rejecting them. The wider confirmation
  // gate below still allows continuous camera and target motion.
  const float handoff_verification_distance_gate = std::max(
      12.0f, reliable_side * kSotHandoffIdentityVerifyDistanceRatio);
  const bool approach_area_growth =
      approach_grow_active && selected_candidate == 0 &&
      candidate_identity_area_ratio >= 0.85f &&
      candidate_identity_area_ratio <= kSotApproachIdentityAreaMax &&
      handoff_direction_consistent;
  const float identity_aspect_min = approach_grow_active
      ? kSotApproachIdentityAspectMin
      : kSotHandoffIdentityMinAspectRatio;
  const float identity_aspect_max = approach_grow_active
      ? kSotApproachIdentityAspectMax
      : kSotHandoffIdentityMaxAspectRatio;
  const bool weak_identity_geometry_shift =
      (!approach_area_growth &&
       (candidate_identity_area_ratio < 0.72f ||
        candidate_identity_area_ratio > 1.38f)) ||
      (!approach_grow_active &&
       (candidate_identity_aspect_ratio < 0.70f ||
        candidate_identity_aspect_ratio > 1.43f)) ||
      (!approach_area_growth &&
       handoff_distance > handoff_verification_distance_gate);
  if (response_identity_ambiguous && weak_identity_geometry_shift) {
    last_response_reject_mask_ |= kSotRejectAmbiguous;
  }
  const bool hard_identity_geometry_shift =
      (!approach_area_growth &&
       (candidate_identity_area_ratio < kSotHandoffIdentityMinAreaRatio ||
        candidate_identity_area_ratio > kSotHandoffIdentityMaxAreaRatio)) ||
      candidate_identity_aspect_ratio < identity_aspect_min ||
      candidate_identity_aspect_ratio > identity_aspect_max;
  const bool primary_path_continuation = selected_candidate == 0 &&
      !hard_identity_geometry_shift &&
      handoff_direction_consistent &&
      handoff_distance <= handoff_distance_gate * 1.5f &&
      (!have_candidate_position || candidate_position_consistent);
  const bool large_handoff_displacement =
      camera_motion_guard_frames_ == 0 &&
      handoff_distance > handoff_distance_gate &&
      !approach_area_growth;
  const bool degraded_moving_handoff =
      camera_motion_guard_frames_ == 0 &&
      !approach_area_growth &&
      reliable_motion_stable_frames_ >= kSotReliableMotionMinimumFrames &&
      handoff_distance > handoff_verification_distance_gate &&
      (identity_suspect || response_identity_ambiguous ||
       soft_follow_frames_ > 0);
  const bool candidate_identity_shift =
      selected_candidate != 0 || hard_identity_geometry_shift ||
      !handoff_direction_consistent || large_handoff_displacement ||
      degraded_moving_handoff ||
      (response_identity_ambiguous && weak_identity_geometry_shift &&
       !primary_path_continuation && candidate_search_frames_ >= 2);
  handoff_candidate_distance_ = handoff_distance;
  handoff_candidate_area_ratio_ = candidate_identity_area_ratio;
  handoff_candidate_aspect_ratio_ = candidate_identity_aspect_ratio;
  if (!initialization_bootstrap_active && candidate_search_followable &&
      candidate_identity_shift &&
      !handoff_identity_active_) {
    handoff_identity_active_ = true;
    handoff_identity_confirmed_ = false;
    handoff_identity_good_frames_ = 0;
    handoff_identity_frames_ = 0;

    // Pre-handoff samples must not refresh the template for an unverified
    // identity after the candidate is accepted.
    template_refresh_vote_window_.clear();
    template_refresh_good_votes_ = 0;
    template_bbox_history_.clear();
    template_recovery_stable_frames_ = 0;
    if (!approach_scale_active_ && !last_grow_observation_ &&
        long_term_template_image_ && adaptive_template_image_ &&
        long_term_template_image_ != adaptive_template_image_ &&
        !template_recovery_long_term_tried_) {
      template_recovery_long_term_tried_ = true;
      template_long_term_probe_frames_ = 0;
      activateTemplate(true);
    }
  }
  if (handoff_identity_active_ && !handoff_identity_confirmed_ &&
      status_ != TrackStatus::LOST) {
    handoff_identity_frames_++;
    const int identity_confirm_frames =
        sot_handoff_identity_confirm_frames(handoff_distance,
                                            reliable_side);
    const float identity_area_max = approach_grow_active
        ? kSotApproachIdentityAreaMax
        : kSotHandoffIdentityMaxAreaRatio;
    const float confirm_aspect_min = approach_grow_active
        ? kSotApproachIdentityAspectMin
        : kSotHandoffIdentityMinAspectRatio;
    const float confirm_aspect_max = approach_grow_active
        ? kSotApproachIdentityAspectMax
        : kSotHandoffIdentityMaxAspectRatio;
    const bool identity_geometry_consistent =
        candidate_identity_area_ratio >=
            kSotHandoffIdentityMinAreaRatio &&
        candidate_identity_area_ratio <= identity_area_max &&
        candidate_identity_aspect_ratio >= confirm_aspect_min &&
        candidate_identity_aspect_ratio <= confirm_aspect_max;
    const bool identity_response_consistent =
        response_reliable && !response_identity_ambiguous &&
        (!response_baseline_ready ||
         (response_psr >= reliable_psr_ema_ * 0.45f ||
          response_peak_margin >= reliable_peak_margin_ema_ * 0.35f));
    const float far_handoff_psr_gate = response_baseline_ready
        ? std::max(kSotRelockMinResponsePsr,
                   reliable_psr_ema_ * kSotFarHandoffBaselinePsrRatio)
        : kSotRefinedNearHandoffMinimumPsr;
    const float far_handoff_margin_gate = response_baseline_ready
        ? std::max(kSotRelockMinPeakMargin,
                   reliable_peak_margin_ema_ *
                       kSotFarHandoffBaselineMarginRatio)
        : kSotRefinedNearHandoffMinimumMargin;
    const bool far_handoff_response_distinct =
        response_psr >= far_handoff_psr_gate * kSotFarHandoffPsrBoost ||
        (response_psr >= far_handoff_psr_gate &&
         response_peak_margin >= far_handoff_margin_gate);
    const bool far_handoff_response_strong =
        response_quality_available && !response_identity_ambiguous &&
        score >= std::max(tracking_score_threshold_,
                          reliable_score_ema_ *
                              kSotFarHandoffBaselineScoreRatio) &&
        far_handoff_response_distinct;
    const float refined_near_distance_gate = std::max(
        3.0f, sot_bbox_min_side(reliable_reference) *
                  kSotRefinedNearHandoffDistanceRatio);
    const float refined_near_psr_gate = std::max(
        kSotRefinedNearHandoffMinimumPsr,
        reliable_psr_ema_ * 0.75f);
    const float refined_near_margin_gate = std::max(
        kSotRefinedNearHandoffMinimumMargin,
        reliable_peak_margin_ema_ * 0.50f);
    const bool refined_near_response_distinct =
        response_psr >= refined_near_psr_gate * 1.35f ||
        (response_psr >= refined_near_psr_gate &&
         response_peak_margin >= refined_near_margin_gate);
    const bool refined_near_response_strong =
        response_quality_available &&
        score >= std::max(tracking_score_threshold_,
                          reliable_score_ema_ * 0.85f) &&
        refined_near_response_distinct;
    const bool refined_near_geometry_consistent =
        candidate_identity_area_ratio >= 0.72f &&
        candidate_identity_area_ratio <= 1.38f &&
        candidate_identity_aspect_ratio >= 0.70f &&
        candidate_identity_aspect_ratio <= 1.43f;
    const float refined_near_reliable_speed = std::sqrt(
        reliable_velocity_x_ * reliable_velocity_x_ +
        reliable_velocity_y_ * reliable_velocity_y_);
    const float refined_near_motion_gate = std::max(
        1.0f, sot_bbox_min_side(reliable_reference) *
                  kSotRefinedNearHandoffMotionRatio);
    const bool refined_near_motion_safe =
        stationary_reliable_frames_ >= kSotStationaryReliableFrames &&
        camera_motion_guard_frames_ == 0 &&
        refined_near_reliable_speed <= refined_near_motion_gate;
    const bool refined_near_continuation =
        refined_small_target_ && small_target && selected_candidate == 0 &&
        !template_backup_image_ && !use_search_prior && !use_shadow_prior &&
        !use_jump_candidate_prior && !use_kalman_search_prior &&
        refined_near_motion_safe &&
        handoff_distance <= refined_near_distance_gate &&
        candidate_position_consistent && refined_near_geometry_consistent &&
        refined_near_response_strong;
    const bool identity_template_verified =
        inference_uses_identity_template || refined_near_continuation;
    const bool far_handoff =
        handoff_distance > handoff_distance_gate;
    const bool handoff_position_verified = far_handoff
        ? candidate_position_strictly_consistent &&
              far_handoff_response_strong
        : handoff_distance <= handoff_distance_gate;
    if (identity_template_verified &&
        identity_geometry_consistent && identity_response_consistent &&
        candidate_position_consistent && handoff_direction_consistent &&
        handoff_position_verified) {
      handoff_identity_good_frames_++;
    } else {
      handoff_identity_good_frames_ = 0;
    }
    handoff_identity_confirmed_ =
        handoff_identity_good_frames_ >=
            (refined_near_continuation
                 ? kSotRefinedNearHandoffConfirmFrames
                 : identity_confirm_frames);

  }
  const bool handoff_motion_close =
      handoff_distance <= handoff_distance_gate;
  const bool pending_handoff_path_followable =
      handoff_identity_active_ && !handoff_identity_confirmed_ &&
      handoff_direction_consistent &&
      (handoff_motion_close || candidate_position_strictly_consistent);
  const bool soft_position_followable =
      candidate_search_followable &&
      (!handoff_identity_active_ || handoff_identity_confirmed_ ||
       pending_handoff_path_followable) &&
      soft_follow_frames_ < kSotSoftFollowMaxFrames;
  const float jump_limit_scale =
      2.5f - 1.75f * small_target_factor;
  const float small_target_jump_limit = std::max(
      4.0f, sot_bbox_min_side(reliable_reference) * jump_limit_scale);
  const bool low_detail_target_jump = adaptive_target &&
      soft_follow_frames_ == 0 &&
      !initialization_bootstrap_active && !use_search_expansion &&
      center_displacement > small_target_jump_limit;
  const float relock_baseline_score_ratio = adaptive_target
      ? kSotSmallTargetRelockBaselineScoreRatio
      : kSotRelockBaselineScoreRatio;
  const float relock_baseline_psr_ratio = adaptive_target
      ? kSotSmallTargetRelockBaselinePsrRatio
      : kSotRelockBaselinePsrRatio;
  const float relock_baseline_margin_ratio = adaptive_target
      ? kSotSmallTargetRelockBaselineMarginRatio
      : kSotRelockBaselineMarginRatio;
  const float relock_min_score = response_baseline_ready
      ? std::max(tracking_score_threshold_, reliable_score_ema_ *
                                             relock_baseline_score_ratio)
      : tracking_score_threshold_;
  const float relock_min_psr = response_baseline_ready
      ? std::max(kSotMinResponsePsr,
                 reliable_psr_ema_ * relock_baseline_psr_ratio)
      : kSotRelockMinResponsePsr;
  const float relock_min_peak_margin = response_baseline_ready
      ? std::max(kSotMinPeakMargin,
                 reliable_peak_margin_ema_ *
                     relock_baseline_margin_ratio)
      : kSotRelockMinPeakMargin;
  const bool relock_response_quality = adaptive_target ?
      (response_psr >= relock_min_psr ||
       response_peak_margin >= relock_min_peak_margin) :
      (response_psr >= relock_min_psr &&
       response_peak_margin >= relock_min_peak_margin);
  const bool relock_response_strong =
      score >= relock_min_score &&
      response_quality_available && !response_ambiguous &&
      relock_response_quality;
  const bool relock_response_reliable = response_reliable &&
      (score_lst_.size() < 10 ||
       score_ratio_ >= (adaptive_target
                            ? kSotSmallTargetRelockScoreRatio
                            : kSotRelockMinScoreRatio)) &&
      relock_response_strong;
  const bool adaptive_candidate_confirmable =
      adaptive_target && response_reliable && candidate_quality_floor &&
      candidate_position_consistent && candidate_geometry_consistent;
  bool soft_follow_recovery_confirmed =
      soft_follow_frames_ == 0 &&
      (!handoff_identity_active_ || handoff_identity_confirmed_);
  if (soft_follow_frames_ > 0) {
    // The reliable box is intentionally not moved by weak responses. Recover
    // from FOLLOWING against the continuous candidate path instead of the
    // stale reliable position, otherwise a genuinely moving target can never
    // become reliable again.
    if (response_reliable && candidate_position_consistent &&
        (!identity_suspect || adaptive_candidate_confirmable)) {
      soft_follow_recovery_frames_++;
    } else {
      soft_follow_recovery_frames_ = 0;
    }
    const bool recovery_motion_confirmed =
        observeRecoveryMotionCandidate(scaled_bbox, frame_id);
    const int recovery_frames = adaptive_target && !small_target
        ? kSotSoftFollowRecoveryFrames - 1
        : kSotSoftFollowRecoveryFrames;
    soft_follow_recovery_confirmed =
        soft_follow_recovery_frames_ >= recovery_frames &&
        recovery_motion_confirmed &&
        (!handoff_identity_active_ || handoff_identity_confirmed_);
  }
  bool jump_confirmed = true;
  if (use_jump_candidate_prior) {
    jump_confirmed = response_reliable && consistent_with_reliable &&
        !identity_suspect &&
        sot_relock_seed_consistent(scaled_bbox, jump_candidate_bbox_);
    if (jump_confirmed) {
      jump_candidate_bbox_ = scaled_bbox;
      jump_candidate_frames_++;
      search_expansion_motion_ = 0.0f;
      search_expansion_frames_ = 0;
    } else {
      jump_candidate_bbox_.clear();
      jump_candidate_frames_ = 0;
    }
  } else if (low_detail_target_jump && response_reliable &&
      consistent_with_reliable && !identity_suspect) {
    jump_candidate_bbox_ = scaled_bbox;
    jump_candidate_frames_ = 1;
    search_expansion_motion_ = 0.0f;
    search_expansion_frames_ = 0;
    jump_confirmed = false;
  } else {
    jump_candidate_bbox_.clear();
    jump_candidate_frames_ = 0;
  }
  if (status_ == TrackStatus::TRACKED) {
    const bool confirmed_adaptive_continuation =
        adaptive_candidate_confirmable && soft_follow_frames_ > 0 &&
        soft_follow_recovery_confirmed;
    const bool reliable_position_consistent = consistent_with_reliable ||
        (soft_follow_frames_ > 0 && soft_follow_recovery_confirmed);
    if (response_reliable && reliable_position_consistent &&
        (!identity_suspect || confirmed_adaptive_continuation) &&
        jump_confirmed &&
        soft_follow_recovery_confirmed) {
      const bool recovered_from_following = soft_follow_frames_ > 0;
      status_ = TrackStatus::TRACKED;
      kalman_tracker_->update(scaled_bbox, scale_committed_this_frame_);
      commitReliableScore(score);
      if (response_quality_available) {
        updateReliableResponseBaseline(score, response_psr,
                                       response_peak_margin);
      }
      last_reliable_template_bbox_ = scaled_bbox;
      observeReliableMotion(scaled_bbox, frame_id);
      last_observed_score_ = score;
      if (adaptive_target) {
        observeCandidateMotion(scaled_bbox, frame_id);
      } else {
        resetCandidateMotion();
      }
      resetRelockCandidate();
      unstable_frames_ = 0;
      current_bbox_ = last_reliable_template_bbox_;
      sot_info_.template_bbox = last_reliable_template_bbox_;
      search_prior_bbox_ = current_bbox_;
      search_prior_valid_ = false;
      prev_w_h_ratio_ = current_w_h_ratio;
      lost_frames_ = 0;
      suspect_frames_ = 0;
      jump_candidate_bbox_.clear();
      jump_candidate_frames_ = 0;
      candidate_search_bbox_.clear();
      candidate_search_frames_ = 0;
      candidate_search_miss_frames_ = 0;
      soft_follow_frames_ = 0;
      soft_follow_recovery_frames_ = 0;
      handoff_identity_active_ = false;
      handoff_identity_confirmed_ = false;
      handoff_identity_good_frames_ = 0;
      handoff_identity_frames_ = 0;
      tracker_info.status_ = TrackStatus::TRACKED;
      if (recovered_from_following) {
        applyRecoveryTemplateQuarantine(frame_id);
      }
      resetRecoveryMotionValidation();
      const bool refresh_quality_strong =
          !recovered_from_following &&
          !initialization_bootstrap_active &&
          response_baseline_ready &&
          response_quality_available &&
          response_reliable &&
          !identity_suspect && !response_ambiguous &&
          (sot_template_response_strong(
               score, reliable_score_ema_, response_psr,
               reliable_psr_ema_, response_peak_margin,
               reliable_peak_margin_ema_) ||
           (approach_scale_active_ &&
            score >= kSotTemplateRefreshMinimumScore));
      observeReliableTemplate(image, current_bbox_, frame_id,
                              adaptive_target_factor,
                              refresh_quality_strong);
    } else if (((consistent_with_reliable &&
                 score >= tracking_score_threshold_) ||
                candidate_search_followable) &&
               ((response_reliable && identity_suspect) ||
                adaptive_target)) {
      unstable_frames_++;
      observeUnreliableTemplate(false);
      suspect_frames_++;
      if (candidate_search_followable &&
          candidate_search_frames_ < kSotCandidateSearchMaxFrames) {
        observeCandidateMotion(scaled_bbox, frame_id);
        candidate_search_bbox_ = scaled_bbox;
        candidate_search_frames_++;
        candidate_search_miss_frames_ = 0;
        search_expansion_motion_ = 0.0f;
        search_expansion_frames_ = 0;
      } else if (candidate_search_followable) {
        observeCandidateMotion(scaled_bbox, frame_id);
        candidate_search_bbox_ = scaled_bbox;
        candidate_search_miss_frames_ = 0;
      } else {
        std::vector<float> candidate_bridge_bbox;
        if (adaptive_target && buildCandidateMotionBridge(
                                frame_id, image->getWidth(),
                                image->getHeight(),
                                &candidate_bridge_bbox)) {
          candidate_search_bbox_ = candidate_bridge_bbox;
          candidate_search_frames_ =
              std::max(candidate_search_frames_, 1);
          candidate_search_miss_frames_ =
              candidate_motion_bridge_frames_;
          predicted_output_bbox = candidate_bridge_bbox;
          predicted_output_score = last_observed_score_;
        } else {
          candidate_search_bbox_.clear();
          candidate_search_frames_ = 0;
          candidate_search_miss_frames_ = 0;
        }
      }
      if (candidate_search_followable) {
        predicted_output_bbox = scaled_bbox;
        predicted_output_score = score;
        predicted_output_observed = true;
      }
      if (soft_position_followable) {
        if (soft_follow_frames_ == 0) {
          template_recovery_quarantine_pending_ = true;
          beginRecoveryMotionValidation(reliable_reference, frame_id);
          (void)observeRecoveryMotionCandidate(scaled_bbox, frame_id);
        }
        soft_follow_frames_++;
        if (weak_follow_count_ < std::numeric_limits<uint32_t>::max()) {
          weak_follow_count_++;
        }
        unstable_frames_ = 0;
        suspect_frames_ = 0;
        lost_frames_ = 0;
        if (handoff_identity_active_ && !handoff_identity_confirmed_) {
          candidate_output_held_ = true;
          candidate_output_following_ =
              use_candidate_search_prior &&
              candidate_position_strictly_consistent;
          current_bbox_ = reliable_reference;
          predicted_output_bbox = current_bbox_;
          predicted_output_score = last_observed_score_;
          predicted_output_observed = true;
          sot_info_.template_bbox = reliable_reference;
          tracker_info.status_ = TrackStatus::CANDIDATE;
        } else {
          current_bbox_ = scaled_bbox;
          sot_info_.template_bbox = current_bbox_;
          tracker_info.status_ = TrackStatus::FOLLOWING;
        }
      } else {
        current_bbox_ = reliable_reference;
        sot_info_.template_bbox = current_bbox_;
        if (handoff_identity_active_ && !handoff_identity_confirmed_) {
          candidate_output_held_ = true;
          tracker_info.status_ = TrackStatus::CANDIDATE;
        } else {
          tracker_info.status_ = TrackStatus::LOST;
        }
      }
      const int suspect_confirm_frames = adaptive_target
          ? kSotSmallTargetSuspectConfirmFrames
          : kSotSuspectConfirmFrames;
      if (!soft_position_followable &&
          suspect_frames_ >= suspect_confirm_frames) {
        const bool identity_was_pending =
            handoff_identity_active_ && !handoff_identity_confirmed_;
        status_ = TrackStatus::LOST;
        tracker_info.status_ = TrackStatus::LOST;
        if (identity_was_pending) {
          candidate_output_held_ = false;
          current_bbox_ = reliable_reference;
          sot_info_.template_bbox = current_bbox_;
          predicted_output_bbox = current_bbox_;
          predicted_output_score = last_observed_score_;
          predicted_output_observed = false;
        }
        candidate_search_bbox_.clear();
        candidate_search_frames_ = 0;
        candidate_search_miss_frames_ = 0;
        resetRelockCandidate();
        soft_follow_recovery_frames_ = 0;
        template_recovery_quarantine_pending_ = true;
        resetRecoveryMotionValidation();
        handoff_identity_active_ = false;
        handoff_identity_confirmed_ = false;
        handoff_identity_good_frames_ = 0;
        handoff_identity_frames_ = 0;
        lost_frames_ = 1;
      }
    } else {
      unstable_frames_++;
      suspect_frames_++;
      std::vector<float> deferred_bbox;
      const bool have_motion_bridge = adaptive_target &&
          buildCandidateMotionBridge(frame_id, image->getWidth(),
                                     image->getHeight(), &deferred_bbox);
      current_bbox_ = have_motion_bridge ? deferred_bbox
                                         : reliable_reference;
      sot_info_.template_bbox = current_bbox_;
      if (have_motion_bridge) {
        candidate_search_bbox_ = deferred_bbox;
        candidate_search_frames_ = std::max(candidate_search_frames_, 1);
        candidate_search_miss_frames_ = candidate_motion_bridge_frames_;
      } else {
        candidate_search_bbox_.clear();
        candidate_search_frames_ = 0;
        candidate_search_miss_frames_ = 0;
      }
      soft_follow_recovery_frames_ = 0;
      const int hard_lost_confirm_frames = adaptive_target
          ? kSotSmallTargetHardLostConfirmFrames
          : kSotSuspectConfirmFrames;
      if (suspect_frames_ >= hard_lost_confirm_frames) {
        const bool identity_was_pending =
            handoff_identity_active_ && !handoff_identity_confirmed_;
        observeUnreliableTemplate(true);
        status_ = TrackStatus::LOST;
        tracker_info.status_ = TrackStatus::LOST;
        if (identity_was_pending) {
          candidate_output_held_ = false;
          current_bbox_ = reliable_reference;
          sot_info_.template_bbox = current_bbox_;
          predicted_output_bbox = current_bbox_;
          predicted_output_score = last_observed_score_;
          predicted_output_observed = false;
        }
        resetRelockCandidate();
        template_recovery_quarantine_pending_ = true;
        resetRecoveryMotionValidation();
        handoff_identity_active_ = false;
        handoff_identity_confirmed_ = false;
        handoff_identity_good_frames_ = 0;
        handoff_identity_frames_ = 0;
        lost_frames_ = 1;
      } else {
        observeUnreliableTemplate(false);
        status_ = TrackStatus::TRACKED;
        if (handoff_identity_active_ && !handoff_identity_confirmed_) {
          candidate_output_held_ = true;
          current_bbox_ = reliable_reference;
          sot_info_.template_bbox = current_bbox_;
          tracker_info.status_ = TrackStatus::CANDIDATE;
        } else {
          tracker_info.status_ = TrackStatus::FOLLOWING;
        }
        lost_frames_ = 0;
        if (hard_lost_deferred_count_ <
            std::numeric_limits<uint32_t>::max()) {
          hard_lost_deferred_count_++;
        }
      }
    }
  } else {
    const bool have_shadow = sot_bbox_valid(shadow_bbox_);
    const bool response_candidate =
        relock_response_reliable && !geom_anomaly;
    if (!relock_response_reliable) {
      last_response_reject_mask_ |= kSotRejectRelockQuality;
    }
    float relock_candidate_distance = 0.0f;
    float relock_motion_gate = 0.0f;
    const std::vector<float>& relock_motion_reference =
        use_kalman_search_prior ? kalman_search_bbox
                                : candidate_seed_reference;
    const bool relock_motion_consistent = sot_relock_motion_consistent(
        scaled_bbox, relock_motion_reference, reliable_reference,
        lost_frames_, &relock_candidate_distance, &relock_motion_gate);
    float relock_longitudinal_error = 0.0f;
    float relock_backward_gate = 0.0f;
    const bool relock_direction_consistent = candidateDirectionConsistent(
        scaled_bbox, relock_motion_reference,
        &relock_longitudinal_error, &relock_backward_gate);
    const bool relock_candidate_consistent =
        relock_motion_consistent && relock_direction_consistent;
    last_relock_candidate_distance_ = relock_candidate_distance;
    last_relock_motion_gate_ = relock_motion_gate;
    const bool shadow_position_consistent =
        have_shadow && sot_relock_temporally_consistent(
                           scaled_bbox, shadow_bbox_, adaptive_target);
    const bool shadow_continuation =
        response_candidate && shadow_position_consistent &&
        relock_direction_consistent;
    const bool first_shadow_candidate =
        !have_shadow && response_candidate &&
        relock_candidate_consistent &&
        sot_relock_seed_consistent(scaled_bbox, candidate_seed_reference);
    const float relock_target_side = std::max(
        reliable_reference[2], reliable_reference[3]);
    const float near_relock_gate = std::max(
        24.0f, relock_target_side * 1.5f);
    const bool near_relock = adaptive_target && lost_frames_ <= 12 &&
        relock_candidate_consistent &&
        relock_candidate_distance <= near_relock_gate;
    bool candidate_accepted = false;
    tracker_info.status_ = TrackStatus::LOST;
    if (shadow_continuation) {
      shadow_bbox_ = scaled_bbox;
      recordRelockVote(true);
      candidate_accepted = true;
    } else if (first_shadow_candidate) {
      resetRelockCandidate();
      shadow_bbox_ = scaled_bbox;
      recordRelockVote(true);
      candidate_accepted = true;
    } else if (have_shadow) {
      recordRelockMiss();
    } else {
      if (response_candidate && !relock_candidate_consistent) {
        relock_motion_reject_count_++;
        if (!relock_motion_consistent) {
          last_response_reject_mask_ |= kSotRejectRelockMotion;
        }
        if (!relock_direction_consistent) {
          last_response_reject_mask_ |= kSotRejectRelockDirection;
        }
      }
      resetRelockCandidate();
    }

    if (!candidate_accepted && !sot_bbox_valid(shadow_bbox_) &&
        handoff_identity_active_ && !handoff_identity_confirmed_) {
      handoff_identity_active_ = false;
      handoff_identity_good_frames_ = 0;
      handoff_identity_frames_ = 0;
    }

    if (candidate_accepted) {
      if (!handoff_identity_active_) {
        handoff_identity_active_ = true;
        handoff_identity_confirmed_ = false;
        handoff_identity_good_frames_ = 0;
        handoff_identity_frames_ = 0;

        if (!approach_scale_active_ && !last_grow_observation_ &&
            long_term_template_image_ && adaptive_template_image_ &&
            long_term_template_image_ != adaptive_template_image_ &&
            !template_recovery_long_term_tried_) {
          template_recovery_long_term_tried_ = true;
          template_long_term_probe_frames_ = 0;
          activateTemplate(true);
        }
      }
      handoff_identity_frames_++;
      const int identity_confirm_frames =
          sot_handoff_identity_confirm_frames(relock_candidate_distance,
                                              relock_target_side);
      const bool relock_identity_geometry_consistent =
          candidate_identity_area_ratio >=
              kSotHandoffIdentityMinAreaRatio &&
          candidate_identity_area_ratio <=
              kSotHandoffIdentityMaxAreaRatio &&
          candidate_identity_aspect_ratio >=
              kSotHandoffIdentityMinAspectRatio &&
          candidate_identity_aspect_ratio <=
              kSotHandoffIdentityMaxAspectRatio;
      const bool relock_identity_response_consistent =
          inference_uses_identity_template && response_quality_available &&
          !response_identity_ambiguous;
      if (relock_identity_geometry_consistent &&
          relock_identity_response_consistent &&
          relock_candidate_consistent) {
        handoff_identity_good_frames_++;
      } else {
        handoff_identity_good_frames_ = 0;
      }
      handoff_identity_confirmed_ = handoff_identity_good_frames_ >=
          identity_confirm_frames;
      if (adaptive_target) {
        observeCandidateMotion(scaled_bbox, frame_id);
      }
      candidate_search_bbox_ = scaled_bbox;
      candidate_search_frames_ = std::max(candidate_search_frames_, 1);
      candidate_search_miss_frames_ = 0;
      candidate_output_held_ = !handoff_identity_confirmed_;
      predicted_output_bbox = candidate_output_held_
          ? reliable_reference : scaled_bbox;
      predicted_output_score = candidate_output_held_
          ? last_observed_score_ : score;
      predicted_output_observed = true;
    } else if (sot_bbox_valid(shadow_bbox_)) {
      predicted_output_bbox = shadow_bbox_;
      predicted_output_score = score;
    }

    if (!candidate_accepted && adaptive_target) {
      std::vector<float> candidate_bridge_bbox;
      if (buildCandidateMotionBridge(frame_id, image->getWidth(),
                                     image->getHeight(),
                                     &candidate_bridge_bbox)) {
        candidate_search_bbox_ = candidate_bridge_bbox;
        candidate_search_frames_ = std::max(candidate_search_frames_, 1);
        candidate_search_miss_frames_ = candidate_motion_bridge_frames_;
        predicted_output_bbox = candidate_bridge_bbox;
        predicted_output_score = last_observed_score_;
      }
    }
    if (handoff_identity_active_ && !handoff_identity_confirmed_) {
      candidate_output_held_ = true;
      predicted_output_bbox = reliable_reference;
      predicted_output_score = last_observed_score_;
      predicted_output_observed = true;
    }

    if (candidate_accepted && relockConfirmed(near_relock) &&
        handoff_identity_confirmed_) {
      status_ = TrackStatus::TRACKED;
      kalman_tracker_->update(scaled_bbox, false);
      commitReliableScore(score);
      if (response_quality_available) {
        updateReliableResponseBaseline(score, response_psr,
                                       response_peak_margin);
      }
      last_reliable_template_bbox_ = scaled_bbox;
      observeReliableMotion(scaled_bbox, frame_id);
      last_observed_score_ = score;
      current_bbox_ = scaled_bbox;
      sot_info_.template_bbox = current_bbox_;
      search_prior_bbox_ = current_bbox_;
      search_prior_valid_ = false;
      prev_w_h_ratio_ = current_w_h_ratio;
      resetRelockCandidate();
      unstable_frames_ = 0;
      suspect_frames_ = 0;
      jump_candidate_bbox_.clear();
      jump_candidate_frames_ = 0;
      candidate_search_bbox_.clear();
      candidate_search_frames_ = 0;
      candidate_search_miss_frames_ = 0;
      soft_follow_frames_ = 0;
      soft_follow_recovery_frames_ = 0;
      handoff_identity_active_ = false;
      handoff_identity_confirmed_ = false;
      handoff_identity_good_frames_ = 0;
      handoff_identity_frames_ = 0;
      lost_frames_ = 0;
      tracker_info.status_ = TrackStatus::TRACKED;
      applyRecoveryTemplateQuarantine(frame_id);
      resetRecoveryMotionValidation();
      observeReliableTemplate(image, current_bbox_, frame_id,
                              adaptive_target_factor,
                              false);
    } else {
      if (candidate_accepted) {
        template_recovery_miss_frames_ = 0;
        template_recovery_stable_frames_ = 0;
        recordTemplateRefreshVote(false);
        observeTemplateProbation(TemplateProbationObservation::NEUTRAL);
      } else {
        observeUnreliableTemplate(true);
      }
      current_bbox_ = reliable_reference;
      sot_info_.template_bbox = current_bbox_;
      lost_frames_++;
    }
  }

  if (tracker_info.status_ == TrackStatus::TRACKED) {
    sot_set_tracker_output(&tracker_info, current_bbox_, score,
                           TrackStatus::TRACKED);
  } else if (tracker_info.status_ == TrackStatus::FOLLOWING) {
    sot_set_tracker_output(&tracker_info, current_bbox_, score,
                           TrackStatus::FOLLOWING);
  } else if (tracker_info.status_ == TrackStatus::CANDIDATE &&
             sot_bbox_valid(current_bbox_)) {
    sot_set_tracker_output(&tracker_info, current_bbox_,
                           last_observed_score_, TrackStatus::CANDIDATE);
  } else if (sot_bbox_valid(predicted_output_bbox) ||
             unstable_frames_ + lost_frames_ <= kSotCoastingOutputFrames) {
    std::vector<float> coasting_bbox = predicted_output_bbox;
    float coasting_score = predicted_output_score;
    if (!sot_bbox_valid(coasting_bbox)) {
      coasting_bbox = search_bbox;
      coasting_score = last_observed_score_;
    }
    if (!adaptive_target && !sot_bbox_valid(predicted_output_bbox) &&
        !use_search_prior &&
        !use_shadow_prior && !use_candidate_search_prior &&
        sot_bbox_valid(kalman_bbox) &&
        sot_bbox_consistent(kalman_bbox, search_bbox, true)) {
      coasting_bbox = kalman_bbox;
    }
    sot_clamp_bbox_position(coasting_bbox, image->getWidth(),
                            image->getHeight());
    sot_set_tracker_output(
        &tracker_info, coasting_bbox, coasting_score,
        predicted_output_observed ? TrackStatus::CANDIDATE : TrackStatus::NEW);
  }
  frame_id_ = frame_id;
  return 0;
}
