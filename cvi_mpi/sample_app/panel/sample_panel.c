#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <cvi_comm_vo.h>

#include "cvi_comm_mipi_tx.h"
#include "cvi_vo.h"
#include "cvi_mipi_tx.h"
#include "sample_panel.h"
#include "sample_comm.h"
#include "cvi_sys.h"

static int fd;

#define MAX_OPTIONS	128
#define LANE_MAX_NUM   5

typedef enum _ARG_TYPE_ {
	ARG_INT = 0,
	ARG_UINT,
	ARG_STRING,
} ARG_TYPE;

typedef struct _optionExt_ {
	struct option opt;
	int type;
	int64_t min;
	int64_t max;
	const char *help;
} optionExt;

typedef enum {
	DSI_PANEL_3AML069LP01G,
	DSI_PANEL_GM8775C,
	DSI_PANEL_HX8394_EVB,
	DSI_PANEL_HX8399_1080P,
	DSI_PANEL_ICN9707,
	DSI_PANEL_ILI9881C,
	DSI_PANEL_ILI9881D,
	DSI_PANEL_JD9366AB,
	DSI_PANEL_LT9611_1920x1080_60,
	DSI_PANEL_LT9611_1920x1080_30,
	DSI_PANEL_LT9611_1280x720_60,
	DSI_PANEL_LT9611_1024x768_60,
	DSI_PANEL_LT9611_1280x1024_60,
	DSI_PANEL_LT9611_1600x1200_60,
	DSI_PANEL_LT9611_2560x1440_60,
	DSI_PANEL_NT35521,
	DSI_PANEL_OTA7290B_1920,
	DSI_PANEL_OTA7290B,
	DSI_PANEL_ST7701,
	DSI_PANEL_ST7701_480x640,
	DSI_PANEL_ST7703,
	LVDS_PANEL_LCM185X56,
	I80_PANEL_ST7789V3_HW_MCU_240x320_60FPS,
	BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M,
	BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M,
	MS7024_BT656_480P_60FPS,
	MS7024_BT656_480P_30FPS,
	MS7024_BT656_576P_50FPS,
	MS7024_BT656_576P_25FPS,
	MS7024_BT656_480P_30FPS_TESTPATTERN,
	PANEL_MAX
} PANEL_MODEL;

typedef struct _inputPara_ {
	enum mipi_tx_lane_id    lane_id[LANE_MAX_NUM];
	PANEL_MODEL			    panel_model;
	bool                    lane_pn_swap[LANE_MAX_NUM];
	bool					lane_id_flag;
	bool					pn_swap_flag;
	bool					dev_flag;
	CVI_U8					dev_no;
} inputPara;

inputPara g_input_para = {
	.panel_model = DSI_PANEL_HX8394_EVB,
	.dev_no = 0,
};

static struct panel_desc_s g_panel_desc = {
	.panel_mode = "HX8394_EVB",
	.panel_type = PANEL_MODE_DSI,
	.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
	.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
	.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps,
	.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps)
};

static optionExt long_option_ext[] = {
	{{"panel",  required_argument, NULL, 'm'},   ARG_STRING,   0,   0,
		"choose diaply panel model"},
	{{"laneid",    optional_argument, NULL, 'l'},   ARG_STRING,    0,   0,
		"laneid sequence by order"},
	{{"pnswap",    optional_argument, NULL, 'p'},   ARG_STRING,   0,   0,
		"pnswap sequence by order"},
	{{"dsi-control",     no_argument, NULL, 'd'}, ARG_STRING, 0,   0,
		"set/get dsi status or settings." },
	{{"control_pins",  required_argument, NULL, 'c'},   ARG_STRING,   0,   0,
		"control pins to config, pwr_gpio/rst_gpio/bl_gpio."},
	{{"show-pattern", optional_argument, NULL, 's'}, ARG_STRING, 0,   9,
		"show colorbar or snow or rgb color pattern." },
	{{"help",      no_argument, NULL, 'h'},       ARG_STRING, 0,   0,
		"print usage."},
	{{NULL, 0, NULL, 0}, ARG_INT, 0, 0, "no param: just init the panel."}
};

static char *s_panel_model_type_arr[] = {
	"3AML069LP01G",
	"GM8775C",
	"HX8394_EVB",
	"HX8399_1080P",
	"ICN9707",
	"ILI9881C",
	"ILI9881D",
	"JD9366AB",
	"LT9611_1920x1080_60",
	"LT9611_1920x1080_30",
	"LT9611_1280x720_60",
	"LT9611_1024x768_60",
	"LT9611_1280x1024_60",
	"LT9611_1600x1200_60",
	"LT9611_2560x1440_60",
	"NT35521",
	"OTA7290B_1920",
	"OTA7290B",
	"ST7701",
	"ST7701_480x640",
	"ST7703",
	"LCM185X56",
	"ST7789V3_HW_MCU_RGB565_240x320_60FPS",
	"BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M",
	"BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M",
	"MS7024_BT656_480P_60FPS",
	"MS7024_BT656_480P_30FPS",
	"MS7024_BT656_576P_50FPS",
	"MS7024_BT656_576P_25FPS",
	"MS7024_BT656_480P_30FPS_TESTPATTERN",

};

void printdsiHelp(void)
{
	SAMPLE_PRT("\n// ------------------------dsi-control------------------------\n");
	SAMPLE_PRT(" 0: dcs send\n");
	SAMPLE_PRT(" 1: dcs get\n");
	SAMPLE_PRT(" 2: switch to lp\n");
	SAMPLE_PRT(" 3: switch to hs\n");
	SAMPLE_PRT(" 4: get hs settle settings\n");
	SAMPLE_PRT(" 5: set hs settle settings\n");
}

void printpatternHelp(void)
{
	SAMPLE_PRT("\n// ------------------------show-pattern------------------------\n");
	SAMPLE_PRT(" 0: VO_PAT_OFF\n");
	SAMPLE_PRT(" 1: VO_PAT_SNOW\n");
	SAMPLE_PRT(" 2: VO_PAT_AUTO\n");
	SAMPLE_PRT(" 3: VO_PAT_RED\n");
	SAMPLE_PRT(" 4: VO_PAT_GREEN\n");
	SAMPLE_PRT(" 5: VO_PAT_BLUE\n");
	SAMPLE_PRT(" 6: VO_PAT_COLORBAR\n");
	SAMPLE_PRT(" 7: VO_PAT_GRAY_GRAD_H\n");
	SAMPLE_PRT(" 8: VO_PAT_GRAY_GRAD_V\n");
	SAMPLE_PRT(" 9: VO_PAT_BLACK\n");
}

void printHelp(char **argv)
{
	CVI_U32 idx;

	SAMPLE_PRT("// ------------------------help------------------------\n");
	SAMPLE_PRT("\noptional panel mode support list:\n");
	for (idx = 0; idx < sizeof(s_panel_model_type_arr) / sizeof(char *); idx++) {
		SAMPLE_PRT(" %s\n", s_panel_model_type_arr[idx]);
	}

	SAMPLE_PRT("\n.for mipi/lvds panel you can cfg lane seq or pnswap");
	SAMPLE_PRT("\nEX.\n");
	SAMPLE_PRT(" %s --panel=HX8394_EVB --laneid=1,2,0,3,4 --pnswap=1,1,1,1,1 --control_pins=352,353,354\n",
		    argv[0]);
	SAMPLE_PRT("\n.for mipi panel You can also manually set the dsi by -d");
	SAMPLE_PRT("\nEX.\n");
	SAMPLE_PRT(" %s -d\n\n", argv[0]);
	SAMPLE_PRT("\n.After initializing panel, to show specific pattern by --show-pattern");
	SAMPLE_PRT("\nEX.\n");
	SAMPLE_PRT(" %s --panel=HX8394_EVB --show-pattern=6 (To show colorbar)\n", argv[0]);
	SAMPLE_PRT(" %s --panel=HX8394_EVB --show-pattern=0 (To turn off colorbar)\n", argv[0]);
	SAMPLE_PRT("\n.After initializing panel, to show any kind of pattern by -s");
	SAMPLE_PRT("\nEX.\n");
	SAMPLE_PRT(" %s --panel=HX8394_EVB -s\n\n", argv[0]);

	for (idx = 0; idx < sizeof(long_option_ext) / sizeof(optionExt); idx++) {
		if (long_option_ext[idx].opt.name == NULL) {
			break;
		}

		SAMPLE_PRT("--%s\n", long_option_ext[idx].opt.name);
		SAMPLE_PRT("    %s\n", long_option_ext[idx].help);
	}

	SAMPLE_PRT("// ------------------------------------------------\n");
}

int dsi_init(int devno, const struct dsc_instr *cmds, int size)
{
	int ret;

	if (cmds == NULL) {
		return CVI_FAILURE;
	}

	for (int i = 0; i < size; i++) {
		const struct dsc_instr *instr = &cmds[i];
		struct cmd_info_s cmd_info = {
			.devno = devno,
			.cmd_size = instr->size,
			.data_type = instr->data_type,
			.cmd = (void *)instr->data
		};

		ret = mipi_tx_send_cmd(fd, &cmd_info);
		if (instr->delay)
			usleep(instr->delay * 1000);

		if (ret) {
			SAMPLE_PRT("dsi init failed at %d instr.\n", i);
			return ret;
		}
	}
	return ret;
}

CVI_S32 SAMPLE_MIPI_TX_ENABLE(void)
{
	CVI_S32 ret = 0;
	VO_DEV VoDev = g_input_para.dev_no;

#if !defined(CONFIG_DUAL_OS)
	char *mipi_tx_device = MIPI_TX_NAME;
	fd = open(mipi_tx_device, O_RDWR | O_NONBLOCK, 0);
	if (fd == -1) {
		SAMPLE_PRT("Cannot open '%s': %d, %s\n", mipi_tx_device, errno, strerror(errno));
		return CVI_FAILURE;
	}
#endif

	ret = mipi_tx_disable(fd);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("mipi_tx_disable fail!\n");
		return CVI_FAILURE;
	}

	ret = mipi_tx_cfg(fd, (struct combo_dev_cfg_s *)g_panel_desc.stdsicfg.dev_cfg);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("mipi_tx_cfg fail!\n");
		return CVI_FAILURE;
	}

	if (g_panel_desc.stdsicfg.dsi_init_cmds_size) {
		ret = dsi_init(VoDev, g_panel_desc.stdsicfg.dsi_init_cmds,
			       g_panel_desc.stdsicfg.dsi_init_cmds_size);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("dsi_init fail!\n");
			return CVI_FAILURE;
		}
	}

	ret = mipi_tx_set_hs_settle(fd, g_panel_desc.stdsicfg.hs_timing_cfg);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("mipi_tx_set_hs_settle fail!\n");
		return CVI_FAILURE;
	}

	ret = mipi_tx_enable(fd);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("mipi_tx_enable fail!\n");
		return CVI_FAILURE;
	}

	SAMPLE_PRT("Init for MIPI-Driver-%s\n", g_panel_desc.panel_mode);

#if !defined(CONFIG_DUAL_OS)
	close(fd);
#endif

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_PANEL_ShowPattern(VO_DEV VoDev, CVI_S32 patern_cmd)
{
	CVI_S32 ret = 0;

	if (patern_cmd >= 0 && patern_cmd < VO_PAT_MAX) {
		ret = CVI_VO_ShowPattern(VoDev, patern_cmd);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_ShowPattern failed with %#x!\n", ret);
			return ret;
		}
	} else if (patern_cmd == VO_PAT_MAX) {
		do {
			printpatternHelp();
			SAMPLE_PRT(" others: exit\n");
			scanf("%d", &patern_cmd);
			if (patern_cmd >= 0 && patern_cmd < VO_PAT_MAX) {
				ret = CVI_VO_ShowPattern(VoDev, patern_cmd);
				if (ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VO_ShowPattern failed with %#x!\n", ret);
					return CVI_FAILURE;
				}
			} else {
				break;
			}
		} while (1);
	} else {
		SAMPLE_PRT("invalid pattern mode parameter\n");
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_PANEL_ENABLE(void)
{
	CVI_S32 ret = 0;
	VO_DEV VoDev = g_input_para.dev_no;

	if (g_panel_desc.panel_type == PANEL_MODE_DSI) {
		ret = SAMPLE_MIPI_TX_ENABLE();
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_MIPI_TX_ENABLE fail!\n");
			return CVI_FAILURE;
		}
	} else {
		if (g_panel_desc.panel_type == PANEL_MODE_LVDS) {
			ret = CVI_VO_SetPubAttr(VoDev, &g_panel_desc.stlvdscfg.stVoPubAttr);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
			ret = CVI_VO_SetLVDSParam(VoDev,  &g_panel_desc.stlvdscfg.LvdsAttr);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
			ret = CVI_VO_GetLVDSParam(VoDev,  &g_panel_desc.stlvdscfg.LvdsAttr);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
		} else if (g_panel_desc.panel_type == PANEL_MODE_BT) {
			ret = CVI_VO_SetPubAttr(VoDev, &g_panel_desc.stbtcfg.stVoPubAttr);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
			ret = CVI_VO_SetBTParam(VoDev,  &g_panel_desc.stbtcfg.BtAttr);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
			ret = CVI_VO_GetBTParam(VoDev,  &g_panel_desc.stbtcfg.BtAttr);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
		} else if (g_panel_desc.panel_type == PANEL_MODE_MCU) {
			ret = CVI_VO_SetPubAttr(VoDev, &g_panel_desc.stVoPubAttr);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("failed with %#x!\n", ret);
				return CVI_FAILURE;
			}
		}
		SAMPLE_PRT("Init for Driver-%s\n", g_panel_desc.panel_mode);
	}

	return CVI_SUCCESS;
}

void SAMPLE_DSI_CONTROLE(void)
{
	CVI_U32 tmp;

#if !defined(CONFIG_DUAL_OS)
	char *mipi_tx_device = MIPI_TX_NAME;
	fd = open(mipi_tx_device, O_RDWR | O_NONBLOCK, 0);
	if (fd == -1) {
		SAMPLE_PRT("Cannot open '%s': %d, %s\n", mipi_tx_device, errno, strerror(errno));
	}
#endif

	do {
		printdsiHelp();
		SAMPLE_PRT(" others: exit\n");
		scanf("%d", &tmp);
		if (tmp == 0) {
			struct cmd_info_s cmd_info;
			CVI_U8 data[16] = { 0 };
			int len = 0;

			SAMPLE_PRT("data size:\n");
			scanf("%d", &tmp);
			cmd_info.cmd_size = tmp;

			SAMPLE_PRT("data type: 0x\n");
			scanf("%x", &tmp);
			cmd_info.data_type = tmp;

			do {
				SAMPLE_PRT("data[%d]: 0x\n", len);
				scanf("%x", &tmp);
				data[len++] = tmp;
			} while (len < cmd_info.cmd_size);
			cmd_info.cmd = data;
			mipi_tx_send_cmd(fd, &cmd_info);
		} else if (tmp == 1) {
			struct get_cmd_info_s cmd_info;
			CVI_U8 data[4] = { 0 };

			SAMPLE_PRT("get data size:\n");
			scanf("%d", &tmp);
			cmd_info.get_data_size = tmp;

			SAMPLE_PRT("data type: 0x\n");
			scanf("%x", &tmp);
			cmd_info.data_type = tmp;

			SAMPLE_PRT("data param: 0x\n");
			scanf("%x", &tmp);
			cmd_info.data_param = tmp;

			cmd_info.get_data = data;
			mipi_tx_recv_cmd(fd, &cmd_info);
			SAMPLE_PRT("data[0]: %#x [1]: %#x [2]: %#x [3]: %#x\n"
				, cmd_info.get_data[0], cmd_info.get_data[1]
				, cmd_info.get_data[2], cmd_info.get_data[3]);
		} else if (tmp == 2) {
			mipi_tx_disable(fd);
		} else if (tmp == 3) {
			mipi_tx_enable(fd);
		} else if (tmp == 4) {
			struct hs_settle_s hs_cfg;

			mipi_tx_get_hs_settle(fd, &hs_cfg);
			SAMPLE_PRT("prepare(%d) zero(%d) trail(%d)\n",
				hs_cfg.prepare, hs_cfg.zero, hs_cfg.trail);
		} else if (tmp == 5) {
			struct hs_settle_s hs_cfg;

			SAMPLE_PRT("prepare:\n");
			scanf("%d", &tmp);
			hs_cfg.prepare = tmp;

			SAMPLE_PRT("zero:\n");
			scanf("%d", &tmp);
			hs_cfg.zero = tmp;

			SAMPLE_PRT("trail:\n");
			scanf("%d", &tmp);
			hs_cfg.trail = tmp;

			mipi_tx_set_hs_settle(fd, &hs_cfg);
		} else
			break;
	} while (1);

#if !defined(CONFIG_DUAL_OS)
	close(fd);
#endif

}

void SAMPLE_SET_PANEL_DESC(void)
{
	switch (g_input_para.panel_model) {
	case DSI_PANEL_ILI9881C:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_ili9881c_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_ili9881c_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_ili9881c_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ili9881c_720x1280_NULL_4lane_60fps);
		break;
	case DSI_PANEL_ILI9881D:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_ili9881d_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_ili9881d_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_ili9881d_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ili9881d_720x1280_NULL_4lane_60fps);
		break;
	case DSI_PANEL_JD9366AB:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_jd9366ab_800x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_jd9366ab_800x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_jd9366ab_800x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_jd9366ab_800x1280_NULL_4lane_60fps);
		break;
	case DSI_PANEL_NT35521:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_nt35521_800x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_nt35521_800x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_nt35521_800x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_nt35521_800x1280_NULL_4lane_60fps);
		break;
	case DSI_PANEL_OTA7290B:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_ota7290b_320x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_ota7290b_320x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_ota7290b_320x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ota7290b_320x1280_NULL_4lane_60fps);
		break;
	case DSI_PANEL_OTA7290B_1920:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_ota7290b_440x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_ota7290b_440x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_ota7290b_440x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_ota7290b_440x1920_NULL_4lane_60fps);
		break;
	case DSI_PANEL_ICN9707:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_icn9707_480x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_icn9707_480x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_icn9707_480x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_icn9707_480x1920_NULL_4lane_60fps);
		break;
	case DSI_PANEL_3AML069LP01G:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_3aml069lp01g_600x1024_NULL_4lane_60fps);
		break;
	case DSI_PANEL_ST7701:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_st7701_480x800_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_st7701_480x800_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_st7701_480x800_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_st7701_480x800_NULL_2lane_60fps);
		break;
	case DSI_PANEL_ST7701_480x640:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_st7701_480x640_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_st7701_480x640_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_st7701_480x640_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_st7701_480x640_NULL_2lane_60fps);
		break;
	case DSI_PANEL_ST7703:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_st7703_640x480_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_st7703_640x480_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_st7703_640x480_NULL_2lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_st7703_640x480_NULL_2lane_60fps);
		break;
	case DSI_PANEL_HX8399_1080P:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_hx8399_1080x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_hx8399_1080x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_hx8399_1080x1920_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8399_1080x1920_NULL_4lane_60fps);
		break;
	case DSI_PANEL_GM8775C:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_gm8775c_1920x1080_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_gm8775c_1920x1080_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_gm8775c_1920x1080_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_gm8775c_1920x1080_NULL_4lane_60fps);
		break;
	case DSI_PANEL_LT9611_1920x1080_60:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_lt9611_1920x1080_60Hz;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		g_panel_desc.stdsicfg.dsi_init_cmds = NULL;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1920x1080_30:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_lt9611_1920x1080_30Hz;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		g_panel_desc.stdsicfg.dsi_init_cmds = NULL;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1280x720_60:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_lt9611_1280x720_60Hz;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		g_panel_desc.stdsicfg.dsi_init_cmds = NULL;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1024x768_60:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_lt9611_1024x768_60Hz;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		g_panel_desc.stdsicfg.dsi_init_cmds = NULL;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1280x1024_60:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_lt9611_1280x1024_60Hz;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		g_panel_desc.stdsicfg.dsi_init_cmds = NULL;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_1600x1200_60:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_lt9611_1600x1200_60Hz;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		g_panel_desc.stdsicfg.dsi_init_cmds = NULL;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_LT9611_2560x1440_60:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_lt9611_2560x1440_60Hz;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_lt9611;
		g_panel_desc.stdsicfg.dsi_init_cmds = NULL;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = 0;
		break;
	case DSI_PANEL_HX8394_EVB:
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps);
		break;
	case LVDS_PANEL_LCM185X56:
		g_panel_desc.panel_type = PANEL_MODE_LVDS;
		g_panel_desc.stlvdscfg.stVoPubAttr.enIntfType = VO_INTF_LVDS;
		g_panel_desc.stlvdscfg.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stLcm185x56_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 60
		, .u16Vact = 768, .u16Vbb = 20, .u16Vfb = 10
		, .u16Hact = 1366, .u16Hbb = 100, .u16Hfb = 88
		, .u16Vpw = 2, .u16Hpw = 20, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		g_panel_desc.stlvdscfg.stVoPubAttr.stSyncInfo = stLcm185x56_SyncInfo;
		g_panel_desc.stlvdscfg.LvdsAttr = lvds_lcm185x56_cfg;
		break;
	case I80_PANEL_ST7789V3_HW_MCU_240x320_60FPS:
		g_panel_desc.panel_type = PANEL_MODE_MCU;
		g_panel_desc.stVoPubAttr.enIntfType = VO_INTF_HW_MCU;
		g_panel_desc.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S st7789V3_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 60
		, .u16Vact = 320, .u16Vbb = 5, .u16Vfb = 20
		, .u16Hact = 240, .u16Hbb = 20, .u16Hfb = 10
		, .u16Vpw = 5, .u16Hpw = 5, .bIdv = 0, .bIhs = 1, .bIvs = 1};
		g_panel_desc.stVoPubAttr.stSyncInfo = st7789V3_SyncInfo;
		g_panel_desc.stVoPubAttr.stMcuCfg = st7789v3Cfg;
		break;
	case BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M:
		g_panel_desc.panel_type = PANEL_MODE_BT;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfType = VO_INTF_BT656;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stPt1000kbt656_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 25
		, .u16Vact = 720, .u16Vbb = 20, .u16Vfb = 5
		, .u16Hact = 1280, .u16Hbb = 220, .u16Hfb = 440
		, .u16Vpw = 5, .u16Hpw = 40, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		g_panel_desc.stbtcfg.stVoPubAttr.stSyncInfo = stPt1000kbt656_SyncInfo;
		g_panel_desc.stbtcfg.BtAttr = stpt1000kbt656cfg;
		break;
	case BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M:
		g_panel_desc.panel_type = PANEL_MODE_BT;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfType = VO_INTF_BT1120;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stPt1000kbt1120_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 25
		, .u16Vact = 1080, .u16Vbb = 20, .u16Vfb = 20
		, .u16Hact = 1920, .u16Hbb = 356, .u16Hfb = 356
		, .u16Vpw = 5, .u16Hpw = 8, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		g_panel_desc.stbtcfg.stVoPubAttr.stSyncInfo = stPt1000kbt1120_SyncInfo;
		g_panel_desc.stbtcfg.BtAttr = stpt1000kbt1120cfg;
		break;
	case MS7024_BT656_480P_60FPS:
		g_panel_desc.panel_type = PANEL_MODE_BT;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfType = VO_INTF_BT656;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stMS7024bt656_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 60
		, .u16Vact = 480, .u16Vbb = 30, .u16Vfb = 9
		, .u16Hact = 720, .u16Hbb = 60, .u16Hfb = 16
		, .u16Vpw = 6, .u16Hpw = 62, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		g_panel_desc.stbtcfg.stVoPubAttr.stSyncInfo = stMS7024bt656_SyncInfo;
		g_panel_desc.stbtcfg.BtAttr = stMS7024bt656cfg;
		break;
	case MS7024_BT656_480P_30FPS:
	case MS7024_BT656_480P_30FPS_TESTPATTERN: {
		g_panel_desc.panel_type = PANEL_MODE_BT;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfType = VO_INTF_BT656;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stMS7024bt656_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 30
		, .u16Vact = 480, .u16Vbb = 30, .u16Vfb = 9
		, .u16Hact = 720, .u16Hbb = 60, .u16Hfb = 16
		, .u16Vpw = 6, .u16Hpw = 62, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		g_panel_desc.stbtcfg.stVoPubAttr.stSyncInfo = stMS7024bt656_SyncInfo;
		g_panel_desc.stbtcfg.BtAttr = stMS7024bt656cfg;
		break;
	}
	case MS7024_BT656_576P_50FPS: {
		g_panel_desc.panel_type = PANEL_MODE_BT;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfType = VO_INTF_BT656;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stMS7024bt656_576p_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 50
		, .u16Vact = 576, .u16Vbb = 39, .u16Vfb = 5
		, .u16Hact = 720, .u16Hbb = 68, .u16Hfb = 12
		, .u16Vpw = 5, .u16Hpw = 64, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		g_panel_desc.stbtcfg.stVoPubAttr.stSyncInfo = stMS7024bt656_576p_SyncInfo;
		g_panel_desc.stbtcfg.BtAttr = stMS7024bt656cfg;
		break;
	}
	case MS7024_BT656_576P_25FPS: {
		g_panel_desc.panel_type = PANEL_MODE_BT;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfType = VO_INTF_BT656;
		g_panel_desc.stbtcfg.stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
		VO_SYNC_INFO_S stMS7024bt656_576p_SyncInfo = {.bSynm = 1, .bIop = 1, .u16FrameRate = 25
		, .u16Vact = 576, .u16Vbb = 39, .u16Vfb = 5
		, .u16Hact = 720, .u16Hbb = 68, .u16Hfb = 12
		, .u16Vpw = 5, .u16Hpw = 64, .bIdv = 0, .bIhs = 0, .bIvs = 0};
		g_panel_desc.stbtcfg.stVoPubAttr.stSyncInfo = stMS7024bt656_576p_SyncInfo;
		g_panel_desc.stbtcfg.BtAttr = stMS7024bt656cfg;
		break;
	}
	default:
		SAMPLE_PRT("default\n");
		g_panel_desc.panel_type = PANEL_MODE_DSI;
		g_panel_desc.stdsicfg.dev_cfg = (struct combo_dev_cfg_s *)&dev_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.hs_timing_cfg = &hs_timing_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds = dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps;
		g_panel_desc.stdsicfg.dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps);
		break;
	}
	if (g_input_para.pn_swap_flag) {
		if (g_panel_desc.panel_type == PANEL_MODE_LVDS) {
			for (CVI_U32 i = 0; i < LANE_MAX_NUM; i++) {
				g_panel_desc.stlvdscfg.LvdsAttr.lane_pn_swap[i] =
				(VO_LVDS_LANE_ID)g_input_para.lane_pn_swap[i];
			}
		} else if (g_panel_desc.panel_type == PANEL_MODE_DSI) {
			for (CVI_U32 i = 0; i < LANE_MAX_NUM; i++) {
				g_panel_desc.stdsicfg.dev_cfg->lane_pn_swap[i] = g_input_para.lane_pn_swap[i];
			}
		}
	}
	if (g_input_para.lane_id_flag) {
		if (g_panel_desc.panel_type == PANEL_MODE_LVDS) {
			for (CVI_U32 i = 0; i < LANE_MAX_NUM; i++) {
				g_panel_desc.stlvdscfg.LvdsAttr.lane_id[i] =
				(VO_LVDS_LANE_ID)g_input_para.lane_id[i];
			}
		} else if (g_panel_desc.panel_type == PANEL_MODE_DSI) {
			for (CVI_U32 i = 0; i < LANE_MAX_NUM; i++) {
				g_panel_desc.stdsicfg.dev_cfg->lane_id[i] = g_input_para.lane_id[i];
			}
		}
	}
	if (g_input_para.dev_flag) {
		if (g_panel_desc.panel_type == PANEL_MODE_DSI) {
			for (CVI_U32 i = 0; i < LANE_MAX_NUM; i++) {
				g_panel_desc.stdsicfg.dev_cfg->devno = g_input_para.dev_no;
			}
		}
	}
}

CVI_S32 SAMPLE_SET_PANEL_MODEL(char *pinput_str)
{
	CVI_S32 i = 0;
	bool is_find = false;
	PANEL_MODEL panel_model = DSI_PANEL_HX8394_EVB;

	for (i = 0; i < PANEL_MAX; i++) {
		if (strcmp(pinput_str, s_panel_model_type_arr[i]) == 0) {
			is_find = true;
			break;
		}
	}

	if (is_find) {
		panel_model = (PANEL_MODEL)i;
	} else {
		return CVI_FAILURE;
	}

	g_input_para.panel_model = panel_model;
	g_panel_desc.panel_mode = s_panel_model_type_arr[i];
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_SET_LANEID(char *pLaneid)
{
	CVI_S32 lane_id[] = {0, 0, 0, 0, 0};

	if (pLaneid == NULL)
		return CVI_FAILURE;

	CVI_S32 n = sscanf(pLaneid, "%02d,%02d,%02d,%02d,%02d",
		&lane_id[0], &lane_id[1], &lane_id[2], &lane_id[3], &lane_id[4]);

	if (n != sizeof(lane_id)/sizeof(CVI_S32)) {
		return CVI_FAILURE;
	}
	g_input_para.lane_id_flag = true;
	for (CVI_U32 i = 0; i < sizeof(lane_id)/sizeof(CVI_S32); i++) {
		if (lane_id[i] < -1 || lane_id[i] > 5) {
			return CVI_FAILURE;
		}
		g_input_para.lane_id[i] = lane_id[i];
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_SET_PNSWAP(char *pPnswap)
{
	CVI_U32 pnswap[] = {0, 0, 0, 0, 0};

	if (pPnswap == NULL)
		return CVI_FAILURE;

	CVI_S32 n = sscanf(pPnswap, "%02d,%02d,%02d,%02d,%02d",
		&pnswap[0], &pnswap[1], &pnswap[2], &pnswap[3], &pnswap[4]);

	if (n != sizeof(pnswap)/sizeof(CVI_U32)) {
		return CVI_FAILURE;
	}
	g_input_para.pn_swap_flag = true;
	for (CVI_U32 i = 0; i < sizeof(pnswap)/sizeof(CVI_U32); i++) {
		if ((pnswap[i] != 0) && (pnswap[i] != 1)) {
			return CVI_FAILURE;
		}
		g_input_para.lane_pn_swap[i] = (bool)(pnswap[i]);
	}

	return CVI_SUCCESS;
}

// CVI_S32 SAMPLE_PANEL_CONTROL_PINS_CONFIG(char *pControlPins)
// {
//	CVI_S32 pwr_gpio, rst_gpio, bl_gpio;

//	if (pControlPins == NULL)
//		return CVI_FAILURE;

//	CVI_S32 n = sscanf(pControlPins, "%03d,%03d,%03d",
//		&pwr_gpio, &rst_gpio, &bl_gpio);

//	if (n != 3)
//		return CVI_FAILURE;

//	if (GPIO_IN_RANGE(pwr_gpio)) {
//		SAMPLE_COMM_GPIO_Export(pwr_gpio);
//		SAMPLE_COMM_GPIO_SetDirection(pwr_gpio, 1);
//		SAMPLE_COMM_GPIO_SetValue(pwr_gpio, 1);
//	}
//	if (GPIO_IN_RANGE(bl_gpio)) {
//		SAMPLE_COMM_GPIO_Export(bl_gpio);
//		SAMPLE_COMM_GPIO_SetDirection(bl_gpio, 1);
//		SAMPLE_COMM_GPIO_SetValue(bl_gpio, 1);
//	}
//	if (GPIO_IN_RANGE(rst_gpio)) {
//		SAMPLE_COMM_GPIO_Export(rst_gpio);
//		SAMPLE_COMM_GPIO_SetDirection(rst_gpio, 1);
//		SAMPLE_COMM_GPIO_SetValue(rst_gpio, 1);
//		usleep(10 * 1000);
//		SAMPLE_COMM_GPIO_SetValue(rst_gpio, 0);
//		usleep(10 * 1000);
//		SAMPLE_COMM_GPIO_SetValue(rst_gpio, 1);
//		usleep(10 * 1000);
//	}

//	return CVI_SUCCESS;
// }

void SAMPLE_PANEL_I2C_SEND(void)
{
	CVI_S32 ret;

	if (g_input_para.panel_model == BT_PANEL_PT1000K_BT656_1280x720_25FPS_74M) {
		ret = panel_i2c_init(g_input_para.dev_no);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("panel_i2c_init fail");
		}
		for (CVI_U32 i = 0; i < ARRAY_SIZE(bt656_720p25_pt1000k_init_cmds); i++) {
			ret = panel_write_register(g_input_para.dev_no, bt656_720p25_pt1000k_init_cmds[i].addr,
				  bt656_720p25_pt1000k_init_cmds[i].data);
			if (ret != CVI_SUCCESS)
				SAMPLE_PRT("i2c_write fail addr[0x%x]\n", bt656_720p25_pt1000k_init_cmds[i].addr);
		}
	} else if (g_input_para.panel_model == BT_PANEL_PT1000K_BT1120_1920x1080_25FPS_74M) {
		ret = panel_i2c_init(g_input_para.dev_no);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("panel_i2c_init fail");
		}
		for (CVI_U32 i = 0; i < ARRAY_SIZE(bt1120_1080p25_pt1000k_init_cmds); i++) {
			ret = panel_write_register(g_input_para.dev_no, bt1120_1080p25_pt1000k_init_cmds[i].addr,
				  bt1120_1080p25_pt1000k_init_cmds[i].data);
			if (ret != CVI_SUCCESS)
				SAMPLE_PRT("i2c_write fail addr[0x%x]\n", bt1120_1080p25_pt1000k_init_cmds[i].addr);
		}
	} else if (g_input_para.panel_model == MS7024_BT656_480P_60FPS) {
		ret = panel_i2c_init(g_input_para.dev_no);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("panel_i2c_init fail");
		}
		for (CVI_U32 i = 0; i < ARRAY_SIZE(bt656_480p60_ms7024_init_cmds); i++) {
			ret = panel_write_register(g_input_para.dev_no, bt656_480p60_ms7024_init_cmds[i].addr,
				  bt656_480p60_ms7024_init_cmds[i].data);
			if (ret != CVI_SUCCESS)
				SAMPLE_PRT("i2c_write fail addr[0x%x]\n", bt656_480p60_ms7024_init_cmds[i].addr);
			if(bt656_480p60_ms7024_init_cmds[i].delay) {
				usleep(bt656_480p60_ms7024_init_cmds[i].delay * 1000);
			}
		}
	} else if (g_input_para.panel_model == MS7024_BT656_480P_30FPS) {
		ret = panel_i2c_init(g_input_para.dev_no);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("panel_i2c_init fail");
		}
		for (CVI_U32 i = 0; i < ARRAY_SIZE(bt656_480p30_ms7024_init_cmds); i++) {
			ret = panel_write_register(g_input_para.dev_no, bt656_480p30_ms7024_init_cmds[i].addr,
				  bt656_480p30_ms7024_init_cmds[i].data);
			if (ret != CVI_SUCCESS)
				SAMPLE_PRT("i2c_write fail addr[0x%x]\n", bt656_480p30_ms7024_init_cmds[i].addr);
			if(bt656_480p30_ms7024_init_cmds[i].delay) {
				usleep(bt656_480p30_ms7024_init_cmds[i].delay * 1000);
			}
		}
	} else if (g_input_para.panel_model == MS7024_BT656_576P_50FPS) {
		ret = panel_i2c_init(g_input_para.dev_no);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("panel_i2c_init fail");
		}
		for (CVI_U32 i = 0; i < ARRAY_SIZE(bt656_576p50_ms7024_init_cmds); i++) {
			ret = panel_write_register(g_input_para.dev_no, bt656_576p50_ms7024_init_cmds[i].addr,
				  bt656_576p50_ms7024_init_cmds[i].data);
			if (ret != CVI_SUCCESS)
				SAMPLE_PRT("i2c_write fail addr[0x%x]\n", bt656_576p50_ms7024_init_cmds[i].addr);
			if(bt656_576p50_ms7024_init_cmds[i].delay) {
				usleep(bt656_576p50_ms7024_init_cmds[i].delay * 1000);
			}
		}
	} else if (g_input_para.panel_model == MS7024_BT656_576P_25FPS) {
		ret = panel_i2c_init(g_input_para.dev_no);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("panel_i2c_init fail");
		}
		for (CVI_U32 i = 0; i < ARRAY_SIZE(bt656_576p25_ms7024_init_cmds); i++) {
			ret = panel_write_register(g_input_para.dev_no, bt656_576p25_ms7024_init_cmds[i].addr,
				  bt656_576p25_ms7024_init_cmds[i].data);
			if (ret != CVI_SUCCESS)
				SAMPLE_PRT("i2c_write fail addr[0x%x]\n", bt656_576p25_ms7024_init_cmds[i].addr);
			if(bt656_576p25_ms7024_init_cmds[i].delay) {
				usleep(bt656_576p25_ms7024_init_cmds[i].delay * 1000);
			}
		}
	} else if (g_input_para.panel_model == MS7024_BT656_480P_30FPS_TESTPATTERN) {
		ret = panel_i2c_init(g_input_para.dev_no);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("panel_i2c_init fail");
		}
		for (CVI_U32 i = 0; i < ARRAY_SIZE(bt656_480p30_testpattern_ms7024_init_cmds); i++) {
			ret = panel_write_register(g_input_para.dev_no, bt656_480p30_testpattern_ms7024_init_cmds[i].addr,
				  bt656_480p30_testpattern_ms7024_init_cmds[i].data);
			if (ret != CVI_SUCCESS)
				SAMPLE_PRT("i2c_write fail addr[0x%x]\n", bt656_480p30_testpattern_ms7024_init_cmds[i].addr);
			if(bt656_480p30_testpattern_ms7024_init_cmds[i].delay) {
				usleep(bt656_480p30_testpattern_ms7024_init_cmds[i].delay * 1000);
			}
		}
	}
}


int main(int argc, char *argv[])
{
	if (argc == 1) {
		printHelp(argv);
		return CVI_SUCCESS;
	}

	struct option long_options[MAX_OPTIONS + 1];
	CVI_S32 ch, idx, ret, patern_cmd = VO_PAT_MAX;
	bool is_pattern = false;

	memset((void *)long_options, 0, sizeof(long_options));

	for (idx = 0; idx < MAX_OPTIONS; idx++) {
		if (long_option_ext[idx].opt.name == NULL)
			break;

		if (idx >= MAX_OPTIONS) {
			SAMPLE_PRT("too many options\n");
			return -1;
		}

		memcpy(&long_options[idx], &long_option_ext[idx].opt, sizeof(struct option));
	}

#if defined(CONFIG_DUAL_OS)
	if (CVI_SYS_Init() != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}
#endif

	optind = 0;
	while ((ch = getopt_long(argc, argv, "dh", long_options, &idx)) != -1) {
		switch (ch) {
		case 'c':
			// ret = SAMPLE_PANEL_CONTROL_PINS_CONFIG(optarg);
			// if (ret != CVI_SUCCESS) {
			//	SAMPLE_PRT("panel control_pins config fail!\n");
			//	return ret;
			// }
			break;
		case 'l':
			ret = SAMPLE_SET_LANEID(optarg);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("invalid laneid parameter\n");
				return ret;
			}
			break;
		case 'p':
			ret = SAMPLE_SET_PNSWAP(optarg);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("invalid pnswap parameter\n");
				return ret;
			}
			break;
		case 'm':
			ret = SAMPLE_SET_PANEL_MODEL(optarg);
			if (ret != CVI_SUCCESS) {
				SAMPLE_PRT("invalid input panel model\n");
				return ret;
			}
			break;
		case 'd':
			if (argc > 3) {
				SAMPLE_PRT("usage:%s -d. -d can't use in the same time with other command\n", argv[0]);
				return CVI_FAILURE;
			}
			SAMPLE_DSI_CONTROLE();
			break;
		case 's':
			is_pattern = true;
			if (optarg != NULL) {
				ret = sscanf(optarg, "%02d", &patern_cmd);
				if (patern_cmd == VO_PAT_MAX)
					patern_cmd = -1;
			}
			break;
		case 'h':
			printHelp(argv);
			return CVI_SUCCESS;
		default:
			SAMPLE_PRT("ch = %c\n", ch);
			printHelp(argv);
			return CVI_FAILURE;
		}
	}

	if (optind < argc) {
		printHelp(argv);
	}

	SAMPLE_SET_PANEL_DESC();
	SAMPLE_PANEL_ENABLE();

	if (is_pattern) {
		ret = SAMPLE_PANEL_ShowPattern(g_input_para.dev_no, patern_cmd);
		if (ret == CVI_SUCCESS)
			SAMPLE_PRT("sample_panel exit success!\n");
		else
			SAMPLE_PRT("sample_panel exit abnormally!\n");
	}

	if (g_panel_desc.panel_type == PANEL_MODE_BT)
		SAMPLE_PANEL_I2C_SEND();

#if defined(CONFIG_DUAL_OS)
	if (CVI_SYS_Exit() != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Exit failed!\n");
		return CVI_FAILURE;
	}
#endif

	return CVI_SUCCESS;
}
