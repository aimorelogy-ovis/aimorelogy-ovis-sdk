#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#include "ive_tpu.h"

int fill_img (unsigned char *input, int img_size)
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

static int get_image_size(int format, int width, int height){
    int size = 0;
    switch (format){
        case PIXEL_FORMAT_YUV_PLANAR_420:
            size = width * height + 2 * ALIGN((height / 2), 2) * ALIGN((width / 2), 2);
            break;
        case PIXEL_FORMAT_RGB_888_PLANAR:
        case PIXEL_FORMAT_BGR_888_PLANAR:
        case PIXEL_FORMAT_YUV_PLANAR_444:
            size = width * height * 3;
            break;
        case PIXEL_FORMAT_YUV_400:
            size = width * height;
            break;
        default:
            printf("image format(%d) not support!\n", format);
            size = -1;
            break;
    }
    return size;
}

static void get_each_channel_size(int size[3], int format , int height, int width)
{
    switch (format)
    {
    case PIXEL_FORMAT_YUV_PLANAR_420:
        size[0] = height * width;
        size[1] = ALIGN(width / 2, 2) * ALIGN(height / 2, 2);
        size[2] = ALIGN(width / 2, 2) * ALIGN(height / 2, 2);
        break;
    case PIXEL_FORMAT_YUV_400:
        size[0] = height * width;
        size[1] = size[2] = 0;
        break;
    case PIXEL_FORMAT_RGB_888_PLANAR:
    case PIXEL_FORMAT_BGR_888_PLANAR:
    case PIXEL_FORMAT_YUV_PLANAR_444:
        size[0] = size[1] = size[2] = width * height;
        break;
    default:
        printf("image format(%d) not support!\n", format);
        break;
    }
}

int test_subads_tpu(bm_handle_t handle, int height, int width, int format,
                    unsigned char *src1, unsigned char* src2, unsigned char *dst)
{
    int ret = 0;

    int channel = (format == PIXEL_FORMAT_YUV_400) ? 1 : 3;
    int src1_size[3], src2_size[3], dst_size[3];

    bm_device_mem_t src1_img_mem[3], src2_img_mem[3], dst_img_mem[3];

    /* get img each channel size */
    get_each_channel_size(src1_size, format, height, width);
    get_each_channel_size(src2_size, format, height, width);
    get_each_channel_size(dst_size, format, height, width);

    unsigned char *src1_host_ptr[3] = {src1, src1 + src1_size[0], src1 + src1_size[0] + src1_size[1]};
    unsigned char *src2_host_ptr[3] = {src2, src2 + src2_size[0], src2 + src2_size[0] + src2_size[1]};

    for (int c = 0; c < channel; c++) {
        if (BM_SUCCESS != bm_malloc_device_byte(handle, &src1_img_mem[c], sizeof(unsigned char) * src1_size[c])) {
            printf("src1 img malloc device mem failed\n");
            return -1;
        }

        if (BM_SUCCESS != bm_malloc_device_byte(handle, &src2_img_mem[c], sizeof(unsigned char) * src2_size[c])) {
            printf("src2 img malloc device mem failed\n");
            for (int i = 0; i < c; i++)
                bm_free_device(handle, src1_img_mem[i]);
            return -1;
        }

        if (BM_SUCCESS != bm_malloc_device_byte(handle, &dst_img_mem[c], sizeof(unsigned char) * dst_size[c])) {
            printf("dst img malloc device mem failed\n");
            for (int i = 0; i < c; i++) {
                bm_free_device(handle, src1_img_mem[i]);
                bm_free_device(handle, src2_img_mem[i]);
            }
            return -1;
        }

        if (BM_SUCCESS != bm_memcpy_s2d(handle, src1_img_mem[c], src1_host_ptr[c])) {
            printf("left img S2D failed\n");
            for (int i = 0; i < c; i++) {
                bm_free_device(handle, src1_img_mem[i]);
                bm_free_device(handle, src2_img_mem[i]);
                bm_free_device(handle, dst_img_mem[i]);
            }
            return -1;
        }

        if (BM_SUCCESS != bm_memcpy_s2d(handle, src2_img_mem[c], src2_host_ptr[c])) {
            printf("left img S2D failed\n");
            for (int i = 0; i < c; i++) {
                bm_free_device(handle, src1_img_mem[i]);
                bm_free_device(handle, src2_img_mem[i]);
                bm_free_device(handle, dst_img_mem[i]);
            }
            return -1;
        }
    }

    ret = tpu_cv_subads(handle, height, width, (PIXEL_FORMAT_E)format, channel, src1_img_mem, src2_img_mem, dst_img_mem);
    if (ret) {
        printf("tpu subads failed\n");
        for (int i = 0; i < channel; i++) {
            bm_free_device(handle, src1_img_mem[i]);
            bm_free_device(handle, src2_img_mem[i]);
            bm_free_device(handle, dst_img_mem[i]);
        }
        return -1;
    }

    unsigned char *dst_host_ptr[3] = {dst, dst + dst_size[0], dst + dst_size[0] + dst_size[1]};
    for (int c = 0; c < channel; c++) {
        if (BM_SUCCESS != bm_memcpy_d2s(handle, dst_host_ptr[c], dst_img_mem[c])) {
            printf("dst img D2S failed\n");
            for (int i = 0; i < c; i++) {
                bm_free_device(handle, src1_img_mem[i]);
                bm_free_device(handle, src2_img_mem[i]);
                bm_free_device(handle, dst_img_mem[i]);
            }
            return -1;
        }
    }

    for (int i = 0; i < channel; i++) {
        bm_free_device(handle, src1_img_mem[i]);
        bm_free_device(handle, src2_img_mem[i]);
        bm_free_device(handle, dst_img_mem[i]);
    }

    return ret;
}

int main(int argc, char *args[])
{
    struct timespec tp;
    clock_gettime(0, &tp);
    int seed = tp.tv_nsec;
    srand(seed);

    int use_real_img = 0;

    int width = 1 + rand() % 100;
    int height = 1 + rand() % 100;

    int format_num[3] = {2, 13, 15};
    int img_format = format_num[(rand() % 3)];

    char *src1_name = NULL, *src2_name = NULL, *dst_name = NULL;

    if (argc == 2 && atoi(args[1]) == -1) {
        printf("%s use_real_img img_format width height src1_name src2_name dst_name\
                   (when use_real_img = 1,need to set src1_name、src2_name and dst_name) \n", args[0]);
        printf("example:\n");
        printf("%s \n", args[0]);
        printf("%s 1 13 4096 288 ./4096_288_src1.bin ./4096_288_src2.bin ./4096_288_dst.bin", args[0]);
        return 0;
    }

    if (argc > 1) use_real_img = atoi(args[1]);
    if (argc > 2) img_format = atoi(args[2]);
    if (argc > 3) width = atoi(args[3]);
    if (argc > 4) height = atoi(args[4]);
    if (argc > 5) src1_name = args[5];
    if (argc > 6) src2_name = args[6];
    if (argc > 7) dst_name = args[7];

    printf("TEST INFO:\n");
    printf("img_format %d\n", img_format);
    printf("width %d, height %d\n", width, height);

    /* param check */
    if (width > 4096 || height > 4096) {
        printf("Invalid paramer\n");
        return -1;
    }

    if (img_format != 2 && img_format != 13 && img_format != 15) {
        printf("Invalid img_format(%d)\n", img_format);
        return -1;
    }

    bm_handle_t handle;
    int ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("Create bm handle failed. ret = %d\n", ret);
        return -1;
    }

    int img_size = get_image_size(img_format, width, height);

    if (img_size == -1) {
        printf("get img_size failed\n");
        return -1;
    }

    unsigned char *src1_data = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *src2_data = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *dst_data_tpu = (unsigned char*) malloc (img_size * sizeof(unsigned char));

    memset(dst_data_tpu, 0, img_size * sizeof(unsigned char));

    if (use_real_img) {
        read_bin(src1_name, src1_data, img_size);
        read_bin(src2_name, src2_data, img_size);
    } else {
        fill_img(src1_data, img_size);
        fill_img(src2_data, img_size);
    }

    ret = test_subads_tpu(handle, height, width, img_format, src1_data, src2_data, dst_data_tpu);
    if (ret) {
        printf("test subads tpu failed\n");
        free(src1_data);
        free(src2_data);
        free(dst_data_tpu);
        bm_dev_free(handle);
        return -1;
    }

    if (use_real_img)
        write_bin(dst_name, dst_data_tpu, img_size);

    free(src1_data);
    free(src2_data);
    free(dst_data_tpu);
    bm_dev_free(handle);

    return 0;
}