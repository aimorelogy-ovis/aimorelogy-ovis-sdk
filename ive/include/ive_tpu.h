#ifndef __IVE_TPU_H__
#define __IVE_TPU_H__

#include <cvi_common.h>
#include <cvi_comm_video.h>

#include "bmlib_runtime.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef enum _TPU_THRESHOLD_TYPE {
    THRESHOLD_BINARY = 0,
    THRESHOLD_BINARY_INV,
    THRESHOLD_TRUNC,
    THRESHOLD_TOZERO,
    THRESHOLD_TOZERO_INV
} TPU_THRESHOLD_TYPE;

typedef enum _TPU_BLEND_WGT_MODE{
    WGT_YUV_SHARE = 0,
    WGT_UV_SHARE
} TPU_BLEND_WGT_MODE;

bm_status_t tpu_cv_subads(
    bm_handle_t handle,
    CVI_S32 height,
    CVI_S32 width,
    PIXEL_FORMAT_E format,
    CVI_S32 channel,
    bm_device_mem_t *src1_mem,
    bm_device_mem_t *src2_mem,
    bm_device_mem_t *dst_mem);

bm_status_t tpu_cv_threshold(
    bm_handle_t handle,
    CVI_S32 height,
    CVI_S32 width,
    TPU_THRESHOLD_TYPE mode,
    CVI_U32 threshold,
    CVI_U32 max_value,
    bm_device_mem_t *input_mem,
    bm_device_mem_t *output_mem);

typedef struct Image_t{
    int channel;
    PIXEL_FORMAT_E format;
    int width[3];
    int height[3];
    int stride[3];
    int channel_stride[3];
} Image;

int set_blend_Image_param(
    Image *img,
    PIXEL_FORMAT_E img_format,
    int width,
    int *w_stride,
    int height,
    int dsize);

bm_status_t tpu_2way_blending(
    bm_handle_t handle,
    Image *left_img,
    bm_device_mem_t *left_mem,
    Image *right_img,
    bm_device_mem_t *right_mem,
    Image *blend_img,
    bm_device_mem_t *blend_mem,
    short overlay_lx,
    short overlay_rx,
    bm_device_mem_t *wgt_phy_mem,
    TPU_BLEND_WGT_MODE mode);


enum MorphTypes{
    MORPH_ERODE    = 0, //!< see #erode
    MORPH_DILATE   = 1, //!< see #dilate
};

bm_status_t bm_cv_dilate(
	bm_handle_t handle,
	bm_device_mem_t src_mem,
	bm_device_mem_t dst_mem,
	PIXEL_FORMAT_E format,
	int width,
	int height,
	int w_stride,
	int kw, int kh);

bm_status_t bm_cv_erode(
	bm_handle_t handle,
	bm_device_mem_t src_mem,
	bm_device_mem_t dst_mem,
	PIXEL_FORMAT_E format,
	int width,
	int height,
	int w_stride,
	int kw, int kh);

bm_status_t tpu_raw12_to_uint16(
    bm_handle_t handle,
    bm_device_mem_t input_dev_mem,
    bm_device_mem_t output_dev_mem,
    int width,
    int height);

bm_status_t tpu_remap(
    bm_handle_t handle,
    bm_device_mem_t input_addr[3],
    bm_device_mem_t output_addr[3],
    bm_device_mem_t mapx_data_global_addr,
    bm_device_mem_t mapy_data_global_addr,
    int input_width,
    int input_height,
    int output_width,
    int output_height,
    PIXEL_FORMAT_E format);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __IVE_TPU_H__
