#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

void osal_bitmap_set_bit(int nr, unsigned long *addr)
{
	if (addr == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return;
	}
	addr[nr / OSAL_BITS_PER_LONG] |= 1UL << (nr % OSAL_BITS_PER_LONG);
}
// EXPORT_SYMBOL(osal_bitmap_set_bit);

void osal_bitmap_clear_bit(int nr, unsigned long *addr)
{
	if (addr == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return;
	}
	addr[nr / OSAL_BITS_PER_LONG] &= ~(1UL << (nr % OSAL_BITS_PER_LONG));
}
// EXPORT_SYMBOL(osal_bitmap_clear_bit);

void osal_bitmap_change_bit(int nr, unsigned long *addr)
{
	if (addr == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return;
	}
	addr[nr / OSAL_BITS_PER_LONG] ^= (1UL << (nr % OSAL_BITS_PER_LONG));
}
// EXPORT_SYMBOL(osal_bitmap_change_bit);

int  osal_bitmap_test_bit(int nr, unsigned long *addr)
{
	if (addr == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}
	return (addr[nr / OSAL_BITS_PER_LONG] & (1UL << (nr % OSAL_BITS_PER_LONG))) != 0;
}
// EXPORT_SYMBOL(osal_bitmap_test_bit);

int  osal_bitmap_test_and_set_bit(int nr, unsigned long *addr)
{
	unsigned long mask = 1UL << (nr % (sizeof(unsigned long) * 8));
	unsigned long *p = ((unsigned long *)addr) + (nr / (sizeof(unsigned long) * 8));
	unsigned long old;

	old = rt_atomic_exchange(p, *p | mask);

	return (old & mask) != 0;
}
// EXPORT_SYMBOL(osal_bitmap_test_and_set_bit);

int  osal_bitmap_test_and_clear_bit(int nr, unsigned long *addr)
{
	if (addr == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}

	unsigned long mask = 1UL << (nr % OSAL_BITS_PER_LONG);
	unsigned long *p = addr + (nr / OSAL_BITS_PER_LONG);
	int old = (*p & mask) != 0;
	*p &= ~mask; // Clear the bit
	return old;
}
// EXPORT_SYMBOL(osal_bitmap_test_and_clear_bit);

int  osal_bitmap_test_and_change_bit(int nr, unsigned long *addr)
{
	if (addr == NULL) {
		osal_log("parameter invalid!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}
	unsigned long mask = 1UL << (nr % OSAL_BITS_PER_LONG);
	unsigned long *p = addr + (nr / OSAL_BITS_PER_LONG);
	int old = (*p & mask) != 0;
	*p ^= mask;
	return old;
}
// EXPORT_SYMBOL(osal_bitmap_test_and_change_bit);

int osal_bitmap_find_first_zero_bit(const unsigned long *name, unsigned size)
{
	if (name == NULL) {
		osal_log("kmalloc error!\n");
		osal_dump_stack();
		return OSAL_FAILURE;
	}
	unsigned long i;

	for (i = 0; i < size; i++) {
		if (~name[i]) {
			return i * OSAL_BITS_PER_LONG + __builtin_ffsl(~name[i]) - 1;
		}
	}

	return size * OSAL_BITS_PER_LONG;
}
// EXPORT_SYMBOL(osal_bitmap_find_first_zero_bit);

int osal_bitmap_find_first_bit(const unsigned long *name, unsigned size)
{
	const unsigned long *p = name;
	unsigned long word;
	int bit = 0;

	while (size >= sizeof(unsigned long) * 8)
	{
		word = *p++;
		if (word != 0)
		{
			while ((word & 1) == 0)
			{
				word >>= 1;
				bit++;
			}
			return bit;
		}
		bit += sizeof(unsigned long) * 8;
		size -= sizeof(unsigned long) * 8;
	}

	if (size)
	{
		word = *p;
		while ((word & 1) == 0)
		{
			word >>= 1;
			bit++;
			if (bit >= size)
				return size;
		}
		return bit;
	}

	return size;
}
// EXPORT_SYMBOL(osal_bitmap_find_first_bit);


int osal_bitmap_find_next_zero_bit(const unsigned long *name, int size, int offset)
{
	if (name == NULL) {
		osal_log("parameter invalid!\n");
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
// EXPORT_SYMBOL(osal_bitmap_find_next_zero_bit);

int osal_bitmap_find_next_bit(const unsigned long *name, unsigned size, int offset)
{
	if (name == NULL) {
		osal_log("kmalloc error!\n");
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
// EXPORT_SYMBOL(osal_bitmap_find_next_bit);
