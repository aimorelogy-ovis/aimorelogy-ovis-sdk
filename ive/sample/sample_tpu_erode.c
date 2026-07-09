#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#include "ive_tpu.h"

static void read_bin(const char* path, unsigned char* input_data, int size)
{
    FILE *fp_src = fopen(path, "rb");
    if (fread((void *)input_data, 1, size, fp_src) < (unsigned int)size) {
        printf("file size is less than %d required bytes\n", size);
    };

    fclose(fp_src);
}

static void write_bin(const char * path, unsigned char* input_data, int size)
{
    FILE *fp_dst = fopen(path, "wb");
    if (fwrite((void *)input_data, 1, size, fp_dst) < (unsigned int)size){
        printf("file size is less than %d required bytes\n", size);
    };

    fclose(fp_dst);
}

int test_erode_random(bm_handle_t handle,
                       int img_w, int img_h,
                       int kh, int kw,
                       int use_real_img,
                       char *img_name,
                       char *erode_name)
{
    int ret = 0;
    int img_w_stride = ALIGN(img_w, 16) * sizeof(unsigned char);

    unsigned char *src_img = (unsigned char*) malloc (img_w_stride * img_h);
    unsigned char *tpu_res = (unsigned char*) malloc (img_w_stride * img_h);

    memset(src_img, 0, img_w_stride * img_h);
    memset(tpu_res, 0, img_w_stride * img_h);

    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            src_img[y * img_w_stride + x] = rand() % 256;
        }
    }

    if (use_real_img)
        read_bin(img_name, src_img, img_w * img_h);

    bm_device_mem_t src_img_mem = {0}, dst_img_mem = {0};

    if (BM_SUCCESS != bm_malloc_device_byte(handle, &src_img_mem, img_h * img_w_stride)) {
        printf("src img malloc dev mem failed\n");
        free(src_img);
        free(tpu_res);
        return -1;
    }

    if (BM_SUCCESS != bm_malloc_device_byte(handle, &dst_img_mem, img_h * img_w_stride)) {
        printf("dst img malloc dev mem failed\n");
        bm_free_device(handle, src_img_mem);
        free(src_img);
        free(tpu_res);
        return -1;
    }

    if (BM_SUCCESS != bm_memcpy_s2d(handle, src_img_mem, src_img)) {
        printf("src img memcpy s2d failed\n");
        ret = -1;
        goto failed;
    }

    ret = bm_cv_dilate(handle,
                       src_img_mem,
                       dst_img_mem,
                       PIXEL_FORMAT_YUV_400,
                       img_w,
                       img_h,
                       img_w_stride,
                       kw, kh);
    if (ret) {
        printf("bm_cv_dilate failed\n");
        ret = -1;
        goto failed;
    }

    if (BM_SUCCESS != bm_memcpy_d2s(handle, tpu_res, dst_img_mem)) {
        printf("dst img d2s failed\n");
        ret = -1;
        goto failed;
    }

    if (use_real_img)
        write_bin(erode_name, tpu_res, img_w_stride * img_h);

failed:
    bm_free_device(handle, src_img_mem);
    bm_free_device(handle, dst_img_mem);
    free(src_img);
    free(tpu_res);

    return ret;
}

int main(int argc, char *args[])
{
    int use_real_img = 0;
    int img_w = 640;
    int img_h = 361;
    int kw = 5, kh = 5;
    char *src_img_name = NULL;
    char *dilate_name = NULL;

    if (argc > 1) use_real_img = atoi(args[1]);
    if (argc > 2) img_w = atoi(args[2]);
    if (argc > 3) img_h = atoi(args[3]);
    if (argc > 4) src_img_name = args[4];
    if (argc > 5) dilate_name = args[5];

    bm_handle_t handle;
    if (BM_SUCCESS != bm_dev_request(&handle, 0)) {
        printf("get handle failed! \n");
        return -1;
    }

    int ret = test_erode_random(handle, img_w, img_h, kh, kw,
                                use_real_img, src_img_name,
                                dilate_name);
    if (ret) {
        printf("----- Sample Erode Failed -----\n");
        bm_dev_free(handle);
        return -1;
    }
    printf("----- Sample Erode Pass -----\n");

    bm_dev_free(handle);

    return 0;
}