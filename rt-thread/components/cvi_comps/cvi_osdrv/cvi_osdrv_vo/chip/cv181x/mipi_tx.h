/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _MIPI_TX_H_
#define _MIPI_TX_H_

#include "cvi_vip.h"
#include "mipi_tx_uapi.h"

#define MIPI_TX_DEV_NAME "cvi-mipi-tx"
#define MIPI_TX_PROC_NAME "mipi_tx"

#ifndef BIT
#define BIT(bit) (0x01 << (bit))
#endif

#define CVI_TRACE_MIPI_TX(level, fmt, arg...) \
	do { \
		if (level <= mipi_tx_log_lv) { \
			printf("%d:%s(): " fmt, __LINE__, __func__, ## arg); \
		} \
	} while (0)

enum mipi_tx_msg_pri {
	CVI_MIPI_TX_ERR = 0x08,
	CVI_MIPI_TX_WARN = 0x10,
	CVI_MIPI_TX_NOTICE = 0x20,
	CVI_MIPI_TX_INFO = 0x40,
	CVI_MIPI_TX_DBG = 0x80,
};

#endif // _MIPI_TX_H_
