#include <aos/kernel.h>
#include <aos/debug.h>
#include <aos/list.h>
#include <drv/tick.h>

#include "osal.h"
#include "osal_inner.h"

#define US_TO_SEC  1000000

struct timer_list_info {
    aos_timer_t time_list;
    unsigned long private_data; // struct timer_list removed member "data" since 4.15
};

typedef void (*timer_callback_fun)(aos_timer_t *);

void timer_handler_adapter(void *arg1, void *arg2) {
    osal_timer *my_timer = (osal_timer *)arg2;

    if (my_timer->handler) {
        ((timer_callback_fun)(my_timer->handler))
            (&(((struct timer_list_info *)(my_timer->timer))->time_list));
    }
}

unsigned long osal_timer_get_private_data(const void *sys_data) // sys_data = struct timer_list
{
    struct timer_list_info *list_info = NULL;
    if (sys_data == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return 0;
    }
    list_info = aos_container_of(sys_data, struct timer_list_info, time_list);
    return list_info->private_data;
}

int osal_timer_init(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if ((timer == NULL) || (timer->timer != NULL || timer->handler == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

	if (timer->interval <= 0) {
		osal_log("timer->interval=%d parameter invalid!\n", timer->interval);
		return OSAL_FAILURE;
	}

    time_info = (struct timer_list_info *)aos_malloc(sizeof(struct timer_list_info));
    if (time_info == NULL) {
        osal_log("malloc error!\n");
        return OSAL_FAILURE;
    }

    time_info->private_data = timer->data;
    timer->timer = time_info;

    return aos_timer_new_ext((aos_timer_t *)&(time_info->time_list),
        timer_handler_adapter, timer, timer->interval, 1, 0);
}

int osal_timer_mod(osal_timer *timer, unsigned int interval)
{
    if ((timer == NULL) || (timer->timer == NULL) || interval <= 0) {
        osal_log("interval=%d parameter invalid! caller: %p\n", interval, __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    aos_timer_change((aos_timer_t *)&(((struct timer_list_info *)
        (timer->timer))->time_list), interval);
    timer->interval = interval;

    return OSAL_SUCCESS;
}

int osal_timer_start(osal_timer *timer)
{
    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return aos_timer_start((aos_timer_t *)&(((struct timer_list_info *)
        (timer->timer))->time_list));
}

int osal_timer_stop(osal_timer *timer)
{
    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return aos_timer_stop((aos_timer_t *)&(((struct timer_list_info *)
        (timer->timer))->time_list));
}

int osal_timer_destroy(osal_timer *timer)
{
    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    aos_timer_free((aos_timer_t *)&(((struct timer_list_info *)
        (timer->timer))->time_list));
    aos_free(timer->timer);
    timer->timer = NULL;

    return OSAL_SUCCESS;
}

int osal_timer_destroy_sync(osal_timer *timer)
{
    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    aos_timer_free((aos_timer_t *)&(((struct timer_list_info *)
        (timer->timer))->time_list));
    aos_free(timer->timer);
    timer->timer = NULL;

    return OSAL_SUCCESS;
}

void osal_gettimeofday(osal_timeval *tv)
{
    if (tv == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    unsigned long long time_us = csi_tick_get_us();
    tv->tv_sec = time_us / US_TO_SEC;
    tv->tv_usec = time_us % US_TO_SEC;
}

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

unsigned int osal_jiffies_to_msecs(const unsigned int n)
{
    return aos_kernel_tick2ms(n);
}

unsigned long osal_msecs_to_jiffies(const unsigned int m)
{
    return aos_kernel_ms2tick(m);
}
