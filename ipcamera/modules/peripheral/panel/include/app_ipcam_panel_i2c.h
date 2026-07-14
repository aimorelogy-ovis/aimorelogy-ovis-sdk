/*
 * Copyright (C) Cvitek Co., Ltd. 2023. All rights reserved.
 *
 * File Name: app_ipcam_panel_i2c.h
 * Description:
 */

#ifndef __APP_IPCAM_PANEL_I2C_H__
#define __APP_IPCAM_PANEL_I2C_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "cvi_type.h"
#include "cvi_comm_vo.h"
#include "app_ipcam_panel.h"

CVI_S32 app_ipcam_Panel_I2c_Init(VO_DEV VoDev, const PANEL_I2C_CFG_T* const pstI2cCfg);
CVI_S32 app_ipcam_Panel_I2c_Exit(VO_DEV VoDev);
CVI_S32 app_ipcam_Panel_I2c_WriteReg(VO_DEV VoDev, CVI_U8 u8Addr, CVI_U8 u8Data,
    const PANEL_I2C_CFG_T* const pstI2cCfg);
CVI_S32 app_ipcam_Panel_I2c_SendInit(VO_DEV VoDev, const PANEL_TYPE_E enPanelType,
    const PANEL_I2C_CFG_T* const pstI2cCfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
