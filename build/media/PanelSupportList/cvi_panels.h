/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_panels.h
 * Description:
 */

#ifndef __CVI_PANEL_H__
#define __CVI_PANEL_H__

#ifdef __UBOOT__
#include "panels/panel_platform.h"
#else
#include "cvi_comm_mipi_tx.h"
#include "cvi_mipi_tx.h"
#include "cvi_type.h"
#include <cvi_comm_vo.h>
#endif

struct panel_desc_s {
    char* panel_name;
    struct combo_dev_cfg_s* dev_cfg;
    const struct hs_settle_s* hs_timing_cfg;
    const struct dsc_instr* dsi_init_cmds;
    int dsi_init_cmds_size;

    const VO_PUB_ATTR_S stVoPubAttr;
    const VO_LVDS_ATTR_S* lvds_cfg;
    const VO_BT_ATTR_S* bt_attr;
    const void* i80_hw_cfg;
    const void* i80_sw_cfg;
    const void* i80_init_cmds;
    int i80_init_cmds_size;
};

#if CONFIG_DSI_HX8394_720X1280_NULL_4LANE_60FPS
#include "panels/dsi_hx8394_720x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "HX8394-720x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps) };
#elif CONFIG_DSI_ILI9881C_720X1280_NULL_4LANE_60FPS
#include "panels/dsi_ili9881c_720x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "ILI9881C-720x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_ili9881c_720x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_ili9881c_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_ili9881c_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ili9881c_720x1280_NULL_4lane_60fps) };
#elif CONFIG_DSI_ILI9881D_720X1280_NULL_4LANE_60FPS
#include "panels/dsi_ili9881d_720x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "ILI9881D-720x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_ili9881d_720x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_ili9881d_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_ili9881d_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ili9881d_720x1280_NULL_4lane_60fps) };
#elif CONFIG_DSI_JD9366AB_800X1280_NULL_4LANE_60FPS
#include "panels/dsi_jd9366ab_800x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "JD9366AB-800x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_jd9366ab_800x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_jd9366ab_800x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_jd9366ab_800x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_jd9366ab_800x1280_NULL_4lane_60fps) };
#elif CONFIG_DSI_NT35521_800X1280_NULL_4LANE_60FPS
#include "panels/dsi_nt35521_800x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "NT35521-800x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_nt35521_800x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_nt35521_800x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_nt35521_800x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_nt35521_800x1280_NULL_4lane_60fps) };
#elif CONFIG_DSI_OTA7290B_320X1280_NULL_4LANE_60FPS
#include "panels/dsi_ota7290b_320x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "OTA7290B-320x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_ota7290b_320x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_ota7290b_320x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_ota7290b_320x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ota7290b_320x1280_NULL_4lane_60fps) };
#elif CONFIG_DSI_OTA7290B_440X1920_NULL_4LANE_60FPS
#include "panels/dsi_ota7290b_440x1920_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "OTA7290B-440x1920-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_ota7290b_440x1920_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_ota7290b_440x1920_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_ota7290b_440x1920_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ota7290b_440x1920_NULL_4lane_60fps) };
#elif CONFIG_DSI_ICN9707_480X1920_NULL_4LANE_60FPS
#include "panels/dsi_icn9707_480x1920_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "ICN9707-480x1920-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_icn9707_480x1920_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_icn9707_480x1920_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_icn9707_480x1920_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_icn9707_480x1920_NULL_4lane_60fps) };
#elif CONFIG_DSI_3AML069LP01G_600X1024_NULL_4LANE_60FPS
#include "panels/dsi_3aml069lp01g_600x1024_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "3AML069LP01G-600x1024-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps) };
#elif CONFIG_DSI_ST7701_480X800_NULL_2LANE_60FPS
#include "panels/dsi_st7701_480x800_NULL_2lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "ST7701-480x800-NULL-2lane-60fps",
    .dev_cfg = &dev_cfg_dsi_st7701_480x800_NULL_2lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_st7701_480x800_NULL_2lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_st7701_480x800_NULL_2lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_st7701_480x800_NULL_2lane_60fps) };
#elif CONFIG_DSI_ST7703_640X480_NULL_2LANE_60FPS
#include "panels/dsi_st7703_640x480_NULL_2lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "ST7703-640x480-NULL-2lane-60fps",
    .dev_cfg = &dev_cfg_dsi_st7703_640x480_NULL_2lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_st7703_640x480_NULL_2lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_st7703_640x480_NULL_2lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_st7703_640x480_NULL_2lane_60fps) };
#elif CONFIG_DSI_JD9161C_480X640_NULL_2LANE_60FPS
#include "panels/dsi_jd9161c_480x640_NULL_2lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "JD9161C-480x640-NULL-2lane-60fps",
    .dev_cfg = &dev_cfg_dsi_jd9161c_480x640_NULL_2lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_jd9161c_480x640_NULL_2lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_jd9161c_480x640_NULL_2lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_jd9161c_480x640_NULL_2lane_60fps) };
#elif CONFIG_DSI_BJX2836F0_480X640_NULL_2LANE_60FPS
#include "panels/dsi_bjx2836f0_480x640_NULL_2lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "BJX2836F0-480x640",
    .dev_cfg = &dev_cfg_dsi_bjx2836f0_480x640_NULL_2lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_bjx2836f0_480x640_NULL_2lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_bjx2836f0_480x640_NULL_2lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_bjx2836f0_480x640_NULL_2lane_60fps) };
#elif CONFIG_DSI_ILI9488_320X480_NULL_1LANE_60FPS
#include "panels/dsi_ili9488_320x480_NULL_1lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "ILI9488-320x480-NULL-1lane-60fps",
    .dev_cfg = &dev_cfg_dsi_ili9488_320x480_NULL_1lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_ili9488_320x480_NULL_1lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_ili9488_320x480_NULL_1lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ili9488_320x480_NULL_1lane_60fps) };
#elif CONFIG_DSI_ST7701_480X640_NULL_2LANE_60FPS
#include "panels/dsi_st7701_480x640_NULL_2lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "ST7701-480x640",
    .dev_cfg = &dev_cfg_dsi_st7701_480x640_NULL_2lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_st7701_480x640_NULL_2lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_st7701_480x640_NULL_2lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_st7701_480x640_NULL_2lane_60fps) };
#elif CONFIG_DSI_GM8775C_1920X1080_NULL_4LANE_60FPS
#include "panels/dsi_gm8775c_1920x1080_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "GM8775C-1920x1080-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_gm8775c_1920x1080_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_gm8775c_1920x1080_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_gm8775c_1920x1080_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_gm8775c_1920x1080_NULL_4lane_60fps) };
#elif CONFIG_DSI_HX8399_1080X1920_NULL_4LANE_60FPS
#include "panels/dsi_hx8399_1080x1920_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "HX8399-1080x1920-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_hx8399_1080x1920_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_hx8399_1080x1920_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_hx8399_1080x1920_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8399_1080x1920_NULL_4lane_60fps) };
#elif CONFIG_DSI_LT9611_MULTI_NULL_4LANE_60FPS
#include "panels/dsi_lt9611_multi_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = { .panel_name = "LT9611-1920x1080-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_lt9611_multi_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_lt9611_multi_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_lt9611_multi_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_lt9611_multi_NULL_4lane_60fps) };
#elif CONFIG_I80_ST7789V_240X320_NULL_60FPS_HW
#include "panels/i80_st7789v_240x320_NULL_60fps_hw.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "ST7789V-240x320-HW",
#ifndef __UBOOT__
    .stVoPubAttr = {
        .enIntfType = VO_INTF_HW_MCU,
        .enIntfSync = VO_OUTPUT_USER,
        .stSyncInfo = { .bSynm = 1, .bIop = 1, .u16FrameRate = 60, .u16Vact = 320, .u16Vbb = 0, .u16Vfb = 32, .u16Hact = 240, .u16Hbb = 0, .u16Hfb = 16, .u16Vpw = 2, .u16Hpw = 2, .bIdv = 0, .bIhs = 0, .bIvs = 0 },
        .stMcuCfg = st7789vCfg,
    },
#else
    .i80_hw_cfg = &st7789vCfg,
#endif
};
#elif CONFIG_I80_ST7789V3_240X320_NULL_60FPS_HW
#include "panels/i80_st7789v3_240x320_NULL_60fps_hw.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "ST7789V3-240x320-HW",
    .i80_hw_cfg = &st7789v3Cfg,
};
#elif CONFIG_I80_ST7789V3_240X320_NULL_60FPS_HW_MCU
#include "panels/i80_st7789v3_240x320_NULL_60fps_hw_mcu.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "ST7789V3-240x320",
#ifndef __UBOOT__
    .stVoPubAttr = {
        .enIntfType = VO_INTF_HW_MCU,
        .enIntfSync = VO_OUTPUT_USER,
        .stSyncInfo = { .bSynm = 1, .bIop = 1, .u16FrameRate = 60, .u16Vact = 320, .u16Vbb = 5, .u16Vfb = 20, .u16Hact = 240, .u16Hbb = 20, .u16Hfb = 10, .u16Vpw = 5, .u16Hpw = 5, .bIdv = 0, .bIhs = 1, .bIvs = 1 },
        .stMcuCfg = st7789v3Cfg,
    },
#else
    .i80_hw_cfg = &st7789v3Cfg,
#endif
};
#elif CONFIG_I80_ST7789V_240X320_NULL_60FPS_SW
#include "panels/i80_st7789v_240x320_NULL_60fps_sw.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "ST7789V-240x320-SW",
    .i80_sw_cfg = &i80_st7789v_cfg,
    .i80_init_cmds = i80_st7789v_init_cmds,
    .i80_init_cmds_size = ARRAY_SIZE(i80_st7789v_init_cmds),
};
#elif CONFIG_LVDS_EK79202_1280X800_NULL_60FPS
#include "panels/lvds_ek79202_1280x800_NULL_60fps.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "EK79202-1280x800",
    .lvds_cfg = &lvds_ek79202_cfg,
};
#elif CONFIG_LVDS_LCM185X56_NULL_NULL_NULL
#include "panels/lvds_lcm185x56_NULL_NULL_NULL.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "LCM185X56",
    .lvds_cfg = &lvds_lcm185x56_cfg,
};
#elif CONFIG_BT1120_PT1000K_NULL_NULL_NULL
#include "panels/bt1120_pt1000k_NULL_NULL_NULL.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "PT1000K-BT1120",
#ifndef __UBOOT__
    .bt_attr = &stpt1000kbt1120cfg,
#endif
};
#elif CONFIG_BT656_MS7024_NULL_NULL_NULL
#include "panels/bt656_ms7024_NULL_NULL_NULL.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "MS7024-BT656",
#ifndef __UBOOT__
    .bt_attr = &stMS7024bt656cfg,
#endif
};
#elif CONFIG_BT656_PT1000K_NULL_NULL_NULL
#include "panels/bt656_pt1000k_NULL_NULL_NULL.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "PT1000K-BT656",
#ifndef __UBOOT__
    .bt_attr = &stpt1000kbt656cfg,
#endif
};
#else
#include "panels/dsi_hx8394_720x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "HX8394-720x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps)
};
#endif

#endif
