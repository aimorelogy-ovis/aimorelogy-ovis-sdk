#include <aos/kernel.h>
#include <aos/debug.h>
#include "osal.h"
#include "osal_inner.h"

void osal_bitmap_set_bit(int nr, unsigned long *addr)
{
    if (addr == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return;
    }
    __sync_fetch_and_or(&addr[nr / OSAL_BITS_PER_LONG], 1UL << (nr % OSAL_BITS_PER_LONG));
}

void osal_bitmap_clear_bit(int nr, unsigned long *addr)
{
    if (addr == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return;
    }
    __sync_fetch_and_and(&addr[nr / OSAL_BITS_PER_LONG], ~(1UL << (nr % OSAL_BITS_PER_LONG)));
}

void osal_bitmap_change_bit(int nr, unsigned long *addr)
{
    if (addr == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return;
    }
    __sync_fetch_and_xor(&addr[nr / OSAL_BITS_PER_LONG], 1UL << (nr % OSAL_BITS_PER_LONG));
}

int osal_bitmap_test_bit(int nr, unsigned long *addr)
{
    if (addr == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }
    return (__sync_fetch_and_add(&addr[nr / OSAL_BITS_PER_LONG], 0) & (1UL << (nr % OSAL_BITS_PER_LONG))) != 0;
}

int osal_bitmap_test_and_set_bit(int nr, unsigned long *addr)
{
    if (addr == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return -1;
    }
    unsigned long old = __sync_fetch_and_or(&addr[nr / OSAL_BITS_PER_LONG], 1UL << (nr % OSAL_BITS_PER_LONG));
    return (old & (1UL << (nr % OSAL_BITS_PER_LONG))) != 0;
}

int osal_bitmap_test_and_clear_bit(int nr, unsigned long *addr)
{
    if (addr == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }
    unsigned long old = __sync_fetch_and_and(&addr[nr / OSAL_BITS_PER_LONG], ~(1UL << (nr % OSAL_BITS_PER_LONG)));
    return (old & (1UL << (nr % OSAL_BITS_PER_LONG))) != 0;
}

int osal_bitmap_test_and_change_bit(int nr, unsigned long *addr)
{
    if (addr == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }
    unsigned long old = __sync_fetch_and_xor(&addr[nr / OSAL_BITS_PER_LONG], 1UL << (nr % OSAL_BITS_PER_LONG));
    return (old & (1UL << (nr % OSAL_BITS_PER_LONG))) != 0;
}

int osal_bitmap_find_first_zero_bit(const unsigned long *name, unsigned size)
{
    if (name == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }
    unsigned long i;

    for (i = 0; i < size; i++) {
        unsigned long val = __sync_fetch_and_add((unsigned long *)&name[i], 0);
        if (~val) {
            return i * OSAL_BITS_PER_LONG + __builtin_ffsl(~val) - 1;
        }
    }

    return size * OSAL_BITS_PER_LONG;
}

int osal_bitmap_find_first_bit(const unsigned long *name, unsigned size)
{
    if (name == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }
    unsigned long idx;

    for (idx = 0; idx < size; idx++) {
        unsigned long val = __sync_fetch_and_add((unsigned long *)&name[idx], 0);
        if (val)
            return idx * OSAL_BITS_PER_LONG + __builtin_ffsl(val) - 1;
    }

    return size * OSAL_BITS_PER_LONG;
}

int osal_bitmap_find_next_zero_bit(const unsigned long *name, int size, int offset)
{
    if (name == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }

    int bit = offset;
    const unsigned long *p = name + (bit / OSAL_BITS_PER_LONG);
    unsigned long mask = 1UL << (bit % OSAL_BITS_PER_LONG);
    unsigned long word;

    while (bit < size)
    {
        word = *p++;
        while (mask)
        {
            if ((word & mask) == 0)
                return bit;
            mask <<= 1;
            bit++;
            if (bit >= size)
                return size;
        }
        mask = 1UL;
    }

    return size;
}

int osal_bitmap_find_next_bit(const unsigned long *name, unsigned size, int offset)
{
    if (name == NULL) {
        osal_log("parameter invalid! caller: %p\n", __builtin_return_address(0));
        osal_dump_stack();
        return OSAL_FAILURE;
    }
    unsigned long idx = offset / OSAL_BITS_PER_LONG;
    unsigned long bit = offset % OSAL_BITS_PER_LONG;
    unsigned long mask;

    if (idx >= size)
        return size * OSAL_BITS_PER_LONG;

    mask = name[idx] & (~0UL << bit);
    if (mask)
        return idx * OSAL_BITS_PER_LONG + __builtin_ffsl(mask) - 1;

    for (idx++; idx < size; idx++) {
        if (name[idx])
            return idx * OSAL_BITS_PER_LONG + __builtin_ffsl(name[idx]) - 1;
    }

    return size * OSAL_BITS_PER_LONG;
}
