#include <aos/kernel.h>
#include <aos/debug.h>
#include "osal.h"
#include "osal_inner.h"

void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag)
{
    return aos_malloc(size);
}

void *osal_malloc(unsigned long size)
{
    return aos_malloc(size);
}

void *osal_kzalloc(unsigned long size, unsigned int osal_gfp_flag)
{
    return aos_zalloc(size);
}

void *osal_zalloc(unsigned long size)
{
	return aos_zalloc(size);
}

void *osal_calloc(unsigned long size, unsigned long num)
{
	return aos_calloc(size, num);
}

void osal_kfree(void *addr)
{
    if (addr != NULL) {
        aos_free(addr);
    }
    return;
}

void osal_free(void *addr)
{
    if (addr != NULL) {
        aos_free(addr);
    }
    return;
}

void *osal_vmalloc(unsigned long size)
{
    return aos_malloc(size);
}

void *osal_vzalloc(unsigned long size)
{
	return aos_zalloc(size);
}

void osal_vfree(void *addr)
{
    if (addr != NULL) {
        aos_free(addr);
    }
    return;
}

void *osal_ioremap(unsigned long phys_addr, unsigned long size)
{
    return (void *)phys_addr;
}

void *osal_ioremap_nocache(unsigned long phys_addr, unsigned long size)
{
    return (void *)phys_addr;
}

void *osal_ioremap_wc(unsigned long phys_addr, unsigned long size)
{
    return (void *)phys_addr;
}

void *osal_ioremap_cached(unsigned long phys_addr, unsigned long size)
{
    return (void *)phys_addr;
}

void osal_iounmap(void *addr, unsigned long size)
{
    return;
}

unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n)
{
    if (to != NULL && from != NULL) {
        memcpy(to, from, n);
        return FALSE;
    }
    return n;
}

unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n)
{
    if (to != NULL && from != NULL) {
        memcpy(to, from, n);
        return FALSE;
    }
    return n;
}

void *osal_phys_to_virt(unsigned long addr)
{
    return (void *)addr;
}

unsigned long osal_virt_to_phys(const void *virt_addr)
{
    return (unsigned long)virt_addr;
}

void *osal_blockmem_vmap(unsigned long phys_addr, unsigned long size)
{
    return (void *)phys_addr;
}

void *osal_blockmem_vmap_nocache(unsigned long phys_addr, unsigned long size)
{
    return (void *)phys_addr;
}

void osal_blockmem_vunmap(const void *virt_addr)
{
    return;
}

void osal_blockmem_free(unsigned long phys_addr, unsigned long size)
{
    if ((void *)phys_addr != NULL && size > 0) {
        aos_free((void *)phys_addr);
    }
    return;
}

osal_blockmem_status osal_blockmem_get_status(unsigned long phyaddr, unsigned int size)
{
    if (phyaddr == 0 || size == 0) {
        return OSAL_BLOCKMEM_INVALID_PHYADDR;
    }
    return OSAL_BLOCKMEM_VALID;
}

void osal_memset(void *s, int ch, int n)
{
	memset(s, ch, n);
}

void osal_memcpy(void *dst, const void *src, int n)
{
	memcpy(dst, src, n);
}

