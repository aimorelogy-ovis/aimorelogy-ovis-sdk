#include <osal_test.h>

void osal_semaphore_test(void)
{
    int all_success = 1; // Flag to track overall success
    osal_semaphore sem;
    sem.sem = NULL;

    osal_test_log("Starting osal_semaphore test...\n");

    if (osal_sem_init(&sem, 1) != 0)
    {
        osal_test_log("[failed] osal_sem_init\n");
        all_success = 0;
    }

    if (osal_sem_down(&sem) != 0)
    {
        osal_test_log("[failed] osal_sem_down\n");
        all_success = 0;
    }
    else
    {
        msleep(100);
        osal_sem_up(&sem);
    }

    if (osal_sem_trydown(&sem) != 0)
    {
        osal_test_log("[failed] osal_sem_trydown\n");
        all_success = 0;
    }
    else
    {
        msleep(100);
        osal_sem_up(&sem);
    }

    if (osal_sem_down_timeout(&sem, 2000) != 0)
    {
        osal_test_log("[failed] osal_sem_down_timeout\n");
        all_success = 0;
    }
    else
    {
        msleep(100);
        osal_sem_up(&sem);
    }

    osal_sem_destroy(&sem);

    if (all_success)
    {
        osal_test_log("[success] All semaphore operation tests(init, down, trydown, timeout) passed\n");
    }
    else
    {
        osal_test_log("[failed] osal_sem_test\n");
    }

    osal_test_log("osal_semaphore test completed.\n");
}

OSAL_CMD_EXPORT(osal_semaphore_test, Test sem operations);