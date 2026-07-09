#ifndef _CLOG_TYPES_H_
#define _CLOG_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Common buffer size */
#ifndef CLOG_LINE_BUF_SIZE
#define CLOG_LINE_BUF_SIZE  (1 * 1024)
#endif

/* Log levels */
typedef enum {
	CLOG_LVL_ASSERT = 0,
	CLOG_LVL_ERROR,
	CLOG_LVL_WARN,
	CLOG_LVL_INFO,
	CLOG_LVL_DEBUG,
	CLOG_LVL_VERBOSE,
} clog_level_t;

/* Output mode */
typedef enum {
	CLOG_OUTPUT_FILE    = 0,
	CLOG_OUTPUT_TCP     = 1,
} clog_output_mode_t;

#ifdef __cplusplus
}
#endif

#endif /* _CLOG_TYPES_H_ */
