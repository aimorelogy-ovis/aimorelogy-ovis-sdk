
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cvi_ae.h"
#include "cvi_isp.h"

static AE_FLICKER_STATUS_E run_flicker_detect(VI_PIPE ViPipe, ISP_FLICKER_DETECT_S *pstFlickerDetectAttr)
{
#define MAX_DETECT_CNT (30)
	int detect_frame_cnt = MAX_DETECT_CNT;
	AE_FLICKER_STATUS_E enFlickerStatus = AE_FLICKER_NONE;
	CVI_S32 s32Ret;

	pstFlickerDetectAttr->bEnable = CVI_TRUE;
	s32Ret = CVI_ISP_SetFlickerDetectAttr(ViPipe, pstFlickerDetectAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_ISP_SetFlickerDetectAttr failed with %#x!\n", s32Ret);
		return enFlickerStatus;
	}

	printf("Run flicker detect: bEnable=%d, DetectThr=[%d, %d, %d], LumaStableThr=%d, DarkThr=%d\n",
	       pstFlickerDetectAttr->bEnable,
	       pstFlickerDetectAttr->u8DetectThr[0],
	       pstFlickerDetectAttr->u8DetectThr[1],
	       pstFlickerDetectAttr->u8DetectThr[2],
	       pstFlickerDetectAttr->u8LumaStableThr,
	       pstFlickerDetectAttr->u16DarkThr);

	do {
		s32Ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 500);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_ISP_GetVDTimeOut failed with %#x!\n", s32Ret);
			break;
		}
		s32Ret = CVI_ISP_GetFlickerDetectStatus(ViPipe, &enFlickerStatus);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_ISP_GetFlickerDetectStatus failed with %#x!\n", s32Ret);
			break;
		}
		detect_frame_cnt--;
		if (detect_frame_cnt <= 0) {
			printf("Flicker detect timeout!\n");
			break;
		}
	} while (enFlickerStatus == AE_FLICKER_NONE);

	printf("Flicker detect finished, detect frame cnt: %d, detect status: %s\n",
		MAX_DETECT_CNT - detect_frame_cnt,
		(enFlickerStatus == AE_FLICKER_YES) ? "YES" :
		(enFlickerStatus == AE_FLICKER_NO) ? "NO" : "NONE");

	pstFlickerDetectAttr->bEnable = CVI_FALSE;
	s32Ret = CVI_ISP_SetFlickerDetectAttr(ViPipe, pstFlickerDetectAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_ISP_SetFlickerDetectAttr disable failed with %#x!\n", s32Ret);
		return enFlickerStatus;
	}

	return enFlickerStatus;
}

static int set_antiflicer_cfg(VI_PIPE ViPipe, CVI_FLOAT fps, CVI_BOOL enable, CVI_U8 frequency)
{
	CVI_S32 s32Ret;
	ISP_PUB_ATTR_S stIspPubAttr;

	memset(&stIspPubAttr, 0, sizeof(ISP_PUB_ATTR_S));
	s32Ret = CVI_ISP_GetPubAttr(ViPipe, &stIspPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_ISP_GetPubAttr failed with %#x!\n", s32Ret);
		return -1;
	}
	stIspPubAttr.f32FrameRate = fps;
	s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stIspPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_ISP_SetPubAttr failed with %#x!\n", s32Ret);
		return -1;
	}
	s32Ret = CVI_ISP_SetAntiFlicker(ViPipe, enable, frequency);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_ISP_SetAntiFlicker failed with %#x!\n", s32Ret);
		return -1;
	}
	return 0;
}

static int get_antiflicker_cfg(VI_PIPE ViPipe, CVI_FLOAT *pfps, CVI_BOOL *penable, CVI_U8 *pfrequency)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_ISP_QueryFps(ViPipe, pfps);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_ISP_QueryFps failed with %#x!\n", s32Ret);
		return -1;
	}
	s32Ret = CVI_ISP_GetAntiFlicker(ViPipe, penable, pfrequency);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_ISP_GetAntiFlicker failed with %#x!\n", s32Ret);
		return -1;
	}
	return 0;
}

static int run_50hz_60hz_flicker_detect(VI_PIPE ViPipe, ISP_FLICKER_DETECT_S *pstFlickerDetectAttr)
{
#define WAIT_PARAM_ACTIVE_FRAME_CNT (5)
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8 wait_frame_cnt;
	AE_FLICKER_STATUS_E en50hzFlickerStatus = AE_FLICKER_NONE;
	AE_FLICKER_STATUS_E en60hzFlickerStatus = AE_FLICKER_NONE;
	CVI_FLOAT fps;
	CVI_BOOL bAntiflickerEnable;
	CVI_U8 u8AntiflickerFrequency;

	s32Ret = get_antiflicker_cfg(ViPipe, &fps, &bAntiflickerEnable, &u8AntiflickerFrequency);
	if (s32Ret != CVI_SUCCESS) {
		printf("get_antiflicker_cfg failed with %#x!\n", s32Ret);
		return -1;
	}

	printf("Run 50Hz flicker detect...\n");
	s32Ret = set_antiflicer_cfg(ViPipe, 25.0, CVI_TRUE, 50);
	if (s32Ret != CVI_SUCCESS) {
		printf("set_50hz_antiflicker failed with %#x!\n", s32Ret);
		return -1;
	}
	wait_frame_cnt = WAIT_PARAM_ACTIVE_FRAME_CNT;
	do {
		s32Ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 500);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_ISP_GetVDTimeOut failed with %#x!\n", s32Ret);
			return -1;
		}
		wait_frame_cnt--;
	} while (wait_frame_cnt > 0);

	en50hzFlickerStatus = run_flicker_detect(ViPipe, pstFlickerDetectAttr);
	printf("50Hz flicker detect status: %s\n",
	       (en50hzFlickerStatus == AE_FLICKER_YES) ? "YES" :
	       (en50hzFlickerStatus == AE_FLICKER_NO) ? "NO" : "NONE");

	printf("Run 60Hz flicker detect...\n");
	s32Ret = set_antiflicer_cfg(ViPipe, 30.0, CVI_TRUE, 60);
	if (s32Ret != CVI_SUCCESS) {
		printf("set_60hz_antiflicker failed with %#x!\n", s32Ret);
		return -1;
	}

	wait_frame_cnt = WAIT_PARAM_ACTIVE_FRAME_CNT;
	do {
		s32Ret = CVI_ISP_GetVDTimeOut(ViPipe, ISP_VD_BE_END, 500);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_ISP_GetVDTimeOut failed with %#x!\n", s32Ret);
			return -1;
		}
		wait_frame_cnt--;
	} while (wait_frame_cnt > 0);

	en60hzFlickerStatus = run_flicker_detect(ViPipe, pstFlickerDetectAttr);
	printf("60Hz flicker detect status: %s\n",
	       (en60hzFlickerStatus == AE_FLICKER_YES) ? "YES" :
	       (en60hzFlickerStatus == AE_FLICKER_NO) ? "NO" : "NONE");

	if (en50hzFlickerStatus == AE_FLICKER_YES && en60hzFlickerStatus == AE_FLICKER_NO) {
		set_antiflicer_cfg(ViPipe, 30.0, CVI_TRUE, 60);
		printf("Set 30fps and antiflicker frequency to 60Hz\n");
	} else if (en50hzFlickerStatus == AE_FLICKER_NO && en60hzFlickerStatus == AE_FLICKER_YES) {
		set_antiflicer_cfg(ViPipe, 25.0, CVI_TRUE, 50);
		printf("Set 25fps and antiflicker frequency to 50Hz\n");
	} else {
		set_antiflicer_cfg(ViPipe, fps, bAntiflickerEnable, u8AntiflickerFrequency);
		printf("Restore original fps %.2f, antiflicker enable %d, frequency %dHz\n",
		       fps, bAntiflickerEnable, u8AntiflickerFrequency);
	}
	return 0;
}

int flicker_detect_test_main(VI_PIPE ViPipe)
{
	int option;
	ISP_FLICKER_DETECT_S stFlickerDetectAttr;

	memset(&stFlickerDetectAttr, 0, sizeof(ISP_FLICKER_DETECT_S));

	sleep(1);

	system("stty erase ^H");

	do {
		printf("---Flicker Detect Test Menu------------------------------\n");
		printf("1: Run flicker detect use default parameters\n");
		printf("2: Run flicker detect use custom parameters\n");
		printf("255: Exit flicker detect test\n");
		printf("--------------------------------------------------------\n");
		printf("Please input the index: ");

		scanf("%d", &option);

		switch (option) {
		case 1:
			CVI_ISP_GetFlickerDetectAttr(ViPipe, &stFlickerDetectAttr);
			stFlickerDetectAttr.bEnable = CVI_TRUE;
			stFlickerDetectAttr.u8DetectThr[0] = 14;
			stFlickerDetectAttr.u8DetectThr[1] = 20;
			stFlickerDetectAttr.u8DetectThr[2] = 20;
			stFlickerDetectAttr.u8LumaStableThr = 8;
			stFlickerDetectAttr.u16DarkThr = 340;
			run_50hz_60hz_flicker_detect(ViPipe, &stFlickerDetectAttr);
			break;
		case 2:
			CVI_ISP_GetFlickerDetectAttr(ViPipe, &stFlickerDetectAttr);
			stFlickerDetectAttr.bEnable = CVI_TRUE;
			printf("Please input flicker detect parameters:\n");
			printf("Detect Threshold 0 (0-255): ");
			scanf("%hhu", &stFlickerDetectAttr.u8DetectThr[0]);
			printf("Detect Threshold 1 (0-255): ");
			scanf("%hhu", &stFlickerDetectAttr.u8DetectThr[1]);
			printf("Detect Threshold 2 (0-255): ");
			scanf("%hhu", &stFlickerDetectAttr.u8DetectThr[2]);
			printf("Luma Stable Threshold (0-255): ");
			scanf("%hhu", &stFlickerDetectAttr.u8LumaStableThr);
			printf("Dark Threshold (0-65535): ");
			scanf("%hu", &stFlickerDetectAttr.u16DarkThr);
			run_50hz_60hz_flicker_detect(ViPipe, &stFlickerDetectAttr);
			break;
		case 255:
			printf("Exit flicker detect test.\n");
			break;
		default:
			printf("Invalid option! Please try again.\n");
			break;
		}
	} while (option != 255);
	return 0;
}
