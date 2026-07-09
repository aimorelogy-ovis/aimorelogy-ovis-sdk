#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "cvi_common.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_panel_i2c.h"

typedef struct PANEL_I2C_CMD_S {
    CVI_U8 addr;
    CVI_U8 data;
    CVI_U8 delay_ms;
} PANEL_I2C_CMD_T;

static int g_panel_i2c_fd[VO_MAX_DEV_NUM] = {[0 ... (VO_MAX_DEV_NUM - 1)] = -1};

static const PANEL_I2C_CMD_T g_ms7024_i2c_init_cmds[] = {
    {.addr = 0x04, .data = 0x1c, .delay_ms = 0},
    {.addr = 0x0e, .data = 0x57, .delay_ms = 0},
    {.addr = 0x0f, .data = 0x01, .delay_ms = 0},
    {.addr = 0x30, .data = 0x02, .delay_ms = 0},
    {.addr = 0x31, .data = 0x4a, .delay_ms = 0},
    {.addr = 0x32, .data = 0x03, .delay_ms = 0},
    {.addr = 0x33, .data = 0x7a, .delay_ms = 0},
    {.addr = 0x34, .data = 0x00, .delay_ms = 0},
    {.addr = 0x35, .data = 0x07, .delay_ms = 0},
    {.addr = 0x36, .data = 0x02, .delay_ms = 0},
    {.addr = 0x37, .data = 0x27, .delay_ms = 0},
    {.addr = 0x38, .data = 0x00, .delay_ms = 0},
    {.addr = 0x39, .data = 0x00, .delay_ms = 0},
    {.addr = 0x3a, .data = 0x00, .delay_ms = 0},
    {.addr = 0x3b, .data = 0x00, .delay_ms = 0},
    {.addr = 0x3c, .data = 0x00, .delay_ms = 0},
    {.addr = 0x90, .data = 0x02, .delay_ms = 0},
    {.addr = 0x91, .data = 0x00, .delay_ms = 0},
    {.addr = 0x92, .data = 0x00, .delay_ms = 0},
    {.addr = 0x93, .data = 0x00, .delay_ms = 0},
    {.addr = 0x94, .data = 0x00, .delay_ms = 0},
    {.addr = 0x95, .data = 0x00, .delay_ms = 0},
    {.addr = 0x96, .data = 0x00, .delay_ms = 0},
    {.addr = 0x97, .data = 0x00, .delay_ms = 0},
    {.addr = 0x98, .data = 0x00, .delay_ms = 0},
    {.addr = 0x99, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9a, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9b, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9c, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9d, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9e, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9d, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9e, .data = 0x00, .delay_ms = 0},
    {.addr = 0x9f, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa0, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa1, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa2, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa4, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa5, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa6, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa7, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa8, .data = 0x00, .delay_ms = 0},
    {.addr = 0xa9, .data = 0x07, .delay_ms = 0},
    {.addr = 0xaa, .data = 0x02, .delay_ms = 0},
    {.addr = 0xab, .data = 0x05, .delay_ms = 0},
    {.addr = 0xac, .data = 0x15, .delay_ms = 0},
    {.addr = 0xad, .data = 0x89, .delay_ms = 0},
    {.addr = 0x50, .data = 0x00, .delay_ms = 0},
    {.addr = 0x51, .data = 0x16, .delay_ms = 0},
    {.addr = 0x52, .data = 0x0b, .delay_ms = 0},
    {.addr = 0x53, .data = 0x00, .delay_ms = 0},
    {.addr = 0x54, .data = 0x00, .delay_ms = 0},
    {.addr = 0x55, .data = 0x00, .delay_ms = 0},
    {.addr = 0x56, .data = 0x00, .delay_ms = 0},
    {.addr = 0x57, .data = 0x00, .delay_ms = 0},
    {.addr = 0x58, .data = 0x00, .delay_ms = 0},
    {.addr = 0x59, .data = 0x00, .delay_ms = 0},
    {.addr = 0x5a, .data = 0x00, .delay_ms = 0},
    {.addr = 0x20, .data = 0x59, .delay_ms = 0},
    {.addr = 0x21, .data = 0x11, .delay_ms = 0},
    {.addr = 0x22, .data = 0x63, .delay_ms = 0},
    {.addr = 0x23, .data = 0x01, .delay_ms = 0},
    {.addr = 0x24, .data = 0x00, .delay_ms = 0},
    {.addr = 0x25, .data = 0x00, .delay_ms = 0},
    {.addr = 0x26, .data = 0x00, .delay_ms = 0},
    {.addr = 0x27, .data = 0xc1, .delay_ms = 0},
    {.addr = 0x28, .data = 0xc1, .delay_ms = 0},
    {.addr = 0x29, .data = 0x80, .delay_ms = 0},
    {.addr = 0x2a, .data = 0x84, .delay_ms = 0},
    {.addr = 0x2b, .data = 0x00, .delay_ms = 0},
    {.addr = 0x2c, .data = 0x00, .delay_ms = 0},
    {.addr = 0x2d, .data = 0x00, .delay_ms = 0},
    {.addr = 0x60, .data = 0x03, .delay_ms = 0},
    {.addr = 0x61, .data = 0x00, .delay_ms = 0},
    {.addr = 0x62, .data = 0x01, .delay_ms = 0},
    {.addr = 0x63, .data = 0x00, .delay_ms = 0},
    {.addr = 0x64, .data = 0x20, .delay_ms = 0},
    {.addr = 0x66, .data = 0x00, .delay_ms = 0},
    {.addr = 0x67, .data = 0x40, .delay_ms = 0},
    {.addr = 0x68, .data = 0x00, .delay_ms = 0},
    {.addr = 0x69, .data = 0x20, .delay_ms = 0},
    {.addr = 0x6a, .data = 0x40, .delay_ms = 0},
    {.addr = 0x6b, .data = 0x60, .delay_ms = 0},
    {.addr = 0x6c, .data = 0x80, .delay_ms = 0},
    {.addr = 0x6d, .data = 0xa0, .delay_ms = 0},
    {.addr = 0x6e, .data = 0xc0, .delay_ms = 0},
    {.addr = 0x6f, .data = 0xe0, .delay_ms = 0},
    {.addr = 0x70, .data = 0xff, .delay_ms = 0},
    {.addr = 0x71, .data = 0x03, .delay_ms = 0},
    {.addr = 0x72, .data = 0x4b, .delay_ms = 0},
    {.addr = 0x73, .data = 0x40, .delay_ms = 0},
    {.addr = 0x74, .data = 0x40, .delay_ms = 0},
    {.addr = 0x75, .data = 0x40, .delay_ms = 0},
    {.addr = 0x76, .data = 0x40, .delay_ms = 0},
    {.addr = 0x77, .data = 0x5b, .delay_ms = 0},
    {.addr = 0x78, .data = 0x5b, .delay_ms = 0},
    {.addr = 0x79, .data = 0x5b, .delay_ms = 0},
    {.addr = 0x7a, .data = 0x5b, .delay_ms = 0},
    {.addr = 0x7b, .data = 0x02, .delay_ms = 0},
    {.addr = 0x7c, .data = 0x8c, .delay_ms = 0},
    {.addr = 0x7d, .data = 0xd4, .delay_ms = 0},
    {.addr = 0x7e, .data = 0x72, .delay_ms = 0},
    {.addr = 0x7f, .data = 0x00, .delay_ms = 0},
    {.addr = 0x80, .data = 0x00, .delay_ms = 0},
    {.addr = 0x81, .data = 0x00, .delay_ms = 0},
    {.addr = 0x82, .data = 0x04, .delay_ms = 0},
    {.addr = 0x83, .data = 0x00, .delay_ms = 0},
    {.addr = 0x84, .data = 0xff, .delay_ms = 0},
    {.addr = 0x85, .data = 0xce, .delay_ms = 0},
    {.addr = 0x86, .data = 0xb2, .delay_ms = 0},
    {.addr = 0x87, .data = 0x00, .delay_ms = 0},
    {.addr = 0x88, .data = 0x00, .delay_ms = 0},
    {.addr = 0x89, .data = 0x93, .delay_ms = 0},
    {.addr = 0x8a, .data = 0x06, .delay_ms = 0},
    {.addr = 0x5f, .data = 0x01, .delay_ms = 0},
    {.addr = 0x2e, .data = 0x10, .delay_ms = 0},
    {.addr = 0x20, .data = 0x56, .delay_ms = 0},
    {.addr = 0x2e, .data = 0x11, .delay_ms = 0},
    {.addr = 0x20, .data = 0x50, .delay_ms = 10},
    {.addr = 0x20, .data = 0x59, .delay_ms = 10},
    {.addr = 0x20, .data = 0x56, .delay_ms = 0},
    {.addr = 0x20, .data = 0x50, .delay_ms = 10},
    {.addr = 0x20, .data = 0x59, .delay_ms = 100},
    {.addr = 0x05, .data = 0x07, .delay_ms = 0},
    {.addr = 0x06, .data = 0x0f, .delay_ms = 0},
};

static CVI_U8 app_ipcam_Panel_Ms7024CmdData_Get(const PANEL_TYPE_E enPanelType, CVI_U8 u8Addr, CVI_U8 u8Data)
{
    // What changed: Override MS7024 timing registers per output mode.
    // Previous behavior: The fixed 720x480@60 I2C sequence was used for every MS7024 panel type.
    // Impact: 480P30, 576P25, and 576P50 keep the same write order while using mode-specific values.
    switch (enPanelType) {
    case PANEL_BT656_MS7024_720x480_30:
        switch (u8Addr) {
        case 0x04:
            return 0x1a;
        case 0x0e:
            return 0x56;
        case 0x21:
            return 0x08;
        default:
            return u8Data;
        }
    case PANEL_BT656_MS7024_720x576_25:
        switch (u8Addr) {
        case 0x04:
            return 0x1a;
        case 0x0e:
            return 0x56;
        case 0x31:
            return 0x53;
        case 0x33:
            return 0x79;
        case 0x35:
            return 0x6d;
        case 0x37:
            return 0x2d;
        case 0x21:
            return 0x08;
        case 0x60:
            return 0xc3;
        default:
            return u8Data;
        }
    case PANEL_BT656_MS7024_720x576_50:
        switch (u8Addr) {
        case 0x31:
            return 0x53;
        case 0x33:
            return 0x79;
        case 0x35:
            return 0x6d;
        case 0x37:
            return 0x2d;
        case 0x60:
            return 0xc3;
        default:
            return u8Data;
        }
    case PANEL_BT656_MS7024_720x480_60:
    default:
        return u8Data;
    }
}

CVI_S32 app_ipcam_Panel_I2c_Init(VO_DEV VoDev, const PANEL_I2C_CFG_T* const pstI2cCfg)
{
    char dev_file[16] = {0};

    _NULL_POINTER_CHECK_(pstI2cCfg, CVI_FAILURE);

    if (g_panel_i2c_fd[VoDev] >= 0) {
        return CVI_SUCCESS;
    }

    snprintf(dev_file, sizeof(dev_file), "/dev/i2c-%u", pstI2cCfg->s32I2cDev);
    g_panel_i2c_fd[VoDev] = open(dev_file, O_RDWR);
    if (g_panel_i2c_fd[VoDev] < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Open %s error: %s\n", dev_file, strerror(errno));
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Panel_I2c_Exit(VO_DEV VoDev)
{
    if (g_panel_i2c_fd[VoDev] >= 0) {
        close(g_panel_i2c_fd[VoDev]);
        g_panel_i2c_fd[VoDev] = -1;
        return CVI_SUCCESS;
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Panel_I2c_WriteReg(VO_DEV VoDev, CVI_U8 u8Addr, CVI_U8 u8Data,
    const PANEL_I2C_CFG_T* const pstI2cCfg)
{
    CVI_U8 idx = 0;
    CVI_U8 buf[8];
    int ret;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    _NULL_POINTER_CHECK_(pstI2cCfg, CVI_FAILURE);

    messages[0].addr = pstI2cCfg->s32I2cAddr;
    messages[0].len = 2;
    messages[0].flags = 0;

    buf[idx++] = u8Addr & 0xff;
    buf[idx] = u8Data & 0xff;

    messages[0].buf = buf;
    packets.msgs = messages;
    packets.nmsgs = 1;

    ret = ioctl(g_panel_i2c_fd[VoDev], I2C_RDWR, &packets);
    if (ret < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "I2C_WRITE error\n");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Panel_I2c_SendInit(VO_DEV VoDev, const PANEL_TYPE_E enPanelType,
    const PANEL_I2C_CFG_T* const pstI2cCfg)
{
    CVI_S32 ret = CVI_SUCCESS;

    _NULL_POINTER_CHECK_(pstI2cCfg, CVI_FAILURE);

    switch (enPanelType) {
    case PANEL_BT656_MS7024_720x480_60:
    case PANEL_BT656_MS7024_720x480_30:
    case PANEL_BT656_MS7024_720x576_25:
    case PANEL_BT656_MS7024_720x576_50:
        for (CVI_U32 i = 0; i < ARRAY_SIZE(g_ms7024_i2c_init_cmds); i++) {
            CVI_U8 u8Data = app_ipcam_Panel_Ms7024CmdData_Get(enPanelType,
                g_ms7024_i2c_init_cmds[i].addr, g_ms7024_i2c_init_cmds[i].data);

            ret = app_ipcam_Panel_I2c_WriteReg(VoDev, g_ms7024_i2c_init_cmds[i].addr, u8Data, pstI2cCfg);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "i2c_write failed addr=0x%x\n", g_ms7024_i2c_init_cmds[i].addr);
                return ret;
            }
            if (g_ms7024_i2c_init_cmds[i].delay_ms) {
                usleep(g_ms7024_i2c_init_cmds[i].delay_ms * 1000);
            }
        }
        break;
    default:
        // 其他面板初始化序列预留
        break;
    }

    return CVI_SUCCESS;
}
