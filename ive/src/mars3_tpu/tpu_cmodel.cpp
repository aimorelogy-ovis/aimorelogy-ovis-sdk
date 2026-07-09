#include <assert.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <limits.h>

#include "ive_tpu.h"

//remap
typedef uint16_t bf16;

static bf16 float_to_bfloat16(float f) {
    uint32_t* p = (uint32_t*)&f;
    return (bf16)((*p) >> 16);
}

static inline float bfloat16_to_float(uint16_t bf) {
    union {
        uint32_t i;
        float f;
    } converter;

    converter.i = (uint32_t)bf << 16;

    return converter.f;
}

static void prepare_coordinates(
    float x, float y,
    int src_width, int src_height,
    int* x_int, bf16* x_frac,
    int* y_int, bf16* y_frac) {

    float x_clamped = x;
    float y_clamped = y;

    *x_int = (int)floorf(x_clamped);
    *y_int = (int)floorf(y_clamped);

    float x_frac_float = x_clamped - *x_int;
    float y_frac_float = y_clamped - *y_int;

    if (*x_int == src_width - 1) x_frac_float = 0.0f;
    if (*y_int == src_height - 1) y_frac_float = 0.0f;

    *x_frac = float_to_bfloat16(x_frac_float);
    *y_frac = float_to_bfloat16(y_frac_float);
}

static unsigned char bilinear_interpolation_separated(
    unsigned char* src,
    int src_width,
    int src_height,
    int x_int,
    bf16 x_frac,
    int y_int,
    bf16 y_frac,
    int c) {
    float dx = bfloat16_to_float(x_frac);
    float dy = bfloat16_to_float(y_frac);

    const int x0 = (x_int < 0) ? 0 : (x_int >= src_width) ? src_width - 1 : x_int;
    const int y0 = (y_int < 0) ? 0 : (y_int >= src_height) ? src_height - 1 : y_int;

    const int x1 = (x0 < src_width - 1) ? x0 + 1 : x0;
    const int y1 = (y0 < src_height - 1) ? y0 + 1 : y0;

    unsigned char* channel_base = src + c * src_width * src_height;
    const unsigned char p00 = channel_base[y0 * src_width + x0];
    const unsigned char p01 = channel_base[y0 * src_width + x1];
    const unsigned char p10 = channel_base[y1 * src_width + x0];
    const unsigned char p11 = channel_base[y1 * src_width + x1];

    const float w00 = (1.0f - dx) * (1.0f - dy);
    const float w01 = dx * (1.0f - dy);
    const float w10 = (1.0f - dx) * dy;
    const float w11 = dx * dy;

    const float val = p00 * w00 + p01 * w01 + p10 * w10 + p11 * w11;

    float result_f = val + 0.5f;
    if (result_f < 0.0f) result_f = 0.0f;
    if (result_f > 255.0f) result_f = 255.0f;

    return (unsigned char)result_f;
}

void remap_cpu_ref(unsigned char *input, unsigned char *output, float *mapx, float *mapy, int input_height,
                   int input_width, int output_height, int output_width, int format) {
    for (int y = 0; y < output_height; ++y) {
        for (int x = 0; x < output_width; ++x) {
            const int map_idx = y * output_width + x;
            const float src_x = mapx[map_idx];
            const float src_y = mapy[map_idx];

            if (src_x < 0 || src_x >= input_width || src_y < 0 || src_y >= input_height) {
                output[map_idx] = 0;
            } else {
                int x_int;
                bf16 x_frac;
                int y_int;
                bf16 y_frac;

                prepare_coordinates(src_x, src_y, input_width, input_height,
                                    &x_int, &x_frac, &y_int, &y_frac);

                output[map_idx] = bilinear_interpolation_separated(
                    input, input_width, input_height,
                    x_int, x_frac, y_int, y_frac, 0);
            }
        }
    }
    if(format == PIXEL_FORMAT_YUV_PLANAR_420) {
        const int uv_width = output_width / 2;
        const int uv_height = output_height / 2;
        const int uv_size = uv_width * uv_height;

        int* uv_x_int = (int*)malloc(uv_size * sizeof(int));
        bf16* uv_x_frac = (bf16*)malloc(uv_size * sizeof(bf16));
        int* uv_y_int = (int*)malloc(uv_size * sizeof(int));
        bf16* uv_y_frac = (bf16*)malloc(uv_size * sizeof(bf16));

        if (!uv_x_int || !uv_x_frac || !uv_y_int || !uv_y_frac) {
            free(uv_x_int);
            free(uv_x_frac);
            free(uv_y_int);
            free(uv_y_frac);
            return;
        }

        for (int y = 0; y < uv_height; ++y) {
            for (int x = 0; x < uv_width; ++x) {
                const int uv_idx = y * uv_width + x;
                const int y_map_idx = (y * 2) * output_width + (x * 2);

                const float uv_src_x = mapx[y_map_idx] / 2.0f;
                const float uv_src_y = mapy[y_map_idx] / 2.0f;

                if (uv_src_x < 0 || uv_src_x >= input_width/2 ||
                    uv_src_y < 0 || uv_src_y >= input_height/2) {
                    uv_x_int[uv_idx] = -1;
                } else {
                    prepare_coordinates(uv_src_x, uv_src_y, input_width/2, input_height/2,
                                       &uv_x_int[uv_idx], &uv_x_frac[uv_idx],
                                       &uv_y_int[uv_idx], &uv_y_frac[uv_idx]);
                }
            }
        }

        unsigned char *u_output = output + output_width * output_height;
        unsigned char *v_output = u_output + uv_size;

        const int uv_input_offset = input_width * input_height;
        const int uv_input_width = input_width / 2;
        const int uv_input_height = input_height / 2;

        for (int y = 0; y < uv_height; ++y) {
            for (int x = 0; x < uv_width; ++x) {
                const int uv_idx = y * uv_width + x;

                if (uv_x_int[uv_idx] == -1) {
                    u_output[uv_idx] = 0;
                } else {
                    const int x_int = uv_x_int[uv_idx];
                    const bf16 x_frac = uv_x_frac[uv_idx];
                    const int y_int = uv_y_int[uv_idx];
                    const bf16 y_frac = uv_y_frac[uv_idx];

                    u_output[uv_idx] = bilinear_interpolation_separated(
                        input + uv_input_offset,
                        uv_input_width, uv_input_height,
                        x_int, x_frac, y_int, y_frac, 0);
                }
            }
        }

        for (int y = 0; y < uv_height; ++y) {
            for (int x = 0; x < uv_width; ++x) {
                const int uv_idx = y * uv_width + x;

                if (uv_x_int[uv_idx] == -1) {
                    v_output[uv_idx] = 0;
                } else {
                    const int x_int = uv_x_int[uv_idx];
                    const bf16 x_frac = uv_x_frac[uv_idx];
                    const int y_int = uv_y_int[uv_idx];
                    const bf16 y_frac = uv_y_frac[uv_idx];

                    v_output[uv_idx] = bilinear_interpolation_separated(
                        input + uv_input_offset + uv_input_width * uv_input_height,
                        uv_input_width, uv_input_height,
                        x_int, x_frac, y_int, y_frac, 0);
                }
            }
        }

        free(uv_x_int);
        free(uv_x_frac);
        free(uv_y_int);
        free(uv_y_frac);
    }
}

void fill_map(float* map_x, float* map_y, int input_width, int input_height) {
    for (int y = 0; y < input_height; y++) {
        for (int x = 0; x < input_width; x++) {
            map_x[y * input_width + x] = input_width - x - 1;
            map_y[y * input_width + x] = y;
        }
    }
}

void fill_image(unsigned char *input, int input_width, int input_height, float channel) {
    int len = input_height * input_width * channel;
    for(int i = 0; i < len; i++) {
        input[i] = rand() % 256;
    }
}
//remap

//raw12_to_uint16
void fill_raw12(unsigned char* input, int len) {
    for(int i = 0; i < len; i++) {
        input[i] = rand() % 256;
    }
}

void raw12_to_uint16_cpu(unsigned char *input_data, uint16_t *output_cpu, int w, int h) {
    const size_t pixel_cnt = (size_t)w * (size_t)h;
    const size_t group_cnt = pixel_cnt >> 1;

    size_t s_idx = 0;
    size_t d_idx = 0;

    for (size_t g = 0; g < group_cnt; ++g) {
        uint8_t b0 = input_data[s_idx++];
        uint8_t b1 = input_data[s_idx++];
        uint8_t b2 = input_data[s_idx++];

        uint16_t p0 = ((uint16_t)b0 << 4) | ( b2 & 0x0F );
        uint16_t p1 = ((uint16_t)b1 << 4) | ( b2 >> 4 );

        output_cpu[d_idx++] = p0;
        output_cpu[d_idx++] = p1;
    }
}
//raw12_to_uint16
int subads_ref(unsigned char *input1, unsigned char *input2, unsigned char *output, int img_size)
{
    for (int i = 0; i < img_size; i++) output[i] = abs(input1[i] - input2[i]);

    return 0;
}

int threshold_ref(unsigned char *input, unsigned char *output, int height, int width,
                    TPU_THRESHOLD_TYPE threshold_type, unsigned char threshold,
                    unsigned char max_value)
{
    switch (threshold_type) {
      case THRESHOLD_BINARY:
        for (int i = 0; i < width * height; i++) {
          if (input[i] > threshold)
            output[i] = max_value;
          else
            output[i] = 0;
        }
        break;
      case THRESHOLD_BINARY_INV:
        for (int i = 0; i < width * height; i++) {
          if (input[i] > threshold)
            output[i] = 0;
          else
            output[i] = max_value;
        }
        break;
      case THRESHOLD_TRUNC:
        for (int i = 0; i < width * height; i++) {
          if (input[i] > threshold)
            output[i] = threshold;
          else
            output[i] = input[i];
        }
        break;
      case THRESHOLD_TOZERO:
        for (int i = 0; i < width * height; i++) {
          if (input[i] > threshold)
            output[i] = input[i];
          else
            output[i] = 0;
        }
        break;
      case THRESHOLD_TOZERO_INV:
        for (int i = 0; i < width * height; i++) {
          if (input[i] > threshold)
            output[i] = 0;
          else
            output[i] = input[i];
        }
        break;
      default:
        break;
    }

    return 0;
}

static unsigned char blend_pixel(unsigned char left, unsigned char right, unsigned char alpha) {
    float r = (alpha * left + (255 - alpha) * right) / 255;

    return (r > 255) ? 255 : (unsigned char)r;
}

static void process_uv_plane(unsigned char *blend_uv_base, unsigned char *left_uv_base,
                               unsigned char *right_uv_base, int format, int bheight, int bwidth,
                               int uv_bstride, int lheight, int uv_lstride, int rheight, int rwidth,
                               int uv_rstride, unsigned char *wgt, TPU_BLEND_WGT_MODE wgt_mode,
                               int overlay_lx, int overlay_rx)
{
    unsigned char *wgt_uv = NULL;
    unsigned char alpha_u = 0, alpha_v = 0;
    int overlay_w = overlay_rx - overlay_lx + 1;
    int overlay_uv_w = overlay_w / 2;

    int uv_bheight = ALIGN(bheight / 2, 2);
    int uv_bwidth = ALIGN(bwidth / 2, 2);

    int uv_lheight = ALIGN(lheight / 2, 2);

    int uv_rheight = ALIGN(rheight / 2, 2);
    int uv_rwidth = ALIGN(rwidth / 2, 2);

    int buv_size = uv_bheight * uv_bstride;
    int luv_size = uv_lheight * uv_lstride;
    int ruv_size = uv_rheight * uv_rstride;

    if (wgt_mode == WGT_YUV_SHARE) {
      wgt_uv = (unsigned char *)malloc(uv_bheight * overlay_uv_w);
      for (int y = 0; y < uv_bheight; y++) {
        for (int x = 0; x < overlay_uv_w; x++) {
          int offset00 = overlay_w * y * 2 + x * 2;
          int offset01 = overlay_w * y * 2 + x * 2 + 1;
          int offset10 = overlay_w * (y * 2 + 1) + x * 2;
          int offset11 = overlay_w * (y * 2 + 1) + x * 2 + 1;
          wgt_uv[y * overlay_uv_w + x] =
              (wgt[offset00] + wgt[offset01] + wgt[offset10] + wgt[offset11]) >> 2;
        }
      }
    }

    for (int y = 0; y < uv_bheight; y++) {
      for (int x = 0; x < uv_bwidth; x++) {
        int blend_u_offet = y * uv_bstride + x;
        int blend_v_offet = buv_size + y * uv_bstride + x;

        if (format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21) {
          blend_u_offet = y * uv_bstride + (format == PIXEL_FORMAT_NV12 ? 2 * x : 2 * x + 1);
          blend_v_offet = y * uv_bstride + (format == PIXEL_FORMAT_NV12 ? 2 * x + 1 : 2 * x);
        }

        if (x < (overlay_lx / 2)) {
          int left_u_pos = y * uv_lstride + x;
          int left_v_pos = luv_size + y * uv_lstride + x;

          if (format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21) {
            left_u_pos = y * uv_lstride + (format == PIXEL_FORMAT_NV12 ? 2 * x : 2 * x + 1);
            left_v_pos = y * uv_lstride + (format == PIXEL_FORMAT_NV12 ? 2 * x + 1 : 2 * x);
          }

          blend_uv_base[blend_u_offet] = left_uv_base[left_u_pos];
          blend_uv_base[blend_v_offet] = left_uv_base[left_v_pos];

        } else if (x > (overlay_rx / 2)) {
          int right_x = x - overlay_lx / 2;
          if (right_x >= uv_rwidth) right_x = uv_rwidth - 1;

          int right_u_pos = y * uv_rstride + right_x;
          int right_v_pos = ruv_size + y * uv_rstride + right_x;

          if (format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21) {
            right_u_pos =
                y * uv_rstride + (format == PIXEL_FORMAT_NV12 ? right_x * 2 : right_x * 2 + 1);
            right_v_pos =
                y * uv_rstride + (format == PIXEL_FORMAT_NV12 ? right_x * 2 + 1 : right_x * 2);
          }

          blend_uv_base[blend_u_offet] = right_uv_base[right_u_pos];
          blend_uv_base[blend_v_offet] = right_uv_base[right_v_pos];
        } else {
          if (wgt_mode == WGT_YUV_SHARE) {
            alpha_u = alpha_v = wgt_uv[y * overlay_uv_w + (x - overlay_lx / 2)];
          } else if (wgt_mode == WGT_UV_SHARE) {
            if (format == PIXEL_FORMAT_YUV_PLANAR_420) {
              alpha_u = alpha_v = wgt[bheight * overlay_w + y * overlay_uv_w + (x - overlay_lx / 2)];
            } else {
              int alpha_ux = format == PIXEL_FORMAT_NV12 ? 2 * (x - overlay_lx / 2)
                                                         : 2 * (x - overlay_lx / 2) + 1;
              int alpha_vx = format == PIXEL_FORMAT_NV12 ? 2 * (x - overlay_lx / 2) + 1
                                                         : 2 * (x - overlay_lx / 2);

              alpha_u = wgt[overlay_w * bheight + y * overlay_w + alpha_ux];
              alpha_v = wgt[overlay_w * bheight + y * overlay_w + alpha_vx];
            }
          }

          int left_u_pos = y * uv_lstride + x;
          int left_v_pos = luv_size + y * uv_lstride + x;

          if (format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21) {
            left_u_pos = y * uv_lstride + (format == PIXEL_FORMAT_NV12 ? 2 * x : 2 * x + 1);
            left_v_pos = y * uv_lstride + (format == PIXEL_FORMAT_NV12 ? 2 * x + 1 : 2 * x);
          }

          unsigned char left_u = left_uv_base[left_u_pos];
          unsigned char left_v = left_uv_base[left_v_pos];

          int right_x = x - overlay_lx / 2;
          if (right_x >= uv_rwidth) right_x = uv_rwidth - 1;

          int right_u_pos = y * uv_rstride + right_x;
          int right_v_pos = ruv_size + y * uv_rstride + right_x;

          if (format == PIXEL_FORMAT_NV12 || format == PIXEL_FORMAT_NV21) {
            right_u_pos =
                y * uv_rstride + (format == PIXEL_FORMAT_NV12 ? right_x * 2 : right_x * 2 + 1);
            right_v_pos =
                y * uv_rstride + (format == PIXEL_FORMAT_NV12 ? right_x * 2 + 1 : right_x * 2);
          }

          unsigned char right_u = right_uv_base[right_u_pos];
          unsigned char right_v = right_uv_base[right_v_pos];

          blend_uv_base[blend_u_offet] = blend_pixel(left_u, right_u, alpha_u);
          blend_uv_base[blend_v_offet] = blend_pixel(left_v, right_v, alpha_v);
        }
      }
    }

    if (wgt_mode == WGT_YUV_SHARE) free(wgt_uv);
}

int cpu_2way_blend(int lwidth, int lheight, int *left_stride, unsigned char *left_img, int rwidth,
                     int rheight, int *right_stride, unsigned char *right_img, int bwidth,
                     int bheight, int *blend_stride, unsigned char *blend_img, int overlay_lx,
                     int overlay_rx, unsigned char *wgt, TPU_BLEND_WGT_MODE wgt_mode, int channel,
                     int format)
{
    unsigned char alpha = 0;

    if (lheight != rheight) {
      printf("left_img right img height");
    }

    int overlay_w = overlay_rx - overlay_lx + 1;
    int lstride = 0, luv_stride = 0, rstride = 0, ruv_stride = 0, bstride = 0, buv_stride = 0;

    if (left_stride != NULL) {
      lstride = left_stride[0];
      if (format == PIXEL_FORMAT_YUV_PLANAR_420 || format == PIXEL_FORMAT_NV12 ||
          format == PIXEL_FORMAT_NV21)
        luv_stride = left_stride[1];
    }

    if (right_stride != NULL) {
      rstride = right_stride[0];
      if (format == PIXEL_FORMAT_YUV_PLANAR_420 || format == PIXEL_FORMAT_NV12 ||
          format == PIXEL_FORMAT_NV21)
        ruv_stride = right_stride[1];
    }

    if (blend_stride != NULL) {
      bstride = blend_stride[0];
      if (format == PIXEL_FORMAT_YUV_PLANAR_420 || format == PIXEL_FORMAT_NV12 ||
          format == PIXEL_FORMAT_NV21)
        buv_stride = blend_stride[1];
    }

    if (format == PIXEL_FORMAT_YUV_PLANAR_420 || format == PIXEL_FORMAT_NV12 ||
        format == PIXEL_FORMAT_NV21) {
      // Y channel
      for (int y = 0; y < bheight; y++) {
        for (int x = 0; x < bwidth; x++) {
          if (x < overlay_lx) {
            blend_img[y * bstride + x] = left_img[y * lstride + x];
          } else if (x > overlay_rx) {
            int right_x = x - overlay_lx;
            if (right_x >= rwidth) right_x = rwidth - 1;

            blend_img[y * bstride + x] = right_img[y * rstride + right_x];
          } else {
            alpha = wgt[y * overlay_w + (x - overlay_lx)];
            unsigned char left_p = left_img[y * lstride + x];

            int right_x = x - overlay_lx;
            if (right_x < 0) right_x = 0;
            if (right_x >= rwidth) right_x = rwidth - 1;
            unsigned char right_p = right_img[y * rstride + right_x];

            blend_img[y * bstride + x] = blend_pixel(left_p, right_p, alpha);
          }
        }
      }

      process_uv_plane(blend_img + bheight * bstride, left_img + lheight * lstride,
                       right_img + rheight * rstride, format, bheight, bwidth, buv_stride, lheight,
                       luv_stride, rheight, rwidth, ruv_stride, wgt, wgt_mode, overlay_lx,
                       overlay_rx);
    } else {
      for (int c = 0; c < channel; c++) {
        for (int y = 0; y < bheight; y++) {
          for (int x = 0; x < bwidth; x++) {
            if (x < overlay_lx) {
              blend_img[c * bheight * bstride + y * bstride + x] =
                  left_img[c * lheight * lstride + y * lstride + x];
            } else if (x > overlay_rx) {
              int right_x = x - overlay_lx;
              if (right_x >= rwidth) right_x = rwidth - 1;
              blend_img[c * bheight * bstride + y * bstride + x] =
                  right_img[c * rheight * rstride + y * rstride + right_x];
            } else {
              alpha = wgt[y * overlay_w + (x - overlay_lx)];
              unsigned char left_pixel = left_img[c * lheight * lstride + y * lstride + x];
              unsigned char right_pixel =
                  right_img[c * rheight * rstride + y * rstride + (x - overlay_lx)];

              blend_img[c * bheight * bstride + y * bstride + x] =
                  blend_pixel(left_pixel, right_pixel, alpha);
            }
          }
        }
      }
    }

    return 0;
}

void grayscale_dilate(const unsigned char* src,
                      unsigned char* dst,
                      int width, int height,
                      int w_stride, int ksize)
{
    const int radius = ksize / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char max_val = 0;

            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        if (src[ny * w_stride + nx] > max_val) {
                            max_val = src[ny * w_stride + nx];
                        }
                    }
                }
            }
            dst[y * w_stride + x] = max_val;
        }
    }
}

void grayscale_erode(const unsigned char* src,
                      unsigned char* dst,
                      int width, int height,
                      int w_stride, int ksize)
{
    const int radius = ksize / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char min_val = UCHAR_MAX;

            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                        if (src[ny * w_stride + nx] < min_val) {
                            min_val = src[ny * w_stride + nx];
                        }
                    }
                }
            }
            dst[y * w_stride + x] = min_val;
        }
    }
}