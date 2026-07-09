/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023/06/25     flyingcys    first version
 */

#include <rtthread.h>
#include <pthread.h>
#include <stdio.h>
#include "board.h"
#include "drv_uart.h"
#include "parser.h"
#include "spiflash.h"
#include "core_rv64.h"

#define PARTITION_OFFSET  0x02C000
#define LOAD_DDR_ADRR 0x80050000

void boot_load_and_jump()
{
	int ret;
	ChunkHeader chunk_data;
	void (*func)(void);
	uint32_t imtb_offset = PARTITION_OFFSET;
    uint64_t load_addr = LOAD_DDR_ADRR;
    uint64_t *p_load_addr = &load_addr;

	hal_spiflash_read(imtb_offset,(void *)(*p_load_addr),4*1024);
	ret = parse_imtb_file((uint64_t *)(p_load_addr),&chunk_data);
	if(ret)
	{
		goto fail;
	}

    hal_spiflash_read(chunk_data.offset,(void *)(*p_load_addr),chunk_data.size);

    dcache_clean_invalid();
    icache_invalid();

    func = (void (*)(void))((uint64_t *)load_addr);
    printf("j 0x%08lx\n", (uint64_t)(*func));
    (*func)();

	while(1);

fail:
    printf("jump failed. reboot.\n");
    rt_thread_mdelay(200);
    rt_hw_cpu_reset();
}

int main(void)
{
    rt_hw_board_init();

    hal_spiflash_init();

	boot_load_and_jump();

    while (1)
    {
        rt_thread_mdelay(500);
    }

    return 0;
}
