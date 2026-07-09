#include <stdio.h>
#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

static uint32_t event_counter = 0;

int osal_completion_init(osal_completion *com)
{
	if (com == NULL || com->completion != NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}
	char event_name[RT_NAME_MAX];
	snprintf(event_name, sizeof(event_name), "completion_event_%d", event_counter++);

	com->completion = rt_event_create(event_name, RT_IPC_FLAG_FIFO);
	if (com->completion == NULL) {
		osal_log("Failed to create event\n");
		return OSAL_FAILURE;
	}
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_completion_init);

void osal_completion_reinit(osal_completion *com)
{
	if (com == NULL || com->completion == NULL) {
		osal_log("Invalid parameter\n");
		return;
	}

	rt_event_delete(com->completion);

	char event_name[RT_NAME_MAX];
	snprintf(event_name, sizeof(event_name), "completion_event_%d", event_counter++);

	com->completion = rt_event_create(event_name, RT_IPC_FLAG_FIFO);
	if (com->completion == NULL) {
		osal_log("Failed to create event\n");
	}
}
// EXPORT_SYMBOL(osal_completion_reinit);

void osal_complete(osal_completion *com)
{
	if (com == NULL || com->completion == NULL) {
		osal_log("Invalid parameter\n");
		return;
	}

	rt_event_send((rt_event_t)com->completion, 0x01);
}
// EXPORT_SYMBOL(osal_complete);

void osal_wait_for_completion(osal_completion *com)
{
	if (com == NULL || com->completion == NULL) {
		osal_log("Invalid parameter\n");
		return;
	}

	uint32_t recved;
	rt_event_recv((rt_event_t)com->completion, 0x01, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &recved);
}
// EXPORT_SYMBOL(osal_wait_for_completion);

/*
 * Return: 0 if timed out, and positive (at least 1, or number of jiffies left
 * till timeout) if completed.
 */
unsigned long osal_wait_for_completion_timeout(osal_completion *com, unsigned long timeout)
{
	if (com == NULL || com->completion == NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}
	rt_uint32_t recved;
	rt_tick_t ticks = rt_tick_from_millisecond(timeout);
	rt_err_t result = rt_event_recv((rt_event_t)com->completion, 0x01,
						RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, ticks, &recved);
	if (result != RT_EOK) {
		osal_log("wait completion fail with error: %ld\n", result);
		return OSAL_FAILURE;
	}
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_wait_for_completion_timeout);

void osal_complete_all(osal_completion *com)
{
	if (com == NULL || com->completion == NULL) {
		osal_log("parameter invalid!\n");
		return;
	}
	rt_event_send((rt_event_t)com->completion, 0xffffffff);
}
// EXPORT_SYMBOL(osal_complete_all);

void osal_complete_destory(osal_completion *com)
{
	if (com == NULL || com->completion == NULL) {
		osal_log("parameter invalid!\n");
		return;
	}
	rt_event_delete((rt_event_t)com->completion);
	com->completion = NULL;
}
// EXPORT_SYMBOL(osal_complete_destory);
