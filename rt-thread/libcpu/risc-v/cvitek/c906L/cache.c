/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-29     lizhirui     first version
 * 2021-11-05     JasonHu      add c906 cache inst
 * 2022-11-09     WangXiaoyao  Support cache coherence operations;
 *                             improve portability and make
 *                             no assumption on undefined behavior
 */

#include <rthw.h>
#include <rtdef.h>
#include <board.h>
// #include <riscv.h>
#include <csi_rv64_gcc.h>
#include "opcode.h"
#include "cache.h"
#include <rtthread.h>

#define L1_CACHE_BYTES (64)

/**
 * GCC version not support t-head cache flush, so we use fixed code to achieve.
 * The following function cannot be optimized.
 */
static void dcache_wb_range(unsigned long start, unsigned long end)
		__attribute__((optimize("O0")));
static void dcache_inv_range(unsigned long start, unsigned long end)
		__attribute__((optimize("O0")));
static void dcache_wbinv_range(unsigned long start, unsigned long end)
		__attribute__((optimize("O0")));
static void icache_inv_range(unsigned long start, unsigned long end)
		__attribute__((optimize("O0")));

#define CACHE_OP_RS1 %0
#define CACHE_OP_RANGE(instr)                                                  \
	{                                                                          \
		register rt_ubase_t i = start & ~(L1_CACHE_BYTES - 1);                 \
		for (; i < end; i += L1_CACHE_BYTES) {                                 \
			__asm__ volatile(instr ::"r"(i) : "memory");                       \
		}                                                                      \
	}

static void dcache_wb_range(unsigned long start, unsigned long end)
{
	CACHE_OP_RANGE(OPC_DCACHE_CVA(CACHE_OP_RS1));
}

static void dcache_inv_range(unsigned long start, unsigned long end)
{
	CACHE_OP_RANGE(OPC_DCACHE_IVA(CACHE_OP_RS1));
}

static void dcache_wbinv_range(unsigned long start, unsigned long end)
{
	CACHE_OP_RANGE(OPC_DCACHE_CIVA(CACHE_OP_RS1));
}

static void icache_inv_range(unsigned long start, unsigned long end)
{
	CACHE_OP_RANGE(OPC_ICACHE_IVA(CACHE_OP_RS1));
}

rt_inline rt_uint32_t rt_cpu_icache_line_size(void)
{
	return L1_CACHE_BYTES;
}

rt_inline rt_uint32_t rt_cpu_dcache_line_size(void)
{
	return L1_CACHE_BYTES;
}

void rt_hw_cpu_icache_invalidate(void *addr, int size)
{
	icache_inv_range((unsigned long)addr,
					 (unsigned long)((unsigned char *)addr + size));
	rt_hw_cpu_sync_i();
}

void rt_hw_cpu_dcache_invalidate(void *addr, int size)
{
	dcache_inv_range((unsigned long)addr,
					 (unsigned long)((unsigned char *)addr + size));
	rt_hw_cpu_sync();
}

void rt_hw_cpu_dcache_clean(void *addr, int size)
{
	dcache_wb_range((unsigned long)addr,
					(unsigned long)((unsigned char *)addr + size));
	rt_hw_cpu_sync();
}

void rt_hw_cpu_dcache_clean_and_invalidate(void *addr, int size)
{
	dcache_wbinv_range((unsigned long)addr,
					   (unsigned long)((unsigned char *)addr + size));
	rt_hw_cpu_sync();
}

/**
 * =====================================================
 * Architecture Independent API
 * =====================================================
 */

void rt_hw_cpu_icache_ops(int ops, void *addr, int size)
{
	if (ops == RT_HW_CACHE_INVALIDATE) {
		rt_hw_cpu_icache_invalidate(addr, size);
	}
}

void rt_hw_cpu_dcache_ops(int ops, void *addr, int size)
{
	if (ops == RT_HW_CACHE_FLUSH) {
		rt_hw_cpu_dcache_clean(addr, size);
	} else {
		rt_hw_cpu_dcache_invalidate(addr, size);
	}
}

void rt_hw_sync_cache(void *addr, int size)
{
	rt_hw_cpu_dcache_clean(addr, size);
	rt_hw_cpu_icache_invalidate(addr, size);
}

#define TEST_SIZE       128
#define CACHE_LINE_SIZE 32 // 根据实际CPU修改

static void single_core_cache_test(void)
{
    rt_kprintf("\n==== Single Core Cache Test ====\n");

    /* 1. 创建测试缓冲区 */
    uint8_t *buffer = rt_malloc_align(TEST_SIZE, CACHE_LINE_SIZE);
    if (!buffer) {
        rt_kprintf("[ERROR] Memory allocation failed!\n");
        return;
    }

    /* 2. 初始化缓冲区 */
    for (int i = 0; i < TEST_SIZE; i++) {
        buffer[i] = 0x55;
    }
    rt_hw_cpu_dcache_clean(buffer, CACHE_LINE_SIZE); // 刷缓存到内存
    rt_kprintf("[INIT] Buffer initialized to 0\n");
    // rt_hw_mb();
    /* 3. 测试不刷缓存的情况 */
    rt_kprintf("\n[TEST 1] Without cache operations:\n");
    buffer[0] = 0xAA; // 修改缓存
    rt_kprintf("  - Set buffer[0] = 0xAA (cache only)\n");
    rt_kprintf("  - before invalidate: buffer[0] = 0x%02X\n",
               *((volatile uint8_t *)buffer)); // // 读缓存
    rt_hw_cpu_dcache_invalidate(buffer, CACHE_LINE_SIZE); // 使缓存失效
    rt_kprintf("  - After invalidate: buffer[0] = 0x%02X\n",
               *((volatile uint8_t *)buffer)); // 绕过缓存加载内存值

    /* 4. 测试刷缓存的效果 */
    rt_kprintf("\n[TEST 2] With rt_hw_cpu_dcache_clean():\n");
    buffer[0] = 0xBB; // 修改缓存
    rt_kprintf("  - Actual memory value: 0x%02X\n",
               *((volatile uint8_t *)buffer)); // 读缓存
    rt_hw_cpu_dcache_clean(buffer, CACHE_LINE_SIZE); // 刷缓存到内存
    rt_kprintf("  - Set buffer[0] = 0xBB & cleaned cache\n");
    rt_kprintf("  - Actual memory value: 0x%02X\n",
               *((volatile uint8_t *)buffer)); // 读缓存

    /* 5. 测试缓存失效的效果 */
    rt_kprintf("\n[TEST 3] With rt_hw_cpu_dcache_invalidate():\n");
    buffer[0] = 0xCC; // 修改缓存
    rt_kprintf("  - Set buffer[0] = 0xCC (cache only)\n");
    rt_hw_cpu_dcache_invalidate(buffer, CACHE_LINE_SIZE); // 使缓存失效
    rt_kprintf("  - After invalidate: buffer[0] = 0x%02X\n",
               *((volatile uint8_t *)buffer)); // 重新加载内存值

    /* 6. 测试组合操作 */
    rt_kprintf("\n[TEST 4] Clean & Invalidate sequence:\n");
    buffer[0] = 0xDD; // 修改缓存
    rt_hw_cpu_dcache_clean(buffer, CACHE_LINE_SIZE); // 刷到内存
    buffer[0] = 0xEE; // 再次修改缓存
    rt_hw_cpu_dcache_invalidate(buffer, CACHE_LINE_SIZE); // 使缓存失效
    rt_kprintf("  - Final value: buffer[0] = 0x%02X\n", buffer[0]);
    buffer[0] = 0xEE; // 重新修改缓存，上一次的EE值已经被清理
    rt_hw_cpu_dcache_clean(buffer, CACHE_LINE_SIZE); // 刷到内存
    buffer[0] = 0xFF; // 再次修改缓存
    rt_hw_cpu_dcache_invalidate(buffer, CACHE_LINE_SIZE); // 使缓存失效
    rt_kprintf("  - Final value: buffer[0] = 0x%02X\n",
               *((volatile uint8_t *)buffer));
    /* 7. 清理资源 */
    rt_free_align(buffer);

    rt_kprintf("\n==== Test Completed ====\n");
}

/* 导出到 msh 命令 */
MSH_CMD_EXPORT(single_core_cache_test, Single core cache operation test);