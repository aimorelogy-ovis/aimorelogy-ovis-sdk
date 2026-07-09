#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
// #include <aos/cli.h>
// #include "pinctrl-mars.h"
// #include "top_reg.h"
#include <base_cb.h>

#include "mmio.h"
// #include "interrupt.h"
// #include "drv/cvi_irq.h"

#include <cif_uapi.h>

#include "cif.h"
#include "cif_cb.h"
#include "cif_reg.h"
#include "cif_uapi.h"
#include "list.h"
#include "vi_snsr.h"
#include "vip_common.h"

#define CONFIG_PROC_FS
#define MIPI_IF

#define MIPI_RX_DEV_NAME "cvi-mipi-rx"
#define MAX_CIF_PROC_BUF 32

#define dev_dbg(...) \
    {}

enum {
    LANE_SKEW_CROSS_CLK,
    LANE_SKEW_CROSS_DATA_NEAR,
    LANE_SKEW_CROSS_DATA_FAR,
    LANE_SKEW_CLK,
    LANE_SKEW_DATA,
    LANE_SKEW_NUM,
};

static int lane_phase[LANE_SKEW_NUM] = {0x00, 0x03, 0x08, 0x00, 0x03};

static struct cvi_cif_dev *cvi_cif_pdev;

static int mclk0 = CAMPLL_FREQ_NONE;
static int mclk1 = CAMPLL_FREQ_NONE;
static int mclk2 = CAMPLL_FREQ_NONE;

static unsigned int irq_num[MAX_LINK_NUM] = {
    CNTP_CSI_MAC0_INT_ID,
    CNTP_CSI_MAC1_INT_ID,
    CNTP_CSI_MAC2_INT_ID,
};

static int cif_set_output_clk_edge(struct cvi_cif_dev *dev, struct clk_edge_s *clk_edge);

const struct sync_code_s default_sync_code = {
    .norm_bk_sav = 0xAB0,
    .norm_bk_eav = 0xB60,
    .norm_sav    = 0x800,
    .norm_eav    = 0x9D0,
    .n0_bk_sav   = 0x2B0,
    .n0_bk_eav   = 0x360,
    .n1_bk_sav   = 0x6B0,
    .n1_bk_eav   = 0x760,
};
#if 0
static struct cvi_link *ctx_to_link(const struct cif_ctx *ctx)
{
	return container_of(ctx, struct cvi_link, cif_ctx);
}
#endif
const char *_to_string_input_mode(enum input_mode_e input_mode)
{
    switch (input_mode) {
    case INPUT_MODE_MIPI:
        return "MIPI";
    case INPUT_MODE_SUBLVDS:
        return "SUBLVDS";
    case INPUT_MODE_HISPI:
        return "HISPI";
    case INPUT_MODE_CMOS:
        return "CMOS";
    case INPUT_MODE_BT1120:
        return "BT1120";
    case INPUT_MODE_BT601_19B_VHS:
        return "INPUT_MODE_BT601_19B_VHS";
    case INPUT_MODE_BT656_9B:
        return "INPUT_MODE_BT656_9B";
    case INPUT_MODE_CUSTOM_0:
        return "INPUT_MODE_CUSTOM_0";
    case INPUT_MODE_BT_DEMUX:
        return "INPUT_MODE_BT_DEMUX";
    default:
        return "unknown";
    }
}

const char *_to_string_mac_clk(enum rx_mac_clk_e mac_clk)
{
    switch (mac_clk) {
    case RX_MAC_CLK_200M:
        return "200MHZ";
    case RX_MAC_CLK_300M:
        return "300MHZ";
    case RX_MAC_CLK_400M:
        return "400MHZ";
    case RX_MAC_CLK_500M:
        return "500MHZ";
    case RX_MAC_CLK_600M:
        return "600MHZ";
    default:
        return "unknown";
    }
}

#ifdef CVITEK
const char *_to_string_cmd(unsigned int cmd)
{
    switch (cmd) {
    case CVI_MIPI_SET_DEV_ATTR:
        return "CVI_MIPI_SET_DEV_ATTR";
    case CVI_MIPI_SET_HS_MODE:
        return "CVI_MIPI_SET_HS_MODE";
    case CVI_MIPI_SET_OUTPUT_CLK_EDGE:
        return "CVI_MIPI_SET_OUTPUT_CLK_EDGE";
    case CVI_MIPI_RESET_MIPI:
        return "CVI_MIPI_RESET_MIPI";
    case CVI_MIPI_SET_CROP_TOP:
        return "CVI_MIPI_SET_CROP_TOP";
    case CVI_MIPI_SET_WDR_MANUAL:
        return "CVI_MIPI_SET_WDR_MANUAL";
    case CVI_MIPI_SET_LVDS_FP_VS:
        return "CVI_MIPI_SET_LVDS_FP_VS";
    case CVI_MIPI_RESET_SENSOR:
        return "CVI_MIPI_RESET_SENSOR";
    case CVI_MIPI_UNRESET_SENSOR:
        return "CVI_MIPI_UNRESET_SENSOR";
    case CVI_MIPI_ENABLE_SENSOR_CLOCK:
        return "CVI_MIPI_ENABLE_SENSOR_CLOCK";
    case CVI_MIPI_DISABLE_SENSOR_CLOCK:
        return "CVI_MIPI_DISABLE_SENSOR_CLOCK";
    case CVI_MIPI_RESET_LVDS:
        return "CVI_MIPI_RESET_LVDS";
    case CVI_MIPI_GET_CIF_ATTR:
        return "CVI_MIPI_GET_CIF_ATTR";
    case CVI_MIPI_SET_MAX_MAC_CLOCK:
        return "CVI_MIPI_SET_MAX_MAC_CLOCK";
    case CVI_MIPI_SET_CROP_WINDOW:
        return "CVI_MIPI_SET_CROP_WINDOW";
    default:
        return "unknown";
    }
    return "unknown";
}
#endif

const char *_to_string_raw_data_type(enum raw_data_type_e raw_data_type)
{
    switch (raw_data_type) {
    case RAW_DATA_8BIT:
        return "RAW8";
    case RAW_DATA_10BIT:
        return "RAW10";
    case RAW_DATA_12BIT:
        return "RAW12";
    case YUV422_8BIT:
        return "YUV422_8BIT";
    case YUV422_10BIT:
        return "YUV422_10BIT";
    default:
        return "unknown";
    }
}

const char *_to_string_mipi_wdr_mode(enum mipi_wdr_mode_e wdr)
{
    switch (wdr) {
    case CVI_MIPI_WDR_MODE_NONE:
        return "NONE";
    case CVI_MIPI_WDR_MODE_VC:
        return "VC";
    case CVI_MIPI_WDR_MODE_DT:
        return "DT";
    case CVI_MIPI_WDR_MODE_DOL:
        return "DOL";
    case CVI_MIPI_WDR_MODE_MANUAL:
        return "MANUAL";
    default:
        return "unknown";
    }
}

const char *_to_string_wdr_mode(enum wdr_mode_e wdr)
{
    switch (wdr) {
    case CVI_WDR_MODE_NONE:
        return "NONE";
    case CVI_WDR_MODE_2F:
        return "2To1";
    case CVI_WDR_MODE_3F:
        return "3To1";
    case CVI_WDR_MODE_DOL_2F:
        return "DOL2To1";
    case CVI_WDR_MODE_DOL_3F:
        return "DOL3To1";
    default:
        return "unknown";
    }
}

const char *_to_string_lvds_sync_mode(enum lvds_sync_mode_e mode)
{
    switch (mode) {
    case LVDS_SYNC_MODE_SOF:
        return "SOF";
    case LVDS_SYNC_MODE_SAV:
        return "SAV";
    default:
        return "unknown";
    }
}

const char *_to_string_bit_endian(enum lvds_bit_endian endian)
{
    switch (endian) {
    case LVDS_ENDIAN_LITTLE:
        return "LITTLE";
    case LVDS_ENDIAN_BIG:
        return "BIG";
    default:
        return "unknown";
    }
}

const char *_to_string_lvds_vsync_type(enum lvds_vsync_type_e type)
{
    switch (type) {
    case LVDS_VSYNC_NORMAL:
        return "NORMAL";
    case LVDS_VSYNC_SHARE:
        return "SHARE";
    case LVDS_VSYNC_HCONNECT:
        return "HCONNECT";
    default:
        return "unknown";
    }
}

const char *_to_string_lvds_fid_type(enum lvds_fid_type_e type)
{
    switch (type) {
    case LVDS_FID_NONE:
        return "FID_NONE";
    case LVDS_FID_IN_SAV:
        return "FID_IN_SAV";
    default:
        return "unknown";
    }
}

const char *_to_string_mclk(enum cam_pll_freq_e freq)
{
    switch (freq) {
    case CAMPLL_FREQ_NONE:
        return "CAMPLL_FREQ_NONE";
    case CAMPLL_FREQ_37P125M:
        return "CAMPLL_FREQ_37P125M";
    case CAMPLL_FREQ_25M:
        return "CAMPLL_FREQ_25M";
    case CAMPLL_FREQ_27M:
        return "CAMPLL_FREQ_27M";
    case CAMPLL_FREQ_24M:
        return "CAMPLL_FREQ_24M";
    case CAMPLL_FREQ_26M:
        return "CAMPLL_FREQ_26M";
    default:
        return "unknown";
    }
}

const char *_to_string_csi_decode(enum csi_decode_fmt_e fmt)
{
    switch (fmt) {
    case DEC_FMT_YUV422_8:
        return "yuv422-8";
    case DEC_FMT_YUV422_10:
        return "yuv422-10";
    case DEC_FMT_RAW8:
        return "raw8";
    case DEC_FMT_RAW10:
        return "raw10";
    case DEC_FMT_RAW12:
        return "raw12";
    default:
        return "unknown";
    }
}

const char *_to_string_dlane_state(enum mipi_dlane_state_e state)
{
    switch (state) {
    case HS_IDLE:
        return "hs_idle";
    case HS_SYNC:
        return "hs_sync";
    case HS_SKEW_CAL:
        return "skew_cal";
    case HS_ALT_CAL:
        return "alt_cal";
    case HS_PREAMPLE:
        return "preample";
    case HS_HST:
        return "hs_hst";
    case HS_ERR:
        return "hs_err";
    default:
        return "unknown";
    }
}

const char *_to_string_deskew_state(enum mipi_deskew_state_e state)
{
    switch (state) {
    case DESKEW_IDLE:
        return "idle";
    case DESKEW_START:
        return "start";
    case DESKEW_DONE:
        return "done";
    default:
        return "unknown";
    }
}

static void cif_dump_dev_attr(struct cvi_cif_dev *dev, struct combo_dev_attr_s *attr)
{
    // int i;

    dev_dbg("devno = %d, input_mode = %s, mac_clk = %s\n", attr->devno,
            _to_string_input_mode(attr->input_mode), _to_string_mac_clk(attr->mac_clk));
    dev_dbg("mclk%d = %s\n", attr->mclk.cam, _to_string_mclk(attr->mclk.freq));
    dev_dbg("width = %d, height = %d\n", attr->img_size.width, attr->img_size.height);
    switch (attr->input_mode) {
    case INPUT_MODE_MIPI: {
#if 0
		struct mipi_dev_attr_s *mipi = &attr->mipi_attr;

		dev_dbg("raw_data_type = %s\n",
			_to_string_raw_data_type(mipi->raw_data_type));

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			dev_dbg("lane_id[%d] = %d, pn_swap = %s", i,
				mipi->lane_id[i],
				mipi->pn_swap[i] ? "True" : "False");
		}
		dev_dbg("wdr_mode = %s\n",
			_to_string_mipi_wdr_mode(mipi->wdr_mode));
		for (i = 0; i < WDR_VC_NUM; i++) {
			dev_dbg("data_type[%d] = 0x%x\n",
				i, mipi->data_type[i]);
		}
#endif
    } break;
    case INPUT_MODE_SUBLVDS:
    case INPUT_MODE_HISPI: {
#if 0
		struct lvds_dev_attr_s *lvds = &attr->lvds_attr;
		int j;

		dev_dbg("wdr_mode = %s\n",
			_to_string_wdr_mode(lvds->wdr_mode));
		dev_dbg("sync_mode = %s\n",
			_to_string_lvds_sync_mode(lvds->sync_mode));
		dev_dbg("raw_data_type = %s\n",
			_to_string_raw_data_type(lvds->raw_data_type));
		dev_dbg("data_endian = %s\n",
			_to_string_bit_endian(lvds->data_endian));
		dev_dbg("sync_code_endian = %s\n",
			_to_string_bit_endian(lvds->sync_code_endian));
		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			dev_dbg("lane_id[%d] = %d, pn_swap = %s ", i,
				lvds->lane_id[i],
				lvds->pn_swap[i] ? "True" : "False");
		}
		dev_dbg("sync code = {\n");
		for (i = 0; i < MIPI_LANE_NUM; i++) {
			dev_dbg("\t{\n");
			for (j = 0; j < WDR_VC_NUM+1; j++) {
				dev_dbg(_dev,
					"\t\t{ %3x, %3x, %3x, %3x },\n",
					lvds->sync_code[i][j][0],
					lvds->sync_code[i][j][1],
					lvds->sync_code[i][j][2],
					lvds->sync_code[i][j][3]);
			}
			dev_dbg("\t},\n");
		}
		dev_dbg("}\n");
		dev_dbg("vsync_type = %s\n",
			_to_string_lvds_vsync_type(lvds->vsync_type.sync_type));
		dev_dbg("fid = %s\n",
			_to_string_lvds_fid_type(lvds->fid_type.fid));
#endif
    } break;
    case INPUT_MODE_CMOS:
        break;
    case INPUT_MODE_BT1120:
        break;
    default:
        break;
    }
}

#define LANE_IS_PORT1(x)   ((x > 2) ? 1 : 0)
#define IS_SAME_PORT(x, y) (!((LANE_IS_PORT1(x)) ^ (LANE_IS_PORT1(y))))

#define PAD_CTRL_PU        BIT(2)
#define PAD_CTRL_PD        BIT(3)
#define PSD_CTRL_OFFSET(n) ((5 - n) * 8)

#ifdef MIPI_IF
static int _cif_set_attr_mipi(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                              struct mipi_dev_attr_s *attr)
{
    struct combo_dev_attr_s  *combo = container_of(attr, struct combo_dev_attr_s, mipi_attr);
    struct cif_param         *param = ctx->cur_config;
    struct param_csi         *csi   = &param->cfg.csi;
    struct mipi_demux_info_s *info  = &attr->demux;
    struct cvi_link          *link  = &cvi_cif_pdev->link[0];
    uint8_t                   tbl   = 0x1F;
    int                       i, j = 0, clk_port = 0;
    uint32_t                  value;

    param->type = CIF_TYPE_CSI;
    /* config the bit mode. */
    switch (attr->raw_data_type) {
    case RAW_DATA_8BIT:
        csi->fmt         = CSI_RAW_8;
        csi->decode_type = 0x2A;
        break;
    case RAW_DATA_10BIT:
        csi->fmt         = CSI_RAW_10;
        csi->decode_type = 0x2B;
        break;
    case RAW_DATA_12BIT:
        csi->fmt         = CSI_RAW_12;
        csi->decode_type = 0x2C;
        break;
    case YUV422_8BIT:
        csi->fmt         = CSI_YUV422_8B;
        csi->decode_type = 0x1E;
        break;
    case YUV422_10BIT:
        csi->fmt         = CSI_YUV422_10B;
        csi->decode_type = 0x1F;
        break;
    default:
        return -EINVAL;
    }
    /* config the lane id */
    for (i = 0; i < CIF_LANE_NUM; i++) {
        if (attr->lane_id[i] < 0) continue;
        if (attr->lane_id[i] >= CIF_PHY_LANE_NUM) return -EINVAL;
        if (!i)
            clk_port = LANE_IS_PORT1(attr->lane_id[i]);
        else {
            if (LANE_IS_PORT1(attr->lane_id[i]) != clk_port) clk_port = -1;
        }
        cif_set_lane_id(ctx, i, attr->lane_id[i], attr->pn_swap[i]);
        /* clear pad ctrl pu/pd */
        if (dev->pad_ctrl) {
            value = ioread32(dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]));
            value &= ~(PAD_CTRL_PU | PAD_CTRL_PD);
            iowrite32(value, dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]));
            value = ioread32(dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]) + 4);
            value &= ~(PAD_CTRL_PU | PAD_CTRL_PD);
            iowrite32(value, dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]) + 4);
        }
        tbl &= ~(1 << attr->lane_id[i]);
        j++;
    }
    csi->lane_num = j - 1;
    while (ffs(tbl)) {
        uint32_t idx = ffs(tbl) - 1;

        cif_set_lane_id(ctx, j++, idx, 0);
        tbl &= ~(1 << idx);
    }
    /* config  clock buffer direction.
     * 1. When clock is between 0~2 and 1c4d, direction is P0->P1.
     * 2. When clock is between 3~5 and 1c4d, direction is P1->P0.
     * 3. When clock and data is between 0~2 and 1c2d, direction bit is freerun.
     * 4. When clock is between 0~2 but data is not, direction is P0->P1.
     * 5. When clock and data is between 3~5 and 1c2d and mac1 is used, direction bit is freerun.
     * 6. When clock is between 3~5 but data is not, direction is P1->P0.
     */
    if (csi->lane_num == 4) {
        if (LANE_IS_PORT1(attr->lane_id[0]))
            cif_set_clk_dir(ctx, CIF_CLK_P12P0);
        else
            cif_set_clk_dir(ctx, CIF_CLK_P02P1);

        for (i = 0; (i < csi->lane_num + 1); i++) {
            if (!i)
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_CLK]);
            else if (IS_SAME_PORT(attr->lane_id[0], attr->lane_id[i]))
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_DATA_NEAR]);
            else
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_DATA_FAR]);
        }
    } else if (csi->lane_num > 0) {
        if (ctx->mac_num || !clk_port) {
            cif_set_clk_dir(ctx, CIF_CLK_FREERUN);
        } else {
            if (LANE_IS_PORT1(attr->lane_id[0]))
                cif_set_clk_dir(ctx, CIF_CLK_P12P0);
            else
                cif_set_clk_dir(ctx, CIF_CLK_P02P1);
        }
        /* if clk and data are in the same port.*/
        if (clk_port > 0) {
            for (i = 0; i < (csi->lane_num + 1); i++) {
                if (!i)
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CLK]);
                else
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_DATA]);
            }
        } else {
            for (i = 0; i < (csi->lane_num + 1); i++) {
                if (!i)
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_CLK]);
                else if (IS_SAME_PORT(attr->lane_id[0], attr->lane_id[i]))
                    cif_set_lane_deskew(ctx, attr->lane_id[i],
                                        lane_phase[LANE_SKEW_CROSS_DATA_NEAR]);
                else
                    cif_set_lane_deskew(ctx, attr->lane_id[i],
                                        lane_phase[LANE_SKEW_CROSS_DATA_FAR]);
            }
        }
    } else
        return -EINVAL;

    /* config the dphy seting. */
    if (attr->dphy.enable) cif_set_hs_settle(ctx, attr->dphy.hs_settle);
    /* config the wdr mode. */
    switch (attr->wdr_mode) {
    case CVI_MIPI_WDR_MODE_NONE:
        break;
    case CVI_MIPI_WDR_MODE_DT:
        csi->hdr_mode = CSI_HDR_MODE_DT;
        for (i = 0; i < MAX_WDR_FRAME_NUM; i++) csi->data_type[i] = attr->data_type[i];
        break;
    case CVI_MIPI_WDR_MODE_MANUAL:
        param->hdr_manual     = combo->wdr_manu.manual_en;
        param->hdr_shift      = combo->wdr_manu.l2s_distance;
        param->hdr_vsize      = combo->wdr_manu.lsef_length;
        param->hdr_rm_padding = combo->wdr_manu.discard_padding_lines;
        cif_hdr_manual_config(ctx, param, !!combo->wdr_manu.update);
        break;
    case CVI_MIPI_WDR_MODE_VC:
        csi->hdr_mode = CSI_HDR_MODE_VC;
        break;
    case CVI_MIPI_WDR_MODE_DOL:
        csi->hdr_mode = CSI_HDR_MODE_DOL;
        break;
    default:
        return -EINVAL;
    }
    // cif_streaming(ctx, 1, attr->wdr_mode != CVI_MIPI_WDR_MODE_NONE);
    /* config vc mapping */
    if (info->demux_en) {
        for (i = 0; i < MIPI_DEMUX_NUM; i++) {
            csi->vc_mapping[i] = info->vc_mapping[i];
        }
    } else {
        for (i = 0; i < MIPI_DEMUX_NUM; i++) {
            csi->vc_mapping[i] = i;
        }
    }

    param->hdr_en = (attr->wdr_mode != CVI_MIPI_WDR_MODE_NONE);
    cif_streaming(ctx, 1, param->hdr_en);
    link->is_on = 1;
    return 0;
}
#endif  // MIPI_IF

#ifdef SUBLVDS_IF
static int _cif_set_lvds_vsync_type(struct cif_ctx *ctx, struct lvds_dev_attr_s *attr,
                                    struct param_sublvds *sublvds)
{
    struct combo_dev_attr_s  *combo = container_of(attr, struct combo_dev_attr_s, lvds_attr);
    struct lvds_vsync_type_s *type  = &attr->vsync_type;
    struct cvi_link          *link  = ctx_to_link(ctx);

    switch (type->sync_type) {
    case LVDS_VSYNC_NORMAL:
        sublvds->hdr_mode = CIF_SLVDS_HDR_PAT1;
        /* [TODO] use other api to set the fp */
        link->distance_fp = 15;
        cif_set_lvds_vsync_gen(ctx, 15);
        break;
    case LVDS_VSYNC_HCONNECT:
        sublvds->hdr_mode = CIF_SLVDS_HDR_PAT2;
        /* [TODO] use other api to set the fp */
        link->distance_fp = 1;
        cif_set_lvds_vsync_gen(ctx, 1);
        sublvds->hdr_hblank[0] = type->hblank1;
        sublvds->hdr_hblank[1] = type->hblank2;
        sublvds->h_size        = combo->img_size.width;
        /* [TODO] use other api to strip the info line*/
        /* strip the top info line. */
        cif_crop_info_line(ctx, 1, 1);
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int _cif_set_attr_sublvds(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                                 struct lvds_dev_attr_s *attr)
{
    struct cif_param         *param   = ctx->cur_config;
    struct cvi_link          *link    = ctx_to_link(ctx);
    struct param_sublvds     *sublvds = &param->cfg.sublvds;
    uint8_t                   tbl     = 0x1F;
    struct sublvds_sync_code *sc;
    int                       i, j = 0, clk_port = 0;
    int                       rc = 0;
    uint32_t                  value;

    param->type = CIF_TYPE_SUBLVDS;
    /* config the bit mode. */
    switch (attr->raw_data_type) {
    case RAW_DATA_8BIT:
        sublvds->fmt = CIF_SLVDS_8_BIT;
        break;
    case RAW_DATA_10BIT:
        sublvds->fmt = CIF_SLVDS_10_BIT;
        break;
    case RAW_DATA_12BIT:
        sublvds->fmt = CIF_SLVDS_12_BIT;
        break;
    default:
        return -EINVAL;
    }
    /* config the endian. */
    if (attr->data_endian == LVDS_ENDIAN_BIG && attr->sync_code_endian == LVDS_ENDIAN_BIG) {
        sublvds->endian      = CIF_SLVDS_ENDIAN_MSB;
        sublvds->wrap_endian = CIF_SLVDS_ENDIAN_MSB;

    } else if (attr->data_endian == LVDS_ENDIAN_LITTLE &&
               attr->sync_code_endian == LVDS_ENDIAN_BIG) {
        sublvds->endian      = CIF_SLVDS_ENDIAN_LSB;
        sublvds->wrap_endian = CIF_SLVDS_ENDIAN_MSB;

    } else if (attr->data_endian == LVDS_ENDIAN_BIG &&
               attr->sync_code_endian == LVDS_ENDIAN_LITTLE) {
        sublvds->endian      = CIF_SLVDS_ENDIAN_LSB;
        sublvds->wrap_endian = CIF_SLVDS_ENDIAN_LSB;
    } else {
        sublvds->endian      = CIF_SLVDS_ENDIAN_MSB;
        sublvds->wrap_endian = CIF_SLVDS_ENDIAN_LSB;
    }
    /* check the sync mode. */
    if (attr->sync_mode != LVDS_SYNC_MODE_SAV) return -EINVAL;
    /* config the lane id*/
    for (i = 0; i < CIF_LANE_NUM; i++) {
        if (attr->lane_id[i] < 0) continue;
        if (attr->lane_id[i] >= CIF_PHY_LANE_NUM) return -EINVAL;
        if (!i)
            clk_port = LANE_IS_PORT1(attr->lane_id[i]);
        else {
            if (LANE_IS_PORT1(attr->lane_id[i]) != clk_port) clk_port = -1;
        }
        cif_set_lane_id(ctx, i, attr->lane_id[i], attr->pn_swap[i]);
        /* clear pad ctrl pu/pd */
        if (dev->pad_ctrl) {
            value = ioread32(dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]));
            value &= ~(PAD_CTRL_PU | PAD_CTRL_PD);
            iowrite32(value, dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]));
            value = ioread32(dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]) + 4);
            value &= ~(PAD_CTRL_PU | PAD_CTRL_PD);
            iowrite32(value, dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]) + 4);
        }
        tbl &= ~(1 << attr->lane_id[i]);
        j++;
    }
    sublvds->lane_num = j - 1;
    while (ffs(tbl)) {
        uint32_t idx = ffs(tbl) - 1;

        cif_set_lane_id(ctx, j++, idx, 0);
        tbl &= ~(1 << idx);
    }
    /* config  clock buffer direction.
     * 1. When clock is between 0~2 and 1c4d, direction is P0->P1.
     * 2. When clock is between 3~5 and 1c4d, direction is P1->P0.
     * 3. When clock and data is between 0~2 and 1c2d, direction bit is freerun.
     * 4. When clock is between 0~2 but data is not, direction is P0->P1.
     * 5. When clock and data is between 3~5 and 1c2d and mac1 is used, direction bit is freerun.
     * 6. When clock is between 3~5 but data is not, direction is P1->P0.
     */
    if (sublvds->lane_num == 4) {
        if (LANE_IS_PORT1(attr->lane_id[0]))
            cif_set_clk_dir(ctx, CIF_CLK_P12P0);
        else
            cif_set_clk_dir(ctx, CIF_CLK_P02P1);
        for (i = 0; (i < sublvds->lane_num + 1); i++) {
            if (!i)
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_CLK]);
            else if (IS_SAME_PORT(attr->lane_id[0], attr->lane_id[i]))
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_DATA_NEAR]);
            else
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_DATA_FAR]);
        }
    } else if (sublvds->lane_num > 0) {
        if (ctx->mac_num || !clk_port) {
            cif_set_clk_dir(ctx, CIF_CLK_FREERUN);
        } else {
            if (LANE_IS_PORT1(attr->lane_id[0]))
                cif_set_clk_dir(ctx, CIF_CLK_P12P0);
            else
                cif_set_clk_dir(ctx, CIF_CLK_P02P1);
        }
        /* if clk and data are in the same port.*/
        if (clk_port > 0) {
            for (i = 0; i < (sublvds->lane_num + 1); i++) {
                if (!i)
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CLK]);
                else
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_DATA]);
            }
        } else {
            for (i = 0; i < (sublvds->lane_num + 1); i++) {
                if (!i)
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_CLK]);
                else if (IS_SAME_PORT(attr->lane_id[0], attr->lane_id[i]))
                    cif_set_lane_deskew(ctx, attr->lane_id[i],
                                        lane_phase[LANE_SKEW_CROSS_DATA_NEAR]);
                else
                    cif_set_lane_deskew(ctx, attr->lane_id[i],
                                        lane_phase[LANE_SKEW_CROSS_DATA_FAR]);
            }
        }
    } else
        return -EINVAL;

    /* config the sync code */
    memcpy(&sublvds->sync_code, &default_sync_code, sizeof(default_sync_code));
    sc              = &sublvds->sync_code.slvds;
    sc->n0_lef_sav  = attr->sync_code[0][0][0];
    sc->n0_lef_eav  = attr->sync_code[0][0][1];
    sc->n1_lef_sav  = attr->sync_code[0][0][2];
    sc->n1_lef_eav  = attr->sync_code[0][0][3];
    sc->n0_sef_sav  = attr->sync_code[0][1][0];
    sc->n0_sef_eav  = attr->sync_code[0][1][1];
    sc->n1_sef_sav  = attr->sync_code[0][1][2];
    sc->n1_sef_eav  = attr->sync_code[0][1][3];
    sc->n0_lsef_sav = attr->sync_code[0][2][0];
    sc->n0_lsef_eav = attr->sync_code[0][2][1];
    sc->n1_lsef_sav = attr->sync_code[0][2][2];
    sc->n1_lsef_eav = attr->sync_code[0][2][3];

    /* config the wdr */
    switch (attr->wdr_mode) {
    case CVI_WDR_MODE_NONE:
        /* [TODO] use other api to set the fp */
        link->distance_fp = 6;
        cif_set_lvds_vsync_gen(ctx, 6);
        break;
    case CVI_WDR_MODE_DOL_2F:
    case CVI_WDR_MODE_DOL_3F:
        /* [TODO] 3 exposure hdr hw is not ready. */
        /* config th Vsync type */
        rc = _cif_set_lvds_vsync_type(ctx, attr, sublvds);
        if (rc < 0) return rc;
        break;
    default:
        return -EINVAL;
    }
    param->hdr_en = (attr->wdr_mode != CVI_WDR_MODE_NONE);
    /* [TODO] config the fid type. */
    cif_streaming(ctx, 1, attr->wdr_mode != CVI_WDR_MODE_NONE);

    return 0;
}
#endif  // SUBLVDS_IF

#ifdef HISPI_IF
static int _cif_set_hispi_vsync_type(struct cif_ctx *ctx, struct lvds_dev_attr_s *attr,
                                     struct cif_param *param)
{
    struct combo_dev_attr_s  *combo = container_of(attr, struct combo_dev_attr_s, lvds_attr);
    struct lvds_vsync_type_s *type  = &attr->vsync_type;

    switch (type->sync_type) {
    case LVDS_VSYNC_NORMAL:
        break;
    case LVDS_VSYNC_SHARE:
        param->hdr_manual     = combo->wdr_manu.manual_en;
        param->hdr_shift      = combo->wdr_manu.l2s_distance;
        param->hdr_vsize      = combo->wdr_manu.lsef_length;
        param->hdr_rm_padding = combo->wdr_manu.discard_padding_lines;
        cif_hdr_manual_config(ctx, param, !!combo->wdr_manu.update);
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int _cif_set_attr_hispi(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                               struct lvds_dev_attr_s *attr)
{
    struct combo_dev_attr_s *combo = container_of(attr, struct combo_dev_attr_s, lvds_attr);
    struct cif_param        *param = ctx->cur_config;
    struct param_hispi      *hispi = &param->cfg.hispi;
    uint8_t                  tbl   = 0x1F;
    struct hispi_sync_code  *sc;
    int                      i, j = 0, clk_port = 0;
    int                      rc = 0;
    uint32_t                 value;

    param->type = CIF_TYPE_HISPI;
    /* config the bit mode. */
    switch (attr->raw_data_type) {
    case RAW_DATA_8BIT:
        hispi->fmt = CIF_SLVDS_8_BIT;
        break;
    case RAW_DATA_10BIT:
        hispi->fmt = CIF_SLVDS_10_BIT;
        break;
    case RAW_DATA_12BIT:
        hispi->fmt = CIF_SLVDS_12_BIT;
        break;
    default:
        return -EINVAL;
    }
    /* config the endian. */
    if (attr->data_endian == LVDS_ENDIAN_BIG && attr->sync_code_endian == LVDS_ENDIAN_BIG) {
        hispi->endian      = CIF_SLVDS_ENDIAN_MSB;
        hispi->wrap_endian = CIF_SLVDS_ENDIAN_MSB;
    } else if (attr->data_endian == LVDS_ENDIAN_LITTLE &&
               attr->sync_code_endian == LVDS_ENDIAN_BIG) {
        hispi->endian      = CIF_SLVDS_ENDIAN_LSB;
        hispi->wrap_endian = CIF_SLVDS_ENDIAN_MSB;
    } else if (attr->data_endian == LVDS_ENDIAN_BIG &&
               attr->sync_code_endian == LVDS_ENDIAN_LITTLE) {
        hispi->endian      = CIF_SLVDS_ENDIAN_LSB;
        hispi->wrap_endian = CIF_SLVDS_ENDIAN_LSB;
    } else {
        hispi->endian      = CIF_SLVDS_ENDIAN_MSB;
        hispi->wrap_endian = CIF_SLVDS_ENDIAN_LSB;
    }
    /* check the sync mode. */
    if (attr->sync_mode == LVDS_SYNC_MODE_SOF) {
        hispi->mode = CIF_HISPI_MODE_PKT_SP;
    } else if (attr->sync_mode == LVDS_SYNC_MODE_SAV) {
        hispi->mode   = CIF_HISPI_MODE_STREAM_SP;
        hispi->h_size = combo->img_size.width;
    } else {
        return -EINVAL;
    }
    /* config the lane id*/
    for (i = 0; i < CIF_LANE_NUM; i++) {
        if (attr->lane_id[i] < 0) continue;
        if (attr->lane_id[i] >= CIF_PHY_LANE_NUM) return -EINVAL;
        if (!i)
            clk_port = LANE_IS_PORT1(attr->lane_id[i]);
        else {
            if (LANE_IS_PORT1(attr->lane_id[i]) != clk_port) clk_port = -1;
        }
        cif_set_lane_id(ctx, i, attr->lane_id[i], attr->pn_swap[i]);
        /* clear pad ctrl pu/pd */
        if (dev->pad_ctrl) {
            value = ioread32(dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]));
            value &= ~(PAD_CTRL_PU | PAD_CTRL_PD);
            iowrite32(value, dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]));
            value = ioread32(dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]) + 4);
            value &= ~(PAD_CTRL_PU | PAD_CTRL_PD);
            iowrite32(value, dev->pad_ctrl + PSD_CTRL_OFFSET(attr->lane_id[i]) + 4);
        }
        tbl &= ~(1 << attr->lane_id[i]);
        j++;
    }
    hispi->lane_num = j - 1;
    while (ffs(tbl)) {
        uint32_t idx = ffs(tbl) - 1;

        cif_set_lane_id(ctx, j++, idx, 0);
        tbl &= ~(1 << idx);
    }
    /* config  clock buffer direction.
     * 1. When clock is between 0~2 and 1c4d, direction is P0->P1.
     * 2. When clock is between 3~5 and 1c4d, direction is P1->P0.
     * 3. When clock and data is between 0~2 and 1c2d, direction bit is freerun.
     * 4. When clock is between 0~2 but data is not, direction is P0->P1.
     * 5. When clock and data is between 3~5 and 1c2d and mac1 is used, direction bit is freerun.
     * 6. When clock is between 3~5 but data is not, direction is P1->P0.
     */
    if (hispi->lane_num == 4) {
        if (LANE_IS_PORT1(attr->lane_id[0]))
            cif_set_clk_dir(ctx, CIF_CLK_P12P0);
        else
            cif_set_clk_dir(ctx, CIF_CLK_P02P1);
        for (i = 0; (i < hispi->lane_num + 1); i++) {
            if (!i)
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_CLK]);
            else if (IS_SAME_PORT(attr->lane_id[0], attr->lane_id[i]))
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_DATA_NEAR]);
            else
                cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_DATA_FAR]);
        }
    } else if (hispi->lane_num > 0) {
        if (ctx->mac_num || !clk_port) {
            cif_set_clk_dir(ctx, CIF_CLK_FREERUN);
        } else {
            if (LANE_IS_PORT1(attr->lane_id[0]))
                cif_set_clk_dir(ctx, CIF_CLK_P12P0);
            else
                cif_set_clk_dir(ctx, CIF_CLK_P02P1);
        }
        /* if clk and data are in the same port.*/
        if (clk_port > 0) {
            for (i = 0; i < (hispi->lane_num + 1); i++) {
                if (!i)
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CLK]);
                else
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_DATA]);
            }
        } else {
            for (i = 0; i < (hispi->lane_num + 1); i++) {
                if (!i)
                    cif_set_lane_deskew(ctx, attr->lane_id[i], lane_phase[LANE_SKEW_CROSS_CLK]);
                else if (IS_SAME_PORT(attr->lane_id[0], attr->lane_id[i]))
                    cif_set_lane_deskew(ctx, attr->lane_id[i],
                                        lane_phase[LANE_SKEW_CROSS_DATA_NEAR]);
                else
                    cif_set_lane_deskew(ctx, attr->lane_id[i],
                                        lane_phase[LANE_SKEW_CROSS_DATA_FAR]);
            }
        }
    } else
        return -EINVAL;

    /* config the sync code */
    memcpy(&hispi->sync_code, &default_sync_code, sizeof(default_sync_code));
    sc            = &hispi->sync_code.hispi;
    sc->t1_sol    = attr->sync_code[0][0][0];
    sc->t1_eol    = attr->sync_code[0][0][1];
    sc->t1_sof    = attr->sync_code[0][0][2];
    sc->t1_eof    = attr->sync_code[0][0][3];
    sc->t2_sol    = attr->sync_code[0][1][0];
    sc->t2_eol    = attr->sync_code[0][1][1];
    sc->t2_sof    = attr->sync_code[0][1][2];
    sc->t2_eof    = attr->sync_code[0][1][3];
    sc->vsync_gen = sc->t1_sof;

    /* config the wdr */
    switch (attr->wdr_mode) {
    case CVI_WDR_MODE_NONE:
    case CVI_WDR_MODE_2F:
    case CVI_WDR_MODE_3F: /* [TODO] 3 exposure hdr hw is not ready. */
        break;
    default:
        return -EINVAL;
    }
    /* config th Vsync type */
    rc = _cif_set_hispi_vsync_type(ctx, attr, param);
    if (rc < 0) return rc;

    param->hdr_en = (attr->wdr_mode != CVI_WDR_MODE_NONE);
    /* [TODO] config the fid type. */
    cif_streaming(ctx, 1, attr->wdr_mode != CVI_WDR_MODE_NONE);

    return 0;
}
#endif  // HISPI_IF
#define MAX_PAD_NUM 19

#ifdef DVP_IF
struct vi_pin_info {
    uint32_t addr;
    uint32_t offset;
    uint32_t mask;
    uint32_t func;
};

const struct vi_pin_info vi_pin[TTL_VI_SRC_NUM][MAX_PAD_NUM] = {
    [TTL_VI_SRC_VI0] =
        {
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4P_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3N_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3P_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2N_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2P_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX1N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX1N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX1N_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX1P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX1P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX1P_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX0N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX0N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX0N_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX0P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX0P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX0P_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM2,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM2_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM2_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP2,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP2_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP2_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM1,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM1_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM1_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP1,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP1_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP1_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM0,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM0_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXM0_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP0,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP0_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPI_TXP0_MASK,
                1,
            },
        },
    [TTL_VI_SRC_VI1] =
        {
            {
                FMUX_GPIO_FUNCSEL_VIVO_D0,
                FMUX_GPIO_FUNCSEL_VIVO_D0_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D0_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D1,
                FMUX_GPIO_FUNCSEL_VIVO_D1_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D1_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D2,
                FMUX_GPIO_FUNCSEL_VIVO_D2_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D2_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D3,
                FMUX_GPIO_FUNCSEL_VIVO_D3_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D3_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D4,
                FMUX_GPIO_FUNCSEL_VIVO_D4_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D4_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D5,
                FMUX_GPIO_FUNCSEL_VIVO_D5_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D5_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D6,
                FMUX_GPIO_FUNCSEL_VIVO_D6_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D6_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D7,
                FMUX_GPIO_FUNCSEL_VIVO_D7_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D7_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D8,
                FMUX_GPIO_FUNCSEL_VIVO_D8_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D8_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D9,
                FMUX_GPIO_FUNCSEL_VIVO_D9_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D9_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D10,
                FMUX_GPIO_FUNCSEL_VIVO_D10_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D10_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX5N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX5N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX5N_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX5P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX5P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX5P_MASK,
                1,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4N_MASK,
                2,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX4P_MASK,
                2,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3N_MASK,
                2,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3P,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3P_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX3P_MASK,
                2,
            },
            {
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2N,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2N_OFFSET,
                FMUX_GPIO_FUNCSEL_PAD_MIPIRX2N_MASK,
                4,
            },
            {FMUX_GPIO_FUNCSEL_PAD_MIPIRX2P, FMUX_GPIO_FUNCSEL_PAD_MIPIRX2P_OFFSET,
             FMUX_GPIO_FUNCSEL_PAD_MIPIRX2P_MASK, 4},
        },
    [TTL_VI_SRC_VI2] =
        {
            {
                FMUX_GPIO_FUNCSEL_VIVO_D0,
                FMUX_GPIO_FUNCSEL_VIVO_D0_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D0_MASK,
                0,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D1,
                FMUX_GPIO_FUNCSEL_VIVO_D1_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D1_MASK,
                0,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D2,
                FMUX_GPIO_FUNCSEL_VIVO_D2_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D2_MASK,
                0,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D3,
                FMUX_GPIO_FUNCSEL_VIVO_D3_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D3_MASK,
                0,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D4,
                FMUX_GPIO_FUNCSEL_VIVO_D4_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D4_MASK,
                0,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D5,
                FMUX_GPIO_FUNCSEL_VIVO_D5_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D5_MASK,
                0,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D6,
                FMUX_GPIO_FUNCSEL_VIVO_D6_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D6_MASK,
                0,
            },
            {
                FMUX_GPIO_FUNCSEL_VIVO_D7,
                FMUX_GPIO_FUNCSEL_VIVO_D7_OFFSET,
                FMUX_GPIO_FUNCSEL_VIVO_D7_MASK,
                0,
            },
        },
};

static void cif_config_pinmux(enum ttl_src_e vi, uint32_t pad)
{
    mmio_clrsetbits_32(PINMUX_BASE + vi_pin[vi][pad].addr,
                       vi_pin[vi][pad].mask << vi_pin[vi][pad].offset, vi_pin[vi][pad].func);
}

static int _cif_set_attr_cmos(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                              struct combo_dev_attr_s *attr)
{
    struct cif_param *param = ctx->cur_config;
    struct param_ttl *ttl   = &param->cfg.ttl;
    enum ttl_src_e    vi    = attr->ttl_attr.vi;
    int               i;

    if (vi == TTL_VI_SRC_VI2) return -EINVAL;

    /* config the pinmux */
    for (i = 0; i < TTL_PIN_FUNC_NUM; i++) {
        if (attr->ttl_attr.func[i] < 0) continue;
        if (attr->ttl_attr.func[i] >= TTL_PIN_FUNC_NUM) return -EINVAL;
        cif_set_ttl_pinmux(ctx, vi, i, attr->ttl_attr.func[i]);
        cif_config_pinmux(vi, attr->ttl_attr.func[i]);
    }
    if (vi == TTL_VI_SRC_VI0)
        pinmux_config("PAD_MIPIRX4N", VI0_CLK, NULL);
    else if (vi == TTL_VI_SRC_VI1)
        pinmux_config("VIVO_CLK", VI1_CLK, NULL);
    else
        pinmux_config("VIVO_CLK", VI2_CLK, NULL);

    param->type     = CIF_TYPE_TTL;
    ttl->vi_from    = vi;
    ttl->fmt        = TTL_VDE_SENSOR;
    ttl->sensor_fmt = TTL_SENSOR_12_BIT;

    cif_streaming(ctx, 1, 0);

    return 0;
}
#endif  // DVP_IF

#ifdef BT1120_IF
static int _cif_set_attr_bt1120(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                                struct combo_dev_attr_s *attr)
{
    struct cif_param *param = ctx->cur_config;
    struct param_ttl *ttl   = &param->cfg.ttl;
    struct cvi_link  *link  = ctx_to_link(ctx);
    enum ttl_src_e    vi    = attr->ttl_attr.vi;
    int               i;

    if (vi == TTL_VI_SRC_VI2) return -EINVAL;

    /* config the pinmux */
    for (i = 0; i < TTL_PIN_FUNC_NUM; i++) {
        if (attr->ttl_attr.func[i] < 0) continue;
        if (attr->ttl_attr.func[i] >= TTL_PIN_FUNC_NUM) return -EINVAL;
        cif_set_ttl_pinmux(ctx, vi, i, attr->ttl_attr.func[i]);
        cif_config_pinmux(vi, attr->ttl_attr.func[i]);
    }
    if (vi == TTL_VI_SRC_VI0)
        pinmux_config("PAD_MIPIRX4N", VI0_CLK, NULL);
    else if (vi == TTL_VI_SRC_VI1)
        pinmux_config("VIVO_CLK", VI1_CLK, NULL);
    else
        pinmux_config("VIVO_CLK", VI2_CLK, NULL);

    param->type     = CIF_TYPE_TTL;
    ttl->fmt        = TTL_SYNC_PAT_17B_BT1120;
    ttl->width      = attr->img_size.width - 1;
    ttl->height     = attr->img_size.height - 1;
    ttl->sensor_fmt = TTL_SENSOR_12_BIT;
    ttl->clk_inv    = link->clk_edge;
    ttl->vi_sel     = VI_BT1120;
    ttl->v_bp       = (!attr->ttl_attr.v_bp) ? 9 : attr->ttl_attr.v_bp;
    ttl->h_bp       = (!attr->ttl_attr.h_bp) ? 8 : attr->ttl_attr.h_bp;

    cif_streaming(ctx, 1, 0);

    return 0;
}
#endif  // BT1120_IF

#ifdef BT601_IF
static int _cif_set_attr_bt601_19b_vhs(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                                       struct combo_dev_attr_s *attr)
{
    struct cif_param *param = ctx->cur_config;
    struct param_ttl *ttl   = &param->cfg.ttl;
    struct cvi_link  *link  = ctx_to_link(ctx);
    enum ttl_src_e    vi    = attr->ttl_attr.vi;
    int               i;

    if (vi == TTL_VI_SRC_VI2) return -EINVAL;

    /* config the pinmux */
    for (i = 0; i < TTL_PIN_FUNC_NUM; i++) {
        if (attr->ttl_attr.func[i] < 0) continue;
        if (attr->ttl_attr.func[i] >= TTL_PIN_FUNC_NUM) return -EINVAL;
        cif_set_ttl_pinmux(ctx, vi, i, attr->ttl_attr.func[i]);
        cif_config_pinmux(vi, attr->ttl_attr.func[i]);
    }
    if (vi == TTL_VI_SRC_VI0)
        pinmux_config("PAD_MIPIRX4N", VI0_CLK, NULL);
    else if (vi == TTL_VI_SRC_VI1)
        pinmux_config("VIVO_CLK", VI1_CLK, NULL);
    else
        pinmux_config("VIVO_CLK", VI2_CLK, NULL);

    param->type     = CIF_TYPE_TTL;
    ttl->vi_from    = vi;
    ttl->fmt        = TTL_VHS_19B_BT601;
    ttl->width      = attr->img_size.width - 1;
    ttl->height     = attr->img_size.height - 1;
    ttl->sensor_fmt = TTL_SENSOR_12_BIT;
    ttl->clk_inv    = link->clk_edge;
    ttl->vi_sel     = VI_BT601;
    ttl->v_bp       = (!attr->ttl_attr.v_bp) ? 0x23 : attr->ttl_attr.v_bp;
    ttl->h_bp       = (!attr->ttl_attr.h_bp) ? 0xbf : attr->ttl_attr.h_bp;

    cif_streaming(ctx, 1, 0);

    return 0;
}
#endif  // BT601_IF

#ifdef BT_DEMUX_IF
static int _cif_set_attr_bt_demux(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                                  struct combo_dev_attr_s *attr)
{
    struct cif_param       *param   = ctx->cur_config;
    struct param_btdemux   *btdemux = &param->cfg.btdemux;
    struct cvi_link        *link    = ctx_to_link(ctx);
    struct bt_demux_attr_s *info    = &attr->bt_demux_attr;
    int                     i;

    /* config the pinmux */
    for (i = TTL_PIN_FUNC_D0; i < TTL_PIN_FUNC_D8; i++) {
        if (info->func[i] < 0) continue;
        if (info->func[i] >= TTL_PIN_FUNC_D8) return -EINVAL;
        cif_set_ttl_pinmux(ctx, FROM_VI2, i, info->func[i]);
        cif_config_pinmux(FROM_VI2, info->func[i]);
    }
    pinmux_config("VIVO_CLK", VI2_CLK, NULL);
    param->type                  = CIF_TYPE_BT_DMUX;
    btdemux->fmt                 = TTL_SYNC_PAT_9B_BT656;
    btdemux->width               = attr->img_size.width - 1;
    btdemux->height              = attr->img_size.height - 1;
    btdemux->clk_inv             = link->clk_edge;
    btdemux->v_fp                = (!info->v_fp) ? 0x0f : info->v_fp;
    btdemux->h_fp                = (!info->h_fp) ? 0x0f : info->h_fp;
    btdemux->sync_code_part_A[0] = info->sync_code_part_A[0];
    btdemux->sync_code_part_A[1] = info->sync_code_part_A[1];
    btdemux->sync_code_part_A[2] = info->sync_code_part_A[2];
    for (i = 0; i < BT_DEMUX_NUM; i++) {
        btdemux->sync_code_part_B[i].sav_vld = info->sync_code_part_B[i].sav_vld;
        btdemux->sync_code_part_B[i].sav_blk = info->sync_code_part_B[i].sav_blk;
        btdemux->sync_code_part_B[i].eav_vld = info->sync_code_part_B[i].eav_vld;
        btdemux->sync_code_part_B[i].eav_blk = info->sync_code_part_B[i].eav_blk;
    }
    btdemux->demux    = info->mode;
    btdemux->yc_exchg = info->yc_exchg;

    cif_streaming(ctx, 1, 0);

    return 0;
}
#endif  // BT_DEMUX_IF

#ifdef BT656_IF
static int _cif_set_attr_bt656_9b(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                                  struct combo_dev_attr_s *attr)
{
    struct cif_param *param = ctx->cur_config;
    struct param_ttl *ttl   = &param->cfg.ttl;
    struct cvi_link  *link  = ctx_to_link(ctx);
    enum ttl_src_e    vi    = attr->ttl_attr.vi;
    int               i;

    /* config the pinmux */
    for (i = 0; i < TTL_PIN_FUNC_NUM; i++) {
        if (attr->ttl_attr.func[i] < 0) continue;
        if (attr->ttl_attr.func[i] >= TTL_PIN_FUNC_NUM) return -EINVAL;
        cif_set_ttl_pinmux(ctx, vi, i, attr->ttl_attr.func[i]);
        cif_config_pinmux(vi, attr->ttl_attr.func[i]);
    }
    if (vi == TTL_VI_SRC_VI0)
        pinmux_config("PAD_MIPIRX4N", VI0_CLK, NULL);
    else if (vi == TTL_VI_SRC_VI1)
        pinmux_config("VIVO_CLK", VI1_CLK), NULL;
    else
        pinmux_config("VIVO_CLK", VI2_CLK, NULL);

    param->type     = CIF_TYPE_TTL;
    ttl->vi_from    = vi;
    ttl->fmt_out    = TTL_BT_FMT_OUT_CBYCRY;
    ttl->fmt        = TTL_SYNC_PAT_9B_BT656;
    ttl->width      = attr->img_size.width - 1;
    ttl->height     = attr->img_size.height - 1;
    ttl->sensor_fmt = TTL_SENSOR_12_BIT;
    ttl->clk_inv    = link->clk_edge;
    ttl->vi_sel     = VI_BT656;
    ttl->v_bp       = (!attr->ttl_attr.v_bp) ? 0xf : attr->ttl_attr.v_bp;
    ttl->h_bp       = (!attr->ttl_attr.h_bp) ? 0xf : attr->ttl_attr.h_bp;

    cif_streaming(ctx, 1, 0);

    return 0;
}
#endif  // BT656_IF

#ifdef CUSTOM0_IF
static int _cif_set_attr_custom0(struct cvi_cif_dev *dev, struct cif_ctx *ctx,
                                 struct combo_dev_attr_s *attr)
{
    struct cif_param *param = ctx->cur_config;
    struct param_ttl *ttl   = &param->cfg.ttl;
    enum ttl_src_e    vi    = attr->ttl_attr.vi;
    int               i;

    if (vi == TTL_VI_SRC_VI2) return -EINVAL;

    /* config the pinmux */
    for (i = 0; i < TTL_PIN_FUNC_NUM; i++) {
        if (attr->ttl_attr.func[i] < 0) continue;
        if (attr->ttl_attr.func[i] >= TTL_PIN_FUNC_NUM) return -EINVAL;
        cif_set_ttl_pinmux(ctx, vi, i, attr->ttl_attr.func[i]);
        cif_config_pinmux(vi, attr->ttl_attr.func[i]);
    }
    if (vi == TTL_VI_SRC_VI0)
        pinmux_config("PAD_MIPIRX4N", VI0_CLK, NULL);
    else if (vi == TTL_VI_SRC_VI1)
        pinmux_config("VIVO_CLK", VI1_CLK, NULL);
    else
        pinmux_config("VIVO_CLK", VI2_CLK, NULL);

    param->type     = CIF_TYPE_TTL;
    ttl->vi_from    = vi;
    ttl->fmt_out    = TTL_BT_FMT_OUT_CBYCRY;
    ttl->fmt        = TTL_CUSTOM_0;
    ttl->width      = attr->img_size.width - 1;
    ttl->height     = attr->img_size.height - 1;
    ttl->sensor_fmt = TTL_SENSOR_12_BIT;
    ttl->vi_sel     = VI_BT601;
    ttl->v_bp       = (!attr->ttl_attr.v_bp) ? 4095 : attr->ttl_attr.v_bp;
    ttl->h_bp       = (!attr->ttl_attr.h_bp) ? 4 : attr->ttl_attr.h_bp;

    cif_streaming(ctx, 1, 0);

    return 0;
}
#endif  // CUSTOM0_IF

/*
 * 1. Precondition: the mac_max = 400M by default, can be adjust by ioctl (app).
 * 2. If mac_max is below 400M, the vip_sys_2 is 400M (parent: mipimpll).
 * 3. If mac_max = 500M, the vip_sys_2 is 500M. (parent: fpll, 1835 OD case)
 * 4. If mac_max = 600M, the vip_sys_2 is 600M. (parent: mipimpll 1838 case)
 * 5. Return error in the others.
 */
static inline unsigned int _cif_mac_enum_to_value(enum rx_mac_clk_e mac_clk)
{
    unsigned int val;

    switch (mac_clk) {
    case RX_MAC_CLK_200M:
        val = 198;
        break;
    case RX_MAC_CLK_300M:
        val = 297;
        break;
    case RX_MAC_CLK_500M:
        val = 500;
        break;
    case RX_MAC_CLK_600M:
        val = 594;
        break;
    case RX_MAC_CLK_400M:
    default:
        val = 396;
        break;
    }
    return val;
}

#define MAC0_CLK_CTRL1_OFFSET 4U
#define MAC1_CLK_CTRL1_OFFSET 8U
#define MAC2_CLK_CTRL1_OFFSET 30U
#define MAC_CLK_CTRL1_MASK    0x3U

static int cif_set_mac_clk(struct cvi_cif_dev *cdev, uint32_t devno, enum rx_mac_clk_e mac_clk)
{
    struct cvi_link *link = &cdev->link[devno];
    uint32_t         data, mask;
#ifdef CVITEK
    uint32_t tmp;
    u32      clk_val = _cif_mac_enum_to_value(mac_clk);
    uint32_t clk_val = (mac_clk + 99) / 100;
#endif

    link->mac_clk = mac_clk;

    /* select the source to vip_sys2 */
    switch (devno) {
    case 1:
        mask = (MAC_CLK_CTRL1_MASK << MAC1_CLK_CTRL1_OFFSET);
        data = 0x2 << MAC1_CLK_CTRL1_OFFSET;
        break;
    case 2:
        mask = (MAC_CLK_CTRL1_MASK << MAC2_CLK_CTRL1_OFFSET);
        data = 0x2 << MAC2_CLK_CTRL1_OFFSET;
        break;
    case 0:
    default:
        mask = (MAC_CLK_CTRL1_MASK << MAC0_CLK_CTRL1_OFFSET);
        data = 0x2 << MAC0_CLK_CTRL1_OFFSET;
        break;
    }
    vip_sys_reg_write_mask(VIP_SYS_VIP_CLK_CTRL1, mask, data);

#ifdef FPGA_PORTING
    // FPGA don't set mac spped.
    return 0;
#endif

#ifdef CVITEK
    if (clk_val > cdev->max_mac_clk) {
        printf("cannot reach %dM since the max clk is %dM\n", clk_val, cdev->max_mac_clk);
        return -EINVAL;
    }

    /* target = source * (1 + ratio) / 32, ratio <= 0x1F */
    tmp = clk_val * 32 / cdev->max_mac_clk;
    /* roundup */
    if ((clk_val * 32) % cdev->max_mac_clk) tmp++;
    tmp--;

    switch (devno) {
    case 1:
        VIP_NORM_CLK_RATIO_CONFIG(VAL_CSI_MAC1, tmp);
        VIP_NORM_CLK_RATIO_CONFIG(EN_CSI_MAC1, 1);
        VIP_UPDATE_CLK_RATIO(SEL_CSI_MAC1);
        break;
    case 2:
        VIP_NORM_CLK_RATIO_CONFIG(VAL_CSI_MAC2, tmp);
        VIP_NORM_CLK_RATIO_CONFIG(EN_CSI_MAC2, 1);
        VIP_UPDATE_CLK_RATIO(SEL_CSI_MAC2);
        break;
    case 0:
    default:
        VIP_NORM_CLK_RATIO_CONFIG(VAL_CSI_MAC0, tmp);
        VIP_NORM_CLK_RATIO_CONFIG(EN_CSI_MAC0, 1);
        VIP_UPDATE_CLK_RATIO(SEL_CSI_MAC0);
        break;
    }
#endif

    switch (mac_clk) {
    case RX_MAC_CLK_200M:
        /* vipsys2 dividor factor */
        // mmio_write_32(0x03002110, (6<<16)|0x209);
        SET_VIP_SYS_2_CLK_DIV(VIP_SYS_2_SRC_DISPPLL, 6);
        break;
    case RX_MAC_CLK_300M:
        /* vipsys2 dividor factor */
        // mmio_write_32(0x03002110, (4<<16)|0x209);
        SET_VIP_SYS_2_CLK_DIV(VIP_SYS_2_SRC_DISPPLL, 4);
        break;
    case RX_MAC_CLK_500M:
        /* vipsys2 dividor factor */
        // mmio_write_32(0x03002110, (3<<16)|0x309);
        SET_VIP_SYS_2_CLK_DIV(VIP_SYS_2_SRC_FPLL, 3);
        break;
    case RX_MAC_CLK_600M:
        /* vipsys2 dividor factor */
        // mmio_write_32(0x03002110, (2<<16)|0x209);
        SET_VIP_SYS_2_CLK_DIV(VIP_SYS_2_SRC_DISPPLL, 2);
        break;
    case RX_MAC_CLK_400M:
    default:
        /* vipsys2 dividor factor */
        // mmio_write_32(0x03002110, (3<<16)|0x209);
        SET_VIP_SYS_2_CLK_DIV(VIP_SYS_2_SRC_DISPPLL, 3);
        /* do nothing and leave */
        break;
    }

    udelay(5);

    return 0;
}

static int cif_set_output_clk_edge(struct cvi_cif_dev *dev, struct clk_edge_s *clk_edge)
{
    struct cif_ctx *ctx = &dev->link[clk_edge->devno].cif_ctx;

    dev->link[clk_edge->devno].clk_edge = clk_edge->edge;

    cif_set_clk_edge(ctx, CIF_PHY_LANE_0,
                     (clk_edge->edge == CLK_UP_EDGE) ? CIF_CLK_RISING : CIF_CLK_FALLING);
    cif_set_clk_edge(ctx, CIF_PHY_LANE_1,
                     (clk_edge->edge == CLK_UP_EDGE) ? CIF_CLK_RISING : CIF_CLK_FALLING);
    cif_set_clk_edge(ctx, CIF_PHY_LANE_2,
                     (clk_edge->edge == CLK_UP_EDGE) ? CIF_CLK_RISING : CIF_CLK_FALLING);
    cif_set_clk_edge(ctx, CIF_PHY_LANE_3,
                     (clk_edge->edge == CLK_UP_EDGE) ? CIF_CLK_RISING : CIF_CLK_FALLING);
    cif_set_clk_edge(ctx, CIF_PHY_LANE_4,
                     (clk_edge->edge == CLK_UP_EDGE) ? CIF_CLK_RISING : CIF_CLK_FALLING);
    cif_set_clk_edge(ctx, CIF_PHY_LANE_5,
                     (clk_edge->edge == CLK_UP_EDGE) ? CIF_CLK_RISING : CIF_CLK_FALLING);

    return 0;
}

int cif_set_dev_attr(struct combo_dev_attr_s *attr)
{
    struct cvi_cif_dev      *dev        = cvi_cif_pdev;
    enum input_mode_e        input_mode = attr->input_mode;
    struct cif_ctx          *ctx;
    struct cif_param        *param;
    struct combo_dev_attr_s *rx_attr;
    int                      rc = 0;

    /* force the btdmux to devno 0*/
    if (input_mode == INPUT_MODE_BT_DEMUX) {
        attr->devno = 0;
    }

    ctx     = &dev->link[attr->devno].cif_ctx;
    param   = &dev->link[attr->devno].param;
    rx_attr = &dev->link[attr->devno].attr;

    cif_dump_dev_attr(dev, attr);

    memset(param, 0, sizeof(*param));
    ctx->cur_config = param;
    memcpy(rx_attr, attr, sizeof(*rx_attr));

    /* Setting for serial interface. */
    if (input_mode == INPUT_MODE_MIPI || input_mode == INPUT_MODE_SUBLVDS ||
        input_mode == INPUT_MODE_HISPI) {
        struct clk_edge_s clk_edge;

        /* set the default clock edge. */
        clk_edge.devno = attr->devno;
        clk_edge.edge  = CLK_DOWN_EDGE;
        cif_set_output_clk_edge(dev, &clk_edge);
    }

    /* set mac clk */
    rc = cif_set_mac_clk(dev, attr->devno, attr->mac_clk);
    if (rc < 0) return rc;

    /* decide the mclk */
    if (!attr->mclk.cam)
        mclk0 = attr->mclk.freq;
    else if (attr->mclk.cam == 1)
        mclk1 = attr->mclk.freq;
    else if (attr->mclk.cam == 2)
        mclk2 = attr->mclk.freq;

    switch (input_mode) {
#ifdef MIPI_IF
    case INPUT_MODE_MIPI:
        rc = _cif_set_attr_mipi(dev, ctx, &rx_attr->mipi_attr);
        break;
#endif
#ifdef SUBLVDS_IF
    case INPUT_MODE_SUBLVDS:
        rc = _cif_set_attr_sublvds(dev, ctx, &rx_attr->lvds_attr);
        break;
#endif
#ifdef HISPI_IF
    case INPUT_MODE_HISPI:
        rc = _cif_set_attr_hispi(dev, ctx, &rx_attr->lvds_attr);
        break;
#endif
#ifdef DVP_IF
    case INPUT_MODE_CMOS:
        rc = _cif_set_attr_cmos(dev, ctx, rx_attr);
        break;
#endif
#ifdef BT1120_IF
    case INPUT_MODE_BT1120:
        rc = _cif_set_attr_bt1120(dev, ctx, rx_attr);
        break;
#endif
#ifdef BT601_IF
    case INPUT_MODE_BT601_19B_VHS:
        rc = _cif_set_attr_bt601_19b_vhs(dev, ctx, rx_attr);
        break;
#endif
#ifdef BT656_IF
    case INPUT_MODE_BT656_9B:
        rc = _cif_set_attr_bt656_9b(dev, ctx, rx_attr);
        break;
#endif
#ifdef CUSTOM0_IF
    case INPUT_MODE_CUSTOM_0:
        rc = _cif_set_attr_custom0(dev, ctx, rx_attr);
        break;
#endif
#ifdef BTDEMUX_IF
    case INPUT_MODE_BT_DEMUX:
        rc = _cif_set_attr_bt_demux(dev, ctx, rx_attr);
        break;
#endif
    default:
        return -EINVAL;
    }
    if (rc < 0) {
        printf("set attr fail %d\n", rc);
        return rc;
    }
    dev->link[attr->devno].is_on = 1;
    /* unmask the interrupts */
    if (attr->devno < CIF_MAX_CSI_NUM) cif_unmask_csi_int_sts(ctx, 0x1F);

    return 0;
}

static void cif_reset_param(struct cvi_link *link)
{
    link->is_on       = 0;
    link->clk_edge    = CLK_UP_EDGE;
    link->msb         = OUTPUT_NORM_MSB;
    link->crop_top    = 0;
    link->distance_fp = 0;
    memset(&link->param, 0, sizeof(struct cif_param));
    memset(&link->attr, 0, sizeof(struct combo_dev_attr_s));
    memset(&link->sts_csi, 0, sizeof(struct cvi_csi_status));
    memset(&link->sts_lvds, 0, sizeof(struct cvi_lvds_status));
}

int cif_reset_mipi(uint32_t devno)
{
    union vip_sys_reset mask;
    struct cvi_link    *link = &cvi_cif_pdev->link[devno];

    /* mask the interrupts */
    if (link->is_on) cif_mask_csi_int_sts(&link->cif_ctx, 0x1F);

    switch (devno) {
    case 0:
        /* reset phy/mac */
        mmio_clrbits_32(CSI_PHY_REG_08, (1 << 6) | (1 << 7));
        udelay(5);
        mmio_setbits_32(CSI_PHY_REG_08, (1 << 6) | (1 << 7));

        /* sw reset mac by vip register */
        mask.b.csi_mac0 = 1;
        vip_toggle_reset(mask);
        break;
    case 1:
        /* reset phy/mac */
        mmio_clrbits_32(CSI_PHY_REG_08, (1 << 8) | (1 << 9));
        udelay(5);
        mmio_setbits_32(CSI_PHY_REG_08, (1 << 8) | (1 << 9));
        /* sw reset mac by vip register */
        mask.b.csi_mac1 = 1;
        vip_toggle_reset(mask);
        break;
    case 2:
        /* reset phy/mac */
        mmio_clrbits_32(CSI_PHY_REG_08, (1 << 10) | (1 << 11));
        udelay(5);
        mmio_setbits_32(CSI_PHY_REG_08, (1 << 10) | (1 << 11));
        /* sw reset mac by vip register */
        mask.b.csi_mac2 = 1;
        vip_toggle_reset(mask);
    }
    /* reset parameters. */
    cif_reset_param(link);

    return 0;
}

static inline int cif_set_crop_top(struct cvi_cif_dev *dev, struct crop_top_s *crop)
{
    struct cif_ctx *ctx = &dev->link[crop->devno].cif_ctx;

    dev->link[crop->devno].crop_top = crop->crop_top;
    /* strip the top info line. */
    cif_crop_info_line(ctx, crop->crop_top, crop->update);

    return 0;
}

static inline int cif_set_windowing(struct cvi_cif_dev *dev, struct cif_crop_win_s *win)
{
    struct cif_ctx   *ctx = &dev->link[win->devno].cif_ctx;
    struct cif_crop_s crop;

    crop.x      = win->x;
    crop.y      = win->y;
    crop.w      = win->w;
    crop.h      = win->h;
    crop.enable = win->enable;

    cif_set_crop(ctx, &crop);

    return 0;
}

static inline int cif_set_wdr_manual(struct cvi_cif_dev *dev, struct manual_wdr_s *manual)
{
    struct cif_ctx   *ctx   = &dev->link[manual->devno].cif_ctx;
    struct cif_param *param = &dev->link[manual->devno].param;

    param->hdr_manual     = manual->attr.manual_en;
    param->hdr_shift      = manual->attr.l2s_distance;
    param->hdr_vsize      = manual->attr.lsef_length;
    param->hdr_rm_padding = manual->attr.discard_padding_lines;

    cif_hdr_manual_config(ctx, param, !!manual->attr.update);

    return 0;
}

static int cif_get_cif_attr(struct cvi_cif_dev *dev, struct cif_attr_s *cif_attr)
{
    struct cif_param *param = &dev->link[cif_attr->devno].param;

    if (param->type == CIF_TYPE_CSI && param->hdr_en) {
        struct param_csi *csi = &param->cfg.csi;

        cif_attr->stagger_vsync = (csi->hdr_mode == CSI_HDR_MODE_VC);
    } else
        cif_attr->stagger_vsync = 0;

    return 0;
}

static inline int cif_set_lvds_fp_vs(struct cvi_cif_dev *dev, struct vsync_gen_s *vs)
{
    struct cif_ctx *ctx = &dev->link[vs->devno].cif_ctx;

    dev->link[vs->devno].distance_fp = vs->distance_fp;
    cif_set_lvds_vsync_gen(ctx, vs->distance_fp);

    return 0;
}

int cif_enable_snsr_clk(uint32_t devno, uint8_t on)
{
    // uint32_t value;

    if (mclk0 > CAMPLL_FREQ_NONE && mclk0 < CAMPLL_FREQ_NUM) {
        /* set pwd */
        if (on) {
            switch (mclk0) {
            default:
            case CAMPLL_FREQ_37P125M:
                mmio_write_32(CLK_CAM0_SRC_DIV, CAMPLL_37P125M);
                break;
            case CAMPLL_FREQ_27M:
                mmio_write_32(CLK_CAM0_SRC_DIV, CAMPLL_27M);
                break;
            case CAMPLL_FREQ_25M:
                mmio_write_32(CLK_CAM0_SRC_DIV, CAMPLL_25M);
                break;
            case CAMPLL_FREQ_24M:
                mmio_write_32(CLK_CAM0_SRC_DIV, CAMPLL_24M);
                break;
            }
        }
    } else {
        /* camera interface. */
        // pinmux_config(CAM_MCLK0, XGPIOA_0);
    }

    if (mclk1 > CAMPLL_FREQ_NONE && mclk1 < CAMPLL_FREQ_NUM) {
        if (on) {
            switch (mclk1) {
            default:
            case CAMPLL_FREQ_37P125M:
                mmio_write_32(CLK_CAM1_SRC_DIV, CAMPLL_37P125M);
                break;
            case CAMPLL_FREQ_27M:
                mmio_write_32(CLK_CAM1_SRC_DIV, CAMPLL_27M);
                break;
            case CAMPLL_FREQ_25M:
                mmio_write_32(CLK_CAM1_SRC_DIV, CAMPLL_25M);
                break;
            case CAMPLL_FREQ_24M:
                mmio_write_32(CLK_CAM1_SRC_DIV, CAMPLL_24M);
                break;
            }
        }
    } else {
        /* camera interface. */
        // pinmux_config(CAM_MCLK1, XGPIOA_3);
    }

    return 0;
}

static int _cif_reset_snsr_gpio(struct cvi_cif_dev *dev, unsigned int devno, uint8_t on)
{
    struct cvi_link *link;
    int              reset;
    int              ret = 0;

    if (devno >= MAX_LINK_NUM) return -EINVAL;

    link  = &dev->link[devno];
    reset = (link->snsr_rst_pol == OF_GPIO_ACTIVE_LOW) ? 0 : 1;

    // ret = csi_gpio_dir(&link->snsr_rst_gpio, 1 << link->snsr_rst_pin, GPIO_DIRECTION_OUTPUT);
    // if (ret != CSI_OK) {
    // 	printf("csi_gpio_dir failed\r\n");
    // 	return -1;
    // }

    cvi_pin_mode(link->snsr_rst_pin, 0);

    if (on)
        // csi_gpio_write(&link->snsr_rst_gpio, 1 << link->snsr_rst_pin, reset);
        cvi_pin_write(link->snsr_rst_pin, reset);
    else
        // csi_gpio_write(&link->snsr_rst_gpio, 1 << link->snsr_rst_pin, !reset);
        cvi_pin_write(link->snsr_rst_pin, !reset);

    return 0;
}

int cif_reset_snsr_gpio(unsigned int devno, uint8_t on)
{
    return _cif_reset_snsr_gpio(cvi_cif_pdev, devno, on);
}

static inline int cif_reset_lvds(struct cvi_cif_dev *dev, unsigned int devno)
{
    return 0;
}

static inline int cif_bt_fmt_out(struct cvi_cif_dev *dev, struct bt_fmt_out_s *fmt_out)
{
    struct cif_ctx *ctx = &dev->link[fmt_out->devno].cif_ctx;

    dev->link[fmt_out->devno].bt_fmt_out = fmt_out->fmt_out;
    cif_set_bt_fmt_out(ctx, fmt_out->fmt_out);

    return 0;
}

#ifdef VIP_CLK_EN
union vip_clk_reg8 {
    struct {
        u32 clk_12m_usb : 1;
        u32 clk_axi4 : 1;
        u32 clk_axi6 : 1;
        u32 clk_dsi_esc : 1;
        u32 clk_axi_vip : 1;
        u32 clk_src_vip_sys_0 : 1;
        u32 clk_src_vip_sys_1 : 1;
        u32 clk_disp_src_vip : 1;
        u32 clk_axi_video_codec : 1;
        u32 clk_vc_src0 : 1;
        u32 clk_h264c : 1;
        u32 clk_h265c : 1;
        u32 clk_jpeg : 1;
        u32 clk_apb_jpeg : 1;
        u32 clk_apb_h264c : 1;
        u32 clk_apb_h265c : 1;
        u32 clk_cam0 : 1;
        u32 clk_cam1 : 1;
        u32 clk_csi_mac0_vip : 1;
        u32 clk_csi_mac1_vip : 1;
        u32 clk_isp_top_vip : 1;
        u32 clk_img_d_vip : 1;
        u32 clk_img_v_vip : 1;
        u32 clk_sc_top_vip : 1;
        u32 clk_sc_d_vip : 1;
        u32 clk_sc_v1_vip : 1;
        u32 clk_sc_v2_vip : 1;
        u32 clk_sc_v3_vip : 1;
        u32 clk_dwa_vip : 1;  // we call it ldc in 182x
        u32 clk_bt_vip : 1;
        u32 clk_disp_vip : 1;
        u32 clk_dsi_mac_vip : 1;
    } b;
    u32 raw;
};

union vip_clk_regc {
    struct {
        u32 clk_lvds0_vip : 1;
        u32 clk_lvds1_vip : 1;
        u32 clk_csi0_rx_vip : 1;
        u32 clk_csi1_rx_vip : 1;
        u32 clk_pad_vi_vip : 1;
        u32 clk_1m : 1;
        u32 clk_spi : 1;
        u32 clk_i2c : 1;
        u32 clk_pm : 1;
        u32 clk_timer0 : 1;
        u32 clk_timer1 : 1;
        u32 clk_timer2 : 1;
        u32 clk_timer3 : 1;
        u32 clk_timer4 : 1;
        u32 clk_timer5 : 1;
        u32 clk_timer6 : 1;
        u32 clk_timer7 : 1;
        u32 clk_apb_i2c0 : 1;
        u32 clk_apb_i2c1 : 1;
        u32 clk_apb_i2c2 : 1;
        u32 clk_apb_i2c3 : 1;
        u32 clk_apb_i2c4 : 1;
        u32 clk_wgn : 1;
        u32 clk_wgn0 : 1;
        u32 clk_wgn1 : 1;
        u32 clk_wgn2 : 1;
        u32 clk_keyscan : 1;
        u32 clk_ahb_sf1 : 1;
        u32 clk_vc_src1 : 1;
        u32 clk_src_vip_sys_2 : 1;
        u32 clk_pad_vi1_vip : 1;
        u32 clk_cfg_reg_vip : 1;
    } b;
    u32 raw;
};

void vip_clk_en(void)
{
    union vip_clk_reg8 reg8;
    union vip_clk_regc regc;

    reg8.b.clk_axi_vip       = 1;
    reg8.b.clk_src_vip_sys_0 = 1;
    reg8.b.clk_src_vip_sys_1 = 1;
    reg8.b.clk_isp_top_vip   = 1;
    reg8.b.clk_img_d_vip     = 1;
    reg8.b.clk_img_v_vip     = 1;
    reg8.b.clk_sc_top_vip    = 1;
    reg8.b.clk_sc_d_vip      = 1;
    reg8.b.clk_sc_v1_vip     = 1;
    reg8.b.clk_sc_v2_vip     = 1;
    reg8.b.clk_sc_v3_vip     = 1;
    reg8.b.clk_dwa_vip       = 1;
    reg8.b.clk_bt_vip        = 1;
    reg8.b.clk_disp_vip      = 1;
    reg8.b.clk_dsi_mac_vip   = 1;
    reg8.b.clk_csi_mac0_vip  = 1;
    reg8.b.clk_csi_mac1_vip  = 1;

    regc.b.clk_src_vip_sys_2 = 1;
    mmio_setbits_32(0x03002008, reg8.raw);
    mmio_setbits_32(0x0300200c, regc.raw);
}
void vip_csi_bdg_dma_cfg(unsigned int devno, unsigned int hdrMode)
{
    unsigned int snsr_w = 1948, snsr_h = 1097, data_per_line = 0;

    unsigned long long bayer_w_addr[3];

    bayer_w_addr[0] = (unsigned long long)malloc(0x314780);
    bayer_w_addr[1] = (unsigned long long)malloc(0x314780);
    printf("pls use CVD dump raw lef:addr[0]:0x:%llx, sef:addr[1]:0x:%llx. size:0x314780.\n",
           bayer_w_addr[0], bayer_w_addr[1]);
    // 16-alignment
    data_per_line = (snsr_w * 3 / 2 + 15) & (~0xF);

    switch (hdrMode) {
    case 3:
        // shadow update
        mmio_write_32(0x0A070010, 0x30003);
        // csibdg size
        mmio_write_32(0x0A010810, (snsr_h - 1) << 16 | (snsr_w - 1));  // chn 0
        mmio_write_32(0x0A010814, (snsr_h - 1) << 16 | (snsr_w - 1));  // chn 1
        // config dma 0,1
        mmio_write_32(0x0A071400, 0 << 4);
        mmio_write_32(0x0A071404, bayer_w_addr[0]);
        mmio_write_32(0x0A071408, data_per_line);
        mmio_write_32(0x0A071410, snsr_h);
        mmio_write_32(0x0A071420, 1 << 4);
        mmio_write_32(0x0A071424, bayer_w_addr[1]);
        mmio_write_32(0x0A071428, data_per_line);
        mmio_write_32(0x0A071430, snsr_h);
        // csibdg configure
        mmio_write_32(0x0A010800, 0x111000D1);
        // set frame valid, start streaming
        mmio_write_32(0x0a010028, 3);
        break;
    case 0:
    default:
        if (!devno) {
            // shadow update
            mmio_write_32(0x0A070010, 0x10001);
            // csibdg0 size
            mmio_write_32(0x0A010810, (snsr_h - 1) << 16 | (snsr_w - 1));
            // config dma0
            mmio_write_32(0x0A071404, bayer_w_addr[0]);
            mmio_write_32(0x0A071408, data_per_line);
            mmio_write_32(0x0A071410, snsr_h);
            // csibdg0 configure
            mmio_write_32(0x0A010800, 0x11100041);
            // set frame valid, start streaming, preraw0
            mmio_write_32(0x0A010028, 0x1);
        } else {
            // shadow update
            // mmio_write_32(0x0A070008, 0x81FC01FF);
            mmio_write_32(0x0A070010, 0x00112211);
            // csibdg1 size
            mmio_write_32(0x0A018810, (snsr_h - 1) << 16 | (snsr_w - 1));
            // config dma6
            mmio_write_32(0x0A0714c4, bayer_w_addr[0]);
            mmio_write_32(0x0A0714c8, data_per_line);
            mmio_write_32(0x0A0714d0, snsr_h);
            // csibdg1 configure
            mmio_write_32(0x0A018800, 0x11300041);
            // set frame valid, start streaming, preraw1
            mmio_write_32(0x0A018028, 0x1);
        }
        break;
    }
}
#endif

#ifdef CONFIG_PROC_FS
static void cif_show_dev_attr(struct combo_dev_attr_s *attr)
{
    int  i;
    char buf[32]  = {0};
    char buf2[32] = {0};

    printf("%8s%10s%10s%10s%15s%15s%10s%12s%16s\n", "Devno", "WorkMode", "DataType", "WDRMode",
           "LinkId", "PN Swap", "SyncMode", "DataEndian", "SyncCodeEndian");

    printf("%8d%10s", attr->devno, _to_string_input_mode(attr->input_mode));
    switch (attr->input_mode) {
    case INPUT_MODE_MIPI: {
        struct mipi_dev_attr_s *mipi = &attr->mipi_attr;
        char                   *ptr  = buf;

        for (i = 0; i < CIF_LANE_NUM; i++) {
            sprintf(ptr, "%2d,", mipi->lane_id[i]);
            ptr += 3;
        }
        *(ptr - 1) = '\0';
        ptr        = buf2;
        for (i = 0; i < CIF_LANE_NUM; i++) {
            sprintf(ptr, "%2d,", mipi->pn_swap[i]);
            ptr += 3;
        }
        *(ptr - 1) = '\0';

        printf("%10s%10s%15s%15s%10s%12s%16s\n", _to_string_raw_data_type(mipi->raw_data_type),
               _to_string_mipi_wdr_mode(mipi->wdr_mode), buf, buf2, "N/A", "N/A", "N/A");

    } break;
    case INPUT_MODE_SUBLVDS:
    case INPUT_MODE_HISPI: {
        struct lvds_dev_attr_s *lvds = &attr->lvds_attr;
        char                   *ptr  = buf;

        for (i = 0; i < CIF_LANE_NUM; i++) {
            sprintf(ptr, "%2d,", lvds->lane_id[i]);
            ptr += 3;
        }
        *(ptr - 1) = '\0';
        ptr        = buf2;
        for (i = 0; i < CIF_LANE_NUM; i++) {
            sprintf(ptr, "%2d,", lvds->pn_swap[i]);
            ptr += 3;
        }
        *(ptr - 1) = '\0';

        printf("%10s%10s%15s%15s%10s%12s%16s\n", _to_string_raw_data_type(lvds->raw_data_type),
               _to_string_wdr_mode(lvds->wdr_mode), buf, buf2,
               _to_string_lvds_sync_mode(lvds->sync_mode), _to_string_bit_endian(lvds->data_endian),
               _to_string_bit_endian(lvds->sync_code_endian));
    } break;
    case INPUT_MODE_CMOS:
    case INPUT_MODE_BT1120:
        printf("%10s%10s%15s%10s%12s%16s\n", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A");
        break;
    default:
        break;
    }
}

static void cif_show_mipi_sts(struct cvi_link *link)
{
    struct cvi_csi_status *sts = &link->sts_csi;

    printf("%6s%7s%7s%7s%6s%9s%9s\n", "Devno", "EccErr", "CrcErr", "HdrErr", "WcErr", "fifofull",
           "decode");
    printf("%6d%7d%7d%7d%6d%9d%9s\n", link->attr.devno, sts->errcnt_ecc, sts->errcnt_crc,
           sts->errcnt_hdr, sts->errcnt_wc, sts->fifo_full,
           _to_string_csi_decode(cif_get_csi_decode_fmt(&link->cif_ctx)));
}

static void cif_show_phy_sts(struct cvi_link *link)
{
    union mipi_phy_state state;

    cif_get_csi_phy_state(&link->cif_ctx, &state);

    printf("%11s%9s%9s%9s%9s%9s%9s\n", "Physical:", "D0", "D1", "D2", "D3", "D4", "D5");
    printf("%11s%9x%9x%9x%9x%9x%9x\n", " ", cif_get_lane_data(&link->cif_ctx, CIF_PHY_LANE_0),
           cif_get_lane_data(&link->cif_ctx, CIF_PHY_LANE_1),
           cif_get_lane_data(&link->cif_ctx, CIF_PHY_LANE_2),
           cif_get_lane_data(&link->cif_ctx, CIF_PHY_LANE_3),
           cif_get_lane_data(&link->cif_ctx, CIF_PHY_LANE_4),
           cif_get_lane_data(&link->cif_ctx, CIF_PHY_LANE_5));

    printf("%11s%9s%9s%9s%9s%9s%9s%9s%9s%9s\n", "Digital:", "D0", "D1", "D2", "D3", "CK_HS",
           "CK_ULPS", "CK_STOP", "CK_ERR", "Deskew");
    printf("%11s%9s%9s%9s%9s%9d%9d%9d%9d%9s\n", " ",
           _to_string_dlane_state(state.bits.d0_datahs_state),
           _to_string_dlane_state(state.bits.d1_datahs_state),
           _to_string_dlane_state(state.bits.d2_datahs_state),
           _to_string_dlane_state(state.bits.d3_datahs_state),
           link->cif_ctx.mac_num ? state.bits.p1_clk_hs_state : state.bits.clk_hs_state,
           link->cif_ctx.mac_num ? state.bits.p1_clk_ulps_state : state.bits.clk_ulps_state,
           link->cif_ctx.mac_num ? state.bits.p1_clk_stop_state : state.bits.clk_stop_state,
           link->cif_ctx.mac_num ? state.bits.p1_clk_err_state : state.bits.clk_err_state,
           _to_string_deskew_state(link->cif_ctx.mac_num ? state.bits.p1_deskew_state
                                                         : state.bits.deskew_state));
}

int proc_cif_show(void)
{
    struct cvi_cif_dev *dev = cvi_cif_pdev;
    int                 i;

    printf("\n----------Module: [MIPI_RX]-------------\n");
    printf("\n------------Combo DEV ATTR--------------\n");
    for (i = 0; i < MAX_LINK_NUM; i++)
        if (dev->link[i].is_on) cif_show_dev_attr(&dev->link[i].attr);

    printf("\n------------MIPI info-------------------\n");
    for (i = 0; i < MAX_LINK_NUM; i++)
        if (dev->link[i].is_on && (dev->link[i].attr.input_mode == INPUT_MODE_MIPI)) {
            cif_show_mipi_sts(&dev->link[i]);
            cif_show_phy_sts(&dev->link[i]);
        }

    return 0;
}

void proc_mipi_rx(int32_t argc, char **argv)
{
    proc_cif_show();
}
MSH_CMD_EXPORT_ALIAS(proc_mipi_rx, proc_mipi_rx, proc mipi rx);

#endif

int cvi_cif_reset_snsr_gpio_init(int devno, struct snsr_rst_gpio_s *snsr_gpio)
{
    int              ret;
    struct cvi_link *link;

    link = &cvi_cif_pdev->link[devno];

    // link->snsr_rst_gpio = snsr_gpio->snsr_rst_gpio;
    link->snsr_rst_port_idx = snsr_gpio->snsr_rst_port_idx;
    link->snsr_rst_pin      = snsr_gpio->snsr_rst_pin;
    link->snsr_rst_pol      = snsr_gpio->snsr_rst_pol;
    // ret = csi_gpio_init(&link->snsr_rst_gpio, link->snsr_rst_port_idx);
    // if (ret != CSI_OK) {
    // 	printf("sensor reset gpio init failed\r\n");
    // 	return -1;
    // }

    return 0;
}

static void cif_isr(int irq, void *_link)
{
    struct cvi_link *link = (struct cvi_link *)_link;
    struct cif_ctx  *ctx  = &link->cif_ctx;

    if (cif_check_csi_int_sts(ctx, CIF_INT_STS_ECC_ERR_MASK)) link->sts_csi.errcnt_ecc++;
    if (cif_check_csi_int_sts(ctx, CIF_INT_STS_CRC_ERR_MASK)) link->sts_csi.errcnt_crc++;
    if (cif_check_csi_int_sts(ctx, CIF_INT_STS_WC_ERR_MASK)) link->sts_csi.errcnt_wc++;
    if (cif_check_csi_int_sts(ctx, CIF_INT_STS_HDR_ERR_MASK)) link->sts_csi.errcnt_hdr++;
    if (cif_check_csi_int_sts(ctx, CIF_INT_STS_FIFO_FULL_MASK)) link->sts_csi.fifo_full++;

    if (link->sts_csi.errcnt_ecc > 0xFFFF || link->sts_csi.errcnt_crc > 0xFFFF ||
        link->sts_csi.errcnt_hdr > 0xFFFF || link->sts_csi.fifo_full > 0xFFFF ||
        link->sts_csi.errcnt_wc > 0xFFFF) {
        cif_mask_csi_int_sts(ctx, 0x1F);

        printf("mask the interrupt since err cnt is full\n");
        printf("ecc = %u, crc = %u, wc = %u, hdr = %u, fifo_full = %u\n", link->sts_csi.errcnt_ecc,
               link->sts_csi.errcnt_crc, link->sts_csi.errcnt_wc, link->sts_csi.errcnt_hdr,
               link->sts_csi.fifo_full);
    }

    cif_clear_csi_int_sts(ctx);
}

static int cif_cb(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg)
{
    struct cvi_cif_dev *vdev = (struct cvi_cif_dev *)dev;
    int                 rc   = -1;

    switch (cmd) {
    case CIF_CB_RESET_LVDS: {
        rc = 0;
        break;
    }
    case CIF_CB_GET_CIF_ATTR: {
        struct cif_attr_s cif_attr;
        memcpy(&cif_attr, (void *)arg, sizeof(cif_attr));
        cif_get_cif_attr(vdev, &cif_attr);
        memcpy((void *)arg, &cif_attr, sizeof(cif_attr));
        rc = 0;
        break;
    }
    default:
        break;
    }

    return rc;
}

static int cif_rm_cb(void)
{
    return base_rm_module_cb(E_MODULE_CIF);
}

static int cif_register_cb(struct cvi_cif_dev *dev)
{
    struct base_m_cb_info reg_cb;

    reg_cb.module_id = E_MODULE_CIF;
    reg_cb.dev       = (void *)dev;
    reg_cb.cb        = cif_cb;

    return base_reg_module_cb(&reg_cb);
}

int cvi_cif_init(void)
{
    int              i, ret;
    struct cvi_link *link;

    /*alloc buf*/
    cvi_cif_pdev = rt_calloc(1, sizeof(*cvi_cif_pdev));
    if (!cvi_cif_pdev) {
        printf("[%s]alloc buf for cif fail.\n", __func__);
        return -1;
    }

    /* register cif_cb */
    ret = cif_register_cb(cvi_cif_pdev);
    if (ret < 0) {
        printf("cif: failed to register callbacks.\n");
        return -1;
    }

    /* init cif */
    for (i = 0; i < MAX_LINK_NUM; i++) {
        struct cif_ctx *ctx = &cvi_cif_pdev->link[i].cif_ctx;
        ctx->mac_phys_regs  = cif_get_mac_phys_reg_bases(i);
        ctx->wrap_phys_regs = cif_get_wrap_phys_reg_bases(i);
    }

    /*init moudule base addr*/
    cif_set_base_addr(0, (uint32_t *)SENSOR_MAC0_BASE, (uint32_t *)DPHY_TOP_BASE);
    cif_set_base_addr(1, (uint32_t *)SENSOR_MAC1_BASE, (uint32_t *)DPHY_TOP_BASE);
    cif_set_base_addr(2, (uint32_t *)SENSOR_MAC2_BASE, (uint32_t *)DPHY_TOP_BASE);

    cvi_cif_pdev->max_mac_clk = 600;

    for (i = 0; i < MAX_LINK_NUM; ++i) {
        link = &cvi_cif_pdev->link[i];

        /*irq*/
        link->irq_num = irq_num[i];
        // ret = request_irq(link->irq_num, cif_isr, 0, "cif_isr", (void *)link);
        ret = rt_hw_interrupt_install(link->irq_num, cif_isr, RT_NULL, "cif_isr");
        rt_hw_interrupt_umask(link->irq_num);
        if (ret < 0) {
            printf("request irq-%d fail\n", link->irq_num);
            return -1;
        }

        /* set the port id */
        link->cif_ctx.mac_num = i;
    }

    return 0;
}

int cvi_cif_exit(void)
{
    int i;

    if (cif_rm_cb()) {
        printf("cif: failed to rm cb.\n");
    }

    for (i = 0; i < MAX_LINK_NUM; ++i) {
        // csi_irq_disable(irq_num[i]);
        rt_hw_interrupt_mask(irq_num[i]);
    }

    rt_free(cvi_cif_pdev);
    return 0;
}
