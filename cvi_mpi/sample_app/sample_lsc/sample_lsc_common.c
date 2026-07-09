#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "sample_lsc_cali.h"
#include "cvi_sys.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "cvi_math.h"

#define COMPRESSMODE     7
extern void dpcm_rx(const CVI_U16 *idata, CVI_U16 *odata, CVI_U32 width, CVI_U32 height, CVI_U8 decMode);

static void repack_bit6_to_uint16(CVI_U8 *pu8In6bit, uint16_t *outBuf,
		CVI_U32 u32Width, CVI_U32 u32Height, CVI_U32 u32InStride)
{
	CVI_U8 buffer[3];

	for (CVI_U32 i = 0; i < u32Height; ++i, pu8In6bit += u32InStride) {
		for (CVI_U32 j16 = 0, j6 = 0; j16 < u32Width; j16 += 4, j6 += 3) {
			buffer[0] = pu8In6bit[j6];
			buffer[1] = pu8In6bit[j6 + 1];
			buffer[2] = pu8In6bit[j6 + 2];
			outBuf[j16 + 0] = ((buffer[0] & 0x03) << 4) | (buffer[2] & 0x0F);
			outBuf[j16 + 1] = ((buffer[0] & 0xFC) >> 2);
			outBuf[j16 + 2] = ((buffer[2] & 0xF0) >> 4) | ((buffer[1] & 0x03) << 4);
			outBuf[j16 + 3] = ((buffer[1] & 0xFC) >> 2);
		}
		outBuf += u32Width;
	}
}

static void local_decoder_raw(CVI_U8 *inBuf, uint16_t *outBuf,
			      CVI_U32 width, CVI_U32 height, CVI_U32 stride)
{
	repack_bit6_to_uint16(inBuf, outBuf, width, height, stride);
	dpcm_rx(outBuf, outBuf, width, height, COMPRESSMODE);
}

int get_raw_data(int vi_pipe, CVI_U8 *raw_data, CVI_U32 raw_size)
{
	printf("get raw data\n");
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DUMP_ATTR_S stDumpAttr;
	VIDEO_FRAME_INFO_S stFrame[MAX_FRAME_NUM];

	memset(&stDumpAttr, 0, sizeof(VI_DUMP_ATTR_S));

	stDumpAttr.bEnable = CVI_TRUE;
	stDumpAttr.u32Depth = 0;
	stDumpAttr.enDumpType = VI_DUMP_TYPE_RAW;
	s32Ret = CVI_VI_SetPipeDumpAttr(vi_pipe, &stDumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VI_SetPipeDumpAttr failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	memset(stFrame, 0, sizeof(VIDEO_FRAME_INFO_S) * MAX_FRAME_NUM);
	s32Ret = CVI_VI_GetPipeFrame(vi_pipe, stFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VI_GetPipeFrame failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	usleep(50 * 1000);

	if (stFrame[0].stVFrame.u64PhyAddr[0] == 0) {
		printf("Get invalid raw frame phy addr!\n");
		return CVI_FAILURE;
	}

	if (stFrame[0].stVFrame.u32Length[0] != raw_size) {
		printf("raw size mismatch, alloc size: %u, vFrame size: %u\n",
				raw_size, stFrame[0].stVFrame.u32Length[0]);
		CVI_VI_ReleasePipeFrame(vi_pipe, stFrame);

		return CVI_FAILURE;
	}
	raw_size = stFrame[0].stVFrame.u32Length[0];

	stFrame[0].stVFrame.pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_Mmap(
			stFrame[0].stVFrame.u64PhyAddr[0],
			raw_size
	);

	memcpy((void *)raw_data,
			(const void *)stFrame[0].stVFrame.pu8VirAddr[0],
			raw_size
	);

	CVI_SYS_Munmap((void *)stFrame[0].stVFrame.pu8VirAddr[0], raw_size);

	s32Ret = CVI_VI_ReleasePipeFrame(vi_pipe, stFrame);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VI_ReleasePipeFrame failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

int get_raw_info(int vi_pipe, int vi_chn,
			RAW_PACK_MODE_E *raw_pack_mode, BAYER_FORMAT_E *bayer_format, CVI_U32 *raw_size,
			CVI_U32 *width, CVI_U32 *height)

{
	printf("get raw info\n");
	VI_CHN_ATTR_S stChnAttr;
	VI_DEV_ATTR_S stDevAttr;
	COMPRESS_MODE_E compress_mode;

	if (CVI_VI_GetChnAttr(vi_pipe, vi_chn, &stChnAttr) != CVI_SUCCESS) {
		printf("CVI_VI_GetChnAttr fail!\n");
		return CVI_FAILURE;
	}
	if (CVI_VI_GetDevAttr(vi_pipe, &stDevAttr) != CVI_SUCCESS) {
		printf("CVI_VI_GetDevAttr fail!\n");
		return CVI_FAILURE;
	}

	compress_mode = stChnAttr.enCompressMode;
	*width	= stChnAttr.stSize.u32Width;
	*height	= stChnAttr.stSize.u32Height;
	*bayer_format = stDevAttr.enBayerFormat;

	*raw_size = VI_GetRawBufferSize(*width, *height,
		PIXEL_FORMAT_RGB_BAYER_12BPP, compress_mode, DEFAULT_ALIGN, CVI_FALSE);

	if (compress_mode == COMPRESS_MODE_NONE) {
		*raw_pack_mode = RAW_UNCOMPRESS_PACK;
	} else {
		*raw_pack_mode = RAW_COMPRESS;
	}

	printf("width: %u, height: %u, raw_size: %u, bayer_id: %d, raw_pack_mode: %d\n",
			*width, *height, *raw_size, *bayer_format, *raw_pack_mode);
	return CVI_SUCCESS;
}

int unpack_raw(CVI_U8 *raw_pack, uint16_t *raw_unpack,
		CVI_U32 width, CVI_U32 height, CVI_U32 stride, RAW_PACK_MODE_E raw_pack_mode)

{
	printf("unpack raw...\n");

	if (raw_pack_mode == RAW_UNCOMPRESS_UNPACK) {
		memcpy(raw_unpack, raw_pack, width * height * sizeof(uint16_t));
	} else if (raw_pack_mode == RAW_UNCOMPRESS_PACK) {
		CVI_U8 *line_buffer;
		CVI_U8 v0;
		CVI_U8 v1;
		CVI_U8 v2;
		uint16_t *p_dst = raw_unpack;

		for (CVI_U32 h = 0; h < height; ++h) {
			line_buffer = raw_pack + h * stride;
			for (CVI_U32 w = 0; w < width / 2; ++w) {
				v0 = *line_buffer++;
				v1 = *line_buffer++;
				v2 = *line_buffer++;
				*p_dst++ = ((v0 << 4) | ((v2 >> 0) & 0x0f));
				*p_dst++ = ((v1 << 4) | ((v2 >> 4) & 0x0f));
			}
		}
	} else {
		local_decoder_raw(raw_pack, raw_unpack, width, height, stride);
	}

	return CVI_SUCCESS;
}
