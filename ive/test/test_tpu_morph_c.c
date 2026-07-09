#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#include "ive_tpu.h"

#define TIME_COST_US(start, end) ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

extern void grayscale_dilate(const unsigned char* src, unsigned char* dst,
                             int width, int height, int w_stride, int ksize);

extern void grayscale_erode(const unsigned char* src, unsigned char* dst,
                             int width, int height, int w_stride, int ksize);

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

int array_cmp_u8(unsigned char *p_exp,
                        unsigned char *p_got,
                        int dsize,
                        int img_w_stride,
                        int img_w,
                        int img_h,
                        const char *info_label,
                        unsigned char delta) {
    int idx = 0;

    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            idx = y * (img_w_stride / dsize) + x;
            if ((int)fabs(p_exp[idx] - (int)p_got[ idx]) > delta) {
                printf("%s abs error at index %d exp %d got %d\n",
                    info_label,
                    idx,
                    p_exp[idx],
                    p_got[idx]);
                return -1;
            }
        }
    }

    return 0;
}

int test_morph_random(bm_handle_t handle,
                       int img_w, int img_h,
                       int kh, int kw,
                       int op,
                       int use_real_img,
                       char *img_name,
                       char *dst_name)
{
    struct timeval t1, t2;
    int ret = 0;
    int img_w_stride = ALIGN(img_w, 16) * sizeof(unsigned char);

    unsigned char *src_img = (unsigned char*) malloc (img_w_stride * img_h);
    unsigned char *tpu_res = (unsigned char*) malloc (img_w_stride * img_h);
    unsigned char *cpu_res = (unsigned char*)malloc(img_w_stride * img_h);

    memset(src_img, 0, img_w_stride * img_h);
    memset(tpu_res, 0, img_w_stride * img_h);
    memset(cpu_res, 0, img_w_stride * img_h);

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
        free(src_img); free(tpu_res); free(cpu_res);
        return -1;
    }

    if (BM_SUCCESS != bm_malloc_device_byte(handle, &dst_img_mem, img_h * img_w_stride)) {
        printf("dst img malloc dev mem failed\n");
        bm_free_device(handle, src_img_mem);
        free(src_img); free(tpu_res); free(cpu_res);
        return -1;
    }

    if (BM_SUCCESS != bm_memcpy_s2d(handle, src_img_mem, src_img)) {
        printf("src img memcpy s2d failed\n");
        ret = -1;
        goto failed;
    }

    if (op == MORPH_DILATE) {
        /* CPU dilate */
        gettimeofday(&t1, NULL);
        grayscale_dilate(src_img, cpu_res, img_w, img_h, img_w_stride, kw);
        gettimeofday(&t2, NULL);
        printf("Dilate CPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

        /* tpu_dilate */
        gettimeofday(&t1, NULL);
        ret = bm_cv_dilate(handle, src_img_mem,
                           dst_img_mem, PIXEL_FORMAT_YUV_400, img_w,
                           img_h, img_w_stride,
                           kw, kh);
        if (ret) {
            printf("bm_cv_dilate failed\n");
            ret = -1;
            goto failed;
        }
        gettimeofday(&t2, NULL);
        printf("Dilate TPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

        if (BM_SUCCESS != bm_memcpy_d2s(handle, tpu_res, dst_img_mem)) {
            printf("dst img d2s failed\n");
            ret = -1;
            goto failed;
        }

        ret = array_cmp_u8(cpu_res, tpu_res, sizeof(unsigned char), img_w_stride, img_w, img_h, "dilate", 1);

    } else {
        /* CPU erode calc */
        gettimeofday(&t1, NULL);
        grayscale_erode(src_img, cpu_res, img_w, img_h, img_w_stride, kh);
        gettimeofday(&t2, NULL);
        printf("Erode CPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

        gettimeofday(&t1, NULL);
        ret = bm_cv_erode(handle, src_img_mem,
                          dst_img_mem, PIXEL_FORMAT_YUV_400, img_w,
                          img_h, img_w_stride,
                          kw, kh);
        if (ret) {
            printf("bm_cv_dilate failed\n");
            ret = -1;
            goto failed;
        }
        gettimeofday(&t2, NULL);
        printf("Erode TPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

        if (BM_SUCCESS != bm_memcpy_d2s(handle, tpu_res, dst_img_mem)) {
            printf("dst img d2s failed\n");
            ret = -1;
            goto failed;
        }

        ret = array_cmp_u8(cpu_res, tpu_res, sizeof(unsigned char), img_w_stride, img_w, img_h, "erode", 1);
    }

    if (ret) {
        printf("[%s]: img_size %dx%d failed!\n",
                (op == MORPH_DILATE ? "MORPH_DILATE" : "MORPH_ERODE"), img_w, img_h);
        ret = -1;
        goto failed;
    }

    if (use_real_img)
        write_bin(dst_name, tpu_res, img_w_stride * img_h);

    printf("[%s]: img_size %dx%d cmp successful!\n",
                (op == MORPH_DILATE ? "MORPH_DILATE" : "MORPH_ERODE"), img_w, img_h);

failed:
    bm_free_device(handle, src_img_mem);
    bm_free_device(handle, dst_img_mem);
    free(src_img);
    free(tpu_res);
    free(cpu_res);

    return ret;
}

int main(int argc, char *args[])
{
    struct timespec tp;
    clock_gettime(0, &tp);
    int seed = tp.tv_nsec;
    srand(seed);
    int loop = 1;
    int use_real_img = 0;
    int img_w = 8 + rand() % (641 - 8);
    int img_h = 8 + rand() % (361 - 8);
    int op = rand() % 2;
    int kw = 5, kh = 5;
    char *src_img_name = NULL;
    char *dst_name = NULL;

    if (argc > 1) loop = atoi(args[1]);
    if (argc > 2) use_real_img = atoi(args[2]);
    if (argc > 3) img_w = atoi(args[3]);
    if (argc > 4) img_h = atoi(args[4]);
    if (argc > 5) op = atoi(args[5]);
    if (argc > 6) src_img_name = args[6];
    if (argc > 7) dst_name = args[7];

    bm_handle_t handle;
    if (BM_SUCCESS != bm_dev_request(&handle, 0)) {
        printf("get handle failed! \n");
        return -1;
    }

    for (int i = 0; i < loop; i++) {
        if (i != 0) {
            op = rand() % 2;
            img_h = 8 + rand() % (641 - 8);
            img_w = 8 + rand() % (361 - 8);
        }
        int ret = test_morph_random(handle, img_w, img_h, kh, kw, op,
                                use_real_img, src_img_name, dst_name);
        if (ret) {
            printf("----- TEST MORPH FAILED -----\n");
            bm_dev_free(handle);
            return -1;
        }
    }

    printf("----- TEST MORPH SUCCEED -----\n");

    bm_dev_free(handle);

    return 0;
}