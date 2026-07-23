#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"
#include "cvi_sys.h"
#ifdef OSDC_SUPPORT
#include "app_ipcam_osd.h"
#endif

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
SMT_MUTEXAUTOLOCK_INIT(g_PDMutex);
static pthread_mutex_t g_PDStatusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_PDStatusCond = PTHREAD_COND_INITIALIZER;
static CVI_BOOL g_bPDFrameActive = CVI_FALSE;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define APP_PD_MAX_DRAW_OBJECTS 100
#define APP_PD_FRAME_SOURCE_TIMEOUT_MS 100
#define APP_PD_DISPLAY_SLOT_SIZE 128
#define APP_PD_DISPLAY_HOLD_RESULTS 5
#define APP_PD_DISPLAY_HOLD_US 220000
#define APP_PD_DISPLAY_CANDIDATE_DELTA 0.15f
#define APP_PD_DISPLAY_CANDIDATE_MIN 0.10f
#define APP_PD_DISPLAY_EVIDENCE_MAX 6
#define APP_PD_DISPLAY_PRIMARY_EVIDENCE 2
#define APP_PD_DISPLAY_CANDIDATE_EVIDENCE 1
#define APP_PD_DISPLAY_MISS_EVIDENCE 1
#define APP_PD_DISPLAY_VELOCITY_MIN_OBSERVATIONS 3
#define APP_PD_DISPLAY_CENTER_ALPHA 0.72f
#define APP_PD_DISPLAY_VELOCITY_BETA 0.18f
#define APP_PD_DISPLAY_SIZE_ALPHA 0.65f
#define APP_PD_DISPLAY_CANDIDATE_CENTER_ALPHA 0.35f
#define APP_PD_DISPLAY_CANDIDATE_VELOCITY_BETA 0.08f
#define APP_PD_DISPLAY_CANDIDATE_SIZE_ALPHA 0.30f
#define APP_PD_DISPLAY_CLASS_RESCUE_IOU 0.55f
#define APP_PD_DISPLAY_CLASS_CONFIRM_HITS 2
#define APP_PD_DISPLAY_DUPLICATE_IOU 0.80f
#define APP_PD_DISPLAY_MATURE_GATE_RATIO 0.10f
#define APP_PD_DISPLAY_MISSING_GATE_GROWTH 0.35f
#define APP_PD_DISPLAY_GATE_MAX_RATIO 0.22f
#define APP_PD_DISPLAY_RECOVERY_GATE_SCALE 1.50f
#define APP_PD_DISPLAY_RECOVERY_MIN_SIZE_RATIO 0.40f
#define APP_PD_DEFAULT_FRAME_PERIOD_US 40000
#define APP_PD_OSD_ESTIMATED_APPLY_US 15000
#define APP_PD_OSD_PREDICT_MAX_US 120000
#define APP_PD_OSD_MAX_SHIFT_BOX_RATIO 1.5f
#define APP_PD_DISPLAY_MIN_IOU 0.05f
#define APP_PD_DISPLAY_CENTER_GATE_RATIO 0.08f
#define APP_PD_IMAGE_FOLLOW_ENABLE 0
#define APP_PD_OSD_MOTION_PREDICT_ENABLE 0
#define APP_PD_FOLLOW_WIDTH 160
#define APP_PD_FOLLOW_HEIGHT 96
#define APP_PD_FOLLOW_MAX_OBJECTS 16
#define APP_PD_FOLLOW_SEARCH_RADIUS 5
#define APP_PD_FOLLOW_SEARCH_SIZE (APP_PD_FOLLOW_SEARCH_RADIUS * 2 + 1)
#define APP_PD_FOLLOW_SAMPLE_GRID 5
#define APP_PD_FOLLOW_MIN_TEXTURE 20
#define APP_PD_FOLLOW_MAX_COST 28
#define APP_PD_FOLLOW_STILL_RADIUS 1
#define APP_PD_FOLLOW_UNIQUENESS_RADIUS 1
#define APP_PD_FOLLOW_MIN_COST_GAIN 4
#define APP_PD_FOLLOW_MIN_COST_MARGIN 2
#define APP_PD_FOLLOW_MIN_IMPROVEMENT 0.25f
#define APP_PD_FOLLOW_MIN_BOX_SIZE 2.0f
#define APP_PD_FOLLOW_MIN_RESIDUAL_LIMIT 2.0f
#define APP_PD_FOLLOW_MAX_RESIDUAL_BOX_RATIO 1.5f

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct {
    CVI_U64 wait_us;
    CVI_U64 drain_us;
    CVI_U64 wrap_us;
    CVI_U64 inference_us;
    CVI_U64 release_us;
    CVI_U64 publish_us;
    CVI_U64 follow_us;
    CVI_U64 loop_us;
    CVI_U64 source_age_us;
    CVI_U32 dropped_frames;
    CVI_U32 draw_objects;
    CVI_U32 held_objects;
    CVI_U32 maintained_objects;
    CVI_U32 detected_objects;
    CVI_U32 association_new_objects;
    CVI_U32 association_motion_rescues;
    CVI_U32 association_rescues;
    CVI_U32 association_class_rejects;
    CVI_U32 association_gate_rejects;
    CVI_U32 association_merges;
    CVI_U32 followed_objects;
    CVI_U32 follow_moved_objects;
    CVI_U32 follow_rejects;
    CVI_U64 presentation_lead_us;
    CVI_U32 sequence;
    CVI_U32 time_ref;
    CVI_U64 pts;
} APP_PD_PROFILE_S;

typedef struct {
    CVI_BOOL valid;
    CVI_U32 source_width;
    CVI_U32 source_height;
    CVI_U64 pts;
    CVI_U8 data[APP_PD_FOLLOW_WIDTH * APP_PD_FOLLOW_HEIGHT];
} APP_PD_FOLLOW_FRAME_S;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t thread;
    CVI_BOOL running;
    CVI_BOOL paused;
    CVI_BOOL producer_active;
    CVI_BOOL frame_valid;
    VIDEO_FRAME_INFO_S frame;
    CVI_U64 frame_capture_time_us;
    CVI_U32 dropped_frames;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
} APP_PD_FRAME_SOURCE_S;

typedef struct {
    CVI_BOOL used;
    CVI_BOOL velocity_valid;
    CVI_U32 observations;
    CVI_S32 pending_class_id;
    CVI_U32 pending_class_hits;
    CVI_S32 evidence;
    CVI_U64 track_id;
    CVI_U64 last_pts;
    CVI_U32 last_observed_result;
    CVI_FLOAT center_x;
    CVI_FLOAT center_y;
    CVI_FLOAT width;
    CVI_FLOAT height;
    CVI_FLOAT velocity_x_per_us;
    CVI_FLOAT velocity_y_per_us;
    CVI_FLOAT visual_offset_x;
    CVI_FLOAT visual_offset_y;
    TDLObjectInfo object;
} APP_PD_DISPLAY_SLOT_S;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
#if 0
static APP_PARAM_AI_PD_CFG_S g_stPdCfg = {
    .bEnable = 0,
    .VpssGrp = 0,
    .VpssChn = 1,
    .u32GrpWidth = 640,
    .u32GrpHeight = 384,
    .threshold = 0.7,
    .model_id = TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON,
    .model_path = "/mnt/sd/yolov8n_det_monitor_person_256_448_INT8_mars3.bmodel",
};
#else
static APP_PARAM_AI_PD_CFG_S g_stPdCfg;
#endif

static APP_PARAM_AI_PD_CFG_S *g_pstPdCfg = &g_stPdCfg;

static CVI_U32 g_PDFps;
static CVI_U32 g_IntrusionNum;
static CVI_U32 g_PDProc;
static float g_PDScaleX, g_PDScaleY;
static volatile bool g_bPDRunning = CVI_FALSE;
static volatile bool g_bPDPause = CVI_FALSE;
static pthread_t g_PDThreadHandle;
static TDLHandle g_PDAiHandle = NULL;
static TDLObject g_stPDObjDraw;
static APP_AI_RESULT_FRAME_INFO_S g_stPDResultFrame;
static pfpInferenceFunc g_pfpPDInference;
static APP_PD_FRAME_SOURCE_S g_stPDFrameSource = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

static CVI_VOID app_ipcam_Ai_PD_FrameSource_Pause(CVI_BOOL bPause);

APP_PARAM_AI_PD_CFG_S *app_ipcam_Ai_PD_Param_Get(void)
{
    return g_pstPdCfg;
}

static CVI_FLOAT app_ipcam_Ai_PD_CandidateThreshold(CVI_FLOAT fThreshold)
{
    return fminf(fThreshold,
        fmaxf(APP_PD_DISPLAY_CANDIDATE_MIN,
            fThreshold - APP_PD_DISPLAY_CANDIDATE_DELTA));
}

#if APP_PD_OSD_MOTION_PREDICT_ENABLE
static CVI_U64 app_ipcam_Ai_PD_PresentationLeadUs(
    CVI_U64 u64FramePeriodUs, CVI_U64 u64PipelineUs)
{
    CVI_U64 u64LeadUs;

    if (u64FramePeriodUs == 0) {
        u64FramePeriodUs = APP_PD_DEFAULT_FRAME_PERIOD_US;
    }
    u64LeadUs = ((u64PipelineUs + u64FramePeriodUs - 1) /
        u64FramePeriodUs) * u64FramePeriodUs;
    if (u64LeadUs > APP_PD_OSD_PREDICT_MAX_US) {
        u64LeadUs = APP_PD_OSD_PREDICT_MAX_US;
    }
    return u64LeadUs;
}
#endif

CVI_VOID app_ipcam_Ai_PD_ProcStatus_Set(CVI_BOOL flag)
{
    g_bPDRunning = flag;
}

CVI_BOOL app_ipcam_Ai_PD_ProcStatus_Get(void)
{
    return g_bPDRunning;
}

CVI_VOID app_ipcam_Ai_PD_Pause_Set(CVI_BOOL flag)
{
    if (!flag) {
        app_ipcam_Ai_PD_FrameSource_Pause(CVI_FALSE);
    }
    pthread_mutex_lock(&g_PDStatusMutex);
    g_bPDPause = flag;
    while (flag && g_bPDFrameActive) {
        pthread_cond_wait(&g_PDStatusCond, &g_PDStatusMutex);
    }
    pthread_cond_broadcast(&g_PDStatusCond);
    pthread_mutex_unlock(&g_PDStatusMutex);
    if (flag) {
        app_ipcam_Ai_PD_FrameSource_Pause(CVI_TRUE);
    }
}

CVI_BOOL app_ipcam_Ai_PD_Pause_Get(void)
{
    return g_bPDPause;
}

CVI_U32 app_ipcam_Ai_PD_ProcIntrusion_Num_Get(void)
{
    return g_IntrusionNum;
}

CVI_U32 app_ipcam_Ai_PD_ProcFps_Get(void)
{
    return g_PDFps;
}

CVI_S32 app_ipcam_Ai_PD_ProcTime_Get(void)
{
    return g_PDProc;
}

static CVI_U64 app_ipcam_Ai_PD_TimeUs(CVI_VOID)
{
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);
    return (CVI_U64)time.tv_sec * 1000000ULL +
        (CVI_U64)time.tv_nsec / 1000ULL;
}

#if APP_PD_IMAGE_FOLLOW_ENABLE
static CVI_S32 app_ipcam_Ai_PD_FollowFrame_Capture(
    const VIDEO_FRAME_INFO_S *pstFrame, CVI_U64 u64Pts,
    APP_PD_FOLLOW_FRAME_S *pstFollowFrame)
{
    const VIDEO_FRAME_S *pstVideo;
    CVI_VOID *pMappedAddress = NULL;
    CVI_U8 *pu8Luma;
    CVI_U32 u32LumaLength;

    if (pstFrame == NULL || pstFollowFrame == NULL) {
        return CVI_FAILURE;
    }
    pstVideo = &pstFrame->stVFrame;
    if (pstVideo->enPixelFormat != PIXEL_FORMAT_NV12 ||
        pstVideo->u64PhyAddr[0] == 0 || pstVideo->u32Stride[0] == 0 ||
        pstVideo->u32Width == 0 || pstVideo->u32Height == 0) {
        pstFollowFrame->valid = CVI_FALSE;
        return CVI_FAILURE;
    }

    u32LumaLength = pstVideo->u32Stride[0] * pstVideo->u32Height;
    pu8Luma = pstVideo->pu8VirAddr[0];
    if (pu8Luma == NULL) {
        pMappedAddress = CVI_SYS_MmapCache(
            pstVideo->u64PhyAddr[0], u32LumaLength);
        if (pMappedAddress == NULL) {
            pstFollowFrame->valid = CVI_FALSE;
            return CVI_FAILURE;
        }
        pu8Luma = pMappedAddress;
    }
    if (CVI_SYS_IonInvalidateCache(
            pstVideo->u64PhyAddr[0], pu8Luma,
            u32LumaLength) != CVI_SUCCESS) {
        if (pMappedAddress != NULL) {
            CVI_SYS_Munmap(pMappedAddress, u32LumaLength);
        }
        pstFollowFrame->valid = CVI_FALSE;
        return CVI_FAILURE;
    }

    for (CVI_U32 y = 0; y < APP_PD_FOLLOW_HEIGHT; y++) {
        CVI_U32 u32SourceY = ((y * 2 + 1) * pstVideo->u32Height) /
            (APP_PD_FOLLOW_HEIGHT * 2);
        const CVI_U8 *pu8Row = pu8Luma +
            u32SourceY * pstVideo->u32Stride[0];

        for (CVI_U32 x = 0; x < APP_PD_FOLLOW_WIDTH; x++) {
            CVI_U32 u32SourceX = ((x * 2 + 1) * pstVideo->u32Width) /
                (APP_PD_FOLLOW_WIDTH * 2);

            pstFollowFrame->data[y * APP_PD_FOLLOW_WIDTH + x] =
                pu8Row[u32SourceX];
        }
    }
    if (pMappedAddress != NULL) {
        CVI_SYS_Munmap(pMappedAddress, u32LumaLength);
    }
    pstFollowFrame->source_width = pstVideo->u32Width;
    pstFollowFrame->source_height = pstVideo->u32Height;
    pstFollowFrame->pts = u64Pts;
    pstFollowFrame->valid = CVI_TRUE;
    return CVI_SUCCESS;
}

static CVI_VOID app_ipcam_Ai_PD_DisplaySlot_CenterAtPts(
    const APP_PD_DISPLAY_SLOT_S *pstSlot, CVI_U64 u64Pts,
    CVI_FLOAT *pfCenterX, CVI_FLOAT *pfCenterY)
{
    CVI_U64 u64GapUs = 0;

    *pfCenterX = pstSlot->center_x + pstSlot->visual_offset_x;
    *pfCenterY = pstSlot->center_y + pstSlot->visual_offset_y;
    if (!pstSlot->velocity_valid || pstSlot->last_pts == 0 ||
        u64Pts <= pstSlot->last_pts) {
        return;
    }
    u64GapUs = u64Pts - pstSlot->last_pts;
    if (u64GapUs > APP_PD_OSD_PREDICT_MAX_US) {
        u64GapUs = APP_PD_OSD_PREDICT_MAX_US;
    }
    *pfCenterX += pstSlot->velocity_x_per_us * u64GapUs;
    *pfCenterY += pstSlot->velocity_y_per_us * u64GapUs;
}

static CVI_U64 app_ipcam_Ai_PD_FollowCost(
    const APP_PD_FOLLOW_FRAME_S *pstPrevious,
    const APP_PD_FOLLOW_FRAME_S *pstCurrent,
    CVI_S32 s32PreviousCenterX, CVI_S32 s32PreviousCenterY,
    CVI_S32 s32CurrentCenterX, CVI_S32 s32CurrentCenterY,
    CVI_S32 s32SpanX, CVI_S32 s32SpanY,
    CVI_S32 s32ShiftX, CVI_S32 s32ShiftY,
    CVI_U32 *pu32Texture)
{
    CVI_U8 au8Previous[
        APP_PD_FOLLOW_SAMPLE_GRID * APP_PD_FOLLOW_SAMPLE_GRID];
    CVI_U8 au8Current[
        APP_PD_FOLLOW_SAMPLE_GRID * APP_PD_FOLLOW_SAMPLE_GRID];
    CVI_U32 u32PreviousSum = 0;
    CVI_U32 u32CurrentSum = 0;
    CVI_U32 u32PreviousMin = 255;
    CVI_U32 u32PreviousMax = 0;
    CVI_U32 u32Count = 0;
    CVI_U64 u64Cost = 0;

    for (CVI_S32 y = 0; y < APP_PD_FOLLOW_SAMPLE_GRID; y++) {
        CVI_S32 s32OffsetY =
            (2 * y - (APP_PD_FOLLOW_SAMPLE_GRID - 1)) * s32SpanY /
            (APP_PD_FOLLOW_SAMPLE_GRID - 1);

        for (CVI_S32 x = 0; x < APP_PD_FOLLOW_SAMPLE_GRID; x++) {
            CVI_S32 s32OffsetX =
                (2 * x - (APP_PD_FOLLOW_SAMPLE_GRID - 1)) * s32SpanX /
                (APP_PD_FOLLOW_SAMPLE_GRID - 1);
            CVI_S32 s32PreviousX = s32PreviousCenterX + s32OffsetX;
            CVI_S32 s32PreviousY = s32PreviousCenterY + s32OffsetY;
            CVI_S32 s32CurrentX = s32CurrentCenterX + s32ShiftX +
                s32OffsetX;
            CVI_S32 s32CurrentY = s32CurrentCenterY + s32ShiftY +
                s32OffsetY;
            CVI_U8 u8Previous;
            CVI_U8 u8Current;

            if (s32PreviousX < 0 || s32PreviousY < 0 ||
                s32CurrentX < 0 || s32CurrentY < 0 ||
                s32PreviousX >= APP_PD_FOLLOW_WIDTH ||
                s32CurrentX >= APP_PD_FOLLOW_WIDTH ||
                s32PreviousY >= APP_PD_FOLLOW_HEIGHT ||
                s32CurrentY >= APP_PD_FOLLOW_HEIGHT) {
                return UINT64_MAX;
            }
            u8Previous = pstPrevious->data[
                s32PreviousY * APP_PD_FOLLOW_WIDTH + s32PreviousX];
            u8Current = pstCurrent->data[
                s32CurrentY * APP_PD_FOLLOW_WIDTH + s32CurrentX];
            au8Previous[u32Count] = u8Previous;
            au8Current[u32Count] = u8Current;
            u32PreviousSum += u8Previous;
            u32CurrentSum += u8Current;
            if (u8Previous < u32PreviousMin) {
                u32PreviousMin = u8Previous;
            }
            if (u8Previous > u32PreviousMax) {
                u32PreviousMax = u8Previous;
            }
            u32Count++;
        }
    }
    if (pu32Texture != NULL) {
        *pu32Texture = u32PreviousMax - u32PreviousMin;
    }
    for (CVI_U32 i = 0; i < u32Count; i++) {
        CVI_S32 s32Difference =
            (CVI_S32)au8Current[i] - au8Previous[i] -
            ((CVI_S32)u32CurrentSum - (CVI_S32)u32PreviousSum) /
                (CVI_S32)u32Count;

        if (s32Difference < 0) {
            s32Difference = -s32Difference;
        }
        u64Cost += s32Difference;
    }
    return u32Count > 0 ? u64Cost / u32Count : UINT64_MAX;
}

static CVI_VOID app_ipcam_Ai_PD_FollowSlots(
    APP_PD_DISPLAY_SLOT_S *pastSlots,
    const APP_PD_FOLLOW_FRAME_S *pstPrevious,
    const APP_PD_FOLLOW_FRAME_S *pstCurrent,
    APP_PD_PROFILE_S *pstProfile)
{
    CVI_U32 u32Evaluated = 0;

    if (!pstPrevious->valid || !pstCurrent->valid ||
        pstPrevious->source_width != pstCurrent->source_width ||
        pstPrevious->source_height != pstCurrent->source_height ||
        pstCurrent->pts <= pstPrevious->pts) {
        return;
    }
    for (CVI_U32 i = 0;
         i < APP_PD_DISPLAY_SLOT_SIZE &&
         u32Evaluated < APP_PD_FOLLOW_MAX_OBJECTS;
         i++) {
        APP_PD_DISPLAY_SLOT_S *pstSlot = &pastSlots[i];
        CVI_FLOAT fPreviousCenterX;
        CVI_FLOAT fPreviousCenterY;
        CVI_FLOAT fCurrentCenterX;
        CVI_FLOAT fCurrentCenterY;
        CVI_FLOAT fScaleX = APP_PD_FOLLOW_WIDTH /
            (CVI_FLOAT)pstCurrent->source_width;
        CVI_FLOAT fScaleY = APP_PD_FOLLOW_HEIGHT /
            (CVI_FLOAT)pstCurrent->source_height;
        CVI_FLOAT fBoxWidth = pstSlot->width * fScaleX;
        CVI_FLOAT fBoxHeight = pstSlot->height * fScaleY;
        CVI_S32 s32PreviousCenterX;
        CVI_S32 s32PreviousCenterY;
        CVI_S32 s32CurrentCenterX;
        CVI_S32 s32CurrentCenterY;
        CVI_S32 s32SpanX;
        CVI_S32 s32SpanY;
        CVI_S32 s32ResidualLimitX;
        CVI_S32 s32ResidualLimitY;
        CVI_S32 s32BestX = 0;
        CVI_S32 s32BestY = 0;
        CVI_U32 u32BestDistance = UINT32_MAX;
        CVI_U64 u64BestCost = UINT64_MAX;
        CVI_U64 u64ZeroCost = UINT64_MAX;
        CVI_U64 u64SecondCost = UINT64_MAX;
        CVI_U32 u32BestTexture = 0;
        CVI_U16 au16Costs[APP_PD_FOLLOW_SEARCH_SIZE]
            [APP_PD_FOLLOW_SEARCH_SIZE];

        if (!pstSlot->used || pstSlot->evidence <= 0 ||
            pstSlot->last_pts == 0 ||
            fBoxWidth < APP_PD_FOLLOW_MIN_BOX_SIZE ||
            fBoxHeight < APP_PD_FOLLOW_MIN_BOX_SIZE) {
            continue;
        }
        u32Evaluated++;
        app_ipcam_Ai_PD_DisplaySlot_CenterAtPts(
            pstSlot, pstPrevious->pts,
            &fPreviousCenterX, &fPreviousCenterY);
        app_ipcam_Ai_PD_DisplaySlot_CenterAtPts(
            pstSlot, pstCurrent->pts,
            &fCurrentCenterX, &fCurrentCenterY);
        s32PreviousCenterX = lroundf(fPreviousCenterX * fScaleX);
        s32PreviousCenterY = lroundf(fPreviousCenterY * fScaleY);
        s32CurrentCenterX = lroundf(fCurrentCenterX * fScaleX);
        s32CurrentCenterY = lroundf(fCurrentCenterY * fScaleY);
        s32SpanX = lroundf(fmaxf(2.0f, fBoxWidth * 0.30f));
        s32SpanY = lroundf(fmaxf(2.0f, fBoxHeight * 0.30f));
        s32ResidualLimitX = lroundf(fmaxf(
            APP_PD_FOLLOW_MIN_RESIDUAL_LIMIT,
            fBoxWidth * APP_PD_FOLLOW_MAX_RESIDUAL_BOX_RATIO));
        s32ResidualLimitY = lroundf(fmaxf(
            APP_PD_FOLLOW_MIN_RESIDUAL_LIMIT,
            fBoxHeight * APP_PD_FOLLOW_MAX_RESIDUAL_BOX_RATIO));
        memset(au16Costs, 0xff, sizeof(au16Costs));

        for (CVI_S32 y = -APP_PD_FOLLOW_SEARCH_RADIUS;
             y <= APP_PD_FOLLOW_SEARCH_RADIUS; y++) {
            for (CVI_S32 x = -APP_PD_FOLLOW_SEARCH_RADIUS;
                 x <= APP_PD_FOLLOW_SEARCH_RADIUS; x++) {
                CVI_U32 u32Texture = 0;
                CVI_U32 u32Distance = x * x + y * y;
                CVI_U64 u64Cost = app_ipcam_Ai_PD_FollowCost(
                    pstPrevious, pstCurrent,
                    s32PreviousCenterX, s32PreviousCenterY,
                    s32CurrentCenterX, s32CurrentCenterY,
                    s32SpanX, s32SpanY, x, y, &u32Texture);

                if (u64Cost <= UINT16_MAX) {
                    au16Costs[y + APP_PD_FOLLOW_SEARCH_RADIUS]
                        [x + APP_PD_FOLLOW_SEARCH_RADIUS] =
                        (CVI_U16)u64Cost;
                }
                if (x == 0 && y == 0) {
                    u64ZeroCost = u64Cost;
                }
                if (u64Cost < u64BestCost ||
                    (u64Cost == u64BestCost &&
                     u32Distance < u32BestDistance)) {
                    u64BestCost = u64Cost;
                    s32BestX = x;
                    s32BestY = y;
                    u32BestDistance = u32Distance;
                    u32BestTexture = u32Texture;
                }
            }
        }
        /* Keep near-ties stationary; larger motion must have a unique,
         * materially better match than the zero-shift candidate. */
        if ((abs(s32BestX) <= APP_PD_FOLLOW_STILL_RADIUS) &&
            (abs(s32BestY) <= APP_PD_FOLLOW_STILL_RADIUS)) {
            s32BestX = 0;
            s32BestY = 0;
            u64BestCost = u64ZeroCost;
        } else {
            for (CVI_S32 y = -APP_PD_FOLLOW_SEARCH_RADIUS;
                 y <= APP_PD_FOLLOW_SEARCH_RADIUS; y++) {
                for (CVI_S32 x = -APP_PD_FOLLOW_SEARCH_RADIUS;
                     x <= APP_PD_FOLLOW_SEARCH_RADIUS; x++) {
                    CVI_U64 u64Cost = au16Costs[
                        y + APP_PD_FOLLOW_SEARCH_RADIUS]
                        [x + APP_PD_FOLLOW_SEARCH_RADIUS];

                    if (u64Cost == UINT16_MAX) {
                        continue;
                    }
                    if (abs(x - s32BestX) <=
                            APP_PD_FOLLOW_UNIQUENESS_RADIUS &&
                        abs(y - s32BestY) <=
                            APP_PD_FOLLOW_UNIQUENESS_RADIUS) {
                        continue;
                    }
                    if (u64Cost < u64SecondCost) {
                        u64SecondCost = u64Cost;
                    }
                }
            }
        }
        if (u64BestCost == UINT64_MAX ||
            u64BestCost > APP_PD_FOLLOW_MAX_COST ||
            u32BestTexture < APP_PD_FOLLOW_MIN_TEXTURE ||
            ((s32BestX != 0 || s32BestY != 0) &&
             (abs(s32BestX) > s32ResidualLimitX ||
              abs(s32BestY) > s32ResidualLimitY ||
              u64ZeroCost == UINT64_MAX || u64ZeroCost <= u64BestCost ||
              u64ZeroCost - u64BestCost < APP_PD_FOLLOW_MIN_COST_GAIN ||
              (CVI_FLOAT)(u64ZeroCost - u64BestCost) / u64ZeroCost <
                  APP_PD_FOLLOW_MIN_IMPROVEMENT ||
              u64SecondCost == UINT64_MAX ||
              u64SecondCost <= u64BestCost ||
              u64SecondCost - u64BestCost <
                  APP_PD_FOLLOW_MIN_COST_MARGIN))) {
            pstProfile->follow_rejects++;
            continue;
        }
        pstSlot->visual_offset_x += s32BestX / fScaleX;
        pstSlot->visual_offset_y += s32BestY / fScaleY;
        pstProfile->followed_objects++;
        if (s32BestX != 0 || s32BestY != 0) {
            pstProfile->follow_moved_objects++;
        }
    }
}
#endif

static CVI_VOID app_ipcam_Ai_PD_FrameActive_Clear(CVI_VOID)
{
    pthread_mutex_lock(&g_PDStatusMutex);
    g_bPDFrameActive = CVI_FALSE;
    pthread_cond_broadcast(&g_PDStatusCond);
    pthread_mutex_unlock(&g_PDStatusMutex);
}

static CVI_VOID *app_ipcam_Ai_PD_FrameSource_Thread(CVI_VOID *arg)
{
    APP_PD_FRAME_SOURCE_S *pstSource = arg;

    prctl(PR_SET_NAME, "PD_FRAME_SOURCE");
    for (;;) {
        VIDEO_FRAME_INFO_S stFrame = {0};
        VIDEO_FRAME_INFO_S stReleaseFrame = {0};
        CVI_BOOL bReleaseFrame = CVI_FALSE;
        CVI_S32 s32Ret;

        pthread_mutex_lock(&pstSource->mutex);
        while (pstSource->running && pstSource->paused) {
            pstSource->producer_active = CVI_FALSE;
            pthread_cond_broadcast(&pstSource->cond);
            pthread_cond_wait(&pstSource->cond, &pstSource->mutex);
        }
        if (!pstSource->running) {
            pstSource->producer_active = CVI_FALSE;
            pthread_cond_broadcast(&pstSource->cond);
            pthread_mutex_unlock(&pstSource->mutex);
            break;
        }
        pstSource->producer_active = CVI_TRUE;
        pthread_mutex_unlock(&pstSource->mutex);

        s32Ret = CVI_VPSS_GetChnFrame(
            pstSource->VpssGrp, pstSource->VpssChn, &stFrame,
            APP_PD_FRAME_SOURCE_TIMEOUT_MS);

        pthread_mutex_lock(&pstSource->mutex);
        pstSource->producer_active = CVI_FALSE;
        if (s32Ret == CVI_SUCCESS && pstSource->running &&
            !pstSource->paused) {
            if (pstSource->frame_valid) {
                stReleaseFrame = pstSource->frame;
                pstSource->dropped_frames++;
                bReleaseFrame = CVI_TRUE;
            }
            pstSource->frame = stFrame;
            pstSource->frame_capture_time_us = 0;
            pstSource->frame_valid = CVI_TRUE;
            pthread_cond_broadcast(&pstSource->cond);
        } else if (s32Ret == CVI_SUCCESS) {
            stReleaseFrame = stFrame;
            bReleaseFrame = CVI_TRUE;
        }
        pthread_cond_broadcast(&pstSource->cond);
        pthread_mutex_unlock(&pstSource->mutex);

        if (bReleaseFrame) {
            CVI_VPSS_ReleaseChnFrame(
                pstSource->VpssGrp, pstSource->VpssChn, &stReleaseFrame);
        }
    }
    return NULL;
}

static CVI_S32 app_ipcam_Ai_PD_FrameSource_Start(
    VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    APP_PD_FRAME_SOURCE_S *pstSource = &g_stPDFrameSource;
    CVI_S32 s32Ret;

    pthread_mutex_lock(&pstSource->mutex);
    pstSource->VpssGrp = VpssGrp;
    pstSource->VpssChn = VpssChn;
    pstSource->running = CVI_TRUE;
    pstSource->paused = CVI_FALSE;
    pstSource->producer_active = CVI_FALSE;
    pstSource->frame_valid = CVI_FALSE;
    pstSource->frame_capture_time_us = 0;
    pstSource->dropped_frames = 0;
    memset(&pstSource->frame, 0, sizeof(pstSource->frame));
    pthread_mutex_unlock(&pstSource->mutex);

    s32Ret = pthread_create(&pstSource->thread, NULL,
        app_ipcam_Ai_PD_FrameSource_Thread, pstSource);
    if (s32Ret != CVI_SUCCESS) {
        pthread_mutex_lock(&pstSource->mutex);
        pstSource->running = CVI_FALSE;
        pthread_mutex_unlock(&pstSource->mutex);
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "create PD frame source thread failed with %#x\n", s32Ret);
        return s32Ret;
    }
    return CVI_SUCCESS;
}

static CVI_VOID app_ipcam_Ai_PD_FrameSource_Stop(CVI_VOID)
{
    APP_PD_FRAME_SOURCE_S *pstSource = &g_stPDFrameSource;
    VIDEO_FRAME_INFO_S stFrame = {0};
    CVI_BOOL bReleaseFrame = CVI_FALSE;

    pthread_mutex_lock(&pstSource->mutex);
    pstSource->running = CVI_FALSE;
    pstSource->paused = CVI_FALSE;
    pthread_cond_broadcast(&pstSource->cond);
    pthread_mutex_unlock(&pstSource->mutex);

    if (pstSource->thread) {
        pthread_join(pstSource->thread, NULL);
        pstSource->thread = 0;
    }

    pthread_mutex_lock(&pstSource->mutex);
    if (pstSource->frame_valid) {
        stFrame = pstSource->frame;
        pstSource->frame_valid = CVI_FALSE;
        pstSource->frame_capture_time_us = 0;
        bReleaseFrame = CVI_TRUE;
    }
    pstSource->dropped_frames = 0;
    pthread_mutex_unlock(&pstSource->mutex);
    if (bReleaseFrame) {
        CVI_VPSS_ReleaseChnFrame(
            pstSource->VpssGrp, pstSource->VpssChn, &stFrame);
    }
}

static CVI_VOID app_ipcam_Ai_PD_FrameSource_Pause(CVI_BOOL bPause)
{
    APP_PD_FRAME_SOURCE_S *pstSource = &g_stPDFrameSource;
    VIDEO_FRAME_INFO_S stFrame = {0};
    CVI_BOOL bReleaseFrame = CVI_FALSE;

    pthread_mutex_lock(&pstSource->mutex);
    if (!pstSource->running) {
        pthread_mutex_unlock(&pstSource->mutex);
        return;
    }
    pstSource->paused = bPause;
    pthread_cond_broadcast(&pstSource->cond);
    while (bPause && pstSource->producer_active) {
        pthread_cond_wait(&pstSource->cond, &pstSource->mutex);
    }
    if (bPause && pstSource->frame_valid) {
        stFrame = pstSource->frame;
        pstSource->frame_valid = CVI_FALSE;
        pstSource->frame_capture_time_us = 0;
        pstSource->dropped_frames = 0;
        bReleaseFrame = CVI_TRUE;
    }
    pthread_mutex_unlock(&pstSource->mutex);
    if (bReleaseFrame) {
        CVI_VPSS_ReleaseChnFrame(
            pstSource->VpssGrp, pstSource->VpssChn, &stFrame);
    }
}

static CVI_S32 app_ipcam_Ai_PD_FrameSource_Take(
    VIDEO_FRAME_INFO_S *pstFrame, CVI_U64 *pu64CaptureTimeUs,
    CVI_U32 *pu32DroppedFrames)
{
    APP_PD_FRAME_SOURCE_S *pstSource = &g_stPDFrameSource;

    pthread_mutex_lock(&pstSource->mutex);
    while (pstSource->running && !pstSource->paused &&
           !pstSource->frame_valid) {
        pthread_cond_wait(&pstSource->cond, &pstSource->mutex);
    }
    if (!pstSource->running || pstSource->paused ||
        !pstSource->frame_valid) {
        pthread_mutex_unlock(&pstSource->mutex);
        return CVI_FAILURE;
    }
    *pstFrame = pstSource->frame;
    if (pu64CaptureTimeUs != NULL) {
        *pu64CaptureTimeUs = pstSource->frame_capture_time_us;
    }
    pstSource->frame_valid = CVI_FALSE;
    pstSource->frame_capture_time_us = 0;
    if (pu32DroppedFrames != NULL) {
        *pu32DroppedFrames = pstSource->dropped_frames;
    }
    pstSource->dropped_frames = 0;
    pthread_cond_broadcast(&pstSource->cond);
    pthread_mutex_unlock(&pstSource->mutex);
    return CVI_SUCCESS;
}

static CVI_VOID app_ipcam_Ai_PD_ProfileComplete(
    CVI_U32 frame_count, CVI_U64 loop_start, APP_PD_PROFILE_S *profile)
{
    profile->loop_us = app_ipcam_Ai_PD_TimeUs() - loop_start;
    g_PDProc = (CVI_U32)((profile->loop_us + 500) / 1000);
    g_PDFps = profile->loop_us > 0 ?
        (CVI_U32)(1000000 / profile->loop_us) : 0;
    (void)frame_count;
}

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel model_id)
{
    switch (model_id)
    {
        case TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON:
        case TDL_MODEL_YOLOV8N_DET_PERSON_VEHICLE:
        case TDL_MODEL_YOLOV5:
        case TDL_MODEL_YOLOV6:
        case TDL_MODEL_YOLOV7:
        case TDL_MODEL_YOLOV8:
        case TDL_MODEL_YOLOV10:
        case TDL_MODEL_YOLO26:
        case TDL_MODEL_PPYOLOE:
        case TDL_MODEL_YOLOX:
            g_pfpPDInference = TDL_Detection;
        break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "model id (%d) invalid!\n", model_id);
            return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n", 
        g_pstPdCfg->bEnable, g_pstPdCfg->VpssGrp, g_pstPdCfg->VpssChn, g_pstPdCfg->u32GrpWidth, g_pstPdCfg->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "threshold=%f candidate_threshold=%f\n", g_pstPdCfg->threshold,
        app_ipcam_Ai_PD_CandidateThreshold(g_pstPdCfg->threshold));
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s model_path_cfg=%s\n",
        g_pstPdCfg->model_id, g_pstPdCfg->model_path, g_pstPdCfg->model_path_cfg);

}

static CVI_S32 app_ipcam_Ai_PD_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Param_dump();

    if (g_PDAiHandle == NULL)
    {
        g_PDAiHandle = TDL_CreateHandle(0);
        if (g_PDAiHandle == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle failed!\n");
            return CVI_FAILURE;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Ai_InferenceFunc_Get(g_pstPdCfg->model_id);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: %d \n", g_pstPdCfg->model_id);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_PDAiHandle, g_pstPdCfg->model_id,
        g_pstPdCfg->model_path,
        g_pstPdCfg->model_path_cfg[0] == '\0' ? NULL : g_pstPdCfg->model_path_cfg,
        0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetModelPath failed with %#x! maybe reset model path\n", s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_SetModelThreshold(g_PDAiHandle, g_pstPdCfg->model_id,
        app_ipcam_Ai_PD_CandidateThreshold(g_pstPdCfg->threshold));
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetModelThreshold failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    {
        SMT_MutexAutoLock(g_PDMutex, lock);
        if (g_stPDObjDraw.info == NULL) {
            g_stPDObjDraw.info = malloc(
                APP_PD_MAX_DRAW_OBJECTS * sizeof(TDLObjectInfo));
            if (g_stPDObjDraw.info == NULL) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "malloc g_stPDObjDraw.info failed\n");
                return CVI_FAILURE;
            }
        }
        g_stPDObjDraw.size = 0;
        g_stPDObjDraw.width = 0;
        g_stPDObjDraw.height = 0;
        memset(&g_stPDResultFrame, 0, sizeof(g_stPDResultFrame));
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD init ------------------> done \n");

    return CVI_SUCCESS;
}

/* Stable display IDs keep OSD and DET-to-SOT selection independent of MOT. */
static CVI_FLOAT app_ipcam_Ai_PD_Box_Iou(
    const TDLBox *pstBoxA, const TDLBox *pstBoxB)
{
    CVI_FLOAT fX1 = fmaxf(pstBoxA->x1, pstBoxB->x1);
    CVI_FLOAT fY1 = fmaxf(pstBoxA->y1, pstBoxB->y1);
    CVI_FLOAT fX2 = fminf(pstBoxA->x2, pstBoxB->x2);
    CVI_FLOAT fY2 = fminf(pstBoxA->y2, pstBoxB->y2);
    CVI_FLOAT fIntersection = fmaxf(0.0f, fX2 - fX1) *
        fmaxf(0.0f, fY2 - fY1);
    CVI_FLOAT fAreaA = fmaxf(0.0f, pstBoxA->x2 - pstBoxA->x1) *
        fmaxf(0.0f, pstBoxA->y2 - pstBoxA->y1);
    CVI_FLOAT fAreaB = fmaxf(0.0f, pstBoxB->x2 - pstBoxB->x1) *
        fmaxf(0.0f, pstBoxB->y2 - pstBoxB->y1);
    CVI_FLOAT fUnion = fAreaA + fAreaB - fIntersection;

    return fUnion > 0.0f ? fIntersection / fUnion : 0.0f;
}

static CVI_FLOAT app_ipcam_Ai_PD_DisplaySlot_MatchScore(
    const APP_PD_DISPLAY_SLOT_S *pstSlot,
    const TDLObjectInfo *pstObject, CVI_U32 u32ResultIndex,
    CVI_U64 u64Pts,
    CVI_U32 u32Width, CVI_U32 u32Height,
    CVI_BOOL bRecovery, CVI_BOOL *pbClassRescue)
{
    TDLBox stPredictedBox = pstSlot->object.box;
    CVI_FLOAT fSlotWidth;
    CVI_FLOAT fSlotHeight;
    CVI_FLOAT fObjectWidth;
    CVI_FLOAT fObjectHeight;
    CVI_FLOAT fGateX;
    CVI_FLOAT fGateY;
    CVI_FLOAT fDeltaX;
    CVI_FLOAT fDeltaY;
    CVI_FLOAT fDistance;
    CVI_FLOAT fIou;
    CVI_FLOAT fSizeRatio;
    CVI_FLOAT fGateRatio;
    CVI_FLOAT fGateGrowth = 1.0f;
    CVI_FLOAT fImageWidth = u32Width > 0 ? (CVI_FLOAT)u32Width : 1.0f;
    CVI_FLOAT fImageHeight = u32Height > 0 ? (CVI_FLOAT)u32Height : 1.0f;
    CVI_U32 u32MissingResults = 0;
    CVI_U64 u64MaxGapUs = APP_PD_DISPLAY_HOLD_US;

    *pbClassRescue = CVI_FALSE;
    if (!pstSlot->used) {
        return -1.0f;
    }
    if (pstSlot->last_pts > 0 && u64Pts > pstSlot->last_pts &&
        u64Pts - pstSlot->last_pts > u64MaxGapUs) {
        return -1.0f;
    }
    stPredictedBox.x1 += pstSlot->visual_offset_x;
    stPredictedBox.x2 += pstSlot->visual_offset_x;
    stPredictedBox.y1 += pstSlot->visual_offset_y;
    stPredictedBox.y2 += pstSlot->visual_offset_y;
    if (pstSlot->velocity_valid && pstSlot->last_pts > 0 &&
        u64Pts > pstSlot->last_pts) {
        CVI_U64 u64GapUs = u64Pts - pstSlot->last_pts;
        CVI_FLOAT fShiftX;
        CVI_FLOAT fShiftY;

        if (u64GapUs > APP_PD_OSD_PREDICT_MAX_US) {
            u64GapUs = APP_PD_OSD_PREDICT_MAX_US;
        }
        fShiftX = pstSlot->velocity_x_per_us * u64GapUs;
        fShiftY = pstSlot->velocity_y_per_us * u64GapUs;
        stPredictedBox.x1 += fShiftX;
        stPredictedBox.x2 += fShiftX;
        stPredictedBox.y1 += fShiftY;
        stPredictedBox.y2 += fShiftY;
    }

    fSlotWidth = fmaxf(1.0f, stPredictedBox.x2 - stPredictedBox.x1);
    fSlotHeight = fmaxf(1.0f, stPredictedBox.y2 - stPredictedBox.y1);
    fObjectWidth = fmaxf(1.0f, pstObject->box.x2 - pstObject->box.x1);
    fObjectHeight = fmaxf(1.0f, pstObject->box.y2 - pstObject->box.y1);
    fSizeRatio = fminf(fSlotWidth, fObjectWidth) /
        fmaxf(fSlotWidth, fObjectWidth);
    fSizeRatio = fminf(fSizeRatio,
        fminf(fSlotHeight, fObjectHeight) /
            fmaxf(fSlotHeight, fObjectHeight));
    if (u32ResultIndex > pstSlot->last_observed_result) {
        CVI_U32 u32ResultGap = u32ResultIndex -
            pstSlot->last_observed_result;

        if (u32ResultGap > 1) {
            u32MissingResults = u32ResultGap - 1;
        }
        if (u32MissingResults > APP_PD_DISPLAY_HOLD_RESULTS) {
            u32MissingResults = APP_PD_DISPLAY_HOLD_RESULTS;
        }
        fGateGrowth += u32MissingResults *
            APP_PD_DISPLAY_MISSING_GATE_GROWTH;
    }
    fGateRatio = pstSlot->observations >=
        APP_PD_DISPLAY_VELOCITY_MIN_OBSERVATIONS ?
        APP_PD_DISPLAY_MATURE_GATE_RATIO :
        APP_PD_DISPLAY_CENTER_GATE_RATIO;
    if (bRecovery) {
        fGateGrowth *= APP_PD_DISPLAY_RECOVERY_GATE_SCALE;
    }
    fGateX = fmaxf(fmaxf(fSlotWidth, fObjectWidth) *
        2.5f * fGateGrowth, fImageWidth * fGateRatio * fGateGrowth);
    fGateY = fmaxf(fmaxf(fSlotHeight, fObjectHeight) *
        2.5f * fGateGrowth, fImageHeight * fGateRatio * fGateGrowth);
    fGateX = fminf(fGateX,
        fImageWidth * APP_PD_DISPLAY_GATE_MAX_RATIO);
    fGateY = fminf(fGateY,
        fImageHeight * APP_PD_DISPLAY_GATE_MAX_RATIO);
    fDeltaX = ((stPredictedBox.x1 + stPredictedBox.x2) -
        (pstObject->box.x1 + pstObject->box.x2)) * 0.5f;
    fDeltaY = ((stPredictedBox.y1 + stPredictedBox.y2) -
        (pstObject->box.y1 + pstObject->box.y2)) * 0.5f;
    fDistance = (fDeltaX * fDeltaX) / (fGateX * fGateX) +
        (fDeltaY * fDeltaY) / (fGateY * fGateY);
    fIou = app_ipcam_Ai_PD_Box_Iou(&stPredictedBox, &pstObject->box);
    if (pstSlot->object.class_id != pstObject->class_id) {
        if (fIou < APP_PD_DISPLAY_CLASS_RESCUE_IOU) {
            return -1.0f;
        }
        *pbClassRescue = CVI_TRUE;
        return fIou * 2.0f;
    }
    if (bRecovery &&
        fSizeRatio < APP_PD_DISPLAY_RECOVERY_MIN_SIZE_RATIO) {
        return -1.0f;
    }
    if (fIou < APP_PD_DISPLAY_MIN_IOU && fDistance > 1.0f) {
        return -1.0f;
    }
    if (bRecovery) {
        return 0.5f + fIou + fSizeRatio -
            fminf(fDistance, 1.0f) * 0.25f;
    }
    return fIou * 2.0f + 1.0f - fminf(fDistance, 1.0f) +
        0.25f;
}

static CVI_BOOL app_ipcam_Ai_PD_DisplaySlot_IsExpired(
    const APP_PD_DISPLAY_SLOT_S *pstSlot, CVI_U32 u32ResultIndex,
    CVI_U64 u64Pts)
{
    return !pstSlot->used ||
        pstSlot->evidence <= 0 ||
        u32ResultIndex - pstSlot->last_observed_result >
            APP_PD_DISPLAY_HOLD_RESULTS ||
        (pstSlot->last_pts > 0 && u64Pts > pstSlot->last_pts &&
         u64Pts - pstSlot->last_pts > APP_PD_DISPLAY_HOLD_US);
}

static APP_PD_DISPLAY_SLOT_S *app_ipcam_Ai_PD_DisplaySlot_Allocate(
    APP_PD_DISPLAY_SLOT_S *pastSlots, CVI_U32 u32ResultIndex,
    CVI_U64 u64Pts)
{
    APP_PD_DISPLAY_SLOT_S *pstOldest = NULL;

    for (CVI_U32 i = 0; i < APP_PD_DISPLAY_SLOT_SIZE; i++) {
        if (pastSlots[i].used &&
            pastSlots[i].last_observed_result == u32ResultIndex) {
            continue;
        }
        if (app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                &pastSlots[i], u32ResultIndex, u64Pts)) {
            return &pastSlots[i];
        }
        if (pstOldest == NULL ||
            pastSlots[i].last_observed_result <
            pstOldest->last_observed_result) {
            pstOldest = &pastSlots[i];
        }
    }
    return pstOldest;
}

static CVI_VOID app_ipcam_Ai_PD_DisplaySlot_Update(
    APP_PD_DISPLAY_SLOT_S *pstSlot, const TDLObjectInfo *pstObject,
    CVI_U32 u32ResultIndex, CVI_U64 u64Pts, CVI_BOOL bPrimary,
    CVI_BOOL bClassRescue)
{
    CVI_BOOL bExisting = pstSlot->used;
    CVI_BOOL bKeepDisplayClass = CVI_FALSE;
    CVI_S32 s32DisplayClass = pstSlot->object.class_id;
    CVI_FLOAT fDisplayScore = pstSlot->object.score;
    CVI_FLOAT fMeasuredCenterX =
        (pstObject->box.x1 + pstObject->box.x2) * 0.5f;
    CVI_FLOAT fMeasuredCenterY =
        (pstObject->box.y1 + pstObject->box.y2) * 0.5f;
    CVI_FLOAT fMeasuredWidth =
        fmaxf(1.0f, pstObject->box.x2 - pstObject->box.x1);
    CVI_FLOAT fMeasuredHeight =
        fmaxf(1.0f, pstObject->box.y2 - pstObject->box.y1);
    CVI_FLOAT fCenterAlpha = bPrimary ? APP_PD_DISPLAY_CENTER_ALPHA :
        APP_PD_DISPLAY_CANDIDATE_CENTER_ALPHA;
    CVI_FLOAT fVelocityBeta = bPrimary ? APP_PD_DISPLAY_VELOCITY_BETA :
        APP_PD_DISPLAY_CANDIDATE_VELOCITY_BETA;
    CVI_FLOAT fSizeAlpha = bPrimary ? APP_PD_DISPLAY_SIZE_ALPHA :
        APP_PD_DISPLAY_CANDIDATE_SIZE_ALPHA;
    CVI_S32 s32HitEvidence = bPrimary ?
        APP_PD_DISPLAY_PRIMARY_EVIDENCE :
        APP_PD_DISPLAY_CANDIDATE_EVIDENCE;

    if (pstSlot->used && pstSlot->last_pts > 0 &&
        u64Pts > pstSlot->last_pts) {
        CVI_U64 u64PtsGap = u64Pts - pstSlot->last_pts;

        if (u64PtsGap <= 500000) {
            CVI_FLOAT fMeasuredVelocityX =
                (fMeasuredCenterX - pstSlot->center_x) / u64PtsGap;
            CVI_FLOAT fMeasuredVelocityY =
                (fMeasuredCenterY - pstSlot->center_y) / u64PtsGap;

            if (pstSlot->velocity_valid) {
                pstSlot->velocity_x_per_us =
                    (1.0f - fVelocityBeta) *
                    pstSlot->velocity_x_per_us +
                    fVelocityBeta * fMeasuredVelocityX;
                pstSlot->velocity_y_per_us =
                    (1.0f - fVelocityBeta) *
                    pstSlot->velocity_y_per_us +
                    fVelocityBeta * fMeasuredVelocityY;
            } else {
                pstSlot->velocity_x_per_us = fMeasuredVelocityX;
                pstSlot->velocity_y_per_us = fMeasuredVelocityY;
                pstSlot->velocity_valid = CVI_TRUE;
            }
            pstSlot->center_x += fCenterAlpha *
                (fMeasuredCenterX - pstSlot->center_x);
            pstSlot->center_y += fCenterAlpha *
                (fMeasuredCenterY - pstSlot->center_y);
            pstSlot->width += fSizeAlpha *
                (fMeasuredWidth - pstSlot->width);
            pstSlot->height += fSizeAlpha *
                (fMeasuredHeight - pstSlot->height);
        } else {
            pstSlot->center_x = fMeasuredCenterX;
            pstSlot->center_y = fMeasuredCenterY;
            pstSlot->width = fMeasuredWidth;
            pstSlot->height = fMeasuredHeight;
            pstSlot->velocity_valid = CVI_FALSE;
        }
    } else if (!pstSlot->used) {
        pstSlot->center_x = fMeasuredCenterX;
        pstSlot->center_y = fMeasuredCenterY;
        pstSlot->width = fMeasuredWidth;
        pstSlot->height = fMeasuredHeight;
    } else {
        pstSlot->center_x = fMeasuredCenterX;
        pstSlot->center_y = fMeasuredCenterY;
        pstSlot->width = fMeasuredWidth;
        pstSlot->height = fMeasuredHeight;
        pstSlot->velocity_valid = CVI_FALSE;
    }
    pstSlot->visual_offset_x = 0.0f;
    pstSlot->visual_offset_y = 0.0f;
    if (!bExisting) {
        pstSlot->observations = 1;
        pstSlot->evidence = s32HitEvidence;
    } else if (pstSlot->observations < UINT32_MAX) {
        pstSlot->observations++;
        pstSlot->evidence += s32HitEvidence;
        if (pstSlot->evidence > APP_PD_DISPLAY_EVIDENCE_MAX) {
            pstSlot->evidence = APP_PD_DISPLAY_EVIDENCE_MAX;
        }
    }
    pstSlot->used = CVI_TRUE;
    if (bExisting && !bPrimary) {
        bKeepDisplayClass = CVI_TRUE;
    } else if (bExisting && bClassRescue) {
        if (pstSlot->pending_class_hits > 0 &&
            pstSlot->pending_class_id == pstObject->class_id) {
            pstSlot->pending_class_hits++;
        } else {
            pstSlot->pending_class_id = pstObject->class_id;
            pstSlot->pending_class_hits = 1;
        }
        if (pstSlot->pending_class_hits <
            APP_PD_DISPLAY_CLASS_CONFIRM_HITS) {
            bKeepDisplayClass = CVI_TRUE;
        } else {
            pstSlot->pending_class_hits = 0;
        }
    } else {
        pstSlot->pending_class_hits = 0;
    }
    pstSlot->object = *pstObject;
    if (bKeepDisplayClass) {
        pstSlot->object.class_id = s32DisplayClass;
        pstSlot->object.score = fDisplayScore;
    }
    pstSlot->object.track_id = pstSlot->track_id;
    pstSlot->object.landmark_size = 0;
    pstSlot->object.landmark_properity = NULL;
    pstSlot->object.box.x1 = pstSlot->center_x - pstSlot->width * 0.5f;
    pstSlot->object.box.y1 = pstSlot->center_y - pstSlot->height * 0.5f;
    pstSlot->object.box.x2 = pstSlot->center_x + pstSlot->width * 0.5f;
    pstSlot->object.box.y2 = pstSlot->center_y + pstSlot->height * 0.5f;
    pstSlot->last_pts = u64Pts;
    pstSlot->last_observed_result = u32ResultIndex;
}

static CVI_VOID app_ipcam_Ai_PD_DisplaySlot_Project(
    const APP_PD_DISPLAY_SLOT_S *pstSlot, CVI_U64 u64Pts,
    CVI_U32 u32SourceWidth, CVI_U32 u32SourceHeight,
    CVI_U64 u64PresentationLeadUs, TDLObjectInfo *pstObject)
{
    CVI_FLOAT fShiftX = 0.0f;
    CVI_FLOAT fShiftY = 0.0f;
    CVI_FLOAT fHalfWidth = pstSlot->width * 0.5f;
    CVI_FLOAT fHalfHeight = pstSlot->height * 0.5f;

    *pstObject = pstSlot->object;
    if (APP_PD_OSD_MOTION_PREDICT_ENABLE &&
        pstSlot->velocity_valid &&
        pstSlot->observations >=
            APP_PD_DISPLAY_VELOCITY_MIN_OBSERVATIONS) {
        CVI_U64 u64PredictUs = u64PresentationLeadUs;
        CVI_FLOAT fMaxShiftX =
            pstSlot->width * APP_PD_OSD_MAX_SHIFT_BOX_RATIO;
        CVI_FLOAT fMaxShiftY =
            pstSlot->height * APP_PD_OSD_MAX_SHIFT_BOX_RATIO;

        if (pstSlot->last_pts > 0 && u64Pts > pstSlot->last_pts) {
            u64PredictUs += u64Pts - pstSlot->last_pts;
        }
        if (u64PredictUs > APP_PD_OSD_PREDICT_MAX_US) {
            u64PredictUs = APP_PD_OSD_PREDICT_MAX_US;
        }
        fShiftX = pstSlot->velocity_x_per_us * u64PredictUs;
        fShiftY = pstSlot->velocity_y_per_us * u64PredictUs;
        fShiftX = fmaxf(-fMaxShiftX, fminf(fShiftX, fMaxShiftX));
        fShiftY = fmaxf(-fMaxShiftY, fminf(fShiftY, fMaxShiftY));
    }

    pstObject->box.x1 = pstSlot->center_x + pstSlot->visual_offset_x +
        fShiftX - fHalfWidth;
    pstObject->box.y1 = pstSlot->center_y + pstSlot->visual_offset_y +
        fShiftY - fHalfHeight;
    pstObject->box.x2 = pstSlot->center_x + pstSlot->visual_offset_x +
        fShiftX + fHalfWidth;
    pstObject->box.y2 = pstSlot->center_y + pstSlot->visual_offset_y +
        fShiftY + fHalfHeight;
    if (pstObject->box.x1 < 0.0f) {
        pstObject->box.x2 -= pstObject->box.x1;
        pstObject->box.x1 = 0.0f;
    }
    if (pstObject->box.y1 < 0.0f) {
        pstObject->box.y2 -= pstObject->box.y1;
        pstObject->box.y1 = 0.0f;
    }
    if (u32SourceWidth > 0 &&
        pstObject->box.x2 > (CVI_FLOAT)u32SourceWidth) {
        CVI_FLOAT fOverflow =
            pstObject->box.x2 - (CVI_FLOAT)u32SourceWidth;

        pstObject->box.x1 -= fOverflow;
        pstObject->box.x2 = (CVI_FLOAT)u32SourceWidth;
    }
    if (u32SourceHeight > 0 &&
        pstObject->box.y2 > (CVI_FLOAT)u32SourceHeight) {
        CVI_FLOAT fOverflow =
            pstObject->box.y2 - (CVI_FLOAT)u32SourceHeight;

        pstObject->box.y1 -= fOverflow;
        pstObject->box.y2 = (CVI_FLOAT)u32SourceHeight;
    }
    pstObject->box.x1 = fmaxf(0.0f, pstObject->box.x1);
    pstObject->box.y1 = fmaxf(0.0f, pstObject->box.y1);
}

static CVI_U32 app_ipcam_Ai_PD_DisplaySlots_MergeDuplicates(
    APP_PD_DISPLAY_SLOT_S *pastSlots, CVI_U32 u32ResultIndex,
    CVI_U64 u64Pts)
{
    CVI_U32 u32Merged = 0;

    for (CVI_U32 i = 0; i < APP_PD_DISPLAY_SLOT_SIZE; i++) {
        if (!pastSlots[i].used ||
            app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                &pastSlots[i], u32ResultIndex, u64Pts)) {
            continue;
        }
        for (CVI_U32 j = i + 1; j < APP_PD_DISPLAY_SLOT_SIZE; j++) {
            APP_PD_DISPLAY_SLOT_S *pstKeep = &pastSlots[i];
            APP_PD_DISPLAY_SLOT_S *pstDrop = &pastSlots[j];

            if (!pastSlots[j].used ||
                app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                    &pastSlots[j], u32ResultIndex, u64Pts) ||
                app_ipcam_Ai_PD_Box_Iou(
                    &pastSlots[i].object.box,
                    &pastSlots[j].object.box) <
                    APP_PD_DISPLAY_DUPLICATE_IOU) {
                continue;
            }
            if (pastSlots[j].last_observed_result >
                    pastSlots[i].last_observed_result ||
                (pastSlots[j].last_observed_result ==
                     pastSlots[i].last_observed_result &&
                 pastSlots[j].evidence > pastSlots[i].evidence)) {
                pstKeep = &pastSlots[j];
                pstDrop = &pastSlots[i];
            }
            if (pstDrop->evidence > pstKeep->evidence) {
                pstKeep->evidence = pstDrop->evidence;
            }
            if (pstDrop->observations > pstKeep->observations) {
                pstKeep->observations = pstDrop->observations;
            }
            pstDrop->used = CVI_FALSE;
            u32Merged++;
            if (pstDrop == &pastSlots[i]) {
                break;
            }
        }
    }
    return u32Merged;
}

static CVI_VOID app_ipcam_Ai_PD_DrawResult_Build(
    APP_PD_DISPLAY_SLOT_S *pastSlots, TDLObject *pstObjects,
    CVI_U64 *pu64NextDisplayId, CVI_U32 u32ResultIndex, CVI_U64 u64Pts,
    CVI_U64 u64PresentationLeadUs,
    TDLObject *pstDraw, APP_PD_PROFILE_S *pstProfile)
{
    CVI_BOOL abObjectMatched[APP_PD_DISPLAY_SLOT_SIZE] = {0};
    CVI_BOOL abSlotMatched[APP_PD_DISPLAY_SLOT_SIZE] = {0};
    CVI_U32 u32ObjectCount = 0;

    pstDraw->size = 0;
    pstDraw->width = pstObjects->width;
    pstDraw->height = pstObjects->height;
    pstProfile->held_objects = 0;
    pstProfile->maintained_objects = 0;
    if (pstObjects->info != NULL) {
        u32ObjectCount = pstObjects->size < APP_PD_DISPLAY_SLOT_SIZE ?
            pstObjects->size : APP_PD_DISPLAY_SLOT_SIZE;

        /* Select the best remaining object-slot edge globally in each pass.
         * Primary detections claim slots before low-score maintenance. */
        for (CVI_U32 u32Pass = 0; u32Pass < 2; u32Pass++) {
            CVI_BOOL bPrimaryPass = u32Pass == 0;

            while (CVI_TRUE) {
                CVI_FLOAT fBestScore = -1.0f;
                CVI_U32 u32BestObject = APP_PD_DISPLAY_SLOT_SIZE;
                CVI_U32 u32BestSlot = APP_PD_DISPLAY_SLOT_SIZE;
                CVI_BOOL bBestClassRescue = CVI_FALSE;

                for (CVI_U32 i = 0; i < u32ObjectCount; i++) {
                    CVI_BOOL bPrimary = pstObjects->info[i].score >=
                        g_pstPdCfg->threshold;

                    if (abObjectMatched[i] ||
                        bPrimary != bPrimaryPass) {
                        continue;
                    }
                    for (CVI_U32 j = 0;
                         j < APP_PD_DISPLAY_SLOT_SIZE; j++) {
                        CVI_BOOL bClassRescue;
                        CVI_FLOAT fScore;

                        if (abSlotMatched[j] || !pastSlots[j].used ||
                            app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                                &pastSlots[j], u32ResultIndex, u64Pts)) {
                            continue;
                        }
                        fScore = app_ipcam_Ai_PD_DisplaySlot_MatchScore(
                            &pastSlots[j], &pstObjects->info[i],
                            u32ResultIndex, u64Pts,
                            pstObjects->width, pstObjects->height,
                            CVI_FALSE, &bClassRescue);
                        if (fScore > fBestScore ||
                            (fScore == fBestScore &&
                             bBestClassRescue && !bClassRescue)) {
                            fBestScore = fScore;
                            u32BestObject = i;
                            u32BestSlot = j;
                            bBestClassRescue = bClassRescue;
                        }
                    }
                }
                if (u32BestObject == APP_PD_DISPLAY_SLOT_SIZE) {
                    break;
                }
                abObjectMatched[u32BestObject] = CVI_TRUE;
                abSlotMatched[u32BestSlot] = CVI_TRUE;
                pstObjects->info[u32BestObject].track_id =
                    pastSlots[u32BestSlot].track_id;
                app_ipcam_Ai_PD_DisplaySlot_Update(
                    &pastSlots[u32BestSlot],
                    &pstObjects->info[u32BestObject],
                    u32ResultIndex, u64Pts, bPrimaryPass,
                    bBestClassRescue);
                if (bBestClassRescue) {
                    pstProfile->association_rescues++;
                }
                if (!bPrimaryPass) {
                    pstProfile->maintained_objects++;
                }
            }

            if (bPrimaryPass) {
                while (CVI_TRUE) {
                    CVI_FLOAT fBestScore = -1.0f;
                    CVI_U32 u32BestObject = APP_PD_DISPLAY_SLOT_SIZE;
                    CVI_U32 u32BestSlot = APP_PD_DISPLAY_SLOT_SIZE;
                    CVI_BOOL bBestClassRescue = CVI_FALSE;

                    for (CVI_U32 i = 0; i < u32ObjectCount; i++) {
                        if (abObjectMatched[i] ||
                            pstObjects->info[i].score <
                                g_pstPdCfg->threshold) {
                            continue;
                        }
                        for (CVI_U32 j = 0;
                             j < APP_PD_DISPLAY_SLOT_SIZE; j++) {
                            CVI_BOOL bClassRescue;
                            CVI_FLOAT fScore;

                            if (abSlotMatched[j] ||
                                !pastSlots[j].used ||
                                app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                                    &pastSlots[j], u32ResultIndex,
                                    u64Pts)) {
                                continue;
                            }
                            fScore =
                                app_ipcam_Ai_PD_DisplaySlot_MatchScore(
                                    &pastSlots[j], &pstObjects->info[i],
                                    u32ResultIndex, u64Pts,
                                    pstObjects->width,
                                    pstObjects->height,
                                    CVI_TRUE, &bClassRescue);
                            if (fScore > fBestScore ||
                                (fScore == fBestScore &&
                                 bBestClassRescue && !bClassRescue)) {
                                fBestScore = fScore;
                                u32BestObject = i;
                                u32BestSlot = j;
                                bBestClassRescue = bClassRescue;
                            }
                        }
                    }
                    if (u32BestObject == APP_PD_DISPLAY_SLOT_SIZE) {
                        break;
                    }
                    abObjectMatched[u32BestObject] = CVI_TRUE;
                    abSlotMatched[u32BestSlot] = CVI_TRUE;
                    pstObjects->info[u32BestObject].track_id =
                        pastSlots[u32BestSlot].track_id;
                    app_ipcam_Ai_PD_DisplaySlot_Update(
                        &pastSlots[u32BestSlot],
                        &pstObjects->info[u32BestObject],
                        u32ResultIndex, u64Pts, CVI_TRUE,
                        bBestClassRescue);
                    if (bBestClassRescue) {
                        pstProfile->association_rescues++;
                    } else {
                        pstProfile->association_motion_rescues++;
                    }
                }
            }

            for (CVI_U32 i = 0; i < u32ObjectCount; i++) {
                CVI_BOOL bPrimary = pstObjects->info[i].score >=
                    g_pstPdCfg->threshold;

                if (abObjectMatched[i] || bPrimary != bPrimaryPass) {
                    continue;
                }
                if (bPrimaryPass) {
                    APP_PD_DISPLAY_SLOT_S *pstNewSlot =
                        app_ipcam_Ai_PD_DisplaySlot_Allocate(
                            pastSlots, u32ResultIndex, u64Pts);
                    CVI_U32 u32NewSlot;

                    if (pstNewSlot == NULL) {
                        pstObjects->info[i].track_id = 0;
                        continue;
                    }
                    u32NewSlot = (CVI_U32)(pstNewSlot - pastSlots);
                    memset(pstNewSlot, 0, sizeof(*pstNewSlot));
                    pstNewSlot->track_id = (*pu64NextDisplayId)++;
                    if (*pu64NextDisplayId == 0) {
                        *pu64NextDisplayId = 1;
                    }
                    abObjectMatched[i] = CVI_TRUE;
                    abSlotMatched[u32NewSlot] = CVI_TRUE;
                    pstObjects->info[i].track_id = pstNewSlot->track_id;
                    app_ipcam_Ai_PD_DisplaySlot_Update(
                        pstNewSlot, &pstObjects->info[i],
                        u32ResultIndex, u64Pts, CVI_TRUE, CVI_FALSE);
                    pstProfile->association_new_objects++;
                } else {
                    CVI_BOOL bSameClassSlot = CVI_FALSE;

                    for (CVI_U32 j = 0;
                         j < APP_PD_DISPLAY_SLOT_SIZE; j++) {
                        if (pastSlots[j].used &&
                            !app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                                &pastSlots[j], u32ResultIndex, u64Pts) &&
                            pastSlots[j].object.class_id ==
                                pstObjects->info[i].class_id) {
                            bSameClassSlot = CVI_TRUE;
                            break;
                        }
                    }
                    if (bSameClassSlot) {
                        pstProfile->association_gate_rejects++;
                    } else {
                        pstProfile->association_class_rejects++;
                    }
                }
            }
        }
    }

    pstProfile->association_merges =
        app_ipcam_Ai_PD_DisplaySlots_MergeDuplicates(
            pastSlots, u32ResultIndex, u64Pts);

    for (CVI_U32 i = 0; i < APP_PD_DISPLAY_SLOT_SIZE; i++) {
        if (pastSlots[i].used &&
            pastSlots[i].last_observed_result != u32ResultIndex) {
            pastSlots[i].evidence -= APP_PD_DISPLAY_MISS_EVIDENCE;
            if (pastSlots[i].evidence < 0) {
                pastSlots[i].evidence = 0;
            }
        }
    }

    for (CVI_U32 u32Pass = 0; u32Pass < 2; u32Pass++) {
        for (CVI_U32 i = 0;
             i < APP_PD_DISPLAY_SLOT_SIZE &&
             pstDraw->size < APP_PD_MAX_DRAW_OBJECTS;
             i++) {
            CVI_U32 u32MissingResults;
            TDLObjectInfo *pstDrawObject;

            if (!pastSlots[i].used) {
                continue;
            }
            u32MissingResults = u32ResultIndex -
                pastSlots[i].last_observed_result;
            if (app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                    &pastSlots[i], u32ResultIndex, u64Pts)) {
                pastSlots[i].used = CVI_FALSE;
                continue;
            }
            if ((u32Pass == 0 && u32MissingResults > 0) ||
                (u32Pass == 1 && u32MissingResults == 0)) {
                continue;
            }
            pstDrawObject = &pstDraw->info[pstDraw->size++];
            app_ipcam_Ai_PD_DisplaySlot_Project(
                &pastSlots[i], u64Pts, pstDraw->width, pstDraw->height,
                u64PresentationLeadUs, pstDrawObject);
            if (u32MissingResults > 0) {
                pstProfile->held_objects++;
            }
        }
    }
}

#if APP_PD_IMAGE_FOLLOW_ENABLE
static CVI_VOID app_ipcam_Ai_PD_FollowDraw_Build(
    APP_PD_DISPLAY_SLOT_S *pastSlots, CVI_U32 u32ResultIndex,
    CVI_U64 u64Pts, CVI_U32 u32Width, CVI_U32 u32Height,
    TDLObject *pstDraw)
{
    pstDraw->size = 0;
    pstDraw->width = u32Width;
    pstDraw->height = u32Height;
    for (CVI_U32 i = 0;
         i < APP_PD_DISPLAY_SLOT_SIZE &&
         pstDraw->size < APP_PD_MAX_DRAW_OBJECTS;
         i++) {
        if (!pastSlots[i].used ||
            app_ipcam_Ai_PD_DisplaySlot_IsExpired(
                &pastSlots[i], u32ResultIndex, u64Pts)) {
            continue;
        }
        app_ipcam_Ai_PD_DisplaySlot_Project(
            &pastSlots[i], u64Pts, u32Width, u32Height,
            APP_PD_OSD_ESTIMATED_APPLY_US,
            &pstDraw->info[pstDraw->size++]);
    }
}

static CVI_BOOL app_ipcam_Ai_PD_DrawResult_HasSameTracks(
    const TDLObject *pstLeft, const TDLObject *pstRight)
{
    if (pstLeft->size != pstRight->size) {
        return CVI_FALSE;
    }
    for (CVI_U32 i = 0; i < pstLeft->size; i++) {
        CVI_BOOL bFound = CVI_FALSE;

        for (CVI_U32 j = 0; j < pstRight->size; j++) {
            if (pstLeft->info[i].track_id == pstRight->info[j].track_id &&
                pstLeft->info[i].class_id == pstRight->info[j].class_id) {
                bFound = CVI_TRUE;
                break;
            }
        }
        if (!bFound) {
            return CVI_FALSE;
        }
    }
    return CVI_TRUE;
}
#endif

static CVI_VOID app_ipcam_Ai_PD_DrawResult_Copy(
    TDLObject *pstDestination, const TDLObject *pstSource)
{
    pstDestination->size = pstSource->size;
    pstDestination->width = pstSource->width;
    pstDestination->height = pstSource->height;
    if (pstSource->size > 0) {
        memcpy(pstDestination->info, pstSource->info,
            pstSource->size * sizeof(TDLObjectInfo));
    }
}

static CVI_BOOL app_ipcam_Ai_PD_DrawResult_IsSame(
    const TDLObject *pstLeft, const TDLObject *pstRight)
{
    if (pstLeft->size != pstRight->size ||
        pstLeft->width != pstRight->width ||
        pstLeft->height != pstRight->height) {
        return CVI_FALSE;
    }
    for (CVI_U32 i = 0; i < pstLeft->size; i++) {
        const TDLObjectInfo *pstLeftObject = &pstLeft->info[i];
        const TDLObjectInfo *pstRightObject = &pstRight->info[i];

        if (pstLeftObject->class_id != pstRightObject->class_id ||
            pstLeftObject->track_id != pstRightObject->track_id ||
            pstLeftObject->box.x1 != pstRightObject->box.x1 ||
            pstLeftObject->box.y1 != pstRightObject->box.y1 ||
            pstLeftObject->box.x2 != pstRightObject->box.x2 ||
            pstLeftObject->box.y2 != pstRightObject->box.y2) {
            return CVI_FALSE;
        }
    }
    return CVI_TRUE;
}

static CVI_VOID *Thread_PD_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U32 u32FrameCount = 0;
    APP_PD_DISPLAY_SLOT_S astDisplaySlots[APP_PD_DISPLAY_SLOT_SIZE] = {0};
    TDLObjectInfo astLastPublishedDraw[APP_PD_MAX_DRAW_OBJECTS] = {0};
#if APP_PD_IMAGE_FOLLOW_ENABLE
    TDLObjectInfo astFollowDraw[APP_PD_MAX_DRAW_OBJECTS] = {0};
#endif
    TDLObjectInfo astDetectionDraw[APP_PD_MAX_DRAW_OBJECTS] = {0};
    TDLObject stLastPublishedDraw = {0};
#if APP_PD_IMAGE_FOLLOW_ENABLE
    TDLObject stFollowDraw = {0};
#endif
    TDLObject stDetectionDraw = {0};
#if APP_PD_IMAGE_FOLLOW_ENABLE
    APP_PD_FOLLOW_FRAME_S stPreviousFollowFrame = {0};
    APP_PD_FOLLOW_FRAME_S stCurrentFollowFrame = {0};
#endif
    CVI_U64 u64NextDisplayId = 1;
    CVI_BOOL bHavePublishedDraw = CVI_FALSE;

    stLastPublishedDraw.info = astLastPublishedDraw;
#if APP_PD_IMAGE_FOLLOW_ENABLE
    stFollowDraw.info = astFollowDraw;
#endif
    stDetectionDraw.info = astDetectionDraw;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD start running!\n");

    prctl(PR_SET_NAME, "Thread_PD_PROC");

    VPSS_GRP VpssGrp = g_pstPdCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstPdCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    TDLImage image_handle ;

    while (app_ipcam_Ai_PD_ProcStatus_Get()) {
        APP_PD_PROFILE_S stProfile = {0};
        CVI_BOOL bPublishOsd = CVI_FALSE;
#if APP_PD_IMAGE_FOLLOW_ENABLE
        CVI_BOOL bFollowPathActive = CVI_FALSE;
#endif
        CVI_U64 u64LoopStart = app_ipcam_Ai_PD_TimeUs();

        pthread_mutex_lock(&g_PDStatusMutex);
        if (g_bPDPause) {
            pthread_mutex_unlock(&g_PDStatusMutex);
            usleep(10 * 1000);
            continue;
        }
        g_bPDFrameActive = CVI_TRUE;
        pthread_mutex_unlock(&g_PDStatusMutex);

        s32Ret = app_ipcam_Ai_PD_FrameSource_Take(
            &stfdFrame, NULL, NULL);
        if (s32Ret != CVI_SUCCESS) {
            app_ipcam_Ai_PD_FrameActive_Clear();
            if (!app_ipcam_Ai_PD_ProcStatus_Get()) {
                break;
            }
            usleep(1000);
            continue;
        }
        stProfile.sequence = stfdFrame.stVFrame.u32SeqenceNo;
        stProfile.time_ref = stfdFrame.stVFrame.u32TimeRef;
        stProfile.pts = stfdFrame.stVFrame.u64PTS;
#if APP_PD_IMAGE_FOLLOW_ENABLE
        if (app_ipcam_Ai_PD_FollowFrame_Capture(
                &stfdFrame, stProfile.pts,
                &stCurrentFollowFrame) == CVI_SUCCESS) {
            if (stPreviousFollowFrame.valid) {
                CVI_BOOL bFollowPublishOsd = CVI_FALSE;

                app_ipcam_Ai_PD_FollowSlots(
                    astDisplaySlots, &stPreviousFollowFrame,
                    &stCurrentFollowFrame, &stProfile);
                app_ipcam_Ai_PD_FollowDraw_Build(
                    astDisplaySlots, u32FrameCount + 1,
                    stProfile.pts, stfdFrame.stVFrame.u32Width,
                    stfdFrame.stVFrame.u32Height, &stFollowDraw);
                bFollowPathActive = stFollowDraw.size > 0 &&
                    stProfile.followed_objects > 0;
                if (bFollowPathActive) {
                    CVI_U64 u64PublishTimeUs =
                        app_ipcam_Ai_PD_TimeUs();
                    SMT_MutexAutoLock(g_PDMutex, lock);

                    if (g_stPDObjDraw.info != NULL &&
                        (!bHavePublishedDraw ||
                         !app_ipcam_Ai_PD_DrawResult_IsSame(
                            &stFollowDraw, &stLastPublishedDraw))) {
                        app_ipcam_Ai_PD_DrawResult_Copy(
                            &g_stPDObjDraw, &stFollowDraw);
                        app_ipcam_Ai_PD_DrawResult_Copy(
                            &stLastPublishedDraw, &stFollowDraw);
                        g_stPDResultFrame.sequence = stProfile.sequence;
                        g_stPDResultFrame.time_ref = stProfile.time_ref;
                        g_stPDResultFrame.pts = stProfile.pts;
                        g_stPDResultFrame.publish_time_us =
                            u64PublishTimeUs;
                        bHavePublishedDraw = CVI_TRUE;
                        bFollowPublishOsd = CVI_TRUE;
                    }
                }
#ifdef OSDC_SUPPORT
                if (bFollowPublishOsd) {
                    app_ipcam_Osdc_PdRect_Publish();
                }
#endif
            }
            stPreviousFollowFrame = stCurrentFollowFrame;
        } else {
            stPreviousFollowFrame.valid = CVI_FALSE;
        }
#endif
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);

        TDLObject obj_meta;
        memset(&obj_meta, 0, sizeof(TDLObject));
        s32Ret = g_pfpPDInference(g_PDAiHandle, g_pstPdCfg->model_id,
            image_handle, &obj_meta);
        if (s32Ret == CVI_SUCCESS) {
            if (obj_meta.info != NULL) {
                for (CVI_U32 i = 0; i < obj_meta.size; i++) {
                    if (obj_meta.info[i].score >=
                        g_pstPdCfg->threshold) {
                        stProfile.detected_objects++;
                    }
                }
            }
        }
        u32FrameCount++;

        CVI_S32 s32ReleaseRet = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32ReleaseRet != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n",
                VpssGrp, VpssChn, s32ReleaseRet);
        }
        TDL_DestroyImage(image_handle);
        app_ipcam_Ai_PD_FrameActive_Clear();

        if (s32Ret != CVI_SUCCESS) {
            app_ipcam_Ai_PD_ProfileComplete(
                u32FrameCount, u64LoopStart, &stProfile);
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "PD inference failed: frame=%u ret=%#x model=%d\n",
                u32FrameCount, s32Ret, g_pstPdCfg->model_id);
            if (obj_meta.info != NULL) {
                TDL_ReleaseObjectMeta(&obj_meta);
            }
            continue;
        }

        if (obj_meta.size > 0 && obj_meta.info == NULL) {
            app_ipcam_Ai_PD_ProfileComplete(
                u32FrameCount, u64LoopStart, &stProfile);
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "PD invalid metadata: frame=%u objects=%u info=NULL\n",
                u32FrameCount, obj_meta.size);
            continue;
        }

        app_ipcam_Ai_PD_DrawResult_Build(
            astDisplaySlots, &obj_meta, &u64NextDisplayId,
            u32FrameCount, stProfile.pts, 0,
            &stDetectionDraw, &stProfile);
        stProfile.draw_objects = stDetectionDraw.size;
        {
            CVI_U64 u64PublishTimeUs = app_ipcam_Ai_PD_TimeUs();
            SMT_MutexAutoLock(g_PDMutex, lock);

            if (g_stPDObjDraw.info != NULL &&
                (!bHavePublishedDraw ||
                 !app_ipcam_Ai_PD_DrawResult_IsSame(
                    &stDetectionDraw, &stLastPublishedDraw))) {
                app_ipcam_Ai_PD_DrawResult_Copy(
                    &g_stPDObjDraw, &stDetectionDraw);
                app_ipcam_Ai_PD_DrawResult_Copy(
                    &stLastPublishedDraw, &stDetectionDraw);
                g_stPDResultFrame.sequence = stProfile.sequence;
                g_stPDResultFrame.time_ref = stProfile.time_ref;
                g_stPDResultFrame.pts = stProfile.pts;
                g_stPDResultFrame.publish_time_us = u64PublishTimeUs;
                bHavePublishedDraw = CVI_TRUE;
                bPublishOsd = CVI_TRUE;
            }
        }
#ifdef OSDC_SUPPORT
        if (bPublishOsd) {
            app_ipcam_Osdc_PdRect_Publish();
        }
#endif
        app_ipcam_Ai_PD_ProfileComplete(
            u32FrameCount, u64LoopStart, &stProfile);

        if (stProfile.detected_objects == 0) {
            if (obj_meta.info != NULL) {
                TDL_ReleaseObjectMeta(&obj_meta);
            }
            continue;
        }

        TDL_ReleaseObjectMeta(&obj_meta);
    }

    app_ipcam_Ai_PD_FrameActive_Clear();

    pthread_exit(NULL);

    return NULL;
}


int app_ipcam_Ai_PD_ObjDrawInfo_GetWithFrame(
    TDLObject *pstAiObj, APP_AI_RESULT_FRAME_INFO_S *pstFrameInfo)
{
    _NULL_POINTER_CHECK_(pstAiObj, -1);

    if (app_ipcam_Ai_PD_Pause_Get()) {
        pstAiObj->size = 0;
        if (pstFrameInfo != NULL) {
            memset(pstFrameInfo, 0, sizeof(*pstFrameInfo));
        }
        return CVI_SUCCESS;
    }

    if (pstAiObj->info == NULL) {
        pstAiObj->info = malloc(
            APP_PD_MAX_DRAW_OBJECTS * sizeof(TDLObjectInfo));
        if (pstAiObj->info == NULL) {
            pstAiObj->size = 0;
            return CVI_FAILURE;
        }
    }

    SMT_MutexAutoLock(g_PDMutex, lock);
    pstAiObj->size = 0;
    pstAiObj->width = g_stPDObjDraw.width;
    pstAiObj->height = g_stPDObjDraw.height;
    if (pstFrameInfo != NULL) {
        *pstFrameInfo = g_stPDResultFrame;
    }
    if (g_stPDObjDraw.size > 0 && g_stPDObjDraw.info != NULL) {
        pstAiObj->size = g_stPDObjDraw.size <= APP_PD_MAX_DRAW_OBJECTS ?
            g_stPDObjDraw.size : APP_PD_MAX_DRAW_OBJECTS;
        memcpy(pstAiObj->info, g_stPDObjDraw.info,
            pstAiObj->size * sizeof(TDLObjectInfo));
    }
    return CVI_SUCCESS;
}

int app_ipcam_Ai_PD_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    return app_ipcam_Ai_PD_ObjDrawInfo_GetWithFrame(pstAiObj, NULL);
}


int app_ipcam_Ai_PD_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstPdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_PD_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_PD_ProcStatus_Set(CVI_FALSE);
    app_ipcam_Ai_PD_FrameSource_Stop();

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_PDThreadHandle)
    {
        pthread_join(g_PDThreadHandle, NULL);
        g_PDThreadHandle = 0;
    }

    {
        SMT_MutexAutoLock(g_PDMutex, lock);
        free(g_stPDObjDraw.info);
        g_stPDObjDraw.info = NULL;
        g_stPDObjDraw.size = 0;
        g_stPDObjDraw.width = 0;
        g_stPDObjDraw.height = 0;
        memset(&g_stPDResultFrame, 0, sizeof(g_stPDResultFrame));
    }
#ifdef OSDC_SUPPORT
    app_ipcam_Osdc_PdRect_Publish();
#endif

    TDL_CloseModel(g_PDAiHandle, g_pstPdCfg->model_id);
    s32Ret = TDL_DestroyHandle(g_PDAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_PDAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_PD_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_VPSS_GRP_CFG_T *pstVpssCfg = &app_ipcam_Vpss_Param_Get()->astVpssGrpCfg[0];

    if (!g_pstPdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bPDRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_PD_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_PD_Proc_Init failed!\n");
        return s32Ret;
    }
    
    g_PDScaleX = (float)pstVpssCfg->astVpssChnAttr[0].u32Width /
        (float)g_pstPdCfg->u32GrpWidth;
    g_PDScaleY = (float)pstVpssCfg->astVpssChnAttr[0].u32Height /
        (float)g_pstPdCfg->u32GrpHeight;

    app_ipcam_Ai_PD_Pause_Set(CVI_FALSE);
    s32Ret = app_ipcam_Ai_PD_FrameSource_Start(
        g_pstPdCfg->VpssGrp, g_pstPdCfg->VpssChn);
    if (s32Ret != CVI_SUCCESS) {
        return s32Ret;
    }

    app_ipcam_Ai_PD_ProcStatus_Set(CVI_TRUE);
#ifdef OSDC_SUPPORT
    if (app_ipcam_Osdc_PdRect_EnsureEventMode() != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "DET event OSD unavailable, using canvas fallback\n");
    }
#endif
    s32Ret = pthread_create(&g_PDThreadHandle, NULL, Thread_PD_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
        app_ipcam_Ai_PD_ProcStatus_Set(CVI_FALSE);
        app_ipcam_Ai_PD_FrameSource_Stop();
        return s32Ret;
    }
    
    return CVI_SUCCESS;
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/
CVI_S32 app_ipcam_Ai_PD_StatusGet(void)
{
    return g_PDAiHandle ? 1 : 0;
}

CVI_S32 app_ipcam_Pd_threshold_Set(float threshold)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_PDAiHandle) {
        s32Ret = TDL_SetModelThreshold(g_PDAiHandle,
            g_pstPdCfg->model_id,
            app_ipcam_Ai_PD_CandidateThreshold(threshold));
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelThreshold failed with %#x!\n",g_pstPdCfg->model_path, s32Ret);
            return s32Ret;
        }
    }
    g_pstPdCfg->threshold = threshold;

    return s32Ret;
}
