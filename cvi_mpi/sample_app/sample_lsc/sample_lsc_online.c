#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <inttypes.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sample_lsc_cali.h"
#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_sns.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_sns_ctrl.h"
#include "sample_comm.h"

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

static int update_mlsc_gain_lut_attr(ISP_MESH_SHADING_GAIN_LUT_ATTR_S *mlsc_gain_lut_attr, CVI_U16 color_temp,
				int *lsc_r_gain, int *lsc_g_gain, int *lsc_b_gain)

{
	printf("update mlsc gain lut attribute\n");
	int tbl_size = mlsc_gain_lut_attr->Size;
	int insert_idx = -1;
	ISP_MESH_SHADING_GAIN_LUT_S *p_gain_lut = mlsc_gain_lut_attr->LscGainLut;

	// case: table_size != 0, all color temperature is 0
	int real_color_temp_num = 0;

	for (int i = 0; i < tbl_size; ++i) {
		if (p_gain_lut[i].ColorTemperature != 0) {
			real_color_temp_num++;
		}
	}

	mlsc_gain_lut_attr->Size = real_color_temp_num;
	tbl_size = real_color_temp_num;

	for (int i = 0; i < tbl_size; ++i) {
		if (color_temp == p_gain_lut[i].ColorTemperature) {
			ARRAY_CPY(p_gain_lut[i].RGain, lsc_r_gain, CVI_ISP_LSC_GRID_POINTS);
			ARRAY_CPY(p_gain_lut[i].GGain, lsc_g_gain, CVI_ISP_LSC_GRID_POINTS);
			ARRAY_CPY(p_gain_lut[i].BGain, lsc_b_gain, CVI_ISP_LSC_GRID_POINTS);
			return CVI_SUCCESS;
		}
	}

	if (color_temp < p_gain_lut[0].ColorTemperature) {
		insert_idx = 0;
	} else if (color_temp > p_gain_lut[tbl_size-1].ColorTemperature) {
		insert_idx = tbl_size;
	} else {
		for (int i = 0; i < tbl_size - 1; ++i) {
			if (color_temp > p_gain_lut[i].ColorTemperature &&
					color_temp < p_gain_lut[i+1].ColorTemperature) {
				insert_idx = i + 1;
				break;
			}
		}
	}
	// over size, drop one
	if (tbl_size == ISP_MLSC_COLOR_TEMPERATURE_SIZE) {
		int diff_pre = color_temp - p_gain_lut[insert_idx - 1].ColorTemperature;
		int diff_post = p_gain_lut[insert_idx].ColorTemperature - color_temp;

		if (diff_pre < diff_post) {
			insert_idx--;
		}
		p_gain_lut[insert_idx].ColorTemperature = color_temp;
		ARRAY_CPY(p_gain_lut[insert_idx].RGain, lsc_r_gain, CVI_ISP_LSC_GRID_POINTS);
		ARRAY_CPY(p_gain_lut[insert_idx].GGain, lsc_g_gain, CVI_ISP_LSC_GRID_POINTS);
		ARRAY_CPY(p_gain_lut[insert_idx].BGain, lsc_b_gain, CVI_ISP_LSC_GRID_POINTS);
		return CVI_SUCCESS;
	}

	// inside
	if (insert_idx == tbl_size) {
		p_gain_lut[insert_idx].ColorTemperature = color_temp;
		ARRAY_CPY(p_gain_lut[insert_idx].RGain, lsc_r_gain, CVI_ISP_LSC_GRID_POINTS);
		ARRAY_CPY(p_gain_lut[insert_idx].GGain, lsc_g_gain, CVI_ISP_LSC_GRID_POINTS);
		ARRAY_CPY(p_gain_lut[insert_idx].BGain, lsc_b_gain, CVI_ISP_LSC_GRID_POINTS);
	} else {
		for (int i = tbl_size - 1; i >= insert_idx; --i) {
			ARRAY_CPY(p_gain_lut[i+1].RGain, p_gain_lut[i].RGain, CVI_ISP_LSC_GRID_POINTS);
			ARRAY_CPY(p_gain_lut[i+1].GGain, p_gain_lut[i].GGain, CVI_ISP_LSC_GRID_POINTS);
			ARRAY_CPY(p_gain_lut[i+1].BGain, p_gain_lut[i].BGain, CVI_ISP_LSC_GRID_POINTS);
			p_gain_lut[i+1].ColorTemperature = p_gain_lut[i].ColorTemperature;
			if (i == insert_idx) {
				p_gain_lut[insert_idx].ColorTemperature = color_temp;
				ARRAY_CPY(p_gain_lut[insert_idx].RGain, lsc_r_gain, CVI_ISP_LSC_GRID_POINTS);
				ARRAY_CPY(p_gain_lut[insert_idx].GGain, lsc_g_gain, CVI_ISP_LSC_GRID_POINTS);
				ARRAY_CPY(p_gain_lut[insert_idx].BGain, lsc_b_gain, CVI_ISP_LSC_GRID_POINTS);
			}
		}
	}
	// tbl size ++
	mlsc_gain_lut_attr->Size += 1;

	return CVI_SUCCESS;
}

static double get_elapsed_ms(struct timeval *start, struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_usec - start->tv_usec) / 1000.0;
}

static void log_time(const char *prefix, const char *label, double elapsed_ms)
{
	FILE *log_fp = fopen("log.txt", "a");

	if (log_fp) {
		fprintf(log_fp, "[%s] %s: %.2f ms (%.3f s)\n", prefix, label, elapsed_ms, elapsed_ms / 1000.0);
		fclose(log_fp);
	}
	printf("  %s: %.2f ms (%.3f s)\n", label, elapsed_ms, elapsed_ms / 1000.0);
}

static void get_ts_suffix(char *buf, size_t buf_size)
{
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);

	strftime(buf, buf_size, "%Y%m%d%H%M%S", tm);
}

/* create verify output directory: ./result/ret_online_<mode>_<ts>/ */
static int create_verify_output_dir_online(int color_temp, const char *mode_str, char *out_dir, size_t out_size)
{
	char ts[32];

	get_ts_suffix(ts, sizeof(ts));

	char cwd[1024];

	if (!getcwd(cwd, sizeof(cwd))) {
		printf("  fail to get cwd\n");
		return CVI_FAILURE;
	}

	/* create parent result dir first */
	char parent[1024];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(parent, sizeof(parent), "%s/result", cwd);
	mkdir(parent, 0755);
	snprintf(out_dir, out_size, "%s/result/ret_online_%d_%s_%s", cwd, color_temp, mode_str, ts);
#pragma GCC diagnostic pop
	mkdir(out_dir, 0755);

	printf("  verify output dir: %s\n", out_dir);
	return CVI_SUCCESS;
}

/* save lsc_r_gain, lsc_g_gain, lsc_b_gain as 2D grid (num_knot_y rows x num_knot_x cols) */
static int save_lsc_gain(const char *dir_path,
			 int *lsc_r_gain, int *lsc_g_gain, int *lsc_b_gain,
			 int num_knot_x, int num_knot_y)
{
	char ts[32];

	get_ts_suffix(ts, sizeof(ts));

	char filepath[1024];

	snprintf(filepath, sizeof(filepath), "%s/lsc_gain_%s.txt", dir_path, ts);

	FILE *fp = fopen(filepath, "w");

	if (!fp) {
		printf("fail to open gain output file: %s\n", filepath);
		return CVI_FAILURE;
	}

	fprintf(fp, "# LSC Gain Table (num_knot_x=%d, num_knot_y=%d)\n\n", num_knot_x, num_knot_y);

	fprintf(fp, "lsc_r_gain:\n");
	for (int y = 0; y < num_knot_y; y++) {
		for (int x = 0; x < num_knot_x; x++) {
			int idx = y * num_knot_x + x;

			fprintf(fp, "%d", lsc_r_gain[idx]);
			fprintf(fp, (x == num_knot_x - 1) ? "\n" : ", ");
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "lsc_g_gain:\n");
	for (int y = 0; y < num_knot_y; y++) {
		for (int x = 0; x < num_knot_x; x++) {
			int idx = y * num_knot_x + x;

			fprintf(fp, "%d", lsc_g_gain[idx]);
			fprintf(fp, (x == num_knot_x - 1) ? "\n" : ", ");
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "lsc_b_gain:\n");
	for (int y = 0; y < num_knot_y; y++) {
		for (int x = 0; x < num_knot_x; x++) {
			int idx = y * num_knot_x + x;

			fprintf(fp, "%d", lsc_b_gain[idx]);
			fprintf(fp, (x == num_knot_x - 1) ? "\n" : ", ");
		}
	}

	fclose(fp);
	printf("  saved gain table: %s\n", filepath);
	fflush(stdout);
	return CVI_SUCCESS;
}

/* save rlsc params: lsc_radius_gain, center_x, center_y, radius, norm */
static int save_lsc_radius(const char *dir_path,
			   int *lsc_radius_gain,
			   int center_x, int center_y, int radius, int norm)
{
	char ts[32];

	get_ts_suffix(ts, sizeof(ts));

	char filepath[1024];

	snprintf(filepath, sizeof(filepath), "%s/lsc_radius_%s.txt", dir_path, ts);

	FILE *fp = fopen(filepath, "w");

	if (!fp) {
		printf("fail to open radius output file: %s\n", filepath);
		return CVI_FAILURE;
	}

	fprintf(fp, "# LSC Radius Parameters\n");
	fprintf(fp, "center_x  = %d\n", center_x);
	fprintf(fp, "center_y  = %d\n", center_y);
	fprintf(fp, "radius    = %d\n", radius);
	fprintf(fp, "norm      = %d\n\n", norm);

	fprintf(fp, "# lsc_radius_gain (%d elements, x4 channels)\n", ISP_RLSC_WINDOW_SIZE * 4);
	for (int i = 0; i < ISP_RLSC_WINDOW_SIZE * 4; i++) {
		fprintf(fp, "%d", lsc_radius_gain[i]);
		if ((i + 1) % ISP_RLSC_WINDOW_SIZE == 0)
			fprintf(fp, "\n\n");
		else
			fprintf(fp, ", ");
	}

	fclose(fp);
	printf("  saved radius params: %s\n", filepath);
	fflush(stdout);
	return CVI_SUCCESS;
}

/* save packed raw data (before unpack) for debug */
static int save_packed_raw(const char *dir, CVI_U8 *raw_data, CVI_U32 raw_size)
{
	char ts[32];

	get_ts_suffix(ts, sizeof(ts));

	char filepath[512];

	snprintf(filepath, sizeof(filepath), "%s/debug_packed_raw_%s.raw", dir, ts);

	FILE *fp = fopen(filepath, "wb");

	if (!fp) {
		printf("  fail to open packed raw file: %s\n", filepath);
		return CVI_FAILURE;
	}

	fwrite(raw_data, 1, raw_size, fp);
	fclose(fp);
	printf("  saved debug packed raw: %s (%u bytes)\n", filepath, raw_size);
	return CVI_SUCCESS;
}

/* save unpacked raw data (after unpack) for debug */
static int save_debug_unpack_raw(const char *dir, uint16_t *raw_data, int width, int height)
{
	char ts[32];

	get_ts_suffix(ts, sizeof(ts));

	char filepath[512];

	snprintf(filepath, sizeof(filepath), "%s/debug_unpack_raw_%s.raw", dir, ts);

	FILE *fp = fopen(filepath, "wb");

	if (!fp) {
		printf("  fail to open debug unpack raw file: %s\n", filepath);
		return CVI_FAILURE;
	}

	CVI_U32 total = (CVI_U32)width * (CVI_U32)height;

	for (CVI_U32 i = 0; i < total; i++) {
		CVI_U16 val = (CVI_U16)raw_data[i];

		fwrite(&val, sizeof(CVI_U16), 1, fp);
	}

	fclose(fp);
	printf("  saved debug unpack raw: %s (%dx%d)\n", filepath, width, height);
	return CVI_SUCCESS;
}

/* save raw_data_unpack as 16-bit unpacked raw file before verify */
static int save_verify_input_raw(const char *dir, uint16_t *raw_data, int width, int height)
{
	char ts[32];

	get_ts_suffix(ts, sizeof(ts));

	char filepath[512];

	snprintf(filepath, sizeof(filepath), "%s/verify_input_raw_%s.raw", dir, ts);

	FILE *fp = fopen(filepath, "wb");

	if (!fp) {
		printf("  fail to open verify input raw file: %s\n", filepath);
		return CVI_FAILURE;
	}

	CVI_U32 total = (CVI_U32)width * (CVI_U32)height;

	for (CVI_U32 i = 0; i < total; i++) {
		CVI_U16 val = (CVI_U16)raw_data[i];

		fwrite(&val, sizeof(CVI_U16), 1, fp);
	}

	fclose(fp);
	printf("  saved verify input raw: %s (%dx%d)\n", filepath, width, height);
	return CVI_SUCCESS;
}

/* save verify output raw image */
static int save_verify_output_raw(const char *dir, const char *prefix, uint16_t *raw_image,
				  int width, int height)
{
	char ts[32];

	get_ts_suffix(ts, sizeof(ts));

	char filepath[512];

	snprintf(filepath, sizeof(filepath), "%s/%s_%s.raw", dir, prefix, ts);

	FILE *fp = fopen(filepath, "wb");

	if (!fp) {
		printf("  fail to open verify output file: %s\n", filepath);
		return CVI_FAILURE;
	}

	CVI_U32 total = (CVI_U32)width * (CVI_U32)height;

	for (CVI_U32 i = 0; i < total; i++) {
		CVI_U16 val = (CVI_U16)raw_image[i];

		fwrite(&val, sizeof(CVI_U16), 1, fp);
	}

	fclose(fp);
	printf("  saved verify raw: %s (%dx%d)\n", filepath, width, height);
	return CVI_SUCCESS;
}

int run_lsc_calibration_online(int color_temp, int enable_verify)
{
	int ret = CVI_SUCCESS;
	int vi_pipe = 0;
	int vi_chn = 0;
	CVI_U32 width = 2560;
	CVI_U32 height = 1920;
	BAYER_FORMAT_E bayer_id = BAYER_FORMAT_BG;
	CVI_U32 raw_size = 0;
	CVI_U8 *raw_data = NULL;
	RAW_PACK_MODE_E raw_pack_mode = RAW_UNCOMPRESS_UNPACK;

	// get raw info & raw data
	ret = get_raw_info(vi_pipe, vi_chn, &raw_pack_mode, &bayer_id, &raw_size, &width, &height);

	if (ret != CVI_SUCCESS) {
		printf("get raw info fail!\n");
		return CVI_FAILURE;
	}

	CVI_U32 stride = raw_size / height;

	raw_data = (CVI_U8 *)calloc(raw_size, 1);

	if (!raw_data) {
		printf("calloc memory size: %u fail!\n", raw_size);
		return CVI_FAILURE;
	}

	ret = get_raw_data(vi_pipe, raw_data, raw_size);

	if (ret != CVI_SUCCESS) {
		FREE(raw_data);
		printf("get raw data fail!\n");
		return CVI_FAILURE;
	}

	uint16_t *raw_data_unpack = (uint16_t *)calloc(width * height, sizeof(uint16_t));

	if (!raw_data_unpack) {
		FREE(raw_data);
		printf("calloc memory size: %u fail!\n", width * height * sizeof(uint16_t));
		return CVI_FAILURE;
	}

	fflush(stdout);

	ret = unpack_raw(raw_data, raw_data_unpack, width, height, stride, raw_pack_mode);

	if (ret != CVI_SUCCESS) {
		FREE(raw_data);
		FREE(raw_data_unpack);
		return CVI_FAILURE;
	}

	/* ====== online calibration: get/set ISP attrs ====== */

	// get blc, mlsc, rlsc attr
	ISP_BLACK_LEVEL_ATTR_S blc_attr;
	ISP_MESH_SHADING_ATTR_S mlsc_attr;
	ISP_MESH_SHADING_GAIN_LUT_ATTR_S mlsc_gain_lut_attr;
	ISP_RADIAL_SHADING_ATTR_S rlsc_attr;
	ISP_RADIAL_SHADING_GAIN_LUT_ATTR_S rlsc_gain_lut_attr;

	if (CVI_ISP_GetBlackLevelAttr(vi_pipe, &blc_attr) != CVI_SUCCESS) {
		printf("CVI_ISP_GetBlackLevelAttr fail!\n");
		ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
	}

	if (CVI_ISP_GetMeshShadingAttr(vi_pipe, &mlsc_attr) != CVI_SUCCESS) {
		printf("CVI_ISP_GetMeshShadingAttr fail!\n");
		ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
	}

	if (CVI_ISP_GetMeshShadingGainLutAttr(vi_pipe, &mlsc_gain_lut_attr) != CVI_SUCCESS) {
		printf("CVI_ISP_GetMeshShadingGainLutAttr fail!\n");
		ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
	}

	if (CVI_ISP_GetRadialShadingAttr(vi_pipe, &rlsc_attr) != CVI_SUCCESS) {
		printf("CVI_ISP_GetRadialShadingAttr fail!\n");
		ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
	}

	if (CVI_ISP_GetRadialShadingGainLutAttr(vi_pipe, &rlsc_gain_lut_attr) != CVI_SUCCESS) {
		printf("CVI_ISP_GetRadialShadingGainLutAttr fail!\n");
		ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
	}

	int num_knot_x = CVI_ISP_LSC_GRID_COL;
	int num_knot_y = CVI_ISP_LSC_GRID_ROW;
	int calib_flag = 0;
	int fisheye_flag = 0;
	int ob_rr = blc_attr.stAuto.OffsetR[0];
	int ob_gr = blc_attr.stAuto.OffsetGr[0];
	int ob_gb = blc_attr.stAuto.OffsetGb[0];
	int ob_bb = blc_attr.stAuto.OffsetB[0];

	int center_x;
	int center_y;
	int radius = 0;
	int norm;
	int *lsc_r_gain = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
	int *lsc_g_gain = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
	int *lsc_b_gain = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
	int *lsc_radius_gain  = (int *)calloc(ISP_RLSC_WINDOW_SIZE * 4, sizeof(int));

	if (!lsc_r_gain || !lsc_g_gain || !lsc_b_gain || !lsc_radius_gain) {
		printf("alloc the mlsc & rlsc gain fail!\n");
		ret = CVI_FAILURE;
		goto CALI_FAIL_HANDLE;
	}

	int color_tmp_num = mlsc_gain_lut_attr.Size;
	const char *mode_label = (mlsc_attr.MeshCorrectMode == MESH_CHROMA_MODE) ? "chroma" : "chroma_luma";
	struct timeval tv_start, tv_end;

	if (mlsc_attr.MeshCorrectMode == MESH_CHROMA_MODE) {
		printf("mlsc do chroma correction, rlsc do luma correction...\n");
		gettimeofday(&tv_start, NULL);
		// chroma pass
		isp_algo_lsc_calibration(
				raw_data_unpack, width, height, bayer_id,
				num_knot_x, num_knot_y, 1, fisheye_flag,
				ob_rr, ob_gr, ob_gb, ob_bb,
				&center_x, &center_y, &radius, &norm,
				lsc_r_gain, lsc_g_gain, lsc_b_gain, lsc_radius_gain);
		update_mlsc_gain_lut_attr(&mlsc_gain_lut_attr, color_temp, lsc_r_gain, lsc_g_gain, lsc_b_gain);
		// luma pass: use dummy buffers to avoid overwrite chroma gains
		int *dummy_r = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
		int *dummy_g = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
		int *dummy_b = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));

		if (!dummy_r || !dummy_g || !dummy_b) {
			printf("alloc dummy gain fail!\n");
			FREE(dummy_r); FREE(dummy_g); FREE(dummy_b);
			FREE(raw_data_unpack);
			return CVI_FAILURE;
		}
		isp_algo_lsc_calibration(
				raw_data_unpack, width, height, bayer_id,
				num_knot_x, num_knot_y, 2, fisheye_flag,
				ob_rr, ob_gr, ob_gb, ob_bb,
				&center_x, &center_y, &radius, &norm,
				dummy_r, dummy_g, dummy_b, lsc_radius_gain);
		FREE(dummy_r);
		FREE(dummy_g);
		FREE(dummy_b);
		gettimeofday(&tv_end, NULL);
		log_time("online", "calibration (chroma 2-pass)", get_elapsed_ms(&tv_start, &tv_end));
	} else if (mlsc_attr.MeshCorrectMode == MESH_CHROMA_LUMA_MODE) {
		printf("mlsc do chroma and luma correction...\n");
		gettimeofday(&tv_start, NULL);
		isp_algo_lsc_calibration(
				raw_data_unpack, width, height, bayer_id,
				num_knot_x, num_knot_y, calib_flag, fisheye_flag,
				ob_rr, ob_gr, ob_gb, ob_bb,
				&center_x, &center_y, &radius, &norm,
				lsc_r_gain, lsc_g_gain, lsc_b_gain, lsc_radius_gain);
		update_mlsc_gain_lut_attr(&mlsc_gain_lut_attr, color_temp, lsc_r_gain, lsc_g_gain, lsc_b_gain);
		gettimeofday(&tv_end, NULL);
		log_time("online", "calibration (chroma_luma)", get_elapsed_ms(&tv_start, &tv_end));
	}

	/* verify if enabled: save input raw before verify (verify modifies raw_data_unpack in-place) */
	if (enable_verify) {
		char verify_dir[2048];

		create_verify_output_dir_online(color_temp, mode_label, verify_dir, sizeof(verify_dir));

	/* debug: save packed raw (before unpack) and unpacked raw (after unpack) */
		save_packed_raw(verify_dir, raw_data, raw_size);
		save_debug_unpack_raw(verify_dir, raw_data_unpack, width, height);

		save_verify_input_raw(verify_dir, raw_data_unpack, width, height);
		printf("online: running lsc verify...\n");
		fflush(stdout);
		gettimeofday(&tv_start, NULL);

		float blc_rr_gain = 1.0f;
		float blc_gr_gain = 1.0f;
		float blc_gb_gain = 1.0f;
		float blc_bb_gain = 1.0f;

		CVI_U32 img_pixels = width * height;
		uint16_t *lsc_raw_image = (uint16_t *)calloc(img_pixels, sizeof(uint16_t));
		uint16_t *rlsc_raw_image = (uint16_t *)calloc(img_pixels, sizeof(uint16_t));

		if (!lsc_raw_image || !rlsc_raw_image) {
			printf("  alloc verify raw image fail!\n");
			FREE(lsc_raw_image); FREE(rlsc_raw_image);
		} else {
			if (mlsc_attr.MeshCorrectMode == MESH_CHROMA_MODE) {
				isp_algo_lsc_verify_v2(
						raw_data_unpack, width, height, bayer_id,
						num_knot_x, num_knot_y, calib_flag,
						ob_rr, ob_gr, ob_gb, ob_bb,
						blc_rr_gain, blc_gr_gain, blc_gb_gain, blc_bb_gain,
						center_x, center_y, radius, norm,
						lsc_r_gain, lsc_g_gain, lsc_b_gain, lsc_radius_gain,
						lsc_raw_image, rlsc_raw_image);
				save_verify_output_raw(verify_dir, "verify_output_lsc_raw",
						rlsc_raw_image, width, height);
			} else {
				isp_algo_lsc_verify_v1(
						raw_data_unpack, width, height, bayer_id,
						num_knot_x, num_knot_y, calib_flag,
						ob_rr, ob_gr, ob_gb, ob_bb,
						blc_rr_gain, blc_gr_gain, blc_gb_gain, blc_bb_gain,
						center_x, center_y, radius, norm,
						lsc_r_gain, lsc_g_gain, lsc_b_gain, NULL,
						lsc_raw_image, NULL);
				save_verify_output_raw(verify_dir, "verify_output_lsc_raw",
						lsc_raw_image, width, height);
			}

			/* save gain & radius txt to the same verify directory */
			save_lsc_gain(verify_dir, lsc_r_gain, lsc_g_gain, lsc_b_gain, num_knot_x, num_knot_y);
			save_lsc_radius(verify_dir, lsc_radius_gain, center_x, center_y, radius, norm);

			gettimeofday(&tv_end, NULL);
			log_time("online", "verify", get_elapsed_ms(&tv_start, &tv_end));
		}
		FREE(lsc_raw_image);
		FREE(rlsc_raw_image);
	}

	// write rlsc&mlsc attr&gain lut attr
	if (mlsc_attr.MeshCorrectMode == MESH_CHROMA_MODE) {
		printf("update rlsc attr & rlsc gain lut attr...\n");

		if (rlsc_attr.RadiusScaleRGB == 0) {
			rlsc_attr.RadiusScaleRGB = norm;
			rlsc_attr.CenterX = center_x;
			rlsc_attr.CenterY = center_y;
		} else {
			rlsc_attr.RadiusScaleRGB = (rlsc_attr.RadiusScaleRGB * color_tmp_num +
					norm) / (color_tmp_num + 1);
			rlsc_attr.CenterX = (rlsc_attr.CenterX * color_tmp_num + center_x) / (color_tmp_num + 1);
			rlsc_attr.CenterY = (rlsc_attr.CenterY * color_tmp_num + center_y) / (color_tmp_num + 1);
		}

		for (int i = 0; i < ISP_RLSC_WINDOW_SIZE; ++i) {
			if (rlsc_gain_lut_attr.GGain[i] == 0) {
				rlsc_gain_lut_attr.GGain[i] = lsc_radius_gain[i];
			} else {
				rlsc_gain_lut_attr.GGain[i] =
					(rlsc_gain_lut_attr.GGain[i] * color_tmp_num +
					lsc_radius_gain[i]) / (color_tmp_num + 1);
			}
		}
		if (CVI_ISP_SetRadialShadingAttr(vi_pipe, &rlsc_attr) != CVI_SUCCESS) {
			printf("CVI_ISP_SetRadialShadingAttr fail!\n");
			ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
		}

		if (CVI_ISP_SetRadialShadingGainLutAttr(vi_pipe, &rlsc_gain_lut_attr) != CVI_SUCCESS) {
			printf("CVI_ISP_SetRadialShadingGainLutAttr fail!\n");
			ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
		}
	}
	if (CVI_ISP_SetMeshShadingAttr(vi_pipe, &mlsc_attr) != CVI_SUCCESS) {
		printf("CVI_ISP_SetMeshShadingAttr fail!\n");
		ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
	}

	if (CVI_ISP_SetMeshShadingGainLutAttr(vi_pipe, &mlsc_gain_lut_attr) != CVI_SUCCESS) {
		printf("CVI_ISP_SetMeshShadingGainLutAttr fail!\n");
		ret = CVI_FAILURE; goto CALI_FAIL_HANDLE;
	}

CALI_FAIL_HANDLE:
	FREE(lsc_r_gain);
	FREE(lsc_g_gain);
	FREE(lsc_b_gain);
	FREE(lsc_radius_gain);
	FREE(raw_data);
	FREE(raw_data_unpack);

	return ret;
}

/* expose init/deinit for main to call */
int online_sys_vi_init(void)
{
	return sys_vi_init();
}

int online_sys_vi_deinit(void)
{
	return sys_vi_deinit();
}
