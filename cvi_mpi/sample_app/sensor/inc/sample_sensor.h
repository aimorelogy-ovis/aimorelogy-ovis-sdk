/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: ae_test.h
 * Description:
 */

#ifndef __SAMPLE_SENSOR_H_
#define __SAMPLE_SENSOR_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define WDR_MAX_PIPE_NUM 4

typedef struct _TEST_SENSOR_MCLK_ATTR_S {
	CVI_U8 u8Mclk;
	CVI_BOOL bMclkEn;
} TEST_SENSOR_MCLK_ATTR_S;

typedef struct _TEST_SENSOR_INFO_S {
	CVI_SNS_TYPE_E enSnsType;
	CVI_S32 s32SnsId;
	CVI_S32 s32BusId;
	CVI_S32 s32SnsI2cAddr;
	CVI_U8 MipiDev;
	CVI_S16 as16LaneId[MIPI_LANE_NUM + 1];
	CVI_S8  as8PNSwap[MIPI_LANE_NUM + 1];
	CVI_U8  u8HwSync;
	TEST_SENSOR_MCLK_ATTR_S stMclkAttr;
	CVI_U8 u8Orien;	// 0: normal, 1: mirror, 2: flip, 3: mirror and flip.
} TEST_SENSOR_INFO_S;

typedef struct _TEST_DEV_INFO_S {
	VI_DEV ViDev;
	WDR_MODE_E enWDRMode;
	VI_ISP_YUV_SCENE_E enYuvSceneMode;
} TEST_DEV_INFO_S;

typedef struct _TEST_PIPE_INFO_S {
	VI_PIPE aPipe[VI_MAX_PIPE_NUM];
	VI_VPSS_MODE_E enMastPipeMode;
	bool bMultiPipe;
	bool bVcNumCfged;
	bool bIspBypass;
	PIXEL_FORMAT_E enPixFmt;
	CVI_U32 u32VCNum[VI_MAX_PIPE_NUM];
} TEST_PIPE_INFO_S;

typedef struct _TEST_CHN_INFO_S {
	VI_CHN ViChn;
	PIXEL_FORMAT_E enPixFormat;
	DYNAMIC_RANGE_E enDynamicRange;
	VIDEO_FORMAT_E enVideoFormat;
	COMPRESS_MODE_E enCompressMode;
} TEST_CHN_INFO_S;

typedef struct _TEST_VI_INFO_S {
	TEST_SENSOR_INFO_S stSnsInfo;
	TEST_DEV_INFO_S stDevInfo;
	TEST_PIPE_INFO_S stPipeInfo;
	TEST_CHN_INFO_S stChnInfo;
} TEST_VI_INFO_S;

typedef struct _TEST_VI_CONFIG_S {
	TEST_VI_INFO_S astViInfo[VI_MAX_DEV_NUM];
	CVI_S32 as32WorkingViId[VI_MAX_DEV_NUM];
	CVI_S32 s32WorkingViNum;
	CVI_BOOL bViRotation;
} TEST_VI_CONFIG_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif


