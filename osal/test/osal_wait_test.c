#include <osal_test.h>

static int condition_flag = 0;

int wait_condition_func(const void *param)
{
	int *flag = (int *)param;
	return *flag != 0;
}

int wakeup_thread_entry(void *data)
{
	osal_wait *wait = (osal_wait *)data;
	while (1)
	{
		msleep( 200);
		condition_flag = 1;
		osal_wait_wakeup(wait);
		osal_test_log("[THREAD] wake up wait\n");
        if (osal_kthread_should_stop() != OSAL_SUCCESS) {
            break;
        }
	}
	return 0;
}

typedef struct {
	const char *name; // function name
	int (*test_func)(osal_wait *wait); // test function
	const char *description; // test description
} wait_test_case;

static int test_wait_init(osal_wait *wait)
{
	wait->wait = NULL;
	return osal_wait_init(wait);
}

static int test_wait_interruptible(osal_wait *wait)
{
	condition_flag = 0;
	return osal_wait_interruptible(wait, wait_condition_func, &condition_flag);
}

static int test_wait_uninterruptible(osal_wait *wait)
{
	condition_flag = 0;
	return osal_wait_uninterruptible(wait, wait_condition_func, &condition_flag);
}

static int test_wait_timeout_interruptible(osal_wait *wait)
{
	int remaining_ms;
	condition_flag = 0;
	remaining_ms = osal_wait_timeout_interruptible(wait, wait_condition_func, &condition_flag, 300);
    osal_test_log("Timeout remaining: %d ms\n", remaining_ms);
	return (remaining_ms>=0) ? OSAL_SUCCESS : OSAL_FAILURE;
}

static int test_wait_timeout_uninterruptible(osal_wait *wait)
{
	int remaining_ms;
	condition_flag = 0;
	remaining_ms = osal_wait_timeout_uninterruptible(wait, wait_condition_func, &condition_flag, 300);
    osal_test_log("Timeout remaining: %d ms\n", remaining_ms);
	return (remaining_ms>=0) ? OSAL_SUCCESS : OSAL_FAILURE;
}

static int test_wait_wakeup(osal_wait *wait)
{
	osal_wait_wakeup(wait);
	return 0;
}

static int test_wait_wakeup_interruptible(osal_wait *wait)
{
	osal_wait_wakeup_interruptible(wait);
	return 0;
}

void osal_wait_test(void)
{
	osal_wait wait;
	// create wakeup thread
	osal_task *task = NULL;
	// use mapping table to define all test cases
	const wait_test_case test_cases[] = {
		{"osal_wait_init", test_wait_init, "Initializing wait object"},
		{"osal_wait_interruptible", test_wait_interruptible, "Testing osal_wait_interruptible"},
		{"osal_wait_uninterruptible", test_wait_uninterruptible, "Testing osal_wait_uninterruptible"},
		{"osal_wait_timeout_interruptible", test_wait_timeout_interruptible, "Testing osal_wait_timeout_interruptible"},
		{"osal_wait_timeout_uninterruptible", test_wait_timeout_uninterruptible, "Testing osal_wait_timeout_uninterruptible"},
		{"osal_wait_wakeup", test_wait_wakeup, "Testing osal_wait_wakeup"},
		{"osal_wait_wakeup_interruptible", test_wait_wakeup_interruptible, "Testing osal_wait_wakeup_interruptible"},
		{NULL, NULL, NULL}
	};
	int total_tests = 0;
	int failed_index = 0;
	const char *failed_tests[8] = {0};
	condition_flag = 0;

	osal_test_log("Starting osal_wait test...\n");

	// test osal_wait_init
	osal_test_log("[INFO] %s...\n", test_cases[0].description);
	total_tests++;
	if (test_cases[0].test_func(&wait) != 0) {
		failed_tests[failed_index++] = test_cases[0].name;
		osal_test_log("[failed] %s\n", test_cases[0].name);
		return;
	}

	task = osal_kthread_create(wakeup_thread_entry, &wait, "wakeup_thread", 2048);
	osal_test_log("[INFO] Wakeup thread created\n");

	// execute other test cases
	for (int i = 1; test_cases[i].name != NULL; i++) {
		total_tests++;
		osal_test_log("[INFO] %s...\n", test_cases[i].description);
		if (test_cases[i].test_func(&wait) != OSAL_SUCCESS) {
			failed_tests[failed_index++] = test_cases[i].name;
		}
	}

	// clean
	osal_wait_destroy(&wait);
	osal_kthread_destroy(task, 1);
	osal_test_log("[INFO] osal_kthread_destroy called\n");

	msleep(100);

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All wait operation tests(init, interruptible, uninterruptible, timeout_interruptible, timeout_uninterruptible, wakeup, wakeup_interruptible) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_wait test completed.\n");
}
OSAL_CMD_EXPORT(osal_wait_test, Test wait operations);