#ifndef __VPSS_UT_COMM_H__
#define __VPSS_UT_COMM_H__

#include "ut_comm.h"

CVI_S32 FileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
	const CVI_CHAR *filename);
CVI_S32 FbcFileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize,
		PIXEL_FORMAT_E enPixelFormat, const CVI_CHAR filename[4][64], CVI_U32 fbctablelength);
CVI_S32 CompareCmodelRgb2Yuv(VIDEO_FRAME_INFO_S *pstVideoFrameIn, VIDEO_FRAME_INFO_S *pstVideoFrameOut);
CVI_S32 CompareCmodelYuv2rgb(VIDEO_FRAME_INFO_S *pstVideoFrameIn, VIDEO_FRAME_INFO_S *pstVideoFrameOut);

#endif
