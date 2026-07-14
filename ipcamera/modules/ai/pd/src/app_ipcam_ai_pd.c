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
SMT_MUTEXAUTOLOCK_INIT(g_PDMutex);
static pthread_mutex_t g_PDStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

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


static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel model_id)
{
    switch (model_id)
    {
        case TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON:
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
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n", g_pstPdCfg->model_id, g_pstPdCfg->model_path);

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
    
            // landmark_properity 设为 NULL（不拷贝原数据）
            dst->info[i].landmark_properity = NULL;
        }
    } else {
        return 0;
    }

    return 0;
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

    s32Ret = TDL_OpenModel(g_PDAiHandle, g_pstPdCfg->model_id, g_pstPdCfg->model_path, NULL, 0);
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

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_PD_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD start running!\n");

    prctl(PR_SET_NAME, "Thread_PD_PROC");

    VPSS_GRP VpssGrp = g_pstPdCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstPdCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    TDLImage image_handle ;

    while (app_ipcam_Ai_PD_ProcStatus_Get()) {
        pthread_mutex_lock(&g_PDStatusMutex);
        s32Ret = app_ipcam_Ai_PD_Pause_Get();
        
        if (s32Ret) {
            pthread_mutex_unlock(&g_PDStatusMutex);
            usleep(1000*1000);
            continue;
        } 
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);  
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret); 
            pthread_mutex_unlock(&g_PDStatusMutex);
            usleep(100*1000);
            continue;
        }
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);

        pthread_mutex_unlock(&g_PDStatusMutex);  

        TDLObject obj_meta;
        memset(&obj_meta, 0, sizeof(TDLObject));
        g_pfpPDInference(g_PDAiHandle, g_pstPdCfg->model_id, image_handle, &obj_meta);
        // APP_PROF_LOG_PRINT(LEVEL_INFO, "PD obj: %d \n", obj_meta.size);

        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame); 
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_handle);
        // if (obj_meta.size == 0 || obj_meta.info == NULL) {
        //     TDL_ReleaseObjectMeta(&obj_meta);
        //     TDL_DestroyImage(image_handle);
        //     if (g_stPDObjDraw.info != NULL) { 
        //         TDL_ReleaseObjectMeta(&g_stPDObjDraw);
                
        //     }
        //     continue;
        // }
        if (obj_meta.size == 0) {
            continue;
        }
        
        SMT_MutexAutoLock(g_PDMutex, lock);
        if (g_stPDObjDraw.info != NULL) { 
            TDL_ReleaseObjectMeta(&g_stPDObjDraw);
        }

        memset(&g_stPDObjDraw, 0, sizeof(TDLObject));
        deep_copy_tdl_object(&g_stPDObjDraw, &obj_meta);
        TDL_ReleaseObjectMeta(&obj_meta);
    }

    pthread_exit(NULL);

    return NULL;
}


int app_ipcam_Ai_PD_ObjDrawInfo_Get(TDLObject *pstAiObj)
{
    _NULL_POINTER_CHECK_(pstAiObj, -1);

    SMT_MutexAutoLock(g_PDMutex, lock);
    if (g_stPDObjDraw.size == 0) {
        return CVI_SUCCESS;
    } else {
        memset(pstAiObj, 0, sizeof(TDLObject));
        deep_copy_tdl_object(pstAiObj, &g_stPDObjDraw);
        if (g_stPDObjDraw.info != NULL) { 
            TDL_ReleaseObjectMeta(&g_stPDObjDraw);
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
    
    g_PDScaleX = g_PDScaleY = fmax(((float)pstVpssCfg->astVpssChnAttr[0].u32Width / (float)g_pstPdCfg->u32GrpWidth), \
                                     ((float)pstVpssCfg->astVpssChnAttr[0].u32Height / (float)g_pstPdCfg->u32GrpHeight));

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
