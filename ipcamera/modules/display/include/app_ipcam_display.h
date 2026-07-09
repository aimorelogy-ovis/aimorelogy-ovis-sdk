/*
 * Copyright (C) Cvitek Co., Ltd. 2023. All rights reserved.
 *
 * File Name: app_ipcam_display.h
 * Description:
 */

#ifndef __APP_IPCAM_DISPLAY_H__
#define __APP_IPCAM_DISPLAY_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "cvi_defines.h"
#include "cvi_type.h"

#include "app_ipcam_panel.h"
#include "app_ipcam_vo.h"

#define APP_IPCAM_VO_MAX_NUM (VO_MAX_DEV_NUM)

typedef struct APP_PARAM_DISPLAY_CFG_S {
    PANEL_TYPE_E enPanelType;
    PANEL_I2C_CFG_T stPanelI2cCfg;
    APP_PARAM_VO_CFG_T stVoCfg;
} APP_PARAM_DISPLAY_CFG_T;

typedef struct APP_PARAM_MULTI_DISPLAY_CFG_S {
    APP_PARAM_DISPLAY_CFG_T vo_cfg[APP_IPCAM_VO_MAX_NUM];
    uint32_t vo_num;
} APP_PARAM_MULTI_DISPLAY_CFG_T;

APP_PARAM_MULTI_DISPLAY_CFG_T *app_ipcam_Display_Param_Get(void);

CVI_S32 app_ipcam_Display_Init(void);
CVI_S32 app_ipcam_Display_Exit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
