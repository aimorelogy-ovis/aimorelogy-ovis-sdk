#include <stdlib.h>
#include <string.h>
#include "osdc.h"

uint32_t OSDC_EstCmprCanvasSize(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *objs, uint32_t obj_num)
{
	return OSDC_est_cmpr_canvas_size(canvas, objs, obj_num);
}

int OSDC_DrawCmprCanvas(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *objs, uint32_t obj_num,
				uint8_t *obuf, uint32_t buf_size, uint32_t *p_osize)
{
	return OSDC_draw_cmpr_canvas(canvas, objs, obj_num, obuf, buf_size, p_osize);
}

void OSDC_SetRectObjAttr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				int pt_x, int pt_y, int width, int height, bool is_filled, int thickness)
{
	OSDC_set_rect_obj_attr(canvas, obj, color_code, pt_x, pt_y, width, height, is_filled, thickness);

}

void OSDC_SetRectObjAttrEx(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				OSDC_RECT_ATTR_S *rects, int num, bool is_filled)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		OSDC_set_rect_obj_attr(canvas, obj, color_code, rects[i].x, rects[i].y,
			rects[i].width, rects[i].height, is_filled, rects[i].thickness);
	}

}

void OSDC_SetBitmapObjAttr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj_attr, uint8_t *buf,
				int pt_x, int pt_y, int width, int height, bool is_cmpr)
{
	OSDC_set_bitmap_obj_attr(canvas, obj_attr, buf,  pt_x, pt_y, width, height, is_cmpr);
}

void OSDC_SetLineObjAttr(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				int pt_x0, int pt_y0, int pt_x1, int pt_y1, int thickness)
{
	OSDC_set_line_obj_attr(canvas, obj, color_code, pt_x0, pt_y0, pt_x1, pt_y1, thickness);
}

void OSDC_SetLineObjAttrEx(OSDC_Canvas_Attr_S *canvas, OSDC_DRAW_OBJ_S *obj, uint32_t color_code,
				OSDC_POINT_ATTR_S *points, int num, int thickness)
{
	int i = 0, j = 0, delta_x, delta_y;
	OSDC_POINT_ATTR_S *tmp = malloc(sizeof(OSDC_POINT_ATTR_S) * num);

	if (!tmp)
		return;

	memcpy(tmp, points, sizeof(OSDC_POINT_ATTR_S) * num);
	while (j++ < 5) {
		for (i = 0; i < num - 1; ++i) {
			delta_x = abs(tmp[i].x - tmp[i + 1].x);
			delta_y = abs(tmp[i].y - tmp[i + 1].y);
			if (delta_y < thickness)
				tmp[i + 1].y = tmp[i].y;
			if (delta_x < thickness)
				tmp[i + 1].x = tmp[i].x;
		}
		delta_x = abs(tmp[num - 1].x - tmp[0].x);
		delta_y = abs(tmp[num - 1].y - tmp[0].y);
		if (delta_y < thickness)
			tmp[0].y = tmp[num - 1].y;
		if (delta_x < thickness)
			tmp[0].x = tmp[num - 1].x;
		delta_x = abs(tmp[1].x - tmp[0].x);
		delta_y = abs(tmp[1].y - tmp[0].y);
		if ((delta_x > thickness || delta_x == 0) &&
				(delta_y > thickness || delta_y == 0))
			break;
	}

	for (i = 0; i < num - 1; ++i) {
		OSDC_set_line_obj_attr(canvas, obj + i, color_code, tmp[i].x, tmp[i].y,
				tmp[i + 1].x, tmp[i + 1].y, thickness);
	}

	OSDC_set_line_obj_attr(canvas, obj + num - 1, color_code, tmp[num - 1].x, tmp[num - 1].y,
			tmp[0].x, tmp[0].y, thickness);
	free(tmp);
}

int OSDC_CmprBitmap(OSDC_Canvas_Attr_S *canvas, uint8_t *ibuf, uint8_t *obuf, int width, int height,
				int buf_size, uint32_t *p_osize)
{
	return OSDC_cmpr_bitmap(canvas, ibuf, obuf, width, height, buf_size, p_osize);
}
