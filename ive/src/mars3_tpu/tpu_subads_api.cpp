#include <assert.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "ive_tpu.h"

extern bm_status_t sg_tpu_kernel_launch(bm_handle_t handle, const char *func_name, void *param, size_t size);

typedef struct sg_api_cv_subads {
    int channel;
    unsigned long long input1_addr[3];
    unsigned long long input2_addr[3];
    unsigned long long output_addr[3];
    int width[3];
    int height[3];
    int input1_str[3];
    int input2_str[3];
    int output_str[3];
} __attribute__((packed)) sg_api_cv_subads_t;

bm_status_t tpu_subads_param_check(bm_handle_t handle,
                                   int height, int width,
                                   PIXEL_FORMAT_E format) {
  if (handle == NULL) {
    bmlib_log("SUBADS", BMLIB_LOG_ERROR, "Can not get handle!\r\n");
    return BM_ERR_PARAM;
  }

  if (height < 1 || height > 4096) {
    bmlib_log("SUBADS", BMLIB_LOG_ERROR,
              "Invalid height(%d), the img height should between 2 and 4096!\r\n", height);
    return BM_ERR_PARAM;
  }

  if (width < 1 || width > 4096) {
    bmlib_log("SUBADS", BMLIB_LOG_ERROR,
              "Invalid width(%d), the img width should between 2 and 4096!\r\n", width);
    return BM_ERR_PARAM;
  }

  if (format != PIXEL_FORMAT_RGB_888_PLANAR && format != PIXEL_FORMAT_BGR_888_PLANAR &&
      format != PIXEL_FORMAT_YUV_PLANAR_444 && format != PIXEL_FORMAT_YUV_PLANAR_420 &&
      format != PIXEL_FORMAT_YUV_400) {
    bmlib_log("SUBADS", BMLIB_LOG_ERROR, "The img format(%d) not supported!\r\n", format);
    return BM_ERR_PARAM;
  }

  return BM_SUCCESS;
}

bm_status_t tpu_cv_subads(bm_handle_t handle, CVI_S32 height, CVI_S32 width, PIXEL_FORMAT_E format,
                          CVI_S32 channel, bm_device_mem_t *src1_mem, bm_device_mem_t *src2_mem,
                          bm_device_mem_t *dst_mem)
{
    bm_status_t ret = BM_ERR_FAILURE;

    ret = tpu_subads_param_check(handle, height, width, (PIXEL_FORMAT_E)format);
    if (ret != BM_SUCCESS) {
        bmlib_log("SUBADS", BMLIB_LOG_ERROR, "Invalid Parameter!\r\n");
        return BM_ERR_PARAM;
}

    sg_api_cv_subads_t api;
    memset(&api, 0, sizeof(sg_api_cv_subads_t));

    api.channel = channel;

    if (format == PIXEL_FORMAT_YUV_PLANAR_420) {
        api.height[0] = height;
        api.width[0] = width;

        for (int i = 1; i < 3; i++) {
            api.height[i] = ALIGN(height / 2, 2);
            api.width[i] = ALIGN(width / 2, 2);
        }
    } else {
        for (int c = 0; c < channel; c++) {
            api.height[c] = height;
            api.width[c] = width;
        }
    }

    for (int i = 0; i < channel; i++) {
        api.input1_addr[i] = bm_mem_get_device_addr(src1_mem[i]);
        api.input2_addr[i] = bm_mem_get_device_addr(src2_mem[i]);
        api.output_addr[i] = bm_mem_get_device_addr(dst_mem[i]);

        api.input1_str[i] = api.input2_str[i] = api.output_str[i] = api.width[i];
    }

        ret = sg_tpu_kernel_launch(handle, "cv_subads", &api, sizeof(api));
        if (ret != BM_SUCCESS) {
            bmlib_log("SUBADS", BMLIB_LOG_ERROR, "sg_tpu_kernel_launch!\r\n");
            return BM_ERR_FAILURE;
        }

    return ret;
}
