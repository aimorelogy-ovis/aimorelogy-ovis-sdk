#include "rgn_proc.h"

#define GENERATE_STRING(STRING)	(#STRING),
#define RGN_PROC_NAME "cvitek/rgn"

static const char *const MOD_STRING[] = FOREACH_MOD(GENERATE_STRING);
extern struct cvi_rgn_ctx rgn_prc_ctx[RGN_MAX_NUM];

/*************************************************************************
 *	Region proc functions
 *************************************************************************/
static void _pixFmt_to_String(enum _PIXEL_FORMAT_E PixFmt, char *str, int len)
{
	switch (PixFmt) {
	case PIXEL_FORMAT_RGB_888:
		strncpy(str, "RGB_888", len);
		break;
	case PIXEL_FORMAT_BGR_888:
		strncpy(str, "BGR_888", len);
		break;
	case PIXEL_FORMAT_RGB_888_PLANAR:
		strncpy(str, "RGB_888_PLANAR", len);
		break;
	case PIXEL_FORMAT_BGR_888_PLANAR:
		strncpy(str, "BGR_888_PLANAR", len);
		break;
	case PIXEL_FORMAT_ARGB_1555:
		strncpy(str, "ARGB_1555", len);
		break;
	case PIXEL_FORMAT_ARGB_4444:
		strncpy(str, "ARGB_4444", len);
		break;
	case PIXEL_FORMAT_ARGB_8888:
		strncpy(str, "ARGB_8888", len);
		break;
	case PIXEL_FORMAT_RGB_BAYER_8BPP:
		strncpy(str, "RGB_BAYER_8BPP", len);
		break;
	case PIXEL_FORMAT_RGB_BAYER_10BPP:
		strncpy(str, "RGB_BAYER_10BPP", len);
		break;
	case PIXEL_FORMAT_RGB_BAYER_12BPP:
		strncpy(str, "RGB_BAYER_12BPP", len);
		break;
	case PIXEL_FORMAT_RGB_BAYER_14BPP:
		strncpy(str, "RGB_BAYER_14BPP", len);
		break;
	case PIXEL_FORMAT_RGB_BAYER_16BPP:
		strncpy(str, "RGB_BAYER_16BPP", len);
		break;
	case PIXEL_FORMAT_YUV_PLANAR_422:
		strncpy(str, "YUV_PLANAR_422", len);
		break;
	case PIXEL_FORMAT_YUV_PLANAR_420:
		strncpy(str, "YUV_PLANAR_420", len);
		break;
	case PIXEL_FORMAT_YUV_PLANAR_444:
		strncpy(str, "YUV_PLANAR_444", len);
		break;
	case PIXEL_FORMAT_YUV_400:
		strncpy(str, "YUV_400", len);
		break;
	case PIXEL_FORMAT_HSV_888:
		strncpy(str, "HSV_888", len);
		break;
	case PIXEL_FORMAT_HSV_888_PLANAR:
		strncpy(str, "HSV_888_PLANAR", len);
		break;
	case PIXEL_FORMAT_NV12:
		strncpy(str, "NV12", len);
		break;
	case PIXEL_FORMAT_NV21:
		strncpy(str, "NV21", len);
		break;
	case PIXEL_FORMAT_NV16:
		strncpy(str, "NV16", len);
		break;
	case PIXEL_FORMAT_NV61:
		strncpy(str, "NV61", len);
		break;
	case PIXEL_FORMAT_YUYV:
		strncpy(str, "YUYV", len);
		break;
	case PIXEL_FORMAT_UYVY:
		strncpy(str, "UYVY", len);
		break;
	case PIXEL_FORMAT_YVYU:
		strncpy(str, "YVYU", len);
		break;
	case PIXEL_FORMAT_VYUY:
		strncpy(str, "VYUY", len);
		break;
	case PIXEL_FORMAT_FP32_C1:
		strncpy(str, "FP32_C1", len);
		break;
	case PIXEL_FORMAT_FP32_C3_PLANAR:
		strncpy(str, "FP32_C3_PLANAR", len);
		break;
	case PIXEL_FORMAT_INT32_C1:
		strncpy(str, "INT32_C1", len);
		break;
	case PIXEL_FORMAT_INT32_C3_PLANAR:
		strncpy(str, "INT32_C3_PLANAR", len);
		break;
	case PIXEL_FORMAT_UINT32_C1:
		strncpy(str, "UINT32_C1", len);
		break;
	case PIXEL_FORMAT_UINT32_C3_PLANAR:
		strncpy(str, "UINT32_C3_PLANAR", len);
		break;
	case PIXEL_FORMAT_BF16_C1:
		strncpy(str, "BF16_C1", len);
		break;
	case PIXEL_FORMAT_BF16_C3_PLANAR:
		strncpy(str, "BF16_C3_PLANAR", len);
		break;
	case PIXEL_FORMAT_INT16_C1:
		strncpy(str, "INT16_C1", len);
		break;
	case PIXEL_FORMAT_INT16_C3_PLANAR:
		strncpy(str, "INT16_C3_PLANAR", len);
		break;
	case PIXEL_FORMAT_UINT16_C1:
		strncpy(str, "UINT16_C1", len);
		break;
	case PIXEL_FORMAT_UINT16_C3_PLANAR:
		strncpy(str, "UINT16_C3_PLANAR", len);
		break;
	case PIXEL_FORMAT_INT8_C1:
		strncpy(str, "INT8_C1", len);
		break;
	case PIXEL_FORMAT_INT8_C3_PLANAR:
		strncpy(str, "INT8_C3_PLANAR", len);
		break;
	case PIXEL_FORMAT_UINT8_C1:
		strncpy(str, "UINT8_C1", len);
		break;
	case PIXEL_FORMAT_UINT8_C3_PLANAR:
		strncpy(str, "UINT8_C3_PLANAR", len);
		break;
	default:
		strncpy(str, "Unknown Fmt", len);
		break;
	}
}

int rgn_proc_show(CVI_S32 argc, char **argv)
{
	struct cvi_rgn_ctx *prgnCtx = rgn_prc_ctx;
	int i;
	char c[32];

	(void)argc;
	(void)argv;
	if (!prgnCtx) {
		printf("rgn_prc_ctx = NULL\n");
		return -1;
	}

	printf("\nModule: [RGN]\n");
	// Region status of overlay
	printf("\n------REGION STATUS OF OVERLAY--------------------------------------------\n");
	printf("%10s%10s%10s%20s%10s%10s%10s%20s%20s%10s%10s\n",
		"Hdl", "Type", "Used", "PiFmt", "W", "H", "BgColor", "Phy", "Virt", "Stride", "CnvsNum");

	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == OVERLAY_RGN && prgnCtx[i].bCreated) {
			memset(c, 0, sizeof(c));
			_pixFmt_to_String(prgnCtx[i].stRegion.unAttr.stOverlay.enPixelFormat, c, sizeof(c));

			printf("%7s%3d%10d%10s%20s%10d%10d%10x%20llx%20lx%10d%10d\n",
				"#",
				prgnCtx[i].Handle,
				prgnCtx[i].stRegion.enType,
				(prgnCtx[i].bUsed) ? "Y" : "N",
				c,
				prgnCtx[i].stRegion.unAttr.stOverlay.stSize.u32Width,
				prgnCtx[i].stRegion.unAttr.stOverlay.stSize.u32Height,
				prgnCtx[i].stRegion.unAttr.stOverlay.u32BgColor,
				prgnCtx[i].stCanvasInfo[prgnCtx[i].canvas_idx].u64PhyAddr,
				(uintptr_t)prgnCtx[i].stCanvasInfo[prgnCtx[i].canvas_idx].pu8VirtAddr,
				prgnCtx[i].stCanvasInfo[prgnCtx[i].canvas_idx].u32Stride,
				prgnCtx[i].stRegion.unAttr.stOverlay.u32CanvasNum);
		}
	}

	// Region chn status of overlay
	printf("\n------REGION CHN STATUS OF OVERLAY----------------------------------------\n");
	printf("%10s%10s%10s%10s%10s%10s%10s%10s%10s\n",
		"Hdl", "Type", "Mod", "Dev", "Chn", "bShow", "X", "Y", "Layer");

	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == OVERLAY_RGN && prgnCtx[i].bCreated && prgnCtx[i].bUsed) {
			printf("%7s%3d%10d%10s%10d%10d%10s%10d%10d%10d\n",
				"#",
				prgnCtx[i].Handle,
				prgnCtx[i].stRegion.enType,
				MOD_STRING[prgnCtx[i].stChn.enModId],
				prgnCtx[i].stChn.s32DevId,
				prgnCtx[i].stChn.s32ChnId,
				(prgnCtx[i].stChnAttr.bShow) ? "Y" : "N",
				prgnCtx[i].stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X,
				prgnCtx[i].stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y,
				prgnCtx[i].stChnAttr.unChnAttr.stOverlayChn.u32Layer);
		}
	}

	// Region status of cover
	printf("\n------REGION STATUS OF COVER----------------------------------------------\n");
	printf("%10s%10s%10s\n", "Hdl", "Type", "Used");
	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == COVER_RGN && prgnCtx[i].bCreated) {
			printf("%7s%3d%10d%10s\n", "#", prgnCtx[i].Handle, prgnCtx[i].stRegion.enType,
				(prgnCtx[i].bUsed) ? "Y" : "N");
		}
	}

	// Region chn status of rect cover
	printf("\n------REGION CHN STATUS OF RECT COVER-------------------------------------\n");
	printf("%10s%10s%10s%10s%10s%10s\n%10s%10s%10s%10s%10s%10s%10s\n\n",
		"Hdl", "Type", "Mod", "Dev", "Chn", "bShow",
		"X", "Y", "W", "H", "Color", "Layer", "CoorType");

	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == COVER_RGN && prgnCtx[i].bCreated && prgnCtx[i].bUsed
			&& prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.enCoverType == AREA_RECT) {
			printf("%7s%3d%10d%10s%10d%10d%10s\n%10d%10d%10d%10d%10X%10d%10s\n",
				"#",
				prgnCtx[i].Handle,
				prgnCtx[i].stRegion.enType,
				MOD_STRING[prgnCtx[i].stChn.enModId],
				prgnCtx[i].stChn.s32DevId,
				prgnCtx[i].stChn.s32ChnId,
				(prgnCtx[i].stChnAttr.bShow) ? "Y" : "N",
				prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.stRect.s32X,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.stRect.s32Y,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.stRect.u32Width,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.stRect.u32Height,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.u32Color,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.u32Layer,
				(prgnCtx[i].stChnAttr.unChnAttr.stCoverChn.enCoordinate == RGN_ABS_COOR) ?
					"ABS" : "RATIO");
		}
	}

	// Region status of coverex
	printf("\n------REGION STATUS OF COVEREX--------------------------------------------\n");
	printf("%10s%10s%10s\n", "Hdl", "Type", "Used");
	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == COVEREX_RGN && prgnCtx[i].bCreated) {
			printf("%7s%3d%10d%10s\n", "#", prgnCtx[i].Handle, prgnCtx[i].stRegion.enType,
				(prgnCtx[i].bUsed) ? "Y" : "N");
		}
	}

	// Region chn status of rect coverex
	printf("\n------REGION CHN STATUS OF RECT COVEREX-----------------------------------\n");
	printf("%10s%10s%10s%10s%10s%10s\n%10s%10s%10s%10s%10s%10s\n\n",
		"Hdl", "Type", "Mod", "Dev", "Chn", "bShow",
		"X", "Y", "W", "H", "Color", "Layer");

	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == COVEREX_RGN && prgnCtx[i].bCreated && prgnCtx[i].bUsed
			&& prgnCtx[i].stChnAttr.unChnAttr.stCoverExChn.enCoverType == AREA_RECT) {
			printf("%7s%3d%10d%10s%10d%10d%10s\n%10d%10d%10d%10d%10X%10d\n",
				"#",
				prgnCtx[i].Handle,
				prgnCtx[i].stRegion.enType,
				MOD_STRING[prgnCtx[i].stChn.enModId],
				prgnCtx[i].stChn.s32DevId,
				prgnCtx[i].stChn.s32ChnId,
				(prgnCtx[i].stChnAttr.bShow) ? "Y" : "N",
				prgnCtx[i].stChnAttr.unChnAttr.stCoverExChn.stRect.s32X,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverExChn.u32Color,
				prgnCtx[i].stChnAttr.unChnAttr.stCoverExChn.u32Layer);
		}
	}

	// Region status of overlayex
	printf("\n------REGION STATUS OF OVERLAYEX------------------------------------------\n");
	printf("%10s%10s%10s%20s%10s%10s%10s%20s%20s%10s%10s\n",
		"Hdl", "Type", "Used", "PiFmt", "W", "H", "BgColor", "Phy", "Virt", "Stride", "CnvsNum");

	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == OVERLAYEX_RGN && prgnCtx[i].bCreated) {
			memset(c, 0, sizeof(c));
			_pixFmt_to_String(prgnCtx[i].stRegion.unAttr.stOverlayEx.enPixelFormat, c, sizeof(c));

			printf("%7s%3d%10d%10s%20s%10d%10d%10x%20llx%20lx%10d%10d\n",
				"#",
				prgnCtx[i].Handle,
				prgnCtx[i].stRegion.enType,
				(prgnCtx[i].bUsed) ? "Y" : "N",
				c,
				prgnCtx[i].stRegion.unAttr.stOverlayEx.stSize.u32Width,
				prgnCtx[i].stRegion.unAttr.stOverlayEx.stSize.u32Height,
				prgnCtx[i].stRegion.unAttr.stOverlayEx.u32BgColor,
				prgnCtx[i].stCanvasInfo[prgnCtx[i].canvas_idx].u64PhyAddr,
				(uintptr_t)prgnCtx[i].stCanvasInfo[prgnCtx[i].canvas_idx].pu8VirtAddr,
				prgnCtx[i].stCanvasInfo[prgnCtx[i].canvas_idx].u32Stride,
				prgnCtx[i].stRegion.unAttr.stOverlayEx.u32CanvasNum);
		}
	}

	// Region chn status of overlayex
	printf("\n------REGION CHN STATUS OF OVERLAYEX--------------------------------------\n");
	printf("%10s%10s%10s%10s%10s%10s%10s%10s%10s\n",
		"Hdl", "Type", "Mod", "Dev", "Chn", "bShow", "X", "Y", "Layer");

	for (i = 0; i < RGN_MAX_NUM; ++i) {
		if (prgnCtx[i].stRegion.enType == OVERLAYEX_RGN && prgnCtx[i].bCreated && prgnCtx[i].bUsed) {
			printf("%7s%3d%10d%10s%10d%10d%10s%10d%10d%10d\n",
				"#",
				prgnCtx[i].Handle,
				prgnCtx[i].stRegion.enType,
				MOD_STRING[prgnCtx[i].stChn.enModId],
				prgnCtx[i].stChn.s32DevId,
				prgnCtx[i].stChn.s32ChnId,
				(prgnCtx[i].stChnAttr.bShow) ? "Y" : "N",
				prgnCtx[i].stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X,
				prgnCtx[i].stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y,
				prgnCtx[i].stChnAttr.unChnAttr.stOverlayExChn.u32Layer);
		}
	}

	return 0;
}

int rgn_proc_init(void)
{
	int ret = CVI_SUCCESS;

	/* create the /proc file */
	return ret;
}

int rgn_proc_remove(void)
{
	return CVI_SUCCESS;
}
