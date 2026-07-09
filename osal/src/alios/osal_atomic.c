#include "k_api.h"
#include "k_atomic.h"
#include <stdatomic.h>
#include "osal.h"
#include "osal_inner.h"

int osal_atomic_read(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return rhino_atomic_get((atomic_t *)&atomic->counter);
}

void osal_atomic_set(osal_atomic *atomic, int i)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    rhino_atomic_set((atomic_t *)&atomic->counter, i);
}

int osal_atomic_inc_return(osal_atomic *atomic)
{
    atomic_val_t value;

    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    CPSR_ALLOC();

    RHINO_CPU_INTRPT_DISABLE();

    value = ++atomic->counter;

    RHINO_CPU_INTRPT_ENABLE();

    return value;
}

int osal_atomic_add_return(osal_atomic *atomic, unsigned int count)
{
    atomic_val_t value;

    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    CPSR_ALLOC();

    RHINO_CPU_INTRPT_DISABLE();

    atomic->counter += count;
    value = atomic->counter;

    RHINO_CPU_INTRPT_ENABLE();

    return value;
}

int osal_atomic_dec_return(osal_atomic *atomic)
{
    atomic_val_t value;

    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    CPSR_ALLOC();

    RHINO_CPU_INTRPT_DISABLE();
    value = --atomic->counter;
    RHINO_CPU_INTRPT_ENABLE();

    return value;
}

void osal_atomic_inc(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    rhino_atomic_inc((atomic_t *)&atomic->counter);
}

void osal_atomic_dec(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    rhino_atomic_dec((atomic_t *)&atomic->counter);
}

int osal_atomic_dec_and_test(osal_atomic *atomic)
{
    atomic_val_t value;

    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    CPSR_ALLOC();

    RHINO_CPU_INTRPT_DISABLE();
    value = --atomic->counter;
    RHINO_CPU_INTRPT_ENABLE();

    return (value == 0) ? TRUE : FALSE;
}

int osal_atomic_inc_and_test(osal_atomic *atomic)
{
    atomic_val_t value;

    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    CPSR_ALLOC();
    RHINO_CPU_INTRPT_DISABLE();
    value = ++atomic->counter;
    RHINO_CPU_INTRPT_ENABLE();

    return (value == 0) ? TRUE : FALSE;
}

int osal_atomic_inc_not_zero(osal_atomic *atomic)
{
    atomic_val_t value;

    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    CPSR_ALLOC();

    RHINO_CPU_INTRPT_DISABLE();
    value = atomic->counter;
    if (value != 0) {
        atomic->counter++;
        RHINO_CPU_INTRPT_ENABLE();
        return TRUE;
    }
    RHINO_CPU_INTRPT_ENABLE();

    return FALSE;
}

void osal_atomic_add(osal_atomic *atomic, unsigned int count)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    rhino_atomic_add((atomic_t *)&atomic->counter, count);
}

int osal_atomic_fetch_or(unsigned int v, osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return OSAL_FAILURE;
    }

	return atomic_fetch_or((atomic_t *)&atomic->counter, v);
}

int osal_atomic_fetch_and(unsigned int v, osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return OSAL_FAILURE;
    }

	return atomic_fetch_and((atomic_t *)&atomic->counter, v);
}

int osal_atomic_cmpxchg(osal_atomic *atomic, unsigned int old, unsigned int new)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

	CPSR_ALLOC();
	int ret = atomic->counter;

	RHINO_CPU_INTRPT_DISABLE();

	if (atomic->counter == old) {
		atomic->counter = new;
		ret = old;
	}

	RHINO_CPU_INTRPT_ENABLE();

	return ret;
}
