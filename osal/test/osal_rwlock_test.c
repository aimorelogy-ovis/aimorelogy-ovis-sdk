#include <osal_test.h>

#define THREAD_NUM 4
#define STACK_SIZE 2048

static volatile int multi_shared_counter;

void osal_rwlock_test(void)
{
	int shared_counter = 0;
	osal_rwlock rw;
	rw.rwlock = NULL;

	osal_test_log("Starting osal_rwlock test...\n");

	if (osal_rwlock_init(&rw) != 0) {
		osal_test_log("[failed] osal_rwlock_init\n");
		return;
	}
	//osal_test_log("[success] osal_rwlock_init\n");

	osal_rwlock_read_lock(&rw);
	//osal_test_log("[success] osal_rwlock_read_lock shared_counter: %d\n", shared_counter);
	msleep(100);
	osal_rwlock_read_unlock(&rw);
	//osal_test_log("[success] osal_rwlock_read_unlock shared_counter: %d\n", shared_counter);

	osal_rwlock_write_lock(&rw);
	shared_counter++;
	//osal_test_log("[success] osal_rwlock_write_lock shared_counter: %d\n", shared_counter);
	msleep(100);
	osal_rwlock_write_unlock(&rw);
	//osal_test_log("[success] osal_rwlock_write_unlock shared_counter: %d\n", shared_counter);

	osal_rwlock_read_lock(&rw);
	//osal_test_log("[success] osal_rwlock_read_lock shared_counter: %d\n", shared_counter);
	msleep(100);
	osal_rwlock_read_unlock(&rw);
	//osal_test_log("[success] osal_rwlock_read_unlock shared_counter: %d\n", shared_counter);

	osal_rwlock_destory(&rw);
	osal_test_log("[success] All rwlock tests(init, read_lock, read_unlock, write_lock, write_unlock, destory) passed \n");

	osal_test_log("osal_rwlock test completed.\n");
}
OSAL_CMD_EXPORT(osal_rwlock_test, Test rwlock operations);

int test_read_thread_handler(void *data)
{
	osal_rwlock *rwlock = data;
	if (data == NULL) {
		osal_test_log("[fail] multi rwlock test\n");
	}
	osal_rwlock_read_lock(rwlock);
	if (multi_shared_counter == 0) {
		osal_test_log("[success] multi rwlock test\n");
	} else {
		osal_test_log("[fail] multi rwlock test\n");
	}
	msleep(100);
	osal_rwlock_read_unlock(rwlock);
	while (osal_kthread_should_stop() == OSAL_SUCCESS) {
		msleep(10);
	}

	return 0;
}

int test_write_thread_handler(void *data)
{
	osal_rwlock *rwlock = data;
	if (data == NULL) {
		osal_test_log("[fail] multi rwlock test\n");
	}
	osal_rwlock_write_lock(rwlock);
	multi_shared_counter += 1;
	osal_rwlock_write_unlock(rwlock);
	while (osal_kthread_should_stop() == OSAL_SUCCESS){
        msleep(10);
    }

	return 0;
}

void osal_rwlock_multi_test(void)
{
	// create 4 threads, 3 for reading, 1 for writing, and the 3 threads reading should be consistent
	char thread_name[100];
	osal_task *task[THREAD_NUM];
	osal_rwlock rw;
	rw.rwlock = NULL;
	multi_shared_counter = 0;
	osal_test_log("Starting osal_rwlock multi test...\n");
	if (osal_rwlock_init(&rw) != 0) {
		osal_test_log("[failed] osal_rwlock_init\n");
		return;
	}
	for (int i = 0; i < THREAD_NUM - 1; i++) {
		snprintf(thread_name, sizeof(thread_name),
			 "read_multi_rwlock_%d\n", i);
		task[i] = osal_kthread_create(test_read_thread_handler, &rw,
					      thread_name, STACK_SIZE);
	}
	snprintf(thread_name, sizeof(thread_name), "write_multi_rwlock_%d\n",
		 0);
	task[THREAD_NUM - 1] = osal_kthread_create(
		test_write_thread_handler, &rw, thread_name, STACK_SIZE);
	msleep(2000);
	if (multi_shared_counter == 1) {
		osal_test_log("[success] multi rwlock test\n");
	} else {
		osal_test_log("[fail] multi rwlock test\n");
	}
	osal_rwlock_destory(&rw);
	for (int i = 0; i < THREAD_NUM; i++) {
		osal_kthread_destroy(task[i], 1);
	}
}
OSAL_CMD_EXPORT(osal_rwlock_multi_test, Test rwlock operations);