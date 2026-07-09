
#ifndef __API_TEST_MAIN_H__
#define __API_TEST_MAIN_H__

#include "cvi_isp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*api_test_fn)(VI_PIPE ViPipe);

void reg_api_test_fn(api_test_fn fn);

#ifdef __cplusplus
}
#endif
#endif /* __API_TEST_MAIN_H__ */
