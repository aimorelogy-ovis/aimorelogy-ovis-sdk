#include "ive_tpu.h"

typedef struct sg_api_cv_remap {
    unsigned long long input_addr[3];
    unsigned long long output_addr[3];
    unsigned long long mapx_addr;
    unsigned long long mapy_addr;
    unsigned long long uv_mapx_addr;
    unsigned long long uv_mapy_addr;
    int format;
    int input_width;
    int input_height;
    int output_width;
    int output_height;
}__attribute__((packed)) sg_api_cv_remap_t;

extern bm_status_t sg_tpu_kernel_launch(bm_handle_t handle, const char *func_name, void *param, size_t size);

static bm_status_t bmcv_remap_check(bm_handle_t handle, int input_width, int input_height, int output_width,
                                    int output_height, PIXEL_FORMAT_E format) {
    if (handle == NULL) {
        bmlib_log("remap", BMLIB_LOG_ERROR, "Can not get handle!\r\n");
        return BM_ERR_PARAM;
    }
    if (input_width < 8 || input_height < 8) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "Input image min size:8x8\n");
        return BM_NOT_SUPPORTED;
    }
    if (input_height > 2048 || output_width > 2048) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "Input image max size:2048x2048\n");
        return BM_NOT_SUPPORTED;
    }
    if (output_width < 8 || output_height < 8) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "Output image min size:8x8\n");
        return BM_NOT_SUPPORTED;
    }
    if (output_width > 2048 || output_height > 2048) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "Output image max size:2048x2048\n");
        return BM_NOT_SUPPORTED;
    }
    if(format != PIXEL_FORMAT_YUV_400 && format != PIXEL_FORMAT_YUV_PLANAR_420) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "Not supported input image format!\n");
        return BM_NOT_SUPPORTED;
    }
    if(format == PIXEL_FORMAT_YUV_PLANAR_420) {
        if(input_width % 2 != 0 || input_height % 2 != 0 || output_width % 2 != 0 || output_height % 2 != 0) {
            bmlib_log("REMAP", BMLIB_LOG_ERROR, "When the image format is yuv420, the width and height must be multiples of 2!\n");
            return BM_NOT_SUPPORTED;
        }
    }
    return BM_SUCCESS;
}

bm_status_t tpu_remap(bm_handle_t handle, bm_device_mem_t input_addr[3], bm_device_mem_t output_addr[3],
                      bm_device_mem_t mapx_data_global_addr, bm_device_mem_t mapy_data_global_addr,
                      int input_width, int input_height, int output_width, int output_height, PIXEL_FORMAT_E format) {
    bm_status_t ret = BM_SUCCESS;
    ret = bmcv_remap_check(handle, input_width, input_height, output_width, output_height, format);
    if (BM_SUCCESS != ret) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "bmcv_remap_check error\n");
        return ret;
    }
    bm_device_mem_t uv_mapx_addr, uv_mapy_addr;
    ret = bm_malloc_device_byte(handle, &uv_mapx_addr, output_width * output_height);
    if (BM_SUCCESS != ret) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "bm_malloc_device_byte uv_mapx_addr error\n");
        return ret;
    }
    ret = bm_malloc_device_byte(handle, &uv_mapy_addr, output_width * output_height);
    if (BM_SUCCESS != ret) {
        bmlib_log("REMAP", BMLIB_LOG_ERROR, "bm_malloc_device_byte uv_mapy_addr error\n");
        return ret;
    }
    int channel = format == PIXEL_FORMAT_YUV_400 ? 1 : 3;
    sg_api_cv_remap_t api;
    for (int i = 0; i < channel; i++) {
        api.input_addr[i] = bm_mem_get_device_addr(input_addr[i]);
        api.output_addr[i] = bm_mem_get_device_addr(output_addr[i]);
    }
    api.format = format;
    api.input_width = input_width;
    api.input_height = input_height;
    api.output_width = output_width;
    api.output_height = output_height;
    api.mapx_addr = bm_mem_get_device_addr(mapx_data_global_addr);
    api.mapy_addr = bm_mem_get_device_addr(mapy_data_global_addr);
    api.uv_mapx_addr = bm_mem_get_device_addr(uv_mapx_addr);
    api.uv_mapy_addr = bm_mem_get_device_addr(uv_mapy_addr);
    unsigned int chipid;
    ret = bm_get_chipid(handle, &chipid);
    if (BM_SUCCESS != ret) {
        bmlib_log("remap", BMLIB_LOG_ERROR, "remap bm_get_chipid error\n");
        return ret;
    }
    ret = sg_tpu_kernel_launch(handle, "cv_remap_mars3", &api, sizeof(api));
    if (ret) {
        bmlib_log("remap", BMLIB_LOG_ERROR, "remap sg_tpu_kernel_launch error\n");
    }
    bm_free_device(handle, uv_mapx_addr);
    bm_free_device(handle, uv_mapy_addr);
    return ret;
}
