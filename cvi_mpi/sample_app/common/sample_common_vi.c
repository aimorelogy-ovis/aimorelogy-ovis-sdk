#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cvi_mipi.h"
#include "cvi_sns_ctrl.h"
#include "cvi_awb_comm.h"
#include "cvi_af_comm.h"
#include "sample_comm.h"

SENSOR_CFG_S gstSensorCfg = {0};

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

CVI_S32 SAMPLE_COMM_INI_SensorCfg(SENSOR_CFG_S *p_sns_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (p_sns_cfg == NULL) {
		return CVI_FAILURE;
	}

	memcpy(&gstSensorCfg, p_sns_cfg, sizeof(SENSOR_CFG_S));

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_GetDevnumBySnsmode(CVI_SNS_TYPE_E enSnsType)
{
	CVI_S32 i;

	for (i = 0; i < gstSensorCfg.sns_ini_cfg.devNum; i++) {
		if (gstSensorCfg.sns_ini_cfg.enSnsType[i] == enSnsType)
			return i;
	}

	return 0;
}

CVI_S32 SAMPLE_COMM_SnsIni2Vicfg(SNS_INI_CFG_S *pstIniCfg, SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32SnsId = 0;
	CVI_S32 pipeIdx = 0, chnMax = 0, i = 0;
	DYNAMIC_RANGE_E enDynamicRange		= DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E enPixFormat		= VI_PIXEL_FORMAT;
	VIDEO_FORMAT_E enVideoFormat		= VIDEO_FORMAT_LINEAR;
	SAMPLE_VI_INFO_S *viInfo = NULL;

	if (!pstIniCfg) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	if (!pstViConfig) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	pstViConfig->s32ViNum = pstIniCfg->devNum;
	for (s32SnsId = 0; s32SnsId < pstIniCfg->devNum; s32SnsId++) {
		viInfo = &pstViConfig->astViInfo[s32SnsId];
		pstViConfig->as32WorkingViId[s32SnsId]		= s32SnsId;

		viInfo->stDevInfo.ViDev				= s32SnsId;
		viInfo->stDevInfo.enWDRMode			= pstViConfig->stSnsCfg.enWDRMode[s32SnsId];
		viInfo->stDevInfo.mipiDev			= pstIniCfg->MipiDev[s32SnsId];
		viInfo->stDevInfo.rstpin			= pstIniCfg->s32RstPin[s32SnsId];
		viInfo->stDevInfo.rstpol			= pstIniCfg->s32RstPol[s32SnsId];
		viInfo->stDevInfo.rstport			= pstIniCfg->s32RstPort[s32SnsId];

		viInfo->stSnsInfo.enSnsType = pstIniCfg->enSnsType[s32SnsId];
		viInfo->stSnsInfo.s32SnsId = s32SnsId;
		viInfo->stSnsInfo.s32ModeId = pstIniCfg->enSnsMode;
		viInfo->stSnsInfo.s32BusId = pstIniCfg->s32BusId[s32SnsId];
		viInfo->stSnsInfo.s32SnsI2cAddr = pstIniCfg->s32BusId[s32SnsId];
		viInfo->stSnsInfo.s32BusId = pstIniCfg->MipiDev[s32SnsId];
		viInfo->stSnsInfo.u8HwSync = pstIniCfg->u8HwSync[s32SnsId];
		viInfo->stSnsInfo.u8Orien = pstIniCfg->u8Orien[s32SnsId];
		viInfo->stSnsInfo.u8Hsettle = pstIniCfg->u8Hsettle[s32SnsId];
		viInfo->stSnsInfo.bHsettlen = pstIniCfg->bHsettlen[s32SnsId];
		viInfo->stSnsInfo.s32RstPin = pstIniCfg->s32RstPin[s32SnsId];

		viInfo->stDevInfo.enYuvFormat		= pstViConfig->stSnsCfg.enYuvFormat[s32SnsId];
		viInfo->stDevInfo.enFormatMode		= pstViConfig->stSnsCfg.enFormatMode[s32SnsId];
		viInfo->stDevInfo.enBayerFormat		= pstViConfig->stSnsCfg.enBayerFormat[s32SnsId];
		viInfo->stDevInfo.enInterFaceMode	= pstViConfig->stSnsCfg.enInterFaceMode[s32SnsId];
		viInfo->stDevInfo.enChnMode		    = pstViConfig->stSnsCfg.enChnMode[s32SnsId];

		viInfo->stDevInfo.fps					= pstViConfig->stSnsCfg.f32FrameRate[s32SnsId];
		viInfo->stDevInfo.stSize.u32Width		= pstViConfig->astViInfo[0].stDevInfo.bPatgen ? 1920 : pstViConfig->stSnsCfg.u32ImageWigth[s32SnsId];
		viInfo->stDevInfo.stSize.u32Height		= pstViConfig->astViInfo[0].stDevInfo.bPatgen ? 1080 : pstViConfig->stSnsCfg.u32ImageHeight[s32SnsId];

		chnMax = (viInfo->stDevInfo.enFormatMode != SNS_DATA_TYPE_YUV) ?
				 1 :
				 viInfo->stDevInfo.enChnMode + 1;

		// set pipe info
		for (i = 0; i < chnMax; i++) {
			viInfo->stPipeInfo.aPipe[i] = pipeIdx++;
		}

		for (; i < VI_MAX_PIPE_NUM; i++) {
			viInfo->stPipeInfo.aPipe[i] = -1;
		}

		viInfo->stPipeInfo.enCompressMode	= COMPRESS_MODE_NONE;
		viInfo->stPipeInfo.bIspBypass		= pstViConfig->stSnsCfg.bBypassIsp[s32SnsId];

		viInfo->stChnInfo.ViChn				= 0;
		viInfo->stChnInfo.enPixFormat			= enPixFormat;
		viInfo->stChnInfo.enDynamicRange		= enDynamicRange;
		viInfo->stChnInfo.enVideoFormat			= enVideoFormat;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_INI_INIT(SAMPLE_VI_CONFIG_S *pstViConfig, SNS_INI_CFG_S *pstSnsIniCfg, VB_CONFIG_S *pstVbConfig)
{
	CVI_S32		s32Ret;
	CVI_S32		i;
	CVI_U32		u32BlkSize, u32BlkRotSize;
	SENSOR_CFG_S stSensorCfg = {0};

	// Get config from ini if found.
	s32Ret = CVI_SNS_ParseIni(&stSensorCfg);
	if (s32Ret == CVI_FAILURE) {
		SAMPLE_PRT("Not find sensor_cfg.ini in /mnt/data/, use pattern\n");
		stSensorCfg.sns_ini_cfg.devNum = 1;
		pstViConfig->astViInfo[0].stDevInfo.bPatgen = CVI_TRUE;
	} else {
		s32Ret = CVI_SNS_GetConfigInfo(&stSensorCfg);
		if (s32Ret == CVI_FAILURE) {
			SAMPLE_PRT("[ERROR] get sns cfg failed\n");
			return s32Ret;
		}

		s32Ret = CVI_SNS_SetSnsDrvCfg(&stSensorCfg);
		if (s32Ret == CVI_FAILURE) {
			SAMPLE_PRT("[ERROR] set sns_drv failed\n");
		}
	}

	s32Ret = SAMPLE_COMM_INI_SensorCfg(&stSensorCfg);
	if (s32Ret == CVI_FAILURE) {
		SAMPLE_PRT("[ERROR] init global sensor cfg failed\n");
	}

	memcpy(pstSnsIniCfg, &gstSensorCfg.sns_ini_cfg, sizeof(SNS_INI_CFG_S));
	memcpy(&pstViConfig->stSnsCfg, &gstSensorCfg.sns_cfg, sizeof(pstViConfig->stSnsCfg));

	/************************************************
	 * Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_SnsIni2Vicfg(pstSnsIniCfg, pstViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	/************************************************
	 * Get input size
	 ************************************************/
	memset(pstVbConfig, 0, sizeof(VB_CONFIG_S));
	pstVbConfig->u32MaxPoolCnt = 0;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		int Vb_cnt = 0;
		bool createNewPool = true;
		SAMPLE_VI_INFO_S *pViInfo = &pstViConfig->astViInfo[i];
		SAMPLE_DEV_INFO_S *pDevInfo = &pViInfo->stDevInfo;

		if (pstViConfig->stSnsCfg.enFormatMode[i] == SNS_DATA_TYPE_YUV) {
			if (pstViConfig->stSnsCfg.enChnMode[i] == SNS_CHN_MODE_2Multiplex) {
				Vb_cnt = 6;
			} else if (pstViConfig->stSnsCfg.enChnMode[i] == SNS_CHN_MODE_3Multiplex) {
				Vb_cnt = 9;
			} else if (pstViConfig->stSnsCfg.enChnMode[i] == SNS_CHN_MODE_4Multiplex) {
				Vb_cnt = 12;
			} else {
				Vb_cnt = 3;
			}
		} else {
			Vb_cnt = 5;
		}

		u32BlkSize = COMMON_GetPicBufferSize(pDevInfo->stSize.u32Width, pDevInfo->stSize.u32Height,
				VI_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(pDevInfo->stSize.u32Height, pDevInfo->stSize.u32Width,
				VI_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

		for (CVI_U32 j = 0; j < pstVbConfig->u32MaxPoolCnt; j++) {
			if (pstVbConfig->astCommPool[j].u32BlkSize == u32BlkSize) {
				pstVbConfig->astCommPool[j].u32BlkCnt += Vb_cnt;
				createNewPool = false;
				break;
			}
		}
		if (createNewPool) {
			pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkCnt = Vb_cnt;
			pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("[INFO] set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						pstVbConfig->u32MaxPoolCnt, pDevInfo->stSize.u32Width,
						pDevInfo->stSize.u32Height,
						pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkCnt,
						pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkSize);
			pstVbConfig->u32MaxPoolCnt++;
		} else {
			SAMPLE_PRT("[INFO] set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
						pstVbConfig->u32MaxPoolCnt, pDevInfo->stSize.u32Width,
						pDevInfo->stSize.u32Height,
						pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkCnt,
						pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkSize);
		}
	}

	if (pstVbConfig->u32MaxPoolCnt == 1) {
		pstVbConfig->astCommPool[0].u32BlkCnt += 2;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_ResetSensor(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_S32 devno = 0, rstport, rstpin, rstpol;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stDevInfo.mipiDev;
		rstport = pstViInfo->stDevInfo.rstport;
		rstpin = pstViInfo->stDevInfo.rstpin;
		rstpol = pstViInfo->stDevInfo.rstpol;
		s32Ret = CVI_MIPI_SetSensorReset(devno, rstport, rstpin, rstpol, 1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d reset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_ResetMipi(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_S32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stDevInfo.mipiDev;
		s32Ret = CVI_MIPI_SetMipiReset(devno, 1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "mipi %d reset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_UnresetSensor(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_S32 devno = 0, rstport, rstpin, rstpol;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stDevInfo.mipiDev;
		rstport = pstViInfo->stDevInfo.rstport;
		rstpin = pstViInfo->stDevInfo.rstpin;
		rstpol = pstViInfo->stDevInfo.rstpol;
		s32Ret = CVI_MIPI_SetSensorReset(devno, rstport, rstpin, rstpol, 0);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d unreset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_UnresetMipi(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_S32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stDevInfo.mipiDev;
		s32Ret = CVI_MIPI_SetMipiReset(devno, 0);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("mipi %d unreset failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_SetMipiAttr(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	SNS_COMBO_DEV_ATTR_S stDevAttr;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {

		s32Ret = CVI_SNS_GetSnsRxAttr(i, &stDevAttr);
		if (s32Ret!= CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] get mipi dev_%d attr failed!\n", i);
			return s32Ret;
		}

		s32Ret = CVI_MIPI_SetMipiAttr(i, (CVI_VOID *)&stDevAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] set mipi dev_%d attr failed!\n", i);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_EnableSensorClock(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = 0, i;
	CVI_S32 devno = 0;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		devno = pstViInfo->stDevInfo.mipiDev;
		s32Ret = CVI_MIPI_SetSensorClock(devno, 1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d clock enable failed!\n", i);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StartMIPI(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	/*TODO@CF. Need add sample function.*/
	if(pstViConfig->astViInfo[0].stDevInfo.bPatgen){
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_ResetSensor(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ResetSensor failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_ResetMipi(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ResetMipi failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_SetMipiAttr(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_SetMipiAttr failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_EnableSensorClock(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "EnableSensorClock failed! with %#x!\n", s32Ret);
		return s32Ret;
	}

	usleep(20);
	s32Ret = SAMPLE_COMM_VI_UnresetSensor(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "UnresetSensor failed! with %#x!\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StartDev(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret;
	VI_DEV ViDev;
	CVI_S32             s32PipeCnt = 0;
	CVI_S32             i;
	VI_DEV_ATTR_S stViDevAttr;
	VI_DEV_BIND_PIPE_S stViDevBindAttr;
	CVI_BOOL bPatgen = CVI_FALSE;

	ViDev = pstViInfo->stDevInfo.ViDev;
	bPatgen = pstViInfo->stDevInfo.bPatgen;

	stViDevAttr.snrFps 				= bPatgen ? 25 : pstViInfo->stDevInfo.fps;
	stViDevAttr.stSize.u32Width 	= pstViInfo->stDevInfo.stSize.u32Width;
	stViDevAttr.stSize.u32Height 	= pstViInfo->stDevInfo.stSize.u32Height;
	stViDevAttr.enIntfMode 			= bPatgen ? VI_MODE_MIPI : (VI_INTF_MODE_E)pstViInfo->stDevInfo.enInterFaceMode;
	stViDevAttr.enInputDataType 	= bPatgen ? VI_DATA_TYPE_RGB : (VI_DATA_TYPE_E)pstViInfo->stDevInfo.enFormatMode;
	stViDevAttr.enDataSeq 			= bPatgen ? VI_DATA_SEQ_VUVU : (VI_YUV_DATA_SEQ_E)pstViInfo->stDevInfo.enYuvFormat;
	stViDevAttr.stWDRAttr.enWDRMode = bPatgen ? WDR_MODE_NONE : pstViInfo->stDevInfo.enWDRMode;
	stViDevAttr.enWorkMode 			= bPatgen ? VI_WORK_MODE_1Multiplex : (VI_WORK_MODE_E)pstViInfo->stDevInfo.enChnMode;
	stViDevAttr.enBayerFormat		= (BAYER_FORMAT_E)pstViInfo->stDevInfo.enBayerFormat;
	stViDevBindAttr.MipiDev 		= bPatgen ? 0 : pstViInfo->stDevInfo.mipiDev;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM) {
			stViDevBindAttr.PipeId[s32PipeCnt] = pstViInfo->stPipeInfo.aPipe[i];
			s32PipeCnt++;
			stViDevBindAttr.u32Num = s32PipeCnt;
		}
	}

	if (bPatgen) {
		s32Ret = CVI_VI_EnablePatgen(pstViInfo->stDevInfo.ViDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("set patgen failed\n");
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_SetDevBindAttr(ViDev, &stViDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_EnableDev(ViDev);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StopDev(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret;
	VI_DEV ViDev;

	ViDev = pstViInfo->stDevInfo.ViDev;
	s32Ret = CVI_VI_DisableDev(ViDev);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_DisableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	CVI_VI_UnRegChnFlipMirrorCallBack(0, ViDev);
	CVI_VI_UnRegPmCallBack(ViDev);
	memset(&ViPmData[ViDev], 0, sizeof(struct VI_PM_DATA_S));

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StartPipe(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S stPipeAttr;
	int j = 0;

	stPipeAttr.u32MaxW = pstViInfo->stDevInfo.stSize.u32Width;
	stPipeAttr.u32MaxH = pstViInfo->stDevInfo.stSize.u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.bYuvBypassPath = pstViInfo->stPipeInfo.bIspBypass;
	stPipeAttr.enCompressMode = pstViInfo->stPipeInfo.enCompressMode;

	for (j = 0; j < VI_MAX_PIPE_NUM; j++) {
		if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
			ViPipe = pstViInfo->stPipeInfo.aPipe[j];
			s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
				return s32Ret;
			}

			s32Ret = CVI_VI_StartPipe(ViPipe);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StopPipe(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32  s32Ret;
	CVI_S32  i;
	VI_PIPE ViPipe = 0;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		if (pstViInfo->stPipeInfo.aPipe[i] < 0 || pstViInfo->stPipeInfo.aPipe[i] >= VI_MAX_PIPE_NUM)
			continue;

		ViPipe = pstViInfo->stPipeInfo.aPipe[i];
		s32Ret = CVI_VI_StopPipe(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_StopPipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VI_DestroyPipe(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StartChn(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	VI_DEV ViDev = 0;
	VI_CHN_ATTR_S stChnAttr;

	ViPipe = pstViInfo->stPipeInfo.aPipe[0];
	ViChn = pstViInfo->stChnInfo.ViChn;
	ViDev = pstViInfo->stDevInfo.ViDev;

	stChnAttr.stSize.u32Width = pstViInfo->stDevInfo.stSize.u32Width;
	stChnAttr.stSize.u32Height = pstViInfo->stDevInfo.stSize.u32Height;
	stChnAttr.enDynamicRange = pstViInfo->stChnInfo.enDynamicRange;
	stChnAttr.enVideoFormat  = pstViInfo->stChnInfo.enVideoFormat;
	stChnAttr.enCompressMode = pstViInfo->stChnInfo.enCompressMode;
	stChnAttr.enPixelFormat = pstViInfo->stChnInfo.enPixFormat;
	stChnAttr.u32Depth = 1;
	stChnAttr.u32BindVbPool = -1;

	/* fill the sensor orientation */
	if (!pstViInfo->stDevInfo.bPatgen && pstViInfo->stSnsInfo.u8Orien <= 3) {
		stChnAttr.bMirror = pstViInfo->stSnsInfo.u8Orien & 0x1;
		stChnAttr.bFlip = (pstViInfo->stSnsInfo.u8Orien & 0x2) >> 1;
	} else {
		stChnAttr.bMirror = false;
		stChnAttr.bFlip = false;
	}

	s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	if(!pstViInfo->stDevInfo.bPatgen){
		s32Ret = CVI_SNS_SetVIFlipMirrorCB(ViPipe, ViDev);
		if (s32Ret!= CVI_SUCCESS) {
			SAMPLE_PRT("CVI_SNS_SetVIFlipMirrorCB failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_EnableChn failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StopChn(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn;

	ViPipe = pstViInfo->stPipeInfo.aPipe[0];
	ViChn = pstViInfo->stChnInfo.ViChn;

	s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_DisableChn failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_GetSizeBySensor(CVI_SNS_TYPE_E enMode, PIC_SIZE_E *penSize)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 dev_num = SAMPLE_COMM_GetDevnumBySnsmode(enMode);

	if (!penSize)
		return CVI_FAILURE;

	SNS_CFG_S sns_cfg = gstSensorCfg.sns_cfg;

	if (sns_cfg.u32ImageWigth[dev_num] == 352) {
		if (sns_cfg.u32ImageHeight[dev_num] == 288) {
			*penSize = PIC_CIF;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 720) {
		if (sns_cfg.u32ImageHeight[dev_num] == 576) {
			*penSize = PIC_D1_PAL;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 480) {
			*penSize = PIC_D1_NTSC;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 1280) {
		if (sns_cfg.u32ImageHeight[dev_num] == 720) {
			*penSize = PIC_720P;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 800) {
			*penSize = PIC_1280x800;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 960) {
			*penSize = PIC_1280x960;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 1920) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1080) {
			*penSize = PIC_1080P;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 1088) {
			*penSize = PIC_1088;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 1600) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1200) {
			*penSize = PIC_1600x1200;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 2560) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1440) {
			*penSize = PIC_1440P;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 1600) {
			*penSize = PIC_2560x1600;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 2160) {
			*penSize = PIC_2560x2160;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 1944) {
			*penSize = PIC_2560x1944;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 2048) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1536) {
			*penSize = PIC_2048x1536;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 2048) {
			*penSize = PIC_2048x2048;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 2304) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1296) {
			*penSize = PIC_2304x1296;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 2592) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1520) {
			*penSize = PIC_2592x1520;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 1944) {
			*penSize = PIC_2592x1944;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 1536) {
			*penSize = PIC_2592x1536;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 2688) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1520) {
			*penSize = PIC_2688x1520;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 1944) {
			*penSize = PIC_2688x1944;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 2716) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1524) {
			*penSize = PIC_2716x1524;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 2880) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1620) {
			*penSize = PIC_2880x1620;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 3200) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1800) {
			*penSize = PIC_3200x1800;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 3844) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1124) {
			*penSize = PIC_3844x1124;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 3840) {
		if (sns_cfg.u32ImageHeight[dev_num] == 2160) {
			*penSize = PIC_3840x2160;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 8640) {
			*penSize = PIC_3840x8640;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 3000) {
		if (sns_cfg.u32ImageHeight[dev_num] == 3000) {
			*penSize = PIC_3000x3000;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 4000) {
		if (sns_cfg.u32ImageHeight[dev_num] == 3000) {
			*penSize = PIC_4000x3000;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 4032) {
		if (sns_cfg.u32ImageHeight[dev_num] == 3000) {
			*penSize = PIC_4032x3000;
			return s32Ret;
		}
		if (sns_cfg.u32ImageHeight[dev_num] == 2288) {
			*penSize = PIC_4032x2288;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 4096) {
		if (sns_cfg.u32ImageHeight[dev_num] == 2160) {
			*penSize = PIC_4096x2160;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 4608) {
		if (sns_cfg.u32ImageHeight[dev_num] == 4320) {
			*penSize = PIC_4608x4320;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 7688) {
		if (sns_cfg.u32ImageHeight[dev_num] == 1124) {
			*penSize = PIC_7688x1124;
			return s32Ret;
		}

	}
	if (sns_cfg.u32ImageWigth[dev_num] == 7680) {
		if (sns_cfg.u32ImageHeight[dev_num] == 4320) {
			*penSize = PIC_7680x4320;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 8192) {
		if (sns_cfg.u32ImageHeight[dev_num] == 4320) {
			*penSize = PIC_8192x4320;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 5120) {
		if (sns_cfg.u32ImageHeight[dev_num] == 3840) {
			*penSize = PIC_5120x3840;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 640) {
		if (sns_cfg.u32ImageHeight[dev_num] == 480) {
			*penSize = PIC_640x480;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 632) {
		if (sns_cfg.u32ImageHeight[dev_num] == 479) {
			*penSize = PIC_479P;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 400) {
		if (sns_cfg.u32ImageHeight[dev_num] == 400) {
			*penSize = PIC_400x400;
			return s32Ret;
		}
	}
	if (sns_cfg.u32ImageWigth[dev_num] == 384) {
		if (sns_cfg.u32ImageHeight[dev_num] == 288) {
			*penSize = PIC_288P;
			return s32Ret;
		}
	}

	CVI_TRACE_LOG(CVI_DBG_ERR, "sensor_0x%x getsize failed with %#x!\n", enMode, s32Ret);
	CVI_TRACE_LOG(CVI_DBG_ERR, "use default size 1920x1080!\n");
	*penSize = PIC_1080P;

		return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_GetDevAttrBySns(CVI_SNS_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr)
{
	PIC_SIZE_E enPicSize;
	SIZE_S stSize;
	CVI_S32 dev_num = SAMPLE_COMM_GetDevnumBySnsmode(enSnsType);

	memcpy(pstViDevAttr, &DEV_ATTR_SENSOR_BASE, sizeof(VI_DEV_ATTR_S));

	SAMPLE_COMM_VI_GetSizeBySensor(enSnsType, &enPicSize);
	SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

	pstViDevAttr->stSize.u32Width = stSize.u32Width;
	pstViDevAttr->stSize.u32Height = stSize.u32Height;
	pstViDevAttr->stWDRAttr.u32CacheLine = stSize.u32Height;

	pstViDevAttr->stWDRAttr.enWDRMode = gstSensorCfg.sns_cfg.enWDRMode[dev_num];
	pstViDevAttr->enDataSeq = (VI_YUV_DATA_SEQ_E)gstSensorCfg.sns_cfg.enYuvFormat[dev_num];
	pstViDevAttr->enInputDataType = (VI_DATA_TYPE_E)gstSensorCfg.sns_cfg.enFormatMode[dev_num];
	pstViDevAttr->enIntfMode = (VI_INTF_MODE_E)gstSensorCfg.sns_cfg.enInterFaceMode[dev_num];
	pstViDevAttr->enBayerFormat = gstSensorCfg.sns_cfg.enBayerFormat[dev_num];
	pstViDevAttr->enWorkMode = (VI_WORK_MODE_E)gstSensorCfg.sns_cfg.enChnMode[dev_num];

	return CVI_SUCCESS;
}

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

CVI_S32 SAMPLE_COMM_VI_GetChnAttrBySns(CVI_SNS_TYPE_E enSnsType, VI_CHN_ATTR_S *pstChnAttr)
{
	VI_DEV_ATTR_S stViDevAttr;

	memcpy(pstChnAttr, &CHN_ATTR_420_SDR8, sizeof(VI_CHN_ATTR_S));

	SAMPLE_COMM_VI_GetDevAttrBySns(enSnsType, &stViDevAttr);
	if (stViDevAttr.enInputDataType == VI_DATA_TYPE_YUV)
		pstChnAttr->enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;

	pstChnAttr->stSize.u32Width = stViDevAttr.stSize.u32Width;
	pstChnAttr->stSize.u32Height = stViDevAttr.stSize.u32Height;

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_GetYuvBypassSts(CVI_SNS_TYPE_E enSnsType)
{
	CVI_S32 dev_num = SAMPLE_COMM_GetDevnumBySnsmode(enSnsType);
	CVI_S32 s32Ret = 0;

	if (gstSensorCfg.sns_cfg.bBypassIsp[dev_num] == 1) {
		s32Ret = 1;
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VI_DestroySingleVi(SAMPLE_VI_INFO_S *pstViInfo)
{
	SAMPLE_COMM_VI_StopChn(pstViInfo);

	SAMPLE_COMM_VI_StopPipe(pstViInfo);

	SAMPLE_COMM_VI_StopDev(pstViInfo);

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_DestroyVi(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32           i;
	CVI_S32           s32ViNum;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	if (!pstViConfig) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "null ptr\n");
		return CVI_FAILURE;
	}

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		s32ViNum  = pstViConfig->as32WorkingViId[i];
		pstViInfo = &pstViConfig->astViInfo[s32ViNum];

		SAMPLE_COMM_VI_DestroySingleVi(pstViInfo);
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StartSensor(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret, i;
	CVI_U32 u32SnsId;
	VI_PIPE ViPipe;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		ViPipe = pstViInfo->stPipeInfo.aPipe[0];
		u32SnsId = pstViInfo->stSnsInfo.s32SnsId;
		s32Ret = SAMPLE_COMM_ISP_SetSnsObj(u32SnsId, pstViInfo->stSnsInfo.enSnsType);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "update sensor obj(%d) failed!\n", u32SnsId);
			return s32Ret;
		}
		s32Ret = SAMPLE_COMM_ISP_SetSnsInit(u32SnsId, pstViInfo->stSnsInfo.u8HwSync);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "update sensor(%d) hwsync failed !\n", u32SnsId);
			return s32Ret;
		}
		s32Ret = SAMPLE_COMM_ISP_PatchSnsObj(u32SnsId, &pstViInfo->stSnsInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "patch rx attr(%d) failed!\n", u32SnsId);
			return s32Ret;
		}
		s32Ret = SAMPLE_COMM_ISP_Sensor_Regiter_callback(ViPipe, u32SnsId, pstViInfo->stSnsInfo.s32BusId,
								pstViInfo->stSnsInfo.s32SnsI2cAddr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d register callback failed!\n", i);
			return s32Ret;
		}
	}
	s32Ret = SAMPLE_COMM_ISP_SetSensorMode(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sensor %d register callback failed!\n", i);
		return s32Ret;
	}
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_SensorProbe(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS, i;
	CVI_U32 u32SnsId;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		pstViInfo = &pstViConfig->astViInfo[i];
		u32SnsId = pstViInfo->stSnsInfo.s32SnsId;
		s32Ret = CVI_SNS_SetSnsProbe(u32SnsId);
		if (s32Ret!= CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "cannot set the SnsId(%d) probe\n",
				u32SnsId);
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StartIsp(SAMPLE_VI_CONFIG_S *pstViConfig, VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ISP_PUB_ATTR_S stPubAttr;
	ISP_BIND_ATTR_S stBindAttr;

	memset(&stBindAttr, 0, sizeof(ISP_BIND_ATTR_S));
	memset(&stPubAttr, 0, sizeof(ISP_PUB_ATTR_S));

	SAMPLE_COMM_ISP_Aelib_Callback(ViPipe);
	SAMPLE_COMM_ISP_Awblib_Callback(ViPipe);
	#if ENABLE_AF_LIB
	SAMPLE_COMM_ISP_Aflib_Callback(ViPipe);
	#endif

	snprintf(stBindAttr.stAeLib.acLibName, sizeof(CVI_AE_LIB_NAME), "%s", CVI_AE_LIB_NAME);
	stBindAttr.stAeLib.s32Id = ViPipe;
	stBindAttr.sensorId = 0;
	snprintf(stBindAttr.stAwbLib.acLibName, sizeof(CVI_AWB_LIB_NAME), "%s", CVI_AWB_LIB_NAME);
	stBindAttr.stAwbLib.s32Id = ViPipe;
	#if ENABLE_AF_LIB
	snprintf(stBindAttr.stAfLib.acLibName, sizeof(CVI_AF_LIB_NAME), "%s", CVI_AF_LIB_NAME);
	stBindAttr.stAfLib.s32Id = ViPipe;
	#endif
	s32Ret = CVI_ISP_SetBindAttr(ViPipe, &stBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "Bind Algo failed with %#x!\n", s32Ret);
	}
	s32Ret = CVI_ISP_MemInit(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "Init Ext memory failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_COMM_ISP_GetIspPubAttr(ViPipe, pstViConfig, &stPubAttr);

	s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SetPubAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	s32Ret = CVI_ISP_Init(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ISP Init failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_StartViChn(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32             i;
	CVI_S32             s32Ret = CVI_SUCCESS;
	VI_PIPE             ViPipe = 0;
	VI_CHN              ViChn = 0;
	VI_DEV              ViDev = 0;
	CVI_U32             u32SnsId = 0;
	VI_CHN_ATTR_S       stChnAttr;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		if (i < VI_MAX_DEV_NUM) {
			ViPipe	    = pstViConfig->astViInfo[i].stPipeInfo.aPipe[0];
			ViChn	    = pstViConfig->astViInfo[i].stChnInfo.ViChn;
			ViDev	    = pstViConfig->astViInfo[i].stDevInfo.ViDev;
			u32SnsId    = pstViConfig->astViInfo[i].stSnsInfo.s32SnsId;

			SAMPLE_COMM_VI_GetChnAttrBySns(pstViConfig->astViInfo[i].stSnsInfo.enSnsType, &stChnAttr);
			stChnAttr.enDynamicRange = pstViConfig->astViInfo[i].stChnInfo.enDynamicRange;
			stChnAttr.enVideoFormat  = pstViConfig->astViInfo[i].stChnInfo.enVideoFormat;
			stChnAttr.enCompressMode = pstViConfig->astViInfo[i].stChnInfo.enCompressMode;
			stChnAttr.enPixelFormat = pstViConfig->astViInfo[i].stChnInfo.enPixFormat;
			/* fill the sensor orientation */
			if (pstViConfig->astViInfo[i].stSnsInfo.u8Orien <= 3) {
				stChnAttr.bMirror = pstViConfig->astViInfo[i].stSnsInfo.u8Orien & 0x1;
				stChnAttr.bFlip = pstViConfig->astViInfo[i].stSnsInfo.u8Orien & 0x2;
			}

			s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_SNS_SetVIFlipMirrorCB(u32SnsId, ViDev);
			if (s32Ret!= CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SNS_SetVIFlipMirrorCB failed with %#x!\n", s32Ret);
			}

			// TODO
			/* register the power management ops. */
			//ViPmData[ViDev].ViPipe = ViPipe;
			//ViPmData[ViDev].u32SnsId = u32SnsId;
			//ViPmData[ViDev].s32DevNo = pstViConfig->astViInfo[i].stSnsInfo.MipiDev;
			//s32Ret = CVI_VI_RegPmCallBack(ViDev, &vi_ops, (CVI_VOID *)&ViPmData[ViDev]);
			//if (s32Ret != CVI_SUCCESS) {
			//	CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_RegPmCallBack failed with %#x!\n", s32Ret);
			//	return CVI_FAILURE;
			//}

			s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_EnableChn failed with %#x!\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_StopViChn(SAMPLE_VI_INFO_S *pstViInfo)
{
	CVI_S32             s32Ret = CVI_SUCCESS;
	VI_PIPE             ViPipe = 0;
	VI_CHN              ViChn;
	VI_VPSS_MODE_E      enMastPipeMode;

	ViChn  = pstViInfo->stChnInfo.ViChn;

	if (ViChn < VI_MAX_CHN_NUM) {
		enMastPipeMode = pstViInfo->stPipeInfo.enMastPipeMode;

		if (enMastPipeMode == VI_OFFLINE_VPSS_OFFLINE
		    || enMastPipeMode == VI_ONLINE_VPSS_OFFLINE) {
			s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DisableChn failed with %#x!\n",
								s32Ret);
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VI_CreateIsp(SAMPLE_VI_CONFIG_S *pstViConfig)
{
//#define USE_LOAD_ALL_PIPE_PQBIN_API  1

	CVI_S32 s32Ret = CVI_SUCCESS;

	if (!pstViConfig) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	for (int i = 0; i < pstViConfig->s32ViNum; i++) {
		if(pstViConfig->astViInfo[i].stDevInfo.bPatgen){
			continue;
		}
		s32Ret = SAMPLE_COMM_VI_StartIsp(pstViConfig, i);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VI_StartIsp failed !\n");
			return CVI_FAILURE;
		}
#ifndef USE_LOAD_ALL_PIPE_PQBIN_API
		s32Ret = SAMPLE_COMM_BIN_ReadBlockParaFrombin(CVI_BIN_ID_ISP0 + i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "read pqbin fail: %#x, use default param!\n", s32Ret);
		}
#endif
	}

#ifdef USE_LOAD_ALL_PIPE_PQBIN_API
	s32Ret = SAMPLE_COMM_BIN_ReadParaFrombin();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "read pabin fail: %#x,use default param!\n", s32Ret);
	}
#endif

	for (int i = 0; i < pstViConfig->s32ViNum; i++) {
		if(pstViConfig->astViInfo[i].stDevInfo.bPatgen){
			continue;
		}
		s32Ret = SAMPLE_COMM_ISP_Run(i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "ISP_Run failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VI_DestroyIsp(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (!pstViConfig) {
		SAMPLE_PRT("%s: null ptr\n", __func__);
		return CVI_FAILURE;
	}

	for (int i = 0; i < pstViConfig->s32ViNum; i++) {
		if(pstViConfig->astViInfo[i].stDevInfo.bPatgen){
			continue;
		}
		SAMPLE_COMM_ISP_Stop(i);
	}

	return s32Ret;
}
