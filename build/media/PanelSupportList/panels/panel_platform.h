#ifndef __PANEL_PLATFORM_H__
#define __PANEL_PLATFORM_H__

// clang-format off
#define PIXEL_CLK(x)                          \
    ((x##_VACT + x##_VSA + x##_VBP + x##_VFP) \
        * (x##_HACT + x##_HSA + x##_HBP + x##_HFP) * x##_FPS / 1000)

/* First argument of DSI_CMD/DSI_CMD_TYPE is delay in ms after sending. */
#define DSI_DCS_DATA_TYPE_BY_SIZE(size) \
    ((size) == 1 ? 0x05 : ((size) == 2 ? 0x15 : 0x29))

#define DSI_CMD(delay_ms, ...)                                                      \
    {                                                                               \
        .delay = (delay_ms),                                                        \
        .data_type = DSI_DCS_DATA_TYPE_BY_SIZE(sizeof((CVI_U8[]) { __VA_ARGS__ })), \
        .size = sizeof((CVI_U8[]) { __VA_ARGS__ }),                                 \
        .data = (CVI_U8[]) { __VA_ARGS__ }                                          \
    }

#define DSI_CMD_TYPE(delay_ms, type, ...)           \
    {                                               \
        .delay = (delay_ms),                        \
        .data_type = (type),                        \
        .size = sizeof((CVI_U8[]) { __VA_ARGS__ }), \
        .data = (CVI_U8[]) { __VA_ARGS__ }          \
    }
// clang-format on

#ifdef __UBOOT__
#include <cvi_lvds.h>
#include <cvi_mipi.h>
#include <cvi_sw_i80.h>
#include <cvi_hw_i80.h>
#ifndef CVI_U8
#define CVI_U8 u8
#endif
typedef int VO_PUB_ATTR_S;
typedef int VO_BT_ATTR_S;
#else
#include <cvi_comm_mipi_tx.h>
#include <cvi_type.h>
#endif

#endif
