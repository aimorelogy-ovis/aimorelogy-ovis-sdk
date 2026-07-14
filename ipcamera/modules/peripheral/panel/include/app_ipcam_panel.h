/*
 * Copyright (C) Cvitek Co., Ltd. 2023. All rights reserved.
 *
 * File Name: app_ipcam_panel.h
 * Description:
 */

#ifndef __APP_IPCAM_PANEL_H__
#define __APP_IPCAM_PANEL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "cvi_type.h"
#include "cvi_comm_vo.h"
#include "app_ipcam_vo.h"

typedef struct PANEL_DESC_S {
    const CVI_CHAR *pchPanelName;
    struct combo_dev_cfg_s *pstDevCfg;
    const struct hs_settle_s *pstHsTimingCfg;
    const struct dsc_instr *pstDsiInitCmds;
    CVI_S32 s32DsiInitCmdsSize;
} PANEL_DESC_T;

typedef struct PANEL_I2C_CFG_S {
    CVI_S32 s32I2cDev;
    CVI_S32 s32I2cAddr;
} PANEL_I2C_CFG_T;

typedef enum _PANEL_TYPE_E {
    PANEL_DSI_3AML069LP01G,
    PANEL_DSI_GM8775C,
    PANEL_DSI_HX8394_EVB,
    PANEL_DSI_HX8399_1080P,
    PANEL_DSI_ICN9707,
    PANEL_DSI_ILI9881C,
    PANEL_DSI_ILI9881D,
    PANEL_DSI_JD9366AB,
    PANEL_DSI_JD9852,
    PANEL_DSI_LT9611_1024x768_60,
    PANEL_DSI_LT9611_1280x720_60,
    PANEL_DSI_LT9611_1280x1024_60,
    PANEL_DSI_LT9611_1600x1200_60,
    PANEL_DSI_LT9611_1920x1080_30,
    PANEL_DSI_LT9611_1920x1080_60,
    PANEL_DSI_NT35521,
    PANEL_DSI_OTA7290B,
    PANEL_DSI_OTA7290B_1920,
    PANEL_DSI_ST7701,
    PANEL_DSI_ST7785M,
    PANEL_DSI_ST7796S,
    PANEL_BT656_TP2803,
    PANEL_BT656_MS7024_720x480_60,
    PANEL_BT656_MS7024_720x480_30,
    PANEL_BT656_MS7024_720x576_25,
    PANEL_BT656_MS7024_720x576_50,
    PANEL_LVDS_LCM185X56,
    PANEL_I80_ST7789V,
    PANEL_MAX
} PANEL_TYPE_E;

CVI_S32 app_ipcam_Panel_PanelDesc_Get(
    const PANEL_TYPE_E* const penPanelType,
    PANEL_DESC_T* const pstPanelDesc,
    APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Panel_BT_Init(
    const APP_PARAM_VO_CFG_T* const pstVoCfg,
    const PANEL_TYPE_E enPanelType,
    const PANEL_I2C_CFG_T* const pstI2cCfg);
CVI_S32 app_ipcam_Panel_BT_Deinit(const APP_PARAM_VO_CFG_T* const pstVoCfg, const PANEL_TYPE_E enPanelType);
CVI_S32 app_ipcam_Panel_Lvds_Init(const APP_PARAM_VO_CFG_T* const pstVoCfg, const PANEL_TYPE_E enPanelType);
CVI_S32 app_ipcam_Panel_Lvds_Deinit(const APP_PARAM_VO_CFG_T* const pstVoCfg, const PANEL_TYPE_E enPanelType);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
