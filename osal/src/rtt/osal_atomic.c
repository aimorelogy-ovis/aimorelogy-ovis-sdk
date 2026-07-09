#include <rtatomic.h>

#include "osal.h"

#include "osal_inner.h"

int osal_atomic_read(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	return rt_atomic_load((rt_atomic_t *)atomic);
}
// EXPORT_SYMBOL(osal_atomic_read);

void osal_atomic_set(osal_atomic *atomic, int i)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		return;
	}

	rt_atomic_store((rt_atomic_t *)atomic, i);
}
// EXPORT_SYMBOL(osal_atomic_set);

int osal_atomic_inc_return(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	int value = rt_atomic_add((rt_atomic_t *)atomic, 1);
	value += 1;
	return value;
}
// EXPORT_SYMBOL(osal_atomic_inc_return);

int osal_atomic_add_return(osal_atomic *atomic, unsigned int count)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	int value = rt_atomic_add((rt_atomic_t *)atomic, count);
	value += count;
	return value;
}
// EXPORT_SYMBOL(osal_atomic_add_return);

int osal_atomic_dec_return(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		return OSAL_FAILURE;
	}

	int value = rt_atomic_sub((rt_atomic_t *)atomic, 1);
	value -= 1;
	return value;
}
// EXPORT_SYMBOL(osal_atomic_dec_return);

void osal_atomic_inc(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return;
	}

	rt_atomic_add((rt_atomic_t *)atomic, 1);
}
// EXPORT_SYMBOL(osal_atomic_inc);

void osal_atomic_sub(osal_atomic *atomic, unsigned int count)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return;
	}

	rt_atomic_sub((rt_atomic_t *)atomic, count);
}
// EXPORT_SYMBOL(osal_atomic_sub);

void osal_atomic_dec(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return;
	}

	rt_atomic_sub((rt_atomic_t *)atomic, 1);
}
// EXPORT_SYMBOL(osal_atomic_dec);

void osal_atomic_add(osal_atomic *atomic, unsigned int count)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return;
	}

	rt_atomic_add((rt_atomic_t *)atomic, count);
}
// EXPORT_SYMBOL(osal_atomic_add);

int osal_atomic_fetch_or(unsigned int v, osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}

	return rt_atomic_or((rt_atomic_t *)atomic, v);
}
// EXPORT_SYMBOL(osal_atomic_fetch_or);

int osal_atomic_fetch_and(unsigned int v, osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}

	return rt_atomic_and((rt_atomic_t *)atomic, v);
}
// EXPORT_SYMBOL(osal_atomic_fetch_and);

int osal_atomic_cmpxchg(osal_atomic *atomic, unsigned int old, unsigned int new)
{
    if (atomic == NULL) {
        osal_log("parameter invalid!\n");
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    unsigned int expected = old;
	atomic_int *atomic_ptr = (atomic_int *)&atomic->counter;

    if (atomic_compare_exchange_strong(atomic_ptr, &expected, new)) {
        return expected;
    } else {
        return new;
    }
}
// EXPORT_SYMBOL(osal_atomic_cmpxchg);

int osal_atomic_dec_and_test(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}

	int old_value = rt_atomic_sub((rt_atomic_t *)atomic, 1);

	if (old_value - 1 == 0) {
		return OSAL_SUCCESS;
	} else {
		return OSAL_FAILURE;
	}
}
// EXPORT_SYMBOL(osal_atomic_dec_and_test);

int osal_atomic_inc_and_test(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}

	int value = rt_atomic_add((rt_atomic_t *)atomic, 1);
	if (value + 1 == 0) {
		return OSAL_SUCCESS;
	} else {
		return OSAL_FAILURE;
	}
}
// EXPORT_SYMBOL(osal_atomic_inc_and_test);

int osal_atomic_inc_not_zero(osal_atomic *atomic)
{
	if (atomic == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}

	int value = rt_atomic_load((rt_atomic_t *)atomic);
	if (value == 0) {
		return OSAL_FAILURE;
	} else {
		if (rt_atomic_add((rt_atomic_t *)atomic, 1) == value) {
			return OSAL_SUCCESS;
		}
		return OSAL_FAILURE;
	}
}
// EXPORT_SYMBOL(osal_atomic_inc_not_zero);
