#include <aos/kernel.h>

#include "osal.h"
#include "osal_inner.h"

int osal_mutex_init(osal_mutex *mutex)
{
    if (mutex == NULL || mutex->mutex != NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (aos_mutex_new((aos_mutex_t*)&(mutex->mutex))) {
        osal_log("mutex_init error!\n");
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

int osal_mutex_lock(osal_mutex *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (aos_mutex_lock((aos_mutex_t*)&(mutex->mutex), AOS_WAIT_FOREVER)) {
        osal_log("mutex_lock error!\n");
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

int osal_mutex_lock_interruptible(osal_mutex *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (aos_mutex_lock((aos_mutex_t*)&(mutex->mutex), AOS_WAIT_FOREVER)) {
        osal_log("mutex_lock error!\n");
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

int osal_mutex_trylock(osal_mutex *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return FALSE;
    }

    if (aos_mutex_lock((aos_mutex_t*)&(mutex->mutex), AOS_NO_WAIT)) {
        osal_log("mutex_trylock error!\n");
        return FALSE;
    }

    return TRUE;
}

int osal_mutex_lock_timeout(osal_mutex *mutex, unsigned int timeout)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (aos_mutex_lock((aos_mutex_t*)&(mutex->mutex), timeout)) {
        osal_log("mutex_lock_timeout error!\n");
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

void osal_mutex_unlock(osal_mutex *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_mutex_unlock((aos_mutex_t*)(&(mutex->mutex)));
}

void osal_mutex_destroy(osal_mutex *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_mutex_free((aos_mutex_t*)(&(mutex->mutex)));
}

int osal_mutex_is_locked(osal_mutex *mutex)
{
    int ret;

    if (mutex == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return FALSE;
    }

    ret = aos_mutex_lock((aos_mutex_t*)&(mutex->mutex), 0);

    if (ret == 0) {
        aos_mutex_unlock((aos_mutex_t*)&(mutex->mutex));
        return FALSE;
    } else {
        return TRUE;
    }
}