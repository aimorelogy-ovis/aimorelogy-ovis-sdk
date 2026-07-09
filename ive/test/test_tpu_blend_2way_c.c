#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#include "ive_tpu.h"

#define TIME_COST_US(start, end) ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

#define IMG_MAX_SIZE 400
#define OVERLAY_MAX_W 200

typedef struct {
    int loop_num;
    int use_real_img;
    int img_format;
    int wgt_mode;
    int lwidth;
    int lstride;
    int lheight;
    char *left_name;
    int rwidth;
    int rstride;
    int rheight;
    char *right_name;
    int bwidth;
    int bstride;
    int bheight;
    char *blend_name;
    int overlay_lx;
    int overlay_rx;
    char *wgt_name;
    bm_handle_t handle;
} tpu_blend_2way_thread_args_t;

extern int cpu_2way_blend(int lwidth, int lheight, int *left_stride, unsigned char *left_img,
    int rwidth, int rheight, int *right_stride, unsigned char *right_img,
    int bwidth, int bheight, int *blend_stride, unsigned char *blend_img,
    int overlay_lx, int overlay_rx, unsigned char *wgt,
    TPU_BLEND_WGT_MODE wgt_mode, int channel, int format);

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

static int cmp_yuv420(unsigned char *p_exp, unsigned char *p_got,
                        int format, int w, int w_stride, int h,
                        const char *info_label, unsigned char delta)
{
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w_stride + x;
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

    if (format == PIXEL_FORMAT_YUV_PLANAR_420) {
        for (int c = 0; c < 2; c++) {
            for (int y = 0; y < ALIGN(h / 2, 2); y++) {
                for (int x = 0; x < ALIGN(w / 2, 2); x++) {
                    int idx = h * w_stride + (c * ALIGN(h / 2, 2) * ALIGN((w_stride / 2), 2)) +
                              y * (w_stride / 2) + x;
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
        }
    } else if (format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21){
        for (int y = 0; y < h / 2; y++) {
            for (int x = 0; x < w / 2; x++) {
                int idx_u = h * w_stride + y * w_stride + (format == PIXEL_FORMAT_NV12 ? 2*x : 2*x + 1);
                int idx_v = h * w_stride + y * w_stride + (format == PIXEL_FORMAT_NV12 ? 2*x + 1 : 2*x);

                if ((int)fabs(p_exp[idx_u] - (int)p_got[ idx_u]) > delta) {
                    printf("%s abs error at index %d exp %d got %d\n",
                        info_label,
                        idx_u,
                        p_exp[idx_u],
                        p_got[idx_u]);
                    return -1;
                }

                if ((int)fabs(p_exp[idx_v] - (int)p_got[ idx_v]) > delta) {
                    printf("%s abs error at index %d exp %d got %d\n",
                        info_label,
                        idx_v,
                        p_exp[idx_v],
                        p_got[idx_v]);
                    return -1;
                }
            }
        }
    }


    return 0;
}

static int array_cmp_u8(unsigned char *p_exp, unsigned char *p_got,
                        int format, int c, int w, int w_stride, int h,
                        const char *info_label, unsigned char delta)
{
    int ret = 0;
    switch (format)
    {
    case PIXEL_FORMAT_YUV_PLANAR_420:
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
        ret = cmp_yuv420(p_exp, p_got, format, w, w_stride, h, info_label, delta);
        break;
    case PIXEL_FORMAT_YUV_400:
    case PIXEL_FORMAT_RGB_888_PLANAR:
        for (int c_idx = 0; c_idx < c; c_idx++) {
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int idx = c_idx * h * w_stride + y * w_stride + x;
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
        }
        break;
    default:
        break;
    }


    return ret;
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
    struct timeval t1, t2;
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

    gettimeofday(&t1, NULL);
    ret = tpu_2way_blending(handle, &left_img, left_img_mem, &right_img, right_img_mem, &blend_img, blend_img_mem,
                            overlay_lx, overlay_rx, wgt_mem, wgt_mode);
    if (ret) {
        printf("[BLEND ERROR] tpu_2way_blending failed\n");
        ret = -1;
        goto failed;
    }
    gettimeofday(&t2, NULL);
    printf("BLEND_2WAY TPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

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

static int test_blend_2way_random(bm_handle_t handle, int use_real_img,
                                  int lwidth, int lstride, int lheight,
                                  int rwidth, int rstride, int rheight,
                                  int bwidth, int bstride, int bheight,
                                  int overlay_lx, int overlay_rx,
                                  int format, int wgt_mode,
                                  char *left_name, char *right_name,
                                  char *blend_name, char *wgt_name)
{
    struct timeval t1, t2;
    int channel = (format == PIXEL_FORMAT_YUV_400) ? 1 : 3;

    if(format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21)
        channel = 2;

    int overlay_w = overlay_rx - overlay_lx + 1;

    int left_stride[3] = {0}, right_stride[3] = {0}, blend_stride[3] = {0};

    printf("TEST INFO:\n");
    printf("\timg_format:%d, wgt_mode %d\n", format, wgt_mode);
    printf("\tleft_img_size:%d x %d, lstride %d\n", lwidth, lheight, lstride);
    printf("\tright_img_size:%d x %d, rstide %d\n", rwidth, rheight, rstride);
    printf("\toverly_lx %d, overlay_rx %d, overlay_w %d\n", overlay_lx, overlay_rx, (overlay_rx - overlay_lx + 1));
    printf("\tblend_img_size:%d x %d, bstride %d\n", bwidth, bheight, bstride);

    switch (format)
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

    int left_img_size = get_image_size(format, left_stride, lheight);
    int right_img_size = get_image_size(format, right_stride, rheight);
    int blend_img_size = get_image_size(format, blend_stride, bheight);

    int wgt_size = overlay_w * bheight; // YUV SHARE MODE

    if (wgt_mode == WGT_UV_SHARE) {
        if (format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21)
            wgt_size = overlay_w * bheight + ALIGN(bheight / 2, 2) * overlay_w;
        else if (format == PIXEL_FORMAT_YUV_PLANAR_420)
            wgt_size = overlay_w * bheight + ALIGN(bheight / 2, 2) * ALIGN(overlay_w / 2, 2);
    }

    unsigned char *left_input = (unsigned char*) malloc (left_img_size * sizeof(unsigned char));
    unsigned char *right_input = (unsigned char*) malloc (right_img_size * sizeof(unsigned char));
    unsigned char *cpu_blend_output = (unsigned char*) malloc (blend_img_size * sizeof(unsigned char));
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

    memset(cpu_blend_output, 0, blend_img_size * sizeof(unsigned char));
    memset(tpu_blend_output, 0, blend_img_size * sizeof(unsigned char));

    /* calc ref */
    gettimeofday(&t1, NULL);
    cpu_2way_blend(lwidth, lheight, left_stride, left_input,
                   rwidth, rheight, right_stride, right_input,
                   bwidth, bheight, blend_stride, cpu_blend_output,
                   overlay_lx, overlay_rx, wgt, (TPU_BLEND_WGT_MODE)wgt_mode, channel, format);
    gettimeofday(&t2, NULL);
    printf("BLEND_2WAY CPU using time = %ld(us)\n", (long)TIME_COST_US(t1, t2));

    int ret = test_tpu_2way_blending(handle, (PIXEL_FORMAT_E)format,
                                     lwidth, lheight, left_stride, left_input,
                                     rwidth, rheight, right_stride, right_input,
                                     bwidth, bheight, blend_stride, tpu_blend_output,
                                     overlay_lx, overlay_rx, (TPU_BLEND_WGT_MODE)wgt_mode, wgt);
    if (ret) {
        printf("tpu 2way blending failed\n");
        ret = -1;
        goto failed;
    }

    ret = array_cmp_u8(cpu_blend_output, tpu_blend_output, format, channel, bwidth, bstride, bheight, "BLEND 2WAY", 1);
    if (ret) {
        printf("tpu 2way blending cmp failed\n");
        ret = -1;
        goto failed;
    }

    printf("tpu 2way blending cmp successful\n");

    if (use_real_img)
        write_bin(blend_name, tpu_blend_output, blend_img_size * sizeof(unsigned char));

failed:
    free(left_input);
    free(right_input);
    free(cpu_blend_output);
    free(tpu_blend_output);
    if (wgt_size != 0) free(wgt);

    return ret;
}

void *test_thread_blend_2way(void *args)
{
    tpu_blend_2way_thread_args_t *blend_args = (tpu_blend_2way_thread_args_t*)args;
    int loop = blend_args->loop_num;

    for (int i = 0; i < loop; i++) {
        int ret = test_blend_2way_random(blend_args->handle,
                                         blend_args->use_real_img,
                                         blend_args->lwidth,
                                         blend_args->lstride,
                                         blend_args->lheight,
                                         blend_args->rwidth,
                                         blend_args->rstride,
                                         blend_args->rheight,
                                         blend_args->bwidth,
                                         blend_args->bstride,
                                         blend_args->bheight,
                                         blend_args->overlay_lx,
                                         blend_args->overlay_rx,
                                         blend_args->img_format,
                                         blend_args->wgt_mode,
                                         blend_args->left_name,
                                         blend_args->right_name,
                                         blend_args->blend_name,
                                         blend_args->wgt_name);
        if (ret) {
            printf("----- loop %d, test_blend_2way_random failed -----\n", i);
            exit(-1);
        }
        printf("----- loop %d, test_blend_2way_random succed -----\n", i);
    }

    return (void*)0;
}

int main(int argc, char* args[])
{
    struct timespec tp;
    clock_gettime(0, &tp);
    int seed = tp.tv_nsec;
    srand(seed);

    int loop = 1, use_real_img = 0, thread_num = 1;
    int overlap = 0;
    int overlay_lx = 0, overlay_rx = 0;
    int lheight = 0, rheight = 0, bheight = 0;
    char *left_name = NULL, *right_name = NULL, *blend_name = NULL, *wgt_name = NULL;

    int lstride = 0, rstride = 0, bstride = 0;

    int lwidth = 8 + 4 * (rand() % (((IMG_MAX_SIZE - 4 - 8) / 4) + 1));
    int rwidth = 8 + 4 * (rand() % (((IMG_MAX_SIZE - 4 - 8) / 4) + 1));

    bheight = lheight = rheight = 8 + 4 * (rand() % (((IMG_MAX_SIZE - 4 - 8) / 4) + 1));

    int has_overlap = (rand() % 10) < 7;

    if (has_overlap) {
        int max_overlay = (lwidth < rwidth) ? ((lwidth < OVERLAY_MAX_W) ? lwidth : OVERLAY_MAX_W) :
                          ((rwidth < OVERLAY_MAX_W) ? rwidth : OVERLAY_MAX_W);
        int steps = max_overlay / 4 + 1;

        overlap = rand() % steps * 4;
        overlay_lx = lwidth - overlap;
        overlay_rx = overlay_lx + overlap - 1;
    } else {
        overlap = 0;
        overlay_lx = lwidth;
        overlay_rx = lwidth - 1;
    }

    int bwidth = lwidth + rwidth - overlap;

    lstride = ALIGN(lwidth, 16);
    rstride = ALIGN(rwidth, 16);
    bstride = ALIGN(bwidth, 16);

    int format_num[5] = {PIXEL_FORMAT_RGB_888_PLANAR, PIXEL_FORMAT_YUV_PLANAR_420, PIXEL_FORMAT_YUV_400, PIXEL_FORMAT_NV12, PIXEL_FORMAT_NV21};
    int img_format = format_num[(rand() % 5)];

    int wgt_mode = WGT_YUV_SHARE;

    if (argc == 2 && atoi(args[1]) == -1) {
        printf("%s thread_num loop use_real_img use_stride img_format wgt_mode left_width left_height left_name\
                right_width right_height right_name  blend_width blend_height blend_name \
                overlay_lx overlay_rx wgt_name\n", args[0]);
        printf("example:\n");
        printf("%s \n", args[0]);
        printf("%s 1 1 0\n", args[0]);
        printf("%s 1 1 1 1 13 1 200 300 ./200_alpha.bin 200 288 ./200x288_left.bin 200 288 ./200x288_right.bin 300 288 ./300_288_blend.bin\n", args[0]);
        return 0;
    }

    if (argc > 1) thread_num = atoi(args[1]);
    if (argc > 2) loop = atoi(args[2]);
    if (argc > 3) use_real_img = atoi(args[3]);
    if (argc > 4) img_format = atoi(args[4]);
    if (argc > 5) wgt_mode = atoi(args[5]);
    if (argc > 6) overlay_lx = atoi(args[6]);
    if (argc > 7) overlay_rx = atoi(args[7]);
    if (argc > 8) wgt_name = args[8];
    if (argc > 9) lwidth = atoi(args[9]);
    if (argc > 10) lstride = atoi(args[10]);
    if (argc > 11) lheight = atoi(args[11]);
    if (argc > 12) left_name = args[12];
    if (argc > 13) rwidth = atoi(args[13]);
    if (argc > 14) rstride = atoi(args[14]);
    if (argc > 15) rheight = atoi(args[15]);
    if (argc > 16) right_name = args[16];
    if (argc > 17) bwidth = atoi(args[17]);
    if (argc > 18) bheight = atoi(args[18]);
    if (argc > 19) bstride = atoi(args[19]);
    if (argc > 20) blend_name = args[20];

    bm_handle_t handle;
    bm_status_t ret = bm_dev_request(&handle, 0);
    if (ret != BM_SUCCESS) {
        printf("Create bm handle failed. ret = %d\n", ret);
        return -1;
    }

    /* test for multi-thread */
    pthread_t pid[thread_num];
    tpu_blend_2way_thread_args_t blend_args[thread_num];
    for (int i = 0; i < thread_num; i++) {
        blend_args[i].loop_num = loop;
        blend_args[i].use_real_img = use_real_img;
        blend_args[i].img_format = img_format;
        blend_args[i].wgt_mode = wgt_mode;
        blend_args[i].lwidth = lwidth;
        blend_args[i].lstride = lstride;
        blend_args[i].lheight = lheight;
        blend_args[i].left_name = left_name;
        blend_args[i].rwidth = rwidth;
        blend_args[i].rstride = rstride;
        blend_args[i].rheight = rheight;
        blend_args[i].right_name = right_name;
        blend_args[i].bwidth = bwidth;
        blend_args[i].bstride = bstride;
        blend_args[i].bheight = bheight;
        blend_args[i].blend_name = blend_name;
        blend_args[i].overlay_lx = overlay_lx;
        blend_args[i].overlay_rx = overlay_rx;
        blend_args[i].wgt_name = wgt_name;
        blend_args[i].handle = handle;

        if (pthread_create(&pid[i], NULL, test_thread_blend_2way, &blend_args[i]) != 0) {
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