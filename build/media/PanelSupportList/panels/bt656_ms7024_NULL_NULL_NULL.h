#ifndef _BT656_MS7024_H_
#define _BT656_MS7024_H_

#include <cvi_comm_vo.h>

const VO_BT_ATTR_S stMS7024bt656cfg = {
    .pin_num = 9,
    .bt_clk_inv = 0,
    .bt_vs_inv = 0,
    .bt_hs_inv = 0,
    .data_seq = VO_BT_DATA_SEQ0,
    .d_pins = {
        { VO_MIPI_TXM2, VO_MUX_BT_DATA0 },
        { VO_MIPI_TXM0, VO_MUX_BT_DATA1 },
        { VO_MIPI_TXP0, VO_MUX_BT_DATA2 },
        { VO_SD1_D0, VO_MUX_BT_DATA3 },
        { VO_SD1_D1, VO_MUX_BT_DATA4 },
        { VO_SD1_D2, VO_MUX_BT_DATA5 },
        { VO_SD1_D3, VO_MUX_BT_DATA6 },
        { VO_SD1_CLK, VO_MUX_BT_DATA7 },
        { VO_MIPI_TXP2, VO_MUX_BT_CLK },
    },
};

#endif // _BT656_MS7024_H_