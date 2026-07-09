#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "sample_lsc_cali.h"
#include "cvi_ispd2.h"

static CVI_BOOL g_bEnableRun;

static void signal_handler(int signo)
{
	if (g_bEnableRun) {
		signal(signo, SIG_IGN);
		g_bEnableRun = CVI_FALSE;
	} else {
		exit(EXIT_FAILURE);
	}
}

void print_usage(const char *prog)
{
	printf("Usage:\n");
	printf("  %s                                     # online mode (default)\n", prog);
	printf("  %s online [color_temp] [verify]          # online mode\n", prog);
	printf("  %s offline <raw_file> <w> <h> <bayer_id>\n", prog);
	printf("         [color_temp] [calib_mode] [verify]  # offline single file\n");
	printf("  %s offline_dir <base_dir> [mode] [verify]  # offline mode (directory with ct subdirs)\n", prog);
	printf("\n");
	printf("  color_temp: color temperature in Kelvin (default: 5000)\n");
	printf("  bayer_id:   0=RG, 1=GR, 2=GB, 3=BG\n");
	printf("  base_dir:   directory containing color temp subdirs (e.g. res/)\n");
	printf("  mode:       0=MESH_CHROMA_MODE(2-pass), 1=MESH_CHROMA_LUMA_MODE(single, default)\n");
	printf("  calib_mode: 0=MESH_CHROMA_MODE(2-pass), 1=MESH_CHROMA_LUMA_MODE(single, default)\n");
	printf("  verify:     1=run verify after calibration, 0=skip (default)\n");
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	LSC_CALI_MODE_E cali_mode = MODE_ONLINE;
	const char *offline_raw_path = NULL;
	const char *offline_dir_path = NULL;
	CVI_U32 offline_width = 0;
	CVI_U32 offline_height = 0;
	BAYER_FORMAT_E offline_bayer_id = BAYER_FORMAT_BG;
	int color_temp = 5000;
	int calib_mode = 1;   /* default MESH_CHROMA_LUMA_MODE */
	int enable_verify = 0;

	if (argc >= 2) {
		if (strcmp(argv[1], "online") == 0) {
			cali_mode = MODE_ONLINE;
			if (argc >= 3) {
				int tmp_val = atoi(argv[2]);

				if (tmp_val > 0) {
					color_temp = tmp_val;
				}
			}
			if (argc >= 4) {
				enable_verify = atoi(argv[3]);
			}
		} else if (strcmp(argv[1], "offline_dir") == 0) {
			cali_mode = MODE_OFFLINE;
			if (argc < 3) {
				print_usage(argv[0]);
				return -1;
			}
			offline_dir_path = argv[2];
			if (argc >= 4) {
				calib_mode = atoi(argv[3]);
			}
			if (argc >= 5) {
				enable_verify = atoi(argv[4]);
			}
		} else if (strcmp(argv[1], "offline") == 0) {
			cali_mode = MODE_OFFLINE;
			if (argc < 6) {
				print_usage(argv[0]);
				return -1;
			}
			offline_raw_path = argv[2];
			offline_width = atoi(argv[3]);
			offline_height = atoi(argv[4]);
			offline_bayer_id = (BAYER_FORMAT_E)atoi(argv[5]);
			if (argc >= 7) {
				int tmp_val = atoi(argv[6]);

				if (tmp_val > 0) {
					color_temp = tmp_val;
				}
			}
			if (argc >= 8) {
				calib_mode = atoi(argv[7]);
			}
			if (argc >= 9) {
				enable_verify = atoi(argv[8]);
			}
		} else {
			/* backward compat: bare number = online mode color temp */
			int tmp_val = atoi(argv[1]);

			if (tmp_val > 0) {
				color_temp = tmp_val;
			} else {
				print_usage(argv[0]);
				return -1;
			}
		}
	}

	if (cali_mode == MODE_ONLINE) {
		printf("do lsc correction (online) in color temperature: %d, verify=%d\n",
				color_temp, enable_verify);
	} else if (offline_dir_path) {
		printf("do lsc correction (offline dir): base_dir=%s, mode=%d, verify=%d\n",
				offline_dir_path, calib_mode, enable_verify);
	} else {
		printf("do lsc correction (offline): file=%s, w=%u, h=%u, bayer=%d,\n"
				"color_temp=%d, calib_mode=%d, verify=%d\n",
				offline_raw_path, offline_width, offline_height,
				offline_bayer_id, color_temp, calib_mode, enable_verify);
	}

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	if (cali_mode == MODE_ONLINE) {
		s32Ret = online_sys_vi_init();
		if (s32Ret != CVI_SUCCESS) {
			printf("sys vi init failed!\n");
			return s32Ret;
		}

		isp_daemon2_init(JSONRPC_PORT);
	}

	g_bEnableRun = CVI_TRUE;

	if (cali_mode == MODE_OFFLINE) {
		if (offline_dir_path) {
			/* directory mode: traverse all ct subdirs */
			int cali_ret = run_lsc_calibration_offline_dir(offline_dir_path, calib_mode, enable_verify);

			if (cali_ret != CVI_SUCCESS) {
				printf("run lsc calibration offline dir fail!\n");
				s32Ret = -1;
			}
		} else {
			/* single file mode */
			int cali_ret = run_lsc_calibration_offline(
					offline_raw_path, color_temp,
					offline_bayer_id, offline_width, offline_height, calib_mode, enable_verify);
			if (cali_ret != CVI_SUCCESS) {
				printf("run lsc calibration offline fail!\n");
				s32Ret = -1;
			}
		}
	} else {
		/* online: interactive loop */
		while (g_bEnableRun) {
			printf("Press CTRL+D to exit, press other key to run lsc calibration once!\n");
			if (getchar() != EOF) {
				printf("Run lsc calibration once...\n");
				int cali_ret = run_lsc_calibration_online(color_temp, enable_verify);

				if (cali_ret != CVI_SUCCESS) {
					printf("run lsc calibration fail!\n");
				}
			} else {
				break;
			}
		}
	}

	if (cali_mode == MODE_ONLINE) {
		isp_daemon2_uninit();
		online_sys_vi_deinit();
	}

	return s32Ret;
}
