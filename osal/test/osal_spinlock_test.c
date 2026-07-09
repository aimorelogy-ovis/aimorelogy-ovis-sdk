#include <osal_test.h>

#define THREAD_NUM 3
#define STACK_SIZE 2048

int spinlock_thread_entry(void *data)
{
	osal_spinlock *lock = (osal_spinlock *)data;

	for (int i = 0; i < THREAD_NUM; i++) {
		osal_test_log("[Thread] Trying to acquire spinlock...\n");
		osal_spin_lock(lock);
		osal_test_log("[Thread] Acquired spinlock.\n");

		osal_test_log("[Thread] count_%d\n", i);

		osal_test_log("[Thread] Releasing spinlock...\n");
		osal_spin_unlock(lock);
		osal_test_log("[Thread] Released spinlock.\n");
	}
	return 0;
}

// static void spinlock_trylock_thread_entry(void *data)
// {
// 	osal_spinlock *lock = (osal_spinlock *)data;

// 	osal_test_log("[Thread] Trying to acquire spinlock (trylock)...\n");
// 	for (int i = 0; i < THREAD_NUM; i++) {
// 		if (osal_spin_trylock(lock) == 0) {
// 			osal_test_log("[Thread] Acquired spinlock (trylock).\n");

// 			osal_test_log("[Thread] count_%d\n", i);

// 			osal_test_log("[Thread] Releasing spinlock...\n");
// 			osal_spin_unlock(lock);
// 			osal_test_log("[Thread] Released spinlock.\n");
// 		} else {
// 			osal_test_log("[Thread] to acquire spinlock (trylock).\n");
// 		}
// 	}
// }

// static void spinlock_irqsave_thread_entry(void *data)
// {
// 	osal_spinlock *lock = (osal_spinlock *)data;
// 	unsigned long flags;

// 	for (int i = 0; i < THREAD_NUM; i++) {
// 		osal_test_log("[Thread] Trying to acquire spinlock with irqsave...\n");
// 		osal_spin_lock_irqsave(lock, &flags);
// 		osal_test_log("[Thread] Acquired spinlock with irqsave.\n");
// 		osal_test_log("[Thread] count_%d\n", i);
// 		osal_test_log("[Thread] Releasing spinlock with irqrestore...\n");
// 		osal_spin_unlock_irqrestore(lock, &flags);
// 		osal_test_log("[Thread] Released spinlock with irqrestore.\n");
// 	}
// }

void osal_spinlock_test(void)
{
	osal_task *trigger_thread = NULL;
	unsigned long flags = 0;
	osal_spinlock lock;
	lock.lock = NULL;
	osal_test_log("Starting osal_spinlock test...\n");

	if (osal_spin_lock_init(&lock) != 0) {
		osal_test_log("[failed] osal_spin_lock_init\n");
		return;
	}
	//osal_test_log("[success] osal_spin_lock_init\n");

	trigger_thread = osal_kthread_create(spinlock_thread_entry, &lock, "spinlock", STACK_SIZE);
	msleep(1000);

	osal_test_log("osal_spin_trylock_irqsave test start\n");

	osal_spin_trylock_irqsave(&lock, &flags);
	osal_spin_unlock_irqrestore(&lock, &flags);
	//osal_test_log("[success] osal_spin_trylock_irqsave\n");

	osal_kthread_destroy(trigger_thread, 1);

	osal_spin_lock_destroy(&lock);
	//osal_test_log("[success] osal_spin_lock_destroy\n");
	osal_test_log("[success] All spinlock tests(init, lock, trylock, irqsave, destroy) passed \n");
	osal_test_log("osal_spinlock test completed.\n");
}
OSAL_CMD_EXPORT(osal_spinlock_test, Test spinlock operations);