
#ifndef __DAEMON_AUTO_TEST_H__
#define __DAEMON_AUTO_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	ISP_TEST_NONE = 0,
	ISP_TEST_MODULE_API = 1,
	ISP_TEST_DUMP = 2,
	ISP_TEST_PQBIN = 3,
	ISP_TEST_FLICKER_DETECT = 4,
	ISP_TEST_MAX
} ISP_AUTO_TEST_CASE_E;

int run_isp_auto_test_case(ISP_AUTO_TEST_CASE_E test_case, int dev_num);

#ifdef __cplusplus
}
#endif
#endif
