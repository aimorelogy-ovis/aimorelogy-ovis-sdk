/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024/01/11     flyingcys    The first version
 */
#include <rtthread.h>
#include <stdio.h>
#include "comm.h"

extern void GetCommInfo(void);
extern void main_create_comm_tasks(void);
int main(void)
{
    rt_kprintf("Hello, RISC-V1!\n");

    GetCommInfo();
    main_create_comm_tasks();

    return 0;
}
