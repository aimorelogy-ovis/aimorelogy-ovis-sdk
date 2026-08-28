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
        { VO_VIVO_D2, VO_MUX_BT_DATA0 },
        { VO_VIVO_D3, VO_MUX_BT_DATA1 },
        { VO_VIVO_D4, VO_MUX_BT_DATA2 },
        { VO_VIVO_D5, VO_MUX_BT_DATA3 },
        { VO_VIVO_D6, VO_MUX_BT_DATA4 },
        { VO_VIVO_D7, VO_MUX_BT_DATA5 },
        { VO_VIVO_D8, VO_MUX_BT_DATA6 },
        { VO_VIVO_D9, VO_MUX_BT_DATA7 },
        { VO_VIVO_CLK, VO_MUX_BT_CLK },
    },
};

#endif // _BT656_MS7024_H_
