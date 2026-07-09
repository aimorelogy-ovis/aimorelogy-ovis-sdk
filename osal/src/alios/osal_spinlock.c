#include <aos/kernel.h>
#include <k_api.h>
#include "osal.h"
#include "osal_inner.h"

int osal_spin_lock_init(osal_spinlock *lock)
{
    kspinlock_t *p = NULL;
    if (lock == NULL || lock->lock != NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    p = (kspinlock_t *)aos_malloc(sizeof(kspinlock_t));
    krhino_spin_lock_init(p);
    lock->lock = p;
    return OSAL_SUCCESS;
}

void osal_spin_lock(osal_spinlock *lock)
{
    kspinlock_t *p = NULL;
    osal_unused1(p);
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    p = (kspinlock_t *)(lock->lock);
    krhino_spin_lock(p);
}

void osal_spin_lock_bh(osal_spinlock *lock)
{
    kspinlock_t *p = NULL;
    osal_unused1(p);
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    p = (kspinlock_t *)(lock->lock);
    krhino_spin_lock(p);
}

void osal_spin_unlock_bh(osal_spinlock *lock)
{
    kspinlock_t *p = NULL;
    osal_unused1(p);
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    p = (kspinlock_t *)(lock->lock);
    krhino_spin_unlock(p);
}

int osal_spin_trylock(osal_spinlock *lock)
{
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return FALSE;
    }

    if (((kspinlock_t *)(lock->lock))->owner) {
        return FALSE;
    } else {
        krhino_spin_lock((kspinlock_t *)(lock->lock));
    }

    return TRUE;
}

void osal_spin_unlock(osal_spinlock *lock)
{
    kspinlock_t *p = NULL;
    osal_unused1(p);
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    p = (kspinlock_t *)(lock->lock);
    krhino_spin_unlock(p);
}

void osal_spin_lock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
    kspinlock_t *p = NULL;
    if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    p = (kspinlock_t *)(lock->lock);
    krhino_spin_lock_irq_save(p, *flags);
}

void osal_spin_unlock_irqrestore(osal_spinlock *lock, unsigned long *flags)
{
    kspinlock_t *p = NULL;
    if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    p = (kspinlock_t *)(lock->lock);
    krhino_spin_unlock_irq_restore(p, *flags);
}

void osal_spin_lock_destroy(osal_spinlock *lock)
{
    kspinlock_t *p = NULL;
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    p = (kspinlock_t *)(lock->lock);
    aos_free(p);
    lock->lock = NULL;
}

int osal_spin_trylock_irq(osal_spinlock *lock)
{
    if (lock == NULL || lock->lock == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return FALSE;
    }

    if (((kspinlock_t *)(lock->lock))->owner) {
        return FALSE;
    } else {
        krhino_spin_lock((kspinlock_t *)(lock->lock));
    }

    return TRUE;
}

void osal_spin_trylock_irqsave(osal_spinlock *lock, unsigned long *flags)
{
    if ((lock == NULL) || (flags == NULL) || (lock->lock == NULL)) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    if (((kspinlock_t *)(lock->lock))->owner) {
        return;
    } else {
        krhino_spin_lock_irq_save((kspinlock_t *)(lock->lock), *flags);
    }
}