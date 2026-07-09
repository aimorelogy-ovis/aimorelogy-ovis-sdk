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
SMT_MUTEXAUTOLOCK_INIT(g_LPRMutex);
static pthread_mutex_t g_LPRStatusMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_AI_LPR_CFG_S g_stLprCfg;
static APP_PARAM_AI_LPR_CFG_S *g_pstLprCfg = &g_stLprCfg;

static CVI_U32 g_LPRFps;
static CVI_U32 g_LPRProc;
static volatile bool g_bLPRRunning = CVI_FALSE;
static volatile bool g_bLPRPause = CVI_FALSE;
static pthread_t g_LPRThreadHandle;
static TDLHandle g_LPRAiHandle = NULL;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_LPR_CFG_S *app_ipcam_Ai_LPR_Param_Get(void)
{
    return g_pstLprCfg;
}

CVI_VOID app_ipcam_Ai_LPR_ProcStatus_Set(CVI_BOOL flag)
{
    g_bLPRRunning = flag;
}

CVI_BOOL app_ipcam_Ai_LPR_ProcStatus_Get(void)
{
    return g_bLPRRunning;
}

CVI_VOID app_ipcam_Ai_LPR_Pause_Set(CVI_BOOL flag)
{
    pthread_mutex_lock(&g_LPRStatusMutex);
    g_bLPRPause = flag;
    pthread_mutex_unlock(&g_LPRStatusMutex);
}

CVI_BOOL app_ipcam_Ai_LPR_Pause_Get(void)
{
    return g_bLPRPause;
}

CVI_U32 app_ipcam_Ai_LPR_ProcFps_Get(void)
{
    return g_LPRFps;
}

CVI_S32 app_ipcam_Ai_LPR_ProcTime_Get(void)
{
    return g_LPRProc;
}

static void app_ipcam_Ai_LPR_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n",
        g_pstLprCfg->bEnable, g_pstLprCfg->VpssGrp, g_pstLprCfg->VpssChn,
        g_pstLprCfg->u32GrpWidth, g_pstLprCfg->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold=%f\n", g_pstLprCfg->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_det=%d model_path_det=%s\n",
        g_pstLprCfg->model_id_det, g_pstLprCfg->model_path_det);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_kp=%d model_path_kp=%s\n",
        g_pstLprCfg->model_id_kp, g_pstLprCfg->model_path_kp);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_rec=%d model_path_rec=%s\n",
        g_pstLprCfg->model_id_rec, g_pstLprCfg->model_path_rec);
}

static CVI_S32 app_ipcam_Ai_LPR_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI LPR init ------------------> start \n");

    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_LPR_Param_dump();

    if (g_LPRAiHandle == NULL)
    {
        g_LPRAiHandle = TDL_CreateHandle(0);
        if (g_LPRAiHandle == NULL)
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

    s32Ret = TDL_OpenModel(g_LPRAiHandle, g_pstLprCfg->model_id_det,
                            g_pstLprCfg->model_path_det, NULL, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel det failed with %#x!\n", s32Ret);
        goto exit0;
    }

    s32Ret = TDL_OpenModel(g_LPRAiHandle, g_pstLprCfg->model_id_kp,
                            g_pstLprCfg->model_path_kp, NULL, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel kp failed with %#x!\n", s32Ret);
        goto exit1;
    }

    s32Ret = TDL_OpenModel(g_LPRAiHandle, g_pstLprCfg->model_id_rec,
                            g_pstLprCfg->model_path_rec, NULL, 0);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_OpenModel rec failed with %#x!\n", s32Ret);
        goto exit2;
    }

    s32Ret = TDL_SetModelThreshold(g_LPRAiHandle, g_pstLprCfg->model_id_det,
                                    g_pstLprCfg->threshold);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_SetModelThreshold failed with %#x!\n", s32Ret);
        goto exit3;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI LPR init ------------------> done \n");
    return CVI_SUCCESS;

exit3:
    TDL_CloseModel(g_LPRAiHandle, g_pstLprCfg->model_id_rec);
exit2:
    TDL_CloseModel(g_LPRAiHandle, g_pstLprCfg->model_id_kp);
exit1:
    TDL_CloseModel(g_LPRAiHandle, g_pstLprCfg->model_id_det);
exit0:
    TDL_DestroyHandle(g_LPRAiHandle);
    g_LPRAiHandle = NULL;
    return s32Ret;
}

static CVI_VOID *Thread_LPR_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI LPR start running!\n");

    prctl(PR_SET_NAME, "Thread_LPR_PROC");

    VPSS_GRP VpssGrp = g_pstLprCfg->VpssGrp;
    VPSS_CHN VpssChn = g_pstLprCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};
    TDLImage image_handle;

    while (app_ipcam_Ai_LPR_ProcStatus_Get()) {
        pthread_mutex_lock(&g_LPRStatusMutex);
        if (app_ipcam_Ai_LPR_Pause_Get()) {
            pthread_mutex_unlock(&g_LPRStatusMutex);
            usleep(1000*1000);
            continue;
        }

        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 3000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n",
                VpssGrp, VpssChn, s32Ret);
            pthread_mutex_unlock(&g_LPRStatusMutex);
            usleep(100*1000);
            continue;
        }
        image_handle = TDL_WrapFrame((void*)&stfdFrame, false, false);

        pthread_mutex_unlock(&g_LPRStatusMutex);

        /* Step 1: Detection — 找到车牌框 */
        TDLObject obj_meta;
        memset(&obj_meta, 0, sizeof(TDLObject));
        s32Ret = TDL_Detection(g_LPRAiHandle, g_pstLprCfg->model_id_det,
                                image_handle, &obj_meta);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_Detection failed with %#x!\n", s32Ret);
            goto frame_cleanup;
        }

        if (obj_meta.size == 0) {
            goto frame_cleanup;
        }

        /* Step 2: DetectionKeypoint — 定位角点+裁剪车牌图像 */
        TDLImage *crop_image = (TDLImage *)malloc(sizeof(TDLImage) * obj_meta.size);
        if (crop_image == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc crop_image failed!\n");
            goto frame_cleanup;
        }
        memset(crop_image, 0, sizeof(TDLImage) * obj_meta.size);

        s32Ret = TDL_DetectionKeypoint(g_LPRAiHandle, g_pstLprCfg->model_id_kp,
                                        image_handle, &obj_meta, crop_image);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_DetectionKeypoint failed with %#x!\n", s32Ret);
            for (uint32_t i = 0; i < obj_meta.size; i++) {
                if (crop_image[i] != NULL) {
                    TDL_DestroyImage(crop_image[i]);
                }
            }
            free(crop_image);
            crop_image = NULL;
            TDL_ReleaseObjectMeta(&obj_meta);
            goto frame_cleanup;
        }

        /* Step 3: CharacterRecognition — OCR识别车牌字符 */
        for (uint32_t i = 0; i < obj_meta.size; i++) {
            TDLText ocr_meta = {0};
            s32Ret = TDL_CharacterRecognition(g_LPRAiHandle, g_pstLprCfg->model_id_rec,
                                              crop_image[i], &ocr_meta);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_CharacterRecognition[%d] failed with %#x!\n", i, s32Ret);
            } else {
                if (ocr_meta.size > 0) {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "plate[%d]: %s\n", i, ocr_meta.text_info);
                }
            }
            TDL_ReleaseCharacterMeta(&ocr_meta);
        }

        /* 释放 crop_image 和 obj_meta */
        for (uint32_t i = 0; i < obj_meta.size; i++) {
            if (crop_image[i] != NULL) {
                TDL_DestroyImage(crop_image[i]);
            }
        }
        free(crop_image);
        crop_image = NULL;
        TDL_ReleaseObjectMeta(&obj_meta);

frame_cleanup:
        CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        TDL_DestroyImage(image_handle);
    }

    pthread_exit(NULL);
    return NULL;
}

int app_ipcam_Ai_LPR_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstLprCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI LPR not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_LPR_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI LPR has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_LPR_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (g_LPRThreadHandle)
    {
        pthread_join(g_LPRThreadHandle, NULL);
        g_LPRThreadHandle = 0;
    }

    TDL_CloseModel(g_LPRAiHandle, g_pstLprCfg->model_id_rec);
    TDL_CloseModel(g_LPRAiHandle, g_pstLprCfg->model_id_kp);
    TDL_CloseModel(g_LPRAiHandle, g_pstLprCfg->model_id_det);
    s32Ret = TDL_DestroyHandle(g_LPRAiHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "TDL_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        g_LPRAiHandle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI LPR Thread exit takes %u ms\n",
        (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_LPR_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstLprCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI LPR not enable\n");
        return CVI_SUCCESS;
    }

    if (g_bLPRRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI LPR has started\n");
        return CVI_SUCCESS;
    }

    s32Ret = app_ipcam_Ai_LPR_Proc_Init();
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Ai_LPR_Proc_Init failed!\n");
        return s32Ret;
    }

    app_ipcam_Ai_LPR_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&g_LPRThreadHandle, NULL, Thread_LPR_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI LPR pthread_create failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}

/*****************************************************************
 *  The following API for command test used
 * **************************************************************/
CVI_S32 app_ipcam_Ai_LPR_StatusGet(void)
{
    return g_LPRAiHandle ? 1 : 0;
}

CVI_S32 app_ipcam_Lpr_threshold_Set(float threshold)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (g_LPRAiHandle) {
        s32Ret = TDL_SetModelThreshold(g_LPRAiHandle, g_pstLprCfg->model_id_det, threshold);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s TDL_SetModelThreshold failed with %#x!\n",
                g_pstLprCfg->model_path_det, s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}