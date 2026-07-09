#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_gdc.h"

#include "ldc_ut_comm.h"

#define LDC_EXT_OP_CLR_VB (1)
#define LDC_DBG_VERPOSE (0)

CVI_S32 GDC_COMM_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		UT_PRT("Null pointer!\n");
		return CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		UT_PRT("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

#if LDC_EXT_OP_CLR_VB
		for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
			if (pstVideoFrame->stVFrame.u32Length[i] == 0)
				continue;
			pstVideoFrame->stVFrame.pu8VirAddr[i] = CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

#if LDC_DBG_VERPOSE
			UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n", i
				, pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i]
				, pstVideoFrame->stVFrame.u32Stride[i], pstVideoFrame->stVFrame.u32Length[i]);
#endif
			memset(pstVideoFrame->stVFrame.pu8VirAddr[i], 0, pstVideoFrame->stVFrame.u32Length[i]);
			CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		}
#endif

	return CVI_SUCCESS;
}

CVI_S32 GDCFileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	CVI_U32 u32len;
	CVI_S32 Ret;
	int i;
	FILE *fp;
	SIZE_S CfgSize;
	CVI_U32 ReadLength;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (!pstVideoFrame) {
		UT_PRT("pstVideoFrame is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	if (!filename) {
		UT_PRT("filename is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	CfgSize.u32Width = ALIGN(stSize->u32Width, DEFAULT_ALIGN);
	CfgSize.u32Height = ALIGN(stSize->u32Height, DEFAULT_ALIGN);
	Ret = GDC_COMM_PrepareFrame(&CfgSize, enPixelFormat, pstVideoFrame);
	if (Ret != CVI_SUCCESS) {
		UT_PRT("GDC_COMM_PrepareFrame FAIL,get VB fail\n");
		return CVI_FAILURE;
	}

	blk = CVI_VB_PhysAddr2Handle(pstVideoFrame->stVFrame.u64PhyAddr[0]);

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		UT_PRT("open data file[%s] error\n", filename);
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

#if LDC_DBG_VERPOSE
		UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i],
			   pstVideoFrame->stVFrame.u32Length[i]);
#endif
		//u32len = fread(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i], 1, fp);
		COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
			, COMPRESS_MODE_NONE, 1, &stVbCalConfig);
		ReadLength = (i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize;
		u32len = fread(pstVideoFrame->stVFrame.pu8VirAddr[i], ReadLength, 1, fp);
		if (u32len <= 0) {
			UT_PRT("file to frame: fread plane%d error\n", i);
			CVI_VB_ReleaseBlock(blk);
			Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	if (Ret)
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

	fclose(fp);

	return Ret;
}

CVI_S32 GDC_COMM_PrepareFrame2(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		UT_PRT("Null pointer!\n");
		return CVI_FAILURE;
	}

	//COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		//, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);
	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, 1, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		UT_PRT("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

#if LDC_EXT_OP_CLR_VB
		for (int i = 0; i < 3; ++i) {
			if (pstVideoFrame->stVFrame.u32Length[i] == 0)
				continue;
			pstVideoFrame->stVFrame.pu8VirAddr[i] = CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

#if LDC_DBG_VERPOSE
			UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
				   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
				   pstVideoFrame->stVFrame.pu8VirAddr[i],
				   pstVideoFrame->stVFrame.u32Stride[i],
				   pstVideoFrame->stVFrame.u32Length[i]);
#endif
			memset(pstVideoFrame->stVFrame.pu8VirAddr[i], 0, pstVideoFrame->stVFrame.u32Length[i]);
			CVI_SYS_IonFlushCache(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
			CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		}
#endif

	return CVI_SUCCESS;
}

CVI_S32 GDCFileToFrame2(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	CVI_U32 u32len;
	CVI_S32 Ret;
	int i;
	FILE *fp;

	if (!pstVideoFrame) {
		UT_PRT("pstVideoFrame is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	if (!filename) {
		UT_PRT("filename is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	Ret = GDC_COMM_PrepareFrame2(stSize, enPixelFormat, pstVideoFrame);
	if (Ret != CVI_SUCCESS) {
		UT_PRT("GDC_COMM_PrepareFrame FAIL,get VB fail\n");
		return CVI_FAILURE;
	}

	blk = CVI_VB_PhysAddr2Handle(pstVideoFrame->stVFrame.u64PhyAddr[0]);

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		UT_PRT("open data file[%s] error\n", filename);
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

#if LDC_DBG_VERPOSE
		UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i],
			   pstVideoFrame->stVFrame.u32Length[i]);
#endif
		u32len = fread(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			UT_PRT("file to frame: fread plane%d error\n", i);
			CVI_VB_ReleaseBlock(blk);
			Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	if (Ret)
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

	fclose(fp);

	return Ret;
}