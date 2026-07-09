#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include <cvi_common.h>
#include <cvi_comm_vpss.h>
#include <cvi_comm_vb.h>
#include "cvi_region.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"

#include "fontmod.h"
#include "rgn_ut_fun.h"

#define VPSS_FILENAME_IN   "res/rgn/golden.yuv422"
#define VPSS_FILENAME_IN1  "res/rgn/vpss_3840x2160_rgb888.bin"
#define VPSS_FILENAME_OUT  "output"

#define colorbar_bmp      "res/rgn/colorbar.bmp"
#define dog_bmp           "res/rgn/dog.bmp"
#define tiger_bmp         "res/rgn/tiger.bmp"
#define tiger_8bitmode    "res/rgn/tiger_8bitmode.bmp"

#define test_bmp          tiger_bmp
#define test_hw_bs        "res/rgn/hw_bs_1280x720.bin"
#define test_sw_bs        "res/rgn/sw_bs_1280x720.bin"
#define BYTE_BITS               8
#define NOASCII_CHARACTER_BYTES 2
#define OSD_LIB_FONT_W          24
#define OSD_LIB_FONT_H          24

#define dog_argb8888_bin          "res/rgn/dog_s_72x60_pngto8888.bin"
#define dog_argb4444_bin          "res/rgn/dog_s_80x60_pngto1555.bin"
#define dog_argb1555_bin          "res/rgn/dog_s_80x60_pngto4444.bin"
#define dog_4bitmode_bin          "res/rgn/dog_s_96x60_4bit_pngtoLUT.bin"
#define dog_4bitmode_lut_bin      "res/rgn/dog_s_96x60_4bit_pngLUT.bin"
#define dog_8bitmode_bin          "res/rgn/dog_s_96x60_8bit_pngtoLUT.bin"
#define dog_8bitmode_lut_bin      "res/rgn/dog_s_96x60_8bit_pngLUT.bin"
#define fontmode_bin              "res/rgn/fb_box_384x24x16.bin"
#define argb8888_2160_bin         "res/rgn/1920x2160_argb8888.bin"
#define argb8888_1080_bin         "res/rgn/960x1080_argb8888.bin"

#define RGN_VPSS_REF_MD5			"f39400d5a57f92467b72c100d3e8b179"
#define RGN_SET_BITMAP_REF_MD5			"d7a0d15f2e805386d0f5c3b4d221ddbf"
#define RGN_OVERLAY_UPDATE_CANVAS_REF_MD5	"d7a0d15f2e805386d0f5c3b4d221ddbf"
#define RGN_COVER_UPDATE_CANVAS_REF_MD5		"c910c0ac4a5fb1efb69c7840665c7f4f"
#define RGN_UPDATE_8BIT_MODE_REF_MD5		"d049f2bdb01619415b1e42fbf69363ed"
#define RGN_HW_CMPR_GETUPCANVAS_REF_MD5		"84912c7791210ba66c98526b2a30debb"
#define RGN_FORMATS_REF_MD5			"a0ed357ccd3a5dd5e44df05d47bd9ab1"
#define RGN_OVERLAY_SC_V1_CAPABILITY_REF_MD5	"d4dedb92258c11c4c5da9a6b3ca6245f"
#define RGN_COVER_SC_V1_CAPABILITY_REF_MD5	"cb972b7c48ab654e14aa00c5381907c9"
#define RGN_OVERLAY_SC_V2_CAPABILITY_REF_MD5	"d18bec0a38b532dcfc01fb9aece1c965"
#define RGN_COVER_SC_V2_CAPABILITY_REF_MD5	"3f37506c6424846d69f944a5703dc8c6"
#define RGN_OVERLAY_MIX_CAPABILITY_REF_MD5	"8b1b00fb4d6d2945fe5ecb1af1ee831c"
#define RGN_COVER_MIX_CAPABILITY_REF_MD5	"074b3f305ce998471af57f4528bad441"

#define RGN_VPSS_COVEREX_REF_MD5    "e40e35cefca8cca23148eae728cb351a"

#define VPSS_1_TO_4_CHN0_MD5        "17c3d23164400d9b779a76d94e741df8"
#define VPSS_1_TO_4_CHN1_MD5        "3129a77396e15c3e550ccc103aa7642a"
#define VPSS_1_TO_4_CHN2_MD5        "c09c95b6ce1d18719133314938d37f36"
#define VPSS_1_TO_4_CHN3_MD5        "a48c44a5c4d907ed4a4fd3c8da1451a2"

#define OverlayMinHandle        0
#define OverlayExMinHandle      20
#define CoverMinHandle          40
#define CoverExMinHandle        60
#define MosaicMinHandle         80
#define OdecHandle              100

#define IsASCII(a)    (((a) >= 0x00 && (a) <= 0x7F) ? 1 : 0)
#define MAX_STR_LEN  (64)

#define RGN_UT_ARGB8888 0
#define RGN_UT_ARGB1555 1
#define RGN_UT_ARGB4444 2

#ifndef FPGA_PORTING
#define UT_TIMEOUT_MS 1000
#else
#define UT_TIMEOUT_MS 60000
#endif
struct VPSS_CHN_PARAM {
	CVI_BOOL bEnable;
	VPSS_CHN VpssChn;
	SIZE_S stSizeOut;
	CVI_BOOL bMirror;
	CVI_BOOL bFlip;
	PIXEL_FORMAT_E enFormatOut;
	ASPECT_RATIO_S stAspectRatio;
	VPSS_NORMALIZE_S stNormalize;
	CVI_U32 u32CheckSum;
	CVI_CHAR aszMD5Sum[33];
	CVI_CHAR aszFileNameOut[64];
	CVI_CHAR aszFileNameRef[64];
};

typedef struct _VPSS_MULTI_TEST_PARAM {
	VPSS_GRP VpssGrp;
	SIZE_S stSizeIn;
	PIXEL_FORMAT_E enFormatIn;
	CVI_CHAR aszFileNameIn[64];
	struct VPSS_CHN_PARAM astChnParam[VPSS_MAX_CHN_NUM];
} VPSS_MULTI_TEST_PARAM;
typedef struct _RGN_TEST_PARAM {
	// normal rgn
	CVI_S32 u32HdlNum;
	RGN_TYPE_E enType;

	// compressed rgn
	CVI_U32 u32OdecFileSize;
	SIZE_S stOdecSize;

	// vpss settings
	MMF_CHN_S stChn;
	SIZE_S stInputSize;
	SIZE_S stOutputSize;
	PIXEL_FORMAT_E eInputFmt;
	PIXEL_FORMAT_E eOutputFmt;

	// input/output file settings
	CVI_CHAR *inputFile;
	CVI_CHAR *outputFile;
	CVI_CHAR *refFile;
	CVI_CHAR *aszMD5Sum;
	CVI_U32 u32RepeatCnt;
} RGN_TEST_PARAM;

typedef enum _RGN_TEST_OP {
	RGN_VPSS_INIT_START_SEND_FRAME_TEST,
	RGN_BIT_MAP_WITH_VPSS_SEND_FRAME_TEST,
	RGN_CANVAS_WITH_VPSS_SEND_FRAME_TEST,
	RGN_8BIT_MODE_CANVAS_WITH_VPSS_SEND_FRAME_TEST,
	RGN_HW_COMPRESS_SIMPLE_OBJECTS_TEST,
	RGN_HW_COMPRESS_SIMPLE_OBJECTS_AND_BITMAP_TEST, //5
	RGN_VPSS_COVEREX_TEST,
	RGN_VPSS_MOSAIC_TEST,
	RGN_CREATE_DESTROY = 50,
	RGN_CREATE_ATTACTH_DETACH_DESTROY,
	RGN_UPDATE_ATTR_TEST,
	RGN_UPDATE_ATTACH_ATTR_TEST,
	// sw verification
	RGN_VPSS_FORMATS_TEST = 70,
	RGN_VPSS_SC_V1_CAPABILITY_TEST,
	RGN_VPSS_SC_V2_CAPABILITY_TEST,
	RGN_VPSS_MIX_CAPABILITY_TEST,
	RGN_INVERT_TEST,
	RGN_ALL_HW_TEST,
	RGN_PERF_TEST,
	RGN_TEST_AUTO_REGRESSION = 99,
	RGN_UT_CFG,
} RGN_TEST_OP;

CVI_S32 rgn_fd = -1;
static CVI_U8 is_enable = 1;
CVI_CHAR *Path_BMP;

static CVI_S32 rgn_get_font_mod(CVI_CHAR *Character, CVI_U8 **FontMod, CVI_S32 *FontModLen)
{
	CVI_U32 offset = 0;
	CVI_U32 areacode = 0;
	CVI_U32 bitcode = 0;

	if (IsASCII(Character[0])) {
		areacode = 3;
		bitcode = (CVI_U32)((CVI_U8)Character[0] - 0x20);
	} else {
		areacode = (CVI_U32)((CVI_U8)Character[0] - 0xA0);
		bitcode = (CVI_U32)((CVI_U8)Character[1] - 0xA0);
	}
	offset = (94 * (areacode - 1) + (bitcode - 1)) * (OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8);
	*FontMod = (CVI_U8 *)g_fontLib + offset;
	*FontModLen = OSD_LIB_FONT_W*OSD_LIB_FONT_H / 8;
	return CVI_SUCCESS;
}

static CVI_S32 rgn_get_non_asc_num(CVI_CHAR *string, CVI_S32 len)
{
	CVI_S32 i;
	CVI_S32 n = 0;

	for (i = 0; i < len; i++) {
		if (string[i] == '\0')
			break;
		if (!IsASCII(string[i])) {
			i++;
			n++;
		}
	}

	return n;
}

void rgn_get_time_str(const struct tm *pstTime, CVI_CHAR *pazStr, CVI_S32 s32Len)
{
	time_t nowTime;
	struct tm stTime = {
		0,
	};

	if (!pstTime) {
		time(&nowTime);
		localtime_r(&nowTime, &stTime);
		pstTime = &stTime;
	}

	snprintf(pazStr, s32Len, "%04d-%02d-%02d %02d:%02d:%02d",
		pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday,
		pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
}

CVI_S32 rgn_time_bit_map(CVI_CHAR *szStr, BITMAP_S *pstBitmap, CVI_U32 u32Color, CVI_U32 u32BgColor)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32CanvasWidth, u32CanvasHeight;
	SIZE_S stFontSize;
	CVI_S32 s32StrLen = strnlen(szStr, MAX_STR_LEN);
	CVI_S32 NonASCNum = rgn_get_non_asc_num(szStr, s32StrLen);

	u32CanvasWidth = OSD_LIB_FONT_W * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
	u32CanvasHeight = OSD_LIB_FONT_H;
	stFontSize.u32Width = OSD_LIB_FONT_W;
	stFontSize.u32Height = OSD_LIB_FONT_H;

	pstBitmap->u32Width = u32CanvasWidth;
	pstBitmap->u32Height = u32CanvasHeight;
	pstBitmap->pData = malloc(2 * (pstBitmap->u32Width) * (pstBitmap->u32Height));
	if (pstBitmap->pData == NULL)
		UT_PRT("malloc osd memroy err!\n");

	CVI_U16 *puBmData = (CVI_U16 *)pstBitmap->pData;
	CVI_U32 u32BmRow, u32BmCol;

	for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
		CVI_S32 NonASCShow = 0;

		for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
			CVI_S32 s32BmDataIdx = u32BmRow * pstBitmap->u32Width + u32BmCol;
			CVI_S32 s32CharIdx = u32BmCol / stFontSize.u32Width;
			CVI_S32 s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);

			if (NonASCNum > 0 && s32CharIdx > 0) {
				NonASCShow = rgn_get_non_asc_num(szStr, s32StringIdx);
				s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
			}
			CVI_S32 s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
							stFontSize.u32Width;
			CVI_S32 s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
			CVI_S32 s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
			CVI_S32 s32BitOffset = s32CharCol % BYTE_BITS;
			CVI_U8 *FontMod = NULL;
			CVI_S32 FontModLen = 0;

			if (rgn_get_font_mod(&szStr[s32StringIdx], &FontMod, &FontModLen) == CVI_SUCCESS) {
				if (FontMod != NULL && s32HexOffset < FontModLen) {
					CVI_U8 temp = FontMod[s32HexOffset];

					if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1)
						puBmData[s32BmDataIdx] = (CVI_U16)u32Color;
					else
						puBmData[s32BmDataIdx] = (CVI_U16)u32BgColor;
					continue;
				}
			}
			UT_PRT("GetFontMod Fail\n");
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

static void dump_mem(VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_U32 size)
{
	CVI_U32 u32DataLen;

	for (CVI_S32 i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		u32DataLen = size ? size : u32DataLen;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			      (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			      (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
				= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i],
					pstVideoFrame->stVFrame.u32Length[i]);

		UT_PRT("plane(%d): paddr(0x%llx) vaddr(0x%llx) stride(%d)\n", i,
			   (unsigned long long)pstVideoFrame->stVFrame.u64PhyAddr[i],
			   (unsigned long long)(intptr_t)pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		UT_PRT(" data_len(%d) plane_len(%d)\n",
			   u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);

		for (CVI_U32 j = 0; j < u32DataLen/16; j += 16) {
			CVI_U32 *buf = (CVI_U32 *)(pstVideoFrame->stVFrame.pu8VirAddr[i] + j);

			UT_PRT("[%d]%04x: %08x %08x %08x %08x\n",
				   i, j, buf[0], buf[1], buf[2], buf[3]);
		}

		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}
}

static inline CVI_S32 rgn_vb_init(RGN_TEST_PARAM *param)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize = { .u32Width = param->stInputSize.u32Width,
		.u32Height = param->stInputSize.u32Height};
	SIZE_S stSizeOut = { .u32Width = param->stOutputSize.u32Width,
		.u32Height = param->stOutputSize.u32Height };
	PIXEL_FORMAT_E	pixelFormat = param->eInputFmt;
	PIXEL_FORMAT_E	pixelFormatOut = param->eOutputFmt;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	VB_CONFIG_S	stVbConf;
	CVI_U32		u32BlkSize, u32BlkSizeOut;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt		= 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, pixelFormat,
					     DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height, pixelFormatOut,
						DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	u32BlkSize = u32BlkSize > u32BlkSizeOut ? u32BlkSize : u32BlkSizeOut;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 5;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = RGN_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("system init failed with %#x\n", s32Ret);
		return -1;
	}
	return s32Ret;
}

static inline CVI_S32 rgn_vpss_init(RGN_TEST_PARAM *param)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize = { .u32Width = param->stInputSize.u32Width,
		.u32Height = param->stInputSize.u32Height};
	SIZE_S stSizeOut = { .u32Width = param->stOutputSize.u32Width,
		.u32Height = param->stOutputSize.u32Height };
	PIXEL_FORMAT_E	pixelFormat = param->eInputFmt;
	PIXEL_FORMAT_E	pixelFormatOut = param->eOutputFmt;

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP	VpssGrp = param->stChn.s32DevId;
	VPSS_GRP_ATTR_S	stVpssGrpAttr = {0};
	VPSS_CHN	VpssChn = param->stChn.s32ChnId;
	CVI_BOOL	abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S	astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

	// grp0 for right half
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].u32Height		    = stSizeOut.u32Height;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = pixelFormatOut;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[VpssChn] = CVI_TRUE;
	s32Ret = RGN_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = RGN_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static CVI_S32 rgn_ut_vpss_send_frame(RGN_TEST_PARAM *param)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = param->stChn.s32DevId;
	VPSS_CHN VpssChn = param->stChn.s32ChnId;
	SIZE_S stSize = param->stInputSize;
	PIXEL_FORMAT_E pixelFormat = param->eInputFmt;
	CVI_CHAR *fileName = param->inputFile;
	CVI_CHAR *fileNameOut = param->outputFile;
	CVI_CHAR *aszMD5Sum = param->aszMD5Sum;
	CVI_U32 i;
	CVI_BOOL b_rgn_save_file = CVI_TRUE;

	for (i = 0; i < param->u32RepeatCnt; i++) {
		VIDEO_FRAME_INFO_S stVideoFrame;

		s32Ret = RGN_COMM_VPSS_SendFrame(VpssGrp, &stSize, pixelFormat, fileName);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("[%d] RGN_COMM_VPSS_SendFrame fail. s32Ret: 0x%x !\n",
				i, s32Ret);
			break;
		}

		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000/*-1*/);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("[%d] CVI_VPSS_GetChnFrame. s32Ret: 0x%x !\n",
				i, s32Ret);
			break;
		}

		dump_mem(&stVideoFrame, 32);
		if (aszMD5Sum[0]) {
			if (CompareWithMD5(aszMD5Sum, &stVideoFrame)) {
				b_rgn_save_file = CVI_TRUE;
				s32Ret = CVI_FAILURE;
				UT_PRT("Compare MD5 fail, MD5:%s\n", param->aszMD5Sum);
			} else {
				b_rgn_save_file = CVI_FALSE;
			}
		}

		if (b_rgn_save_file) {
			if (RGN_COMM_FRAME_SaveToFile(fileNameOut, &stVideoFrame) != CVI_SUCCESS) {
				UT_PRT("[%d] RGN_COMM_FRAME_SaveToFile!\n", i);
			}
		}

		if (CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame) != CVI_SUCCESS) {
			UT_PRT("[%d] CVI_VPSS_ReleaseChnFrame!\n", i);
			break;
		}
	}
	return s32Ret;
}

static CVI_S32 rgn_vpss_sendframe_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto ERR_VPSS_COMBINE;
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "1280_720_nv21_out2.yuv");
	snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_VPSS_REF_MD5);

	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;

	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
ERR_VPSS_COMBINE:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 RGN_COMM_REGION_SetBitMap(RGN_HANDLE Handle, const char *filename,
	PIXEL_FORMAT_E pixelFormat)
{
	CVI_S32 s32Ret;
	BITMAP_S stBitmap;

	RGN_COMM_REGION_MST_LoadBmp(filename, &stBitmap, CVI_FALSE, 0, pixelFormat);

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS)
		printf("REGION_SetBitMap failed!Handle:%d\n", Handle);
	free(stBitmap.pData);
	return s32Ret;
}

static CVI_S32 rgn_set_bitmap_with_vpss_sendframe_test(CVI_S32 s32ChnId)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 MinHandle, i;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = s32ChnId; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[s32ChnId] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 3;
	param.enType = OVERLAY_RGN;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	if (param.enType == OVERLAY_RGN || param.enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(param.enType);

		for (i = MinHandle; i < MinHandle + param.u32HdlNum; i++) {
			s32Ret = RGN_COMM_REGION_SetBitMap(i, Path_BMP, enPixelFormat);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("RGN_COMM_REGION_SetBitMap failed!\n");
				goto EXIT3;
			}
		}
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "1280_720_nv21_out_set_bitmap.yuv");
	snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_SET_BITMAP_REF_MD5);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_update_canvas_with_vpss_sendframe_fun_test(RGN_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 MinHandle, i;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	SIZE_S stSize = {0};
	BITMAP_S stBitmap = {0};
	RGN_CANVAS_INFO_S stCanvasInfo = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 3;
	param.enType = enType;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	if (param.enType == OVERLAY_RGN || param.enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(param.enType);
		for (i = MinHandle; i < MinHandle + param.u32HdlNum; i++) {
			s32Ret = CVI_RGN_GetCanvasInfo(i, &stCanvasInfo);
			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
				goto EXIT3;
			}
			stBitmap.pData = stCanvasInfo.pu8VirtAddr;
			stSize.u32Width = stCanvasInfo.stSize.u32Width;
			stSize.u32Height = stCanvasInfo.stSize.u32Height;

			RGN_COMM_REGION_MST_UpdateCanvas(Path_BMP, &stBitmap, CVI_FALSE, 0, &stSize,
						stCanvasInfo.u32Stride, PIXEL_FORMAT_ARGB_1555);
			s32Ret = CVI_RGN_UpdateCanvas(i);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
				goto EXIT3;
			}
		}
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	if (enType == OVERLAY_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "1280_720_nv21_out_overlay_update_canvas.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_OVERLAY_UPDATE_CANVAS_REF_MD5);
	} else if (enType == COVER_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "1280_720_nv21_out_cover_update_canvas.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_COVER_UPDATE_CANVAS_REF_MD5);
	}

	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_update_canvas_with_vpss_sendframe_test(void)
{
	RGN_TYPE_E enRgnTypes[2] = {OVERLAY_RGN, COVER_RGN};
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 u32TypeCnt = ARRAY_SIZE(enRgnTypes);

	for (CVI_S32 i = 0; i < u32TypeCnt; i++) {
		s32Ret |= rgn_update_canvas_with_vpss_sendframe_fun_test(enRgnTypes[i]);
	}
	return s32Ret;
}

static CVI_S32 rgn_8bit_mode_canvas_with_vpss_sendframe_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_PALETTE_S stPalette;
	RGN_TEST_PARAM param;
	CVI_S32 i;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN 8bit mode
	 ************************************************/
	Path_BMP = tiger_8bitmode;
	param.u32HdlNum = 3;
	param.enType = OVERLAY_RGN;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_8BIT_MODE;
	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	/* Use indexed palettes format of bmp file in OVERLAY example. */
	for (i = OverlayMinHandle; i < OverlayMinHandle + param.u32HdlNum; i++) {
		s32Ret = RGN_COMM_REGION_SetBitMap(i, Path_BMP, enPixelFormat);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("RGN_COMM_REGION_SetBitMap failed!\n");
			goto EXIT3;
		}
	}

	stPalette.pstPaletteTable = overlay_palette;
	stPalette.lut_length = 256;
	stPalette.pixelFormat = RGN_COLOR_FMT_RGB888;
	s32Ret = CVI_RGN_SetChnPalette(OverlayMinHandle, &param.stChn, &stPalette);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_SetChnPalette failed!\n");
		goto EXIT3;
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "1280_720_nv21_out_update_8bit_mode_canvas.yuv");
	snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_UPDATE_8BIT_MODE_REF_MD5);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_coverex_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	RGN_HANDLE Handle;
	RGN_CHN_ATTR_S stRgnChnAttr;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 4;
	param.enType = COVEREX_RGN;

	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, 0);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	Handle = RGN_COMM_REGION_GetMinHandle(param.enType) + 3;

	CVI_RGN_GetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);
	stRgnChnAttr.unChnAttr.stCoverExChn.u32Color = 0x00ff0000;
	stRgnChnAttr.unChnAttr.stCoverExChn.u32Layer = 0;
	stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32X = 500;
	stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = 500;
	CVI_RGN_SetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "1280_720_nv21_out_vpss_coverex.yuv");
	snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_VPSS_COVEREX_REF_MD5);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_mosaic_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	RGN_HANDLE Handle;
	RGN_CHN_ATTR_S stRgnChnAttr;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33] = {};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	param.u32HdlNum = 3;
	param.enType = MOSAIC_RGN;

	s32Ret = RGN_COMM_REGION_Create(param.u32HdlNum, param.enType, 0);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(param.u32HdlNum,
		param.enType, &param.stChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	Handle = RGN_COMM_REGION_GetMinHandle(param.enType);

	CVI_RGN_GetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.s32X = 600;
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = 30;
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.u32Width = 128;
	stRgnChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 128;
	CVI_RGN_SetDisplayAttr(Handle, &param.stChn, &stRgnChnAttr);

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "1280_720_nv21_out_vpss_mosaic.yuv");
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(param.u32HdlNum, param.enType, &param.stChn);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(param.u32HdlNum, param.enType);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_create_destroy_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;

	memset(&param, 0, sizeof(param));
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;
	rgn_vb_init(&param);

	/************************************************
	 * step3:  Init RGN
	 ************************************************/
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

EXIT1:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}

	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_create_attach_detach_destroy_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = RGN_COMM_REGION_AttachToChn(HandleNum, enType, &stChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(HandleNum, enType, &stChn);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_update_attr_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;

	/************************************************
	 * Init VB
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;
	s32Ret = rgn_vb_init(&param);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("rgn_vb_init failed!\n");
		goto EXIT1;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	CVI_S32 i;
	CVI_S32 MinHandle;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	RGN_ATTR_S stRegion;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	Path_BMP = test_bmp;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	if (enType == OVERLAY_RGN || enType == OVERLAYEX_RGN) {
		MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

		for (i = MinHandle; i < MinHandle + HandleNum; i++) {
			s32Ret = CVI_RGN_GetAttr(MinHandle, &stRegion);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_RGN_GetAttr failed!\n");
			}
			UT_PRT("u32BgColor:	0x%x\n", stRegion.unAttr.stOverlay.u32BgColor);
		}
	}

	//Change BgColor to test set RGN attribute
	stRegion.unAttr.stOverlay.u32BgColor = 0xFF000000;

	s32Ret = CVI_RGN_SetAttr(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_SetAttr failed!\n");
	}
	s32Ret = CVI_RGN_GetAttr(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_GetAttr failed!\n");
	}
	UT_PRT("u32BgColor:	0x%x\n", stRegion.unAttr.stOverlay.u32BgColor);

EXIT1:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}

	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_update_attach_attr_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ExtRet = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_HANDLE cover_hdl;
	RGN_CHN_ATTR_S stRgnChnAttr;

	// create cover at 1st layer.
	HandleNum = 1;
	enType = COVER_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	PIXEL_FORMAT_E enPixelFormat;

	enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = RGN_COMM_REGION_Create(HandleNum, enType, enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}
	s32Ret = RGN_COMM_REGION_AttachToChn(HandleNum, enType, &stChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}
	cover_hdl = RGN_COMM_REGION_GetMinHandle(enType);

	CVI_RGN_GetDisplayAttr(cover_hdl, &stChn, &stRgnChnAttr);
	UT_PRT("u32Color:	0x%x\n", stRgnChnAttr.unChnAttr.stCoverChn.u32Color);

	UT_PRT("Change cover to green!\n");
	stRgnChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ff00;
	CVI_RGN_SetDisplayAttr(cover_hdl, &stChn, &stRgnChnAttr);

	CVI_RGN_GetDisplayAttr(cover_hdl, &stChn, &stRgnChnAttr);
	UT_PRT("u32Color:	0x%x\n", stRgnChnAttr.unChnAttr.stCoverChn.u32Color);

	s32ExtRet = RGN_COMM_REGION_DetachFrmChn(HandleNum, enType, &stChn);
	if (s32ExtRet != CVI_SUCCESS)
		UT_PRT("RGN_COMM_REGION_DetachFrmChn failed!\n");
EXIT2:
	s32ExtRet = RGN_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_Destroy failed!\n");
	}
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_hw_compress_simple_objects_test(CVI_S32 s32ChnId)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHandle;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = s32ChnId; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[s32ChnId] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = s32ChnId;

	MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 1280;
	stRegion.unAttr.stOverlay.stSize.u32Height = 720;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize = RGN_CMPR_MIN_SIZE;

	s32Ret = CVI_RGN_Create(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(MinHandle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(MinHandle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 12;

	pstObjAttr[0].stRgnRect.stRect.s32X = 0;
	pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
	pstObjAttr[0].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[0].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[0].stRgnRect.u32Thick = 1;
	pstObjAttr[0].stRgnRect.u32Color = 0xffff;
	pstObjAttr[0].stRgnRect.u32IsFill = false;
	pstObjAttr[0].enObjType = RGN_CMPR_RECT;
	pstObjAttr[1].stRgnRect.stRect.s32X = 100;
	pstObjAttr[1].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[1].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[1].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[1].stRgnRect.u32Thick = 2;
	pstObjAttr[1].stRgnRect.u32Color = 0x8000;
	pstObjAttr[1].stRgnRect.u32IsFill = false;
	pstObjAttr[1].enObjType = RGN_CMPR_RECT;
	pstObjAttr[2].stRgnRect.stRect.s32X = 200;
	pstObjAttr[2].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[2].stRgnRect.u32Thick = 3;
	pstObjAttr[2].stRgnRect.u32Color = 0x801f;
	pstObjAttr[2].stRgnRect.u32IsFill = false;
	pstObjAttr[2].enObjType = RGN_CMPR_RECT;
	pstObjAttr[3].stRgnRect.stRect.s32X = 300;
	pstObjAttr[3].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[3].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[3].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[3].stRgnRect.u32Thick = 4;
	pstObjAttr[3].stRgnRect.u32Color = 0x83e0;
	pstObjAttr[3].stRgnRect.u32IsFill = false;
	pstObjAttr[3].enObjType = RGN_CMPR_RECT;
	pstObjAttr[4].stRgnRect.stRect.s32X = 400;
	pstObjAttr[4].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[4].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[4].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[4].stRgnRect.u32Thick = 5;
	pstObjAttr[4].stRgnRect.u32Color = 0xfc00;
	pstObjAttr[4].stRgnRect.u32IsFill = false;
	pstObjAttr[4].enObjType = RGN_CMPR_RECT;
	pstObjAttr[5].stRgnRect.stRect.s32X = 500;
	pstObjAttr[5].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[5].stRgnRect.u32Thick = 6;
	pstObjAttr[5].stRgnRect.u32Color = 0xffe0;
	pstObjAttr[5].stRgnRect.u32IsFill = false;
	pstObjAttr[5].enObjType = RGN_CMPR_RECT;
	pstObjAttr[6].stRgnRect.stRect.s32X = 600;
	pstObjAttr[6].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[6].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[6].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[6].stRgnRect.u32Thick = 7;
	pstObjAttr[6].stRgnRect.u32Color = 0xfc1f;
	pstObjAttr[6].stRgnRect.u32IsFill = false;
	pstObjAttr[6].enObjType = RGN_CMPR_RECT;
	pstObjAttr[7].stRgnRect.stRect.s32X = 700;
	pstObjAttr[7].stRgnRect.stRect.s32Y = 000;
	pstObjAttr[7].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[7].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[7].stRgnRect.u32Thick = 8;
	pstObjAttr[7].stRgnRect.u32Color = 0x83ff;
	pstObjAttr[7].stRgnRect.u32IsFill = false;
	pstObjAttr[7].enObjType = RGN_CMPR_RECT;

	pstObjAttr[8].stLine.stPointStart.s32X = 600;
	pstObjAttr[8].stLine.stPointStart.s32Y = 200;
	pstObjAttr[8].stLine.stPointEnd.s32X = 300;
	pstObjAttr[8].stLine.stPointEnd.s32Y = 400;
	pstObjAttr[8].stLine.u32Thick = 8;
	pstObjAttr[8].stLine.u32Color = 0xfc10;
	pstObjAttr[8].enObjType = RGN_CMPR_LINE;
	pstObjAttr[9].stLine.stPointStart.s32X = 300;
	pstObjAttr[9].stLine.stPointStart.s32Y = 400;
	pstObjAttr[9].stLine.stPointEnd.s32X = 800;
	pstObjAttr[9].stLine.stPointEnd.s32Y = 700;
	pstObjAttr[9].stLine.u32Thick = 8;
	pstObjAttr[9].stLine.u32Color = 0xfff0;
	pstObjAttr[9].enObjType = RGN_CMPR_LINE;
	pstObjAttr[10].stLine.stPointStart.s32X = 800;
	pstObjAttr[10].stLine.stPointStart.s32Y = 700;
	pstObjAttr[10].stLine.stPointEnd.s32X = 1100;
	pstObjAttr[10].stLine.stPointEnd.s32Y = 600;
	pstObjAttr[10].stLine.u32Thick = 8;
	pstObjAttr[10].stLine.u32Color = 0xfc7f;
	pstObjAttr[10].enObjType = RGN_CMPR_LINE;
	pstObjAttr[11].stLine.stPointStart.s32X = 1100;
	pstObjAttr[11].stLine.stPointStart.s32Y = 600;
	pstObjAttr[11].stLine.stPointEnd.s32X = 600;
	pstObjAttr[11].stLine.stPointEnd.s32Y = 200;
	pstObjAttr[11].stLine.u32Thick = 8;
	pstObjAttr[11].stLine.u32Color = 0x8fff;
	pstObjAttr[11].enObjType = RGN_CMPR_LINE;

	s32Ret = CVI_RGN_UpdateCanvas(MinHandle);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s",
		"1280_720_nv21_out_hw_cmpr_GetUpCanvas.yuv");
	snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_HW_CMPR_GETUPCANVAS_REF_MD5);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT3:
	CVI_RGN_DetachFromChn(MinHandle, &stChn);
EXIT2:
	CVI_RGN_Destroy(MinHandle);
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_hw_compress_simple_objects_and_bitmap_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHandle;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	RGN_TEST_PARAM param;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	BITMAP_S stBitmap, stBitmapText;
	CVI_U64 u64BitmapPhyAddr, u64BitmapTextPhyAddr;
	CVI_VOID *pBitmapVirAddr, *pBitmapTextVirAddr;
	CVI_CHAR szStr[MAX_STR_LEN];

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;

	MinHandle = RGN_COMM_REGION_GetMinHandle(enType);

	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 1280;
	stRegion.unAttr.stOverlay.stSize.u32Height = 720;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize = RGN_CMPR_MIN_SIZE;

	s32Ret = CVI_RGN_Create(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(MinHandle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(MinHandle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 14;

	CVI_U32 u32Colors[8] = {0xffff, 0x8000, 0x801f, 0x83e0, 0xfc00, 0xffe0, 0xfc1f, 0x83ff};

	for (CVI_S32 i = 0; i < 8; i++) {
		pstObjAttr[i].stRgnRect.stRect.s32X = i * 100;
		pstObjAttr[i].stRgnRect.stRect.s32Y = (i >= 4) ? (7 - i) * 100 : i * 100;
		pstObjAttr[i].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[i].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[i].stRgnRect.u32Thick = i;
		pstObjAttr[i].stRgnRect.u32Color = u32Colors[i];
		pstObjAttr[i].stRgnRect.u32IsFill = false;
		pstObjAttr[i].enObjType = RGN_CMPR_RECT;
	}

	pstObjAttr[8].stLine.stPointStart.s32X = 600;
	pstObjAttr[8].stLine.stPointStart.s32Y = 200;
	pstObjAttr[8].stLine.stPointEnd.s32X = 300;
	pstObjAttr[8].stLine.stPointEnd.s32Y = 400;
	pstObjAttr[8].stLine.u32Thick = 8;
	pstObjAttr[8].stLine.u32Color = 0xe318;
	pstObjAttr[8].enObjType = RGN_CMPR_LINE;
	pstObjAttr[9].stLine.stPointStart.s32X = 300;
	pstObjAttr[9].stLine.stPointStart.s32Y = 400;
	pstObjAttr[9].stLine.stPointEnd.s32X = 800;
	pstObjAttr[9].stLine.stPointEnd.s32Y = 700;
	pstObjAttr[9].stLine.u32Thick = 8;
	pstObjAttr[9].stLine.u32Color = 0xe318;
	pstObjAttr[9].enObjType = RGN_CMPR_LINE;
	pstObjAttr[10].stLine.stPointStart.s32X = 800;
	pstObjAttr[10].stLine.stPointStart.s32Y = 700;
	pstObjAttr[10].stLine.stPointEnd.s32X = 1100;
	pstObjAttr[10].stLine.stPointEnd.s32Y = 600;
	pstObjAttr[10].stLine.u32Thick = 8;
	pstObjAttr[10].stLine.u32Color = 0xe318;
	pstObjAttr[10].enObjType = RGN_CMPR_LINE;
	pstObjAttr[11].stLine.stPointStart.s32X = 1100;
	pstObjAttr[11].stLine.stPointStart.s32Y = 600;
	pstObjAttr[11].stLine.stPointEnd.s32X = 600;
	pstObjAttr[11].stLine.stPointEnd.s32Y = 200;
	pstObjAttr[11].stLine.u32Thick = 8;
	pstObjAttr[11].stLine.u32Color = 0xe318;
	pstObjAttr[11].enObjType = RGN_CMPR_LINE;

	s32Ret = RGN_COMM_REGION_MST_LoadBmp(tiger_bmp, &stBitmap, CVI_FALSE, 0x00,
		pstCanvasCmprAttr->enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_REGION_MST_LoadBmp failed with %#x!\n", s32Ret);
		goto EXIT3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64BitmapPhyAddr, (CVI_VOID **)&pBitmapVirAddr, "rgn_cmpr_bitmap1",
			stBitmap.u32Width * stBitmap.u32Height * 2);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
		goto EXIT4;
	}
	memcpy(pBitmapVirAddr, stBitmap.pData, stBitmap.u32Width * stBitmap.u32Height * 2);
	pstObjAttr[12].stBitmap.stRect.s32X = 20;
	pstObjAttr[12].stBitmap.stRect.s32Y = 100;
	pstObjAttr[12].stBitmap.stRect.u32Width = stBitmap.u32Width;
	pstObjAttr[12].stBitmap.stRect.u32Height = stBitmap.u32Height;
	pstObjAttr[12].stBitmap.u64BitmapPAddr = u64BitmapPhyAddr;
	pstObjAttr[12].enObjType = RGN_CMPR_BIT_MAP;

	rgn_get_time_str(NULL, szStr, MAX_STR_LEN);
	s32Ret = rgn_time_bit_map(szStr, &stBitmapText, 0x9ce7, 0x7fff);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("rgn_time_bit_map failed with %#x!\n", s32Ret);
		goto EXIT5;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64BitmapTextPhyAddr, (CVI_VOID **)&pBitmapTextVirAddr,
		"rgn_cmpr_bitmap2", stBitmapText.u32Width * stBitmapText.u32Height * 2);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
		goto EXIT6;
	}
	memcpy(pBitmapTextVirAddr, stBitmapText.pData, stBitmapText.u32Width * stBitmapText.u32Height * 2);
	pstObjAttr[13].stBitmap.stRect.s32X = 20;
	pstObjAttr[13].stBitmap.stRect.s32Y = 40;
	pstObjAttr[13].stBitmap.stRect.u32Width = stBitmapText.u32Width;
	pstObjAttr[13].stBitmap.stRect.u32Height = stBitmapText.u32Height;
	pstObjAttr[13].stBitmap.u64BitmapPAddr = u64BitmapTextPhyAddr;
	pstObjAttr[13].enObjType = RGN_CMPR_BIT_MAP;

	s32Ret = CVI_RGN_UpdateCanvas(MinHandle);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		goto EXIT7;
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33] = {};

	snprintf(fileName, sizeof(fileName) - 1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut) - 1, "%s",
		"1280_720_nv21_out_hw_cmpr_bitmap_GetUpCanvas.yuv");
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT7:
	CVI_SYS_IonFree(u64BitmapTextPhyAddr, pBitmapTextVirAddr);
EXIT6:
	free(stBitmapText.pData);
EXIT5:
	CVI_SYS_IonFree(u64BitmapPhyAddr, pBitmapVirAddr);
EXIT4:
	free(stBitmap.pData);
EXIT3:
	CVI_RGN_DetachFromChn(MinHandle, &stChn);
EXIT2:
	CVI_RGN_Destroy(MinHandle);
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_formats_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 Handle;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	Handle = 0;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
	stRegion.unAttr.stOverlay.stSize.u32Width = 72;
	stRegion.unAttr.stOverlay.stSize.u32Height = 60;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}
	fp = fopen(dog_argb8888_bin, "rb");
	if (fp == NULL) {
		UT_PRT("fopen failed!\n");
		goto EXIT3;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT3;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT3;
	}
	free(stBitmap.pData);

	Handle = 1;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_4444;
	stRegion.unAttr.stOverlay.stSize.u32Width = 80;
	stRegion.unAttr.stOverlay.stSize.u32Height = 60;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 120;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT4;
	}

	fp = fopen(dog_argb4444_bin, "rb");
	if (fp == NULL) {
		UT_PRT("fopen failed!\n");
		goto EXIT5;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT5;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT5;
	}
	free(stBitmap.pData);

	Handle = 2;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 80;
	stRegion.unAttr.stOverlay.stSize.u32Height = 60;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT5;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 220;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT6;
	}

	fp = fopen(dog_argb1555_bin, "rb");
	if (fp == NULL) {
		UT_PRT("fopen failed!\n");
		goto EXIT7;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT7;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT5;
	}
	free(stBitmap.pData);

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "1280_720_nv21_out_formats_test.yuv");
	snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_FORMATS_REF_MD5);
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT7:
	s32Ret = CVI_RGN_DetachFromChn(2, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_DetachFromChn Handle(2) failed!\n");
EXIT6:
	s32Ret = CVI_RGN_Destroy(2);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_Destroy Handle(2) failed!\n");
EXIT5:
	s32Ret = CVI_RGN_DetachFromChn(1, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_DetachFromChn Handle(1) failed!\n");
EXIT4:
	s32Ret = CVI_RGN_Destroy(1);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_Destroy Handle(1) failed!\n");
EXIT3:
	s32Ret = CVI_RGN_DetachFromChn(0, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_DetachFromChn Handle(0) failed!\n");
EXIT2:
	s32Ret = CVI_RGN_Destroy(0);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_Destroy Handle(0) failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_sc_v1_capability_fun_test(RGN_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 Handle = 0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 3840;
	param.stOutputSize.u32Height = 2160;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	if (enType == OVERLAY_RGN) {
		Handle = OverlayMinHandle;
		stRegion.enType = enType;
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
		stRegion.unAttr.stOverlay.stSize.u32Width = 72;
		stRegion.unAttr.stOverlay.stSize.u32Height = 60;
		stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
		stRegion.unAttr.stOverlay.u32CanvasNum = 1;
		stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	} else if (enType == COVER_RGN) {
		Handle = CoverMinHandle;
		stRegion.enType = enType;
	}

	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}
	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = enType;
	if (enType == OVERLAY_RGN) {
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 3740;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	} else if (enType == COVER_RGN) {
		stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 100;
		stChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ffff;
		stChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
		stChnAttr.unChnAttr.stCoverChn.stRect.s32X = 3740;
		stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 20;
	}

	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	if (enType == OVERLAY_RGN) {
		fp = fopen(dog_argb8888_bin, "rb");
		if (fp == NULL) {
			UT_PRT("fopen failed!\n");
			goto EXIT3;
		}
		fseek(fp, 0L, SEEK_END);
		u32FileSize = ftell(fp);
		rewind(fp);
		stBitmap.pData = malloc(u32FileSize);
		if (stBitmap.pData == NULL) {
			UT_PRT("malloc size(%d) failed!\n", u32FileSize);
			fclose(fp);
			goto EXIT3;
		}
		fread(stBitmap.pData, u32FileSize, 1, fp);
		fclose(fp);
		stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

		s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
			free(stBitmap.pData);
			goto EXIT3;
		}
		free(stBitmap.pData);
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	if (enType == OVERLAY_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "3840_2160_out_overlay_sc_v1_capability_test.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_OVERLAY_SC_V1_CAPABILITY_REF_MD5);
	} else if (enType == COVER_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "3840_2160_out_cover_sc_v1_capability_test.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_COVER_SC_V1_CAPABILITY_REF_MD5);
	}

	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
		goto EXIT3;
	}

EXIT3:
	CVI_RGN_DetachFromChn(Handle, &param.stChn);
EXIT2:
	CVI_RGN_Destroy(Handle);
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_sc_v1_capability_test(void)
{
	RGN_TYPE_E enRgnTypes[2] = {OVERLAY_RGN, COVER_RGN};
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 u32TypeCnt = ARRAY_SIZE(enRgnTypes);

	for (CVI_S32 i = 0; i < u32TypeCnt; i++) {
		s32Ret |= rgn_vpss_sc_v1_capability_fun_test(enRgnTypes[i]);
	}
	return s32Ret;
}

static CVI_S32 rgn_vpss_sc_v2_capability_fun_test(RGN_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 Handle = 0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN1; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1920;
	param.stOutputSize.u32Height = 1080;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN1] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	if (enType == OVERLAY_RGN) {
		Handle = OverlayMinHandle;
		stRegion.enType = enType;
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
		stRegion.unAttr.stOverlay.stSize.u32Width = 72;
		stRegion.unAttr.stOverlay.stSize.u32Height = 60;
		stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
		stRegion.unAttr.stOverlay.u32CanvasNum = 1;
		stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	} else if (enType == COVER_RGN) {
		Handle = CoverMinHandle;
		stRegion.enType = enType;
	}

	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}
	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = enType;
	if (enType == OVERLAY_RGN) {
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 1848;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	} else if (enType == COVER_RGN) {
		stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 100;
		stChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ffff;
		stChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
		stChnAttr.unChnAttr.stCoverChn.stRect.s32X = 1820;
		stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 20;
	}

	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	if (enType == OVERLAY_RGN) {
		fp = fopen(dog_argb8888_bin, "rb");
		if (fp == NULL) {
			UT_PRT("fopen failed!\n");
			goto EXIT3;
		}
		fseek(fp, 0L, SEEK_END);
		u32FileSize = ftell(fp);
		rewind(fp);
		stBitmap.pData = malloc(u32FileSize);
		if (stBitmap.pData == NULL) {
			UT_PRT("malloc size(%d) failed!\n", u32FileSize);
			fclose(fp);
			goto EXIT3;
		}
		fread(stBitmap.pData, u32FileSize, 1, fp);
		fclose(fp);
		stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

		s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
			free(stBitmap.pData);
			goto EXIT3;
		}
		free(stBitmap.pData);
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	if (enType == OVERLAY_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s",
			"1920_1080_nv21_out_overlay_sc_v2_capability_test.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_OVERLAY_SC_V2_CAPABILITY_REF_MD5);
	} else if (enType == COVER_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s",
			"1920_1080_nv21_out_cover_sc_v2_capability_test.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_COVER_SC_V2_CAPABILITY_REF_MD5);
	}

	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
		goto EXIT3;
	}

EXIT3:
	CVI_RGN_DetachFromChn(Handle, &param.stChn);
EXIT2:
	CVI_RGN_Destroy(Handle);
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_sc_v2_capability_test(void)
{
	RGN_TYPE_E enRgnTypes[2] = {OVERLAY_RGN, COVER_RGN};
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 u32TypeCnt = ARRAY_SIZE(enRgnTypes);

	for (CVI_S32 i = 0; i < u32TypeCnt; i++) {
		s32Ret |= rgn_vpss_sc_v2_capability_fun_test(enRgnTypes[i]);
	}
	return s32Ret;
}

static CVI_S32 rgn_vpss_mix_capability_fun_test(RGN_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 Handle = 0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 64;
	param.stOutputSize.u32Height = 64;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	if (enType == OVERLAY_RGN) {
		Handle = OverlayMinHandle;
		stRegion.enType = enType;
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
		stRegion.unAttr.stOverlay.stSize.u32Width = 64;
		stRegion.unAttr.stOverlay.stSize.u32Height = 64;
		stRegion.unAttr.stOverlay.u32BgColor = 0x00;
		stRegion.unAttr.stOverlay.u32CanvasNum = 2;
		stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
		stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize = RGN_CMPR_MIN_SIZE;
	} else if (enType == COVER_RGN) {
		Handle = CoverMinHandle;
		stRegion.enType = enType;
	}

	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}
	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = enType;
	if (enType == OVERLAY_RGN) {
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	} else if (enType == COVER_RGN) {
		stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 64;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 64;
		stChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ffff;
		stChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
		stChnAttr.unChnAttr.stCoverChn.stRect.s32X = 0;
		stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 0;
	}

	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	if (enType == OVERLAY_RGN) {
		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
		RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
		RGN_CANVAS_INFO_S stCanvasInfo;

		s32Ret = CVI_RGN_GetCanvasInfo(OverlayMinHandle, &stCanvasInfo);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
			goto EXIT3;
		}

		pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
		pstObjAttr = stCanvasInfo.pstObjAttr;

		pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
		pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
		pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
		pstCanvasCmprAttr->u32ObjNum = 1;

		pstObjAttr[0].stRgnRect.stRect.s32X = 0;
		pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
		pstObjAttr[0].stRgnRect.stRect.u32Width = 64;
		pstObjAttr[0].stRgnRect.stRect.u32Height = 64;
		pstObjAttr[0].stRgnRect.u32Thick = 4;
		pstObjAttr[0].stRgnRect.u32Color = 0x801f;
		pstObjAttr[0].stRgnRect.u32IsFill = false;
		pstObjAttr[0].enObjType = RGN_CMPR_RECT;

		s32Ret = CVI_RGN_UpdateCanvas(OverlayMinHandle);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
			goto EXIT3;
		}
	}

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33];

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	if (enType == OVERLAY_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "64_64_nv21_out_overlay_mix_capability_test.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_OVERLAY_MIX_CAPABILITY_REF_MD5);
	} else if (enType == COVER_RGN) {
		snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "64_64_nv21_out_cover_mix_capability_test.yuv");
		snprintf(aszMD5Sum, sizeof(aszMD5Sum), "%s", RGN_COVER_MIX_CAPABILITY_REF_MD5);
	}

	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
		goto EXIT3;
	}

EXIT3:
	CVI_RGN_DetachFromChn(Handle, &param.stChn);
EXIT2:
	CVI_RGN_Destroy(Handle);
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_vpss_mix_capability_test(void)
{
	RGN_TYPE_E enRgnTypes[2] = {OVERLAY_RGN, COVER_RGN};
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 u32TypeCnt = ARRAY_SIZE(enRgnTypes);

	for (CVI_S32 i = 0; i < u32TypeCnt; i++) {
		s32Ret |= rgn_vpss_mix_capability_fun_test(enRgnTypes[i]);
	}
	return s32Ret;
}

static CVI_S32 rgn_invert_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TEST_PARAM param;
	CVI_S32 Handle;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	/************************************************
	 * Init VB and VPSS
	 ************************************************/
	memset(&param, 0, sizeof(param));
	param.stChn.enModId = CVI_ID_VPSS;
	param.stChn.s32DevId = 0; // grp
	param.stChn.s32ChnId = VPSS_CHN0; // chn
	param.stInputSize.u32Width = 1920;
	param.stInputSize.u32Height = 1080;
	param.eInputFmt = PIXEL_FORMAT_YUV_PLANAR_422;
	param.stOutputSize.u32Width = 1280;
	param.stOutputSize.u32Height = 720;
	param.eOutputFmt = PIXEL_FORMAT_NV21;

	s32Ret = rgn_vb_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vb_init failed.\n");
		return s32Ret;
	}

	abChnEnable[VPSS_CHN0] = true;
	s32Ret = rgn_vpss_init(&param);
	if (s32Ret) {
		UT_PRT("rgn_vpss_init failed.\n");
		goto EXIT0;
	}

	/************************************************
	 * Init RGN
	 ************************************************/
	Handle = 0;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_FONT;
	stRegion.unAttr.stOverlay.stSize.u32Width = 384;
	stRegion.unAttr.stOverlay.stSize.u32Height = 24;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = 1;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 24;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 24;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 20;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = 1;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT2;
	}
	fp = fopen(fontmode_bin, "rb");
	if (fp == NULL) {
		UT_PRT("fopen failed!\n");
		goto EXIT3;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT3;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT3;
	}
	free(stBitmap.pData);

	Handle = 1;
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_FONT;
	stRegion.unAttr.stOverlay.stSize.u32Width = 384;
	stRegion.unAttr.stOverlay.stSize.u32Height = 24;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT3;
	}

	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 120;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = Handle;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = 1;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 24;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 24;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 20;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = 1;
	s32Ret = CVI_RGN_AttachToChn(Handle, &param.stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT4;
	}

	fp = fopen(fontmode_bin, "rb");
	if (fp == NULL) {
		UT_PRT("fopen failed!\n");
		goto EXIT5;
	}
	fseek(fp, 0L, SEEK_END);
	u32FileSize = ftell(fp);
	rewind(fp);
	stBitmap.pData = malloc(u32FileSize);
	if (stBitmap.pData == NULL) {
		UT_PRT("malloc size(%d) failed!\n", u32FileSize);
		fclose(fp);
		goto EXIT5;
	}
	fread(stBitmap.pData, u32FileSize, 1, fp);
	fclose(fp);

	stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

	s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		free(stBitmap.pData);
		goto EXIT5;
	}
	free(stBitmap.pData);

	CVI_CHAR fileName[256];
	CVI_CHAR fileNameOut[256];
	CVI_CHAR aszMD5Sum[33] = {};

	snprintf(fileName, sizeof(fileName)-1, "%s", VPSS_FILENAME_IN);
	snprintf(fileNameOut, sizeof(fileNameOut)-1, "%s", "1280_720_nv21_out_invert_test.yuv");
	/************************************************
	 * step3:  VPSS work
	 ************************************************/
	param.inputFile = fileName;
	param.outputFile = fileNameOut;
	param.aszMD5Sum = aszMD5Sum;
	param.u32RepeatCnt = 1;
	s32Ret = rgn_ut_vpss_send_frame(&param);
	if (s32Ret) {
		UT_PRT("rgn_ut_vpss_send_frame failed.\n");
	}

EXIT5:
	s32Ret = CVI_RGN_DetachFromChn(1, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_DetachFromChn Handle(1) failed!\n");
EXIT4:
	s32Ret = CVI_RGN_Destroy(1);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_Destroy Handle(1) failed!\n");
EXIT3:
	s32Ret = CVI_RGN_DetachFromChn(0, &param.stChn);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_DetachFromChn Handle(0) failed!\n");
EXIT2:
	s32Ret = CVI_RGN_Destroy(0);
	if (s32Ret != CVI_SUCCESS)
		UT_PRT("CVI_RGN_Destroy Handle(0) failed!\n");
EXIT1:
	RGN_COMM_VPSS_Stop(param.stChn.s32DevId, abChnEnable);
EXIT0:
	RGN_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 rgn_all_hw_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret |= rgn_set_bitmap_with_vpss_sendframe_test(VPSS_CHN0);
	s32Ret |= rgn_set_bitmap_with_vpss_sendframe_test(VPSS_CHN1);
	s32Ret |= rgn_set_bitmap_with_vpss_sendframe_test(VPSS_CHN2);
	s32Ret |= rgn_set_bitmap_with_vpss_sendframe_test(VPSS_CHN3);
	s32Ret |= rgn_hw_compress_simple_objects_test(VPSS_CHN0);
	s32Ret |= rgn_hw_compress_simple_objects_test(VPSS_CHN1);
	s32Ret |= rgn_hw_compress_simple_objects_test(VPSS_CHN2);
	s32Ret |= rgn_hw_compress_simple_objects_test(VPSS_CHN3);

	return s32Ret;
}

static CVI_S32 basic_mutli_chn(VPSS_MULTI_TEST_PARAM *pTestParam)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, n;
	VPSS_GRP VpssGrp = pTestParam->VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame;
	struct VPSS_CHN_PARAM *pstChnParam;
	CVI_BOOL bSaveFile = CVI_TRUE;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(pTestParam->stSizeIn.u32Width, pTestParam->stSizeIn.u32Height,
		pTestParam->enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	n = 1;
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		u32BlkSizeOut = COMMON_GetPicBufferSize(pstChnParam->stSizeOut.u32Width,
					pstChnParam->stSizeOut.u32Height, pstChnParam->enFormatOut,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		stVbConf.astCommPool[n].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[n].u32BlkCnt	= 1;
		stVbConf.astCommPool[n].enRemapMode	= VB_REMAP_MODE_CACHED;
		UT_PRT("common pool[%d] BlkSize %d\n", n, u32BlkSizeOut);
		n++;
	}
	stVbConf.u32MaxPoolCnt = n;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pTestParam->enFormatIn;
	stVpssGrpAttr.u32MaxW			     = pTestParam->stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = pTestParam->stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	n = 1;
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		VpssChn = i;
		stVpssChnAttr.u32Width		    = pstChnParam->stSizeOut.u32Width;
		stVpssChnAttr.u32Height		    = pstChnParam->stSizeOut.u32Height;
		stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
		stVpssChnAttr.enPixelFormat		    = pstChnParam->enFormatOut;
		stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
		stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
		stVpssChnAttr.u32Depth			= 1;
		stVpssChnAttr.bMirror			= pstChnParam->bMirror;
		stVpssChnAttr.bFlip				= pstChnParam->bFlip;
		stVpssChnAttr.stAspectRatio		= pstChnParam->stAspectRatio;
		stVpssChnAttr.stNormalize		= pstChnParam->stNormalize;
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAttr failed with %#x, chn(%d)\n", s32Ret, i);
			goto exit2;
		}

		s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, n);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_EnableChn failed with %#x, chn(%d)\n", s32Ret, i);
			goto exit2;
		}
		n++;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*rgn config*/
	RGN_ATTR_S stRegion = {0};
	RGN_CHN_ATTR_S stChnAttr = {0};
	MMF_CHN_S stChn = {0};
	BITMAP_S stBitmap;
	FILE *fp = NULL;
	CVI_U32 u32FileSize = 0;

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		stRegion.enType = OVERLAY_RGN;
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
		if (i == 0) {
			stRegion.unAttr.stOverlay.stSize.u32Width = 1920;
			stRegion.unAttr.stOverlay.stSize.u32Height = 2160;
		} else {
			stRegion.unAttr.stOverlay.stSize.u32Width = 960;
			stRegion.unAttr.stOverlay.stSize.u32Height = 1080;
		}
		stRegion.unAttr.stOverlay.u32BgColor = 0x00000000;
		stRegion.unAttr.stOverlay.u32CanvasNum = 1;
		stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
			goto exit2;
		}

		stChn.enModId = CVI_ID_VPSS;
		stChn.s32DevId = 0;
		stChn.s32ChnId = i;
		memset(&stChnAttr, 0, sizeof(stChnAttr));
		stChnAttr.bShow = true;
		stChnAttr.enType = OVERLAY_RGN;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
		s32Ret = CVI_RGN_AttachToChn(i, &stChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
			goto exit3;
		}
		if (i == 0) {
			fp = fopen(argb8888_2160_bin, "rb");
		} else {
			fp = fopen(argb8888_1080_bin, "rb");
		}
		if (fp == NULL) {
			UT_PRT("fopen failed!\n");
			goto exit4;
		}
		fseek(fp, 0L, SEEK_END);
		u32FileSize = ftell(fp);
		rewind(fp);
		stBitmap.pData = malloc(u32FileSize);
		if (stBitmap.pData == NULL) {
			UT_PRT("malloc size(%d) failed!\n", u32FileSize);
			fclose(fp);
			goto exit4;
		}
		fread(stBitmap.pData, u32FileSize, 1, fp);
		fclose(fp);

		stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;

		s32Ret = CVI_RGN_SetBitMap(i, &stBitmap);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
			free(stBitmap.pData);
			goto exit4;
		}
		free(stBitmap.pData);
	}

	for (i = 0; i < VPSS_MAX_CHN_NUM - 2; i++) {
		stRegion.enType = OVERLAY_RGN;
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
		if (i == 0) {
			stRegion.unAttr.stOverlay.stSize.u32Width = 1920;
			stRegion.unAttr.stOverlay.stSize.u32Height = 2160;
		} else {
			stRegion.unAttr.stOverlay.stSize.u32Width = 960;
			stRegion.unAttr.stOverlay.stSize.u32Height = 1080;
		}
		stRegion.unAttr.stOverlay.u32BgColor = 0x00;
		stRegion.unAttr.stOverlay.u32CanvasNum = 2;
		stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
		stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize = RGN_CMPR_MIN_SIZE;


		s32Ret = CVI_RGN_Create(i + 4, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
			goto exit2;
		}

		stChn.enModId = CVI_ID_VPSS;
		stChn.s32DevId = 0;
		stChn.s32ChnId = i;
		memset(&stChnAttr, 0, sizeof(stChnAttr));
		stChnAttr.bShow = true;
		stChnAttr.enType = OVERLAY_RGN;
		if (i == 0) {
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 1920;
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
		} else {
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 960;
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
		}

		s32Ret = CVI_RGN_AttachToChn(i + 4, &stChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
			goto exit3;
		}

		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
		RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
		RGN_CANVAS_INFO_S stCanvasInfo;

		s32Ret = CVI_RGN_GetCanvasInfo(i + 4, &stCanvasInfo);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
			goto exit4;
		}

		pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
		pstObjAttr = stCanvasInfo.pstObjAttr;

		pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
		pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
		pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
		pstCanvasCmprAttr->u32ObjNum = 8;

		pstObjAttr[0].stRgnRect.stRect.s32X = 0;
		pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
		pstObjAttr[0].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[0].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[0].stRgnRect.u32Thick = 1;
		pstObjAttr[0].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[0].stRgnRect.u32IsFill = false;
		pstObjAttr[0].enObjType = RGN_CMPR_RECT;
		pstObjAttr[1].stRgnRect.stRect.s32X = 100;
		pstObjAttr[1].stRgnRect.stRect.s32Y = 100;
		pstObjAttr[1].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[1].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[1].stRgnRect.u32Thick = 2;
		pstObjAttr[1].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[1].stRgnRect.u32IsFill = false;
		pstObjAttr[1].enObjType = RGN_CMPR_RECT;
		pstObjAttr[2].stRgnRect.stRect.s32X = 200;
		pstObjAttr[2].stRgnRect.stRect.s32Y = 200;
		pstObjAttr[2].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[2].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[2].stRgnRect.u32Thick = 3;
		pstObjAttr[2].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[2].stRgnRect.u32IsFill = false;
		pstObjAttr[2].enObjType = RGN_CMPR_RECT;
		pstObjAttr[3].stRgnRect.stRect.s32X = 300;
		pstObjAttr[3].stRgnRect.stRect.s32Y = 300;
		pstObjAttr[3].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[3].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[3].stRgnRect.u32Thick = 4;
		pstObjAttr[3].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[3].stRgnRect.u32IsFill = false;
		pstObjAttr[3].enObjType = RGN_CMPR_RECT;
		pstObjAttr[4].stRgnRect.stRect.s32X = 400;
		pstObjAttr[4].stRgnRect.stRect.s32Y = 300;
		pstObjAttr[4].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[4].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[4].stRgnRect.u32Thick = 5;
		pstObjAttr[4].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[4].stRgnRect.u32IsFill = false;
		pstObjAttr[4].enObjType = RGN_CMPR_RECT;
		pstObjAttr[5].stRgnRect.stRect.s32X = 500;
		pstObjAttr[5].stRgnRect.stRect.s32Y = 200;
		pstObjAttr[5].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[5].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[5].stRgnRect.u32Thick = 6;
		pstObjAttr[5].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[5].stRgnRect.u32IsFill = false;
		pstObjAttr[5].enObjType = RGN_CMPR_RECT;
		pstObjAttr[6].stRgnRect.stRect.s32X = 600;
		pstObjAttr[6].stRgnRect.stRect.s32Y = 100;
		pstObjAttr[6].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[6].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[6].stRgnRect.u32Thick = 7;
		pstObjAttr[6].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[6].stRgnRect.u32IsFill = false;
		pstObjAttr[6].enObjType = RGN_CMPR_RECT;
		pstObjAttr[7].stRgnRect.stRect.s32X = 700;
		pstObjAttr[7].stRgnRect.stRect.s32Y = 000;
		pstObjAttr[7].stRgnRect.stRect.u32Width = 200;
		pstObjAttr[7].stRgnRect.stRect.u32Height = 200;
		pstObjAttr[7].stRgnRect.u32Thick = 8;
		pstObjAttr[7].stRgnRect.u32Color = 0xffffffff;
		pstObjAttr[7].stRgnRect.u32IsFill = false;
		pstObjAttr[7].enObjType = RGN_CMPR_RECT;

		pstObjAttr[8].stLine.stPointStart.s32X = 600;
		pstObjAttr[8].stLine.stPointStart.s32Y = 200;
		pstObjAttr[8].stLine.stPointEnd.s32X = 300;
		pstObjAttr[8].stLine.stPointEnd.s32Y = 400;
		pstObjAttr[8].stLine.u32Thick = 8;
		pstObjAttr[8].stLine.u32Color = 0xfc10;
		pstObjAttr[8].enObjType = RGN_CMPR_LINE;
		pstObjAttr[9].stLine.stPointStart.s32X = 300;
		pstObjAttr[9].stLine.stPointStart.s32Y = 400;
		pstObjAttr[9].stLine.stPointEnd.s32X = 800;
		pstObjAttr[9].stLine.stPointEnd.s32Y = 700;
		pstObjAttr[9].stLine.u32Thick = 8;
		pstObjAttr[9].stLine.u32Color = 0xfff0;
		pstObjAttr[9].enObjType = RGN_CMPR_LINE;
		pstObjAttr[10].stLine.stPointStart.s32X = 800;
		pstObjAttr[10].stLine.stPointStart.s32Y = 700;
		pstObjAttr[10].stLine.stPointEnd.s32X = 1100;
		pstObjAttr[10].stLine.stPointEnd.s32Y = 600;
		pstObjAttr[10].stLine.u32Thick = 8;
		pstObjAttr[10].stLine.u32Color = 0xfc7f;
		pstObjAttr[10].enObjType = RGN_CMPR_LINE;
		pstObjAttr[11].stLine.stPointStart.s32X = 1100;
		pstObjAttr[11].stLine.stPointStart.s32Y = 600;
		pstObjAttr[11].stLine.stPointEnd.s32X = 600;
		pstObjAttr[11].stLine.stPointEnd.s32Y = 200;
		pstObjAttr[11].stLine.u32Thick = 8;
		pstObjAttr[11].stLine.u32Color = 0x8fff;
		pstObjAttr[11].enObjType = RGN_CMPR_LINE;

		s32Ret = CVI_RGN_UpdateCanvas(i + 4);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
			goto exit4;
		}
	}

	//send frame
	s32Ret = RGN_COMM_FileSendToVpss(VpssGrp, &pTestParam->stSizeIn,
		pTestParam->enFormatIn, pTestParam->aszFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("RGN_COMM_FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		VpssChn = i;
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Grp(%d) Chn(%d), CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
			goto exit5;
		}
		UT_PRT("***Grp(%d) Chn(%d) CVI_VPSS_GetChnFrame Success***\n", VpssGrp, VpssChn);

		if (pstChnParam->aszMD5Sum[0]) {
			if (CompareWithMD5(pstChnParam->aszMD5Sum, &stVideoFrame)) {
				bSaveFile = CVI_TRUE;
				s32Ret = CVI_FAILURE;
				UT_PRT("chn%d Compare MD5 fail, MD5:%s\n",
					i, pstChnParam->aszMD5Sum);
			} else {
				bSaveFile = CVI_FALSE;
			}
		}

		if (bSaveFile && pstChnParam->aszFileNameOut[0]) {
			if (FrameSaveToFile(pstChnParam->aszFileNameOut, &stVideoFrame)) {
				UT_PRT("Grp(%d) Chn(%d),FrameSaveToFile. s32Ret: 0x%x !\n",
					VpssGrp, VpssChn, s32Ret);
			}
			UT_PRT("output file:%s\n", pstChnParam->aszFileNameOut);
		}

		if (pstChnParam->aszFileNameRef[0]) {
			if (CompareWithFile(pstChnParam->aszFileNameRef, &stVideoFrame)) {
				UT_PRT("Grp(%d) Chn(%d),CompareWithFile fail.\n", VpssGrp, VpssChn);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
				s32Ret = CVI_FAILURE;
				goto exit5;
			}
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);

		if (s32Ret)
			break;
	}

exit5:
	CVI_VPSS_StopGrp(VpssGrp);
exit4:
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		stChn.s32ChnId = i;
		CVI_RGN_DetachFromChn(i, &stChn);
	}
	for (i = 0; i < VPSS_MAX_CHN_NUM - 2; i++) {
		stChn.s32ChnId = i;
		CVI_RGN_DetachFromChn(i + 4, &stChn);
	}
exit3:
	for (i = 0; i < VPSS_MAX_CHN_NUM + 2; i++) {
		CVI_RGN_Destroy(i);
	}
exit2:
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		if (!pTestParam->astChnParam[i].bEnable)
			continue;
		CVI_VPSS_DisableChn(VpssGrp, i);
	}
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 rgn_perf_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32ChnNum = 4;
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = 3840;
	stTestParam.stSizeIn.u32Height = 2160;
	stTestParam.enFormatIn = PIXEL_FORMAT_BGR_888;
	strncpy(stTestParam.aszFileNameIn, VPSS_FILENAME_IN1, sizeof(stTestParam.aszFileNameIn));

	stTestParam.astChnParam[0].enFormatOut = PIXEL_FORMAT_BGR_888;
	stTestParam.astChnParam[1].enFormatOut = PIXEL_FORMAT_BGR_888;
	stTestParam.astChnParam[2].enFormatOut = PIXEL_FORMAT_BGR_888;
	stTestParam.astChnParam[3].enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.astChnParam[0].stSizeOut.u32Width = 3840;
	stTestParam.astChnParam[0].stSizeOut.u32Height = 2160;
	stTestParam.astChnParam[1].stSizeOut.u32Width = 1920;
	stTestParam.astChnParam[1].stSizeOut.u32Height = 1080;
	stTestParam.astChnParam[2].stSizeOut.u32Width = 1920;
	stTestParam.astChnParam[2].stSizeOut.u32Height = 1080;
	stTestParam.astChnParam[3].stSizeOut.u32Width = 1920;
	stTestParam.astChnParam[3].stSizeOut.u32Height = 1080;

	for (i = 0; i < s32ChnNum; i++) {
		stTestParam.astChnParam[i].bEnable = CVI_TRUE;
		stTestParam.astChnParam[i].VpssChn = i;
		stTestParam.astChnParam[i].bMirror = CVI_FALSE;
		stTestParam.astChnParam[i].bFlip = CVI_FALSE;
		stTestParam.astChnParam[i].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stTestParam.astChnParam[i].stNormalize.bEnable = CVI_FALSE;
		snprintf(stTestParam.astChnParam[i].aszFileNameOut, 64, "%s_chn%d_%d_%d_%s.bin",
			__func__, i,
			stTestParam.astChnParam[i].stSizeOut.u32Width,
			stTestParam.astChnParam[i].stSizeOut.u32Height,
			GetFmtName(stTestParam.astChnParam[i].enFormatOut));
	}

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, VPSS_1_TO_4_CHN0_MD5,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, VPSS_1_TO_4_CHN1_MD5,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	strncpy(stTestParam.astChnParam[2].aszMD5Sum, VPSS_1_TO_4_CHN2_MD5,
		sizeof(stTestParam.astChnParam[2].aszMD5Sum));
	strncpy(stTestParam.astChnParam[3].aszMD5Sum, VPSS_1_TO_4_CHN3_MD5,
		sizeof(stTestParam.astChnParam[3].aszMD5Sum));
	s32Ret = basic_mutli_chn(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 rgn_auto_regression(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret |= rgn_vpss_sendframe_test();
	s32Ret |= rgn_set_bitmap_with_vpss_sendframe_test(VPSS_CHN0);
	s32Ret |= rgn_update_canvas_with_vpss_sendframe_test();
	s32Ret |= rgn_8bit_mode_canvas_with_vpss_sendframe_test();
	s32Ret |= rgn_hw_compress_simple_objects_test(VPSS_CHN0);
	s32Ret |= rgn_hw_compress_simple_objects_and_bitmap_test();
	s32Ret |= rgn_create_destroy_test();
	s32Ret |= rgn_create_attach_detach_destroy_test();
	s32Ret |= rgn_update_attr_test();
	s32Ret |= rgn_vpss_formats_test();
	s32Ret |= rgn_vpss_sc_v1_capability_test();
	s32Ret |= rgn_vpss_sc_v2_capability_test();
	s32Ret |= rgn_vpss_mix_capability_test();
	s32Ret |= rgn_vpss_coverex_test();
	s32Ret |= rgn_vpss_mosaic_test();
	s32Ret |= rgn_invert_test();
	s32Ret |= rgn_all_hw_test();
	// s32Ret |= rgn_perf_test(); // Please modify the ion allocation. Larger ion memory is required

	return s32Ret;
}

static CVI_S32 _rgn_ut_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case RGN_VPSS_INIT_START_SEND_FRAME_TEST:
		s32Ret = rgn_vpss_sendframe_test();
		break;

	case RGN_BIT_MAP_WITH_VPSS_SEND_FRAME_TEST:
		s32Ret = rgn_set_bitmap_with_vpss_sendframe_test(VPSS_CHN0);
		break;

	case RGN_CANVAS_WITH_VPSS_SEND_FRAME_TEST:
		s32Ret = rgn_update_canvas_with_vpss_sendframe_test();
		break;

	case RGN_8BIT_MODE_CANVAS_WITH_VPSS_SEND_FRAME_TEST:
		s32Ret = rgn_8bit_mode_canvas_with_vpss_sendframe_test();
		break;

	case RGN_HW_COMPRESS_SIMPLE_OBJECTS_TEST:
		s32Ret = rgn_hw_compress_simple_objects_test(VPSS_CHN0);
		break;

	case RGN_HW_COMPRESS_SIMPLE_OBJECTS_AND_BITMAP_TEST:
		s32Ret = rgn_hw_compress_simple_objects_and_bitmap_test();
		break;

	case RGN_VPSS_COVEREX_TEST:
		s32Ret = rgn_vpss_coverex_test();
		break;

	case RGN_VPSS_MOSAIC_TEST:
		s32Ret = rgn_vpss_mosaic_test();
		break;

	case RGN_CREATE_DESTROY:
		s32Ret = rgn_create_destroy_test();
		break;

	case RGN_CREATE_ATTACTH_DETACH_DESTROY:
		s32Ret = rgn_create_attach_detach_destroy_test();
		break;

	case RGN_UPDATE_ATTR_TEST:
		s32Ret = rgn_update_attr_test();
		break;

	case RGN_UPDATE_ATTACH_ATTR_TEST:
		s32Ret = rgn_update_attach_attr_test();
		break;

	case RGN_VPSS_FORMATS_TEST:
		s32Ret = rgn_vpss_formats_test();
		break;

	case RGN_VPSS_SC_V1_CAPABILITY_TEST:
		s32Ret = rgn_vpss_sc_v1_capability_test();
		break;

	case RGN_VPSS_SC_V2_CAPABILITY_TEST:
		s32Ret = rgn_vpss_sc_v2_capability_test();
		break;

	case RGN_VPSS_MIX_CAPABILITY_TEST:
		s32Ret = rgn_vpss_mix_capability_test();
		break;

	case RGN_INVERT_TEST:
		s32Ret = rgn_invert_test();
		break;

	case RGN_ALL_HW_TEST:
		s32Ret = rgn_all_hw_test();
		break;

	case RGN_PERF_TEST:
		s32Ret = rgn_perf_test();
		break;

	case RGN_TEST_AUTO_REGRESSION:
		s32Ret = rgn_auto_regression();
		break;

	case 255:
		is_enable = 0;
		break;

	default:
		break;
	}

	if (s32Ret == CVI_SUCCESS)
		printf(GREEN"\n=== case(%d) pass ===\n"NONE"\n", op);
	else
		printf(RED"\n=== case(%d) fail===\n"NONE"\n", op);

	return s32Ret;
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 op;
	CVI_S32 s32Ret;

	UNUSED(argc);
	UNUSED(argv);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = _rgn_ut_handle_op(op);
		UT_PRT("rgn ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			UT_PRT("========================== RGN testcase ==========================\n");
			UT_PRT("%03d: vpss send frame.\n", RGN_VPSS_INIT_START_SEND_FRAME_TEST);
			UT_PRT("%03d: RGN set bit map.\n", RGN_BIT_MAP_WITH_VPSS_SEND_FRAME_TEST);
			UT_PRT("%03d: RGN update canvas.\n", RGN_CANVAS_WITH_VPSS_SEND_FRAME_TEST);
			UT_PRT("%03d: RGN update 8bit mode canvas.\n",
				RGN_8BIT_MODE_CANVAS_WITH_VPSS_SEND_FRAME_TEST);
			UT_PRT("%03d : HW compress simple objects test.\n", RGN_HW_COMPRESS_SIMPLE_OBJECTS_TEST);
			UT_PRT("%03d : HW compress simple objects and bitmap test.\n",
				RGN_HW_COMPRESS_SIMPLE_OBJECTS_AND_BITMAP_TEST);
			UT_PRT("%03d: vpss coverex test.\n", RGN_VPSS_COVEREX_TEST);
			UT_PRT("%03d: vpss mosaic test.\n", RGN_VPSS_MOSAIC_TEST);

			UT_PRT("========================== CVI API testcase ==========================\n");
			UT_PRT("%03d: create->destroy.\n", RGN_CREATE_DESTROY);
			UT_PRT("%03d: create->attach->detach->destroy.\n", RGN_CREATE_ATTACTH_DETACH_DESTROY);
			UT_PRT("%03d: create->get_attr->set_attr->get_attr.\n", RGN_UPDATE_ATTR_TEST);
			UT_PRT("%03d: create->attach->get_disp_attr->set_disp_attr->get_disp_attr.\n",
				RGN_UPDATE_ATTACH_ATTR_TEST);

			UT_PRT("==================== RGN sw verification testcase ====================\n");
			UT_PRT("%03d: RGN on VPSS formats test.\n", RGN_VPSS_FORMATS_TEST);
			UT_PRT("%03d: RGN on VPSS sc_v1 capability test.\n", RGN_VPSS_SC_V1_CAPABILITY_TEST);
			UT_PRT("%03d: RGN on VPSS sc_v2 capability test.\n", RGN_VPSS_SC_V2_CAPABILITY_TEST);
			UT_PRT("%03d: RGN on VPSS mix capability test.\n", RGN_VPSS_MIX_CAPABILITY_TEST);
			UT_PRT("%03d: RGN invert test.\n", RGN_INVERT_TEST);
			UT_PRT("%03d: RGN all hw test.\n", RGN_ALL_HW_TEST);
			UT_PRT("%03d: RGN perf test.\n", RGN_PERF_TEST);
			UT_PRT("%03d: RGN auto regression.\n", RGN_TEST_AUTO_REGRESSION);

			UT_PRT("255: exit\n");
			scanf("%d", &op);
			s32Ret = _rgn_ut_handle_op(op);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
		} while (op != 255);
	}

	CVI_SYS_Exit();
	CVI_VB_Exit();

	return 0;
}

