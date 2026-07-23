#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "app_ipcam_teaisp_bnr.h"
#include "app_ipcam_vi.h"
#include "app_ipcam_comm.h"

#ifdef AI_BNR_SUPPORT
#include "cvi_isp.h"
#include "app_ipcam_ai.h"
#endif

#define APP_TEAISP_BNR_PIPE 0
#define APP_TEAISP_BNR_SENSOR_30FPS_TYPE 0x20B83320U
#define APP_TEAISP_BNR_SENSOR_60FPS_TYPE 0x20B83420U

/* Match isp-tool-daemon's default model-list ISO and 10 percent tolerance. */
#ifndef APP_TEAISP_BNR_MODEL_ENTER_ISO
#define APP_TEAISP_BNR_MODEL_ENTER_ISO 3200U
#endif
#ifndef APP_TEAISP_BNR_MODEL_TOLERANCE
#define APP_TEAISP_BNR_MODEL_TOLERANCE (APP_TEAISP_BNR_MODEL_ENTER_ISO * 10U / 100U)
#endif

#ifdef AI_BNR_SUPPORT
typedef enum APP_TEAISP_BNR_STATE_E {
    APP_TEAISP_BNR_DISABLED = 0,
    APP_TEAISP_BNR_DRIVER_READY,
    APP_TEAISP_BNR_MODEL_READY,
} APP_TEAISP_BNR_STATE_E;

static APP_TEAISP_BNR_STATE_E g_enTeaispBnrState;
#endif

CVI_BOOL app_ipcam_TeaispBnr_IsEnabled(VI_PIPE ViPipe)
{
#ifdef AI_BNR_SUPPORT
    APP_PARAM_VI_CTX_S *pstViCfg = app_ipcam_Vi_Param_Get();

    if (ViPipe < 0 || ViPipe >= VI_MAX_DEV_NUM)
        return CVI_FALSE;
    return pstViCfg->astIspCfg[ViPipe].stTeaispBnrCfg.bEnable;
#else
    (void)ViPipe;
    return CVI_FALSE;
#endif
}

CVI_BOOL app_ipcam_TeaispBnr_IsCertified(void)
{
#ifdef AI_BNR_SUPPORT
    return APP_TEAISP_BNR_MODEL_ENTER_ISO > 0U &&
        APP_TEAISP_BNR_MODEL_TOLERANCE > 0U;
#else
    return CVI_FALSE;
#endif
}

CVI_S32 app_ipcam_TeaispBnr_ValidateAiExclusion(void)
{
#ifdef AI_BNR_SUPPORT
    CVI_BOOL bConflict = CVI_FALSE;

    if (!app_ipcam_TeaispBnr_IsEnabled(APP_TEAISP_BNR_PIPE))
        return CVI_SUCCESS;

#ifdef PD_SUPPORT
    if (app_ipcam_Ai_PD_Param_Get()->bEnable)
        bConflict = CVI_TRUE;
#endif
#ifdef FACE_SUPPORT
    if (app_ipcam_Ai_FD_Param_Get()->FD_bEnable)
        bConflict = CVI_TRUE;
#endif
#ifdef MD_SUPPORT
    if (app_ipcam_Ai_MD_Param_Get()->bEnable)
        bConflict = CVI_TRUE;
#endif
#ifdef HUMAN_KEYPOINT_SUPPORT
    if (app_ipcam_Ai_Human_Keypoint_Param_Get()->bEnable)
        bConflict = CVI_TRUE;
#endif
#ifdef OBJECT_TRACK_SUPPORT
    if (app_ipcam_Ai_Object_Track_Param_Get()->bEnable)
        bConflict = CVI_TRUE;
#endif
    if (bConflict) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "AI BNR cannot run with object, face, motion, human pose or object tracking\n");
        return CVI_FAILURE;
    }
#else
    (void)0;
#endif
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_TeaispBnr_ValidateConfiguration(void)
{
#ifdef AI_BNR_SUPPORT
    APP_PARAM_VI_CTX_S *pstViCfg = app_ipcam_Vi_Param_Get();
    APP_PARAM_PIPE_CFG_T *pstPipeCfg = &pstViCfg->astPipeInfo[0];

    if (!app_ipcam_TeaispBnr_IsEnabled(APP_TEAISP_BNR_PIPE))
        return CVI_SUCCESS;
    if (!app_ipcam_TeaispBnr_IsCertified()) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "AI BNR model metadata is not certified; enterISO/tolerance are unset\n");
        return CVI_FAILURE;
    }
    if (app_ipcam_TeaispBnr_ValidateAiExclusion() != CVI_SUCCESS)
        return CVI_FAILURE;
    if (pstViCfg->u32WorkSnsCnt != 1 || pstPipeCfg->aPipe[0] != APP_TEAISP_BNR_PIPE ||
        pstPipeCfg->aPipe[1] != -1 || pstPipeCfg->aPipe[2] != -1 ||
        pstPipeCfg->aPipe[3] != -1) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI BNR requires one sensor on pipe 0\n");
        return CVI_FAILURE;
    }
    if (((CVI_U32)pstViCfg->stSensorCfg.sns_ini_cfg.enSnsType[0] !=
            APP_TEAISP_BNR_SENSOR_30FPS_TYPE &&
         (CVI_U32)pstViCfg->stSensorCfg.sns_ini_cfg.enSnsType[0] !=
            APP_TEAISP_BNR_SENSOR_60FPS_TYPE) ||
        pstViCfg->astDevInfo[0].enWDRMode != WDR_MODE_NONE) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI BNR requires SC235HAI in SDR mode\n");
        return CVI_FAILURE;
    }
    if (pstViCfg->astChnInfo[0].u32Width != 1920 ||
        pstViCfg->astChnInfo[0].u32Height != 1080) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI BNR requires 1920x1080 input\n");
        return CVI_FAILURE;
    }
#endif
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_TeaispBnr_DriverInit(VI_PIPE ViPipe)
{
#ifdef AI_BNR_SUPPORT
    TEAISP_DRIVER_CFG_S stDriverCfg = {
        .enFeProcessType = TEAISP_FE_PROCESS_BF16,
    };
    CVI_S32 s32Ret;

    if (!app_ipcam_TeaispBnr_IsEnabled(ViPipe))
        return CVI_SUCCESS;
    if (g_enTeaispBnrState != APP_TEAISP_BNR_DISABLED)
        return CVI_SUCCESS;
    s32Ret = app_ipcam_TeaispBnr_ValidateConfiguration();
    if (s32Ret != CVI_SUCCESS)
        return s32Ret;
    s32Ret = CVI_TEAISP_Init(ViPipe, 1);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TEAISP_Init(%d) failed with %#x\n",
            ViPipe, s32Ret);
        return s32Ret;
    }
    s32Ret = CVI_TEAISP_BNR_Set_Driver_Init(ViPipe, &stDriverCfg);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_TEAISP_BNR_Set_Driver_Init(%d) failed with %#x\n", ViPipe, s32Ret);
        return s32Ret;
    }
    g_enTeaispBnrState = APP_TEAISP_BNR_DRIVER_READY;
#else
    (void)ViPipe;
#endif
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_TeaispBnr_ValidatePq(VI_PIPE ViPipe)
{
#ifdef AI_BNR_SUPPORT
    TEAISP_BNR_ATTR_S stAttr;
    CVI_S32 s32Ret;

    if (!app_ipcam_TeaispBnr_IsEnabled(ViPipe))
        return CVI_SUCCESS;
    memset(&stAttr, 0, sizeof(stAttr));
    s32Ret = CVI_TEAISP_BNR_GetAttr(ViPipe, &stAttr);
    if (s32Ret != CVI_SUCCESS || stAttr.enable != CVI_TRUE) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "AI BNR PQ attributes are unavailable or disabled, ret=%#x enable=%d\n",
            s32Ret, stAttr.enable);
        return CVI_FAILURE;
    }
#else
    (void)ViPipe;
#endif
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_TeaispBnr_LoadModel(VI_PIPE ViPipe)
{
#ifdef AI_BNR_SUPPORT
    TEAISP_BNR_MODEL_INFO_S stModelInfo;
    CVI_S32 s32Ret;

    if (!app_ipcam_TeaispBnr_IsEnabled(ViPipe))
        return CVI_SUCCESS;
    if (g_enTeaispBnrState == APP_TEAISP_BNR_MODEL_READY)
        return CVI_SUCCESS;
    if (g_enTeaispBnrState != APP_TEAISP_BNR_DRIVER_READY ||
        !app_ipcam_TeaispBnr_IsCertified())
        return CVI_FAILURE;
    if (strlen(APP_TEAISP_BNR_MODEL_PATH) >= TEAISP_MODEL_PATH_LEN ||
        access(APP_TEAISP_BNR_MODEL_PATH, R_OK) != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI BNR model is not readable: %s\n",
            APP_TEAISP_BNR_MODEL_PATH);
        return CVI_FAILURE;
    }

    memset(&stModelInfo, 0, sizeof(stModelInfo));
    snprintf(stModelInfo.path, sizeof(stModelInfo.path), "%s",
        APP_TEAISP_BNR_MODEL_PATH);
    stModelInfo.enterISO = APP_TEAISP_BNR_MODEL_ENTER_ISO;
    stModelInfo.tolerance = APP_TEAISP_BNR_MODEL_TOLERANCE;
    s32Ret = CVI_TEAISP_BNR_SetModel(ViPipe, &stModelInfo);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_TEAISP_BNR_SetModel(%d, %s) failed with %#x\n",
            ViPipe, APP_TEAISP_BNR_MODEL_PATH, s32Ret);
        return s32Ret;
    }
    g_enTeaispBnrState = APP_TEAISP_BNR_MODEL_READY;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI BNR model registered on pipe %d: %s\n",
        ViPipe, APP_TEAISP_BNR_MODEL_PATH);
#else
    (void)ViPipe;
#endif
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_TeaispBnr_DriverDeInit(VI_PIPE ViPipe)
{
#ifdef AI_BNR_SUPPORT
    CVI_S32 s32Ret;

    if (g_enTeaispBnrState == APP_TEAISP_BNR_DISABLED)
        return CVI_SUCCESS;
    s32Ret = CVI_TEAISP_BNR_Set_Driver_Deinit(ViPipe);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_TEAISP_BNR_Set_Driver_Deinit(%d) failed with %#x\n", ViPipe, s32Ret);
        return s32Ret;
    }
    g_enTeaispBnrState = APP_TEAISP_BNR_DISABLED;
#else
    (void)ViPipe;
#endif
    return CVI_SUCCESS;
}
