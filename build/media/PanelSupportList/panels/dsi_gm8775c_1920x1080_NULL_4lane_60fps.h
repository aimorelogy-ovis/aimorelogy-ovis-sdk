#ifndef PANELS_DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_H
#define PANELS_DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_H

#include "panel_platform.h"

#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VACT 1080
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VSA 5
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VBP 36
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VFP 4
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HACT 1920
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HSA 44
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HBP 148
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HFP 88
#define DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_FPS 60

struct combo_dev_cfg_s dev_cfg_dsi_gm8775c_1920x1080_NULL_4lane_60fps = {
    .devno = 0,
    .lane_id = { MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK,
        MIPI_TX_LANE_2, MIPI_TX_LANE_3 },
    .lane_pn_swap = { true, true, true, true, true },
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .video_mode = BURST_MODE,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .sync_info = {
        .vid_hsa_pixels = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HSA,
        .vid_hbp_pixels = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HBP,
        .vid_hfp_pixels = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HFP,
        .vid_hline_pixels = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_HACT,
        .vid_vsa_lines = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VSA,
        .vid_vbp_lines = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VBP,
        .vid_vfp_lines = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VFP,
        .vid_active_lines = DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_VACT,
        .vid_vsa_pos_polarity = false,
        .vid_hsa_pos_polarity = true,
    },
    .pixel_clk = PIXEL_CLK(DSI_GM8775C_1920X1080_NULL_4LANE_60FPS),
};

struct hs_settle_s hs_timing_cfg_dsi_gm8775c_1920x1080_NULL_4lane_60fps = { .prepare = 6, .zero = 32, .trail = 1 };

#define _BIST_COLOR 0

struct dsc_instr dsi_init_cmds_dsi_gm8775c_1920x1080_NULL_4lane_60fps[] = {
    DSI_CMD_TYPE(0, 0x23, 0x27, 0xAA),
    DSI_CMD_TYPE(0, 0x23, 0x48, 0x02),
    DSI_CMD_TYPE(0, 0x23, 0xB6, 0x20),
    DSI_CMD_TYPE(0, 0x23, 0x01, 0x80),
    DSI_CMD_TYPE(0, 0x23, 0x02, 0x38),
    DSI_CMD_TYPE(0, 0x23, 0x03, 0x47),
    DSI_CMD_TYPE(0, 0x23, 0x04, 0x58),
    DSI_CMD_TYPE(0, 0x23, 0x05, 0x2c),
    DSI_CMD_TYPE(0, 0x23, 0x06, 0x94),
    DSI_CMD_TYPE(0, 0x23, 0x07, 0x00),
    DSI_CMD_TYPE(0, 0x23, 0x08, 0x04),
    DSI_CMD_TYPE(0, 0x23, 0x09, 0x05),
    DSI_CMD_TYPE(0, 0x23, 0x0A, 0x24),
    /* use mipi clk */
    DSI_CMD_TYPE(0, 0x23, 0x0B, 0x82),
    DSI_CMD_TYPE(0, 0x23, 0x0C, 0x14),
    DSI_CMD_TYPE(0, 0x23, 0x0D, 0x01),
    DSI_CMD_TYPE(0, 0x23, 0x0E, 0x80),
    DSI_CMD_TYPE(0, 0x23, 0x0F, 0x20),
    DSI_CMD_TYPE(0, 0x23, 0x10, 0x20),
    DSI_CMD_TYPE(0, 0x23, 0x11, 0x03),
    DSI_CMD_TYPE(0, 0x23, 0x12, 0x1B),
    DSI_CMD_TYPE(0, 0x23, 0x13, 0x53),
    DSI_CMD_TYPE(0, 0x23, 0x14, 0x01),
    DSI_CMD_TYPE(0, 0x23, 0x15, 0x23),
    DSI_CMD_TYPE(0, 0x23, 0x16, 0x40),
    DSI_CMD_TYPE(0, 0x23, 0x17, 0x00),
    DSI_CMD_TYPE(0, 0x23, 0x18, 0x01),
    DSI_CMD_TYPE(0, 0x23, 0x19, 0x23),
    DSI_CMD_TYPE(0, 0x23, 0x1A, 0x40),
    DSI_CMD_TYPE(0, 0x23, 0x1B, 0x00),
    DSI_CMD_TYPE(0, 0x23, 0x1E, 0x46),
    DSI_CMD_TYPE(0, 0x23, 0x51, 0x30),
    DSI_CMD_TYPE(0, 0x23, 0x1F, 0x10),
#if _BIST_COLOR
    DSI_CMD_TYPE(0, 0x23, 0x2A, 0x4D),
#else
    DSI_CMD_TYPE(0, 0x23, 0x2A, 0x01),
#endif
};

#else
#error "MIPI_TX_PARAM multi-declaration!!"
#endif // PANELS_DSI_GM8775C_1920X1080_NULL_4LANE_60FPS_H
