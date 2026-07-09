#include <aos/kernel.h>
#include <aos/debug.h>
#include "osal.h"
#include "osal_inner.h"

typedef struct osal_aos_wait {
    aos_mutex_t mutex;
    aos_event_t wait;
} osal_aos_wait;

#define RHINO_BLK_TIMEOUT -110

int osal_wait_init(osal_wait *wait)
{
    if (wait == NULL || wait->wait != NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    osal_aos_wait * aos_wait = NULL;

    aos_wait = (osal_aos_wait *)malloc(sizeof(osal_aos_wait));

    if (aos_mutex_new(&aos_wait->mutex) != 0) {
        printf("Failed to create mutex for waitqueue.\n");
        return OSAL_FAILURE;
    }

    if (aos_event_new(&aos_wait->wait, 0) != 0) {
        printf("Failed to create event.\n");
        aos_mutex_free(&aos_wait->mutex);
        return OSAL_FAILURE;
    }

    wait->wait = aos_wait;

    return OSAL_SUCCESS;
}

int osal_wait_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param)
{
    int ret = OSAL_FAILURE;
    osal_aos_wait * aos_wait = NULL;
    unsigned int actl_flags;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (func == NULL) {
        return OSAL_SUCCESS;
    }

    aos_wait = (osal_aos_wait *)(wait->wait);

    aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
    if (func(param)) {
        aos_mutex_unlock(&aos_wait->mutex);
        return OSAL_SUCCESS;
    }

    aos_mutex_unlock(&aos_wait->mutex);

    while(1) {
        ret = aos_event_get(&aos_wait->wait, 0x1, AOS_EVENT_OR_CLEAR, &actl_flags, AOS_WAIT_FOREVER);
        if (ret == 0) {
            aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
            if (func(param)) {
                aos_mutex_unlock(&aos_wait->mutex);
                return OSAL_SUCCESS;
            }
            aos_mutex_unlock(&aos_wait->mutex);
        } else {
            return OSAL_FAILURE;
        }
    }

    return OSAL_SUCCESS;
}

int osal_wait_uninterruptible(osal_wait *wait, osal_wait_condition_func func, const void *param)
{
    int ret = OSAL_FAILURE;
    osal_aos_wait * aos_wait = NULL;
    unsigned int actl_flags;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (func == NULL) {
        return OSAL_SUCCESS;
    }

    aos_wait = (osal_aos_wait *)(wait->wait);

    aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
    if (func(param)) {
        aos_mutex_unlock(&aos_wait->mutex);
        return OSAL_SUCCESS;
    }

    aos_mutex_unlock(&aos_wait->mutex);

    while(1) {
        ret = aos_event_get(&aos_wait->wait, 0x1, AOS_EVENT_OR_CLEAR, &actl_flags, AOS_WAIT_FOREVER);
        if (ret == 0) {
            aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
            if (func(param)) {
                aos_mutex_unlock(&aos_wait->mutex);
                return OSAL_SUCCESS;
            }
            aos_mutex_unlock(&aos_wait->mutex);
        } else {
            return OSAL_FAILURE;
        }
    }

    return OSAL_SUCCESS;
}

int osal_wait_timeout_interruptible(osal_wait *wait, osal_wait_condition_func func, const void *param, unsigned long ms)
{
    osal_aos_wait *aos_wait = NULL;
    int ret;
    unsigned long start_ms;
    unsigned long elapsed;
    unsigned int actl_flags;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return -EINVAL;
    }

    aos_wait = (osal_aos_wait *)(wait->wait);
    start_ms = aos_now_ms();

    if (func == NULL) {
        ret = aos_event_get(&aos_wait->wait, 0x1, AOS_EVENT_OR_CLEAR, &actl_flags, ms);
        if (ret == 0) {
            elapsed = aos_now_ms() - start_ms;
            return (elapsed >= ms) ? 1 : (int)(ms - elapsed);
        }
        return 0;
    }

    aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
    if (func(param)) {
        aos_mutex_unlock(&aos_wait->mutex);
        return ms ? (int)ms : 0;
    }
    aos_mutex_unlock(&aos_wait->mutex);

    elapsed = aos_now_ms() - start_ms;
    while(elapsed < ms) {
        ret = aos_event_get(&aos_wait->wait, 0x1, AOS_EVENT_OR_CLEAR, &actl_flags, ms);
        if (ret == 0) {
            aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
            if (func(param)) {
                elapsed = aos_now_ms() - start_ms;
                if (elapsed >= ms) {
                    aos_mutex_unlock(&aos_wait->mutex);
                    return 0;
                } else {
                    aos_mutex_unlock(&aos_wait->mutex);
                    return (int)(ms - elapsed);
                }
            } else {
                elapsed = aos_now_ms() - start_ms;
            }
            aos_mutex_unlock(&aos_wait->mutex);
        } else {
            return 0;
        }
    }

    return 0;
}

int osal_wait_timeout_uninterruptible(osal_wait *wait, osal_wait_condition_func func,
    const void *param, unsigned long ms)
{
    osal_aos_wait *aos_wait = NULL;
    int ret;
    unsigned long start_ms;
    unsigned long elapsed;
    unsigned int actl_flags;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return -EINVAL;
    }

    aos_wait = (osal_aos_wait *)(wait->wait);
    start_ms = aos_now_ms();

    if (func == NULL) {
        ret = aos_event_get(&aos_wait->wait, 0x1, AOS_EVENT_OR_CLEAR, &actl_flags, ms);
        if (ret == 0) {
            elapsed = aos_now_ms() - start_ms;
            return (elapsed >= ms) ? 1 : (int)(ms - elapsed);
        }
        return 0;
    }

    aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
    if (func(param)) {
        aos_mutex_unlock(&aos_wait->mutex);
        return ms ? (int)ms : 0;
    }
    aos_mutex_unlock(&aos_wait->mutex);

    elapsed = aos_now_ms() - start_ms;
    while(elapsed < ms) {
        ret = aos_event_get(&aos_wait->wait, 0x1, AOS_EVENT_OR_CLEAR, &actl_flags, ms);
        if (ret == 0) {
            aos_mutex_lock(&aos_wait->mutex, AOS_WAIT_FOREVER);
            if (func(param)) {
                elapsed = aos_now_ms() - start_ms;
                if (elapsed >= ms) {
                    aos_mutex_unlock(&aos_wait->mutex);
                    return 0;
                } else {
                    aos_mutex_unlock(&aos_wait->mutex);
                    return (int)(ms - elapsed);
                }
            } else {
                elapsed = aos_now_ms() - start_ms;
            }
            aos_mutex_unlock(&aos_wait->mutex);
        } else {
            return 0;
        }
    }

    return 0;
}

void osal_wait_wakeup(osal_wait *wait)
{
    osal_aos_wait * aos_wait = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("wait is NULL! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_wait = (osal_aos_wait *)(wait->wait);

    aos_event_set(&aos_wait->wait, 0x1, AOS_EVENT_OR);
}

void osal_wait_wakeup_interruptible(osal_wait *wait)
{
    osal_aos_wait * aos_wait = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("wait is NULL! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_wait = (osal_aos_wait *)(wait->wait);

    aos_event_set(&aos_wait->wait, 0x1, AOS_EVENT_OR);
}

void osal_wait_destroy(osal_wait *wait)
{
    osal_aos_wait * aos_wait = NULL;

    if (wait == NULL || wait->wait == NULL) {
        osal_log("wait is NULL! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_wait = (osal_aos_wait *)(wait->wait);

    aos_event_free(&aos_wait->wait);
    aos_mutex_free(&aos_wait->mutex);
    free(aos_wait);
    wait->wait = NULL;
}