#include <osal_test.h>

static volatile int tasklet_handled = 0;
static volatile unsigned long received_data = 0;

/* Tasklet handler function */
static void test_tasklet_handler(unsigned long data) {
    tasklet_handled = 1;
    received_data = data;
    osal_test_log("Handler executed! Data=0x%lx\n", data);
}

void osal_interrupt_test(void) {
    osal_tasklet tasklet = {0};
    int failed_index = 0;
    const char *failed_tests[5] = {0};
    tasklet.handler = test_tasklet_handler;
    tasklet.data = (unsigned long)0x1234;

    osal_test_log("Starting osal_interrupt test...\n");

    /* test osal_tasklet_init */
    if (osal_tasklet_init(&tasklet) != OSAL_SUCCESS) {
        failed_tests[failed_index++] = "osal_tasklet_init";
        osal_test_log("[failed] osal_tasklet_init\n");
    }

    /* test osal_tasklet_schedule */
    if (osal_tasklet_schedule(&tasklet) != OSAL_SUCCESS) {
        failed_tests[failed_index++] = "osal_tasklet_schedule";
        osal_test_log("[failed] osal_tasklet_schedule\n");
    }
    osal_msleep(1500);
    if (tasklet_handled != 1 || received_data != 0x1234) {
        osal_test_log("[failed] tasklet handler not executed!\n");
    }

    /* Test Update parameters */
    tasklet.handler = test_tasklet_handler;
    tasklet.data = (unsigned long)0x2234;

    tasklet_handled = 0;
    if (osal_tasklet_update(&tasklet) != OSAL_SUCCESS) {
        failed_tests[failed_index++] = "osal_tasklet_update";
        osal_test_log("[failed] osal_tasklet_update\n");
    }

    /* Test Verify updated handler */
    if (osal_tasklet_schedule(&tasklet) != OSAL_SUCCESS) {
        failed_tests[failed_index++] = "reschedule_after_update";
        osal_test_log("[failed] reschedule after update\n");
    }
    osal_msleep(1500);
    if (tasklet_handled != 1 || received_data != 0x2234) {
        osal_test_log("[failed] Updated handler not executed!\n");
    }

    /* test osal_tasklet_kill */
    if (osal_tasklet_kill(&tasklet) != OSAL_SUCCESS) {
        failed_tests[failed_index++] = "osal_tasklet_kill";
        osal_test_log("[failed] osal_tasklet_kill\n");
    }

    /* output overall result */
    if (failed_index == 0) {
        osal_test_log("[success] All tasklet operations(init/update/schedule/kill) passed\n");
    } else {
        osal_test_log("[failed] %d/%d tests failed:\n", failed_index, 5);
        for (int i = 0; i < failed_index; i++) {
            osal_test_log("  - %s\n", failed_tests[i]);
        }
    }

    osal_test_log("osal_interrupt test completed.\n");
}
OSAL_CMD_EXPORT(osal_interrupt_test, Test tasklet operations);