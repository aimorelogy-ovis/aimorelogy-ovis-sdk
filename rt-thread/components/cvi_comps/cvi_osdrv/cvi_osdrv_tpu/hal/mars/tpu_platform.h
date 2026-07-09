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

// #include <linux/kernel.h>
// #include <linux/interrupt.h>
// #include "io.h"
#include <debug/dbg.h>
// #include "aos/cli.h"
#include "rtos_types.h"
#include "semaphore.h"
#include "drv/list.h"
#include "pthread.h"

struct cvi_kernel_work {
	pthread_t work_thread;
	sem_t task_wait_sem;
	sem_t done_wait_sem;
	dlist_t task_list;
	pthread_mutex_t task_list_lock;
	dlist_t done_list;
	pthread_mutex_t done_list_lock;
	int work_run;
};

struct cvi_tpu_device {
	// struct device *dev;
	// struct reset_control *rst_tdma;
	// struct reset_control *rst_tpu;
	// struct reset_control *rst_tpusys;
	// struct clk *clk_tpu_axi;
	// struct clk *clk_tpu_fab;
	// dev_t cdev_id;
	// struct cdev cdev;
	sem_t tdma_done;
	uintptr_t tdma_paddr;	//
	uintptr_t tiu_paddr;
	int tdma_irq;
	pthread_mutex_t dev_lock;
	pthread_mutex_t  close_lock;
	int use_count;
	int running_count;
	int suspend_count;
	int resume_count;
	void *private_data;
	struct cvi_kernel_work kernel_work;
	int tdma_irq_num;
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
	TPU_ERR 	= 0x0001,
	TPU_WARN	= 0x0002,
	TPU_INFO	= 0x0004,
	TPU_DBG 	= 0x0008,
	TPU_VB2 	= 0x0010,
	TPU_ISP_IRQ = 0x0020,
};

#ifndef TPU_PRINTK_LEVEL
#define TPU_PRINTK_LEVEL 2
#endif

#define tpu_printf(level, fmt, arg...) \
		do { \
			if (level <= TPU_PRINTK_LEVEL) \
				debug_printf("[%s():%d] " fmt, __FUNCTION__, __LINE__, ## arg); \
		} while (0)

struct TPU_PLATFORM_CFG {
	uintptr_t iomem_tdmaBase;
	uintptr_t iomem_tiuBase;
	uint32_t pmubuf_size;
	uint64_t pmubuf_addr_p;
};

struct tpu_tee_load_info {
	//ree
	uint64_t cmdbuf_addr_ree;
	uint32_t cmdbuf_len_ree;
	uint64_t weight_addr_ree;
	uint32_t weight_len_ree;
	uint64_t neuron_addr_ree;

	//tee
	uint64_t dmabuf_addr_tee;
};

struct tpu_tee_submit_info {
	//tee
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

#define TPU_IRQ_1		(75)
#define TPU_IRQ_2		(76)
int platform_loadcmdbuf_tee(struct cvi_tpu_device *ndev, struct tpu_tee_load_info *p_info);
int platform_run_dmabuf_tee(struct cvi_tpu_device *ndev, struct tpu_tee_submit_info *p_info);
int platform_unload_tee(struct cvi_tpu_device *ndev, uint64_t paddr, uint64_t size);

void platform_tdma_irq(struct cvi_tpu_device *ndev);
int platform_run_dmabuf(struct cvi_tpu_device *ndev, void *dmabuf_v, uint64_t dmabuf_p);

int platform_tpu_suspend(struct cvi_tpu_device *ndev);
int platform_tpu_resume(struct cvi_tpu_device *ndev);
int platform_tpu_open(struct cvi_tpu_device *ndev);
int platform_tpu_reset(struct cvi_tpu_device *ndev);
int platform_tpu_init(struct cvi_tpu_device *ndev);
void platform_tpu_deinit(struct cvi_tpu_device *ndev);
void platform_tpu_spll_divide(struct cvi_tpu_device *ndev, u32 div);
int platform_tpu_probe_setting(struct cvi_tpu_device *ndev);
int platform_run_pio(struct cvi_tpu_device *ndev, struct tpu_tdma_pio_info *info);
void platform_clear_int(struct cvi_tpu_device *ndev);
void disable_tdma_enable_bit(void);
#define tpu_readl(addr) \
    ({ unsigned int __v = (*(volatile unsigned int *) (addr)); __v; })

#define tpu_writel(addr,b) ((*(volatile unsigned int *) (addr)) = (b))

#define RAW_READ32(addr) tpu_readl(addr)
#define RAW_WRITE32(addr, value) tpu_writel(addr,value)

#ifdef __cplusplus
}
#endif
#endif
