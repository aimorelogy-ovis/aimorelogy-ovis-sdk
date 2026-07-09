/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __VO_INTERFACES_H__
#define __VO_INTERFACES_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "scaler.h"
#include <base_cb.h>

static const char *const clk_disp_name = "clk_disp";
static const char *const clk_bt_name = "clk_bt";
static const char *const clk_dsi_name = "clk_dsi";

/*******************************************************
 *  Common interface for core
 ******************************************************/
int vo_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg);
void vo_irq_handler(struct cvi_vo_dev *vdev, union sclr_intr intr_status);
int vo_create_instance(struct cvi_vo_dev *pdev);
int vo_destroy_instance(struct cvi_vo_dev *pdev);

#ifdef __cplusplus
}
#endif

#endif /* __VO_INTERFACES_H__ */
