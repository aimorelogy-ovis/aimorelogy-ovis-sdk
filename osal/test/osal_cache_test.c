//#include <stdio.h>
#include <osal_test.h>
#include "osal.h"  // Ensure this header provides the necessary OSAL interfaces

void test_osal_dcache_region_wb(void)
{
    void *test_kvirt = osal_kmalloc(64, OSAL_GFP_KERNEL);
	unsigned long test_phys_addr = osal_virt_to_phys(test_kvirt);
    unsigned long test_size = 64;
    if (!test_kvirt) {
        osal_test_log("Failed to allocate memory for test_kvirt\n");
        return;
    }

    osal_dcache_region_wb(test_kvirt, test_phys_addr, test_size);

    osal_test_log("[Success] test_osal_dcache_region_wb.\n");
    osal_kfree(test_kvirt);
}

void test_osal_dcache_region_inv(void)
{
    void *test_addr = osal_kmalloc(128, OSAL_GFP_KERNEL);
    unsigned long test_size = 128;
    if (!test_addr) {
        osal_test_log("Failed to allocate memory for test_addr\n");
        return;
    }

    osal_dcache_region_inv(test_addr, test_size);
    osal_test_log("[Success] test_osal_dcache_region_inv.\n");
    osal_kfree(test_addr);
}

void osal_cache_test(void)
{
    test_osal_dcache_region_wb();
    test_osal_dcache_region_inv();
}
OSAL_CMD_EXPORT(osal_cache_test, Test cache operations);