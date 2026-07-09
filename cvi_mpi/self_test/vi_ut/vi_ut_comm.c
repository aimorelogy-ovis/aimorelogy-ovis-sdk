#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <fcntl.h>

#include "cvi_comm_vb.h"
#include "cvi_comm_vpss.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_sns_ctrl.h"
#include "cvi_vi.h"
#include "cvi_buffer.h"
#include "cvi_sensor.h"
#include "sensor_cfg.h"
#include "cvi_mipi.h"

#include "vi_ut_comm.h"
#ifdef CV184X_FPGA_RAW_REPLAY
#include "vi_ut_isp_comm.h"
#include "vi_ut_isp_rawreplayoffline.h"
#else
#include "vi_ut_isp_helper.h"
#endif
#include "cvi_bin.h"

struct VI_PM_DATA_S {
	VI_PIPE ViPipe;
	CVI_U32 u32SnsId;
	CVI_S32 s32DevNo;
};

static struct VI_PM_DATA_S ViPmData[VI_MAX_DEV_NUM] = { 0 };

// default is MIPI-CSI Bayer format sensor
VI_DEV_ATTR_S DEV_ATTR_SENSOR_BASE = {
	VI_MODE_MIPI,
	VI_WORK_MODE_1Multiplex,
	VI_SCAN_PROGRESSIVE,
	{-1, -1, -1, -1},
	VI_DATA_SEQ_YUYV,

	{
	/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
	VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,
	VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,

	/*hsync_hfb    hsync_act    hsync_hhb*/
	{0,            1920,        0,
	/*vsync0_vhb vsync0_act vsync0_hhb*/
	 0,            1080,        0,
	/*vsync1_vhb vsync1_act vsync1_hhb*/
	 0,            0,            0}
	},
	VI_DATA_TYPE_RGB,
	{1920, 1080},
	{
		WDR_MODE_NONE,
		1080
	},
	.enBayerFormat = BAYER_FORMAT_BG,
};

// default is output YUV420 image
VI_CHN_ATTR_S CHN_ATTR_420_SDR8 = {
	{1920, 1080},
	PIXEL_FORMAT_YUV_PLANAR_420,
	DYNAMIC_RANGE_SDR8,
	VIDEO_FORMAT_LINEAR,
	COMPRESS_MODE_NONE,
	CVI_FALSE, CVI_FALSE,
	0,
	{ -1, -1},
	-1,
	CVI_FALSE,
};

static CVI_S32 vi_patgen_getconfig(SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	SNS_INI_CFG_S *sns_ini_cfg;
	SNS_CFG_S *sns_cfg;

	if (sensor_cfg == CVI_NULL) {
		SNS_DBG_PRT("sensor_cfg is NULL point\n");
		return CVI_FAILURE;
	}

	sns_ini_cfg = &sensor_cfg->sns_ini_cfg;
	sns_cfg = &sensor_cfg->sns_cfg;

	for (i = 0; i < sensor_cfg->sns_ini_cfg.devNum; i++) {
		sns_cfg->enBayerFormat[i] = BAYER_FORMAT_RG;
		sns_cfg->enFormatMode[i] = SNS_DATA_TYPE_RGB;
		sns_cfg->enInterFaceMode[i] = SNS_MODE_MIPI;

		switch (sns_ini_cfg->enSnsType[i]) {
		case CVSENS_CV2003_MIPI_2M_1080P_30FPS_10BIT:
		case SONY_IMX327_MIPI_2M_30FPS_12BIT:
		case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
			sns_cfg->u32ImageWigth[i] = 1920;
			sns_cfg->u32ImageHeight[i] = 1080;
			break;
		case BOARD_FULL_SIZE_MIPI_30FPS_12BIT:
			sns_cfg->u32ImageWigth[i] = 2880;
			sns_cfg->u32ImageHeight[i] = 2160;
			sns_cfg->enWDRMode[i] = WDR_MODE_2To1_LINE;
			break;
		case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
		case OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT:
		case OV_OS04A10_MIPI_4M_1440P_30FPS_10BIT_WDR2TO1:
			sns_cfg->u32ImageWigth[i] = 2560;
			sns_cfg->u32ImageHeight[i] = 1440;
			break;
		case GCORE_GC8613_MIPI_8M_30FPS_10BIT:
			sns_cfg->u32ImageWigth[i] = 3840;
			sns_cfg->u32ImageHeight[i] = 2160;
			break;
		case SMS_SC500AI_MIPI_5M_30FPS_10BIT:
		case SMS_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
			sns_cfg->u32ImageWigth[i] = 2880;
			sns_cfg->u32ImageHeight[i] = 1620;
			break;
		case OV_OS04C10_MIPI_4M_30FPS_12BIT:
		case OV_OS04C10_MIPI_4M_30FPS_10BIT_WDR2TO1:
			sns_cfg->u32ImageWigth[i] = 2688;
			sns_cfg->u32ImageHeight[i] = 1520;
			break;
		case PIXELPLUS_PR2100_2M_2CH_25FPS_8BIT:
			sns_cfg->u32ImageWigth[i] = 1920;
			sns_cfg->u32ImageHeight[i] = 1080;
			sns_cfg->enChnMode[i] = SNS_CHN_MODE_2Multiplex;
			sns_cfg->enYuvFormat[i] = SNS_DATA_SEQ_YUYV;
			sns_cfg->enFormatMode[i] = SNS_DATA_TYPE_YUV;
			sns_cfg->enInterFaceMode[i] = SNS_MODE_MIPI_YUV422;
			sns_cfg->bBypassIsp[i] = 1;
			break;
		default:
			UT_PRT("get sns type %d config failed\n", sns_ini_cfg->enSnsType[i]);
			s32Ret = CVI_FAILURE;
			break;
		}

		if (sns_ini_cfg->enSnsType[i] & 0x00000080)
			sns_cfg->enWDRMode[i] = WDR_MODE_2To1_LINE;

		UT_PRT("sensor %d: 0x%x, %d, %d, %d, %d\n",
			i, sns_ini_cfg->enSnsType[i], sns_cfg->u32ImageWigth[i],
			sns_cfg->u32ImageHeight[i], sns_cfg->enChnMode[i], sns_cfg->enWDRMode[i]);
	}

	return s32Ret;
}

static CVI_S32 VI_SnsIni2Vicfg(VI_UT_CTX *pUtCtx, SNS_INI_CFG_S *pstIniCfg, VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32SnsId = 0;
	CVI_S32 pipeIdx = 0, chnMax = 0, i = 0;
	DYNAMIC_RANGE_E enDynamicRange = DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E enPixFormat = VI_PIXEL_FORMAT;
	VIDEO_FORMAT_E enVideoFormat = VIDEO_FORMAT_LINEAR;
	VI_INFO_S *viInfo = NULL;
	VI_USR_PIC_INFO_S *picInfo = &pUtCtx->rawReplayInfo;

	if (!pstIniCfg) {
		UT_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	if (!pstViConfig) {
		UT_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	pstViConfig->s32ViNum = pstIniCfg->devNum;
	for (s32SnsId = 0; s32SnsId < pstIniCfg->devNum; s32SnsId++) {
		viInfo = &pstViConfig->astViInfo[s32SnsId];

		//set dev info
		pstViConfig->as32WorkingViId[s32SnsId]		= s32SnsId;
		viInfo->stDevInfo.ViDev				= s32SnsId;
		viInfo->stDevInfo.enWDRMode = pUtCtx->isRawReplay ?
						      (picInfo->isHdrOn ? WDR_MODE_2To1_LINE : WDR_MODE_NONE) :
						      pstViConfig->stSnsCfg.enWDRMode[s32SnsId];
		viInfo->stDevInfo.mipiDev = pstIniCfg->MipiDev[s32SnsId];

		if (pUtCtx->isRawReplay) {
			viInfo->stDevInfo.enYuvFormat		= VI_DATA_SEQ_VUVU;
			viInfo->stDevInfo.enFormatMode		= VI_DATA_TYPE_RGB;
			viInfo->stDevInfo.enInterFaceMode	= VI_MODE_MIPI;
			viInfo->stDevInfo.enBayerFormat		= BAYER_FORMAT_RG;
		} else {
			viInfo->stDevInfo.enSnsMode		= pstIniCfg->enSnsMode;
			viInfo->stDevInfo.stRstInfo.s32RstPort	= pstIniCfg->s32RstPort[s32SnsId];
			viInfo->stDevInfo.stRstInfo.s32RstPin	= pstIniCfg->s32RstPin[s32SnsId];
			viInfo->stDevInfo.stRstInfo.s32RstPol	= pstIniCfg->s32RstPol[s32SnsId];
			viInfo->stDevInfo.stHsettle.bHsettlen	= pstIniCfg->bHsettlen[s32SnsId];
			viInfo->stDevInfo.stHsettle.u8Hsettle	= pstIniCfg->u8Hsettle[s32SnsId];

			viInfo->stDevInfo.enBayerFormat		= pstViConfig->stSnsCfg.enBayerFormat[s32SnsId];
			viInfo->stDevInfo.enYuvFormat		= (VI_YUV_DATA_SEQ_E)pstViConfig->stSnsCfg.enYuvFormat[s32SnsId];
			viInfo->stDevInfo.enFormatMode		= (VI_DATA_TYPE_E)pstViConfig->stSnsCfg.enFormatMode[s32SnsId];
			viInfo->stDevInfo.enInterFaceMode	= (VI_INTF_MODE_E)pstViConfig->stSnsCfg.enInterFaceMode[s32SnsId];
			viInfo->stDevInfo.enChnMode		= (VI_WORK_MODE_E)pstViConfig->stSnsCfg.enChnMode[s32SnsId];

			if (viInfo->stDevInfo.enFormatMode == VI_DATA_TYPE_YUV) {
				viInfo->stDevInfo.enYuvScene = pUtCtx->isWithIsp
									? VI_ISP_YUV_SCENE_ISP
									: VI_ISP_YUV_SCENE_BYPASS;
			} else {
				viInfo->stDevInfo.enYuvScene = pstViConfig->stSnsCfg.bBypassIsp[s32SnsId]
									? VI_ISP_YUV_SCENE_BYPASS
									: VI_ISP_YUV_SCENE_ISP;
			}
		}

		viInfo->stDevInfo.bPatgen = pUtCtx->isPatgen;
		viInfo->stDevInfo.bMuxDev = pstIniCfg->u8MuxDev[s32SnsId];
		viInfo->stDevInfo.s32AttchDev = pstIniCfg->u8AttachDev[s32SnsId];
		for (i = 0; i < SWITCH_GPIO_NUM; i++) {
			viInfo->stDevInfo.s32SwitchPort[i] = pstIniCfg->s32SwitchPort[s32SnsId][i];
			viInfo->stDevInfo.s32SwitchPin[i] = pstIniCfg->s32SwitchPin[s32SnsId][i];
			viInfo->stDevInfo.s32SwitchPol[i] = pstIniCfg->s32SwitchPol[s32SnsId][i];
		}

		viInfo->stDevInfo.stSize.u32Width =
			pUtCtx->isRawReplay ? picInfo->u32ImgWidth : pstViConfig->stSnsCfg.u32ImageWigth[s32SnsId];
		viInfo->stDevInfo.stSize.u32Height =
			pUtCtx->isRawReplay ? picInfo->u32ImgHeight : pstViConfig->stSnsCfg.u32ImageHeight[s32SnsId];

		chnMax = (viInfo->stDevInfo.enFormatMode != VI_DATA_TYPE_YUV) ?
				 1 :
				 viInfo->stDevInfo.enChnMode + 1;

		// set pipe info
		for (i = 0; i < chnMax; i++) {
			viInfo->stPipeInfo.aPipe[i] = pipeIdx++;
		}

		for (; i < VI_MAX_PIPE_NUM; i++)
			viInfo->stPipeInfo.aPipe[i] = -1;

		viInfo->stPipeInfo.enCompressMode		= pUtCtx->isDpcmOn
									? COMPRESS_MODE_TILE
									: COMPRESS_MODE_NONE;

		// set channel info
		viInfo->stChnInfo.ViChn				= 0;
		viInfo->stChnInfo.enPixFormat			= enPixFormat;
		viInfo->stChnInfo.enDynamicRange		= enDynamicRange;
		viInfo->stChnInfo.enVideoFormat			= enVideoFormat;
		viInfo->stChnInfo.enCompressMode		= pUtCtx->isDpcmOn
									? COMPRESS_MODE_TILE
									: COMPRESS_MODE_NONE;
		viInfo->stChnInfo.u32Depth			= 1;
	}

	return CVI_SUCCESS;
}

CVI_S32 vi_ut_sys_init(VI_UT_CTX *pUtCtx)
{
	CVI_S32		s32Ret;
	CVI_S32		i;
	CVI_U32		blkcnt;
	VB_CONFIG_S	stVbConf;
	CVI_U32		u32BlkSize, u32BlkRotSize;
	VI_CONFIG_S	*pstViConfig = &pUtCtx->viConfig;
	SNS_INI_CFG_S	stSnsIniCfg = {
		.devNum    = 1,
		.enSnsType[0] = SONY_IMX327_MIPI_2M_30FPS_12BIT,
		.s32BusId[0]  = 3,
		.s32SnsI2cAddr[0] = -1,
		.MipiDev[0]   = 0,
	};
	SENSOR_CFG_S sensor_cfg;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&sensor_cfg, 0, sizeof(SENSOR_CFG_S));

	// Get config from ini if found.
	s32Ret = CVI_SNS_ParseIni(&sensor_cfg);
	if (s32Ret == CVI_FAILURE) {
		UT_PRT("[ERROR] parse ini failed\n");
	}

	if (pUtCtx->isPatgen || pUtCtx->isRawReplay) {
		vi_patgen_getconfig(&sensor_cfg);
	} else {
		s32Ret = CVI_SNS_GetConfigInfo(&sensor_cfg);
		if (s32Ret == CVI_FAILURE) {
			UT_PRT("[ERROR] get sns cfg failed\n");
		}

		s32Ret = CVI_SNS_SetSnsDrvCfg(&sensor_cfg);
		if (s32Ret == CVI_FAILURE) {
			UT_PRT("[ERROR] set sns_drv failed\n");
		}
	}

	memcpy(&stSnsIniCfg, &sensor_cfg.sns_ini_cfg, sizeof(SNS_INI_CFG_S));
	memcpy(&pstViConfig->stSnsCfg, &sensor_cfg.sns_cfg, sizeof(pstViConfig->stSnsCfg));

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = VI_SnsIni2Vicfg(pUtCtx, &stSnsIniCfg, pstViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		bool createNewPool = true;
		VI_INFO_S *pViInfo = &pstViConfig->astViInfo[i];
		DEV_INFO_S *pDevInfo = &pViInfo->stDevInfo;

		u32BlkSize = COMMON_GetPicBufferSize(pDevInfo->stSize.u32Width, pDevInfo->stSize.u32Height,
				VI_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(pDevInfo->stSize.u32Height, pDevInfo->stSize.u32Width,
				VI_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += 2;
				createNewPool = false;
				break;
			}
		}

		if (createNewPool) {
			blkcnt = (pViInfo->stDevInfo.enWDRMode != WDR_MODE_NONE) ? 2 : 1; //for dump raw
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt =
				2 + pViInfo->stChnInfo.u32Depth + blkcnt;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.u32MaxPoolCnt++;
			UT_PRT("creat New Pool(%d) size=%d cnt(%d)\n", stVbConf.u32MaxPoolCnt - 1, u32BlkSize,
					stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt);
		}

		UT_PRT("Create VB Pool(%d) size(%d), cnt(%d)\n", stVbConf.u32MaxPoolCnt - 1, u32BlkSize,
				stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt);
	}

	if (stVbConf.u32MaxPoolCnt == 1) {
		stVbConf.astCommPool[0].u32BlkCnt += 2;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_ut_set_vi_vpss_mode(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8 pipe = 0;
	VI_VPSS_MODE_S	stVIVPSSMode;
	VPSS_MODE_S stVPSSMode;

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	for (pipe = 0; pipe < VI_MAX_PIPE_NUM; pipe++) {
		stVIVPSSMode.aenMode[pipe] = pUtCtx->viVpssMode;
	}

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (!pUtCtx->isOnlineSc)
		return CVI_SUCCESS;

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;

	if (pUtCtx->viVpssMode == VI_ONLINE_VPSS_ONLINE ||
		pUtCtx->viVpssMode == VI_OFFLINE_VPSS_ONLINE) {
		stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;
	} else {
		stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;
	}

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_VOID vi_ut_sys_exit()
{
	CVI_VB_Exit();
	CVI_SYS_Exit();
}

static CVI_S32 vi_ut_start_dev(VI_INFO_S *pstViInfo)
{
	CVI_S32             s32Ret;
	CVI_S32             i;
	CVI_S32             s32PipeCnt = 0;
	VI_DEV              ViDev;
	//SAMPLE_SNS_TYPE_E   enSnsType;
	VI_DEV_ATTR_S       stViDevAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;
	VI_DEV_ATTR_EX_S    stViDevAttrEx;

	ViDev = pstViInfo->stDevInfo.ViDev;

	memcpy(&stViDevAttr, &DEV_ATTR_SENSOR_BASE, sizeof(VI_DEV_ATTR_S));

	stViDevAttr.stSize.u32Width = pstViInfo->stDevInfo.stSize.u32Width;
	stViDevAttr.stSize.u32Height = pstViInfo->stDevInfo.stSize.u32Height;
	stViDevAttr.enIntfMode = pstViInfo->stDevInfo.enInterFaceMode;
	stViDevAttr.enInputDataType = pstViInfo->stDevInfo.enFormatMode;
	stViDevAttr.enDataSeq = pstViInfo->stDevInfo.enYuvFormat;
	stViDevAttr.stWDRAttr.enWDRMode = pstViInfo->stDevInfo.enWDRMode;
	stViDevAttr.enWorkMode = pstViInfo->stDevInfo.enChnMode;
	stViDevAttr.enYuvSceneMode = pstViInfo->stDevInfo.enYuvScene;
	stViDevAttr.enBayerFormat = pstViInfo->stDevInfo.enBayerFormat;

	stViDevBindAttr.MipiDev = pstViInfo->stDevInfo.mipiDev;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			stViDevBindAttr.PipeId[s32PipeCnt] = pstViInfo->stPipeInfo.aPipe[i];
			s32PipeCnt++;
			stViDevBindAttr.u32Num = s32PipeCnt;
		}
	}

	s32Ret = CVI_VI_SetDevBindAttr(ViDev, &stViDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	if (pstViInfo->stDevInfo.bPatgen) {
		s32Ret = CVI_VI_EnablePatgen(ViDev);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("set patgen failed\n");
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	if (pstViInfo->stDevInfo.bMuxDev) {
		stViDevAttrEx.bMuxDev = true;
		stViDevAttrEx.phyDev = pstViInfo->stDevInfo.s32AttchDev;
		for (i = 0; i < SWITCH_GPIO_NUM; i++) {
			stViDevAttrEx.stGpioCfg[i].bEnable = (pstViInfo->stDevInfo.s32SwitchPort[i]) ? true : false;
			stViDevAttrEx.stGpioCfg[i].s32GpioPort = pstViInfo->stDevInfo.s32SwitchPort[i];
			stViDevAttrEx.stGpioCfg[i].s32GpioPin = pstViInfo->stDevInfo.s32SwitchPin[i];
			stViDevAttrEx.stGpioCfg[i].s32GpioPol = pstViInfo->stDevInfo.s32SwitchPol[i];
		}

		s32Ret = CVI_VI_SetDevAttrEx(ViDev, &stViDevAttrEx);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetDevAttrEx failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_EnableDev(ViDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 vi_ut_stop_dev(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret;
	VI_DEV ViDev;

	ViDev = pstViInfo->stDevInfo.ViDev;
	s32Ret = CVI_VI_DisableDev(ViDev);

	CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev);
	CVI_VI_UnRegPmCallBack(ViDev);
	memset(&ViPmData[ViDev], 0, sizeof(struct VI_PM_DATA_S));

	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_DisableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 vi_ut_start_pipe(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 j = 0;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S stPipeAttr;

	stPipeAttr.u32MaxW = pstViInfo->stDevInfo.stSize.u32Width;
	stPipeAttr.u32MaxH = pstViInfo->stDevInfo.stSize.u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.bYuvBypassPath = CVI_FALSE;
	stPipeAttr.enCompressMode = pstViInfo->stPipeInfo.enCompressMode;

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[j];
			s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
				return s32Ret;
			}

			s32Ret = CVI_VI_StartPipe(ViPipe);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

CVI_S32 vi_ut_stop_pipe(VI_INFO_S *pstViInfo)
{
	CVI_S32  s32Ret;
	CVI_S32  i;
	VI_PIPE ViPipe = 0;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0 && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[i];

			s32Ret = CVI_VI_StopPipe(ViPipe);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VI_StopPipe failed with %#x!\n", s32Ret);
				return s32Ret;
			}

			s32Ret = CVI_VI_DestroyPipe(ViPipe);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

CVI_S32 vi_ut_start_chn(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i = 0;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	//CVI_U32 u32SnsId = 0;
	VI_CHN_ATTR_S stChnAttr;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0 && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[i];
			ViChn = pstViInfo->stChnInfo.ViChn;

			memcpy(&stChnAttr, &CHN_ATTR_420_SDR8, sizeof(VI_CHN_ATTR_S));

			stChnAttr.stSize.u32Width = pstViInfo->stDevInfo.stSize.u32Width;
			stChnAttr.stSize.u32Height = pstViInfo->stDevInfo.stSize.u32Height;
			stChnAttr.enDynamicRange = pstViInfo->stChnInfo.enDynamicRange;
			stChnAttr.enVideoFormat  = pstViInfo->stChnInfo.enVideoFormat;
			stChnAttr.enCompressMode = pstViInfo->stChnInfo.enCompressMode;
			stChnAttr.enPixelFormat = pstViInfo->stChnInfo.enPixFormat;
			stChnAttr.u32Depth = pstViInfo->stChnInfo.u32Depth;
			stChnAttr.u32BindVbPool = -1;
			/* fill the sensor orientation */
			stChnAttr.bMirror = false;
			stChnAttr.bFlip = false;
			stChnAttr.bSingleVb = true;//pstViInfo->stDevInfo.bMuxDev;

			s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VI_EnableChn failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return s32Ret;
}

CVI_S32 vi_ut_stop_chn(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn;
	CVI_S32 i = 0;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0 && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[i];
			ViChn = pstViInfo->stChnInfo.ViChn;

			s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VI_DisableChn failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

static CVI_S32 rawreplay_set_usr_pic(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DEV_TIMING_ATTR_S stTimingAttr;
	VI_USR_PIC_INFO_S *picInfo = &pUtCtx->rawReplayInfo;
	VI_PIPE_FRAME_SOURCE_E frameSource = VI_PIPE_FRAME_SOURCE_DEV;

	s32Ret = CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_USER_BE);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetPipeFrameSource failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_GetPipeFrameSource(0, &frameSource);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetPipeFrameSource failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (frameSource != VI_PIPE_FRAME_SOURCE_USER_BE) {
		UT_PRT("CVI_VI_GetPipeFrameSource failed with %d\n", frameSource);
		return CVI_FAILURE;
	}

	stTimingAttr.bEnable = true;
	stTimingAttr.s32FrmRate = picInfo->s32FrmRate;
	s32Ret = CVI_VI_SetDevTimingAttr(0, &stTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetDevTimingAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	memset(&stTimingAttr, 0, sizeof(stTimingAttr));
	s32Ret = CVI_VI_GetDevTimingAttr(0, &stTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetDevTimingAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (!stTimingAttr.bEnable || stTimingAttr.s32FrmRate != picInfo->s32FrmRate) {
		UT_PRT("CVI_VI_GetDevTimingAttr\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 rawreplay_send_usr_pic(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_USR_PIC_INFO_S *picInfo = &pUtCtx->rawReplayInfo;
	VIDEO_FRAME_INFO_S stVideoFrame;
	const VIDEO_FRAME_INFO_S *pstVideoFrame[1];
	FILE *fp_le = NULL, *fp_se = NULL;
	VI_PIPE PipeId[] = {0};
	VB_POOL_CONFIG_S cfg = {0};
	CVI_U32 u32BlkSize = 0;
	CVI_U32 i = 0, u32Len = 0;

	fp_le = fopen(picInfo->file[0], "r");
	if (!fp_le) {
		UT_PRT("open data file le error\n");
		return CVI_FAILURE;
	}

	if (picInfo->isHdrOn) {
		fp_se = fopen(picInfo->file[1], "r");
		if (!fp_se) {
			UT_PRT("open data file se error\n");
			goto release_fp_le;
		}
	}

	stVideoFrame.stVFrame.enDynamicRange = picInfo->isHdrOn ? DYNAMIC_RANGE_HDR10 : DYNAMIC_RANGE_SDR8;
	stVideoFrame.stVFrame.u32Width = picInfo->u32ImgWidth;
	stVideoFrame.stVFrame.u32Height = picInfo->u32ImgHeight;
	stVideoFrame.stVFrame.s16OffsetLeft = 0;
	stVideoFrame.stVFrame.s16OffsetTop = 0;
	stVideoFrame.stVFrame.s16OffsetRight = 0;
	stVideoFrame.stVFrame.s16OffsetBottom = 0;
	stVideoFrame.stVFrame.enBayerFormat = picInfo->bayFormat;
	stVideoFrame.stVFrame.u32TimeRef = i; //from [1,n)

	u32BlkSize = VI_GetRawBufferSize(stVideoFrame.stVFrame.u32Width,
				stVideoFrame.stVFrame.u32Height, PIXEL_FORMAT_RGB_BAYER_12BPP,
				pUtCtx->isDpcmOn, DEFAULT_ALIGN, 0);

	if (!picInfo->usrBlk[0] && !picInfo->usrBlk[1]) {
		cfg.u32BlkCnt = picInfo->isHdrOn ? 2 : 1;
		cfg.u32BlkSize = u32BlkSize;
		picInfo->poolId = VB_INVALID_POOLID;

		picInfo->poolId = CVI_VB_CreatePool(&cfg);
		if (picInfo->poolId == VB_INVALID_POOLID) {
			UT_PRT("create vb pool failed\n");
			s32Ret = CVI_FAILURE;
			goto release_fp_se;
		}

		for (i = 0; i < cfg.u32BlkCnt; i++) {
			picInfo->usrBlk[i] = CVI_VB_GetBlock(picInfo->poolId, u32BlkSize);
			if (picInfo->usrBlk[i] == VB_INVALID_HANDLE) {
				UT_PRT("get VB blk failed\n");
				s32Ret = CVI_FAILURE;
				goto destoryPool;
			}

			picInfo->usrPhyAddr[i] = CVI_VB_Handle2PhysAddr(picInfo->usrBlk[i]);
			picInfo->usrVirAddr[i] = CVI_SYS_Mmap(picInfo->usrPhyAddr[i], u32BlkSize);
		}
	}

	if (picInfo->usrVirAddr[0]) {
		u32Len = fread(picInfo->usrVirAddr[0], 1, u32BlkSize, fp_le);
		if (u32Len != u32BlkSize) {
			UT_PRT("fread raw error expect size %d, but now %d\n", u32BlkSize, u32Len);
			s32Ret = CVI_FAILURE;
			goto ummap_sys;
		}
	}

	if (picInfo->usrVirAddr[1]) {
		u32Len = fread(picInfo->usrVirAddr[1], 1, u32BlkSize, fp_se);
		if (u32Len != u32BlkSize) {
			s32Ret = CVI_FAILURE;
			UT_PRT("fread raw error expect size %d, but now %d\n", u32BlkSize, u32Len);
			goto ummap_sys;
		}
	}

	stVideoFrame.stVFrame.u64PhyAddr[0] = picInfo->usrPhyAddr[0];
	stVideoFrame.stVFrame.u64PhyAddr[1] = picInfo->usrPhyAddr[1];
	pstVideoFrame[0] = &stVideoFrame;
	CVI_VI_SendPipeRaw(1, PipeId, pstVideoFrame, 80);

	return s32Ret;

ummap_sys:
	for (i = 0; i < cfg.u32BlkCnt; i++) {
		if (picInfo->usrVirAddr[i])
			CVI_SYS_Munmap(picInfo->usrVirAddr[i], u32BlkSize);
	}
destoryPool:
	CVI_VB_DestroyPool(picInfo->poolId);
release_fp_se:
	fclose(fp_se);
release_fp_le:
	fclose(fp_le);

	return s32Ret;
}

#ifdef CV184X_FPGA_RAW_REPLAY
CVI_S32 isp_rawreplay_send_usr_pic(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_USR_PIC_INFO_S *picInfo = &pUtCtx->rawReplayInfo;
	VIDEO_FRAME_INFO_S stVideoFrame;
	const VIDEO_FRAME_INFO_S *pstVideoFrame[1];
	VI_PIPE PipeId[] = {0};
	VB_POOL_CONFIG_S cfg = {0};
	CVI_U32 u32BlkSize = 0;
	CVI_U32 i = 0;

	stVideoFrame.stVFrame.enDynamicRange = picInfo->isHdrOn ? DYNAMIC_RANGE_HDR10 : DYNAMIC_RANGE_SDR8;
	stVideoFrame.stVFrame.u32Width = picInfo->u32ImgWidth;
	stVideoFrame.stVFrame.u32Height = picInfo->u32ImgHeight;
	stVideoFrame.stVFrame.s16OffsetLeft = 0;
	stVideoFrame.stVFrame.s16OffsetTop = 0;
	stVideoFrame.stVFrame.s16OffsetRight = 0;
	stVideoFrame.stVFrame.s16OffsetBottom = 0;
	stVideoFrame.stVFrame.enBayerFormat = picInfo->bayFormat;

	u32BlkSize = VI_GetRawBufferSize(stVideoFrame.stVFrame.u32Width,
				stVideoFrame.stVFrame.u32Height, PIXEL_FORMAT_RGB_BAYER_12BPP,
				pUtCtx->isDpcmOn, DEFAULT_ALIGN, 0);

	if (!picInfo->usrBlk[0] && !picInfo->usrBlk[1]) {
		cfg.u32BlkCnt = picInfo->isHdrOn ? 2 : 1;
		cfg.u32BlkSize = u32BlkSize;
		picInfo->poolId = VB_INVALID_POOLID;

		picInfo->poolId = CVI_VB_CreatePool(&cfg);
		if (picInfo->poolId == VB_INVALID_POOLID) {
			UT_PRT("create vb pool failed\n");
			s32Ret = CVI_FAILURE;
			return s32Ret;
		}

		for (i = 0; i < cfg.u32BlkCnt; i++) {
			picInfo->usrBlk[i] = CVI_VB_GetBlock(picInfo->poolId, u32BlkSize);
			if (picInfo->usrBlk[i] == VB_INVALID_HANDLE) {
				UT_PRT("get VB blk failed\n");
				s32Ret = CVI_FAILURE;
				goto destoryPool;
			}

			picInfo->usrPhyAddr[i] = CVI_VB_Handle2PhysAddr(picInfo->usrBlk[i]);
			picInfo->usrVirAddr[i] = CVI_SYS_Mmap(picInfo->usrPhyAddr[i], u32BlkSize);
		}
	}

	stVideoFrame.stVFrame.u64PhyAddr[0] = picInfo->usrPhyAddr[0];
	stVideoFrame.stVFrame.u64PhyAddr[1] = picInfo->usrPhyAddr[1];
	pstVideoFrame[0] = &stVideoFrame;
	CVI_VI_SendPipeRaw(1, PipeId, pstVideoFrame, 80);

	return s32Ret;

destoryPool:
	CVI_VB_DestroyPool(picInfo->poolId);

	return s32Ret;
}
#endif

static CVI_S32 vi_ut_stop_mipi(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DEV ViDev = pstViInfo->stDevInfo.ViDev;
	SNSR_RST_S *pstRstInfo = &pstViInfo->stDevInfo.stRstInfo;
	CVI_S32 mipiDev = pstViInfo->stDevInfo.mipiDev;

	s32Ret = CVI_MIPI_SetSensorReset(mipiDev, pstRstInfo->s32RstPort,
						pstRstInfo->s32RstPin, pstRstInfo->s32RstPol, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] sensor_%d reset failed!\n", ViDev);
		return s32Ret;
	}

	s32Ret = CVI_MIPI_SetSensorClock(mipiDev, 0);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] sensor %d clock enable failed!\n", ViDev);
		return s32Ret;
	}

	s32Ret = CVI_MIPI_SetMipiReset(mipiDev, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] mipi dev_%d reset failed!\n", ViDev);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vi_ut_start_mipi(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DEV ViDev = pstViInfo->stDevInfo.ViDev;
	SNSR_RST_S *pstRstInfo = &pstViInfo->stDevInfo.stRstInfo;
	SNSR_HSETTLE_S *pstHsettle = &pstViInfo->stDevInfo.stHsettle;
	SNS_COMBO_DEV_ATTR_S stDevAttr;
	CVI_S32 mipiDev = pstViInfo->stDevInfo.mipiDev;

	/************************************************
	 * Set sns reset, probe; Set MIPI attr
	 ************************************************/
	s32Ret = CVI_MIPI_SetSensorReset(mipiDev, pstRstInfo->s32RstPort,
						pstRstInfo->s32RstPin, pstRstInfo->s32RstPol, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] sensor_%d reset failed!\n", ViDev);
		return s32Ret;
	}

	s32Ret = CVI_MIPI_SetMipiReset(mipiDev, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] mipi dev_%d reset failed!\n", ViDev);
		return s32Ret;
	}

	if (CVI_SNS_GetSnsRxAttr(ViDev, &stDevAttr) != CVI_SUCCESS) {
		UT_PRT("[ERROR] get mipi dev_%d attr failed!\n", ViDev);
		return CVI_FAILURE;
	}

	if (stDevAttr.input_mode == INPUT_MODE_MIPI) {
		stDevAttr.mipi_attr.dphy.enable = pstHsettle->bHsettlen;
		stDevAttr.mipi_attr.dphy.hs_settle = pstHsettle->u8Hsettle;
	}

	if (stDevAttr.input_mode == INPUT_MODE_MIPI ||
		stDevAttr.input_mode == INPUT_MODE_SUBLVDS ||
		stDevAttr.input_mode == INPUT_MODE_HISPI) {
		stDevAttr.cif_mode = pstViInfo->stDevInfo.enSnsMode;
	}

	s32Ret = CVI_MIPI_SetMipiAttr(ViDev, (CVI_VOID *)&stDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] set mipi dev_%d attr failed!\n", ViDev);
		return s32Ret;
	}

	s32Ret = CVI_MIPI_SetSensorClock(mipiDev, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] sensor %d clock enable failed!\n", ViDev);
		return s32Ret;
	}

	//Wait for the clock to stabilize before setting XCLR, eg. 500ns(IMX327)
	usleep(200 * 1000);

	s32Ret = CVI_MIPI_SetSensorReset(mipiDev, pstRstInfo->s32RstPort,
						pstRstInfo->s32RstPin, pstRstInfo->s32RstPol, 0);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[ERROR] sensor_%d unreset failed!\n", ViDev);
		return s32Ret;
	}

	//Communication start after reset, eg. 20us(IMX327)
	usleep(200 * 1000);

	if (CVI_SNS_SetSnsProbe(ViDev) != CVI_SUCCESS) {
		UT_PRT("[ERROR] sensor_%d probe failed!\n", ViDev);
		return CVI_FAILURE;
	}

	return s32Ret;
}

static CVI_S32 vi_ut_start_snsr(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 j = 0;
	VI_PIPE ViPipe = 0;
	VI_DEV ViDev = pstViInfo->stDevInfo.ViDev;

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[j];

			s32Ret = CVI_SNS_SetVIFlipMirrorCB(ViPipe, ViDev);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_SNS_SetVIFlipMirrorCB failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	//maybe we need split the sensor init to each pipe, but not dev
	s32Ret = CVI_SNS_SetSnsInit(ViDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SNS_SetSnsInit failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 vi_ut_start_isp(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret;
	CVI_S32 j = 0;
	VI_PIPE ViPipe = 0;
	SAMPLE_ISP_CONFIG_S isp_config = {0};

#ifdef CV184X_FPGA_RAW_REPLAY
	SAMPLE_ISP_RAW_REPLAY_CONFIG_S isp_raw_replay_config = {0};

	if (pUtCtx->isRawReplay) {
		isp_raw_replay_config.width = pUtCtx->rawReplayInfo.u32ImgWidth;
		isp_raw_replay_config.height = pUtCtx->rawReplayInfo.u32ImgHeight;

		isp_raw_replay_config.wdr_mode =
			pUtCtx->rawReplayInfo.isHdrOn ? WDR_MODE_2To1_LINE : WDR_MODE_NONE;

		isp_raw_replay_config.frame_rate = pUtCtx->rawReplayInfo.s32FrmRate;

		isp_raw_replay_config.bayer_id = pUtCtx->rawReplayInfo.bayFormat;
		isp_raw_replay_config.is_dpcm = pUtCtx->isDpcmOn;

		isp_config.is_raw_replay_mode = 1;
		isp_config.isp_raw_replay_config = &isp_raw_replay_config;
	}
#endif

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[j];
			s32Ret = SAMPLE_ISP_Init(ViPipe, &isp_config);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("VI_CreateIsp failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

static CVI_S32 vi_ut_stop_isp(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret;
	CVI_S32 j = 0;
	VI_PIPE ViPipe = 0;

#ifdef CV184X_FPGA_RAW_REPLAY
	if (pUtCtx->is_use_isp_raw_replay) {
		stop_raw_replay_offline();
	}
	UT_PRT("raw replay ending!\n");
#endif
	UT_PRT("exit the isp\n");
	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[j];
			s32Ret = SAMPLE_ISP_Exit(ViPipe);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("VI_DestoryIsp failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 vi_ut_init_single(VI_UT_CTX *pUtCtx, CVI_S32 snsrId)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CONFIG_S *pViConfig = &pUtCtx->viConfig;

	UT_PRT("snsrId %d start\n", snsrId);

	if (!pUtCtx->isSkipSensor) {
		s32Ret = vi_ut_start_mipi(&pViConfig->astViInfo[snsrId]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_start_mipi failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = vi_ut_start_dev(&pViConfig->astViInfo[snsrId]);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_start_dev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = vi_ut_start_pipe(&pViConfig->astViInfo[snsrId]);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_start_pipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	if (!pUtCtx->isSkipSensor) {
		s32Ret = vi_ut_start_isp(&pViConfig->astViInfo[snsrId]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_start_isp failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = vi_ut_start_snsr(&pViConfig->astViInfo[snsrId]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("sns init failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = vi_ut_start_chn(&pViConfig->astViInfo[snsrId]);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_start_chn failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 vi_ut_deinit_single(VI_UT_CTX *pUtCtx, CVI_S32 snsrId)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CONFIG_S *pViConfig = &pUtCtx->viConfig;

	UT_PRT("snsrId %d stop\n", snsrId);

	s32Ret = vi_ut_stop_chn(&pViConfig->astViInfo[snsrId]);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_stop_chn failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	if (!pUtCtx->isSkipSensor) {
		s32Ret = vi_ut_stop_isp(&pViConfig->astViInfo[snsrId]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_stop_isp failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = vi_ut_stop_pipe(&pViConfig->astViInfo[snsrId]);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_stop_pipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = vi_ut_stop_dev(&pViConfig->astViInfo[snsrId]);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_stop_dev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	if (!pUtCtx->isSkipSensor) {
		s32Ret = vi_ut_stop_mipi(&pViConfig->astViInfo[snsrId]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_stop_mipi failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}


CVI_S32 vi_ut_multi_vi_init(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret, snsrId;
	bool isInit = true;
	VI_CONFIG_S *pViConfig = &pUtCtx->viConfig;
	int cnt = 0;

	UT_PRT("vi_ut_multi_init_thread start\n");

	while (!atomic_load(&pUtCtx->multiInit.exitFlag)) {
		snsrId = atomic_load(&pUtCtx->multiInit.snsrId);
		if (snsrId == 0) {
			isInit = true;
			sleep(1); // Sleep for 2s to avoid busy loop
		} else if (snsrId == pViConfig->s32ViNum) {
			if (cnt++ > 5 || !pUtCtx->isAutoTest) {
				UT_PRT("isAutoTest[%d], Test[%d]\n", pUtCtx->isAutoTest, cnt);
				break;
			}
			isInit = false;
			sleep(1); // Sleep for 2s to avoid busy loop
		}

		if (!isInit) {
			atomic_fetch_sub(&pUtCtx->multiInit.snsrId, 1);
			snsrId = atomic_load(&pUtCtx->multiInit.snsrId);

			s32Ret = vi_ut_deinit_single(pUtCtx, snsrId);
			if (s32Ret != CVI_SUCCESS)
				break;
		} else {

			s32Ret = vi_ut_init_single(pUtCtx, snsrId);
			if (s32Ret != CVI_SUCCESS)
				break;

			atomic_fetch_add(&pUtCtx->multiInit.snsrId, 1);
		}

		sleep(2); // Sleep for 2s to avoid busy loop
	}

	return s32Ret;
}

CVI_S32 vi_ut_vi_init(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret;
	CVI_S32 i = 0;
	VI_CONFIG_S *pViConfig = &pUtCtx->viConfig;

	if (!pUtCtx->isSkipSensor) {
		for (i = 0; i < pViConfig->s32ViNum; i++) {
			s32Ret = vi_ut_start_mipi(&pViConfig->astViInfo[i]);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi_ut_start_dev failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	if (pUtCtx->isRawReplay) {
		s32Ret = rawreplay_set_usr_pic(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("set usr pic fail\n");
			return s32Ret;
		}

#ifdef CV184X_FPGA_RAW_REPLAY
		if (pUtCtx->is_use_isp_raw_replay) {
			s32Ret = isp_rawreplay_send_usr_pic(pUtCtx);
		} else {
			s32Ret = rawreplay_send_usr_pic(pUtCtx);
		}
#else
		s32Ret = rawreplay_send_usr_pic(pUtCtx);
#endif
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("send usr pic fail\n");
			return s32Ret;
		}
	}

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		s32Ret = vi_ut_start_dev(&pViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_start_dev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		s32Ret = vi_ut_start_pipe(&pViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_start_pipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	if (!pUtCtx->isSkipSensor) {
		for (i = 0; i < pViConfig->s32ViNum; i++) {
			s32Ret = vi_ut_start_isp(&pViConfig->astViInfo[i]);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi_ut_start_isp failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}

		for (i = 0; i < pViConfig->s32ViNum; i++) {
			s32Ret = vi_ut_start_snsr(&pViConfig->astViInfo[i]);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("sns init failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		s32Ret = vi_ut_start_chn(&pViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_start_chn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

#ifdef CV184X_FPGA_RAW_REPLAY
	if (pUtCtx->isRawReplay && pUtCtx->is_use_isp_raw_replay) {
		char board_path[1024] = {0};
		char *raw_replay_dir = getenv("RAW_REPLAY_DIR");

		if (raw_replay_dir == NULL) {
			if (pUtCtx->rawReplayInfo.isHdrOn) {
				printf("Not set the RAW_REPLAY_DIR! So set the default: /mnt/sd/raw/hdr/hdr_indoor\n");
				setenv("RAW_REPLAY_DIR", "/mnt/sd/raw/hdr/hdr_indoor", 1);
			} else {
				printf("Not set the RAW_REPLAY_DIR! So set the default: /mnt/sd/raw/sdr/sdr_still\n");
				setenv("RAW_REPLAY_DIR", "/mnt/sd/raw/sdr/sdr_still", 1);
			}
		}

		raw_replay_dir = getenv("RAW_REPLAY_DIR");

		if (raw_replay_dir == NULL) {
			printf("set the default raw replay path fail!\n");
			return -1;
		} else {
			snprintf(board_path, sizeof(board_path), "%s", raw_replay_dir);
		}

		s32Ret = raw_replay_offline_init(board_path, &(pUtCtx->rawReplayInfo));
		if (s32Ret != 0) {
			printf("raw replay offline init fail!\n");
			return -1;
		} else {
			printf("raw replay offline init success!\n");
		}

		s32Ret = start_raw_replay_offline(0);

		if (s32Ret != 0) {
			printf("raw start fail!\n");
			return -1;
		} else {
			printf("raw start success!\n");
		}


		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("isp send usr pic fail\n");
			return s32Ret;
		}
	}
#endif
	return s32Ret;
}

CVI_S32 vi_ut_vi_deinit(VI_UT_CTX *pUtCtx)
{
	CVI_S32 i = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8 cnt = 0;
	VI_CONFIG_S *pViConfig = &pUtCtx->viConfig;
	VI_USR_PIC_INFO_S *picInfo = &pUtCtx->rawReplayInfo;

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		s32Ret = vi_ut_stop_chn(&pViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_stop_chn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}
	UT_PRT("stop chn success\n");

	if (!pUtCtx->isSkipSensor) {
		for (i = 0; i < pViConfig->s32ViNum; i++) {
			s32Ret = vi_ut_stop_isp(&pViConfig->astViInfo[i]);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi_ut_stop_isp failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		s32Ret = vi_ut_stop_pipe(&pViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_stop_pipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}
	UT_PRT("stop pipe success\n");

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		s32Ret = vi_ut_stop_dev(&pViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_stop_dev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}
	UT_PRT("stop dev success\n");

	for (i = 0; i < pViConfig->s32ViNum; i++) {
		s32Ret = vi_ut_stop_mipi(&pViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_stop_mipi failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	if (!pUtCtx->isRawReplay)
		return s32Ret;

	cnt = picInfo->isHdrOn ? 2 : 1;
	for (i = 0; i < cnt; i++) {
		if (!picInfo->usrBlk[i])
			continue;
		CVI_VB_ReleaseBlock(picInfo->usrBlk[i]);
	}

	s32Ret = CVI_VB_DestroyPool(picInfo->poolId);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_DestroyPool failed with %#x!\n", s32Ret);
		return s32Ret;
	}

#ifdef CV184X_FPGA_RAW_REPLAY
	if (pUtCtx->is_use_isp_raw_replay) {
		raw_replay_offline_uninit();
	}
#endif

	return s32Ret;
}

static CVI_S32 vi_ut_vi_bind_vpss(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
	CVI_S32 j = 0;
	VI_PIPE ViPipe = 0;
	VPSS_GRP VpssGrp = 0;

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[j];
			VpssGrp = j;

			stSrcChn.enModId = CVI_ID_VI;
			stSrcChn.s32DevId = ViPipe;
			stSrcChn.s32ChnId = 0;

			stDestChn.enModId = CVI_ID_VPSS;
			stDestChn.s32DevId = VpssGrp;
			stDestChn.s32ChnId = 0;

			s32Ret = CVI_SYS_Bind(&stSrcChn, &stDestChn);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_SYS_Bind failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

static CVI_S32 vi_ut_vi_unbind_vpss(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
	CVI_S32 j = 0;
	VI_PIPE ViPipe = 0;
	VPSS_GRP VpssGrp = 0;

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[j];
			VpssGrp = j;

			stSrcChn.enModId = CVI_ID_VI;
			stSrcChn.s32DevId = ViPipe;
			stSrcChn.s32ChnId = 0;

			stDestChn.enModId = CVI_ID_VPSS;
			stDestChn.s32DevId = VpssGrp;
			stDestChn.s32ChnId = 0;

			s32Ret = CVI_SYS_UnBind(&stSrcChn, &stDestChn);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_SYS_UnBind failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

static CVI_S32 vi_ut_start_vpss(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 j = 0;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			VpssGrp = pstViInfo->stPipeInfo.aPipe[j];

			stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
			stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
			stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_NV21;
			stVpssGrpAttr.u32MaxW			     = pstViInfo->stDevInfo.stSize.u32Width;
			stVpssGrpAttr.u32MaxH			     = pstViInfo->stDevInfo.stSize.u32Height;
			stVpssGrpAttr.u8VpssDev			     = 1;
			/*start vpss*/
			s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
				return s32Ret;
			}

			astVpssChnAttr[VpssChn].u32Width		    = pstViInfo->stDevInfo.stSize.u32Width;
			astVpssChnAttr[VpssChn].u32Height		    = pstViInfo->stDevInfo.stSize.u32Height;
			astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
			astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_NV12;
			astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
			astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
			astVpssChnAttr[VpssChn].u32Depth		    = 0;
			astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
			astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
			astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_NONE;
			astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &astVpssChnAttr[VpssChn]);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return s32Ret;
			}

			s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return s32Ret;
			}

			/*start vpss*/
			s32Ret = CVI_VPSS_StartGrp(VpssGrp);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
				return s32Ret;
			}

			// for isp tool
			s32Ret = CVI_BIN_SetVpssGrpParams(VpssGrp);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_BIN_SetVpssGrpParams failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 vi_ut_stop_vpss(VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 j = 0;
	VPSS_GRP VpssGrp = 0;

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			VpssGrp = pstViInfo->stPipeInfo.aPipe[j];

			s32Ret = CVI_VPSS_StopGrp(VpssGrp);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_StopGrp failed with %#x!\n", s32Ret);
				return s32Ret;
			}

			s32Ret = CVI_VPSS_DisableChn(VpssGrp, 0);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_DisableChn failed with %#x!\n", s32Ret);
				return s32Ret;
			}

			s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_DestroyGrp failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

static CVI_S32 vi_ut_vpss_init(VI_UT_CTX *pUtCtx)
{
	CVI_S32 i = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	for (i = 0; i < pUtCtx->viConfig.s32ViNum; i++) {
		s32Ret = vi_ut_start_vpss(&pUtCtx->viConfig.astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_start_vpss failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		if (pUtCtx->viVpssMode == VI_ONLINE_VPSS_OFFLINE ||
			pUtCtx->viVpssMode == VI_OFFLINE_VPSS_OFFLINE) {
			s32Ret = vi_ut_vi_bind_vpss(&pUtCtx->viConfig.astViInfo[i]);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi_ut_start_vpss failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

CVI_S32 vi_ut_vpss_deinit(VI_UT_CTX *pUtCtx)
{
	CVI_S32 i = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	for (i = 0; i < pUtCtx->viConfig.s32ViNum; i++) {
		s32Ret = vi_ut_stop_vpss(&pUtCtx->viConfig.astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_start_vpss failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		if (pUtCtx->viVpssMode == VI_ONLINE_VPSS_OFFLINE ||
			pUtCtx->viVpssMode == VI_OFFLINE_VPSS_OFFLINE) {
			s32Ret = vi_ut_vi_unbind_vpss(&pUtCtx->viConfig.astViInfo[i]);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi_ut_vi_unbind_vpss failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

CVI_S32 vi_test(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UT_PRT("isPatgen=%d, isOnlineSc=%d, isRawReplay=%d, isSkipSensor=%d\n",
		pUtCtx->isPatgen, pUtCtx->isOnlineSc, pUtCtx->isRawReplay, pUtCtx->isSkipSensor);

	s32Ret = vi_ut_sys_init(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_plat_sys_init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = vi_ut_set_vi_vpss_mode(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("_sys_config_online_mode failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->multiInit.isInit) {
		s32Ret = vi_ut_multi_vi_init(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_plat_vi_init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	} else {
		s32Ret = vi_ut_vi_init(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_vi_init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	if (pUtCtx->isOnlineSc) {
		s32Ret = vi_ut_vpss_init(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("_vpss_config_online_mode failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

#define AI_ISP_TEST_LOOP_CNT (10)

static pthread_t ai_isp_thread_id;
static volatile CVI_BOOL ai_isp_thread_running = CVI_FALSE;

static void *ai_isp_raw_loop_thread(void *arg)
{
	VI_PIPE ViPipe = (VI_PIPE)(uintptr_t)arg;
	VI_AI_ISP_INFO_WRAP_S wrap_info;
	CVI_S32 s32Ret;
	CVI_U32 loop_cnt = 0;

	UT_PRT("[AI_ISP] raw loop thread start, pipe=%d\n", ViPipe);

	prctl(PR_SET_NAME, "ai_isp_raw_loop");

	while (ai_isp_thread_running) {
		memset(&wrap_info, 0, sizeof(wrap_info));
		wrap_info.viPipe = ViPipe;
		wrap_info.enCmd = VI_CMD_GET_AI_ISP_RAW;

		s32Ret = CVI_VI_AiIspInfo(&wrap_info);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("[AI_ISP] CVI_VI_AiIspInfo GET failed, ret=%#x\n", s32Ret);
			usleep(5000);
			continue;
		}

		if (wrap_info.stIspInfo.inputAddr[0] == 0 || wrap_info.stIspInfo.outputAddr[0] == 0) {
			UT_PRT("[AI_ISP] get raw addr zero, retry\n");
			usleep(5000);
			continue;
		}

		UT_PRT("[AI_ISP] loop %u: input=0x%llx output=0x%llx size=%u\n",
			loop_cnt,
			(unsigned long long)wrap_info.stIspInfo.inputAddr[0],
			(unsigned long long)wrap_info.stIspInfo.outputAddr[0],
			wrap_info.stIspInfo.size);

		// bypass TPU: memcpy input raw to output raw
		{
			CVI_U32 size = wrap_info.stIspInfo.size;
			void *src = CVI_SYS_Mmap(wrap_info.stIspInfo.inputAddr[0], size);
			void *dst = CVI_SYS_Mmap(wrap_info.stIspInfo.outputAddr[0], size);

			if (src && dst) {
				memcpy(dst, src, size);
				CVI_SYS_Munmap(src, size);
				CVI_SYS_Munmap(dst, size);
			} else {
				UT_PRT("[AI_ISP] mmap failed\n");
				if (src)
					CVI_SYS_Munmap(src, size);
				if (dst)
					CVI_SYS_Munmap(dst, size);
			}
		}

		memset(&wrap_info, 0, sizeof(wrap_info));
		wrap_info.viPipe = ViPipe;
		wrap_info.enCmd = VI_CMD_PUT_AI_ISP_RAW;

		s32Ret = CVI_VI_AiIspInfo(&wrap_info);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("[AI_ISP] CVI_VI_AiIspInfo PUT failed, ret=%#x\n", s32Ret);
		}

		loop_cnt++;
		if (loop_cnt >= AI_ISP_TEST_LOOP_CNT) {
			UT_PRT("[AI_ISP] reached test loop count %u, stopping\n", loop_cnt);
			break;
		}
	}

	ai_isp_thread_running = CVI_FALSE;
	UT_PRT("[AI_ISP] raw loop thread end, pipe=%d, loops=%u\n", ViPipe, loop_cnt);
	return NULL;
}

CVI_S32 vi_ut_ai_isp_test(VI_UT_CTX *pUtCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_AI_ISP_CFG_S ai_isp_cfg;

	UT_PRT("[AI_ISP] test start\n");

	s32Ret = vi_ut_sys_init(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] sys init failed, ret=%#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = vi_ut_set_vi_vpss_mode(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] set vi_vpss mode failed, ret=%#x\n", s32Ret);
		return s32Ret;
	}

	// Step 1: AI ISP INIT
	memset(&ai_isp_cfg, 0, sizeof(ai_isp_cfg));
	ai_isp_cfg.viPipe = ViPipe;
	ai_isp_cfg.enAiIspType = VI_AI_ISP_CFG_INIT;
	ai_isp_cfg.reserved[0] = 1; // fp16

	s32Ret = CVI_VI_AiIspCfg(&ai_isp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] INIT failed, ret=%#x\n", s32Ret);
		goto ai_isp_test_deinit;
	}
	UT_PRT("[AI_ISP] INIT success\n");

	s32Ret = vi_ut_vi_init(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] vi init failed, ret=%#x\n", s32Ret);
		return s32Ret;
	}

	// Step 2: AI ISP ENABLE
	memset(&ai_isp_cfg, 0, sizeof(ai_isp_cfg));
	ai_isp_cfg.viPipe = ViPipe;
	ai_isp_cfg.enAiIspType = VI_AI_ISP_CFG_ENABLE;

	s32Ret = CVI_VI_AiIspCfg(&ai_isp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] ENABLE failed, ret=%#x\n", s32Ret);
		goto ai_isp_test_deinit_vi;
	}
	UT_PRT("[AI_ISP] ENABLE success %d\n", ai_isp_cfg.enAiIspType);

	// Step 3: start raw loop thread (get -> bypass memcpy -> put)
	ai_isp_thread_running = CVI_TRUE;
	s32Ret = pthread_create(&ai_isp_thread_id, NULL,
		ai_isp_raw_loop_thread, (void *)(uintptr_t)ViPipe);
	if (s32Ret != 0) {
		UT_PRT("[AI_ISP] create thread failed, ret=%d\n", s32Ret);
		ai_isp_thread_running = CVI_FALSE;
		goto ai_isp_test_disable;
	}

	// wait for thread to finish
	pthread_join(ai_isp_thread_id, NULL);
	UT_PRT("[AI_ISP] raw loop thread joined\n");

ai_isp_test_disable:
	// Step 4: AI ISP DISABLE
	memset(&ai_isp_cfg, 0, sizeof(ai_isp_cfg));
	ai_isp_cfg.viPipe = ViPipe;
	ai_isp_cfg.enAiIspType = VI_AI_ISP_CFG_DISABLE;

	s32Ret = CVI_VI_AiIspCfg(&ai_isp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] DISABLE failed, ret=%#x\n", s32Ret);
	} else {
		UT_PRT("[AI_ISP] DISABLE success\n");
	}

ai_isp_test_deinit_vi:
	// Step 5: AI ISP DEINIT
	memset(&ai_isp_cfg, 0, sizeof(ai_isp_cfg));
	ai_isp_cfg.viPipe = ViPipe;
	ai_isp_cfg.enAiIspType = VI_AI_ISP_CFG_DEINIT;

	s32Ret = CVI_VI_AiIspCfg(&ai_isp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] DEINIT failed, ret=%#x\n", s32Ret);
	} else {
		UT_PRT("[AI_ISP] DEINIT success\n");
	}

	return s32Ret;

ai_isp_test_deinit:
	s32Ret = vi_ut_vi_deinit(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("[AI_ISP] vi deinit failed, ret=%#x\n", s32Ret);
	}

	vi_ut_sys_exit();
	UT_PRT("[AI_ISP] test end\n");

	return s32Ret;
}
