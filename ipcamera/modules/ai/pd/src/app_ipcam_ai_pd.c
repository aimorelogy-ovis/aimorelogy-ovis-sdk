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

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
SMT_MUTEXAUTOLOCK_INIT(g_PDMutex);
static pthread_mutex_t g_PDStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define APP_PD_DIAG_INTERVAL 30
#define APP_PD_MAX_DRAW_OBJECTS 100

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
    CVI_U64 loop_us;
    CVI_U32 dropped_frames;
    CVI_U32 sequence;
    CVI_U32 time_ref;
    CVI_U64 pts;
    TDLModelPerformance model;
} APP_PD_PROFILE_S;

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
static pfpInferenceFunc g_pfpPDInference;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_PD_CFG_S *app_ipcam_Ai_PD_Param_Get(void)
{
    return g_pstPdCfg;
}

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
    pthread_mutex_lock(&g_PDStatusMutex);
    g_bPDPause = flag;
    pthread_mutex_unlock(&g_PDStatusMutex);
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

static CVI_VOID app_ipcam_Ai_PD_ProfileLog(
    CVI_U32 frame_count, const APP_PD_PROFILE_S *profile)
{
    double loop_ms;
    double fps;

    if (frame_count != 1 && (frame_count % APP_PD_DIAG_INTERVAL) != 0) {
        return;
    }

    loop_ms = profile->loop_us / 1000.0;
    fps = profile->loop_us > 0 ? 1000000.0 / profile->loop_us : 0.0;
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "PD PERF frame=%u seq=%u ref=%u pts=%llu drop=%u "
        "wait=%.2f drain=%.2f wrap=%.2f pre=%.2f tpu=%.2f post=%.2f "
        "tdl=%.2f api=%.2f release=%.2f publish=%.2f loop=%.2f fps=%.2f\n",
        frame_count, profile->sequence, profile->time_ref,
        (unsigned long long)profile->pts, profile->dropped_frames,
        profile->wait_us / 1000.0, profile->drain_us / 1000.0,
        profile->wrap_us / 1000.0, profile->model.preprocess_ms,
        profile->model.tpu_ms, profile->model.postprocess_ms,
        profile->model.total_ms, profile->inference_us / 1000.0,
        profile->release_us / 1000.0, profile->publish_us / 1000.0,
        loop_ms, fps);
}

static CVI_VOID app_ipcam_Ai_PD_ProfileComplete(
    CVI_U32 frame_count, CVI_U64 loop_start, APP_PD_PROFILE_S *profile)
{
    profile->loop_us = app_ipcam_Ai_PD_TimeUs() - loop_start;
    g_PDProc = (CVI_U32)((profile->loop_us + 500) / 1000);
    g_PDFps = profile->loop_us > 0 ?
        (CVI_U32)(1000000 / profile->loop_us) : 0;
    app_ipcam_Ai_PD_ProfileLog(frame_count, profile);
}


static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel model_id)
{
    switch (model_id)
    {
        case TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON:
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
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold=%f\n", g_pstPdCfg->threshold);
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
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
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

    s32Ret = TDL_SetModelThreshold(g_PDAiHandle, g_pstPdCfg->model_id, g_pstPdCfg->threshold);
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
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_PD_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U32 u32FrameCount = 0;
    CVI_U32 u32InferenceErrors = 0;
    CVI_U32 u32EmptyFrames = 0;
    CVI_U32 u32DetectedFrames = 0;
    CVI_U32 u32StaleFrames = 0;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD start running!\n");

    prctl(PR_SET_NAME, "Thread_PD_PROC");

    VPSS_GRP VpssGrp = g_pstPdCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstPdCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    TDLImage image_handle ;

    while (app_ipcam_Ai_PD_ProcStatus_Get()) {
        APP_PD_PROFILE_S stProfile = {0};
        CVI_U64 u64LoopStart = app_ipcam_Ai_PD_TimeUs();
        CVI_U64 u64StageStart;

        pthread_mutex_lock(&g_PDStatusMutex);
        s32Ret = app_ipcam_Ai_PD_Pause_Get();
        
        if (s32Ret) {
            pthread_mutex_unlock(&g_PDStatusMutex);
            usleep(1000*1000);
            continue;
        } 
        u64StageStart = app_ipcam_Ai_PD_TimeUs();
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        stProfile.wait_us = app_ipcam_Ai_PD_TimeUs() - u64StageStart;
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret); 
            pthread_mutex_unlock(&g_PDStatusMutex);
            usleep(100*1000);
            continue;
        }
        u64StageStart = app_ipcam_Ai_PD_TimeUs();
        for (;;) {
            VIDEO_FRAME_INFO_S stLatestFrame = {0};
            CVI_S32 s32LatestRet = CVI_VPSS_GetChnFrame(
                VpssGrp, VpssChn, &stLatestFrame, 0);

            if (s32LatestRet != CVI_SUCCESS) {
                break;
            }
            s32LatestRet = CVI_VPSS_ReleaseChnFrame(
                VpssGrp, VpssChn, &stfdFrame);
            if (s32LatestRet != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "Grp(%d)-Chn(%d) release stale frame failed with %#x\n",
                    VpssGrp, VpssChn, s32LatestRet);
                CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stLatestFrame);
                break;
            }
            stfdFrame = stLatestFrame;
            u32StaleFrames++;
            stProfile.dropped_frames++;
        }
        stProfile.drain_us = app_ipcam_Ai_PD_TimeUs() - u64StageStart;
        stProfile.sequence = stfdFrame.stVFrame.u32SeqenceNo;
        stProfile.time_ref = stfdFrame.stVFrame.u32TimeRef;
        stProfile.pts = stfdFrame.stVFrame.u64PTS;
        if (u32FrameCount == 0) {
            APP_PROF_LOG_PRINT(LEVEL_INFO,
                "PD input frame: grp=%d chn=%d size=%ux%u format=%d "
                "stride=[%u,%u,%u] configured=%ux%u preprocessed=0\n",
                VpssGrp, VpssChn, stfdFrame.stVFrame.u32Width,
                stfdFrame.stVFrame.u32Height, stfdFrame.stVFrame.enPixelFormat,
                stfdFrame.stVFrame.u32Stride[0], stfdFrame.stVFrame.u32Stride[1],
                stfdFrame.stVFrame.u32Stride[2], g_pstPdCfg->u32GrpWidth,
                g_pstPdCfg->u32GrpHeight);
        }
        u64StageStart = app_ipcam_Ai_PD_TimeUs();
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);
        stProfile.wrap_us = app_ipcam_Ai_PD_TimeUs() - u64StageStart;

        pthread_mutex_unlock(&g_PDStatusMutex);  

        TDLObject obj_meta;
        memset(&obj_meta, 0, sizeof(TDLObject));
        u64StageStart = app_ipcam_Ai_PD_TimeUs();
        s32Ret = g_pfpPDInference(g_PDAiHandle, g_pstPdCfg->model_id,
            image_handle, &obj_meta);
        stProfile.inference_us = app_ipcam_Ai_PD_TimeUs() - u64StageStart;
        if (s32Ret == CVI_SUCCESS) {
            TDL_GetModelPerformance(g_PDAiHandle, g_pstPdCfg->model_id,
                &stProfile.model);
        }
        u32FrameCount++;

        u64StageStart = app_ipcam_Ai_PD_TimeUs();
        CVI_S32 s32ReleaseRet = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32ReleaseRet != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n",
                VpssGrp, VpssChn, s32ReleaseRet);
        }
        TDL_DestroyImage(image_handle);
        stProfile.release_us = app_ipcam_Ai_PD_TimeUs() - u64StageStart;

        if (s32Ret != CVI_SUCCESS) {
            app_ipcam_Ai_PD_ProfileComplete(
                u32FrameCount, u64LoopStart, &stProfile);
            u32InferenceErrors++;
            if (u32InferenceErrors == 1 ||
                (u32FrameCount % APP_PD_DIAG_INTERVAL) == 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "PD DIAG inference failed: frame=%u ret=%#x time=%u ms "
                    "errors=%u model=%d\n",
                    u32FrameCount, s32Ret,
                    (CVI_U32)((stProfile.inference_us + 500) / 1000),
                    u32InferenceErrors, g_pstPdCfg->model_id);
            }
            if (obj_meta.info != NULL) {
                TDL_ReleaseObjectMeta(&obj_meta);
            }
            continue;
        }

        if (obj_meta.size > 0 && obj_meta.info == NULL) {
            app_ipcam_Ai_PD_ProfileComplete(
                u32FrameCount, u64LoopStart, &stProfile);
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "PD DIAG invalid metadata: frame=%u objects=%u info=NULL\n",
                u32FrameCount, obj_meta.size);
            continue;
        }

        u64StageStart = app_ipcam_Ai_PD_TimeUs();
        {
            SMT_MutexAutoLock(g_PDMutex, lock);
            g_stPDObjDraw.size = 0;
            g_stPDObjDraw.width = obj_meta.width;
            g_stPDObjDraw.height = obj_meta.height;
            if (obj_meta.size > 0 && g_stPDObjDraw.info != NULL) {
                g_stPDObjDraw.size = obj_meta.size <= APP_PD_MAX_DRAW_OBJECTS ?
                    obj_meta.size : APP_PD_MAX_DRAW_OBJECTS;
                memcpy(g_stPDObjDraw.info, obj_meta.info,
                    g_stPDObjDraw.size * sizeof(TDLObjectInfo));
            }
        }
        stProfile.publish_us = app_ipcam_Ai_PD_TimeUs() - u64StageStart;
        app_ipcam_Ai_PD_ProfileComplete(
            u32FrameCount, u64LoopStart, &stProfile);

        if (obj_meta.size == 0) {
            u32EmptyFrames++;
            if (u32FrameCount == 1 ||
                (u32FrameCount % APP_PD_DIAG_INTERVAL) == 0) {
                APP_PROF_LOG_PRINT(LEVEL_INFO,
                    "PD DIAG no objects: frame=%u time=%u ms empty=%u "
                    "detected=%u errors=%u threshold=%.3f\n",
                    u32FrameCount,
                    (CVI_U32)((stProfile.inference_us + 500) / 1000),
                    u32EmptyFrames,
                    u32DetectedFrames, u32InferenceErrors, g_pstPdCfg->threshold);
            }
            if (obj_meta.info != NULL) {
                TDL_ReleaseObjectMeta(&obj_meta);
            }
            continue;
        }

        u32DetectedFrames++;
        if (u32DetectedFrames == 1 ||
            (u32FrameCount % APP_PD_DIAG_INTERVAL) == 0) {
            APP_PROF_LOG_PRINT(LEVEL_INFO,
                "PD DIAG detected: frame=%u time=%u ms objects=%u "
                "meta=%ux%u class=%d score=%.3f box=[%.1f,%.1f,%.1f,%.1f] "
                "detected=%u empty=%u errors=%u\n",
                u32FrameCount,
                (CVI_U32)((stProfile.inference_us + 500) / 1000),
                obj_meta.size,
                obj_meta.width, obj_meta.height,
                obj_meta.info[0].class_id, obj_meta.info[0].score,
                obj_meta.info[0].box.x1, obj_meta.info[0].box.y1,
                obj_meta.info[0].box.x2, obj_meta.info[0].box.y2,
                u32DetectedFrames, u32EmptyFrames, u32InferenceErrors);
        }

        TDL_ReleaseObjectMeta(&obj_meta);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "PD DIAG stopped: frames=%u detected=%u empty=%u errors=%u stale=%u\n",
        u32FrameCount, u32DetectedFrames, u32EmptyFrames,
        u32InferenceErrors, u32StaleFrames);

    pthread_exit(NULL);

    return NULL;
}


int app_ipcam_Ai_PD_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    static CVI_U32 u32OsdHandoffCount = 0;

    _NULL_POINTER_CHECK_(pstAiObj, -1);

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
    if (g_stPDObjDraw.size > 0 && g_stPDObjDraw.info != NULL) {
        pstAiObj->size = g_stPDObjDraw.size <= APP_PD_MAX_DRAW_OBJECTS ?
            g_stPDObjDraw.size : APP_PD_MAX_DRAW_OBJECTS;
        memcpy(pstAiObj->info, g_stPDObjDraw.info,
            pstAiObj->size * sizeof(TDLObjectInfo));

        u32OsdHandoffCount++;
        if (u32OsdHandoffCount == 1 ||
            (u32OsdHandoffCount % APP_PD_DIAG_INTERVAL) == 0) {
            APP_PROF_LOG_PRINT(LEVEL_INFO,
                "PD DIAG OSD handoff: count=%u objects=%u "
                "class=%d score=%.3f\n",
                u32OsdHandoffCount, pstAiObj->size,
                pstAiObj->info[0].class_id, pstAiObj->info[0].score);
        }
    }
    return CVI_SUCCESS;
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
    }

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

    app_ipcam_Ai_PD_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_PDThreadHandle, NULL, Thread_PD_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
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
        s32Ret = TDL_SetModelThreshold(g_PDAiHandle, g_pstPdCfg->model_id, threshold);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelThreshold failed with %#x!\n",g_pstPdCfg->model_path, s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}
