#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

static rt_list_t dw_list = RT_LIST_OBJECT_INIT(dw_list);
static rt_spinlock_t lock;
static int timer_counter = 0;

typedef struct dw_node_ {
	osal_delayedwork *delayedwork;
	rt_list_t node;
	rt_timer_t timer;
	rt_bool_t detached;
} dw_node;

static void delayedwork_timer_callback(void *parameter)
{
	dw_node *node = (dw_node *)parameter;
	osal_delayedwork *work = node->delayedwork;

	if (work->handler) {
		work->handler(work);
	}

	rt_spin_lock(&lock);
	rt_list_remove(&node->node);
	rt_spin_unlock(&lock);

	rt_timer_detach(node->timer);
	node->detached = TRUE;
	work->work = NULL;
}

int osal_delayedwork_init(osal_delayedwork *work, osal_delayedwork_handler handler)
{
	if (work == NULL || handler == NULL) {
		osal_log("Delayed work or handler is NULL\n");
		return OSAL_FAILURE;
	}

	work->handler = handler;
	work->work = NULL;

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_delayedwork_init);

void osal_delayedwork_destroy(osal_delayedwork *work)
{
	if (work == NULL) {
		osal_log("Delayed work is NULL\n");
		return;
	}

	// rt_spin_lock(&lock);

	rt_list_t *list_node;
	rt_list_t *next;

	rt_list_for_each_safe(list_node, next, &dw_list) {
		dw_node *dw_node = rt_list_entry(list_node, struct dw_node_, node);
		if (dw_node->delayedwork == work) {
			rt_list_remove(list_node);
			if (dw_node->detached) {
				rt_timer_delete(dw_node->timer);
			}
			rt_free(dw_node);
		}
	}

	// rt_spin_unlock(&lock);

	work->work = NULL;
}
// EXPORT_SYMBOL(osal_delayedwork_destroy);

int osal_delayedwork_schedule(osal_delayedwork *work, int timeout)
{
	if (work == NULL || timeout < 0) {
		osal_log("Invalid parameters\n");
		return OSAL_FAILURE;
	}

	dw_node *node = rt_malloc(sizeof(dw_node));
	if (node == NULL) {
		osal_log("Memory allocation failed\n");
		return OSAL_FAILURE;
	}

	node->timer = rt_malloc(sizeof(struct rt_timer));
	if (node->timer == NULL) {
		osal_log("Timer memory allocation failed\n");
		rt_free(node);
		return OSAL_FAILURE;
	}

	node->delayedwork = work;
	node->detached = FALSE; // 初始化 detached 标志
	rt_list_init(&node->node);

	char timer_name[32];
	rt_snprintf(timer_name, sizeof(timer_name), "dw_timer_%d", timer_counter++);

	rt_timer_init(node->timer, timer_name, delayedwork_timer_callback, node, timeout, RT_TIMER_FLAG_ONE_SHOT);
	if (node->timer == NULL) {
		osal_log("Timer initialization failed\n");
		rt_free(node);
		return OSAL_FAILURE;
	}

	work->work = node;

	rt_spin_lock(&lock);
	rt_list_insert_after(&dw_list, &node->node);
	rt_spin_unlock(&lock);

	if (rt_timer_start(node->timer) != RT_EOK) {
		osal_log("Timer start failed\n");
		rt_spin_lock(&lock);
		rt_list_remove(&node->node);
		rt_spin_unlock(&lock);
		rt_timer_detach(node->timer);
		rt_free(node->timer);
		rt_free(node);
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_delayedwork_schedule);

int osal_delayedwork_cancel_sync(osal_delayedwork *work)
{
	if (work == NULL) {
		osal_log("Delayed work is NULL\n");
		return OSAL_FAILURE;
	}

	rt_spin_lock(&lock);

	dw_node *node = (dw_node *)work->work;
	if (node == NULL) {
		rt_spin_unlock(&lock);
		osal_log("Delayed work node is NULL\n");
		return OSAL_FAILURE;
	}

	if (!rt_list_isempty(&node->node)) {
		rt_list_remove(&node->node);
	}
	rt_spin_unlock(&lock);

	if (node->timer == NULL) {
		osal_log("Timer is NULL\n");
		work->work = NULL;
		return OSAL_FAILURE;
	}

	if (node->detached) {
		osal_log("Timer already detached\n");
		work->work = NULL;
		return OSAL_FAILURE;
	}

	if (rt_timer_stop(node->timer) != RT_EOK) {
		osal_log("Failed to stop timer\n");
	}

	rt_timer_detach(node->timer);
	node->detached = TRUE;
	work->work = NULL;

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_delayedwork_cancel_sync);

/**
 * this inner implementation is meaningless in single-core cpu
 * in case of we use multi-core cpu, we reserve it
 */
int init_delayed_work(void)
{
	rt_list_init(&dw_list);
	rt_spin_lock_init(&lock);
	osal_log("init_delayed_work success!\n");
	return RT_EOK;
}
INIT_PREV_EXPORT(init_delayed_work);