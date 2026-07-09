#include <aos/kernel.h>
#include <string.h>
#include <soc.h>
#include <csi_core.h>
#include "sched.h"
#include "osal.h"
#include "osal_inner.h"

#define PTHREAD_DEFAULT_PRIORITY   30
#define DEFAULT_TASK_STACK_SIZE 8192

struct task_sync_sem {
    void *join_stop_sem;
    void *join_destroy_sem;
};

osal_task *osal_kthread_create(osal_kthread_handler handler, void *data, const char *name, unsigned int stack_size)
{
    osal_task *p = NULL;
    int policy = 0;
    int priority = 0;
    int ret = 0;
    struct task_sync_sem *sync_sem = NULL;
    unsigned int stack_size_tmp = 0;

    sync_sem = (struct task_sync_sem *)malloc(sizeof(struct task_sync_sem));
    if (sync_sem == NULL) {
        osal_log("malloc error!\n");
        return NULL;
    }
    p = (osal_task *)malloc(sizeof(osal_task));
    if (p == NULL) {
        osal_log("malloc error!\n");
        free(sync_sem);
        return NULL;
    }
    (void)memset(sync_sem, 0, sizeof(struct task_sync_sem));
    (void)memset(p, 0, sizeof(osal_task));

    policy = aos_task_sched_policy_get_default();
    priority = PTHREAD_DEFAULT_PRIORITY;
    int pri = aos_sched_get_priority_max(policy) - priority;
    priority = pri < 0 ? 0 : pri;

    stack_size_tmp = stack_size ? stack_size : DEFAULT_TASK_STACK_SIZE;
    ret = aos_task_create((aos_task_t *)&(p->task), name, (void *)handler,
                          data, NULL, stack_size_tmp, priority, 1);

    aos_sem_new((aos_sem_t *)&(sync_sem->join_stop_sem), 0);
    aos_sem_new((aos_sem_t *)&(sync_sem->join_destroy_sem), 0);

    aos_task_ptcb_set((aos_task_t *)&(p->task), sync_sem);
    if (ret != 0) {
        osal_log("kthread_create error!\n");
        free(sync_sem);
        free(p);
        return NULL;
    }
    return p;
}

int osal_kthread_set_priority(osal_task *task, unsigned int priority)
{
    uint8_t old_priority = 0;
    int ret = 0;

    if (task == NULL || task->task == NULL) {
        osal_log("task is invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    ret = aos_task_pri_get((aos_task_t *)&(task->task), &old_priority);

    ret |= aos_task_pri_change((aos_task_t *)&(task->task), priority, &old_priority);
    if (ret != 0) {
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

int osal_kthread_should_stop(void)
{
    struct task_sync_sem *sync_sem = NULL;
    aos_task_t task;
    task = aos_task_self();
    aos_task_ptcb_get(&task, (void **)&sync_sem);

    if (!aos_sem_wait(&(sync_sem->join_stop_sem), 0)) {
        aos_sem_signal(&(sync_sem->join_destroy_sem));
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}

void osal_kthread_exit(void)
{
    struct task_sync_sem *sync_sem = NULL;
    aos_task_t task;
    task = aos_task_self();
    aos_task_ptcb_get(&task, (void **)&sync_sem);

    aos_sem_signal(&(sync_sem->join_destroy_sem));
}

void osal_kthread_destroy(osal_task *task, unsigned int stop_flag)
{
    if (task == NULL || task->task == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    struct task_sync_sem *sync_sem = NULL;

    aos_task_ptcb_get((aos_task_t *)&(task->task), (void **)&sync_sem);
    if (!sync_sem) {
        osal_log("sync_sem gets failed!\n");
        return;
    }
    /* note: When you call the Kthread_stop function, the thread function cannot be finished, otherwise it will oops. */
    if (stop_flag != 0) {
        aos_sem_signal(&(sync_sem->join_stop_sem));
        aos_sem_wait(&(sync_sem->join_destroy_sem), -1);
        aos_task_delete((aos_task_t *)&(task->task));
    }
    aos_sem_free((aos_sem_t *)&(sync_sem->join_stop_sem));
    aos_sem_free((aos_sem_t *)&(sync_sem->join_destroy_sem));
    task->task = NULL;
    free(task);
    free(sync_sem);
}

void osal_yield(void)
{
    aos_task_yield();
}

char *osal_get_current_taskname(void)
{
    return (char *)aos_task_name();
}

int osal_kthread_wakeup_process(osal_task *task)
{
    if (task == NULL) {
        return -1;
    }
    return aos_task_resume((aos_task_t *)&(task->task));
}

unsigned long osal_msleep(unsigned int msecs)
{
    aos_msleep(msecs);
    return 0;
}

void osal_msleep_uninterruptible(unsigned int msecs)
{
    aos_msleep(msecs);
}

static void _10udelay(void)
{
    uint64_t start = csi_clint_get_value();
    uint32_t cnt = (soc_get_coretim_freq() / 1000U / 100U);

    while (1) {
        uint64_t cur = csi_clint_get_value();

        if (start > cur) {
            if ((start - cur) >= cnt) {
                break;
            }
        } else {
            if (cur - start >= cnt) {
                break;
            }
        }
    }
}

void osal_udelay(unsigned int usecs)
{
    usecs /= 10U;

    while (usecs) {
        usecs--;
        _10udelay();
    }
}

static void _500usdelay(void)
{
    uint64_t start = csi_clint_get_value();
    uint64_t cur;
    uint32_t cnt = (soc_get_coretim_freq() / 1000U / 2U);

    while (1) {
        cur = csi_clint_get_value();

        if (start > cur) {
            if ((start - cur) >= cnt) {
                break;
            }
        } else {
            if (cur - start >= cnt) {
                break;
            }
        }
    }
}

void osal_mdelay(unsigned int msecs)
{
    while (msecs) {
        msecs--;
        _500usdelay();
        _500usdelay();
    }
}

void osal_usleep_range(unsigned long min, unsigned long max)
{
    if (min > max) {
        printf("min is greater than max!");
        return;
    }

    unsigned long range = max - min + 1;
    uint64_t time_ms = (osal_get_random_int() % range) + min;

    if (time_ms > 1000) {
        time_ms = time_ms/1000;
    } else {
        time_ms = 1;
    }
    aos_msleep(time_ms);
}