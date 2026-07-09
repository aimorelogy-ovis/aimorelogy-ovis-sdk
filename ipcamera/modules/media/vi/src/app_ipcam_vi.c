#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include "cvi_comm_sys.h"
#include "cvi_bin.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_sensor.h"
#include "app_ipcam_vi.h"
#include "cvi_ispd2.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_ircut.h"
#ifdef SUPPORT_ISP_PQTOOL
#include <dlfcn.h>
#endif


/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#ifdef SUPPORT_ISP_PQTOOL
#define ISPD_LIBNAME "libcvi_ispd2.so"
#define ISPD_CONNECT_PORT 5566
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

APP_PARAM_VI_CTX_S g_stViCtx, *g_pstViCtx = &g_stViCtx;

static pthread_t AF_pthread;
static CVI_BOOL bAfFilterEnable;
static pthread_t g_IspPid[VI_MAX_DEV_NUM];

#ifdef SUPPORT_ISP_PQTOOL
static CVI_BOOL bISPDaemon = CVI_FALSE;
#endif

APP_PARAM_VI_PM_DATA_S ViPmData[VI_MAX_DEV_NUM] = { 0 };

VI_DEV_ATTR_S vi_dev_attr_base = {
    .enIntfMode = VI_MODE_MIPI,
    .enWorkMode = VI_WORK_MODE_1Multiplex,
    .enScanMode = VI_SCAN_PROGRESSIVE,
    .as32AdChnId = {-1, -1, -1, -1},
    .enDataSeq = VI_DATA_SEQ_YUYV,
    .stSynCfg = {
    /*port_vsync    port_vsync_neg    port_hsync              port_hsync_neg*/
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,
    /*port_vsync_valid     port_vsync_valid_neg*/
    VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb  hsync_act  hsync_hhb*/
    {0,           1920,       0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,           1080,       0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,         0}
    },
    .enInputDataType = VI_DATA_TYPE_RGB,
    .stSize = {1920, 1080},
    .stWDRAttr = {WDR_MODE_NONE, 1080},
    .enBayerFormat = BAYER_FORMAT_BG,
};

VI_PIPE_ATTR_S vi_pipe_attr_base = {
    .enPipeBypassMode = VI_PIPE_BYPASS_NONE,
    .bYuvSkip = CVI_FALSE,
    .bIspBypass = CVI_FALSE,
    .u32MaxW = 1920,
    .u32MaxH = 1080,
    .enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP,
    .enCompressMode = COMPRESS_MODE_NONE,
    .enBitWidth = DATA_BITWIDTH_12,
    .bNrEn = CVI_TRUE,
    .bSharpenEn = CVI_FALSE,
    .stFrameRate = {-1, -1},
    .bDiscardProPic = CVI_FALSE,
    .bYuvBypassPath = CVI_FALSE,
};

VI_CHN_ATTR_S vi_chn_attr_base = {
    .stSize = {1920, 1080},
    .enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420,
    .enDynamicRange = DYNAMIC_RANGE_SDR8,
    .enVideoFormat = VIDEO_FORMAT_LINEAR,
    .enCompressMode = COMPRESS_MODE_NONE,
    .bMirror = CVI_FALSE,
    .bFlip = CVI_FALSE,
    .u32Depth = 0,
    .stFrameRate = {-1, -1},
};

ISP_PUB_ATTR_S isp_pub_attr_base = {
    .stWndRect = {0, 0, 1920, 1080},
    .stSnsSize = {1920, 1080},
    .f32FrameRate = 25.0f,
    .enBayer = BAYER_BGGR,
    .enWDRMode = WDR_MODE_NONE,
    .u8SnsMode = 0,
};

static pthread_t g_RgbIR_Thread;
static CVI_BOOL Auto_Rgb_Ir_Enable;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_VI_CTX_S *app_ipcam_Vi_Param_Get(void)
{
    return g_pstViCtx;
}

static CVI_S32 app_ipcam_Isp_AfFilter_Init(CVI_VOID)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VI_PIPE ViPipe = 0;
    //CVI_CHAR input[10];
    ISP_STATISTICS_CFG_S stsCfg;
    ISP_PUB_ATTR_S stPubAttr;

    ISP_CTRL_PARAM_S stIspCtrlParam;
    CVI_ISP_GetCtrlParam(ViPipe, &stIspCtrlParam);
    stIspCtrlParam.u32AFStatIntvl = 1;
    CVI_ISP_SetCtrlParam(ViPipe, &stIspCtrlParam);

    // Get current statistic and related size setting.
    s32Ret = CVI_ISP_GetStatisticsConfig(ViPipe, &stsCfg);
    s32Ret |= CVI_ISP_GetPubAttr(ViPipe, &stPubAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Get Statistic info fail with %#x\n", s32Ret);
        return s32Ret;
    }
    // Config AF Enable.
    stsCfg.stFocusCfg.stConfig.bEnable = 1;
    // Config low pass filter.
    stsCfg.stFocusCfg.stConfig.u8HFltShift = 0;
    stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[0] = 0;
    stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[1] = 1;
    stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[2] = 2;
    stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[3] = 3;
    stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[4] = 4;
    // Config gamma enable.
    stsCfg.stFocusCfg.stConfig.stRawCfg.PreGammaEn  = 0;
    // Config pre NR enable.
    stsCfg.stFocusCfg.stConfig.stPreFltCfg.PreFltEn = 1;
    // Config H & V window.
    stsCfg.stFocusCfg.stConfig.u16Hwnd = 17;
    stsCfg.stFocusCfg.stConfig.u16Vwnd = 15;
    // Config crop related setting. Has some limitation
    stsCfg.stFocusCfg.stConfig.stCrop.bEnable = 1;
    stsCfg.stFocusCfg.stConfig.stCrop.u16X = 8;
    stsCfg.stFocusCfg.stConfig.stCrop.u16Y = 2;
    stsCfg.stFocusCfg.stConfig.stCrop.u16W = stPubAttr.stWndRect.u32Width - 8 * 2;
    stsCfg.stFocusCfg.stConfig.stCrop.u16H = stPubAttr.stWndRect.u32Height - 2 * 2;
    // Config first horizontal high pass filter.
    stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[0] = 0;
    stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[1] = 0;
    stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[2] = 13;
    stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[3] = 24;
    stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[4] = 0;
    // Config 2nd horizontal high pass filter.
    stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[0] = 1;
    stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[1] = 2;
    stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[2] = 4;
    stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[3] = 8;
    stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[4] = 0;

    // Config vertical high pass filter.
    stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[0] = 8;
    stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[1] = -15;
    stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[2 ] = 0;
    stsCfg.unKey.bit1FEAfStat = 1;

    // Config high luma thr
    stsCfg.stFocusCfg.stConfig.u16HighLumaTh = 3800;

    //LDG
    // stsCfg.stFocusCfg.stConfig.u8ThLow = 0;
    // stsCfg.stFocusCfg.stConfig.u8ThHigh = 255;
    // stsCfg.stFocusCfg.stConfig.u8GainLow = 30;
    // stsCfg.stFocusCfg.stConfig.u8GainHigh = 20;
    // stsCfg.stFocusCfg.stConfig.u8SlopLow = 8;
    // stsCfg.stFocusCfg.stConfig.u8SlopHigh = 15;

    s32Ret = CVI_ISP_SetStatisticsConfig(ViPipe, &stsCfg);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Set Statistic info fail with %#x\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

void app_ipcam_Isp_AfFilter_Get(ISP_AF_STATISTICS_S *pAfStat)
{
    CVI_U64 FVP = 0, FVPn = 0, FVQ = 0, FVQn = 0;
    CVI_U32 totalWeightSum = 0;
    CVI_U32 row, col;
    const CVI_U32 weight1 = 1, weight2 = 1, weight3 = 1;

    for (row = 0; row < AF_ZONE_ROW; row++) {
        for (col = 0; col < AF_ZONE_COLUMN; col++) {
            CVI_U64 h0 = pAfStat->stFEAFStat.stZoneMetrics[row][col].u64h0;
            CVI_U64 h1 = pAfStat->stFEAFStat.stZoneMetrics[row][col].u64h1;
            CVI_U32 v0 = pAfStat->stFEAFStat.stZoneMetrics[row][col].u32v0;
            FVPn = (weight1 * h1 + weight3 * v0) / (weight1 + weight3);
            FVQn = (weight1 * h0 + weight2 * v0) / (weight1 + weight2);
            FVP += FVPn;
            FVQ += FVQn;
            totalWeightSum += 1;
        }
    }

    FVP = FVP / totalWeightSum;
    FVQ = FVQ / totalWeightSum;

    APP_PROF_LOG_PRINT(LEVEL_TRACE, "FVP = %llu FVQ = %llu\n", (long long unsigned int)FVP, (long long unsigned int)FVQ);

    ISP_EXP_INFO_S stExpInfo;
    CVI_ISP_QueryExposureInfo(0, &stExpInfo);

    CVI_U8 u8P1 = FVP & 0xFF00;
    CVI_U8 u8P2 = FVP & 0x00FF;
    CVI_U8 u8Q1 = FVQ & 0xFF00;
    CVI_U8 u8Q2 = FVQ & 0x00FF;
    CVI_U8 u8R1 = log(stExpInfo.u32ISO)/log(2);

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "u32ISO=%d log(stExpInfo.u32ISO/100)/log(2)=%f\n", stExpInfo.u32ISO, log(stExpInfo.u32ISO/100)/log(2));

    APP_PROF_LOG_PRINT(LEVEL_TRACE, "R1=%d P1=%d P2=%d Q1=%d Q2=%d\n", u8R1, u8P1, u8P2, u8Q1, u8Q2);
}

static void *Thread_AF_Filter_Proc(void *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_CHAR TaskName[64];
    sprintf(TaskName, "AF_Filter_Get");
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Thread_AF_Filter_Proc task started \n");

    ISP_AF_STATISTICS_S afStat;
    CVI_U32 row, col;

    VI_PIPE ViPipe = 0;
    ISP_VD_TYPE_E enIspVDType = ISP_VD_FE_START;
    CVI_U64 FVn = 0, FV = 0;
    CVI_U32 totalWeightSum = 0;
    // weight for each statistic
    const CVI_U32 weight1 = 1, weight2 = 1, weight3 = 1;
    const CVI_U32 blockWeightSum = weight1 + weight2 + weight3;

    while (bAfFilterEnable) {
        s32Ret = CVI_ISP_GetVDTimeOut(ViPipe, enIspVDType, 5000);
        s32Ret |= CVI_ISP_GetFocusStatistics(ViPipe, &afStat);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Get Statistic failed with %#x\n", s32Ret);
            continue;
        }
        FVn = 0, FV = 0;
        totalWeightSum = 0;
        // calculate AF statistics
        for (row = 0; row < AF_ZONE_ROW; row++) {
            for (col = 0; col < AF_ZONE_COLUMN; col++) {
                CVI_U64 h0 = afStat.stFEAFStat.stZoneMetrics[row][col].u64h0;
                CVI_U64 h1 = afStat.stFEAFStat.stZoneMetrics[row][col].u64h1;
                CVI_U32 v0 = afStat.stFEAFStat.stZoneMetrics[row][col].u32v0;
                FVn = (weight1 * h0 + weight2 * h1 + weight3 * v0) / blockWeightSum;
                FV += FVn;
                totalWeightSum += 1;
            }
        }

        FV = FV / totalWeightSum;

        CVI_U32 u32Fv = FV & 0xFFFFFFFF;

        APP_PROF_LOG_PRINT(LEVEL_TRACE, "FV = %llu, u32Fv = %u\n", (long long unsigned int)FV, u32Fv);

        /* for customer used */
        app_ipcam_Isp_AfFilter_Get(&afStat);
    }

    return (void *) CVI_SUCCESS;
}


int app_ipcam_Isp_AfFilter_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    s32Ret = app_ipcam_Isp_AfFilter_Init();
    if(s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Isp_AfFilter_Start failed with %#x\n", s32Ret);
        return CVI_FAILURE;
    }

    pthread_attr_t pthread_attr;
    pthread_attr_init(&pthread_attr);

    s32Ret = pthread_create(
                    &AF_pthread,
                    &pthread_attr,
                    Thread_AF_Filter_Proc,
                    NULL);
    if (s32Ret) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AF filter pthread_create failed:0x%x\n", s32Ret);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

#if  !defined(DUAL_OS)
static CVI_S32 app_ipcam_ISP_ProcInfo_Open(CVI_U32 ProcLogLev)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (ProcLogLev == ISP_PROC_LOG_LEVEL_NONE) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "isp proc log not enable\n");
    } else {
        ISP_CTRL_PARAM_S setParam;
        memset(&setParam, 0, sizeof(ISP_CTRL_PARAM_S));

        setParam.u32AEStatIntvl = 1;    // AE info update frequency (unit:frame; rang:(0,0xffffffff])
        setParam.u32AWBStatIntvl = 6;    // AW info update frequency (unit:frame; rang:(0,0xffffffff])
        setParam.u32AFStatIntvl = 1;    // AF info update frequency (unit:frame; rang:(0,0xffffffff])
        setParam.u32PwmNumber = 0;        // PWM Num ID; Not used now

        s32Ret = CVI_ISP_SetCtrlParam(0, &setParam);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ISP_SetCtrlParam failed with %#x\n", s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}
#endif

#ifdef SUPPORT_ISP_PQTOOL
CVI_VOID app_ipcam_Ispd_Load(CVI_VOID)
{
    if (!bISPDaemon) {
        isp_daemon2_init(ISPD_CONNECT_PORT);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Isp_daemon2_init %d success\n", ISPD_CONNECT_PORT);
        bISPDaemon = CVI_TRUE;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "%s already loaded\n", ISPD_LIBNAME);
    }
}

static CVI_VOID app_ipcam_Ispd_Unload(CVI_VOID)
{
    if (bISPDaemon) {
        isp_daemon2_uninit();
        bISPDaemon = CVI_FALSE;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "%s not load yet!\n", ISPD_LIBNAME);
    }
}

#endif

int app_ipcam_Vi_framerate_Set(VI_PIPE ViPipe, CVI_S32 framerate)
{
    ISP_PUB_ATTR_S pubAttr = {0};

    CVI_ISP_GetPubAttr(ViPipe, &pubAttr);

    pubAttr.f32FrameRate = (CVI_FLOAT)framerate;

    APP_CHK_RET(CVI_ISP_SetPubAttr(ViPipe, &pubAttr), "set vi framerate");

    return CVI_SUCCESS;
}

int app_ipcam_Vi_Mipi_Start(void)   // Initialize MIPI interface for sensors
{
    CVI_S32 s32Ret;
    VI_PIPE ViPipe;
    SNS_COMBO_DEV_ATTR_S combo_dev_attr;
    CVI_S32 devno = 0, rstport, rstpin, rstpol;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++)
    {
        APP_PARAM_PIPE_CFG_T *pstPipeCfg = &g_pstViCtx->astPipeInfo[i];
        ViPipe = pstPipeCfg->aPipe[0];
        devno = g_pstViCtx->stSensorCfg.sns_ini_cfg.MipiDev[i];
        rstport = g_pstViCtx->stSensorCfg.sns_ini_cfg.s32RstPort[i];
        rstpin = g_pstViCtx->stSensorCfg.sns_ini_cfg.s32RstPin[i];
        rstpol = g_pstViCtx->stSensorCfg.sns_ini_cfg.s32RstPol[i];

        s32Ret = CVI_MIPI_SetSensorReset(devno, rstport, rstpin, rstpol, 1);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_MIPI_SetSensorReset(%d) failed!\n", ViPipe);

        s32Ret = CVI_MIPI_SetMipiReset(ViPipe, 1);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_MIPI_SetMipiReset(%d) failed!\n", ViPipe);

    }
    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++)
    {

        if ((g_pstViCtx->stSensorCfg.sns_ini_cfg.enSnsType[i] == VIVO_MCS369_2M_30FPS_12BIT) ||
            (g_pstViCtx->stSensorCfg.sns_ini_cfg.enSnsType[i] == VIVO_MCS369Q_4M_30FPS_12BIT)) {
            CVI_MIPI_SetClkEdge(ViPipe, 0);
        }

        if (CVI_SNS_GetSnsRxAttr(i, &combo_dev_attr) != CVI_SUCCESS) {
			APP_PROF_LOG_PRINT(LEVEL_ERROR, "get mipi dev_%d attr failed!\n", i);
			return CVI_FAILURE;
		}
        s32Ret = CVI_MIPI_SetMipiAttr(ViPipe, (CVI_VOID*)&combo_dev_attr);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_MIPI_SetMipiAttr(%d) failed!\n", ViPipe);

        s32Ret = CVI_MIPI_SetSensorClock(ViPipe, 1);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_MIPI_SetSensorClock(%d) failed!\n", ViPipe);

        usleep(20);

        s32Ret = CVI_MIPI_SetSensorReset(devno, rstport, rstpin, rstpol, 0);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_MIPI_SetSensorReset(%d) failed!\n", ViPipe);

        if (CVI_SNS_SetSnsProbe(i) != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "sensor_%d probe failed!\n", i);
            return CVI_FAILURE;
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vi_Dev_Start(void)
{
    CVI_S32 s32Ret;

    VI_DEV              ViDev;
    VI_DEV_ATTR_S       stViDevAttr;
    VI_DEV_BIND_PIPE_S  stViDevBindAttr;
    VI_DEV_ATTR_EX_S    stViDevAttrEx;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++)
    {
        APP_PARAM_CHN_CFG_T *pstChnCfg = &g_pstViCtx->astChnInfo[i];
        ViDev = pstChnCfg->s32ChnId;
        stViDevAttr.snrFps				= g_pstViCtx->stSensorCfg.sns_cfg.f32FrameRate[i];
        stViDevAttr.stSize.u32Width		= g_pstViCtx->stSensorCfg.sns_cfg.u32ImageWigth[i];
        stViDevAttr.enBayerFormat       = g_pstViCtx->stSensorCfg.sns_cfg.enBayerFormat[i];
        stViDevAttr.stSize.u32Height	= g_pstViCtx->stSensorCfg.sns_cfg.u32ImageHeight[i];
        stViDevAttr.enIntfMode			= (VI_INTF_MODE_E)g_pstViCtx->stSensorCfg.sns_cfg.enInterFaceMode[i];
        stViDevAttr.enInputDataType		= (VI_DATA_TYPE_E)g_pstViCtx->stSensorCfg.sns_cfg.enFormatMode[i];
        stViDevAttr.enDataSeq			= (VI_YUV_DATA_SEQ_E)g_pstViCtx->stSensorCfg.sns_cfg.enYuvFormat[i];
        stViDevAttr.stWDRAttr.enWDRMode	= g_pstViCtx->stSensorCfg.sns_cfg.enWDRMode[i];
        stViDevAttr.enWorkMode			= (VI_WORK_MODE_E)g_pstViCtx->stSensorCfg.sns_cfg.enChnMode[i];
        stViDevAttr.enScanMode          = VI_SCAN_PROGRESSIVE;

        if(pstChnCfg->enPixFormat == PIXEL_FORMAT_YUYV||pstChnCfg->enPixFormat == PIXEL_FORMAT_YVYU
            ||pstChnCfg->enPixFormat == PIXEL_FORMAT_UYVY||pstChnCfg->enPixFormat == PIXEL_FORMAT_VYUY) {
                stViDevAttr.enYuvSceneMode = VI_ISP_YUV_SCENE_BYPASS;
        }
        CVI_S32             s32PipeCnt = 0;

        APP_PROF_LOG_PRINT(LEVEL_INFO, "videv %d, snrFps %d, size %dx%d, intfMode %d, inputDataType %d, dataSeq %d, wdrMode %d, workMode %d\n",
                                        ViDev,
                                        stViDevAttr.snrFps,
                                        stViDevAttr.stSize.u32Width,
                                        stViDevAttr.stSize.u32Height,
                                        stViDevAttr.enIntfMode,
                                        stViDevAttr.enInputDataType,
                                        stViDevAttr.enDataSeq,
                                        stViDevAttr.stWDRAttr.enWDRMode,
                                        stViDevAttr.enWorkMode);

        APP_PARAM_PIPE_CFG_T *pstPipeCfg = &g_pstViCtx->astPipeInfo[i];
        for (int j = 0; j < WDR_MAX_PIPE_NUM; j++) {
            if (pstPipeCfg->aPipe[j] >= 0  && pstPipeCfg->aPipe[j] < VI_MAX_PIPE_NUM) {
                stViDevBindAttr.PipeId[j] = pstPipeCfg->aPipe[j];
                s32PipeCnt++;
                stViDevBindAttr.u32Num = s32PipeCnt;
            }
        }
        stViDevBindAttr.u32Num			= 1;
        stViDevBindAttr.MipiDev = g_pstViCtx->stSensorCfg.sns_ini_cfg.MipiDev[i];
        s32Ret = CVI_VI_SetDevBindAttr(ViDev, &stViDevBindAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_VI_SetDevAttr(ViDev, &stViDevAttr);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_VI_SetDevAttr(%d) failed!\n", ViDev);

        if(g_pstViCtx->stSensorCfg.sns_ini_cfg.u8MuxDev[i])
        {
            stViDevAttrEx.bMuxDev = true;
            stViDevAttrEx.u8SnsrNum = g_pstViCtx->u32WorkSnsCnt;
            stViDevAttrEx.phyDev = g_pstViCtx->stSensorCfg.sns_ini_cfg.u8AttachDev[i];
            for (int j = 0; j < SWITCH_GPIO_NUM; j++) {
                stViDevAttrEx.stGpioCfg[j].bEnable = g_pstViCtx->stSensorCfg.sns_ini_cfg.s32SwitchPort[i][j] != -1 ? true : false;
                stViDevAttrEx.stGpioCfg[j].s32GpioPort = g_pstViCtx->stSensorCfg.sns_ini_cfg.s32SwitchPort[i][j];
                stViDevAttrEx.stGpioCfg[j].s32GpioPin = g_pstViCtx->stSensorCfg.sns_ini_cfg.s32SwitchPin[i][j];
                stViDevAttrEx.stGpioCfg[j].s32GpioPol = g_pstViCtx->stSensorCfg.sns_ini_cfg.s32SwitchPol[i][j];
            }
            s32Ret = CVI_VI_SetDevAttrEx(ViDev, &stViDevAttrEx);
            if (s32Ret != CVI_SUCCESS) {
                APP_IPCAM_CHECK_RET(s32Ret, "CVI_VI_SetDevAttrEx failed with %#x!\n", s32Ret);
                return s32Ret;
            }
        }

        s32Ret = CVI_VI_EnableDev(ViDev);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_VI_EnableDev(%d) failed!\n", ViDev);
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vi_Dev_Stop(void)
{
    CVI_S32 s32Ret;
    VI_DEV ViDev;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) {
        APP_PARAM_CHN_CFG_T *pstChnCfg = &g_pstViCtx->astChnInfo[i];
        ViDev = pstChnCfg->s32ChnId;
        s32Ret  = CVI_VI_DisableDev(ViDev);

        // CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev);
        // CVI_VI_UnRegPmCallBack(ViDev);
        // memset(&ViPmData[ViDev], 0, sizeof(struct VI_PM_DATA_S));

        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VI_DisableDev failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vi_Pipe_Start(void)
{
    CVI_S32 s32Ret;

    VI_PIPE        ViPipe;
    VI_PIPE_ATTR_S stViPipeAttr;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++)
    {
        APP_PARAM_CHN_CFG_T *pstChnCfg = &g_pstViCtx->astChnInfo[i];
        APP_PARAM_PIPE_CFG_T *psPipeCfg = &g_pstViCtx->astPipeInfo[i];

        stViPipeAttr.u32MaxW						= g_pstViCtx->stSensorCfg.sns_cfg.u32ImageWigth[i];
        stViPipeAttr.u32MaxH						= g_pstViCtx->stSensorCfg.sns_cfg.u32ImageHeight[i];
        stViPipeAttr.enPixFmt						= PIXEL_FORMAT_RGB_BAYER_12BPP;
        stViPipeAttr.enBitWidth					= DATA_BITWIDTH_12;
        stViPipeAttr.stFrameRate.s32SrcFrameRate	= -1;
        stViPipeAttr.stFrameRate.s32DstFrameRate	= -1;
        stViPipeAttr.bNrEn						= CVI_TRUE;
        stViPipeAttr.bYuvBypassPath				= g_pstViCtx->stSensorCfg.sns_cfg.bBypassIsp[i];
        stViPipeAttr.enCompressMode				= pstChnCfg->enCompressMode;

        for (int j = 0; j < WDR_MAX_PIPE_NUM; j++) {
            if ((psPipeCfg->aPipe[j] >= 0) && (psPipeCfg->aPipe[j] < WDR_MAX_PIPE_NUM)) {
                ViPipe = psPipeCfg->aPipe[j];

                s32Ret = CVI_VI_CreatePipe(ViPipe, &stViPipeAttr);
                APP_IPCAM_CHECK_RET(s32Ret, "CVI_VI_CreatePipe(%d) failed!\n", ViPipe);
                s32Ret = CVI_VI_StartPipe(ViPipe);
                APP_IPCAM_CHECK_RET(s32Ret, "CVI_VI_StartPipe(%d) failed!\n", ViPipe);
            }
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_PQBin_Load(const CVI_CHAR *pBinPath)
{
    CVI_S32 ret = CVI_SUCCESS;
    FILE *fp = NULL;
    CVI_U8 *buf = NULL;
    CVI_U64 file_size;

    fp = fopen((const CVI_CHAR *)pBinPath, "rb");
    if (fp == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Can't find bin(%s), use default parameters\n", pBinPath);
        return CVI_FAILURE;
    }

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buf = (CVI_U8 *)malloc(file_size);
    if (buf == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s\n", "Allocae memory fail");
        fclose(fp);
        return CVI_FAILURE;
    }

    fread(buf, file_size, 1, fp);

    if (fp != NULL) {
        fclose(fp);
    }
    ret = CVI_BIN_ImportBinData(buf, (CVI_U32)file_size);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "CVI_BIN_ImportBinData error! value:(0x%x)\n", ret);
        free(buf);
        return CVI_FAILURE;
    }

    free(buf);

    return CVI_SUCCESS;
}

int app_ipcam_Vi_Pipe_Stop(void)
{
    CVI_S32 s32Ret;
    VI_PIPE ViPipe;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) {
        APP_PARAM_PIPE_CFG_T *psPipeCfg = &g_pstViCtx->astPipeInfo[i];
        ViPipe = psPipeCfg->aPipe[0];
        s32Ret = CVI_VI_StopPipe(ViPipe);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VI_StopPipe failed with %#x!\n", s32Ret);
            return s32Ret;
        }
        s32Ret = CVI_VI_DestroyPipe(ViPipe);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    return CVI_SUCCESS;
}

void app_ipcam_Framerate_Set(CVI_U8 viPipe, CVI_U8 fps)
{
    ISP_PUB_ATTR_S stPubAttr;

    memset(&stPubAttr, 0, sizeof(stPubAttr));

    CVI_ISP_GetPubAttr(viPipe, &stPubAttr);

    stPubAttr.f32FrameRate = fps;

    APP_PROF_LOG_PRINT(LEVEL_DEBUG,"set pipe: %d, fps: %d\n", viPipe, fps);

    CVI_ISP_SetPubAttr(viPipe, &stPubAttr);
}

CVI_U8 app_ipcam_Framerate_Get(CVI_U8 viPipe)
{
    ISP_PUB_ATTR_S stPubAttr;

    memset(&stPubAttr, 0, sizeof(stPubAttr));

    CVI_ISP_GetPubAttr(viPipe, &stPubAttr);

    return stPubAttr.f32FrameRate;
}

int app_ipcam_Vi_Isp_Init(void)
{
    CVI_S32 s32Ret;

    VI_PIPE              ViPipe;
    ISP_PUB_ATTR_S       stPubAttr;
    ISP_STATISTICS_CFG_S stsCfg;
    ISP_BIND_ATTR_S      stBindAttr;
    ALG_LIB_S            stAeLib;
    ALG_LIB_S            stAwbLib;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) 
    {
        APP_PARAM_CHN_CFG_T *pstChnCfg = &g_pstViCtx->astChnInfo[i];
        APP_PARAM_PIPE_CFG_T *pstPipeCfg = &g_pstViCtx->astPipeInfo[i];
        ViPipe = pstPipeCfg->aPipe[0];

        stAeLib.s32Id = ViPipe;
        strcpy(stAeLib.acLibName, CVI_AE_LIB_NAME);//, sizeof(CVI_AE_LIB_NAME));
        s32Ret = CVI_AE_Register(ViPipe, &stAeLib);
        APP_IPCAM_CHECK_RET(s32Ret, "AE Algo register fail, ViPipe[%d]\n", ViPipe);

        stAwbLib.s32Id = ViPipe;
        strcpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME);//, sizeof(CVI_AWB_LIB_NAME));
        s32Ret = CVI_AWB_Register(ViPipe, &stAwbLib);
        APP_IPCAM_CHECK_RET(s32Ret, "AWB Algo register fail, ViPipe[%d]\n", ViPipe);

        memset(&stBindAttr, 0, sizeof(ISP_BIND_ATTR_S));
        stBindAttr.sensorId = 0;
        snprintf(stBindAttr.stAeLib.acLibName, sizeof(CVI_AE_LIB_NAME), "%s", CVI_AE_LIB_NAME);
        stBindAttr.stAeLib.s32Id = ViPipe;
        snprintf(stBindAttr.stAwbLib.acLibName, sizeof(CVI_AWB_LIB_NAME), "%s", CVI_AWB_LIB_NAME);
        stBindAttr.stAwbLib.s32Id = ViPipe;
        s32Ret = CVI_ISP_SetBindAttr(ViPipe, &stBindAttr);
        APP_IPCAM_CHECK_RET(s32Ret, "Bind Algo fail, ViPipe[%d]\n", ViPipe);

        s32Ret = CVI_ISP_MemInit(ViPipe);
        APP_IPCAM_CHECK_RET(s32Ret, "Init Ext memory fail, ViPipe[%d]\n", ViPipe);

        stPubAttr.enBayer = (ISP_BAYER_FORMAT_E)g_pstViCtx->stSensorCfg.sns_cfg.enBayerFormat[i];

        stPubAttr.stWndRect.s32X = 0;
        stPubAttr.stWndRect.s32Y = 0;
        stPubAttr.stWndRect.u32Width  = pstChnCfg->u32Width;
        stPubAttr.stWndRect.u32Height = pstChnCfg->u32Height;
        stPubAttr.stSnsSize.u32Width  = pstChnCfg->u32Width;
        stPubAttr.stSnsSize.u32Height = pstChnCfg->u32Height;
        stPubAttr.f32FrameRate        = g_pstViCtx->stSensorCfg.sns_cfg.f32FrameRate[i];

        stPubAttr.enWDRMode           = g_pstViCtx->stSensorCfg.sns_cfg.enWDRMode[i];

        s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
            APP_IPCAM_CHECK_RET(s32Ret, "SetPubAttr fail, ViPipe[%d]\n", ViPipe);

        memset(&stsCfg, 0, sizeof(ISP_STATISTICS_CFG_S));
        s32Ret = CVI_ISP_GetStatisticsConfig(ViPipe, &stsCfg);
        APP_IPCAM_CHECK_RET(s32Ret, "ISP Get Statistic fail, ViPipe[%d]\n", ViPipe);
        stsCfg.stAECfg.stCrop[0].bEnable = 0;
        stsCfg.stAECfg.stCrop[0].u16X = 0;
        stsCfg.stAECfg.stCrop[0].u16Y = 0;
        stsCfg.stAECfg.stCrop[0].u16W = stPubAttr.stWndRect.u32Width;
        stsCfg.stAECfg.stCrop[0].u16H = stPubAttr.stWndRect.u32Height;
        memset(stsCfg.stAECfg.au8Weight, 1,
                AE_WEIGHT_ZONE_ROW * AE_WEIGHT_ZONE_COLUMN * sizeof(CVI_U8));

        // stsCfg.stAECfg.stCrop[1].bEnable = 0;
        // stsCfg.stAECfg.stCrop[1].u16X = 0;
        // stsCfg.stAECfg.stCrop[1].u16Y = 0;
        // stsCfg.stAECfg.stCrop[1].u16W = stPubAttr.stWndRect.u32Width;
        // stsCfg.stAECfg.stCrop[1].u16H = stPubAttr.stWndRect.u32Height;

        stsCfg.stWBCfg.u16ZoneRow = AWB_ZONE_ORIG_ROW;
        stsCfg.stWBCfg.u16ZoneCol = AWB_ZONE_ORIG_COLUMN;
        stsCfg.stWBCfg.stCrop.u16X = 0;
        stsCfg.stWBCfg.stCrop.u16Y = 0;
        stsCfg.stWBCfg.stCrop.u16W = stPubAttr.stWndRect.u32Width;
        stsCfg.stWBCfg.stCrop.u16H = stPubAttr.stWndRect.u32Height;
        stsCfg.stWBCfg.u16BlackLevel = 0;
        stsCfg.stWBCfg.u16WhiteLevel = 4095;
        stsCfg.stFocusCfg.stConfig.bEnable = 1;
        stsCfg.stFocusCfg.stConfig.u8HFltShift = 1;
        stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[0] = 1;
        stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[1] = 2;
        stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[2] = 3;
        stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[3] = 5;
        stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[4] = 10;
        stsCfg.stFocusCfg.stConfig.stRawCfg.PreGammaEn = 0;
        stsCfg.stFocusCfg.stConfig.stPreFltCfg.PreFltEn = 1;
        stsCfg.stFocusCfg.stConfig.u16Hwnd = 17;
        stsCfg.stFocusCfg.stConfig.u16Vwnd = 15;
        stsCfg.stFocusCfg.stConfig.stCrop.bEnable = 0;
        // AF offset and size has some limitation.
        stsCfg.stFocusCfg.stConfig.stCrop.u16X = AF_XOFFSET_MIN;
        stsCfg.stFocusCfg.stConfig.stCrop.u16Y = AF_YOFFSET_MIN;
        stsCfg.stFocusCfg.stConfig.stCrop.u16W = stPubAttr.stWndRect.u32Width - AF_XOFFSET_MIN * 2;
        stsCfg.stFocusCfg.stConfig.stCrop.u16H = stPubAttr.stWndRect.u32Height - AF_YOFFSET_MIN * 2;
        //Horizontal HP0
        stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[0] = 0;
        stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[1] = 0;
        stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[2] = 13;
        stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[3] = 24;
        stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[4] = 0;
        //Horizontal HP1
        stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[0] = 1;
        stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[1] = 2;
        stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[2] = 4;
        stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[3] = 8;
        stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[4] = 0;
        //Vertical HP
        stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[0] = 13;
        stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[1] = 24;
        stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[2] = 0;

        stsCfg.unKey.bit1FEAeGloStat = 1;
        stsCfg.unKey.bit1FEAeLocStat = 1;
        stsCfg.unKey.bit1AwbStat1 = 1;
        stsCfg.unKey.bit1AwbStat2 = 1;
        stsCfg.unKey.bit1FEAfStat = 1;

        //LDG
        stsCfg.stFocusCfg.stConfig.u8ThLow = 0;
        stsCfg.stFocusCfg.stConfig.u8ThHigh = 255;
        stsCfg.stFocusCfg.stConfig.u8GainLow = 30;
        stsCfg.stFocusCfg.stConfig.u8GainHigh = 20;
        stsCfg.stFocusCfg.stConfig.u8SlopLow = 8;
        stsCfg.stFocusCfg.stConfig.u8SlopHigh = 15;

        s32Ret = CVI_ISP_SetStatisticsConfig(ViPipe, &stsCfg);
        APP_IPCAM_CHECK_RET(s32Ret, "ISP Set Statistic fail, ViPipe[%d]\n", ViPipe);

        s32Ret = CVI_ISP_Init(i);
        APP_IPCAM_CHECK_RET(s32Ret, "ISP Init fail, ViPipe[%d]\n", i);
    }

    if (access(PQ_BIN_SDR, F_OK) == 0) {
        s32Ret = app_ipcam_PQBin_Load(PQ_BIN_SDR);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_WARN, "load %s failed with %#x!\n", PQ_BIN_SDR, s32Ret);
        }
    }

#ifdef SUPPORT_ISP_PQTOOL
    app_ipcam_Ispd_Load();
#endif

    return CVI_SUCCESS;
}


int app_ipcam_Vi_Isp_DeInit(void)
{
#ifdef DUAL_OS
    return CVI_SUCCESS;
#else

    CVI_S32             s32Ret;
    VI_PIPE             ViPipe;
    ALG_LIB_S           ae_lib;
    ALG_LIB_S           awb_lib;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) 
    {
        APP_PARAM_PIPE_CFG_T *pstPipeCfg = &g_pstViCtx->astPipeInfo[i];
        ViPipe = pstPipeCfg->aPipe[0];
        ae_lib.s32Id = ViPipe;
        awb_lib.s32Id = ViPipe;

        strcpy(ae_lib.acLibName, CVI_AE_LIB_NAME);//, sizeof(CVI_AE_LIB_NAME));
        strcpy(awb_lib.acLibName, CVI_AWB_LIB_NAME);//, sizeof(CVI_AWB_LIB_NAME));
        CVI_SNS_UnRegCallback(ViPipe, i);

        s32Ret = CVI_AE_UnRegister(ViPipe, &ae_lib);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_AE_UnRegister(%d) fail\n", ViPipe);

        s32Ret = CVI_AWB_UnRegister(ViPipe, &awb_lib);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_AWB_UnRegister(%d) fail\n", ViPipe);

    }
    return CVI_SUCCESS;

#endif
}
#if  !defined(DUAL_OS)
void *ISP_Thread(void *arg)
{
    CVI_S32 s32Ret = 0;
    VI_PIPE ViPipe = *(VI_PIPE *)arg;
    char szThreadName[20];

    snprintf(szThreadName, sizeof(szThreadName), "ISP%d_RUN", ViPipe);
    prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "ISP Dev %d running!\n", ViPipe);
    //No matter how many pipes, ISP run only once
    ViPipe = ViPipe;
    s32Ret = CVI_ISP_Run(ViPipe);
    if (s32Ret != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_ISP_Run(%d) failed with %#x!\n", ViPipe, s32Ret);
    }

    return CVI_NULL;
}
#endif

int app_ipcam_Vi_Isp_Start(void)
{
#ifdef DUAL_OS
    return CVI_SUCCESS;
#else

    CVI_S32 s32Ret;
    struct sched_param param;
    pthread_attr_t attr;

    VI_PIPE ViPipe;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) {
        APP_PARAM_PIPE_CFG_T *pstPipeCfg = &g_pstViCtx->astPipeInfo[i];
        ViPipe = pstPipeCfg->aPipe[0];

        param.sched_priority = 80;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        s32Ret = pthread_create(&g_IspPid[ViPipe], &attr, ISP_Thread, (void *)&pstPipeCfg->aPipe[0]);
        APP_IPCAM_CHECK_RET(s32Ret, "create isp running thread(%d) fail\n", ViPipe);
    }

    VI_DEV_ATTR_S pstDevAttr;
    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) 
    {
        APP_PARAM_PIPE_CFG_T *pstPipeCfg = &g_pstViCtx->astPipeInfo[i];
        ViPipe = pstPipeCfg->aPipe[0];
        CVI_VI_GetDevAttr(ViPipe, &pstDevAttr);
        s32Ret = CVI_BIN_SetBinName(pstDevAttr.stWDRAttr.enWDRMode, PQ_BIN_SDR);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_BIN_SetBinName %s failed with %#x!\n", PQ_BIN_SDR, s32Ret);
            return s32Ret;
        }

        s32Ret = app_ipcam_Vi_framerate_Set(ViPipe, g_pstViCtx->stSensorCfg.sns_cfg.f32FrameRate[i]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Vi_framerate_Set failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    s32Ret = app_ipcam_ISP_ProcInfo_Open(ISP_PROC_LOG_LEVEL_NONE);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_ISP_ProcInfo_Open failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    bAfFilterEnable = g_pstViCtx->astIspCfg[0].bAfFliter;
    if (bAfFilterEnable) {
        s32Ret = app_ipcam_Isp_AfFilter_Start();
        if(s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Isp_AfFilter_Start failed with %#x\n", s32Ret);
            return s32Ret;
        }
    }

#ifdef SUPPORT_ISP_PQTOOL
    app_ipcam_Ispd_Load();
#endif

    return CVI_SUCCESS;
#endif
}


int app_ipcam_Vi_Isp_Stop(void)
{
    CVI_S32     s32Ret;
    VI_PIPE     ViPipe;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) {
        APP_PARAM_PIPE_CFG_T *pstPipeCfg = &g_pstViCtx->astPipeInfo[i];
        ViPipe = pstPipeCfg->aPipe[0];

        #ifdef SUPPORT_ISP_PQTOOL
        app_ipcam_Ispd_Unload();
        #endif

        if (g_IspPid[ViPipe]) {
            s32Ret = CVI_ISP_Exit(ViPipe);
            if (s32Ret != CVI_SUCCESS) {
                 APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ISP_Exit fail with %#x!\n", s32Ret);
                return CVI_FAILURE;
            }
            pthread_join(g_IspPid[ViPipe], NULL);
            g_IspPid[ViPipe] = 0;
        }

    }
// #endif
    return CVI_SUCCESS;
}

int app_ipcam_Vi_Chn_Start(void)
{
    CVI_S32 s32Ret;

    VI_PIPE        ViPipe = 0;
    VI_CHN_ATTR_S  stViChnAttr;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) 
    {
        APP_PARAM_CHN_CFG_T *pstChnCfg = &g_pstViCtx->astChnInfo[i];
        ViPipe = pstChnCfg->s32ChnId;
        stViChnAttr.stSize.u32Width = g_pstViCtx->stSensorCfg.sns_cfg.u32ImageWigth[i];
        stViChnAttr.stSize.u32Height = g_pstViCtx->stSensorCfg.sns_cfg.u32ImageHeight[i];
        stViChnAttr.enDynamicRange = pstChnCfg->enDynamicRange;
        stViChnAttr.enVideoFormat  = pstChnCfg->enVideoFormat;
        stViChnAttr.enCompressMode = pstChnCfg->enCompressMode;
        stViChnAttr.enPixelFormat = pstChnCfg->enPixFormat;
        stViChnAttr.stFrameRate.s32SrcFrameRate = -1;
        stViChnAttr.stFrameRate.s32DstFrameRate = -1;
        stViChnAttr.u32Depth = 1;
        stViChnAttr.u32BindVbPool = -1;
        stViChnAttr.bSingleVb = 0;

        /* fill the sensor orientation */
        if (g_pstViCtx->stSensorCfg.sns_ini_cfg.u8Orien[i] <= 3) {
            stViChnAttr.bMirror = g_pstViCtx->stSensorCfg.sns_ini_cfg.u8Orien[i] & 0x1;
            stViChnAttr.bFlip = g_pstViCtx->stSensorCfg.sns_ini_cfg.u8Orien[i] & 0x2;
        }
        if(g_pstViCtx->stSensorCfg.sns_ini_cfg.u8MuxDev[i]) {
            stViChnAttr.bSingleVb = g_pstViCtx->stSensorCfg.sns_ini_cfg.u8MuxDev[i];
        }

        s32Ret = CVI_VI_SetChnAttr(ViPipe, 0, &stViChnAttr);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_VI_SetChnAttr(%d) failed!\n", ViPipe);

        if (CVI_SNS_SetVIFlipMirrorCB(ViPipe, i) != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SNS_SetVIFlipMirrorCB failed!\n");
        }

        s32Ret = CVI_VI_EnableChn(ViPipe, 0);
        APP_IPCAM_CHECK_RET(s32Ret, "CVI_VI_EnableChn(%d) failed!\n", ViPipe);

    }
    return CVI_SUCCESS;
}

int app_ipcam_Vi_Chn_Stop(void)
{
    CVI_S32 s32Ret;
    VI_CHN ViChn;
    VI_PIPE ViPipe;

    for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) {
        APP_PARAM_CHN_CFG_T *pstChnCfg = &g_pstViCtx->astChnInfo[i];
        ViPipe = pstChnCfg->s32ChnId;
        ViChn = pstChnCfg->s32ChnId;

        if (ViChn < VI_MAX_CHN_NUM) {
            CVI_VI_UnRegChnFlipMirrorCallBack(ViPipe, ViChn);
            s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VI_DisableChn failed with %#x!\n", s32Ret);
                return s32Ret;
            }
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vi_DeInit(void)
{
    APP_PARAM_MODULE_CFG_S * pModuleCfg = app_ipcam_Module_Param_Get();

    if(!pModuleCfg->alios_vi_mode){
        if (Auto_Rgb_Ir_Enable)
        {
            Auto_Rgb_Ir_Enable = CVI_FALSE;
            pthread_join(g_RgbIR_Thread, CVI_NULL);
            g_RgbIR_Thread = 0;
        }

        APP_CHK_RET(app_ipcam_Vi_Isp_Stop(),    "app_ipcam_Vi_Isp_Stop");
        APP_CHK_RET(app_ipcam_Vi_Isp_DeInit(),  "app_ipcam_Vi_Isp_DeInit");
        APP_CHK_RET(app_ipcam_Vi_Chn_Stop(),    "app_ipcam_Vi_Chn_Stop");
        APP_CHK_RET(app_ipcam_Vi_Pipe_Stop(),   "app_ipcam_Vi_Pipe_Stop");
        APP_CHK_RET(app_ipcam_Vi_Dev_Stop(),    "app_ipcam_Vi_Dev_Stop");
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vi_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PARAM_MODULE_CFG_S * pModuleCfg = app_ipcam_Module_Param_Get();
///////////////////////////////////////////////////////////////////////////////////////////////////
    // only support for dual_os
    if (pModuleCfg->alios_vi_mode) {
        for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) {
            s32Ret = CVI_ISP_MemInit(g_pstViCtx->astPipeInfo[i].aPipe[0]);
            if(s32Ret != CVI_SUCCESS){
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ISP_MemInit(%d) failed with %#x\n", i, s32Ret);
                return s32Ret;
            }
            s32Ret = CVI_ISP_Init(g_pstViCtx->astPipeInfo[i].aPipe[0]);
            if(s32Ret != CVI_SUCCESS){
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_ISP_Init(%d) failed with %#x\n", i, s32Ret);
                return s32Ret;
            }
        }
#ifdef SUPPORT_ISP_PQTOOL
        app_ipcam_Ispd_Load();
#endif
        return s32Ret;
    }
///////////////////////////////////////////////////////////////////////////////////////////////////
    else{
        s32Ret = CVI_SNS_GetConfigInfo(&g_pstViCtx->stSensorCfg);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "get sns cfg failed\n");
        }
        s32Ret = CVI_SNS_SetSnsDrvCfg(&g_pstViCtx->stSensorCfg);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "set sns_drv failed\n");
        }

        s32Ret = app_ipcam_Vi_Mipi_Start();
        if(s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Mipi_Start failed with %#x\n", s32Ret);
            goto VI_EXIT0;
        }

        s32Ret = app_ipcam_Vi_Dev_Start();
        if(s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Dev_Start failed with %#x\n", s32Ret);
            goto VI_EXIT0;
        }

        s32Ret = app_ipcam_Vi_Pipe_Start();
        if(s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Pipe_Start failed with %#x\n", s32Ret);
            goto VI_EXIT1;
        }

        s32Ret = app_ipcam_Vi_Isp_Init();
        if(s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Isp_Init failed with %#x\n", s32Ret);
            goto VI_EXIT2;
        }

        s32Ret = app_ipcam_Vi_Isp_Start();
        if(s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Isp_Start failed with %#x\n", s32Ret);
            goto VI_EXIT3;
        }

        for (CVI_U32 i = 0; i < g_pstViCtx->u32WorkSnsCnt; i++) {
            if (CVI_SNS_SetSnsInit(i) != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "sensor_%d init failed!\n", i);
                goto VI_EXIT3;
            }
        }

        s32Ret = app_ipcam_Vi_Chn_Start();
        if(s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Chn_Start failed with %#x\n", s32Ret);
            goto VI_EXIT4;
        }
    }

    return CVI_SUCCESS;

VI_EXIT4:
    app_ipcam_Vi_Isp_Stop();

VI_EXIT3:
    app_ipcam_Vi_Isp_DeInit();

VI_EXIT2:
    app_ipcam_Vi_Pipe_Stop();

VI_EXIT1:
    app_ipcam_Vi_Dev_Stop();

VI_EXIT0:
    app_ipcam_Sys_DeInit();

    return s32Ret;
}
