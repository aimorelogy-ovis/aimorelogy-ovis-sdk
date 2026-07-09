#include <osal_test.h>

void delayedwork_handler(osal_delayedwork *delayedwork)
{
	osal_test_log("Delayed work handler executed.\n");
}

void osal_delaywork_test(void)
{
	osal_delayedwork work;
	int result;
	int total_tests = 4;  // total test nums
	int failed_index = 0;
	const char *failed_tests[4] = {0};  // store names of failed tests
	memset(&work, 0, sizeof(work)); // zero the structure

	osal_test_log("Testing osal_delayedwork interfaces...\n");

	// test osal_delayedwork_init
	osal_test_log("Testing osal_delayedwork_init...\n");
	result = osal_delayedwork_init(&work, delayedwork_handler);
	if (result != 0)
		failed_tests[failed_index++] = "osal_delayedwork_init";

	// test osal_delayedwork_schedule
	osal_test_log("Testing osal_delayedwork_schedule...\n");
	result = osal_delayedwork_schedule(&work, 1000);
	if (result != 0)
		failed_tests[failed_index++] = "osal_delayedwork_schedule";

	msleep(1200);

	// test osal_delayedwork_cancel_sync
	osal_test_log("Testing osal_delayedwork_cancel_sync...\n");
	result = osal_delayedwork_schedule(&work, 1000);
	if (result == 0)
	{
		result = osal_delayedwork_cancel_sync(&work);
		if (result != 0)
			failed_tests[failed_index++] = "osal_delayedwork_cancel_sync";
	}
	else
	{
		failed_tests[failed_index++] = "osal_delayedwork_schedule(for cancel_sync)";
	}

	msleep(1200);

	// test osal_delayedwork_destroy
	osal_test_log("Testing osal_delayedwork_destroy...\n");
	osal_delayedwork_destroy(&work);
	//osal_test_log("[success] osal_delayedwork_destroy\n");

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All delayedwork tests(init, schedule, cancel_sync, destroy) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_delayedwork test completed.\n");
}
OSAL_CMD_EXPORT(osal_delaywork_test, Test delaywork operations);