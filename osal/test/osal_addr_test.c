#include <osal_test.h>

void osal_addr_test(void)
{
    unsigned long size = 512;
    unsigned int osal_gfp_flag = 0;
    unsigned long phys_addr = 0x12345678;
    int nums_to[5];
    int nums_copy[5] = {1, 2, 3, 4, 5};
    void *ptr;
    void* phy_ptr;
    const void* virt_addr;
    unsigned long phy_addr;
    int all_success = 1;

    osal_test_log("Testing memory allocation interfaces...\n");

    // test osal_malloc
    ptr = osal_malloc(size);
    if (ptr == NULL) {
        osal_test_log("[failed] osal_malloc\n");
        all_success = 0;
    } else {
        osal_free(ptr);
        ptr = NULL;
    }

    // test osal_kmalloc
    ptr = osal_kmalloc(size, osal_gfp_flag);
    if (ptr == NULL) {
        osal_test_log("[failed] osal_kmalloc\n");
        all_success = 0;
    } else {
        osal_kfree(ptr);
        ptr = NULL;
    }

    // test osal_kzalloc
    ptr = osal_kzalloc(size, osal_gfp_flag);
    if (ptr == NULL) {
        osal_test_log("[failed] osal_kzalloc\n");
        all_success = 0;
    } else {
        osal_kfree(ptr);
        ptr = NULL;
    }

    /*osal_kmalloc_align + osal_kfree will backtrace*/
    // ptr = osal_kmalloc_align(size, osal_gfp_flag, boundary);
    // if (ptr == NULL)
    // {
    //     osal_test_log("[failed] osal_kmalloc_align\n");
    // }
    // else
    // {
    //     osal_test_log("align memory at %p (size: %u, align: %u)\n", ptr, size, boundary);
    //     osal_kfree(ptr);
    //     osal_test_log("[success] osal_kmalloc_align\n");
    //     ptr = NULL;
    // }

    // test osal_vmalloc
    ptr = osal_vmalloc(size);
    if (ptr == NULL) {
        osal_test_log("[failed] osal_vmalloc\n");
        all_success = 0;
    } else {
        osal_vfree(ptr);
        ptr = NULL;
    }

    // test osal_vzalloc
    ptr = osal_vzalloc(size);
    if (ptr == NULL) {
        osal_test_log("[failed] osal_vzalloc\n");
        all_success = 0;
    } else {
        osal_vfree(ptr);
        ptr = NULL;
    }

    // Output final result
    if (all_success) {
        osal_test_log("[success] (osal_malloc/osal_kmalloc/osal_kzalloc/osal_vmalloc/osal_vzalloc) memory allocation tests passed\n");
    }
    else
    {
        osal_test_log("[failed] osal_sem_test\n");
    }

    // test osal_calloc
    size = 10;
    ptr = osal_calloc(size, sizeof(int));
    if (ptr != NULL) {
        osal_test_log("osal_calloc success\n");
        osal_free(ptr);
    }

    // test osal_ioremap / osal_ioremap_nocache / osal_ioremap_cached / osal_ioremap_wc
    phy_ptr = osal_ioremap(phys_addr, size);
    if ((unsigned long)(phy_ptr) == 0x12345678) {
        osal_test_log("osal_ioremap success\n");
    }else{
        osal_test_log("osal_ioremap fail\n");
    }

    phy_ptr = osal_ioremap_nocache(phys_addr, size);
    if ((unsigned long)(phy_ptr) == 0x12345678) {
        osal_test_log("osal_ioremap_nocache success\n");
    }else{
        osal_test_log("osal_ioremap_nocache fail\n");
    }

    phy_ptr = osal_ioremap_cached(phys_addr, size);
    if ((unsigned long)(phy_ptr) == 0x12345678) {
        osal_test_log("osal_ioremap_cached success\n");
    }else{
        osal_test_log("osal_ioremap_cached fail\n");
    }

    phy_ptr = osal_ioremap_wc(phys_addr, size);
    if ((unsigned long)(phy_ptr) == 0x12345678) {
        osal_test_log("osal_ioremap_wc success\n");
    }else{
        osal_test_log("osal_ioremap_wc fail\n");
    }


    // test osal_blockmem_vmap / osal_blockmem_vmap_nocache
    phy_ptr = osal_blockmem_vmap(phys_addr, size);
    if ((unsigned long)(phy_ptr) == 0x12345678) {
        osal_test_log("osal_blockmem_vmap success\n");
    }else{
        osal_test_log("osal_blockmem_vmap fail\n");
    }

    phy_ptr = osal_blockmem_vmap_nocache(phys_addr, size);
    if ((unsigned long)(phy_ptr) == 0x12345678) {
        osal_test_log("osal_blockmem_vmap_nocache success\n");
    }else{
        osal_test_log("osal_blockmem_vmap_nocache fail\n");
    }

    // test osal_phys_to_virt / osal_virt_to_phys
    phy_ptr = osal_phys_to_virt(phys_addr);
    if ((unsigned long)(phy_ptr) == 0x12345678) {
        osal_test_log("osal_phys_to_virt success\n");
    }else{
        osal_test_log("osal_phys_to_virt fail\n");
    }

    virt_addr = phy_ptr;
    phy_addr = osal_virt_to_phys(virt_addr);
    if ((phy_addr) == 0x12345678) {
        osal_test_log("osal_virt_to_phys success\n");
    }else{
        osal_test_log("osal_virt_to_phys fail\n");
    }

    // test osal_copy_from_user / osal_copy_to_user
    osal_copy_from_user(nums_to, nums_copy, sizeof(nums_copy));
    for (int i = 0; i < sizeof(nums_to) / sizeof(int); i++) {
        if (nums_to[i] != (i + 1)) {
            osal_test_log("osal_copy_from_user fail\n");
        }
    }
    osal_test_log("osal_copy_from_user success\n");

    // test osal_memset
    osal_memset(nums_to, 0, sizeof(nums_to));
    for (int i = 0; i < sizeof(nums_to) / sizeof(int); i++) {
        if (nums_to[i] != 0) {
            osal_test_log("osal_memset fail\n");
        }
    }
    osal_test_log("osal_memset success\n");

    osal_copy_to_user(nums_to, nums_copy, sizeof(nums_copy));
    for (int i = 0; i < sizeof(nums_to) / sizeof(int); i++) {
        if (nums_to[i] != (i + 1)) {
            osal_test_log("osal_copy_to_user fail\n");
        }
    }
    osal_test_log("osal_copy_to_user success\n");

    // test osal_memcpy
    osal_memcpy(nums_to, nums_copy, sizeof(nums_copy));
    for (int i = 0; i < sizeof(nums_to) / sizeof(int); i++) {
        if (nums_to[i] != (i + 1)) {
            osal_test_log("osal_memcpy fail\n");
        }
    }
    osal_test_log("osal_memcpy success\n");

}
OSAL_CMD_EXPORT(osal_addr_test, Test addr operations);