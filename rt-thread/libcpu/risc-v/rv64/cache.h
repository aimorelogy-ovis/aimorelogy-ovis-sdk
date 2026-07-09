#ifndef __CACHE_H__
#define __CACHE_H__

#ifdef RT_USING_CACHE

#include <rtthread.h>
#include <rthw.h>
#include <rtdef.h>

void rt_hw_cpu_icache_enable(void);
void rt_hw_cpu_icache_disable(void);
void rt_hw_cpu_icache_ops(int ops, void *addr, int size);

rt_base_t rt_hw_cpu_icache_status(void);

void rt_hw_cpu_dcache_enable(void);
void rt_hw_cpu_dcache_disable(void);
void rt_hw_cpu_dcache_ops(int ops, void *addr, int size);
void rt_hw_cpu_dcache_clean(void *addr, int size);
void rt_hw_cpu_dcache_invalidate(void *addr, int size);
void rt_hw_cpu_dcache_clean_invalidate(void *addr, int size);

rt_base_t rt_hw_cpu_dcache_status(void);

#endif /* RT_USING_CACHE */

#endif /* __CACHE_H__ */
