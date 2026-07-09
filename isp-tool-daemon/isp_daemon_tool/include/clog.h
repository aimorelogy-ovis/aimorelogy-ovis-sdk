
#ifndef _CLOG_H_
#define _CLOG_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "clog_types.h"

/* Enable consumer by default, define CLOG_DISABLE_CONSUMER to disable */
#ifndef CLOG_DISABLE_CONSUMER
#define CLOG_ENABLE_CONSUMER 1
#else
#define CLOG_ENABLE_CONSUMER 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************/
/* clog config */
typedef struct {
	uint32_t ringbuf_size;         /* ring buffer size */
	clog_output_mode_t mode;       /* output mode */
	const char *file_path;         /* file path (for FILE mode) */
	uint32_t file_split_size;      /* file split size in bytes */
	uint16_t tcp_port;             /* TCP port for server to listen on (default: 5568) */
} clog_config_t;

/*******************************************************************************/
#define CLOG_LVL_ASSERT                      0
#define CLOG_LVL_ERROR                       1
#define CLOG_LVL_WARN                        2
#define CLOG_LVL_INFO                        3
#define CLOG_LVL_DEBUG                       4
#define CLOG_LVL_VERBOSE                     5

#ifndef CLOG_OUTPUT_LVL
#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#endif

#if CLOG_OUTPUT_LVL >= CLOG_LVL_ASSERT
	#define clog_assert(tag_level, tag, ...) \
			clog_output(tag_level, CLOG_LVL_ASSERT, tag, __func__, __LINE__, __VA_ARGS__)
#else
	#define clog_assert(tag_level, tag, ...)
#endif

#if CLOG_OUTPUT_LVL >= CLOG_LVL_ERROR
	#define clog_error(tag_level, tag, ...) \
			clog_output(tag_level, CLOG_LVL_ERROR, tag, __func__, __LINE__, __VA_ARGS__)
#else
	#define clog_error(tag_level, tag, ...)
#endif

#if CLOG_OUTPUT_LVL >= CLOG_LVL_WARN
	#define clog_warn(tag_level, tag, ...) \
			clog_output(tag_level, CLOG_LVL_WARN, tag, __func__, __LINE__, __VA_ARGS__)
#else
	#define clog_warn(tag_level, tag, ...)
#endif

#if CLOG_OUTPUT_LVL >= CLOG_LVL_INFO
	#define clog_info(tag_level, tag, ...) \
			clog_output(tag_level, CLOG_LVL_INFO, tag, __func__, __LINE__, __VA_ARGS__)
#else
	#define clog_info(tag_level, tag, ...)
#endif

#if CLOG_OUTPUT_LVL >= CLOG_LVL_DEBUG
	#define clog_debug(tag_level, tag, ...) \
			clog_output(tag_level, CLOG_LVL_DEBUG, tag, __func__, __LINE__, __VA_ARGS__)
#else
	#define clog_debug(tag_level, tag, ...)
#endif

#if CLOG_OUTPUT_LVL >= CLOG_LVL_VERBOSE
	#define clog_verbose(tag_level, tag, ...) \
			clog_output(tag_level, CLOG_LVL_VERBOSE, tag, __func__, __LINE__, __VA_ARGS__)
#else
	#define clog_verbose(tag_level, tag, ...)
#endif

#ifndef CLOG_TAG
#define CLOG_TAG "isp"
#endif

#ifndef CLOG_TAG_LEVEL
#define CLOG_TAG_LEVEL CLOG_LVL_DEBUG
#endif

#define clog_a(...)     clog_assert(CLOG_TAG_LEVEL, CLOG_TAG, __VA_ARGS__)
#define clog_e(...)     clog_error(CLOG_TAG_LEVEL, CLOG_TAG, __VA_ARGS__)
#define clog_w(...)     clog_warn(CLOG_TAG_LEVEL, CLOG_TAG, __VA_ARGS__)
#define clog_i(...)     clog_info(CLOG_TAG_LEVEL, CLOG_TAG, __VA_ARGS__)
#define clog_d(...)     clog_debug(CLOG_TAG_LEVEL, CLOG_TAG, __VA_ARGS__)
#define clog_v(...)     clog_verbose(CLOG_TAG_LEVEL, CLOG_TAG, __VA_ARGS__)
#define clog_raw(raw_level, ...)   clog_output_raw(CLOG_TAG_LEVEL, raw_level, __VA_ARGS__)

#define CLOG_ASSERT(EXPR)                        \
	{if (!(EXPR)) {                              \
		clog_a("assert", "%s\n", __func__);      \
	}}

int clog_init(const clog_config_t *config);
int clog_deinit(void);
void clog_output(uint8_t tag_level, uint8_t level, const char *tag, const char *func,
	const long line, const char *format, ...);
void clog_output_raw(uint8_t tag_level, uint8_t level, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
