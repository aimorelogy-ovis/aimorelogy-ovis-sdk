#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "sample_comm.h"

CVI_S32 SAMPLE_COMM_VO_GetWH(VO_INTF_SYNC_E enIntfSync, CVI_U32 *pu32W, CVI_U32 *pu32H, CVI_U32 *pu32Frm)
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
		SAMPLE_PRT("vo enIntfSync %d not support!\n", enIntfSync);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_SetPubAttr(VoDev, pstPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_Enable(VoDev);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VO_StopDev(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_Disable(VoDev);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VO_StopLayer(VO_LAYER VoLayer)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VO_StartChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode)
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
		SAMPLE_PRT("Undefined VO_MODE(%d)!\n", enMode);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;
	SAMPLE_PRT("u32Width:%d, u32Height:%d, u32Square:%d\n", u32Width, u32Height, u32Square);
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
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VO_EnableChn(VoLayer, i);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VO_StopChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode)
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
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	for (i = 0; i < u32WndNum; i++) {
		s32Ret = CVI_VO_DisableChn(VoLayer, i);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_VO_GetDefConfig(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
	if (pstVoConfig == NULL) {
		SAMPLE_PRT("Error:argument can not be NULL\n");
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

CVI_S32 SAMPLE_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
	/*******************************************
	 * VO device VoDev# information declaration.
	 *******************************************/
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = 0;
	SAMPLE_VO_MODE_E enVoMode = 0;
	VO_PUB_ATTR_S stVoPubAttr = { 0 };
	VO_VIDEO_LAYER_ATTR_S stLayerAttr = { 0 };
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstVoConfig == NULL) {
		SAMPLE_PRT("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}
	VoDev = pstVoConfig->VoDev;
	VoLayer = pstVoConfig->VoDev;
	enVoMode = pstVoConfig->enVoMode;

	/********************************
	 * Set and start VO device VoDev#.
	 ********************************/
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &pstVoConfig->stVoPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VO_StartDev failed!\n");
		return s32Ret;
	}

	/******************************
	 * Set and start layer VoDev#.
	 ********************************/

	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, &stLayerAttr.stDispRect.u32Width,
			  &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VO_GetWH failed!\n");
		SAMPLE_COMM_VO_StopDev(VoDev);
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
			SAMPLE_PRT("CVI_VO_SetDisplayBufLen failed with %#x!\n", s32Ret);
			SAMPLE_COMM_VO_StopDev(VoDev);
			return s32Ret;
		}
	}

	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VO_Start video layer failed!\n");
		SAMPLE_COMM_VO_StopDev(VoDev);
		return s32Ret;
	}

	/******************************
	 * start vo channels.
	 ********************************/
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
		SAMPLE_COMM_VO_StopLayer(VoLayer);
		SAMPLE_COMM_VO_StopDev(VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = 0;
	SAMPLE_VO_MODE_E enVoMode = VO_MODE_BUTT;

	if (pstVoConfig == NULL) {
		SAMPLE_PRT("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}

	VoDev = pstVoConfig->VoDev;
	VoLayer = pstVoConfig->VoDev;
	enVoMode = pstVoConfig->enVoMode;

	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	SAMPLE_COMM_VO_StopLayer(VoLayer);
	SAMPLE_COMM_VO_StopDev(VoDev);

	return CVI_SUCCESS;
}