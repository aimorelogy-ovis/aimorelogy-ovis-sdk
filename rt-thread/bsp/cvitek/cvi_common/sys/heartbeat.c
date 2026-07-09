#include <rtthread.h>
#include <mmio.h>
#include <board.h>
#include <stdio.h>
#define WDT2_TOC_ADDR (0x0301201C)
static void hb_timer_cb(void *parameter)
{
    static uint32_t count;
    mmio_write_32(WDT2_TOC_ADDR, ++count);
}

int cvi_sys_heartbeatinit(void)
{
    static int inited;
    if (inited) return 0;

    rt_timer_t t = rt_timer_create("hb",
                                   hb_timer_cb,
                                   RT_NULL,
                                   rt_tick_from_millisecond(500),
                                   RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    if (!t) return -1;

    rt_timer_start(t);
    inited = 1;
    return 0;
}
INIT_APP_EXPORT(cvi_sys_heartbeatinit);