#include <rtthread.h>
#include <stdio.h>
#include "osal.h"
#include "osal_inner.h"

static long long mutex_index = 0;

int osal_mutex_init(osal_mutex *mutex)
{

	if (mutex == NULL || mutex->mutex != NULL) {
		osal_log("mutex parameter invalid!\n");
		return OSAL_FAILURE;
	}

	char temp_buffer[50];
	snprintf(temp_buffer, sizeof(temp_buffer), "mutex_%lld", mutex_index++);

	mutex->mutex = (void *)(rt_mutex_create(temp_buffer, RT_IPC_FLAG_PRIO));
	if (mutex->mutex == NULL) {
		osal_log("init mutex[%s] failed\n", temp_buffer);
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_mutex_init);

int osal_mutex_lock(osal_mutex *mutex)
{

	if (mutex == NULL || mutex->mutex == NULL) {
		osal_log("mutex is null!\n");
		return OSAL_FAILURE;
	}

	if (rt_mutex_take((rt_mutex_t)mutex->mutex, 3000) != RT_EOK) {
		osal_log("lock mutex timeout\n");
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_mutex_lock);

int osal_mutex_lock_interruptible(osal_mutex *mutex)
{

	if (mutex == NULL || mutex->mutex == NULL) {
		osal_log("mutex is null!\n");
		return OSAL_FAILURE;
	}

	if (rt_mutex_take_interruptible((rt_mutex_t)mutex->mutex, 3000) != RT_EOK) {
		osal_log("lock int mutex timeout\n");
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_mutex_lock_interruptible);

/*
 * Returns 1 if the mutex has been acquired successfully, and 0 on contention.
 */
int osal_mutex_trylock(osal_mutex *mutex)
{

	if (mutex == NULL || mutex->mutex == NULL) {
		osal_log("mutex is null!\n");
		return FALSE;
	}

	if (rt_mutex_trytake((rt_mutex_t)mutex->mutex) != RT_EOK) {
		return FALSE;
	} else {
		return TRUE;
	}
}
// EXPORT_SYMBOL(osal_mutex_trylock);

void osal_mutex_unlock(osal_mutex *mutex)
{

	if (mutex == NULL || mutex->mutex == NULL) {
		osal_log("mutex is null!\n");
		return;
	}

	if (rt_mutex_release((rt_mutex_t)mutex->mutex) != RT_EOK) {
		osal_log("unlock mutex failed\n");
	}
}
// EXPORT_SYMBOL(osal_mutex_unlock);

void osal_mutex_destroy(osal_mutex *mutex)
{

	if (mutex == NULL || mutex->mutex == NULL) {
		osal_log("mutex is null!\n");
		return;
	}

	if (rt_mutex_delete((rt_mutex_t)mutex->mutex) != RT_EOK) {
		osal_log("delete mutex failed\n");
	}
	mutex->mutex = NULL;
}
// EXPORT_SYMBOL(osal_mutex_destroy);

int osal_mutex_is_locked(osal_mutex *mutex)
{

	if (mutex == NULL || mutex->mutex == NULL) {
		osal_log("mutex is null!\n");
		return FALSE;
	}

	if (rt_mutex_trytake((rt_mutex_t)mutex->mutex) == RT_EOK) {
		rt_mutex_release((rt_mutex_t)mutex->mutex);
		return FALSE;
	} else {
		return TRUE;
	}
}
// EXPORT_SYMBOL(osal_mutex_is_locked);