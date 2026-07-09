#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "sample_lsc_cali.h"

typedef struct {
	char raw_path[512];
	char txt_path[512];
	char dir_path[512];
	CVI_U32 width;
	CVI_U32 height;
	BAYER_FORMAT_E bayer_id;
	CVI_BOOL is_wdr;
	int color_temp;
	int blc_offset_r;
	int blc_offset_gr;
	int blc_offset_gb;
	int blc_offset_b;
	int blc_gain_r;
	int blc_gain_gr;
	int blc_gain_gb;
	int blc_gain_b;
} RAW_SAMPLE_INFO;

static double get_elapsed_ms(struct timeval *start, struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_usec - start->tv_usec) / 1000.0;
}

static void log_time(const char *prefix, const char *label, double elapsed_ms)
{
	FILE *log_fp = fopen("log.txt", "a");

	if (log_fp) {
		fprintf(log_fp, "[%s] %s: %.2f ms (%.3f s)\n", prefix, label, elapsed_ms, elapsed_ms / 1000.0);
		fclose(log_fp);
	}
	printf("  %s: %.2f ms (%.3f s)\n", label, elapsed_ms, elapsed_ms / 1000.0);
}

static char *skip_spaces(char *p)
{
	while (*p && isspace((unsigned char)*p))
		p++;
	return p;
}

static char *find_equal(char *p)
{
	while (*p && *p != '=')
		p++;
	return p;
}

static int parse_txt_file(const char *txt_path, RAW_SAMPLE_INFO *info)
{
	FILE *fp = fopen(txt_path, "r");

	if (!fp) {
		printf("fail to open txt file: %s\n", txt_path);
		return CVI_FAILURE;
	}

	// set defaults
	info->color_temp = 5000;
	info->blc_offset_r = 0;
	info->blc_offset_gr = 0;
	info->blc_offset_gb = 0;
	info->blc_offset_b = 0;
	info->blc_gain_r = 1024;
	info->blc_gain_gr = 1024;
	info->blc_gain_gb = 1024;
	info->blc_gain_b = 1024;

	char line[1024];

	while (fgets(line, sizeof(line), fp)) {
		char *p = line;
		int val;

		p = skip_spaces(p);
		if (strncmp(p, "Color Temp.", 11) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->color_temp = val;
		} else if (strncmp(p, "reg_blc_offset_r", 16) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_offset_r = val;
		} else if (strncmp(p, "reg_blc_offset_gr", 17) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_offset_gr = val;
		} else if (strncmp(p, "reg_blc_offset_gb", 17) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_offset_gb = val;
		} else if (strncmp(p, "reg_blc_offset_b", 16) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_offset_b = val;
		} else if (strncmp(p, "reg_blc_gain_r", 14) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_gain_r = val;
		} else if (strncmp(p, "reg_blc_gain_gr", 15) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_gain_gr = val;
		} else if (strncmp(p, "reg_blc_gain_gb", 15) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_gain_gb = val;
		} else if (strncmp(p, "reg_blc_gain_b", 14) == 0) {
			char *eq = find_equal(p);

			if (*eq == '=' && sscanf(eq + 1, "%d", &val) == 1)
				info->blc_gain_b = val;
		}
	}

	fclose(fp);
	return CVI_SUCCESS;
}

static BAYER_FORMAT_E parse_bayer_format(const char *name)
{
	if (strstr(name, "RGGB") || strstr(name, "rggb"))
		return BAYER_FORMAT_RG;
	if (strstr(name, "GRBG") || strstr(name, "grbg"))
		return BAYER_FORMAT_GR;
	if (strstr(name, "GBRG") || strstr(name, "gbrg"))
		return BAYER_FORMAT_GB;
	return BAYER_FORMAT_BG;
}

static int parse_raw_filename(const char *filename, RAW_SAMPLE_INFO *info)
{
	// Format: WxH_BAYER_Linear-color=X_-bits=X_-frame=X_-hdr=X_ISO=X_YYYYMMDDHHMMSS.raw
	// WDR variant: WxH_BAYER_WDR_-color=X_-bits=X_-frame=X_-hdr=X_ISO=X_YYYYMMDDHHMMSS.raw
	char tmp[512];

	strncpy(tmp, filename, sizeof(tmp) - 1);
	tmp[sizeof(tmp) - 1] = '\0';

	char *dot = strrchr(tmp, '.');

	if (dot)
		*dot = '\0';

	char *x_pos = strchr(tmp, 'X');

	if (!x_pos)
		return CVI_FAILURE;
	*x_pos = '\0';
	info->width = (CVI_U32)atoi(tmp);
	info->height = (CVI_U32)atoi(x_pos + 1);

	info->is_wdr = CVI_FALSE;
	if (strstr(tmp, "WDR") || strstr(tmp, "wdr"))
		info->is_wdr = CVI_TRUE;

	info->bayer_id = parse_bayer_format(x_pos + 1);

	return CVI_SUCCESS;
}

static int is_dir(const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0)
		return 0;
	return S_ISDIR(st.st_mode);
}

static int extract_color_temp_from_dirname(const char *dirname)
{
	// e.g. "A(2800K)" -> 2800, "D75(7500K)" -> 7500
	const char *p = strchr(dirname, '(');

	if (!p)
		return 0;
	return atoi(p + 1);
}

static int collect_raw_samples(const char *ct_dir, RAW_SAMPLE_INFO **out_samples, int *out_count)
{
	DIR *dir = opendir(ct_dir);

	if (!dir) {
		printf("fail to open directory: %s\n", ct_dir);
		return CVI_FAILURE;
	}

	int max_samples = 0;
	struct dirent *entry;

	while ((entry = readdir(dir)) != NULL) {
		if (strstr(entry->d_name, ".txt"))
			max_samples++;
	}
	rewinddir(dir);

	if (max_samples == 0) {
		closedir(dir);
		printf("no .txt files found in: %s\n", ct_dir);
		return CVI_FAILURE;
	}

	RAW_SAMPLE_INFO *samples = (RAW_SAMPLE_INFO *)calloc(max_samples, sizeof(RAW_SAMPLE_INFO));

	if (!samples) {
		closedir(dir);
		printf("fail to allocate memory for samples\n");
		return CVI_FAILURE;
	}

	int count = 0;

	while ((entry = readdir(dir)) != NULL) {
		if (!strstr(entry->d_name, ".txt"))
			continue;
		if (strstr(entry->d_name, "-aelog") || strstr(entry->d_name, "-awblog") ||
				strstr(entry->d_name, "-expInfo"))
			continue;

		RAW_SAMPLE_INFO *si = &samples[count];

		snprintf(si->txt_path, sizeof(si->txt_path), "%s/%s", ct_dir, entry->d_name);
		snprintf(si->dir_path, sizeof(si->dir_path), "%s", ct_dir);

		char raw_name[512];

		strncpy(raw_name, entry->d_name, sizeof(raw_name) - 1);
		raw_name[sizeof(raw_name) - 1] = '\0';
		char *txt_ext = strstr(raw_name, ".txt");

		if (txt_ext)
			*txt_ext = '\0';
		strncat(raw_name, ".raw", sizeof(raw_name) - strlen(raw_name) - 1);

		char raw_path_buf[1024];

		snprintf(raw_path_buf, sizeof(raw_path_buf), "%s/%s", ct_dir, raw_name);
		strncpy(si->raw_path, raw_path_buf, sizeof(si->raw_path) - 1);

		struct stat st;

		if (stat(si->raw_path, &st) != 0) {
			printf("raw file not found: %s, skipping\n", si->raw_path);
			continue;
		}

		if (parse_raw_filename(raw_name, si) != CVI_SUCCESS) {
			printf("fail to parse raw filename: %s, skipping\n", raw_name);
			continue;
		}

		if (parse_txt_file(si->txt_path, si) != CVI_SUCCESS) {
			printf("fail to parse txt file: %s, skipping\n", si->txt_path);
			continue;
		}

		count++;
	}

	closedir(dir);

	*out_samples = samples;
	*out_count = count;
	return CVI_SUCCESS;
}

static void get_timestamp_suffix(char *buf, size_t buf_size)
{
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);

	strftime(buf, buf_size, "%Y%m%d%H%M%S", tm);
}

/* create verify output directory: ./result/ret_offline_<mode>_<ts>/ */
static int create_verify_output_dir_offline(const char *mode_str, int color_temp, char *out_dir, size_t out_size)
{
	char ts[32];

	get_timestamp_suffix(ts, sizeof(ts));

	char cwd[1024];

	if (!getcwd(cwd, sizeof(cwd))) {
		printf("  fail to get cwd\n");
		return CVI_FAILURE;
	}

	/* create parent result dir first */
	char parent[1024];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(out_dir, out_size, "%s/result/ret_offline_%d_%s_%s", cwd, color_temp, mode_str, ts);
	snprintf(parent, sizeof(parent), "%s/result", cwd);
#pragma GCC diagnostic pop
	mkdir(parent, 0755);
	mkdir(out_dir, 0755);

	printf("  verify output dir: %s\n", out_dir);
	return CVI_SUCCESS;
}

/* save lsc_r_gain, lsc_g_gain, lsc_b_gain as 2D grid (num_knot_y rows x num_knot_x cols) */
static int save_lsc_gain(const char *dir_path,
			 int *lsc_r_gain, int *lsc_g_gain, int *lsc_b_gain,
			 int num_knot_x, int num_knot_y)
{
	char ts[32];

	get_timestamp_suffix(ts, sizeof(ts));

	char filepath[1024];

	snprintf(filepath, sizeof(filepath), "%s/lsc_gain_%s.txt", dir_path, ts);

	FILE *fp = fopen(filepath, "w");

	if (!fp) {
		printf("fail to open gain output file: %s\n", filepath);
		return CVI_FAILURE;
	}

	fprintf(fp, "# LSC Gain Table (num_knot_x=%d, num_knot_y=%d)\n\n", num_knot_x, num_knot_y);

	fprintf(fp, "lsc_r_gain:\n");
	for (int y = 0; y < num_knot_y; y++) {
		for (int x = 0; x < num_knot_x; x++) {
			int idx = y * num_knot_x + x;

			fprintf(fp, "%d", lsc_r_gain[idx]);
			fprintf(fp, (x == num_knot_x - 1) ? "\n" : ", ");
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "lsc_g_gain:\n");
	for (int y = 0; y < num_knot_y; y++) {
		for (int x = 0; x < num_knot_x; x++) {
			int idx = y * num_knot_x + x;

			fprintf(fp, "%d", lsc_g_gain[idx]);
			fprintf(fp, (x == num_knot_x - 1) ? "\n" : ", ");
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "lsc_b_gain:\n");
	for (int y = 0; y < num_knot_y; y++) {
		for (int x = 0; x < num_knot_x; x++) {
			int idx = y * num_knot_x + x;

			fprintf(fp, "%d", lsc_b_gain[idx]);
			fprintf(fp, (x == num_knot_x - 1) ? "\n" : ", ");
		}
	}

	fclose(fp);
	printf("  saved gain table: %s\n", filepath);
	fflush(stdout);
	return CVI_SUCCESS;
}

/* save rlsc params: lsc_radius_gain, center_x, center_y, radius, norm */
static int save_lsc_radius(const char *dir_path,
			   int *lsc_radius_gain,
			   int center_x, int center_y, int radius, int norm)
{
	char ts[32];

	get_timestamp_suffix(ts, sizeof(ts));

	char filepath[1024];

	snprintf(filepath, sizeof(filepath), "%s/lsc_radius_%s.txt", dir_path, ts);

	FILE *fp = fopen(filepath, "w");

	if (!fp) {
		printf("fail to open radius output file: %s\n", filepath);
		return CVI_FAILURE;
	}

	fprintf(fp, "# LSC Radius Parameters\n");
	fprintf(fp, "center_x  = %d\n", center_x);
	fprintf(fp, "center_y  = %d\n", center_y);
	fprintf(fp, "radius    = %d\n", radius);
	fprintf(fp, "norm      = %d\n\n", norm);

	fprintf(fp, "# lsc_radius_gain (%d elements, x4 channels)\n", ISP_RLSC_WINDOW_SIZE * 4);
	for (int i = 0; i < ISP_RLSC_WINDOW_SIZE * 4; i++) {
		fprintf(fp, "%d", lsc_radius_gain[i]);
		if ((i + 1) % ISP_RLSC_WINDOW_SIZE == 0)
			fprintf(fp, "\n\n");
		else
			fprintf(fp, ", ");
	}

	fclose(fp);
	printf("  saved radius params: %s\n", filepath);
	fflush(stdout);
	return CVI_SUCCESS;
}

/* save verify raw image: lsc_raw_image or rlsc_raw_image */
static int save_verify_raw(const char *dir_path, const char *prefix, uint16_t *raw_image,
			   int width, int height)
{
	char ts[32];

	get_timestamp_suffix(ts, sizeof(ts));

	char filepath[1024];

	snprintf(filepath, sizeof(filepath), "%s/%s_%s.raw", dir_path, prefix, ts);

	FILE *fp = fopen(filepath, "wb");

	if (!fp) {
		printf("  fail to open verify output file: %s\n", filepath);
		return CVI_FAILURE;
	}

	/* write as raw 16-bit unpacked data */
	CVI_U32 total = (CVI_U32)width * (CVI_U32)height;

	for (CVI_U32 i = 0; i < total; i++) {
		CVI_U16 val = (CVI_U16)raw_image[i];

		fwrite(&val, sizeof(CVI_U16), 1, fp);
	}

	fclose(fp);
	printf("  saved verify raw: %s (%dx%d)\n", filepath, width, height);
	return CVI_SUCCESS;
}

/* offline calibration: no CVI_ISP_Get/Set, only call algo + save results */
/* calib_mode: 0=MESH_CHROMA_MODE(chroma+luma 2-pass), 1=MESH_CHROMA_LUMA_MODE(single pass) */
static int do_offline_calibration_one(RAW_SAMPLE_INFO *info, int calib_mode, int enable_verify)
{
	CVI_U32 effective_width = info->width;

	if (info->is_wdr)
		effective_width = info->width / 2;

	/* 16-bit unpack: 2 bytes per pixel */
	CVI_U32 read_size = effective_width * info->height * 2;
	CVI_U32 stride = effective_width * 2;

	FILE *fp = fopen(info->raw_path, "rb");

	if (!fp) {
		printf("fail to open raw file: %s\n", info->raw_path);
		return CVI_FAILURE;
	}

	fseek(fp, 0, SEEK_END);
	CVI_U32 file_len = (CVI_U32)ftell(fp);

	fseek(fp, 0, SEEK_SET);

	if (file_len <= 0) {
		printf("raw file is empty: %s\n", info->raw_path);
		fclose(fp);
		return CVI_FAILURE;
	}

	CVI_U8 *raw_data = (CVI_U8 *)calloc(read_size, 1);

	if (!raw_data) {
		printf("calloc memory size: %u fail!\n", read_size);
		fclose(fp);
		return CVI_FAILURE;
	}

	size_t n_read = fread(raw_data, 1, read_size, fp);

	fclose(fp);

	if (n_read != read_size) {
		printf("fread incomplete: expected %u, got %zu\n", read_size, n_read);
		FREE(raw_data);
		return CVI_FAILURE;
	}

	printf("loaded raw file: %s\n", info->raw_path);
	printf("  w=%u, h=%u, bayer_id=%d, is_wdr=%d, color_temp=%d\n",
			effective_width, info->height, info->bayer_id,
			info->is_wdr, info->color_temp);
	printf("  blc_offset: r=%d gr=%d gb=%d b=%d\n",
			info->blc_offset_r, info->blc_offset_gr,
			info->blc_offset_gb, info->blc_offset_b);

	uint16_t *raw_data_unpack = (uint16_t *)calloc(effective_width * info->height, sizeof(uint16_t));

	if (!raw_data_unpack) {
		printf("calloc memory size: %u fail!\n", effective_width * info->height * sizeof(uint16_t));
		FREE(raw_data);
		return CVI_FAILURE;
	}

	int ret = unpack_raw(raw_data, raw_data_unpack, effective_width, info->height,
			     stride, RAW_UNCOMPRESS_UNPACK);

	FREE(raw_data);
	if (ret != CVI_SUCCESS) {
		FREE(raw_data_unpack);
		return CVI_FAILURE;
	}

	/* call algo directly with blc from txt file, no ISP interaction */
	int num_knot_x = CVI_ISP_LSC_GRID_COL;
	int num_knot_y = CVI_ISP_LSC_GRID_ROW;
	int fisheye_flag = 0;

	int center_x, center_y, radius = 0, norm;
	int *lsc_r_gain = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
	int *lsc_g_gain = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
	int *lsc_b_gain = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
	int *lsc_radius_gain = (int *)calloc(ISP_RLSC_WINDOW_SIZE * 4, sizeof(int));

	if (!lsc_r_gain || !lsc_g_gain || !lsc_b_gain || !lsc_radius_gain) {
		printf("alloc the mlsc & rlsc gain fail!\n");
		FREE(lsc_r_gain); FREE(lsc_g_gain); FREE(lsc_b_gain); FREE(lsc_radius_gain);
		FREE(raw_data_unpack);
		return CVI_FAILURE;
	}

	if (calib_mode == MESH_CHROMA_MODE) {
		/* MESH_CHROMA_MODE: mlsc do chroma, rlsc do luma (two passes) */
		printf("offline: mlsc do chroma correction, rlsc do luma correction...\n");
		struct timeval tv_start, tv_end;

		gettimeofday(&tv_start, NULL);
		isp_algo_lsc_calibration(
				raw_data_unpack, effective_width, info->height, info->bayer_id,
				num_knot_x, num_knot_y, 1, fisheye_flag,
				info->blc_offset_r, info->blc_offset_gr,
				info->blc_offset_gb, info->blc_offset_b,
				&center_x, &center_y, &radius, &norm,
				lsc_r_gain, lsc_g_gain, lsc_b_gain, lsc_radius_gain);
		int *dummy_r = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
		int *dummy_g = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));
		int *dummy_b = (int *)calloc(CVI_ISP_LSC_GRID_POINTS, sizeof(int));

		if (!dummy_r || !dummy_g || !dummy_b) {
			printf("alloc dummy gain fail!\n");
			FREE(dummy_r); FREE(dummy_g); FREE(dummy_b);
			FREE(raw_data_unpack);
			return CVI_FAILURE;
		}
		isp_algo_lsc_calibration(
				raw_data_unpack, effective_width, info->height, info->bayer_id,
				num_knot_x, num_knot_y, 2, fisheye_flag,
				info->blc_offset_r, info->blc_offset_gr,
				info->blc_offset_gb, info->blc_offset_b,
				&center_x, &center_y, &radius, &norm,
				dummy_r, dummy_g, dummy_b, lsc_radius_gain);
		FREE(dummy_r);
		FREE(dummy_g);
		FREE(dummy_b);
		gettimeofday(&tv_end, NULL);
		log_time("offline", "calibration (chroma 2-pass)", get_elapsed_ms(&tv_start, &tv_end));
	} else {
		printf("offline: mlsc do chroma and luma correction...\n");
		struct timeval tv_start, tv_end;

		gettimeofday(&tv_start, NULL);
		isp_algo_lsc_calibration(
				raw_data_unpack, effective_width, info->height, info->bayer_id,
				num_knot_x, num_knot_y, 0, fisheye_flag,
				info->blc_offset_r, info->blc_offset_gr,
				info->blc_offset_gb, info->blc_offset_b,
				&center_x, &center_y, &radius, &norm,
				lsc_r_gain, lsc_g_gain, lsc_b_gain, lsc_radius_gain);
		gettimeofday(&tv_end, NULL);
		log_time("offline", "calibration (chroma_luma)", get_elapsed_ms(&tv_start, &tv_end));
	}

	/* when verify enabled, create per-color-temp verify dir and save results there */
	const char *save_dir = info->dir_path; /* default: original dir */
	char verify_dir[512] = {0};

	if (enable_verify) {
		const char *mode_str = (calib_mode == MESH_CHROMA_MODE) ? "chroma" : "chroma_luma";

		create_verify_output_dir_offline(mode_str, info->color_temp, verify_dir, sizeof(verify_dir));
		save_dir = verify_dir;
	}

	/* save results */
	save_lsc_gain(save_dir, lsc_r_gain, lsc_g_gain, lsc_b_gain, num_knot_x, num_knot_y);
	save_lsc_radius(save_dir, lsc_radius_gain, center_x, center_y, radius, norm);

	/* verify if enabled */
	if (enable_verify) {
		/* save input raw before verify (verify modifies raw_data_unpack in-place) */
		save_verify_raw(save_dir, "verify_input_raw", raw_data_unpack,
				effective_width, info->height);

		printf("offline: running lsc verify...\n");
		fflush(stdout);
		struct timeval tv_start, tv_end;

		gettimeofday(&tv_start, NULL);

		float blc_rr_gain = info->blc_gain_r / 1024.0f;
		float blc_gr_gain = info->blc_gain_gr / 1024.0f;
		float blc_gb_gain = info->blc_gain_gb / 1024.0f;
		float blc_bb_gain = info->blc_gain_b / 1024.0f;

		CVI_U32 img_pixels = effective_width * info->height;
		uint16_t *lsc_raw_image = (uint16_t *)calloc(img_pixels, sizeof(uint16_t));
		uint16_t *rlsc_raw_image = (uint16_t *)calloc(img_pixels, sizeof(uint16_t));

		if (!lsc_raw_image || !rlsc_raw_image) {
			printf("  alloc verify raw image fail!\n");
			FREE(lsc_raw_image); FREE(rlsc_raw_image);
		} else {
			if (calib_mode == MESH_CHROMA_MODE) {
				/* MESH_CHROMA_MODE: call v2 */
				isp_algo_lsc_verify_v2(
						raw_data_unpack, effective_width, info->height, info->bayer_id,
						num_knot_x, num_knot_y, 0,
						info->blc_offset_r, info->blc_offset_gr,
						info->blc_offset_gb, info->blc_offset_b,
						blc_rr_gain, blc_gr_gain, blc_gb_gain, blc_bb_gain,
						center_x, center_y, radius, norm,
						lsc_r_gain, lsc_g_gain, lsc_b_gain, lsc_radius_gain,
						lsc_raw_image, rlsc_raw_image);
				save_verify_raw(save_dir, "verify_output_lsc_raw", rlsc_raw_image,
						effective_width, info->height);
			} else {
				/* MESH_CHROMA_LUMA_MODE: call v1 */
				isp_algo_lsc_verify_v1(
						raw_data_unpack, effective_width, info->height, info->bayer_id,
						num_knot_x, num_knot_y, 0,
						info->blc_offset_r, info->blc_offset_gr,
						info->blc_offset_gb, info->blc_offset_b,
						blc_rr_gain, blc_gr_gain, blc_gb_gain, blc_bb_gain,
						center_x, center_y, radius, norm,
						lsc_r_gain, lsc_g_gain, lsc_b_gain, NULL,
						lsc_raw_image, NULL);
				save_verify_raw(save_dir, "verify_output_lsc_raw", lsc_raw_image,
						effective_width, info->height);
			}

			gettimeofday(&tv_end, NULL);
			log_time("offline", "verify", get_elapsed_ms(&tv_start, &tv_end));
		}
		FREE(lsc_raw_image);
		FREE(rlsc_raw_image);
	}

	FREE(lsc_r_gain);
	FREE(lsc_g_gain);
	FREE(lsc_b_gain);
	FREE(lsc_radius_gain);
	FREE(raw_data_unpack);

	return CVI_SUCCESS;
}

int run_lsc_calibration_offline_dir(const char *base_dir, int calib_mode, int enable_verify)
{
	DIR *dir = opendir(base_dir);

	if (!dir) {
		printf("fail to open base directory: %s\n", base_dir);
		return CVI_FAILURE;
	}

	struct dirent *entry;
	int total = 0;
	int success = 0;

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;

		char ct_path[512];

		snprintf(ct_path, sizeof(ct_path), "%s/%s", base_dir, entry->d_name);

		if (!is_dir(ct_path))
			continue;

		int dir_color_temp = extract_color_temp_from_dirname(entry->d_name);

		if (dir_color_temp > 0)
			printf("Processing color temperature directory: %s (ct=%d)\n", entry->d_name, dir_color_temp);
		else
			printf("Processing directory: %s\n", entry->d_name);

		RAW_SAMPLE_INFO *samples = NULL;
		int sample_count = 0;

		if (collect_raw_samples(ct_path, &samples, &sample_count) != CVI_SUCCESS) {
			printf("  no valid samples in %s\n", ct_path);
			continue;
		}

		printf("  found %d raw sample(s)\n", sample_count);

		for (int i = 0; i < sample_count; i++) {
			total++;
			printf("  [%d/%d] calibrating: %s\n", i + 1, sample_count, samples[i].raw_path);

			int ret = do_offline_calibration_one(&samples[i], calib_mode, enable_verify);

			if (ret == CVI_SUCCESS) {
				success++;
				printf("  => success\n");
			} else {
				printf("  => failed\n");
			}
		}

		free(samples);
	}

	closedir(dir);

	printf("\nOffline calibration summary: %d/%d succeeded\n", success, total);
	return (total == success && total > 0) ? CVI_SUCCESS : CVI_FAILURE;
}

int run_lsc_calibration_offline(const char *raw_file_path, int color_temp,
				BAYER_FORMAT_E bayer_id, CVI_U32 width, CVI_U32 height,
				int calib_mode, int enable_verify)

{
	RAW_SAMPLE_INFO info = {0};

	snprintf(info.raw_path, sizeof(info.raw_path), "%s", raw_file_path);
	info.width = width;
	info.height = height;
	info.bayer_id = bayer_id;
	info.color_temp = color_temp;
	info.is_wdr = CVI_FALSE;

	return do_offline_calibration_one(&info, calib_mode, enable_verify);
}
