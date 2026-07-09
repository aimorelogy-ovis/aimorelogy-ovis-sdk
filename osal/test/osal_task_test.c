#include <osal_test.h>
#include <osal_timer.h>

#define TEST_NUM 8

int test_thread_handler(void *data)
{
	int *counter = (int *)data;

	while (1) {
		(*counter)++;
		if (*counter % 10 == 0) {
			osal_test_log("Thread 1 running, counter = %d\n", *counter);
		}
		if (osal_kthread_should_stop() != OSAL_SUCCESS) {
			osal_test_log("Thread 1 should stop\n");
			break;
		}

		osal_kthread_schedule(100000000); // 100ms
	}

	return 0;
}

int test_thread_handler2(void *data)
{
	int *counter = (int *)data;

	while (1) {
		(*counter)++;
		if (*counter % 10 == 0) {
			osal_test_log("Thread 2 running, counter2 = %d\n", *counter);
		}
		if (osal_kthread_should_stop() != OSAL_SUCCESS) {
			osal_test_log("Thread 2 should stop\n");
			break;
		}
		osal_kthread_schedule(100000000); // 100ms
	}

	return 0;
}

void osal_task_test(void)
{
	osal_task *task = NULL;
	osal_task *task2 = NULL;
	int counter = 0;
	int total_tests = TEST_NUM;  // total test nums: create thread, set priority, thread execution, destroy thread
	int failed_index = 0;
	const char *failed_tests[TEST_NUM] = {0};
	int counter2 = 0;
    const unsigned long tolerance_ms = 15; // Allowable tolerance in milliseconds (adjust as needed)
	unsigned long long start;
	unsigned long long end;
	unsigned long long diff; // Store difference in milliseconds

	task = osal_kthread_create(test_thread_handler, &counter, "test_thread", 4096);
	task2 = osal_kthread_create(test_thread_handler2, &counter2, "test_thread2", 4096);
	osal_test_log("Starting osal_kthread test...\n");

	if (task == NULL) {
		failed_tests[failed_index++] = "osal_kthread_create";
	}

	if (osal_kthread_set_priority(task, 10) != 0) {
		failed_tests[failed_index++] = "osal_kthread_set_priority";
	}

	osal_kthread_set_affinity(task, 0x01);
	osal_test_log("[INFO] osal_kthread_set_affinity called with cpu_mask = 0x01\n");

	msleep(500);

	osal_kthread_destroy(task, 1);
	osal_test_log("[INFO] osal_kthread_destroy called\n");
	osal_kthread_destroy(task2, 1);
	osal_test_log("[INFO] osal_kthread_destroy 2 called\n");

	msleep(100);
	if (counter <= 0) {
		failed_tests[failed_index++] = "thread execution";
	}

	start = osal_sched_clock();
    osal_msleep(100);
	end = osal_sched_clock();
	diff = end - start;
	#ifndef __KERNEL__
	#else
	diff = osal_div64_u64(diff, 1000000);
	#endif
    // Check if duration is within expected range [expected, expected + tolerance]
    if (diff < 100 || diff > (100 + tolerance_ms)) {
		osal_test_log("osal_msleep diff: %llu\n", diff);
        failed_tests[failed_index++] = "osal_msleep";
    }

    // --- Test osal_msleep_uninterruptible ---
    start = osal_sched_clock();
    osal_msleep_uninterruptible(200);
    end = osal_sched_clock();
    diff = end - start;
	#ifndef __KERNEL__
	#else
	diff = osal_div64_u64(diff, 1000000);
	#endif
    if (diff < 200 || diff > (200 + tolerance_ms)) {
		osal_test_log("osal_msleep_uninterruptible diff: %llu\n", diff);
        failed_tests[failed_index++] = "osal_msleep_uninterruptible";
    }

    // --- Test osal_udelay --- (Busy wait, might be more precise but still allow tolerance)
    start  = osal_sched_clock();
    osal_udelay(2000); // 2000 microseconds = 2 milliseconds
    end = osal_sched_clock();
    diff = end - start;
	#ifndef __KERNEL__
	#else
	diff = osal_div64_u64(diff, 1000000);
	#endif
    // Check if close to 2ms. Allow some tolerance. Busy waits can be slightly off too.
    if (diff < 2 || diff > (2 + tolerance_ms)) { // Or use a tighter tolerance for udelay
		osal_test_log("osal_udelay diff: %llu\n", diff);
        failed_tests[failed_index++] = "osal_udelay";
    }

    // --- Test osal_mdelay --- (Busy wait)
    start  = osal_sched_clock();
    osal_mdelay(1); // 1 millisecond
    end = osal_sched_clock();
    diff = end - start;
	#ifndef __KERNEL__
	#else
	diff = osal_div64_u64(diff, 1000000);
	#endif
    if (diff < 1 || diff > (1 + tolerance_ms)) { // Or use a tighter tolerance
		osal_test_log("osal_mdelay diff: %llu\n", diff);
        failed_tests[failed_index++] = "osal_mdelay";
    }

    // --- Test osal_usleep_range --- (Should sleep AT LEAST min)
    start  = osal_sched_clock();
    osal_usleep_range(1000, 2000); // 1000 microseconds = 1 millisecond
    end = osal_sched_clock();
    diff = end - start;
	#ifndef __KERNEL__
	#else
	diff = osal_div64_u64(diff, 1000000);
	#endif
    // Check if duration is at least 1ms and within some reasonable upper bound
    if (diff < 1 || diff > (2 + tolerance_ms)) { // Should be at least 1ms, not much more than 2ms+tolerance
		osal_test_log("osal_usleep_range diff: %llu\n", diff);
        failed_tests[failed_index++] = "osal_usleep_range";
    }

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All thread operation tests(create, set priority, execution, destroy, msleep, msleep_uninterruptible, udelay, mdelay, usleep_range) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_kthread test completed.\n");
}
OSAL_CMD_EXPORT(osal_task_test, Test task operations);
