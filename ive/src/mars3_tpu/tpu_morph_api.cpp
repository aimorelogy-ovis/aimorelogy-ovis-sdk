#include <assert.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "ive_tpu.h"

extern bm_status_t sg_tpu_kernel_launch(bm_handle_t handle, const char *func_name, void *param, size_t size);

typedef struct sg_api_cv_morph {
    unsigned long long src_addr;
    unsigned long long dst_addr;
    int width;
    int height;
    int kh;
    int kw;
    int stride_i;
    int stride_o;
    int op;
}__attribute__((packed)) sg_api_cv_morph_t;

bm_status_t bm_cv_morph_check(bm_handle_t handle,
                              PIXEL_FORMAT_E format,
                              int img_height,
                              int img_width,
                              int kw, int kh,
                              enum MorphTypes op)
{
    const char *info_label = (op == MORPH_DILATE ? "MORPH_DILATE" : "MORPH_ERODE");
    if (handle == NULL) {
        bmlib_log(info_label, BMLIB_LOG_ERROR, "Can not get handle!\r\n");
        return BM_ERR_PARAM;
    }

    if (format != PIXEL_FORMAT_YUV_400) {
        bmlib_log(info_label, BMLIB_LOG_ERROR, "Only Support PIXEL_FORMAT_YUV_400!\r\n");
        return BM_ERR_PARAM;
    }

    if (op != MORPH_DILATE && op != MORPH_ERODE) {
        bmlib_log(info_label, BMLIB_LOG_ERROR, "MorphTypes(%d) Not Support!\r\n", op);
        return BM_ERR_PARAM;
    }

    if (kw > 7 || kh > 7) {
        bmlib_log(info_label, BMLIB_LOG_ERROR, "The kernel size must be not greater than 7!\r\n");
        return BM_ERR_PARAM;
    }

    if (img_height > 1440 && img_height < 5) {
        bmlib_log(info_label, BMLIB_LOG_ERROR, "the img height should between 8 and 1440!\r\n");
        return BM_ERR_PARAM;
    }

    if (img_width + kw - 1 > 2700 && img_width + kw - 1 < 5) {
        bmlib_log(info_label, BMLIB_LOG_ERROR, "image width is too large!\r\n");
        return BM_ERR_PARAM;
    }

    return BM_SUCCESS;
}

bm_status_t bm_cv_morph(bm_handle_t handle,
                        bm_device_mem_t src_mem,
                        bm_device_mem_t dst_mem,
                        int kh,
                        int kw,
                        int img_height,
                        int img_width,
                        int img_w_stride,
                        enum MorphTypes op)
{
    bm_status_t ret = BM_SUCCESS;

    sg_api_cv_morph api;
    memset(&api, 0, sizeof(sg_api_cv_morph));

    api.src_addr = bm_mem_get_device_addr(src_mem);
    api.dst_addr = bm_mem_get_device_addr(dst_mem);
    api.width = img_width;
    api.height = img_height;
    api.stride_i = img_w_stride;
    api.stride_o = img_w_stride;
    api.op = op;
    api.kh = kh;
    api.kw = kw;

    ret = sg_tpu_kernel_launch(handle, "cv_morph", &api, sizeof(api));
    if (ret) {
    printf("cv_morph tpu_kernel_launch failed\n");
    return BM_ERR_FAILURE;
    }

    return ret;
}

bm_status_t bm_cv_dilate(bm_handle_t handle,
                         bm_device_mem_t src_mem,
                         bm_device_mem_t dst_mem,
                         PIXEL_FORMAT_E format,
                         int width,
                         int height,
                         int w_stride,
                         int kw, int kh)
{
    bm_status_t ret = bm_cv_morph_check(handle, format, height, width, kw, kh, MORPH_DILATE);
    if (ret != BM_SUCCESS) {
        return ret;
    }

    return bm_cv_morph(handle, src_mem, dst_mem, kh, kw, height, width, w_stride, MORPH_DILATE);
}

bm_status_t bm_cv_erode(bm_handle_t handle,
                        bm_device_mem_t src_mem,
                        bm_device_mem_t dst_mem,
                        PIXEL_FORMAT_E format,
                        int width,
                        int height,
                        int w_stride,
                        int kw, int kh)
{
    bm_status_t ret = bm_cv_morph_check(handle, format, height, width, kw, kh, MORPH_ERODE);
    if (ret != BM_SUCCESS) {
        return ret;
    }

    return bm_cv_morph(handle, src_mem, dst_mem, kh, kw, height, width, w_stride, MORPH_ERODE);
}
