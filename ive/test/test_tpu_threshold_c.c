#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#include "ive_tpu.h"

#define TIME_COST_US(start, end) ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

typedef struct {
    int loop_num;
    int use_real_img;
    int width;
    int height;
    int threshold_type;
    char *src_name;
    char *dst_name;
    bm_handle_t handle;
} tpu_threshold_thread_args_t;

extern int threshold_ref(
    unsigned char *input, unsigned char *output,
    int height, int width, TPU_THRESHOLD_TYPE threshold_type,
    unsigned char threshold, unsigned char max_value);

static int fill_img(unsigned char *input, int img_size)
{
    for (int i = 0; i < img_size; i++)
        input[i] = rand() % 256;

    return 0;
}

static void read_bin(const char* path, unsigned char* input_data, int size)
{
    FILE *fp_src = fopen(path, "rb");
    printf("input_data %s\n", path);
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

static int array_cmp_u8(unsigned char *p_exp, unsigned char *p_got,
                        int len, const char *info_label, unsigned char delta)
{
    int idx = 0;
    for (idx = 0; idx < len; idx++) {
        if ((int)fabs(p_exp[idx] - (int)p_got[ idx]) > delta) {
            printf("%s abs error at index %d exp %d got %d\n",
                        info_label,
                        idx,
                        p_exp[idx],
                        p_got[idx]);
        return -1;
        }
    }
    return 0;
}

static int test_threshold_tpu(bm_handle_t handle, int height, int width,
                              TPU_THRESHOLD_TYPE mode, unsigned int threshold,
                              unsigned int max_value, unsigned char *input_data,
                              unsigned char *output_data)
{
    int ret = 0;
    bm_device_mem_t input_mem, output_mem;
    struct timeval t1, t2;

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

    gettimeofday(&t1, NULL);
    ret = tpu_cv_threshold(handle, height, width, mode, threshold, max_value, &input_mem, &output_mem);
    if (ret) {
        printf("tpu cv threshold failed\n");
        ret = -1;
        goto failed;
    }
    gettimeofday(&t2, NULL);
    printf("THRESHOLD TPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

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

static int test_threshold_random(bm_handle_t handle, int use_real_img,
                                 int height, int width,
                                 int threshold_type,
                                 char *src_name, char *dst_name)
{
    struct timeval t1, t2;
    unsigned int threshold = 50;
    unsigned int max_value = 228;

    printf("TEST INFO:\n");
    printf("height %d, width %d\n", height, width);
    printf("threshold_type %d\n", threshold_type);
    printf("threshold %d, max_value %d\n", threshold, max_value);

    // int format = PIXEL_FORMAT_YUV_400; // Only Support PIXEL_FORMAT_YUV_400
    int img_size = height * width;

    unsigned char *input_data = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *output_cpu = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *output_tpu = (unsigned char*) malloc (img_size * sizeof(unsigned char));

    memset(output_cpu, 0, img_size * sizeof(unsigned char));
    memset(output_tpu, 0, img_size * sizeof(unsigned char));

    if (use_real_img)
        read_bin(src_name, input_data, img_size);
    else
        fill_img(input_data, img_size);


    gettimeofday(&t1, NULL);
    threshold_ref(input_data, output_cpu, height, width,
            (TPU_THRESHOLD_TYPE)threshold_type, threshold, max_value);
    gettimeofday(&t2, NULL);
    printf("THRESHOLD CPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

    int ret = test_threshold_tpu(handle, height, width,
            (TPU_THRESHOLD_TYPE) threshold_type, threshold, max_value, input_data, output_tpu);
    if (ret) {
        printf("test threshold tpu failed\n");
        goto failed;
    }

    ret = array_cmp_u8(output_cpu, output_tpu, img_size, "TPU_THRESHOLD", 0);
    if (ret) {
        printf("test threshold tpu failed\n");
        goto failed;
    }

    if (use_real_img)
        write_bin(dst_name, output_tpu, img_size);

failed:
    free(input_data);
    free(output_cpu);
    free(output_tpu);

    return ret;
}

void *test_thread_threshold(void *args)
{
    tpu_threshold_thread_args_t *threshold_args = (tpu_threshold_thread_args_t*)args;

    int loop = threshold_args->loop_num;

    for (int i = 0; i < loop; i++) {
        int ret = test_threshold_random(threshold_args->handle,
                                        threshold_args->use_real_img,
                                        threshold_args->height,
                                        threshold_args->width,
                                        threshold_args->threshold_type,
                                        threshold_args->src_name,
                                        threshold_args->dst_name);
        if (ret) {
            printf("----- TEST THRESHOLD FAILED -----\n");
            exit(-1);
        }
        printf("----- TEST THRESHOLD SUCCED -----\n");
    }

    return (void*)0;
}

int main(int argc, char* args[])
{
    struct timespec tp;
    clock_gettime(0, &tp);
    int seed = tp.tv_nsec;
    srand(seed);

    bm_handle_t handle;
    int height = 2 + rand() % 1920;
    int width = 2 + rand() % 1080;
    int threshold_type = rand() % 5;
    int thread_num = 1, loop = 1, use_real_img = 0;

    char *src_name = NULL;
    char *dst_name = NULL;

    printf("height %d, width %d threshold_type %d\n", height, width, threshold_type);

    if (argc == 2 && atoi(args[1]) == -1) {
        printf("%s thread_num loop use_real_img threshold_type height width src_name dst_name\n", args[0]);
        printf("example:\n");
        printf("%s \n", args[0]);
        printf("%s 1 1 0\n", args[0]);
        printf("%s 1 1 1 2 1920 1080 ./src_1920_1080.bin ./dst_1920_1080.bin", args[0]);
        return 0;
    }

    if (argc > 1) thread_num = atoi(args[1]);
    if (argc > 2) loop = atoi(args[2]);
    if (argc > 3) use_real_img = atoi(args[3]);
    if (argc > 4) threshold_type = atoi(args[4]);
    if (argc > 5) height = atoi(args[5]);
    if (argc > 6) width = atoi(args[6]);
    if (argc > 7) src_name = args[7];
    if (argc > 8) dst_name = args[8];

    /* param check */

    bm_status_t ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("Create bm handle failed. ret = %d\n", ret);
        return -1;
    }

    /* test for multi-thread */
    pthread_t pid[thread_num];
    tpu_threshold_thread_args_t threshold_arg[thread_num];

    for (int i = 0; i < thread_num; i++) {
        threshold_arg[i].loop_num = loop;
        threshold_arg[i].use_real_img = use_real_img;
        threshold_arg[i].threshold_type = threshold_type;
        threshold_arg[i].height = height;
        threshold_arg[i].width = width;
        threshold_arg[i].src_name = src_name;
        threshold_arg[i].dst_name = dst_name;
        threshold_arg[i].handle = handle;

        if (pthread_create(&pid[i], NULL, test_thread_threshold, &threshold_arg[i]) != 0) {
            printf("Create thread failed\n");
            bm_dev_free(handle);
            return -1;
        }
    }

    for (int i = 0; i < thread_num; i++) {
        if (pthread_join(pid[i], NULL) != 0) {
            printf("Thread join failed\n");
            bm_dev_free(handle);
            exit(-1);
        }
    }

    bm_dev_free(handle);

    return 0;
}