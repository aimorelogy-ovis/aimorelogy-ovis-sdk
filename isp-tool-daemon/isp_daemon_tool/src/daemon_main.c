
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "cvi_sys.h"
#include "cvi_ispd2.h"
#include "raw_dump.h"
#include "raw_dump_internal.h"
#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_pipe.h"
#include "daemon_auto_test.h"

int g_main_loop_run = 1;

static void signal_handler(int signo)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		g_main_loop_run = 0;
		break;
	default:
		break;
	}
}

static void register_signal_handler(void)
{
	struct sigaction sigaInst;

	sigaInst.sa_flags = 0;
	sigemptyset(&sigaInst.sa_mask);
	sigaddset(&sigaInst.sa_mask, SIGINT);
	sigaddset(&sigaInst.sa_mask, SIGTERM);

	sigaInst.sa_handler = signal_handler;
	sigaction(SIGINT, &sigaInst, NULL);
	sigaction(SIGTERM, &sigaInst, NULL);
}

#define JSONRPC_PORT (5566)
#define DAEMON_PIPE_CFG_JSON_PATH "cfg.json"

#define ENABLE_AUTO_RAW_DUMP 1
#ifdef ENABLE_AUTO_RAW_DUMP
#define ENV_RAW_DUMP_SAVE_PATH  "RAW_DUMP_SAVE_PATH"
#define ENV_RAW_DUMP_PARAM      "RAW_DUMP_PARAM"
#define ENV_RAW_DUMP_REPEAT     "RAW_DUMP_REPEAT"

static int raw_dump_repeat;
static RAW_DUMP_INFO_S st_raw_dump_info;
static int init_raw_dump(void);
#define MAX_RAW_DUMP_PATH_LEN 128
static char raw_dump_path[MAX_RAW_DUMP_PATH_LEN];
static char *get_raw_dump_path(void);
#endif

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	int ret = 0;

	daemon_pipe_cfg_t *p_pipe_cfg = NULL;

	register_signal_handler();

	ret = CVI_SYS_Init();
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_SYS_Init failed!\n");
		return ret;
	}

	if (daemon_pipe_cfg_init(DAEMON_PIPE_CFG_JSON_PATH, &p_pipe_cfg) < 0) {
		clog_e("parse pipe json: %s cfg error!\n",
		       DAEMON_PIPE_CFG_JSON_PATH);
		return -1;
	}

	g_main_loop_run = 1;

	isp_daemon2_init(JSONRPC_PORT);
	cvi_raw_dump_init();
	if (create_pipe(p_pipe_cfg) != 0) {
		clog_e("create pipe fail!\n");
		ret = -1;
		goto ERROR;
	}

#ifdef ENABLE_AUTO_RAW_DUMP
	int enable_raw_dump = 0;

	if (init_raw_dump() >= 0) {
		enable_raw_dump = 1;
	}
#endif

	const char *auto_test_case = getenv("CVI_ISP_AUTO_TEST_CASE");
	int ret_auto_test = 0;

	if (auto_test_case != NULL && auto_test_case[0] != '\0') {
		int case_num = atoi(auto_test_case);

		ret_auto_test = run_isp_auto_test_case(case_num, p_pipe_cfg->dev_num);
		g_main_loop_run = 0; // exit after auto test
	}

	while (g_main_loop_run) {
		sleep(1);

#ifdef ENABLE_AUTO_RAW_DUMP
		if (enable_raw_dump) {
			if (raw_dump_repeat > 0 || raw_dump_repeat == -1) {
				st_raw_dump_info.pathPrefix = get_raw_dump_path();
				cvi_raw_dump(0, &st_raw_dump_info);
				if (raw_dump_repeat > 0) {
					raw_dump_repeat--;
				}
			}
		}
#endif
	}
ERROR:
	destroy_pipe(p_pipe_cfg);
	cvi_raw_dump_uninit();
	isp_daemon2_uninit();
	daemon_pipe_cfg_deinit(&p_pipe_cfg);

	ret = CVI_SYS_Exit();
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_SYS_Exit failed!\n");
	}

	return (ret | ret_auto_test);
}

#ifdef ENABLE_AUTO_RAW_DUMP
static char *get_raw_dump_path(void)
{
	struct tm *t;
	time_t tt;
	char cmd[MAX_RAW_DUMP_PATH_LEN + 32];
	const char *path = getenv(ENV_RAW_DUMP_SAVE_PATH);

	time(&tt);
	t = localtime(&tt);

	snprintf(raw_dump_path, MAX_RAW_DUMP_PATH_LEN, "%s/%04d%02d%02d%02d%02d%02d",
		path,
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);


	snprintf(cmd, (MAX_RAW_DUMP_PATH_LEN + 32), "mkdir %s;sync", raw_dump_path);
	system(cmd);

	return raw_dump_path;
}

static int init_raw_dump(void)
{
	const char *path = getenv(ENV_RAW_DUMP_SAVE_PATH);
	const char *param = getenv(ENV_RAW_DUMP_PARAM);
	const char *repeat = getenv(ENV_RAW_DUMP_REPEAT);

	if (path == NULL) {
		return -1;
	}

	if (param == NULL || repeat == NULL) {
		clog_e("run raw dump fail, parameter missing...\n");
		return -1;
	}

	clog_i("init raw dump, %s %s %s\n", path, param, repeat);

	memset(&st_raw_dump_info, 0, sizeof(st_raw_dump_info));

	if (access(path, F_OK) == 0) {
		st_raw_dump_info.pathPrefix = (char *) path;
	} else {
		clog_e("%s not exist...\n", path);
		return -1;
	}

	sscanf(param, "%d,%d,%d,%d,%d",
		&st_raw_dump_info.u32TotalFrameCnt,
		&st_raw_dump_info.stRoiRect.s32X,
		&st_raw_dump_info.stRoiRect.s32Y,
		&st_raw_dump_info.stRoiRect.u32Width,
		&st_raw_dump_info.stRoiRect.u32Height);

	clog_i("raw dump count: %d, roi: %d,%d,%d,%d\n",
		st_raw_dump_info.u32TotalFrameCnt,
		st_raw_dump_info.stRoiRect.s32X,
		st_raw_dump_info.stRoiRect.s32Y,
		st_raw_dump_info.stRoiRect.u32Width,
		st_raw_dump_info.stRoiRect.u32Height);

	raw_dump_repeat = atoi(repeat);

	return 0;
}
#endif // ENABLE_AUTO_RAW_DUMP
