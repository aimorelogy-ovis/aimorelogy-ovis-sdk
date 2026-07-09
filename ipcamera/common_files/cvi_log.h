#ifndef __CVI_LOG_H__
#define __CVI_LOG_H__

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "cvi_datatype.h"

/* debug log level define */
#define CVI_LOG_DEBUG        (5)	// 5
#define CVI_LOG_INFO         (4)	// 4
#define CVI_LOG_WARNING      (3)    // 3
#define CVI_LOG_ERROR        (2)	// 2
#define CVI_LOG_FATAL        (1)	// 1
#define CVI_LOG_NONE         (0)	// 0

/* default builtin log*/
#ifndef BUILT_LOG_LEVEL
#define BUILT_LOG_LEVEL CVI_LOG_INFO
#endif

/* If DEBUG is defined in the file, output DEBUG logs for this file */
#ifdef DEBUG
#undef BUILT_LOG_LEVEL
#define BUILT_LOG_LEVEL CVI_LOG_DEBUG
#endif

#define CVI_LOG(log_level, level_str, fmt, ...)                                                                                 \
	do {                                                                                                  \
		if (BUILT_LOG_LEVEL >= log_level) {                                                               \
			printf(level_str "[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
		}                                                                                                  \
	} while (0)

#define CVI_LOGD(fmt, ...) CVI_LOG(CVI_LOG_DEBUG, "[DBG]", fmt, ##__VA_ARGS__)
#define CVI_LOGI(fmt, ...) CVI_LOG(CVI_LOG_INFO, "[INF]", fmt, ##__VA_ARGS__)
#define CVI_LOGW(fmt, ...) CVI_LOG(CVI_LOG_WARNING, "[WRN]", fmt, ##__VA_ARGS__)
#define CVI_LOGE(fmt, ...) CVI_LOG(CVI_LOG_ERROR, "[ERR]", fmt, ##__VA_ARGS__)
#define CVI_LOGF(fmt, ...) CVI_LOG(CVI_LOG_FATAL, "[FATAL]", fmt, ##__VA_ARGS__)

#ifndef CVI_LOG_ASSERT
#define CVI_LOG_ASSERT(x, ...)     \
	do {                           \
		if (!(x)) {                \
			CVI_LOGE(__VA_ARGS__); \
			abort();               \
		}                          \
	} while (0)
#endif

#endif //__CVI_LOG_H__
