/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_mipi.c
 * Description:
 *
 */
#include <errno.h>
#include <sys/ioctl.h>

#include "cvi_comm_isp.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "isp_debug.h"
#include "isp_main_local.h"

CVI_S32 CVI_MIPI_SetMipiReset(CVI_S32 devno, CVI_U32 reset)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!reset) {
        return s32Ret;
    }

    s32Ret = cif_reset_mipi(devno) < 0 ? CVI_FAILURE : CVI_SUCCESS;

    return s32Ret;
}

CVI_S32 CVI_MIPI_SetSensorClock(CVI_S32 devno, CVI_U32 enable)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    s32Ret = cif_enable_snsr_clk(devno, enable) < 0 ? CVI_FAILURE : CVI_SUCCESS;

    return s32Ret;
}

CVI_S32 CVI_MIPI_SetSnsGpioInit(CVI_S32 devno, CVI_S32 rst_port_idx, CVI_S32 rst_pin,
                                CVI_S32 rst_pol)
{
    CVI_S32                s32Ret = CVI_SUCCESS;
    struct snsr_rst_gpio_s snsr_gpio;

    if (rst_port_idx < 0 || rst_pin < 0 || rst_pol < 0) {
        printf("sensor reset GPIO ont legal\n");
        printf("rst_port_idx = %d, rst_pin = %d, rst_pol = %d\n", rst_port_idx, rst_pin, rst_pol);
        return CVI_FAILURE;
    }

    snsr_gpio.snsr_rst_port_idx = rst_port_idx;
    snsr_gpio.snsr_rst_pin      = rst_pin;
    snsr_gpio.snsr_rst_pol      = rst_pol;

    s32Ret = cvi_cif_reset_snsr_gpio_init(devno, &snsr_gpio) < 0 ? CVI_FAILURE : CVI_SUCCESS;

    return s32Ret;
}

CVI_S32 CVI_MIPI_SetSensorReset(CVI_S32 devno, CVI_U32 reset)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    s32Ret = cif_reset_snsr_gpio(devno, reset) < 0 ? CVI_FAILURE : CVI_SUCCESS;

    return s32Ret;
}

CVI_S32 CVI_MIPI_SetMipiAttr(CVI_S32 ViPipe, CVI_VOID *devAttr)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
        ISP_LOG_ERR("ViPipe %d value error\n", ViPipe);
        return -ENODEV;
    }

    if (devAttr == CVI_NULL) {
        return CVI_FAILURE;
    }

    s32Ret = cif_set_dev_attr(devAttr) < 0 ? CVI_FAILURE : CVI_SUCCESS;

    return s32Ret;
}

CVI_S32 CVI_MIPI_SetClkEdge(CVI_S32 devno, CVI_U32 is_up)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    // todo: edge config
    return s32Ret;
}

CVI_S32 CVI_MIPI_SetSnsMclk(SNS_MCLK_S *mclk)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    // todo: set_clk
    return s32Ret;
}
