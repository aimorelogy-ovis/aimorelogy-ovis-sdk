#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cvi_log.h"
#include "osal.h"

#define VAR_UNUSED(var) (void)var

static int test_time(void)
{
	int rc = 0;
	uint64_t time_us_1 = 0;
	uint64_t time_us_2 = 0;
	rc |= OSAL_TIME_GetBootTimeUs(&time_us_1);
	usleep(1500000);
	rc |= OSAL_TIME_GetBootTimeUs(&time_us_2);
	CVI_LOGI("Test Time: %" PRIu64 " -> %" PRIu64 ", %" PRIu64 " us\n",
			 time_us_1, time_us_2, time_us_2 - time_us_1);
	return rc;
}

static OSAL_MUTEX_ATTR_S static_mutex = OSAL_MUTEX_INITIALIZER;
static OSAL_MUTEX_ATTR_S static_mutex_r = OSAL_MUTEX_INITIALIZER_R;
static int test_mutex(void)
{
	OSAL_MUTEX_Lock(&static_mutex);
	OSAL_MUTEX_Lock(&static_mutex_r);
	OSAL_MUTEX_Lock(&static_mutex_r);
	int rc = 0;
	OSAL_MUTEX_ATTR_S ma;
	memset(&ma, 0, sizeof(ma));
	OSAL_MUTEX_HANDLE_S mutex;
	ma.name = "Test Mutex 1";
	ma.type = PTHREAD_MUTEX_NORMAL;
	// ma.type = PTHREAD_MUTEX_RECURSIVE;
	rc = OSAL_MUTEX_Create(&ma, &mutex);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGI("OSAL_MUTEX_Create %s failed\n", ma.name);
		exit(-1);
	}
	rc = OSAL_MUTEX_Lock(mutex);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGI("OSAL_MUTEX_Lock %s failed\n", mutex->attr.name);
		exit(-1);
	}
	rc = OSAL_MUTEX_Unlock(mutex);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGI("OSAL_MUTEX_Unlock %s failed\n", mutex->attr.name);
		exit(-1);
	}
	rc = OSAL_MUTEX_Destroy(mutex);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGI("OSAL_MUTEX_Destroy %s failed\n", ma.name);
		exit(-1);
	}
	OSAL_MUTEX_Unlock(&static_mutex_r);
	OSAL_MUTEX_Unlock(&static_mutex_r);
	OSAL_MUTEX_Unlock(&static_mutex);
	CVI_LOGI("Test mutex %s succeeded\n", ma.name);
	return rc;
}

static void test_task_entry(void *p)
{
	int i;
	CVI_LOGI("test_task_entry: >>> %s\n", (char *)p);
	for (i = 0; i < 3; i++) {
		CVI_LOGI("  [%s] count %d\n", (char *)p, i);
		OSAL_TASK_Sleep(1000000);
		OSAL_TASK_Resched();
	}
	CVI_LOGI("test_task_entry: <<< %s\n", (char *)p);
}

static int test_task(void)
{
	int rc = 0;
	OSAL_MUTEX_ATTR_S ta;
	OSAL_TASK_HANDLE_S task;

	// joinable task
	ta.name = "Test Task Joinable";
	//ta.entry = test_task_entry;
	//ta.param = (void *)ta.name; // carry the name for testing
	//ta.priority = OSAL_TASK_PRI_NORMAL;
	//ta.detached = false;
	//ta.stack_size = 128 * 1024;
	rc = OSAL_TASK_Create(&ta, &task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGI("OSAL_TASK_Create %s failed\n", ta.name);
		exit(-1);
	}
	rc = OSAL_TASK_Join(task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGI("OSAL_TASK_Join %s failed\n", ta.name);
		exit(-1);
	}

	// detached task
	ta.name = "Test Task Detached";
	//ta.entry = test_task_entry;
	//ta.param = (void *)ta.name; // carry the name for testing
	//ta.priority = OSAL_TASK_PRI_NORMAL;
	//ta.detached = true;
	//ta.stack_size = 128 * 1024;
	rc = OSAL_TASK_Create(&ta, &task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGI("OSAL_TASK_Create %s failed\n", ta.name);
		exit(-1);
	}
	usleep(4000000);

	CVI_LOGI("Test task %s succeeded\n", ta.name);
	return rc;
}

int main(int argc, char **argv)
{
	VAR_UNUSED(argc);
	VAR_UNUSED(argv);

	int rc = 0;
	rc |= test_time();
	rc |= test_mutex();
	rc |= test_task();
	return rc;
}
