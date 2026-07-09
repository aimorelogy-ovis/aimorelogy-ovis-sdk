/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023/06/25     flyingcys    first version
 */

#include <rtthread.h>
#include <pthread.h>
#include <stdio.h>
#include <drivers/dev_pin.h>


int main(void)
{
#ifdef RT_USING_SMART
    rt_kprintf("Hello RT-Smart!\n");
#else
    rt_kprintf("Hello RISC-V!\n");
#endif

    while (1)
    {
        rt_thread_mdelay(500);
    }

    return 0;
}
