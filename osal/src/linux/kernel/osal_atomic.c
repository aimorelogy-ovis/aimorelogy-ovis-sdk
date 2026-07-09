#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include "osal.h"

#include "osal_inner.h"

int osal_atomic_read(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return atomic_read((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_read);

void osal_atomic_set(osal_atomic *atomic, int i)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return;
    }

    atomic_set((atomic_t *)atomic, i);
}
EXPORT_SYMBOL(osal_atomic_set);

int osal_atomic_inc_return(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return atomic_inc_return((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_inc_return);

int osal_atomic_add_return(osal_atomic *atomic, unsigned int count)
{
	if (atomic == NULL) {
		osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
		return OSAL_FAILURE;
	}

	return atomic_add_return(count, (atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_add_return);

int osal_atomic_dec_return(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        return OSAL_FAILURE;
    }

    return atomic_dec_return((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_dec_return);

void osal_atomic_inc(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return;
    }

    atomic_inc((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_inc);

void osal_atomic_sub(osal_atomic *atomic, unsigned int count)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return;
    }

    atomic_sub(count, (atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_sub);

void osal_atomic_dec(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return;
    }

    atomic_dec((atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_dec);

void osal_atomic_add(osal_atomic *atomic, unsigned int count)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return;
    }

    atomic_add(count, (atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_add);

int osal_atomic_fetch_or(unsigned int v, osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
		return OSAL_FAILURE;
    }

	return atomic_fetch_or(v, (atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_fetch_or);

int osal_atomic_fetch_and(unsigned int v, osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
		return OSAL_FAILURE;
    }

	return atomic_fetch_and(v, (atomic_t *)atomic);
}
EXPORT_SYMBOL(osal_atomic_fetch_and);

int osal_atomic_cmpxchg(osal_atomic *atomic, unsigned int old, unsigned int new)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }

	return atomic_cmpxchg((atomic_t *)atomic, old, new);
}
EXPORT_SYMBOL(osal_atomic_cmpxchg);

int osal_atomic_dec_and_test(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    return atomic_dec_and_test((atomic_t *)atomic) ? OSAL_SUCCESS : OSAL_FAILURE;
}
EXPORT_SYMBOL(osal_atomic_dec_and_test);

int osal_atomic_inc_and_test(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    return atomic_inc_and_test((atomic_t *)atomic) ? OSAL_SUCCESS : OSAL_FAILURE;
}
EXPORT_SYMBOL(osal_atomic_inc_and_test);

int osal_atomic_inc_not_zero(osal_atomic *atomic)
{
    if (atomic == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    return atomic_inc_not_zero((atomic_t *)atomic) ? OSAL_SUCCESS : OSAL_FAILURE;
}
EXPORT_SYMBOL(osal_atomic_inc_not_zero);
