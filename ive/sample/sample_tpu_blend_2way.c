#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#include "ive_tpu.h"

int fill_img_data (unsigned char *input, int img_size)
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

static int get_image_size(int format, int *stride, int height){
    int size = 0;
    switch (format){
        case PIXEL_FORMAT_YUV_PLANAR_420:
            size = stride[0] * height + ALIGN((height / 2), 2) * stride[1] + ALIGN((height / 2), 2) * stride[2];
            break;
        case PIXEL_FORMAT_RGB_888_PLANAR:
        case PIXEL_FORMAT_BGR_888_PLANAR:
            size = stride[0] * height * 3;
            break;
        case PIXEL_FORMAT_YUV_400:
            size = stride[0] * height;
            break;
        case PIXEL_FORMAT_NV12:
        case PIXEL_FORMAT_NV21:
            size = stride[0] * height + stride[1] * ALIGN(height / 2, 2);
            break;
        default:
            printf("image format error \n");
            break;
    }
    return size;
}

int test_tpu_2way_blending(bm_handle_t handle,
                           PIXEL_FORMAT_E img_format,
                           int lwidth,
                           int lheight,
                           int *lstride,
                           unsigned char *left_input,
                           int rwidth,
                           int rheight,
                           int *rstride,
                           unsigned char *right_input,
                           int bwidth,
                           int bheight,
                           int *bstride,
                           unsigned char *blend,
                           short overlay_lx, short overlay_rx,
                           TPU_BLEND_WGT_MODE wgt_mode,
                           unsigned char *wgt)
{
    int ret = 0, wgt_idx = 0;
    int overlay_w = overlay_rx - overlay_lx + 1;

    bm_device_mem_t left_img_mem[3], right_img_mem[3], blend_img_mem[3], wgt_mem[2];
    Image left_img, right_img, blend_img;

    int dsize = sizeof(unsigned char);

    if (set_blend_Image_param(&left_img, img_format, lwidth, lstride, lheight, dsize) != 0) {
        printf("create left image param failed\n");
        return -1;
    }

    if (set_blend_Image_param(&right_img, img_format, rwidth, rstride, rheight, dsize) != 0) {
        printf("create right image param failed\n");
        return -1;
    }

    if (set_blend_Image_param(&blend_img, img_format, bwidth, bstride, bheight, dsize) != 0) {
        printf("create blend image param failed\n");
        return -1;
    }

    int channel = blend_img.channel;

    /* malloc wgt_mem device */
    if (overlay_w != 0) {
        int wgt_size[2] = {overlay_w * bheight, 0};
        wgt_idx = 1;

        if (wgt_mode == WGT_UV_SHARE) {
            if (img_format == PIXEL_FORMAT_NV12 || img_format == PIXEL_FORMAT_NV21)
                wgt_size[1] = ALIGN(bheight / 2, 2) * overlay_w;
            else if (img_format == PIXEL_FORMAT_YUV_PLANAR_420)
                wgt_size[1] = ALIGN(overlay_w/2, 2) * ALIGN(bheight/2, 2);
            wgt_idx = 2;
        }

        unsigned char *wgt_host_prt[2] = {wgt, wgt + wgt_size[0]};

        for (int i = 0; i < wgt_idx; i++) {
            if (BM_SUCCESS != bm_malloc_device_byte(handle, &wgt_mem[i], sizeof(unsigned char) * wgt_size[i])) {
                printf("[BLEND ERROR] wgt mem malloc device mem failed\n");
                return -1;
            }

            if (BM_SUCCESS != bm_memcpy_s2d(handle, wgt_mem[i], wgt_host_prt[i])) {
                printf("[BLEND ERROR] wgt s2d failed\n");
                for (int j = 0; j < i; j++)
                    bm_free_device(handle, wgt_mem[j]);
                return -1;
            }
        }
    }

    unsigned char *left_host_prt[3] = {left_input,
                                       left_input + left_img.channel_stride[0],
                                       left_input + left_img.channel_stride[0] +
                                       left_img.channel_stride[1]};
    unsigned char *right_host_ptr[3] = {right_input,
                                        right_input + right_img.channel_stride[0],
                                        right_input + right_img.channel_stride[0] +
                                        right_img.channel_stride[1]};
    unsigned char *blend_host_ptr[3] = {blend,
                                        blend + blend_img.channel_stride[0],
                                        blend + blend_img.channel_stride[0] + blend_img.channel_stride[1]};

    for (int c = 0; c < channel; c++) {
        if (BM_SUCCESS != bm_malloc_device_byte(handle, &left_img_mem[c], sizeof(unsigned char) * left_img.channel_stride[c])) {
            printf("[BLEND ERROR] left img malloc device mem failed\n");
            if (overlay_w != 0) {
                for (int i = 0; i < wgt_idx; i++)
                    bm_free_device(handle, wgt_mem[i]);
            }
            return -1;
        }

        if (BM_SUCCESS != bm_malloc_device_byte(handle, &right_img_mem[c], sizeof(unsigned char) * right_img.channel_stride[c])) {
            printf("[BLEND ERROR] right img malloc device mem failed\n");
            if (overlay_w != 0) {
                for (int i = 0; i < wgt_idx; i++)
                    bm_free_device(handle, wgt_mem[i]);
            }

            for (int i = 0; i < c; i++) {
                bm_free_device(handle, left_img_mem[i]);
            }
            return -1;
        }

        if (BM_SUCCESS != bm_malloc_device_byte(handle, &blend_img_mem[c], sizeof(unsigned char) * blend_img.channel_stride[c])) {
            printf("[BLEND ERROR] blend img malloc device mem failed\n");
            if (overlay_w != 0) {
                for (int i = 0; i < wgt_idx; i++)
                    bm_free_device(handle, wgt_mem[i]);
            }

            for (int i = 0; i < c; i++) {
                bm_free_device(handle, left_img_mem[i]);
                bm_free_device(handle, right_img_mem[i]);
            }
            return -1;
        }

        if (BM_SUCCESS != bm_memcpy_s2d(handle, left_img_mem[c], left_host_prt[c])) {
            printf("[BLEND ERROR] left img S2D failed\n");
            for (int i = 0; i < c; i++) {
                if (overlay_w != 0) {
                    for (int i = 0; i < wgt_idx; i++)
                        bm_free_device(handle, wgt_mem[i]);
                }
                bm_free_device(handle, left_img_mem[i]);
                bm_free_device(handle, right_img_mem[i]);
                bm_free_device(handle, blend_img_mem[i]);
            }
            return -1;
        }

        if (BM_SUCCESS != bm_memcpy_s2d(handle, right_img_mem[c], right_host_ptr[c])) {
            printf("[BLEND ERROR] right img S2D failed\n");
            for (int i = 0; i < c; i++) {
                if (overlay_w != 0) {
                    for (int i = 0; i < wgt_idx; i++)
                        bm_free_device(handle, wgt_mem[i]);
                }
                bm_free_device(handle, left_img_mem[i]);
                bm_free_device(handle, right_img_mem[i]);
                bm_free_device(handle, blend_img_mem[i]);
            }
            return -1;
        }
    }

    ret = tpu_2way_blending(handle, &left_img, left_img_mem, &right_img, right_img_mem, &blend_img, blend_img_mem,
                            overlay_lx, overlay_rx, wgt_mem, wgt_mode);
    if (ret) {
        printf("[BLEND ERROR] tpu_2way_blending failed\n");
        ret = -1;
        goto failed;
    }

    for (int c = 0; c < channel; c++) {
        if (BM_SUCCESS != bm_memcpy_d2s(handle, blend_host_ptr[c], blend_img_mem[c])) {
            printf("[BLEND ERROR] blend img D2S failed\n");
            ret = -1;
            goto failed;
        }
    }

failed:
    for (int i = 0; i < channel; i++) {
        bm_free_device(handle, left_img_mem[i]);
        bm_free_device(handle, right_img_mem[i]);
        bm_free_device(handle, blend_img_mem[i]);
    }

    if (overlay_w != 0) {
        for (int i = 0; i < wgt_idx; i++)
            bm_free_device(handle, wgt_mem[i]);
    }

    return ret;
}

int main(int argc, char* args[])
{
    struct timespec tp;
    clock_gettime(0, &tp);
    int seed = tp.tv_nsec;
    srand(seed);

    int use_real_img = 0;
    int overlay_lx = 100, overlay_rx = 199;
    int lheight = 0, rheight = 0, bheight = 0;
    char *left_name = NULL, *right_name = NULL, *blend_name = NULL, *wgt_name = NULL;

    int lstride = 0, rstride = 0, bstride = 0;

    int lwidth = 200, rwidth = 200, bwidth = 300;

    lstride = ALIGN(lwidth, 16);
    rstride = ALIGN(rwidth, 16);
    bstride = ALIGN(bwidth, 16);

    bheight = lheight = rheight = 300;

    int img_format = PIXEL_FORMAT_NV12;
    int wgt_mode = WGT_UV_SHARE;

    if (argc == 2 && atoi(args[1]) == -1) {
        printf("%suse_real_img img_format wgt_mode left_width lstride left_height left_name\
                right_width rstride right_height right_name blend_width bstride blend_height blend_name \
                overlay_lx overlay_rx wgt_name\n", args[0]);
        printf("example:\n");
        printf("%s \n", args[0]);
        printf("%s 1 13 1 200 300 ./200_alpha.bin 200 288 ./200x288_left.bin 200 288 ./200x288_right.bin 300 288 ./300_288_blend.bin\n", args[0]);
        return 0;
    }

    if (argc > 1) use_real_img = atoi(args[1]);
    if (argc > 2) img_format = atoi(args[2]);
    if (argc > 3) wgt_mode = atoi(args[3]);
    if (argc > 4) overlay_lx = atoi(args[4]);
    if (argc > 5) overlay_rx = atoi(args[5]);
    if (argc > 6) wgt_name = args[6];
    if (argc > 7) lwidth = atoi(args[7]);
    if (argc > 8) lstride = atoi(args[8]);
    if (argc > 9) lheight = atoi(args[9]);
    if (argc > 10) left_name = args[10];
    if (argc > 11) rwidth = atoi(args[11]);
    if (argc > 12) rstride = atoi(args[12]);
    if (argc > 13) rheight = atoi(args[13]);
    if (argc > 14) right_name = args[14];
    if (argc > 15) bwidth = atoi(args[15]);
    if (argc > 16) bheight = atoi(args[16]);
    if (argc > 17) bstride = atoi(args[17]);
    if (argc > 18) blend_name = args[18];

    printf("TEST INFO:\n");
    printf("\timg_format:%d, wgt_mode %d\n", img_format, wgt_mode);
    printf("\tleft_img_size:%d x %d\n", lwidth, lheight);
    printf("\tright_img_size:%d x %d\n", rwidth, rheight);
    printf("\toverly_lx %d, overlay_rx %d, overlay_w %d\n", overlay_lx, overlay_rx, (overlay_rx - overlay_lx + 1));
    printf("\tblend_img_size:%d x %d\n", bwidth, bheight);

    bm_handle_t handle;
    int ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("Create bm handle failed. ret = %d\n", ret);
        return -1;
    }

    int channel = (img_format == PIXEL_FORMAT_YUV_400) ? 1 : 3;

    if(img_format == PIXEL_FORMAT_NV12 || img_format == PIXEL_FORMAT_NV21)
        channel = 2;

    int overlay_w = overlay_rx - overlay_lx + 1;

    int left_stride[3] = {0}, right_stride[3] = {0}, blend_stride[3] = {0};

    switch (img_format)
    {
    case PIXEL_FORMAT_YUV_PLANAR_420:
        left_stride[0] = lstride;
        left_stride[1] = left_stride[2] = ALIGN(lstride/2, 2);

        right_stride[0] = rstride;
        right_stride[1] = right_stride[2] = ALIGN(rstride/2, 2);

        blend_stride[0] = bstride;
        blend_stride[1] = blend_stride[2] = ALIGN(bstride/2, 2);
        break;
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
    case PIXEL_FORMAT_YUV_400:
    case PIXEL_FORMAT_RGB_888_PLANAR:
        for (int i = 0; i < channel; i++) {
                left_stride[i] = lstride;
                right_stride[i] = rstride;
                blend_stride[i] = bstride;
        }
        break;
    default:
        break;
    }

    int left_img_size = get_image_size(img_format, left_stride, lheight);
    int right_img_size = get_image_size(img_format, right_stride, rheight);
    int blend_img_size = get_image_size(img_format, blend_stride, bheight);

    int wgt_size = overlay_w * bheight; // YUV SHARE MODE

    if (wgt_mode == WGT_UV_SHARE) {
        if (img_format == PIXEL_FORMAT_NV12 || img_format == PIXEL_FORMAT_NV21)
            wgt_size = overlay_w * bheight + ALIGN(bheight / 2, 2) * overlay_w;
        else if (img_format == PIXEL_FORMAT_YUV_PLANAR_420)
            wgt_size = overlay_w * bheight + ALIGN(bheight / 2, 2) * ALIGN(overlay_w / 2, 2);
    }

    unsigned char *left_input = (unsigned char*) malloc (left_img_size * sizeof(unsigned char));
    unsigned char *right_input = (unsigned char*) malloc (right_img_size * sizeof(unsigned char));
    unsigned char *tpu_blend_output = (unsigned char*) malloc (blend_img_size * sizeof(unsigned char));

    unsigned char *wgt = (unsigned char*) malloc (wgt_size * sizeof(unsigned char));

    if (use_real_img) {
        read_bin(left_name, left_input, left_img_size);
        read_bin(right_name, right_input, right_img_size);
    } else {
        fill_img_data(left_input, left_img_size);
        fill_img_data(right_input, right_img_size);
    }

    if (wgt_size != 0) {
        if(use_real_img && wgt_name != NULL)
            read_bin(wgt_name, wgt, wgt_size);
        else
            fill_img_data(wgt, wgt_size);
    }

    memset(tpu_blend_output, 0, blend_img_size * sizeof(unsigned char));

    ret = test_tpu_2way_blending(handle, (PIXEL_FORMAT_E)img_format,
                                     lwidth, lheight, left_stride, left_input,
                                     rwidth, rheight, right_stride, right_input,
                                     bwidth, bheight, blend_stride, tpu_blend_output,
                                     overlay_lx, overlay_rx,
                                     (TPU_BLEND_WGT_MODE)wgt_mode, wgt);
    if (ret) {
        printf("tpu 2way blending failed\n");
        free(left_input);
        free(right_input);
        free(tpu_blend_output);
        bm_dev_free(handle);
        return -1;
    }

    if (use_real_img)
        write_bin(blend_name, tpu_blend_output, blend_img_size * sizeof(unsigned char));

    free(left_input);
    free(right_input);
    free(tpu_blend_output);

    bm_dev_free(handle);

    return 0;
}
