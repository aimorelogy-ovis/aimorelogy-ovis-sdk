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
#include "cvi_vpss.h"

#include "vpss_ut_comm.h"
#include "vpss_cmodel.h"

//#define FANCY_ENABLE

CVI_S32 FileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize,
		PIXEL_FORMAT_E enPixelFormat, const CVI_CHAR *filename)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
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

	UT_PRT("read file done and send vpss frame.\n");
	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_VPSS_SendFrame fail.\n");

	CVI_VB_ReleaseBlock(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	return s32Ret;
}

//in:rgb888 packed out: yuv444 planar
CVI_VOID RgbTo444(CVI_U8 *pu8InData, CVI_U32 u32Width, CVI_U32 u32Height,
			CVI_U32 u32Stride, CVI_U8 *pu8OutData)
{
	CVI_U32 h, w;
	CVI_U8 *inptr, *y, *u, *v;
	CVI_U8 yuvData[3];

	y = pu8OutData;
	u = pu8OutData + u32Width * u32Height;
	v = pu8OutData + u32Width * u32Height * 2;

	for (h = 0; h < u32Height; h++) {
		inptr = pu8InData + h * u32Stride;

		for (w = 0; w < u32Width; w++) {
			VpssCscRgb2Yuv(inptr, yuvData);
			*y = yuvData[0];
			*u = yuvData[1];
			*v = yuvData[2];

			inptr += 3;
			y++;
			u++;
			v++;
		}
	}
	UT_PRT("---\n");
}

//in:yuv444 planar out: yuv422 planar
CVI_VOID Yuv444To422(CVI_U8 *pu8InData, CVI_U8 *pu8OutData, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U32 h, w;
	CVI_U8 *u, *v, *u_422, *v_422;

	//copy y
	memcpy(pu8OutData, pu8InData, u32Width * u32Height);

	u = pu8InData + u32Width * u32Height;
	v = pu8InData + u32Width * u32Height * 2;

	u_422 = pu8OutData + u32Width * u32Height;
	v_422 = u_422 + u32Width * u32Height / 2;

	for (h = 0; h < u32Height; h++) {
		for (w = 0; w < (u32Width / 2); w++) {
			*u_422++ = (*u + *(u + 1)) / 2;
			*v_422++ = (*v + *(v + 1)) / 2;
			u += 2;
			v += 2;
		}
	}
	UT_PRT("---\n");
}

//in:yuv444 planar out: yuv420 planar
CVI_VOID Yuv444To420(CVI_U8 *pu8InData, CVI_U8 *pu8OutData, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U32 h, w, tmp1, tmp2, tmp;
	CVI_U8 *u, *v, *u_420, *v_420;

	//copy y
	memcpy(pu8OutData, pu8InData, u32Width * u32Height);

	u_420 = pu8OutData + u32Width * u32Height;
	v_420 = u_420 + u32Width * u32Height / 4;

	for (h = 0; h < u32Height; h = h + 2) {
		u = pu8InData + u32Width * u32Height + u32Width * h;
		v = pu8InData + u32Width * u32Height * 2 + u32Width * h;
		for (w = 0; w < (u32Width / 2); w++) {
			tmp1 = *u + *(u + 1);
			tmp1 = (tmp1 > 256) ? (tmp1 + 1) / 2 : tmp1 / 2;
			tmp2 = *(u + u32Width) + *(u + u32Width + 1);
			tmp2 = (tmp2 > 256) ? (tmp2 + 1) / 2 : tmp2 / 2;
			tmp = tmp1 + tmp2;
			*u_420++ = (tmp > 256) ? (tmp + 1) / 2 : tmp / 2;

			tmp1 = *v + *(v + 1);
			tmp1 = (tmp1 > 256) ? (tmp1 + 1) / 2 : tmp1 / 2;
			tmp2 = *(v + u32Width) + *(v + u32Width + 1);
			tmp2 = (tmp2 > 256) ? (tmp2 + 1) / 2 : tmp2 / 2;
			tmp = tmp1 + tmp2;
			*v_420++ = (tmp > 256) ? (tmp + 1) / 2 : tmp / 2;

			u += 2;
			v += 2;
		}
	}
	UT_PRT("---\n");
}

CVI_S32 CompareCmodelRgb2Yuv(VIDEO_FRAME_INFO_S *pstVideoFrameIn, VIDEO_FRAME_INFO_S *pstVideoFrameOut)
{
	CVI_U32 i, h, uv_w_len = 0, uv_h_len = 0;
	CVI_S32 result = CVI_SUCCESS;
	CVI_U8 *y, *u, *v;
	CVI_U8 *yuv444_data = NULL;
	CVI_S32 yuv444_data_len = pstVideoFrameOut->stVFrame.u32Height * pstVideoFrameOut->stVFrame.u32Width * 3;
	CVI_U8 *dst_data = NULL;
	CVI_S32 dst_data_len;
	CVI_U8 *dst_y, *dst_u, *dst_v;

	//rgb packed
	pstVideoFrameIn->stVFrame.pu8VirAddr[0]
		= CVI_SYS_Mmap(pstVideoFrameIn->stVFrame.u64PhyAddr[0], pstVideoFrameIn->stVFrame.u32Length[0]);

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameOut->stVFrame.u32Length[i] == 0)
			continue;

		pstVideoFrameOut->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrameOut->stVFrame.u64PhyAddr[i],
			pstVideoFrameOut->stVFrame.u32Length[i]);

		CVI_SYS_IonInvalidateCache(pstVideoFrameOut->stVFrame.u64PhyAddr[i],
			pstVideoFrameOut->stVFrame.pu8VirAddr[i], pstVideoFrameOut->stVFrame.u32Length[i]);
		UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
				i, pstVideoFrameOut->stVFrame.u64PhyAddr[i],
				pstVideoFrameOut->stVFrame.pu8VirAddr[i],
				pstVideoFrameOut->stVFrame.u32Stride[i],
				pstVideoFrameOut->stVFrame.u32Length[i]);
	}

	yuv444_data = malloc(yuv444_data_len);
	if (!yuv444_data) {
		UT_PRT("malloc fail\n");
		result = CVI_FAILURE;
		goto exit;
	}

	RgbTo444(pstVideoFrameIn->stVFrame.pu8VirAddr[0],
			pstVideoFrameIn->stVFrame.u32Width,
			pstVideoFrameIn->stVFrame.u32Height,
			pstVideoFrameIn->stVFrame.u32Stride[0], yuv444_data);

	if (pstVideoFrameOut->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
		uv_w_len = pstVideoFrameOut->stVFrame.u32Width / 2;
		uv_h_len = pstVideoFrameOut->stVFrame.u32Height / 2;
		dst_data_len = yuv444_data_len / 2;

		dst_data = malloc(dst_data_len);
		if (!dst_data) {
			UT_PRT("malloc fail\n");
			result = CVI_FAILURE;
			goto exit;
		}
		Yuv444To420(yuv444_data, dst_data,
			pstVideoFrameOut->stVFrame.u32Width,
			pstVideoFrameOut->stVFrame.u32Height);
		dst_y = dst_data;
		dst_u = dst_data + pstVideoFrameOut->stVFrame.u32Width * pstVideoFrameOut->stVFrame.u32Height;
		dst_v = dst_u + uv_w_len * uv_h_len;
	} else if (pstVideoFrameOut->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
		uv_w_len = pstVideoFrameOut->stVFrame.u32Width / 2;
		uv_h_len = pstVideoFrameOut->stVFrame.u32Height;
		dst_data_len = (yuv444_data_len / 3) * 2;

		dst_data = malloc(dst_data_len);
		if (!dst_data) {
			UT_PRT("malloc fail\n");
			result = CVI_FAILURE;
			goto exit;
		}
		Yuv444To422(yuv444_data, dst_data,
			pstVideoFrameOut->stVFrame.u32Width,
			pstVideoFrameOut->stVFrame.u32Height);
		dst_y = dst_data;
		dst_u = dst_data + pstVideoFrameOut->stVFrame.u32Width * pstVideoFrameOut->stVFrame.u32Height;
		dst_v = dst_u + uv_w_len * uv_h_len;
	} else if (pstVideoFrameOut->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444) {
		uv_w_len = pstVideoFrameOut->stVFrame.u32Width;
		uv_h_len = pstVideoFrameOut->stVFrame.u32Height;
		dst_y = yuv444_data;
		dst_u = dst_y + pstVideoFrameOut->stVFrame.u32Width * pstVideoFrameOut->stVFrame.u32Height;
		dst_v = dst_u + pstVideoFrameOut->stVFrame.u32Width * pstVideoFrameOut->stVFrame.u32Height;
	} else {
		result = CVI_FAILURE;
		goto exit;
	}

	y = pstVideoFrameOut->stVFrame.pu8VirAddr[0];
	u = pstVideoFrameOut->stVFrame.pu8VirAddr[1];
	v = pstVideoFrameOut->stVFrame.pu8VirAddr[2];

	for (h = 0; h < pstVideoFrameOut->stVFrame.u32Height; h++) {
		if (MemCmp(y, dst_y, pstVideoFrameOut->stVFrame.u32Width)) {
			result = CVI_FAILURE;
			UT_PRT("Y data error, h(%d).\n", h);
			goto exit;
		}
		y += pstVideoFrameOut->stVFrame.u32Stride[0];
		dst_y += pstVideoFrameOut->stVFrame.u32Width;
	}

	for (h = 0; h < uv_h_len; h++) {
		if (MemCmp(u, dst_u, uv_w_len)) {
			result = CVI_FAILURE;
			UT_PRT("U data error, h(%d).\n", h);
			goto exit;
		}
		if (MemCmp(v, dst_v, uv_w_len)) {
			result = CVI_FAILURE;
			UT_PRT("V data error, h(%d).\n", h);
			goto exit;
		}
		u += pstVideoFrameOut->stVFrame.u32Stride[1];
		v += pstVideoFrameOut->stVFrame.u32Stride[2];
		dst_u += uv_w_len;
		dst_v += uv_w_len;
	}

exit:
	if (yuv444_data)
		free(yuv444_data);
	if (dst_data)
		free(dst_data);
	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameOut->stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(pstVideoFrameOut->stVFrame.pu8VirAddr[i], pstVideoFrameOut->stVFrame.u32Length[i]);
	}
	CVI_SYS_Munmap(pstVideoFrameIn->stVFrame.pu8VirAddr[0], pstVideoFrameIn->stVFrame.u32Length[0]);

	return result;
}

#ifdef FANCY_ENABLE
/*fancy upsample*/
CVI_VOID yuv420to444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
		CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U8 v, i;
	CVI_U32 u32Width_uv = u32Width / 2;
	CVI_U32 u32Height_uv = u32Height / 2;
	CVI_U32 in_h_num = 0, out_h_unm = 0;
	CVI_U8 *inptr0, *inptr1, *outptr;
	CVI_U8 *input_data, *output_data;
	CVI_U32 thiscolsum, nextcolsum, lastcolsum;
	CVI_S32 colctr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		input_data = pu8InData[i];
		output_data = pu8OutData + u32Width * u32Height * i;
		out_h_unm = in_h_num = 0;

		while (out_h_unm < u32Height) {
			for (v = 0; v < 2; v++) {
				/* inptr0 points to nearest input row, inptr1 points to next nearest */
				inptr0 = input_data + u32Stride[i] * in_h_num;
				if (v == 0)		/* next nearest is row above */
					inptr1 = inptr0 - (in_h_num ? u32Stride[i] : 0);
				else			/* next nearest is row below */
					inptr1 = inptr0 + ((in_h_num ==  u32Height_uv - 1) ? 0 : u32Stride[i]);

				outptr = output_data +  u32Width * out_h_unm++;
				thiscolsum = (*inptr0++) * 3 + (*inptr1++); /* Special case for first column */
				nextcolsum = (*inptr0++) * 3 + (*inptr1++);
				*outptr++ = ((thiscolsum * 4 + 8) >> 4);
				*outptr++ = ((thiscolsum * 3 + nextcolsum + 7) >> 4);
				lastcolsum = thiscolsum;
				thiscolsum = nextcolsum;

				for (colctr = u32Width_uv - 2; colctr > 0; colctr--) {
					/* General case: 3/4 * nearer pixel + 1/4 * further pixel in each */
					nextcolsum = (*inptr0++) * 3 + (*inptr1++);
					/* dimension, thus 9/16, 3/16, 3/16, 1/16 overall */
					*outptr++ = ((thiscolsum * 3 + lastcolsum + 8) >> 4);
					*outptr++ = ((thiscolsum * 3 + nextcolsum + 7) >> 4);
					lastcolsum = thiscolsum;
					thiscolsum = nextcolsum;
				}
				*outptr++ = ((thiscolsum * 3 + lastcolsum + 8) >> 4); /* Special case for last column */
				*outptr++ = ((thiscolsum * 4 + 7) >> 4);
			}
			in_h_num++;
		}
	}

	UT_PRT("---\n");
}

CVI_VOID yuv422to444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
		CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U32 i, h, invalue;
	CVI_U32 u32Width_uv = u32Width / 2;
	CVI_U8 *inptr, *outptr;
	CVI_U8 *input_data, *output_data;
	CVI_S32 colctr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		input_data = pu8InData[i];
		output_data = pu8OutData + u32Width * u32Height * i;
		for (h = 0; h < u32Height; h++) {
			inptr = input_data + u32Stride[i] * h;
			outptr = output_data + u32Width * h;

			/* Special case for first column */
			invalue = (*inptr++);
			*outptr++ = invalue;
			*outptr++ = ((invalue * 3 + (*inptr) + 2) >> 2);

			for (colctr = u32Width_uv - 2; colctr > 0; colctr--) {
				/* General case: 3/4 * nearer pixel + 1/4 * further pixel */
				invalue = (*inptr++) * 3;
				*outptr++ = ((invalue + (inptr[-2]) + 1) >> 2);
				*outptr++ = ((invalue + (*inptr) + 2) >> 2);
			}

			/* Special case for last column */
			invalue = (*inptr);
			*outptr++ = ((invalue * 3 + (inptr[-1]) + 1) >> 2);
			*outptr++ = invalue;
		}
	}
	UT_PRT("---\n");
}

#else
/*no fancy
u00 u01
u10 u11
right bottom, yuv420 use u11, yuv422 use u01/u11 */
CVI_VOID Yuv420To444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
			CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U32 i, h, w;
	CVI_U8 *inptr, *outptr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		outptr = pu8OutData + u32Width * u32Height * i;
		for (h = 0; h < u32Height; h++) {
			inptr = pu8InData[i] + u32Stride[i] * (h / 2);
			for (w = 0; w < u32Width; w++) {
				*outptr++ = *inptr;
				if (w % 2)
					inptr++;
			}
		}
	}
	UT_PRT("---\n");
}

CVI_VOID Yuv422To444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
			CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U32 i, h, w;
	CVI_U8 *inptr, *outptr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		outptr = pu8OutData + u32Width * u32Height * i;
		for (h = 0; h < u32Height; h++) {
			inptr = pu8InData[i] + u32Stride[i] * h;
			for (w = 0; w < u32Width; w++) {
				*outptr++ = *inptr;
				if (w % 2)
					inptr++;
			}
		}
	}
	UT_PRT("---\n");
}

#endif

CVI_S32 CompareCmodelYuv2rgb(VIDEO_FRAME_INFO_S *pstVideoFrameIn, VIDEO_FRAME_INFO_S *pstVideoFrameOut)
{
	CVI_U32 i, w, h;
	CVI_S32 result = CVI_SUCCESS;
	CVI_U8 yuvData[3], rgbData[3];
	CVI_U8 *p, *y, *u = NULL, *v = NULL;
	CVI_U8 *yuv444_data = NULL;
	CVI_S32 yuv444_data_len = pstVideoFrameIn->stVFrame.u32Height * pstVideoFrameIn->stVFrame.u32Width * 3;

	//rgb packed
	pstVideoFrameOut->stVFrame.pu8VirAddr[0]
		= CVI_SYS_Mmap(pstVideoFrameOut->stVFrame.u64PhyAddr[0], pstVideoFrameOut->stVFrame.u32Length[0]);

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameIn->stVFrame.u32Length[i] == 0)
			continue;

		pstVideoFrameIn->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrameIn->stVFrame.u64PhyAddr[i], pstVideoFrameIn->stVFrame.u32Length[i]);

		UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
				i, pstVideoFrameIn->stVFrame.u64PhyAddr[i],
				pstVideoFrameIn->stVFrame.pu8VirAddr[i],
				pstVideoFrameIn->stVFrame.u32Stride[i],
				pstVideoFrameIn->stVFrame.u32Length[i]);
	}

	if (pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
		yuv444_data = malloc(yuv444_data_len);
		if (!yuv444_data) {
			UT_PRT("malloc fail\n");
			result = CVI_FAILURE;
			goto exit;
		}
		Yuv420To444(pstVideoFrameIn->stVFrame.pu8VirAddr,
					pstVideoFrameIn->stVFrame.u32Width,
					pstVideoFrameIn->stVFrame.u32Height,
					pstVideoFrameIn->stVFrame.u32Stride,
					yuv444_data);
	} else if (pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
		yuv444_data = malloc(yuv444_data_len);
		if (!yuv444_data) {
			UT_PRT("malloc fail\n");
			result = CVI_FAILURE;
			goto exit;
		}
		Yuv422To444(pstVideoFrameIn->stVFrame.pu8VirAddr,
					pstVideoFrameIn->stVFrame.u32Width,
					pstVideoFrameIn->stVFrame.u32Height,
					pstVideoFrameIn->stVFrame.u32Stride,
					yuv444_data);
	}

	for (h = 0; h < pstVideoFrameOut->stVFrame.u32Height; h++) {
		p = pstVideoFrameOut->stVFrame.pu8VirAddr[0] + h * pstVideoFrameOut->stVFrame.u32Stride[0];

		if ((pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420)
			|| (pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422)) {
			y = yuv444_data + pstVideoFrameIn->stVFrame.u32Width * h;
			u = yuv444_data + pstVideoFrameIn->stVFrame.u32Height * pstVideoFrameIn->stVFrame.u32Width
				+ pstVideoFrameIn->stVFrame.u32Width * h;
			v = yuv444_data + pstVideoFrameIn->stVFrame.u32Height * pstVideoFrameIn->stVFrame.u32Width * 2
				+ pstVideoFrameIn->stVFrame.u32Width * h;

		} else {
			y = pstVideoFrameIn->stVFrame.pu8VirAddr[0] + pstVideoFrameIn->stVFrame.u32Stride[0] * h;
			u = pstVideoFrameIn->stVFrame.pu8VirAddr[1] + pstVideoFrameIn->stVFrame.u32Stride[1] * h;
			v = pstVideoFrameIn->stVFrame.pu8VirAddr[2] + pstVideoFrameIn->stVFrame.u32Stride[2] * h;
		}

		for (w = 0; w < pstVideoFrameOut->stVFrame.u32Width; w++) {
			yuvData[0] = *y++;
			yuvData[1] = *u++;
			yuvData[2] = *v++;

			VpssCscYuv2Rgb(yuvData, rgbData);

			if (rgbData[0] != *p) {
				UT_PRT("R data error, yuv(%d %d %d), rgb(sw:%d hw:%d), w:%d h:%d\n",
					yuvData[0], yuvData[1], yuvData[2], rgbData[0], *p, w, h);
				result = CVI_FAILURE;
				break;
			}
			p++;
			if (rgbData[1] != *p) {
				UT_PRT("G data error, yuv(%d %d %d), rgb(sw:%d hw:%d), w:%d h:%d\n",
					yuvData[0], yuvData[1], yuvData[2], rgbData[1], *p, w, h);
				result = CVI_FAILURE;
				break;
			}
			p++;
			if (rgbData[2] != *p) {
				UT_PRT("B data error, yuv(%d %d %d), rgb(sw:%d hw:%d), w:%d h:%d\n",
					yuvData[0], yuvData[1], yuvData[2], rgbData[2], *p, w, h);
				result = CVI_FAILURE;
				break;
			}
			p++;
		}
	}

exit:
	if (yuv444_data)
		free(yuv444_data);
	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameIn->stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(pstVideoFrameIn->stVFrame.pu8VirAddr[i], pstVideoFrameIn->stVFrame.u32Length[i]);
	}

	CVI_SYS_Munmap(pstVideoFrameOut->stVFrame.pu8VirAddr[0], pstVideoFrameOut->stVFrame.u32Length[0]);

	return result;
}

