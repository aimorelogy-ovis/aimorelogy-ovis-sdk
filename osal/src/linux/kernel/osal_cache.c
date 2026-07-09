#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include "osal.h"
#include "osal_inner.h"
#include <linux/highmem.h>
#include <linux/mm.h>

// static struct device *dev; // 设备指针

// void osal_dcache_region_wb(void *kvirt, unsigned long phys_addr, unsigned long size)
// {
//     if (!dev) {
//         osal_log("Device context is NULL in osal_dcache_region_wb\n");
//         return;
//     }
//     if (!kvirt) {
//         osal_log("Invalid virtual address in osal_dcache_region_wb\n");
//         return;
//     }

//     /* 使用有效的设备上下文 */
//     dma_sync_single_for_device(dev, phys_addr, size, DMA_TO_DEVICE);
// }
// EXPORT_SYMBOL(osal_dcache_region_wb);

// void osal_dcache_region_inv(void *addr, unsigned long size)
// {

//     unsigned long phys_addr = virt_to_phys(addr);
//     if (!dev) {
//         osal_log("Device context is NULL in osal_dcache_region_inv\n");
//         return;
//     }
//     if (!addr) {
//         osal_log("Invalid address in osal_dcache_region_inv\n");
//         return;
//     }
//     /* 使用有效的设备上下文 */
//     dma_sync_single_for_cpu(dev, phys_addr, size, DMA_FROM_DEVICE);
// }
// EXPORT_SYMBOL(osal_dcache_region_inv);

void osal_dcache_region_wb(void *kvirt, unsigned long phys_addr, unsigned long size)
{
    unsigned long start = (unsigned long)kvirt;
    unsigned long end = start + size;
    struct page *page;

    for (; start < end; start += PAGE_SIZE) {
        page = virt_to_page((void *)start);
        flush_dcache_page(page);
    }
}
EXPORT_SYMBOL(osal_dcache_region_wb);

void osal_dcache_region_inv(void *addr, unsigned long size)
{
    unsigned long start = (unsigned long)addr;
    unsigned long end = start + size;
    struct page *page;

    for (; start < end; start += PAGE_SIZE) {
        page = virt_to_page((void *)start);
        flush_dcache_page(page);
    }
}
EXPORT_SYMBOL(osal_dcache_region_inv);