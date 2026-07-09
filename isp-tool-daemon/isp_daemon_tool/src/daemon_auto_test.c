#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLOG_TAG "AUTO_TEST"
#include "daemon_base.h"
#include "daemon_auto_test.h"
#include "cvi_isp.h"

#define _TEST_FAIL_TAG "[ISP_TEST_FAIL] "
#define _ISP_AUTO_TEST_LIB_NAME "libisp_test.so"

// isp/inc/isp_debug.c
extern int CVI_DEBUG_SetDebugLevel(int level);

typedef int (*test_fn)(VI_PIPE ViPipe);

#define DEFINE_ISP_TEST_CASE(test_name, fn_name, use_loop) \
static int run_isp_##test_name##_test_case(int dev_num) \
{ \
	clog_i("Run ISP " #test_name " test case\n"); \
	\
	void *dl = dlopen(_ISP_AUTO_TEST_LIB_NAME, RTLD_LAZY); \
	\
	if (dl == NULL) { \
		clog_a(_TEST_FAIL_TAG"dlopen [%s] failed: %s\n", _ISP_AUTO_TEST_LIB_NAME, dlerror()); \
		return -1; \
	} \
	\
	test_fn fn = (test_fn)dlsym(dl, #fn_name); \
	\
	if (fn == NULL) { \
		clog_a(_TEST_FAIL_TAG"dlsym [%s] failed: %s\n", #fn_name, dlerror()); \
		dlclose(dl); \
		return -1; \
	} \
	\
	int ret = 0; \
	\
	if (use_loop) { \
		for (int pipe = 0; pipe < dev_num; ++pipe) { \
			ret |= fn(pipe); \
		} \
	} else { \
		ret = fn(dev_num); \
	} \
	\
	dlclose(dl); \
	return ret; \
}

DEFINE_ISP_TEST_CASE(module_api, api_test_main, 1)
DEFINE_ISP_TEST_CASE(dump, dump_test_main, 1)
DEFINE_ISP_TEST_CASE(pqbin, pqbin_test_main, 0)
DEFINE_ISP_TEST_CASE(flicker_detect, flicker_detect_test_main, 1)

int run_isp_auto_test_case(ISP_AUTO_TEST_CASE_E test_case, int dev_num)
{
	int ret = 0;

	clog_i("Run ISP auto test case: %d\n", test_case);

	switch (test_case) {
	case ISP_TEST_NONE:
		ret = 0;
		break;
	case ISP_TEST_MODULE_API:
		CVI_DEBUG_SetDebugLevel(CLOG_LVL_ASSERT);
		ret = run_isp_module_api_test_case(dev_num);
		CVI_DEBUG_SetDebugLevel(CLOG_LVL_ERROR);
		if (ret == 0) {
			printf("ISP_MODULE_API_TEST SUCCESS!\n");
		}
		break;
	case ISP_TEST_DUMP:
		ret = run_isp_dump_test_case(dev_num);
		if (ret == 0) {
			printf("ISP_DUMP_TEST SUCCESS!\n");
		}
		break;
	case ISP_TEST_PQBIN:
		sleep(2);
		ret = run_isp_pqbin_test_case(dev_num);
		if (ret == 0) {
			printf("ISP_PQBIN_TEST SUCCESS!\n");
		}
		break;
	case ISP_TEST_FLICKER_DETECT:
		ret = run_isp_flicker_detect_test_case(dev_num);
		if (ret == 0) {
			printf("ISP_FLICKER_DETECT_TEST SUCCESS!\n");
		}
		break;
	default:
		break;
	}

	clog_i("Run ISP auto test case %d finished with result: %d\n", test_case, ret);

	return ret;
}
