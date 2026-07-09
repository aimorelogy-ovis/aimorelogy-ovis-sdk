#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#include "ive_tpu.h"

static int fill_img(unsigned char *input, int img_size)
{
    for (int i = 0; i < img_size; i++)
        input[i] = rand() % 256;

    return 0;
}

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

static int test_threshold_tpu(bm_handle_t handle, int height, int width,
                              TPU_THRESHOLD_TYPE mode, unsigned int threshold,
                              unsigned int max_value, unsigned char *input_data,
                              unsigned char *output_data)
{
    int ret = 0;
    bm_device_mem_t input_mem, output_mem;

    if (BM_SUCCESS != bm_malloc_device_byte(handle, &input_mem, width * height * sizeof(unsigned char))) {
        printf("input img malloc device mem failed\n");
        return -1;
    }

    if (BM_SUCCESS != bm_malloc_device_byte(handle, &output_mem, width * height * sizeof(unsigned char))) {
        printf("output img malloc device mem failed\n");
        bm_free_device(handle, input_mem);
        return -1;
    }

    if (BM_SUCCESS != bm_memcpy_s2d(handle, input_mem, input_data)) {
        printf("input img malloc device mem failed\n");
        ret = -1;
        goto failed;
    }

    ret = tpu_cv_threshold(handle, height, width, mode, threshold, max_value, &input_mem, &output_mem);
    if (ret) {
        printf("tpu cv threshold failed\n");
        ret = -1;
        goto failed;
    }

    if (BM_SUCCESS != bm_memcpy_d2s(handle, output_data, output_mem)) {
        printf("output d2s failed\n");
        ret = -1;
        goto failed;
    }

failed:
    bm_free_device(handle, input_mem);
    bm_free_device(handle, output_mem);

    return ret;
}

int main(int argc, char* args[])
{
    struct timespec tp;
    clock_gettime(0, &tp);
    int seed = tp.tv_nsec;
    srand(seed);

    bm_handle_t handle;
    int height = 2 + rand() % 100;
    int width = 2 + rand() % 100;
    int threshold_type = rand() % 5; // ? threshold_type?
    int use_real_img = 0;
    unsigned int threshold = 50;
    unsigned int max_value = 228;
    // int format = PIXEL_FORMAT_YUV_400; // Only Support PIXEL_FORMAT_YUV_400

    char *src_name = NULL;
    char *dst_name = NULL;

    if (argc == 2 && atoi(args[1]) == -1) {
        printf("%s use_real_img threshold_type height width src_name dst_name\n", args[0]);
        printf("example:\n");
        printf("%s 1 1 1920 1080 ./src_1920_1080.bin ./dst_1920_1080.bin", args[0]);
        return 0;
    }

    if (argc > 1) use_real_img = atoi(args[1]);
    if (argc > 2) threshold_type = atoi(args[2]);
    if (argc > 3) height = atoi(args[3]);
    if (argc > 4) width = atoi(args[4]);
    if (argc > 5) src_name = args[5];
    if (argc > 6) dst_name = args[6];

    printf("TEST INFO:\n");
    printf("threshold_type %d\n", threshold_type);
    printf("height %d, width %d\n", height, width);

    int ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("Create bm handle failed. ret = %d\n", ret);
        return -1;
    }

    int img_size = height * width;

    unsigned char *input_data = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *output_tpu = (unsigned char*) malloc (img_size * sizeof(unsigned char));

    memset(output_tpu, 0, img_size * sizeof(unsigned char));


    if (use_real_img)
        read_bin(src_name, input_data, img_size);
    else
        fill_img(input_data, img_size);

    ret = test_threshold_tpu(handle, height, width,
            (TPU_THRESHOLD_TYPE) threshold_type, threshold, max_value, input_data, output_tpu);
    if (ret) {
        printf("test threshold tpu failed\n");
        free(input_data);
        free(output_tpu);
        bm_dev_free(handle);
        return -1;
    }

    if (use_real_img)
        write_bin(dst_name, output_tpu, img_size);

    free(input_data);
    free(output_tpu);
    bm_dev_free(handle);

    return 0;
}