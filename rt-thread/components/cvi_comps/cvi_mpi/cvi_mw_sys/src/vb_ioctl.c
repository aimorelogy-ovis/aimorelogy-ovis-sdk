#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#include "cvi_debug.h"
#include "cvi_base.h"
#include "vb_uapi.h"

#define VB_CTRL_PTR(_cfg, _ioctl)                              \
    do {                                                       \
        struct vb_ext_control ec1;                             \
        int ret;                                               \
        memset(&ec1, 0, sizeof(ec1));                          \
        ec1.id = _ioctl;                                       \
        ec1.ptr = (void *)_cfg;                                \
        ret = vb_ctrl(&ec1);                                   \
        if (ret < 0)                                           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "IOCTL_VB_CMD - NG\n"); \
        return ret;                                            \
    } while (0)

#define VB_CTRL_S_VALUE(_cfg, _ioctl)                          \
    do {                                                       \
        struct vb_ext_control ec1;                             \
        int ret;                                               \
        memset(&ec1, 0, sizeof(ec1));                          \
        ec1.id = _ioctl;                                       \
        ec1.value = _cfg;                                      \
        ret = vb_ctrl(&ec1);                                   \
        if (ret < 0)                                           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "IOCTL_VB_CMD - NG\n"); \
        return ret;                                            \
    } while (0)

#define VB_CTRL_S_VALUE64(_cfg, _ioctl)                        \
    do {                                                       \
        struct vb_ext_control ec1;                             \
        int ret;                                               \
        memset(&ec1, 0, sizeof(ec1));                          \
        ec1.id = _ioctl;                                       \
        ec1.value64 = _cfg;                                    \
        ret = vb_ctrl(&ec1);                                   \
        if (ret < 0)                                           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "IOCTL_VB_CMD - NG\n"); \
        return ret;                                            \
    } while (0)

#define VB_CTRL_G_VALUE(_out, _ioctl)                          \
    do {                                                       \
        struct vb_ext_control ec1;                             \
        int ret;                                               \
        memset(&ec1, 0, sizeof(ec1));                          \
        ec1.id = _ioctl;                                       \
        ec1.value = 0;                                         \
        ret = vb_ctrl(&ec1);                                   \
        if (ret < 0)                                           \
            CVI_TRACE_SYS(CVI_DBG_ERR, "IOCTL_VB_CMD - NG\n"); \
        *_out = ec1.value;                                     \
        return ret;                                            \
    } while (0)

int vb_ioctl_set_config(struct cvi_vb_cfg *cfg)
{
    VB_CTRL_PTR(cfg, VB_IOCTL_SET_CONFIG);
}

int vb_ioctl_get_config(struct cvi_vb_cfg *cfg)
{
    VB_CTRL_PTR(cfg, VB_IOCTL_GET_CONFIG);
}

int vb_ioctl_init(struct cvi_vb_cfg *cfg)
{
    VB_CTRL_PTR(cfg, VB_IOCTL_INIT);
}

int vb_ioctl_exit()
{
    VB_CTRL_PTR(NULL, VB_IOCTL_EXIT);
}

int vb_ioctl_create_pool(struct cvi_vb_pool_cfg *cfg)
{
    VB_CTRL_PTR(cfg, VB_IOCTL_CREATE_POOL);
}

int vb_ioctl_destroy_pool(VB_POOL poolId)
{
    VB_CTRL_S_VALUE(poolId, VB_IOCTL_DESTROY_POOL);
}

int vb_ioctl_phys_to_handle(struct cvi_vb_blk_info *blk_info)
{
    VB_CTRL_PTR(blk_info, VB_IOCTL_PHYS_TO_HANDLE);
}

int vb_ioctl_get_blk_info(struct cvi_vb_blk_info *blk_info)
{
    VB_CTRL_PTR(blk_info, VB_IOCTL_GET_BLK_INFO);
}

int vb_ioctl_get_block(struct cvi_vb_blk_cfg *blk_cfg)
{
    VB_CTRL_PTR(blk_cfg, VB_IOCTL_GET_BLOCK);
}

int vb_ioctl_release_block(VB_BLK blk)
{
    VB_CTRL_S_VALUE64(blk, VB_IOCTL_RELEASE_BLOCK);
}

int vb_ioctl_get_pool_max_cnt(CVI_U32 *vb_max_pools)
{
    VB_CTRL_G_VALUE(vb_max_pools, VB_IOCTL_GET_POOL_MAX_CNT);
}

int vb_ioctl_print_pool(VB_POOL poolId)
{
    VB_CTRL_S_VALUE(poolId, VB_IOCTL_PRINT_POOL);
}

int vb_ioctl_unit_test(CVI_U32 op)
{
    VB_CTRL_S_VALUE(op, VB_IOCTL_UNIT_TEST);
}
