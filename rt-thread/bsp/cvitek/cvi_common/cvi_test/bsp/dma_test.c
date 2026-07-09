#include <rtthread.h>
#include <rtdevice.h>
#include "drv_dma.h"


#define DMA_MEM2MEM_TEST_SIZE  (1024)
#define DMA_MEM2MEM_TEST_VALUE (0x5A)

static rt_sem_t* dma_sem;

static void dma_test_callback(hal_dma_ch_t *dma_ch, dma_event_t event,
                              void *arg)
{
    if (event == DMA_EVENT_TRANSFER_DONE) {
        rt_sem_release(dma_sem);
    }
}

void dma_mem2mem_test(int argc, char **argv)
{
    void               *src_addr  = NULL;
    void               *dst_addr  = NULL;
    int                 ret       = 0;
    bool                test_pass = true;
    hal_dma_ch_t        dma_ch;
    hal_dma_ch_config_t config;
    dma_sem = rt_sem_create("dma_sem", 0, RT_IPC_FLAG_FIFO);
    if (dma_sem == RT_NULL) {
        rt_kprintf("Failed to create semaphore.\n");
        return;
    }

    src_addr = rt_malloc(DMA_MEM2MEM_TEST_SIZE);
    if (!src_addr) {
        rt_kprintf("Failed to allocate source memory.\n");
        goto free_sem;
    }

    dst_addr = rt_malloc(DMA_MEM2MEM_TEST_SIZE);
    if (!dst_addr) {
        rt_kprintf("Failed to allocate destination memory.\n");
        rt_free(src_addr);
        goto free_sem;
    }
	rt_kprintf("src_addr=%p, dst_addr=%p\r\n", src_addr, dst_addr);
    memset(src_addr, DMA_MEM2MEM_TEST_VALUE, DMA_MEM2MEM_TEST_SIZE);
    memset(dst_addr, 0, DMA_MEM2MEM_TEST_SIZE);

	printf("--- Source Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)src_addr)[i]);
    }
    printf("\n");

    printf("--- Destination Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)dst_addr)[i]);
    }
    printf("\n");

    ret = hal_dma_channel_request(&dma_ch, 4, 0);
    if (ret != 0) {
        rt_kprintf("Failed to allocate DMA channel.\n");
        test_pass = false;
        goto free_mem;
    }

    config.src_inc   = DMA_ADDR_INC;
    config.dst_inc   = DMA_ADDR_INC;
    config.src_tw    = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw    = DMA_DATA_WIDTH_32_BITS;
    config.trans_dir = DMA_MEM2MEM;
    config.group_len = 16;
    config.handshake = 0;
	config.src_reload_en = 0;
	config.dst_reload_en = 0;
	config.half_int_en   = 0;
	config.lli_src_en    = 0;

    hal_dma_ch_config(&dma_ch, &config);
    // dma_ch.user_data = &dma_sem; // Pass semaphore to callback
    hal_dma_ch_attach_callback(&dma_ch, dma_test_callback, NULL);
    hal_dma_ch_start(&dma_ch, src_addr, dst_addr, DMA_MEM2MEM_TEST_SIZE);

    if (rt_sem_take(dma_sem, 5000) != RT_EOK) {
        rt_kprintf("DMA transfer timeout.\n");
        hal_dma_ch_stop(&dma_ch);
        test_pass = false;
    }

    if (test_pass && memcmp(src_addr, dst_addr, DMA_MEM2MEM_TEST_SIZE) != 0) {
        rt_kprintf("Memory comparison failed.\n");
        test_pass = false;
    }

    printf("--- end transfer ---\n");
    printf("--- Source Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)src_addr)[i]);
    }
    printf("\n");

    printf("--- Destination Buffer (first 32 bytes) ---\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x ", ((uint8_t *)dst_addr)[i]);
    }
    printf("\n");
    hal_dma_ch_detach_callback(&dma_ch);
    hal_dma_ch_free(&dma_ch);

free_mem:
    rt_free(src_addr);
    rt_free(dst_addr);
free_sem:
    rt_sem_delete(dma_sem);

    if (test_pass) {
        rt_kprintf("DMA mem2mem test PASSED.\n");
    } else {
        rt_kprintf("DMA mem2mem test FAILED.\n");
    }
}

MSH_CMD_EXPORT(dma_mem2mem_test, DMA mem2mem test e.g: dma_mem2mem_test);