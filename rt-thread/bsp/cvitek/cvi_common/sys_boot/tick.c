/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024/01/11     flyingcys    The first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <encoding.h>
#include "rtos_types.h"
#include "tick.h"

#define PLIC_BASE           (0x70000000UL)
#define CORET_BASE          (PLIC_BASE + 0x4000000UL)               /*!< CORET Base Address */
#define PLIC                ((PLIC_Type *)PLIC_BASE)

#define BSP_TIMER_CLK_FREQ 25000000
#define CLINT_BASE (C906_PLIC_PHY_ADDR + 0x4000000UL)

static volatile uint64_t timer_init_value = 0U;
static volatile rt_uint64_t time_elapsed = 0;
static volatile unsigned long tick_cycles = 0;
static volatile rt_uint32_t *mtimecmp_l = (volatile rt_uint32_t *)(CLINT_BASE + 0x4000UL);
static volatile rt_uint32_t *mtimecmp_h = (volatile rt_uint32_t *)(CLINT_BASE + 0x4004UL);

rt_uint64_t __get_MTIME(void)
{
    uint64_t result;

    __asm__ __volatile__("rdtime %0" : "=r"(result));
    return (result);
}

void set_ticks()
{
    uint64_t value = (((uint64_t)*mtimecmp_h) << 32) + (uint64_t)*mtimecmp_l;

    if ((value != 0) && (value != 0xffffffffffffffff)) {
        value = value + (uint64_t)(BSP_TIMER_CLK_FREQ / RT_TICK_PER_SECOND);
    } else {
        value = __get_MTIME() + (uint64_t)(BSP_TIMER_CLK_FREQ / RT_TICK_PER_SECOND);
    }
    *mtimecmp_h = (uint32_t)(value >> 32);
    *mtimecmp_l = (uint32_t)value;
}

rt_uint64_t get_ticks(void)
{
    uint64_t result;
    __asm__ __volatile__("csrr %0, 0xc01" : "=r"(result));
    return result;
}

int rt_hw_tick_isr(void)
{
    set_ticks();
    return 0;
}

/* Sets and enable the timer interrupt */
int rt_hw_tick_init(void)
{
    /* Clear the Machine-Timer bit in MIE */
    clear_csr(mie, MIP_MTIP);
    timer_init_value = get_ticks();
    tick_cycles = BSP_TIMER_CLK_FREQ / RT_TICK_PER_SECOND;

    /* Enable the Machine-Timer bit in MIE */
    set_csr(mie, MIP_MTIP);

    return 0;
}

uint32_t rt_tick_get_ms(void)
{
    return rt_tick_get_us() / 1000;
}

uint32_t rt_tick_get_us(void)
{
    uint32_t time;

    time = (uint32_t)(get_ticks() / (BSP_TIMER_CLK_FREQ / 1000000));
    return time;
}

/**
 * This function will delay for some us.
 *
 * @param us the delay time of us
 */
void rt_hw_us_delay(rt_uint32_t us)
{
    unsigned long start_time;
    unsigned long end_time;
    unsigned long run_time;

    start_time = get_ticks();
    end_time = start_time + us * (BSP_TIMER_CLK_FREQ / 1000000);
    do{
        run_time = get_ticks();
    } while(run_time < end_time);
}

/**
 * This function will delay for some ms.
 *
 * @param us the delay time of ms
 */
void rt_hw_ms_delay(rt_uint32_t ms)
{
    rt_hw_us_delay(1000);
}