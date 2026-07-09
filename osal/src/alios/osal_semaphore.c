#include <aos/kernel.h>
#include <semaphore.h>
#include <aos/errno.h>
#include "osal.h"
#include "osal_inner.h"

int osal_sem_init(osal_semaphore *sem, int val)
{
    if (sem == NULL || sem->sem != NULL || val < 0) {
        osal_log("val=%d parameter invalid! caller: %p\n", val, __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return aos_sem_new((aos_sem_t *)&(sem->sem), val);
}

int osal_sem_down(osal_semaphore *sem)
{
    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return aos_sem_wait((aos_sem_t *)&(sem->sem), -1);
}

int osal_sem_down_interruptible(osal_semaphore *sem)
{

    if (sem == NULL || sem->sem == NULL) {
        osal_log("sem is null! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return aos_sem_wait((aos_sem_t *)&(sem->sem), -1);
}

int osal_sem_trydown(osal_semaphore *sem)
{
    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return aos_sem_wait((aos_sem_t *)&(sem->sem), 0);
}

int osal_sem_down_timeout(osal_semaphore *sem, unsigned int timeout)
{
    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

	return aos_sem_wait((aos_sem_t *)&(sem->sem), timeout);
}

void osal_sem_up(osal_semaphore *sem)
{
    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_sem_signal((aos_sem_t *)&(sem->sem));
}

void osal_sem_destroy(osal_semaphore *sem)
{
    if (sem == NULL || sem->sem == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_sem_free((aos_sem_t *)&(sem->sem));
}