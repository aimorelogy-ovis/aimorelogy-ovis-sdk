#ifndef _CLOG_RINGBUF_H_
#define _CLOG_RINGBUF_H_

#include "clog_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE     64
#endif

typedef struct {
	volatile uint32_t write_idx;      /* written by multiple producers */
	uint8_t padding1[CACHE_LINE_SIZE - 4];

	volatile uint32_t read_idx;       /* written by single consumer */
	uint8_t padding2[CACHE_LINE_SIZE - 4];

	uint32_t size;                   /* size of the buffer in bytes */
	uint64_t buffer_phy_addr;        /* physical address of the buffer */
} clog_ringbuf_shared_t;

typedef struct {
	uint64_t shared_phy_addr;       /* physical address of shared control */
	clog_ringbuf_shared_t *shared;  /* shared ring buffer control */
	uint8_t *buffer;                /* buffer memory */
	uint8_t is_client;
} clog_ringbuf_t;

/*******************************************************************************/
/* Ring buffer initialization and cleanup */
/*******************************************************************************/

/**
 * Initialize ring buffer
 * @param rb Ring buffer pointer
 * @param size Buffer size
 * @return 0 on success, -1 on error
 */
int clog_ringbuf_init(clog_ringbuf_t *rb, uint32_t size);

/**
 * Deinitialize ring buffer
 * @param rb Ring buffer pointer
 */
void clog_ringbuf_deinit(clog_ringbuf_t *rb);

/*******************************************************************************/
/* Producer APIs (thread-safe for multiple producers) */
/*******************************************************************************/

/**
 * Write data to ring buffer (copy mode)
 * @param rb Ring buffer pointer
 * @param data Data to write
 * @param len Data length
 * @return 0 on success, -1 if buffer full
 */
int clog_ringbuf_write(clog_ringbuf_t *rb, const char *data, uint32_t len);

/*******************************************************************************/
/* Consumer APIs (single-thread only) */
/*******************************************************************************/

/**
 * Read data from ring buffer (zero-copy mode)
 * Returns all available contiguous data up to wrap point
 * If data wraps around, call again to get remaining data
 * @param rb Ring buffer pointer
 * @param data Output: pointer to data
 * @param len Output: data length
 * @return 0 on success, -1 if buffer empty
 */
int clog_ringbuf_read_zerocopy(clog_ringbuf_t *rb, const char **data, uint32_t *len);

/**
 * Commit read after processing zerocopy data
 * Advances read pointer by the length returned from last read_zerocopy
 * @param rb Ring buffer pointer
 * @param len Length of data that was processed
 */
void clog_ringbuf_read_commit(clog_ringbuf_t *rb, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* _CLOG_RINGBUF_H_ */
