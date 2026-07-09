#include <osal_test.h>

#define TEST_NUMS  21

static int hrtimer_counter = 0;
static int timer_counter = 0;

osal_hrtimer_restart hrtimer_callback(void *parameter)
{
	hrtimer_counter++;
	osal_test_log("High-resolution timer callback, counter = %d\n", hrtimer_counter);
	return OSAL_HRTIMER_NORESTART;
}

void timer_callback(unsigned long data)
{
	timer_counter++;
	osal_test_log("Timer callback, counter = %d\n", timer_counter);
}

void osal_timer_test(void)
{
	osal_hrtimer hrtimer;
	osal_timer timer;
	osal_timer timer1;
	osal_timeval timeval;
	int total_tests = TEST_NUMS; // total test nums
	int failed_index = 0;
	const char *failed_tests[TEST_NUMS] = {0};
	unsigned long long tick_time  = osal_sched_clock();

	hrtimer_counter = 0;
	timer_counter = 0;

	hrtimer.timer = NULL;
	hrtimer.handler = hrtimer_callback;
	hrtimer.interval = 100; // 100 ms

	osal_test_log("Starting osal_timer test...\n");

	if (osal_hrtimer_create(&hrtimer) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "osal_hrtimer_create";
		osal_test_log("[failed] osal_hrtimer_create\n");
		return;
	}

	if (osal_hrtimer_start(&hrtimer) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "osal_hrtimer_start";
		osal_test_log("[failed] osal_hrtimer_start\n");
		osal_hrtimer_destroy(&hrtimer);
		return;
	}

	msleep(500);

	if (osal_hrtimer_destroy(&hrtimer) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "osal_hrtimer_destroy";
	}

	if (hrtimer_counter <= 0) {
		failed_tests[failed_index++] = "hrtimer_callback execution";
		osal_test_log("[failed] High-resolution timer did not run, counter = %d\n", hrtimer_counter);
	}

	osal_test_log("High-resolution timer counter = %d\n", hrtimer_counter);

	timer.timer = NULL;
	timer.handler = timer_callback;
	timer.data = (unsigned long)&timer;
	timer.interval = 100; // 100 ms

	if (osal_timer_init(&timer) != 0) {
		failed_tests[failed_index++] = "osal_timer_init";
		osal_test_log("[failed] osal_timer_init\n");
		return;
	}

	if (osal_timer_mod(&timer, 200) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "osal_timer_mod";
		osal_test_log("[failed] osal_timer_mod\n");
	}

	if (osal_timer_start(&timer) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "osal_timer_start";
		osal_test_log("[failed] osal_timer_start\n");
		osal_timer_destroy(&timer);
		return;
	}

	msleep(500);

	if (osal_timer_stop(&timer) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "osal_timer_stop";
	}

	if (osal_timer_destroy(&timer) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "osal_timer_destroy";
	}

	if (timer_counter <= 0) {
		failed_tests[failed_index++] = "timer_callback execution";
		osal_test_log("[failed] Timer did not run, counter = %d\n", timer_counter);
	}

	timer1.timer = NULL;
	timer1.handler = timer_callback;
	timer1.data = (unsigned long)&timer;
	timer1.interval = 100; // 100 ms

    if (osal_timer_init(&timer1) != 0) {
		failed_tests[failed_index++] = "osal_timer_init";
		return;
	}

	if (osal_timer_mod(&timer1, 200) != 0) {
		failed_tests[failed_index++] = "osal_timer_mod";
	}

	if (osal_timer_start(&timer1) != 0) {
		failed_tests[failed_index++] = "osal_timer_start";
		osal_timer_destroy(&timer1);
		return;
	}

	msleep(500);

	if (osal_timer_stop(&timer1) != 0) {
		failed_tests[failed_index++] = "osal_timer_stop";
	}

	if (osal_timer_destroy_sync(&timer1) != 0) {
		failed_tests[failed_index++] = "osal_timer_destroy";
	}

	if (timer_counter == 0) {
		failed_tests[failed_index++] = "timer_callback execution";
	}

    if (tick_time <= 0) {
        failed_tests[failed_index++] = "osal_sched_clock";
    }

    tick_time = osal_get_jiffies();
    if (tick_time <= 0) {
        failed_tests[failed_index++] = "osal_get_jiffies";
    }

    if (osal_msecs_to_jiffies(3000) != 750) {
        failed_tests[failed_index++] = "osal_msecs_to_jiffies";
    }

    if (osal_jiffies_to_msecs(3000) != 12000) {
        failed_tests[failed_index++] = "osal_jiffies_to_msecs";
    }

    osal_gettimeofday(&timeval);
    osal_test_log("tv_sec = %ld, tv_usec = %ld\n", timeval.tv_sec, timeval.tv_usec);
    if (timeval.tv_sec <= 0 || timeval.tv_usec <= 0 || timeval.tv_usec >= 1000000) {
        failed_tests[failed_index++] = "osal_gettimeofday fail";
    }

	// output counter status
	osal_test_log("Timer counter = %d\n", timer_counter);

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All timer tests(hrtimer_create/start/destroy/destroy_sync, timer_init/mod/start/stop/destroy, osal_sched_clock/osal_get_jiffies/osal_msecs_to_jiffies/osal_jiffies_to_msecs) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_timer test completed.\n");
}
OSAL_CMD_EXPORT(osal_timer_test, Test timer operations);