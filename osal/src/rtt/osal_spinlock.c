#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

int osal_spin_lock_init(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock != NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}
	lock->lock = (void *)rt_malloc(sizeof(struct rt_spinlock));
	if (lock->lock == NULL) {
		osal_log("malloc error!\n");
		return OSAL_FAILURE;
	}
	rt_spin_lock_init((struct rt_spinlock *)lock->lock);

	return OSAL_SUCCESS;
}
// EXPORT_SYMBOL(osal_spin_lock_init);

void osal_spin_lock(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock == NULL) {
		osal_log("spinlock is null!\n");
		return;
	}

	rt_spin_lock((struct rt_spinlock *)lock->lock);
}
// EXPORT_SYMBOL(osal_spin_lock);

void osal_spin_lock_bh(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock == NULL) {
		osal_log("spinlock is null!\n");
		return;
	}

	rt_spin_lock((struct rt_spinlock *)lock->lock);
}
// EXPORT_SYMBOL(osal_spin_lock_bh);

void osal_spin_unlock_bh(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock == NULL) {
		osal_log("spinlock is null!\n");
		return;
	}

	rt_spin_unlock((struct rt_spinlock *)lock->lock);
}
// EXPORT_SYMBOL(osal_spin_unlock_bh);

int osal_spin_trylock(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock == NULL) {
		osal_log("spinlock is null!\n");
		return FALSE;
	}

	if (((struct rt_spinlock *)lock->lock)->lock == 1) {
		return FALSE;
	} else {
		rt_spin_lock((struct rt_spinlock *)lock->lock);
	}

	return TRUE;
}
// EXPORT_SYMBOL(osal_spin_trylock);

void osal_spin_unlock(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock == NULL) {
		osal_log("spinlock is null!\n");
		return;
	}

	rt_spin_unlock((struct rt_spinlock *)lock->lock);
}
// EXPORT_SYMBOL(osal_spin_unlock);

void osal_spin_lock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
	rt_base_t f;

	if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
		osal_log("spinlock is null!\n");
		return;
	}

	f = rt_spin_lock_irqsave((struct rt_spinlock *)lock->lock);

	*flags = f;
}
// EXPORT_SYMBOL(osal_spin_lock_irqsave);

void osal_spin_unlock_irqrestore(osal_spinlock *lock, unsigned long *flags)
{
	if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
		osal_log("spinlock is null!\n");
		return;
	}

	rt_spin_unlock_irqrestore((struct rt_spinlock *)lock->lock, *flags);
}
// EXPORT_SYMBOL(osal_spin_unlock_irqrestore);

void osal_spin_lock_destroy(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock == NULL) {
		osal_log("spinlock is null!\n");
		return;
	}

	rt_free((struct rt_spinlock *)lock->lock);
	lock->lock = NULL;
}
// EXPORT_SYMBOL(osal_spin_lock_destroy);

int osal_spin_trylock_irq(osal_spinlock *lock)
{
	if (lock == NULL || lock->lock == NULL) {
		osal_log("spinlock is null!\n");
		return FALSE;
	}

	if (((struct rt_spinlock *)lock->lock)->lock == 1) {
		return FALSE;
	} else {
		rt_spin_lock((struct rt_spinlock *)lock->lock);
	}

	return TRUE;
}
// EXPORT_SYMBOL(osal_spin_trylock_irq);

void osal_spin_trylock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
	rt_base_t f;

	if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
		osal_log("spinlock is null!\n");
		return;
	}

	if (((struct rt_spinlock *)lock->lock)->lock == 1) {
		return;
	} else {
		f = rt_spin_lock_irqsave((struct rt_spinlock *)lock->lock);
		*flags = f;
	}
}
// EXPORT_SYMBOL(osal_spin_trylock_irqsave);
