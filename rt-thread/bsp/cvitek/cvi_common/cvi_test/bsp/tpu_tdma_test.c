// #include "aos/kernel.h"
#include <pthread.h>
#include <rtdbg.h>
#include <rtthread.h>
#include <semaphore.h>
#include <stdlib.h>

#include "drv_tpu.h"
#include "hal_barrier.h"
#include "hal_tpu.h"
#define TEST_NUM 20

int cvi_tpu_tdma_test_multi_times(void)
{
    struct cvi_tpu_device *ndev = NULL;

    ndev = cvi_tpu_open();
    if (ndev == NULL) {
        rt_kprintf("open tpu device failed!\r\n");
        return -1;
    }

    rt_kprintf("tdmaBaseAddr = 0x%x , tiuBaseAddr = 0x%x\r\n", ndev->tdma_paddr, ndev->tiu_paddr);

    char                    *paddr_src[TEST_NUM];
    char                    *paddr_dst[TEST_NUM];
    struct cvi_tdma_copy_arg pioCfg[TEST_NUM];
    struct cvi_cache_op_arg  opCfg;
    char                    *p, *q;
    for (uint32_t pioCfgIndex = 0; pioCfgIndex < TEST_NUM; pioCfgIndex++) {
        pioCfg[pioCfgIndex].enable_2d  = 0;
        pioCfg[pioCfgIndex].seq_no     = pioCfgIndex;
        pioCfg[pioCfgIndex].leng_bytes = 1024;
        paddr_src[pioCfgIndex]         = (char *)rt_malloc(pioCfg[pioCfgIndex].leng_bytes + 0x3f);
        paddr_dst[pioCfgIndex]         = (char *)rt_malloc(pioCfg[pioCfgIndex].leng_bytes + 0x3f);
        if (paddr_dst[pioCfgIndex] == NULL || paddr_src[pioCfgIndex] == NULL) {
            rt_kprintf("rt_malloc src/dst failed!\r\n");
            return -1;
        }
        pioCfg[pioCfgIndex].paddr_dst = (uintptr_t)paddr_dst[pioCfgIndex];
        pioCfg[pioCfgIndex].paddr_src = (uintptr_t)paddr_src[pioCfgIndex];
        // 64 byte align
        pioCfg[pioCfgIndex].paddr_dst = (pioCfg[pioCfgIndex].paddr_dst + 0x3f) & (~0x3f);
        pioCfg[pioCfgIndex].paddr_src = (pioCfg[pioCfgIndex].paddr_src + 0x3f) & (~0x3f);
        p                             = (char *)pioCfg[pioCfgIndex].paddr_src;
        for (uint32_t tmpIndex = 0; tmpIndex < pioCfg[pioCfgIndex].leng_bytes; tmpIndex++) {
            p[tmpIndex] = 'A' + pioCfgIndex;
        }

        // tpu cache flush
        opCfg.paddr = pioCfg[pioCfgIndex].paddr_src;
        opCfg.size  = pioCfg[pioCfgIndex].leng_bytes;
        cvi_tpu_ioctl(ndev, CVITPU_DMABUF_FLUSH, (unsigned long)&opCfg);

        opCfg.paddr = pioCfg[pioCfgIndex].paddr_dst;
        opCfg.size  = pioCfg[pioCfgIndex].leng_bytes;
        cvi_tpu_ioctl(ndev, CVITPU_DMABUF_INVLD, (unsigned long)&opCfg);

        // tpu tdma process
        rt_kprintf("platform_run_pio()\r\n");
        cvi_tpu_ioctl(ndev, CVITPU_SUBMIT_PIO, (unsigned long)&(pioCfg[pioCfgIndex]));
    }
    rt_kprintf("CVITPU_WAIT_PIO\r\n");

    // wait for pio done
    int                      result  = 0;
    struct cvi_tdma_wait_arg waitCfg = {.seq_no = 1, .ret = 0};
    for (uint32_t pioCfgIndex = 0; pioCfgIndex < TEST_NUM; pioCfgIndex++) {
        waitCfg.seq_no = pioCfgIndex;
        waitCfg.ret    = 0;
        rt_kprintf("CVITPU_WAIT_PIO\r\n");
        cvi_tpu_ioctl(ndev, CVITPU_WAIT_PIO, (unsigned long)&waitCfg);
        // tpu release
        // tpu dma invalidate
        opCfg.paddr = pioCfg[pioCfgIndex].paddr_dst;
        opCfg.size  = pioCfg[pioCfgIndex].leng_bytes;

        rt_kprintf("CVITPU_DMABUF_INVLD\r\n");
        cvi_tpu_ioctl(ndev, CVITPU_DMABUF_INVLD, (unsigned long)&opCfg);
        rt_kprintf("CVITPU_DMABUF_INVLD end\r\n");
        // printf dst data
        p = (char *)pioCfg[pioCfgIndex].paddr_src;
        q = (char *)pioCfg[pioCfgIndex].paddr_dst;

        for (uint32_t tmpIndex = 0; tmpIndex < pioCfg[pioCfgIndex].leng_bytes; tmpIndex++) {
            if (p[tmpIndex] != q[tmpIndex]) {
                rt_kprintf("[%s %d] [%d\t%d]:%d->%d error\r\n", __FUNCTION__, __LINE__, pioCfgIndex,
                           tmpIndex, p[tmpIndex], q[tmpIndex]);
                result = 1;
            } else {
                rt_kprintf("[%s %d] [%d\t%d]:%d->%d success\r\n", __FUNCTION__, __LINE__,
                           pioCfgIndex, tmpIndex, p[tmpIndex], q[tmpIndex]);
            }
        }
        rt_kprintf("\r\n");
    }
    if (result)
        rt_kprintf("test error\r\n");
    else
        rt_kprintf("test success\r\n");
    for (uint32_t pioCfgIndex = 0; pioCfgIndex < TEST_NUM; pioCfgIndex++) {
        if (NULL != paddr_src[pioCfgIndex]) rt_free(paddr_src[pioCfgIndex]);
        if (NULL != paddr_dst[pioCfgIndex]) rt_free(paddr_dst[pioCfgIndex]);
    }
    cvi_tpu_close();

    return 0;
}

int cvi_tpu_tdma_test_once(void)
{
    struct cvi_tpu_device *ndev = NULL;

    ndev = cvi_tpu_open();
    if (ndev == NULL) {
        rt_kprintf("open tpu device failed!\r\n");
        return -1;
    }
    rt_kprintf("tdmaBaseAddr = 0x%lx , tiuBaseAddr = 0x%lx\r\n", ndev->tdma_paddr, ndev->tiu_paddr);

    // set pio cfg
    struct cvi_tdma_copy_arg pioCfg;
    pioCfg.enable_2d  = 0;
    pioCfg.seq_no     = 1;
    pioCfg.leng_bytes = 1024;
    char *paddr_src   = (char *)rt_malloc(pioCfg.leng_bytes + 0x3f);
    char *paddr_dst   = (char *)rt_malloc(pioCfg.leng_bytes + 0x3f);
    rt_kprintf("[%s %d] paddr_src=0x%x  paddr_dst=0x%x\r\n", __FUNCTION__, __LINE__, paddr_src,
               paddr_dst);

    if (paddr_dst == NULL || paddr_src == NULL) {
        rt_kprintf("rt_malloc src/dst failed!\r\n");
        return -1;
    }
    pioCfg.paddr_dst = (uintptr_t)paddr_dst;
    pioCfg.paddr_src = (uintptr_t)paddr_src;
    // 64 byte align
    pioCfg.paddr_dst = (pioCfg.paddr_dst + 0x3f) & (~0x3f);
    pioCfg.paddr_src = (pioCfg.paddr_src + 0x3f) & (~0x3f);
    rt_kprintf("[%s %d] pioCfg.paddr_src=0x%x  pioCfg.paddr_dst=0x%x\r\n", __FUNCTION__, __LINE__,
               pioCfg.paddr_src, pioCfg.paddr_dst);
    char *p = (char *)pioCfg.paddr_src;
    for (uint32_t tmpIndex = 0; tmpIndex < pioCfg.leng_bytes; tmpIndex++) {
        p[tmpIndex] = tmpIndex % 200;
    }

    // tpu cache flush
    struct cvi_cache_op_arg opCfg;
    opCfg.paddr = pioCfg.paddr_src;
    opCfg.size  = pioCfg.leng_bytes;
    cvi_tpu_ioctl(ndev, CVITPU_DMABUF_FLUSH, (unsigned long)&opCfg);
    opCfg.paddr = pioCfg.paddr_dst;
    opCfg.size  = pioCfg.leng_bytes;
    cvi_tpu_ioctl(ndev, CVITPU_DMABUF_INVLD, (unsigned long)&opCfg);

    // tpu tdma process
    rt_kprintf("CVITPU_SUBMIT_PIO\r\n");
    cvi_tpu_ioctl(ndev, CVITPU_SUBMIT_PIO, (unsigned long)&pioCfg);
    // wait for pio done
    struct cvi_tdma_wait_arg waitCfg = {.seq_no = 1, .ret = 0};
    rt_kprintf("CVITPU_WAIT_PIO\r\n");
    cvi_tpu_ioctl(ndev, CVITPU_WAIT_PIO, (unsigned long)&waitCfg);
    // tpu release
    // tpu dma invalidate
    opCfg.paddr = pioCfg.paddr_dst;
    opCfg.size  = pioCfg.leng_bytes;
    rt_kprintf("CVITPU_DMABUF_INVLD\r\n");
    cvi_tpu_ioctl(ndev, CVITPU_DMABUF_INVLD, (unsigned long)&opCfg);
    // printf dst data
    char *q    = (char *)pioCfg.paddr_src;
    p          = (char *)pioCfg.paddr_dst;
    int result = 0;
    for (uint32_t tmpIndex = 0; tmpIndex < pioCfg.leng_bytes; tmpIndex++) {
        if (q[tmpIndex] != p[tmpIndex]) result = 1;
        rt_kprintf("[%d]:%d->%d\t", tmpIndex, q[tmpIndex], p[tmpIndex]);
        if (tmpIndex % 5 == 0) rt_kprintf("\r\n");
    }
    if (result)
        rt_kprintf("\r\ntest error\r\n");
    else
        rt_kprintf("\r\ntest success\r\n");
    if (NULL != paddr_src) rt_free(paddr_src);
    if (NULL != paddr_dst) rt_free(paddr_dst);
    cvi_tpu_close();
    return 0;
}

void cvi_tpu_test(int32_t argc, char **argv)
{
    if (2 != argc) {
        rt_kprintf("para error\r\n");
    } else if (1 == atoi(argv[1])) {
        cvi_tpu_tdma_test_once();
    } else if (2 == atoi(argv[1])) {
        cvi_tpu_tdma_test_Multi_times();
    } else {
        rt_kprintf("para error\r\n");
    }
}

MSH_CMD_EXPORT(cvi_tpu_test, tpu tdma);
