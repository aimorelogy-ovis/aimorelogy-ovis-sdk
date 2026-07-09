
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <inttypes.h>
#include <signal.h>

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_sns.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "sample_comm.h"

#define DELAY_500MS() (usleep(500 * 1000))

#define SAMPLE_IR_CALIBRATION_MODE 0
#define SAMPLE_IR_AUTO_MODE        1

static VI_PIPE ViPipe;
static CVI_BOOL g_bEnableRun;

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SENSOR_CFG_S g_stSensorCfg;

static int sys_vi_init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MMF_VERSION_S stVersion;
	LOG_LEVEL_CONF_S log_conf;
	VI_DEV_ATTR_S stVidevAttr;
	CVI_U32 Vb_cnt;

	memset(&stVersion, 0, sizeof(MMF_VERSION_S));
	memset(&log_conf, 0, sizeof(LOG_LEVEL_CONF_S));
	memset(&stVidevAttr, 0, sizeof(VI_DEV_ATTR_S));

	SENSOR_CFG_S *sensor_cfg = &g_stSensorCfg;

	memset(sensor_cfg, 0, sizeof(SENSOR_CFG_S));
	memset(&g_stViConfig, 0, sizeof(SAMPLE_VI_CONFIG_S));

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		printf("[ERROR] CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	CVI_SYS_GetVersion(&stVersion);
	printf("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_INFO;
	CVI_LOG_SetLevelConf(&log_conf);

	/************************************************
	 * Parse sensor cfg ini, init vi config
	 ************************************************/
#define SENSOR_CFG_INI_PATH "/mnt/data/sensor_cfg.ini"
	s32Ret = CVI_SNS_SetIniPath(SENSOR_CFG_INI_PATH);
	if (s32Ret == CVI_FAILURE) {
		printf("[ERROR] set ini path: %s fail\n", SENSOR_CFG_INI_PATH);
		return s32Ret;
	}

	s32Ret = CVI_SNS_ParseIni(sensor_cfg);
	if (s32Ret == CVI_FAILURE) {
		printf("[ERROR] Parse fail\n");
		return s32Ret;
	}

	s32Ret = CVI_SNS_GetConfigInfo(sensor_cfg);
	if (s32Ret == CVI_FAILURE) {
		printf("[ERROR] get sns cfg failed\n");
		return s32Ret;
	}

	s32Ret = CVI_SNS_SetSnsDrvCfg(sensor_cfg);
	if (s32Ret == CVI_FAILURE) {
		printf("[ERROR] set sns_drv failed\n");
		return s32Ret;
	}

	VI_VPSS_MODE_E enViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_TILE;
	SNS_INI_CFG_S *stSnsIniCfg = &sensor_cfg->sns_ini_cfg;
	SNS_CFG_S *stSnsCfg = &sensor_cfg->sns_cfg;

	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		g_stViConfig.s32ViNum = 1 + i;
		g_stViConfig.as32WorkingViId[i] = i;

		g_stViConfig.astViInfo[i].stDevInfo.ViDev = i;
		g_stViConfig.astViInfo[i].stDevInfo.enWDRMode =
			stSnsCfg->enWDRMode[i];

		for (int j = 0; j < WDR_MAX_PIPE_NUM; j++) {
			g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] =
				j == 0 ? i : -1;
		}
		g_stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode =
			enViVpssMode;
		g_stViConfig.astViInfo[i].stPipeInfo.bMultiPipe = CVI_FALSE;
		g_stViConfig.astViInfo[i].stPipeInfo.bVcNumCfged = CVI_FALSE;

		g_stViConfig.astViInfo[i].stChnInfo.ViChn = 0;
		g_stViConfig.astViInfo[i].stChnInfo.enPixFormat =
			stSnsCfg->bBypassIsp[i] ? PIXEL_FORMAT_YUYV :
						  SAMPLE_PIXEL_FORMAT;
		g_stViConfig.astViInfo[i].stChnInfo.enDynamicRange =
			DYNAMIC_RANGE_SDR8;
		g_stViConfig.astViInfo[i].stChnInfo.enVideoFormat =
			VIDEO_FORMAT_LINEAR;
		g_stViConfig.astViInfo[i].stChnInfo.enCompressMode =
			enCompressMode;
	}

	g_stViConfig.stSnsCfg = *stSnsCfg;

	/************************************************
	 * Config vi vpss mode
	 ************************************************/
	VI_VPSS_MODE_S stVIVPSSMode = {0};
	VPSS_MODE_S stVPSSMode = {0};

	s32Ret = CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_SYS_GetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	for (CVI_S32 i = 0; i < g_stViConfig.s32ViNum; i++) {
		stVIVPSSMode.aenMode[i] = g_stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode;
	}

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_SYS_SetVPSSModeEx failed with %#x\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * Set VB config
	 ************************************************/
	CVI_U32 u32BlkSize;
	CVI_U32 u32BlkRotSize;
	VB_CONFIG_S stVbConf;
	SIZE_S stSize;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 0;

	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		Vb_cnt = 0;
		bool createNewPool = true;

		if (stSnsCfg->enFormatMode[i] == (SNS_DATA_TYPE_E) VI_DATA_TYPE_YUV) {
			if (stSnsCfg->enChnMode[i] ==
				(SNS_CHN_MODE_E) VI_WORK_MODE_2Multiplex) {
				Vb_cnt = 6;
			} else if (stSnsCfg->enChnMode[i] ==
				(SNS_CHN_MODE_E) VI_WORK_MODE_3Multiplex) {
				Vb_cnt = 9;
			} else if (stSnsCfg->enChnMode[i] ==
				(SNS_CHN_MODE_E) VI_WORK_MODE_4Multiplex) {
				Vb_cnt = 12;
			} else {
				Vb_cnt = 3;
			}
		} else {
			Vb_cnt = 3;
		}

		stSize.u32Width = stSnsCfg->u32ImageWigth[i];
		stSize.u32Height = stSnsCfg->u32ImageHeight[i];

		u32BlkSize = COMMON_GetPicBufferSize(
			stSize.u32Width, stSize.u32Height,
			g_stViConfig.astViInfo[i].stChnInfo.enPixFormat,
			DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(
			stSize.u32Height, stSize.u32Width,
			g_stViConfig.astViInfo[i].stChnInfo.enPixFormat,
			DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize =
			u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (stVbConf.astCommPool[j].u32BlkSize == u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt += Vb_cnt;
				createNewPool = false;
				break;
			}
		}

		if (createNewPool) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize =
				u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt =
				Vb_cnt;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt]
				.enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.u32MaxPoolCnt++;
			printf("[INFO]  createNewPool VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSnsCfg->u32ImageWigth[i],
						stSnsCfg->u32ImageHeight[i],
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkSize);
		} else {
			printf("[INFO] set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						stVbConf.u32MaxPoolCnt, stSnsCfg->u32ImageWigth[i],
						stSnsCfg->u32ImageHeight[i],
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkCnt,
						stVbConf.astCommPool[stVbConf.u32MaxPoolCnt - 1].u32BlkSize);
		}
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	/************************************************
	 * Set sns reset, probe; Set MIPI attr
	 ************************************************/
	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetSensorReset(stSnsIniCfg->MipiDev[i], stSnsIniCfg->s32RstPort[i],
						stSnsIniCfg->s32RstPin[i], stSnsIniCfg->s32RstPol[i], 1);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d reset failed!\n", i);
			return s32Ret;
		}
	}

	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetMipiReset(stSnsIniCfg->MipiDev[i], 1);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] mipi dev_%d reset failed!\n", i);
			return s32Ret;
		}
	}

	SNS_COMBO_DEV_ATTR_S pstRxAttr;

	memset(&pstRxAttr, 0, sizeof(SNS_COMBO_DEV_ATTR_S));
	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		if (CVI_SNS_GetSnsRxAttr(i, &pstRxAttr) != CVI_SUCCESS) {
			printf("[ERROR] get mipi dev_%d attr failed!\n", i);
			return CVI_FAILURE;
		}
		if (pstRxAttr.input_mode == INPUT_MODE_MIPI) {
			if (stSnsIniCfg->bHsettlen[i]) {
				pstRxAttr.mipi_attr.dphy.enable = 1;
				pstRxAttr.mipi_attr.dphy.hs_settle = stSnsIniCfg->u8Hsettle[i];
			}
		}
		if (pstRxAttr.input_mode == INPUT_MODE_MIPI ||
			pstRxAttr.input_mode == INPUT_MODE_SUBLVDS ||
			pstRxAttr.input_mode == INPUT_MODE_HISPI) {
			pstRxAttr.cif_mode = stSnsIniCfg->enSnsMode;
		}
		s32Ret = CVI_MIPI_SetMipiAttr(i, (CVI_VOID *)&pstRxAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] set mipi dev_%d attr failed!\n", i);
			return s32Ret;
		}
	}

	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetSensorClock(stSnsIniCfg->MipiDev[i], 1);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] sensor %d clock enable failed!\n", i);
			return s32Ret;
		}
	}

	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		s32Ret = CVI_MIPI_SetSensorReset(stSnsIniCfg->MipiDev[i], stSnsIniCfg->s32RstPort[i],
							stSnsIniCfg->s32RstPin[i], stSnsIniCfg->s32RstPol[i], 0);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d unreset failed!\n", i);
			return s32Ret;
		}
	}

	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		if (CVI_SNS_SetSnsProbe(i) != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d probe failed!\n", i);
			return CVI_FAILURE;
		}
	}

	/************************************************
	 * Set VI dev config
	 ************************************************/
	VI_DEV_ATTR_S       stViDevAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;

	memset(&stViDevAttr, 0, sizeof(VI_DEV_ATTR_S));
	memset(&stViDevBindAttr, 0, sizeof(VI_DEV_BIND_PIPE_S));
	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		stViDevAttr.snrFps				= stSnsCfg->f32FrameRate[i];
		stViDevAttr.stSize.u32Width		= stSnsCfg->u32ImageWigth[i];
		stViDevAttr.stSize.u32Height	= stSnsCfg->u32ImageHeight[i];
		stViDevAttr.enIntfMode			= (VI_INTF_MODE_E)stSnsCfg->enInterFaceMode[i];
		stViDevAttr.enInputDataType		= (VI_DATA_TYPE_E)stSnsCfg->enFormatMode[i];
		stViDevAttr.enDataSeq			= (VI_YUV_DATA_SEQ_E)stSnsCfg->enYuvFormat[i];
		stViDevAttr.stWDRAttr.enWDRMode	= g_stViConfig.astViInfo[i].stDevInfo.enWDRMode;
		stViDevAttr.enWorkMode			= (VI_WORK_MODE_E)stSnsCfg->enChnMode[i];
		stViDevAttr.enBayerFormat       = (BAYER_FORMAT_E)stSnsCfg->enBayerFormat[i];
		stViDevBindAttr.PipeId[0]		= stSnsIniCfg->MipiDev[i];
		stViDevBindAttr.u32Num			= 1;
		stViDevBindAttr.MipiDev			= stSnsIniCfg->MipiDev[i];

		s32Ret = CVI_VI_SetDevAttr(g_stViConfig.astViInfo[i].stDevInfo.ViDev, &stViDevAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}
		s32Ret = CVI_VI_SetDevBindAttr(g_stViConfig.astViInfo[i].stDevInfo.ViDev, &stViDevBindAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VI_EnableDev(g_stViConfig.astViInfo[i].stDevInfo.ViDev);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_EnableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	/************************************************
	 * Set VI pipe config
	 ************************************************/
	VI_PIPE_ATTR_S stPipeAttr;

	memset(&stPipeAttr, 0, sizeof(VI_PIPE_ATTR_S));
	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		stPipeAttr.u32MaxW						= stSnsCfg->u32ImageWigth[i];
		stPipeAttr.u32MaxH						= stSnsCfg->u32ImageHeight[i];
		stPipeAttr.enPixFmt						= PIXEL_FORMAT_RGB_BAYER_12BPP;
		stPipeAttr.enBitWidth					= DATA_BITWIDTH_12;
		stPipeAttr.stFrameRate.s32SrcFrameRate	= -1;
		stPipeAttr.stFrameRate.s32DstFrameRate	= -1;
		stPipeAttr.bNrEn						= CVI_TRUE;
		stPipeAttr.bYuvBypassPath				= stSnsCfg->bBypassIsp[i];
		stPipeAttr.enCompressMode				=
							g_stViConfig.astViInfo[i].stChnInfo.enCompressMode;

		for (int j = 0; j < VI_MAX_PIPE_NUM; j++) {
			if (g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] >= 0 &&
				g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				s32Ret = CVI_VI_CreatePipe(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j], &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					printf("[ERROR] CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

				s32Ret = CVI_VI_StartPipe(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[j]);
				if (s32Ret != CVI_SUCCESS) {
					printf("[ERROR] CVI_VI_StartPipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}
			}
		}
	}

	/************************************************
	 * start isp
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_CreateIsp(&g_stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		printf("VI_CreateIsp failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * Set sensor init
	 ************************************************/
	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		if (CVI_SNS_SetSnsInit(i) != CVI_SUCCESS) {
			printf("[ERROR] sensor_%d init failed!\n", i);
			return CVI_FAILURE;
		}
	}

	/************************************************
	 * Set VI chn config
	 ************************************************/
	VI_CHN_ATTR_S stChnAttr;

	memset(&stChnAttr, 0, sizeof(VI_CHN_ATTR_S));
	for (int i = 0; i < stSnsIniCfg->devNum; i++) {
		stChnAttr.stSize.u32Width = stSnsCfg->u32ImageWigth[i];
		stChnAttr.stSize.u32Height = stSnsCfg->u32ImageHeight[i];
		stChnAttr.enDynamicRange = g_stViConfig.astViInfo[i].stChnInfo.enDynamicRange;
		stChnAttr.enVideoFormat  = g_stViConfig.astViInfo[i].stChnInfo.enVideoFormat;
		stChnAttr.enCompressMode = g_stViConfig.astViInfo[i].stChnInfo.enCompressMode;
		stChnAttr.enPixelFormat = g_stViConfig.astViInfo[i].stChnInfo.enPixFormat;
		stChnAttr.u32Depth = 0;
		stChnAttr.u32BindVbPool = -1;

		/* fill the sensor orientation */
		stChnAttr.bMirror = false;
		stChnAttr.bFlip = false;

		s32Ret = CVI_VI_SetChnAttr(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[0],
						g_stViConfig.astViInfo[i].stChnInfo.ViChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		if (CVI_SNS_SetVIFlipMirrorCB(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[0],
				g_stViConfig.astViInfo[i].stDevInfo.ViDev) != CVI_SUCCESS) {
			printf("[ERROR] CVI_SNS_SetVIFlipMirrorCB failed!\n");
		}

		s32Ret = CVI_VI_EnableChn(g_stViConfig.astViInfo[i].stPipeInfo.aPipe[0],
									g_stViConfig.astViInfo[i].stChnInfo.ViChn);
		if (s32Ret != CVI_SUCCESS) {
			printf("[ERROR] CVI_VI_EnableChn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}
	return CVI_SUCCESS;
}

static int sys_vi_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ViNum;
	CVI_S32 i;
	SAMPLE_VI_INFO_S stViInfo;
	VI_CHN ViChn;
	VI_PIPE ViPipe = 0;
	VI_DEV ViDev;

	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);

	for (i = 0; i < g_stViConfig.s32ViNum; i++) {
		s32ViNum  = g_stViConfig.as32WorkingViId[i];
		stViInfo = g_stViConfig.astViInfo[s32ViNum];

		/************************************************
		 *  VI chn stop
		 ************************************************/
		ViChn  = stViInfo.stChnInfo.ViChn;
		for (i = 0; i < WDR_MAX_PIPE_NUM; i++) {
			if (stViInfo.stPipeInfo.aPipe[i] >= 0 && stViInfo.stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
				ViPipe = stViInfo.stPipeInfo.aPipe[i];
				s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VI_DisableChn failed with %#x!\n", s32Ret);
					return s32Ret;
				}
			}
		}

		/************************************************
		 *  VI pipe stop
		 ************************************************/
		for (i = 0; i < WDR_MAX_PIPE_NUM; i++) {
			if (stViInfo.stPipeInfo.aPipe[i] >= 0  && stViInfo.stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
				ViPipe = stViInfo.stPipeInfo.aPipe[i];
				s32Ret = CVI_VI_StopPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VI_StopPipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

				s32Ret = CVI_VI_DestroyPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

			}
		}

		/************************************************
		 *  VI dev stop
		 ************************************************/
		ViDev   = stViInfo.stDevInfo.ViDev;
		s32Ret  = CVI_VI_DisableDev(ViDev);

		CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev);
		CVI_VI_UnRegPmCallBack(ViDev);

		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VI_DisableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	CVI_VB_Exit();
	CVI_SYS_Exit();
	return CVI_SUCCESS;
}

static void signal_handler(int signo)
{
	if (g_bEnableRun) {
		signal(signo, SIG_IGN);
		g_bEnableRun = CVI_FALSE;
	} else {
		exit(-1);
	}
}

static void print_usage(char *sPrgNm)
{
	printf("Usage : %s <mode> <u32Normal2IrIsoThr> <u32Ir2NormalIsoThr> ", sPrgNm);
	printf("<u32RGMax> <u32RGMin> <u32BGMax> <u32BGMin> <enIrStatus>\n");

	printf("mode:\n");
	printf("\t 0) SAMPLE_IR_CALIBRATION_MODE.\n");
	printf("\t 1) SAMPLE_IR_AUTO_MODE.\n");

	printf("u32Normal2IrIsoThr:\n");
	printf("\t ISO threshold of switching from normal to IR mode.\n");

	printf("u32Ir2NormalIsoThr:\n");
	printf("\t ISO threshold of switching from IR to normal mode.\n");

	printf("u32RGMax/u32RGMin/u32BGMax/u32BGMin:\n");
	printf("\t Maximum(Minimum) value of R/G(B/G) in IR scene.\n");

	printf("enIrStatus:\n");
	printf("\t Current IR status. 0: Normal mode; 1: IR mode.\n");

	printf("e.g : %s 0 (SAMPLE_IR_CALIBRATION_MODE)\n", sPrgNm);
	printf("e.g : %s 1 16000 400 280 190 280 190 0 (SAMPLE_IR_AUTO_MODE, user_define parameters)\n", sPrgNm);
}

static void switch_to_ir(void)
{
	// 1. switch pq BIN
	// 2. switch ir cut

	printf("\nNormal --> IR\n");
}

static void switch_to_normal(void)
{
	// 1. switch pq BIN
	// 2. switch ir cut

	printf("\nIR --> Normal\n");
}

static void get_ae_awb_info(CVI_U32 *u32ISO, CVI_U32 *u32RGgain, CVI_U32 *u32BGgain)
{
#define IR_WB_GAIN_FORMAT	256
#define IR_DIV_0_TO_1(a)	((0 == (a)) ? 1 : (a))

	ISP_EXP_INFO_S aeInfo;
	CVI_U16 grayWorldRgain, grayWorldBgain;

	CVI_ISP_QueryExposureInfo(ViPipe, &aeInfo);
	CVI_ISP_GetGrayWorldAwbInfo(ViPipe, &grayWorldRgain, &grayWorldBgain);

	*u32ISO = aeInfo.u32ISO;
	*u32RGgain = IR_WB_GAIN_FORMAT * 1024 / IR_DIV_0_TO_1(grayWorldRgain);
	*u32BGgain = IR_WB_GAIN_FORMAT * 1024 / IR_DIV_0_TO_1(grayWorldBgain);
}

static int run_calibration(int argc, char **argv)
{
#define GAIN_MAX_COEF 280
#define GAIN_MIN_COEF 190

	CVI_U32 u32ISO;
	CVI_U32 RGgain, BGgain;

	UNUSED(argc);
	UNUSED(argv);

	switch_to_ir();

	while (g_bEnableRun) {

		get_ae_awb_info(&u32ISO, &RGgain, &BGgain);

		printf("\n");
		printf("ISO: %d, RGgain: %d, BGgain: %d\n", u32ISO, RGgain, BGgain);
		printf("Reference range: RGMax: %d, RGMin: %d, BGMax: %d, BGMin: %d\n",
			(RGgain * GAIN_MAX_COEF) >> 8, (RGgain * GAIN_MIN_COEF) >> 8,
			(BGgain * GAIN_MAX_COEF) >> 8, (BGgain * GAIN_MIN_COEF) >> 8);

		DELAY_500MS();
	}

	return CVI_SUCCESS;
}

#define ENABLE_RUN_IR_AUTO_DEBUG

#ifdef ENABLE_RUN_IR_AUTO_DEBUG
static void print_ae_awb_info(void)
{
	CVI_U32 u32ISO;
	CVI_U32 RGgain, BGgain;

	get_ae_awb_info(&u32ISO, &RGgain, &BGgain);
	printf("Current, ISO: %d, RGgain: %d, BGgain: %d\n", u32ISO, RGgain, BGgain);
}
#endif

static int run_ir_auto(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

#define STABLE_COUNT_THR   3
	CVI_U8 u8StableCount = 0;

	ISP_IR_AUTO_ATTR_S stIrAttr;

	stIrAttr.bEnable = CVI_TRUE;

	if (argc > 7) {
		stIrAttr.u32Normal2IrIsoThr = atoi(argv[2]);
		stIrAttr.u32Ir2NormalIsoThr = atoi(argv[3]);
		stIrAttr.u32RGMax = atoi(argv[4]);
		stIrAttr.u32RGMin = atoi(argv[5]);
		stIrAttr.u32BGMax = atoi(argv[6]);
		stIrAttr.u32BGMin = atoi(argv[7]);
		stIrAttr.enIrStatus = atoi(argv[8]);

		if (stIrAttr.enIrStatus != ISP_IR_STATUS_NORMAL &&
			stIrAttr.enIrStatus != ISP_IR_STATUS_IR) {
			printf("the mode is invalid!\n");
			goto exit;
		}
	} else {
		printf("Invalid parameter!\n");
		goto exit;
	}

	while (g_bEnableRun) {

#ifdef ENABLE_RUN_IR_AUTO_DEBUG
		printf("\n");
		printf("input, u32Normal2IrIsoThr: %d, u32Ir2NormalIsoThr: %d, ",
			stIrAttr.u32Normal2IrIsoThr,
			stIrAttr.u32Ir2NormalIsoThr);
		printf("RG: %d - %d, BG: %d - %d, enIrStatus: %d\n",
			stIrAttr.u32RGMax,
			stIrAttr.u32RGMin,
			stIrAttr.u32BGMax,
			stIrAttr.u32BGMin,
			stIrAttr.enIrStatus);
		print_ae_awb_info();
#endif

		s32Ret = CVI_ISP_IrAutoRunOnce(ViPipe, &stIrAttr);

#ifdef ENABLE_RUN_IR_AUTO_DEBUG
		printf("enIrSwitch: %d, u8StableCount: %d\n", stIrAttr.enIrSwitch, u8StableCount);
#endif

		if (stIrAttr.enIrSwitch == ISP_IR_SWITCH_TO_IR &&
			u8StableCount++ > STABLE_COUNT_THR) {

			switch_to_ir();

			stIrAttr.enIrStatus = ISP_IR_STATUS_IR;

		} else if (stIrAttr.enIrSwitch == ISP_IR_SWITCH_TO_NORMAL &&
			u8StableCount++ > STABLE_COUNT_THR) {

			switch_to_normal();

			stIrAttr.enIrStatus = ISP_IR_STATUS_NORMAL;

		}

		if (stIrAttr.enIrSwitch == ISP_IR_SWITCH_NONE) {
			u8StableCount = 0;
		}

		DELAY_500MS();
	}

	return s32Ret;

exit:
	print_usage(argv[0]);
	return CVI_FAILURE;
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32Mode = SAMPLE_IR_CALIBRATION_MODE;

	if (argc < 2) {
		print_usage(argv[0]);
		return CVI_FAILURE;
	}

	u32Mode = atoi(argv[1]);

	if (u32Mode != SAMPLE_IR_CALIBRATION_MODE &&
		u32Mode != SAMPLE_IR_AUTO_MODE) {
		printf("the mode is invalid!\n");
		print_usage(argv[0]);
		return CVI_FAILURE;
	}

	s32Ret = sys_vi_init();
	if (s32Ret != CVI_SUCCESS) {
		printf("sys vi init failed!\n");
		return s32Ret;
	}

	ViPipe = 0;
	g_bEnableRun = CVI_TRUE;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	if (u32Mode == SAMPLE_IR_CALIBRATION_MODE) {
		s32Ret = run_calibration(argc, argv);
	} else if (u32Mode == SAMPLE_IR_AUTO_MODE) {
		s32Ret = run_ir_auto(argc, argv);
	}

	sys_vi_deinit();

	return s32Ret;
}

