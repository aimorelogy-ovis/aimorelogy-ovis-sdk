#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <cvi_math.h>
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_region.h"
#include "cvi_comm_osdc.h"
#include "cvi_osdc.h"
#include "msg_rgn.h"
#include "cvi_msg_client.h"

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

struct rgn_canvas {
	STAILQ_ENTRY(rgn_canvas) stailq;
	RGN_HANDLE Handle;
	CVI_U64 u64PhyAddr;
	CVI_U8 *pu8VirtAddr;
	CVI_U32 u32Size;
};

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t canvas_q_lock = PTHREAD_MUTEX_INITIALIZER;
STAILQ_HEAD(rgn_canvas_q, rgn_canvas) canvas_q;

CVI_S32 get_rgn_fd(CVI_VOID)
{
	return MODFD2(CVI_ID_RGN, 0, 0, 0);
}

static void rgn_init(void)
{
	STAILQ_INIT(&canvas_q);
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 platform_rgn_create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);

	// Driver control
	fd = get_rgn_fd();
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_CREATE,
		(CVI_VOID *)pstRegion, sizeof(RGN_ATTR_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Create RGN fail.\n");
		return s32Ret;
	}

	pthread_mutex_lock(&canvas_q_lock);
	pthread_once(&once, rgn_init);
	pthread_mutex_unlock(&canvas_q_lock);

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_destroy(RGN_HANDLE Handle)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;

	fd = get_rgn_fd();
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_DESTROY, NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Destroy RGN fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_getattr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);

	// Driver control
	fd = MODFD2(CVI_ID_RGN, 0, 0, 1);
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_GET_ATTR,
		pstRegion, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setattr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);
	// Driver control
	fd = get_rgn_fd();
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_SET_ATTR,
				(CVI_VOID *)pstRegion, sizeof(RGN_ATTR_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setbitmap(RGN_HANDLE Handle, const BITMAP_S *pstBitmap)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	BITMAP_S stBitmapTemp;
	CVI_U64 u64PhyAddr;
	CVI_VOID *pVirAddr;
	RGN_ATTR_S stRegion;
	CVI_U32 u32Len;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstBitmap);

	switch (pstBitmap->enPixelFormat) {
	case PIXEL_FORMAT_ARGB_8888: {
		u32Len = pstBitmap->u32Width * pstBitmap->u32Height * 4;
		break;
	}

	case PIXEL_FORMAT_8BIT_MODE: {
		u32Len = pstBitmap->u32Width * pstBitmap->u32Height;
		break;
	}

	case PIXEL_FORMAT_4BIT_MODE: {
		u32Len = pstBitmap->u32Width * pstBitmap->u32Height * 0.5;
		break;
	}

	case PIXEL_FORMAT_ARGB_4444:
	case PIXEL_FORMAT_ARGB_1555:
	default: {
		u32Len = pstBitmap->u32Width * pstBitmap->u32Height * 2;
		break;
	}
	}

	// Driver control
	fd = get_rgn_fd();
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_GET_ATTR,
				&stRegion, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	memcpy(&stBitmapTemp, pstBitmap, sizeof(BITMAP_S));
	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_SW) {
		u32Len = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	} else if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		u32Len = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	}

	s32Ret = CVI_SYS_IonAlloc_Cached(&u64PhyAddr, &pVirAddr, "CVI_RGN_SetBitMap", u32Len);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "IonAlloc(%d) failed with s32Ret:%x\n", u32Len, s32Ret);
		return s32Ret;
	}

	memcpy(pVirAddr, pstBitmap->pData, u32Len);
	CVI_SYS_IonFlushCache(u64PhyAddr, pVirAddr, u32Len);
	stBitmapTemp.pData = (CVI_VOID *)(unsigned long)u64PhyAddr;
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_SET_BITMAP,
				&stBitmapTemp, sizeof(BITMAP_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN Bitmap fail.\n");
	}

	CVI_SYS_IonFree(u64PhyAddr, pVirAddr);

	return s32Ret;
}

CVI_S32 platform_rgn_attachtochn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_VOID *pvMsg;
	CVI_U32 u32Len;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

#ifdef __CV180X__
	if (pstChn->enModId == CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No vo device, cannot attach to vo!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
#endif

	u32Len = sizeof(MMF_CHN_S) + sizeof(RGN_CHN_ATTR_S);
	pvMsg = calloc(u32Len, 1);
	if (!pvMsg) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return CVI_ERR_RGN_NOMEM;
	}

	// Driver control
	fd = get_rgn_fd();
	stPrivData.as32PrivData[0] = Handle;
	memcpy(pvMsg, (CVI_VOID *)pstChn, sizeof(MMF_CHN_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S), (CVI_VOID *)pstChnAttr, sizeof(RGN_CHN_ATTR_S));

	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_ATTACH_TO_CHN,
				pvMsg, u32Len, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Attach RGN to channel fail.\n");
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_detachfromchn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);

#ifdef __CV180X__
	if (pstChn->enModId == CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No vo device, cannot detach from vo!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
#endif

	// Driver control
	fd = MODFD2(CVI_ID_RGN, 0, 0, 1);
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_DETACH_FROM_CHN,
				(CVI_VOID *)pstChn, sizeof(MMF_CHN_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Detach RGN from channel fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setdisplayattr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_VOID *pvMsg;
	CVI_U32 u32Len;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	u32Len = sizeof(MMF_CHN_S) + sizeof(RGN_CHN_ATTR_S);
	pvMsg = calloc(u32Len, 1);
	if (!pvMsg) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return CVI_ERR_RGN_NOMEM;
	}

	stPrivData.as32PrivData[0] = Handle;
	memcpy(pvMsg, (CVI_VOID *)pstChn, sizeof(MMF_CHN_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S), (CVI_VOID *)pstChnAttr, sizeof(RGN_CHN_ATTR_S));

	// Driver control
	fd = get_rgn_fd();
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_SET_DISP_ATTR,
				pvMsg, u32Len, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set display RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_getdisplayattr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = -1, s32Ret;
	MSG_PRIV_DATA_S stPrivData;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	// Driver control
	fd = MODFD2(CVI_ID_RGN, 0, 0, 1);
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync2(fd, MSG_CMD_RGN_GET_DISP_ATTR,
				(CVI_VOID *)pstChn, sizeof(MMF_CHN_S), pstChnAttr, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get display RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static inline CVI_S32 rgn_get_bytesperline(PIXEL_FORMAT_E enPixelFormat, CVI_U32 width, CVI_U32 *bytesperline)
{
	switch (enPixelFormat) {
	case PIXEL_FORMAT_ARGB_8888:
		*bytesperline = width << 2;
		break;
	case PIXEL_FORMAT_ARGB_4444:
	case PIXEL_FORMAT_ARGB_1555:
		*bytesperline = width << 1;
		break;
	case PIXEL_FORMAT_8BIT_MODE:
		*bytesperline = width;
		break;
	case PIXEL_FORMAT_4BIT_MODE:
		*bytesperline = width >> 1;
		break;
	default:
		CVI_TRACE_RGN(CVI_DBG_ERR, "not supported pxl-fmt(%d).\n", enPixelFormat);
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static CVI_S32 cvi_rgn_get_ion_len(CVI_S32 fd, RGN_HANDLE Handle, CVI_S32 *pS32Len)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	RGN_ATTR_S stRegion;
	CVI_U32 bytesperline = 0;
	CVI_U32 u32Stride = 0;

	// Driver control
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_GET_ATTR,
				&stRegion, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "failed with s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_SW) {
		*pS32Len = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	} else if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		*pS32Len = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	} else {
		PIXEL_FORMAT_E pixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		SIZE_S rgnSize = stRegion.unAttr.stOverlay.stSize;
		rgn_get_bytesperline(pixelFormat, rgnSize.u32Width, &bytesperline);
		u32Stride = ALIGN(bytesperline, 32);
		*pS32Len = u32Stride * rgnSize.u32Height;
	}

	return s32Ret;
}

CVI_S32 platform_rgn_getcanvasinfo(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo)
{
	CVI_S32 fd = -1, s32Ret, s32IonLen;
	struct rgn_canvas *canvas;
	MSG_PRIV_DATA_S stPrivData;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstCanvasInfo);

	// Driver control
	fd = MODFD2(CVI_ID_RGN, 0, 0, 1);

	s32Ret = cvi_rgn_get_ion_len(fd, Handle, &s32IonLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_WARN, "Get RGN ion length fail.\n");
		return s32Ret;
	}

	stPrivData.as32PrivData[0] = Handle;

	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_GET_CANVAS_INFO,
					pstCanvasInfo, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN canvas information fail.\n");
		return s32Ret;
	}

	canvas = calloc(1, sizeof(struct rgn_canvas));
	if (!canvas) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "malloc failed.\n");
		return CVI_ERR_RGN_NOMEM;
	}
	pthread_mutex_lock(&canvas_q_lock);
	canvas->u64PhyAddr = pstCanvasInfo->u64PhyAddr;
	canvas->u32Size = s32IonLen;
	pstCanvasInfo->pu8VirtAddr = canvas->pu8VirtAddr =
			CVI_SYS_MmapCache(pstCanvasInfo->u64PhyAddr, canvas->u32Size);
	if (pstCanvasInfo->pu8VirtAddr == NULL) {
		free(canvas);
		CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_SYS_MmapCache NG.\n");
		return CVI_FAILURE;
	}
	canvas->Handle = Handle;
	pstCanvasInfo->pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)pstCanvasInfo->pu8VirtAddr;
	pstCanvasInfo->pstObjAttr = (RGN_CMPR_OBJ_ATTR_S *)(pstCanvasInfo->pu8VirtAddr +
			sizeof(RGN_CANVAS_CMPR_ATTR_S));
	STAILQ_INSERT_TAIL(&canvas_q, canvas, stailq);
	pthread_mutex_unlock(&canvas_q_lock);

	return CVI_SUCCESS;
}

struct cvi_rgn_bitmap {
	CVI_VOID *pBitmapVAddr;
	CVI_U32 u32BitmapSize;
};
CVI_S32 platform_rgn_updatecanvas(RGN_HANDLE Handle)
{
	CVI_S32 fd = -1, s32Ret;
	struct rgn_canvas *canvas;

	MSG_PRIV_DATA_S stPrivData;

	// Driver control
	fd = MODFD2(CVI_ID_RGN, 0, 0, 1);

#if !defined(CONFIG_OSDC_DUAL_906)
	RGN_ATTR_S stRegion;
	struct cvi_rgn_bitmap *pstBitmaps;
	CVI_U32 u32Bpp, i = 0, j = 0;
	CVI_S32 s32IonLen;

	s32Ret = cvi_rgn_get_ion_len(fd, Handle, &s32IonLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_WARN, "Get RGN ion length fail.\n");
		return s32Ret;
	}

	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_GET_ATTR,
				&stRegion, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN attr fail.\n");
		return s32Ret;
	}

	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(canvas, &canvas_q, stailq) {
			if (canvas->Handle == Handle) {
				break;
			}
		}
	} else {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No corresponding Handle(%d) found.\n", Handle);
		pthread_mutex_unlock(&canvas_q_lock);
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
	pthread_mutex_unlock(&canvas_q_lock);

	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr = NULL;
		RGN_CMPR_OBJ_ATTR_S *pstObjAttr = NULL;
		CVI_S32 status;
		OSDC_Canvas_Attr_S osdc_canvas;
		OSDC_DRAW_OBJ_S *obj_vec;
		CVI_U32 bs_size;

		//cmpr canvas info is stored in ion which needs to be flush before reading
		CVI_SYS_IonFlushCache(canvas->u64PhyAddr,
			canvas->pu8VirtAddr, canvas->u32Size);
		pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)canvas->pu8VirtAddr;
		pstObjAttr = (RGN_CMPR_OBJ_ATTR_S *)(canvas->pu8VirtAddr +
			sizeof(RGN_CANVAS_CMPR_ATTR_S));

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "start(%d %d) end(%d %d) Thick(%d) Color(0x%x)\n",
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick,
					pstObjAttr[i].stLine.u32Color);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG,
					"xywh(%d %d %d %d) Thick(%d) Color(0x%x) is_fill(%d)\n",
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32Thick,
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.u32IsFill);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "xywh(%d %d %d %d) u64BitmapPAddr(%"PRIx64")\n",
					pstObjAttr[i].stBitmap.stRect.s32X,
					pstObjAttr[i].stBitmap.stRect.s32Y,
					pstObjAttr[i].stBitmap.stRect.u32Width,
					pstObjAttr[i].stBitmap.stRect.u32Height,
					pstObjAttr[i].stBitmap.u64BitmapPAddr);
			}
		}

		osdc_canvas.width = stRegion.unAttr.stOverlay.stSize.u32Width;
		osdc_canvas.height = stRegion.unAttr.stOverlay.stSize.u32Height;
		osdc_canvas.bg_color_code = stRegion.unAttr.stOverlay.u32BgColor;
		switch (stRegion.unAttr.stOverlay.enPixelFormat) {
		case PIXEL_FORMAT_ARGB_8888:
			osdc_canvas.format = OSD_ARGB8888;
			u32Bpp = 4;
			break;

		case PIXEL_FORMAT_ARGB_4444:
			osdc_canvas.format = OSD_ARGB4444;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_ARGB_1555:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_8BIT_MODE:
			osdc_canvas.format = OSD_LUT8;
			u32Bpp = 1;
			break;
		case PIXEL_FORMAT_4BIT_MODE:
			osdc_canvas.format = OSD_LUT4;
			u32Bpp = 0;
			break;

		default:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;
		}

		obj_vec = calloc(sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum, 1);
		if (!obj_vec) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum);
			return CVI_ERR_RGN_NOBUF;
		}
		pstBitmaps = (struct cvi_rgn_bitmap *)calloc(pstCanvasCmprAttr->u32ObjNum,
						sizeof(struct cvi_rgn_bitmap));
		if (!pstBitmaps) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							pstCanvasCmprAttr->u32ObjNum * sizeof(struct cvi_rgn_bitmap));
			free(obj_vec);
			return CVI_ERR_RGN_NOBUF;
		}

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				CVI_OSDC_SetLineObjAttr(&osdc_canvas, &obj_vec[i],
				pstObjAttr[i].stLine.u32Color,
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				CVI_OSDC_SetRectObjAttr(&osdc_canvas, &obj_vec[i],
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32IsFill,
					pstObjAttr[i].stRgnRect.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				if (u32Bpp == 0) {
					// 4bit mode: 2 pixels per byte
					pstBitmaps[j].u32BitmapSize = (pstObjAttr[i].stBitmap.stRect.u32Width *
									pstObjAttr[i].stBitmap.stRect.u32Height + 1) / 2;
				} else {
					pstBitmaps[j].u32BitmapSize = pstObjAttr[i].stBitmap.stRect.u32Width *
									pstObjAttr[i].stBitmap.stRect.u32Height * u32Bpp;
				}
				pstBitmaps[j].pBitmapVAddr = CVI_SYS_MmapCache(pstObjAttr[i].stBitmap.u64BitmapPAddr,
								pstBitmaps[j].u32BitmapSize);

					CVI_OSDC_SetBitmapObjAttr(&osdc_canvas, &obj_vec[i],
						pstBitmaps[j++].pBitmapVAddr,
						pstObjAttr[i].stBitmap.stRect.s32X,
						pstObjAttr[i].stBitmap.stRect.s32Y,
						pstObjAttr[i].stBitmap.stRect.u32Width,
						pstObjAttr[i].stBitmap.stRect.u32Height,
						false);
			}
		}

		status = CVI_OSDC_DrawCmprCanvas(&osdc_canvas, &obj_vec[0], pstCanvasCmprAttr->u32ObjNum,
			canvas->pu8VirtAddr, s32IonLen,  &bs_size);
		if (status != 1) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Region(%d) needs ion size(%d), current size(%d)!\n",
				Handle, bs_size, s32IonLen);
			CVI_OSDC_DrawCmprCanvas(&osdc_canvas, &obj_vec[0], 0,
				canvas->pu8VirtAddr, s32IonLen,  &bs_size);
		}
		if (j) {
			for (i = 0; i < j; i++) {
				CVI_SYS_Munmap(pstBitmaps[i].pBitmapVAddr, pstBitmaps[i].u32BitmapSize);
			}
		}
		free(pstBitmaps);
		// store bitstream size in bit[32:63], after driver gets it,
		// it should be restored to image width and height
		/*use ioctl instead*/
		// *((unsigned int *)canvas->pu8VirtAddr + 1) = bs_size;
		stPrivData.as32PrivData[0] = Handle;
		stPrivData.as32PrivData[1] = bs_size;
		s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_SET_IONSIZE,
					NULL, 0, &stPrivData);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Set compress size fail.\n");
			pthread_mutex_unlock(&canvas_q_lock);
			return s32Ret;
		}

		free(obj_vec);
		stPrivData.as32PrivData[0] = Handle;
		stPrivData.as32PrivData[1] = osdc_canvas.width;
		stPrivData.as32PrivData[2] = osdc_canvas.height;
	}

	CVI_SYS_IonFlushCache(canvas->u64PhyAddr, canvas->pu8VirtAddr, canvas->u32Size);
#else
	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(canvas, &canvas_q, stailq) {
			if (canvas->Handle == Handle) {
				break;
			}
		}
	} else {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No corresponding Handle(%d) found.\n", Handle);
		pthread_mutex_unlock(&canvas_q_lock);
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
	pthread_mutex_unlock(&canvas_q_lock);

	CVI_SYS_IonFlushCache(canvas->u64PhyAddr,
		canvas->pu8VirtAddr, canvas->u32Size);
#endif
	stPrivData.as32PrivData[0] = Handle;
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_UPDATE_CANVAS,
					NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Update RGN canvas fail.\n");
		pthread_mutex_unlock(&canvas_q_lock);
		return s32Ret;
	}

	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(canvas, &canvas_q, stailq) {
			if (canvas->Handle == Handle) {
				STAILQ_REMOVE(&canvas_q, canvas, rgn_canvas, stailq);
				CVI_SYS_Munmap(canvas->pu8VirtAddr, canvas->u32Size);
				free(canvas);
				break;
			}
		}
	}
	pthread_mutex_unlock(&canvas_q_lock);

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_invertcolor(RGN_HANDLE Handle, MMF_CHN_S *pstChn, CVI_U32 *pu32Color)
{
	// CVI_S32 fd = -1, s32Ret;
	(void)(Handle);
	(void)(pstChn);
	(void)(pu32Color);

	// MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	// MOD_CHECK_NULL_PTR(CVI_ID_RGN, pu32Color);

	// // Driver control
	// fd = get_rgn_fd();
	// s32Ret = rgn_invert_color(fd, Handle, pstChn, (void *)pu32Color);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_RGN(CVI_DBG_ERR, "Invert RGN color fail.\n");
	// 	return CVI_FAILURE;
	// }

	return CVI_SUCCESS;
}

CVI_S32 platform_rgn_setchnpalette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette)
{
	CVI_S32 fd = -1, s32Ret;
	CVI_U32 u32Len;
	CVI_VOID *pvMsg;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstPalette);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstPalette->pstPaletteTable);
	MSG_PRIV_DATA_S stPrivData;

	// Driver control
	fd = get_rgn_fd();

	u32Len = sizeof(MMF_CHN_S) + sizeof(RGN_PALETTE_S) + sizeof(RGN_RGBQUARD_S) * pstPalette->lut_length;
	pvMsg = calloc(u32Len, 1);
	if (!pvMsg) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return CVI_ERR_RGN_NOMEM;
	}
	stPrivData.as32PrivData[0] = Handle;
	memcpy(pvMsg, (CVI_VOID *)pstChn, sizeof(MMF_CHN_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S), (CVI_VOID *)pstPalette, sizeof(RGN_PALETTE_S));
	memcpy(pvMsg +  sizeof(MMF_CHN_S) + sizeof(RGN_PALETTE_S), (CVI_VOID *)pstPalette->pstPaletteTable,
			sizeof(RGN_RGBQUARD_S) * pstPalette->lut_length);
	s32Ret = CVI_MSG_SendSync(fd, MSG_CMD_RGN_SET_CHN_PALETTE,
				pvMsg, u32Len, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set chn palette fail.\n");
		return s32Ret;
	}

	free(pvMsg);
	pvMsg = NULL;
	return s32Ret;
}
