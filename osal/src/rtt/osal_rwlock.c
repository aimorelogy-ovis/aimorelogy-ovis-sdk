#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

typedef struct {
	rt_mutex_t mutex;
	rt_sem_t rw_sem;
	int readers;
} rwlock_t;

static long long mutex_index = 0;
static long long sem_index = 0;

void rwlock_init(rwlock_t *lock)
{
	char mutex_name[50];
	char sem_name[50];

	rt_snprintf(mutex_name, sizeof(mutex_name), "rwlock_%lld", mutex_index++);
	rt_snprintf(sem_name, sizeof(sem_name), "rwsem_%lld", sem_index++);

	lock->mutex = rt_mutex_create(mutex_name, RT_IPC_FLAG_FIFO);
	lock->rw_sem = rt_sem_create(sem_name, 1, RT_IPC_FLAG_FIFO);
	lock->readers = 0;
}

void read_lock(rwlock_t *lock)
{
	rt_mutex_take(lock->mutex, RT_WAITING_FOREVER);
	lock->readers++;
	if (lock->readers == 1) {
		rt_sem_take(lock->rw_sem, RT_WAITING_FOREVER);
	}
	rt_mutex_release(lock->mutex);
}

void read_unlock(rwlock_t *lock)
{
	rt_mutex_take(lock->mutex, RT_WAITING_FOREVER);
	lock->readers--;
	if (lock->readers == 0) {
		rt_sem_release(lock->rw_sem);
	}
	rt_mutex_release(lock->mutex);
}

void write_lock(rwlock_t *lock)
{
	rt_sem_take(lock->rw_sem, RT_WAITING_FOREVER);
}

void write_unlock(rwlock_t *lock)
{
	rt_sem_release(lock->rw_sem);
}

int osal_rwlock_init(osal_rwlock *rw)
{
	if (rw == NULL || rw->rwlock != NULL) {
		osal_log("rwlock parameter invalid!\n");
		return OSAL_FAILURE;
	}
	rw->rwlock = rt_malloc(sizeof(rwlock_t));
	if (rw->rwlock == NULL) {
		osal_log("malloc fail!\n");
		return OSAL_FAILURE;
	}
	rwlock_init(((rwlock_t *)(rw->rwlock)));
	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_rwlock_init);

void osal_rwlock_read_lock(osal_rwlock *rw)
{
	if (rw == NULL || rw->rwlock == NULL) {
		osal_log("rwlock is null!\n");
		return;
	}
	read_lock(((rwlock_t *)(rw->rwlock)));
}
// EXPORT_SYMBOL(osal_rwlock_read_lock);

void osal_rwlock_read_unlock(osal_rwlock *rw)
{
	if (rw == NULL || rw->rwlock == NULL) {
		osal_log("rwlock is null!\n");
		return;
	}
	read_unlock(((rwlock_t *)(rw->rwlock)));
}
// EXPORT_SYMBOL(osal_rwlock_read_unlock);

void osal_rwlock_write_lock(osal_rwlock *rw)
{
	if (rw == NULL || rw->rwlock == NULL) {
		osal_log("rwlock is null!\n");
		return;
	}
	write_lock(((rwlock_t *)(rw->rwlock)));
}
// EXPORT_SYMBOL(osal_rwlock_write_lock);

void osal_rwlock_write_unlock(osal_rwlock *rw)
{
	if (rw == NULL || rw->rwlock == NULL) {
		osal_log("rwlock is null!\n");
		return;
	}
	write_unlock(((rwlock_t *)(rw->rwlock)));
}
// EXPORT_SYMBOL(osal_rwlock_write_unlock);

void osal_rwlock_destory(osal_rwlock *rw)
{
	if (rw == NULL || rw->rwlock == NULL) {
		osal_log("rwlock is null!\n");
		return;
	}
	rwlock_t *lock = (rwlock_t *)rw->rwlock;
	rt_mutex_delete(lock->mutex);
	rt_sem_delete(lock->rw_sem);
	rt_free(rw->rwlock);
	rw->rwlock = NULL;
}
// EXPORT_SYMBOL(osal_rwlock_destory);
