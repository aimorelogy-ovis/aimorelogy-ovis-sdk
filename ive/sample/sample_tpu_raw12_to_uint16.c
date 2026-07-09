#include <stdio.h>
#include "stdlib.h"
#include <sys/time.h>
#include <stdint.h>
#include "bmlib_runtime.h"
#include "ive_tpu.h"

#define TIME_COST_US(start, end) ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

static int read_bin(const char *input_path, unsigned char *input_data, int len){
    FILE *fp = fopen(input_path, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open read file: %s\n", input_path);
        return -1;
    }
    int bytes_read = fread(input_data, sizeof(unsigned char), len, fp);
    if (bytes_read != len) {
        fprintf(stderr, "read error: Expected %d bytes, Actual %d bytes\n", len, bytes_read);
        return -1;
    }
    fclose(fp);
    return 0;
}

static int write_uint16_t_bin(const char *output_path, uint16_t *output_data, int len) {
    FILE *fp_dst = fopen(output_path, "wb");
    if (fp_dst == NULL) {
        printf("Unable to open write file %s\n", output_path);
        return -1;
    }
    int bytes_write = fwrite(output_data, sizeof(uint16_t), len, fp_dst);
    if (bytes_write != len) {
        fprintf(stderr, "write error: Expected %d bytes, Actual %d bytes\n", len, bytes_write);
        return -1;
    }
    fclose(fp_dst);
    return 0;
}

int main(int argc, char* args[]) {
    int height_mode = rand() % 3;
    int height_num[3] = {2412, 4824, 7236};
    int height = height_num[height_mode];
    int width = 1344;
    const char* input_path = NULL;
    const char* output_path = NULL;
    int int_ret = 0;
    if (argc > 1) width = atoi(args[1]);
    if (argc > 2) height = atoi(args[2]);
    if (argc > 3) input_path = args[3];
    if (argc > 4) output_path = args[4];

    struct timeval t1, t2;
    bm_handle_t handle;
    bm_status_t ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("Create bm handle failed. ret = %d\n", ret);
        return -1;
    }
    printf("width: %d , height: %d\n", width, height);
    unsigned char* input_data = (unsigned char*)malloc(width * height / 2 * 3 * sizeof(unsigned char));
    uint16_t* output_tpu = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    int_ret = read_bin(input_path, input_data, width * height / 2 * 3);
    if(int_ret != 0) {
        free(input_data);
        free(output_tpu);
        bm_dev_free(handle);
        return -1;
    }
    bm_device_mem_t input_dev_mem, output_dev_mem;
    bm_malloc_device_byte(handle, &input_dev_mem, width * height / 2 * 3 * sizeof(unsigned char));
    bm_malloc_device_byte(handle, &output_dev_mem, width * height * sizeof(uint16_t));
    bm_memcpy_s2d(handle, input_dev_mem, input_data);
    gettimeofday(&t1, NULL);
    ret = tpu_raw12_to_uint16(handle, input_dev_mem, output_dev_mem, width, height);
    if (ret != BM_SUCCESS) {
        printf("tpu_cv_raw12_to_uint16 API process failed\n");
        bm_free_device(handle, input_dev_mem);
        bm_free_device(handle, output_dev_mem);
        free(input_data);
        free(output_tpu);
        bm_dev_free(handle);
        return -1;
    }
    gettimeofday(&t2, NULL);
    printf("Raw12_to_uint16 TPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));
    bm_memcpy_d2s(handle, output_tpu, output_dev_mem);
    write_uint16_t_bin(output_path, output_tpu, width * height);
    bm_free_device(handle, input_dev_mem);
    bm_free_device(handle, output_dev_mem);
    free(input_data);
    free(output_tpu);
    bm_dev_free(handle);
    return ret;
}
