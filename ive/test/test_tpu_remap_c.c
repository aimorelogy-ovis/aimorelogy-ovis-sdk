#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include "stdlib.h"
#include "string.h"
#include "ive_tpu.h"

#define TIME_COST_US(start, end) ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))
extern void fill_image(unsigned char *input, int input_width, int input_height, float channel);
extern void fill_map(float* map_x, float* map_y, int input_width, int input_height);
extern void remap_cpu_ref(unsigned char *input, unsigned char *output, float *mapx, float *mapy, int input_height,
                          int input_width, int output_height, int output_width, int format);

typedef struct {
    int loop_num;
    int use_real_img;
    int channel;
    int input_height;
    int input_width;
    int output_height;
    int output_width;
    PIXEL_FORMAT_E format;
    const char *mapx_data_path;
    const char *mapy_data_path;
    const char *input_path;
    const char *output_path;
    bm_handle_t handle;
} remap_thread_arg_t;

static int remap_tpu(bm_handle_t handle, unsigned char *input, unsigned char *output, int input_height, int input_width,
                     int output_height, int output_width, PIXEL_FORMAT_E format, float channel, float *mapx_data, float *mapy_data) {
    bm_status_t ret = BM_SUCCESS;
    bm_device_mem_t input_device_addr[3], output_device_addr[3];
    bm_device_mem_t mapx_data_global_addr, mapy_data_global_addr;
    struct timeval t1, t2;
    ret = bm_malloc_device_byte(handle, &mapx_data_global_addr, output_width * output_height * sizeof(float));
    if (BM_SUCCESS != ret) {
        printf("bm_malloc_device_byte mapx_data_global_addr error\n");
        return ret;
    }
    ret = bm_malloc_device_byte(handle, &mapy_data_global_addr, output_width * output_height * sizeof(float));
    if (BM_SUCCESS != ret) {
        printf("bm_malloc_device_byte mapy_data_global_addr error\n");
        return ret;
    }
    if(format == PIXEL_FORMAT_YUV_PLANAR_420) {
        ret = bm_malloc_device_byte(handle, &input_device_addr[0], input_width * input_height * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte input_device_addr[0] error\n");
            return ret;
        }
        ret = bm_malloc_device_byte(handle, &input_device_addr[1], input_width * input_height / 4 * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte input_device_addr[1] error\n");
            return ret;
        }
        ret = bm_malloc_device_byte(handle, &input_device_addr[2], input_width * input_height / 4 * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte input_device_addr[2] error\n");
            return ret;
        }
        ret = bm_malloc_device_byte(handle, &output_device_addr[0], output_width * output_height * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte output_device_addr[0] error\n");
            return ret;
        }
        ret = bm_malloc_device_byte(handle, &output_device_addr[1], output_width * output_height / 4 * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte input_device_addr[1] error\n");
            return ret;
        }
        ret = bm_malloc_device_byte(handle, &output_device_addr[2], output_width * output_height / 4 * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte input_device_addr[2] error\n");
            return ret;
        }
        for(int i = 0; i < 3; i++) {
            unsigned char *input_addr[3] = {input, input + input_height * input_width, input + input_height * input_width * 5 / 4};
            ret = bm_memcpy_s2d(handle, input_device_addr[i], bm_mem_get_system_addr(bm_mem_from_system(input_addr[i])));
            if (ret != BM_SUCCESS) {
                printf("bm_memcpy_s2d input error\n");
                return ret;
            }
        }
    } else {
        ret = bm_malloc_device_byte(handle, &input_device_addr[0], input_width * input_height * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte input_device_addr[0] error\n");
            return ret;
        }
        ret = bm_malloc_device_byte(handle, &output_device_addr[0], output_width * output_height * sizeof(unsigned char));
        if (BM_SUCCESS != ret) {
            printf("bm_malloc_device_byte output_device_addr[0] error\n");
            return ret;
        }
        ret = bm_memcpy_s2d(handle, input_device_addr[0], input);
        if (ret != BM_SUCCESS) {
            printf("bm_memcpy_s2d input error\n");
            return ret;
        }
    }
    ret = bm_memcpy_s2d(handle, mapx_data_global_addr, bm_mem_get_system_addr(bm_mem_from_system(mapx_data)));
    if (ret != BM_SUCCESS) {
        printf("bm_memcpy_s2d mapx_data error\n");
        return ret;
    }
    ret = bm_memcpy_s2d(handle, mapy_data_global_addr, bm_mem_get_system_addr(bm_mem_from_system(mapy_data)));
    if (ret != BM_SUCCESS) {
        printf("bm_memcpy_s2d mapy_data error\n");
        return ret;
    }
    gettimeofday(&t1, NULL);
    ret = tpu_remap(handle, input_device_addr, output_device_addr, mapx_data_global_addr, mapy_data_global_addr,
                           input_width, input_height, output_width, output_height, format);
    if(ret != BM_SUCCESS){
        printf("tpu_remap error\n");
        return ret;
    }
    gettimeofday(&t2, NULL);
    printf("remap TPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));
    if (format == PIXEL_FORMAT_YUV_400) {
        ret = bm_memcpy_d2s(handle, output, output_device_addr[0]);
        if (ret != BM_SUCCESS) {
            printf("bm_image_copy_device_to_host output_image error\n");
        }
    } else {
        unsigned char *output_addr[3] = {output, output + output_height * output_width, output + output_height * output_width * 5 / 4};
        for(int i = 0; i < 3; i++) {
            ret = bm_memcpy_d2s(handle, output_addr[i], output_device_addr[i]);
            if (ret != BM_SUCCESS) {
                printf("bm_image_copy_device_to_host output_image error\n");
            }
        }
    }
    if(format == PIXEL_FORMAT_YUV_PLANAR_420) {
        for(int i = 0; i < 3; i++) {
            bm_free_device(handle, input_device_addr[i]);
            bm_free_device(handle, output_device_addr[i]);
        }
    } else {
        bm_free_device(handle, input_device_addr[0]);
        bm_free_device(handle, output_device_addr[0]);
    }
    bm_free_device(handle, mapx_data_global_addr);
    bm_free_device(handle, mapy_data_global_addr);
    return ret;
}

static int cmp(unsigned char *got, unsigned char *exp, int len) {
    for (int i = 0; i < len; i++) {
        if (abs(got[i] - exp[i]) > 5) {
            for (int j = 0; j < 5; j++)
                printf("cmp error: idx=%d  exp=%d  got=%d\n", i + j, exp[i + j], got[i + j]);
            return -1;
        }
    }
    return 0;
}

static void read_bin_float(const char *input_path, float *input_data, int width, int height, int channel) {
    FILE *fp_src = fopen(input_path, "rb");
    if (fp_src == NULL) {
        printf("Unable to open output file %s\n", input_path);
        return;
    }
    size_t elements_to_read = width * height * channel;
    if(fread(input_data, sizeof(float), elements_to_read, fp_src) != elements_to_read) {
        printf("read map success\n");
    }
    fclose(fp_src);
}

static void read_bin(const char *input_path, unsigned char *input_data, int input_width, int input_height, float channel) {
    FILE *fp_src = fopen(input_path, "rb");
    if (fp_src == NULL) {
        printf("Unable to open input file %s\n", input_path);
        return;
    }
    size_t elements_to_read = input_width * input_height * channel;
    if(fread(input_data, sizeof(char), elements_to_read, fp_src) != elements_to_read) {
        printf("read image success\n");
    }
    fclose(fp_src);
}

static void write_bin(const char *output_path, unsigned char *output_data, int width, int height, float channel) {
    FILE *fp_dst = fopen(output_path, "wb");
    if (fp_dst == NULL) {
        printf("Unable to open output file %s\n", output_path);
        return;
    }
    fwrite(output_data, sizeof(unsigned char), width * height * channel, fp_dst);
    fclose(fp_dst);
    printf("write image success\n");
}

static int test_remap_random(int use_real_img, int input_height, int input_width, int output_height, int output_width,
                             PIXEL_FORMAT_E format, const char *mapx_path, const char *mapy_path, const char *input_path,
                             const char *output_path, bm_handle_t handle) {
    printf("use_real_img       = %d\n", use_real_img);
    printf("input_width        = %d\n", input_width);
    printf("input_height       = %d\n", input_height);
    printf("output_width       = %d\n", output_width);
    printf("output_height      = %d\n", output_height);
    printf("format             = %d\n", format);
    int ret = 0;
    struct timeval t1, t2;
    float channel = format == PIXEL_FORMAT_YUV_400 ? 1.0f : 1.5f;
    unsigned char *input_data, *output_tpu, *output_cpu;
    float *mapx_data = (float*)malloc(output_width * output_height * sizeof(float));
    float *mapy_data = (float*)malloc(output_width * output_height * sizeof(float));
    if (format == PIXEL_FORMAT_YUV_400) {
        input_data = (unsigned char*)malloc(input_width * input_height);
        output_tpu = (unsigned char*)malloc(output_width * output_height);
        output_cpu = (unsigned char*)malloc(output_width * output_height);
    } else {
        input_data = (unsigned char*)malloc(input_width * input_height * 1.5);
        output_tpu = (unsigned char*)malloc(output_width * output_height * 1.5);
        output_cpu = (unsigned char*)malloc(output_width * output_height * 1.5);
    }
    if (use_real_img) {
        read_bin(input_path, input_data, input_width, input_height, channel);
        read_bin_float(mapx_path, mapx_data, output_width, output_height, channel);
        read_bin_float(mapy_path, mapy_data, output_width, output_height, channel);
    } else {
        fill_image(input_data, input_width, input_height, channel);
        fill_map(mapx_data, mapy_data, output_width, output_height);
    }
    gettimeofday(&t1, NULL);
    remap_cpu_ref(input_data, output_cpu, mapx_data, mapy_data, input_height, input_width, output_height,
                  output_width, format);
    gettimeofday(&t2, NULL);
    printf("remap CPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));
    if(0 != remap_tpu(handle, input_data, output_tpu, input_height, input_width, output_height, output_width, format, channel, mapx_data, mapy_data)){
        free(input_data);
        free(output_tpu);
        free(output_cpu);
        free(mapx_data);
        free(mapy_data);
        return -1;
    }
    ret = cmp(output_tpu, output_cpu, output_width * output_height * channel);
    if (ret == 0) {
        printf("TPU and CPU results comparison successful!\n");
        if (use_real_img == 1) {
            write_bin(output_path, output_tpu, output_width, output_height, channel);
        }
    } else {
        if (use_real_img == 1) {
            write_bin(output_path, output_tpu, output_width, output_height, channel);
        }
        printf("TPU and CPU results comparison failed!\n");
    }
    free(input_data);
    free(output_tpu);
    free(output_cpu);
    free(mapx_data);
    free(mapy_data);
    return ret;
}

void* test_remap(void* args) {
    remap_thread_arg_t* remap_thread_arg = (remap_thread_arg_t*)args;
    int loop_num = remap_thread_arg->loop_num;
    int use_real_img = remap_thread_arg->use_real_img;
    int input_height = remap_thread_arg->input_height;
    int input_width = remap_thread_arg->input_width;
    int output_height = remap_thread_arg->output_height;
    int output_width = remap_thread_arg->output_width;
    PIXEL_FORMAT_E format = remap_thread_arg->format;
    const char* mapx_path = remap_thread_arg->mapx_data_path;
    const char* mapy_path = remap_thread_arg->mapy_data_path;
    const char* input_path = remap_thread_arg->input_path;
    const char* output_path = remap_thread_arg->output_path;
    bm_handle_t handle = remap_thread_arg->handle;
    for (int i = 0; i < loop_num; i++) {
        if(loop_num > 1) {
            input_width = 8 + rand() % 2041;
            input_height = 8 + rand() % 2041;
            output_width = 8 + rand() % 2041;
            output_height = 8 + rand() % 2041;
            format = rand() % 2 == 1 ? PIXEL_FORMAT_YUV_400 : PIXEL_FORMAT_YUV_PLANAR_420;
        }
        if (0 != test_remap_random(use_real_img, input_height, input_width, output_height, output_width,
                                   format, mapx_path, mapy_path, input_path, output_path, handle)) {
            printf("------TEST REMAP FAILED------\n");
            exit(-1);
        }
        printf("------TEST REMAP PASSED!------\n");
    }
    return NULL;
}

int main(int argc, char *args[]) {
    struct timespec tp;
    clock_gettime(0, &tp);
    unsigned int seed = tp.tv_nsec;
    srand(seed);
    printf("seed = %d\n", seed);
    int thread_num = 1;
    int loop = 1;
    int use_real_img = 0;
    int input_width = 1920;
    int input_height = 1080;
    int output_width = 1920;
    int output_height = 1080;
    PIXEL_FORMAT_E format = PIXEL_FORMAT_YUV_400; //15-gray 13-yuv420
    const char *mapx_path = NULL;
    const char *mapy_path = NULL;
    const char *input_path = NULL;
    const char *output_path = NULL;
    int ret = 0;
    bm_handle_t handle;
    ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("bm_dev_request failed. ret = %d\n", ret);
        return -1;
    }
    if (argc == 2 && atoi(args[1]) == -1) {
        printf("usage: \n");
        printf("%s thread_num loop use_real_img format input_width input_height output_width output_height mapx mapy input_path output_path(when use_real_img = 1,need to set input_path and output_path) \n", args[0]);
        printf("example:\n");
        printf("%s \n", args[0]);
        printf("%s 2\n", args[0]);
        printf("%s 2 1\n", args[0]);
        printf("%s 2 1 0 15 512 512 1024 1024\n", args[0]);
        printf("%s 1 1 1 15 1920 1080 1920 1080 mapx.bin mapy.bin 1920x1080_rgb.bin out_remap.bin \n", args[0]);
        return 0;
    }

    if (argc > 1) thread_num = atoi(args[1]);
    if (argc > 2) loop = atoi(args[2]);
    if (argc > 3) use_real_img = atoi(args[3]);
    if (argc > 4) format = (PIXEL_FORMAT_E)atoi(args[4]);
    if (argc > 5) input_width = atoi(args[5]);
    if (argc > 6) input_height = atoi(args[6]);
    if (argc > 7) output_width = atoi(args[7]);
    if (argc > 8) output_height = atoi(args[8]);
    if (argc > 9) mapx_path = args[9];
    if (argc > 10) mapy_path = args[10];
    if (argc > 11) input_path = args[11];
    if (argc > 12) output_path = args[12];

    printf("thread_num = %d\n", thread_num);
    printf("loop_num   = %d\n", loop);
    // test for multi-thread
    pthread_t pid[thread_num];
    remap_thread_arg_t remap_thread_arg[thread_num];
    for (int i = 0; i < thread_num; i++) {
        remap_thread_arg[i].loop_num = loop;
        remap_thread_arg[i].use_real_img = use_real_img;
        remap_thread_arg[i].input_height = input_height;
        remap_thread_arg[i].input_width = input_width;
        remap_thread_arg[i].output_height = output_height;
        remap_thread_arg[i].output_width = output_width;
        remap_thread_arg[i].format = format;
        remap_thread_arg[i].mapx_data_path = mapx_path;
        remap_thread_arg[i].mapy_data_path = mapy_path;
        remap_thread_arg[i].input_path = input_path;
        remap_thread_arg[i].output_path = output_path;
        remap_thread_arg[i].handle = handle;
        if (pthread_create(pid + i, NULL, test_remap, remap_thread_arg + i) != 0) {
            printf("create thread failed\n");
            bm_dev_free(handle);
            return -1;
        }
    }
    for (int i = 0; i < thread_num; i++) {
        int ret = pthread_join(pid[i], NULL);
        if (ret != 0) {
            printf("Thread join failed\n");
            bm_dev_free(handle);
            exit(-1);
        }
    }
    bm_dev_free(handle);
    return ret;
}
