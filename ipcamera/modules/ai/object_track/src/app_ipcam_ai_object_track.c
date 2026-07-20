#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "stdbool.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "app_ipcam_ai.h"
#include "app_ipcam_sys.h"
#include "tdl_sdk.h"
#include <pthread.h>
#include <stdio.h>
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define FEATURE_SIZE 256
#define MAX_DET_NUM 100
#define TRACK_REQUEST_PATH "/tmp/track"
#define TRACK_FPS_STATUS_PATH "/tmp/object_track_fps"
#define TRACK_FPS_STATUS_TMP_PATH "/tmp/object_track_fps.tmp"
#define TRACK_PERF_WINDOW_US (1000ULL * 1000ULL)
#define DEFAULT_SELECTION_BOX_SIZE 120
#define OBJECT_TRACK_REFERENCE_CHN 0
#define OBJECT_TRACK_SOT_POOL 7

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
} APP_OBJECT_TRACK_REQUEST_TYPE_E;

typedef struct APP_OBJECT_TRACK_REQUEST_T {
    APP_OBJECT_TRACK_REQUEST_TYPE_E type;
    int32_t point_x;
    int32_t point_y;
    int32_t view_width;
    int32_t view_height;
    int32_t box[4];
    uint64_t track_id;
} APP_OBJECT_TRACK_REQUEST_S;

typedef struct APP_OBJECT_TRACK_SELECTION_T {
    CVI_BOOL valid;
    CVI_BOOL from_det;
    CVI_BOOL point_prompt;
    int32_t class_id;
    uint64_t track_id;
    int32_t point[2];
    int32_t box[4];
} APP_OBJECT_TRACK_SELECTION_S;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_OBJECT_TRACK_MODE g_mode = DETECTION;
static uint64_t g_frame_id = 0;
static uint32_t g_lost_start_time;          // Start time when the object is lost
static bool g_lost_timer_started = false;   // Whether the lost timer has started
static APP_PARAM_AI_OBJECT_TRACK_CFG_S g_stObjTrackCfg;
static APP_PARAM_AI_OBJECT_TRACK_CFG_S *g_pstObjTrackCfg = &g_stObjTrackCfg;
static volatile bool g_bObjectTrackRunning = CVI_FALSE;
static volatile bool g_bObjectTrackPause = CVI_FALSE;
static pthread_t g_ObjectTrackHandle;
static TDLHandle g_ObjectTrackTDLHandle;
static TDLObject g_stObjDraw = {0};
static CVI_U32 g_u32DetInputWidth = 0;
static CVI_U32 g_u32DetInputHeight = 0;
SMT_MUTEXAUTOLOCK_INIT(g_Mutex);
static pthread_mutex_t g_StatusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_ModeMutex = PTHREAD_MUTEX_INITIALIZER;
static CVI_BOOL g_bPipelineInitialized = CVI_FALSE;
static CVI_BOOL g_bSharedPipelinePrepared = CVI_FALSE;
static APP_PARAM_OBJECT_TRACK_MODE g_PipelineMode = DETECTION;
static VPSS_CHN_ATTR_S g_stDetPipelineAttr = {0};
static VPSS_CHN_ATTR_S g_stSotPipelineAttr = {0};
static VB_POOL g_DetPipelinePool = VB_INVALID_POOLID;
static VB_POOL g_SotPipelinePool = VB_INVALID_POOLID;
const int LOST_TIMEOUT_SECONDS = 5;  // Timeout for object lost

typedef struct APP_OBJECT_TRACK_PERF_T {
    APP_PARAM_OBJECT_TRACK_MODE mode;
    uint64_t window_start_us;
    uint64_t input_frames;
    uint64_t det_frames;
    uint64_t sot_frames;
    uint64_t sot_init_frames;
    uint64_t frame_wait_total_us;
    uint64_t det_total_us;
    uint64_t sot_total_us;
    uint64_t sot_init_total_us;
} APP_OBJECT_TRACK_PERF_S;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static uint32_t get_time_in_ms() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) {
    return 0;
  }
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static uint64_t app_ipcam_Ai_Object_Track_TimeUs(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ULL * 1000ULL +
           (uint64_t)ts.tv_nsec / 1000ULL;
}

static CVI_VOID app_ipcam_Ai_Object_Track_Perf_Reset(
    APP_OBJECT_TRACK_PERF_S *pstPerf, APP_PARAM_OBJECT_TRACK_MODE mode,
    uint64_t now_us)
{
    memset(pstPerf, 0, sizeof(*pstPerf));
    pstPerf->mode = mode;
    pstPerf->window_start_us = now_us;
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
            "frame_wait_avg_ms=%.3f\n"
            "det_avg_ms=%.3f\n"
            "sot_avg_ms=%.3f\n"
            "sot_init_avg_ms=%.3f\n"
            "template_input_cache=1\n"
            "direct_search_tensor=1\n",
            pstPerf->mode == TRACKING ? "tracking" : "detection",
            pstPerf->input_frames / elapsed_seconds,
            pstPerf->det_frames / elapsed_seconds,
            pstPerf->sot_frames / elapsed_seconds,
            pstPerf->input_frames > 0 ?
                (double)pstPerf->frame_wait_total_us /
                    pstPerf->input_frames / 1000.0 : 0.0,
            pstPerf->det_frames > 0 ?
                (double)pstPerf->det_total_us /
                    pstPerf->det_frames / 1000.0 : 0.0,
            pstPerf->sot_frames > 0 ?
                (double)pstPerf->sot_total_us /
                    pstPerf->sot_frames / 1000.0 : 0.0,
            pstPerf->sot_init_frames > 0 ?
                (double)pstPerf->sot_init_total_us /
                    pstPerf->sot_init_frames / 1000.0 : 0.0);
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
    if (pstSysCfg->vb_pool_num <= OBJECT_TRACK_SOT_POOL ||
        !pstSysCfg->vb_pool[OBJECT_TRACK_SOT_POOL].bEnable ||
        pstSysCfg->vb_pool[OBJECT_TRACK_SOT_POOL].width !=
            g_pstObjTrackCfg->u32SotGrpWidth ||
        pstSysCfg->vb_pool[OBJECT_TRACK_SOT_POOL].height !=
            g_pstObjTrackCfg->u32SotGrpHeight ||
        pstSysCfg->vb_pool[OBJECT_TRACK_SOT_POOL].fmt != PIXEL_FORMAT_NV12) {
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
    g_SotPipelinePool = OBJECT_TRACK_SOT_POOL;

    g_bSharedPipelinePrepared = CVI_TRUE;
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "ObjectTrack AI VPSS channel: grp=%d chn=%d, DET pool=%u, SOT pool=%u\n",
        g_pstObjTrackCfg->VpssGrp, g_pstObjTrackCfg->VpssChn,
        g_DetPipelinePool, g_SotPipelinePool);
    return CVI_SUCCESS;
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
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "switch ObjectTrack VPSS pipeline to %s failed with %#x\n",
            mode == TRACKING ? "tracking" : "detection", s32Ret);
        return s32Ret;
    }

    if (mode == TRACKING) {
        SMT_MutexAutoLock(g_Mutex, lock);
        g_stObjDraw.size = 0;
    }
    g_PipelineMode = mode;
    g_bPipelineInitialized = CVI_TRUE;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "ObjectTrack VPSS pipeline: %s\n",
        mode == TRACKING ? "tracking" : "detection");
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

static CVI_BOOL app_ipcam_Ai_Object_Track_Request_Load(
    APP_OBJECT_TRACK_REQUEST_S *pstRequest)
{
    char text[128] = {0};
    FILE *pFile = NULL;

    if (pstRequest == NULL) {
        return CVI_FALSE;
    }
    pFile = fopen(TRACK_REQUEST_PATH, "r");
    if (pFile == NULL) {
        return CVI_FALSE;
    }
    if (fgets(text, sizeof(text), pFile) == NULL) {
        fclose(pFile);
        unlink(TRACK_REQUEST_PATH);
        return CVI_FALSE;
    }
    fclose(pFile);
    unlink(TRACK_REQUEST_PATH);
    return app_ipcam_Ai_Object_Track_Request_Parse(text, pstRequest);
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
        pstSelection->valid = CVI_TRUE;
        pstSelection->from_det = CVI_TRUE;
        pstSelection->class_id = pstObject->info[i].class_id;
        pstSelection->track_id = track_id;
        pstSelection->box[0] = (int32_t)pstObject->info[i].box.x1;
        pstSelection->box[1] = (int32_t)pstObject->info[i].box.y1;
        pstSelection->box[2] = (int32_t)pstObject->info[i].box.x2;
        pstSelection->box[3] = (int32_t)pstObject->info[i].box.y2;
        app_ipcam_Ai_Object_Track_Box_Clamp(pstSelection->box);
        return CVI_TRUE;
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

    if (pstRequest->type == APP_OBJECT_TRACK_REQUEST_ID) {
        return app_ipcam_Ai_Object_Track_Select_Det_By_Id(
            pstObject, pstRequest->track_id, pstSelection);
    }
    if (pstRequest->type == APP_OBJECT_TRACK_REQUEST_POINT) {
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
    pthread_mutex_lock(&g_ModeMutex);
    g_mode = mode;
    pthread_mutex_unlock(&g_ModeMutex);
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
    VPSS_GRP VpssGrp = g_pstObjTrackCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstObjTrackCfg->VpssChn;
    VPSS_GRP SotVpssGrp = g_pstObjTrackCfg->SotVpssGrp;
    VPSS_CHN SotVpssChn = g_pstObjTrackCfg->SotVpssChn;
    TDLObject det_obj_meta = {0};
    TDLObject cur_det_meta = {0};
    TDLTracker track_meta = {0};
    APP_OBJECT_TRACK_REQUEST_S pending_request = {0};
    APP_OBJECT_TRACK_SELECTION_S selection = {0};
    bool track_init = CVI_FALSE;
    bool pending_request_valid = false;
    bool preprocessed_input_error_reported = false;
    bool sot_input_error_reported = false;
    APP_OBJECT_TRACK_PERF_S stPerf = {0};

    (void)pArgs;
    g_bPipelineInitialized = CVI_FALSE;
    g_bSharedPipelinePrepared = CVI_FALSE;
    unlink(TRACK_FPS_STATUS_PATH);
    unlink(TRACK_FPS_STATUS_TMP_PATH);
    app_ipcam_Ai_Object_Track_Perf_Reset(
        &stPerf, DETECTION, app_ipcam_Ai_Object_Track_TimeUs());

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
     * 单线程主循环调度说明：
     * 1. 每次循环只取一次 VPSS 帧，DET 和 TRACK 分别使用专用通道。
     * 2. DET 使用模型输入尺寸，TRACK 使用高分辨率 NV12，并统一换算 OSD 坐标。
     * 3. /tmp/track 是一次性目标选择命令；stop、丢失或失败时回检测态。
     * 4. 统一在循环末尾释放 frame/image/meta，保证资源生命周期稳定可控。
     */
    while (app_ipcam_Ai_Object_Track_ProcStatus_Get()) {
        VIDEO_FRAME_INFO_S stFrame = {0};
        TDLImage image = NULL;
        VPSS_GRP FrameVpssGrp = VpssGrp;
        VPSS_CHN FrameVpssChn = VpssChn;
        APP_PARAM_OBJECT_TRACK_MODE mode = DETECTION;
        APP_OBJECT_TRACK_REQUEST_S request = {0};
        bool frame_acquired = false;
        bool det_input_preprocessed = false;
        uint64_t frame_wait_start_us = 0;
        uint64_t inference_start_us = 0;

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
                g_lost_timer_started = false;
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "ObjectTrack stop request\n");
            } else {
                pending_request = request;
                pending_request_valid = true;
                memset(&selection, 0, sizeof(selection));
                track_init = CVI_FALSE;
                g_lost_timer_started = false;
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "ObjectTrack target request type=%d\n", request.type);
            }
        }

        mode = app_ipcam_Ai_Object_Track_Mode_Get();
        if (app_ipcam_Ai_Object_Track_Pipeline_Set(mode) != CVI_SUCCESS) {
            usleep(10 * 1000);
            continue;
        }
        if (stPerf.mode != mode) {
            app_ipcam_Ai_Object_Track_Perf_Reset(
                &stPerf, mode, app_ipcam_Ai_Object_Track_TimeUs());
        }
        if (mode == TRACKING) {
            FrameVpssGrp = SotVpssGrp;
            FrameVpssChn = SotVpssChn;
        }

        frame_wait_start_us = app_ipcam_Ai_Object_Track_TimeUs();
        s32Ret = CVI_VPSS_GetChnFrame(FrameVpssGrp, FrameVpssChn, &stFrame, 3000);
        if (s32Ret != 0){
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "Grp(%d)-Chn(%d) get frame failed with %#x\n",
                               FrameVpssGrp, FrameVpssChn, s32Ret);
            continue;
        }
        frame_acquired = true;
        stPerf.input_frames++;
        stPerf.frame_wait_total_us +=
            app_ipcam_Ai_Object_Track_TimeUs() - frame_wait_start_us;

        g_frame_id++;

        det_input_preprocessed =
            g_pstObjTrackCfg->bDetInputPreprocessed &&
            mode == DETECTION;
        if (det_input_preprocessed &&
            (stFrame.stVFrame.enPixelFormat != PIXEL_FORMAT_UINT8_C3_PLANAR ||
             stFrame.stVFrame.u32Width != g_u32DetInputWidth ||
             stFrame.stVFrame.u32Height != g_u32DetInputHeight)) {
            if (!preprocessed_input_error_reported) {
                APP_PROF_LOG_PRINT(
                    LEVEL_ERROR,
                    "preprocessed DET input mismatch: got %ux%u format=%d, expected %ux%u PIXEL_FORMAT_UINT8_C3_PLANAR\n",
                    stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,
                    stFrame.stVFrame.enPixelFormat, g_u32DetInputWidth,
                    g_u32DetInputHeight);
                preprocessed_input_error_reported = true;
            }
            goto loop_cleanup;
        }

        if (mode == TRACKING &&
            (stFrame.stVFrame.enPixelFormat != PIXEL_FORMAT_NV12 ||
             stFrame.stVFrame.u32Width != g_pstObjTrackCfg->u32SotGrpWidth ||
             stFrame.stVFrame.u32Height != g_pstObjTrackCfg->u32SotGrpHeight)) {
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

        image = TDL_WrapFrame(&stFrame, true, det_input_preprocessed);
        if (image == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Failed to wrap frame \n");
            goto loop_cleanup;
        }

        /*
         * 检测阶段：每帧更新检测结果缓存 det_obj_meta，并同步更新 OSD 绘制数据。
         * det_obj_meta 仅在本线程内读写，为后续目标选择保留最近一次检测结果。
         */
        if (mode == DETECTION) {
            inference_start_us = app_ipcam_Ai_Object_Track_TimeUs();
            s32Ret = TDL_Detection(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det, image, &cur_det_meta);
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_Detection failed with %#x!\n", s32Ret);
                goto loop_cleanup;
            }
            stPerf.det_frames++;
            stPerf.det_total_us +=
                app_ipcam_Ai_Object_Track_TimeUs() - inference_start_us;
            // What changed: Gate test-only [OBS] detection count by debug_log_enable.
            // Previous behavior: Only LEVEL_DEBUG "Detect N objects" existed and was compiled out.
            // Impact: Production logs stay quiet unless tracking debug is enabled.
            // Debug params: det_n=detected object count this frame; used to confirm detector running pre-trigger.
            if (g_pstObjTrackCfg->debug_log_enable) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[OBS] f=%llu det_n=%d\n",
                                   (unsigned long long)g_frame_id, (int)cur_det_meta.size);
            }

            TDL_ReleaseObjectMeta(&det_obj_meta);
            TDL_CopyObjectMeta(&cur_det_meta, &det_obj_meta);
            if (det_obj_meta.size > 0 && det_obj_meta.info == NULL) {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "det_obj_meta copy invalid, reset to empty\n");
                det_obj_meta.size = 0;
            }

            {
                SMT_MutexAutoLock(g_Mutex, lock);
                g_stObjDraw.size = 0;
                if (cur_det_meta.size > 0 &&
                    cur_det_meta.info != NULL &&
                    g_stObjDraw.info != NULL) {
                    g_stObjDraw.size = cur_det_meta.size <= MAX_DET_NUM ? cur_det_meta.size : MAX_DET_NUM;
                    memcpy(g_stObjDraw.info, cur_det_meta.info, g_stObjDraw.size * sizeof(TDLObjectInfo));
                }
            }

            if (pending_request_valid) {
                if (app_ipcam_Ai_Object_Track_Select(
                        &det_obj_meta, &pending_request, &selection)) {
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
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_WARN,
                        "ObjectTrack target selection failed, stay in detection mode\n");
                    memset(&selection, 0, sizeof(selection));
                }
                app_ipcam_Ai_Object_Track_Request_Default(&pending_request);
                pending_request_valid = false;
            }

            goto loop_cleanup;
        }

        /*
         * 追踪初始化阶段：读取一次 /tmp/track 事件并调用 SetSingleObjectTracking。
         * 初始化成功后切换到持续追踪；失败则清理事件并回到检测态。
         */
        if (!track_init) {
            const char *model_path = NULL;
            TDLTargetSearchTypeE search_type = g_pstObjTrackCfg->search_type;
            TDLObject empty_det_meta = {0};
            int32_t set_values[4] = {0};
            int32_t set_value_count = 4;
            const char *selection_source = NULL;

            if (!selection.valid) {
                APP_PROF_LOG_PRINT(LEVEL_WARN,
                    "ObjectTrack has no valid selection, return to detection\n");
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
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
                set_value_count = 2;
                selection_source = "point";
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "ObjectTrack initialize source=%s prompt=[%d,%d] search_type=%d\n",
                    selection_source, set_values[0], set_values[1], search_type);
            } else {
                memcpy(set_values, selection.box, sizeof(set_values));
                app_ipcam_Ai_Object_Track_Box_Scale(
                    set_values, g_pstObjTrackCfg->u32GrpWidth,
                    g_pstObjTrackCfg->u32GrpHeight,
                    g_pstObjTrackCfg->u32SotGrpWidth,
                    g_pstObjTrackCfg->u32SotGrpHeight);
                selection_source = selection.from_det ? "DET" : "box";
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "ObjectTrack initialize source=%s box=[%d,%d,%d,%d] search_type=%d\n",
                    selection_source, set_values[0], set_values[1],
                    set_values[2], set_values[3], search_type);
            }

            inference_start_us = app_ipcam_Ai_Object_Track_TimeUs();
            s32Ret = TDL_SetSingleObjectTracking(
                g_ObjectTrackTDLHandle, image, &empty_det_meta, set_values,
                set_value_count, g_frame_id, search_type, model_path);
            stPerf.sot_init_frames++;
            stPerf.sot_init_total_us +=
                app_ipcam_Ai_Object_Track_TimeUs() - inference_start_us;
            if (s32Ret != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTracking failed with %#x!\n", s32Ret);
                app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                track_init = CVI_FALSE;
                memset(&selection, 0, sizeof(selection));
            } else {
                track_init = CVI_TRUE;
                g_lost_timer_started = false;
            }
        } else {
            /*
             * 持续追踪阶段：每帧调用 SingleObjectTracking 并更新单目标框。
             * 当连续 LOST 超时后，按原逻辑回退检测态并等待下一次外部触发。
             */
            inference_start_us = app_ipcam_Ai_Object_Track_TimeUs();
            s32Ret = TDL_SingleObjectTracking(g_ObjectTrackTDLHandle, image, &track_meta, g_frame_id);
            if (s32Ret != 0) {
               APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SingleObjectTracking failed with %#x!\n", s32Ret);
               app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
               track_init = CVI_FALSE;
               memset(&selection, 0, sizeof(selection));
               goto loop_cleanup;
            }
            stPerf.sot_frames++;
            stPerf.sot_total_us +=
                app_ipcam_Ai_Object_Track_TimeUs() - inference_start_us;

            // What changed: Gate test-only [OBS] SOT score/threshold/pass/box print.
            // Previous behavior: Only LEVEL_DEBUG "track score lower than threshold" existed and was compiled out.
            // Impact: Production logs stay quiet unless tracking debug is enabled.
            // Debug params: f=frame id; score=SOT confidence; thr=ini gate; pass=score>=thr; box=track bbox;
            //               used to compare threshold tightness and kalman effect across groups.
            if (g_pstObjTrackCfg->debug_log_enable) {
                if (track_meta.info != NULL) {
                    APP_PROF_LOG_PRINT(LEVEL_INFO,
                        "[OBS] f=%llu score=%.3f thr=%.3f pass=%d box=[%.0f,%.0f,%.0f,%.0f]\n",
                        (unsigned long long)g_frame_id,
                        track_meta.info[0].score,
                        g_pstObjTrackCfg->tracking_score_threshold,
                        (track_meta.info[0].score >= g_pstObjTrackCfg->tracking_score_threshold) ? 1 : 0,
                        (float)track_meta.info[0].bbox.x1, (float)track_meta.info[0].bbox.y1,
                        (float)track_meta.info[0].bbox.x2, (float)track_meta.info[0].bbox.y2);
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_INFO,
                        "[OBS] f=%llu info=NULL(LOST)\n", (unsigned long long)g_frame_id);
                }
            }
            if (track_meta.info != NULL &&
                track_meta.info[0].score >= g_pstObjTrackCfg->tracking_score_threshold) {
                g_lost_timer_started = false;
                SMT_MutexAutoLock(g_Mutex, lock);
                if (g_stObjDraw.info != NULL) {
                    float scale_x = (float)g_pstObjTrackCfg->u32GrpWidth /
                                    g_pstObjTrackCfg->u32SotGrpWidth;
                    float scale_y = (float)g_pstObjTrackCfg->u32GrpHeight /
                                    g_pstObjTrackCfg->u32SotGrpHeight;
                    g_stObjDraw.size = 1;
                    g_stObjDraw.info[0].box.x1 = track_meta.info[0].bbox.x1 * scale_x;
                    g_stObjDraw.info[0].box.x2 = track_meta.info[0].bbox.x2 * scale_x;
                    g_stObjDraw.info[0].box.y1 = track_meta.info[0].bbox.y1 * scale_y;
                    g_stObjDraw.info[0].box.y2 = track_meta.info[0].bbox.y2 * scale_y;
                } else {
                    g_stObjDraw.size = 0;
                }
            } else {
                if (track_meta.info != NULL) {
                    // What changed: Log weak tracking scores before lost handling.
                    // Previous behavior: Low-score tracking boxes had no visibility.
                    // Impact: Helps tune tracking_score_threshold for board scenes.
                    // Debug params: score means SOT confidence, used to verify weak-track filtering;
                    // threshold means ini gate, used to tune lost detection.
                    APP_PROF_LOG_PRINT(LEVEL_DEBUG,
                                       "track score %.3f lower than threshold %.3f\n",
                                       track_meta.info[0].score,
                                       g_pstObjTrackCfg->tracking_score_threshold);
                }
                if (!g_lost_timer_started) {
                    g_lost_start_time = get_time_in_ms();
                    g_lost_timer_started = true;
                } else {
                    uint32_t current_time = get_time_in_ms();
                    uint32_t elapsed_time = current_time - g_lost_start_time;
                    if (elapsed_time >= (uint32_t)(LOST_TIMEOUT_SECONDS * 1000)) {
                        APP_PROF_LOG_PRINT(LEVEL_WARN,
                                           "The target has been lost for more than [%d] seconds, "
                                           "switching to detection state\n",
                                           LOST_TIMEOUT_SECONDS);
                        app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
                        g_lost_timer_started = false;
                        track_init = CVI_FALSE;
                        memset(&selection, 0, sizeof(selection));
                    }
                }
            }
        }

loop_cleanup:
        if (track_meta.info != NULL) {
            TDL_ReleaseTrackMeta(&track_meta);
            memset(&track_meta, 0, sizeof(track_meta));
        }
        if (cur_det_meta.info != NULL) {
            TDL_ReleaseObjectMeta(&cur_det_meta);
            memset(&cur_det_meta, 0, sizeof(cur_det_meta));
        }
        if (image != NULL) {
            TDL_DestroyImage(image);
        }
        if (frame_acquired) {
            s32Ret = CVI_VPSS_ReleaseChnFrame(FrameVpssGrp, FrameVpssChn, &stFrame);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                                   "Grp(%d)-Chn(%d) release frame failed with %#x\n",
                                   FrameVpssGrp, FrameVpssChn, s32Ret);
            }
        }
        app_ipcam_Ai_Object_Track_Perf_Write(
            &stPerf, app_ipcam_Ai_Object_Track_TimeUs());
    }

    if (app_ipcam_Ai_Object_Track_Pipeline_Set(DETECTION) != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "restore ObjectTrack detection pipeline failed\n");
    }
    TDL_ReleaseObjectMeta(&det_obj_meta);
    unlink(TRACK_FPS_STATUS_TMP_PATH);
    pthread_exit(NULL);

    return NULL;
}

static CVI_S32 app_ipcam_Ai_Object_Track_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI ObjectTrack init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;
    TDLPreprocessParams stDetPreprocessParams = {0};

    if (g_pstObjTrackCfg->u32GrpWidth == 0 ||
        g_pstObjTrackCfg->u32GrpHeight == 0 ||
        g_pstObjTrackCfg->u32SotGrpWidth == 0 ||
        g_pstObjTrackCfg->u32SotGrpHeight == 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "invalid ObjectTrack frame size: DET=%ux%u SOT=%ux%u\n",
                           g_pstObjTrackCfg->u32GrpWidth,
                           g_pstObjTrackCfg->u32GrpHeight,
                           g_pstObjTrackCfg->u32SotGrpWidth,
                           g_pstObjTrackCfg->u32SotGrpHeight);
        return CVI_FAILURE;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "ObjectTrack pipeline: DET Grp(%d)-Chn(%d) %ux%u, "
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

    s32Ret = TDL_OpenModelSkipInputAlloc(
        g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det,
        g_pstObjTrackCfg->model_path_det, g_pstObjTrackCfg->model_path_cfg, 0,
        g_pstObjTrackCfg->bDetInputPreprocessed);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel DET failed with %#x!\n", s32Ret);
        goto init_failed;
    }else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "TDL_OpenModel DET success, preprocessed_input=%d !\n",
                           g_pstObjTrackCfg->bDetInputPreprocessed);
    }

    if (g_pstObjTrackCfg->bDetInputPreprocessed) {
        s32Ret = TDL_GetPreprocessParameters(
            g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det,
            &stDetPreprocessParams);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "TDL_GetPreprocessParameters DET failed with %#x!\n",
                               s32Ret);
            goto init_failed;
        }
        g_u32DetInputWidth = stDetPreprocessParams.dst_width;
        g_u32DetInputHeight = stDetPreprocessParams.dst_height;
        if (g_pstObjTrackCfg->u32GrpWidth != g_u32DetInputWidth ||
            g_pstObjTrackCfg->u32GrpHeight != g_u32DetInputHeight) {
            APP_PROF_LOG_PRINT(
                LEVEL_ERROR,
                "preprocessed DET config mismatch: configured %ux%u, model expects %ux%u\n",
                g_pstObjTrackCfg->u32GrpWidth,
                g_pstObjTrackCfg->u32GrpHeight, g_u32DetInputWidth,
                g_u32DetInputHeight);
            s32Ret = CVI_FAILURE;
            goto init_failed;
        }
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

    s32Ret = TDL_SetSingleObjectTrackingUseKalman(g_ObjectTrackTDLHandle,
                                                  g_pstObjTrackCfg->use_kalman);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetSingleObjectTrackingUseKalman failed with %#x!\n", s32Ret);
        goto init_failed;
    }

    if (g_pstObjTrackCfg->debug_log_enable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "[OBS] cfg use_kalman=%d tracking_score_threshold=%.3f search_type=%d kalman_ret=0x%x\n",
            g_pstObjTrackCfg->use_kalman ? 1 : 0,
            g_pstObjTrackCfg->tracking_score_threshold,
            g_pstObjTrackCfg->search_type,
            s32Ret);
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
    g_u32DetInputWidth = 0;
    g_u32DetInputHeight = 0;
    g_bPipelineInitialized = CVI_FALSE;
    g_bSharedPipelinePrepared = CVI_FALSE;
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

    app_ipcam_Ai_Object_Track_Mode_Set(DETECTION);
    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_FALSE);
    pthread_join(g_ObjectTrackHandle, NULL);
    g_ObjectTrackHandle = 0;

    {
        SMT_MutexAutoLock(g_Mutex, lock);
        if (g_stObjDraw.info != NULL) {
            free(g_stObjDraw.info);
            g_stObjDraw.info = NULL;
        }
        g_stObjDraw.size = 0;
    }

    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_det);
    TDL_CloseModel(g_ObjectTrackTDLHandle, g_pstObjTrackCfg->model_id_sot);

    s32Ret = TDL_DestroyHandle(g_ObjectTrackTDLHandle);

    if(s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "enter TDL_DestroyHandle fail \n");
        return s32Ret;
    }
    g_ObjectTrackTDLHandle = NULL;
    g_u32DetInputWidth = 0;
    g_u32DetInputHeight = 0;
    g_bPipelineInitialized = CVI_FALSE;
    g_bSharedPipelinePrepared = CVI_FALSE;

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

    app_ipcam_Ai_Object_Track_ProcStatus_Set(CVI_TRUE);
    s32Ret = pthread_create(&g_ObjectTrackHandle, NULL, Thread_Object_Track_Proc, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI ObjectTrack_pthread_create failed!\n");
        return s32Ret;
    }

    return s32Ret;
}
