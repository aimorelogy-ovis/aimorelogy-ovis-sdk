#ifndef __UT_COMM_H__
#define __UT_COMM_H__

#include "cvi_type.h"
#include "cvi_comm_video.h"
#include <time.h>

#include "md5sum.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

#define UT_PRT(fmt...)                               \
	do {                                                  \
		printf("[%s]-%d: ", __func__, __LINE__);          \
		printf(fmt);                                      \
	} while (0)

#define UT_CHECK_CASE_RET(s32Ret) \
	do { \
		sleep(1); \
		if (s32Ret == CVI_SUCCESS) \
			printf(GREEN"\n=== %s pass ===\n"NONE"\n", __func__); \
		else \
			printf(RED"\n=== %s fail ===\n"NONE"\n", __func__); \
		sleep(1); \
	} while (0)

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif

#ifndef BIT
#define BIT(nr)      (UINT64_C(1) << (nr))
#endif

/* 函数声明 */
CVI_S64 GetTimeDiffInUs(struct timespec t1, struct timespec t2);
CVI_S32 MemCmp(CVI_U8 *pu8Data1, CVI_U8 *pu8Data2, CVI_U32 u32Len);
CVI_CHAR *GetFmtName(PIXEL_FORMAT_E enPixFmt);
CVI_VOID GetChromaSizeShiftFactor(PIXEL_FORMAT_E enPixelFormat,
        CVI_S32 *w_shift, CVI_S32 *h_shift, CVI_S32 *s32PixelSize, CVI_U32 *u32Planar);
CVI_S32 FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
	CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 FrameFullSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 CompareWithMD5(const CVI_CHAR *md5sum, VIDEO_FRAME_INFO_S *pstVideoFrame);

#endif /* __UT_COMM_H__ */
