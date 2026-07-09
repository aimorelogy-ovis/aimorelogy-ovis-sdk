#include <osal_test.h>

#define THREAD_COUNT 4
#define STACK_SIZE   4096

typedef struct {
	osal_completion com;
	int count;
} com_data;

// define global exit flag to control the infinite loop trigger thread exit
static volatile int trigger_exit = 0;

int trigger_thread_entry(void* data)
{
	osal_completion* completion = (osal_completion*)data;
	while (!trigger_exit) {
		msleep(10);
		osal_complete(completion);
	}
	return 0;
}

int test_osal_wait_for_completion(void* data)
{
	osal_completion* completion = &((com_data*)data)->com;
	((com_data*)data)->count += 1;
	osal_test_log("call test_osal_wait_for_completion\n");
	osal_complete(completion);
	return 0;
}

void osal_completion_test(void)
{
	osal_completion com;
	int result;
	unsigned long timeout_result;
	int total_tests = 7;  // total test nums
	int failed_index = 0;
	const char *failed_tests[8] = {0};  // reserve one extra space for safety
	osal_task* trigger_thread = NULL;
	com_data data		= {.count = 0};

	com.completion = NULL;
	osal_test_log("Testing osal_completion interfaces...\n");

	// test osal_completion_init
	osal_test_log("Testing osal_completion_init...\n");
	result = osal_completion_init(&com);
	if (result != 0)
		failed_tests[failed_index++] = "osal_completion_init";

	// test osal_completion_reinit
	osal_test_log("Testing osal_completion_reinit...\n");
	osal_completion_reinit(&com);

	// test osal_complete
	osal_test_log("Testing osal_complete...\n");
	osal_complete(&com);

	// test osal_wait_for_completion
	osal_test_log("Testing osal_wait_for_completion...\n");
	result = osal_completion_init(&data.com);
	if (result != 0)
		failed_tests[failed_index++] = "osal_completion_init (second)";

	trigger_thread = osal_kthread_create(test_osal_wait_for_completion, &data,
		"test_osal_wait_for_completion", STACK_SIZE);
	osal_wait_for_completion(&data.com);
	osal_complete_destory(&data.com);
	if (data.count != 1) {
		failed_tests[failed_index++] = "osal_wait_for_completion";
	}

	// test osal_wait_for_completion_timeout
	osal_test_log("Testing osal_wait_for_completion_timeout...\n");

	// reset exit flag, ensure it's 0 before creating trigger thread
	trigger_exit = 0;
	trigger_thread = osal_kthread_create(trigger_thread_entry, &com, "trigger_thread", STACK_SIZE);

	timeout_result = osal_wait_for_completion_timeout(&com, 50);
	if (timeout_result != 0)
		failed_tests[failed_index++] = "osal_wait_for_completion_timeout";

	// notify thread to exit and wait for thread termination before destroying completion object
	trigger_exit = 1;
	osal_kthread_destroy(trigger_thread, 0);

	// test osal_complete_all
	osal_test_log("Testing osal_complete_all...\n");
	osal_completion_reinit(&com);
	osal_complete_all(&com);

	// test osal_complete_destory
	osal_test_log("Testing osal_complete_destory...\n");
	osal_complete_destory(&com);
	osal_kthread_destroy(trigger_thread, 0);

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All completion tests passed (init, reinit, complete, wait_for_completion, wait_for_completion_timeout, complete_all, complete_destory)\n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}
}
OSAL_CMD_EXPORT(osal_completion_test, Test completion operations);

int test_multi_completion(void* arg)
{
	osal_completion* completion = (osal_completion*)arg;
	msleep(10);
	osal_complete(completion);
	return 0;
}

void osal_completion_multi_test(void)
{
	osal_task* task[THREAD_COUNT];
	osal_completion com[THREAD_COUNT];
	int result;
	int i;
	int failed_index = 0;
	const char *failed_tests[THREAD_COUNT * 2] = {0};  // reserve space for initialization and creation of each thread
	char thread_name[100];

	for (i = 0; i < THREAD_COUNT; i++) {
		com[i].completion = NULL;
		result = osal_completion_init(&com[i]);

		if (result != 0) {
			failed_tests[failed_index++] = "completion initialization";
			return;
		}
		snprintf(thread_name, sizeof(thread_name), "test_multi_completion:%d\n", i);
		task[i] = osal_kthread_create(test_multi_completion, &com[i], thread_name, STACK_SIZE);
		if (task[i] == NULL) {
			failed_tests[failed_index++] = "thread creation";
			return;
		}
	}

	for (i = 0; i < THREAD_COUNT; i++) {
		osal_wait_for_completion(&com[i]);
		osal_complete_destory(&com[i]);
	}

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All multi-completion tests(init, create, wait_for_completion, complete_destory) passed \n");
	} else {
		osal_test_log("[failed] %d tests failed in multi-completion test:\n", failed_index);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}
}
OSAL_CMD_EXPORT(osal_completion_multi_test, Test completion operations);