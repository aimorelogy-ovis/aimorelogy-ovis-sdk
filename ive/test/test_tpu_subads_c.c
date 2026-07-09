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
    int img_format;
    int height;
    int width;
    char *src1_name;
    char *src2_name;
    char *dst_name;
    bm_handle_t handle;
} tpu_subads_thread_args_t;

extern int subads_ref(unsigned char *input1, unsigned char *input2, unsigned char *output, int img_size);

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
    struct timeval t1, t2;

    int channel = (format == PIXEL_FORMAT_YUV_400) ? 1 : 3;
    int src1_size[3], src2_size[3], dst_size[3];

    bm_device_mem_t src1_img_mem[3], src2_img_mem[3], dst_img_mem[3];

    printf("TEST INFO:\n");
    printf("height %d, width %d\n", height, width);
    printf("format %d\n", format);

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

    gettimeofday(&t1, NULL);
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
    gettimeofday(&t2, NULL);
    printf("SUBADS TPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

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

int test_subads_random(bm_handle_t handle, int use_real_img,
                       int format, int height, int width,
                       char *src1_name, char *src2_name, char *dst_name)
{
    struct timeval t1, t2;
    int img_size = get_image_size(format, width, height);
    if (img_size == -1) {
        printf("get img_size failed\n");
        return -1;
    }

    unsigned char *src1_data = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *src2_data = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *dst_data_cpu = (unsigned char*) malloc (img_size * sizeof(unsigned char));
    unsigned char *dst_data_tpu = (unsigned char*) malloc (img_size * sizeof(unsigned char));

    memset(dst_data_cpu, 0, img_size * sizeof(unsigned char));
    memset(dst_data_tpu, 0, img_size * sizeof(unsigned char));

    if (use_real_img) {
        read_bin(src1_name, src1_data, img_size);
        read_bin(src1_name, src2_data, img_size);
    } else {
        fill_img(src1_data, img_size);
        fill_img(src2_data, img_size);
    }

    /* calc ref */
    gettimeofday(&t1, NULL);
    subads_ref(src1_data, src2_data, dst_data_cpu, img_size);
    gettimeofday(&t2, NULL);
    printf("SUBADS CPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

    int ret = test_subads_tpu(handle, height, width, format, src1_data, src2_data, dst_data_tpu);
    if (ret) {
        printf("test subads tpu failed\n");
        ret = -1;
        goto failed;
    }

    ret = array_cmp_u8(dst_data_cpu, dst_data_tpu, img_size, "SUBADS", 0);
    if (ret) {
        printf("subads cmp failed\n");
        ret = -1;
        goto failed;
    }

    if (use_real_img)
        write_bin(dst_name, dst_data_tpu, img_size);

failed:
    free(src1_data);
    free(src2_data);
    free(dst_data_cpu);
    free(dst_data_tpu);

    return ret;
}

void *test_thread_subads(void *args)
{
    tpu_subads_thread_args_t *subads_args = (tpu_subads_thread_args_t*)args;
    int loop = subads_args->loop_num;

    for (int i = 0; i < loop; i++) {
        int ret = test_subads_random(subads_args->handle, subads_args->use_real_img,
                                     subads_args->img_format, subads_args->height,
                                     subads_args->width, subads_args->src1_name,
                                     subads_args->src2_name, subads_args->dst_name);
        if (ret) {
            printf("----- TEST SUBADS FAILED -----\n");
            exit(-1);
        }
        printf("----- TEST SUBADS SUCCED -----\n");
    }

    return (void*)0;
}


int main(int argc, char *args[])
{
    struct timespec tp;
    clock_gettime(0, &tp);
    int seed = tp.tv_nsec;
    srand(seed);

    int loop = 1, use_real_img = 0, thread_num = 1;

    int width = 1 + rand() % 800;
    int height = 1 + rand() % 600;

    int format_num[3] = {2, 13, 15};
    int img_format = format_num[(rand() % 3)];

    char *src1_name = NULL, *src2_name = NULL, *dst_name = NULL;

    if (argc == 2 && atoi(args[1]) == -1) {
        printf("%s thread_num loop use_real_img img_format(2/13/15) width height src1_name src2_name dst_name\
                   (when use_real_img = 1,need to set src1_name、src2_name and dst_name) \n", args[0]);
        printf("example:\n");
        printf("%s \n", args[0]);
        printf("%s 1 1 0\n", args[0]);
        printf("%s 1 1 1 13 4096 288 ./4096_288_src1.bin ./4096_288_src2.bin ./4096_288_dst.bin", args[0]);
        return 0;
    }

    if (argc > 1) thread_num = atoi(args[1]);
    if (argc > 2) loop = atoi(args[2]);
    if (argc > 3) use_real_img = atoi(args[3]);
    if (argc > 4) img_format = atoi(args[4]);
    if (argc > 5) width = atoi(args[5]);
    if (argc > 6) height = atoi(args[6]);
    if (argc > 7) src1_name = args[7];
    if (argc > 8) src2_name = args[8];
    if (argc > 9) dst_name = args[9];

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
    bm_status_t ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("Create bm handle failed. ret = %d\n", ret);
        return -1;
    }

    /* test for multi-thread */
    pthread_t pid[thread_num];
    tpu_subads_thread_args_t subads_args[thread_num];
    for (int i = 0; i < thread_num; i++) {
        subads_args[i].loop_num = loop;
        subads_args[i].use_real_img = use_real_img;
        subads_args[i].img_format = img_format;
        subads_args[i].width = width;
        subads_args[i].height = height;
        subads_args[i].src1_name = src1_name;
        subads_args[i].src2_name = src2_name;
        subads_args[i].dst_name = dst_name;
        subads_args[i].handle = handle;

        if (pthread_create(&pid[i], NULL, test_thread_subads, &subads_args[i]) != 0){
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