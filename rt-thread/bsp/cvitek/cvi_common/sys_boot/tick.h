/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024/01/11     flyingcys    The first version
 */

#ifndef __TICK_H__
#define __TICK_H__

#include <rtthread.h>

int tick_isr(void);
int rt_hw_tick_init(void);
int rt_hw_tick_isr(void);
uint32_t rt_tick_get_ms(void);
uint32_t rt_tick_get_us(void);
void rt_hw_irq_isr(void);
void rt_hw_us_delay(rt_uint32_t us);
void rt_hw_ms_delay(rt_uint32_t ms);

#endif /* __TICK_H__ */
