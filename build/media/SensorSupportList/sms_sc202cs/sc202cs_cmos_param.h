#ifndef __SC202CS_CMOS_PARAM_H_
#define __SC202CS_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "sc202cs_cmos_ex.h"

static const SC202CS_MODE_S g_astSC202CS_mode[SC202CS_MODE_NUM] = {
	[SC202CS_MODE_800X600P30] = {
		.name = "800x600p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 800,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 2.28, /* 1250 * 30 / 0x3FFF */
		.u32HtsDef = 2560,
		.u32VtsDef = 1250,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1250 - 6,
			.u16Def = 1244,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 3968,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC202CS_MODE_512X320P30_1L_MASTER_10BIT] = {
		.name = "512x320p30_master_10bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 512,
				.u32Height = 320,
			},
			.stMaxSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 2.28, /* 1250 * 30 / 0x3FFF */
		.u32HtsDef = 2560,
		.u32VtsDef = 1250,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1250 - 6,
			.u16Def = 1244,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 3968,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC202CS_MODE_512X320P30_1L_SLAVE_10BIT] = {
		.name = "512x320p30_slave_10bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 512,
				.u32Height = 320,
			},
			.stMaxSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 2.28, /* 1250 * 30 / 0x3FFF */
		.u32HtsDef = 2560,
		.u32VtsDef = 1250,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1250 - 6,
			.u16Def = 1244,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 3968,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {
			260, 260, 260, 260, 0, 0, 0, 0
		},
		.stAuto = {
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

struct combo_dev_attr_s sc202cs_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {0, 1, -1, -1, -1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.pn_swap = {0, 0, 0, 0, 0},
		.dphy = {
			.enable = CVI_TRUE,
			.hs_settle = 8,
		}
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC202CS_CMOS_PARAM_H_ */
