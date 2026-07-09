#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <fcntl.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "cvi_venc.h"
#include "cvi_vi.h"
#include "cvi_region.h"
#include "cvi_sensor.h"
#include "cvi_mipi.h"

#define SAMPLE_VPSS_DEFAULT_FILE_IN      "res/1080p.yuv420"
#define dog_argb1555_bin          "res/dog_s_80x60_pngto1555.bin"

#define SAMPLE_STREAM_OFFLINE    "offline_sbm.h264"
#define SAMPLE_STREAM_ONLINE    "online_sbm.h264"
#define SAMPLE_STREAM_ONLINE_MAIN    "online_sbm_main.h265"
#define SAMPLE_STREAM_ONLINE_SMALL    "online_frm_small.h265"

static CVI_BOOL gSampleVencExit = CVI_FALSE;
#ifndef FPGA_PORTING
#define WAIT_STREAM_TIMEOUT 2000
#else
#define WAIT_STREAM_TIMEOUT 20000
#endif

#define SAMPLE_PRT(fmt...) \
	do { \
		printf("[%s]-%d: ", __func__, __LINE__); \
		printf(fmt); \
	} while (0)

typedef struct _VENC_GET_STEAM_PARA_S {
	CVI_BOOL ThreadStart;
	VENC_CHN VencChn[VENC_MAX_CHN_NUM];
	FILE * Fp[VENC_MAX_CHN_NUM];
	CVI_S32 Cnt;
	CVI_BOOL SaveFile;
} VENC_GET_STEAM_PARA_S;

typedef struct _VENC_GET_STEAM_PROC_INFO_S {
	FILE * FileFp[VENC_MAX_CHN_NUM];
	CVI_S32 VencFd[VENC_MAX_CHN_NUM];
	CVI_S32 MaxFd;
	VENC_CHN VencChn;
	CVI_S32 ChnTotal;
	CVI_BOOL SaveFile;
} VENC_GET_STEAM_PROC_INFO_S;

static VENC_GET_STEAM_PARA_S gPara = {
	.ThreadStart = CVI_FALSE,
	.Cnt = 0,
	.SaveFile = CVI_FALSE
};

pthread_t gVencPid;

CVI_S32 SAMPLE_VENC_SaveOneChannelStream(VENC_CHN VencChn, FILE *fp)
{
	VENC_CHN_STATUS_S stStat;
	VENC_STREAM_S stStream;
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
				VencChn, s32Ret);
		return s32Ret;
	}

	if (!stStat.u32CurPacks) {
		SAMPLE_PRT("u32CurPacks = NULL!\n");
		return CVI_FAILURE;
	}

	stStream.pstPack =
		(VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);

	if (stStream.pstPack == NULL) {
		SAMPLE_PRT("malloc memory failed!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VENC_GetStream(VencChn, &stStream, WAIT_STREAM_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		free(stStream.pstPack);
		stStream.pstPack = CVI_NULL;
		return CVI_FAILURE;
	}
	//SAMPLE_PRT("get chn:%d count:%d\n", VencChn, stStream.u32PackCount);
	for (CVI_U32 i = 0; i < stStream.u32PackCount; i++) {
		fwrite(stStream.pstPack[i].pu8Addr, 1, stStream.pstPack[i].u32Len, fp);
	}
	s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
	if (s32Ret != CVI_SUCCESS) {
		free(stStream.pstPack);
		stStream.pstPack = CVI_NULL;
		return CVI_FAILURE;
	}

	free(stStream.pstPack);
	stStream.pstPack = NULL;
	return s32Ret;
}

static CVI_S32 SAMPLE_VENC_SetNameSaveStream(VENC_GET_STEAM_PROC_INFO_S *pStreamProcInfo,
	VENC_GET_STEAM_PARA_S *pPara)
{
	CVI_S32 i;

	for (i = 0; (i < pStreamProcInfo->ChnTotal) && (i < VENC_MAX_CHN_NUM); i++) {
		pStreamProcInfo->FileFp[i] = pPara->Fp[i];
		/* set venc fd*/
		pStreamProcInfo->VencFd[i] = CVI_VENC_GetFd(pPara->VencChn[i]);
		if (pStreamProcInfo->VencFd[i] < 0) {
			SAMPLE_PRT("CVI_VENC_GetFd failed with %#x!\n",
				pStreamProcInfo->VencFd[i]);
			return CVI_FAILURE;
		}

		if (pStreamProcInfo->MaxFd <= pStreamProcInfo->VencFd[i]) {
			pStreamProcInfo->MaxFd = pStreamProcInfo->VencFd[i];
		}
	}

	return CVI_SUCCESS;
}

#if !defined(CONFIG_DUAL_OS)
static CVI_VOID SAMPLE_FD_IsSet(VENC_GET_STEAM_PROC_INFO_S *pStreamProcInfo,
	fd_set *read_fds, VENC_GET_STEAM_PARA_S *para)
{
	CVI_S32 i, s32Ret;

	for (i = 0; (i < pStreamProcInfo->ChnTotal) && (i < VENC_MAX_CHN_NUM); i++) {
		if (FD_ISSET(pStreamProcInfo->VencFd[i], read_fds)) {
			pStreamProcInfo->VencChn = para->VencChn[i];
			s32Ret = SAMPLE_VENC_SaveOneChannelStream(pStreamProcInfo->VencChn,
				pStreamProcInfo->FileFp[i]);
			if(s32Ret)
				SAMPLE_PRT("venc save stream fail, chn:%d\n",i);
		}
	}
}
#endif

#define PR_SET_NAME 15
static CVI_VOID *SAMPLE_VENC_GetVencStreamProc(CVI_VOID *Para)
{
	VENC_GET_STEAM_PARA_S *pPara = (VENC_GET_STEAM_PARA_S *)Para;
	VENC_GET_STEAM_PROC_INFO_S StreamProcInfo = {0};
	CVI_S32 i, s32Ret;

	prctl(PR_SET_NAME, "VencGetStream", 0, 0, 0);
	StreamProcInfo.ChnTotal = pPara->Cnt;
	StreamProcInfo.SaveFile = pPara->SaveFile;

	if (StreamProcInfo.ChnTotal >= VENC_MAX_CHN_NUM) {
		SAMPLE_PRT("input count invalid\n");
		return CVI_NULL;
	}
	s32Ret = SAMPLE_VENC_SetNameSaveStream(&StreamProcInfo, pPara);
	if (s32Ret != CVI_SUCCESS)
		return NULL;

	while (pPara->ThreadStart == CVI_TRUE) {
#if defined(CONFIG_DUAL_OS)
		for (i = 0; (i < pPara->Cnt) && (i < VENC_MAX_CHN_NUM); i++) {
			s32Ret = SAMPLE_VENC_SaveOneChannelStream(pPara->VencChn[i],
				pPara->Fp[i]);
			if(s32Ret){
				SAMPLE_PRT("save venc stream fail, chn:%d\n", i);
			}
		}
#else
		fd_set ReadFds;
		struct timeval timeout_val;

		FD_ZERO(&ReadFds);
		for (i = 0; (i < StreamProcInfo.ChnTotal) && (i < VENC_MAX_CHN_NUM); i++)
			FD_SET(StreamProcInfo.VencFd[i], &ReadFds);

		timeout_val.tv_sec = 10; /* 2 is a number */
		timeout_val.tv_usec = 0;
		s32Ret = select(StreamProcInfo.MaxFd + 1, &ReadFds, CVI_NULL, CVI_NULL, &timeout_val);
		if (s32Ret < 0) {
			SAMPLE_PRT("select failed exit!\n");
			break;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("select stream timeout, continue\n");
			continue;
		} else {
			SAMPLE_FD_IsSet(&StreamProcInfo, &ReadFds, pPara);
		}
#endif
	}
	return (CVI_VOID *) CVI_SUCCESS;
}

static CVI_S32 SAMPLE_VENC_StartGetStream(CVI_S32 *VencChn, CVI_S32 Cnt)
{
	struct sched_param param;
	pthread_attr_t attr;
	CVI_S32 i = 0;

	gPara.ThreadStart = CVI_TRUE;
	gPara.Cnt = Cnt;
	for (i = 0; (i < Cnt) && (i < VENC_MAX_CHN_NUM); i++) {
		gPara.VencChn[i] = VencChn[i];
	}

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(
			&gVencPid,
			&attr,
			SAMPLE_VENC_GetVencStreamProc,
			(CVI_VOID *)&gPara);

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_VENC_StopGetStream(CVI_VOID)
{
	if (gPara.ThreadStart == CVI_TRUE) {
		gPara.ThreadStart = CVI_FALSE;
		pthread_join(gVencPid, 0);
	}
	return CVI_SUCCESS;
}

static void SAMPLE_VENC_ExitProcess(CVI_VOID)
{
	SAMPLE_PRT("\npress 'ctrl + c' to exit this sample.\n");
	while (!gSampleVencExit) {
		sleep(1);
	}

	SAMPLE_VENC_StopGetStream();
}

CVI_S32 SAMPLE_FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;
	FILE *fp;

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = enPixelFormat;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	stVideoFrame.stVFrame.u32Width = stSize->u32Width;
	stVideoFrame.stVFrame.u32Height = stSize->u32Height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_PRT("CVI_VB_GetBlock fail\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	stVideoFrame.stVFrame.u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		stVideoFrame.stVFrame.u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		stVideoFrame.stVFrame.pu8VirAddr[i]
			= CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[i], stVideoFrame.stVFrame.u32Length[i]);

		u32len = fread(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("vpss send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	SAMPLE_PRT("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);
	SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2]);
	SAMPLE_PRT("vir addr(%p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2]);

	fclose(fp);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	memcpy(pstVideoFrame, &stVideoFrame, sizeof(stVideoFrame));

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_VPSS_FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file(%s) error\n", filename);
		return CVI_FAILURE;
	}

	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
					   pstVideoFrame->stVFrame.pu8VirAddr[i],
					   pstVideoFrame->stVFrame.u32Length[i]);

		SAMPLE_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		SAMPLE_PRT(" data_len(%d) plane_len(%d)\n",
				  u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return s32Ret;
}


CVI_S32 SAMPLE_VENC_ChnSetup(VENC_CHN VeChn, SIZE_S *stSize, PAYLOAD_TYPE_E enType)
{
	CVI_S32 ret = 0;
	VENC_CHN_ATTR_S stAttr = {0};
	VENC_RC_PARAM_S stRcParam = {0};
	VENC_RECV_PIC_PARAM_S stRecvParam;

	stAttr.stVencAttr.u32MaxPicWidth = 4096;
	stAttr.stVencAttr.u32MaxPicHeight = 2304;
	stAttr.stVencAttr.u32BufSize = 2 * 1024 * 1024;
	stAttr.stVencAttr.u32PicWidth = stSize->u32Width;
	stAttr.stVencAttr.u32PicHeight = stSize->u32Height;
	stAttr.stVencAttr.enType = enType;
	stAttr.stVencAttr.bIsoSendFrmEn = 1;
	stAttr.stVencAttr.bEsBufQueueEn = 1;

	if (stAttr.stVencAttr.enType == PT_H264) {
		stAttr.stVencAttr.stAttrH264e.bSingleLumaBuf = CVI_FALSE;
		stAttr.stVencAttr.stAttrH264e.bRcnRefShareBuf = CVI_TRUE;
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
		stAttr.stRcAttr.stH264Cbr.u32Gop = 25;
		stAttr.stRcAttr.stH264Cbr.u32StatTime = 2;
		stAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = 25;
		stAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = 25;
		stAttr.stRcAttr.stH264Cbr.u32BitRate = 2000;
		stAttr.stRcAttr.stH264Cbr.bVariFpsEn = CVI_FALSE;
	} else if (stAttr.stVencAttr.enType == PT_H265) {
		stAttr.stVencAttr.stAttrH265e.bRcnRefShareBuf = CVI_TRUE;
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
		stAttr.stRcAttr.stH265Cbr.u32Gop = 25;
		stAttr.stRcAttr.stH265Cbr.u32StatTime = 2;
		stAttr.stRcAttr.stH265Cbr.u32SrcFrameRate = 25;
		stAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = 25;
		stAttr.stRcAttr.stH265Cbr.u32BitRate = 2000;
		stAttr.stRcAttr.stH265Cbr.bVariFpsEn = CVI_FALSE;

	} else if (stAttr.stVencAttr.enType == PT_JPEG) {
		stAttr.stVencAttr.stAttrJpege.bSupportDCF = CVI_FALSE;
		stAttr.stVencAttr.stAttrJpege.stMPFCfg.u8LargeThumbNailNum = 0;
		stAttr.stVencAttr.stAttrJpege.enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
	}

	if (stAttr.stVencAttr.enType == PT_H264 || stAttr.stVencAttr.enType == PT_H265) {
		stAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
		stAttr.stGopAttr.stNormalP.s32IPQpDelta = 2;
	}

	ret = CVI_VENC_CreateChn(VeChn, &stAttr);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_CreateChn FAIL: 0x%x\n", ret);
		return -1;
	}

	ret =  CVI_VENC_GetRcParam(VeChn, &stRcParam);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetRcParam FAIL: 0x%x\n", ret);
		return -1;
	}
	stRcParam.s32FirstFrameStartQp = 32;
	stRcParam.s32InitialDelay = CVI_INITIAL_DELAY_DEFAULT;
	stRcParam.u32ThrdLv = 2;

	if (stAttr.stVencAttr.enType == PT_H264) {
		stRcParam.stParamH264Cbr.bQpMapEn = CVI_FALSE;
		stRcParam.stParamH264Cbr.u32MaxIprop = CVI_H26X_MAX_I_PROP_MAX;
		stRcParam.stParamH264Cbr.u32MinIprop = CVI_H26X_MAX_I_PROP_MIN;
		stRcParam.stParamH264Cbr.u32MaxIQp = CVI_H26X_MAXIQP_MAX;
		stRcParam.stParamH264Cbr.u32MinIQp = CVI_H26X_MINIQP_MIN;
		stRcParam.stParamH264Cbr.u32MaxQp = CVI_H26X_MAXQP_MAX;
		stRcParam.stParamH264Cbr.u32MinQp = CVI_H26X_MINQP_MIN;

	}

	if (stAttr.stVencAttr.enType == PT_H265) {
		stRcParam.stParamH265Cbr.bQpMapEn = CVI_FALSE;
		stRcParam.stParamH265Cbr.u32MaxIprop = CVI_H26X_MAX_I_PROP_MAX;
		stRcParam.stParamH265Cbr.u32MinIprop = CVI_H26X_MAX_I_PROP_MIN;
		stRcParam.stParamH265Cbr.u32MaxIQp = CVI_H26X_MAXIQP_MAX;
		stRcParam.stParamH265Cbr.u32MinIQp = CVI_H26X_MINIQP_MIN;
		stRcParam.stParamH265Cbr.u32MaxQp = CVI_H26X_MAXQP_MAX;
		stRcParam.stParamH265Cbr.u32MinQp = CVI_H26X_MINQP_MIN;
	}

	ret = CVI_VENC_SetRcParam(VeChn, &stRcParam);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetRcParam FAIL: 0x%x\n", ret);
		return -1;
	}

	ret = CVI_VENC_StartRecvFrame(VeChn, &stRecvParam);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_StartRecvFrame FAIL: 0x%x\n", ret);
		return -1;
	}
	return ret;
}

static CVI_S32 SAMPLE_Snsr_Parser(SENSOR_CFG_S *cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	// Get config from ini if found.
	s32Ret = CVI_SNS_ParseIni(cfg);
	if (s32Ret == CVI_FAILURE) {
		SAMPLE_PRT("[ERROR] parse ini failed, default use patgen\n");
	} else {
		s32Ret = CVI_SNS_GetConfigInfo(cfg);
		if (s32Ret == CVI_FAILURE) {
			SAMPLE_PRT("[ERROR] get sns cfg failed\n");
		}

		s32Ret = CVI_SNS_SetSnsDrvCfg(cfg);
		if (s32Ret == CVI_FAILURE) {
			SAMPLE_PRT("[ERROR] set sns_drv failed\n");
		}

		if (cfg->sns_cfg.enInterFaceMode[0] != SNS_MODE_MIPI ||
			cfg->sns_cfg.enFormatMode[0] != SNS_DATA_TYPE_RGB) {
			SAMPLE_PRT("[ERROR] only support mipi rgb\n");
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_Snsr_Setup(SENSOR_CFG_S *cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32SnsId = 0;
	VI_DEV ViDev = 0;
	SNS_COMBO_DEV_ATTR_S stDevAttr;
	CVI_S32 mipiDev = cfg->sns_ini_cfg.MipiDev[s32SnsId];

	/************************************************
	 * Set sns reset, probe; Set MIPI attr
	 ************************************************/
	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		mipiDev = cfg->sns_ini_cfg.MipiDev[s32SnsId];
		s32Ret = CVI_MIPI_SetSensorReset(mipiDev, cfg->sns_ini_cfg.s32RstPort[s32SnsId],
						cfg->sns_ini_cfg.s32RstPin[s32SnsId],
						cfg->sns_ini_cfg.s32RstPol[s32SnsId], 1);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] sensor_%d reset failed!\n", ViDev);
			return s32Ret;
		}
	}

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		mipiDev = cfg->sns_ini_cfg.MipiDev[s32SnsId];
		s32Ret = CVI_MIPI_SetMipiReset(mipiDev, 1);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] mipi dev_%d reset failed!\n", ViDev);
			return s32Ret;
		}
	}

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		mipiDev = cfg->sns_ini_cfg.MipiDev[s32SnsId];
		if (CVI_SNS_GetSnsRxAttr(ViDev, &stDevAttr) != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] get mipi dev_%d attr failed!\n", ViDev);
			return CVI_FAILURE;
		}

		if (stDevAttr.input_mode == INPUT_MODE_MIPI) {
			stDevAttr.mipi_attr.dphy.enable = cfg->sns_ini_cfg.bHsettlen[s32SnsId];
			stDevAttr.mipi_attr.dphy.hs_settle = cfg->sns_ini_cfg.u8Hsettle[s32SnsId];
		}

		if (stDevAttr.input_mode == INPUT_MODE_MIPI ||
			stDevAttr.input_mode == INPUT_MODE_SUBLVDS ||
			stDevAttr.input_mode == INPUT_MODE_HISPI) {
			stDevAttr.cif_mode = cfg->sns_ini_cfg.enSnsMode;
		}

		s32Ret = CVI_MIPI_SetMipiAttr(ViDev, (CVI_VOID *)&stDevAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] set mipi dev_%d attr failed!\n", ViDev);
			return s32Ret;
		}
	}

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		mipiDev = cfg->sns_ini_cfg.MipiDev[s32SnsId];
		s32Ret = CVI_MIPI_SetSensorClock(mipiDev, 1);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] sensor %d clock enable failed!\n", ViDev);
			return s32Ret;
		}
	}

	//Wait for the clock to stabilize before setting XCLR, eg. 500ns(IMX327)
	usleep(200 * 1000);
	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		mipiDev = cfg->sns_ini_cfg.MipiDev[s32SnsId];
		s32Ret = CVI_MIPI_SetSensorReset(mipiDev, cfg->sns_ini_cfg.s32RstPort[s32SnsId],
							cfg->sns_ini_cfg.s32RstPin[s32SnsId],
							cfg->sns_ini_cfg.s32RstPol[s32SnsId], 0);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] sensor_%d unreset failed!\n", ViDev);
			return s32Ret;
		}
	}

	//Communication start after reset, eg. 20us(IMX327)
	usleep(200 * 1000);
	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		if (CVI_SNS_SetSnsProbe(ViDev) != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] sensor_%d probe failed!\n", ViDev);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_VI_Setup(CVI_BOOL isPatgen, SENSOR_CFG_S *cfg, SIZE_S *pstSize)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32SnsId = 0;
	VI_DEV ViDev = 0;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	VI_DEV_ATTR_S stViDevAttr;
	VI_PIPE_ATTR_S stPipeAttr;
	VI_CHN_ATTR_S stChnAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		ViPipe = s32SnsId;
		if (isPatgen) {
			stViDevAttr.snrFps = 25;
			stViDevAttr.stSize.u32Width = pstSize->u32Width;
			stViDevAttr.stSize.u32Height = pstSize->u32Height;
			stViDevAttr.enIntfMode = VI_MODE_MIPI;
			stViDevAttr.enInputDataType = VI_DATA_TYPE_RGB;
			stViDevAttr.enDataSeq = VI_DATA_SEQ_VUVU;
			stViDevAttr.stWDRAttr.enWDRMode = WDR_MODE_NONE;
			stViDevAttr.enWorkMode = VI_WORK_MODE_1Multiplex;
			stViDevBindAttr.MipiDev = 0;
		} else {
			stViDevAttr.stSize.u32Width = cfg->sns_cfg.u32ImageWigth[s32SnsId];
			stViDevAttr.stSize.u32Height = cfg->sns_cfg.u32ImageHeight[s32SnsId];
			stViDevAttr.enIntfMode = (VI_INTF_MODE_E)cfg->sns_cfg.enInterFaceMode[s32SnsId];
			stViDevAttr.enInputDataType = (VI_DATA_TYPE_E)cfg->sns_cfg.enFormatMode[s32SnsId];
			stViDevAttr.enDataSeq = (VI_YUV_DATA_SEQ_E)cfg->sns_cfg.enYuvFormat[s32SnsId];
			stViDevAttr.stWDRAttr.enWDRMode = cfg->sns_cfg.enWDRMode[s32SnsId];
			stViDevAttr.enWorkMode = (VI_WORK_MODE_E)cfg->sns_cfg.enChnMode[s32SnsId];
			stViDevBindAttr.MipiDev = cfg->sns_ini_cfg.MipiDev[s32SnsId];
		}

		//ini vi
		stViDevBindAttr.PipeId[0] = ViPipe;
		stViDevBindAttr.u32Num = 1;
		s32Ret = CVI_VI_SetDevBindAttr(ViDev, &stViDevBindAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		if (isPatgen) {
			s32Ret = CVI_VI_EnablePatgen(ViDev);
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

		s32Ret = CVI_VI_EnableDev(ViDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_EnableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViPipe = s32SnsId;

		stPipeAttr.u32MaxW = isPatgen ? pstSize->u32Width : cfg->sns_cfg.u32ImageWigth[s32SnsId];
		stPipeAttr.u32MaxH = isPatgen ? pstSize->u32Height : cfg->sns_cfg.u32ImageHeight[s32SnsId];
		stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
		stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
		stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
		stPipeAttr.stFrameRate.s32DstFrameRate = -1;
		stPipeAttr.bNrEn = CVI_TRUE;
		stPipeAttr.bYuvBypassPath = CVI_FALSE;

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

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		if (!isPatgen) {
			s32Ret = CVI_SNS_SetSnsInit(ViDev);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("[ERROR] sensor_%d init failed!\n", ViDev);
				return s32Ret;
			}
		}
	}

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViPipe = s32SnsId;
		ViDev = s32SnsId;
		stChnAttr.stSize.u32Width = isPatgen ? pstSize->u32Width : cfg->sns_cfg.u32ImageWigth[s32SnsId];
		stChnAttr.stSize.u32Height = isPatgen ? pstSize->u32Height : cfg->sns_cfg.u32ImageHeight[s32SnsId];
		stChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
		stChnAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
		stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
		stChnAttr.enPixelFormat = PIXEL_FORMAT_NV12;
		stChnAttr.u32Depth = 1;
		stChnAttr.u32BindVbPool = -1;
		/* fill the sensor orientation */
		stChnAttr.bMirror = false;
		stChnAttr.bFlip = false;
		stChnAttr.stFrameRate.s32SrcFrameRate = -1;
		stChnAttr.stFrameRate.s32DstFrameRate = -1;

		s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_EnableChn failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 SAMPLE_VI_Destory(SENSOR_CFG_S *cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	VI_DEV ViDev = 0;
	CVI_S32 s32SnsId = 0;

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViPipe = s32SnsId;
		s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_DisableChn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViPipe = s32SnsId;

		s32Ret = CVI_VI_StopPipe(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_DisablePipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VI_DestroyPipe(ViPipe);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (s32SnsId = 0; s32SnsId < cfg->sns_ini_cfg.devNum; s32SnsId++) {
		ViDev = s32SnsId;
		s32Ret = CVI_VI_DisableDev(ViDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_DisableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_RGN_Setup(MMF_CHN_S stChn, SIZE_S stSize, CVI_S32 s32Handle, CVI_BOOL bCompressed)
{
	CVI_S32 s32Ret = 0;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = stSize.u32Width;
	stRegion.unAttr.stOverlay.stSize.u32Height = stSize.u32Height;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize = RGN_CMPR_MIN_SIZE;

	if (bCompressed) {
		stRegion.unAttr.stOverlay.u32CanvasNum = 2;
		stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 200;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 50;
	} else {
		stRegion.unAttr.stOverlay.u32CanvasNum = 1;
		stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 40;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 50;
	}

	s32Ret = CVI_RGN_Create(s32Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = OVERLAY_RGN;
	s32Ret = CVI_RGN_AttachToChn(s32Handle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	if (bCompressed) {
		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
		RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
		RGN_CANVAS_INFO_S stCanvasInfo;

		s32Ret = CVI_RGN_GetCanvasInfo(s32Handle, &stCanvasInfo);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
		pstObjAttr = stCanvasInfo.pstObjAttr;

		pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
		pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
		pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
		pstCanvasCmprAttr->u32ObjNum = 1;

		pstObjAttr[0].stRgnRect.stRect.s32X = 0;
		pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
		pstObjAttr[0].stRgnRect.stRect.u32Width = stSize.u32Width;
		pstObjAttr[0].stRgnRect.stRect.u32Height = stSize.u32Height;
		pstObjAttr[0].stRgnRect.u32Thick = 10;
		pstObjAttr[0].stRgnRect.u32Color = 0x801f;
		pstObjAttr[0].stRgnRect.u32IsFill = false;
		pstObjAttr[0].enObjType = RGN_CMPR_RECT;

		s32Ret = CVI_RGN_UpdateCanvas(s32Handle);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	} else {
		fp = fopen(dog_argb1555_bin, "rb");
		if (fp == NULL) {
			SAMPLE_PRT("fopen failed!\n");
			return s32Ret;
		}
		fseek(fp, 0L, SEEK_END);
		u32FileSize = ftell(fp);
		rewind(fp);
		stBitmap.pData = malloc(u32FileSize);
		if (stBitmap.pData == NULL) {
			SAMPLE_PRT("malloc size(%d) failed!\n", u32FileSize);
			fclose(fp);
			return s32Ret;
		}
		fread(stBitmap.pData, u32FileSize, 1, fp);
		fclose(fp);

		stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

		s32Ret = CVI_RGN_SetBitMap(s32Handle, &stBitmap);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
			free(stBitmap.pData);
			return s32Ret;
		}
		free(stBitmap.pData);
	}

	return s32Ret;
}

CVI_S32 SAMPLE_SBM_Offline(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn;
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enPixelFormatOut = PIXEL_FORMAT_NV12;
	SIZE_S stSize = {1920, 1080};
	CVI_CHAR *pFileNameIn = SAMPLE_VPSS_DEFAULT_FILE_IN;
	CVI_CHAR *pFileNameOut = SAMPLE_STREAM_OFFLINE;
	FILE *fpOutput = NULL;
	VI_VPSS_MODE_S stVIVPSSMode;

	fpOutput = fopen(pFileNameOut, "wb");
	if (fpOutput == NULL) {
		SAMPLE_PRT("can't open file %s\n", pFileNameOut);
		return CVI_FAILURE;
	}

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		fclose(fpOutput);
		return CVI_FAILURE;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	//vb in
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		goto exit1;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP		   VpssGrp		  = 0;
	VPSS_CHN		   VpssChn		  = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};
	VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;
	VPSS_MODE_S stVPSSMode;

	stVPSSMode.enMode = VPSS_MODE_SINGLE;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.u32MaxW						= stSize.u32Width;
	stVpssGrpAttr.u32MaxH						= stSize.u32Height;
	stVpssGrpAttr.enPixelFormat					= enPixelFormatIn;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	= -1;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	stVpssChnAttr.u32Width						= stSize.u32Width;
	stVpssChnAttr.u32Height						= stSize.u32Height;
	stVpssChnAttr.enVideoFormat					= VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat					= enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssChnAttr.u32Depth						= 0;
	stVpssChnAttr.bMirror						= CVI_FALSE;
	stVpssChnAttr.bFlip							= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode			= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable			= CVI_FALSE;

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	stVpssChnBufWrap.bEnable = CVI_TRUE;
	stVpssChnBufWrap.u32BufLine = 64;
	stVpssChnBufWrap.u32WrapBufferSize = 5;

	s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, &stVpssChnBufWrap);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnBufWrapAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	/************************************************
	 * step3:  Init VENC
	 ************************************************/
	VENC_CHN VencChn = 0;
	PAYLOAD_TYPE_E enType = PT_H264;
	MMF_CHN_S stSrcChn = {CVI_ID_VPSS, VpssGrp, VpssChn};
	MMF_CHN_S stDestChn = {CVI_ID_VENC, 0, VencChn};
	CVI_S32 VencFd;

	s32Ret = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Bind failed with %#x\n", s32Ret);
		goto exit4;
	}

	s32Ret = SAMPLE_VENC_ChnSetup(VencChn, &stSize, enType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	/************************************************
	 * step4:  Get Stream
	 ************************************************/
	s32Ret = SAMPLE_FileToFrame(&stSize, enPixelFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VPSS_FileToFrame failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit6;
	}

	VencFd = CVI_VENC_GetFd(VencChn);
	if (VencFd < 0)
		goto exit6;

	i = 60;
	while (i--) {
		int cnt = 0;

		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_SendFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit7;
		}
again:
		if (gSampleVencExit || cnt > 10) {
			break;
		}
#if defined(CONFIG_DUAL_OS)
		s32Ret = SAMPLE_VENC_SaveOneChannelStream(VencChn, fpOutput);
		if (s32Ret) {
			SAMPLE_PRT("get venc stream time out, try get again\n");
			cnt++;
			goto again;
		}
#else
		struct timeval timeout_val;
		fd_set ReadFds;

		FD_ZERO(&ReadFds);
		FD_SET(VencFd, &ReadFds);
		timeout_val.tv_sec = 2; /* 2 is a number */
		timeout_val.tv_usec = 0;
		s32Ret = select(VencFd + 1, &ReadFds, CVI_NULL, CVI_NULL, &timeout_val);
		if (s32Ret < 0) {
			SAMPLE_PRT("select failed!\n");
			break;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("get venc stream time out, try get again\n");
			cnt++;
			goto again;
		} else {
			if (FD_ISSET(VencFd, &ReadFds)){
				s32Ret = SAMPLE_VENC_SaveOneChannelStream(VencChn, fpOutput);
				if(s32Ret){
					SAMPLE_PRT("save venc stream fail, chn:%d\n", VencChn);
				}
			}
		}
#endif
	}

exit7:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit6:

	CVI_VENC_StopRecvFrame(VencChn);
	CVI_VENC_DestroyChn(VencChn);
exit5:
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();
	if (fpOutput)
		fclose(fpOutput);
	return s32Ret;
}

CVI_S32 SAMPLE_SBM_Online(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_NV12;
	SIZE_S stSize = {1920, 1080};
	CVI_CHAR *pFileNameOut = SAMPLE_STREAM_ONLINE;
	FILE *fpOutput = NULL;
	VI_VPSS_MODE_S stVIVPSSMode;
	SENSOR_CFG_S sensor_cfg;
	CVI_BOOL bPatgen = CVI_FALSE;

	fpOutput = fopen(pFileNameOut, "wb");
	if (fpOutput == NULL) {
		SAMPLE_PRT("can't open file %s\n", pFileNameOut);
		return CVI_FAILURE;
	}

	/************************************************
	 * step0:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		fclose(fpOutput);
		return CVI_FAILURE;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 1;

	//dummy vb
	u32BlkSize = COMMON_GetPicBufferSize(64, 64,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step1: sensor config
	 ************************************************/
	memset(&sensor_cfg, 0, sizeof(SENSOR_CFG_S));
	s32Ret = SAMPLE_Snsr_Parser(&sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Use Patgen\n");
		bPatgen = CVI_TRUE;
		sensor_cfg.sns_ini_cfg.devNum = 1;
	} else {
		stSize.u32Width = sensor_cfg.sns_cfg.u32ImageWigth[0];
		stSize.u32Height = sensor_cfg.sns_cfg.u32ImageHeight[0];
	}

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		goto exit1;
	}

	/************************************************
	 * step2:  Init VI
	 ************************************************/
	if (!bPatgen) {
		s32Ret = SAMPLE_Snsr_Setup(&sensor_cfg);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_Snsr_Setup failed!\n");
			goto exit1;
		}
	}

	s32Ret = SAMPLE_VI_Setup(bPatgen, &sensor_cfg, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VI_Setup failed!\n");
		goto exit1;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	VPSS_GRP		   VpssGrp		  = 0;
	VPSS_CHN		   VpssChn		  = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};
	VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;
	VPSS_MODE_S stVPSSMode;

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit2;
	}

	stVpssGrpAttr.u32MaxW						= stSize.u32Width;
	stVpssGrpAttr.u32MaxH						= stSize.u32Height;
	stVpssGrpAttr.enPixelFormat					= enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssGrpAttr.u8VpssDev						= 1;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit2;
	}

	stVpssChnAttr.u32Width						= stSize.u32Width;
	stVpssChnAttr.u32Height						= stSize.u32Height;
	stVpssChnAttr.enVideoFormat					= VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat					= enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssChnAttr.u32Depth						= 0;
	stVpssChnAttr.bMirror						= CVI_FALSE;
	stVpssChnAttr.bFlip							= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode			= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable			= CVI_FALSE;

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	stVpssChnBufWrap.bEnable = CVI_TRUE;
	stVpssChnBufWrap.u32BufLine = 64;
	stVpssChnBufWrap.u32WrapBufferSize = 5;

	s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, &stVpssChnBufWrap);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnBufWrapAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}
	/************************************************
	 * step4:  Init VENC
	 ************************************************/
	VENC_CHN VencChn = 0;
	PAYLOAD_TYPE_E enType = PT_H264;
	CVI_S32 MaxChnNum = 1;
	MMF_CHN_S stSrcChn = {CVI_ID_VPSS, VpssGrp, VpssChn};
	MMF_CHN_S stDestChn = {CVI_ID_VENC, 0, VencChn};
	gPara.Fp[0] = fpOutput;

	s32Ret = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Bind failed with %#x\n", s32Ret);
		goto exit4;
	}
	s32Ret = SAMPLE_VENC_ChnSetup(VencChn, &stSize, enType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}
	s32Ret = SAMPLE_VENC_StartGetStream(&VencChn, MaxChnNum);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit6;
	}

	SAMPLE_VENC_ExitProcess();
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);
	SAMPLE_VI_Destory(&sensor_cfg);
	CVI_VPSS_StopGrp(VpssGrp);
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
	CVI_VPSS_DestroyGrp(VpssGrp);
	CVI_VENC_StopRecvFrame(VencChn);
	CVI_VENC_DestroyChn(VencChn);
	goto exit1;

exit6:
	CVI_VENC_StopRecvFrame(VencChn);
	CVI_VENC_DestroyChn(VencChn);
exit5:
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
	SAMPLE_VI_Destory(&sensor_cfg);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	if (fpOutput)
		fclose(fpOutput);
	return s32Ret;
}

CVI_S32 SAMPLE_SBM_FRM_Online(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_NV12;
	SIZE_S stSize = {3840, 2160};
	SIZE_S stSizeSmall = {704, 576};
	CVI_CHAR *pFileNameOutMain = SAMPLE_STREAM_ONLINE_MAIN;
	CVI_CHAR *pFileNameOutSmall = SAMPLE_STREAM_ONLINE_SMALL;
	FILE *fpOutputMain = NULL;
	FILE *fpOutputSmall = NULL;
	VI_VPSS_MODE_S stVIVPSSMode;
	CVI_S32 s32Handle0 = 0, s32Handle1 = 0;
	MMF_CHN_S stChn0;
	SIZE_S stSizeRect = {1000, 1000};
	SIZE_S stSizeBitmap = {80, 60};
	CVI_BOOL bCompressed;
	SENSOR_CFG_S sensor_cfg;
	CVI_BOOL bPatgen = CVI_FALSE;

	fpOutputMain = fopen(pFileNameOutMain, "wb");
	if (fpOutputMain == NULL) {
		SAMPLE_PRT("can't open file %s\n", pFileNameOutMain);
		return CVI_FAILURE;
	}
	fpOutputSmall = fopen(pFileNameOutSmall, "wb");
	if (fpOutputSmall == NULL) {
		SAMPLE_PRT("can't open file %s\n", pFileNameOutSmall);
		fclose(fpOutputMain);
		return CVI_FAILURE;
	}

	/************************************************
	 * step0:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		fclose(fpOutputSmall);
		fclose(fpOutputMain);
		return CVI_FAILURE;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSizeSmall.u32Width, stSizeSmall.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	//vb in
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("pool[0] u32BlkSize=%d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step1: sensor config
	 ************************************************/
	memset(&sensor_cfg, 0, sizeof(SENSOR_CFG_S));
	s32Ret = SAMPLE_Snsr_Parser(&sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Use Patgen\n");
		bPatgen = CVI_TRUE;
	} else {
		stSize.u32Width = sensor_cfg.sns_cfg.u32ImageWigth[0];
		stSize.u32Height = sensor_cfg.sns_cfg.u32ImageHeight[0];
	}

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		goto exit1;
	}

	/************************************************
	 * step2:  Init VI
	 ************************************************/
	if (!bPatgen) {
		s32Ret = SAMPLE_Snsr_Setup(&sensor_cfg);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_Snsr_Setup failed!\n");
			goto exit1;
		}
	}

	s32Ret = SAMPLE_VI_Setup(bPatgen, &sensor_cfg, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VI_Setup failed!\n");
		goto exit1;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	VPSS_GRP		   VpssGrp		  = 0;
	VPSS_CHN		   VpssChn0		  = VPSS_CHN0;
	VPSS_CHN		   VpssChn1		  = VPSS_CHN1;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};
	VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;
	VPSS_MODE_S stVPSSMode;

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit2;
	}

	stVpssGrpAttr.u32MaxW						= stSize.u32Width;
	stVpssGrpAttr.u32MaxH						= stSize.u32Height;
	stVpssGrpAttr.enPixelFormat					= enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssGrpAttr.u8VpssDev						= 1;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit2;
	}

	//chn0
	stVpssChnAttr.u32Width						= stSize.u32Width;
	stVpssChnAttr.u32Height						= stSize.u32Height;
	stVpssChnAttr.enVideoFormat					= VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat					= enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssChnAttr.u32Depth						= 0;
	stVpssChnAttr.bMirror						= CVI_FALSE;
	stVpssChnAttr.bFlip							= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode			= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable			= CVI_FALSE;

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn0, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	stVpssChnBufWrap.bEnable = CVI_TRUE;
	stVpssChnBufWrap.u32BufLine = 64;
	stVpssChnBufWrap.u32WrapBufferSize = 5;

	s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn0, &stVpssChnBufWrap);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnBufWrapAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	//chn1
	stVpssChnAttr.u32Width						= stSizeSmall.u32Width;
	stVpssChnAttr.u32Height						= stSizeSmall.u32Height;
	stVpssChnAttr.enVideoFormat					= VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat					= enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssChnAttr.u32Depth						= 0;
	stVpssChnAttr.bMirror						= CVI_FALSE;
	stVpssChnAttr.bFlip							= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode			= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable			= CVI_FALSE;

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn1, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit3;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn1);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit3;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	/************************************************
	 * step4:  Init Region
	 ************************************************/
	//grp0 chn0 osdc
	stChn0.enModId = CVI_ID_VPSS;
	stChn0.s32DevId = 0;
	stChn0.s32ChnId = 0;
	s32Handle0 = 0;
	bCompressed = CVI_TRUE;
	s32Ret = SAMPLE_RGN_Setup(stChn0, stSizeRect, s32Handle0, bCompressed);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_RGN_Setup failed!\n");
		goto exit4;
	}

	//grp0 chn0 osd
	stChn0.enModId = CVI_ID_VPSS;
	stChn0.s32DevId = 0;
	stChn0.s32ChnId = 0;
	s32Handle1 = 1;
	bCompressed = CVI_FALSE;
	s32Ret = SAMPLE_RGN_Setup(stChn0, stSizeBitmap, s32Handle1, bCompressed);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_RGN_Setup failed!\n");
		goto exit5;
	}

	/************************************************
	 * step5:  Init VENC
	 ************************************************/
	PAYLOAD_TYPE_E enType = PT_H265;
	VENC_CHN VencChn[2] = { 0, 1 };
	MMF_CHN_S stSrcChn = {CVI_ID_VPSS, VpssGrp, VpssChn0};
	MMF_CHN_S stDestChn = {CVI_ID_VENC, 0, VencChn[0]};
	CVI_S32 MaxChnNum = 2;
	MMF_CHN_S stSrcChn1 = {CVI_ID_VPSS, VpssGrp, VpssChn1};
	MMF_CHN_S stDestChn1 = {CVI_ID_VENC, 0, VencChn[1]};


	gPara.Fp[0] = fpOutputMain;
	gPara.Fp[1] = fpOutputSmall;

	s32Ret = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Bind failed with %#x\n", s32Ret);
		goto exit6;
	}

	s32Ret = CVI_SYS_Bind(&stSrcChn1, &stDestChn1);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Bind failed with %#x\n", s32Ret);
		goto exit7;
	}

	s32Ret = SAMPLE_VENC_ChnSetup(VencChn[0], &stSize, enType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit8;
	}
	s32Ret = SAMPLE_VENC_ChnSetup(VencChn[1], &stSizeSmall, enType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit9;
	}

	s32Ret = SAMPLE_VENC_StartGetStream((CVI_S32 *)&VencChn, MaxChnNum);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_StartGetStream failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit10;
	}

	SAMPLE_VENC_ExitProcess();
	CVI_SYS_UnBind(&stSrcChn1, &stDestChn1);
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);

	SAMPLE_VI_Destory(&sensor_cfg);

	CVI_RGN_DetachFromChn(s32Handle1, &stChn0);
	CVI_RGN_Destroy(s32Handle1);
	CVI_RGN_DetachFromChn(s32Handle0, &stChn0);
	CVI_RGN_Destroy(s32Handle0);

	CVI_VPSS_StopGrp(VpssGrp);
	CVI_VPSS_DisableChn(VpssGrp, VpssChn0);
	CVI_VPSS_DisableChn(VpssGrp, VpssChn1);
	CVI_VPSS_DestroyGrp(VpssGrp);

	CVI_VENC_StopRecvFrame(VencChn[1]);
	CVI_VENC_DestroyChn(VencChn[1]);

	CVI_VENC_StopRecvFrame(VencChn[0]);
	CVI_VENC_DestroyChn(VencChn[0]);
	goto exit1;

exit10:
	CVI_VENC_StopRecvFrame(VencChn[1]);
	CVI_VENC_DestroyChn(VencChn[1]);
exit9:
	CVI_VENC_StopRecvFrame(VencChn[0]);
	CVI_VENC_DestroyChn(VencChn[0]);
exit8:
	CVI_SYS_UnBind(&stSrcChn1, &stDestChn1);
exit7:
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);
exit6:
	CVI_RGN_DetachFromChn(s32Handle1, &stChn0);
	CVI_RGN_Destroy(s32Handle1);
exit5:
	CVI_RGN_DetachFromChn(s32Handle0, &stChn0);
	CVI_RGN_Destroy(s32Handle0);
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn0);
	CVI_VPSS_DisableChn(VpssGrp, VpssChn1);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
	SAMPLE_VI_Destory(&sensor_cfg);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	if (fpOutputMain)
		fclose(fpOutputMain);
	if (fpOutputSmall)
		fclose(fpOutputSmall);
	return s32Ret;
}


CVI_S32 SAMPLE_DOUBLE_SBM(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_NV12;
	SIZE_S stSize[2] = {{1920, 1080}, {1920, 1080}};
	CVI_CHAR *pFileNameOut1 = "chn0.h264";
	CVI_CHAR *pFileNameOut2 = "chn1.h264";
	FILE *fpOutput1 = NULL;
	FILE *fpOutput2 = NULL;
	VI_VPSS_MODE_S stVIVPSSMode;
	SENSOR_CFG_S sensor_cfg;
	CVI_BOOL bPatgen = CVI_FALSE;

	fpOutput1 = fopen(pFileNameOut1, "wb");
	if (fpOutput1 == NULL) {
		SAMPLE_PRT("can't open file %s\n", pFileNameOut1);
		return CVI_FAILURE;
	}
	fpOutput2 = fopen(pFileNameOut2, "wb");
	if (fpOutput2 == NULL) {
		SAMPLE_PRT("can't open file %s\n", pFileNameOut2);
		fclose(fpOutput1);
		return CVI_FAILURE;
	}

	/************************************************
	 * step0:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		fclose(fpOutput1);
		fclose(fpOutput2);
		return CVI_FAILURE;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 1;

	//dummy vb
	u32BlkSize = COMMON_GetPicBufferSize(64, 64,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step1: sensor config
	 ************************************************/
	memset(&sensor_cfg, 0, sizeof(SENSOR_CFG_S));
	s32Ret = SAMPLE_Snsr_Parser(&sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Use Patgen\n");
		bPatgen = CVI_TRUE;
	} else {
		stSize[0].u32Width = sensor_cfg.sns_cfg.u32ImageWigth[0];
		stSize[0].u32Height = sensor_cfg.sns_cfg.u32ImageHeight[0];
		stSize[1].u32Width = sensor_cfg.sns_cfg.u32ImageWigth[1];
		stSize[1].u32Height = sensor_cfg.sns_cfg.u32ImageHeight[1];
	}

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		goto exit1;
	}

	/************************************************
	 * step2:  Init VI
	 ************************************************/
	if (!bPatgen) {
		s32Ret = SAMPLE_Snsr_Setup(&sensor_cfg);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_Snsr_Setup failed!\n");
			goto exit1;
		}
	}

	s32Ret = SAMPLE_VI_Setup(bPatgen, &sensor_cfg, &stSize[0]);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VI_Setup failed!\n");
		goto exit1;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	VPSS_GRP		   VpssGrp		  = 0;
	VPSS_CHN		   VpssChn		  = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};
	VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;
	VPSS_MODE_S stVPSSMode;

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit2;
	}

	for (i = 0; i < 2; i++) {
		VpssGrp = i;

		stVpssGrpAttr.u32MaxW						= stSize[i].u32Width;
		stVpssGrpAttr.u32MaxH						= stSize[i].u32Height;
		stVpssGrpAttr.enPixelFormat					= enPixelFormat;
		stVpssGrpAttr.stFrameRate.s32SrcFrameRate	= -1;
		stVpssGrpAttr.stFrameRate.s32DstFrameRate	= -1;
		stVpssGrpAttr.u8VpssDev						= 1;

		s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
			goto exit3;
		}

		stVpssChnAttr.u32Width						= stSize[i].u32Width;
		stVpssChnAttr.u32Height						= stSize[i].u32Height;
		stVpssChnAttr.enVideoFormat					= VIDEO_FORMAT_LINEAR;
		stVpssChnAttr.enPixelFormat					= enPixelFormat;
		stVpssChnAttr.stFrameRate.s32SrcFrameRate	= -1;
		stVpssChnAttr.stFrameRate.s32DstFrameRate	= -1;
		stVpssChnAttr.u32Depth						= 0;
		stVpssChnAttr.bMirror						= CVI_FALSE;
		stVpssChnAttr.bFlip							= CVI_FALSE;
		stVpssChnAttr.stAspectRatio.enMode			= ASPECT_RATIO_NONE;
		stVpssChnAttr.stNormalize.bEnable			= CVI_FALSE;

		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit3;
		}

		stVpssChnBufWrap.bEnable = CVI_TRUE;
		stVpssChnBufWrap.u32BufLine = 64;
		stVpssChnBufWrap.u32WrapBufferSize = 5;

		s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, &stVpssChnBufWrap);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_SetChnBufWrapAttr failed with %#x\n", s32Ret);
			goto exit3;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit3;
		}

		/*start vpss*/
		s32Ret = CVI_VPSS_StartGrp(VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
			goto exit3;
		}
	}
	/************************************************
	 * step4:  Init VENC
	 ************************************************/
	PAYLOAD_TYPE_E enType = PT_H264;
	VENC_CHN VencChn[2] = { 0, 1 };
	MMF_CHN_S stSrcChn = {CVI_ID_VPSS, 0, 0};
	MMF_CHN_S stDestChn = {CVI_ID_VENC, 0, VencChn[0]};
	CVI_S32 MaxChnNum = 2;
	MMF_CHN_S stSrcChn1 = {CVI_ID_VPSS, 1, 0};
	MMF_CHN_S stDestChn1 = {CVI_ID_VENC, 0, VencChn[1]};


	gPara.Fp[0] = fpOutput1;
	gPara.Fp[1] = fpOutput2;

	s32Ret = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Bind failed with %#x\n", s32Ret);
		goto exit3;
	}

	s32Ret = CVI_SYS_Bind(&stSrcChn1, &stDestChn1);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Bind failed with %#x\n", s32Ret);
		goto exit4;
	}

	s32Ret = SAMPLE_VENC_ChnSetup(VencChn[0], &stSize[0], enType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}
	s32Ret = SAMPLE_VENC_ChnSetup(VencChn[1], &stSize[1], enType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit6;
	}

	s32Ret = SAMPLE_VENC_StartGetStream((CVI_S32 *)&VencChn, MaxChnNum);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_StartGetStream failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit7;
	}

	SAMPLE_VENC_ExitProcess();
	CVI_SYS_UnBind(&stSrcChn1, &stDestChn1);
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);

	SAMPLE_VI_Destory(&sensor_cfg);

	for (i = 0; i < 2; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, VpssChn);
		CVI_VPSS_DestroyGrp(i);
	}
	CVI_VENC_StopRecvFrame(VencChn[1]);
	CVI_VENC_StopRecvFrame(VencChn[0]);
	CVI_VENC_DestroyChn(VencChn[1]);
	CVI_VENC_DestroyChn(VencChn[0]);
	goto exit1;

exit7:
	CVI_VENC_DestroyChn(VencChn[1]);
exit6:
	CVI_VENC_DestroyChn(VencChn[0]);
exit5:
	CVI_SYS_UnBind(&stSrcChn1, &stDestChn1);
exit4:
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);
exit3:
	for (i = 0; i < 2; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, VpssChn);
		CVI_VPSS_DestroyGrp(i);
	}
exit2:
	SAMPLE_VI_Destory(&sensor_cfg);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	if (fpOutput1)
		fclose(fpOutput1);
	if (fpOutput2)
		fclose(fpOutput2);

	return s32Ret;
}

CVI_S32 SAMPLE_JPEG_SBM(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_NV12;
	SIZE_S stSize = {1920, 1080};
	SIZE_S stJpegSize = {8192, 8192};
	FILE *fpOutput = NULL;
	CVI_CHAR astJpegName[64];
	VI_VPSS_MODE_S stVIVPSSMode;
	SENSOR_CFG_S sensor_cfg;
	CVI_BOOL bPatgen = CVI_FALSE;
	CVI_S32 i = 0;
	VIDEO_FRAME_INFO_S stVideoFrame;

	/************************************************
	 * step0:  Init SYS
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step1: sensor config
	 ************************************************/
	memset(&sensor_cfg, 0, sizeof(SENSOR_CFG_S));
	s32Ret = SAMPLE_Snsr_Parser(&sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Use Patgen\n");
		bPatgen = CVI_TRUE;
		sensor_cfg.sns_ini_cfg.devNum = 1;
	} else {
		stSize.u32Width = sensor_cfg.sns_cfg.u32ImageWigth[0];
		stSize.u32Height = sensor_cfg.sns_cfg.u32ImageHeight[0];
	}

	/************************************************
	 * step2:  Init common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * Config vpss online mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		goto exit1;
	}

	/************************************************
	 * step3:  Init VI
	 ************************************************/
	if (!bPatgen) {
		s32Ret = SAMPLE_Snsr_Setup(&sensor_cfg);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_Snsr_Setup failed!\n");
			goto exit1;
		}
	}

	s32Ret = SAMPLE_VI_Setup(bPatgen, &sensor_cfg, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VI_Setup failed!\n");
		goto exit1;
	}

	/************************************************
	 * step4:  Init VPSS
	 ************************************************/
	VPSS_GRP		   VpssGrp		  = 0;
	VPSS_CHN		   VpssChn		  = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};
	VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;
	VPSS_MODE_S stVPSSMode;

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit2;
	}

	stVpssGrpAttr.u32MaxW						= stSize.u32Width;
	stVpssGrpAttr.u32MaxH						= stSize.u32Height;
	stVpssGrpAttr.enPixelFormat					= enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssGrpAttr.u8VpssDev						= 1;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit2;
	}

	stVpssChnAttr.u32Width						= stJpegSize.u32Width;
	stVpssChnAttr.u32Height						= stJpegSize.u32Height;
	stVpssChnAttr.enVideoFormat					= VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat					= enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssChnAttr.u32Depth						= 0;
	stVpssChnAttr.bMirror						= CVI_FALSE;
	stVpssChnAttr.bFlip							= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode			= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable			= CVI_FALSE;

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	stVpssChnBufWrap.bEnable = CVI_TRUE;
	stVpssChnBufWrap.u32BufLine = 128;
	stVpssChnBufWrap.u32WrapBufferSize = 5;

	s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, &stVpssChnBufWrap);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnBufWrapAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	/************************************************
	 * step5:  Init VENC
	 ************************************************/
	VENC_CHN VencChn = 0;
	PAYLOAD_TYPE_E enType = PT_JPEG;
	MMF_CHN_S stSrcChn = {CVI_ID_VPSS, VpssGrp, VpssChn};
	MMF_CHN_S stDestChn = {CVI_ID_VENC, 0, VencChn};
	CVI_S32 VencFd;

	s32Ret = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Bind failed with %#x\n", s32Ret);
		goto exit4;
	}
	s32Ret = SAMPLE_VENC_ChnSetup(VencChn, &stJpegSize, enType);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_ChnSetup failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	VencFd = CVI_VENC_GetFd(VencChn);
	if (VencFd < 0)
		goto exit6;

	while (!gSampleVencExit) {
		SAMPLE_PRT("\npress 'Enter' to Snap frame.\n");
		getchar();
		if (gSampleVencExit) {
			break;
		}

		s32Ret = CVI_VI_GetChnFrame(0, 0, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_SendFrame fail. s32Ret: 0x%x !\n", s32Ret);
			CVI_VI_ReleaseChnFrame(0, 0, &stVideoFrame);
			break;
		}
		CVI_VI_ReleaseChnFrame(0, 0, &stVideoFrame);

		snprintf(astJpegName, 63, "sbm-%dx%d_%d.jpeg", stJpegSize.u32Width, stJpegSize.u32Height, i++);
		fpOutput = fopen(astJpegName, "wb");
		if (fpOutput == NULL) {
			SAMPLE_PRT("can't open file %s\n", astJpegName);
			break;
		}

again:
#if defined(CONFIG_DUAL_OS)
		s32Ret = SAMPLE_VENC_SaveOneChannelStream(VencChn, fpOutput);
		if (s32Ret) {
			SAMPLE_PRT("get venc stream time out, try get again\n");
			goto again;
		}
#else
		struct timeval timeout_val;
		fd_set ReadFds;

		FD_ZERO(&ReadFds);
		FD_SET(VencFd, &ReadFds);
		timeout_val.tv_sec = 2; /* 2 is a number */
		timeout_val.tv_usec = 0;
		s32Ret = select(VencFd + 1, &ReadFds, CVI_NULL, CVI_NULL, &timeout_val);
		if (s32Ret < 0) {
			SAMPLE_PRT("select failed!\n");
			break;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("get venc stream time out, try get again\n");
			goto again;
		} else {
			if (FD_ISSET(VencFd, &ReadFds)){
				s32Ret = SAMPLE_VENC_SaveOneChannelStream(VencChn, fpOutput);
				if(s32Ret){
					SAMPLE_PRT("save venc stream fail, chn:%d\n", VencChn);
				}
			}
		}
#endif
		SAMPLE_PRT("Save file:%s\n", astJpegName);
		if (fpOutput) {
			fclose(fpOutput);
			fpOutput = NULL;
		}
	}

	if (fpOutput)
			fclose(fpOutput);

	CVI_SYS_UnBind(&stSrcChn, &stDestChn);

	SAMPLE_VI_Destory(&sensor_cfg);

	CVI_VPSS_StopGrp(VpssGrp);
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
	CVI_VPSS_DestroyGrp(VpssGrp);

	CVI_VENC_StopRecvFrame(VencChn);
	CVI_VENC_DestroyChn(VencChn);
	goto exit1;
exit6:
	CVI_VENC_StopRecvFrame(VencChn);
	CVI_VENC_DestroyChn(VencChn);
exit5:
	CVI_SYS_UnBind(&stSrcChn, &stDestChn);
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
	SAMPLE_VI_Destory(&sensor_cfg);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();
	return s32Ret;
}

CVI_VOID SAMPLE_SBM_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		//todo for release
		gSampleVencExit = CVI_TRUE;
		SAMPLE_PRT("Program termination abnormally\n");
	}
}

CVI_VOID SAMPLE_SBM_Usage(CVI_CHAR *sPrgNm)
{
	SAMPLE_PRT("Usage : %s <index>\n", sPrgNm);
	SAMPLE_PRT("index:\n");
	SAMPLE_PRT("\t 0)read file, vpss-venc sbm\n");
	SAMPLE_PRT("\t 1)vi-vpss online, vpss-venc sbm\n");
	SAMPLE_PRT("\t 2)vi-vpss online, vpss-venc sbm + frame mode\n");
	SAMPLE_PRT("\t 3)double sbm\n");
	SAMPLE_PRT("\t 4)jpeg sbm\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32Index;

	if (argc < 2) {
		SAMPLE_SBM_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (!strncmp(argv[1], "-h", 2)) {
		SAMPLE_SBM_Usage(argv[0]);
		return CVI_SUCCESS;
	}

	signal(SIGINT, SAMPLE_SBM_HandleSig);
	signal(SIGTERM, SAMPLE_SBM_HandleSig);

	s32Index = atoi(argv[1]);
	switch (s32Index) {
	case 0:
		s32Ret = SAMPLE_SBM_Offline();
		break;
	case 1:
		s32Ret = SAMPLE_SBM_Online();
		break;
	case 2:
		s32Ret = SAMPLE_SBM_FRM_Online();
		break;
	case 3:
		s32Ret = SAMPLE_DOUBLE_SBM();
		break;
	case 4:
		s32Ret = SAMPLE_JPEG_SBM();
		break;
	default:
		SAMPLE_PRT("the index %d is invaild!\n", s32Index);
		SAMPLE_SBM_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (s32Ret == CVI_SUCCESS)
		SAMPLE_PRT("sample_sbm exit success!\n");
	else
		SAMPLE_PRT("sample_sbm exit abnormally!\n");

	return s32Ret;
}

