
#ifndef __DAEMON_BASE_H__
#define __DAEMON_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#ifndef CLOG_TAG
#define CLOG_TAG "daemon"
#endif
#include "clog.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define DAEMON_TIMEOUT_MS (500)

typedef struct module_queue_t {
	void **data;
	uint32_t size;
	uint32_t capacity;
	uint32_t head;
	uint32_t tail;

	pthread_mutex_t mutex;
	pthread_condattr_t condattr;
	pthread_cond_t cond_not_full;
	pthread_cond_t cond_not_empty;
} module_queue_t;

int module_queue_init(module_queue_t *queue, uint32_t capacity);
int module_queue_deinit(module_queue_t *queue);
int module_queue_push(module_queue_t *queue, void *data,
		      int timeout_ms); // set -1 for wait forever
int module_queue_pop(module_queue_t *queue, void **data,
		     int timeout_ms); // set -1 for wait forever
int module_queue_size(module_queue_t *queue);

struct module_t;
typedef struct module_fun_t {
	int (*init)(struct module_t *thiz);
	int (*deinit)(struct module_t *thiz);
	int (*start)(struct module_t *thiz);
	int (*stop)(struct module_t *thiz);
	int (*get)(struct module_t *thiz, void **data);
	int (*put)(struct module_t *thiz, void *data);
} module_fun_t;

typedef struct module_t {
	const char *name;
	int pipe_id;
	void *module_cfg;
	void *module_ctx;
	uint8_t thread_run;
	pthread_t thread_id;
	module_queue_t queue;
	module_fun_t fun;
} module_t;

typedef struct module_pipe_node_t {
	struct module_pipe_node_t *prev;
	struct module_pipe_node_t *next;
	module_t module;
} module_pipe_node_t;

typedef struct module_pipe_t {
	module_pipe_node_t *head;
	module_pipe_node_t *tail;
	const char *name;
	void *pipe_cfg;
} module_pipe_t;

#define GET_MODULE_PIPE_NODE_PTR(module_ptr)                                   \
	((module_pipe_node_t *)((                                              \
		char *)(module_ptr)-offsetof(module_pipe_node_t, module)))

module_t *module_pipe_add(module_pipe_t *pipe, module_t *module);
int module_pipe_destroy(module_pipe_t *pipe);
int module_pipe_init(module_pipe_t *pipe);
int module_pipe_deinit(module_pipe_t *pipe);
int module_pipe_start(module_pipe_t *pipe);
int module_pipe_stop(module_pipe_t *pipe);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif // __DAEMON_BASE_H__