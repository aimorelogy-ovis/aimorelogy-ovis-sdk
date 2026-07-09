/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: cvi_ispd2_uart.c
 * Description:
 */
#include "cvi_ispd2_uart.h"

#include <unistd.h>

#include "cvi_ispd2_callback_funcs_apps.h"
#include "cvi_ispd2_callback_funcs_dump.h"
#include "cvi_ispd2_event_server.h"
#include "cvi_ispd2_local.h"

TISPDaemon2Info gtObject_uart = {0};

// -----------------------------------------------------------------------------
static void isp_daemon2_uart_set_isConnectionByUart(TISPDaemon2Info *ptObject)
{
    ptObject->isConnectionByUart = CVI_TRUE;
}

// -----------------------------------------------------------------------------
void isp_daemon2_uart_init(void)
{
    CVI_ISPD2_InitialDaemonInfo(&gtObject_uart);
    isp_daemon2_uart_set_isConnectionByUart(&gtObject_uart);
    CVI_ISPD2_ConfigMessageHandler(&(gtObject_uart.tHandlerInfo));
    CVI_ISPD2_Dump_Init(&gtObject_uart);
    CVI_ISPD2_ES_RunService_Uart(&gtObject_uart);
}

// -----------------------------------------------------------------------------
void isp_daemon2_uart_uninit(void)
{
    CVI_ISPD2_ES_DestoryService_Uart(&gtObject_uart);
    CVI_ISPD2_ReleaseDaemonInfo(&gtObject_uart);
}

// -----------------------------------------------------------------------------
int write_by_uart(int fd, const void *buf, int size)
{
    return write_uart(gtObject_uart.isConnectionByUart, fd, buf, size);
}

int receive_by_uart(void *data, uint32_t size, uint32_t timeout_ms)
{
    return receive_uart(data, size, timeout_ms);
}

void uart_deinit_pqtool(void)
{
    pqtool_uart_deinit();
}

void console_restart(int idx, uint32_t baud, uint16_t buf_size)
{
    if (get_console_need_restart()) {
        console_init(idx, baud, buf_size);
        set_console_need_restart(CVI_FALSE);
    }
}

TISPDaemon2Info *CVI_ISPD2_Uart_GetDaemon2Info(void)
{
    return &gtObject_uart;
}