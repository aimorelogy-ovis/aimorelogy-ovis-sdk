#ifndef _CLOG_PRODUCER_H_
#define _CLOG_PRODUCER_H_

#include "clog_types.h"
#include "clog_ringbuf.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Producer context (opaque) */
typedef struct clog_producer_ctx clog_producer_ctx_t;

/*******************************************************************************/
/* Producer lifecycle */
/*******************************************************************************/

/**
 * Create and initialize producer
 * @param ringbuf Ring buffer to produce to (can be NULL for console-only mode)
 * @return Producer context pointer, or NULL on error
 */
clog_producer_ctx_t *clog_producer_create(clog_ringbuf_t *ringbuf);

/**
 * Destroy producer and cleanup
 * @param ctx Producer context
 */
void clog_producer_destroy(clog_producer_ctx_t *ctx);

/*******************************************************************************/
/* Producer APIs (thread-safe) */
/*******************************************************************************/

/**
 * Output formatted log (thread-safe)
 * @param ctx Producer context
 * @param level Log level
 * @param tag Log tag
 * @param func Function name
 * @param line Line number
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void clog_producer_output(clog_producer_ctx_t *ctx, uint8_t level,
						  const char *tag, const char *func, const long line,
						  const char *format, ...);

/**
 * Output raw log without formatting (thread-safe)
 * @param ctx Producer context
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void clog_producer_output_raw(clog_producer_ctx_t *ctx,
							   const char *format, ...);

/**
 * Output formatted log with va_list (thread-safe)
 * @param ctx Producer context
 * @param level Log level
 * @param tag Log tag
 * @param func Function name
 * @param line Line number
 * @param format Printf-style format string
 * @param args Variable argument list
 */
void clog_producer_output_va(clog_producer_ctx_t *ctx, uint8_t level,
							 const char *tag, const char *func, const long line,
							 const char *format, va_list args);

#ifdef __cplusplus
}
#endif

#endif /* _CLOG_PRODUCER_H_ */
