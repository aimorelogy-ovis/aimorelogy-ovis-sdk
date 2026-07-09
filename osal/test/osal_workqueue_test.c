#include <osal_test.h>


void work_handler(osal_workqueue *workqueue) {
	osal_test_log("Work handler called with queue_flag = %d\n", workqueue->queue_flag);
}

void osal_workqueue_test(void)
{
	int total_tests = 5;  // init, schedule, highpri_schedule, flush, destroy
	int failed_index = 0;
	const char *failed_tests[5] = {0};
	osal_workqueue work;
	work.queue_flag = 0;
	work.work = NULL;
	work.handler = work_handler;

	osal_test_log("Starting osal_workqueue test...\n");

	// test initialization
	if (osal_workqueue_init(&work, work_handler) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "workqueue_init";
		osal_test_log("[failed] osal_workqueue_init\n");
		return;
	}

	osal_test_log("[INFO] Scheduling workqueue with normal priority...\n");
	if (osal_workqueue_schedule(&work) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "workqueue_schedule";
		osal_test_log("[failed] osal_workqueue_schedule\n");
	}
	msleep(100);

	osal_test_log("[INFO] Scheduling workqueue with high priority...\n");
	if (osal_workqueue_schedule_highpri(&work) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "workqueue_schedule_highpri";
		osal_test_log("[failed] osal_workqueue_schedule_highpri\n");
	}
	msleep(100);

	osal_test_log("[INFO] Flushing workqueue...\n");
	if (osal_workqueue_flush(&work) != OSAL_SUCCESS) {
		failed_tests[failed_index++] = "workqueue_flush";
		osal_test_log("[failed] osal_workqueue_flush\n");
	}

	osal_test_log("[INFO] Destroying workqueue...\n");
	osal_workqueue_destroy(&work);

	// output final test results
	if (failed_index == 0) {
		osal_test_log("[success] All workqueue tests(init, schedule, highpri_schedule, flush, destroy) passed \n");
	} else {
		osal_test_log("[failed] %d/%d tests failed:\n", failed_index, total_tests);
		for (int i = 0; i < failed_index; i++) {
			osal_test_log("  - %s\n", failed_tests[i]);
		}
	}

	osal_test_log("osal_workqueue test completed.\n");
}
OSAL_CMD_EXPORT(osal_workqueue_test, Test workqueue operations);