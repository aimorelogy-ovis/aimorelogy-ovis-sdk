#ifndef SAMPLE_LSC_CALI_H
#define SAMPLE_LSC_CALI_H

#include "cvi_comm_isp.h"
#include "cvi_comm_video.h"
#include "cvi_isp.h"
#include "cvi_vi.h"

#define JSONRPC_PORT (5566)
#define MAX_FRAME_NUM (4)

#define ARRAY_CPY(dst, src, size) \
	do {\
		for (int k = 0; k < size; ++k) {\
			(dst)[k] = (src)[k];\
		} \
	} while (0)

#define FREE(x) \
	do {\
		if (x) {\
			free(x);\
			x = NULL;\
		} \
	} while (0)

typedef enum {
	RAW_UNCOMPRESS_UNPACK,
	RAW_UNCOMPRESS_PACK,
	RAW_COMPRESS,
} RAW_PACK_MODE_E;

typedef enum {
	MODE_ONLINE,
	MODE_OFFLINE,
} LSC_CALI_MODE_E;

typedef struct {
	CVI_U32 width;
	CVI_U32 height;
	CVI_U32 stride;
	CVI_U8 *buffer;
} RAW_INFO;

/* extern from isp_algo */
extern int isp_algo_lsc_calibration(uint16_t *raw_image, int width, int height, int bayer_id,
	int num_knot_y, int num_knot_x, int calib_flag, int fisheye_flag,
	int ob_rr, int ob_gr, int ob_gb, int ob_bb,
	int *center_x, int *center_y, int *radius, int *norm,
	int *lsc_r_gain,
	int *lsc_g_gain,
	int *lsc_b_gain,
	int *lsc_radius_gain);

extern int isp_algo_lsc_verify_v2(uint16_t *raw_image, int width, int height, int bayer_id,
	int num_knot_x, int num_knot_y, int calib_flag,
	int ob_rr, int ob_gr, int ob_gb, int ob_bb,
	float blc_rr_gain, float blc_gr_gain, float blc_gb_gain, float blc_bb_gain,
	int center_x, int center_y, int radius, int norm,
	int *lsc_r_gain,
	int *lsc_g_gain,
	int *lsc_b_gain,
	int *lsc_radius_gain,
	uint16_t *lsc_raw_image,
	uint16_t *rlsc_raw_image);

extern int isp_algo_lsc_verify_v1(uint16_t *raw_image, int width, int height, int bayer_id,
	int num_knot_x, int num_knot_y, int calib_flag,
	int ob_rr, int ob_gr, int ob_gb, int ob_bb,
	float blc_rr_gain, float blc_gr_gain, float blc_gb_gain, float blc_bb_gain,
	int center_x, int center_y, int radius, int norm,
	int *lsc_r_gain,
	int *lsc_g_gain,
	int *lsc_b_gain,
	int *lsc_radius_gain,
	uint16_t *lsc_raw_image,
	uint16_t *rlsc_raw_image);

/* common utilities */
int get_raw_info(int vi_pipe, int vi_chn, RAW_PACK_MODE_E *raw_pack_mode,
	BAYER_FORMAT_E *bayer_format, CVI_U32 *raw_size, CVI_U32 *width, CVI_U32 *height);

int get_raw_data(int vi_pipe, CVI_U8 *raw_data, CVI_U32 raw_size);

int unpack_raw(CVI_U8 *raw_pack, uint16_t *raw_unpack,
	CVI_U32 width, CVI_U32 height, CVI_U32 stride, RAW_PACK_MODE_E raw_pack_mode);

/* online init/deinit */
int online_sys_vi_init(void);
int online_sys_vi_deinit(void);

/* online / offline entry points */
int run_lsc_calibration_online(int color_temp, int enable_verify);

int run_lsc_calibration_offline(const char *raw_file_path, int color_temp,
	BAYER_FORMAT_E bayer_id, CVI_U32 width, CVI_U32 height, int calib_mode, int enable_verify);

/* calib_mode: 0=MESH_CHROMA_MODE(chroma+luma 2-pass), 1=MESH_CHROMA_LUMA_MODE(single pass) */
int run_lsc_calibration_offline_dir(const char *base_dir, int calib_mode, int enable_verify);

void print_usage(const char *prog);

#endif /* SAMPLE_LSC_CALI_H */
