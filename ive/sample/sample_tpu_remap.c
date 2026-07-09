#include <stdio.h>
#include "stdlib.h"
#include <sys/time.h>
#include <stdint.h>
#include "bmlib_runtime.h"
#include "ive_tpu.h"

typedef uint16_t bf16;
#define TIME_COST_US(start, end) ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

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
    
    if(fwrite(output_data, sizeof(unsigned char), width * height * channel, fp_dst) != 0) {
        printf("write image success\n");
    };
    
    fclose(fp_dst);
}

int main(int argc, char *args[]) {
    int input_width = 1920;
    int input_height = 1080;
    int output_width = 1920;
    int output_height = 1080;
    PIXEL_FORMAT_E format = PIXEL_FORMAT_YUV_PLANAR_420; //15-gray 13-yuv420
    const char *mapx_path = "path/to/mapx_file";
    const char *mapy_path = "path/to/mapy_file";
    const char *input_path = "path/to/input_file";
    const char *output_path = "path/to/output_file";
    
    bm_status_t ret = BM_SUCCESS;
    bm_handle_t handle;
    ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("bm_dev_request failed. ret = %d\n", ret);
        return -1;
    }
    
    struct timeval t1, t2;
    float channel = format == PIXEL_FORMAT_YUV_400 ? 1.0f : 1.5f;
    
    float *mapx_data = (float*)malloc(output_width * output_height * sizeof(float));
    float *mapy_data = (float*)malloc(output_width * output_height * sizeof(float));
    unsigned char *input_data = (unsigned char*)malloc(input_width * input_height * channel);
    unsigned char *output_tpu = (unsigned char*)malloc(output_width * output_height * channel);
    
    read_bin(input_path, input_data, input_width, input_height, channel);
    read_bin_float(mapx_path, mapx_data, output_width, output_height, channel);
    read_bin_float(mapy_path, mapy_data, output_width, output_height, channel);
    
    bm_device_mem_t input_device_addr[3], output_device_addr[3];
    bm_device_mem_t mapx_data_global_addr, mapy_data_global_addr;
    
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
        unsigned char *input_addr[3] = {input_data, input_data + input_height * input_width, input_data + input_height * input_width * 5 / 4};
        ret = bm_memcpy_s2d(handle, input_device_addr[i], bm_mem_get_system_addr(bm_mem_from_system(input_addr[i])));
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
    
    unsigned char *output_addr[3] = {output_tpu, output_tpu + output_height * output_width, output_tpu + output_height * output_width * 5 / 4};
    for(int i = 0; i < 3; i++) {
        ret = bm_memcpy_d2s(handle, output_addr[i], output_device_addr[i]);
        if (ret != BM_SUCCESS) {
            printf("bm_image_copy_device_to_host output_image error\n");
        }
    }
    
    write_bin(output_path, output_tpu, output_width, output_height, channel);
    
    for(int i = 0; i < 3; i++) {
        bm_free_device(handle, input_device_addr[i]);
        bm_free_device(handle, output_device_addr[i]);
    }
    
    bm_free_device(handle, mapx_data_global_addr);
    bm_free_device(handle, mapy_data_global_addr);
    
    free(mapx_data);
    free(mapy_data);
    free(input_data);
    free(output_tpu);
    
    bm_dev_free(handle);
    return 0;
}