#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#include <linux/sched/clock.h>
#include <linux/hrtimer.h>
#else
#include <linux/sched.h>
#endif
#include "osal.h"
#include "osal_inner.h"

#define NS_TO_USEC  1000

struct osal_hrtimer_data {
    struct hrtimer linux_timer;
    osal_hrtimer *hrt;
};

static enum hrtimer_restart osal_hrtimer_callback(struct hrtimer *timer)
{
    struct osal_hrtimer_data *data = container_of(timer, struct osal_hrtimer_data, linux_timer);
    osal_hrtimer_restart ret = OSAL_HRTIMER_NORESTART;

    if (data->hrt && data->hrt->handler) {
        ret = data->hrt->handler(data->hrt);
        // Use the correct OSAL enumeration value
        if (ret == OSAL_HRTIMER_RESTART) {
            ktime_t ktime = ktime_set(0, data->hrt->interval * NSEC_PER_MSEC);
            hrtimer_forward_now(&data->linux_timer, ktime);
            return HRTIMER_RESTART;
        }
    }

    return HRTIMER_NORESTART;
}

int osal_hrtimer_create(osal_hrtimer *hrtimer)
{
    struct osal_hrtimer_data *data;

    if (hrtimer == NULL || hrtimer->timer != NULL || hrtimer->handler == NULL) {
        osal_log("time handler or hrtimer is null!\n");
        return OSAL_FAILURE;
    }

    data = kmalloc(sizeof(struct osal_hrtimer_data), GFP_KERNEL);
    if (data == NULL) {
        osal_log("kmalloc error!\n");
        return OSAL_FAILURE;
    }

    hrtimer_init(&data->linux_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    data->linux_timer.function = osal_hrtimer_callback;
    data->hrt = hrtimer;

    hrtimer->timer = data;
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_hrtimer_create);

int osal_hrtimer_start(osal_hrtimer *hrtimer)
{
    ktime_t ktime;
    struct osal_hrtimer_data *data;

    if (hrtimer == NULL || hrtimer->timer == NULL || hrtimer->handler == NULL) {
        osal_log("time handler or hrtimer is null!\n");
        return OSAL_FAILURE;
    }

    data = (struct osal_hrtimer_data *)hrtimer->timer;
    // Convert milliseconds to nanoseconds
    ktime = ktime_set(0, hrtimer->interval * NSEC_PER_MSEC);
    hrtimer_start(&data->linux_timer, ktime, HRTIMER_MODE_REL);

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_hrtimer_start);

int osal_hrtimer_destroy(osal_hrtimer *hrtimer)
{
    struct osal_hrtimer_data *data;
    int ret;

    if (hrtimer == NULL || hrtimer->timer == NULL) {
        return OSAL_FAILURE;
    }

    data = (struct osal_hrtimer_data *)hrtimer->timer;
    ret = hrtimer_cancel(&data->linux_timer);
    kfree(data);
    hrtimer->timer = NULL;

    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_hrtimer_destroy);

struct timer_list_info {
    struct timer_list time_list;
    unsigned long private_data; // struct timer_list removed member "data" since 4.15
};
typedef void (*timer_callback_fun)(struct timer_list *);

unsigned long osal_timer_get_private_data(const void *sys_data) // sys_data = struct timer_list
{
    struct timer_list_info *list_info = NULL;
    if (sys_data == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return 0;
    }
    list_info = container_of(sys_data, typeof(*list_info), time_list);
    return list_info->private_data;
}
EXPORT_SYMBOL(osal_timer_get_private_data);

int osal_timer_init(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if (timer == NULL || timer->timer != NULL || timer->handler == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    if (msecs_to_jiffies(timer->interval) <= 0) {
        osal_log("timer->interval=%d parameter invalid!\n", timer->interval);
        return OSAL_FAILURE;
    }

    time_info = (struct timer_list_info *)kmalloc(sizeof(struct timer_list_info), GFP_KERNEL);
    if (time_info == NULL) {
        osal_log("kmalloc error!\n");
        return OSAL_FAILURE;
    }
    time_info->private_data = timer->data;
    time_info->time_list.expires = jiffies + msecs_to_jiffies(timer->interval) - 1;
    timer_setup(&time_info->time_list, (timer_callback_fun)(uintptr_t)timer->handler, 0);
    timer->timer = time_info;
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_timer_init);

int osal_timer_mod(osal_timer *timer, unsigned int interval)
{
    unsigned long expires;
    struct timer_list_info *time_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || msecs_to_jiffies(interval) <= 0) {
        osal_log("interval=%d parameter invalid! caller: %p\n", interval, __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    time_info = (struct timer_list_info *)timer->timer;
    expires = jiffies + msecs_to_jiffies(interval) - 1;
    mod_timer(&time_info->time_list, expires);
    timer->interval = interval;
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_timer_mod);

int osal_timer_start(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    time_info = (struct timer_list_info *)timer->timer;
    // Check timer status to ensure it is not added repeatedly
    if (!timer_pending(&time_info->time_list)) {
        add_timer(&time_info->time_list);
    } else {
        mod_timer(&time_info->time_list, time_info->time_list.expires);
    }
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_timer_start);

int osal_timer_start_on(osal_timer *timer, unsigned long delay, int cpu)
{
    struct timer_list_info *time_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }
    time_info = (struct timer_list_info *)timer->timer;
    time_info->time_list.expires = jiffies + msecs_to_jiffies(delay);
    // Check timer status to ensure it is not added repeatedly
    if (!timer_pending(&time_info->time_list)) {
        add_timer(&time_info->time_list);
    } else {
        mod_timer(&time_info->time_list, time_info->time_list.expires);
    }
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_timer_start_on);

int osal_timer_stop(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    time_info = (struct timer_list_info *)timer->timer;
    return del_timer(&time_info->time_list);
}
EXPORT_SYMBOL(osal_timer_stop);

int osal_timer_destroy(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    time_info = (struct timer_list_info *)timer->timer;
    del_timer(&time_info->time_list);
    kfree(time_info);
    timer->timer = NULL;
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_timer_destroy);
int osal_timer_destroy_sync(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return -1;
    }

    time_info = (struct timer_list_info *)timer->timer;
    del_timer_sync(&time_info->time_list);
    kfree(time_info);
    timer->timer = NULL;
    return 0;
}
EXPORT_SYMBOL(osal_timer_destroy_sync);

unsigned int osal_jiffies_to_msecs(const unsigned int n)
{
    return jiffies_to_msecs(n);
}
EXPORT_SYMBOL(osal_jiffies_to_msecs);

unsigned long long osal_sched_clock(void)
{
    return sched_clock();
}
EXPORT_SYMBOL(osal_sched_clock);

void osal_time_get_ts64(osal_timeval *tv)
{
    struct timespec64 now;
    if (tv == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    ktime_get_ts64(&now);
    tv->tv_sec = now.tv_sec;
    tv->tv_usec = now.tv_nsec / NS_TO_USEC;
}
EXPORT_SYMBOL(osal_time_get_ts64);

void osal_time_get_real_ts64(osal_timeval *tv)
{
    struct timespec64 now;
    if (tv == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    ktime_get_real_ts64(&now);
    tv->tv_sec = now.tv_sec;
    tv->tv_usec = now.tv_nsec / NS_TO_USEC;
}
EXPORT_SYMBOL(osal_time_get_real_ts64);

void osal_time_get_boottime_ts64(osal_timeval *tv)
{
    struct timespec64 now;
    if (tv == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    ktime_get_boottime_ts64(&now);
    tv->tv_sec = now.tv_sec;
    tv->tv_usec = now.tv_nsec / NS_TO_USEC;
}
EXPORT_SYMBOL(osal_time_get_boottime_ts64);

void osal_time_get_raw_ts64(osal_timeval *tv)
{
    struct timespec64 now;
    if (tv == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    ktime_get_raw_ts64(&now);
    tv->tv_sec = now.tv_sec;
    tv->tv_usec = now.tv_nsec / NS_TO_USEC;
}
EXPORT_SYMBOL(osal_time_get_raw_ts64);

void osal_gettimeofday(osal_timeval *tv)
{
    struct timespec64 now;
    if (tv == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }
    ktime_get_raw_ts64(&now);
    tv->tv_sec = now.tv_sec;
    tv->tv_usec = now.tv_nsec / NS_TO_USEC;
}
EXPORT_SYMBOL(osal_gettimeofday);

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
EXPORT_SYMBOL(osal_timeval_sub);

unsigned long osal_msecs_to_jiffies(const unsigned int m)
{
    return msecs_to_jiffies(m);
}
EXPORT_SYMBOL(osal_msecs_to_jiffies);

unsigned long long osal_get_jiffies(void)
{
    return (jiffies);
}
EXPORT_SYMBOL(osal_get_jiffies);
