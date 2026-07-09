/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_mipi.c
 * Description:
 *
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "cvi_type.h"
#include "cvi_debug.h"
#include "cvi_mipi.h"
#include "cvi_comm_cif.h"

#define MIPI_DEV_NODE "/dev/cv184x-mipi-rx"

CVI_S32 fd_mipi = -1;

int open_mipi_rx(const char *dev_name, CVI_S32 *fd)
{
	struct stat st;

	*fd = open(dev_name, O_RDWR /* required */  | O_NONBLOCK | O_CLOEXEC, 0);
	if (-1 == *fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno,
			strerror(errno));
		return -1;
	}

	if (-1 == fstat(*fd, &st)) {
		close(*fd);
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name,
			errno, strerror(errno));
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		close(*fd);
		fprintf(stderr, "%s is no device\n", dev_name);
		return -ENODEV;
	}
	return 0;
}

CVI_S32 mipi_open_dev(CVI_VOID)
{
	open_mipi_rx(MIPI_DEV_NODE, &fd_mipi);
	if (fd_mipi < 0)
		return CVI_FAILURE;

	return CVI_SUCCESS;
}

CVI_S32 platform_mipi_SetMipiReset(CVI_S32 devno, CVI_U32 reset)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32Devno = devno;

	if (fd_mipi < 0) {
		s32Ret = mipi_open_dev();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	if (reset == 0) {
		if (ioctl(fd_mipi, CVI_MIPI_UNRESET_MIPI, (void *)(uintptr_t)&s32Devno) < 0) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_UNRESET_MIPI - %d NG\n", s32Devno);
			return errno;
		}
	} else {
		if (ioctl(fd_mipi, CVI_MIPI_RESET_MIPI, (void *)(uintptr_t)&s32Devno) < 0)  {
			CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_RESET_MIPI - %d NG\n", s32Devno);
			return errno;
		}
	}
	return s32Ret;
}

CVI_S32 platform_mipi_SetSensorClock(CVI_S32 devno, CVI_U32 enable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32Devno = devno;

	if (fd_mipi < 0) {
		s32Ret = mipi_open_dev();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	if (enable == 0) {
		if (ioctl(fd_mipi, CVI_MIPI_DISABLE_SENSOR_CLOCK, (void *)(uintptr_t)&s32Devno) < 0) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_DISABLE_SENSOR_CLOCK - %d NG\n", s32Devno);
			return errno;
		}
	} else {
		if (ioctl(fd_mipi, CVI_MIPI_ENABLE_SENSOR_CLOCK, (void *)(uintptr_t)&s32Devno) < 0) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_ENABLE_SENSOR_CLOCK - %d NG\n", s32Devno);
			return errno;
		}
	}
	return s32Ret;
}

CVI_S32 platform_mipi_SetSensorReset(CVI_S32 devno, CVI_U32 reset_port,
				CVI_U32 reset_pin, CVI_U32 reset_pol, CVI_U32 reset_enable)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	sns_rst_config sns_rst_info;

	if (fd_mipi < 0) {
		s32Ret = mipi_open_dev();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	sns_rst_info.devno = devno;
	sns_rst_info.gpio_port = reset_port;
	sns_rst_info.gpio_pin = reset_pin;
	sns_rst_info.gpio_active = reset_pol;

	if (reset_enable == 0) {
		if (ioctl(fd_mipi, CVI_MIPI_UNRESET_SENSOR, (void *)(uintptr_t)&sns_rst_info) < 0)  {
			CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_DISABLE_SENSOR_CLOCK - %d NG\n", devno);
			return errno;
		}
	} else {
		if (ioctl(fd_mipi, CVI_MIPI_RESET_SENSOR, (void *)(uintptr_t)&sns_rst_info) < 0) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_RESET_SENSOR - %d NG\n", devno);
			return errno;
		}
	}
	return s32Ret;
}

CVI_S32 platform_mipi_SetMipiAttr(CVI_S32 ViPipe, const CVI_VOID *devAttr)
{
	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return -ENODEV;
	}

	if (devAttr == CVI_NULL) {
		return CVI_FAILURE;
	}

	CVI_S32 s32Ret = 0;
	struct combo_dev_attr_s *comboAttr;

	if (fd_mipi < 0) {
		s32Ret = mipi_open_dev();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	comboAttr = (struct combo_dev_attr_s *)devAttr;
	if (ioctl(fd_mipi, CVI_MIPI_SET_DEV_ATTR, (void *)(uintptr_t)comboAttr) < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_SET_DEV_ATTR NG\n");
		return errno;
	}
	return s32Ret;
}

CVI_S32 platform_mipi_SetClkEdge(CVI_S32 devno, CVI_U32 is_up)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct clk_edge_s clk;

	if (fd_mipi < 0) {
		s32Ret = mipi_open_dev();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	clk.devno = devno;
	clk.edge = is_up ? CLK_UP_EDGE : CLK_DOWN_EDGE;
	if (ioctl(fd_mipi, CVI_MIPI_SET_OUTPUT_CLK_EDGE, (void *)(uintptr_t)&clk) < 0)  {
		CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_SET_OUTPUT_CLK_EDGE, - %d NG\n", devno);
		return errno;
	}
	return s32Ret;
}

CVI_S32 platform_mipi_SetSnsMclk(struct mclk_pll_s *mclk)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (mclk == CVI_NULL) {
		return CVI_FAILURE;
	}

	if (fd_mipi < 0) {
		s32Ret = mipi_open_dev();
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	if (ioctl(fd_mipi, CVI_MIPI_SET_SENSOR_CLOCK, (void *)&mclk) < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "CVI_MIPI_SET_SENSOR_CLOCK NG\n");
		return errno;
	}
	return s32Ret;
}
