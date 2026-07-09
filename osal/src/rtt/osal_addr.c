#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include "osal.h"
#include "osal_inner.h"

#define OSAL_ADDR_RESERVED_SIZE_MAX 0xF000000

void *osal_malloc(unsigned long size)
{
	return rt_malloc(size);
}
// EXPORT_SYMBOL(osal_malloc);

void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag)
{
	return rt_malloc(size);
}
// EXPORT_SYMBOL(osal_kmalloc);

void *osal_kzalloc(unsigned long size, unsigned int osal_gfp_flag)
{
	void *ptr = rt_malloc(size);
	if (ptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}
// EXPORT_SYMBOL(osal_kzalloc);

void *osal_kmalloc_align(unsigned int size, unsigned int osal_gfp_flag, unsigned int boundary)
{
    RT_UNUSED(osal_gfp_flag);
	void *ptr = rt_malloc_align(size, boundary);
	return ptr;
}
// EXPORT_SYMBOL(osal_kmalloc_align);

void *osal_calloc(unsigned long size, unsigned long num)
{
	return rt_calloc(num, size);
}
// EXPORT_SYMBOL(osal_calloc);

void osal_free(void *addr)
{
	rt_free(addr);
}
// EXPORT_SYMBOL(osal_free);

void osal_kfree(void *addr)
{
	rt_free(addr);
}
// EXPORT_SYMBOL(osal_kfree);

void osal_kfree_align(void *addr)
{
    rt_free_align(addr);
}
// EXPORT_SYMBOL(osal_kfree_align);

void *osal_vmalloc(unsigned long size)
{
	return rt_malloc(size);
}
// EXPORT_SYMBOL(osal_vmalloc);

void *osal_vzalloc(unsigned long size)
{
	void *ptr = rt_malloc(size);
	if (ptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}
// EXPORT_SYMBOL(osal_vzalloc);

void osal_vfree(void *addr)
{
	rt_free(addr);
}
// EXPORT_SYMBOL(osal_vfree);

static uint32_t pool_counter = 0;
int osal_pool_mem_init(void *pool, unsigned int size)
{
	struct rt_mempool *mp = (struct rt_mempool *)pool;
	void *start_addr = rt_malloc(size);
	if (start_addr == NULL) {
		return OSAL_FAILURE;
	}
	char pool_name[RT_NAME_MAX];
	snprintf(pool_name, sizeof(pool_name), "pool_%d", pool_counter++);
	return rt_mp_init(mp, pool_name, start_addr, size, size);
}
// EXPORT_SYMBOL(osal_pool_mem_init);

void *osal_pool_mem_alloc(void *pool, unsigned int size)
{
	struct rt_mempool *mp = (struct rt_mempool *)pool;
	return rt_mp_alloc(mp, size);
}
// EXPORT_SYMBOL(osal_pool_mem_alloc);

void *osal_pool_mem_alloc_align(void *pool, unsigned int size, unsigned int boundary)
{
    osal_log("not support osal_pool_mem_alloc_align\n");
	return NULL;
}
// EXPORT_SYMBOL(osal_pool_mem_alloc_align);

void osal_pool_mem_free(void *pool, const void *addr)
{
	rt_mp_free((void *)addr);
}
// EXPORT_SYMBOL(osal_pool_mem_free);

int osal_pool_mem_deinit(void *pool)
{
	return rt_mp_delete((rt_mp_t)pool);
}
// EXPORT_SYMBOL(osal_pool_mem_deinit);

osal_blockmem_status osal_blockmem_get_status(unsigned long phyaddr, unsigned int size)
{
	if (phyaddr == 0 || size == 0) {
		return OSAL_BLOCKMEM_INVALID_PHYADDR;
	}
	return OSAL_BLOCKMEM_VALID;
}
// EXPORT_SYMBOL(osal_blockmem_get_status);

void *osal_ioremap(unsigned long phys_addr, unsigned long size)
{
    RT_UNUSED(size);
	return (void *)phys_addr;
}
// EXPORT_SYMBOL(osal_ioremap);

void *osal_ioremap_nocache(unsigned long phys_addr, unsigned long size)
{
    RT_UNUSED(size);
	return (void *)phys_addr;
}
// EXPORT_SYMBOL(osal_ioremap_nocache);

void *osal_ioremap_wc(unsigned long phys_addr, unsigned long size)
{
    RT_UNUSED(size);
	return (void *)phys_addr;
}
// EXPORT_SYMBOL(osal_ioremap_wc);

void *osal_ioremap_cached(unsigned long phys_addr, unsigned long size)
{
    RT_UNUSED(size);
	return (void *)phys_addr;
}
// EXPORT_SYMBOL(osal_ioremap_cached);

void osal_iounmap(void *addr, unsigned long size)
{
    osal_log("useless in standard rt-thread\n");
	return;
}
// EXPORT_SYMBOL(osal_iounmap);

unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n)
{
	memcpy(to, from, n);
	return n;
}
// EXPORT_SYMBOL(osal_copy_from_user);

unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n)
{
	memcpy(to, from, n);
	return n;
}
// EXPORT_SYMBOL(osal_copy_to_user);

int osal_access_ok(int type, const void *addr, unsigned long size)
{
    osal_log("osal_access_ok always return true\n");
	return TRUE;
}
// EXPORT_SYMBOL(osal_access_ok);

void *osal_phys_to_virt(unsigned long addr)
{
	return (void *)addr;
}
// EXPORT_SYMBOL(osal_phys_to_virt);

unsigned long osal_virt_to_phys(const void *virt_addr)
{
	return (unsigned long)virt_addr;
}
// EXPORT_SYMBOL(osal_virt_to_phys);

void *osal_blockmem_vmap(unsigned long phys_addr, unsigned long size)
{
    RT_UNUSED(size);
	return (void *)phys_addr;
}
// EXPORT_SYMBOL(osal_blockmem_vmap);

void *osal_blockmem_vmap_nocache(unsigned long phys_addr, unsigned long size)
{
    RT_UNUSED(size);
	return (void *)phys_addr;
}
// EXPORT_SYMBOL(osal_blockmem_vmap_nocache);

/*
 * Free the virtually contiguous memory area starting at @virt_addr
 * which was created from the phys_addr passed to osal_vunmap()
 * Must not be called in interrupt context.
 * */
void osal_blockmem_vunmap(const void *virt_addr)
{
    osal_log("do nothing in osal_blockmem_vunmap\n");
	return;
}
// EXPORT_SYMBOL(osal_blockmem_vunmap);

/*
 * Free the reserved memory which has been defined in product
 **/
void osal_blockmem_free(unsigned long phys_addr, unsigned long size)
{
    osal_log("do nothing in osal_blockmem_free\n");
	return;
}
// EXPORT_SYMBOL(osal_blockmem_free);

void osal_memset(void *s, int ch, int n)
{
	memset(s, ch, n);
}
// EXPORT_SYMBOL(osal_memset);

void osal_memcpy(void *dst, const void *src, int n)
{
	memcpy(dst, src, n);
}
// EXPORT_SYMBOL(osal_memcpy);

