#ifndef _CLOG_CONSUMER_H_
#define _CLOG_CONSUMER_H_

#include "clog_types.h"
#include "clog_ringbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Consumer configuration */
typedef struct {
	clog_output_mode_t mode;

	/* File output config */
	const char *file_path;
	uint32_t file_split_size;

	/* TCP output config (server mode) */
	uint16_t tcp_port;
} clog_consumer_config_t;

/* Consumer context (opaque) */
typedef struct clog_consumer_ctx clog_consumer_ctx_t;

/*******************************************************************************/
/* Consumer lifecycle */
/*******************************************************************************/

/**
 * Create and initialize consumer
 * @param ringbuf Ring buffer to consume from
 * @param config Consumer configuration
 * @return Consumer context pointer, or NULL on error
 */
clog_consumer_ctx_t *clog_consumer_create(clog_ringbuf_t *ringbuf,
										  const clog_consumer_config_t *config);

/**
 * Start consumer thread
 * @param ctx Consumer context
 * @return 0 on success, -1 on error
 */
int clog_consumer_start(clog_consumer_ctx_t *ctx);

/**
 * Stop consumer thread and cleanup
 * @param ctx Consumer context
 */
void clog_consumer_destroy(clog_consumer_ctx_t *ctx);

/*******************************************************************************/
/* Consumer control (optional, for advanced usage) */
/*******************************************************************************/

/**
 * Process one log entry (for manual polling mode)
 * @param ctx Consumer context
 * @return 0 if processed, -1 if no data available
 */
int clog_consumer_process_one(clog_consumer_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* _CLOG_CONSUMER_H_ */
