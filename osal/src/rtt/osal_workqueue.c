#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#include "osal.h"
#include "osal_inner.h"

#ifndef USE_IPC_API

static struct rt_mutex g_mutex_workqueue;
static uint32_t thread_counter = 0;
OSAL_LIST_HEAD(wq_list);
struct wq_node {
	osal_workqueue *osal_work;
	struct osal_list_head node;
};

static void workqueue_thread_entry(void *parameter)
{
	osal_workqueue *work = (osal_workqueue *)parameter;
	struct wq_node *node;
	struct osal_list_head *pos, *n;

	while (1) {
		rt_mutex_take(&g_mutex_workqueue, RT_WAITING_FOREVER);
		for (pos = wq_list.next; pos != &wq_list; pos = n) {
			n = pos->next;
			node = rt_container_of(pos, struct wq_node, node);
			if (node->osal_work == work) {
				osal_list_del(&node->node);
				rt_mutex_release(&g_mutex_workqueue);
				if (work->handler) {
					work->handler(work);
				}
				rt_free(node);
				break;
			}
		}
		rt_mutex_release(&g_mutex_workqueue);
		rt_thread_mdelay(10);
	}
}

int osal_workqueue_init(osal_workqueue *work, osal_workqueue_handler handler)
{
	if (work == NULL || handler == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	work->handler = handler;
	work->queue_flag = 0;
	work->work = NULL;

	char thread_name[RT_NAME_MAX];
	snprintf(thread_name, sizeof(thread_name), "workqueue_%d", thread_counter++);

	rt_thread_t thread = rt_thread_create(thread_name, workqueue_thread_entry, work, 4096, 25, 10);
	if (thread == NULL) {
		osal_log("Failed to create thread\n");
		return OSAL_FAILURE;
	}

	if (rt_thread_startup(thread) != RT_EOK) {
		osal_log("Failed to start thread\n");
		rt_thread_delete(thread);
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_workqueue_init);

int osal_workqueue_schedule(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	struct wq_node *node = rt_malloc(sizeof(struct wq_node));
	if (node == NULL) {
		osal_log("Failed to allocate memory\n");
		return OSAL_FAILURE;
	}

	node->osal_work = work;
	OSAL_INIT_LIST_HEAD(&node->node);

	rt_mutex_take(&g_mutex_workqueue, RT_WAITING_FOREVER);
	osal_list_add_tail(&node->node, &wq_list);
	rt_mutex_release(&g_mutex_workqueue);

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_workqueue_schedule);

int osal_workqueue_schedule_highpri(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	struct wq_node *node = rt_malloc(sizeof(struct wq_node));
	if (node == NULL) {
		osal_log("Failed to allocate memory\n");
		return OSAL_FAILURE;
	}

	node->osal_work = work;
	OSAL_INIT_LIST_HEAD(&node->node);

	rt_mutex_take(&g_mutex_workqueue, RT_WAITING_FOREVER);
	osal_list_add(&node->node, &wq_list);
	rt_mutex_release(&g_mutex_workqueue);

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_workqueue_schedule_highpri);

void osal_workqueue_destroy(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return;
	}

	rt_mutex_take(&g_mutex_workqueue, RT_WAITING_FOREVER);
	struct osal_list_head *pos, *n;
	for (pos = wq_list.next; pos != &wq_list; pos = n) {
		n = pos->next;
		struct wq_node *node = rt_container_of(pos, struct wq_node, node);
		if (node->osal_work == work) {
			osal_list_del(&node->node);
			rt_free(node);
		}
	}
	rt_mutex_release(&g_mutex_workqueue);

	if (work->work) {
		rt_free(work->work);
		work->work = NULL;
	}
}
// EXPORT_SYMBOL(osal_workqueue_destroy);

int osal_workqueue_flush(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	rt_mutex_take(&g_mutex_workqueue, RT_WAITING_FOREVER);
	struct osal_list_head *pos, *n;
	for (pos = wq_list.next; pos != &wq_list; pos = n) {
		n = pos->next;
		struct wq_node *node = rt_container_of(pos, struct wq_node, node);
		if (node->osal_work == work) {
			osal_list_del(&node->node);
			rt_free(node);
		}
	}
	rt_mutex_release(&g_mutex_workqueue);

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_workqueue_flush);

int init_workqueue(void)
{
	if (rt_mutex_init(&g_mutex_workqueue, "g_mutex_workqueue", RT_IPC_FLAG_FIFO) != RT_EOK) {
		osal_log("Failed to initialize workqueue global mutex\n");
		return -RT_ERROR;
	}
	osal_log("init_workqueue success!\n");
	return RT_EOK;
}
INIT_PREV_EXPORT(init_workqueue);

#else

typedef struct {
	struct rt_workqueue *rt_workqueue;
	struct rt_work rt_work;
} osal_workqueue_internal;

static void osal_work_func(struct rt_work *work, void *work_data)
{
	osal_workqueue *osal_work = (osal_workqueue *)work_data;
	if (osal_work->handler) {
		osal_work->handler(osal_work);
	}
}

int osal_workqueue_init(osal_workqueue *work, osal_workqueue_handler handler)
{
	if (work == NULL || handler == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	work->handler = handler;
	osal_workqueue_internal *internal = rt_malloc(sizeof(osal_workqueue_internal));
	if (!internal) {
		osal_log("Failed to allocate memory for internal structure\n");
		return OSAL_FAILURE;
	}

	internal->rt_workqueue = rt_workqueue_create("osal_workqueue", 2048, RT_THREAD_PRIORITY_MAX - 1);
	if (!internal->rt_workqueue) {
		osal_log("Failed to create rt_workqueue\n");
		rt_free(internal);
		return OSAL_FAILURE;
	}

	rt_work_init(&internal->rt_work, osal_work_func, work);
	internal->rt_work.workqueue = internal->rt_workqueue;
	work->work = internal;
	work->queue_flag = 1;

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_workqueue_init);

int osal_workqueue_schedule(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	osal_workqueue_internal *internal = (osal_workqueue_internal *)work->work;
	return rt_workqueue_dowork(internal->rt_workqueue, &internal->rt_work);
}
// EXPORT_SYMBOL(osal_workqueue_schedule);

int osal_workqueue_schedule_highpri(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	osal_workqueue_internal *internal = (osal_workqueue_internal *)work->work;
	if (internal == NULL || internal->rt_workqueue == NULL) {
		osal_log("Invalid rt_work or rt_workqueue\n");
		return OSAL_FAILURE;
	}

	return rt_workqueue_urgent_work(internal->rt_workqueue, &internal->rt_work);
}
// EXPORT_SYMBOL(osal_workqueue_schedule_highpri);

void osal_workqueue_destroy(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return;
	}

	if (work && work->work && work->queue_flag == 1) {
		osal_workqueue_internal *internal = (osal_workqueue_internal *)work->work;
		rt_workqueue_destroy(internal->rt_workqueue);
		rt_free(internal);
		work->work = RT_NULL;
		work->queue_flag = 0;
		osal_log("osal_workqueue_destroy succeeded\n");
	}
}
// EXPORT_SYMBOL(osal_workqueue_destroy);

int osal_workqueue_flush(osal_workqueue *work)
{
	if (work == NULL) {
		osal_log("Invalid parameter\n");
		return OSAL_FAILURE;
	}

	osal_workqueue_internal *internal = (osal_workqueue_internal *)work->work;
	return rt_workqueue_cancel_all_work(internal->rt_workqueue);
}
// EXPORT_SYMBOL(osal_workqueue_flush);

#endif