#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <dirent.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "app_ipcam_ai.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

SMT_MUTEXAUTOLOCK_INIT(g_FDMutex);
static pthread_mutex_t g_FDStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_FD_CFG_S g_stFDCfg;
// static APP_PARAM_AI_FD_CFG_S g_stFDCfg = {
//     .FD_bEnable = 0,
//     .VpssGrp = 0,
//     .VpssChn = 1,
//     .u32GrpWidth = 640,
//     .u32GrpHeight = 384,
//     .threshold_fd = 0.7,
//     .model_id_fd = TDL_MODEL_SCRFD_DET_FACE,
//     .model_path_fd = "/mnt/scrfd_det_face_432_768_INT8_mars3.bmodel",
// };
static APP_PARAM_AI_FD_CFG_S *g_pstFDCfg = &g_stFDCfg;

static volatile bool g_bFDRunning = CVI_FALSE;
static volatile bool g_bFDPause = CVI_FALSE;
static pthread_t g_FDThreadHandle;
static TDLHandle g_FDAiHandle = NULL;
static TDLFace g_stFDObjDraw;
static pfpFaceInferenceFunc g_pfpFDInference;

// #define IMAGE_DIR "/mnt/sd/picture/"
struct LinkList
{
    int index;
    char name[128];
    struct LinkList *next;
};
static CVI_U32 g_FDFps;
static CVI_U32 g_FDProc;


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_FD_CFG_S *app_ipcam_Ai_FD_Param_Get(void)
{
    return g_pstFDCfg;
}

CVI_VOID app_ipcam_Ai_FD_ProcStatus_Set(CVI_BOOL flag)
{
    g_bFDRunning = flag;
}

CVI_BOOL app_ipcam_Ai_FD_ProcStatus_Get(void)
{
    return g_bFDRunning;
}

CVI_VOID app_ipcam_Ai_FD_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_FDStatusMutex);
    g_bFDPause = flag;
    pthread_mutex_unlock(&g_FDStatusMutex);
}

CVI_BOOL app_ipcam_Ai_FD_Pause_Get(void)
{
    return g_bFDPause;
}

CVI_U32 app_ipcam_Ai_FD_ProcFps_Get(void)
{
    return g_FDFps;
}

CVI_S32 app_ipcam_Ai_FD_ProcTime_Get(void)
{
    return g_FDProc;
}

// static void app_ipcam_Ai_Fd_Param_dump(void)
// {
//     // APP_PROF_LOG_PRINT(LEVEL_INFO, "FD_bEnable=%d FR_bEnable=%d MASK_bEnable=%d FACE_AE_bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n", 
//     //     g_pstFDCfg->FD_bEnable, g_pstFDCfg->FR_bEnable,g_pstFDCfg->MASK_bEnable, g_pstFDCfg->FACE_AE_bEnable,
//     //             g_pstFDCfg->VpssGrp, g_pstFDCfg->VpssChn, g_pstFDCfg->u32GrpWidth, g_pstFDCfg->u32GrpHeight);

//     // APP_PROF_LOG_PRINT(LEVEL_INFO, "model_w=%d model_h=%d bSkip=%d FdPoolId=%d threshold_fd=%f threshold_fr=%f  threshold_mask=%f \n",
//     //     g_pstFDCfg->model_size_w, g_pstFDCfg->model_size_h, g_pstFDCfg->bVpssPreProcSkip, g_pstFDCfg->FdPoolId,
//     //     g_pstFDCfg->threshold_fd,g_pstFDCfg->threshold_fr,g_pstFDCfg->threshold_mask);
//     // APP_PROF_LOG_PRINT(LEVEL_INFO, " model_id_fd=%d model_path_fd=%s model_id_fr=%d model_path_fr=%s model_id_mask=%d model_path_mask=%s\n",
//     //                 g_pstFDCfg->model_id_fd, g_pstFDCfg->model_path_fd,
//     //                 g_pstFDCfg->model_id_fr, g_pstFDCfg->model_path_fr,
//     //                 g_pstFDCfg->model_id_mask, g_pstFDCfg->model_path_mask);
// }
static void app_ipcam_Ai_Fd_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "FD_bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d \n", \
        g_pstFDCfg->FD_bEnable, g_pstFDCfg->VpssGrp, g_pstFDCfg->VpssChn, g_pstFDCfg->u32GrpWidth, g_pstFDCfg->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold_fd=%f \n",  g_pstFDCfg->threshold_fd);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_fd=%d model_path_fd=%s \n", g_pstFDCfg->model_id_fd, g_pstFDCfg->model_path_fd);
}

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(TDLModel model_id)
{
    switch (model_id)
    {
        case TDL_MODEL_SCRFD_DET_FACE:
            g_pfpFDInference = TDL_FaceDetection;
        break;

        default:
            // g_pfpFDInference = TDL_FaceDetection;
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "model id (%d) invalid!\n", model_id);
            return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}


static CVI_S32 app_ipcam_Ai_FD_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Fd_Param_dump();

    /* 1.Creat Handle */
    if (g_FDAiHandle == NULL)
    {
        g_FDAiHandle = TDL_CreateHandle(0);
        if (g_FDAiHandle == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_FD_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TDL_FD_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Ai_InferenceFunc_Get(g_pstFDCfg->model_id_fd);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: %d \n", g_pstFDCfg->model_id_fd);
        return s32Ret;
    }

    s32Ret = TDL_OpenModel(g_FDAiHandle, g_pstFDCfg->model_id_fd, g_pstFDCfg->model_path_fd, NULL, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelPath failed with %#x!\n", g_pstFDCfg->model_path_fd, s32Ret);
        return s32Ret;
    }

    s32Ret = TDL_SetModelThreshold(g_FDAiHandle,  g_pstFDCfg->model_id_fd, g_pstFDCfg->threshold_fd);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s CVI_TDL_SetModelThreshold failed with %#x!\n", g_pstFDCfg->model_path_fd, s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD init ------------------> done \n");
    return CVI_SUCCESS;
}

/**
 * 深度拷贝 TDLFace 结构体
 * @param dst 目标对象（需预先分配内存）
 * @param src 源对象
 * @return 0成功，-1失败
 */
static int DeepCopy_TDLFace(TDLFace* dst, const TDLFace* src) {
    // 1. 参数检查
    if (dst == NULL || src == NULL) {
        return -1;
    }

    // 2. 拷贝基本成员
    dst->size = src->size;
    dst->width = src->width;
    dst->height = src->height;

    // 3. 处理 TDLFaceInfo 数组
    if (src->info != NULL && src->size > 0) {
        // 分配新数组内存
        dst->info = (TDLFaceInfo*)malloc(src->size * sizeof(TDLFaceInfo));
        if (dst->info == NULL) {
            return -1;
        }

        // 逐个拷贝人脸信息
        for (uint32_t i = 0; i < src->size; i++) {
            // 3.1 拷贝简单类型成员
            memcpy(dst->info[i].name, src->info[i].name, sizeof(src->info[i].name));
            dst->info[i].score = src->info[i].score;
            dst->info[i].box.x1 = src->info[i].box.x1;
            dst->info[i].box.x2 = src->info[i].box.x2;
            dst->info[i].box.y1 = src->info[i].box.y1;
            dst->info[i].box.y2 = src->info[i].box.y2;

            // 3.2 深度拷贝landmarks
            dst->info[i].landmarks.size = src->info[i].landmarks.size;
            dst->info[i].landmarks.score = src->info[i].landmarks.score;
            
            if(src->info[i].landmarks.x != NULL) {
                // if(dst->info[i].landmarks.x == NULL) {
                    dst->info[i].landmarks.x = (float*)malloc(sizeof(float));
                    if (dst->info[i].landmarks.x == NULL) {
                        fprintf(stderr, "Error: Failed to allocate memory for landmarks x\n");
                        return -1;
                    }
                // }
                memcpy(dst->info[i].landmarks.x, src->info[i].landmarks.x, sizeof(float));
            }
            
            if(src->info[i].landmarks.y != NULL) {
                // if(dst->info[i].landmarks.y == NULL) {
                    dst->info[i].landmarks.y = (float*)malloc(sizeof(float));
                    if (dst->info[i].landmarks.y == NULL) {
                        fprintf(stderr, "Error: Failed to allocate memory for landmarks y\n");
                        return -1;
                    }
                // }
                memcpy(dst->info[i].landmarks.y, src->info[i].landmarks.y, sizeof(float));
            }
            // 拷贝坐标数据
            // memcpy(dst->info[i].landmarks.x, src->info[i].landmarks.x, sizeof(float));
            // memcpy(dst->info[i].landmarks.y, src->info[i].landmarks.y, sizeof(float));
        }
    } else {
        dst->info = NULL;
        dst->size = 0;
    }

    return 0;
}

static CVI_VOID *Thread_FD_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD start running!\n");
    prctl(PR_SET_NAME, "Thread_FD_PROC");

    VPSS_GRP VpssGrp = g_pstFDCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstFDCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    // TDLFace face;
    TDLImage image_handle;
    
    while (app_ipcam_Ai_FD_ProcStatus_Get()) {
        pthread_mutex_lock(&g_FDStatusMutex);
        s32Ret = app_ipcam_Ai_FD_Pause_Get();

        if (s32Ret) {
            pthread_mutex_unlock(&g_FDStatusMutex);
            usleep(1000*1000);
            continue;
        }
        /* 1.Get frame */
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        if (s32Ret != CVI_SUCCESS){
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_FDStatusMutex);
            usleep(100*1000);
            continue;
        }
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);
      
        pthread_mutex_unlock(&g_FDStatusMutex);
        // iTime_proc = GetCurTimeInMsec();
        /* 2. Face Detect*/
        TDLFace face;
        memset(&face, 0, sizeof(TDLFace));
        g_pfpFDInference(g_FDAiHandle, g_pstFDCfg->model_id_fd, image_handle, &face);
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }
        TDL_DestroyImage(image_handle);

        // if (face.size == 0 || face.info == NULL) {
        //     TDL_ReleaseFaceMeta(&face);
        //     TDL_DestroyImage(image_handle);
        //     if (g_stFDObjDraw.info != NULL) {
        //         TDL_ReleaseFaceMeta(&g_stFDObjDraw);
        //     }
        //     continue;
        // }
        if (face.size == 0) {
            continue;
        }

        SMT_MutexAutoLock(g_FDMutex, lock);
        if (g_stFDObjDraw.info != NULL) {
            TDL_ReleaseFaceMeta(&g_stFDObjDraw);
        }
        memset(&g_stFDObjDraw, 0, sizeof(TDLFace));
        DeepCopy_TDLFace(&g_stFDObjDraw, &face);
        TDL_ReleaseFaceMeta(&face);
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_FD_ObjDrawInfo_Get(TDLFace *pstAiObj)
{
    _NULL_POINTER_CHECK_(pstAiObj, -1);

    SMT_MutexAutoLock(g_FDMutex, lock);
    if (g_stFDObjDraw.size == 0){
        return CVI_SUCCESS;
    }
    else
    {
        memset(pstAiObj, 0, sizeof(TDLFace));
        DeepCopy_TDLFace(pstAiObj, &g_stFDObjDraw);
        if (g_stFDObjDraw.info != NULL) { 
            TDL_ReleaseFaceMeta(&g_stFDObjDraw);
        }
    }
    return CVI_SUCCESS;
}

int app_ipcam_Ai_FD_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstFDCfg->FD_bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_FD_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_FD_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_FDThreadHandle)
    {
        pthread_join(g_FDThreadHandle, NULL);
        g_FDThreadHandle = 0;
    }
    TDL_CloseModel(g_FDAiHandle, g_pstFDCfg->model_id_fd);
    s32Ret = TDL_DestroyHandle(g_FDAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_FD_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_FDAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI FD Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}


int app_ipcam_Ai_FD_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstFDCfg->FD_bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bFDRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI FD has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_FD_Proc_Init();
	if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_Fd_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_FD_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_FDThreadHandle, NULL, Thread_FD_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI Fd_pthread_create failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}
