#ifndef __RGN_H__
#define __RGN_H__

#include <vip_common.h>
#include <cvi_common.h>
#include <cvi_comm_video.h>
#include <cvi_comm_vpss.h>
#include <cvi_comm_region.h>
#include <rgn_uapi.h>

#include <base_cb.h>
#include <base_ctx.h>
#include <rgn_common.h>
#include <rgn_defines.h>

/*********************************************************************************************/
/* INTERNAL */
static int _rgn_sw_init(struct cvi_rgn_dev *vdev);
static int _rgn_release_op(struct cvi_rgn_dev *rdev);
static void _rgn_destroy_proc(struct cvi_rgn_dev *rdev);
static void _rgn_destroy_proc(struct cvi_rgn_dev *rdev);
CVI_S32 _rgn_init(void);
CVI_S32 _rgn_exit(void);
CVI_U32 _rgn_proc_get_idx(RGN_HANDLE hHandle);
bool is_rect_overlap(RECT_S *r0, RECT_S *r1);
CVI_BOOL _rgn_check_order(RECT_S *r0, RECT_S *r1);
int _rgn_insert(RGN_HANDLE hdls[], CVI_U8 size, RGN_HANDLE hdl);
int _rgn_ex_insert(RGN_HANDLE hdls[], CVI_U8 size, RGN_HANDLE hdl);
int _rgn_remove(RGN_HANDLE hdls[], CVI_U8 size, RGN_HANDLE hdl);
void _rgn_fill_pattern(void *buf, CVI_U32 len, CVI_U32 color, CVI_U8 bpp);
static CVI_S32 _rgn_set_hw_cfg(RGN_HANDLE hdls[], CVI_U8 size, struct cvi_rgn_cfg *cfg);
static CVI_S32 _rgn_ex_set_hw_cfg(RGN_HANDLE hdls[], CVI_U8 size, struct cvi_rgn_ex_cfg *cfg);
CVI_S32 _rgn_check_chn_attr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr);

#endif /* __RGN_H__ */
