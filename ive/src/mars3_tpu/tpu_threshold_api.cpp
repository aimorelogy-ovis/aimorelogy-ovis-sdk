#include <assert.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "ive_tpu.h"

extern bm_status_t sg_tpu_kernel_launch(bm_handle_t handle, const char *func_name, void *param, size_t size);

typedef struct {
    int channel;
    unsigned long long input_addr[3];
    unsigned long long output_addr[3];
    int width[3];
    int height[3];
    int input_str[3];
    int output_str[3];
    int type;
    unsigned int thresh;
    unsigned int max_value;
} __attribute__((packed)) sg_api_cv_threshold_t;

bm_status_t tpu_threshold_check(bm_handle_t handle,
                                CVI_S32 height,
                                CVI_S32 width,
                                TPU_THRESHOLD_TYPE mode,
                                CVI_U32 threshold,
                                CVI_U32 max_value)
{
    if (handle == NULL) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR, "Can not get handle!\r\n");
        return BM_ERR_PARAM;
    }

    if (height < 2 || height > 4096) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR,
                  "Invalid height(%d), the img height should between 2 and 4096!\r\n", height);
        return BM_ERR_PARAM;
    }

    if (width < 2 || width > 4096) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR,
                  "Invalid height(%d), the img height should between 2 and 4096!\r\n", height);
        return BM_ERR_PARAM;
    }

    if (mode != THRESHOLD_BINARY && mode != THRESHOLD_BINARY_INV && mode != THRESHOLD_TRUNC &&
        mode != THRESHOLD_TOZERO && mode != THRESHOLD_TOZERO_INV) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR,
                  "Invalid threshold type(%d), the threshold type should between 0 and 4!\r\n", mode);
        return BM_ERR_PARAM;
    }

    if (max_value > 255 || threshold > 255) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR,
                  "Invalid threshold value(%d), the threshold value should between 0 and 255!\r\n",
                  max_value);
        return BM_ERR_PARAM;
    }

    return BM_SUCCESS;
}

bm_status_t tpu_cv_threshold(bm_handle_t handle,
                             CVI_S32 height,
                             CVI_S32 width,
                             TPU_THRESHOLD_TYPE mode,
                             CVI_U32 threshold,
                             CVI_U32 max_value,
                             bm_device_mem_t *input_mem,
                             bm_device_mem_t *output_mem)
{
    bm_status_t ret = BM_ERR_FAILURE;

    ret = tpu_threshold_check(handle, height, width, mode, threshold, max_value);
    if (ret) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR, "Invalid Parameter!\r\n");
        return BM_ERR_PARAM;
    }

    sg_api_cv_threshold_t api;
    memset(&api, 0, sizeof(api));

    api.input_addr[0] = bm_mem_get_device_addr(*input_mem);
    api.output_addr[0] = bm_mem_get_device_addr(*output_mem);
    api.width[0] = api.input_str[0] = api.output_str[0] = width;
    api.height[0] = height;
    api.type = mode;
    api.thresh = threshold;
    api.max_value = max_value;
    api.channel = 1;  // Only Support PIXEL_FORMAT_YUV_400

    ret = sg_tpu_kernel_launch(handle, "cv_threshold", &api, sizeof(api));
    if (ret != BM_SUCCESS) {
        bmlib_log("THRESHOLD", BMLIB_LOG_ERROR, "sg_tpu_kernel_launch!\r\n");
        return BM_ERR_FAILURE;
    }

    return ret;
}