#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_debug.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "cvi_region.h"
#include "cvi_buffer.h"
#include "cvi_math.h"

#include "rgn_ut_fun.h"
#include "loadbmp.h"
#include "md5sum.h"

#define OverlayMinHandle 0
#define OverlayExMinHandle 20
#define CoverMinHandle 40
#define CoverExMinHandle 60
#define MosaicMinHandle 80
#define OdecHandle 100

#define COLOR_10_RGB_BLUE RGB(0, 0, 0x3FF)

RGN_RGBQUARD_S overlay_palette[256];
CVI_S32 RGN_COMM_REGION_MST_LoadBmp(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
			CVI_U32 u16FilColor, PIXEL_FORMAT_E enPixelFormat)
{
	OSD_SURFACE_S Surface;
	OSD_BITMAPFILEHEADER bmpFileHeader;
	OSD_BITMAPINFO bmpInfo;
	CVI_S32 Bpp;
	CVI_U32 nColors;
	CVI_U32 i, u32PdataSize;

	if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
		printf("GetBmpInfo err!\n");
		return CVI_FAILURE;
	}
	Bpp = bmpInfo.bmiHeader.biBitCount/8;
	nColors = 0;
	if (Bpp == 1) {
		if (bmpInfo.bmiHeader.biClrUsed == 0)
			nColors = 1 << bmpInfo.bmiHeader.biBitCount;
		else
			nColors = bmpInfo.bmiHeader.biClrUsed;

		if (nColors > 256) {
			printf("Number of indexed palette is over 256.");
			return CVI_FAILURE;
		}

		/* Create the palette */
		for (i = 0; i < nColors; i++) {
			overlay_palette[i].argbAlpha = bmpInfo.bmiColors[i].rgbReserved;
			overlay_palette[i].argbRed = bmpInfo.bmiColors[i].rgbRed;
			overlay_palette[i].argbGreen = bmpInfo.bmiColors[i].rgbGreen;
			overlay_palette[i].argbBlue = bmpInfo.bmiColors[i].rgbBlue;
#ifdef _RGN_COMMON_REGION_DEBUG_
			CVI_U32 u32Pixel =
				((overlay_palette[i].argbBlue | overlay_palette[i].argbGreen << 8) |
				(overlay_palette[i].argbRed << 16 | overlay_palette[i].argbAlpha << 24));
			printf("overlay_palette index(%d) (0x%x).\n", i, u32Pixel);
#endif
		}
	}

	if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
	} else if (enPixelFormat == PIXEL_FORMAT_8BIT_MODE) {
		Surface.enColorFmt = OSD_COLOR_FMT_8BIT_MODE;
	} else {
		printf("enPixelFormat err %d\n", enPixelFormat);
		return CVI_FAILURE;
	}

	u32PdataSize = Bpp * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight);
	pstBitmap->pData = malloc(u32PdataSize);
	if (pstBitmap->pData == NULL) {
		printf("malloc osd memory err!\n");
		return CVI_FAILURE;
	}

	CreateSurfaceByBitMap(filename, &Surface, (CVI_U8 *)(pstBitmap->pData));

	pstBitmap->u32Width = Surface.u16Width;
	pstBitmap->u32Height = Surface.u16Height;
	pstBitmap->enPixelFormat = enPixelFormat;

	if (bFil) {
		CVI_U32 i, j;
		CVI_U16 *pu16Temp;

		pu16Temp = (CVI_U16 *)pstBitmap->pData;
		for (i = 0; i < pstBitmap->u32Height; i++) {
			for (j = 0; j < pstBitmap->u32Width; j++) {
				if (u16FilColor == *pu16Temp) {
					*pu16Temp &= 0x7FFF;
				}

				pu16Temp++;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_VOID RGN_COMM_SYS_Exit(void)
{
	CVI_VB_Exit();
	CVI_SYS_Exit();
}

CVI_S32 RGN_COMM_SYS_Init(VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret = CVI_FAILURE;

	CVI_VB_Exit();
	CVI_SYS_Exit();

	if (pstVbConfig == NULL) {
		UT_PRT("input parameter is null, it is invaild!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_SetConfig(pstVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VPSS_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	VPSS_CHN VpssChn;
	CVI_S32 s32Ret;
	CVI_S32 j;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return CVI_FAILURE;
	}

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);

			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);

			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	CVI_S32 s32Ret;

	UNUSED(pabChnEnable);
	UNUSED(pstVpssGrpAttr);
	UNUSED(pastVpssChnAttr);

	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("%s failed with %#x\n", __func__, s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 REGION_CreateOverLay(CVI_S32 HandleNum, PIXEL_FORMAT_E pixelFormat)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = OVERLAY_RGN;
	if (pixelFormat ==  PIXEL_FORMAT_8BIT_MODE)
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_8BIT_MODE;
	else
		stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;

	stRegion.unAttr.stOverlay.stSize.u32Height = 200;
	stRegion.unAttr.stOverlay.stSize.u32Width = 300;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	for (i = OverlayMinHandle; i < OverlayMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateOverLayEx(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = OVERLAYEX_RGN;
	stRegion.unAttr.stOverlayEx.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlayEx.stSize.u32Height = 200;
	stRegion.unAttr.stOverlayEx.stSize.u32Width = 300;
	stRegion.unAttr.stOverlayEx.u32BgColor = 0x00000000; // ARGB1555 transparent
	stRegion.unAttr.stOverlayEx.u32CanvasNum = 2;
	stRegion.unAttr.stOverlayEx.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	for (i = OverlayExMinHandle; i < OverlayExMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateCover(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = COVER_RGN;

	for (i = CoverMinHandle; i < CoverMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateCoverEx(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = COVEREX_RGN;

	for (i = CoverExMinHandle; i < CoverExMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 REGION_CreateMosaic(CVI_S32 HandleNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;
	RGN_ATTR_S stRegion;

	stRegion.enType = MOSAIC_RGN;

	for (i = MosaicMinHandle; i < MosaicMinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_Create(CVI_S32 HandleNum, RGN_TYPE_E enType, PIXEL_FORMAT_E pixelFormat)
{
	CVI_S32 s32Ret;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	switch (enType) {
	case OVERLAY_RGN:
		s32Ret = REGION_CreateOverLay(HandleNum, pixelFormat);
		break;
	case OVERLAYEX_RGN:
		s32Ret = REGION_CreateOverLayEx(HandleNum);
		break;
	case COVER_RGN:
		s32Ret = REGION_CreateCover(HandleNum);
		break;
	case COVEREX_RGN:
		s32Ret = REGION_CreateCoverEx(HandleNum);
		break;
	case MOSAIC_RGN:
		s32Ret = REGION_CreateMosaic(HandleNum);
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}
	if (s32Ret != CVI_SUCCESS) {
		printf("%s failed! HandleNum%d, entype:%d!\n", __func__, HandleNum, enType);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 REGION_AttachToChn(RGN_HANDLE Handle, MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_AttachToChn(Handle, pstChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 REGION_DetachFromChn(RGN_HANDLE Handle, MMF_CHN_S *pstChn)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_DetachFromChn(Handle, pstChn);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_DetachFromChn failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_AttachToChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn)
{
	CVI_S32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHadle;
	RGN_CHN_ATTR_S stChnAttr;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	if (pstChn == CVI_NULL) {
		printf("pstChn is NULL !\n");
		return CVI_FAILURE;
	}
	memset(&stChnAttr, 0, sizeof(stChnAttr));

	/*set the chn config*/
	stChnAttr.bShow = CVI_TRUE;
	switch (enType) {
	case OVERLAY_RGN:
		MinHadle = OverlayMinHandle;
		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = OVERLAY_RGN;
		stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
		break;
	case OVERLAYEX_RGN:
		MinHadle = OverlayExMinHandle;
		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = OVERLAYEX_RGN;
		stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.bInvColEn = CVI_FALSE;
		break;
	case COVER_RGN:
		MinHadle = CoverMinHandle;

		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = COVER_RGN;
		stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;

		stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
		stChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 100;

		stChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ffff;

		stChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
		break;
	case COVEREX_RGN:
		MinHadle = CoverExMinHandle;

		stChnAttr.bShow = CVI_TRUE;
		stChnAttr.enType = COVEREX_RGN;
		stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;

		stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 100;
		stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width = 100;

		stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x0000ffff;
		break;
	case MOSAIC_RGN:
		MinHadle = MosaicMinHandle;
		stChnAttr.enType = MOSAIC_RGN;
		stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8;
		stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 96; // 8 pixel align
		stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width = 96;
		break;
	default:
		return CVI_FAILURE;
	}
	/*attach to Chn*/
	for (i = MinHadle; i < MinHadle + HandleNum; i++) {
		if (enType == OVERLAY_RGN) {
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20 + 200 * (i - OverlayMinHandle);
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20 + 200 * (i - OverlayMinHandle);
			stChnAttr.unChnAttr.stOverlayChn.u32Layer = (i - OverlayMinHandle);
		}
		if (enType == OVERLAYEX_RGN) {
			stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 20 + 200 * (i - OverlayExMinHandle);
			stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 20 + 200 * (i - OverlayExMinHandle);
			stChnAttr.unChnAttr.stOverlayExChn.u32Layer = i - OverlayExMinHandle;
		}
		if (enType == COVER_RGN) {
			stChnAttr.unChnAttr.stCoverChn.stRect.s32X = 20 + 200 * (i - CoverMinHandle);
			stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 20 + 200 * (i - CoverMinHandle);
			stChnAttr.unChnAttr.stCoverChn.u32Layer = (i - CoverMinHandle);
		}
		if (enType == COVEREX_RGN) {
			stChnAttr.unChnAttr.stCoverExChn.stRect.s32X = 20 + 200 * (i - CoverExMinHandle);
			stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = 20 + 200 * (i - CoverExMinHandle);
			stChnAttr.unChnAttr.stCoverExChn.u32Layer = i - CoverExMinHandle;
		}
		if (enType == MOSAIC_RGN) {
			stChnAttr.unChnAttr.stMosaicChn.stRect.s32X = 20 + 200 * (i - MosaicMinHandle);
			stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = 20 + 200 * (i - MosaicMinHandle);
			stChnAttr.unChnAttr.stMosaicChn.u32Layer = i - MosaicMinHandle;
		}
		s32Ret = REGION_AttachToChn(i, pstChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("REGION_AttachToChn failed!\n");
			break;
		}
	}
	/*detach region from chn */
	if (s32Ret != CVI_SUCCESS && i > 0) {
		i--;
		for (; i >= MinHadle; i--)
			s32Ret = REGION_DetachFromChn(i, pstChn);
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_GetMinHandle(RGN_TYPE_E enType)
{
	CVI_S32 MinHandle;

	switch (enType) {
	case OVERLAY_RGN:
		MinHandle = OverlayMinHandle;
		break;
	case OVERLAYEX_RGN:
		MinHandle = OverlayExMinHandle;
		break;
	case COVER_RGN:
		MinHandle = CoverMinHandle;
		break;
	case COVEREX_RGN:
		MinHandle = CoverExMinHandle;
		break;
	case MOSAIC_RGN:
		MinHandle = MosaicMinHandle;
		break;
	default:
		MinHandle = -1;
		break;
	}
	return MinHandle;
}

CVI_S32 RGN_COMM_REGION_MST_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
				CVI_U32 u16FilColor, SIZE_S *pstSize, CVI_U32 u32Stride, PIXEL_FORMAT_E enPixelFormat)
{
	OSD_SURFACE_S Surface;
	OSD_BITMAPFILEHEADER bmpFileHeader;
	OSD_BITMAPINFO bmpInfo;

	if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
		printf("GetBmpInfo err!\n");
		return CVI_FAILURE;
	}

	if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
	} else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
	} else if (enPixelFormat == PIXEL_FORMAT_8BIT_MODE) {
		Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
	} else {
		printf("Pixel format is not support!\n");
		return CVI_FAILURE;
	}

	if (pstBitmap->pData == NULL) {
		printf("malloc osd memory err!\n");
		return CVI_FAILURE;
	}

	CreateSurfaceByCanvas(filename, &Surface, (CVI_U8 *)(pstBitmap->pData)
			     , pstSize->u32Width, pstSize->u32Height, u32Stride);

	pstBitmap->u32Width = Surface.u16Width;
	pstBitmap->u32Height = Surface.u16Height;
	pstBitmap->enPixelFormat = enPixelFormat;

	// if pixel value match color, make it transparent.
	// Only works for ARGB1555
	if (bFil) {
		CVI_U32 i, j;
		CVI_U16 *pu16Temp;

		pu16Temp = (CVI_U16 *)pstBitmap->pData;
		for (i = 0; i < pstBitmap->u32Height; i++) {
			for (j = 0; j < pstBitmap->u32Width; j++) {
				if (u16FilColor == *pu16Temp)
					*pu16Temp &= 0x7FFF;

				pu16Temp++;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_VPSS_SendFrame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	FILE *fp;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;

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
		printf("%s: Can't acquire vb block\n", __func__);
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		printf("open data file error\n");
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
			printf("vpss send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	printf("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);
	printf("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2]);
	printf("vir addr(%p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2]);

	fclose(fp);

	printf("read file done and send out frame.\n");
	CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, -1);
	CVI_VB_ReleaseBlock(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	return CVI_SUCCESS;
}

CVI_BOOL RGN_COMM_FRAME_CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, plane_len, data_len;
	CVI_U32 u32LumaData, u32ChromaData = 0, data_height;
	bool result = true;
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
		printf("open data file, %s, error\n", filename);
		return false;
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

		u32len = fread(buffer, plane_len, 1, fp);
		if (u32len <= 0) {
			printf("fread data(%d) error\n", i);
			result = false;
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

				result = false;
				break;
			}
			offset += pstVideoFrame->stVFrame.u32Stride[i];
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return result;
}

CVI_S32 RGN_COMM_FRAME_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		UT_PRT("open data file error\n");
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

		UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		UT_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			UT_PRT("fwrite data(%d) error\n", i);
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 RGN_COMM_REGION_DetachFrmChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn)
{
	CVI_S32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHadle;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	if (pstChn == CVI_NULL) {
		printf("pstChn is NULL !\n");
		return CVI_FAILURE;
	}
	switch (enType) {
	case OVERLAY_RGN:
		MinHadle = OverlayMinHandle;
		break;
	case OVERLAYEX_RGN:
		MinHadle = OverlayExMinHandle;
		break;
	case COVER_RGN:
		MinHadle = CoverMinHandle;
		break;
	case COVEREX_RGN:
		MinHadle = CoverExMinHandle;
		break;
	case MOSAIC_RGN:
		MinHadle = MosaicMinHandle;
		break;
	default:
		return CVI_FAILURE;
	}
	for (i = MinHadle; i < MinHadle + HandleNum; i++) {
		s32Ret = REGION_DetachFromChn(i, pstChn);
		if (s32Ret != CVI_SUCCESS)
			printf("REGION_DetachFromChn failed! Handle:%d\n", i);
	}
	return s32Ret;
}

CVI_S32 REGION_Destroy(RGN_HANDLE Handle)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_Destroy(Handle);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_Destroy failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_REGION_Destroy(CVI_S32 HandleNum, RGN_TYPE_E enType)
{
	CVI_S32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 MinHandle;

	if (HandleNum <= 0 || HandleNum > 16) {
		printf("HandleNum is illegal %d!\n", HandleNum);
		return CVI_FAILURE;
	}
	if (enType < OVERLAY_RGN || enType >= RGN_BUTT) {
		printf("enType is illegal %d!\n", enType);
		return CVI_FAILURE;
	}
	switch (enType) {
	case OVERLAY_RGN:
		MinHandle = OverlayMinHandle;
		break;
	case OVERLAYEX_RGN:
		MinHandle = OverlayExMinHandle;
		break;
	case COVER_RGN:
		MinHandle = CoverMinHandle;
		break;
	case COVEREX_RGN:
		MinHandle = CoverExMinHandle;
		break;
	case MOSAIC_RGN:
		MinHandle = MosaicMinHandle;
		break;
	default:
		return CVI_FAILURE;
	}
	for (i = MinHandle; i < MinHandle + HandleNum; i++) {
		s32Ret = REGION_Destroy(i);
		if (s32Ret != CVI_SUCCESS)
			printf("%s failed with %#x\n", __func__, s32Ret);
	}
	return s32Ret;
}

CVI_S32 RGN_COMM_VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable)
{
	CVI_S32 j;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_CHN VpssChn;

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
			if (s32Ret != CVI_SUCCESS) {
				printf("Vpss stop Grp %d channel %d failed! Please check param\n",
				VpssGrp, VpssChn);
				return CVI_FAILURE;
			}
		}
	}

	s32Ret = CVI_VPSS_StopGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("Vpss Stop Grp %d failed! Please check param\n", VpssGrp);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		printf("Vpss Destroy Grp %d failed! Please check\n", VpssGrp);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 REGION_SetBitMap(RGN_HANDLE Handle, BITMAP_S *pstBitmap)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_RGN_SetBitMap(Handle, pstBitmap);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	return s32Ret;
}



CVI_S32 RGN_COMM_ODEC_REGION_Create(CVI_U32 u32FileSize, SIZE_S *stSize)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_ATTR_S stRegion;

	memset(&stRegion, 0, sizeof(stRegion));
	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_8888;
	stRegion.unAttr.stOverlay.stSize.u32Height = stSize->u32Height;
	stRegion.unAttr.stOverlay.stSize.u32Width = stSize->u32Width;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize = u32FileSize;
	s32Ret = CVI_RGN_Create(OdecHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS)
		printf("CVI_RGN_Create failed with %#x, hdl(%d)\n", s32Ret, OdecHandle);

	return s32Ret;
}

CVI_S32 RGN_COMM_ODEC_REGION_Destroy(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = REGION_Destroy(OdecHandle);
	if (s32Ret != CVI_SUCCESS)
		printf("REGION_Destroy failed with %#x, hdl(%d)\n",
			s32Ret, OdecHandle);
	return s32Ret;
}

CVI_S32 RGN_COMM_ODEC_REGION_AttachToChn(MMF_CHN_S *pstChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_CHN_ATTR_S stChnAttr;

	if (pstChn == CVI_NULL) {
		printf("pstChn is NULL !\n");
		return CVI_FAILURE;
	}

	/*set the chn config*/
	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = REGION_AttachToChn(OdecHandle, pstChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS)
		printf("REGION_AttachToChn failed with %#x, hdl(%d), chn(%d %d %d)\n",
			s32Ret, OdecHandle, pstChn->enModId, pstChn->s32DevId, pstChn->s32ChnId);

	return s32Ret;
}

CVI_S32 RGN_COMM_ODEC_REGION_DetachFrmChn(MMF_CHN_S *pstChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstChn == CVI_NULL) {
		printf("pstChn is NULL !\n");
		return CVI_FAILURE;
	}

	s32Ret = REGION_DetachFromChn(OdecHandle, pstChn);
	if (s32Ret != CVI_SUCCESS)
		printf("REGION_DetachFromChn failedwith %#x, hdl(%d), chn(%d %d %d)\n",
			s32Ret, OdecHandle, pstChn->enModId, pstChn->s32DevId, pstChn->s32ChnId);
	return s32Ret;
}

static CVI_VOID get_chroma_size_shift_factor(PIXEL_FORMAT_E enPixelFormat,
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

CVI_S32 RGN_COMM_CompareWithMD5(const CVI_CHAR *md5sum, VIDEO_FRAME_INFO_S *pstVideoFrame)
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
	CVI_CHAR md_str[32];
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

	get_chroma_size_shift_factor(pstVFrame->enPixelFormat, &c_w_shift,
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
		s32Index += snprintf(p + s32Index, 32, "%02x", md[i]);

	if (strncmp(md5sum, md_str, 32)) {
		printf("md5sum error, frame md5sum:%s\n", md_str);
		result = CVI_FAILURE;
	}

	return result;
}

CVI_S32 RGN_COMM_FileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize,
	PIXEL_FORMAT_E enPixelFormat, const CVI_CHAR *filename)
{
CVI_S32 s32Ret = CVI_SUCCESS;
VIDEO_FRAME_INFO_S stVideoFrame;
VB_BLK blk;
CVI_U32 u32len;
VB_CAL_CONFIG_S stVbCalConfig;
FILE *fp;

COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
	, COMPRESS_MODE_NONE, 64, &stVbCalConfig);

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