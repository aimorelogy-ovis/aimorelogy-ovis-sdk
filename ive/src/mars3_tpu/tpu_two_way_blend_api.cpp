#include <assert.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "ive_tpu.h"

extern bm_status_t sg_tpu_kernel_launch(bm_handle_t handle, const char *func_name, void *param, size_t size);

typedef struct sg_cv_blend_2way {
    unsigned long long left_img_addr[3];
    int left_width[3];
    int left_stride[3];
    int left_height[3];
    unsigned long long right_img_addr[3];
    int right_width[3];
    int right_stride[3];
    int right_height[3];
    unsigned long long wgt_mem_addr[2];
    int overlay_lx;
    int overlay_rx;
    unsigned long long blend_img_addr[3];
    int blend_width[3];
    int blend_stride[3];
    int blend_height[3];
    int channel;
    int format;
    int wgt_mode;
} __attribute__((packed)) sg_cv_blend_2way_t;

bm_status_t tpu_blend_param_check(bm_handle_t handle,
                                  Image *left_img,
                                  Image *right_img,
                                  Image *blend_img,
                                  short overlay_lx,
                                  short overlay_rx,
                                  TPU_BLEND_WGT_MODE mode)
{
    if (handle == NULL) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "Can not get handel!\r\n");
        return BM_ERR_FAILURE;
    }

    if (left_img->format != right_img->format && left_img->format != blend_img->format) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "Images formats must be same!\r\n");
        return BM_ERR_PARAM;
    }

    if (blend_img->format != PIXEL_FORMAT_RGB_888_PLANAR &&
        blend_img->format != PIXEL_FORMAT_YUV_PLANAR_420 &&
        blend_img->format != PIXEL_FORMAT_YUV_400 && blend_img->format != PIXEL_FORMAT_NV12 &&
        blend_img->format != PIXEL_FORMAT_NV21) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The img format(%d) not supported!\r\n", blend_img->format);
        return BM_ERR_PARAM;
    }

    if (left_img->height[0] != right_img->height[0] && left_img->height[0] != blend_img->height[0]) {
        bmlib_log(
            "BLEND", BMLIB_LOG_ERROR,
            "The heights of the left image, right image, and blend image should be the same.\r\n");
        return BM_ERR_PARAM;
    }

    if (left_img->width[0] < 8 || left_img->width[0] > 4096) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The Left img width should be [8, 4096(%d)]\r\n",
                left_img->width[0]);
        return BM_ERR_PARAM;
    }

    if (left_img->height[0] < 8 || left_img->height[0] > 4096) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The Left img height should be [8, 4096(%d)]\r\n",
                left_img->height[0]);
        return BM_ERR_PARAM;
    }

    if (right_img->width[0] < 8 || right_img->width[0] > 4096) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The Right img width should be [8, 4096(%d)]\r\n",
                right_img->width[0]);
        return BM_ERR_PARAM;
    }

    if (right_img->height[0] < 8 || right_img->height[0] > 4096) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The Right img height should be [8, 4096(%d)]\r\n",
                right_img->height[0]);
        return BM_ERR_PARAM;
    }

    if (blend_img->width[0] < 8) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The Blend img width should be [8, 4096(%d)]\r\n",
                blend_img->width[0]);
        return BM_ERR_PARAM;
    }

    if (blend_img->height[0] < 8 || blend_img->height[0] > 4096) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The Blend img height should be [8, 4096(%d)]\r\n",
                blend_img->height[0]);
        return BM_ERR_PARAM;
    }

    bool is_target_format =
        (blend_img->format == PIXEL_FORMAT_YUV_PLANAR_420 || blend_img->format == PIXEL_FORMAT_NV12 ||
        blend_img->format == PIXEL_FORMAT_NV21);

    if (is_target_format && (blend_img->height[0] % 2 != 0)) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR,
                "In PIXEL_FORMAT_YUV_PLANAR_420 mode, the img height should be 2-aligned\r\n");
        return BM_ERR_PARAM;
    }

    if (is_target_format && (left_img->width[0] % 4 != 0)) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR,
                "In YUV420P or YUV420SP mode, the left image width should be 4-aligned\r\n");
        return BM_ERR_PARAM;
    }

    if (is_target_format && (right_img->width[0] % 4 != 0)) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR,
                "In YUV420P or YUV420SP mode, the right image width should be 4-aligned\r\n");
        return BM_ERR_PARAM;
    }

    if (is_target_format && (blend_img->width[0] % 4 != 0)) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR,
                "In YUV420P or YUV420SP mode, the blend image width should be 4-aligned\r\n");
        return BM_ERR_PARAM;
    }

    if (overlay_lx < 0 || overlay_lx > left_img->width[0]) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The overlay of left must be [0, lwidth(%d)]\r\n",
                left_img->width[0]);
        return BM_ERR_PARAM;
    }

    if (overlay_rx < 0 || overlay_rx >= blend_img->width[0]) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "The overlay of right must be [0, rwidth(%d)]\r\n",
                blend_img->width[0]);
        return BM_ERR_PARAM;
    }

    if ((overlay_rx - overlay_lx + 1) < 0 || (overlay_rx - overlay_lx + 1) > 2000) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR,
                "The overlap width must be strictly between 0 and 2000 \r\n");
        return BM_ERR_PARAM;
    }

    if (is_target_format && ((overlay_rx - overlay_lx + 1) % 2 != 0)) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "Img format(%d), the overlay width shoud be 2-aligned\r\n",
                blend_img->format);
        return BM_ERR_PARAM;
    }

    if (mode != WGT_YUV_SHARE && mode != WGT_UV_SHARE) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "Invalid BLEND WGT MODE (%d)!\r\n", mode);
        return BM_ERR_PARAM;
    }

    return BM_SUCCESS;
}

bm_status_t tpu_2way_blending(bm_handle_t handle,
                              Image *left_img,
                              bm_device_mem_t *left_mem,
                              Image *right_img,
                              bm_device_mem_t *right_mem,
                              Image *blend_img,
                              bm_device_mem_t *blend_mem,
                              short overlay_lx,
                              short overlay_rx,
                              bm_device_mem_t *wgt_phy_mem,
                              TPU_BLEND_WGT_MODE mode)
{
    bm_status_t ret = BM_ERR_FAILURE;
    sg_cv_blend_2way_t api;
    memset(&api, 0, sizeof(sg_cv_blend_2way_t));

    ret = tpu_blend_param_check(handle, left_img, right_img, blend_img, overlay_lx, overlay_rx, mode);
    if (ret != BM_SUCCESS) {
        bmlib_log("BLEND", BMLIB_LOG_ERROR, "Invalid Parameter!\r\n");
        return BM_ERR_PARAM;
    }

    api.overlay_lx = overlay_lx;
    api.overlay_rx = overlay_rx;
    api.channel = blend_img->channel;
    api.format = blend_img->format;
    api.wgt_mode = (int)mode;

    if ((overlay_rx - overlay_lx + 1) != 0) {
        api.wgt_mem_addr[0] = bm_mem_get_device_addr(wgt_phy_mem[0]);
        if (mode == WGT_UV_SHARE) api.wgt_mem_addr[1] = bm_mem_get_device_addr(wgt_phy_mem[1]);
    }

    for (int i = 0; i < blend_img->channel; i++) {
        api.left_height[i] = left_img->height[i];
        api.left_width[i] = left_img->width[i];
        api.left_stride[i] = left_img->stride[i];
        api.left_img_addr[i] = bm_mem_get_device_addr(left_mem[i]);

        api.right_height[i] = right_img->height[i];
        api.right_width[i] = right_img->width[i];
        api.right_stride[i] = right_img->stride[i];
        api.right_img_addr[i] = bm_mem_get_device_addr(right_mem[i]);

        api.blend_height[i] = blend_img->height[i];
        api.blend_width[i] = blend_img->width[i];
        api.blend_stride[i] = blend_img->stride[i];
        api.blend_img_addr[i] = bm_mem_get_device_addr(blend_mem[i]);
    }

    ret = sg_tpu_kernel_launch(handle, "cv_blend_2way", &api, sizeof(api));
    if (ret != BM_SUCCESS) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR, "sg_tpu_kernel_launch!\r\n");
        return BM_ERR_FAILURE;
    }

    return ret;
}
