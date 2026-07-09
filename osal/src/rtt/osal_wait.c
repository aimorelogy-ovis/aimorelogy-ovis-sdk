#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

static long long wait_index = 0;

int osal_wait_init(osal_wait *wait)
{
	if (wait == NULL || wait->wait != NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	char temp_buffer[50];
	const char *const_buffer;

	rt_snprintf(temp_buffer, sizeof(temp_buffer), "wait_%d", wait_index++);

	const_buffer = rt_strdup(temp_buffer);

	wait->wait = (void *)rt_event_create(temp_buffer, RT_IPC_FLAG_FIFO);
	if (wait->wait == NULL) {
		osal_log("wait queue create failed!\n");
		rt_free((void*)const_buffer);
		return OSAL_FAILURE;
	}
	rt_free((void*)const_buffer);
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_wait_init);

int osal_wait_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param)
{
	if (wait == NULL || wait->wait == NULL || func == NULL) {
		osal_log("wait parameter invalid!\n");
		return OSAL_FAILURE;
	}

	while (!func(param)) {
		rt_event_recv_interruptible((rt_event_t)wait->wait, 1,
			RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, NULL);
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_wait_interruptible);

int osal_wait_uninterruptible(osal_wait *wait, osal_wait_condition_func func, const void *param)
{
	if (wait == NULL || wait->wait == NULL || func == NULL) {
		osal_log("wait parameter invalid!\n");
		return OSAL_FAILURE;
	}

	while (!func(param)) {
		rt_event_recv((rt_event_t)wait->wait, 1, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, NULL);
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_wait_uninterruptible);

int osal_wait_timeout_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param, unsigned long ms) {
    if (wait == NULL || wait->wait == NULL || func == NULL) {
        osal_log("wait parameter invalid!\n");
        return OSAL_FAILURE;
    }

    rt_tick_t start_time = rt_tick_get();
    rt_tick_t end_time = start_time + rt_tick_from_millisecond(ms);
    rt_tick_t remaining_time;

    while (!func(param)) {
        remaining_time = end_time - rt_tick_get();
        if (remaining_time <= 0) {
            osal_log("Timeout triggered!\n");
            return 0;
        }

        rt_err_t ret = rt_event_recv_interruptible(
            (rt_event_t)wait->wait,
            1,
            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
            remaining_time,
            NULL
        );

        if (ret != RT_EOK && ret != -RT_ETIMEOUT) {
            osal_log("Wait interrupted by error: %d\n", ret);
            return OSAL_FAILURE;
        }
    }

    rt_tick_t elapsed_ticks  = rt_tick_get() - start_time;
	unsigned long remaining_ms = ms - (elapsed_ticks * 1000) / RT_TICK_PER_SECOND;

    return (remaining_ms > 0) ? (int)remaining_ms : 0;
}

// EXPORT_SYMBOL(osal_wait_timeout_interruptible);

int osal_wait_timeout_uninterruptible(osal_wait *wait, osal_wait_condition_func func,
	const void *param, unsigned long ms)
{
	if (wait == NULL || wait->wait == NULL || func == NULL) {
		osal_log("wait parameter invalid!\n");
		return OSAL_FAILURE;
	}
	rt_tick_t start_time = rt_tick_get();
	rt_tick_t end_time = rt_tick_get() + rt_tick_from_millisecond(ms);
	while (!func(param)) {
		rt_tick_t remaining_time = end_time - rt_tick_get();
		if (remaining_time <= 0) {
			return OSAL_FAILURE;
		}
		rt_err_t ret = rt_event_recv(
            (rt_event_t)wait->wait,
            1,
            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
            remaining_time,
            NULL
        );
		if (ret != RT_EOK && ret != -RT_ETIMEOUT) {
            osal_log("Wait error: %d\n", ret);
            return OSAL_FAILURE;
        }
	}

	rt_tick_t elapsed_ticks = rt_tick_get() - start_time;
    unsigned long remaining_ms = ms - (elapsed_ticks * 1000) / RT_TICK_PER_SECOND;
	return (remaining_ms > 0) ? (int)remaining_ms : 0;
}

// EXPORT_SYMBOL(osal_wait_timeout_uninterruptible);

void osal_wait_wakeup(osal_wait *wait)
{
	if (wait == NULL || wait->wait == NULL) {
		osal_log("wait is NULL!\n");
		return;
	}

	rt_event_send((rt_event_t)wait->wait, 0x01);
}
// EXPORT_SYMBOL(osal_wait_wakeup);

void osal_wait_wakeup_interruptible(osal_wait *wait)
{
	if (wait == NULL || wait->wait == NULL) {
		osal_log("wait is NULL!\n");
		return;
	}

	rt_event_send((rt_event_t)wait->wait, 0x01);
}
// EXPORT_SYMBOL(osal_wait_wakeup_interruptible);

void osal_wait_destroy(osal_wait *wait)
{
	if (wait == NULL || wait->wait == NULL) {
		osal_log("wait is NULL!\n");
		return;
	}

	rt_event_delete((rt_event_t)wait->wait);
	wait->wait = NULL;
}
// EXPORT_SYMBOL(osal_wait_destroy);
