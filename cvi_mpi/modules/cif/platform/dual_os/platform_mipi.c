/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_mipi.c
 * Description:
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <inttypes.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_gdc.h"
#include "cvi_debug.h"
#include "cvi_comm_cif.h"
#include "msg_sensor.h"
#include "cvi_msg_client.h"

CVI_S32 platform_mipi_SetMipiReset(CVI_S32 devno, CVI_U32 reset)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devno, 0);

	stPrivData.as32PrivData[0] = reset;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_RESET_MIPI, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset MIPI fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_mipi_SetSensorClock(CVI_S32 devno, CVI_U32 enable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devno, 0);

	stPrivData.as32PrivData[0] = enable;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_EN_SNS_CLK, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset MIPI fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_mipi_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port,
				CVI_U32 reset_pin, CVI_U32 reset_pol, CVI_U32 reset_enable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	sns_rst_config sns_rst_info;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devno, 0);

	sns_rst_info.devno = devno;
	sns_rst_info.gpio_port = reset_port;
	sns_rst_info.gpio_pin = reset_pin;
	sns_rst_info.gpio_active = reset_pol;

	stPrivData.as32PrivData[0] = reset_enable;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_RESET_GPIO, (CVI_VOID *)&sns_rst_info, sizeof(sns_rst_config), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset sensor gpio fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_mipi_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct combo_dev_attr_s *comboAttr;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = ViPipe;

	comboAttr = (struct combo_dev_attr_s *)devAttr;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_MIPI_ATTR, (CVI_VOID *)comboAttr, sizeof(struct combo_dev_attr_s), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset sensor gpio fail!\n");
	}
	return s32Ret;
}

CVI_S32 platform_mipi_SetClkEdge(CVI_S32 devno, CVI_U32 is_up)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct clk_edge_s clk;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, devno, 0);

	clk.devno = devno;
	clk.edge = is_up ? CLK_UP_EDGE : CLK_DOWN_EDGE;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_OUTPUT_CLK_EDGE, (CVI_VOID *)&clk, sizeof(struct clk_edge_s), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset sensor gpio fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_mipi_SetSnsMclk(struct mclk_pll_s *mclk)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, 0, 0);

	if (mclk == CVI_NULL) {
		return CVI_FAILURE;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_CLK, (CVI_VOID *)mclk, sizeof(struct mclk_pll_s), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "reset sensor gpio fail!\n");
	}
	return s32Ret;
}
