// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: cvi_spinlock.c
 * Description:
 */
#define RTOS_BSP 1
#ifdef RTOS_BSP
#include "stdint.h"
#include <stdio.h>
// #include "types.h"
#include "csr.h"
#include "csi_rv64_gcc.h"
#include "core_rv64.h"
#include "arch_time.h"
#include "top_reg.h"
#include "drv_spinlock.h"
// #include "delay.h"
// #include "tick.h"
#include "mmio.h"
#include <rtthread.h>
#else
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/of_reserved_mem.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/io.h>
#include "cvi_spinlock.h"
#endif

#ifndef RTOS_BSP
static unsigned long reg_base;
#else
static unsigned long reg_base = SPINLOCK_REG_BASE;
#endif

// #include "semphr.h"
// SemaphoreHandle_t    reg_write_lock          = NULL;
struct rt_semaphore  reg_write_lock;

static unsigned char lockCount[SPIN_MAX + 1] = { 0 };

void cvi_spinlock_init(void)
{
	rt_err_t ret;
	ret = rt_sem_init(&reg_write_lock, "reg_wr", 1, RT_IPC_FLAG_FIFO);
	if (ret != RT_EOK) {
		rt_kprintf("rt_sem_init failed!\n");
	} else {
		rt_kprintf("[%s] success\n", __func__);
	}
}

void cvi_spinlock_deinit(void)
{
	rt_err_t ret;

	ret = rt_sem_detach(&reg_write_lock);
	if (ret != RT_EOK) {
		rt_kprintf("rt_sem_detach failed!\n");
	}
}
void spinlock_base(unsigned long mb_base)
{
	reg_base = mb_base;
}

static inline int hw_spin_trylock(hw_raw_spinlock_t *lock)
{
#ifndef RISCV_QEMU
	if (readw((void *)(reg_base + sizeof(int) * lock->hw_field)) != 0) {
		return MAILBOX_LOCK_FAILED;
	}
	writew(lock->locks, reg_base + sizeof(int) * lock->hw_field);
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	if (readw(reg_base + sizeof(int) * lock->hw_field) == lock->locks)
		return MAILBOX_LOCK_SUCCESS;
	return MAILBOX_LOCK_FAILED;
#else
	return MAILBOX_LOCK_SUCCESS;
#endif
}

int hw_spin_lock(hw_raw_spinlock_t *lock)
{
	u64               i;
	u64               loops = 1000000;
	hw_raw_spinlock_t _lock = { .hw_field = lock->hw_field,
								.locks    = lock->locks };

	if (lock->hw_field >= SPIN_LINUX_RTOS) {
		if (rt_sem_take(&reg_write_lock, RT_WAITING_FOREVER) != RT_EOK) {
			rt_kprintf("rt_sem_take failed!\n");
			return MAILBOX_LOCK_FAILED;
		}
		if (lockCount[lock->hw_field] == 0) {
			lockCount[lock->hw_field]++;
		}
		_lock.locks = (lockCount[lock->hw_field] << 8);
		lockCount[lock->hw_field]++;
		rt_sem_release(&reg_write_lock);
	} else {
		unsigned long systime = GetSysTime();
		/* lock ID can not be 0, so set it to 1 at least */
		if ((systime & 0xFFFF) == 0)
			systime = 1;
		lock->locks = (unsigned short)(systime & 0xFFFF);
	}
	for (i = 0; i < loops; i++) {
		if (hw_spin_trylock(&_lock) == MAILBOX_LOCK_SUCCESS) {
			lock->locks = _lock.locks;
			return MAILBOX_LOCK_SUCCESS;
		}
		rt_hw_us_delay(1);
	}

// #ifdef RTOS_BSP
	rt_kprintf("__spin_lock_debug fail\n");
	return MAILBOX_LOCK_FAILED;
}

int _hw_raw_spin_lock_irqsave(hw_raw_spinlock_t *lock)
{
	int flag = 0;

#ifdef RTOS_BSP
	// save and disable irq
	flag = (__get_MSTATUS() & 8);
	__disable_irq();
#endif

	// lock
	if (hw_spin_lock(lock) == MAILBOX_LOCK_FAILED) {
#ifdef RTOS_BSP
		// if spinlock failed , restore irq
		if (flag) {
			__enable_irq();
		}
#endif
		// uart_puts("spin lock fail! reg_val=0x%x, lock->locks=0x%x\n",
		//           readw(reg_base + sizeof(int) * lock->hw_field), lock->locks);
		return MAILBOX_LOCK_FAILED;
	}
	return flag;
}

void _hw_raw_spin_unlock_irqrestore(hw_raw_spinlock_t *lock, int flag)
{
#ifndef RISCV_QEMU
	// unlock
	if (readw(reg_base + sizeof(int) * lock->hw_field) == lock->locks) {
		writew(lock->locks, reg_base + sizeof(int) * lock->hw_field);

#ifdef RTOS_BSP
		// restore irq
		if (flag) {
			__enable_irq();
		}
#endif
	} else {
// #ifdef RTOS_BSP
//         uart_puts("spin unlock fail! reg_val=0x%x, lock->locks=0x%x\n",
//                   readw(reg_base + sizeof(int) * lock->hw_field), lock->locks);
// #else
//         pr_err("spin unlock fail\n");
// #endif
	}
#else
	if (flag) {
		__enable_irq();
	}

#endif
}
