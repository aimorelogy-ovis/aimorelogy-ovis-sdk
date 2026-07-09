#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <time.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"

#include "md5sum.h"
#include "ut_comm.h"

CVI_S64 GetTimeDiffInUs(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec - t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

CVI_S32 MemCmp(CVI_U8 *pu8Data1, CVI_U8 *pu8Data2, CVI_U32 u32Len)
{
	CVI_U32 i;

	for (i = 0; i < u32Len; i++) {
		if (*pu8Data1 != *pu8Data2) {
			UT_PRT("memcmp fail，index=%d value(%d, %d)\n", i, *pu8Data1, *pu8Data2);
			return CVI_FAILURE;
		}
		pu8Data1++;
		pu8Data2++;
	}

	return CVI_SUCCESS;
}

CVI_CHAR *GetFmtName(PIXEL_FORMAT_E enPixFmt)
{
	switch (enPixFmt) {
	case PIXEL_FORMAT_RGB_888:
		return "rgb";
	case PIXEL_FORMAT_BGR_888:
		return "bgr";
	case PIXEL_FORMAT_RGB_888_PLANAR:
		return "rgbm";
	case PIXEL_FORMAT_BGR_888_PLANAR:
		return "bgrm";
	case PIXEL_FORMAT_YUV_PLANAR_422:
		return "422";
	case PIXEL_FORMAT_YUV_PLANAR_420:
		return "420";
	case PIXEL_FORMAT_YUV_PLANAR_444:
		return "444";
	case PIXEL_FORMAT_YUV_400:
		return "y";
	case PIXEL_FORMAT_HSV_888:
		return "hsv";
	case PIXEL_FORMAT_HSV_888_PLANAR:
		return "hsvm";
	case PIXEL_FORMAT_NV12:
		return "nv12";
	case PIXEL_FORMAT_NV21:
		return "nv21";
	case PIXEL_FORMAT_NV16:
		return "nv16";
	case PIXEL_FORMAT_NV61:
		return "nv61";
	case PIXEL_FORMAT_YUYV:
		return "yuyv";
	case PIXEL_FORMAT_UYVY:
		return "uyvy";
	case PIXEL_FORMAT_YVYU:
		return "yvyu";
	case PIXEL_FORMAT_VYUY:
		return "vyuy";
	case PIXEL_FORMAT_FP32_C3_PLANAR:
		return "fp32";
	case PIXEL_FORMAT_FP16_C3_PLANAR:
		return "fp16";
	case PIXEL_FORMAT_BF16_C3_PLANAR:
		return "bf16";
	case PIXEL_FORMAT_INT8_C3_PLANAR:
		return "int8";
	case PIXEL_FORMAT_UINT8_C3_PLANAR:
		return "uint8";

	default:
		return "unknown";
	}
}

CVI_VOID GetChromaSizeShiftFactor(PIXEL_FORMAT_E enPixelFormat,
		CVI_S32 *w_shift, CVI_S32 *h_shift, CVI_S32 *s32PixelSize, CVI_U32 *u32Planar)
{
	switch (enPixelFormat) {
	case PIXEL_FORMAT_YUV_PLANAR_420:
		*w_shift = 1;
		*h_shift = 1;
		*s32PixelSize = 1;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_422:
		*w_shift = 1;
		*h_shift = 0;
		*s32PixelSize = 1;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_444:
	case PIXEL_FORMAT_RGB_888_PLANAR:
	case PIXEL_FORMAT_BGR_888_PLANAR:
	case PIXEL_FORMAT_HSV_888_PLANAR:
	case PIXEL_FORMAT_INT8_C3_PLANAR:
	case PIXEL_FORMAT_UINT8_C3_PLANAR:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 1;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_RGB_888:
	case PIXEL_FORMAT_BGR_888:
	case PIXEL_FORMAT_HSV_888:
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 3;
		*u32Planar = 1;
		break;
	case PIXEL_FORMAT_NV12:
	case PIXEL_FORMAT_NV21:
		*w_shift = 0;
		*h_shift = 1;
		*s32PixelSize = 1;
		*u32Planar = 2;
		break;
	case PIXEL_FORMAT_NV16:
	case PIXEL_FORMAT_NV61:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 1;
		*u32Planar = 2;
		break;
	case PIXEL_FORMAT_YUYV:
	case PIXEL_FORMAT_UYVY:
	case PIXEL_FORMAT_YVYU:
	case PIXEL_FORMAT_VYUY:
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 2;
		*u32Planar = 1;
		break;
	case PIXEL_FORMAT_YUV_400: // no chroma
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 1;
		*u32Planar = 1;
		break;
	case PIXEL_FORMAT_FP32_C3_PLANAR:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 4;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_FP16_C3_PLANAR:
	case PIXEL_FORMAT_BF16_C3_PLANAR:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 2;
		*u32Planar = 3;
		break;
	default:
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 1;
		*u32Planar = 1;
		break;
	}
}

CVI_S32 FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
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
		UT_PRT("CVI_VB_GetBlock fail\n");
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
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[i],
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

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	memcpy(pstVideoFrame, &stVideoFrame, sizeof(stVideoFrame));

	return CVI_SUCCESS;
}

CVI_S32 FrameFullSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		UT_PRT("open data file(%s) error\n", filename);
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
			pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		UT_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			UT_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return s32Ret;
}

CVI_S32 FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 i;
	CVI_S32 c_w_shift, c_h_shift; // chroma width/height shift
	CVI_S32 s32PixelSize;
	CVI_U32 u32Planar;
	CVI_U8 *w_ptr;
	CVI_U32 image_size = 0;
	CVI_S32 plane_offset = 0;
	CVI_VOID *vir_addr = NULL;
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		UT_PRT("open data file(%s) error\n", filename);
		return CVI_FAILURE;
	}

	image_size = pstVFrame->u32Length[0] + pstVFrame->u32Length[1] + pstVFrame->u32Length[2];
	vir_addr = CVI_SYS_Mmap(pstVFrame->u64PhyAddr[0], image_size);
	CVI_SYS_IonInvalidateCache(pstVFrame->u64PhyAddr[0], vir_addr, image_size);

	for (i = 0; i < 3; i++) {
		if (pstVFrame->u32Length[i] == 0)
			continue;
		pstVFrame->pu8VirAddr[i] = vir_addr + plane_offset;
		plane_offset += pstVFrame->u32Length[i];
	}

	UT_PRT("u32Width = %d, u32Height = %d\n", pstVFrame->u32Width, pstVFrame->u32Height);
	UT_PRT("u32Stride[0] = %d, u32Stride[1] = %d, u32Stride[2] = %d\n",
		 pstVFrame->u32Stride[0], pstVFrame->u32Stride[1], pstVFrame->u32Stride[2]);
	UT_PRT("u32Length[0] = %d, u32Length[1] = %d, u32Length[2] = %d\n",
		 pstVFrame->u32Length[0], pstVFrame->u32Length[1], pstVFrame->u32Length[2]);

	GetChromaSizeShiftFactor(pstVFrame->enPixelFormat, &c_w_shift,
					  &c_h_shift, &s32PixelSize, &u32Planar);

	// save Y
	w_ptr = pstVFrame->pu8VirAddr[0];
	for (i = 0; i < pstVFrame->u32Height; i++) {
		fwrite(w_ptr + i * pstVFrame->u32Stride[0], s32PixelSize,
		       pstVFrame->u32Width, fp);
	}
	// save U
	if (u32Planar >= 2) {
		w_ptr = pstVFrame->pu8VirAddr[1];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			fwrite(w_ptr + i * pstVFrame->u32Stride[1], s32PixelSize,
			       pstVFrame->u32Width >> c_w_shift, fp);
		}
	}
	// save V
	if (u32Planar >= 3) {
		w_ptr = pstVFrame->pu8VirAddr[2];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			fwrite(w_ptr + i * pstVFrame->u32Stride[2], s32PixelSize,
			       pstVFrame->u32Width >> c_w_shift, fp);
		}
	}

	CVI_SYS_Munmap(vir_addr, image_size);
	fclose(fp);

	return 0;
}

CVI_S32 CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, plane_len, data_len;
	CVI_U32 u32LumaData, u32ChromaData = 0, data_height;
	CVI_S32 result = CVI_SUCCESS;
	VB_CAL_CONFIG_S stVbCalConfig;

	u32LumaData = pstVideoFrame->stVFrame.u32Width;
	data_height = pstVideoFrame->stVFrame.u32Height;

	COMMON_GetPicBufferConfig(pstVideoFrame->stVFrame.u32Width, pstVideoFrame->stVFrame.u32Height,
		pstVideoFrame->stVFrame.enPixelFormat, DATA_BITWIDTH_8,
		COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_BGR_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21) {
		u32ChromaData = u32LumaData;
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV16 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV61) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUYV ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_UYVY ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YVYU ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_VYUY) {
		u32LumaData *= 2;
		u32ChromaData = 0;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_400) {
		u32ChromaData = 0;
	}

	UT_PRT("u32LumaSize(%d): u32ChromaSize(%d)\n",
		stVbCalConfig.u32MainYSize, stVbCalConfig.u32MainCSize);
	UT_PRT("u32LumaData(%d): u32ChromaData(%d)\n", u32LumaData, u32ChromaData);
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		UT_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	CVI_U8 buffer[stVbCalConfig.u32MainYSize];
	CVI_U32 offset = 0;

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		plane_len = (i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize;
		if (plane_len == 0)
			continue;
		data_len = (i == 0) ? u32LumaData : u32ChromaData;
		offset = 0;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
			pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		u32len = fread(buffer, plane_len, 1, fp);
		if (u32len <= 0) {
			UT_PRT("fread data(%d) error\n", i);
			result = CVI_FAILURE;
			break;
		}
		// line by line check to avoid padding data mismatch problem.
		for (CVI_U32 line = 0; line < data_height; ++line) {
			if (memcmp(buffer + offset, pstVideoFrame->stVFrame.pu8VirAddr[i] + offset, data_len) != 0) {
				UT_PRT("plane(%d) line(%d) offset(%d) data mismatch:\n",
					      i, line, offset);
				UT_PRT(" paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
					      pstVideoFrame->stVFrame.u64PhyAddr[i],
					      pstVideoFrame->stVFrame.pu8VirAddr[i],
					      pstVideoFrame->stVFrame.u32Stride[i]);

				result = CVI_FAILURE;
				break;
			}
			offset += pstVideoFrame->stVFrame.u32Stride[i];
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return result;
}

CVI_S32 CompareWithMD5(const CVI_CHAR *md5sum, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 result = CVI_SUCCESS;
	CVI_S32 c_w_shift, c_h_shift; // chroma width/height shift
	CVI_S32 s32PixelSize;
	CVI_U32 i, u32Planar;
	CVI_U8 *w_ptr;
	CVI_U32 image_size = 0;
	CVI_S32 plane_offset = 0;
	CVI_VOID *vir_addr = NULL;
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	MD5_CTX md5_ctx;
	CVI_CHAR md[MD5_DIGEST_LENGTH];
	CVI_CHAR md_str[33];
	CVI_CHAR *p = md_str;
	CVI_S32 s32Index = 0;

	memset(md, 0, MD5_DIGEST_LENGTH);

	image_size = pstVFrame->u32Length[0] + pstVFrame->u32Length[1] + pstVFrame->u32Length[2];
	vir_addr = CVI_SYS_Mmap(pstVFrame->u64PhyAddr[0], image_size);
	CVI_SYS_IonInvalidateCache(pstVFrame->u64PhyAddr[0], vir_addr, image_size);

	for (i = 0; i < 3; i++) {
		if (pstVFrame->u32Length[i] == 0)
			continue;
		pstVFrame->pu8VirAddr[i] = vir_addr + plane_offset;
		plane_offset += pstVFrame->u32Length[i];
	}

	GetChromaSizeShiftFactor(pstVFrame->enPixelFormat, &c_w_shift,
					  &c_h_shift, &s32PixelSize, &u32Planar);
	MD5_Init(&md5_ctx);

	// Compare Y
	w_ptr = pstVFrame->pu8VirAddr[0];
	for (i = 0; i < pstVFrame->u32Height; i++) {
		MD5_Update(&md5_ctx, w_ptr + i * pstVFrame->u32Stride[0],
			s32PixelSize * pstVFrame->u32Width);
	}
	// Compare U
	if (u32Planar >= 2) {
		w_ptr = pstVFrame->pu8VirAddr[1];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			MD5_Update(&md5_ctx, w_ptr + i * pstVFrame->u32Stride[1],
				s32PixelSize * (pstVFrame->u32Width >> c_w_shift));
		}
	}
	// Compare V
	if (u32Planar >= 3) {
		w_ptr = pstVFrame->pu8VirAddr[2];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			MD5_Update(&md5_ctx, w_ptr + i * pstVFrame->u32Stride[2],
				s32PixelSize * (pstVFrame->u32Width >> c_w_shift));
		}
	}

	MD5_Final((unsigned char *)md, &md5_ctx);
	CVI_SYS_Munmap(vir_addr, image_size);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		s32Index += snprintf(p + s32Index, 33 - s32Index, "%02x", md[i]);

	if (strncmp(md5sum, md_str, 32)) {
		UT_PRT("md5sum error, frame md5sum:%s\n", md_str);
		result = CVI_FAILURE;
	}

	return result;
}
