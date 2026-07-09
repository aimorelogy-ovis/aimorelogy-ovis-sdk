#ifndef __LDC_UT_COMM_H__
#define __LDC_UT_COMM_H__

#include "ut_comm.h"

CVI_S32 GDC_COMM_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 GDC_COMM_PrepareFrame2(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 GDCFileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 GDCFileToFrame2(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

#endif
