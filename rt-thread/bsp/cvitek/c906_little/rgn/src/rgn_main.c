#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "drv_rtoscmu.h"

#include "rgn_main.h"
#include "osdc.h"
#include "comm.h"

rt_mq_t xQueueRGN;
rt_mq_t xQueueRGNCmdqu;

#define ALIGN(x, a)    (((x) + (a) - 1) & ~((a) - 1))
// #define __RGN_DEBUG__

#ifdef __RGN_DEBUG__
#define rgn_printf printf
#else
#define rgn_printf(...)
#endif

void prvRGNRunTask(void *pvParameters)
{
	/* Remove compiler warning about unused parameter. */
	(void)pvParameters;
	cmdqu_t rtos_cmdq = {0};

	int i;
	void *pvAddr = NULL;
	unsigned char obj_num = 0, line_num = 0;
	int bs_size, status;
	RGN_CANVAS_CMPR_ATTR_S *canvas_cmpr_attr = NULL;
	RGN_CMPR_OBJ_ATTR_S *obj_attr = NULL;
	OSDC_Canvas_Attr_S canvas = {0};
	OSDC_DRAW_OBJ_S *obj_vec = NULL;

	rgn_printf("%s run\n", __func__);
	xQueueRGN = main_GetMODHandle(E_QUEUE_RGN);
	xQueueRGNCmdqu = main_GetMODHandle(E_QUEUE_CMDQU);

	for (;;) {
		rt_mq_recv(xQueueRGN, &rtos_cmdq, sizeof(rtos_cmdq), RT_WAITING_FOREVER);
		rgn_printf("%s ip=%d cmd=%d para=%#x\n",
		       __func__, rtos_cmdq.ip_id, rtos_cmdq.cmd_id, rtos_cmdq.param_ptr);
		/* send cmd back to Cmdqu task and send mailbox to linux */
		if (rtos_cmdq.ip_id != IP_RGN) {
			//send back the err msg
			rgn_printf("RGNRunTask got invalid ip_id[%d]\n", rtos_cmdq.ip_id);
			rt_mq_send(xQueueRGNCmdqu, &rtos_cmdq, sizeof(rtos_cmdq));
			goto WRONG_CMD_IP_ID;
		}

		// rgn_printf("xPortGetFreeHeapSize(%d)\n", xPortGetFreeHeapSize());

		canvas_cmpr_attr = (RGN_CANVAS_CMPR_ATTR_S *)rtos_cmdq.param_ptr;
		rt_hw_cpu_dcache_invalidate((void *)canvas_cmpr_attr, sizeof(RGN_CANVAS_CMPR_ATTR_S));
		rt_hw_cpu_dcache_invalidate((void *)canvas_cmpr_attr, canvas_cmpr_attr->u32BsSize);

		rgn_printf("phyAddr(%x) u32Width(%d) u32Height(%d) u32BgColor(%x) enPixelFormat(%d) u32ObjNum(%d)\n",
			rtos_cmdq.param_ptr,
			canvas_cmpr_attr->u32Width,
			canvas_cmpr_attr->u32Height,
			canvas_cmpr_attr->u32BgColor,
			canvas_cmpr_attr->enPixelFormat,
			canvas_cmpr_attr->u32ObjNum);

		canvas.width = canvas_cmpr_attr->u32Width;
		canvas.height = canvas_cmpr_attr->u32Height;
		obj_num = canvas_cmpr_attr->u32ObjNum;
		line_num = 0;

		obj_vec = (OSDC_DRAW_OBJ_S *)malloc((obj_num ? obj_num : 1) * sizeof(OSDC_DRAW_OBJ_S));
		if (obj_vec == NULL) {
			rgn_printf("(%s) malloc failed!\n", __func__);
			goto WRONG_CMD_IP_ID;
		}
		memset(obj_vec, 0, (obj_num ? obj_num : 1) * sizeof(OSDC_DRAW_OBJ_S));

		obj_attr = (RGN_CMPR_OBJ_ATTR_S *)((CVI_U8 *)rtos_cmdq.param_ptr + sizeof(RGN_CANVAS_CMPR_ATTR_S));
		if (obj_num) {
			for (i = 0; i < obj_num; ++i) {
				if (obj_attr[i].enObjType == RGN_CMPR_LINE) {
					rgn_printf("start(%d %d) end(%d %d) Thick(%d) Color(0x%x)\n",
						obj_attr[i].stLine.stPointStart.s32X,
						obj_attr[i].stLine.stPointStart.s32Y,
						obj_attr[i].stLine.stPointEnd.s32X,
						obj_attr[i].stLine.stPointEnd.s32Y,
						obj_attr[i].stLine.u32Thick,
						obj_attr[i].stLine.u32Color);
					line_num++;
				} else if (obj_attr[i].enObjType == RGN_CMPR_RECT) {
					rgn_printf("xywh(%d %d %d %d) Thick(%d) Color(0x%x) is_fill(%d)\n",
						obj_attr[i].stRgnRect.stRect.s32X,
						obj_attr[i].stRgnRect.stRect.s32Y,
						obj_attr[i].stRgnRect.stRect.u32Width,
						obj_attr[i].stRgnRect.stRect.u32Height,
						obj_attr[i].stRgnRect.u32Thick,
						obj_attr[i].stRgnRect.u32Color,
						obj_attr[i].stRgnRect.u32IsFill);
				} else if (obj_attr[i].enObjType == RGN_CMPR_BIT_MAP) {
					rgn_printf("xywh(%d %d %d %d) u32BitmapPAddr(%x)\n",
						obj_attr[i].stBitmap.stRect.s32X,
						obj_attr[i].stBitmap.stRect.s32Y,
						obj_attr[i].stBitmap.stRect.u32Width,
						obj_attr[i].stBitmap.stRect.u32Height,
						obj_attr[i].stBitmap.u32BitmapPAddr);
				}
			}
		}

		if (obj_num) {
			for (i = 0; i < obj_num; ++i) {
				if (obj_attr[i].enObjType == RGN_CMPR_LINE) {
					OSDC_SetLineObjAttr(
						&canvas,
						&obj_vec[i],
						obj_attr[i].stLine.u32Color,
						obj_attr[i].stLine.stPointStart.s32X,
						obj_attr[i].stLine.stPointStart.s32Y,
						obj_attr[i].stLine.stPointEnd.s32X,
						obj_attr[i].stLine.stPointEnd.s32Y,
						obj_attr[i].stLine.u32Thick);
				} else if (obj_attr[i].enObjType == RGN_CMPR_RECT) {
					OSDC_SetRectObjAttr(
						&canvas,
						&obj_vec[i],
						obj_attr[i].stRgnRect.u32Color,
						obj_attr[i].stRgnRect.stRect.s32X,
						obj_attr[i].stRgnRect.stRect.s32Y,
						obj_attr[i].stRgnRect.stRect.u32Width,
						obj_attr[i].stRgnRect.stRect.u32Height,
						obj_attr[i].stRgnRect.u32IsFill,
						obj_attr[i].stRgnRect.u32Thick);
				} else if (obj_attr[i].enObjType == RGN_CMPR_BIT_MAP) {
					OSDC_SetBitmapObjAttr(
						&canvas,
						&obj_vec[i],
						(void *)obj_attr[i].stBitmap.u32BitmapPAddr,
						obj_attr[i].stBitmap.stRect.s32X,
						obj_attr[i].stBitmap.stRect.s32Y,
						obj_attr[i].stBitmap.stRect.u32Width,
						obj_attr[i].stBitmap.stRect.u32Height,
						false);
				}
			}
		}

		switch (canvas_cmpr_attr->enPixelFormat) {
		case PIXEL_FORMAT_ARGB_8888:
			canvas.format = OSD_ARGB8888;
			break;

		case PIXEL_FORMAT_8BIT_MODE:
			canvas.format = OSD_LUT8;
			break;

		case PIXEL_FORMAT_4BIT_MODE:
			canvas.format = OSD_LUT4;
			break;

		case PIXEL_FORMAT_ARGB_4444:
			canvas.format = OSD_ARGB4444;
			break;

		default:
		case PIXEL_FORMAT_ARGB_1555:
			canvas.format = OSD_ARGB1555;
			break;
		}

		switch (rtos_cmdq.cmd_id) {
		case CMDQU_CB_RGN_GET_COMPRESS_SIZE:
		{
			pvAddr = (void *)rtos_cmdq.param_ptr;
			bs_size = OSDC_EstCmprCanvasSize(&canvas, &obj_vec[0], obj_num);
			canvas_cmpr_attr->u32BsSize = bs_size;
			rgn_printf("OSDC_EstCmprCanvasSize bs_size(%d)!\n", bs_size);
			rt_hw_cpu_dcache_clean((void *)pvAddr, ALIGN(sizeof(RGN_CANVAS_CMPR_ATTR_S), 64));
			rtos_cmdq.ip_id  = IP_RGN;
			rtos_cmdq.param_ptr = 0;
			rt_mq_send(xQueueRGNCmdqu, &rtos_cmdq, sizeof(rtos_cmdq));
		}
		break;

		case CMDQU_CB_RGN_COMPRESS:
		{
			pvAddr = (void *)rtos_cmdq.param_ptr;
			status = OSDC_DrawCmprCanvas(&canvas, &obj_vec[0], obj_num ? obj_num : 0, pvAddr,
						canvas_cmpr_attr->u32BsSize, &bs_size);

			if (status != 1) {
				rgn_printf("OSDC_DrawCmprCanvas failed!\n");
				// return status(0xFFFFFFFF) and bs_size to C906B
				*(unsigned int *)pvAddr = 0xffffffff;
				*((unsigned int *)pvAddr + 1) = bs_size;
			} else {
				// save bitstream size in bit[32:63], after C906B gets it,
				// it should be restored to image width and height
				*((unsigned int *)pvAddr + 1) = bs_size;
				rgn_printf("%s bs_size(%d)!, (unsigned int *)pvAddr + 1 is %p\n", __func__, *((unsigned int *)pvAddr + 1), (unsigned int *)pvAddr + 1);
			}

			rt_hw_cpu_dcache_clean((void *)pvAddr, ALIGN(bs_size, 64));
			rtos_cmdq.ip_id  = IP_RGN;
			rtos_cmdq.param_ptr = 0;
			rt_mq_send(xQueueRGNCmdqu, &rtos_cmdq, sizeof(rtos_cmdq));
		}
		break;

		default:
			rgn_printf("%s rtos_cmdq.cmd_id(%d)!\n", __func__, rtos_cmdq.cmd_id);
		break;
		}
		free(obj_vec);

WRONG_CMD_IP_ID:
		rtos_cmdq.ip_id  = -1;
		rtos_cmdq.cmd_id = -1;
		rtos_cmdq.param_ptr = 0;
	}
}