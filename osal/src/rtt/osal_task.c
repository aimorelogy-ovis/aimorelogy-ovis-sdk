#include <stdio.h>
#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

struct task_sync_sem {
	rt_sem_t join_stop_sem;
	rt_sem_t join_destroy_sem;
};

static long long stop_sem_index = 0;
static long long destroy_sem_index = 0;

osal_task *osal_kthread_create(osal_kthread_handler handler, void *data, const char *name, unsigned int stack_size)
{
	osal_task *p = NULL;
	struct task_sync_sem *thread_sync_sem = NULL;
	rt_thread_t thread;

	if (handler == NULL) {
		osal_log("task handler is null!\n");
		return NULL;
	}

	p = (osal_task *)rt_malloc(sizeof(osal_task));
	if (p == NULL) {
		osal_log("osal_task kmalloc error!\n");
		return NULL;
	}

	rt_memset(p, 0, sizeof(osal_task));
	thread_sync_sem = (struct task_sync_sem *)rt_malloc(sizeof(struct task_sync_sem));
	if (thread_sync_sem == NULL) {
		osal_log("thread_sync_sem kmalloc error!\n");
		rt_free(p);
		return NULL;
	}

	char temp_buffer[50];
	const char *const_buffer;

	snprintf(temp_buffer, sizeof(temp_buffer), "stop_sem_%lld", stop_sem_index++);

	const_buffer = rt_strdup(temp_buffer);

	thread_sync_sem->join_stop_sem = (void *)rt_sem_create(const_buffer, 0, RT_IPC_FLAG_PRIO);

	if (thread_sync_sem->join_stop_sem == NULL) {
		osal_log("init stop_sem[%s] failed\n", temp_buffer);
		rt_free((void *)const_buffer);
		rt_free(thread_sync_sem);
		rt_free(p);
		return NULL;
	}
	rt_free((void *)const_buffer);

	snprintf(temp_buffer, sizeof(temp_buffer), "destroy_sem_%lld", destroy_sem_index++);

	const_buffer = rt_strdup(temp_buffer);

	thread_sync_sem->join_destroy_sem = (void *)rt_sem_create(const_buffer, 0, RT_IPC_FLAG_PRIO);
	if (thread_sync_sem->join_destroy_sem == NULL) {
		osal_log("init destroy_sem[%s] failed\n", temp_buffer);
		rt_sem_delete(thread_sync_sem->join_stop_sem);
		rt_free((void *)const_buffer);
		rt_free(thread_sync_sem);
		rt_free(p);
		return NULL;
	}
	rt_free((void *)const_buffer);

	thread = rt_thread_create(name, (void (*)(void *))handler, data, stack_size, RT_THREAD_PRIORITY_MAX / 2, 20);
	if (thread == NULL) {
		osal_log("thread create error for thread: %s!\n", name);
		rt_sem_delete(thread_sync_sem->join_stop_sem);
		rt_sem_delete(thread_sync_sem->join_destroy_sem);
		rt_free(thread_sync_sem);
		rt_free(p);
		return NULL;
	}
	thread->user_data = (rt_ubase_t)thread_sync_sem;
	if (rt_thread_startup(thread) != RT_EOK) {
		osal_log("thread startup error for thread: %s!\n", name);
		rt_sem_delete(thread_sync_sem->join_stop_sem);
		rt_sem_delete(thread_sync_sem->join_destroy_sem);
		rt_free(thread_sync_sem);
		rt_free(p);
		return NULL;
	}

	p->task = thread;
	return p;
}
// EXPORT_SYMBOL(osal_kthread_create);

int osal_kthread_set_priority(osal_task *task, unsigned int priority)
{
	if (task == NULL) {
		osal_log("task is null!\n");
		return OSAL_FAILURE;
	}

	uint8_t new_priority = (uint8_t)priority;
	if (rt_thread_control((rt_thread_t)(task->task), RT_THREAD_CTRL_CHANGE_PRIORITY, &new_priority) != RT_EOK) {
		osal_log("set priority failed!\n");
		return OSAL_FAILURE;
	}
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_kthread_set_priority);

void osal_kthread_set_affinity(osal_task *task, int cpu_mask)
{
	if (task == NULL) {
		return;
	}

	if (cpu_mask == 0) {
		return;
	}

	uint8_t new_cpu_mask = (uint8_t)cpu_mask;
	if (rt_thread_control((rt_thread_t)(task->task), RT_THREAD_CTRL_CHANGE_PRIORITY, &new_cpu_mask) != RT_EOK) {
		osal_log("set priority failed!\n");
		return;
	}
}
// EXPORT_SYMBOL(osal_kthread_set_affinity);

int osal_kthread_should_stop(void)
{
	rt_thread_t thread = rt_thread_self();
	struct task_sync_sem *thread_sync_sem = (struct task_sync_sem *)thread->user_data;

	if (thread->user_data == 0) {
		osal_log("thread sem is null, check create api!\n");
		return OSAL_SUCCESS;
	}

	if (thread_sync_sem->join_stop_sem == NULL) {
		osal_log("thread stop sem is null!\n");
		return OSAL_SUCCESS;
	}

	if (thread_sync_sem->join_destroy_sem == NULL) {
		osal_log("thread destroy sem is null!\n");
		return OSAL_SUCCESS;
	}

	if (rt_sem_trytake(thread_sync_sem->join_stop_sem) != RT_EOK) {
		return OSAL_SUCCESS;
	}

	rt_sem_release(thread_sync_sem->join_destroy_sem);

	return OSAL_FAILURE;
}
// EXPORT_SYMBOL(osal_kthread_should_stop);

static int osal_kthread_destroy_thread(rt_thread_t thread)
{
	if (((thread->sched_thread_ctx.stat & RT_THREAD_STAT_MASK) != RT_THREAD_CLOSE)
		&& ((thread->sched_thread_ctx.stat & RT_THREAD_STAT_MASK) != RT_THREAD_INIT)) {
		if (rt_thread_delete(thread) != RT_EOK) {
			osal_log("destroy thread failed!\n");
			return OSAL_FAILURE;
		}
	} else {
		osal_log("thread is already suspended or exited\n");
	}

	return OSAL_SUCCESS;
}

void osal_kthread_exit(void)
{
	return;
}

void osal_kthread_destroy(osal_task *task, unsigned int stop_flag)
{
	if (task == NULL || task->task == NULL) {
		osal_log("thread is null!\n");
		return;
	}

	if (stop_flag == 0) {
		osal_log("flag is [0], no need to destroy.\n");
		return;
	}

	rt_thread_t thread = (rt_thread_t)(task->task);
	struct task_sync_sem *thread_sync_sem = (struct task_sync_sem *)(thread->user_data);

	if (thread->user_data == 0) {
		osal_log("thread sem is null, will destroy thread!\n");
		if (osal_kthread_destroy_thread(thread) != OSAL_SUCCESS) {
			osal_log("destroy thread failed!\n");
			return;
		}
		return;
	}

	if (thread_sync_sem->join_stop_sem == NULL) {
		osal_log("thread stop sem is null!\n");
		return;
	}

	if (thread_sync_sem->join_destroy_sem == NULL) {
		osal_log("thread destroy sem is null!\n");
		return;
	}

	rt_sem_release(thread_sync_sem->join_stop_sem);

	if (rt_sem_take(thread_sync_sem->join_destroy_sem, 3000) != RT_EOK) {
		osal_log("destroy signal take timeout 3000!\n");
		return;
	}

	if (osal_kthread_destroy_thread(thread) != OSAL_SUCCESS) {
		osal_log("destroy thread failed!\n");
		return;
	}

	rt_sem_delete(thread_sync_sem->join_stop_sem);
	rt_sem_delete(thread_sync_sem->join_destroy_sem);
	rt_free(thread_sync_sem);

	task->task = NULL;
	rt_free(task);
}
// EXPORT_SYMBOL(osal_kthread_destroy);

void osal_schedule(void)
{
	rt_schedule();
}
// EXPORT_SYMBOL(osal_schedule);

void osal_kthread_schedule(unsigned int sleep_ns)
{
	rt_thread_delay((rt_tick_t)(RT_TICK_PER_SECOND * sleep_ns / 1000000000));
}
// EXPORT_SYMBOL(osal_kthread_schedule);

void osal_kthread_set_uninterrupt(void)
{
	//RTT do not need this function
}
// EXPORT_SYMBOL(osal_kthread_set_uninterrupt);

void osal_kthread_set_running(void)
{
	//RTT do not need this function
}
// EXPORT_SYMBOL(osal_kthread_set_running);

void osal_cond_resched(void)
{
	//RTT do not need this function
}
// EXPORT_SYMBOL(osal_cond_resched);

void osal_yield(void)
{
	rt_thread_yield();
}
// EXPORT_SYMBOL(osal_yield);

void osal_kneon_begin(void)
{
	// RT-Thread does not support NEON
}
// EXPORT_SYMBOL(osal_kneon_begin);

void osal_kneon_end(void)
{
	// RT-Thread does not support NEON
}
// EXPORT_SYMBOL(osal_kneon_end);

long osal_get_current_pid(void)
{
	// RT-Thread only have tid
#ifdef RT_USING_SMART
	return rt_thread_self()->tid;
#else
	return 0;
#endif
}
// EXPORT_SYMBOL(osal_get_current_pid);

long osal_get_current_tid(void)
{
#ifdef RT_USING_SMART
	return rt_thread_self()->tid;
#else
	return 0;
#endif
}
// EXPORT_SYMBOL(osal_get_current_tid);

int osal_get_current_tgid(void)
{
	// RT-Thread only have tid
#ifdef RT_USING_SMART
	return rt_thread_self()->tid;
#else
	return 0;
#endif
}
// EXPORT_SYMBOL(osal_get_current_tgid);

char *osal_get_current_taskname(void)
{
	static char name[RT_NAME_MAX];
	rt_thread_t current_thread = rt_thread_self();

	if (rt_object_get_name((rt_object_t)current_thread, name, sizeof(name)) == RT_EOK) {
		return name;
	} else {
		return "Unknown";
	}
}
// EXPORT_SYMBOL(osal_get_current_taskname);

int osal_kthread_wakeup_process(osal_task *task)
{
	if (task == NULL) {
		return -1;
	}

	rt_thread_t thread = (rt_thread_t)(task->task);

	if (rt_thread_resume((rt_thread_t)(thread)) != RT_EOK) {
		osal_log("wake thread failed!\n");
		return 0;
	}
	return 1;
}
// EXPORT_SYMBOL(osal_kthread_wakeup_process);

void osal_kthread_bind(osal_task *task, unsigned int cpu)
{
	if (task == NULL || task->task == NULL) {
		osal_log("thread is null!\n");
		return;
	}

	rt_thread_t thread = (rt_thread_t)(task->task);

	if (rt_thread_control(thread, RT_THREAD_CTRL_BIND_CPU, (void *)(size_t)cpu) != RT_EOK) {
		osal_log("bind thread to CPU %u failed!\n", cpu);
	}
}
// EXPORT_SYMBOL(osal_kthread_bind);

unsigned long osal_msleep(unsigned int msecs)
{
	rt_thread_mdelay(msecs);
	return 0;
}
// EXPORT_SYMBOL(osal_msleep);

void osal_msleep_uninterruptible(unsigned int msecs)
{
	rt_thread_mdelay(msecs);
}
// EXPORT_SYMBOL(osal_msleep_uninterruptible);

void osal_udelay(unsigned int usecs)
{
	rt_thread_delay(RT_TICK_PER_SECOND * usecs / 1000000);
}
// EXPORT_SYMBOL(osal_udelay);

void osal_mdelay(unsigned int msecs)
{
	rt_thread_mdelay(msecs);
}
// EXPORT_SYMBOL(osal_mdelay);

void osal_usleep_range(unsigned long min, unsigned long max)
{
    RT_UNUSED(max);
	rt_thread_delay(RT_TICK_PER_SECOND * min / 1000000);
}
// EXPORT_SYMBOL(osal_usleep_range);