#include "vo_ut_comm.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_math.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"

CVI_S32 VO_GetWH(VO_INTF_SYNC_E enIntfSync, CVI_U32 *pu32W, CVI_U32 *pu32H, CVI_U32 *pu32Frm)
{
	switch (enIntfSync) {
	case VO_OUTPUT_PAL:
		*pu32W = 720;
		*pu32H = 576;
		*pu32Frm = 25;
		break;
	case VO_OUTPUT_NTSC:
		*pu32W = 720;
		*pu32H = 480;
		*pu32Frm = 30;
		break;
	case VO_OUTPUT_1080P24:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 24;
		break;
	case VO_OUTPUT_1080P25:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 25;
		break;
	case VO_OUTPUT_1080P30:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 30;
		break;
	case VO_OUTPUT_720P50:
		*pu32W = 1280;
		*pu32H = 720;
		*pu32Frm = 50;
		break;
	case VO_OUTPUT_720P60:
		*pu32W = 1280;
		*pu32H = 720;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1080P50:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 50;
		break;
	case VO_OUTPUT_1080P60:
		*pu32W = 1920;
		*pu32H = 1080;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_576P50:
		*pu32W = 720;
		*pu32H = 576;
		*pu32Frm = 50;
		break;
	case VO_OUTPUT_480P60:
		*pu32W = 720;
		*pu32H = 480;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_800x600_60:
		*pu32W = 800;
		*pu32H = 600;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1024x768_60:
		*pu32W = 1024;
		*pu32H = 768;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1280x1024_60:
		*pu32W = 1280;
		*pu32H = 1024;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1366x768_60:
		*pu32W = 1366;
		*pu32H = 768;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1440x900_60:
		*pu32W = 1440;
		*pu32H = 900;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1280x800_60:
		*pu32W = 1280;
		*pu32H = 800;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1600x1200_60:
		*pu32W = 1600;
		*pu32H = 1200;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1680x1050_60:
		*pu32W = 1680;
		*pu32H = 1050;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1920x1200_60:
		*pu32W = 1920;
		*pu32H = 1200;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_640x480_60:
		*pu32W = 640;
		*pu32H = 480;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_720x1280_60:
		*pu32W = 720;
		*pu32H = 1280;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_1080x1920_60:
		*pu32W = 1080;
		*pu32H = 1920;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_480x800_60:
		*pu32W = 480;
		*pu32H = 800;
		*pu32Frm = 60;
		break;
	case VO_OUTPUT_USER:
		*pu32W = 720;
		*pu32H = 576;
		*pu32Frm = 25;
		break;
	default:
		UT_PRT("vo enIntfSync %d not support!\n", enIntfSync);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_SetPubAttr(VoDev, pstPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_Enable(VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 VO_StopDev(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_Disable(VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 VO_StopLayer(VO_LAYER VoLayer)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 VO_StartChn(VO_LAYER VoLayer, VO_MODE_E enMode)
{
	CVI_U32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32WndNum = 0;
	CVI_U32 u32Square = 0;
	CVI_U32 u32Row = 0;
	CVI_U32 u32Col = 0;
	CVI_U32 u32Width = 0;
	CVI_U32 u32Height = 0;
	VO_CHN_ATTR_S stChnAttr = {0};
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	switch (enMode) {
	case VO_MODE_1MUX:
		u32WndNum = 1;
		u32Square = 1;
		break;
	case VO_MODE_2MUX:
		u32WndNum = 2;
		u32Square = 2;
		break;
	case VO_MODE_4MUX:
		u32WndNum = 4;
		u32Square = 2;
		break;
	case VO_MODE_8MUX:
		u32WndNum = 8;
		u32Square = 3;
		break;
	case VO_MODE_9MUX:
		u32WndNum = 9;
		u32Square = 3;
		break;
	case VO_MODE_16MUX:
		u32WndNum = 16;
		u32Square = 4;
		break;
	case VO_MODE_25MUX:
		u32WndNum = 25;
		u32Square = 5;
		break;
	case VO_MODE_36MUX:
		u32WndNum = 36;
		u32Square = 6;
		break;
	case VO_MODE_49MUX:
		u32WndNum = 49;
		u32Square = 7;
		break;
	case VO_MODE_64MUX:
		u32WndNum = 64;
		u32Square = 8;
		break;
	case VO_MODE_2X4:
		u32WndNum = 8;
		u32Square = 3;
		u32Row = 4;
		u32Col = 2;
		break;
	default:
		UT_PRT("Undefined VO_MODE(%d)!\n", enMode);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;
	UT_PRT("u32Width:%d, u32Height:%d, u32Square:%d\n", u32Width, u32Height, u32Square);
	for (i = 0; i < u32WndNum; i++) {
		if (enMode == VO_MODE_1MUX || enMode == VO_MODE_2MUX || enMode == VO_MODE_4MUX ||
		    enMode == VO_MODE_8MUX || enMode == VO_MODE_9MUX || enMode == VO_MODE_16MUX ||
		    enMode == VO_MODE_25MUX || enMode == VO_MODE_36MUX || enMode == VO_MODE_49MUX ||
		    enMode == VO_MODE_64MUX) {
			stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
			stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
			stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Square, 2);
			stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Square, 2);
			stChnAttr.u32Priority = 0;
		} else if (enMode == VO_MODE_2X4) {
			stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Col) * (i % u32Col), 2);
			stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Row) * (i / u32Col), 2);
			stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Col, 2);
			stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Row, 2);
			stChnAttr.u32Priority = 0;
		}

		s32Ret = CVI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VO_EnableChn(VoLayer, i);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 VO_StopChn(VO_LAYER VoLayer, VO_MODE_E enMode)
{
	CVI_U32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32WndNum = 0;

	switch (enMode) {
	case VO_MODE_1MUX: {
		u32WndNum = 1;
		break;
	}
	case VO_MODE_2MUX: {
		u32WndNum = 2;
		break;
	}
	case VO_MODE_4MUX: {
		u32WndNum = 4;
		break;
	}
	case VO_MODE_8MUX: {
		u32WndNum = 8;
		break;
	}
	default:
		UT_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	for (i = 0; i < u32WndNum; i++) {
		s32Ret = CVI_VO_DisableChn(VoLayer, i);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

/*
 * Name : VO_GetDefConfig
 * Desc : An instance of SAMPLE_VO_CONFIG_S, which allows you to use vo immediately.
 */
CVI_S32 VO_GetDefConfig(VO_CONFIG_S *pstVoConfig)
{
	if (pstVoConfig == NULL) {
		UT_PRT("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}

	pstVoConfig->VoDev             = 0;

	pstVoConfig->stVoPubAttr.enIntfType = VO_INTF_MIPI;

	RECT_S stDefDispRect  = {0, 0, 1920, 1080};
	SIZE_S stDefImageSize = {1920, 1080};

	pstVoConfig->stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	pstVoConfig->stDispRect    = stDefDispRect;
	pstVoConfig->stImageSize   = stDefImageSize;
	pstVoConfig->enPixFormat   = PIXEL_FORMAT_RGB_888_PLANAR;
	pstVoConfig->stVoPubAttr.u32BgColor = 0;
	pstVoConfig->u32DisBufLen  = 3;
	pstVoConfig->enVoMode      = VO_MODE_1MUX;

	return CVI_SUCCESS;
}

CVI_S32 VO_StartVO(VO_CONFIG_S *pstVoConfig)
{
	/*******************************************
	 * VO device VoDev# information declaration.
	 *******************************************/
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = 0;
	VO_MODE_E enVoMode = 0;
	VO_PUB_ATTR_S stVoPubAttr = pstVoConfig->stVoPubAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr = { 0 };
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstVoConfig == NULL) {
		UT_PRT("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}
	VoDev = pstVoConfig->VoDev;
	VoLayer = pstVoConfig->VoDev;
	enVoMode = pstVoConfig->enVoMode;

	/********************************
	 * Set and start VO device VoDev#.
	 ********************************/
	s32Ret = VO_StartDev(VoDev, &pstVoConfig->stVoPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("VO_StartDev failed!\n");
		return s32Ret;
	}

	/******************************
	 * Set and start layer VoDev#.
	 ********************************/

	s32Ret = VO_GetWH(stVoPubAttr.enIntfSync, &stLayerAttr.stDispRect.u32Width,
			  &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("VO_GetWH failed!\n");
		VO_StopDev(VoDev);
		return s32Ret;
	}
	stLayerAttr.enPixFormat = pstVoConfig->enPixFormat;

	stLayerAttr.stDispRect.s32X = 0;
	stLayerAttr.stDispRect.s32Y = 0;

	/******************************
	 * Set display rectangle if changed.
	 ********************************/
	if (memcmp(&pstVoConfig->stDispRect, &stLayerAttr.stDispRect, sizeof(RECT_S)) != 0)
		stLayerAttr.stDispRect = pstVoConfig->stDispRect;
	stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

	/******************************
	 * Set image size if changed.
	 ********************************/
	if (memcmp(&pstVoConfig->stImageSize, &stLayerAttr.stImageSize, sizeof(SIZE_S)) != 0)
		stLayerAttr.stImageSize = pstVoConfig->stImageSize;

	if (pstVoConfig->u32DisBufLen) {
		s32Ret = CVI_VO_SetDisplayBufLen(VoLayer, pstVoConfig->u32DisBufLen);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VO_SetDisplayBufLen failed with %#x!\n", s32Ret);
			VO_StopDev(VoDev);
			return s32Ret;
		}
	}

	s32Ret = VO_StartLayer(VoLayer, &stLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("VO_Start video layer failed!\n");
		VO_StopDev(VoDev);
		return s32Ret;
	}

	/******************************
	 * start vo channels.
	 ********************************/
	s32Ret = VO_StartChn(VoLayer, enVoMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("VO_StartChn failed!\n");
		VO_StopLayer(VoLayer);
		VO_StopDev(VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 VO_StopVO(VO_CONFIG_S *pstVoConfig)
{
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = 0;
	VO_MODE_E enVoMode = VO_MODE_BUTT;

	if (pstVoConfig == NULL) {
		UT_PRT("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}

	VoDev = pstVoConfig->VoDev;
	VoLayer = pstVoConfig->VoDev;
	enVoMode = pstVoConfig->enVoMode;

	VO_StopChn(VoLayer, enVoMode);
	VO_StopLayer(VoLayer);
	VO_StopDev(VoDev);

	return CVI_SUCCESS;
}

CVI_S32 VPSS_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
		  VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	VPSS_CHN VpssChn;
	CVI_S32 s32Ret;
	CVI_S32 j;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return CVI_FAILURE;
	}

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);

			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);

			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
		   VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	CVI_S32 s32Ret;
	(void)(pabChnEnable);
	(void)(pstVpssGrpAttr);
	(void)(pastVpssChnAttr);

	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable)
{
	CVI_S32 j;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_CHN VpssChn;

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("Vpss stop Grp %d channel %d failed! Please check param\n",
				VpssGrp, VpssChn);
				return CVI_FAILURE;
			}
		}
	}

	s32Ret = CVI_VPSS_StopGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("Vpss Stop Grp %d failed! Please check param\n", VpssGrp);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("Vpss Destroy Grp %d failed! Please check\n", VpssGrp);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 VPSS_Bind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn)
{
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;

	stSrcChn.enModId = CVI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;

	stDestChn.enModId = CVI_ID_VO;
	stDestChn.s32DevId = VoLayer;
	stDestChn.s32ChnId = VoChn;

	CVI_SYS_Bind(&stSrcChn, &stDestChn);

	return CVI_SUCCESS;
}

CVI_S32 VPSS_UnBind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn)
{
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;

	stSrcChn.enModId = CVI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;

	stDestChn.enModId = CVI_ID_VO;
	stDestChn.s32DevId = VoLayer;
	stDestChn.s32ChnId = VoChn;

	CVI_SYS_UnBind(&stSrcChn, &stDestChn);

	return CVI_SUCCESS;
}

CVI_S32 VPSS_SendFrame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	FILE *fp;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;

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
		UT_PRT("SAMPLE_COMM_VPSS_SendFrame: Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		UT_PRT("open data file error\n");
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
			UT_PRT("vpss send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	UT_PRT("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);
	UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2]);
	UT_PRT("vir addr(%p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2]);

	fclose(fp);

	UT_PRT("read file done and send out frame.\n");
	CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, -1);
	CVI_VB_ReleaseBlock(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	return CVI_SUCCESS;
}