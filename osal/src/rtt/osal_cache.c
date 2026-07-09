#include <stdio.h>
#include <rtthread.h>
#include "osal.h"
#include "osal_inner.h"

void osal_dcache_region_wb(void *kvirt, unsigned long phys_addr,
			   unsigned long size)
{
	RT_UNUSED(kvirt);
	rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)phys_addr, size);
}

void osal_dcache_region_inv(void *addr, unsigned long size)
{
	rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, addr, size);
}