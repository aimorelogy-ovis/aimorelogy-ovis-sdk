
#include "daemon_base.h"

int module_queue_init(module_queue_t *queue, uint32_t capacity)
{
	if (!queue || capacity == 0) {
		clog_e("invalid input\n");
		return -1;
	}

	queue->data = (void **)malloc(sizeof(void *) * capacity);
	if (!queue->data) {
		clog_e("malloc failed\n");
		return -1;
	}

	queue->size = 0;
	queue->capacity = capacity;
	queue->head = 0;
	queue->tail = 0;

	pthread_mutex_init(&queue->mutex, NULL);
	pthread_condattr_init(&queue->condattr);
	pthread_condattr_setclock(&queue->condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&queue->cond_not_full, &queue->condattr);
	pthread_cond_init(&queue->cond_not_empty, &queue->condattr);

	return 0;
}

int module_queue_deinit(module_queue_t *queue)
{
	if (!queue) {
		clog_e("invalid input\n");
		return -1;
	}

	free(queue->data);
	queue->data = NULL;

	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->cond_not_full);
	pthread_cond_destroy(&queue->cond_not_empty);
	pthread_condattr_destroy(&queue->condattr);

	return 0;
}

int module_queue_push(module_queue_t *queue, void *data, int timeout_ms)
{
	if (!queue || !data) {
		clog_e("invalid input\n");
		return -1;
	}

	pthread_mutex_lock(&queue->mutex);
	while (queue->size == queue->capacity) {
		if (timeout_ms <= 0) {
			pthread_cond_wait(&queue->cond_not_full, &queue->mutex);
		} else {
			struct timespec ts;

			clock_gettime(CLOCK_MONOTONIC, &ts);
			uint64_t nsec =
				ts.tv_nsec + (timeout_ms % 1000) * 1000000;
			ts.tv_nsec = nsec % 1000000000;
			ts.tv_sec += timeout_ms / 1000 + nsec / 1000000000;
			if (pthread_cond_timedwait(&queue->cond_not_full,
						   &queue->mutex, &ts) != 0) {
				//clog_e("timeout\n");
				pthread_mutex_unlock(&queue->mutex);
				return -1;
			}
		}
	}

	queue->data[queue->tail] = data;
	queue->tail = (queue->tail + 1) % queue->capacity;
	queue->size++;

	pthread_cond_signal(&queue->cond_not_empty);
	pthread_mutex_unlock(&queue->mutex);

	return 0;
}

int module_queue_pop(module_queue_t *queue, void **data, int timeout_ms)
{
	if (!queue || !data) {
		clog_e("invalid input\n");
		return -1;
	}

	pthread_mutex_lock(&queue->mutex);
	while (queue->size == 0) {
		if (timeout_ms <= 0) {
			pthread_cond_wait(&queue->cond_not_empty, &queue->mutex);
		} else {
			struct timespec ts;

			clock_gettime(CLOCK_MONOTONIC, &ts);
			uint64_t nsec =
				ts.tv_nsec + (timeout_ms % 1000) * 1000000;
			ts.tv_nsec = nsec % 1000000000;
			ts.tv_sec += timeout_ms / 1000 + nsec / 1000000000;
			if (pthread_cond_timedwait(&queue->cond_not_empty,
						   &queue->mutex, &ts) != 0) {
				//clog_e("timeout\n");
				pthread_mutex_unlock(&queue->mutex);
				return -1;
			}
		}
	}

	*data = queue->data[queue->head];
	queue->head = (queue->head + 1) % queue->capacity;
	queue->size--;

	pthread_cond_signal(&queue->cond_not_full);
	pthread_mutex_unlock(&queue->mutex);

	return 0;
}

int module_queue_size(module_queue_t *queue)
{
	if (!queue) {
		clog_e("invalid input\n");
		return -1;
	}

	return queue->size;
}

module_t *module_pipe_add(module_pipe_t *pipe, module_t *module)
{
	if (!pipe || !module) {
		clog_e("invalid input\n");
		return NULL;
	}

	module_pipe_node_t *node =
		(module_pipe_node_t *)malloc(sizeof(module_pipe_node_t));

	if (!node) {
		clog_e("malloc failed\n");
		return NULL;
	}

	node->module = *module;

	if (pipe->head == NULL) {
		pipe->head = node;
	}

	if (pipe->tail) {
		pipe->tail->next = node;
		node->prev = pipe->tail;
		node->next = NULL;
		pipe->tail = node;
	} else {
		pipe->tail = node;
		node->prev = NULL;
		node->next = NULL;
	}

	return &node->module;
}

int module_pipe_destroy(module_pipe_t *pipe)
{
	if (!pipe) {
		clog_e("invalid input\n");
		return -1;
	}

	module_pipe_node_t *node = pipe->head;

	while (node) {
		module_pipe_node_t *next = node->next;

		free(node);
		node = next;
	}

	pipe->head = NULL;
	pipe->tail = NULL;

	return 0;
}

int module_pipe_init(module_pipe_t *pipe)
{
	if (!pipe) {
		clog_e("invalid input\n");
		return -1;
	}

	module_pipe_node_t *node = pipe->head;

	while (node) {
		if (node->module.fun.init) {
			if (node->module.fun.init(&(node->module)) != 0) {
				clog_e("pipe: %s, module: %s init failed\n",
				       pipe->name, node->module.name);
				return -1;
			}
		}
		node = node->next;
	}

	return 0;
}

int module_pipe_deinit(module_pipe_t *pipe)
{
	if (!pipe) {
		clog_e("invalid input\n");
		return -1;
	}

	module_pipe_node_t *node = pipe->tail;

	while (node) {
		if (node->module.fun.deinit) {
			if (node->module.fun.deinit(&(node->module)) != 0) {
				clog_e("pipe: %s, module: %s deinit failed\n",
				       pipe->name, node->module.name);
				return -1;
			}
		}
		node = node->prev;
	}

	return 0;
}

int module_pipe_start(module_pipe_t *pipe)
{
	if (!pipe) {
		clog_e("invalid input\n");
		return -1;
	}

	module_pipe_node_t *node = pipe->head;

	while (node) {
		if (node->module.fun.start) {
			if (node->module.fun.start(&(node->module)) != 0) {
				clog_e("pipe: %s, module: %s run failed\n",
				       pipe->name, node->module.name);
				return -1;
			}
		}
		node = node->next;
	}

	return 0;
}

int module_pipe_stop(module_pipe_t *pipe)
{
	if (!pipe) {
		clog_e("invalid input\n");
		return -1;
	}

	module_pipe_node_t *node = pipe->tail;

	while (node) {
		if (node->module.fun.stop) {
			if (node->module.fun.stop(&(node->module)) != 0) {
				clog_e("pipe: %s, module: %s stop failed\n",
				       pipe->name, node->module.name);
				return -1;
			}
		}
		node = node->prev;
	}

	return 0;
}
