/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024/01/11     flyingcys    The first version
 */
#include "comm.h"
#include "comm_def.h"
#include <stdio.h>
#include <string.h>
#include "drv_rtoscmu.h"
#include "drv_spinlock.h"
#include "drv_mailbox.h"
#include "drv_dump_uart.h"

#include <string.h>
// #define __DEBUG__
#ifdef __DEBUG__
#define debug_printf rt_kprintf
#else
#define debug_printf(...)
#endif

extern struct transfer_config_t transfer_config;
extern struct trace_snapshot_t  snapshot;

volatile struct mailbox_set_register  *mbox_reg;
volatile struct mailbox_done_register *mbox_done_reg;
volatile unsigned long *mailbox_context; // mailbox buffer context is 64 Bytess

DEFINE_CVI_SPINLOCK(mailbox_lock, SPIN_MBOX);

TASK_CTX_S gTaskCtx[E_QUEUE_MAX] = {
	{
			.name       = "ISP",
			.stack_size = RT_STACK_SIZE_MIN * 8,
			.priority   = RT_PRIORITY_BASE + 3,
			.runTask    = RT_NULL,
			.queLength  = 1,
			.queHandle  = RT_NULL,
	},
	{
			.name       = "VCODEC",
			.stack_size = RT_STACK_SIZE_MIN,
			.priority   = RT_PRIORITY_BASE + 3,
			.runTask    = RT_NULL,
			.queLength  = 1,
			.queHandle  = RT_NULL,
	},
	{
			.name       = "VI",
			.stack_size = RT_STACK_SIZE_MIN,
			.priority   = RT_PRIORITY_BASE + 3,
			.runTask    = RT_NULL,
			.queLength  = 1,
			.queHandle  = RT_NULL,
	},
	{
			.name       = "CAMERA",
			.stack_size = RT_STACK_SIZE_MIN,
			.priority   = RT_PRIORITY_BASE + 3,
			.runTask    = RT_NULL,
			.queLength  = 1,
			.queHandle  = RT_NULL,
	},
	{
			.name       = "CMDQU",
			.stack_size = RT_STACK_SIZE_MIN * 10,
			.priority   = RT_PRIORITY_BASE + 5,
			.runTask    = prvCmdQuRunTask,
			.queLength  = 30,
			.queHandle  = RT_NULL,
	},
	{
			.name       = "AUDIO",
			.stack_size = RT_STACK_SIZE_MIN * 15,
			.priority   = RT_PRIORITY_BASE + 3,
			.runTask    = prvAudioRunTask,
			.queLength  = 10,
			.queHandle  = RT_NULL,
	},
	{
		.name       = "RGN",
		.stack_size = RT_STACK_SIZE_MIN * 5,
		.priority   = RT_PRIORITY_BASE + 3,
		.runTask    = prvRGNRunTask,
		.queLength  = 10,
		.queHandle  = RT_NULL,
	},
};

#define TASK_INIT(_idx)                                                        \
	do {                                                                       \
		gTaskCtx[_idx].queHandle =                                             \
				rt_mq_create(gTaskCtx[_idx].name, sizeof(cmdqu_t),             \
							 gTaskCtx[_idx].queLength * sizeof(cmdqu_t),       \
							 RT_IPC_FLAG_FIFO);                                \
		if (gTaskCtx[_idx].queHandle && gTaskCtx[_idx].runTask) {              \
			rt_thread_t tid = rt_thread_create(                                \
					gTaskCtx[_idx].name, gTaskCtx[_idx].runTask, RT_NULL,      \
					gTaskCtx[_idx].stack_size, gTaskCtx[_idx].priority, 10);   \
			if (tid)                                                           \
				rt_thread_startup(tid);                                        \
		}                                                                      \
	} while (0)

rt_mq_t main_GetMODHandle(QUEUE_HANDLE_E handle_idx)
{
	if (handle_idx >= E_QUEUE_MAX)
		return NULL;
	return gTaskCtx[handle_idx].queHandle;
}

void main_create_comm_tasks(void)
{
	rt_hw_interrupt_install(BSP_MAILBOX_IRQ_BASE, prvQueueISR, RT_NULL,
							"mailbox");
	rt_hw_interrupt_umask(BSP_MAILBOX_IRQ_BASE);

	for (int i = 0; i < ARRAY_SIZE(gTaskCtx); i++) {
		TASK_INIT(i);
	}
}

void prvCmdQuRunTask(void *parameter)
{
	(void)parameter;

	cmdqu_t    rtos_cmdq;
	cmdqu_t   *cmdq;
	cmdqu_t   *rtos_cmdqu_t;
	static int stop_ip = 0;
	// int        ret     = 0;
	int flags;
	int valid;
	int send_to_cpu = SEND_TO_CPU1;

	unsigned int reg_base = MAILBOX_REG_BASE;

	transfer_config.mcu_status = MCU_STATUS_RTOS_T1_RUNNING;

	if (transfer_config.conf_magic == C906_MAGIC_HEADER)
		send_to_cpu = SEND_TO_CPU1;
	else if (transfer_config.conf_magic == CA53_MAGIC_HEADER)
		send_to_cpu = SEND_TO_CPU0;

	cmdq            = &rtos_cmdq;
	mbox_reg        = (struct mailbox_set_register *)reg_base;
	mbox_done_reg   = (struct mailbox_done_register *)(reg_base + 2);
	mailbox_context = (unsigned long *)(MAILBOX_REG_BUFF);

	cvi_spinlock_init();
	rt_kprintf("prvCmdQuRunTask run\n");

	for (;;) {
		rt_mq_recv(gTaskCtx[E_QUEUE_CMDQU].queHandle, &rtos_cmdq,
				   sizeof(rtos_cmdq), RT_WAITING_FOREVER);

		switch (rtos_cmdq.cmd_id) {
#if (configUSETRACE_FACILITY == 1)
		case SYS_CMD_INFO_TRACE_SNAPSHOT_START:
			debug_printf("SYS_CMD_INFO_TRACE_SNAPSHOT_START\n");
			vTraceEnable(TRC_START);
			break;
		case SYS_CMD_INFO_TRACE_SNAPSHOT_STOP:
			snapshot.ptr        = xTraceGetTraceBuffer();
			snapshot.size       = uiTraceGetTraceBufferSize();
			snapshot.type       = 0;
			rtos_cmdq.param_ptr = &snapshot;
			vTraceStop();
			rt_hw_cpu_dcache_clean(&snapshot, sizeof(struct trace_snapshot_t));
			rt_hw_cpu_dcache_clean(snapshot.ptr, snapshot.size);
			debug_printf("SYS_CMD_INFO_TRACE_SNAPSHOT_STOP PA =%lx\n",
						 &snapshot);
			debug_printf("SYS_CMD_INFO_TRACE_SNAPSHOT_STOP ptr =%lx\n",
						 snapshot.ptr);
			debug_printf("SYS_CMD_INFO_TRACE_SNAPSHOT_STOP size =%lx\n",
						 snapshot.size);
			goto send_label;
			break;
#endif
		case SYS_CMD_INFO_DUMP_JPG:
			break;
		case SYS_CMD_INFO_DUMP_EN:
			// dump_uart_enable();
			break;
		case SYS_CMD_INFO_DUMP_DIS:
			// dump_uart_disable();
			break;
		case SYS_CMD_INFO_DUMP_MSG:
			rtos_cmdq.cmd_id = SYS_CMD_INFO_DUMP_MSG;
			// rtos_cmdq.param_ptr = (unsigned int)dump_uart_msg();
			goto send_label;
			break;
		case SYS_CMD_INFO_LINUX_INIT_DONE:
			rtos_cmdq.cmd_id    = SYS_CMD_INFO_RTOS_INIT_DONE;
			rtos_cmdq.param_ptr = (uintptr_t)&transfer_config;
			goto send_label;
			break;
		case SYS_CMD_INFO_STOP_ISR:
			stop_ip          = 0;
			rtos_cmdq.cmd_id = SYS_CMD_INFO_STOP_ISR;
			rtos_cmdq.ip_id  = IP_VI;
			rt_mq_send(gTaskCtx[E_QUEUE_VI].queHandle, &rtos_cmdq,
					   sizeof(rtos_cmdq));
			break;
		case SYS_CMD_INFO_STOP_ISR_DONE:
			if (rtos_cmdq.ip_id == IP_VI) {
				stop_ip |= STOP_CMD_DONE_VI;
				rtos_cmdq.ip_id  = IP_VCODEC;
				rtos_cmdq.cmd_id = SYS_CMD_INFO_STOP_ISR;
				rt_mq_send(gTaskCtx[E_QUEUE_VCODEC].queHandle, &rtos_cmdq,
						   sizeof(rtos_cmdq));
				break;
			}
			if (rtos_cmdq.ip_id == IP_VCODEC)
				stop_ip |= STOP_CMD_DONE_VCODE;
			if (stop_ip != STOP_CMD_DONE_ALL)
				break;
			else {
				rtos_cmdq.ip_id = IP_SYSTEM;
			}
		case SYS_CMD_INFO_LINUX:
		default:
		send_label:
			rtos_cmdqu_t = (cmdqu_t *)mailbox_context;

			debug_printf("RTOS_CMDQU_SEND\n");
			debug_printf("ip_id=%d cmd_id=%d param_ptr=%x\n", cmdq->ip_id,
						 cmdq->cmd_id, (unsigned int)cmdq->param_ptr);
			debug_printf("mailbox_context = %x\n", mailbox_context);
			debug_printf("linux_cmdqu_t = %x\n", rtos_cmdqu_t);
			debug_printf("cmdq->ip_id = %d\n", cmdq->ip_id);
			debug_printf("cmdq->cmd_id = %d\n", cmdq->cmd_id);
			debug_printf("cmdq->block = %d\n", cmdq->block);
			debug_printf("cmdq->para_ptr = %x\n", cmdq->param_ptr);

			drv_spin_lock_irqsave(&mailbox_lock, flags);
			if (flags == MAILBOX_LOCK_FAILED) {
				rt_kprintf(
						"[%s][%d] drv_spin_lock_irqsave failed! ip_id = %d , cmd_id = %d\n",
						cmdq->ip_id, cmdq->cmd_id);
				break;
			}

			for (valid = 0; valid < MAILBOX_MAX_NUM; valid++) {
				if (rtos_cmdqu_t->resv.valid.linux_valid == 0 &&
					rtos_cmdqu_t->resv.valid.rtos_valid == 0) {
					int *ptr = (int *)rtos_cmdqu_t;

					cmdq->resv.valid.rtos_valid = 1;
					*ptr = ((cmdq->ip_id << 0) | (cmdq->cmd_id << 8) |
							(cmdq->block << 15) |
							(cmdq->resv.valid.linux_valid << 16) |
							(cmdq->resv.valid.rtos_valid << 24));
					rtos_cmdqu_t->param_ptr = cmdq->param_ptr;
					debug_printf("rtos_cmdqu_t->linux_valid = %d\n",
								 rtos_cmdqu_t->resv.valid.linux_valid);
					debug_printf("rtos_cmdqu_t->rtos_valid = %d\n",
								 rtos_cmdqu_t->resv.valid.rtos_valid);
					debug_printf("rtos_cmdqu_t->ip_id =%x %d\n",
								 &rtos_cmdqu_t->ip_id, rtos_cmdqu_t->ip_id);
					debug_printf("rtos_cmdqu_t->cmd_id = %d\n",
								 rtos_cmdqu_t->cmd_id);
					debug_printf("rtos_cmdqu_t->block = %d\n",
								 rtos_cmdqu_t->block);
					debug_printf("rtos_cmdqu_t->param_ptr addr=%x %x\n",
								 &rtos_cmdqu_t->param_ptr,
								 rtos_cmdqu_t->param_ptr);
					debug_printf("*ptr = %x\n", *ptr);
					mbox_reg->cpu_mbox_set[send_to_cpu]
							.cpu_mbox_int_clr.mbox_int_clr = (1 << valid);
					mbox_reg->cpu_mbox_en[send_to_cpu].mbox_info |=
							(1 << valid);
					mbox_reg->mbox_set.mbox_set = (1 << valid);
					break;
				}
				rtos_cmdqu_t++;
			}
			drv_spin_unlock_irqrestore(&mailbox_lock, flags);
			if (valid >= MAILBOX_MAX_NUM) {
				rt_kprintf("No valid mailbox is available\n");
				return;
			}
			break;
		}
	}
}

void prvQueueISR(int vector, void *param)
{
	unsigned char set_val;
	unsigned char valid_val;
	int           i;
	cmdqu_t      *cmdq;
	uint32_t      value = *(volatile uint32_t *)MAILBOX_REG_BASE;
	set_val = mbox_reg->cpu_mbox_set[RECEIVE_CPU].cpu_mbox_int_int.mbox_int;
	if (set_val) {
		for (i = 0; i < MAILBOX_MAX_NUM; i++) {
			valid_val = set_val & (1 << i);

			if (valid_val) {
				volatile cmdqu_t rtos_cmdq;
				cmdq = ((cmdqu_t *)mailbox_context) + i;

				debug_printf("mailbox_context =%x\n", mailbox_context);
				debug_printf("sizeof mailbox_context =%x\n",
							 (unsigned int)sizeof(cmdqu_t));
				// need to disable enable bit
				mbox_reg->cpu_mbox_en[RECEIVE_CPU].mbox_info &= ~valid_val;
				/* mailbox buffer context is send from linux, clear mailbox interrupt */
				mbox_reg->cpu_mbox_set[RECEIVE_CPU]
						.cpu_mbox_int_clr.mbox_int_clr = valid_val;

				// copy cmdq context (8 bytes) to buffer ASAP
				*((unsigned long *) &rtos_cmdq) = *((unsigned long *)cmdq);
				asm volatile("fence iorw, iorw" ::: "memory");
				/* need to clear mailbox interrupt before clear mailbox buffer */
				*((unsigned long*) cmdq) = 0;

				/* mailbox buffer context is send from linux*/
				if (rtos_cmdq.resv.valid.linux_valid == 1) {
					debug_printf("cmdq=%x\n", cmdq);
					debug_printf("cmdq->ip_id =%d\n", rtos_cmdq.ip_id);
					debug_printf("cmdq->cmd_id =%d\n", rtos_cmdq.cmd_id);
					debug_printf("cmdq->param_ptr =%x\n",
								 (unsigned int)rtos_cmdq.param_ptr);
					debug_printf("cmdq->block =%x\n", rtos_cmdq.block);
					debug_printf("cmdq->linux_valid =%d\n",
								 rtos_cmdq.resv.valid.linux_valid);
					debug_printf("cmdq->rtos_valid =%x\n",
								 rtos_cmdq.resv.valid.rtos_valid);

					switch (rtos_cmdq.ip_id) {
					case IP_ISP:
						rt_mq_send(gTaskCtx[E_QUEUE_ISP].queHandle, &rtos_cmdq,
								   sizeof(rtos_cmdq));
						break;
					case IP_VCODEC:
						rt_mq_send(gTaskCtx[E_QUEUE_VCODEC].queHandle,
								   &rtos_cmdq, sizeof(rtos_cmdq));
						break;
					case IP_VI:
						rt_mq_send(gTaskCtx[E_QUEUE_VI].queHandle, &rtos_cmdq,
								   sizeof(rtos_cmdq));
						break;
					case IP_RGN:
						rt_mq_send(gTaskCtx[E_QUEUE_RGN].queHandle, &rtos_cmdq,
								   sizeof(rtos_cmdq));
						break;
					case IP_AUDIO:
						rt_mq_send(gTaskCtx[E_QUEUE_AUDIO].queHandle,
								   &rtos_cmdq, sizeof(rtos_cmdq));
						break;
					case IP_SYSTEM:
						debug_printf("queue head = %p \n",
									 gTaskCtx[E_QUEUE_CMDQU]
											 .queHandle->msg_queue_head);

						debug_printf("queue tail = %p \n",
									 gTaskCtx[E_QUEUE_CMDQU]
											 .queHandle->msg_queue_tail);
						rt_mq_send(gTaskCtx[E_QUEUE_CMDQU].queHandle,
								   &rtos_cmdq, sizeof(rtos_cmdq));
						debug_printf("queue head = %p \n",
									 gTaskCtx[E_QUEUE_CMDQU]
											 .queHandle->msg_queue_head);

						debug_printf("queue tail = %p \n",
									 gTaskCtx[E_QUEUE_CMDQU]
											 .queHandle->msg_queue_tail);
						break;
					case IP_CAMERA:
						rt_mq_send(gTaskCtx[E_QUEUE_CAMERA].queHandle,
								   &rtos_cmdq, sizeof(rtos_cmdq));
						break;
					default:
						rt_kprintf("unknown ip_id =%d cmd_id=%d\n",
								   rtos_cmdq.ip_id, rtos_cmdq.cmd_id);
						break;
					}
				} else {
					rt_kprintf("rtos cmdq is not valid %d, ip=%d , cmd=%d\n",
							   rtos_cmdq.resv.valid.rtos_valid, rtos_cmdq.ip_id,
							   rtos_cmdq.cmd_id);
				}
			}
		}
	}
}

void GetCommInfo(void)
{
	cmdqu_t        rtos_cmdq;
	unsigned short checksum = 0;
	unsigned char *ptr      = &transfer_config;
	/* get communication information from mailbox 0x1900400 */
	memcpy((char *)&transfer_config, (char *)MAILBOX_REG_BUFF,
		   sizeof(struct transfer_config_t));
	/* clear communication information from mailbox 0x1900400 */
	memset((char *)MAILBOX_REG_BUFF, 0, sizeof(cmdqu_t) * MAILBOX_MAX_NUM);
	debug_printf("transfer_config addr = %x\n", &transfer_config);
	debug_printf("transfer_config magic = %x\n", transfer_config.conf_magic);
	debug_printf("transfer_config status = %x\n", transfer_config.mcu_status);
	debug_printf("transfer_config image_type = %x\n",
				 transfer_config.image_type);
	debug_printf("transfer_config dump_print_enable = %x\n",
				 transfer_config.dump_print_enable);
	debug_printf("transfer_config dump_print_size = %x\n",
				 1 << transfer_config.dump_print_size_idx);
	debug_printf("transfer_config checksum = %x\n", transfer_config.checksum);

	/* check configuration from fsbl */
	for (int i = 0; i < transfer_config.conf_size; i++, ptr++) {
		checksum += *ptr;
	}
	if (checksum != transfer_config.checksum) {
		debug_printf("checksum fail (%x, %x)\n", transfer_config.checksum,
					 checksum);
		/* use the default setting */
		debug_printf("no default confi setting\n");
	}

	/* init uart dump feature*/
	dump_uart_init();

	transfer_config.mcu_status   = MCU_STATUS_RTOS_T1_INIT;
	transfer_config.linux_status = MCU_STATUS_LINUX_INIT;
	rt_hw_cpu_dcache_clean(&transfer_config, sizeof(struct transfer_config_t));
}