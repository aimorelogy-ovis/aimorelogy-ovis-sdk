#ifndef __VPSS_CMODEL_H__
#define __VPSS_CMODEL_H__

#include "vpss_ut_comm.h"
#include "cvi_type.h"

CVI_VOID VpssCscRgb2Yuv(CVI_U8 rgbData[3], CVI_U8 yuvData[3]);
CVI_VOID VpssCscYuv2Rgb(CVI_U8 yuvData[3], CVI_U8 rgbData[3]);

#endif
