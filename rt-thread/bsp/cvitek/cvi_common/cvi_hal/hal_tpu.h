/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: tpu_platform.h
 * Description: hw driver header file
 */

#ifndef __TPU_PLATFORM_H__
#define __TPU_PLATFORM_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <rtdbg.h>
#include <list.h>

#include "pthread.h"
#include "rtos_types.h"
#include "semaphore.h"

struct cvi_kernel_work {
    pthread_t       work_thread;
    sem_t           task_wait_sem;
    sem_t           done_wait_sem;
    dlist_t         task_list;
    pthread_mutex_t task_list_lock;
    dlist_t         done_list;
    pthread_mutex_t done_list_lock;
    int             work_run;
};

struct cvi_tpu_device {
    sem_t                  tdma_done;
    uintptr_t              tdma_paddr;  //
    uintptr_t              tiu_paddr;
    int                    tdma_irq;
    pthread_mutex_t        dev_lock;
    pthread_mutex_t        close_lock;
    int                    use_count;
    int                    running_count;
    int                    suspend_count;
    int                    resume_count;
    void                  *private_data;
    struct cvi_kernel_work kernel_work;
    int                    tdma_irq_num;
};

struct CMD_ID_NODE {
    unsigned int bd_cmd_id;
    unsigned int tdma_cmd_id;
};

struct tpu_tdma_pio_info {
    uint64_t paddr_src;
    uint64_t paddr_dst;
    uint32_t h;
    uint32_t w_bytes;
    uint32_t stride_bytes_src;
    uint32_t stride_bytes_dst;
    uint32_t enable_2d;
    uint32_t leng_bytes;
};

enum tpu_msg_prio {
    TPU_ERR     = 0x0001,
    TPU_WARN    = 0x0002,
    TPU_INFO    = 0x0004,
    TPU_DBG     = 0x0008,
    TPU_VB2     = 0x0010,
    TPU_ISP_IRQ = 0x0020,
};

#ifndef TPU_PRINTK_LEVEL
#define TPU_PRINTK_LEVEL 2
#endif

#define tpu_printf(level, fmt, arg...)                                   \
    do {                                                                 \
        if (level <= TPU_PRINTK_LEVEL)                                   \
            rt_kprintf("[%s():%d] " fmt, __FUNCTION__, __LINE__, ##arg); \
    } while (0)

struct TPU_PLATFORM_CFG {
    uintptr_t iomem_tdmaBase;
    uintptr_t iomem_tiuBase;
    uint32_t  pmubuf_size;
    uint64_t  pmubuf_addr_p;
};

struct tpu_tee_load_info {
    // ree
    uint64_t cmdbuf_addr_ree;
    uint32_t cmdbuf_len_ree;
    uint64_t weight_addr_ree;
    uint32_t weight_len_ree;
    uint64_t neuron_addr_ree;

    // tee
    uint64_t dmabuf_addr_tee;
};

struct tpu_tee_submit_info {
    // tee
    uint64_t dmabuf_paddr;
    uint64_t gaddr_base2;
    uint64_t gaddr_base3;
    uint64_t gaddr_base4;
    uint64_t gaddr_base5;
    uint64_t gaddr_base6;
    uint64_t gaddr_base7;
};

enum TPU_SEC_SMCCALL {
    TPU_SEC_SMC_LOADCMD = 0x1001,
    TPU_SEC_SMC_RUN,
    TPU_SEC_SMC_WAIT,
};

#define TPU_IRQ_1 (75)
#define TPU_IRQ_2 (76)
int platform_loadcmdbuf_tee(struct cvi_tpu_device *ndev, struct tpu_tee_load_info *p_info);
int platform_run_dmabuf_tee(struct cvi_tpu_device *ndev, struct tpu_tee_submit_info *p_info);
int platform_unload_tee(struct cvi_tpu_device *ndev, uint64_t paddr, uint64_t size);

void platform_tdma_irq(struct cvi_tpu_device *ndev);
int  platform_run_dmabuf(struct cvi_tpu_device *ndev, void *dmabuf_v, uint64_t dmabuf_p);

int  platform_tpu_suspend(struct cvi_tpu_device *ndev);
int  platform_tpu_resume(struct cvi_tpu_device *ndev);
int  platform_tpu_open(struct cvi_tpu_device *ndev);
int  platform_tpu_reset(struct cvi_tpu_device *ndev);
int  platform_tpu_init(struct cvi_tpu_device *ndev);
void platform_tpu_deinit(struct cvi_tpu_device *ndev);
void platform_tpu_spll_divide(struct cvi_tpu_device *ndev, u32 div);
int  platform_tpu_probe_setting(struct cvi_tpu_device *ndev);
int  platform_run_pio(struct cvi_tpu_device *ndev, struct tpu_tdma_pio_info *info);
void platform_clear_int(struct cvi_tpu_device *ndev);
void disable_tdma_enable_bit(void);

enum TPU_PMUEVENT {
    TPU_PMUEVENT_BANKCONFLICT = 0x0,
    TPU_PMUEVENT_STALLCNT     = 0x1,
    TPU_PMUEVENT_TDMABW       = 0x2,
    TPU_PMUEVENT_TDMAWSTRB    = 0x3,
};

enum TPU_PMUTYPE {
    TPU_PMUTYPE_TDMALOAD  = 1,
    TPU_PMUTYPE_TDMASTORE = 2,
    TPU_PMUTYPE_TDMAMOVE  = 3,
    TPU_PMUTYPE_TIU       = 4,
};

struct TPU_PMU_DOUBLEEVENT {
    u64 type : 4;
    u64 desID : 16;
    u64 eventCnt0 : 22;
    u64 eventCnt1 : 22;
    u32 endTime;
    u32 startTime;
};

#define CPU_ENGINE_DESCRIPTOR_NUM 56

struct cvi_cpu_sync_desc_t {
    u32  op_type;      // CPU_CMD_ACCPI0
    u32  num_bd;       // CPU_CMD_ACCPI1
    u32  num_gdma;     // CPU_CMD_ACCPI2
    u32  offset_bd;    // CPU_CMD_ACCPI3
    u32  offset_gdma;  // CPU_CMD_ACCPI4
    u32  reserved[2];  // CPU_CMD_ACCPI5-CPU_CMD_ACCPI6
    char str[(CPU_ENGINE_DESCRIPTOR_NUM - 7) * sizeof(u32)];
};

int TPUPMU_Enable(struct TPU_PLATFORM_CFG *pCfg, u8 enable, enum TPU_PMUEVENT event);
int TPUPMU_ParsingResult(u8 *pbuf_start);

#define tpu_readl(addr)                                        \
    ({                                                         \
        unsigned int __v = (*(volatile unsigned int *)(addr)); \
        __v;                                                   \
    })

#define tpu_writel(addr, b) ((*(volatile unsigned int *)(addr)) = (b))

#define RAW_READ32(addr)         tpu_readl(addr)
#define RAW_WRITE32(addr, value) tpu_writel(addr, value)

#ifdef __cplusplus
}
#endif
#endif
