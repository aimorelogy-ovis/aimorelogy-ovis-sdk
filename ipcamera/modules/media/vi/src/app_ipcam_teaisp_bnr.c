#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "app_ipcam_teaisp_bnr.h"
#include "app_ipcam_vi.h"
#include "app_ipcam_comm.h"

#ifdef AI_BNR_SUPPORT
#include "cvi_ae.h"
#include "cvi_isp.h"
#include "app_ipcam_ai.h"
#endif

#define APP_TEAISP_BNR_PIPE 0
#define APP_TEAISP_BNR_SENSOR_30FPS_TYPE 0x20B83320U
#define APP_TEAISP_BNR_SENSOR_60FPS_TYPE 0x20B83420U
#define APP_TEAISP_BNR_PERF_WINDOW_FRAMES 60U
#define APP_TEAISP_BNR_PERF_TIMEOUT_MS 1000
#define APP_TEAISP_BNR_ISP_LOG_ASSERT 0

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
static pthread_t g_stTeaispBnrPerfThread;
static CVI_BOOL g_bTeaispBnrPerfRunning;
static CVI_BOOL g_bTeaispBnrPerfStarted;
static CVI_BOOL g_bTeaispBnrIspLogSuppressed;
static CVI_S32 g_s32TeaispBnrSavedIspLogLevel;

extern CVI_S32 g_isp_debug_level;
extern CVI_VOID CVI_DEBUG_SetDebugLevel(CVI_S32 s32Level);

static CVI_VOID app_ipcam_TeaispBnr_SuppressIspLog(void)
{
    if (g_bTeaispBnrIspLogSuppressed)
        return;
    g_s32TeaispBnrSavedIspLogLevel = g_isp_debug_level;
    CVI_DEBUG_SetDebugLevel(APP_TEAISP_BNR_ISP_LOG_ASSERT);
    g_bTeaispBnrIspLogSuppressed = CVI_TRUE;
}

static CVI_VOID app_ipcam_TeaispBnr_RestoreIspLog(void)
{
    if (!g_bTeaispBnrIspLogSuppressed)
        return;
    CVI_DEBUG_SetDebugLevel(g_s32TeaispBnrSavedIspLogLevel);
    g_bTeaispBnrIspLogSuppressed = CVI_FALSE;
}

static CVI_U64 app_ipcam_TeaispBnr_TimeUs(void)
{
    struct timespec stTime;

    if (clock_gettime(CLOCK_MONOTONIC, &stTime) != 0)
        return 0;
    return (CVI_U64)stTime.tv_sec * 1000000ULL + (CVI_U64)stTime.tv_nsec / 1000ULL;
}

static CVI_VOID *app_ipcam_TeaispBnr_PerfThread(CVI_VOID *pArg)
{
    VI_PIPE ViPipe = (VI_PIPE)(intptr_t)pArg;
    CVI_U64 u64WindowStartUs = 0;
    CVI_U64 u64LastFrameUs = 0;
    CVI_U64 u64IntervalTotalUs = 0;
    CVI_U64 u64IntervalMinUs = 0;
    CVI_U64 u64IntervalMaxUs = 0;
    CVI_U32 u32Frames = 0;
    CVI_U32 u32Timeouts = 0;
    CVI_BOOL bExpectedModelActive = CVI_FALSE;

    while (g_bTeaispBnrPerfRunning) {
        CVI_U64 u64NowUs;
        CVI_S32 s32Ret;

        s32Ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END,
            APP_TEAISP_BNR_PERF_TIMEOUT_MS);
        u64NowUs = app_ipcam_TeaispBnr_TimeUs();
        if (s32Ret != CVI_SUCCESS) {
            if (!g_bTeaispBnrPerfRunning)
                break;
            u32Timeouts++;
            if (u32Timeouts == 1 || (u32Timeouts % 10U) == 0) {
                APP_PROF_LOG_PRINT(LEVEL_WARN,
                    "AI BNR PERF BE timeout: pipe=%d ret=%#x total=%u\n",
                    ViPipe, s32Ret, u32Timeouts);
            }
            continue;
        }

        if (u64WindowStartUs == 0) {
            u64WindowStartUs = u64NowUs;
            u64LastFrameUs = u64NowUs;
            continue;
        }

        CVI_U64 u64IntervalUs = u64NowUs - u64LastFrameUs;

        u64LastFrameUs = u64NowUs;
        u64IntervalTotalUs += u64IntervalUs;
        if (u64IntervalMinUs == 0 || u64IntervalUs < u64IntervalMinUs)
            u64IntervalMinUs = u64IntervalUs;
        if (u64IntervalUs > u64IntervalMaxUs)
            u64IntervalMaxUs = u64IntervalUs;
        u32Frames++;

        if (u32Frames >= APP_TEAISP_BNR_PERF_WINDOW_FRAMES) {
            ISP_EXP_INFO_S stExpInfo = {0};
            CVI_FLOAT f32AeFps = 0.0f;
            CVI_U64 u64ElapsedUs = u64NowUs - u64WindowStartUs;
            CVI_U64 u64AverageUs = u64IntervalTotalUs / u32Frames;
            CVI_U64 u64FpsX100 = u64ElapsedUs > 0 ?
                (CVI_U64)u32Frames * 100000000ULL / u64ElapsedUs : 0;
            CVI_S32 s32ExpRet = CVI_ISP_QueryExposureInfo(ViPipe, &stExpInfo);
            CVI_S32 s32FpsRet = CVI_ISP_QueryFps(ViPipe, &f32AeFps);

            if (s32ExpRet == CVI_SUCCESS) {
                if (stExpInfo.u32ISO >= APP_TEAISP_BNR_MODEL_ENTER_ISO) {
                    bExpectedModelActive = CVI_TRUE;
                } else if (stExpInfo.u32ISO + APP_TEAISP_BNR_MODEL_TOLERANCE <=
                    APP_TEAISP_BNR_MODEL_ENTER_ISO) {
                    bExpectedModelActive = CVI_FALSE;
                }
            }

            APP_PROF_LOG_PRINT(LEVEL_INFO,
                "AI BNR PERF pipe=%d be_fps=%llu.%02llu interval_us(avg/min/max)="
                "%llu/%llu/%llu ae_fps=%.2f iso=%u expected_model=%d "
                "query_ret=%#x/%#x timeouts=%u\n",
                ViPipe,
                (unsigned long long)(u64FpsX100 / 100ULL),
                (unsigned long long)(u64FpsX100 % 100ULL),
                (unsigned long long)u64AverageUs,
                (unsigned long long)u64IntervalMinUs,
                (unsigned long long)u64IntervalMaxUs,
                s32FpsRet == CVI_SUCCESS ? f32AeFps : 0.0f,
                s32ExpRet == CVI_SUCCESS ? stExpInfo.u32ISO : 0U,
                bExpectedModelActive, s32FpsRet, s32ExpRet, u32Timeouts);

            u64WindowStartUs = u64NowUs;
            u64IntervalTotalUs = 0;
            u64IntervalMinUs = 0;
            u64IntervalMaxUs = 0;
            u32Frames = 0;
            u32Timeouts = 0;
        }
    }

    return NULL;
}

static CVI_VOID app_ipcam_TeaispBnr_PerfStart(VI_PIPE ViPipe)
{
    CVI_S32 s32Ret;

    if (g_bTeaispBnrPerfStarted)
        return;
    g_bTeaispBnrPerfRunning = CVI_TRUE;
    s32Ret = pthread_create(&g_stTeaispBnrPerfThread, NULL,
        app_ipcam_TeaispBnr_PerfThread, (CVI_VOID *)(intptr_t)ViPipe);
    if (s32Ret != 0) {
        g_bTeaispBnrPerfRunning = CVI_FALSE;
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "AI BNR PERF thread create failed: pipe=%d ret=%d\n", ViPipe, s32Ret);
        return;
    }
    g_bTeaispBnrPerfStarted = CVI_TRUE;
}

static CVI_VOID app_ipcam_TeaispBnr_PerfStop(void)
{
    if (!g_bTeaispBnrPerfStarted)
        return;
    g_bTeaispBnrPerfRunning = CVI_FALSE;
    pthread_join(g_stTeaispBnrPerfThread, NULL);
    g_bTeaispBnrPerfStarted = CVI_FALSE;
}
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
    CVI_U64 u64StartUs;
    CVI_U64 u64TeaispInitUs;
    CVI_U64 u64DriverInitUs;

    if (!app_ipcam_TeaispBnr_IsEnabled(ViPipe))
        return CVI_SUCCESS;
    if (g_enTeaispBnrState != APP_TEAISP_BNR_DISABLED)
        return CVI_SUCCESS;
    s32Ret = app_ipcam_TeaispBnr_ValidateConfiguration();
    if (s32Ret != CVI_SUCCESS)
        return s32Ret;
    u64StartUs = app_ipcam_TeaispBnr_TimeUs();
    s32Ret = CVI_TEAISP_Init(ViPipe, 1);
    u64TeaispInitUs = app_ipcam_TeaispBnr_TimeUs();
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_TEAISP_Init(%d) failed with %#x\n",
            ViPipe, s32Ret);
        return s32Ret;
    }
    s32Ret = CVI_TEAISP_BNR_Set_Driver_Init(ViPipe, &stDriverCfg);
    u64DriverInitUs = app_ipcam_TeaispBnr_TimeUs();
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_TEAISP_BNR_Set_Driver_Init(%d) failed with %#x\n", ViPipe, s32Ret);
        return s32Ret;
    }
    g_enTeaispBnrState = APP_TEAISP_BNR_DRIVER_READY;
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "AI BNR PERF init: pipe=%d fe=BF16 teaisp=%lluus driver=%lluus total=%lluus\n",
        ViPipe,
        (unsigned long long)(u64TeaispInitUs - u64StartUs),
        (unsigned long long)(u64DriverInitUs - u64TeaispInitUs),
        (unsigned long long)(u64DriverInitUs - u64StartUs));
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
    CVI_U64 u64StartUs;

    if (!app_ipcam_TeaispBnr_IsEnabled(ViPipe))
        return CVI_SUCCESS;
    memset(&stAttr, 0, sizeof(stAttr));
    u64StartUs = app_ipcam_TeaispBnr_TimeUs();
    s32Ret = CVI_TEAISP_BNR_GetAttr(ViPipe, &stAttr);
    if (s32Ret != CVI_SUCCESS || stAttr.enable != CVI_TRUE) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "AI BNR PQ attributes are unavailable or disabled, ret=%#x enable=%d\n",
            s32Ret, stAttr.enable);
        return CVI_FAILURE;
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "AI BNR PERF PQ: pipe=%d get_attr=%lluus enable=%d op=%d interval=%u debug=%u\n",
        ViPipe,
        (unsigned long long)(app_ipcam_TeaispBnr_TimeUs() - u64StartUs),
        stAttr.enable, stAttr.enOpType, stAttr.UpdateInterval, stAttr.DebugMode);
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
    CVI_U64 u64StartUs;

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
    u64StartUs = app_ipcam_TeaispBnr_TimeUs();
    app_ipcam_TeaispBnr_SuppressIspLog();
    s32Ret = CVI_TEAISP_BNR_SetModel(ViPipe, &stModelInfo);
    if (s32Ret != CVI_SUCCESS) {
        app_ipcam_TeaispBnr_RestoreIspLog();
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_TEAISP_BNR_SetModel(%d, %s) failed with %#x\n",
            ViPipe, APP_TEAISP_BNR_MODEL_PATH, s32Ret);
        return s32Ret;
    }
    g_enTeaispBnrState = APP_TEAISP_BNR_MODEL_READY;
    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "AI BNR PERF model: pipe=%d load=%lluus enter_iso=%u tolerance=%u path=%s\n",
        ViPipe,
        (unsigned long long)(app_ipcam_TeaispBnr_TimeUs() - u64StartUs),
        stModelInfo.enterISO, stModelInfo.tolerance, APP_TEAISP_BNR_MODEL_PATH);
    app_ipcam_TeaispBnr_PerfStart(ViPipe);
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
    app_ipcam_TeaispBnr_PerfStop();
    s32Ret = CVI_TEAISP_BNR_Set_Driver_Deinit(ViPipe);
    app_ipcam_TeaispBnr_RestoreIspLog();
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
