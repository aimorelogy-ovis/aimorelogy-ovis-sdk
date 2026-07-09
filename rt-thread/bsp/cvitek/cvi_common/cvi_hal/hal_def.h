#ifndef __HAL_DEF_H__
#define __HAL_DEF_H__

#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static inline uint32_t reg_read_32(uintptr_t addr)
{
    return *(volatile uint32_t *)addr;
}

static inline void reg_write_32(uintptr_t addr, uint32_t value)
{
    *(volatile uint32_t *)addr = value;
}

static inline uint64_t reg_read_64(uintptr_t addr)
{
    return *(volatile uint64_t *)addr;
}

static inline void reg_write_64(uintptr_t addr, uint64_t value)
{
    *(volatile uint64_t *)addr = value;
}

static inline void barrier(void)
{
    asm volatile("" : : : "memory");
}

static inline unsigned long __ffs(uint64_t word)
{
    return __builtin_ctzl(word);
}

static inline int __fls(int x)
{
    return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}

#endif