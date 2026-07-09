#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvi_debug.h"
#include "sys_ioctl.h"
#include <cvi_comm_sys.h>
#include <errno.h>
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>

#define SYS_S_CTRL_VALUE(_cfg, _ioctl)                           \
    do {                                                         \
        struct sys_ext_control ec1;                              \
        memset(&ec1, 0, sizeof(ec1));                            \
        ec1.id = _ioctl;                                         \
        ec1.value = _cfg;                                        \
        if (cvi_sys_ioctl(SYS_IOC_S_CTRL, &ec1) < 0) {           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_S_CTRL - NG\n"); \
            return -1;                                           \
        }                                                        \
        return 0;                                                \
    } while (0)

#define SYS_S_CTRL_VALUE64(_cfg, _ioctl)                         \
    do {                                                         \
        struct sys_ext_control ec1;                              \
        memset(&ec1, 0, sizeof(ec1));                            \
        ec1.id = _ioctl;                                         \
        ec1.value64 = _cfg;                                      \
        if (cvi_sys_ioctl(SYS_IOC_S_CTRL, &ec1) < 0) {           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_S_CTRL - NG\n"); \
            return -1;                                           \
        }                                                        \
        return 0;                                                \
    } while (0)

#define SYS_S_CTRL_PTR(_cfg, _ioctl)                             \
    do {                                                         \
        struct sys_ext_control ec1;                              \
        memset(&ec1, 0, sizeof(ec1));                            \
        ec1.id = _ioctl;                                         \
        ec1.ptr = (void *)_cfg;                                  \
        if (cvi_sys_ioctl(SYS_IOC_S_CTRL, &ec1) < 0) {           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_S_CTRL - NG\n"); \
            return -1;                                           \
        }                                                        \
        return 0;                                                \
    } while (0)

#define SYS_G_CTRL_VALUE(_out, _ioctl)                           \
    do {                                                         \
        struct sys_ext_control ec1;                              \
        memset(&ec1, 0, sizeof(ec1));                            \
        ec1.id = _ioctl;                                         \
        ec1.value = 0;                                           \
        if (cvi_sys_ioctl(SYS_IOC_G_CTRL, &ec1) < 0) {           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_G_CTRL - NG\n"); \
            return -1;                                           \
        }                                                        \
        *_out = ec1.value;                                       \
        return 0;                                                \
    } while (0)

#define SYS_G_CTRL_PTR(_cfg, _ioctl)                             \
    do {                                                         \
        struct sys_ext_control ec1;                              \
        memset(&ec1, 0, sizeof(ec1));                            \
        ec1.id = _ioctl;                                         \
        ec1.ptr = (void *)_cfg;                                  \
        if (cvi_sys_ioctl(SYS_IOC_G_CTRL, &ec1) < 0) {           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_G_CTRL - NG\n"); \
            return -1;                                           \
        }                                                        \
        return 0;                                                \
    } while (0)

int sys_set_vivpssmode(const VI_VPSS_MODE_S *cfg)
{
    SYS_S_CTRL_PTR(cfg, SYS_IOCTL_SET_VIVPSSMODE);
}

int sys_set_vpssmode(VPSS_MODE_E val)
{
    SYS_S_CTRL_VALUE(val, SYS_IOCTL_SET_VPSSMODE);
}

int sys_set_vpssmodeex(const VPSS_MODE_S *cfg)
{
    SYS_S_CTRL_PTR(cfg, SYS_IOCTL_SET_VPSSMODE_EX);
}

int sys_set_sys_init()
{
    SYS_S_CTRL_VALUE(0, SYS_IOCTL_SET_SYS_INIT);
}

int sys_get_vivpssmode(const VI_VPSS_MODE_S *cfg)
{
    SYS_G_CTRL_PTR(cfg, SYS_IOCTL_GET_VIVPSSMODE);
}

int sys_get_vpssmode(VPSS_MODE_E *val)
{
    SYS_G_CTRL_VALUE(val, SYS_IOCTL_GET_VPSSMODE);
}

int sys_get_vpssmodeex(const VPSS_MODE_S *cfg)
{
    SYS_G_CTRL_PTR(cfg, SYS_IOCTL_GET_VPSSMODE_EX);
}

int sys_get_sys_init(CVI_U32 *val)
{
    SYS_G_CTRL_VALUE(val, SYS_IOCTL_GET_SYS_INIT);
}
