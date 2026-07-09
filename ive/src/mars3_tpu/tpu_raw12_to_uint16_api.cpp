#include "ive_tpu.h"

typedef struct sg_api_raw12_to_uint16 {
    unsigned long long input_addr;
    unsigned long long output_addr;
    int width;
    int height;
}__attribute__((packed)) sg_api_raw12_to_uint16_t;

extern bm_status_t sg_tpu_kernel_launch(bm_handle_t handle, const char *func_name, void *param, size_t size);

static bm_status_t bmcv_raw12_to_uint16_check(bm_handle_t handle, int width, int height) {
    if (handle == NULL) {
        bmlib_log("RAW12_TO_UINT16", BMLIB_LOG_ERROR, "Can not get handle!\r\n");
        return BM_ERR_PARAM;
    }
    if (width != 1344) {
        bmlib_log("RAW12_TO_UINT16", BMLIB_LOG_ERROR, "width must be 1344!\r\n");
        return BM_ERR_PARAM;
    }
    if (height != 2412 && height != 4824 && height != 7236) {
        bmlib_log("RAW12_TO_UINT16", BMLIB_LOG_ERROR, "height must be 2412/4824/7236!\r\n");
        return BM_ERR_PARAM;
    }
    return BM_SUCCESS;
}

bm_status_t tpu_raw12_to_uint16(bm_handle_t handle, bm_device_mem_t input_dev_mem, bm_device_mem_t output_dev_mem,
                                   int width, int height) {
    bm_status_t ret = BM_SUCCESS;
    ret = bmcv_raw12_to_uint16_check(handle, width, height);
    if (BM_SUCCESS != ret) {
        bmlib_log("RAW12_TO_UINT16", BMLIB_LOG_ERROR, "bmcv_raw12_to_uint16_check failed!\r\n");
        return ret;
    }
    sg_api_raw12_to_uint16_t api;
    api.input_addr = bm_mem_get_device_addr(input_dev_mem);
    api.output_addr = bm_mem_get_device_addr(output_dev_mem);
    api.width = width;
    api.height = height;
    ret = sg_tpu_kernel_launch(handle, "raw12_to_uint16", &api, sizeof(api));
    if (ret != BM_SUCCESS) {
        printf("raw12_to_uint16 tpu_kernel_launch failed\n");
        return BM_ERR_FAILURE;
    }

    return BM_SUCCESS;
}