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
#include "drivers/dev_rtc.h"
#include <time.h>
#include <sys/time.h>

#define TEST_RTC_DEVICE_NAME    "rtc"

static void rtc_test_thread(void *parameter)
{
	rt_device_t rtc_dev = RT_NULL;
	time_t set_time, get_time;
	struct timeval set_tv, get_tv;
	// struct rt_rtc_wkalarm alarm;
	struct tm time_tm;
	int i, ret,result;

	rtc_dev = rt_device_find(TEST_RTC_DEVICE_NAME);
	if (!rtc_dev) {
		rt_kprintf("[FAIL] RTC device not found!\n");
		return;
	}

	if (rt_device_open(rtc_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK) {
		rt_kprintf("[FAIL] Open RTC device failed!\n");
		return;
	}
	result = 0;
	rt_kprintf("\n--- Starting RTC Driver Test ---\n");

	/*******************************
	 * Test basic time function
	 *******************************/
	set_time = 1704067200;  // 2024-01-01 00:00:00 UTC

	// set time
	ret = rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_SET_TIME, &set_time);
	if (ret != RT_EOK) {
		rt_kprintf("[FAIL] Set time via RT_DEVICE_CTRL_RTC_SET_TIME failed: %d\n", ret);
		result++;
	}
	rt_kprintf("\n[PASS] Basic Time Function Test\n");

	rt_thread_mdelay(50);

	// get time
	ret = rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIME, &get_time);
	if (ret != RT_EOK) {
		rt_kprintf("[FAIL] Get time via RT_DEVICE_CTRL_RTC_GET_TIME failed: %d\n", ret);
		result++;
	}

	if (set_time == get_time) {
		rt_kprintf("[PASS] Set/Get time via secs: %ld -> %ld\n", set_time, get_time);
	} else {
		rt_kprintf("[FAIL] Time mismatch: set=%ld, get=%ld\n", set_time, get_time);
		result++;
	}

	/*******************************
	 * timeval test
	 *******************************/

	set_tv.tv_sec = 1717200000;  // 2024-06-01 00:00:00 UTC
	set_tv.tv_usec = 0;

	// set timeval
	ret = rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_SET_TIMEVAL, &set_tv);
	if (ret != RT_EOK) {
		rt_kprintf("[FAIL] Set time via RT_DEVICE_CTRL_RTC_SET_TIMEVAL failed: %d\n", ret);
		result++;
	}

	rt_thread_mdelay(50);

	// get timeval
	rt_memset(&get_tv, 0, sizeof(get_tv));
	ret = rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIMEVAL, &get_tv);
	if (ret != RT_EOK) {
		rt_kprintf("[FAIL] Get time via RT_DEVICE_CTRL_RTC_GET_TIMEVAL failed: %d\n", ret);
		result++;
	}

	if (set_tv.tv_sec == get_tv.tv_sec && get_tv.tv_usec == 0) {
		rt_kprintf("[PASS] [3/5]Set/Get time via timeval: %ld.%06ld -> %ld.%06ld\n", 
				  set_tv.tv_sec, set_tv.tv_usec, get_tv.tv_sec, get_tv.tv_usec);
	} else {
		rt_kprintf("[FAIL] [3/5]Timeval mismatch: set=%ld.%06ld, get=%ld.%06ld\n", 
				  set_tv.tv_sec, set_tv.tv_usec, get_tv.tv_sec, get_tv.tv_usec);
		result++;
	}

	// test time continuity

	rt_uint32_t prev_sec = 0;
	for (i = 0; i < 5; i++) {
		rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIMEVAL, &get_tv);
		
		if (i > 0 && get_tv.tv_sec <= prev_sec) {
			rt_kprintf("[FAIL] Time not increasing! %ld <= %ld\n", get_tv.tv_sec, prev_sec);
			result++;
		}

		gmtime_r(&get_tv.tv_sec, &time_tm);
		rt_kprintf("[%d/5] Current time: %04d-%02d-%02d %02d:%02d:%02d (usec=%ld)\n", 
					i, time_tm.tm_year + 1900, time_tm.tm_mon + 1, 
					time_tm.tm_mday, time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec,
					get_tv.tv_usec);

		prev_sec = get_tv.tv_sec;
		rt_thread_mdelay(2000);
	}

	// test tv_usec
	if (get_tv.tv_usec == 0) {
		rt_kprintf("\n[PASS] Time Continuity Test\n");
	} else {
		rt_kprintf("[FAIL] tv_usec should be 0 but got %ld\n", get_tv.tv_usec);
		result++;
	}

	/*******************************
	 * boundry value test
	 *******************************/
	// set time to zero
	set_tv.tv_sec = 0;
	set_tv.tv_usec = 0;
	rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_SET_TIMEVAL, &set_tv);
	rt_thread_mdelay(50);
	rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIMEVAL, &get_tv);
	if (get_tv.tv_sec == 0) {
		rt_kprintf("[PASS] Set/Get zero time: %ld.%06ld\n", get_tv.tv_sec, get_tv.tv_usec);
	} else {
		rt_kprintf("[FAIL] Zero time test failed: %ld != 0\n", get_tv.tv_sec);
		result++;
	}

	// set time to uint32 max
	set_tv.tv_sec = 0xFFFFFFFF;
	set_tv.tv_usec = 0;
	rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_SET_TIMEVAL, &set_tv);
	rt_thread_mdelay(50);
	rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_GET_TIMEVAL, &get_tv);
	if (get_tv.tv_sec == 0xFFFFFFFF) {
		rt_kprintf("[PASS] Max time value: %lu\n", get_tv.tv_sec);
	} else {
		rt_kprintf("[FAIL] Max time test failed: %lu != %lu\n", get_tv.tv_sec, 0xFFFFFFFFUL);
		result++;
	}

	/*******************************
	 * Test End
	 *******************************/

	rt_kprintf("\n[RTC] All tests completed!\n");

	rt_device_close(rtc_dev);

	if (result == 0) {
		rt_kprintf("\n[RTC] All tests pass\n");
	} else {
		rt_kprintf("\n[RTC] Some tests failed: %d\n", result);
	}
}

static int rtc_test(int argc, char *argv[])
{
	rt_thread_t tid = rt_thread_create("rtc_test",
									  rtc_test_thread,
									  RT_NULL,
									  2048,
									  RT_THREAD_PRIORITY_MAX / 2,
									  20);

	if (tid) {
		rt_thread_startup(tid);
		return RT_EOK;
	}
	return -RT_ERROR;
}
MSH_CMD_EXPORT(rtc_test, Run RTC driver test);