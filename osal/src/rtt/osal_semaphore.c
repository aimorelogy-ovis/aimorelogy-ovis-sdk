#include <rtthread.h>
#include <stdio.h>
#include "osal.h"
#include "osal_inner.h"

static long long sem_index = 0;

int osal_sem_init(osal_semaphore *sem, int val)
{
	if (sem == NULL || sem->sem != NULL || val < 0) {
		osal_log("sem val=%d parameter invalid!\n", val);
		return OSAL_FAILURE;
	}

	char temp_buffer[50];
	const char *const_buffer;

	snprintf(temp_buffer, sizeof(temp_buffer), "sem_%lld", sem_index++);

	const_buffer = rt_strdup(temp_buffer);

	sem->sem = (void *)rt_sem_create(const_buffer, val, RT_IPC_FLAG_PRIO);
	if (sem->sem == NULL) {
		osal_log("init sem[%s] failed\n", temp_buffer);
		rt_free((void *)const_buffer);
		return OSAL_FAILURE;
	}
	rt_free((void *)const_buffer);

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_sem_init);

int osal_sem_down(osal_semaphore *sem)
{
	if (sem == NULL || sem->sem == NULL) {
		osal_log("sem is null!\n");
		return OSAL_FAILURE;
	}

	if (rt_sem_take((rt_sem_t)sem->sem, RT_WAITING_NO) != RT_EOK) {
		osal_log("sem down failed!\n");
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_sem_down);

int osal_sem_down_interruptible(osal_semaphore *sem)
{
	if (sem == NULL || sem->sem == NULL) {
		osal_log("sem is null!\n");
		return OSAL_FAILURE;
	}

	if (rt_sem_take_interruptible((rt_sem_t)sem->sem, RT_WAITING_NO) != RT_EOK) {
		osal_log("sem down failed!\n");
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_sem_down_interruptible);

int osal_sem_trydown(osal_semaphore *sem)
{
	if (sem == NULL || sem->sem == NULL) {
		osal_log("sem is null!\n");
		return OSAL_FAILURE;
	}

	if (rt_sem_trytake((rt_sem_t)sem->sem) != RT_EOK) {
		osal_log("sem down timeout\n");
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_sem_trydown);

int osal_sem_down_timeout(osal_semaphore *sem, unsigned int timeout)
{
	if (sem == NULL || sem->sem == NULL) {
		osal_log("sem is null!\n");
		return OSAL_FAILURE;
	}

	if (rt_sem_take((rt_sem_t)sem->sem, 3000) != RT_EOK) {
		osal_log("sem down timeout\n");
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_sem_down_timeout);

void osal_sem_up(osal_semaphore *sem)
{
	if (sem == NULL || sem->sem == NULL) {
		osal_log("sem is null!\n");
		return;
	}

	if (rt_sem_release((rt_sem_t)sem->sem) != RT_EOK) {
		osal_log("sem down timeout\n");
		return;
	}
}
// EXPORT_SYMBOL(osal_sem_up);

void osal_sem_destroy(osal_semaphore *sem)
{
	if (sem == NULL || sem->sem == NULL) {
		osal_log("sem is null!\n");
		return;
	}

	if (rt_sem_delete((rt_sem_t)sem->sem) != RT_EOK) {
		osal_log("delete sem failed\n");
	}
	sem->sem = NULL;
}
// EXPORT_SYMBOL(osal_sem_destroy);
