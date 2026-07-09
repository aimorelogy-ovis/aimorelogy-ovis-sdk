#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

#define NS_TO_USEC  1000

static long long hrtimer_index = 0;
static long long timer_index = 0;

int osal_hrtimer_create(osal_hrtimer *hrtimer)
{
	if (hrtimer == NULL || hrtimer->timer != NULL || hrtimer->handler == NULL) {
		osal_log("time handler or hrtimer is null!\n");
		return OSAL_FAILURE;
	}

	char temp_buffer[50];
	const char *const_buffer;

	rt_snprintf(temp_buffer, sizeof(temp_buffer), "hrtimer_%d", hrtimer_index++);

	const_buffer = rt_strdup(temp_buffer);
	hrtimer->timer = (void *)rt_timer_create(const_buffer, (void (*)(void *))hrtimer->handler,
		hrtimer, hrtimer->interval, RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
	if (hrtimer->timer == NULL) {
		osal_log("init timer failed!\n");
		rt_free((void*)const_buffer);
		return OSAL_FAILURE;
	}
	rt_free((void*)const_buffer);
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_hrtimer_create);

int osal_hrtimer_start(osal_hrtimer *hrtimer)
{
	if (hrtimer == NULL || hrtimer->handler == NULL) {
		osal_log("time handler or hrtimer is null!\n");
		return OSAL_FAILURE;
	}

	if (rt_timer_start((rt_timer_t)hrtimer->timer) != RT_EOK) {
		osal_log("start timer failed!\n");
		return OSAL_FAILURE;
	}

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_hrtimer_start);

int osal_hrtimer_destroy(osal_hrtimer *hrtimer)
{
	if (hrtimer == NULL || hrtimer->timer == NULL) {
		return OSAL_FAILURE;
	}

	rt_timer_stop((rt_timer_t)hrtimer->timer);
	if (rt_timer_delete((rt_timer_t)hrtimer->timer) != RT_EOK) {
		osal_log("destroy timer failed!\n");
		return OSAL_FAILURE;
	}

	hrtimer->timer = NULL;

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_hrtimer_destroy);

struct timer_list_info {
	rt_timer_t time_list;
	unsigned long private_data;
};
typedef void (*timer_callback_fun)(rt_timer_t *);

unsigned long osal_timer_get_private_data(const void *sys_data)
{
	struct timer_list_info *list_info = NULL;
	if (sys_data == NULL) {
		return OSAL_SUCCESS;
	}
	list_info = rt_container_of(sys_data, typeof(*list_info), time_list);
	return list_info->private_data;
}
// EXPORT_SYMBOL(osal_timer_get_private_data);

int osal_timer_init(osal_timer *timer)
{
	if (timer == NULL || timer->timer != NULL || timer->handler == NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	if (timer->interval <= 0) {
		osal_log("timer->interval=%d parameter invalid!\n", timer->interval);
		return OSAL_FAILURE;
	}

	char temp_buffer[50];
	const char *const_buffer;

	rt_snprintf(temp_buffer, sizeof(temp_buffer), "timer_%d", timer_index++);

	const_buffer = rt_strdup(temp_buffer);
	timer->timer = (void *)rt_timer_create(const_buffer, (void (*)(void *))timer->handler,
		timer, timer->interval, RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
	if (timer->timer == NULL) {
		osal_log("init timer failed!\n");
		rt_free((void*)const_buffer);
		return OSAL_FAILURE;
	}
	rt_free((void*)const_buffer);
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_timer_init);

int osal_timer_mod(osal_timer *timer, unsigned int interval)
{
	if ((timer == NULL) || (timer->timer == NULL) || interval <= 0) {
		osal_log("interval=%d parameter invalid!\n", interval);
		return OSAL_FAILURE;
	}
	timer->interval = interval;
	if (rt_timer_control((rt_timer_t)timer->timer, RT_TIMER_CTRL_SET_TIME, &interval) != RT_EOK) {
		osal_log("change interval failed!\n");
		return OSAL_FAILURE;
	}
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_timer_mod);

int osal_timer_start(osal_timer *timer)
{
	if ((timer == NULL) || (timer->timer == NULL)) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}
	if (rt_timer_start((rt_timer_t)timer->timer) != RT_EOK) {
		osal_log("start time failed!\n");
		return OSAL_FAILURE;
	}
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_timer_start);

int osal_timer_start_on(osal_timer *timer, unsigned long delay, int cpu)
{
	if ((timer == NULL) || (timer->timer == NULL)) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}
	osal_log("Specifying CPU for timers is not supported!\n");
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_timer_start_on);

int osal_timer_stop(osal_timer *timer)
{
	if ((timer == NULL) || (timer->timer == NULL)) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	rt_timer_stop((rt_timer_t)timer->timer);
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_timer_stop);

int osal_timer_destroy(osal_timer *timer)
{
	if ((timer == NULL) || (timer->timer == NULL)) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	if (rt_timer_delete((rt_timer_t)timer->timer) != RT_EOK) {
		osal_log("destroy timer failed!\n");
		return OSAL_FAILURE;
	}
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_timer_destroy);
int osal_timer_destroy_sync(osal_timer *timer)
{
	return osal_timer_destroy(timer);
}
// EXPORT_SYMBOL(osal_timer_destroy_sync);

//Linux milliseconds:jiffies = 4 : 1
unsigned int osal_jiffies_to_msecs(const unsigned int n)
{
	return (uint64_t)n * 4000 / RT_TICK_PER_SECOND;
}
// EXPORT_SYMBOL(osal_jiffies_to_msecs);

unsigned long long osal_sched_clock(void)
{
	return rt_tick_get();
}
// EXPORT_SYMBOL(osal_sched_clock);

void osal_gettimeofday(osal_timeval *tv)
{
	if (tv == NULL) {
		osal_log("parameter invalid!\n");
		return;
	}

    unsigned long long tick_time = rt_tick_get();
	tv->tv_sec = tick_time / RT_TICK_PER_SECOND;
	tv->tv_usec = (tick_time % RT_TICK_PER_SECOND) * 1000;
}
// EXPORT_SYMBOL(osal_gettimeofday);

long osal_timeval_sub(osal_timeval end, osal_timeval start)
{
	long ts_us;
	long ts_s;

	ts_s = end.tv_sec - start.tv_sec;

	if (end.tv_usec < start.tv_usec) {
		ts_us = 1000000 + end.tv_usec - start.tv_usec;
		ts_s--;
	} else {
		ts_us = end.tv_usec - start.tv_usec;
	}

	ts_us += 1000000 * ts_s;

	return ts_us;
}

//Linux milliseconds:jiffies = 4 : 1
unsigned long osal_msecs_to_jiffies(const unsigned int m)
{
	return ((uint64_t)m * RT_TICK_PER_SECOND) / 4000;
}
// EXPORT_SYMBOL(osal_msecs_to_jiffies);

unsigned long long osal_get_jiffies(void)
{
	return rt_tick_get();
}
// EXPORT_SYMBOL(osal_get_jiffies);
