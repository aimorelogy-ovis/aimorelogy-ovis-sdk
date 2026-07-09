#include <osal_test.h>
#include "osal_task.h"

#define NUM_THREADS 3

osal_mutex test_mutex;
int shared_counter = 0;

void *thread_function(void *arg)
{
	int thread_id = *(int *)arg;

	osal_test_log("Thread %d: attempting to lock mutex...\n", thread_id);

	if (osal_mutex_lock(&test_mutex) == 0)
	{
		int temp;
		osal_test_log("Thread %d: acquired mutex lock\n", thread_id);

		msleep(10);
		temp = shared_counter;			//lock read
		shared_counter = temp + 1;		//lock modify

		osal_test_log("Thread %d: shared_counter = %d\n", thread_id, shared_counter);
		osal_mutex_unlock(&test_mutex);
		osal_test_log("Thread %d: released mutex lock\n", thread_id);
	}
	else
	{
		osal_test_log("Thread %d: failed to acquire mutex lock\n", thread_id);
	}

	return NULL;
}

int thread_function_wrapper(void *arg)
{
	thread_function(arg);
	return 0;
}

void osal_mutex_test(void)
{
	int total_tests = 4;  // total test nums: init, threads, trylock, destroy
	int failed_index = 0;
	const char *failed_tests[4] = {0};

	// test multi-thread synchronization
	osal_task *threads[NUM_THREADS];
	int thread_ids[NUM_THREADS];
	char thread_name[32];

	int trylock_failed = 1;

	osal_test_log("Testing osal_mutex interfaces...\n");

	if (osal_mutex_init(&test_mutex) != 0) {
		failed_tests[failed_index++] = "osal_mutex_init";
		return;
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		thread_ids[i] = i;
		snprintf(thread_name, sizeof(thread_name), "test_thread_%d", i);
		threads[i] = osal_kthread_create((osal_kthread_handler)thread_function_wrapper	, &thread_ids[i], thread_name, 2048);
		if (threads[i] == NULL)
		{
			osal_test_log("Failed to create thread %d\n", i);
		}
	}

	msleep(100);

	for (int i = 0; i < NUM_THREADS; i++) {
		if (threads[i] != NULL) {
			osal_kthread_destroy(threads[i], 0);
		}
	}

	if (shared_counter != NUM_THREADS) {
		failed_tests[failed_index++] = "multi-thread synchronization";
	}
	shared_counter = 0;

	osal_test_log("Testing osal_mutex_trylock...\n");

	while (!osal_mutex_trylock(&test_mutex)) {
		msleep(10);
		osal_test_log("trylock failed, try again after 10ms\n");
	}
	trylock_failed = 0;
	osal_mutex_unlock(&test_mutex);

	if (trylock_failed) {
		failed_tests[failed_index++] = "osal_mutex_trylock";
	}

	// test destroy
	osal_test_log("Testing osal_mutex_destroy...\n");
	osal_mutex_destroy(&test_mutex);

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All mutex tests(init, threads, trylock, destroy) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_mutex test completed.\n");
}
OSAL_CMD_EXPORT(osal_mutex_test, Test mutex operations);