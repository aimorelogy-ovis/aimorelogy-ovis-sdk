/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: cvi_ispd2_uart.h
 * Description:
 */

#ifndef _CVI_ISPD2_UART_H_
#define _CVI_ISPD2_UART_H_

#include "cvi_ispd2_local.h"

// Defined externally
void pqtool_uart_deinit(void);
void console_init(int idx, uint32_t baud, uint16_t buf_size);
int  receive_uart(void *data, uint32_t size, uint32_t timeout_ms);
int  write_uart(bool isConnectionByUart, int fd, const void *buf, int size);
int  get_console_need_restart(void);
int  set_console_need_restart(int value);

// Defined internally
void             uart_deinit_pqtool(void);
void             console_restart(int idx, uint32_t baud, uint16_t buf_size);
int              receive_by_uart(void *data, uint32_t size, uint32_t timeout_ms);
int              write_by_uart(int fd, const void *buf, int size);
TISPDaemon2Info *CVI_ISPD2_Uart_GetDaemon2Info(void);
void             CVI_ISPD2_InitialDaemonInfo(TISPDaemon2Info *ptObject);
void             CVI_ISPD2_ReleaseDaemonInfo(TISPDaemon2Info *ptObject);

#endif  // _CVI_ISPD2_UART_H_
