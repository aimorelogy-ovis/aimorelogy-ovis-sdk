#ifndef _BT656_PT1000K_H_
#define _BT656_PT1000K_H_

#include <cvi_comm_vo.h>

const VO_BT_ATTR_S stpt1000kbt656cfg = {
    .pin_num = 9,
    .bt_clk_inv = 0,
    .bt_vs_inv = 0,
    .bt_hs_inv = 0,
    .data_seq = VO_BT_DATA_SEQ0,
    .d_pins = {
        { VO_MIPI_TXP0, VO_MUX_BT_DATA0 },
        { VO_MIPI_TXM0, VO_MUX_BT_DATA1 },
        { VO_MIPI_TXP1, VO_MUX_BT_DATA2 },
        { VO_MIPI_TXM1, VO_MUX_BT_DATA3 },
        { VO_MIPI_TXM2, VO_MUX_BT_DATA4 },
        { VO_MIPI_TXP3, VO_MUX_BT_DATA5 },
        { VO_MIPI_TXM3, VO_MUX_BT_DATA6 },
        { VO_MIPI_TXP4, VO_MUX_BT_DATA7 },
        { VO_MIPI_TXP2, VO_MUX_BT_CLK },
    },
};

#endif // _BT656_PT1000K_H_