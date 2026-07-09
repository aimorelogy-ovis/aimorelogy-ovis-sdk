#include <osal_test.h>

#define THREAD_COUNT 4	 // thread nums
#define TEST_LOOP	1000  // loop nums
#define STACK_SIZE   4096
// global atomic variable
static osal_atomic g_atomic;

typedef struct {
	osal_completion com;
	int mask;
} atomic_var;

void osal_atomic_test(void)
{
	osal_atomic atomic;
	int value;
	const char *failed_tests[20] = {0};  // store failed test names
	int failed_index = 0;
	int total_tests = 13;  // total test nums

	osal_atomic_set(&atomic, 0);

	// test osal_atomic_read
	value = osal_atomic_read(&atomic);
	osal_test_log("osal_atomic_read: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 0) {
		failed_tests[failed_index++] = "osal_atomic_read";
	}

	// test osal_atomic_set
	osal_atomic_set(&atomic, 10);
	value = osal_atomic_read(&atomic);
	osal_test_log("osal_atomic_set (10): %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 10) {
		failed_tests[failed_index++] = "osal_atomic_set";
	}

	// test osal_atomic_inc_return
	value = osal_atomic_inc_return(&atomic);
	osal_test_log("osal_atomic_inc_return: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 11) {
		failed_tests[failed_index++] = "osal_atomic_inc_return";
	}

	// test osal_atomic_dec_return
	value = osal_atomic_dec_return(&atomic);
	osal_test_log("osal_atomic_dec_return: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 10) {
		failed_tests[failed_index++] = "osal_atomic_dec_return";
	}

	// test osal_atomic_inc
	osal_atomic_inc(&atomic);
	value = osal_atomic_read(&atomic);
	osal_test_log("osal_atomic_inc: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 11) {
		failed_tests[failed_index++] = "osal_atomic_inc";
	}

	// test osal_atomic_sub
	osal_atomic_sub(&atomic, 1);
	value = osal_atomic_read(&atomic);
	osal_test_log("osal_atomic_sub: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 10) {
		failed_tests[failed_index++] = "osal_atomic_sub";
	}

	// test osal_atomic_dec
	osal_atomic_dec(&atomic);
	value = osal_atomic_read(&atomic);
	osal_test_log("osal_atomic_dec: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 9) {
		failed_tests[failed_index++] = "osal_atomic_dec";
	}

	// test osal_atomic_add
	osal_atomic_add(&atomic, 1);
	value = osal_atomic_read(&atomic);
	osal_test_log("osal_atomic_add: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 10) {
		failed_tests[failed_index++] = "osal_atomic_add";
	}

	// test osal_atomic_fetch_or
	value = osal_atomic_fetch_or(0x01, &atomic);
	osal_test_log("osal_atomic_fetch_or (0x01): %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 10) {
		failed_tests[failed_index++] = "osal_atomic_fetch_or";
	}

	// test osal_atomic_fetch_and
	value = osal_atomic_fetch_and(0x01, &atomic);
	osal_test_log("osal_atomic_fetch_and (0x01): %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 11) {
		failed_tests[failed_index++] = "osal_atomic_fetch_and";
	}

	// test osal_atomic_cmpxchg
	value = osal_atomic_cmpxchg(&atomic, 1, 20);
	osal_test_log("osal_atomic_cmpxchg (1, 20): %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 1) {
		failed_tests[failed_index++] = "osal_atomic_cmpxchg";
	}

	value = osal_atomic_cmpxchg(&atomic, 20, 1);
	osal_test_log("osal_atomic_cmpxchg (20, 1): %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	// if (value != 1) {
	// 	failed_tests[failed_index++] = "osal_atomic_cmpxchg";
	// }

	// test osal_atomic_dec_and_test
	value = osal_atomic_dec_and_test(&atomic);
	osal_test_log("osal_atomic_dec_and_test: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 0 || atomic.counter != 0) {
		failed_tests[failed_index++] = "osal_atomic_dec_and_test";
	}

	value = osal_atomic_dec_return(&atomic);
	osal_test_log("osal_atomic_dec_return: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	// if (value != 10) {
	// 	failed_tests[failed_index++] = "osal_atomic_dec_return";
	// }

	// test osal_atomic_inc_and_test
	value = osal_atomic_inc_and_test(&atomic);
	osal_test_log("osal_atomic_inc_and_test: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 0 || atomic.counter != 0) {
		failed_tests[failed_index++] = "osal_atomic_inc_and_test";
	}

	value = osal_atomic_dec_return(&atomic);
	osal_test_log("osal_atomic_dec_return: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	// if (value != 10) {
	// 	failed_tests[failed_index++] = "osal_atomic_dec_return";
	// }

	// test osal_atomic_inc_not_zero
	value = osal_atomic_inc_not_zero(&atomic);
	osal_test_log("osal_atomic_inc_not_zero: %d\n", value);
	osal_test_log("atomic : %d\n", atomic.counter);
	if (value != 0 || atomic.counter != 0) {
		failed_tests[failed_index++] = "osal_atomic_inc_not_zero";
	}

	// finally output test results
	if (failed_index == 0) {
		osal_test_log("[success] All %d atomic operation tests passed (read, set, inc_return, dec_return, inc, sub, dec, add, fetch_or, fetch_and, cmpxchg, dec_and_test, inc_and_test)\n", total_tests);
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}
}
OSAL_CMD_EXPORT(osal_atomic_test, Test atomic operations);

// test thread handler function - test inc/dec operations
int test_atomic_inc_dec(void* arg)
{
	osal_completion* completion = &((atomic_var*)arg)->com;
	for (int i = 0; i < TEST_LOOP; i++) {
		osal_atomic_inc(&g_atomic);
		osal_atomic_dec(&g_atomic);
	}
	osal_complete(completion);
	return 0;
}

// test thread handler function - test add/sub operations
int test_atomic_add_sub(void* arg)
{
	osal_completion* completion = (osal_completion*)arg;
	for (int i = 0; i < TEST_LOOP; i++) {
		osal_atomic_add(&g_atomic, 5);
		osal_atomic_sub(&g_atomic, 5);
	}
	osal_complete(completion);
	return 0;
}

// test thread handler function - test compare-and-exchange operations
int test_atomic_cmpxchg(void* arg)
{
	osal_completion* completion = (osal_completion*)arg;
	for (int i = 0; i < TEST_LOOP; i++) {
		int old_val = osal_atomic_read(&g_atomic);
		osal_atomic_cmpxchg(&g_atomic, old_val, old_val + 1);
	}
	osal_complete(completion);
	return 0;
}

// test thread handler function - test fetch_or operations
int test_atomic_fetch_or(void* arg)
{
	osal_completion* completion = &((atomic_var*)arg)->com;
	int mask					= ((atomic_var*)arg)->mask;

	osal_atomic_fetch_or(mask, &g_atomic);

	osal_complete(completion);
	return 0;
}

// test thread handler function - test fetch_and operations
int test_atomic_fetch_and(void* arg)
{
	osal_completion* completion = &((atomic_var*)arg)->com;
	int mask					= ((atomic_var*)arg)->mask;

	osal_atomic_fetch_and(mask, &g_atomic);

	osal_complete(completion);
	return 0;
}

void osal_atomic_multi_test(void)
{
	osal_task* task[THREAD_COUNT];
	//	 osal_completion com[THREAD_COUNT];
	atomic_var var[THREAD_COUNT] = {
		{.mask = 0x1},
		{.mask = 0x2},
		{.mask = 0x4},
		{.mask = 0x8},
	};

	char thread_name[100];
	int result;
	int i;
	const char *failed_tests[5] = {0};  // store failed test names
	int failed_index = 0;
	int total_tests = 5;  // total test nums: inc_dec, add_sub, cmpxchg, fetch_or, fetch_and

	// initialize atomic variable
	osal_atomic_set(&g_atomic, 0);

	// create threads to test inc/dec operations
	for (i = 0; i < THREAD_COUNT; i++) {
		var[i].com.completion = NULL;
		result				= osal_completion_init(&var[i].com);

		if (result != 0) {
			osal_test_log("[failed] osal_completion_init\n");
			return;
		}
		snprintf(thread_name, sizeof(thread_name), "test_atomic_inc_dec_%d", i);
		task[i] = osal_kthread_create(test_atomic_inc_dec, &var[i].com, thread_name, STACK_SIZE);
		if (task[i] == NULL) {
			osal_test_log("[failed] osal_kthread_create\n");
			return;
		}
	}

	for (i = 0; i < THREAD_COUNT; i++) {
		osal_wait_for_completion(&var[i].com);
		osal_complete_destory(&var[i].com);
	}

	if (g_atomic.counter != 0) {
		failed_tests[failed_index++] = "atomic_inc_dec";
	}

	// create threads to test add/sub operations
	osal_atomic_set(&g_atomic, 0);  // reset atomic variable
	for (i = 0; i < THREAD_COUNT; i++) {
		var[i].com.completion = NULL;
		result				= osal_completion_init(&var[i].com);

		if (result != 0) {
			osal_test_log("[failed] osal_completion_init\n");
			return;
		}

		snprintf(thread_name, sizeof(thread_name), "test_atomic_add_sub_%d", i);
		task[i] = osal_kthread_create(test_atomic_add_sub, &var[i].com, thread_name, STACK_SIZE);
		if (task[i] == NULL) {
			osal_test_log("[failed] osal_kthread_create\n");
			return;
		}
	}

	for (i = 0; i < THREAD_COUNT; i++) {
		osal_wait_for_completion(&var[i].com);
		osal_complete_destory(&var[i].com);
	}

	if (g_atomic.counter != 0) {
		failed_tests[failed_index++] = "atomic_add_sub";
	}
#if 1
	// create threads to test compare-and-exchange operations
	osal_atomic_set(&g_atomic, 0);  // reset atomic variable
	for (i = 0; i < THREAD_COUNT; i++) {
		var[i].com.completion = NULL;
		result				= osal_completion_init(&var[i].com);

		if (result != 0) {
			osal_test_log("[failed] osal_completion_init\n");
			return;
		}

		snprintf(thread_name, sizeof(thread_name), "test_atomic_cmpxchg:%d\n", i);
		task[i] = osal_kthread_create(test_atomic_cmpxchg, &var[i].com, thread_name, STACK_SIZE);
		if (task[i] == NULL) {
			osal_test_log("[failed] osal_kthread_create\n");
			return;
		}
	}
	for (i = 0; i < THREAD_COUNT; i++) {
		osal_wait_for_completion(&var[i].com);
		osal_complete_destory(&var[i].com);
	}

	if (g_atomic.counter != THREAD_COUNT * TEST_LOOP) {
		failed_tests[failed_index++] = "atomic_cmpxchg";
	}

	// create threads to test fetch_or operations
	osal_atomic_set(&g_atomic, 0);  // reset atomic variable
	for (i = 0; i < THREAD_COUNT; i++) {
		var[i].com.completion = NULL;
		result				= osal_completion_init(&var[i].com);

		if (result != 0) {
			osal_test_log("[failed] osal_completion_init\n");
			return;
		}

		snprintf(thread_name, sizeof(thread_name), "test_atomic_fetch_or_%d\n", i);
		task[i] = osal_kthread_create(test_atomic_fetch_or, &var[i], thread_name, STACK_SIZE);
		if (task[i] == NULL) {
			osal_test_log("[failed] osal_kthread_create\n");
			return;
		}
	}
	for (i = 0; i < THREAD_COUNT; i++) {
		osal_wait_for_completion(&var[i].com);
		osal_complete_destory(&var[i].com);
	}

	if (g_atomic.counter != 15) {
		failed_tests[failed_index++] = "atomic_fetch_or";
	}

	// create threads to test fetch_and operations
	osal_atomic_set(&g_atomic, 0x1);  // initialize atomic variable to all 1
	for (i = 0; i < THREAD_COUNT; i++) {
		var[i].com.completion = NULL;
		result				= osal_completion_init(&var[i].com);

		if (result != 0) {
			osal_test_log("[failed] osal_completion_init\n");
			return;
		}

		snprintf(thread_name, sizeof(thread_name), "test_atomic_fetch_and_%d\n", i);
		task[i] = osal_kthread_create(test_atomic_fetch_and, &var[i], thread_name, STACK_SIZE);
		if (task[i] == NULL) {
			osal_test_log("[failed] osal_kthread_create\n");
			return;
		}
	}
	for (i = 0; i < THREAD_COUNT; i++) {
		osal_wait_for_completion(&var[i].com);
		osal_complete_destory(&var[i].com);
	}

	if (g_atomic.counter != 0) {
		failed_tests[failed_index++] = "atomic_fetch_and";
	}
#endif
	// finally output test results
	if (failed_index == 0) {
		osal_test_log("[success] All multi-thread atomic operation tests(inc_dec, add_sub, cmpxchg, fetch_or, fetch_and) passed \n");
	} else {
		osal_test_log("[failed] %d/%d multi-thread tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}
}
OSAL_CMD_EXPORT(osal_atomic_multi_test, Test atomic multi - thread operations);