#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include "sample_comm.h"
#include "cvi_venc.h"

#define MAX_ROI_NUM 4
//test cases for ROI H264/H265
const VENC_ROI_ATTR_S gRoiAttrTestCase[2][4] = {
	// H264 4 ROI test cases.
	{
		{
			.u32Index = 0,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_TRUE,
			.s32Qp = 10,
			.stRect = { .s32X = 64, .s32Y = 64, .u32Width = 64, .u32Height = 64 }
		},
		{
			.u32Index = 1,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_FALSE,
			.s32Qp = -10,
			.stRect = { .s32X = 256, .s32Y = 64, .u32Width = 64, .u32Height = 64 }
		},
		{
			.u32Index = 2,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_TRUE,
			.s32Qp = 5,
			.stRect = { .s32X = 64, .s32Y = 256, .u32Width = 64, .u32Height = 64 }
		},
		{
			.u32Index = 3,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_FALSE,
			.s32Qp = -5,
			.stRect = { .s32X = 256, .s32Y = 256, .u32Width = 64, .u32Height = 64 }
		},
	},
	// H265 4 ROI test cases.
	{
		{
			.u32Index = 0,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_FALSE,
			.s32Qp = -8,
			.stRect = { .s32X = 64, .s32Y = 64, .u32Width = 64, .u32Height = 64 }
		},
		{
			.u32Index = 1,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_FALSE,
			.s32Qp = -5,
			.stRect = { .s32X = 256, .s32Y = 64, .u32Width = 64, .u32Height = 64 }
		},
		{
			.u32Index = 2,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_FALSE,
			.s32Qp = -8,
			.stRect = { .s32X = 64, .s32Y = 256, .u32Width = 64, .u32Height = 64 }
		},
		{
			.u32Index = 3,
			.bEnable = CVI_TRUE,
			.bAbsQp = CVI_FALSE,
			.s32Qp = -5,
			.stRect = { .s32X = 256, .s32Y = 256, .u32Width = 64, .u32Height = 64 }
		},
	},
};

#define GET_FILE_POSTFIX(payload, postfix) \
	do { \
		switch (payload) { \
			case PT_H264: strcpy(postfix, ".h264"); break; \
			case PT_H265: strcpy(postfix, ".h265"); break; \
			case PT_JPEG: strcpy(postfix, ".jpg"); break; \
			case PT_MJPEG: strcpy(postfix, ".mjp"); break; \
			default: return CVI_FAILURE; \
		} \
	} while (0)

static CVI_U32 raster2zscan16[16] = {
	0,  1,  4,  5,
	2,  3,  6,  7,
	8,  9,  12, 13,
	10, 11, 14, 15
};

VB_POOL gVencPicVbPool[VB_MAX_COMM_POOLS] = { [0 ...(VB_MAX_COMM_POOLS - 1)] = VB_INVALID_POOLID };
VB_POOL gVencPicInfoVbPool[VB_MAX_COMM_POOLS] = { [0 ...(VB_MAX_COMM_POOLS - 1)] = VB_INVALID_POOLID };

typedef struct _SAMPLE_COMM_VENC_GET_STREAM_ {
	SAMPLE_VENC_GETSTREAM_PARA_S *pstPara;
	VENC_CHN_ATTR_S stVencChnAttr;
	PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
	CVI_CHAR file_ext[16];
	CVI_CHAR fileName[VENC_MAX_CHN_NUM][MAX_STRING_LEN];
	CVI_S32 maxfd;
	FILE *pFile[VENC_MAX_CHN_NUM];
	CVI_S32 VencFd[VENC_MAX_CHN_NUM];
	CVI_U32 u32PictureCnt[VENC_MAX_CHN_NUM];
} SAMPLE_COMM_VENC_GET_STREAM;

enum _SAMPLE_COMM_VENC_STAT_ {
	SCV_STAT_CONTINUE = 1,
	SCV_STAT_BREAK,
} SAMPLE_COMM_VENC_STAT;

pthread_t gs_VencTask[VENC_MAX_CHN_NUM];
pthread_t gs_VencSendTask[VENC_MAX_CHN_NUM];

static CVI_S32 SAMPLE_COMM_VENC_GetDataType(PAYLOAD_TYPE_E enType, VENC_PACK_S *ppack);
static CVI_S32 SAMPLE_COMM_VENC_SetChnAttr(
		chnInputCfg * pIc,
		VENC_CHN_ATTR_S *pstVencChnAttr,
		PAYLOAD_TYPE_E enType,
		PIC_SIZE_E enSize,
		SAMPLE_RC_E enRcMode,
		CVI_U32 u32Profile,
		VENC_GOP_ATTR_S *pstGopAttr,
		CVI_BOOL bRcnRefShareBuf);
static CVI_S32	SAMPLE_COMM_VENC_SetRcParam(
		chnInputCfg * pIc,
		VENC_CHN VencChn);
static CVI_S32	SAMPLE_COMM_VENC_SetRefParam(
		chnInputCfg * pIc,
		VENC_CHN VencChn);
static CVI_S32 SAMPLE_COMM_VENC_SetFrameLost(
		chnInputCfg * pIc,
		VENC_CHN VencChn);
static CVI_S32 SAMPLE_COMM_VENC_SetSuperFrame(
		chnInputCfg * pIc,
		VENC_CHN VencChn);
static CVI_S32 SAMPLE_COMM_VENC_SetCuPrediction(
		chnInputCfg * pIc,
		VENC_CHN VencChn);
static CVI_S32 SAMPLE_COMM_VENC_DetachVbPool(VENC_CHN VencChn);

static CVI_S32 SAMPLE_COMM_VENC_SetDebreathEffect(
		chnInputCfg * pIc,
		VENC_CHN VencChn);

CVI_VOID SAMPLE_COMM_VENC_InitCommonInputCfg(commonInputCfg *pCic)
{
	if (!pCic) {
		SAMPLE_PRT("pCic = NULL\n");
		return;
	}

	memset(pCic, 0, sizeof(commonInputCfg));
	pCic->ifInitVb = 1;
	pCic->vbMode = VB_SOURCE_COMMON;
	pCic->h265RefreshType = 0;
	pCic->jpegMarkerOrder = 0;
	pCic->bThreadDisable = 0;
}

CVI_VOID SAMPLE_COMM_VENC_InitChnInputCfg(chnInputCfg *pIc)
{
	if (!pIc) {
		SAMPLE_PRT("pIc = NULL\n");
		return;
	}

	memset(pIc, 0, sizeof(chnInputCfg));
	pIc->u32Profile = CVI_H264_PROFILE_DEFAULT;
	pIc->rcMode = -1;
	pIc->iqp = -1;
	pIc->pqp = -1;
	pIc->gop = CVI_H26X_GOP_DEFAULT;
	pIc->gopMode = CVI_H26X_GOP_MODE_DEFAULT;
	pIc->bitrate = -1;
	pIc->firstFrmstartQp = -1;
	pIc->num_frames = -1;
	pIc->framerate = 30;
	pIc->bVariFpsEn = 0;
	pIc->maxIprop = CVI_H26X_MAX_I_PROP_DEFAULT;
	pIc->minIprop = CVI_H26X_MIN_I_PROP_DEFAULT;
	pIc->maxQp = -1;
	pIc->minQp = -1;
	pIc->maxIqp = -1;
	pIc->minIqp = -1;
	pIc->quality = -1;
	pIc->maxbitrate = -1;
	pIc->statTime = -1;
	pIc->bind_mode = VENC_BIND_DISABLE;
	pIc->pixel_format = 0;
	pIc->bitstreamBufSize = 0;
	pIc->forceIdr = -1;
	pIc->chgNum = -1;
	pIc->tempLayer = 0;
	pIc->bgInterval = CVI_H26X_SMARTP_BG_INTERVAL_DEFAULT;
	pIc->frameLost = -1;
	pIc->frameLostBspThr = -1;
	pIc->frameLostGap = -1;
	pIc->MCUPerECS = 0;
	pIc->sendframe_timeout = 20000;
	pIc->getstream_timeout = -1;
	pIc->s32IPQpDelta = CVI_H26X_NORMALP_IP_QP_DELTA_DEFAULT;
	pIc->s32BgQpDelta = CVI_H26X_SMARTP_BG_QP_DELTA_DEFAULT;
	pIc->s32ViQpDelta = CVI_H26X_SMARTP_VI_QP_DELTA_DEFAULT;
	pIc->initialDelay = CVI_INITIAL_DELAY_DEFAULT;
	pIc->h264EntropyMode = H264E_ENTROPY_CABAC;
	pIc->h264ChromaQpOffset = 0;
	pIc->h265CbQpOffset = 0;
	pIc->h265CrQpOffset = 0;
	pIc->u32RowQpDelta = CVI_H26X_ROW_QP_DELTA_DEFAULT;
	pIc->enSuperFrmMode = CVI_H26X_SUPER_FRM_MODE_DEFAULT;
	pIc->u32SuperIFrmBitsThr = CVI_H26X_SUPER_I_BITS_THR_DEFAULT;
	pIc->u32SuperPFrmBitsThr = CVI_H26X_SUPER_P_BITS_THR_DEFAULT;
	pIc->s32MaxReEncodeTimes = CVI_H26X_MAX_RE_ENCODE_DEFAULT;

	pIc->aspectRatioInfoPresentFlag = CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_DEFAULT;
	pIc->aspectRatioIdc = CVI_H26X_ASPECT_RATIO_IDC_DEFAULT;
	pIc->overscanInfoPresentFlag = CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_DEFAULT;
	pIc->overscanAppropriateFlag = CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_DEFAULT;
	pIc->sarWidth = CVI_H26X_SAR_WIDTH_DEFAULT;
	pIc->sarHeight = CVI_H26X_SAR_HEIGHT_DEFAULT;

	pIc->timingInfoPresentFlag = CVI_H26X_TIMING_INFO_PRESENT_FLAG_DEFAULT;
	pIc->fixedFrameRateFlag = CVI_H264_FIXED_FRAME_RATE_FLAG_DEFAULT;
	pIc->numUnitsInTick = CVI_H26X_NUM_UNITS_IN_TICK_DEFAULT;
	pIc->timeScale = CVI_H26X_TIME_SCALE_DEFAULT;

	pIc->videoSignalTypePresentFlag = CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_DEFAULT;
	pIc->videoFormat = CVI_H26X_VIDEO_FORMAT_DEFAULT;
	pIc->videoFullRangeFlag = CVI_H26X_VIDEO_FULL_RANGE_FLAG_DEFAULT;
	pIc->colourDescriptionPresentFlag = CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_DEFAULT;
	pIc->colourPrimaries = CVI_H26X_COLOUR_PRIMARIES_DEFAULT;
	pIc->transferCharacteristics = CVI_H26X_TRANSFER_CHARACTERISTICS_DEFAULT;
	pIc->matrixCoefficients = CVI_H26X_MATRIX_COEFFICIENTS_DEFAULT;

	pIc->u32FrameQp = CVI_H26X_FRAME_QP_DEFAULT;
	pIc->bEsBufQueueEn = CVI_H26X_ES_BUFFER_QUEUE_DEFAULT;
	pIc->bIsoSendFrmEn = CVI_H26X_ISO_SEND_FRAME_DEFAUL;
	pIc->bSensorEn = CVI_H26X_SENSOR_EN_DEFAULT;
	pIc->bDebreathEn = CVI_FALSE;

	pIc->u32SliceCnt = 1;

}

// Map command line input pixel format to PIXEL_FORMAT_E.
PIXEL_FORMAT_E vencMapPixelFormat(CVI_S32 pixel_format)
{
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	switch (pixel_format) {
	case 0:
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		break;
	case 1:
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;
		break;
	case 2:
		enPixelFormat = PIXEL_FORMAT_NV12;
		break;
	case 3:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	case 4:
		enPixelFormat = PIXEL_FORMAT_NV16;
		break;
	case 5:
		enPixelFormat = PIXEL_FORMAT_NV61;
		break;
	case 6:
		enPixelFormat = PIXEL_FORMAT_YUYV;
		break;
	case 7:
		enPixelFormat = PIXEL_FORMAT_UYVY;
		break;
	case 8:
		enPixelFormat = PIXEL_FORMAT_YVYU;
		break;
	case 9:
		enPixelFormat = PIXEL_FORMAT_VYUY;
		break;
	case 10:
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_444;
		break;
	case 11:
		enPixelFormat = PIXEL_FORMAT_YUV_400;
		break;
	default:
		SAMPLE_PRT("Unknown input pixel format. Assume YUV420P.\n");
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		break;
	}

	return enPixelFormat;
}

CVI_S32 SAMPLE_COMM_VENC_SaveStream(PAYLOAD_TYPE_E enType,
		FILE *pFd, VENC_STREAM_S *pstStream)
{
	VENC_PACK_S *ppack;
	CVI_S32 dataType;

	if (!pFd) {
		SAMPLE_PRT("pFd = NULL\n");
		return CVI_FAILURE;
	}

	SAMPLE_PRT("u32PackCount = %d\n", pstStream->u32PackCount);

	for (CVI_U32 i = 0; i < pstStream->u32PackCount; i++) {
		ppack = &pstStream->pstPack[i];
		fwrite(ppack->pu8Addr + ppack->u32Offset,
				ppack->u32Len - ppack->u32Offset, 1, pFd);

		dataType = SAMPLE_COMM_VENC_GetDataType(enType, ppack);
		if (dataType < 0) {
			SAMPLE_PRT("dataType = %d\n", dataType);
			return CVI_FAILURE;
		}
		// SAMPLE_PRT("pack[%d], PTS = %"PRId64", DTS = %"PRId64", DataType = %d\n",
		// 		i, ppack->u64PTS, ppack->u64DTS, dataType);
		// SAMPLE_PRT("Addr = %p, Len = 0x%X, Offset = 0x%X\n",
		// 		ppack->pu8Addr, ppack->u32Len, ppack->u32Offset);
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VENC_SaveChannelStream(vencChnCtx *pvecc)
{
	VENC_CHN_STATUS_S stStat;
	VENC_CHN_ATTR_S stVencChnAttr;
	VENC_STREAM_S stStream;
	VENC_CHN VencChn = pvecc->VencChn;
	chnInputCfg *pIc = &pvecc->chnIc;
	CVI_S32 s32Ret;

	do {
		s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			return s32Ret;
		}

		if (!stStat.u32CurPacks) {
			SAMPLE_PRT("u32CurPacks = NULL!\n");
			return s32Ret;
		}

		stStream.pstPack =
			(VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
		if (stStream.pstPack == NULL) {
			SAMPLE_PRT("malloc memory failed!\n");
			return s32Ret;
		}
		s32Ret = CVI_VENC_GetStream(VencChn, &stStream, pIc->getstream_timeout);
		if (s32Ret != CVI_SUCCESS) {
			if (s32Ret == CVI_ERR_VENC_BUSY) {
				// SAMPLE_PRT("CVI_VENC_GetStream, VencChn Retry= %d,s32Ret = 0x%X\n",
				// VencChn, s32Ret);
				return CVI_ERR_VENC_BUSY;
			} else if (s32Ret == CVI_ERR_VENC_GET_STREAM_END) {
				s32Ret = CVI_ERR_VENC_GET_STREAM_END;
				break;
			} else {
				SAMPLE_PRT("CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n",
					VencChn, s32Ret);
				break;
			}
		}

		SAMPLE_PRT("get chn:%d count:%d\n", pvecc->VencChn, stStream.u32PackCount);

		if (!(pvecc->perf == 1)) {
			s32Ret = SAMPLE_COMM_VENC_SaveStream(
					stVencChnAttr.stVencAttr.enType,
					pvecc->pFile,
					&stStream);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_VENC_SaveStream, s32Ret = %d\n", s32Ret);
				break;
			}
		}

		s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_ReleaseStream, s32Ret = %d\n", s32Ret);
			break;
		}
	} while (CVI_FALSE);

	free(stStream.pstPack);
	stStream.pstPack = NULL;

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_GetDataType(PAYLOAD_TYPE_E enType, VENC_PACK_S *ppack)
{
	if (enType == PT_H264)
		return ppack->DataType.enH264EType;
	else if (enType == PT_H265)
		return ppack->DataType.enH265EType;
	else if (enType == PT_JPEG || enType == PT_MJPEG)
		return ppack->DataType.enJPEGEType;

	SAMPLE_PRT("enType = %d\n", enType);
	return CVI_FAILURE;
}

static CVI_S32 SAMPLE_COMM_VENC_DetachVbPool(VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_SOURCE_E eVbSource;
	VENC_CHN_ATTR_S stChnAttr;
	VENC_PARAM_MOD_S stModParam;

	s32Ret = CVI_VENC_GetChnAttr(VencChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetChnAttr vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}

	if (stChnAttr.stVencAttr.enType == PT_H264) {
		stModParam.enVencModType = MODTYPE_H264E;
	} else if (stChnAttr.stVencAttr.enType == PT_H265) {
		stModParam.enVencModType = MODTYPE_H265E;
	} else {
		return s32Ret;
	}

	s32Ret = CVI_VENC_GetModParam(&stModParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetModParam vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}

	if (stChnAttr.stVencAttr.enType == PT_H264)
		eVbSource = stModParam.stH264eModParam.enH264eVBSource;
	else if (stChnAttr.stVencAttr.enType == PT_H265)
		eVbSource = stModParam.stH265eModParam.enH265eVBSource;
	else {
		return s32Ret;
	}
	//get_modparam	-> user mode
	SAMPLE_PRT("eVbSource[%d]\n", eVbSource);
	if (eVbSource == VB_SOURCE_USER) {
		s32Ret = CVI_VENC_DetachVbPool(VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_DetachVbPool vechn[%d] failed with %#x!\n",
					VencChn, s32Ret);
			return CVI_FAILURE;
		}
		if(gVencPicVbPool[VencChn] != VB_INVALID_POOLID)
			CVI_VB_DestroyPool(gVencPicVbPool[VencChn]);
	}
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_Stop(VENC_CHN VencChn)
{
	CVI_S32 s32Ret;

	if (gs_VencTask[VencChn] != 0) {
		pthread_join(gs_VencTask[VencChn], CVI_NULL);
		SAMPLE_PRT("GetVencStreamProc done\n");

		gs_VencTask[VencChn] = 0;
	}

	if (gs_VencSendTask[VencChn] != 0) {
		pthread_join(gs_VencSendTask[VencChn], CVI_NULL);
		SAMPLE_PRT("SednVencFrameProc done, chn:%d\n", VencChn);

		gs_VencSendTask[VencChn] = 0;
	}

	s32Ret = CVI_VENC_StopRecvFrame(VencChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VENC_ResetChn(VencChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_ResetChn vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VENC_DetachVbPool(VencChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VENC_DetachVbPool vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VENC_DestroyChn(VencChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_DestroyChn vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}


CVI_S32 SAMPLE_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, char *szFilePostfix)
{
	GET_FILE_POSTFIX(enPayload, szFilePostfix);
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VENC_GetGopAttr(VENC_GOP_MODE_E enGopMode, VENC_GOP_ATTR_S *pstGopAttr)
{
	switch (enGopMode) {
	case VENC_GOPMODE_NORMALP:
		pstGopAttr->enGopMode = VENC_GOPMODE_NORMALP;
		pstGopAttr->stNormalP.s32IPQpDelta = CVI_H26X_NORMALP_IP_QP_DELTA_DEFAULT;
		SAMPLE_PRT("s32IPQpDelta = %d\n", pstGopAttr->stNormalP.s32IPQpDelta);
		break;

	case VENC_GOPMODE_SMARTP:
		pstGopAttr->enGopMode = VENC_GOPMODE_SMARTP;
		pstGopAttr->stSmartP.s32BgQpDelta = CVI_H26X_SMARTP_BG_QP_DELTA_DEFAULT;
		pstGopAttr->stSmartP.s32ViQpDelta = CVI_H26X_SMARTP_VI_QP_DELTA_DEFAULT;
		pstGopAttr->stSmartP.u32BgInterval = CVI_H26X_SMARTP_BG_INTERVAL_DEFAULT;
		SAMPLE_PRT("u32BgInterval %d, s32BgQpDelta %d, s32ViQpDelta %d\n",
				pstGopAttr->stSmartP.u32BgInterval,
				pstGopAttr->stSmartP.s32BgQpDelta,
				pstGopAttr->stSmartP.s32ViQpDelta);
		break;

	default:
		SAMPLE_PRT("not support the gop mode %d!\n", enGopMode);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VENC_CloseReEncode(VENC_CHN VencChn)
{
	CVI_S32 s32Ret;
	VENC_RC_PARAM_S stRcParam;
	VENC_CHN_ATTR_S stChnAttr;

	s32Ret = CVI_VENC_GetChnAttr(VencChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetChnAttr failed!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VENC_GetRcParam(VencChn, &stRcParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetRcParam failed!\n");
		return CVI_FAILURE;
	}

	if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264CBR)
		stRcParam.stParamH264Cbr.s32MaxReEncodeTimes = 0;
	else if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264VBR)
		stRcParam.stParamH264Vbr.s32MaxReEncodeTimes = 0;
	else if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265CBR)
		stRcParam.stParamH264Cbr.s32MaxReEncodeTimes = 0;
	else if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265VBR)
		stRcParam.stParamH264Vbr.s32MaxReEncodeTimes = 0;
	else
		return CVI_SUCCESS;

	s32Ret = CVI_VENC_SetRcParam(VencChn, &stRcParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetRcParam failed!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 SetRoiAttr(VENC_CHN VencChn, const VENC_ROI_ATTR_S *roiAttr) {
	CVI_S32 s32Ret = CVI_VENC_SetRoiAttr(VencChn, roiAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetRoiAttr failed!\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VENC_SetRoiAttr(VENC_CHN VencChn, PAYLOAD_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int type = (enType == PT_H264) ? 0 : 1;
	CVI_U32 i = 0;

	for (i = 0; i < MAX_ROI_NUM; i++) {
		VENC_ROI_ATTR_S RoiAttr;

		s32Ret = CVI_VENC_GetRoiAttr(VencChn, i, &RoiAttr);
		if (s32Ret != CVI_SUCCESS || i != RoiAttr.u32Index) {
			SAMPLE_PRT("GetRoiAttr failed!\n");
			return CVI_FAILURE;
		}

		RoiAttr = gRoiAttrTestCase[type][i];
		s32Ret = SetRoiAttr(VencChn, &RoiAttr);
	}

	return s32Ret;
}

static void alignQpRoi(VENC_ROI_ATTR_S *pRoiAttr)
{
	if (pRoiAttr->bEnable == CVI_FALSE)
		return;
	int start_x = (pRoiAttr->stRect.s32X >> 4) << 4;
	int start_y = (pRoiAttr->stRect.s32Y >> 4) << 4;
	int end_x = ((pRoiAttr->stRect.s32X + pRoiAttr->stRect.u32Width + 15) >> 4) << 4;
	int end_y = ((pRoiAttr->stRect.s32Y + pRoiAttr->stRect.u32Height + 15) >> 4) << 4;

	pRoiAttr->stRect.s32X = start_x;
	pRoiAttr->stRect.s32Y = start_y;
	pRoiAttr->stRect.u32Width = end_x - start_x;
	pRoiAttr->stRect.u32Height = end_y - start_y;
}
#define QP_MAP_SKIP_OFFSET      7
#define QP_MAP_BIT_MODE_OFFSET  6
#define QP_MAP_BIT_QP_OFFSET    0
static int packQpMapByte(int skip, int mode, int qp)
{
	int info = ((skip & 0x01) << QP_MAP_SKIP_OFFSET) |
				((mode & 0x01) << QP_MAP_BIT_MODE_OFFSET) |
				((qp   & 0x3f) << QP_MAP_BIT_QP_OFFSET);
	return info;
}

static void clearQpMapBoundarySkip(int frame_width, int frame_height, CVI_U8 *pu8QpMap)
{
	int x, y;
	int ctbHeight = 0, ctbStride = 0;

	ctbHeight = ((frame_width + 15) & ~15) >> 4;
	ctbStride= ((frame_height + 15) & ~15) >> 4;

	if ((frame_width & 0xf) != 0) {
		x = (frame_width >> 4);
		for (y = 0; y < ctbHeight; y++) {
			pu8QpMap[ctbStride * y + x] = (pu8QpMap[ctbStride * y + x] & 0x7);
		}
	}

	if ((frame_height & 0xf) != 0) {
		y = (frame_height >> 4);
		for (x = 0; x < ctbStride; x++) {
			pu8QpMap[ctbStride * y + x] = (pu8QpMap[ctbStride * y + x] & 0x7);
		}
	}
}



static CVI_U32 convert2HwFmt(CVI_U8 *pu8QpMap, CVI_U8 *tmp_map, CVI_U32 u32Width,
	CVI_U32 u32Height, PAYLOAD_TYPE_E enPayLoad) {
	CVI_U32 qpMapSize;
	CVI_U32 x, y, i;
	CVI_U32 cnt = 0;

	if (enPayLoad == PT_H265) {
		CVI_U32 ctbHeight64 = ((u32Height + 63) & ~63) >> 6;
		CVI_U32 ctbWidth64 = ((u32Width + 511) & ~511) >> 6;
		CVI_U32 ctuStride  = ctbWidth64;
		CVI_U32 cu16stride = ((u32Width + 15) & ~15) >> 4;
		qpMapSize = ctbHeight64 * ctbWidth64 * 16;

		for (y = 0; y < ctbHeight64; y++) {
			for (x = 0; x < ctbWidth64; x++) {
				CVI_U32 ctu64Addr = y * ctuStride + x;

				if ((x << 6) < u32Width) {
					for (i = 0; i < 16; i++) {
						CVI_U32 cu16X = i % 4;
						CVI_U32 cu16Y = i / 4;
						CVI_U32 cu16Addr = 0;

						cu16X += x * 4;
						cu16Y += y * 4;
						cu16Addr = cu16X + cu16Y * cu16stride;
						pu8QpMap[ctu64Addr * 16 + raster2zscan16[i]] = tmp_map[cu16Addr];
					}
				} else {
					memset(&pu8QpMap[cnt], 0, 16);
				}
				cnt += 16;
			}
		}
	} else {
		CVI_U32 ctbHeight16 = ((u32Height + 15) & ~15) >> 4;
		CVI_U32 ctbWd16A2048 = ((u32Width + 2047) & ~2047) >> 4;
		CVI_U32 ctbWd16A16 = ((u32Width + 15) & ~15) >> 4;
		qpMapSize = ctbHeight16 * ctbWd16A2048;

		for (y = 0; y < ctbHeight16; y++) {
			for (x = 0; x < ctbWd16A2048; x++) {
				if ((x << 4) < u32Width)
					pu8QpMap[y * ctbWd16A2048 + x] = tmp_map[y * ctbWd16A16 + x];
				else
					pu8QpMap[y * ctbWd16A2048 + x] = 0;
			}
		}
	}
	return qpMapSize;
}

CVI_S32 SAMPLE_COMM_VENC_SetQpMapByCfgFile(VENC_CHN VencChn,
		SAMPLE_COMM_VENC_ROI *vencRoi, CVI_U32 frameIdx,
		CVI_U8 *pu8QpMap, CVI_BOOL *pbQpMapValid,
		CVI_U32 u32Width, CVI_U32 u32Height, PAYLOAD_TYPE_E enPayLoad, CVI_U32 *u32QpMapSize)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_COMM_VENC_ROI *pVencRoi;
	VENC_ROI_ATTR_S *pRoiAttr;
	(void) VencChn;
	CVI_U32 s32Left, s32Right;
	CVI_U32 s32Top, s32Bottom;
	CVI_U32 i, row, col;
	CVI_U32 ctbHeight = 0, ctbStride = 0;
	CVI_U8 *tmp_map = NULL;


	ctbHeight = ((u32Height + 15) & ~15) >> 4;
	ctbStride= ((u32Width + 15) & ~15) >> 4;

	tmp_map = malloc(ctbHeight * ctbStride);
	if (!tmp_map) {
		SAMPLE_PRT("Failed to allocate memory for tmp_map\n");
		return CVI_FAILURE;
	}
	memset(tmp_map, 0, ctbHeight * ctbStride);

	*pbQpMapValid = CVI_FALSE;

	for (i = 0; i < MAX_NUM_ROI; ++i) {
		pVencRoi = &vencRoi[i];
		pRoiAttr = &pVencRoi->stVencRoiAttr;

		if (frameIdx >= pVencRoi->u32FrameStart &&
			frameIdx <= pVencRoi->u32FrameEnd) {
			pRoiAttr->bEnable = true;
			*pbQpMapValid = CVI_TRUE;
		} else {
			pRoiAttr->bEnable = false;
			continue;
		}

		alignQpRoi(pRoiAttr);
		s32Left = pRoiAttr->stRect.s32X >> 4;
		s32Right = (pRoiAttr->stRect.s32X + pRoiAttr->stRect.u32Width - 1) >> 4;
		if (s32Left > s32Right || s32Right >= ctbStride) {
			SAMPLE_PRT("s32Left = %d, s32Right = %d, ctbStride = %d\n",
					s32Left, s32Right, ctbStride);
			s32Ret = CVI_FAILURE;
			goto CLEANUP;
		}

		s32Top = pRoiAttr->stRect.s32Y >> 4;
		s32Bottom = (pRoiAttr->stRect.s32Y + pRoiAttr->stRect.u32Height - 1) >> 4;
		if (s32Top > s32Bottom || s32Bottom >= ctbHeight) {
			SAMPLE_PRT("s32Top = %d, s32Bottom = %d, ctbHeight = %d\n",
					s32Top, s32Bottom, ctbHeight);
			s32Ret = CVI_FAILURE;
			goto CLEANUP;
		}

		for (row = s32Top; row <= s32Bottom; row++) {
			for (col = s32Left; col <= s32Right; col++) {
				CVI_U8 map =  packQpMapByte(pRoiAttr->bSkip, pRoiAttr->bAbsQp, pRoiAttr->s32Qp);
				tmp_map[ctbStride * row + col] = map;
			}
		}
	}
	clearQpMapBoundarySkip(u32Width, u32Height, pu8QpMap);
	*u32QpMapSize = convert2HwFmt(pu8QpMap, tmp_map, u32Width, u32Height, enPayLoad);

CLEANUP:
	if (tmp_map)
		free(tmp_map);
#if DEBUG
	{
		for (row = 0; row < ctbHeight; row++) {
			for (col = 0; col < ctbStride; col++) {
				SAMPLE_PRT("%2x ", pu8QpMap[ctbStride * row + col]);
			}
			SAMPLE_PRT("\n");
		}
	}
#endif
	return s32Ret;
}

static void AssignRoiAttr(VENC_ROI_ATTR_S *dst, const VENC_ROI_ATTR_S *src) {
	dst->bEnable = src->bEnable;
	dst->bAbsQp = src->bAbsQp;
	dst->s32Qp = src->s32Qp;
	dst->stRect = src->stRect;
	dst->bSkip = src->bSkip;
}

CVI_S32 SAMPLE_COMM_VENC_SetRoiAttrByCfgFile(VENC_CHN VencChn, SAMPLE_COMM_VENC_ROI *vencRoi, CVI_U32 frameIdx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	for (CVI_U32 i = 0; i < MAX_NUM_ROI; ++i) {
		VENC_ROI_ATTR_S RoiAttr;
		SAMPLE_COMM_VENC_ROI *pVencRoi = &vencRoi[i];

		if (frameIdx >= pVencRoi->u32FrameStart && frameIdx <= pVencRoi->u32FrameEnd) {
			pVencRoi->stVencRoiAttr.bEnable = CVI_TRUE;
		} else {
			pVencRoi->stVencRoiAttr.bEnable = CVI_FALSE;
		}

		s32Ret = CVI_VENC_GetRoiAttr(VencChn, i, &RoiAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("GetRoiAttr failed for index %d\n", i);
			return CVI_FAILURE;
		}

		AssignRoiAttr(&RoiAttr, &pVencRoi->stVencRoiAttr);

		s32Ret = SetRoiAttr(VencChn, &RoiAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SetRoiAttr failed for index %d\n", i);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

static CVI_S32 ParseRoiConfigLine(const char *line, SAMPLE_COMM_VENC_ROI *roi) {
	return sscanf(line, "%u %d %d %d %d %u %u %u %u",
					&roi->stVencRoiAttr.u32Index,
					(int *)&roi->stVencRoiAttr.bAbsQp,
					&roi->stVencRoiAttr.s32Qp,
					&roi->stVencRoiAttr.stRect.s32X,
					&roi->stVencRoiAttr.stRect.s32Y,
					&roi->stVencRoiAttr.stRect.u32Width,
					&roi->stVencRoiAttr.stRect.u32Height,
					&roi->u32FrameStart,
					&roi->u32FrameEnd);
}

CVI_S32 SAMPLE_COMM_VENC_LoadRoiCfgFile(SAMPLE_COMM_VENC_ROI *vencRoi, CVI_CHAR *cfgFileName)
{
	FILE *cfgFile = fopen(cfgFileName, "r");
	char line[256];

	if (!cfgFile) {
		SAMPLE_PRT("Failed to open ROI config file: %s\n", cfgFileName);
		return CVI_FAILURE;
	}

	while (fgets(line, sizeof(line), cfgFile)) {
		SAMPLE_COMM_VENC_ROI roi;

		memset(&roi, 0, sizeof(SAMPLE_COMM_VENC_ROI));
		if (line[0] == '#' || line[0] == ';' || line[0] == ':') {
			continue;
		}

		if (ParseRoiConfigLine(line, &roi) != 9) {
			SAMPLE_PRT("Invalid ROI config line: %s\n", line);
			fclose(cfgFile);
			return CVI_FAILURE;
		}

		if (roi.stVencRoiAttr.u32Index >= MAX_NUM_ROI) {
			SAMPLE_PRT("Invalid ROI index: %u\n", roi.stVencRoiAttr.u32Index);
			fclose(cfgFile);
			return CVI_FAILURE;
		}

		memcpy(&vencRoi[roi.stVencRoiAttr.u32Index], &roi, sizeof(SAMPLE_COMM_VENC_ROI));
	}

	fclose(cfgFile);
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VENC_Create(
		chnInputCfg * pIc,
		VENC_CHN VencChn,
		PAYLOAD_TYPE_E enType,
		PIC_SIZE_E enSize,
		SAMPLE_RC_E enRcMode,
		CVI_U32 u32Profile,
		CVI_BOOL bRcnRefShareBuf,
		VENC_GOP_ATTR_S *pstGopAttr)
{
	CVI_S32 s32Ret;
	VENC_CHN_ATTR_S stVencChnAttr, *pstVencChnAttr = &stVencChnAttr;

	s32Ret = SAMPLE_COMM_VENC_SetChnAttr(
			pIc,
			pstVencChnAttr,
			enType,
			enSize,
			enRcMode,
			u32Profile,
			pstGopAttr,
			bRcnRefShareBuf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Get picture size failed!\n");
		return CVI_FAILURE;
	}

	if (pIc->bCreateChn == CVI_FALSE) {
		s32Ret = CVI_VENC_CreateChn(VencChn, pstVencChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_CreateChn [%d] failed with %d\n", VencChn, s32Ret);
			return s32Ret;
		}
		pIc->bCreateChn = CVI_TRUE;
	}

	if (enType != PT_JPEG) {
		s32Ret = SAMPLE_COMM_VENC_SetRcParam(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetRcParam, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetRefParam(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetRefParam, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetCuPrediction(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetCuPrediction, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetFrameLost(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetFrameLost, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetSuperFrame(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetSuperFrame, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetDebreathEffect(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetDebreathEffect, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
	}

	if (enType != PT_JPEG && enType != PT_MJPEG) {
		VB_SOURCE_E eVbSource;
		VENC_PARAM_MOD_S stModParam;

		if (enType == PT_H264)
			stModParam.enVencModType = MODTYPE_H264E;
		else if (enType == PT_H265)
			stModParam.enVencModType = MODTYPE_H265E;
		else {
			SAMPLE_PRT("Unsupport type[%d]\n", enType);
			eVbSource = VB_SOURCE_MODULE;
			return CVI_FAILURE;
		}

		s32Ret = CVI_VENC_GetModParam(&stModParam);
		if (enType == PT_H264)
			eVbSource = stModParam.stH264eModParam.enH264eVBSource;
		else if (enType == PT_H265)
			eVbSource = stModParam.stH265eModParam.enH265eVBSource;
		else {
			SAMPLE_PRT("Unknown type[%d]\n", enType);
			eVbSource = VB_SOURCE_MODULE;
		}
		//get_modparam  -> user mode
		SAMPLE_PRT("eVbSource[%d]\n", eVbSource);
		if (eVbSource == VB_SOURCE_USER) {
			//attachvbpool
			VENC_CHN_POOL_S stPool;

			stPool.hPicVbPool = gVencPicVbPool[VencChn];
			stPool.hPicInfoVbPool = gVencPicInfoVbPool[VencChn];
			SAMPLE_PRT("CVI_VENC_AttachVbPool chn[%d]\n",  VencChn);
			s32Ret = CVI_VENC_AttachVbPool(VencChn, &stPool);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VENC_AttachVbPool, %d\n", s32Ret);
				goto ERR_SAMPLE_COMM_VENC_CREATE;
			}
		}

		s32Ret = SAMPLE_COMM_VENC_EnableSvc(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_VENC_EnableSvc, %d\n", s32Ret);
				goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetSvcParam(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_VENC_SetSvcParam, %d\n", s32Ret);
				goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
	}

	if (enType == PT_H264) {
		VENC_H264_ENTROPY_S h264Entropy = { 0 };

		switch (pIc->h264EntropyMode) {
		case 0:
			h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CAVLC;
			h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CAVLC;
			break;
		case 1:
			h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CABAC;
			h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CABAC;
			break;
		default:
			h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CABAC;
			h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CABAC;
			break;
		}

		s32Ret = CVI_VENC_SetH264Entropy(VencChn, &h264Entropy);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_SetH264Entropy, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetH264Trans(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH264Trans, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetH264Vui(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH264Vui, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
		s32Ret = SAMPLE_COMM_VENC_SetH264Dblk(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH264Dblk, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetH264IntraPred(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH264IntraPred, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
	}

	if (enType == PT_H265) {
		s32Ret = SAMPLE_COMM_VENC_SetH265Trans(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH265Trans, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetH265Vui(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH265Vui, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}

		s32Ret = SAMPLE_COMM_VENC_SetH265Dblk(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH265Dblk, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
		s32Ret = SAMPLE_COMM_VENC_SetH265PredUnit(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetH265PredUnit, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
	}

	s32Ret = SAMPLE_COMM_VENC_SetChnParam(pIc, VencChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VENC_SetChnParam, %d\n", s32Ret);
		goto ERR_SAMPLE_COMM_VENC_CREATE;
	}

	if (enType == PT_JPEG) {
		s32Ret = SAMPLE_COMM_VENC_SetJpegParam(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetJpegParam, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
	}

	if (enType == PT_MJPEG) {
		s32Ret = SAMPLE_COMM_VENC_SetMjpegParam(pIc, VencChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetMjpegParam, %d\n", s32Ret);
			goto ERR_SAMPLE_COMM_VENC_CREATE;
		}
	}

	return s32Ret;
ERR_SAMPLE_COMM_VENC_CREATE:
	CVI_VENC_DestroyChn(VencChn);

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetDebreathEffect(
		chnInputCfg * pIc,
		VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	(void)pIc;
	VENC_DEBREATHEFFECT_S stDebreathEffect, *pstDebreathEffect = &stDebreathEffect;

	s32Ret = CVI_VENC_GetDebreathEffect(VencChn, pstDebreathEffect);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetFrameLostStrategy failed!\n");
		return CVI_FAILURE;
	}

	pstDebreathEffect->bEnable =  pIc->bDebreathEn;
	pstDebreathEffect->s32Strength0 = pIc->s32Strength0;
	pstDebreathEffect->s32Strength1 = pIc->s32Strength1;

	s32Ret = CVI_VENC_SetDebreathEffect(VencChn, pstDebreathEffect);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetDebreathEffect failed!\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetChnAttr(
		chnInputCfg * pIc,
		VENC_CHN_ATTR_S *pstVencChnAttr,
		PAYLOAD_TYPE_E enType,
		PIC_SIZE_E enSize,
		SAMPLE_RC_E enRcMode,
		CVI_U32 u32Profile,
		VENC_GOP_ATTR_S *pstGopAttr,
		CVI_BOOL bRcnRefShareBuf)
{
	SIZE_S stPicSize = {1920, 1080};
	CVI_U32 u32StatTime = 0;
	CVI_U32 u32Gop = 30;
	CVI_U32 u32FrameRate = pIc->framerate;
	CVI_U32 u32SrcFrameRate = pIc->srcFramerate;
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (enSize == PIC_CUSTOMIZE) {
		stPicSize.u32Width = pIc->width;
		stPicSize.u32Height = pIc->height;
	} /*else {
		s32Ret = SAMPLE_COMM_SYS_GetPicSize(enSize, &stPicSize);
	}*/

	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Get picture size failed!\n");
		return CVI_FAILURE;
	}

	memset(pstVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));

	pstVencChnAttr->stVencAttr.enType = enType;
	pstVencChnAttr->stVencAttr.u32MaxPicWidth = stPicSize.u32Width;
	pstVencChnAttr->stVencAttr.u32MaxPicHeight = stPicSize.u32Height;
	pstVencChnAttr->stVencAttr.u32PicWidth = stPicSize.u32Width;
	pstVencChnAttr->stVencAttr.u32PicHeight = stPicSize.u32Height;
	pstVencChnAttr->stVencAttr.u32BufSize = pIc->bitstreamBufSize;
	pstVencChnAttr->stVencAttr.bEsBufQueueEn = pIc->bEsBufQueueEn;
	pstVencChnAttr->stVencAttr.bIsoSendFrmEn = pIc->bIsoSendFrmEn;
	pstVencChnAttr->stVencAttr.u32Profile = u32Profile;
	pstVencChnAttr->stVencAttr.bByFrame = CVI_TRUE; // get stream mode is slice mode or
													// frame mode ?
	if (pstGopAttr->enGopMode == VENC_GOPMODE_NORMALP) {
		pstGopAttr->stNormalP.s32IPQpDelta = pIc->s32IPQpDelta;
		SAMPLE_PRT("s32IPQpDelta = %d\n", pstGopAttr->stNormalP.s32IPQpDelta);
		u32StatTime = pIc->statTime;
	} else if (pstGopAttr->enGopMode == VENC_GOPMODE_SMARTP) {
		pstGopAttr->stSmartP.u32BgInterval = pIc->bgInterval;
		u32StatTime = pstGopAttr->stSmartP.u32BgInterval / u32Gop;

		pstGopAttr->stSmartP.s32BgQpDelta = pIc->s32BgQpDelta;
		pstGopAttr->stSmartP.s32ViQpDelta = pIc->s32ViQpDelta;
		SAMPLE_PRT("s32BgQpDelta = %d\n", pstGopAttr->stSmartP.s32BgQpDelta);
		SAMPLE_PRT("s32ViQpDelta = %d\n", pstGopAttr->stSmartP.s32ViQpDelta);
	} else {
		u32StatTime = pIc->statTime;
	}

	switch (enType) {
	case PT_H265: {
		if (enRcMode == SAMPLE_RC_CBR) {
			VENC_H265_CBR_S *pstH265Cbr = &pstVencChnAttr->stRcAttr.stH265Cbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
			pstH265Cbr->u32Gop = pIc->gop;
			pstH265Cbr->u32StatTime = u32StatTime;
			pstH265Cbr->u32SrcFrameRate = u32SrcFrameRate;
			pstH265Cbr->fr32DstFrameRate = u32FrameRate;
			pstH265Cbr->bVariFpsEn = pIc->bVariFpsEn;
			pstH265Cbr->u32BitRate = pIc->bitrate;
			SAMPLE_PRT("u32BitRate = %d\n", pstH265Cbr->u32BitRate);
		} else if (enRcMode == SAMPLE_RC_FIXQP) {
			VENC_H265_FIXQP_S *pstH265FixQp = &pstVencChnAttr->stRcAttr.stH265FixQp;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
			pstH265FixQp->u32Gop = pIc->gop;
			pstH265FixQp->u32SrcFrameRate = u32SrcFrameRate;
			pstH265FixQp->fr32DstFrameRate = u32FrameRate;
			pstH265FixQp->bVariFpsEn = pIc->bVariFpsEn;
			pstH265FixQp->u32IQp = pIc->iqp;
			pstH265FixQp->u32PQp = pIc->pqp;
			SAMPLE_PRT("u32Gop = %d, u32IQp = %d, u32PQp = %d\n",
					pstH265FixQp->u32Gop,
					pstH265FixQp->u32IQp,
					pstH265FixQp->u32PQp);
		} else if (enRcMode == SAMPLE_RC_VBR) {
			VENC_H265_VBR_S *pstH265Vbr = &pstVencChnAttr->stRcAttr.stH265Vbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
			pstH265Vbr->u32Gop = pIc->gop;
			pstH265Vbr->u32StatTime = u32StatTime;
			pstH265Vbr->u32SrcFrameRate = u32SrcFrameRate;
			pstH265Vbr->fr32DstFrameRate = u32FrameRate;
			pstH265Vbr->bVariFpsEn = pIc->bVariFpsEn;
			pstH265Vbr->u32MaxBitRate = pIc->maxbitrate;
			SAMPLE_PRT("u32StatTime = %d, u32Gop = %d, u32MaxBitRate = %d\n",
					pstH265Vbr->u32StatTime,
					pstH265Vbr->u32Gop,
					pstH265Vbr->u32MaxBitRate);
		} else if (enRcMode == SAMPLE_RC_AVBR) {
			VENC_H265_AVBR_S *pstH265AVbr = &pstVencChnAttr->stRcAttr.stH265AVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265AVBR;
			pstH265AVbr->u32Gop = pIc->gop;
			pstH265AVbr->u32StatTime = u32StatTime;
			pstH265AVbr->u32SrcFrameRate = u32SrcFrameRate;
			pstH265AVbr->fr32DstFrameRate = u32FrameRate;
			pstH265AVbr->bVariFpsEn = pIc->bVariFpsEn;
			pstH265AVbr->u32MaxBitRate = pIc->maxbitrate;
			SAMPLE_PRT("u32StatTime = %d, u32Gop = %d, u32MaxBitRate = %d\n",
					pstH265AVbr->u32StatTime,
					pstH265AVbr->u32Gop,
					pstH265AVbr->u32MaxBitRate);
		} else if (enRcMode == SAMPLE_RC_QVBR) {
			VENC_H265_QVBR_S stH265QVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265QVBR;
			stH265QVbr.u32Gop = u32Gop;
			stH265QVbr.u32StatTime = u32StatTime;
			stH265QVbr.u32SrcFrameRate = u32SrcFrameRate;
			stH265QVbr.fr32DstFrameRate = u32FrameRate;

			switch (enSize) {
			case PIC_720P:
				stH265QVbr.u32TargetBitRate = 1024 * 2 + 1024 * u32FrameRate / 30;
				break;
			case PIC_1080P:
				stH265QVbr.u32TargetBitRate = 1024 * 2 + 2048 * u32FrameRate / 30;
				break;
			case PIC_2592x1944:
				stH265QVbr.u32TargetBitRate = 1024 * 3 + 3072 * u32FrameRate / 30;
				break;
			case PIC_3840x2160:
				stH265QVbr.u32TargetBitRate = 1024 * 5 + 5120 * u32FrameRate / 30;
				break;
			case PIC_4000x3000:
				stH265QVbr.u32TargetBitRate = 1024 * 10 + 5120 * u32FrameRate / 30;
				break;
			default:
				stH265QVbr.u32TargetBitRate = 1024 * 15 + 2048 * u32FrameRate / 30;
				break;
			}
			memcpy(&pstVencChnAttr->stRcAttr.stH265QVbr, &stH265QVbr, sizeof(VENC_H265_QVBR_S));
		} else if (enRcMode == SAMPLE_RC_QPMAP) {
			VENC_H265_QPMAP_S *pstH265QpMap = &pstVencChnAttr->stRcAttr.stH265QpMap;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265QPMAP;
			pstH265QpMap->u32Gop = pIc->gop;
			pstH265QpMap->u32StatTime = u32StatTime;
			pstH265QpMap->u32SrcFrameRate = u32SrcFrameRate;
			pstH265QpMap->fr32DstFrameRate = u32FrameRate;
			pstH265QpMap->bVariFpsEn = pIc->bVariFpsEn;
			pstH265QpMap->enQpMapMode = VENC_RC_QPMAP_MODE_MEANQP;
			SAMPLE_PRT("u32StatTime = %d, u32Gop = %d\n",
					pstH265QpMap->u32StatTime,
					pstH265QpMap->u32Gop);
		} else if (enRcMode == SAMPLE_RC_UBR) {
			VENC_H265_UBR_S *pstH265Ubr = &pstVencChnAttr->stRcAttr.stH265Ubr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265UBR;
			pstH265Ubr->u32Gop = pIc->gop;
			pstH265Ubr->u32StatTime = u32StatTime;
			pstH265Ubr->u32SrcFrameRate = u32SrcFrameRate;
			pstH265Ubr->fr32DstFrameRate = u32FrameRate;
			pstH265Ubr->bVariFpsEn = pIc->bVariFpsEn;
			pstH265Ubr->u32BitRate = pIc->bitrate;
			SAMPLE_PRT("u32BitRate = %d\n", pstH265Ubr->u32BitRate);
		} else {
			SAMPLE_PRT("enRcMode(%d) not support\n", enRcMode);
			return CVI_FAILURE;
		}
		pstVencChnAttr->stVencAttr.stAttrH265e.bRcnRefShareBuf = bRcnRefShareBuf;
	} break;
	case PT_H264: {
		if (enRcMode == SAMPLE_RC_CBR) {
			VENC_H264_CBR_S *pstH264Cbr = &pstVencChnAttr->stRcAttr.stH264Cbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
			pstH264Cbr->u32Gop = pIc->gop;
			pstH264Cbr->u32StatTime = u32StatTime;
			pstH264Cbr->u32SrcFrameRate = u32SrcFrameRate;
			pstH264Cbr->fr32DstFrameRate = u32FrameRate;
			pstH264Cbr->bVariFpsEn = pIc->bVariFpsEn;
			pstH264Cbr->u32BitRate = pIc->bitrate;
			SAMPLE_PRT("bitrate = %d, u32BitRate = %d\n",
					pIc->bitrate,
					pstH264Cbr->u32BitRate);
		} else if (enRcMode == SAMPLE_RC_FIXQP) {
			VENC_H264_FIXQP_S *pstH264FixQp = &pstVencChnAttr->stRcAttr.stH264FixQp;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
			pstH264FixQp->u32Gop = pIc->gop;
			pstH264FixQp->u32SrcFrameRate = u32SrcFrameRate;
			pstH264FixQp->fr32DstFrameRate = u32FrameRate;
			pstH264FixQp->bVariFpsEn = pIc->bVariFpsEn;
			pstH264FixQp->u32IQp = pIc->iqp;
			pstH264FixQp->u32PQp = pIc->pqp;
			SAMPLE_PRT("u32Gop = %d, u32IQp = %d, u32PQp = %d\n",
					pstH264FixQp->u32Gop,
					pstH264FixQp->u32IQp,
					pstH264FixQp->u32PQp);
		} else if (enRcMode == SAMPLE_RC_VBR) {
			VENC_H264_VBR_S *pstH264Vbr = &pstVencChnAttr->stRcAttr.stH264Vbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
			pstH264Vbr->u32Gop = pIc->gop;
			pstH264Vbr->u32StatTime = u32StatTime;
			pstH264Vbr->u32SrcFrameRate = u32SrcFrameRate;
			pstH264Vbr->fr32DstFrameRate = u32FrameRate;
			pstH264Vbr->bVariFpsEn = pIc->bVariFpsEn;
			pstH264Vbr->u32MaxBitRate = pIc->maxbitrate;
			SAMPLE_PRT("u32StatTime = %d, u32Gop = %d, u32MaxBitRate = %d\n",
					pstH264Vbr->u32StatTime,
					pstH264Vbr->u32Gop,
					pstH264Vbr->u32MaxBitRate);
		} else if (enRcMode == SAMPLE_RC_AVBR) {
			VENC_H264_AVBR_S *pstH264AVbr = &pstVencChnAttr->stRcAttr.stH264AVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;
			pstH264AVbr->u32Gop = pIc->gop;
			pstH264AVbr->u32StatTime = u32StatTime;
			pstH264AVbr->u32SrcFrameRate = u32SrcFrameRate;
			pstH264AVbr->fr32DstFrameRate = u32FrameRate;
			pstH264AVbr->bVariFpsEn = pIc->bVariFpsEn;
			pstH264AVbr->u32MaxBitRate = pIc->maxbitrate;
			SAMPLE_PRT("u32StatTime = %d, u32Gop = %d, u32MaxBitRate = %d\n",
					pstH264AVbr->u32StatTime,
					pstH264AVbr->u32Gop,
					pstH264AVbr->u32MaxBitRate);
		} else if (enRcMode == SAMPLE_RC_QVBR) {
			VENC_H264_QVBR_S stH264QVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264QVBR;
			stH264QVbr.u32Gop = u32Gop;
			stH264QVbr.u32StatTime = u32StatTime;
			stH264QVbr.u32SrcFrameRate = u32SrcFrameRate;
			stH264QVbr.fr32DstFrameRate = u32FrameRate;
			switch (enSize) {
			case PIC_720P:
				stH264QVbr.u32TargetBitRate = 1024 * 2 + 1024 * u32FrameRate / 30;
				break;
			case PIC_1080P:
				stH264QVbr.u32TargetBitRate = 1024 * 2 + 2048 * u32FrameRate / 30;
				break;
			case PIC_2592x1944:
				stH264QVbr.u32TargetBitRate = 1024 * 3 + 3072 * u32FrameRate / 30;
				break;
			case PIC_3840x2160:
				stH264QVbr.u32TargetBitRate = 1024 * 5 + 5120 * u32FrameRate / 30;
				break;
			case PIC_4000x3000:
				stH264QVbr.u32TargetBitRate = 1024 * 10 + 5120 * u32FrameRate / 30;
				break;
			default:
				stH264QVbr.u32TargetBitRate = 1024 * 15 + 2048 * u32FrameRate / 30;
				break;
			}
			memcpy(&pstVencChnAttr->stRcAttr.stH264QVbr, &stH264QVbr, sizeof(VENC_H264_QVBR_S));
		} else if (enRcMode == SAMPLE_RC_UBR) {
			VENC_H264_UBR_S *pstH264Ubr = &pstVencChnAttr->stRcAttr.stH264Ubr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264UBR;
			pstH264Ubr->u32Gop = pIc->gop;
			pstH264Ubr->u32StatTime = u32StatTime;
			pstH264Ubr->u32SrcFrameRate = u32SrcFrameRate;
			pstH264Ubr->fr32DstFrameRate = u32FrameRate;
			pstH264Ubr->bVariFpsEn = pIc->bVariFpsEn;
			pstH264Ubr->u32BitRate = pIc->bitrate;
			SAMPLE_PRT("u32BitRate = %d\n", pstH264Ubr->u32BitRate);
		} else {
			SAMPLE_PRT("H.264 enRcMode(%d) not support\n", enRcMode);
			return CVI_FAILURE;
		}
		pstVencChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf = bRcnRefShareBuf;
		pstVencChnAttr->stVencAttr.stAttrH264e.bSingleLumaBuf = 0;
	} break;
	case PT_MJPEG: {
		if (enRcMode == SAMPLE_RC_FIXQP) {
			VENC_MJPEG_FIXQP_S *pstMjpegeFixQp = &pstVencChnAttr->stRcAttr.stMjpegFixQp;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;

			// 0 use old q-table for forward compatible.
			pstMjpegeFixQp->u32Qfactor = (pIc->quality > 0) ? pIc->quality : 0;
			pstMjpegeFixQp->u32SrcFrameRate = u32SrcFrameRate;
			pstMjpegeFixQp->fr32DstFrameRate = u32FrameRate;
			pstMjpegeFixQp->bVariFpsEn = pIc->bVariFpsEn;
		} else if (enRcMode == SAMPLE_RC_CBR) {
			VENC_MJPEG_CBR_S *pstMjpegeCbr = &pstVencChnAttr->stRcAttr.stMjpegCbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
			pstMjpegeCbr->u32StatTime = u32StatTime;
			pstMjpegeCbr->u32SrcFrameRate = u32SrcFrameRate;
			pstMjpegeCbr->fr32DstFrameRate = u32FrameRate;
			pstMjpegeCbr->bVariFpsEn = pIc->bVariFpsEn;
			pstMjpegeCbr->u32BitRate = pIc->bitrate;
		} else if ((enRcMode == SAMPLE_RC_VBR) || (enRcMode == SAMPLE_RC_AVBR) ||
			   (enRcMode == SAMPLE_RC_QVBR)) {
			VENC_MJPEG_VBR_S stMjpegVbr;

			if (enRcMode == SAMPLE_RC_AVBR)
				SAMPLE_PRT("Mjpege not support AVBR, so change rcmode to VBR!\n");

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
			stMjpegVbr.u32StatTime = u32StatTime;
			stMjpegVbr.u32SrcFrameRate = u32SrcFrameRate;
			stMjpegVbr.fr32DstFrameRate = 5;

			switch (enSize) {
			case PIC_720P:
				stMjpegVbr.u32MaxBitRate = 1024 * 5 + 1024 * u32FrameRate / 30;
				break;
			case PIC_1080P:
				stMjpegVbr.u32MaxBitRate = 1024 * 8 + 2048 * u32FrameRate / 30;
				break;
			case PIC_2592x1944:
				stMjpegVbr.u32MaxBitRate = 1024 * 20 + 3072 * u32FrameRate / 30;
				break;
			case PIC_3840x2160:
				stMjpegVbr.u32MaxBitRate = 1024 * 25 + 5120 * u32FrameRate / 30;
				break;
			case PIC_4000x3000:
				stMjpegVbr.u32MaxBitRate = 1024 * 30 + 5120 * u32FrameRate / 30;
				break;
			default:
				stMjpegVbr.u32MaxBitRate = 1024 * 20 + 2048 * u32FrameRate / 30;
				break;
			}

			memcpy(&pstVencChnAttr->stRcAttr.stMjpegVbr, &stMjpegVbr, sizeof(VENC_MJPEG_VBR_S));
		} else {
			SAMPLE_PRT("cann't support other mode(%d) in this version!\n", enRcMode);
			return CVI_FAILURE;
		}
	} break;

	case PT_JPEG: {
		VENC_ATTR_JPEG_S *pstJpegAttr = &pstVencChnAttr->stVencAttr.stAttrJpege;

		pstJpegAttr->bSupportDCF = CVI_FALSE;
		pstJpegAttr->stMPFCfg.u8LargeThumbNailNum = 0;
		pstJpegAttr->enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
	} break;

	default:
		SAMPLE_PRT("cann't support this enType (%d) in this version!\n", enType);
		return CVI_ERR_VENC_NOT_SUPPORT;
	}

	if (PT_MJPEG == enType || PT_JPEG == enType) {
		pstVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
		pstVencChnAttr->stGopAttr.stNormalP.s32IPQpDelta = 0;
	} else {
		memcpy(&pstVencChnAttr->stGopAttr, pstGopAttr, sizeof(VENC_GOP_ATTR_S));

		if ((pstGopAttr->enGopMode == VENC_GOPMODE_BIPREDB) && (enType == PT_H264)) {
			if (pstVencChnAttr->stVencAttr.u32Profile == 0) {
				pstVencChnAttr->stVencAttr.u32Profile = 1;

				SAMPLE_PRT("H.264 base not support BIPREDB, change to main\n");
			}
		}

		if ((pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264QPMAP) ||
			(pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QPMAP)) {
			if (pstGopAttr->enGopMode == VENC_GOPMODE_ADVSMARTP) {
				pstVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;

				SAMPLE_PRT("advsmartp not support QPMAP, so change gopmode to smartp!\n");
			}
		}
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetRefParam(
		chnInputCfg * pIc,
		VENC_CHN VencChn)
{
	CVI_S32 s32Ret;
	VENC_REF_PARAM_S stRefParam, *pstRefParam = &stRefParam;

	s32Ret = CVI_VENC_GetRefParam(VencChn, pstRefParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetRefParam failed!\n");
		return CVI_FAILURE;
	}

	if (pIc->tempLayer == 2) {
		pstRefParam->u32Base = 1;
		pstRefParam->u32Enhance = 1;
		pstRefParam->bEnablePred = CVI_TRUE;
	} else if (pIc->tempLayer == 3) {
		pstRefParam->u32Base = 2;
		pstRefParam->u32Enhance = 1;
		pstRefParam->bEnablePred = CVI_TRUE;
	} else {
		pstRefParam->u32Base = 0;
		pstRefParam->u32Enhance = 0;
		pstRefParam->bEnablePred = CVI_TRUE;
	}

	s32Ret = CVI_VENC_SetRefParam(VencChn, pstRefParam);

	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetRefParam failed!\n");
		return CVI_FAILURE;
	}
	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetCuPrediction(
		chnInputCfg * pIc,
		VENC_CHN VencChn)
{
	CVI_S32 s32Ret;
	VENC_CU_PREDICTION_S stCuPrediction, *pstCuPrediction = &stCuPrediction;

	s32Ret = CVI_VENC_GetCuPrediction(VencChn, pstCuPrediction);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetCuPrediction, 0x%X\n", s32Ret);
		return s32Ret;
	}

	pstCuPrediction->u32IntraCost = pIc->u32IntraCost;

	s32Ret = CVI_VENC_SetCuPrediction(VencChn, pstCuPrediction);

	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetCuPrediction, 0x%X\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetRcParam(
		chnInputCfg * pIc,
		VENC_CHN VencChn)
{
	CVI_S32 s32Ret;
	VENC_RC_PARAM_S stRcParam, *pstRcParam = &stRcParam;

	s32Ret = CVI_VENC_GetRcParam(VencChn, pstRcParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetRcParam, 0x%X\n", s32Ret);
		return s32Ret;
	}

	pstRcParam->u32ThrdLv = pIc->u32ThrdLv;
	pstRcParam->bBgEnhanceEn = pIc->bBgEnhanceEn;
	pstRcParam->s32BgDeltaQp = pIc->s32BgDeltaQp;
	pstRcParam->u32RowQpDelta = pIc->u32RowQpDelta;
	pstRcParam->s32FirstFrameStartQp = pIc->firstFrmstartQp;
	pstRcParam->s32InitialDelay = pIc->initialDelay;

	if (!strcmp(pIc->codec, "264") && pIc->rcMode == SAMPLE_RC_CBR) {
		pstRcParam->stParamH264Cbr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH264Cbr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH264Cbr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH264Cbr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH264Cbr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH264Cbr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH264Cbr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	} else if (!strcmp(pIc->codec, "265") && pIc->rcMode == SAMPLE_RC_CBR) {
		pstRcParam->stParamH265Cbr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH265Cbr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH265Cbr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH265Cbr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH265Cbr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH265Cbr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH265Cbr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	}  else if (!strcmp(pIc->codec, "264") && pIc->rcMode == SAMPLE_RC_VBR) {
		pstRcParam->stParamH264Vbr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH264Vbr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH264Vbr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH264Vbr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH264Vbr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH264Vbr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH264Vbr.s32ChangePos = pIc->s32ChangePos;
		pstRcParam->stParamH264Vbr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	}  else if (!strcmp(pIc->codec, "265") && pIc->rcMode == SAMPLE_RC_VBR) {
		pstRcParam->stParamH265Vbr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH265Vbr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH265Vbr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH265Vbr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH265Vbr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH265Vbr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH265Vbr.s32ChangePos = pIc->s32ChangePos;
		pstRcParam->stParamH265Vbr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	}  else if (!strcmp(pIc->codec, "264") && pIc->rcMode == SAMPLE_RC_AVBR) {
		pstRcParam->stParamH264AVbr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH264AVbr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH264AVbr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH264AVbr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH264AVbr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH264AVbr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH264AVbr.s32ChangePos = pIc->s32ChangePos;
		pstRcParam->stParamH264AVbr.s32MinStillPercent = pIc->s32MinStillPercent;
		pstRcParam->stParamH264AVbr.u32MaxStillQP = pIc->u32MaxStillQP;
		pstRcParam->stParamH264AVbr.u32MotionSensitivity = pIc->u32MotionSensitivity;
		pstRcParam->stParamH264AVbr.s32AvbrFrmLostOpen = pIc->s32AvbrFrmLostOpen;
		pstRcParam->stParamH264AVbr.s32AvbrFrmGap = pIc->s32AvbrFrmGap;
		pstRcParam->stParamH264AVbr.s32AvbrPureStillThr = pIc->s32AvbrPureStillThr;
		pstRcParam->stParamH264AVbr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	}  else if (!strcmp(pIc->codec, "265") && pIc->rcMode == SAMPLE_RC_AVBR) {
		pstRcParam->stParamH265AVbr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH265AVbr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH265AVbr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH265AVbr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH265AVbr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH265AVbr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH265AVbr.s32ChangePos = pIc->s32ChangePos;
		pstRcParam->stParamH265AVbr.s32MinStillPercent = pIc->s32MinStillPercent;
		pstRcParam->stParamH265AVbr.u32MaxStillQP = pIc->u32MaxStillQP;
		pstRcParam->stParamH265AVbr.u32MotionSensitivity = pIc->u32MotionSensitivity;
		pstRcParam->stParamH265AVbr.s32AvbrFrmLostOpen = pIc->s32AvbrFrmLostOpen;
		pstRcParam->stParamH265AVbr.s32AvbrFrmGap = pIc->s32AvbrFrmGap;
		pstRcParam->stParamH265AVbr.s32AvbrPureStillThr = pIc->s32AvbrPureStillThr;
		pstRcParam->stParamH265AVbr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	} else if (!strcmp(pIc->codec, "264") && pIc->rcMode == SAMPLE_RC_UBR) {
		pstRcParam->stParamH264Ubr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH264Ubr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH264Ubr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH264Ubr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH264Ubr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH264Ubr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH264Ubr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	} else if (!strcmp(pIc->codec, "265") && pIc->rcMode == SAMPLE_RC_UBR) {
		pstRcParam->stParamH265Ubr.u32MaxIprop = pIc->maxIprop;
		pstRcParam->stParamH265Ubr.u32MinIprop = pIc->minIprop;
		pstRcParam->stParamH265Ubr.u32MaxIQp = pIc->maxIqp;
		pstRcParam->stParamH265Ubr.u32MinIQp = pIc->minIqp;
		pstRcParam->stParamH265Ubr.u32MaxQp = pIc->maxQp;
		pstRcParam->stParamH265Ubr.u32MinQp = pIc->minQp;
		pstRcParam->stParamH265Ubr.s32MaxReEncodeTimes = pIc->s32MaxReEncodeTimes;
	}

	s32Ret = CVI_VENC_SetRcParam(VencChn, pstRcParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetRcParam, 0x%X\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetFrameLost(
	chnInputCfg * pIc,
	VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_FRAMELOST_S stFL, *pstFL = &stFL;

	s32Ret = CVI_VENC_GetFrameLostStrategy(VencChn, pstFL);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetFrameLostStrategy failed!\n");
		return CVI_FAILURE;
	}

	pstFL->bFrmLostOpen = (pIc->frameLost) == 1 ? CVI_TRUE : CVI_FALSE;
	pstFL->enFrmLostMode = FRMLOST_PSKIP;
	pstFL->u32EncFrmGaps = pIc->frameLostGap;
	pstFL->u32FrmLostBpsThr = pIc->frameLostBspThr;

	s32Ret = CVI_VENC_SetFrameLostStrategy(VencChn, pstFL);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetFrameLostStrategy failed!\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetSuperFrame(
	chnInputCfg * pIc,
	VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_SUPERFRAME_CFG_S stsf, *pstsf = &stsf;

	s32Ret = CVI_VENC_GetSuperFrameStrategy(VencChn, pstsf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetSuperFrameStrategy failed!\n");
		return CVI_FAILURE;
	}

	pstsf->enSuperFrmMode = (pIc->enSuperFrmMode)
		? SUPERFRM_REENCODE_IDR
		: SUPERFRM_NONE;
	pstsf->u32SuperIFrmBitsThr = pIc->u32SuperIFrmBitsThr;
	pstsf->u32SuperPFrmBitsThr = pIc->u32SuperPFrmBitsThr;

	s32Ret = CVI_VENC_SetSuperFrameStrategy(VencChn, pstsf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetSuperFrameStrategy failed!\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetModParam(const commonInputCfg *pCic)
{
	CVI_S32 s32Ret;
	VB_SOURCE_E eVbSource = pCic->vbMode;

	switch (eVbSource) {
	case VB_SOURCE_COMMON:
		SAMPLE_PRT("vbMode VB_SOURCE_COMMON\n");
		break;
	case VB_SOURCE_MODULE:
		SAMPLE_PRT("vbMode VB_SOURCE_MODULE\n");
		break;
	case VB_SOURCE_PRIVATE:
		SAMPLE_PRT("vbMode VB_SOURCE_PRIVATE\n");
		break;
	case VB_SOURCE_USER:
		SAMPLE_PRT("vbMode VB_SOURCE_USER\n");
		break;
	default:
		SAMPLE_PRT("Invalid VB mode %d. Force to Priavte Mode.\n", eVbSource);
		eVbSource = VB_SOURCE_PRIVATE;
		SAMPLE_PRT("vbMode VB_SOURCE_PRIVATE\n");
		break;
	}

	//vb buffer mode only support non jpeg mode
	for (VENC_MODTYPE_E modtype = MODTYPE_H264E; modtype <= MODTYPE_JPEGE; modtype++) {
		VENC_PARAM_MOD_S stModParam;

		stModParam.enVencModType = modtype;
		s32Ret = CVI_VENC_GetModParam(&stModParam);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_GetModParam type %d failure\n", modtype);
			return CVI_FAILURE;
		}

		switch (modtype) {
		case MODTYPE_H264E:
			stModParam.stH264eModParam.enH264eVBSource = eVbSource;
			stModParam.stH264eModParam.u32UserDataMaxLen = 3072;
			stModParam.stH264eModParam.bSingleEsBuf = false;
			stModParam.stH264eModParam.u32SingleEsBufSize = 0;
			break;
		case MODTYPE_H265E:
			stModParam.stH265eModParam.enH265eVBSource = eVbSource;
			stModParam.stH265eModParam.u32UserDataMaxLen = 3072;
			stModParam.stH265eModParam.bSingleEsBuf = false;
			stModParam.stH265eModParam.u32SingleEsBufSize = 0;
			stModParam.stH265eModParam.enRefreshType = pCic->h265RefreshType;
			break;
		case MODTYPE_JPEGE:
			stModParam.stJpegeModParam.bSingleEsBuf = false;
			stModParam.stJpegeModParam.u32SingleEsBufSize = 0;
			switch (pCic->jpegMarkerOrder) {
			case 2:
				stModParam.stJpegeModParam.enJpegeFormat = JPEGE_FORMAT_CUSTOM;
				stModParam.stJpegeModParam.JpegMarkerOrder[0] = JPEGE_MARKER_SOI;
				stModParam.stJpegeModParam.JpegMarkerOrder[1] = JPEGE_MARKER_JFIF;
				stModParam.stJpegeModParam.JpegMarkerOrder[2] = JPEGE_MARKER_FRAME_INDEX;
				stModParam.stJpegeModParam.JpegMarkerOrder[3] = JPEGE_MARKER_USER_DATA;
				stModParam.stJpegeModParam.JpegMarkerOrder[4] = JPEGE_MARKER_DRI_OPT;
				stModParam.stJpegeModParam.JpegMarkerOrder[5] = JPEGE_MARKER_DQT;
				stModParam.stJpegeModParam.JpegMarkerOrder[6] = JPEGE_MARKER_DHT;
				stModParam.stJpegeModParam.JpegMarkerOrder[7] = JPEGE_MARKER_SOF0;
				stModParam.stJpegeModParam.JpegMarkerOrder[8] = JPEGE_MARKER_BUTT;
				break;
			case 1:
				stModParam.stJpegeModParam.enJpegeFormat = JPEGE_FORMAT_TYPE_1;
				break;
			case 0:
			default:
				stModParam.stJpegeModParam.enJpegeFormat = JPEGE_FORMAT_DEFAULT;
				break;
			}
			break;
		default:
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetModParam invalid type %d failure\n", modtype);
			return CVI_FAILURE;
		}
		s32Ret = CVI_VENC_SetModParam(&stModParam);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_SetModParam type %d failure\n", modtype);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}


static CVI_S32 SAMPLE_COMM_VENC_LoadJpegQTable(CVI_U32 *pu32QTable, CVI_CHAR *cfgFileName)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *cfgFile = NULL;
	char line[256] = { 0 };
	CVI_U32 i = 0;

	cfgFile = fopen(cfgFileName, "r");
	if (cfgFile == NULL) {
		SAMPLE_PRT("Missing Jpeg Quality Table config file, %s\n", cfgFileName);
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	if (!pu32QTable) {
		SAMPLE_PRT("NULL Pointer \n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	while (fgets(line, 256, cfgFile) != NULL) {
		if ((line[0] == '#') || (line[0] == ';') || (line[0] == ':'))
			continue;

		if (sscanf(line, "%d , %d , %d , %d , %d , %d , %d , %d ,",
				&pu32QTable[i],
				&pu32QTable[i+1],
				&pu32QTable[i+2],
				&pu32QTable[i+3],
				&pu32QTable[i+4],
				&pu32QTable[i+5],
				&pu32QTable[i+6],
				&pu32QTable[i+7]
				) == 0) {
			SAMPLE_PRT("Failed to parse jpeg table files\n");
			s32Ret = -1;
			break;
		}

		i += 8;
		// only u8YQt, u8CbQt
		if (i >= 128)
			break;
	}
	fclose(cfgFile);

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetJpegParam(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_JPEG_PARAM_S stJpegParam, *pstJpegParam = &stJpegParam;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 i;
	CVI_U32 u32QTable[192] = {0};

	s32Ret = CVI_VENC_GetJpegParam(VencChn, pstJpegParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetJpegParam\n");
		return CVI_FAILURE;
	}

	if (strlen(pIc->jpegQTableCfgFile)) {
		s32Ret = SAMPLE_COMM_VENC_LoadJpegQTable(u32QTable, pIc->jpegQTableCfgFile);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("LoadJpegQTable fail\n");
			return CVI_FAILURE;
		}
		// use q-table must set 50
		stJpegParam.u32Qfactor = 50;
		for (i = 0; i < 64;i ++) {
			pstJpegParam->u8YQt[i] = (CVI_U8) (u32QTable[i] & 0xFF);
			pstJpegParam->u8CbQt[i] = (CVI_U8) (u32QTable[i+64] & 0xFF);
		}
	}
	else {
		pstJpegParam->u32Qfactor = pIc->quality;
	}

	pstJpegParam->u32MCUPerECS = pIc->MCUPerECS;
	s32Ret = CVI_VENC_SetJpegParam(VencChn, pstJpegParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetJpegParam fail\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetMjpegParam(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_MJPEG_PARAM_S stMjpegParam, *pstMjpegParam = &stMjpegParam;
	VENC_CHN_ATTR_S stChnAttr;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32QTable[192] = {0};
	CVI_U32 i;

	s32Ret = CVI_VENC_GetMjpegParam(VencChn, pstMjpegParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetJpegParam\n");
		return CVI_FAILURE;
	}

	if (strlen(pIc->jpegQTableCfgFile)) {
		s32Ret = CVI_VENC_GetChnAttr(VencChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("get venc attr fail, venc:%d ret:%d\n", VencChn, s32Ret);
			return CVI_FAILURE;
		}

		if (stChnAttr.stRcAttr.enRcMode != VENC_RC_MODE_MJPEGFIXQP) {
			SAMPLE_PRT("venc:%d rcMode:%d not MJPEGFIXQP \n", VencChn, stChnAttr.stRcAttr.enRcMode);
			return CVI_FAILURE;
		}

		stChnAttr.stRcAttr.stMjpegFixQp.u32Qfactor = 50;
		s32Ret = CVI_VENC_SetChnAttr(VencChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("set venc attr fail, venc:%d ret:%d\n", VencChn, s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = SAMPLE_COMM_VENC_LoadJpegQTable(u32QTable, pIc->jpegQTableCfgFile);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("LoadJpegQTable fail\n");
			return CVI_FAILURE;
		}

		for (i = 0; i < 64;i ++) {
			pstMjpegParam->u8YQt[i] = (CVI_U8) (u32QTable[i] & 0xFF);
			pstMjpegParam->u8CbQt[i] = (CVI_U8) (u32QTable[i+64] & 0xFF);
		}
	}

	pstMjpegParam->u32MCUPerECS = pIc->MCUPerECS;

	s32Ret = CVI_VENC_SetMjpegParam(VencChn, pstMjpegParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetMjpegParam fail\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_InitVBPool(vencChnCtx *pvecc, VENC_CHN VencChnIdx)
{

	VB_POOL_CONFIG_S stVbPoolCfg;
	CVI_U32 u32BlkSize;
	SIZE_S stSize;

	if (pvecc->stSize.u32Width || pvecc->stSize.u32Height) {
		stSize.u32Width = pvecc->stSize.u32Width;
		stSize.u32Height = pvecc->stSize.u32Height;
	} else {
		SAMPLE_PRT("Invalid width or height[%d][%d]\n",
										pvecc->stSize.u32Width,
										pvecc->stSize.u32Height);
		return CVI_FAILURE;
	}

	//create venc own vbpool
	memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
	u32BlkSize = COMMON_GetVencFrameBufferSize(pvecc->enPayLoad,
					stSize.u32Width,
					stSize.u32Height);
	if (u32BlkSize == 0) {
		SAMPLE_PRT("Invalid type for VENC to calculate frame buffer size\n");
		return CVI_FAILURE;
	}

	stVbPoolCfg.u32BlkSize	= ALIGN(u32BlkSize, 4096);
	stVbPoolCfg.u32BlkCnt	= 3;
	stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;
	gVencPicVbPool[VencChnIdx] = CVI_VB_CreatePool(&stVbPoolCfg);
	if (gVencPicVbPool[VencChnIdx] == VB_INVALID_POOLID) {
		SAMPLE_PRT("VencChnIdx[%d]\n", VencChnIdx);
		return CVI_FAILURE;
	}

	SAMPLE_PRT("CVI_VB_CreatePool : id:%d, u32BlkSize=0x%x, u32BlkCnt=%d chn[%d]\n",
		gVencPicVbPool[VencChnIdx], stVbPoolCfg.u32BlkSize, stVbPoolCfg.u32BlkCnt, VencChnIdx);



	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VENC_SetH264Trans(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_H264_TRANS_S h264Trans = { 0 };
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_VENC_GetH264Trans(VencChn, &h264Trans);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetH264Trans, %d\n", s32Ret);
		return s32Ret;
	}

	h264Trans.chroma_qp_index_offset = pIc->h264ChromaQpOffset;
	s32Ret = CVI_VENC_SetH264Trans(VencChn, &h264Trans);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetH264Trans, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetH265Trans(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_H265_TRANS_S h265Trans = { 0 };
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_VENC_GetH265Trans(VencChn, &h265Trans);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetH265Trans, %d\n", s32Ret);
		return s32Ret;
	}

	h265Trans.cb_qp_offset = pIc->h265CbQpOffset;
	h265Trans.cr_qp_offset = pIc->h265CrQpOffset;
	s32Ret = CVI_VENC_SetH265Trans(VencChn, &h265Trans);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetH265Trans, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetH264Vui(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_H264_VUI_S h264Vui = { 0 };
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_VENC_GetH264Vui(VencChn, &h264Vui);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetH264Vui, %d\n", s32Ret);
		return s32Ret;
	}

	h264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = pIc->aspectRatioInfoPresentFlag;
	if (h264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag) {
		h264Vui.stVuiAspectRatio.aspect_ratio_idc = pIc->aspectRatioIdc;
		h264Vui.stVuiAspectRatio.sar_width = pIc->sarWidth;
		h264Vui.stVuiAspectRatio.sar_height = pIc->sarHeight;
	}

	h264Vui.stVuiAspectRatio.overscan_info_present_flag = pIc->overscanInfoPresentFlag;
	if (h264Vui.stVuiAspectRatio.overscan_info_present_flag) {
		h264Vui.stVuiAspectRatio.overscan_appropriate_flag = pIc->overscanAppropriateFlag;
	}

	h264Vui.stVuiTimeInfo.timing_info_present_flag = pIc->timingInfoPresentFlag;
	if (h264Vui.stVuiTimeInfo.timing_info_present_flag) {
		h264Vui.stVuiTimeInfo.fixed_frame_rate_flag = pIc->fixedFrameRateFlag;
		h264Vui.stVuiTimeInfo.num_units_in_tick = pIc->numUnitsInTick;
		h264Vui.stVuiTimeInfo.time_scale = pIc->timeScale;
	}

	h264Vui.stVuiVideoSignal.video_signal_type_present_flag = pIc->videoSignalTypePresentFlag;
	if (h264Vui.stVuiVideoSignal.video_signal_type_present_flag) {
		h264Vui.stVuiVideoSignal.video_format = pIc->videoFormat;
		h264Vui.stVuiVideoSignal.video_full_range_flag = pIc->videoFullRangeFlag;
		h264Vui.stVuiVideoSignal.colour_description_present_flag = pIc->colourDescriptionPresentFlag;
		if (h264Vui.stVuiVideoSignal.colour_description_present_flag) {
			h264Vui.stVuiVideoSignal.colour_primaries = pIc->colourPrimaries;
			h264Vui.stVuiVideoSignal.transfer_characteristics = pIc->transferCharacteristics;
			h264Vui.stVuiVideoSignal.matrix_coefficients = pIc->matrixCoefficients;
		}
	}

	s32Ret = CVI_VENC_SetH264Vui(VencChn, &h264Vui);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetH264Vui, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetH265Vui(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_H265_VUI_S h265Vui = { 0 };
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_VENC_GetH265Vui(VencChn, &h265Vui);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetH265Vui, %d\n", s32Ret);
		return s32Ret;
	}

	h265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = pIc->aspectRatioInfoPresentFlag;
	if (h265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag) {
		h265Vui.stVuiAspectRatio.aspect_ratio_idc = pIc->aspectRatioIdc;
		h265Vui.stVuiAspectRatio.sar_width = pIc->sarWidth;
		h265Vui.stVuiAspectRatio.sar_height = pIc->sarHeight;
	}

	h265Vui.stVuiAspectRatio.overscan_info_present_flag = pIc->overscanInfoPresentFlag;
	if (h265Vui.stVuiAspectRatio.overscan_info_present_flag) {
		h265Vui.stVuiAspectRatio.overscan_appropriate_flag = pIc->overscanAppropriateFlag;
	}

	h265Vui.stVuiTimeInfo.timing_info_present_flag = pIc->timingInfoPresentFlag;
	if (h265Vui.stVuiTimeInfo.timing_info_present_flag) {
		h265Vui.stVuiTimeInfo.num_units_in_tick = pIc->numUnitsInTick;
		h265Vui.stVuiTimeInfo.time_scale = pIc->timeScale;
	}

	h265Vui.stVuiVideoSignal.video_signal_type_present_flag = pIc->videoSignalTypePresentFlag;
	if (h265Vui.stVuiVideoSignal.video_signal_type_present_flag) {
		h265Vui.stVuiVideoSignal.video_format = pIc->videoFormat;
		h265Vui.stVuiVideoSignal.video_full_range_flag = pIc->videoFullRangeFlag;
		h265Vui.stVuiVideoSignal.colour_description_present_flag = pIc->colourDescriptionPresentFlag;
		if (h265Vui.stVuiVideoSignal.colour_description_present_flag) {
			h265Vui.stVuiVideoSignal.colour_primaries = pIc->colourPrimaries;
			h265Vui.stVuiVideoSignal.transfer_characteristics = pIc->transferCharacteristics;
			h265Vui.stVuiVideoSignal.matrix_coefficients = pIc->matrixCoefficients;
		}
	}

	s32Ret = CVI_VENC_SetH265Vui(VencChn, &h265Vui);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetH265Vui, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetChnParam(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_CHN_PARAM_S stChnParam, *pstChnParam = &stChnParam;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VENC_GetChnParam(VencChn, pstChnParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetJpegParam\n");
		return CVI_FAILURE;
	}

	pstChnParam->stCropCfg.bEnable = (pIc->posX || pIc->posY);
	pstChnParam->stCropCfg.stRect.s32X = pIc->posX;
	pstChnParam->stCropCfg.stRect.s32Y = pIc->posY;
	pstChnParam->stCropCfg.stRect.u32Width = pIc->width;
	pstChnParam->stCropCfg.stRect.u32Height = pIc->height;
	SAMPLE_PRT("s32X = %d, s32Y = %d\n",
			pstChnParam->stCropCfg.stRect.s32X,
			pstChnParam->stCropCfg.stRect.s32Y);

	pstChnParam->stFrameRate.s32SrcFrmRate = pIc->srcFramerate;
	pstChnParam->stFrameRate.s32DstFrmRate = pIc->framerate;

	s32Ret = CVI_VENC_SetChnParam(VencChn, pstChnParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetChnParam fail\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetH264Dblk(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_H264_DBLK_S h264Dblk = { 0 };
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_VENC_GetH264Dblk(VencChn, &h264Dblk);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetH264Dblk, %d\n", s32Ret);
		return s32Ret;
	}

	h264Dblk.disable_deblocking_filter_idc = pIc->bDisableDeblk;
	h264Dblk.slice_alpha_c0_offset_div2 = pIc->alphaOffset;
	h264Dblk.slice_beta_offset_div2 = pIc->betaOffset;

	s32Ret = CVI_VENC_SetH264Dblk(VencChn, &h264Dblk);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetH264Dblk, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetH265Dblk(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_H265_DBLK_S h265Dblk = { 0 };
	CVI_S32 s32Ret = CVI_FAILURE;

	s32Ret = CVI_VENC_GetH265Dblk(VencChn, &h265Dblk);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetH265Dblk, %d\n", s32Ret);
		return s32Ret;
	}

	h265Dblk.slice_deblocking_filter_disabled_flag = pIc->bDisableDeblk;
	h265Dblk.slice_beta_offset_div2 = pIc->betaOffset;
	h265Dblk.slice_tc_offset_div2 = pIc->alphaOffset;

	s32Ret = CVI_VENC_SetH265Dblk(VencChn, &h265Dblk);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetH265Dblk, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetH264IntraPred(chnInputCfg *pIc, VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_H264_INTRA_PRED_S stH264IntraPred, *pstH264IntraPred = &stH264IntraPred;

	s32Ret = CVI_VENC_GetH264IntraPred(VencChn, pstH264IntraPred);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetH264IntraPred failed!\n");
		return CVI_FAILURE;
	}

	pstH264IntraPred->constrained_intra_pred_flag = pIc->bIntraPred;

	s32Ret = CVI_VENC_SetH264IntraPred(VencChn, pstH264IntraPred);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetH264IntraPred failed!\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetH265PredUnit(chnInputCfg *pIc, VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_H265_PU_S stH265PredUnit, *pstH265PredUnit = &stH265PredUnit;

	(void) pIc;
	s32Ret = CVI_VENC_GetH265PredUnit(VencChn, pstH265PredUnit);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetH265PredUnit failed!\n");
		return CVI_FAILURE;
	}

	if (pIc->bSetPredUnit) {
		pstH265PredUnit->constrained_intra_pred_flag = pIc->bIntraPred;
		pstH265PredUnit->strong_intra_smoothing_enabled_flag = pIc->bSmoothingEnable;
	}

	s32Ret = CVI_VENC_SetH265PredUnit(VencChn, pstH265PredUnit);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SetH265PredUnit failed!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VENC_GetH265PredUnit(VencChn, pstH265PredUnit);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("GetH265PredUnit failed!\n");
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_EnableSvc(
		chnInputCfg *pIc,
		VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pIc->svc_enable) {
		s32Ret = CVI_VENC_EnableSvc(VencChn, pIc->svc_enable);
		if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("Svc enable failed!\n");
				return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VENC_SetSvcParam(
		chnInputCfg *pIc,
		VENC_CHN VencChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_SVC_PARAM_S stSvcParam, *pstSvcParam = &stSvcParam;

	if (pIc->svc_enable) {
		s32Ret = CVI_VENC_GetSvcParam(VencChn, pstSvcParam);
		if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("GetSvcParam failed!\n");
				return CVI_FAILURE;
		}
		pstSvcParam->bFgProtectEn = pIc->fg_protect_en;
		pstSvcParam->u32FgDealtQp = pIc->fg_dealt_qp;
		if (pIc->complex_scene_detect_en) {
				pstSvcParam->bComplexSceneDetectEn = pIc->complex_scene_detect_en;
				if (pIc->complex_scene_hight_th > pIc->complex_scene_low_th &&
				pIc->complex_scene_low_th) {
						pstSvcParam->u32ComplexSceneHightTh = pIc->complex_scene_hight_th;
						pstSvcParam->u32ComplexSceneLowTh = pIc->complex_scene_low_th;
				}
				if (pIc->complex_min_percent > pIc->middle_min_percent &&
				pIc->middle_min_percent) {
						pstSvcParam->u32MiddleMinPercent = pIc->middle_min_percent;
						pstSvcParam->u32ComplexMinPercent = pIc->complex_min_percent;
				}
		}
		pstSvcParam->bSmartAiEn = pIc->smart_ai_en;
		if (pstSvcParam->bSmartAiEn){
			memcpy(pstSvcParam->s8ObjTab, pIc->obj_tab, sizeof(pIc->obj_tab));
			memcpy(pstSvcParam->s32DqpTable, pIc->dqp_tab, sizeof(pIc->dqp_tab));
			pstSvcParam->u32DqpVaild = 1;
			pstSvcParam->u32ObjTabSize = pIc->obj_size;
		}
		s32Ret = CVI_VENC_SetSvcParam(VencChn, pstSvcParam);
		if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SetSvcParam failed!\n");
				return CVI_FAILURE;
		}
	}

	return s32Ret;
}


CVI_S32 SAMPLE_COMM_VENC_Start(
		chnInputCfg * pIc,
		VENC_CHN VencChn,
		PAYLOAD_TYPE_E enType,
		PIC_SIZE_E enSize,
		SAMPLE_RC_E enRcMode,
		CVI_U32 u32Profile,
		CVI_BOOL bRcnRefShareBuf,
		VENC_GOP_ATTR_S *pstGopAttr)
{
	CVI_S32 s32Ret;
	VENC_RECV_PIC_PARAM_S stRecvParam;
	//VENC_INITIAL_INFO_S stEncInitialInfo;


	s32Ret = SAMPLE_COMM_VENC_Create(
			pIc, VencChn, enType, enSize, enRcMode,
			u32Profile, bRcnRefShareBuf, pstGopAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VENC_Create failed with %d\n", s32Ret);
		return CVI_FAILURE;
	}

#if 0
	if (pIc->bind_mode == VENC_BIND_VPSS) {
		SAMPLE_PRT("VPSS_Bind_VENC, vpss Grp = %d, Chn = %d, VencChn = %d\n",
				pIc->vpssGrp, pIc->vpssChn, VencChn);
		SAMPLE_COMM_VPSS_Bind_VENC(pIc->vpssGrp, pIc->vpssChn, VencChn);
	}
#endif

	stRecvParam.s32RecvPicNum = pIc->num_frames;
	s32Ret = CVI_VENC_StartRecvFrame(VencChn, &stRecvParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_StartRecvPic failed with %d\n", s32Ret);
		return CVI_FAILURE;
	}

	// get enc initial info
	/*if (!strcmp(pIc->codec, "264") ||  !strcmp(pIc->codec, "265")) {
		s32Ret = CVI_VENC_GetIntialInfo(VencChn, &stEncInitialInfo);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_StartRecvPic failed get initial info with %d\n", s32Ret);
			return CVI_FAILURE;
		}
		pIc->u32MinSrcCount = stEncInitialInfo.min_num_src_fb;
		pIc->u32MinBsBufSize = stEncInitialInfo.min_bs_buf_size;

		SAMPLE_PRT("get enc recon frame cnt:%d, src frame cnt:%d, bs buf size:%d\n"
			, stEncInitialInfo.min_num_rec_fb, stEncInitialInfo.min_num_src_fb, pIc->u32MinBsBufSize);
	}*/


	return CVI_SUCCESS;
}
