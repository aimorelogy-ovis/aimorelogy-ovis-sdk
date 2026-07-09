#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_bin.h"

#define ENV_PQBIN_DIR "PQBIN_DIR"
#define PQBIN_DIR_DEFAULT "/mnt/sd/res_mars3/pqbin"
#define PQBIN_PATH_MAX 512
#define PATH_LS_CAP 16

#ifdef SAFE_FREE
#undef SAFE_FREE
#endif
#define SAFE_FREE(prt) \
	do { \
		if (prt) { \
			free(prt); \
			prt = NULL; \
		} \
	} while (0)

#ifdef SAFE_CLOSE
#undef SAFE_CLOSE
#endif
#define SAFE_CLOSE(fp) \
	do { \
		if (fp) { \
			fclose(fp); \
			fp = NULL; \
		} \
	} while (0)

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef MAGIC_NUMBER
#define MAGIC_NUMBER 0x1835
#endif

#define printf_pqbin(format, ...) \
	printf("[pqbin test]" format, ##__VA_ARGS__)


typedef struct _BIN_PATHS {
	char **path_ls;
	uint32_t count;
	uint32_t cap;
} BIN_PATHS;

static void free_bin_paths(BIN_PATHS *bin_paths)
{
	for (uint32_t cnt = 0; cnt < bin_paths->count; ++cnt) {
		if (bin_paths->path_ls[cnt]) {
			SAFE_FREE(bin_paths->path_ls[cnt]);
			bin_paths->path_ls[cnt] = NULL;
		}
	}
	if (bin_paths->path_ls) {
		SAFE_FREE(bin_paths->path_ls);
	}

	bin_paths->cap = 0;
	bin_paths->count = 0;
}

static const char *get_sensor_mode()
{
	ISP_PUB_ATTR_S isp_pub_attr = {0};
	const char *sensor_mode;

	CVI_ISP_GetPubAttr(0, &isp_pub_attr);

	if (isp_pub_attr.enWDRMode <= WDR_MODE_2To1_LINE) {
		sensor_mode = "sdr";
	} else {
		sensor_mode = "wdr";
	}

	return sensor_mode;
}

static int32_t get_import_path(BIN_PATHS *bin_paths)
{
	const char *dir_path;
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char full_path[PQBIN_PATH_MAX];

	const char *sensor_mode = get_sensor_mode();

	printf_pqbin("sensor mode: %s\n", sensor_mode);

	dir_path = getenv(ENV_PQBIN_DIR);

	if (dir_path == NULL || strlen(dir_path) == 0) {
		printf_pqbin("Can not find the env var: PQBIN_DIR, use the default pqbin dir: %s\n", PQBIN_DIR_DEFAULT);
		dir_path = PQBIN_DIR_DEFAULT;
	} else {
		printf_pqbin("PQBIN DIR：%s\n", dir_path);
	}

	// get the pqbin path
	if ((dp = opendir(dir_path)) == NULL) {
		printf_pqbin("opendir failed");
		return -1;
	}

	while ((entry = readdir(dp)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		snprintf(full_path, PQBIN_PATH_MAX, "%s/%s", dir_path, entry->d_name);
		if (lstat(full_path, &statbuf) == -1) {
			printf_pqbin("lstat failed");
			continue;
		}

		if (S_ISREG(statbuf.st_mode)) {
			if (strstr(entry->d_name, sensor_mode) != NULL) {
				char *tmp = (char *)calloc(1, strlen(full_path) + 1);
				if (tmp == NULL) {
					continue;
				}
				strncpy(tmp, full_path, strlen(full_path));
				if (bin_paths->count >= bin_paths->cap) {
					char **path_ls_tmp = (char **)realloc(bin_paths->path_ls, (bin_paths->cap+PATH_LS_CAP)*sizeof(char *));
					if (path_ls_tmp == NULL) {
						printf_pqbin("bin paths reallocate fail!\n");
						return -1;
					}
					bin_paths->path_ls = path_ls_tmp;
					bin_paths->cap += PATH_LS_CAP;
				}
				bin_paths->path_ls[bin_paths->count++] = tmp;
			}
		}
	}

	closedir(dp);
	return 0;
}

static int32_t get_export_path(BIN_PATHS *bin_paths, uint32_t path_cnt)
{
	const char *dir_path;
	char full_path[PQBIN_PATH_MAX];
	const char *sensor_mode = get_sensor_mode();

	if (path_cnt < 1) {
		path_cnt = 1;
	}

	printf_pqbin("sensor mode: %s\n", sensor_mode);

	dir_path = getenv(ENV_PQBIN_DIR);

	if (dir_path == NULL || strlen(dir_path) == 0) {
		printf_pqbin("Can not find the env var: PQBIN_DIR, use the default pqbin dir: %s\n", PQBIN_DIR_DEFAULT);
		dir_path = PQBIN_DIR_DEFAULT;
	} else {
		printf_pqbin("PQBIN DIR：%s\n", dir_path);
	}

	// check the dir path
	if ((access(dir_path, F_OK) == -1) && (mkdir(dir_path, 0) == -1)) {
		printf_pqbin("%s not exists, and make it fail!\n", dir_path);
		return -1;
	}

	// get time
	char time_str[20];
	time_t now = time(NULL);
	struct tm *local_time = localtime(&now);

	strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", local_time);

	for (uint32_t i = 0; i < path_cnt; ++i) {
		snprintf(full_path, PQBIN_PATH_MAX, "%s/export_%s_%s_%d", dir_path, sensor_mode, time_str, i);

		char *tmp = (char *)calloc(1, strlen(full_path) + 1);

		if (tmp == NULL) {
			continue;
		}

		strncpy(tmp, full_path, strlen(full_path));
		if (bin_paths->count >= bin_paths->cap) {
			char **path_ls_tmp = (char **)realloc(bin_paths->path_ls, (bin_paths->cap+PATH_LS_CAP)*sizeof(char *));
			if (path_ls_tmp == NULL) {
				printf_pqbin("bin paths reallocate fail!\n");
				return -1;
			}
			bin_paths->path_ls = path_ls_tmp;
			bin_paths->cap += PATH_LS_CAP;
		}
		bin_paths->path_ls[bin_paths->count++] = tmp;
	}

	return 0;
}

static int32_t getFileSize(FILE *fp, uint32_t *size)
{
	int32_t ret = 0;

	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	rewind(fp);

	return ret;
}

static void cmp_and_rm_export_path(const char *export_path, BIN_PATHS *import_paths)
{
	FILE *fp_e = NULL;
	FILE *fp_i = NULL;

	fp_e = fopen(export_path, "rb");
	if (fp_e == NULL) {
		printf_pqbin("fopen %s fail!\n", export_path);
		return;
	}

	uint32_t e_size = 0;
	uint32_t i_size = 0;
	uint8_t *e_buffer = NULL;
	uint8_t *i_buffer = NULL;

	getFileSize(fp_e, &e_size);

	e_buffer = (uint8_t *)malloc(e_size * sizeof(uint8_t));

	if (e_buffer == NULL) {
		printf_pqbin("fail to malloc the memory!\n");
		SAFE_CLOSE(fp_e);
		return;
	}

	fread(e_buffer, e_size, 1, fp_e);

	for (uint32_t i = 0; i < import_paths->count && import_paths->path_ls[i]; ++i) {
		if (strcmp(export_path, import_paths->path_ls[i]) == 0) {
			continue;
		}

		fp_i = fopen(import_paths->path_ls[i], "rb");
		getFileSize(fp_i, &i_size);

		if (i_size != e_size) {
			SAFE_CLOSE(fp_i);
			continue;
		}

		i_buffer = (uint8_t *)malloc(i_size * sizeof(uint8_t));

		if (i_buffer == NULL) {
			printf_pqbin("fail to malloc the memory!\n");
		}

		fread(i_buffer, i_size, 1, fp_i);

		if (memcmp(e_buffer, i_buffer, i_size) == 0) {
			// remove the export path
			remove(export_path);
			SAFE_FREE(i_buffer);
			SAFE_CLOSE(fp_i);
			printf_pqbin("remove %s\n", export_path);
			break;
		}

		SAFE_FREE(i_buffer);
		SAFE_CLOSE(fp_i);
	}

	SAFE_FREE(e_buffer);
	SAFE_CLOSE(fp_e);
}

static int32_t test_import_pqbin(const char *bin_path, int dev_num)
{
	int32_t ret = 0;
	FILE *fp = NULL;
	uint8_t *buf = NULL;
	uint32_t u32TempLen = 0;
	uint32_t u32FileSize = 0;

	fp = fopen(bin_path, "rb");
	if (fp == NULL) {
		printf_pqbin("Can't find bin(%s)\n", bin_path);
		ret = -1;
		goto ERROR_HANDLER;
	} else {
		printf_pqbin("import bin: (%s)\n", bin_path);
	}
	getFileSize(fp, &u32FileSize);

	buf = (uint8_t *)malloc(u32FileSize);
	if (buf == NULL) {
		ret = -1;
		printf_pqbin("Allocate memory fail\n");
		goto ERROR_HANDLER;
	}
	u32TempLen = fread(buf, u32FileSize, 1, fp);
	if (u32TempLen <= 0) {
		printf_pqbin("read data to buff fail!\n");
		ret = -1;
		goto ERROR_HANDLER;
	}

	// CVI_BIN_ImportBinData
	ret = CVI_BIN_ImportBinData(buf, u32FileSize);
	if (ret != 0) {
		printf_pqbin("CVI_BIN_ImportBinData error! value:(0x%x)\n", ret);
		goto ERROR_HANDLER;
	}

	for (int pipe = 0; pipe < dev_num; ++pipe) {
		ret = CVI_ISP_GetVDTimeOut(pipe, ISP_VD_BE_END, 200);
		if (ret != 0) {
			printf_pqbin("pipe: %d, CVI_BIN_ImportBinData get video fail after import pqbin data!\n", pipe);
		} else {
			printf_pqbin("pipe: %d, CVI_BIN_ImportBinData get video success after import pqbin data!\n", pipe);
		}
	}
	// CVI_BIN_LoadParamFromBinEx
	for (int pipe = 0; pipe < dev_num; ++pipe) {
		ret = CVI_BIN_LoadParamFromBinEx(CVI_BIN_ID_ISP0 + pipe, buf, u32FileSize);
		if (ret != 0) {
			printf_pqbin("pipe: %d, CVI_BIN_LoadParamFromBinEx error! value:(0x%x)\n", pipe, ret);
			goto ERROR_HANDLER;
		}

		ret = CVI_ISP_GetVDTimeOut(pipe, ISP_VD_BE_END, 200);
		if (ret != 0) {
			printf_pqbin("pipe: %d, CVI_BIN_LoadParamFromBinEx get video fail after import pqbin data!\n", pipe);
		} else {
			printf_pqbin("pipe: %d, CVI_BIN_LoadParamFromBinEx get video success after import pqbin data!\n", pipe);
		}
	}

ERROR_HANDLER:
	SAFE_CLOSE(fp);
	SAFE_FREE(buf);

	return ret;
}

static int32_t test_export_pqbin(const char *bin_path, int dev_num)
{
	int32_t ret = 0;
	char rt_path[PQBIN_PATH_MAX] = {0};
	FILE *fp = NULL;

	CVI_BIN_HEADER header = { 0 };
	CVI_BIN_EXTRA_S extraInfo = { 0 };

	uint32_t u32TempLen = 0;
	uint32_t u32DataLength = CVI_BIN_GetBinTotalLen();
	uint32_t u32DataLengthSingle = 0;
	uint8_t *pu8Buffer = NULL;

	// total
	pu8Buffer = (uint8_t *)malloc(sizeof(uint8_t) * u32DataLength);

	if (pu8Buffer == NULL) {
		printf_pqbin("malloc fail!\n");
		return -1;
	}

	header.chipId = MAGIC_NUMBER;
	header.extraInfo = extraInfo;
	header.size[CVI_BIN_ID_HEADER] = sizeof(CVI_BIN_HEADER);

	memcpy(pu8Buffer, &header, sizeof(CVI_BIN_HEADER));
	snprintf(rt_path, PQBIN_PATH_MAX, "%s_all.bin", bin_path);

	ret = CVI_BIN_ExportBinData(pu8Buffer, u32DataLength);

	if (ret != 0) {
		printf_pqbin("export bin: %s fail!\n", rt_path);
		goto TEST_EXPORT_FAIL;
	} else {
		printf_pqbin("export bin: %s success!\n", rt_path);
	}

	fp = fopen(rt_path, "wb");
	if (fp == NULL) {
		printf_pqbin("open path: %s fail!\n", rt_path);
		return -1;
	}
	u32TempLen = fwrite(pu8Buffer, sizeof(uint8_t) * u32DataLength, 1, fp);
	if (u32TempLen != 1) {
		ret = -1;
		printf_pqbin("Write bin data to file fail!\n");
		goto TEST_EXPORT_FAIL;
	}
	SAFE_FREE(pu8Buffer);
	SAFE_CLOSE(fp);

	// single data
	for (int pipe = 0; pipe < dev_num; ++pipe) {
		u32DataLengthSingle = CVI_BIN_GetSingleISPBinLen(CVI_BIN_ID_ISP0 + pipe);
		pu8Buffer = (uint8_t *)malloc(sizeof(uint8_t) * u32DataLengthSingle);

		if (pu8Buffer == NULL) {
			printf_pqbin("malloc fail!\n");
			return -1;
		}

		snprintf(rt_path, PQBIN_PATH_MAX, "%s_pipe_%d.bin", bin_path, pipe);
		ret =CVI_BIN_ExportSingleISPBinData(CVI_BIN_ID_ISP0 + pipe, pu8Buffer, u32DataLengthSingle);

		if (ret != 0) {
			printf_pqbin("export bin: %s fail!\n", rt_path);
			goto TEST_EXPORT_FAIL;
		} else {
			printf_pqbin("export bin: %s success!\n", rt_path);
		}

		fp = fopen(rt_path, "wb");
		if (fp == NULL) {
			printf_pqbin("open path: %s fail!\n", rt_path);
			return -1;
		}
		u32TempLen = fwrite(pu8Buffer, sizeof(uint8_t) * u32DataLengthSingle, 1, fp);
		if (u32TempLen != 1) {
			ret = -1;
			printf_pqbin("Write bin data to file fail!\n");
			goto TEST_EXPORT_FAIL;
		}
		SAFE_FREE(pu8Buffer);
		SAFE_CLOSE(fp);
	}

TEST_EXPORT_FAIL:
	SAFE_FREE(pu8Buffer);
	SAFE_CLOSE(fp);

	return ret;
}

int32_t pqbin_test_main(int32_t dev_num)
{
	int32_t ret = 0;
	int32_t import_bin_cnt = 0;
	int32_t import_bin_cnt_fail = 0;
	int32_t export_bin_cnt = 0;
	int32_t export_bin_cnt_fail = 0;

	const uint32_t every_bin_total_import_cnt = 2;
	uint32_t every_bin_import_cnt = 1;
	char rt_path[PQBIN_PATH_MAX] = {0};

	BIN_PATHS export_paths = { 0 };

	get_export_path(&export_paths, 1);

	if (export_paths.count > 0) {
		printf_pqbin("test export pq bin\n");
		for (uint32_t cnt = 0; cnt < export_paths.count && export_paths.path_ls[cnt]; ++cnt) {
			export_bin_cnt++;
			ret = test_export_pqbin(export_paths.path_ls[cnt], dev_num);

			if (ret == 0) {
				printf_pqbin("[success] bin path: %s, export success!\n", export_paths.path_ls[cnt]);
			} else {
				printf_pqbin("[fail] bin path: %s, export fail!\n", export_paths.path_ls[cnt]);
				export_bin_cnt_fail++;
			}
		}
		printf_pqbin("export bin, total: %d, sucess: %d, fail: %d, success rate: %.2f\n",
				export_bin_cnt, export_bin_cnt - export_bin_cnt_fail,
				export_bin_cnt_fail, (export_bin_cnt - export_bin_cnt_fail)/(float)export_bin_cnt);
	} else {
		printf_pqbin("get export pq bin paths fail! Skip pqbin export test!\n");
		export_bin_cnt_fail = 1;
		ret = false;
	}

	BIN_PATHS import_paths = { 0 };

	get_import_path(&import_paths);

	if (import_paths.count > 0) {
		printf_pqbin("test import pq bin\n");
		for (uint32_t cnt = 0; cnt < import_paths.count && import_paths.path_ls[cnt]; ++cnt) {
			import_bin_cnt++;
			every_bin_import_cnt = 1;

			while ((ret = test_import_pqbin(import_paths.path_ls[cnt], dev_num)) == 0) {
				if (every_bin_import_cnt++ >= every_bin_total_import_cnt) {
					break;
				}
				sleep(1);
			}

			if (ret == 0) {
				printf_pqbin("[success] bin path: %s, import success!\n", import_paths.path_ls[cnt]);
			} else {
				printf_pqbin("[fail] bin path: %s, import fail!\n", import_paths.path_ls[cnt]);
				import_bin_cnt_fail++;
			}
		}

		printf_pqbin("import bin, total: %d, sucess: %d, fail: %d, success rate: %.2f\n",
				import_bin_cnt, import_bin_cnt - import_bin_cnt_fail,
				import_bin_cnt_fail, (import_bin_cnt - import_bin_cnt_fail)/(float)import_bin_cnt);
	} else {
		printf_pqbin("get import pq bin paths fail! Skip pqbin import test!\n");
		ret = false;
	}

	// remove the same bin
	printf_pqbin("export cnt: %u\n", export_paths.count);
	for (uint32_t cnt = 0; cnt < export_paths.count && export_paths.path_ls[cnt]; ++cnt) {
		printf_pqbin("cnt: %u\n", cnt);
		snprintf(rt_path, PQBIN_PATH_MAX, "%s_all.bin", export_paths.path_ls[cnt]);
		cmp_and_rm_export_path((const char *)&rt_path, &import_paths);

		for (int pipe = 0; pipe < dev_num; ++pipe) {
			snprintf(rt_path, PQBIN_PATH_MAX, "%s_pipe_%d.bin", export_paths.path_ls[cnt], pipe);
			cmp_and_rm_export_path((const char *)rt_path, &import_paths);
		}
	}

	// free
	free_bin_paths(&import_paths);
	free_bin_paths(&export_paths);

	if (import_bin_cnt_fail > 0 || export_bin_cnt_fail > 0) {
		ret = false;
	}

	return ret;
}
