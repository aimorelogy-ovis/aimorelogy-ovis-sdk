#include <aos/kernel.h>
#include <aos/debug.h>

#include "osal.h"
#include "osal_inner.h"

int osal_workqueue_init(osal_workqueue *work, osal_workqueue_handler handler)
{
    if (work == NULL || work->work != NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (aos_work_init((aos_work_t *)&(work->work), (void (*)(void *))handler, work, 0)) {
        osal_log("osal_workqueue_init failed!\n");
        return OSAL_FAILURE;
    }

    work->handler = handler;

    return OSAL_SUCCESS;
}

int osal_workqueue_schedule(osal_workqueue *work)
{
    if (work == NULL || work->work == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return FALSE;
    }

    if (aos_work_sched((aos_work_t *)&(work->work))) {
        osal_log("osal_workqueue_schedule failed!\n");
        return OSAL_FAILURE;
    }

    return TRUE;
}

int osal_workqueue_schedule_highpri(osal_workqueue *work)
{
    return osal_workqueue_schedule(work);
}

void osal_workqueue_destroy(osal_workqueue *work)
{
    if (work == NULL || work->work == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    aos_work_destroy((aos_work_t *)&(work->work));
    work->work = NULL;
}

