#ifndef PANELS_DSI_ILI9488_320X480_NULL_1LANE_60FPS_H
#define PANELS_DSI_ILI9488_320X480_NULL_1LANE_60FPS_H

#include "panel_platform.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_VACT 480
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_VSA 2
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_VBP 2
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_VFP 4
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_HACT 320
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_HSA 30
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_HBP 70
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_HFP 170
#define DSI_ILI9488_320X480_NULL_1LANE_60FPS_FPS 60

const struct combo_dev_cfg_s dev_cfg_dsi_ili9488_320x480_NULL_1lane_60fps = {
    .devno = 0,
    .lane_id = { -1, MIPI_TX_LANE_CLK, MIPI_TX_LANE_0, -1, -1 },
    .lane_pn_swap = { false, false, false, false, false },
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .video_mode = BURST_MODE,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .sync_info = {
        .vid_hsa_pixels = DSI_ILI9488_320X480_NULL_1LANE_60FPS_HSA,
        .vid_hbp_pixels = DSI_ILI9488_320X480_NULL_1LANE_60FPS_HBP,
        .vid_hfp_pixels = DSI_ILI9488_320X480_NULL_1LANE_60FPS_HFP,
        .vid_hline_pixels = DSI_ILI9488_320X480_NULL_1LANE_60FPS_HACT,
        .vid_vsa_lines = DSI_ILI9488_320X480_NULL_1LANE_60FPS_VSA,
        .vid_vbp_lines = DSI_ILI9488_320X480_NULL_1LANE_60FPS_VBP,
        .vid_vfp_lines = DSI_ILI9488_320X480_NULL_1LANE_60FPS_VFP,
        .vid_active_lines = DSI_ILI9488_320X480_NULL_1LANE_60FPS_VACT,
        .vid_vsa_pos_polarity = false,
        .vid_hsa_pos_polarity = false,
    },
    .pixel_clk = 20000,
};

const struct hs_settle_s hs_timing_cfg_dsi_ili9488_320x480_NULL_1lane_60fps = { .prepare = 6, .zero = 32, .trail = 1 };
struct dsc_instr dsi_init_cmds_dsi_ili9488_320x480_NULL_1lane_60fps[] = {
    DSI_CMD(0, 0xF7, 0xA9, 0x51, 0x2C, 0x82),
    DSI_CMD(0, 0x36, 0x48),
    DSI_CMD(0, 0x3A, 0x55),
    DSI_CMD(0, 0xB4, 0x02),
    DSI_CMD(0, 0xB1, 0xA0, 0x11), /* FRAME RATE */
    /* Power Control 1   Vreg1out=4.56  Vreg2out=-4.56 */
    DSI_CMD(0, 0xC0, 0x0F, 0x0F),
    /* Power Control 2    VGH=15.81 ,VGL=-10.41,DDVDH=5.35，DDVDL=-5.23  VCL=-2.7 */
    DSI_CMD(0, 0xC1, 0x41),
    DSI_CMD(0, 0xC2, 0x22),
    DSI_CMD(0, 0xB7, 0xC6),
    DSI_CMD(0, 0xC5, 0x00, 0x53, 0x80), /* DC VCOM */
    DSI_CMD(0, 0xBE, 0x00, 0x04),
    DSI_CMD(0, 0xE9, 0x00),
    DSI_CMD(0, 0xE0, 0x00, 0x08, 0x0C, 0x02, 0x0E, 0x04, 0x30, 0x45, 0x47, 0x04, 0x0C, 0x0A, 0x2E, 0x34, 0x0F),
    DSI_CMD(0, 0xE1, 0x00, 0x11, 0x0D, 0x01, 0x0F, 0x05, 0x39, 0x36, 0x51, 0x06, 0x0F, 0x0D, 0x33, 0x37, 0x0F),
    DSI_CMD(0, 0x21),
    DSI_CMD(150, 0x11),
    DSI_CMD(0, 0x29),
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // PANELS_DSI_ILI9488_320X480_NULL_1LANE_60FPS_H
