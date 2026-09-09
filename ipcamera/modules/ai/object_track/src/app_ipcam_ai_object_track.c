#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "stdbool.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "app_ipcam_ai.h"
#include "app_ipcam_sys.h"
#include "app_ipcam_object_track_gmc.h"
#ifdef OSDC_SUPPORT
#include "app_ipcam_osd.h"
#endif
#include "tdl_sdk.h"
#include <pthread.h>
#include <stdio.h>
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define FEATURE_SIZE 256
#define MAX_DET_NUM 100
#define TRACK_REQUEST_PATH "/tmp/track"
#define DET_TRACK_REQUEST_PATH "/tmp/det_track"
#define TRACK_FPS_STATUS_PATH "/tmp/object_track_fps"
#define TRACK_FPS_STATUS_TMP_PATH "/tmp/object_track_fps.tmp"
#define TRACK_PERF_WINDOW_US (1000ULL * 1000ULL)
#define TRACK_DET_HANDOFF_MAX_AGE_US (500ULL * 1000ULL)
#define TRACK_DET_CENTER_LATCH_MAX_AGE_US (180ULL * 1000ULL)
#define TRACK_DET_CENTER_REQUEST_TIMEOUT_US (1200ULL * 1000ULL)
#define DEFAULT_SELECTION_BOX_SIZE 120
#define OBJECT_TRACK_REFERENCE_CHN 0
#define OBJECT_TRACK_SOT_POOL 7
#define OBJECT_TRACK_PIPELINE_READY_RETRIES 5
#define OBJECT_TRACK_PIPELINE_READY_TIMEOUT_MS 100
#define OBJECT_TRACK_PIPELINE_DRAIN_LIMIT 8
#define OBJECT_TRACK_OSD_HOLD_FRAMES 8
#define OBJECT_TRACK_SMALL_TARGET_OSD_HOLD_FRAMES 120
#define OBJECT_TRACK_PREDICTED_DISPLAY_FRAMES 2
#define OBJECT_TRACK_LOST_TIMEOUT_US (1200ULL * 1000ULL)
#define OBJECT_TRACK_CANDIDATE_TIMEOUT_US (1200ULL * 1000ULL)
#define OBJECT_TRACK_RECOVERY_CONFIRM_FRAMES 2
#define OBJECT_TRACK_FRAME_TIMEOUT_MS 250
#define OBJECT_TRACK_FRAME_ERROR_LIMIT 3
#define OBJECT_TRACK_GMC_MIN_INTERVAL 2
#define OBJECT_TRACK_GMC_RESULT_MAX_AGE_FRAMES 8
#define OBJECT_TRACK_GMC_RECOVERY_MIN_CONFIDENCE 0.15f
#define OBJECT_TRACK_GMC_STABLE_MIN_CONFIDENCE 0.30f
#define OBJECT_TRACK_GMC_SMALL_TARGET_MAX_AGE_FRAMES 2ULL
#define OBJECT_TRACK_GMC_SMALL_TARGET_RECOVERY_MIN_CONFIDENCE 0.08f
#define OBJECT_TRACK_GMC_SMALL_TARGET_STABLE_MIN_CONFIDENCE 0.08f
#define OBJECT_TRACK_GMC_SMALL_TARGET_MAX_MOTION_HORIZON 3U
#define OBJECT_TRACK_TINY_TARGET_SOT_SIDE 16.0f
#define OBJECT_TRACK_ADAPTIVE_TARGET_SOT_SIDE 80.0f
#define OBJECT_TRACK_FEARTRACK_INIT_BOX_SIZE 80
#ifdef OSDC_SUPPORT
#define OBJECT_TRACK_OSD_PREDICT_LEAD_FRAMES 0.0f
#define OBJECT_TRACK_OSD_VELOCITY_ALPHA 0.75f
#define OBJECT_TRACK_OSD_DIRECTION_CHANGE_SCALE 0.45f
#define OBJECT_TRACK_OSD_DEAD_ZONE_PX 0.75f
#define OBJECT_TRACK_OSD_MAX_TIME_REF_GAP 8U
#define OBJECT_TRACK_OSD_MIN_FRAME_PERIOD_US 5000ULL
#define OBJECT_TRACK_OSD_MAX_FRAME_PERIOD_US 100000ULL
#endif
#define OBJECT_TRACK_DISPLAY_CENTER_ALPHA 0.70f
#define OBJECT_TRACK_DISPLAY_SIZE_ALPHA 0.35f
#define OBJECT_TRACK_DISPLAY_SIZE_DEADBAND 0.015f
#define OBJECT_TRACK_DISPLAY_SNAP_CENTER_RATIO 0.45f

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef enum APP_OBJECT_TRACK_REQUEST_TYPE_T {
    APP_OBJECT_TRACK_REQUEST_NONE = 0,
    APP_OBJECT_TRACK_REQUEST_DEFAULT,
    APP_OBJECT_TRACK_REQUEST_STOP,
    APP_OBJECT_TRACK_REQUEST_POINT,
    APP_OBJECT_TRACK_REQUEST_BOX,
    APP_OBJECT_TRACK_REQUEST_ID,
    APP_OBJECT_TRACK_REQUEST_DET_CENTER,
} APP_OBJECT_TRACK_REQUEST_TYPE_E;

typedef struct APP_OBJECT_TRACK_REQUEST_T {
    APP_OBJECT_TRACK_REQUEST_TYPE_E type;
    CVI_BOOL search_type_override;
    TDLTargetSearchTypeE search_type;
    int32_t point_x;
    int32_t point_y;
    int32_t view_width;
    int32_t view_height;
    int32_t box[4];
    uint64_t track_id;
    uint64_t request_time_us;
} APP_OBJECT_TRACK_REQUEST_S;

typedef struct APP_OBJECT_TRACK_SELECTION_T {
    CVI_BOOL valid;
    CVI_BOOL from_det;
    CVI_BOOL point_prompt;
    CVI_BOOL search_type_override;
    TDLTargetSearchTypeE search_type;
    int32_t class_id;
    uint64_t track_id;
    int32_t point[2];
    int32_t box[4];
} APP_OBJECT_TRACK_SELECTION_S;

typedef struct APP_OBJECT_TRACK_DISPLAY_FILTER_T {
    CVI_BOOL initialized;
    CVI_FLOAT cx;
    CVI_FLOAT cy;
    CVI_FLOAT width;
    CVI_FLOAT height;
} APP_OBJECT_TRACK_DISPLAY_FILTER_S;

#ifdef OSDC_SUPPORT
typedef struct APP_OBJECT_TRACK_OSD_PREDICTOR_T {
    CVI_BOOL initialized;
    CVI_BOOL velocity_valid;
    CVI_FLOAT prev_cx;
    CVI_FLOAT prev_cy;
    CVI_FLOAT velocity_x_per_us;
    CVI_FLOAT velocity_y_per_us;
    CVI_FLOAT frame_period_us;
    CVI_U32 prev_time_ref;
    CVI_U64 prev_pts;
} APP_OBJECT_TRACK_OSD_PREDICTOR_S;
#endif

typedef struct APP_OBJECT_TRACK_GMC_ASYNC_T {
    pthread_t thread;
    CVI_BOOL started;
    CVI_BOOL running;
    CVI_BOOL busy;
    CVI_BOOL pending;
    CVI_BOOL result_ready;
    APP_OBJECT_TRACK_GMC_SAMPLE_S pending_sample;
    CVI_U64 epoch;
    CVI_U64 pending_epoch;
    CVI_U64 result_epoch;
    CVI_U64 result_frame_id;
    CVI_U32 input_count;
    CVI_S32 result_status;
    APP_OBJECT_TRACK_GMC_RESULT_S result;
} APP_OBJECT_TRACK_GMC_ASYNC_S;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_OBJECT_TRACK_MODE g_mode = WAIT_TARGET;
static uint64_t g_frame_id = 0;
static APP_PARAM_AI_OBJECT_TRACK_CFG_S g_stObjTrackCfg;
static APP_PARAM_AI_OBJECT_TRACK_CFG_S *g_pstObjTrackCfg = &g_stObjTrackCfg;
static volatile bool g_bObjectTrackRunning = CVI_FALSE;
static volatile bool g_bObjectTrackPause = CVI_FALSE;
static pthread_t g_ObjectTrackHandle;
static TDLHandle g_ObjectTrackTDLHandle;
static TDLObject g_stObjDraw = {0};
SMT_MUTEXAUTOLOCK_INIT(g_Mutex);
static pthread_mutex_t g_StatusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_ModeMutex = PTHREAD_MUTEX_INITIALIZER;
static CVI_BOOL g_bSharedPipelinePrepared = CVI_FALSE;
static VPSS_CHN_ATTR_S g_stDetPipelineAttr = {0};
static VPSS_CHN_ATTR_S g_stSotPipelineAttr = {0};
static VB_POOL g_DetPipelinePool = VB_INVALID_POOLID;
static VB_POOL g_SotPipelinePool = VB_INVALID_POOLID;
static CVI_BOOL g_bPipelineInitialized = CVI_FALSE;
static APP_PARAM_OBJECT_TRACK_MODE g_PipelineMode = DETECTION;
static APP_OBJECT_TRACK_GMC_STATE_S g_GmcState;
static CVI_S32 g_GmcExclusion[4] = {0};
static APP_OBJECT_TRACK_GMC_ASYNC_S g_GmcAsync = {0};
static pthread_mutex_t g_GmcAsyncMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_GmcAsyncCond = PTHREAD_COND_INITIALIZER;
static APP_OBJECT_TRACK_DISPLAY_FILTER_S g_DisplayFilter = {0};

typedef struct APP_OBJECT_TRACK_PERF_T {
    APP_PARAM_OBJECT_TRACK_MODE mode;
    uint64_t window_start_us;
    uint64_t input_frames;
    uint64_t det_frames;
    uint64_t sot_frames;
    uint64_t sot_init_frames;
    uint64_t sot_observed_frames;
    uint64_t sot_following_frames;
    uint64_t sot_candidate_frames;
    uint64_t sot_predicted_frames;
    uint64_t sot_lost_frames;
    uint64_t stale_frames;
    uint64_t gmc_samples;
    uint64_t gmc_baseline_samples;
    uint64_t gmc_evaluations;
    uint64_t gmc_failures;
    uint64_t gmc_valid;
    uint64_t gmc_local_valid;
    uint64_t gmc_applied;
    uint64_t gmc_expanded;
    uint64_t frame_wait_total_us;
    uint64_t frame_drain_total_us;
    uint64_t wrap_total_us;
    uint64_t gmc_cache_total_us;
    uint64_t gmc_grid_total_us;
    uint64_t gmc_search_total_us;
    uint64_t gmc_local_search_total_us;
    uint64_t gmc_total_us;
    uint64_t det_total_us;
    uint64_t sot_total_us;
    uint64_t sot_init_total_us;
    uint64_t result_total_us;
    uint64_t cleanup_total_us;
    double sot_model_preprocess_total_ms;
    double sot_model_tpu_total_ms;
    double sot_model_postprocess_total_ms;
    double sot_model_total_ms;
    float gmc_last_dx;
    float gmc_last_dy;
    float gmc_last_applied_dx;
    float gmc_last_applied_dy;
    float gmc_last_confidence;
    float gmc_local_last_dx;
    float gmc_local_last_dy;
    float gmc_local_last_confidence;
    CVI_U32 gmc_last_frame_gap;
    CVI_U32 gmc_local_last_frame_gap;
    CVI_U32 gmc_last_applied_frames;
    float gmc_last_expansion_pixels;
    float sot_target_width;
    float sot_target_height;
    float sot_target_size_factor;
    CVI_BOOL sot_small_target;
    CVI_BOOL sot_refined_target;
    CVI_U32 gmc_submit_interval;
    CVI_U32 template_update_count;
    CVI_U32 template_rollback_count;
    CVI_U32 template_probation_frames;
    CVI_U64 template_age_frames;
    CVI_U32 template_refresh_votes;
    CVI_U32 template_refresh_window;
    CVI_U32 template_using_long_term;
    CVI_U32 template_switch_count;
    CVI_U32 template_recovery_stable_frames;
    CVI_U32 kalman_search_horizon;
    CVI_FLOAT kalman_search_dx;
    CVI_FLOAT kalman_search_dy;
    CVI_U32 relock_votes;
    CVI_U32 relock_good_votes;
    CVI_U32 relock_motion_rejects;
    CVI_FLOAT relock_candidate_distance;
    CVI_FLOAT relock_motion_gate;
    CVI_FLOAT sot_response_score;
    CVI_FLOAT sot_response_psr;
    CVI_FLOAT sot_response_peak_margin;
    CVI_FLOAT sot_response_area_ratio;
    CVI_FLOAT sot_response_aspect_ratio;
    CVI_U32 sot_response_reject_mask;
    CVI_U32 sot_bootstrap_active;
    CVI_U32 sot_bootstrap_retry_count;
    CVI_U32 sot_weak_follow_count;
    CVI_U32 sot_hard_lost_deferred_count;
    CVI_U32 sot_low_detail_target;
    CVI_U32 sot_response_selected_candidate;
    CVI_FLOAT sot_response_psr_baseline;
    CVI_FLOAT sot_response_peak_margin_baseline;
    CVI_FLOAT sot_search_target_pixels;
    CVI_U32 template_recovery_attempted;
    CVI_U32 handoff_identity_active;
    CVI_U32 handoff_identity_confirmed;
    CVI_U32 handoff_identity_frames;
    CVI_U32 handoff_identity_good_frames;
    CVI_U32 candidate_output_held;
    CVI_U32 candidate_output_following;
    CVI_FLOAT candidate_follow_dx;
    CVI_FLOAT candidate_follow_dy;
    CVI_FLOAT handoff_candidate_distance;
    CVI_FLOAT handoff_candidate_area_ratio;
    CVI_FLOAT handoff_candidate_aspect_ratio;
    CVI_U32 scale_update_count;
    CVI_U32 scale_reject_count;
    CVI_U32 scale_pending_frames;
    CVI_U32 scale_stable_frames;
    CVI_FLOAT scale_raw_ratio;
    CVI_FLOAT scale_raw_width_ratio;
    CVI_FLOAT scale_raw_height_ratio;
    CVI_U32 preserve_aspect_context;
    CVI_U32 search_context_width;
    CVI_U32 search_context_height;
    CVI_FLOAT scale_raw_bias_width;
    CVI_FLOAT scale_raw_bias_height;
    CVI_U32 scale_raw_bias_samples;
    CVI_FLOAT scale_trusted_width;
    CVI_FLOAT scale_trusted_height;
    CVI_U32 scale_approach_active;
    CVI_U32 scale_approach_frames;
    double sot_score_total;
    float sot_score_min;
    float sot_score_max;
} APP_OBJECT_TRACK_PERF_S;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static uint64_t app_ipcam_Ai_Object_Track_TimeUs(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ULL * 1000ULL +
           (uint64_t)ts.tv_nsec / 1000ULL;
}

static CVI_FLOAT app_ipcam_Ai_Object_Track_TargetSizeFactor(
    CVI_FLOAT fSotWidth, CVI_FLOAT fSotHeight)
{
    CVI_FLOAT fSide;

    if (fSotWidth <= 1.0f || fSotHeight <= 1.0f) {
        return 0.0f;
    }
    fSide = fminf(fSotWidth, fSotHeight);
    if (fSide >= OBJECT_TRACK_ADAPTIVE_TARGET_SOT_SIDE) {
        return 0.0f;
    }
    if (fSide <= OBJECT_TRACK_TINY_TARGET_SOT_SIDE) {
        return 1.0f;
    }
    return (OBJECT_TRACK_ADAPTIVE_TARGET_SOT_SIDE - fSide) /
        (OBJECT_TRACK_ADAPTIVE_TARGET_SOT_SIDE -
         OBJECT_TRACK_TINY_TARGET_SOT_SIDE);
}

static CVI_BOOL app_ipcam_Ai_Object_Track_IsSmallTarget(
    CVI_FLOAT fSotWidth, CVI_FLOAT fSotHeight)
{
    return app_ipcam_Ai_Object_Track_TargetSizeFactor(
        fSotWidth, fSotHeight) > 0.0f;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Display_Reset(
    APP_OBJECT_TRACK_DISPLAY_FILTER_S *pstFilter)
{
    if (pstFilter == NULL) {
        return;
    }
    memset(pstFilter, 0, sizeof(*pstFilter));
}

static CVI_VOID app_ipcam_Ai_Object_Track_Display_Smooth(
    APP_OBJECT_TRACK_DISPLAY_FILTER_S *pstFilter,
    CVI_FLOAT *pfX1, CVI_FLOAT *pfY1, CVI_FLOAT *pfX2, CVI_FLOAT *pfY2)
{
    CVI_FLOAT fWidth;
    CVI_FLOAT fHeight;
    CVI_FLOAT fCenterX;
    CVI_FLOAT fCenterY;
    CVI_FLOAT fSizeRatio;
    CVI_FLOAT fCenterAlpha = OBJECT_TRACK_DISPLAY_CENTER_ALPHA;
    CVI_FLOAT fSizeAlpha = OBJECT_TRACK_DISPLAY_SIZE_ALPHA;

    if (pstFilter == NULL || pfX1 == NULL || pfY1 == NULL ||
        pfX2 == NULL || pfY2 == NULL) {
        return;
    }
    fWidth = *pfX2 - *pfX1;
    fHeight = *pfY2 - *pfY1;
    if (fWidth <= 1.0f || fHeight <= 1.0f) {
        return;
    }
    fCenterX = *pfX1 + fWidth * 0.5f;
    fCenterY = *pfY1 + fHeight * 0.5f;
    if (!pstFilter->initialized) {
        pstFilter->initialized = CVI_TRUE;
        pstFilter->cx = fCenterX;
        pstFilter->cy = fCenterY;
        pstFilter->width = fWidth;
        pstFilter->height = fHeight;
    } else {
        CVI_FLOAT fJump = hypotf(fCenterX - pstFilter->cx,
                                 fCenterY - pstFilter->cy);
        CVI_FLOAT fSide = fmaxf(pstFilter->width, pstFilter->height);
        if (fJump > fSide * OBJECT_TRACK_DISPLAY_SNAP_CENTER_RATIO) {
            fCenterAlpha = 0.90f;
        }
        pstFilter->cx += (fCenterX - pstFilter->cx) * fCenterAlpha;
        pstFilter->cy += (fCenterY - pstFilter->cy) * fCenterAlpha;
        fSizeRatio = sqrtf((fWidth * fHeight) /
            fmaxf(pstFilter->width * pstFilter->height, 1.0f));
        if (fabsf(fSizeRatio - 1.0f) < OBJECT_TRACK_DISPLAY_SIZE_DEADBAND) {
            fSizeAlpha = 0.0f;
        }
        pstFilter->width += (fWidth - pstFilter->width) * fSizeAlpha;
        pstFilter->height += (fHeight - pstFilter->height) * fSizeAlpha;
    }
    *pfX1 = pstFilter->cx - pstFilter->width * 0.5f;
    *pfY1 = pstFilter->cy - pstFilter->height * 0.5f;
    *pfX2 = pstFilter->cx + pstFilter->width * 0.5f;
    *pfY2 = pstFilter->cy + pstFilter->height * 0.5f;
}

static CVI_U32 app_ipcam_Ai_Object_Track_GmcInterval(
    CVI_FLOAT fTargetSizeFactor, CVI_BOOL bUnreliable)
{
    CVI_U32 u32BaseInterval = g_pstObjTrackCfg->sot_gmc_interval;

    if (u32BaseInterval < OBJECT_TRACK_GMC_MIN_INTERVAL) {
        u32BaseInterval = OBJECT_TRACK_GMC_MIN_INTERVAL;
    }
    if (bUnreliable && fTargetSizeFactor <= 0.0f) {
        return OBJECT_TRACK_GMC_MIN_INTERVAL;
    }
    return u32BaseInterval;
}

static CVI_U32 app_ipcam_Ai_Object_Track_OsdHoldFrames(
    CVI_FLOAT fTargetSizeFactor)
{
    return fTargetSizeFactor > 0.0f ?
        OBJECT_TRACK_SMALL_TARGET_OSD_HOLD_FRAMES :
        OBJECT_TRACK_OSD_HOLD_FRAMES;
}

#ifdef OSDC_SUPPORT
static CVI_VOID app_ipcam_Ai_Object_Track_OsdPredict_FilterReset(
    APP_OBJECT_TRACK_OSD_PREDICTOR_S *pstPredictor)
{
    pstPredictor->initialized = CVI_FALSE;
    pstPredictor->velocity_valid = CVI_FALSE;
    pstPredictor->velocity_x_per_us = 0.0f;
    pstPredictor->velocity_y_per_us = 0.0f;
    pstPredictor->frame_period_us = 0.0f;
    pstPredictor->prev_time_ref = 0;
    pstPredictor->prev_pts = 0;
}

static CVI_VOID app_ipcam_Ai_Object_Track_OsdPredict_SetReference(
    APP_OBJECT_TRACK_OSD_PREDICTOR_S *pstPredictor,
    CVI_FLOAT fCenterX, CVI_FLOAT fCenterY,
    CVI_U32 u32TimeRef, CVI_U64 u64Pts)
{
    pstPredictor->initialized = CVI_TRUE;
    pstPredictor->prev_cx = fCenterX;
    pstPredictor->prev_cy = fCenterY;
    pstPredictor->prev_time_ref = u32TimeRef;
    pstPredictor->prev_pts = u64Pts;
}

static CVI_VOID app_ipcam_Ai_Object_Track_OsdPredict(
    APP_OBJECT_TRACK_OSD_PREDICTOR_S *pstPredictor,
    CVI_FLOAT fRawX1, CVI_FLOAT fRawY1,
    CVI_FLOAT fRawX2, CVI_FLOAT fRawY2,
    CVI_FLOAT fScore, CVI_U32 u32State,
    CVI_U32 u32TimeRef, CVI_U64 u64Pts,
    CVI_U32 u32SourceWidth, CVI_U32 u32SourceHeight,
    CVI_FLOAT *pfDisplayX1, CVI_FLOAT *pfDisplayY1,
    CVI_FLOAT *pfDisplayX2, CVI_FLOAT *pfDisplayY2)
{
    CVI_FLOAT fWidth = fRawX2 - fRawX1;
    CVI_FLOAT fHeight = fRawY2 - fRawY1;
    CVI_FLOAT fCenterX = fRawX1 + fWidth * 0.5f;
    CVI_FLOAT fCenterY = fRawY1 + fHeight * 0.5f;
    CVI_FLOAT fDeltaX = 0.0f;
    CVI_FLOAT fDeltaY = 0.0f;
    CVI_FLOAT fShiftX = 0.0f;
    CVI_FLOAT fShiftY = 0.0f;

    *pfDisplayX1 = fRawX1;
    *pfDisplayY1 = fRawY1;
    *pfDisplayX2 = fRawX2;
    *pfDisplayY2 = fRawY2;
    if (u32State != TDL_TRACK_STATE_TRACKED ||
        u32SourceWidth == 0 || u32SourceHeight == 0 ||
        u64Pts == 0 || fWidth <= 1.0f || fHeight <= 1.0f ||
        !isfinite(fCenterX) || !isfinite(fCenterY)) {
        app_ipcam_Ai_Object_Track_OsdPredict_FilterReset(pstPredictor);
        goto predict_done;
    }
    if (!pstPredictor->initialized) {
        app_ipcam_Ai_Object_Track_OsdPredict_SetReference(
            pstPredictor, fCenterX, fCenterY, u32TimeRef, u64Pts);
        goto predict_done;
    }

    {
        CVI_U32 u32FrameGap = u32TimeRef - pstPredictor->prev_time_ref;
        CVI_U64 u64PtsGap = u64Pts > pstPredictor->prev_pts ?
            u64Pts - pstPredictor->prev_pts : 0;
        CVI_FLOAT fFramePeriodUs = u32FrameGap > 0 ?
            (CVI_FLOAT)u64PtsGap / u32FrameGap : 0.0f;
        CVI_FLOAT fInstantVelocityX;
        CVI_FLOAT fInstantVelocityY;
        CVI_FLOAT fDirectionScale = 1.0f;
        CVI_FLOAT fScoreScale;
        CVI_FLOAT fPredictionUs;
        CVI_FLOAT fMaxShiftX;
        CVI_FLOAT fMaxShiftY;
        fDeltaX = fCenterX - pstPredictor->prev_cx;
        fDeltaY = fCenterY - pstPredictor->prev_cy;
        if (u32FrameGap == 0 ||
            u32FrameGap > OBJECT_TRACK_OSD_MAX_TIME_REF_GAP ||
            u64PtsGap == 0 ||
            fFramePeriodUs < OBJECT_TRACK_OSD_MIN_FRAME_PERIOD_US ||
            fFramePeriodUs > OBJECT_TRACK_OSD_MAX_FRAME_PERIOD_US ||
            fabsf(fDeltaX) > u32SourceWidth * 0.40f ||
            fabsf(fDeltaY) > u32SourceHeight * 0.40f) {
            app_ipcam_Ai_Object_Track_OsdPredict_FilterReset(pstPredictor);
            app_ipcam_Ai_Object_Track_OsdPredict_SetReference(
                pstPredictor, fCenterX, fCenterY, u32TimeRef, u64Pts);
            goto predict_done;
        }

        fInstantVelocityX = fDeltaX / (CVI_FLOAT)u64PtsGap;
        fInstantVelocityY = fDeltaY / (CVI_FLOAT)u64PtsGap;
        if (pstPredictor->velocity_valid) {
            CVI_FLOAT fOldSpeed = hypotf(
                pstPredictor->velocity_x_per_us,
                pstPredictor->velocity_y_per_us) * fFramePeriodUs;
            CVI_FLOAT fInstantSpeed = hypotf(
                fInstantVelocityX, fInstantVelocityY) * fFramePeriodUs;
            CVI_FLOAT fDirectionDot =
                pstPredictor->velocity_x_per_us * fInstantVelocityX +
                pstPredictor->velocity_y_per_us * fInstantVelocityY;

            if (fDirectionDot < 0.0f &&
                fOldSpeed > OBJECT_TRACK_OSD_DEAD_ZONE_PX &&
                fInstantSpeed > OBJECT_TRACK_OSD_DEAD_ZONE_PX) {
                fDirectionScale =
                    OBJECT_TRACK_OSD_DIRECTION_CHANGE_SCALE;
            }
            pstPredictor->velocity_x_per_us =
                OBJECT_TRACK_OSD_VELOCITY_ALPHA * fInstantVelocityX +
                (1.0f - OBJECT_TRACK_OSD_VELOCITY_ALPHA) *
                    pstPredictor->velocity_x_per_us;
            pstPredictor->velocity_y_per_us =
                OBJECT_TRACK_OSD_VELOCITY_ALPHA * fInstantVelocityY +
                (1.0f - OBJECT_TRACK_OSD_VELOCITY_ALPHA) *
                    pstPredictor->velocity_y_per_us;
        } else {
            pstPredictor->velocity_x_per_us = fInstantVelocityX;
            pstPredictor->velocity_y_per_us = fInstantVelocityY;
            pstPredictor->velocity_valid = CVI_TRUE;
        }
        if (pstPredictor->frame_period_us > 0.0f) {
            pstPredictor->frame_period_us =
                pstPredictor->frame_period_us * 0.75f +
                fFramePeriodUs * 0.25f;
        } else {
            pstPredictor->frame_period_us = fFramePeriodUs;
        }

        fScoreScale = fScore >= 0.70f ? 1.0f :
            fmaxf(0.35f, fScore / 0.70f);
        fPredictionUs = pstPredictor->frame_period_us *
            OBJECT_TRACK_OSD_PREDICT_LEAD_FRAMES *
            fScoreScale * fDirectionScale;
        fShiftX = pstPredictor->velocity_x_per_us * fPredictionUs;
        fShiftY = pstPredictor->velocity_y_per_us * fPredictionUs;
        if (fabsf(fShiftX) < OBJECT_TRACK_OSD_DEAD_ZONE_PX) {
            fShiftX = 0.0f;
        }
        if (fabsf(fShiftY) < OBJECT_TRACK_OSD_DEAD_ZONE_PX) {
            fShiftY = 0.0f;
        }

        fMaxShiftX = fminf(u32SourceWidth * 0.20f,
            fmaxf(fWidth * 1.5f, fabsf(fDeltaX) * 2.0f));
        fMaxShiftY = fminf(u32SourceHeight * 0.20f,
            fmaxf(fHeight * 1.5f, fabsf(fDeltaY) * 2.0f));
        fShiftX = fmaxf(-fMaxShiftX, fminf(fShiftX, fMaxShiftX));
        fShiftY = fmaxf(-fMaxShiftY, fminf(fShiftY, fMaxShiftY));
        fCenterX = fmaxf(fWidth * 0.5f,
            fminf(fCenterX + fShiftX,
                u32SourceWidth - fWidth * 0.5f));
        fCenterY = fmaxf(fHeight * 0.5f,
            fminf(fCenterY + fShiftY,
                u32SourceHeight - fHeight * 0.5f));

        if (fShiftX != 0.0f || fShiftY != 0.0f) {
            *pfDisplayX1 = fCenterX - fWidth * 0.5f;
            *pfDisplayY1 = fCenterY - fHeight * 0.5f;
            *pfDisplayX2 = fCenterX + fWidth * 0.5f;
            *pfDisplayY2 = fCenterY + fHeight * 0.5f;
        }
        app_ipcam_Ai_Object_Track_OsdPredict_SetReference(
            pstPredictor,
            fRawX1 + fWidth * 0.5f,
            fRawY1 + fHeight * 0.5f,
            u32TimeRef, u64Pts);
    }

predict_done:
    return;
}
#endif

static CVI_BOOL app_ipcam_Ai_Object_Track_Draw_Update(
    const TDLTrackerInfo *pstTrackInfo,
    CVI_FLOAT *pfRawX1, CVI_FLOAT *pfRawY1,
    CVI_FLOAT *pfRawX2, CVI_FLOAT *pfRawY2)
{
    CVI_BOOL bDrawReady = CVI_FALSE;

    if (pstTrackInfo == NULL || pfRawX1 == NULL || pfRawY1 == NULL ||
        pfRawX2 == NULL || pfRawY2 == NULL ||
        g_pstObjTrackCfg->u32SotGrpWidth == 0 ||
        g_pstObjTrackCfg->u32SotGrpHeight == 0 ||
        pstTrackInfo->bbox.x2 <= pstTrackInfo->bbox.x1 ||
        pstTrackInfo->bbox.y2 <= pstTrackInfo->bbox.y1) {
        return CVI_FALSE;
    }

    *pfRawX1 = pstTrackInfo->bbox.x1 *
        (CVI_FLOAT)g_pstObjTrackCfg->u32GrpWidth /
        g_pstObjTrackCfg->u32SotGrpWidth;
    *pfRawX2 = pstTrackInfo->bbox.x2 *
        (CVI_FLOAT)g_pstObjTrackCfg->u32GrpWidth /
        g_pstObjTrackCfg->u32SotGrpWidth;
    *pfRawY1 = pstTrackInfo->bbox.y1 *
        (CVI_FLOAT)g_pstObjTrackCfg->u32GrpHeight /
        g_pstObjTrackCfg->u32SotGrpHeight;
    *pfRawY2 = pstTrackInfo->bbox.y2 *
        (CVI_FLOAT)g_pstObjTrackCfg->u32GrpHeight /
        g_pstObjTrackCfg->u32SotGrpHeight;
    app_ipcam_Ai_Object_Track_Display_Smooth(
        &g_DisplayFilter, pfRawX1, pfRawY1, pfRawX2, pfRawY2);

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        if (g_stObjDraw.info != NULL) {
            g_stObjDraw.size = 1;
            g_stObjDraw.info[0].box.x1 = *pfRawX1;
            g_stObjDraw.info[0].box.x2 = *pfRawX2;
            g_stObjDraw.info[0].box.y1 = *pfRawY1;
            g_stObjDraw.info[0].box.y2 = *pfRawY2;
            bDrawReady = CVI_TRUE;
        } else {
            g_stObjDraw.size = 0;
        }
    }

    return bDrawReady;
}

#ifdef OSDC_SUPPORT
static CVI_VOID app_ipcam_Ai_Object_Track_Osd_Publish(
    APP_OBJECT_TRACK_OSD_PREDICTOR_S *pstPredictor,
    const TDLTrackerInfo *pstTrackInfo,
    const VIDEO_FRAME_INFO_S *pstFrame,
    CVI_FLOAT fRawX1, CVI_FLOAT fRawY1,
    CVI_FLOAT fRawX2, CVI_FLOAT fRawY2)
{
    CVI_FLOAT fDisplayX1;
    CVI_FLOAT fDisplayY1;
    CVI_FLOAT fDisplayX2;
    CVI_FLOAT fDisplayY2;
    CVI_FLOAT fSotWidth = pstTrackInfo->bbox.x2 - pstTrackInfo->bbox.x1;
    CVI_FLOAT fSotHeight = pstTrackInfo->bbox.y2 - pstTrackInfo->bbox.y1;

    if (app_ipcam_Ai_Object_Track_IsSmallTarget(
            fSotWidth, fSotHeight)) {
        app_ipcam_Ai_Object_Track_OsdPredict_FilterReset(pstPredictor);
        fDisplayX1 = fRawX1;
        fDisplayY1 = fRawY1;
        fDisplayX2 = fRawX2;
        fDisplayY2 = fRawY2;
    } else {
        app_ipcam_Ai_Object_Track_OsdPredict(
            pstPredictor,
            fRawX1, fRawY1, fRawX2, fRawY2,
            pstTrackInfo->score, pstTrackInfo->state,
            pstFrame->stVFrame.u32TimeRef,
            pstFrame->stVFrame.u64PTS,
            g_pstObjTrackCfg->u32GrpWidth,
            g_pstObjTrackCfg->u32GrpHeight,
            &fDisplayX1, &fDisplayY1,
            &fDisplayX2, &fDisplayY2);
    }
    app_ipcam_Osdc_ObjectTrackRect_Publish(
        CVI_TRUE,
        pstTrackInfo->state != TDL_TRACK_STATE_TRACKED &&
            pstTrackInfo->state != TDL_TRACK_STATE_CANDIDATE &&
            pstTrackInfo->state != TDL_TRACK_STATE_FOLLOWING,
        fDisplayX1, fDisplayY1, fDisplayX2, fDisplayY2,
        g_pstObjTrackCfg->u32GrpWidth,
        g_pstObjTrackCfg->u32GrpHeight);
}
#endif

static CVI_VOID *app_ipcam_Ai_Object_Track_GmcAsync_Proc(CVI_VOID *pArgs)
{
    CVI_U64 state_epoch = (CVI_U64)-1;

    (void)pArgs;
    prctl(PR_SET_NAME, "OBJ_TRACK_GMC", 0, 0, 0);
    while (CVI_TRUE) {
        APP_OBJECT_TRACK_GMC_SAMPLE_S stSample = {0};
        CVI_U64 work_epoch = 0;
        APP_OBJECT_TRACK_GMC_RESULT_S stResult = {0};
        CVI_S32 s32Ret = CVI_SUCCESS;

        pthread_mutex_lock(&g_GmcAsyncMutex);
        while (g_GmcAsync.running && !g_GmcAsync.pending) {
            pthread_cond_wait(&g_GmcAsyncCond, &g_GmcAsyncMutex);
        }
        if (!g_GmcAsync.running) {
            g_GmcAsync.pending = CVI_FALSE;
            pthread_mutex_unlock(&g_GmcAsyncMutex);
            break;
        }

        stSample = g_GmcAsync.pending_sample;
        work_epoch = g_GmcAsync.pending_epoch;
        g_GmcAsync.pending = CVI_FALSE;
        g_GmcAsync.busy = CVI_TRUE;
        pthread_mutex_unlock(&g_GmcAsyncMutex);

        if (state_epoch != work_epoch) {
            app_ipcam_ObjectTrackGmc_Reset(&g_GmcState);
            state_epoch = work_epoch;
        }
        s32Ret = app_ipcam_ObjectTrackGmc_ProcessSample(
            &g_GmcState, &stSample, &stResult);

        pthread_mutex_lock(&g_GmcAsyncMutex);
        g_GmcAsync.busy = CVI_FALSE;
        if (work_epoch == g_GmcAsync.epoch) {
            g_GmcAsync.result = stResult;
            g_GmcAsync.result_status = s32Ret;
            g_GmcAsync.result_epoch = work_epoch;
            g_GmcAsync.result_frame_id = stSample.frame_id;
            g_GmcAsync.result_ready = CVI_TRUE;
        }
        pthread_cond_broadcast(&g_GmcAsyncCond);
        pthread_mutex_unlock(&g_GmcAsyncMutex);
    }
    return NULL;
}

static CVI_S32 app_ipcam_Ai_Object_Track_GmcAsync_Start(CVI_VOID)
{
    CVI_S32 s32Ret;

    pthread_mutex_lock(&g_GmcAsyncMutex);
    memset(&g_GmcAsync, 0, sizeof(g_GmcAsync));
    g_GmcAsync.running = CVI_TRUE;
    g_GmcAsync.epoch = 1;
    app_ipcam_ObjectTrackGmc_Reset(&g_GmcState);
    pthread_mutex_unlock(&g_GmcAsyncMutex);

    s32Ret = pthread_create(&g_GmcAsync.thread, NULL,
                            app_ipcam_Ai_Object_Track_GmcAsync_Proc, NULL);
    if (s32Ret != 0) {
        pthread_mutex_lock(&g_GmcAsyncMutex);
        g_GmcAsync.running = CVI_FALSE;
        pthread_mutex_unlock(&g_GmcAsyncMutex);
        return CVI_FAILURE;
    }

    pthread_mutex_lock(&g_GmcAsyncMutex);
    g_GmcAsync.started = CVI_TRUE;
    pthread_mutex_unlock(&g_GmcAsyncMutex);
    return CVI_SUCCESS;
}

static CVI_VOID app_ipcam_Ai_Object_Track_GmcAsync_Stop(CVI_VOID)
{
    pthread_t thread;

    pthread_mutex_lock(&g_GmcAsyncMutex);
    if (!g_GmcAsync.started) {
        pthread_mutex_unlock(&g_GmcAsyncMutex);
        return;
    }
    g_GmcAsync.running = CVI_FALSE;
    thread = g_GmcAsync.thread;
    pthread_cond_broadcast(&g_GmcAsyncCond);
    pthread_mutex_unlock(&g_GmcAsyncMutex);

    pthread_join(thread, NULL);
    pthread_mutex_lock(&g_GmcAsyncMutex);
    memset(&g_GmcAsync, 0, sizeof(g_GmcAsync));
    app_ipcam_ObjectTrackGmc_Reset(&g_GmcState);
    pthread_mutex_unlock(&g_GmcAsyncMutex);
}

static CVI_VOID app_ipcam_Ai_Object_Track_GmcAsync_Reset(CVI_VOID)
{
    pthread_mutex_lock(&g_GmcAsyncMutex);
    if (!g_GmcAsync.started) {
        app_ipcam_ObjectTrackGmc_Reset(&g_GmcState);
        pthread_mutex_unlock(&g_GmcAsyncMutex);
        return;
    }
    g_GmcAsync.epoch++;
    g_GmcAsync.input_count = 0;
    g_GmcAsync.result_ready = CVI_FALSE;
    g_GmcAsync.pending = CVI_FALSE;
    while (g_GmcAsync.busy) {
        pthread_cond_wait(&g_GmcAsyncCond, &g_GmcAsyncMutex);
    }
    app_ipcam_ObjectTrackGmc_Reset(&g_GmcState);
    pthread_mutex_unlock(&g_GmcAsyncMutex);
}

static CVI_BOOL app_ipcam_Ai_Object_Track_GmcAsync_Submit(
    const VIDEO_FRAME_INFO_S *pstFrame, CVI_U64 frame_id, CVI_U32 interval,
    const CVI_S32 exclusion[4], CVI_BOOL bEvaluate)
{
    APP_OBJECT_TRACK_GMC_SAMPLE_S stSample = {0};
    CVI_U32 effective_interval = interval < OBJECT_TRACK_GMC_MIN_INTERVAL ?
        OBJECT_TRACK_GMC_MIN_INTERVAL : interval;
    CVI_U64 work_epoch;

    pthread_mutex_lock(&g_GmcAsyncMutex);
    if (!g_GmcAsync.started || !g_GmcAsync.running) {
        pthread_mutex_unlock(&g_GmcAsyncMutex);
        return CVI_FALSE;
    }
    g_GmcAsync.input_count++;
    if (g_GmcAsync.input_count % effective_interval != 0) {
        pthread_mutex_unlock(&g_GmcAsyncMutex);
        return CVI_FALSE;
    }
    work_epoch = g_GmcAsync.epoch;
    pthread_mutex_unlock(&g_GmcAsyncMutex);

    /* 归还 VPSS 帧前仅复制 GMC 小网格，后台线程不再持有 VB 块。 */
    if (app_ipcam_ObjectTrackGmc_Sample(
            pstFrame, frame_id, exclusion, &stSample) != CVI_SUCCESS) {
        return CVI_FALSE;
    }
    stSample.evaluate = bEvaluate;

    pthread_mutex_lock(&g_GmcAsyncMutex);
    if (!g_GmcAsync.started || !g_GmcAsync.running ||
        g_GmcAsync.epoch != work_epoch) {
        pthread_mutex_unlock(&g_GmcAsyncMutex);
        return CVI_FALSE;
    }
    g_GmcAsync.pending_sample = stSample;
    g_GmcAsync.pending_epoch = work_epoch;
    g_GmcAsync.pending = CVI_TRUE;
    pthread_cond_signal(&g_GmcAsyncCond);
    pthread_mutex_unlock(&g_GmcAsyncMutex);

    return CVI_TRUE;
}

static CVI_BOOL app_ipcam_Ai_Object_Track_GmcAsync_Poll(
    APP_OBJECT_TRACK_GMC_RESULT_S *pstResult, CVI_S32 *ps32Status,
    CVI_U64 *pu64FrameId)
{
    CVI_BOOL available = CVI_FALSE;

    pthread_mutex_lock(&g_GmcAsyncMutex);
    if (g_GmcAsync.result_ready &&
        g_GmcAsync.result_epoch == g_GmcAsync.epoch) {
        *pstResult = g_GmcAsync.result;
        *ps32Status = g_GmcAsync.result_status;
        *pu64FrameId = g_GmcAsync.result_frame_id;
        g_GmcAsync.result_ready = CVI_FALSE;
        available = CVI_TRUE;
    }
    pthread_mutex_unlock(&g_GmcAsyncMutex);
    return available;
}

static CVI_VOID app_ipcam_Ai_Object_Track_ObjDraw_Clear(CVI_VOID)
{
    CVI_BOOL was_visible;

    app_ipcam_Ai_Object_Track_Display_Reset(&g_DisplayFilter);

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        was_visible = g_stObjDraw.size > 0;
        g_stObjDraw.size = 0;
    }
    if (!was_visible) {
        return;
    }
#ifdef OSDC_SUPPORT
    app_ipcam_Osdc_ObjectTrackRect_Publish(
        CVI_FALSE, CVI_FALSE, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0);
#endif
}

static CVI_VOID app_ipcam_Ai_Object_Track_Gmc_Reset(CVI_VOID)
{
    app_ipcam_Ai_Object_Track_GmcAsync_Reset();
    memset(g_GmcExclusion, 0, sizeof(g_GmcExclusion));
}

static CVI_VOID app_ipcam_Ai_Object_Track_Perf_Reset(
    APP_OBJECT_TRACK_PERF_S *pstPerf, APP_PARAM_OBJECT_TRACK_MODE mode,
    uint64_t now_us)
{
    memset(pstPerf, 0, sizeof(*pstPerf));
    pstPerf->mode = mode;
    pstPerf->window_start_us = now_us;
}

static const char *app_ipcam_Ai_Object_Track_Mode_Name(
    APP_PARAM_OBJECT_TRACK_MODE mode)
{
    if (mode == TRACKING) {
        return "tracking";
    }
    if (mode == DETECTION) {
        return "detection";
    }
    return "waiting_target";
}




static APP_PARAM_OBJECT_TRACK_MODE app_ipcam_Ai_Object_Track_Idle_Mode_Get(
    CVI_VOID)
{
    APP_PARAM_AI_PD_CFG_S *pstPdCfg = app_ipcam_Ai_PD_Param_Get();

    return pstPdCfg != NULL && pstPdCfg->bEnable ? DETECTION : WAIT_TARGET;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Perf_Write(
    APP_OBJECT_TRACK_PERF_S *pstPerf, uint64_t now_us)
{
    uint64_t elapsed_us = now_us - pstPerf->window_start_us;
    FILE *pFile = NULL;
    double elapsed_seconds = 0.0;

    if (elapsed_us < TRACK_PERF_WINDOW_US) {
        return;
    }

    elapsed_seconds = (double)elapsed_us / 1000000.0;
    pFile = fopen(TRACK_FPS_STATUS_TMP_PATH, "w");
    if (pFile != NULL) {
        fprintf(pFile,
            "mode=%s\n"
            "input_fps=%.2f\n"
            "det_fps=%.2f\n"
            "sot_fps=%.2f\n"
            "sot_observed_frames=%llu\n"
            "sot_predicted_frames=%llu\n"
            "sot_lost_frames=%llu\n"
            "sot_score_avg=%.3f\n"
            "sot_score_min=%.3f\n"
            "sot_score_max=%.3f\n"
            "sot_min_observed_score=%.3f\n"
            "frame_wait_avg_ms=%.3f\n"
            "frame_drain_avg_ms=%.3f\n"
            "stale_frames=%llu\n"
            "wrap_avg_ms=%.3f\n"
            "gmc_evaluations=%llu\n"
            "gmc_failures=%llu\n"
            "gmc_valid=%llu\n"
            "gmc_applied=%llu\n"
            "gmc_cache_avg_ms=%.3f\n"
            "gmc_grid_avg_ms=%.3f\n"
            "gmc_search_avg_ms=%.3f\n"
            "gmc_total_avg_ms=%.3f\n"
            "gmc_last_dx=%.2f\n"
            "gmc_last_dy=%.2f\n"
            "gmc_last_applied_dx=%.2f\n"
            "gmc_last_applied_dy=%.2f\n"
            "gmc_last_confidence=%.3f\n"
            "gmc_last_frame_gap=%u\n"
            "gmc_last_applied_frames=%u\n"
            "det_avg_ms=%.3f\n"
            "sot_avg_ms=%.3f\n"
            "sot_init_avg_ms=%.3f\n"
            "result_avg_ms=%.3f\n"
            "cleanup_avg_ms=%.3f\n"
            "template_input_cache=1\n"
            "direct_search_tensor=1\n",
            app_ipcam_Ai_Object_Track_Mode_Name(pstPerf->mode),
            pstPerf->input_frames / elapsed_seconds,
            pstPerf->det_frames / elapsed_seconds,
            pstPerf->sot_frames / elapsed_seconds,
            (unsigned long long)pstPerf->sot_observed_frames,
            (unsigned long long)pstPerf->sot_predicted_frames,
            (unsigned long long)pstPerf->sot_lost_frames,
            pstPerf->sot_observed_frames > 0 ?
                pstPerf->sot_score_total / pstPerf->sot_observed_frames : 0.0,
            pstPerf->sot_observed_frames > 0 ? pstPerf->sot_score_min : 0.0f,
            pstPerf->sot_observed_frames > 0 ? pstPerf->sot_score_max : 0.0f,
            g_pstObjTrackCfg->sot_min_observed_score,
            pstPerf->input_frames > 0 ?
                (double)pstPerf->frame_wait_total_us /
                    pstPerf->input_frames / 1000.0 : 0.0,
            pstPerf->input_frames > 0 ?
                (double)pstPerf->frame_drain_total_us /
                    pstPerf->input_frames / 1000.0 : 0.0,
            (unsigned long long)pstPerf->stale_frames,
            pstPerf->input_frames > 0 ?
                (double)pstPerf->wrap_total_us /
                    pstPerf->input_frames / 1000.0 : 0.0,
            (unsigned long long)pstPerf->gmc_evaluations,
            (unsigned long long)pstPerf->gmc_failures,
            (unsigned long long)pstPerf->gmc_valid,
            (unsigned long long)pstPerf->gmc_applied,
            pstPerf->gmc_evaluations > 0 ?
                (double)pstPerf->gmc_cache_total_us /
                    pstPerf->gmc_evaluations / 1000.0 : 0.0,
            pstPerf->gmc_evaluations > 0 ?
                (double)pstPerf->gmc_grid_total_us /
                    pstPerf->gmc_evaluations / 1000.0 : 0.0,
            pstPerf->gmc_evaluations > 0 ?
                (double)pstPerf->gmc_search_total_us /
                    pstPerf->gmc_evaluations / 1000.0 : 0.0,
            pstPerf->gmc_evaluations > 0 ?
                (double)pstPerf->gmc_total_us /
                    pstPerf->gmc_evaluations / 1000.0 : 0.0,
            pstPerf->gmc_last_dx,
            pstPerf->gmc_last_dy,
            pstPerf->gmc_last_applied_dx,
            pstPerf->gmc_last_applied_dy,
            pstPerf->gmc_last_confidence,
            pstPerf->gmc_last_frame_gap,
            pstPerf->gmc_last_applied_frames,
            pstPerf->det_frames > 0 ?
                (double)pstPerf->det_total_us /
                    pstPerf->det_frames / 1000.0 : 0.0,
            pstPerf->sot_frames > 0 ?
                (double)pstPerf->sot_total_us /
                    pstPerf->sot_frames / 1000.0 : 0.0,
            pstPerf->sot_init_frames > 0 ?
                (double)pstPerf->sot_init_total_us /
                    pstPerf->sot_init_frames / 1000.0 : 0.0,
            pstPerf->sot_frames > 0 ?
                (double)pstPerf->result_total_us /
                    pstPerf->sot_frames / 1000.0 : 0.0,
            pstPerf->input_frames > 0 ?
                (double)pstPerf->cleanup_total_us /
                    pstPerf->input_frames / 1000.0 : 0.0);
        if (fclose(pFile) == 0) {
            rename(TRACK_FPS_STATUS_TMP_PATH, TRACK_FPS_STATUS_PATH);
        }
    }

    app_ipcam_Ai_Object_Track_Perf_Reset(pstPerf, pstPerf->mode, now_us);
}

static CVI_S32 app_ipcam_Ai_Object_Track_Shared_Pipeline_Prepare(CVI_VOID)
{
    APP_PARAM_SYS_CFG_S *pstSysCfg = NULL;
    APP_PARAM_VPSS_CFG_T *pstVpssCfg = NULL;
    APP_VPSS_GRP_CFG_T *pstGrpCfg = NULL;
    CVI_S32 s32SotPool = -1;

    if (g_bSharedPipelinePrepared) {
        return CVI_SUCCESS;
    }

    pstSysCfg = app_ipcam_Sys_Param_Get();
    pstVpssCfg = app_ipcam_Vpss_Param_Get();
    if (pstSysCfg == NULL || pstVpssCfg == NULL ||
        g_pstObjTrackCfg->VpssGrp < 0 ||
        g_pstObjTrackCfg->VpssGrp >= CVI_MAX_VPSS_GRP ||
        g_pstObjTrackCfg->VpssChn < 0 ||
        g_pstObjTrackCfg->VpssChn >= VPSS_MAX_PHY_CHN_NUM) {
        return CVI_FAILURE;
    }

    pstGrpCfg = &pstVpssCfg->astVpssGrpCfg[g_pstObjTrackCfg->VpssGrp];
    if (!pstGrpCfg->aAttachEn[g_pstObjTrackCfg->VpssChn]) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "ObjectTrack DET channel requires a dedicated VB pool\n");
        return CVI_FAILURE;
    }
    s32SotPool = app_ipcam_Sys_VbPoolId_Get(OBJECT_TRACK_SOT_POOL);
    if (s32SotPool < 0 || s32SotPool >= (CVI_S32)pstSysCfg->vb_pool_num ||
	    !pstSysCfg->vb_pool[s32SotPool].bEnable ||
	    pstSysCfg->vb_pool[s32SotPool].width !=
	        g_pstObjTrackCfg->u32SotGrpWidth ||
	    pstSysCfg->vb_pool[s32SotPool].height !=
	        g_pstObjTrackCfg->u32SotGrpHeight ||
	    pstSysCfg->vb_pool[s32SotPool].fmt != PIXEL_FORMAT_NV12) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "ObjectTrack SOT pool%d must be %ux%u NV12\n",
            OBJECT_TRACK_SOT_POOL, g_pstObjTrackCfg->u32SotGrpWidth,
            g_pstObjTrackCfg->u32SotGrpHeight);
        return CVI_FAILURE;
    }

    g_stDetPipelineAttr =
        pstGrpCfg->astVpssChnAttr[g_pstObjTrackCfg->VpssChn];
    g_DetPipelinePool =
        pstGrpCfg->aAttachPool[g_pstObjTrackCfg->VpssChn];

    g_stSotPipelineAttr =
        pstGrpCfg->astVpssChnAttr[OBJECT_TRACK_REFERENCE_CHN];
    g_stSotPipelineAttr.u32Width = g_pstObjTrackCfg->u32SotGrpWidth;
    g_stSotPipelineAttr.u32Height = g_pstObjTrackCfg->u32SotGrpHeight;
    g_stSotPipelineAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
    g_stSotPipelineAttr.enPixelFormat = PIXEL_FORMAT_NV12;
    g_stSotPipelineAttr.u32Depth = 1;
    g_stSotPipelineAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
    memset(&g_stSotPipelineAttr.stNormalize, 0,
           sizeof(g_stSotPipelineAttr.stNormalize));
    g_SotPipelinePool = (VB_POOL)s32SotPool;

    g_bSharedPipelinePrepared = CVI_TRUE;
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "ObjectTrack AI VPSS channel: grp=%d chn=%d, DET pool=%u, SOT pool=%u\n",
        g_pstObjTrackCfg->VpssGrp, g_pstObjTrackCfg->VpssChn,
        g_DetPipelinePool, g_SotPipelinePool);
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Ai_Object_Track_Pipeline_WaitReady(
    VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    VIDEO_FRAME_INFO_S stFrame = {0};
    CVI_S32 s32Ret = CVI_FAILURE;
    CVI_U32 u32Attempt = 0;

    for (u32Attempt = 0;
         u32Attempt < OBJECT_TRACK_PIPELINE_READY_RETRIES;
         ++u32Attempt) {
        memset(&stFrame, 0, sizeof(stFrame));
        s32Ret = CVI_VPSS_GetChnFrame(
            VpssGrp, VpssChn, &stFrame,
            OBJECT_TRACK_PIPELINE_READY_TIMEOUT_MS);
        if (s32Ret != CVI_SUCCESS) {
            continue;
        }

        if (stFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12 &&
            stFrame.stVFrame.u32Width == g_pstObjTrackCfg->u32SotGrpWidth &&
            stFrame.stVFrame.u32Height == g_pstObjTrackCfg->u32SotGrpHeight) {
            s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);
            if (s32Ret == CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "ObjectTrack VPSS tracking frame ready after %u attempt(s)\n",
                    u32Attempt + 1);
            }
            return s32Ret;
        }

        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "ObjectTrack discards stale VPSS frame %ux%u format=%d "
            "while waiting for %ux%u NV12\n",
            stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,
            stFrame.stVFrame.enPixelFormat,
            g_pstObjTrackCfg->u32SotGrpWidth,
            g_pstObjTrackCfg->u32SotGrpHeight);
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);
        if (s32Ret != CVI_SUCCESS) {
            return s32Ret;
        }
    }

    return CVI_FAILURE;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Pipeline_Drain(
    VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    VIDEO_FRAME_INFO_S stFrame = {0};
    CVI_U32 u32Drained = 0;

    while (u32Drained < OBJECT_TRACK_PIPELINE_DRAIN_LIMIT &&
           CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrame, 0) ==
               CVI_SUCCESS) {
        if (CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame) !=
            CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_WARN,
                "ObjectTrack failed to release a drained VPSS frame\n");
            break;
        }
        memset(&stFrame, 0, sizeof(stFrame));
        u32Drained++;
    }

    if (u32Drained > 0) {
        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "ObjectTrack drained %u VPSS frame(s) before reconfigure\n",
            u32Drained);
    }
}

static CVI_S32 app_ipcam_Ai_Object_Track_Pipeline_Set(
    APP_PARAM_OBJECT_TRACK_MODE mode)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_BOOL bSameGroup =
        g_pstObjTrackCfg->SotVpssGrp == g_pstObjTrackCfg->VpssGrp;
    CVI_BOOL bSharedSotChannel = bSameGroup &&
        g_pstObjTrackCfg->SotVpssChn == g_pstObjTrackCfg->VpssChn;

    if (g_bPipelineInitialized && g_PipelineMode == mode) {
        if (mode != TRACKING) {
            APP_PARAM_AI_PD_CFG_S *pstPdCfg = app_ipcam_Ai_PD_Param_Get();
            if (pstPdCfg != NULL && pstPdCfg->bEnable) {
                app_ipcam_Ai_PD_Pause_Set(CVI_FALSE);
            }
        }
        return CVI_SUCCESS;
    }

    if (bSharedSotChannel) {
        const VPSS_CHN_ATTR_S *pstChnAttr = NULL;
        VB_POOL VbPool = VB_INVALID_POOLID;

        s32Ret = app_ipcam_Ai_Object_Track_Shared_Pipeline_Prepare();
        if (s32Ret != CVI_SUCCESS) {
            goto pipeline_failed;
        }
        if (mode == TRACKING) {
            pstChnAttr = &g_stSotPipelineAttr;
            VbPool = g_SotPipelinePool;
        } else {
            pstChnAttr = &g_stDetPipelineAttr;
            VbPool = g_DetPipelinePool;
        }
        app_ipcam_Ai_Object_Track_Pipeline_Drain(
            g_pstObjTrackCfg->VpssGrp, g_pstObjTrackCfg->VpssChn);
        s32Ret = app_ipcam_Vpss_Chn_Reconfigure(
            g_pstObjTrackCfg->VpssGrp, g_pstObjTrackCfg->VpssChn,
            pstChnAttr, CVI_TRUE, VbPool);
    } else if (mode == TRACKING) {
        s32Ret = app_ipcam_Vpss_Chn_SetEnabled(
            g_pstObjTrackCfg->SotVpssGrp, g_pstObjTrackCfg->SotVpssChn,
            CVI_TRUE);
        if (s32Ret == CVI_SUCCESS && !bSameGroup) {
            s32Ret = app_ipcam_Vpss_Bind(g_pstObjTrackCfg->SotVpssGrp);
        }
        if (s32Ret == CVI_SUCCESS) {
            s32Ret = app_ipcam_Vpss_Chn_SetEnabled(
                g_pstObjTrackCfg->VpssGrp, g_pstObjTrackCfg->VpssChn,
                CVI_FALSE);
        }
    } else {
        if (!bSameGroup) {
            s32Ret = app_ipcam_Vpss_Unbind(g_pstObjTrackCfg->SotVpssGrp);
        }
        if (s32Ret == CVI_SUCCESS) {
            s32Ret = app_ipcam_Vpss_Chn_SetEnabled(
                g_pstObjTrackCfg->SotVpssGrp,
                g_pstObjTrackCfg->SotVpssChn, CVI_FALSE);
        }
        if (s32Ret == CVI_SUCCESS) {
            s32Ret = app_ipcam_Vpss_Chn_SetEnabled(
                g_pstObjTrackCfg->VpssGrp, g_pstObjTrackCfg->VpssChn,
                CVI_TRUE);
        }
    }

pipeline_failed:
    if (s32Ret != CVI_SUCCESS) {
        g_bPipelineInitialized = CVI_FALSE;
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "switch ObjectTrack VPSS pipeline to %s failed with %#x\n",
            app_ipcam_Ai_Object_Track_Mode_Name(mode), s32Ret);
        return s32Ret;
    }

    if (mode == TRACKING) {
        s32Ret = app_ipcam_Ai_Object_Track_Pipeline_WaitReady(
            g_pstObjTrackCfg->SotVpssGrp,
            g_pstObjTrackCfg->SotVpssChn);
        if (s32Ret != CVI_SUCCESS) {
                g_bPipelineInitialized = CVI_FALSE;
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "ObjectTrack VPSS tracking warmup timed out, "
                "returning to idle pipeline, ret=%#x\n",
                s32Ret);
            return s32Ret;
        }
    }

    if (mode == TRACKING) {
        app_ipcam_Ai_Object_Track_ObjDraw_Clear();
    }
    g_PipelineMode = mode;
    g_bPipelineInitialized = CVI_TRUE;
    if (mode != TRACKING) {
        APP_PARAM_AI_PD_CFG_S *pstPdCfg = app_ipcam_Ai_PD_Param_Get();
        if (pstPdCfg != NULL && pstPdCfg->bEnable) {
            app_ipcam_Ai_PD_Pause_Set(CVI_FALSE);
        }
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "ObjectTrack VPSS pipeline: %s\n",
        app_ipcam_Ai_Object_Track_Mode_Name(mode));
    return CVI_SUCCESS;
}

CVI_VOID app_ipcam_Ai_Object_Track_DefaultBox_Get(int32_t box[4])
{
    CVI_U32 grp_width = g_pstObjTrackCfg->u32GrpWidth;
    CVI_U32 grp_height = g_pstObjTrackCfg->u32GrpHeight;
    CVI_U32 box_size = DEFAULT_SELECTION_BOX_SIZE;
    CVI_U32 center_x = 0;
    CVI_U32 center_y = 0;

    if (box == NULL) return;

    if (grp_width == 0 || grp_height == 0) {
        grp_width = 640;
        grp_height = 384;
    }

    if (g_pstObjTrackCfg->bFixedInitBox &&
        g_pstObjTrackCfg->u32SotGrpWidth > 0 &&
        g_pstObjTrackCfg->u32SotGrpHeight > 0) {
        CVI_U32 box_width = fmax(1, fmin(grp_width,
            ((CVI_U64)OBJECT_TRACK_FEARTRACK_INIT_BOX_SIZE * grp_width +
             g_pstObjTrackCfg->u32SotGrpWidth / 2) /
                g_pstObjTrackCfg->u32SotGrpWidth));
        CVI_U32 box_height = fmax(1, fmin(grp_height,
            ((CVI_U64)OBJECT_TRACK_FEARTRACK_INIT_BOX_SIZE * grp_height +
             g_pstObjTrackCfg->u32SotGrpHeight / 2) /
                g_pstObjTrackCfg->u32SotGrpHeight));
        box[0] = (grp_width - box_width) / 2;
        box[1] = (grp_height - box_height) / 2;
        box[2] = box[0] + box_width;
        box[3] = box[1] + box_height;
        return;
    }

    if (box_size > grp_width) {
        box_size = grp_width;
    }
    if (box_size > grp_height) {
        box_size = grp_height;
    }

    center_x = grp_width / 2;
    center_y = grp_height / 2;
    box[0] = (int32_t)(center_x - box_size / 2);
    box[1] = (int32_t)(center_y - box_size / 2);
    box[2] = (int32_t)(box[0] + box_size);
    box[3] = (int32_t)(box[1] + box_size);
}

static CVI_VOID app_ipcam_Ai_Object_Track_Box_Scale(int32_t box[4],
                                                    CVI_U32 src_width,
                                                    CVI_U32 src_height,
                                                    CVI_U32 dst_width,
                                                    CVI_U32 dst_height)
{
    if (box == NULL || src_width == 0 || src_height == 0 ||
        dst_width == 0 || dst_height == 0) {
        return;
    }

    box[0] = (int32_t)((int64_t)box[0] * dst_width / src_width);
    box[1] = (int32_t)((int64_t)box[1] * dst_height / src_height);
    box[2] = (int32_t)((int64_t)box[2] * dst_width / src_width);
    box[3] = (int32_t)((int64_t)box[3] * dst_height / src_height);

    if (box[0] < 0) box[0] = 0;
    if (box[1] < 0) box[1] = 0;
    if (box[0] >= (int32_t)dst_width) box[0] = (int32_t)dst_width - 1;
    if (box[1] >= (int32_t)dst_height) box[1] = (int32_t)dst_height - 1;
    if (box[2] > (int32_t)dst_width) box[2] = (int32_t)dst_width;
    if (box[3] > (int32_t)dst_height) box[3] = (int32_t)dst_height;
    if (box[2] <= box[0]) box[2] = box[0] + 1;
    if (box[3] <= box[1]) box[3] = box[1] + 1;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Box_Clamp(int32_t box[4])
{
    int32_t width = (int32_t)g_pstObjTrackCfg->u32GrpWidth;
    int32_t height = (int32_t)g_pstObjTrackCfg->u32GrpHeight;

    if (box == NULL || width <= 0 || height <= 0) {
        return;
    }
    if (box[0] < 0) box[0] = 0;
    if (box[1] < 0) box[1] = 0;
    if (box[0] >= width) box[0] = width - 1;
    if (box[1] >= height) box[1] = height - 1;
    if (box[2] > width) box[2] = width;
    if (box[3] > height) box[3] = height;
    if (box[2] <= box[0]) box[2] = box[0] + 1;
    if (box[3] <= box[1]) box[3] = box[1] + 1;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Box_FixedSquare(
    int32_t box[4], CVI_U32 frame_width, CVI_U32 frame_height,
    CVI_U32 box_size)
{
    int32_t width = (int32_t)frame_width;
    int32_t height = (int32_t)frame_height;
    int32_t size = (int32_t)box_size;
    int32_t center_x = 0;
    int32_t center_y = 0;
    int32_t x1 = 0;
    int32_t y1 = 0;

    if (box == NULL || width <= 0 || height <= 0 || size <= 0) {
        return;
    }
    if (size > width) size = width;
    if (size > height) size = height;

    center_x = (int32_t)(((int64_t)box[0] + box[2]) / 2);
    center_y = (int32_t)(((int64_t)box[1] + box[3]) / 2);
    x1 = center_x - size / 2;
    y1 = center_y - size / 2;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x1 > width - size) x1 = width - size;
    if (y1 > height - size) y1 = height - size;

    box[0] = x1;
    box[1] = y1;
    box[2] = x1 + size;
    box[3] = y1 + size;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Request_Default(
    APP_OBJECT_TRACK_REQUEST_S *pstRequest)
{
    if (pstRequest == NULL) {
        return;
    }
    memset(pstRequest, 0, sizeof(*pstRequest));
    pstRequest->type = APP_OBJECT_TRACK_REQUEST_NONE;
}

static CVI_BOOL app_ipcam_Ai_Object_Track_Request_Parse(
    const char *text, APP_OBJECT_TRACK_REQUEST_S *pstRequest)
{
    char command[16] = {0};
    int32_t x = 0;
    int32_t y = 0;
    int32_t view_width = 0;
    int32_t view_height = 0;
    int32_t x1 = 0;
    int32_t y1 = 0;
    int32_t x2 = 0;
    int32_t y2 = 0;
    unsigned long long track_id = 0;

    if (text == NULL || pstRequest == NULL) {
        return CVI_FALSE;
    }
    app_ipcam_Ai_Object_Track_Request_Default(pstRequest);
    if (sscanf(text, "%15s", command) != 1) {
        return CVI_FALSE;
    }
    if (strcmp(command, "fastsam") == 0 || strcmp(command, "color") == 0 ||
        strcmp(command, "direct") == 0) {
        const char *nested = strchr(text, ' ');
        TDLTargetSearchTypeE search_type = strcmp(command, "fastsam") == 0 ?
            TDL_FASTSAM : strcmp(command, "color") == 0 ? TDL_COLOR : TDL_REJECT;

        while (nested != NULL && *nested == ' ') {
            nested++;
        }
        if (nested == NULL || *nested == '\0' ||
            !app_ipcam_Ai_Object_Track_Request_Parse(nested, pstRequest)) {
            return CVI_FALSE;
        }
        pstRequest->search_type_override = CVI_TRUE;
        pstRequest->search_type = search_type;
        return CVI_TRUE;
    }
    if (strcmp(command, "1") == 0 || strcmp(command, "start") == 0 ||
        strcmp(command, "default") == 0) {
        pstRequest->type = APP_OBJECT_TRACK_REQUEST_DEFAULT;
        return CVI_TRUE;
    }
    if (strcmp(command, "stop") == 0 || strcmp(command, "0") == 0) {
        pstRequest->type = APP_OBJECT_TRACK_REQUEST_STOP;
        return CVI_TRUE;
    }
    if (strcmp(command, "point") == 0 &&
        sscanf(text, "%15s %d %d %d %d", command, &x, &y,
               &view_width, &view_height) == 5 &&
        view_width > 0 && view_height > 0) {
        pstRequest->type = APP_OBJECT_TRACK_REQUEST_POINT;
        pstRequest->point_x = x;
        pstRequest->point_y = y;
        pstRequest->view_width = view_width;
        pstRequest->view_height = view_height;
        return CVI_TRUE;
    }
    if (strcmp(command, "box") == 0 &&
        sscanf(text, "%15s %d %d %d %d %d %d", command,
               &x1, &y1, &x2, &y2, &view_width, &view_height) == 7 &&
        view_width > 0 && view_height > 0) {
        pstRequest->type = APP_OBJECT_TRACK_REQUEST_BOX;
        pstRequest->box[0] = x1;
        pstRequest->box[1] = y1;
        pstRequest->box[2] = x2;
        pstRequest->box[3] = y2;
        pstRequest->view_width = view_width;
        pstRequest->view_height = view_height;
        return CVI_TRUE;
    }
    if (strcmp(command, "box") == 0 &&
        sscanf(text, "%15s %d %d %d %d", command,
               &x1, &y1, &x2, &y2) == 5) {
        pstRequest->type = APP_OBJECT_TRACK_REQUEST_BOX;
        pstRequest->box[0] = x1;
        pstRequest->box[1] = y1;
        pstRequest->box[2] = x2;
        pstRequest->box[3] = y2;
        return CVI_TRUE;
    }
    if (strcmp(command, "id") == 0 &&
        sscanf(text, "%15s %llu", command, &track_id) == 2) {
        pstRequest->type = APP_OBJECT_TRACK_REQUEST_ID;
        pstRequest->track_id = (uint64_t)track_id;
        return CVI_TRUE;
    }
    APP_PROF_LOG_PRINT(LEVEL_WARN, "invalid track request: %s\n", text);
    return CVI_FALSE;
}

static CVI_BOOL app_ipcam_Ai_Object_Track_Request_File_Load(
    const char *pszPath, char *pszText, size_t u32TextSize)
{
    FILE *pFile = NULL;

    if (pszPath == NULL || pszText == NULL || u32TextSize == 0) {
        return CVI_FALSE;
    }
    pFile = fopen(pszPath, "r");
    if (pFile == NULL) {
        return CVI_FALSE;
    }
    if (fgets(pszText, u32TextSize, pFile) == NULL) {
        fclose(pFile);
        unlink(pszPath);
        return CVI_FALSE;
    }
    fclose(pFile);
    unlink(pszPath);
    return CVI_TRUE;
}

static CVI_BOOL app_ipcam_Ai_Object_Track_Request_Load(
    APP_OBJECT_TRACK_REQUEST_S *pstRequest)
{
    char text[128] = {0};
    char command[16] = {0};

    if (pstRequest == NULL) {
        return CVI_FALSE;
    }
    if (app_ipcam_Ai_Object_Track_Request_File_Load(
            TRACK_REQUEST_PATH, text, sizeof(text))) {
        unlink(DET_TRACK_REQUEST_PATH);
        return app_ipcam_Ai_Object_Track_Request_Parse(text, pstRequest);
    }
    if (!app_ipcam_Ai_Object_Track_Request_File_Load(
            DET_TRACK_REQUEST_PATH, text, sizeof(text))) {
        return CVI_FALSE;
    }
    if (sscanf(text, "%15s", command) != 1 ||
        (strcmp(command, "1") != 0 && strcmp(command, "start") != 0 &&
         strcmp(command, "default") != 0)) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "invalid DET track request: %s\n", text);
        return CVI_FALSE;
    }
    app_ipcam_Ai_Object_Track_Request_Default(pstRequest);
    pstRequest->type = APP_OBJECT_TRACK_REQUEST_DET_CENTER;
    pstRequest->search_type_override = CVI_TRUE;
    pstRequest->search_type = TDL_REJECT;
    pstRequest->request_time_us =
        app_ipcam_Ai_Object_Track_TimeUs();
    return CVI_TRUE;
}

static CVI_BOOL app_ipcam_Ai_Object_Track_Select_Det_Object(
    const TDLObjectInfo *pstObject, CVI_U32 u32Width, CVI_U32 u32Height,
    APP_OBJECT_TRACK_SELECTION_S *pstSelection)
{
    if (pstObject == NULL || pstSelection == NULL ||
        pstObject->box.x2 <= pstObject->box.x1 ||
        pstObject->box.y2 <= pstObject->box.y1) {
        return CVI_FALSE;
    }
    pstSelection->valid = CVI_TRUE;
    pstSelection->from_det = CVI_TRUE;
    pstSelection->class_id = pstObject->class_id;
    pstSelection->track_id = pstObject->track_id;
    pstSelection->box[0] = (int32_t)pstObject->box.x1;
    pstSelection->box[1] = (int32_t)pstObject->box.y1;
    pstSelection->box[2] = (int32_t)pstObject->box.x2;
    pstSelection->box[3] = (int32_t)pstObject->box.y2;
    if (u32Width > 0 && u32Height > 0) {
        app_ipcam_Ai_Object_Track_Box_Scale(
            pstSelection->box, u32Width, u32Height,
            g_pstObjTrackCfg->u32GrpWidth,
            g_pstObjTrackCfg->u32GrpHeight);
    } else {
        app_ipcam_Ai_Object_Track_Box_Clamp(pstSelection->box);
    }
    return CVI_TRUE;
}

static CVI_BOOL app_ipcam_Ai_Object_Track_Select_Det_By_Id(
    const TDLObject *pstObject, uint64_t track_id,
    APP_OBJECT_TRACK_SELECTION_S *pstSelection)
{
    if (pstObject == NULL || pstObject->info == NULL ||
        pstSelection == NULL || track_id == 0) {
        return CVI_FALSE;
    }
    for (CVI_U32 i = 0; i < pstObject->size; i++) {
        if (pstObject->info[i].track_id != track_id) {
            continue;
        }
        return app_ipcam_Ai_Object_Track_Select_Det_Object(
            &pstObject->info[i], pstObject->width, pstObject->height,
            pstSelection);
    }
    return CVI_FALSE;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Box_Around_Point(
    int32_t x, int32_t y, int32_t box_size, int32_t box[4])
{
    box[0] = x - box_size / 2;
    box[1] = y - box_size / 2;
    box[2] = box[0] + box_size;
    box[3] = box[1] + box_size;
    app_ipcam_Ai_Object_Track_Box_Clamp(box);
}

static CVI_BOOL app_ipcam_Ai_Object_Track_Select(
    const TDLObject *pstObject, const APP_OBJECT_TRACK_REQUEST_S *pstRequest,
    APP_OBJECT_TRACK_SELECTION_S *pstSelection)
{
    int32_t point_x = 0;
    int32_t point_y = 0;

    if (pstRequest == NULL || pstSelection == NULL) {
        return CVI_FALSE;
    }
    memset(pstSelection, 0, sizeof(*pstSelection));
    pstSelection->class_id = -1;
    pstSelection->search_type_override = pstRequest->search_type_override;
    pstSelection->search_type = pstRequest->search_type;

    if (pstRequest->type == APP_OBJECT_TRACK_REQUEST_ID) {
        return app_ipcam_Ai_Object_Track_Select_Det_By_Id(
            pstObject, pstRequest->track_id, pstSelection);
    }
    if (pstRequest->type == APP_OBJECT_TRACK_REQUEST_POINT) {
        if (g_pstObjTrackCfg->bSelectAtReticle) {
            app_ipcam_Ai_Object_Track_DefaultBox_Get(pstSelection->box);
            pstSelection->valid = CVI_TRUE;
            pstSelection->search_type_override = CVI_TRUE;
            pstSelection->search_type = TDL_REJECT;
            return CVI_TRUE;
        }
        point_x = (int32_t)((int64_t)pstRequest->point_x *
            g_pstObjTrackCfg->u32GrpWidth / pstRequest->view_width);
        point_y = (int32_t)((int64_t)pstRequest->point_y *
            g_pstObjTrackCfg->u32GrpHeight / pstRequest->view_height);
        if (point_x < 0) point_x = 0;
        if (point_y < 0) point_y = 0;
        if (point_x >= (int32_t)g_pstObjTrackCfg->u32GrpWidth) {
            point_x = (int32_t)g_pstObjTrackCfg->u32GrpWidth - 1;
        }
        if (point_y >= (int32_t)g_pstObjTrackCfg->u32GrpHeight) {
            point_y = (int32_t)g_pstObjTrackCfg->u32GrpHeight - 1;
        }
        pstSelection->valid = CVI_TRUE;
        pstSelection->point_prompt = CVI_TRUE;
        pstSelection->point[0] = point_x;
        pstSelection->point[1] = point_y;
        app_ipcam_Ai_Object_Track_Box_Around_Point(
            point_x, point_y, DEFAULT_SELECTION_BOX_SIZE, pstSelection->box);
        return CVI_TRUE;
    }
    if (pstRequest->type == APP_OBJECT_TRACK_REQUEST_BOX) {
        memcpy(pstSelection->box, pstRequest->box,
               sizeof(pstSelection->box));
        if (pstRequest->view_width > 0 && pstRequest->view_height > 0) {
            app_ipcam_Ai_Object_Track_Box_Scale(
                pstSelection->box, pstRequest->view_width,
                pstRequest->view_height, g_pstObjTrackCfg->u32GrpWidth,
                g_pstObjTrackCfg->u32GrpHeight);
        } else {
            app_ipcam_Ai_Object_Track_Box_Clamp(pstSelection->box);
        }
        pstSelection->valid = CVI_TRUE;
        return CVI_TRUE;
    } else if (pstRequest->type == APP_OBJECT_TRACK_REQUEST_DEFAULT) {
        app_ipcam_Ai_Object_Track_DefaultBox_Get(pstSelection->box);
        pstSelection->point_prompt = CVI_TRUE;
        pstSelection->point[0] =
            (pstSelection->box[0] + pstSelection->box[2]) / 2;
        pstSelection->point[1] =
            (pstSelection->box[1] + pstSelection->box[3]) / 2;
        pstSelection->valid = CVI_TRUE;
        return CVI_TRUE;
    } else {
        return CVI_FALSE;
    }
}

APP_PARAM_AI_OBJECT_TRACK_CFG_S *app_ipcam_Ai_Object_Track_Param_Get(void)
{
    return g_pstObjTrackCfg;
}

APP_PARAM_OBJECT_TRACK_MODE app_ipcam_Ai_Object_Track_Mode_Get(void) {
    pthread_mutex_lock(&g_ModeMutex);
    APP_PARAM_OBJECT_TRACK_MODE mode = g_mode;
    pthread_mutex_unlock(&g_ModeMutex);
    return mode;
}

CVI_VOID app_ipcam_Ai_Object_Track_Mode_Set(APP_PARAM_OBJECT_TRACK_MODE mode) {
    APP_PARAM_AI_PD_CFG_S *pstPdCfg = app_ipcam_Ai_PD_Param_Get();

    pthread_mutex_lock(&g_ModeMutex);
    g_mode = mode;
    pthread_mutex_unlock(&g_ModeMutex);

    if (mode != TRACKING) {
        app_ipcam_Ai_Object_Track_ObjDraw_Clear();
    }

    if (pstPdCfg != NULL && pstPdCfg->bEnable && mode == TRACKING) {
        app_ipcam_Ai_PD_Pause_Set(CVI_TRUE);
    }
}

CVI_VOID app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_BOOL flag)
{
    g_bObjectTrackRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Object_Track_ProcStatus_Get(void)
{
    return g_bObjectTrackRunning;
}

CVI_VOID app_ipcam_Ai_Object_Track_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_StatusMutex);
    g_bObjectTrackPause = flag;
    pthread_mutex_unlock(&g_StatusMutex);
}

CVI_BOOL app_ipcam_Ai_Object_Track_Pause_Get(void)
{
    pthread_mutex_lock(&g_StatusMutex);
    CVI_BOOL Pause = g_bObjectTrackPause;
    pthread_mutex_unlock(&g_StatusMutex);
    return Pause;
}

CVI_VOID app_ipcam_Ai_Object_Track_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    if (pstAiObj == NULL) return;
    if (pstAiObj->info == NULL) {
        pstAiObj->info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
    }
    if (pstAiObj->info == NULL) {
        pstAiObj->size = 0;
        return;
    }

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        if (g_stObjDraw.size == 0 || g_stObjDraw.info == NULL) {
            pstAiObj->size = 0;
            return;
        }
        pstAiObj->size = g_stObjDraw.size <= MAX_DET_NUM ? g_stObjDraw.size : MAX_DET_NUM;
        memcpy(pstAiObj->info, g_stObjDraw.info, pstAiObj->size * sizeof(TDLObjectInfo));
    }

}

static CVI_VOID *Thread_Object_Track_Proc(CVI_VOID *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPSS_GRP SotVpssGrp = g_pstObjTrackCfg->SotVpssGrp;
    VPSS_CHN SotVpssChn = g_pstObjTrackCfg->SotVpssChn;
    TDLObject cur_det_meta = {0};
    APP_AI_RESULT_FRAME_INFO_S cur_det_frame = {0};
    TDLTracker track_meta = {0};
    APP_OBJECT_TRACK_REQUEST_S pending_request = {0};
    APP_OBJECT_TRACK_SELECTION_S selection = {0};
    bool track_init = CVI_FALSE;
    bool pending_request_valid = false;
    bool sot_input_error_reported = false;
    CVI_U32 sot_unreliable_frames = 0;
    CVI_U32 sot_recovery_frames = 0;
    CVI_BOOL bSotHardLostObserved = CVI_FALSE;
    CVI_U32 frame_error_count = 0;
    uint64_t lost_start_us = 0;
    uint64_t candidate_start_us = 0;
    uint64_t frame_error_start_us = 0;
    APP_OBJECT_TRACK_GMC_RESULT_S stLatestGmcResult = {0};
    CVI_U64 u64LatestGmcFrameId = 0;
    CVI_BOOL bLatestGmcAvailable = CVI_FALSE;
    CVI_BOOL bGmcPredictionActive = CVI_FALSE;
    CVI_BOOL bRefinedTarget = CVI_FALSE;
    CVI_FLOAT fTargetSizeFactor = 0.0f;
    APP_OBJECT_TRACK_PERF_S stPerf = {0};
#ifdef OSDC_SUPPORT
    APP_OBJECT_TRACK_OSD_PREDICTOR_S stOsdPredictor = {0};
#endif

    (void)pArgs;
    g_bSharedPipelinePrepared = CVI_FALSE;
    g_bPipelineInitialized = CVI_FALSE;
    unlink(TRACK_FPS_STATUS_PATH);
    unlink(TRACK_FPS_STATUS_TMP_PATH);
    app_ipcam_Ai_Object_Track_Mode_Set(
        app_ipcam_Ai_Object_Track_Idle_Mode_Get());
    app_ipcam_Ai_Object_Track_Perf_Reset(
        &stPerf, app_ipcam_Ai_Object_Track_Mode_Get(),
        app_ipcam_Ai_Object_Track_TimeUs());
    app_ipcam_Ai_Object_Track_Gmc_Reset();

    for (CVI_U32 wait_ms = 0;
         !app_ipcam_Venc_All_Stream_Ready() && wait_ms < 8000;
         wait_ms += 10) {
        usleep(10 * 1000);
    }
    if (!app_ipcam_Venc_All_Stream_Ready()) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
                           "VENC streams not stable after 8000 ms, start ObjectTrack frame loop anyway\n");
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO,
                           "VENC streams stable, start ObjectTrack frame loop\n");
    }

    /*
     * 主跟踪线程调度说明：
     * 1. DET 结果由独立的 ai_pd 管线提供，本线程不再加载或执行检测模型。
     * 2. TRACK 可以接收 DET 目标，也可以由 FastSAM、Color 或框选直接初始化。
     * 3. /tmp/track 默认/点选对准星中心正方形；/tmp/det_track 仍用最新 DET 中心目标。
     * 4. 停止或丢失后按 DET 开关决定空闲态。
     * 5. 低频帧先抽取 GMC 小网格再交给单槽线程，VPSS 帧在当轮释放。
     */
    while (app_ipcam_Ai_Object_Track_ProcStatus_Get()) {
        VIDEO_FRAME_INFO_S stFrame = {0};
        TDLImage image = NULL;
        VPSS_GRP FrameVpssGrp = SotVpssGrp;
        VPSS_CHN FrameVpssChn = SotVpssChn;
        APP_PARAM_OBJECT_TRACK_MODE mode = WAIT_TARGET;
        APP_OBJECT_TRACK_REQUEST_S request = {0};
        bool frame_acquired = false;
        CVI_U32 u32StaleFrames = 0;
        uint64_t frame_wait_start_us = 0;
        uint64_t frame_drain_start_us = 0;
        uint64_t wrap_start_us = 0;
        uint64_t inference_start_us = 0;
        uint64_t result_start_us = 0;
        uint64_t cleanup_start_us = 0;
        APP_OBJECT_TRACK_GMC_RESULT_S gmc_result = {0};
        CVI_S32 gmc_status = CVI_SUCCESS;
        CVI_U64 gmc_result_frame_id = 0;

        if (app_ipcam_Ai_Object_Track_Pause_Get()) {
            usleep(1000*1000);
            continue;
        }

        if (app_ipcam_Ai_Object_Track_Request_Load(&request)) {
            if (request.type == APP_OBJECT_TRACK_REQUEST_STOP) {
                app_ipcam_Ai_Object_Track_Request_Default(&pending_request);
                pending_request_valid = false;
                memset(&selection, 0, sizeof(selection));
                track_init = CVI_FALSE;
                sot_unreliable_frames = 0;
                sot_recovery_frames = 0;
                lost_start_us = 0;
                candidate_start_us = 0;
                bSotHardLostObserved = CVI_FALSE;
                frame_error_count = 0;
                frame_error_start_us = 0;
                bLatestGmcAvailable = CVI_FALSE;
                bGmcPredictionActive = CVI_FALSE;
                bRefinedTarget = CVI_FALSE;
                fTargetSizeFactor = 0.0f;
                app_ipcam_Ai_Object_Track_Gmc_Reset();
                app_ipcam_Ai_Object_Track_Mode_Set(
                    app_ipcam_Ai_Object_Track_Idle_Mode_Get());
                APP_PROF_LOG_PRINT(LEVEL_INFO, "ObjectTrack stop request\n");
            } else {
                pending_request = request;
                pending_request_valid = true;
                memset(&selection, 0, sizeof(selection));
                track_init = CVI_FALSE;
                sot_unreliable_frames = 0;
                sot_recovery_frames = 0;
                lost_start_us = 0;
                candidate_start_us = 0;
                bSotHardLostObserved = CVI_FALSE;
                frame_error_count = 0;
                frame_error_start_us = 0;
                bLatestGmcAvailable = CVI_FALSE;
                bGmcPredictionActive = CVI_FALSE;
                bRefinedTarget = CVI_FALSE;
                fTargetSizeFactor = 0.0f;
                app_ipcam_Ai_Object_Track_Gmc_Reset();
                app_ipcam_Ai_Object_Track_Mode_Set(
                    app_ipcam_Ai_Object_Track_Idle_Mode_Get());
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "ObjectTrack target request type=%d\n", request.type);
            }
        }

        mode = app_ipcam_Ai_Object_Track_Mode_Get();
        if (app_ipcam_Ai_Object_Track_Pipeline_Set(mode) != CVI_SUCCESS) {
            if (mode == TRACKING) {
                track_init = CVI_FALSE;
                sot_unreliable_frames = 0;
                sot_recovery_frames = 0;
                lost_start_us = 0;
                candidate_start_us = 0;
                bSotHardLostObserved = CVI_FALSE;
                frame_error_count = 0;
                frame_error_start_us = 0;
                bLatestGmcAvailable = CVI_FALSE;
                bGmcPredictionActive = CVI_FALSE;
                bRefinedTarget = CVI_FALSE;
                fTargetSizeFactor = 0.0f;
                memset(&selection, 0, sizeof(selection));
                app_ipcam_Ai_Object_Track_Gmc_Reset();
                app_ipcam_Ai_Object_Track_Mode_Set(
                    app_ipcam_Ai_Object_Track_Idle_Mode_Get());
            }
            usleep(10 * 1000);
            continue;
        }
        if (stPerf.mode != mode) {
            app_ipcam_Ai_Object_Track_Perf_Reset(
                &stPerf, mode, app_ipcam_Ai_Object_Track_TimeUs());
        }

        /*
         * 等待目标时只读取 PD 发布的最新结果，不再与 PD 竞争同一 VPSS 通道。
         * 点选和框选不依赖 DET，也可以在 WAIT_TARGET 状态直接进入跟踪。
         */
        if (mode != TRACKING) {
            sot_unreliable_frames = 0;
            sot_recovery_frames = 0;
            lost_start_us = 0;
            candidate_start_us = 0;
            bSotHardLostObserved = CVI_FALSE;
            frame_error_count = 0;
            frame_error_start_us = 0;
            bLatestGmcAvailable = CVI_FALSE;
            bGmcPredictionActive = CVI_FALSE;
            bRefinedTarget = CVI_FALSE;
            fTargetSizeFactor = 0.0f;
            if (mode == DETECTION) {
                s32Ret = app_ipcam_Ai_PD_ObjDrawInfo_GetWithFrame(
                    &cur_det_meta, &cur_det_frame);
                if (s32Ret != CVI_SUCCESS ||
                    (cur_det_meta.size > 0 && cur_det_meta.info == NULL)) {
                    cur_det_meta.size = 0;
                    memset(&cur_det_frame, 0, sizeof(cur_det_frame));
                }
            } else {
                cur_det_meta.size = 0;
                memset(&cur_det_frame, 0, sizeof(cur_det_frame));
            }

            app_ipcam_Ai_Object_Track_ObjDraw_Clear();

            if (pending_request_valid) {
                uint64_t now_us = app_ipcam_Ai_Object_Track_TimeUs();
                CVI_BOOL bRequestComplete = CVI_TRUE;
                CVI_BOOL bSelectionReady = CVI_FALSE;

                if (pending_request.type ==
                    APP_OBJECT_TRACK_REQUEST_DET_CENTER) {
                    TDLObjectInfo stNearestObject = {0};
                    APP_AI_RESULT_FRAME_INFO_S stNearestFrame = {0};
                    CVI_U32 u32NearestWidth = 0;
                    CVI_U32 u32NearestHeight = 0;
                    CVI_BOOL bFreshResult = CVI_FALSE;

                    if (mode == DETECTION &&
                        app_ipcam_Ai_PD_NearestTarget_Get(
                            &stNearestObject, &u32NearestWidth,
                            &u32NearestHeight,
                            &stNearestFrame) == CVI_SUCCESS &&
                        stNearestFrame.publish_time_us > 0 &&
                        stNearestFrame.publish_time_us <= now_us &&
                        now_us - stNearestFrame.publish_time_us <=
                            TRACK_DET_CENTER_LATCH_MAX_AGE_US) {
                        bFreshResult = CVI_TRUE;
                    }
                    if (bFreshResult) {
                        memset(&selection, 0, sizeof(selection));
                        selection.class_id = -1;
                        selection.search_type_override = CVI_TRUE;
                        selection.search_type = TDL_REJECT;
                        bSelectionReady =
                            app_ipcam_Ai_Object_Track_Select_Det_Object(
                                &stNearestObject, u32NearestWidth,
                                u32NearestHeight, &selection);
                    } else if (now_us >=
                                   pending_request.request_time_us &&
                               now_us - pending_request.request_time_us <
                                   TRACK_DET_CENTER_REQUEST_TIMEOUT_US) {
                        bRequestComplete = CVI_FALSE;
                    } else {
                        APP_PROF_LOG_PRINT(LEVEL_WARN,
                            "ObjectTrack no fresh detection target near center within 1.2 seconds\n");
                        memset(&selection, 0, sizeof(selection));
                    }
                } else {
                    bool det_request_stale =
                        pending_request.type ==
                            APP_OBJECT_TRACK_REQUEST_ID &&
                        (cur_det_frame.publish_time_us == 0 ||
                         cur_det_frame.publish_time_us > now_us ||
                         now_us - cur_det_frame.publish_time_us >
                             TRACK_DET_HANDOFF_MAX_AGE_US);

                    if (det_request_stale) {
                        APP_PROF_LOG_PRINT(LEVEL_WARN,
                            "ObjectTrack detection target is stale, wait for a new selection\n");
                        memset(&selection, 0, sizeof(selection));
                    } else {
                        bSelectionReady = app_ipcam_Ai_Object_Track_Select(
                            &cur_det_meta, &pending_request, &selection);
                        if (!bSelectionReady) {
                            APP_PROF_LOG_PRINT(LEVEL_WARN,
                                "ObjectTrack target selection failed, stay in %s mode\n",
                                app_ipcam_Ai_Object_Track_Mode_Name(mode));
                            memset(&selection, 0, sizeof(selection));
                        }
                    }
                }

                if (bSelectionReady) {
                    if (selection.point_prompt) {
                        APP_PROF_LOG_PRINT(LEVEL_INFO,
                            "ObjectTrack selected point=[%d,%d]\n",
                            selection.point[0], selection.point[1]);
                    } else {
                        APP_PROF_LOG_PRINT(LEVEL_INFO,
                            "ObjectTrack selected %s box=[%d,%d,%d,%d] class=%d track_id=%llu\n",
                            selection.from_det ? "DET" : "box",
                            selection.box[0], selection.box[1],
                            selection.box[2], selection.box[3],
                            selection.class_id,
                            (unsigned long long)selection.track_id);
                    }
                    app_ipcam_Ai_Object_Track_Mode_Set(TRACKING);
                }
                if (bRequestComplete) {
                    app_ipcam_Ai_Object_Track_Request_Default(
                        &pending_request);
                    pending_request_valid = false;
                }
            }

            usleep(10 * 1000);
            goto loop_cleanup;
        }

        frame_wait_start_us = app_ipcam_Ai_Object_Track_TimeUs();
        s32Ret = CVI_VPSS_GetChnFrame(
            FrameVpssGrp, FrameVpssChn, &stFrame,
            OBJECT_TRACK_FRAME_TIMEOUT_MS);
        if (s32Ret != CVI_SUCCESS) {
            uint64_t now_us = app_ipcam_Ai_Object_Track_TimeUs();

            if (frame_error_count == 0) {
                frame_error_start_us = frame_wait_start_us;
                APP_PROF_LOG_PRINT(LEVEL_WARN,
                    "Grp(%d)-Chn(%d) tracking frame unavailable, ret=%#x\n",
                    FrameVpssGrp, FrameVpssChn, s32Ret);
            }
            if (frame_error_count < UINT32_MAX) {
                frame_error_count++;
            }
            if (frame_error_count >= OBJECT_TRACK_FRAME_ERROR_LIMIT) {
                uint64_t stalled_us = now_us >= frame_error_start_us ?
                    now_us - frame_error_start_us : 0;

                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "Grp(%d)-Chn(%d) tracking frame stalled for %llu ms "
                    "after %u failures, returning to idle\n",
                    FrameVpssGrp, FrameVpssChn,
                    (unsigned long long)(stalled_us / 1000ULL),
                    frame_error_count);
                track_init = CVI_FALSE;
                sot_unreliable_frames = 0;
                sot_recovery_frames = 0;
                lost_start_us = 0;
                candidate_start_us = 0;
                bSotHardLostObserved = CVI_FALSE;
                frame_error_count = 0;
                frame_error_start_us = 0;
                bLatestGmcAvailable = CVI_FALSE;
                bGmcPredictionActive = CVI_FALSE;
                memset(&selection, 0, sizeof(selection));
                app_ipcam_Ai_Object_Track_Gmc_Reset();
                app_ipcam_Ai_Object_Track_Mode_Set(
                    app_ipcam_Ai_Object_Track_Idle_Mode_Get());
            }
            goto loop_cleanup;
        }
        frame_error_count = 0;
        frame_error_start_us = 0;
        frame_acquired = true;
        stPerf.frame_wait_total_us +=
            app_ipcam_Ai_Object_Track_TimeUs() - frame_wait_start_us;
        frame_drain_start_us = app_ipcam_Ai_Object_Track_TimeUs();
        while (u32StaleFrames < OBJECT_TRACK_PIPELINE_DRAIN_LIMIT) {
            VIDEO_FRAME_INFO_S stLatestFrame = {0};
            CVI_S32 s32LatestRet = CVI_VPSS_GetChnFrame(
                FrameVpssGrp, FrameVpssChn, &stLatestFrame, 0);

            if (s32LatestRet != CVI_SUCCESS) {
                break;
            }
            s32LatestRet = CVI_VPSS_ReleaseChnFrame(
                FrameVpssGrp, FrameVpssChn, &stFrame);
            if (s32LatestRet != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "Grp(%d)-Chn(%d) release stale tracking frame "
                    "failed with %#x\n",
                    FrameVpssGrp, FrameVpssChn, s32LatestRet);
                CVI_VPSS_ReleaseChnFrame(
                    FrameVpssGrp, FrameVpssChn, &stLatestFrame);
                break;
            }
            stFrame = stLatestFrame;
            u32StaleFrames++;
        }
        stPerf.frame_drain_total_us +=
            app_ipcam_Ai_Object_Track_TimeUs() - frame_drain_start_us;
        stPerf.stale_frames += u32StaleFrames;
        stPerf.input_frames++;

        g_frame_id++;

        if (stFrame.stVFrame.enPixelFormat != PIXEL_FORMAT_NV12 ||
            stFrame.stVFrame.u32Width != g_pstObjTrackCfg->u32SotGrpWidth ||
            stFrame.stVFrame.u32Height != g_pstObjTrackCfg->u32SotGrpHeight) {
            if (!sot_input_error_reported) {
                APP_PROF_LOG_PRINT(
                    LEVEL_ERROR,
                    "SOT input mismatch: got %ux%u format=%d, expected %ux%u PIXEL_FORMAT_NV12\n",
                    stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,
                    stFrame.stVFrame.enPixelFormat,
                    g_pstObjTrackCfg->u32SotGrpWidth,
                    g_pstObjTrackCfg->u32SotGrpHeight);
                sot_input_error_reported = true;
            }
            goto loop_cleanup;
        }

        if (g_pstObjTrackCfg->sot_gmc_enable &&
            app_ipcam_Ai_Object_Track_GmcAsync_Poll(
                &gmc_result, &gmc_status, &gmc_result_frame_id)) {
            if (gmc_status == CVI_SUCCESS) {
                stPerf.gmc_samples++;
                stPerf.gmc_cache_total_us += gmc_result.cache_us;
                stPerf.gmc_grid_total_us += gmc_result.grid_us;
                stPerf.gmc_total_us += gmc_result.total_us;
                if (!gmc_result.evaluated) {
                    stPerf.gmc_baseline_samples++;
                }
            }
            if (gmc_status == CVI_SUCCESS && gmc_result.evaluated) {
                stPerf.gmc_evaluations++;
                stPerf.gmc_search_total_us += gmc_result.search_us;
                stPerf.gmc_local_search_total_us +=
                    gmc_result.local_search_us;
                stPerf.gmc_last_dx = gmc_result.dx;
                stPerf.gmc_last_dy = gmc_result.dy;
                stPerf.gmc_last_confidence = gmc_result.confidence;
                stPerf.gmc_last_frame_gap = gmc_result.frame_gap;
                stPerf.gmc_local_last_dx = gmc_result.local_dx;
                stPerf.gmc_local_last_dy = gmc_result.local_dy;
                stPerf.gmc_local_last_confidence =
                    gmc_result.local_confidence;
                stPerf.gmc_local_last_frame_gap =
                    gmc_result.local_frame_gap;
                if (gmc_result.valid) {
                    stPerf.gmc_valid++;
                }
                if (gmc_result.local_valid) {
                    stPerf.gmc_local_valid++;
                }
                if (gmc_result_frame_id > g_frame_id ||
                    g_frame_id - gmc_result_frame_id >
                        OBJECT_TRACK_GMC_RESULT_MAX_AGE_FRAMES) {
                    gmc_result.valid = CVI_FALSE;
                    gmc_result.local_valid = CVI_FALSE;
                }
                stLatestGmcResult = gmc_result;
                u64LatestGmcFrameId = gmc_result_frame_id;
                bLatestGmcAvailable =
                    gmc_result.valid || gmc_result.local_valid;
            } else if (gmc_status != CVI_SUCCESS) {
                stPerf.gmc_failures++;
                bLatestGmcAvailable = CVI_FALSE;
            }
        }

        wrap_start_us = app_ipcam_Ai_Object_Track_TimeUs();
        image = TDL_WrapFrame(&stFrame, true, false);
        stPerf.wrap_total_us +=
            app_ipcam_Ai_Object_Track_TimeUs() - wrap_start_us;
        if (image == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to wrap frame \n");
            goto loop_cleanup;
        }

        /*
         * 追踪初始化阶段：读取一次 /tmp/track 事件并调用 SetSingleObjectTracking。
         * 初始化成功后切换到持续追踪；失败则清理事件并回到当前空闲态。
         */
        if (!track_init) {
            const char *model_path = NULL;
            TDLTargetSearchTypeE search_type = selection.search_type_override ?
                selection.search_type : g_pstObjTrackCfg->search_type;
            TDLObject empty_det_meta = {0};
            int32_t set_values[4] = {0};
            int32_t hint_values[4] = {0};
            int32_t set_value_count = 4;
            const char *selection_source = NULL;
            TDLBox point_hint = {0};
            TDLBox actual_target = {0};
            bool refined_target = false;

#ifdef OSDC_SUPPORT
            app_ipcam_Ai_Object_Track_OsdPredict_FilterReset(&stOsdPredictor);
#endif

            if (!selection.valid) {
                APP_PROF_LOG_PRINT(LEVEL_WARN,
                    "ObjectTrack has no valid selection, return to idle\n");
                app_ipcam_Ai_Object_Track_Mode_Set(
                    app_ipcam_Ai_Object_Track_Idle_Mode_Get());
                goto loop_cleanup;
            }
            if (search_type < TDL_REJECT || search_type > TDL_FASTSAM) {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "invalid search_type(%d), fallback to TDL_REJECT\n", search_type);
                search_type = TDL_REJECT;
            }
            if (selection.from_det &&
                !g_pstObjTrackCfg->bSotRefineSelectedDet) {
                search_type = TDL_REJECT;
            }
            if (g_pstObjTrackCfg->bFixedInitBox) {
                search_type = TDL_REJECT;
            }
            if (search_type == TDL_FASTSAM) {
                model_path = g_pstObjTrackCfg->model_path_sam;
            }
            if (selection.point_prompt && search_type == TDL_FASTSAM) {
                set_values[0] = (int32_t)((int64_t)selection.point[0] *
                    g_pstObjTrackCfg->u32SotGrpWidth /
                    g_pstObjTrackCfg->u32GrpWidth);
                set_values[1] = (int32_t)((int64_t)selection.point[1] *
                    g_pstObjTrackCfg->u32SotGrpHeight /
                    g_pstObjTrackCfg->u32GrpHeight);
                memcpy(hint_values, selection.box, sizeof(hint_values));
                app_ipcam_Ai_Object_Track_Box_Scale(
                    hint_values, g_pstObjTrackCfg->u32GrpWidth,
                    g_pstObjTrackCfg->u32GrpHeight,
                    g_pstObjTrackCfg->u32SotGrpWidth,
                    g_pstObjTrackCfg->u32SotGrpHeight);
                point_hint.x1 = hint_values[0];
                point_hint.y1 = hint_values[1];
                point_hint.x2 = hint_values[2];
                point_hint.y2 = hint_values[3];
                set_value_count = 2;
                selection_source = "point";
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "ObjectTrack initialize source=%s prompt=[%d,%d] "
                    "hint=[%d,%d,%d,%d] search_type=%d\n",
                    selection_source, set_values[0], set_values[1],
                    hint_values[0], hint_values[1], hint_values[2],
                    hint_values[3], search_type);
            } else {
                memcpy(set_values, selection.box, sizeof(set_values));
                app_ipcam_Ai_Object_Track_Box_Scale(
                    set_values, g_pstObjTrackCfg->u32GrpWidth,
                    g_pstObjTrackCfg->u32GrpHeight,
                    g_pstObjTrackCfg->u32SotGrpWidth,
                    g_pstObjTrackCfg->u32SotGrpHeight);
                if (g_pstObjTrackCfg->bFixedInitBox) {
                    if (selection.point_prompt) {
                        set_values[0] = set_values[2] = (int32_t)(
                            (int64_t)selection.point[0] * g_pstObjTrackCfg->u32SotGrpWidth /
                            g_pstObjTrackCfg->u32GrpWidth);
                        set_values[1] = set_values[3] = (int32_t)(
                            (int64_t)selection.point[1] * g_pstObjTrackCfg->u32SotGrpHeight /
                            g_pstObjTrackCfg->u32GrpHeight);
                    }
                    app_ipcam_Ai_Object_Track_Box_FixedSquare(
                        set_values, g_pstObjTrackCfg->u32SotGrpWidth,
                        g_pstObjTrackCfg->u32SotGrpHeight,
                        OBJECT_TRACK_FEARTRACK_INIT_BOX_SIZE);
                }
                selection_source = selection.from_det ? "DET" : "box";
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "ObjectTrack initialize source=%s box=[%d,%d,%d,%d] search_type=%d\n",
                    selection_source, set_values[0], set_values[1],
                    set_values[2], set_values[3], search_type);
            }

            inference_start_us = app_ipcam_Ai_Object_Track_TimeUs();
            if (selection.point_prompt && search_type == TDL_FASTSAM) {
                s32Ret = TDL_SetSingleObjectTrackingByPoint(
                    g_ObjectTrackTDLHandle, image, &empty_det_meta,
                    set_values[0], set_values[1], &point_hint, g_frame_id,
                    search_type, model_path);
            } else {
                s32Ret = TDL_SetSingleObjectTracking(
                    g_ObjectTrackTDLHandle, image, &empty_det_meta, set_values,
                    set_value_count, g_frame_id, search_type, model_path);
            }
            stPerf.sot_init_frames++;
            stPerf.sot_init_total_us +=
                app_ipcam_Ai_Object_Track_TimeUs() - inference_start_us;
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTracking failed with %#x!\n", s32Ret);
                app_ipcam_Ai_Object_Track_Mode_Set(
                    app_ipcam_Ai_Object_Track_Idle_Mode_Get());
                track_init = CVI_FALSE;
                bRefinedTarget = CVI_FALSE;
                fTargetSizeFactor = 0.0f;
                memset(&selection, 0, sizeof(selection));
            } else {
                track_init = CVI_TRUE;
                sot_unreliable_frames = 0;
                sot_recovery_frames = 0;
                lost_start_us = 0;
                candidate_start_us = 0;
                bSotHardLostObserved = CVI_FALSE;
                bLatestGmcAvailable = CVI_FALSE;
                bGmcPredictionActive = CVI_FALSE;
                bRefinedTarget = CVI_FALSE;
                if (TDL_GetSingleObjectTrackingTargetInfo(
                        g_ObjectTrackTDLHandle, &actual_target,
                        &refined_target) == CVI_SUCCESS &&
                    actual_target.x2 > actual_target.x1 &&
                    actual_target.y2 > actual_target.y1) {
                    g_GmcExclusion[0] = (CVI_S32)actual_target.x1;
                    g_GmcExclusion[1] = (CVI_S32)actual_target.y1;
                    g_GmcExclusion[2] = (CVI_S32)actual_target.x2;
                    g_GmcExclusion[3] = (CVI_S32)actual_target.y2;
                    bRefinedTarget = refined_target ? CVI_TRUE : CVI_FALSE;
                    APP_PROF_LOG_PRINT(LEVEL_INFO,
                        "ObjectTrack actual target=[%d,%d,%d,%d] "
                        "refined=%d\n",
                        g_GmcExclusion[0], g_GmcExclusion[1],
                        g_GmcExclusion[2], g_GmcExclusion[3],
                        bRefinedTarget ? 1 : 0);
                } else if (selection.point_prompt &&
                           search_type == TDL_FASTSAM) {
                    memcpy(g_GmcExclusion, hint_values,
                           sizeof(g_GmcExclusion));
                } else {
                    memcpy(g_GmcExclusion, set_values,
                           sizeof(g_GmcExclusion));
                }
                fTargetSizeFactor =
                    app_ipcam_Ai_Object_Track_TargetSizeFactor(
                        g_GmcExclusion[2] - g_GmcExclusion[0],
                        g_GmcExclusion[3] - g_GmcExclusion[1]);
            }
        } else {
            /*
             * 持续追踪阶段：每帧调用 SingleObjectTracking 并更新单目标框。
             * 当连续 LOST 超时后，根据 DET 开关恢复检测联动或等待外部触发。
             */
            if (bLatestGmcAvailable) {
                CVI_BOOL result_fresh = u64LatestGmcFrameId <= g_frame_id &&
                    g_frame_id - u64LatestGmcFrameId <=
                        OBJECT_TRACK_GMC_RESULT_MAX_AGE_FRAMES;

                if (result_fresh) {
                    CVI_U64 gmc_age = g_frame_id - u64LatestGmcFrameId;
                    CVI_FLOAT ref_w = g_GmcExclusion[2] > g_GmcExclusion[0] ?
                        g_GmcExclusion[2] - g_GmcExclusion[0] : 1.0f;
                    CVI_FLOAT ref_h = g_GmcExclusion[3] > g_GmcExclusion[1] ?
                        g_GmcExclusion[3] - g_GmcExclusion[1] : 1.0f;
                    CVI_FLOAT large_x = fmaxf(4.0f, ref_w * 0.45f);
                    CVI_FLOAT large_y = fmaxf(4.0f, ref_h * 0.45f);
                    CVI_FLOAT target_size_factor =
                        app_ipcam_Ai_Object_Track_TargetSizeFactor(
                            ref_w, ref_h);
                    CVI_BOOL small_target = target_size_factor > 0.0f;
                    CVI_FLOAT local_priority =
                        stLatestGmcResult.local_confidence *
                        (0.5f + 0.5f * target_size_factor);
                    CVI_FLOAT global_priority =
                        stLatestGmcResult.confidence *
                        (1.0f - 0.35f * target_size_factor);
                    CVI_BOOL use_local_motion =
                        small_target && stLatestGmcResult.local_valid &&
                        (!stLatestGmcResult.valid ||
                         local_priority >= global_priority);
                    CVI_BOOL motion_valid = use_local_motion ?
                        stLatestGmcResult.local_valid :
                        stLatestGmcResult.valid;
                    CVI_FLOAT motion_dx = use_local_motion ?
                        stLatestGmcResult.local_dx : stLatestGmcResult.dx;
                    CVI_FLOAT motion_dy = use_local_motion ?
                        stLatestGmcResult.local_dy : stLatestGmcResult.dy;
                    CVI_FLOAT motion_confidence = use_local_motion ?
                        stLatestGmcResult.local_confidence :
                        stLatestGmcResult.confidence;
                    CVI_U32 motion_frame_gap = use_local_motion ?
                        stLatestGmcResult.local_frame_gap :
                        stLatestGmcResult.frame_gap;
                    CVI_U32 motion_span_frames = motion_frame_gap > 0 ?
                        motion_frame_gap : 1;
                    CVI_FLOAT minimum_motion_pixels = 4.0f -
                        2.0f * target_size_factor;
                    CVI_FLOAT recovery_min_confidence =
                        OBJECT_TRACK_GMC_RECOVERY_MIN_CONFIDENCE +
                        (OBJECT_TRACK_GMC_SMALL_TARGET_RECOVERY_MIN_CONFIDENCE -
                         OBJECT_TRACK_GMC_RECOVERY_MIN_CONFIDENCE) *
                            target_size_factor;
                    CVI_FLOAT stable_min_confidence =
                        OBJECT_TRACK_GMC_STABLE_MIN_CONFIDENCE +
                        (OBJECT_TRACK_GMC_SMALL_TARGET_STABLE_MIN_CONFIDENCE -
                         OBJECT_TRACK_GMC_STABLE_MIN_CONFIDENCE) *
                            target_size_factor;
                    CVI_U64 result_max_age =
                        OBJECT_TRACK_GMC_RESULT_MAX_AGE_FRAMES -
                        (CVI_U64)(
                            target_size_factor *
                            (OBJECT_TRACK_GMC_RESULT_MAX_AGE_FRAMES -
                             OBJECT_TRACK_GMC_SMALL_TARGET_MAX_AGE_FRAMES) +
                            0.5f);
                    CVI_BOOL large_motion =
                        fabsf(stLatestGmcResult.dx) >= large_x ||
                        fabsf(stLatestGmcResult.dy) >= large_y;
                    CVI_BOOL small_motion =
                        fabsf(motion_dx) * motion_span_frames >=
                            minimum_motion_pixels ||
                        fabsf(motion_dy) * motion_span_frames >=
                            minimum_motion_pixels;
                    CVI_BOOL recovery_motion = sot_unreliable_frames > 0 &&
                        motion_valid &&
                        (!small_target || small_motion) &&
                        motion_confidence >= recovery_min_confidence;
                    CVI_BOOL stable_motion = small_target ?
                        (motion_valid && small_motion &&
                         motion_confidence >= stable_min_confidence) :
                        (motion_valid && large_motion &&
                         stLatestGmcResult.confidence >=
                             OBJECT_TRACK_GMC_STABLE_MIN_CONFIDENCE);
                    CVI_BOOL result_fresh_for_target =
                        gmc_age <= result_max_age;

                    if (result_fresh_for_target &&
                        (recovery_motion || stable_motion)) {
                        CVI_FLOAT confidence = motion_confidence *
                            (1.0f - 0.05f * gmc_age);

                        if (confidence < 0.05f) {
                            confidence = 0.05f;
                        }

                        if (small_target) {
                            CVI_U64 available_frames =
                                motion_frame_gap + gmc_age;
                            CVI_U32 expansion_frames;
                            CVI_FLOAT expansion_pixels;

                            if (available_frames == 0) {
                                available_frames = 1;
                            }
                            expansion_frames = (CVI_U32)available_frames;
                            if (expansion_frames >
                                OBJECT_TRACK_GMC_SMALL_TARGET_MAX_MOTION_HORIZON) {
                                expansion_frames =
                                    OBJECT_TRACK_GMC_SMALL_TARGET_MAX_MOTION_HORIZON;
                            }
                            expansion_pixels = fmaxf(
                                fabsf(motion_dx), fabsf(motion_dy)) *
                                expansion_frames;
                            if (TDL_SetSingleObjectTrackingSearchExpansionHint(
                                    g_ObjectTrackTDLHandle,
                                    expansion_pixels,
                                    confidence) == CVI_SUCCESS) {
                                stPerf.gmc_expanded++;
                                stPerf.gmc_last_expansion_pixels =
                                    expansion_pixels;
                            }
                        } else {
                            CVI_U32 hint_frames = 1;
                            CVI_FLOAT hint_dx;
                            CVI_FLOAT hint_dy;

                            if (sot_unreliable_frames > 0) {
                                if (!bGmcPredictionActive) {
                                    CVI_U64 available_frames =
                                        stLatestGmcResult.frame_gap + gmc_age;
                                    CVI_U64 required_frames =
                                        (CVI_U64)sot_unreliable_frames + 1;

                                    hint_frames = (CVI_U32)(
                                        available_frames < required_frames ?
                                        available_frames : required_frames);
                                    if (hint_frames == 0) {
                                        hint_frames = 1;
                                    }
                                    bGmcPredictionActive = CVI_TRUE;
                                } else {
                                    hint_frames =
                                        stLatestGmcResult.frame_gap > 0 ?
                                        stLatestGmcResult.frame_gap : 1;
                                }
                            }
                            hint_dx = stLatestGmcResult.dx * hint_frames;
                            hint_dy = stLatestGmcResult.dy * hint_frames;
                            if (TDL_SetSingleObjectTrackingSearchMotionHint(
                                    g_ObjectTrackTDLHandle, hint_dx, hint_dy,
                                    confidence) == CVI_SUCCESS) {
                                stPerf.gmc_applied++;
                                stPerf.gmc_last_applied_dx = hint_dx;
                                stPerf.gmc_last_applied_dy = hint_dy;
                                stPerf.gmc_last_applied_frames = hint_frames;
                            }
                        }
                    }
                }
                bLatestGmcAvailable = CVI_FALSE;
            }
            inference_start_us = app_ipcam_Ai_Object_Track_TimeUs();
            s32Ret = TDL_SingleObjectTracking(g_ObjectTrackTDLHandle, image, &track_meta, g_frame_id);
            if (s32Ret != 0) {
               APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SingleObjectTracking failed with %#x!\n", s32Ret);
               app_ipcam_Ai_Object_Track_Mode_Set(
                   app_ipcam_Ai_Object_Track_Idle_Mode_Get());
               track_init = CVI_FALSE;
               sot_unreliable_frames = 0;
               sot_recovery_frames = 0;
               lost_start_us = 0;
               candidate_start_us = 0;
               bSotHardLostObserved = CVI_FALSE;
               bLatestGmcAvailable = CVI_FALSE;
               bGmcPredictionActive = CVI_FALSE;
               bRefinedTarget = CVI_FALSE;
               fTargetSizeFactor = 0.0f;
               memset(&selection, 0, sizeof(selection));
               app_ipcam_Ai_Object_Track_Gmc_Reset();
               goto loop_cleanup;
            }
            stPerf.sot_frames++;
            stPerf.sot_total_us +=
                app_ipcam_Ai_Object_Track_TimeUs() - inference_start_us;
            result_start_us = app_ipcam_Ai_Object_Track_TimeUs();

            if (track_meta.info != NULL &&
                track_meta.info[0].state == TDL_TRACK_STATE_TRACKED) {
                float score = track_meta.info[0].score;
                if (stPerf.sot_observed_frames == 0) {
                    stPerf.sot_score_min = score;
                    stPerf.sot_score_max = score;
                } else {
                    if (score < stPerf.sot_score_min) {
                        stPerf.sot_score_min = score;
                    }
                    if (score > stPerf.sot_score_max) {
                        stPerf.sot_score_max = score;
                    }
                }
                stPerf.sot_observed_frames++;
                stPerf.sot_score_total += score;
            } else if (track_meta.info != NULL &&
                       track_meta.info[0].state ==
                           TDL_TRACK_STATE_FOLLOWING) {
                stPerf.sot_following_frames++;
            } else if (track_meta.info != NULL &&
                       track_meta.info[0].state ==
                           TDL_TRACK_STATE_CANDIDATE) {
                stPerf.sot_candidate_frames++;
            } else if (track_meta.info != NULL &&
                       track_meta.info[0].state ==
                           TDL_TRACK_STATE_PREDICTED) {
                stPerf.sot_predicted_frames++;
            } else {
                stPerf.sot_lost_frames++;
            }

            {
                CVI_BOOL reliable_candidate = track_meta.info != NULL &&
                    track_meta.info[0].state == TDL_TRACK_STATE_TRACKED &&
                    track_meta.info[0].score >=
                        g_pstObjTrackCfg->sot_min_observed_score;
                CVI_BOOL following_candidate = track_meta.info != NULL &&
                    track_meta.info[0].state ==
                        TDL_TRACK_STATE_FOLLOWING;
                CVI_BOOL observed_candidate = track_meta.info != NULL &&
                    track_meta.info[0].state ==
                        TDL_TRACK_STATE_CANDIDATE;
                CVI_BOOL reliable = reliable_candidate || following_candidate;
                CVI_BOOL bReturnToIdle = CVI_FALSE;
                CVI_BOOL bCandidateTimeout = CVI_FALSE;

                if (lost_start_us != 0 && reliable_candidate &&
                    bSotHardLostObserved) {
                    /* Avoid ending a search window on a one-frame false recovery. */
                    if (sot_recovery_frames < UINT32_MAX) {
                        sot_recovery_frames++;
                    }
                    reliable = sot_recovery_frames >=
                        OBJECT_TRACK_RECOVERY_CONFIRM_FRAMES;
                } else if (!reliable_candidate) {
                    sot_recovery_frames = 0;
                }

                if (reliable) {
                    sot_unreliable_frames = 0;
                    sot_recovery_frames = 0;
                    lost_start_us = 0;
                    candidate_start_us = 0;
                    bSotHardLostObserved = CVI_FALSE;
                    bGmcPredictionActive = CVI_FALSE;
                    g_GmcExclusion[0] = (CVI_S32)track_meta.info[0].bbox.x1;
                    g_GmcExclusion[1] = (CVI_S32)track_meta.info[0].bbox.y1;
                    g_GmcExclusion[2] = (CVI_S32)track_meta.info[0].bbox.x2;
                    g_GmcExclusion[3] = (CVI_S32)track_meta.info[0].bbox.y2;
                    fTargetSizeFactor =
                        app_ipcam_Ai_Object_Track_TargetSizeFactor(
                            track_meta.info[0].bbox.x2 -
                                track_meta.info[0].bbox.x1,
                            track_meta.info[0].bbox.y2 -
                                track_meta.info[0].bbox.y1);
                    {
                        CVI_FLOAT fRawX1;
                        CVI_FLOAT fRawY1;
                        CVI_FLOAT fRawX2;
                        CVI_FLOAT fRawY2;
                        CVI_BOOL bDrawReady =
                            app_ipcam_Ai_Object_Track_Draw_Update(
                                &track_meta.info[0],
                                &fRawX1, &fRawY1, &fRawX2, &fRawY2);
#ifdef OSDC_SUPPORT
                        if (bDrawReady) {
                            app_ipcam_Ai_Object_Track_Osd_Publish(
                                &stOsdPredictor,
                                &track_meta.info[0], &stFrame,
                                fRawX1, fRawY1, fRawX2, fRawY2);
                        }
#else
                        (void)bDrawReady;
#endif
                    }
                } else if (observed_candidate) {
                    uint64_t now_us =
                        app_ipcam_Ai_Object_Track_TimeUs();
                    CVI_FLOAT fRawX1;
                    CVI_FLOAT fRawY1;
                    CVI_FLOAT fRawX2;
                    CVI_FLOAT fRawY2;
                    CVI_BOOL bDrawReady;

                    sot_unreliable_frames = 0;
                    sot_recovery_frames = 0;
                    lost_start_us = 0;
                    bSotHardLostObserved = CVI_FALSE;
                    bLatestGmcAvailable = CVI_FALSE;
                    bGmcPredictionActive = CVI_FALSE;
                    g_GmcExclusion[0] =
                        (CVI_S32)track_meta.info[0].bbox.x1;
                    g_GmcExclusion[1] =
                        (CVI_S32)track_meta.info[0].bbox.y1;
                    g_GmcExclusion[2] =
                        (CVI_S32)track_meta.info[0].bbox.x2;
                    g_GmcExclusion[3] =
                        (CVI_S32)track_meta.info[0].bbox.y2;
                    fTargetSizeFactor =
                        app_ipcam_Ai_Object_Track_TargetSizeFactor(
                            track_meta.info[0].bbox.x2 -
                                track_meta.info[0].bbox.x1,
                            track_meta.info[0].bbox.y2 -
                                track_meta.info[0].bbox.y1);
                    bDrawReady = app_ipcam_Ai_Object_Track_Draw_Update(
                        &track_meta.info[0],
                        &fRawX1, &fRawY1, &fRawX2, &fRawY2);
#ifdef OSDC_SUPPORT
                    if (bDrawReady) {
                        app_ipcam_Ai_Object_Track_Osd_Publish(
                            &stOsdPredictor,
                            &track_meta.info[0], &stFrame,
                            fRawX1, fRawY1, fRawX2, fRawY2);
                    }
#else
                    (void)bDrawReady;
#endif
                    if (candidate_start_us == 0) {
                        candidate_start_us = now_us;
                    } else if (now_us >= candidate_start_us &&
                               now_us - candidate_start_us >=
                                   OBJECT_TRACK_CANDIDATE_TIMEOUT_US) {
                        bReturnToIdle = CVI_TRUE;
                        bCandidateTimeout = CVI_TRUE;
                    }
                } else {
                    uint64_t now_us = app_ipcam_Ai_Object_Track_TimeUs();
                    CVI_BOOL bPredicted = track_meta.info != NULL &&
                        track_meta.info[0].state ==
                            TDL_TRACK_STATE_PREDICTED;

                    if (track_meta.info != NULL &&
                        track_meta.info[0].state == TDL_TRACK_STATE_TRACKED &&
                        track_meta.info[0].score <
                            g_pstObjTrackCfg->sot_min_observed_score) {
                        APP_PROF_LOG_PRINT(LEVEL_DEBUG,
                            "track score %.3f lower than threshold %.3f\n",
                            track_meta.info[0].score,
                            g_pstObjTrackCfg->sot_min_observed_score);
                    }
                    if (sot_unreliable_frames < UINT32_MAX) {
                        sot_unreliable_frames++;
                    }
                    if (bPredicted && fTargetSizeFactor > 0.0f &&
                        sot_unreliable_frames <=
                            OBJECT_TRACK_PREDICTED_DISPLAY_FRAMES) {
                        CVI_FLOAT fRawX1;
                        CVI_FLOAT fRawY1;
                        CVI_FLOAT fRawX2;
                        CVI_FLOAT fRawY2;
                        CVI_BOOL bDrawReady;

                        g_GmcExclusion[0] =
                            (CVI_S32)track_meta.info[0].bbox.x1;
                        g_GmcExclusion[1] =
                            (CVI_S32)track_meta.info[0].bbox.y1;
                        g_GmcExclusion[2] =
                            (CVI_S32)track_meta.info[0].bbox.x2;
                        g_GmcExclusion[3] =
                            (CVI_S32)track_meta.info[0].bbox.y2;
                        fTargetSizeFactor =
                            app_ipcam_Ai_Object_Track_TargetSizeFactor(
                                track_meta.info[0].bbox.x2 -
                                    track_meta.info[0].bbox.x1,
                                track_meta.info[0].bbox.y2 -
                                    track_meta.info[0].bbox.y1);
                        bDrawReady = app_ipcam_Ai_Object_Track_Draw_Update(
                            &track_meta.info[0],
                            &fRawX1, &fRawY1, &fRawX2, &fRawY2);
#ifdef OSDC_SUPPORT
                        if (bDrawReady) {
                            app_ipcam_Ai_Object_Track_Osd_Publish(
                                &stOsdPredictor,
                                &track_meta.info[0], &stFrame,
                                fRawX1, fRawY1, fRawX2, fRawY2);
                        }
#else
                        (void)bDrawReady;
#endif
                    }
                    if (!bPredicted) {
                        bSotHardLostObserved = CVI_TRUE;
                    }
                    if (sot_unreliable_frames >=
                        app_ipcam_Ai_Object_Track_OsdHoldFrames(
                            fTargetSizeFactor)) {
                        app_ipcam_Ai_Object_Track_ObjDraw_Clear();
                    }
                    if (lost_start_us == 0) {
                        lost_start_us = now_us;
                    } else if (now_us >= lost_start_us &&
                               now_us - lost_start_us >=
                                   OBJECT_TRACK_LOST_TIMEOUT_US) {
                        bReturnToIdle = CVI_TRUE;
                    }
                }
                if (bReturnToIdle) {
                    if (bCandidateTimeout) {
                        APP_PROF_LOG_PRINT(LEVEL_WARN,
                            "ObjectTrack candidate unconfirmed for 1.2 "
                            "seconds, returning to idle\n");
                    } else {
                        APP_PROF_LOG_PRINT(LEVEL_WARN,
                            "ObjectTrack target lost for 1.2 seconds, "
                            "returning to idle\n");
                    }
                    app_ipcam_Ai_Object_Track_Mode_Set(
                        app_ipcam_Ai_Object_Track_Idle_Mode_Get());
                    track_init = CVI_FALSE;
                    sot_unreliable_frames = 0;
                    sot_recovery_frames = 0;
                    lost_start_us = 0;
                    candidate_start_us = 0;
                    bSotHardLostObserved = CVI_FALSE;
                    bLatestGmcAvailable = CVI_FALSE;
                    bGmcPredictionActive = CVI_FALSE;
                    bRefinedTarget = CVI_FALSE;
                    fTargetSizeFactor = 0.0f;
                    memset(&selection, 0, sizeof(selection));
                    app_ipcam_Ai_Object_Track_Gmc_Reset();
                }
            }
            stPerf.result_total_us +=
                app_ipcam_Ai_Object_Track_TimeUs() - result_start_us;
        }

loop_cleanup:
        cleanup_start_us = app_ipcam_Ai_Object_Track_TimeUs();
        stPerf.sot_target_width = g_GmcExclusion[2] > g_GmcExclusion[0] ?
            g_GmcExclusion[2] - g_GmcExclusion[0] : 0.0f;
        stPerf.sot_target_height = g_GmcExclusion[3] > g_GmcExclusion[1] ?
            g_GmcExclusion[3] - g_GmcExclusion[1] : 0.0f;
        stPerf.sot_target_size_factor =
            app_ipcam_Ai_Object_Track_TargetSizeFactor(
                stPerf.sot_target_width, stPerf.sot_target_height);
        stPerf.sot_small_target = app_ipcam_Ai_Object_Track_IsSmallTarget(
            stPerf.sot_target_width, stPerf.sot_target_height);
        stPerf.sot_refined_target = bRefinedTarget;
        stPerf.gmc_submit_interval =
            app_ipcam_Ai_Object_Track_GmcInterval(
                fTargetSizeFactor,
                sot_unreliable_frames > 0 ? CVI_TRUE : CVI_FALSE);
        if (track_meta.info != NULL) {
            TDL_ReleaseTrackMeta(&track_meta);
            memset(&track_meta, 0, sizeof(track_meta));
        }
        if (image != NULL) {
            TDL_DestroyImage(image);
        }
        if (frame_acquired && track_init &&
            g_pstObjTrackCfg->sot_gmc_enable &&
            app_ipcam_Ai_Object_Track_Mode_Get() == TRACKING) {
            app_ipcam_Ai_Object_Track_GmcAsync_Submit(
                &stFrame, g_frame_id,
                stPerf.gmc_submit_interval,
                g_GmcExclusion,
                sot_unreliable_frames > 0 ? CVI_TRUE : CVI_FALSE);
        }
        if (frame_acquired) {
            s32Ret = CVI_VPSS_ReleaseChnFrame(FrameVpssGrp, FrameVpssChn, &stFrame);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                                   "Grp(%d)-Chn(%d) release frame failed with %#x\n",
                                   FrameVpssGrp, FrameVpssChn, s32Ret);
            }
        }
        stPerf.cleanup_total_us +=
            app_ipcam_Ai_Object_Track_TimeUs() - cleanup_start_us;
        app_ipcam_Ai_Object_Track_Perf_Write(
            &stPerf, app_ipcam_Ai_Object_Track_TimeUs());
    }

    app_ipcam_Ai_Object_Track_Gmc_Reset();
    app_ipcam_Ai_Object_Track_Mode_Set(
        app_ipcam_Ai_Object_Track_Idle_Mode_Get());
    if (app_ipcam_Ai_Object_Track_Pipeline_Set(
            app_ipcam_Ai_Object_Track_Mode_Get()) != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "restore ObjectTrack idle pipeline failed\n");
    }
    TDL_ReleaseObjectMeta(&cur_det_meta);
    unlink(TRACK_FPS_STATUS_TMP_PATH);
    pthread_exit(NULL);

    return NULL;
}

static CVI_S32 app_ipcam_Ai_Object_Track_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;
    if (g_pstObjTrackCfg->u32GrpWidth == 0 ||
        g_pstObjTrackCfg->u32GrpHeight == 0 ||
        g_pstObjTrackCfg->u32SotGrpWidth == 0 ||
        g_pstObjTrackCfg->u32SotGrpHeight == 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "invalid ObjectTrack frame size: selection=%ux%u SOT=%ux%u\n",
                           g_pstObjTrackCfg->u32GrpWidth,
                           g_pstObjTrackCfg->u32GrpHeight,
                           g_pstObjTrackCfg->u32SotGrpWidth,
                           g_pstObjTrackCfg->u32SotGrpHeight);
        return CVI_FAILURE;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "ObjectTrack pipeline: selection Grp(%d)-Chn(%d) %ux%u, "
                       "SOT Grp(%d)-Chn(%d) %ux%u\n",
                       g_pstObjTrackCfg->VpssGrp,
                       g_pstObjTrackCfg->VpssChn,
                       g_pstObjTrackCfg->u32GrpWidth,
                       g_pstObjTrackCfg->u32GrpHeight,
                       g_pstObjTrackCfg->SotVpssGrp,
                       g_pstObjTrackCfg->SotVpssChn,
                       g_pstObjTrackCfg->u32SotGrpWidth,
                       g_pstObjTrackCfg->u32SotGrpHeight);

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "wait all VENC streams stable before loading ObjectTrack models\n");
    for (CVI_U32 wait_ms = 0;
         !app_ipcam_Venc_All_Stream_Ready() && wait_ms < 8000;
         wait_ms += 10) {
        usleep(10 * 1000);
    }
    if (!app_ipcam_Venc_All_Stream_Ready()) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
                           "VENC streams not stable after 8000 ms, load ObjectTrack models anyway\n");
    }

    if (g_ObjectTrackTDLHandle == NULL)
    {
        g_ObjectTrackTDLHandle = TDL_CreateHandle(0);
        if (g_ObjectTrackTDLHandle == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle failed\n");
            return CVI_FAILURE;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDLHandle has created\n");
        return CVI_FAILURE;
    }

    s32Ret = TDL_OpenModel(g_ObjectTrackTDLHandle,
                           g_pstObjTrackCfg->model_id_sot,
                           g_pstObjTrackCfg->model_path_sot,
                           g_pstObjTrackCfg->model_path_cfg, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel SOT failed with %#x!\n", s32Ret);
        goto init_failed;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "TDL_OpenModel SOT success !\n");
    }

    if (g_pstObjTrackCfg->model_path_sam[0] != '\0') {
        APP_PROF_LOG_PRINT(LEVEL_INFO,
                           "prepare FastSAM target search model before frame loop\n");
        s32Ret = TDL_PrepareSingleObjectTrackingTargetSearch(
            g_ObjectTrackTDLHandle, TDL_FASTSAM,
            g_pstObjTrackCfg->model_path_sam);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "prepare FastSAM target search failed with %#x!\n",
                               s32Ret);
            if (g_pstObjTrackCfg->search_type == TDL_FASTSAM) {
                goto init_failed;
            }
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO,
                               "prepare FastSAM target search success\n");
        }
    }

    s32Ret = TDL_SetSingleObjectTrackingUseKalman(g_ObjectTrackTDLHandle,
                                                  g_pstObjTrackCfg->use_kalman);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTrackingUseKalman failed with %#x!\n", s32Ret);
        goto init_failed;
    }

    s32Ret = TDL_SetSingleObjectTrackingThreshold(
        g_ObjectTrackTDLHandle,
        g_pstObjTrackCfg->sot_min_observed_score);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "TDL_SetSingleObjectTrackingThreshold failed with %#x!\n",
            s32Ret);
        goto init_failed;
    }

    // s32Ret = TDL_SetSotModelThreshold(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->threshold_occluded, g_pstObjTrackCfg->threshold_reappear);
    // if (s32Ret != CVI_SUCCESS)
    // {
    //     APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSotModelThreshold failed with %#x!\n", s32Ret);
    //     return s32Ret;
    // }

    if (g_stObjDraw.info == NULL) {
        g_stObjDraw.info = malloc(MAX_DET_NUM * sizeof(TDLObjectInfo));
        if (g_stObjDraw.info == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc g_stObjDraw.info failed\n");
            s32Ret = CVI_FAILURE;
            goto init_failed;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> done \n");

    return CVI_SUCCESS;

init_failed:
    TDL_DestroyHandle(g_ObjectTrackTDLHandle);
    g_ObjectTrackTDLHandle = NULL;
    g_bSharedPipelinePrepared = CVI_FALSE;
    g_bPipelineInitialized = CVI_FALSE;
    return s32Ret;
}

int app_ipcam_Ai_Object_Track_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    app_ipcam_Ai_Object_Track_Mode_Set(
        app_ipcam_Ai_Object_Track_Idle_Mode_Get());
    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_FALSE);
    pthread_join(g_ObjectTrackHandle, NULL);
    g_ObjectTrackHandle = 0;
    app_ipcam_Ai_Object_Track_GmcAsync_Stop();

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        if (g_stObjDraw.info != NULL) {
            free(g_stObjDraw.info);
            g_stObjDraw.info = NULL;
        }
        g_stObjDraw.size = 0;
    }

    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_sot);

    s32Ret = TDL_DestroyHandle(g_ObjectTrackTDLHandle);

    if(s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "enter TDL_DestroyHandle fail \n");
        return s32Ret;
    }
    g_ObjectTrackTDLHandle = NULL;
    g_bSharedPipelinePrepared = CVI_FALSE;
    g_bPipelineInitialized = CVI_FALSE;

    return s32Ret;
}

int app_ipcam_Ai_Object_Track_Start(void){
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstObjTrackCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bObjectTrackRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI ObjectTrack has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Object_Track_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Object_Track_Proc_Init failed!\n");
        return s32Ret;
    }

    if (g_pstObjTrackCfg->sot_gmc_enable &&
        app_ipcam_Ai_Object_Track_GmcAsync_Start() != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "ObjectTrack GMC worker start failed, continue without GMC\n");
        g_pstObjTrackCfg->sot_gmc_enable = CVI_FALSE;
    } else if (g_pstObjTrackCfg->sot_gmc_enable) {
        CVI_U32 effective_interval =
            g_pstObjTrackCfg->sot_gmc_interval < OBJECT_TRACK_GMC_MIN_INTERVAL ?
            OBJECT_TRACK_GMC_MIN_INTERVAL :
            g_pstObjTrackCfg->sot_gmc_interval;

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "ObjectTrack GMC worker active: interval=%u grid=%ux%u\n",
            effective_interval, APP_OBJECT_TRACK_GMC_GRID_WIDTH,
            APP_OBJECT_TRACK_GMC_GRID_HEIGHT);
    }

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_TRUE);
    s32Ret = pthread_create(&g_ObjectTrackHandle, NULL, Thread_Object_Track_Proc, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI ObjectTrack_pthread_create failed!\n");
        app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_FALSE);
        app_ipcam_Ai_Object_Track_GmcAsync_Stop();
        return s32Ret;
    }

    return s32Ret;
}
