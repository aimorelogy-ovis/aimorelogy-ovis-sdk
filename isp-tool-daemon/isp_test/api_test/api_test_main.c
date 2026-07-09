
#include <stdio.h>
#include <stdlib.h>

#include "api_test_main.h"

static api_test_fn *g_api_test_fn_arr;
static int g_api_test_fn_cnt;
static int g_api_test_fn_cap;

void reg_api_test_fn(api_test_fn fn)
{
	if (g_api_test_fn_cnt >= g_api_test_fn_cap) {
		g_api_test_fn_cap += 16;
		g_api_test_fn_arr =
			realloc(g_api_test_fn_arr, g_api_test_fn_cap * sizeof(api_test_fn));
	}
	g_api_test_fn_arr[g_api_test_fn_cnt++] = fn;
}

int api_test_main(VI_PIPE ViPipe)
{
	int ret = 0;
	int passed_cnt = 0;
	int failed_cnt = 0;

	if (g_api_test_fn_arr == NULL || g_api_test_fn_cnt <= 0) {
		printf("No API test function registered\n");
		return -1;
	}

	for (int i = 0; i < g_api_test_fn_cnt; i++) {
		ret = g_api_test_fn_arr[i](ViPipe);
		if (ret != 0) {
			failed_cnt++;
		} else {
			passed_cnt++;
		}
	}

	printf("ISP_MODULE_API_TEST finished[pipe: %d], total: %d, passed: %d, failed: %d, passed_ratio: %.2f%%\n",
	       ViPipe, g_api_test_fn_cnt, passed_cnt, failed_cnt,
	       (float)passed_cnt / g_api_test_fn_cnt * 100);

	if (failed_cnt) {
		printf("ISP_MODULE_API_TEST pipe %d fail!\n", ViPipe);
		ret = -1;
	} else {
		ret = 0;
		printf("ISP_MODULE_API_TEST pipe %d success!\n", ViPipe);
	}


	return ret;
}
