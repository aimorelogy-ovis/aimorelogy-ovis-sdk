#include <assert.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <dlfcn.h>
#include <unistd.h>

#include "ive_tpu.h"
#include <pthread.h>

#define FIRMWARE_NAME "libtpu_kernel_module.so"

const  char* fw_fname = FIRMWARE_NAME;
static pthread_once_t fw_path_once = PTHREAD_ONCE_INIT;
static char fw_path[512] = {0};
static int fw_path_status = -1;

int find_tpufirmaware_path(char fw_path[512], const char* name){
    char* ptr;
    int dirname_len;
    int ret = 0;

    Dl_info dl_info;
    const char* path1 = "/mnt/tpu_files/lib/";

    /* 1.test /mnt/tpu_files/lib/libtpu_kernel_module.so */
    memset(fw_path, 0, 512);
    strcpy(fw_path, path1);
    strcat(fw_path, name);
    ret = access(fw_path, F_OK);
    if (ret == 0)
        return ret;

    /* 2.test libcvi_ive_tpu_so_path/libtpu_kernel_module.so */
    ret = dladdr((void*)find_tpufirmaware_path, &dl_info);
    if (ret == 0){
        printf("dladdr() failed: %s\n", dlerror());
        return -1;
    }
    if (dl_info.dli_fname == NULL){
        printf("%s is NOT a symbol\n", __FUNCTION__);
        return -1;
    }

    ptr = (char*)strrchr(dl_info.dli_fname, '/');
    if (!ptr){
        printf("Invalid absolute path name of libbmvpu.so\n");
        return -1;
    }

    dirname_len = ptr - dl_info.dli_fname + 1;
    if (dirname_len <= 0){
        printf("Invalid length of folder name\n");
        return -1;
    }

    memset(fw_path, 0, 512);
    strncpy(fw_path, dl_info.dli_fname, dirname_len);
    strcat(fw_path, name);
    ret = access(fw_path, F_OK);

    return ret;
}

static void init_fw_path() {
    fw_path_status = find_tpufirmaware_path(fw_path, fw_fname);
}

bm_status_t sg_load_tpu_module(bm_handle_t handle, tpu_kernel_module_t *tpu_module)
{
    pthread_once(&fw_path_once, init_fw_path);

    if (fw_path_status != 0) {
        printf("libtpu_kernel_module.so does not exist\n");
        return BM_ERR_FAILURE;
    }

    *tpu_module = tpu_kernel_load_module_file(handle, fw_path);
    if(*tpu_module == NULL){
        printf("tpu_module is null\n");
        return BM_ERR_FAILURE;
    }
    return BM_SUCCESS;
}

bm_status_t sg_tpu_kernel_launch(bm_handle_t handle,
                                 const char *func_name,
                                 void *param,
                                 size_t size)
{
    bm_status_t ret = BM_SUCCESS;
    tpu_kernel_module_t tpu_module = NULL;
    tpu_kernel_function_t func_id = 0;

    ret = sg_load_tpu_module(handle, &tpu_module);
    if(ret != BM_SUCCESS){
        printf("module load error! \n");
        return BM_ERR_FAILURE;
    }

    func_id = tpu_kernel_get_function(handle, tpu_module, (char *)func_name);
    ret = tpu_kernel_launch(handle, func_id, param, size);
    if (tpu_kernel_free_module(handle, tpu_module)) {
        printf("%s:[ERROR] tpu module unload failed\n", __func__);
        return BM_ERR_FAILURE;
    }

    return ret;
}

static int get_channel_info(int size[3], int stride[3], int h[3], int w[3],
                            int *channel, int width, int height, int format, int dsize)
{
    int ret = 0;

    switch (format) {
    case PIXEL_FORMAT_RGB_888_PLANAR:
    case PIXEL_FORMAT_BGR_888_PLANAR:
    case PIXEL_FORMAT_YUV_PLANAR_444:
        for (int i = 0; i < 3; i++) {
            size[i] = width * height;
            stride[i] = width * dsize;
            w[i] = width;
            h[i] = height;
        }

        *channel = 3;
    break;
    case PIXEL_FORMAT_YUV_PLANAR_420:
        size[0] = width * height;
        stride[0] = width * dsize;
        w[0] = width;
        h[0] = height;

        for (int i = 1; i < 3; i++) {
            size[i] = ALIGN(width / 2, 2) * ALIGN(height / 2, 2);
            stride[i] = ALIGN(width / 2, 2) * dsize;
            w[i] = ALIGN(width / 2, 2);
            h[i] = ALIGN(height / 2, 2);
        }

        *channel = 3;
    break;
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
        size[0] = width * height;
        size[1] = ALIGN(height / 2, 2) * width;

        stride[0] = stride[1] = width * dsize;
        stride[2] = 0;

        w[1] = w[0] = width;

        h[0] = height;
        h[1] = ALIGN(height / 2, 2);

        *channel = 2;
    break;
    case PIXEL_FORMAT_YUV_400:
        stride[0] = width * dsize;
        w[0] = width;
        h[0] = height;
        size[0] = width * height;

        *channel = 1;
    break;
    default:
        printf("%s: Img Format is not Supported\n", __func__);
        ret = -1;
    break;
    }

    return ret;
}

int set_blend_Image_param(Image *img, PIXEL_FORMAT_E img_format, int width, int *w_stride,
                          int height, int dsize)
{
    int channel_stride[3] = {0}, stride[3] = {0}, w[3] = {0}, h[3] = {0}, channel = 3;
    int ret = 0;

    if (get_channel_info(channel_stride, stride, h, w, &channel, width, height, img_format, dsize) != 0) {
        printf("%s: blend get_channel info failed\n", __func__);
        return -1;
    }

    img->format = img_format;
    img->channel = channel;

    if (w_stride != NULL) {
        for (int i = 0; i < img->channel; i++) {
            if (w_stride[i] >= stride[i]) {
                stride[i] = w_stride[i];
            } else {
                ASSERT(w_stride[i] > 0 && ((w_stride[i] & (w_stride[i] - 1)) == 0));
                int stride_temp = stride[i];
                stride[i] = ALIGN(stride_temp * dsize, w_stride[i]);
            }
            channel_stride[i] = stride[i] * h[i];
        }
    }

    for (int i = 0; i < channel; i++) {
        img->stride[i] = stride[i];
        img->channel_stride[i] = channel_stride[i];
        img->width[i] = w[i];
        img->height[i] = h[i];
    }

    return ret;
}