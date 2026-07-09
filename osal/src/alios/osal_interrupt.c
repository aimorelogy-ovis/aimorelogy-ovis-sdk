#include <aos/kernel.h>
#include <drv/irq.h>
#include <aos/debug.h>
#include <string.h>
#include "osal.h"
#include "osal_inner.h"
#include "k_api.h"

#define MAX_IRQ_NAME_LEN 32

#define CONFIG_IRQ_NUM              1023U
#define CONFIG_REAL_IRQ_CNT         105

// csi_dev_t *g_irq_table[CONFIG_IRQ_NUM];
extern csi_dev_t g_irq_dev_table[CONFIG_REAL_IRQ_CNT];

void *osal_irq_get_private_dev(void *param_dev)
{
    if (param_dev == NULL) {
        osal_log("Invalid parameter! caller: %p\n", __builtin_return_address(0));
        return NULL;
    }

    return param_dev;
}

int osal_irq_request(unsigned int irq, osal_irq_handler handler, osal_irq_handler thread_fn,
    const char *name, void *dev)
{
    csi_irq_attach2(irq, handler, &g_irq_dev_table[irq], dev);
    csi_irq_enable(irq);

    return OSAL_SUCCESS;
}

void osal_irq_free(unsigned int irq, void *dev)
{
    if (dev == NULL) {
        osal_log("Invalid parameter! caller: %p\n", __builtin_return_address(0));
        return;
    }

    csi_irq_detach(irq);
    csi_irq_disable(irq);
}

int osal_irq_set_priority(unsigned int irq, unsigned short priority)
{
    csi_irq_priority(irq, priority);
    return OSAL_SUCCESS;
}

void osal_irq_restore(unsigned int irq_status)
{
    csi_irq_restore(irq_status);
}

int osal_in_interrupt(void)
{
    return csi_irq_context();
}

void osal_irq_enable(unsigned int irq)
{
    csi_irq_enable(irq);
}

void osal_irq_disable(unsigned int irq)
{
    csi_irq_disable(irq);
}

/*
 * Tasklet Implementation Notes for AliOS:
 *
 * In Linux, tasklets are a bottom-half mechanism that runs in softirq context,
 * executing soon after the hardware IRQ handler completes. They provide deferred
 * interrupt processing with minimal latency.
 *
 * AliOS does not have a direct softirq equivalent. This implementation uses
 * aos_work_t (work queue) as the closest approximation:
 *
 * - aos_work_t: Executes in a dedicated work thread context, scheduled by the
 *               kernel scheduler. Lower latency than timers, but not as immediate
 *               as true softirq. Can sleep/block if needed.
 *
 * Alternative considered but NOT used:
 * - aos_timer: Would introduce a fixed 1-tick delay, which violates tasklet's
 *              "execute as soon as possible" semantic.
 *
 * Trade-offs:
 * + Work queue provides reasonably quick execution after IRQ
 * + Maintains RTOS thread context (can use mutexes, sleep, etc.)
 * - Slightly higher latency than true softirq (subject to thread scheduling)
 * - Not running in atomic context like Linux tasklets
 */

static void osal_tasklet_work_handler(void *arg)
{
    osal_tasklet *tasklet = (osal_tasklet *)arg;
    if (tasklet && tasklet->handler) {
        tasklet->handler(tasklet->data);
    }
}

int osal_tasklet_init(osal_tasklet *tasklet)
{
    if (tasklet == NULL || tasklet->tasklet != NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (aos_work_init((aos_work_t *)&(tasklet->tasklet), osal_tasklet_work_handler, tasklet, 0)) {
        osal_log("osal_tasklet_init failed!\n");
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

int osal_tasklet_update(osal_tasklet *tasklet)
{
    if (tasklet == NULL || tasklet->tasklet == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

int osal_tasklet_schedule(osal_tasklet *tasklet)
{
    CPSR_ALLOC();
    kwork_t *kwork = NULL;

    if (tasklet == NULL || tasklet->tasklet == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    kwork = (kwork_t *)(tasklet->tasklet);

    RHINO_CRITICAL_ENTER();
    if (g_workqueue_default.work_current == kwork ||
        kwork->work_exit == 1) {
        RHINO_CRITICAL_EXIT();
        return OSAL_SUCCESS;
    }
    RHINO_CRITICAL_EXIT();

    if (aos_work_sched((aos_work_t *)&(tasklet->tasklet))) {
        osal_log("osal_tasklet_schedule failed! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return OSAL_SUCCESS;
}

int osal_tasklet_kill(osal_tasklet *tasklet)
{
    if (tasklet == NULL || tasklet->tasklet == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    aos_work_destroy((aos_work_t *)&(tasklet->tasklet));
    tasklet->tasklet = NULL;
    return OSAL_SUCCESS;
}
