/*
 * Copyright (c) 2018-2025, Sophgo Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2025-07-09     shuhan.zhang    first version
 */
#include <rtthread.h>
#include <rtdevice.h>
static void wdt_test_thread(void *param)
{
    rt_device_t wdt_dev = RT_NULL;
    rt_uint32_t timeout = 5;
    rt_uint32_t timeleft;
    int count = 0;
    const char *wdt_name = "wdt0";

    rt_kprintf("\n--- Starting Watchdog Timer Driver Test ---\n");
    wdt_dev = rt_device_find(wdt_name);
    if (!wdt_dev) {
        rt_kprintf("[FAIL] Find %s device failed!\n", wdt_name);
        return;
    }

    if (rt_device_open(wdt_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK) {
        rt_kprintf("[FAIL] Open %s device failed!\n", wdt_name);
        return;
    }

    // set timeout value
    if (rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout) != RT_EOK) {
        rt_kprintf("[FAIL] Set timeout failed!\n");
        goto exit;
    }
    rt_kprintf("Set timeout: %d seconds\n", timeout);

    // start watchdog
    if (rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL) != RT_EOK) {
        rt_kprintf("[FAIL] Start watchdog failed!\n");
        goto exit;
    }
    rt_kprintf("Watchdog started!\n");

    // loop test
    while (count < 10) {
        count++;
        rt_thread_mdelay(1000);

        // set remain time
        rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_GET_TIMELEFT, &timeleft);
        rt_kprintf("[%02d] Time left: %d seconds", count, timeleft);

        // feed every 3s
        if (count % 3 != 0) {
            rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
            rt_kprintf(" - Feed dog\n");
        } else {
            rt_kprintf(" - Skip feeding\n");
        }

        // stop watchdog
        if (count == 6) {
            rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_STOP, RT_NULL);
            rt_kprintf(">>> Watchdog stopped! <<<\n");
        }

        // restart watchdog
        if (count == 8) {
            rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
            rt_kprintf(">>> Watchdog restarted! <<<\n");
        }
    }

exit:
    rt_device_close(wdt_dev);
    rt_kprintf("Test finished! Waitting for watchdog timeout...\n");
}

// wdt init
static int wdt_test(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("wdt_test", 
                           wdt_test_thread, 
                           RT_NULL, 
                           4096, 
                           RT_THREAD_PRIORITY_MAX/2, 
                           20);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
        return RT_EOK;
    }

    return -RT_ERROR;
}

MSH_CMD_EXPORT(wdt_test, cvi wdt test);
