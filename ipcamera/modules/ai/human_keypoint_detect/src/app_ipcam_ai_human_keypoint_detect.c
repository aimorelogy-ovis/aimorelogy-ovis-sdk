#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
SMT_MUTEXAUTOLOCK_INIT(g_HumanKeypointMutex);
static pthread_mutex_t g_HumanKeypointStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S g_stHumanKeypointCfg;

static APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S *g_pstHumanKeypointCfg = &g_stHumanKeypointCfg;

static CVI_U32 g_HumanKeypointProc;
static volatile bool g_bHumanKeypointRunning = CVI_FALSE;
static volatile bool g_bHumanKeypointPause = CVI_FALSE;
static pthread_t g_HumanKeypointThreadHandle;
static TDLHandle g_HumanKeypointAiHandle = NULL;
// static cvitdl_service_handle_t g_HumanKeypointAiServiceHandle = NULL;
static TDLObject g_stHumanKeypointObjDraw;
static pfpInferenceFunc g_pfpHumanKeypointInference;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S *app_ipcam_Ai_Human_Keypoint_Param_Get(void)
{
    return g_pstHumanKeypointCfg;
}

CVI_VOID app_ipcam_Ai_Human_Keypoint_ProcStatus_Set(CVI_BOOL flag)
{
    g_bHumanKeypointRunning = flag;
}

CVI_BOOL app_ipcam_Ai_Human_Keypoint_ProcStatus_Get(void)
{
    return g_bHumanKeypointRunning;
}

CVI_VOID app_ipcam_Ai_Human_Keypoint_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_HumanKeypointStatusMutex);
    g_bHumanKeypointPause = flag;
    pthread_mutex_unlock(&g_HumanKeypointStatusMutex);
}

CVI_BOOL app_ipcam_Ai_Human_Keypoint_Pause_Get(void)
{
    return g_bHumanKeypointPause;
}

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel model_id)
{
    switch(model_id)
    {
        case TDL_MODEL_KEYPOINT_YOLOV8POSE_PERSON17:
            g_pfpHumanKeypointInference = TDL_Detection;
        break;
        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "model id (%d) invalid!\n", model_id);
            return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d\n", 
        g_pstHumanKeypointCfg->bEnable, g_pstHumanKeypointCfg->VpssGrp, g_pstHumanKeypointCfg->VpssChn);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_w=%d model_h=%d bSkip=%d threshold=%f\n",
        g_pstHumanKeypointCfg->model_size_w, g_pstHumanKeypointCfg->model_size_h, g_pstHumanKeypointCfg->bVpssPreProcSkip, g_pstHumanKeypointCfg->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, " model_id=%d model_path=%s\n",
        g_pstHumanKeypointCfg->model_id, g_pstHumanKeypointCfg->model_path);
}
/**
 * 深拷贝 TDLObject 结构体
 * @param dst 目标对象（需预先分配内存）
 * @param src 源对象
 * @return 0成功，-1失败
 */
static int deep_copy_tdl_object(TDLObject* dst, const TDLObject* src) {
    // 1. 参数检查
    if (dst == NULL || src == NULL) {
        return -1;
    }

    // 2. 拷贝基本成员
    dst->size = src->size;
    dst->width = src->width;
    dst->height = src->height;

    // 3. 处理 TDLObjectInfo 数组
    if (src->info != NULL && src->size > 0) {
        // 分配新数组内存
        dst->info = (TDLObjectInfo*)malloc(src->size * sizeof(TDLObjectInfo));
        if (dst->info == NULL) {
            return -1;
        }

        // 逐个拷贝对象信息
        for (uint32_t i = 0; i < src->size; i++) {
            // 拷贝基本成员
            dst->info[i].box.x1 = src->info[i].box.x1;
            dst->info[i].box.x2 = src->info[i].box.x2;
            dst->info[i].box.y1 = src->info[i].box.y1;
            dst->info[i].box.y2 = src->info[i].box.y2;
            dst->info[i].score = src->info[i].score;
            dst->info[i].class_id = src->info[i].class_id;
            dst->info[i].landmark_size = src->info[i].landmark_size;
            dst->info[i].obj_type = src->info[i].obj_type;
            
            // landmark_properity
            dst->info[i].landmark_properity = (TDLLandmarkInfo*)malloc(17 * sizeof(TDLLandmarkInfo));
            if (dst->info[i].landmark_properity == NULL) {
                return -1;
            }
            for (uint32_t j = 0; j < 17; j++) {
                dst->info[i].landmark_properity[j].x = src->info[i].landmark_properity[j].x;
                dst->info[i].landmark_properity[j].y = src->info[i].landmark_properity[j].y;
                dst->info[i].landmark_properity[j].score = src->info[i].landmark_properity[j].score;
            }
        }
    } else {
        return 0;
    }

    return 0;
}

static CVI_S32 app_ipcam_Ai_Human_Keypoint_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Human Keypoint Detection init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Param_dump();

    if (g_HumanKeypointAiHandle == NULL)
    {
        g_HumanKeypointAiHandle = TDL_CreateHandle(0);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Ai_InferenceFunc_Get(g_pstHumanKeypointCfg->model_id);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: %d \n", g_pstHumanKeypointCfg->model_id);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_HumanKeypointAiHandle, g_pstHumanKeypointCfg->model_id, g_pstHumanKeypointCfg->model_path, NULL, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetModelPath failed with %#x! maybe reset model path\n", s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_SetModelThreshold(g_HumanKeypointAiHandle,  g_pstHumanKeypointCfg->model_id, g_pstHumanKeypointCfg->threshold);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_SetModelThreshold failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Human Keypoint Detection init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_Human_Keypoint_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI Human Keypoint Detection start running!\n");

    prctl(PR_SET_NAME, "Thread_Human_Keypoint_PROC");

    VPSS_GRP VpssGrp = g_pstHumanKeypointCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstHumanKeypointCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    TDLImage image_handle ;

    while (app_ipcam_Ai_Human_Keypoint_ProcStatus_Get()) {
        pthread_mutex_lock(&g_HumanKeypointStatusMutex);
        s32Ret = app_ipcam_Ai_Human_Keypoint_Pause_Get();
        
        if (s32Ret) {
            pthread_mutex_unlock(&g_HumanKeypointStatusMutex);
            usleep(1000*1000);
            continue;
        }
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_HumanKeypointStatusMutex);
            usleep(100*1000);
            continue;
        }
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);

        pthread_mutex_unlock(&g_HumanKeypointStatusMutex);

        TDLObject obj_meta;
        memset(&obj_meta, 0, sizeof(TDLObject));

        TDL_Detection(g_HumanKeypointAiHandle, g_pstHumanKeypointCfg->model_id, image_handle, &obj_meta);
        // APP_PROF_LOG_PRINT(LEVEL_ERROR, "Human Keypoint Detect obj: %d \n", obj_meta.size);
        APP_PROF_LOG_PRINT(LEVEL_TRACE, "Human Keypoint Detect process takes %d\n", g_HumanKeypointProc);
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_handle);
        if (obj_meta.size == 0) {
            continue;
        }
        SMT_MutexAutoLock(g_HumanKeypointMutex, lock);
        if (g_stHumanKeypointObjDraw.info != NULL) {
            TDL_ReleaseObjectMeta(&g_stHumanKeypointObjDraw);
        }
        memset(&g_stHumanKeypointObjDraw, 0, sizeof(TDLObject));
        deep_copy_tdl_object(&g_stHumanKeypointObjDraw, &obj_meta);
        TDL_ReleaseObjectMeta(&obj_meta);
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_Human_Keypoint_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    _NULL_POINTER_CHECK_(pstAiObj, -1);

    SMT_MutexAutoLock(g_HumanKeypointMutex, lock);
    if (g_stHumanKeypointObjDraw.size == 0) {
        return CVI_SUCCESS;
    } else {
        memset(pstAiObj, 0, sizeof(TDLObject));
        deep_copy_tdl_object(pstAiObj, &g_stHumanKeypointObjDraw);
        if (g_stHumanKeypointObjDraw.info != NULL) { 
            TDL_ReleaseObjectMeta(&g_stHumanKeypointObjDraw);
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Human_Keypoint_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstHumanKeypointCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_Human_Keypoint_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Human_Keypoint_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_HumanKeypointThreadHandle)
    {
        pthread_join(g_HumanKeypointThreadHandle, NULL);
        g_HumanKeypointThreadHandle = 0;
    }

    TDL_CloseModel(g_HumanKeypointAiHandle, g_pstHumanKeypointCfg->model_id);
    s32Ret = TDL_DestroyHandle(g_HumanKeypointAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_HumanKeypointAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI HumanKeypoint Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_Human_Keypoint_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstHumanKeypointCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bHumanKeypointRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI Human Keypoint Detection has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_Human_Keypoint_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_HumanKeypoint_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_Human_Keypoint_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_HumanKeypointThreadHandle, NULL, Thread_Human_Keypoint_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
        return s32Ret;
    }
    
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Ai_Human_Keypoint_StatusGet(void)
{
    return g_HumanKeypointAiHandle ? 1 : 0;
}

CVI_S32 app_ipcam_Human_Keypoint_threshold_Set(float threshold)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_HumanKeypointAiHandle) {
        s32Ret = TDL_SetModelThreshold(g_HumanKeypointAiHandle, g_pstHumanKeypointCfg->model_id, threshold);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelThreshold failed with %#x!\n", g_pstHumanKeypointCfg->model_path, s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}
