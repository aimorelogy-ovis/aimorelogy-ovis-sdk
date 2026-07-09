#include "vi_ioctl.h"

#include <assert.h>
#include <cvi_comm_video.h>
#include <cvi_comm_vo.h>
#include <errno.h>
#include <fcntl.h> /* low-level i/o */
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
// #include "rtos_types.h"
#include <cvi_debug.h>
#include <vi_uapi.h>

#define S_CTRL_VALUE(_cfg, _ioctl)                                                               \
    do {                                                                                         \
        struct vi_ext_control ec1;                                                               \
        memset(&ec1, 0, sizeof(ec1));                                                            \
        ec1.id    = _ioctl;                                                                      \
        ec1.value = _cfg;                                                                        \
        if (vi_ioctl(VI_IOC_S_CTRL, &ec1) < 0) {                                                 \
            CVI_TRACE_VI(CVI_DBG_ERR, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno)); \
            return -1;                                                                           \
        }                                                                                        \
        return 0;                                                                                \
    } while (0)

#define S_CTRL_VALUE64(_cfg, _ioctl)                                                             \
    do {                                                                                         \
        struct vi_ext_control ec1;                                                               \
        memset(&ec1, 0, sizeof(ec1));                                                            \
        ec1.id      = _ioctl;                                                                    \
        ec1.value64 = _cfg;                                                                      \
        if (vi_ioctl(VI_IOC_S_CTRL, &ec1) < 0) {                                                 \
            CVI_TRACE_VI(CVI_DBG_ERR, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno)); \
            return -1;                                                                           \
        }                                                                                        \
        return 0;                                                                                \
    } while (0)

#define S_CTRL_PTR(_cfg, _ioctl)                                                                 \
    do {                                                                                         \
        struct vi_ext_control ec1;                                                               \
        memset(&ec1, 0, sizeof(ec1));                                                            \
        ec1.id  = _ioctl;                                                                        \
        ec1.ptr = (void *)_cfg;                                                                  \
        if (vi_ioctl(VI_IOC_S_CTRL, &ec1) < 0) {                                                 \
            CVI_TRACE_VI(CVI_DBG_ERR, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno)); \
            return -1;                                                                           \
        }                                                                                        \
        return 0;                                                                                \
    } while (0)

#define G_CTRL_VALUE(_out, _ioctl)                                                               \
    do {                                                                                         \
        struct vi_ext_control ec1;                                                               \
        memset(&ec1, 0, sizeof(ec1));                                                            \
        ec1.id    = _ioctl;                                                                      \
        ec1.value = 0;                                                                           \
        if (vi_ioctl(VI_IOC_G_CTRL, &ec1) < 0) {                                                 \
            CVI_TRACE_VI(CVI_DBG_ERR, "VI_IOC_G_CTRL - %s NG, %s\n", __func__, strerror(errno)); \
            return -1;                                                                           \
        }                                                                                        \
        *_out = ec1.value;                                                                       \
        return 0;                                                                                \
    } while (0)

#define G_CTRL_PTR(_cfg, _ioctl)                                                                 \
    do {                                                                                         \
        struct vi_ext_control ec1;                                                               \
        memset(&ec1, 0, sizeof(ec1));                                                            \
        ec1.id  = _ioctl;                                                                        \
        ec1.ptr = (void *)_cfg;                                                                  \
        if (vi_ioctl(VI_IOC_G_CTRL, &ec1) < 0) {                                                 \
            CVI_TRACE_VI(CVI_DBG_ERR, "VI_IOC_G_CTRL - %s NG, %s\n", __func__, strerror(errno)); \
            return -1;                                                                           \
        }                                                                                        \
        return 0;                                                                                \
    } while (0)

#define SDK_CTRL_SET_CFG(_cfg, _ioctl, _dev, _pipe, _chn, _val)                    \
    do {                                                                           \
        struct vi_ext_control ec1;                                                 \
        memset(&ec1, 0, sizeof(ec1));                                              \
        ec1.id           = VI_IOCTL_SDK_CTRL;                                      \
        ec1.sdk_id       = _ioctl;                                                 \
        ec1.sdk_cfg.dev  = _dev;                                                   \
        ec1.sdk_cfg.pipe = _pipe;                                                  \
        ec1.sdk_cfg.chn  = _chn;                                                   \
        ec1.sdk_cfg.val  = _val;                                                   \
        ec1.sdk_cfg.ptr  = (void *)_cfg;                                           \
        if (vi_ioctl(VI_IOC_S_CTRL, &ec1) < 0) {                                   \
            CVI_TRACE_VI(CVI_DBG_ERR, "VI_SDK_IOC_S_CTRL - %s NG, %s\n", __func__, \
                         strerror(errno));                                         \
            return -1;                                                             \
        }                                                                          \
        return 0;                                                                  \
    } while (0)

int vi_enable_usr_pic(int fd, bool enable)
{
    S_CTRL_VALUE(enable, VI_IOCTL_USR_PIC_ONOFF);
}

int vi_set_usr_pic(int fd, struct cvi_isp_usr_pic_cfg *cfg)
{
    S_CTRL_PTR(cfg, VI_IOCTL_USR_PIC_CFG);
}

int vi_put_usr_pic(int fd, CVI_U64 phyAddr)
{
    S_CTRL_VALUE64(phyAddr, VI_IOCTL_USR_PIC_PUT);
}

int vi_set_usr_pic_timing(int fd, CVI_U32 fps)
{
    S_CTRL_VALUE(fps, VI_IOCTL_USR_PIC_TIMING);
}

int vi_set_be_online(int fd, CVI_BOOL online)
{
    S_CTRL_VALUE(online, VI_IOCTL_BE_ONLINE);
}

int vi_set_online(int fd, CVI_BOOL online)
{
    S_CTRL_VALUE(online, VI_IOCTL_ONLINE);
}

int vi_set_hdr(int fd, CVI_BOOL is_hdr_on)
{
    S_CTRL_VALUE(is_hdr_on, VI_IOCTL_HDR);
}

int vi_set_3dnr(int fd, CVI_BOOL is_3dnr_on)
{
    S_CTRL_VALUE(is_3dnr_on, VI_IOCTL_3DNR);
}

int vi_get_pipe_dump(int fd, struct cvi_vip_isp_raw_blk *raw_dump)
{
    G_CTRL_PTR(raw_dump, VI_IOCTL_GET_PIPE_DUMP);
}

int vi_put_pipe_dump(int fd, CVI_U32 dev_num)
{
    S_CTRL_VALUE(dev_num, VI_IOCTL_PUT_PIPE_DUMP);
}

int vi_set_yuv_bypass_path(int fd, struct cvi_vip_isp_yuv_param *param)
{
    S_CTRL_PTR(param, VI_IOCTL_YUV_BYPASS_PATH);
}

int vi_set_compress_mode(int fd, CVI_BOOL is_compress_on)
{
    S_CTRL_VALUE(is_compress_on, VI_IOCTL_COMPRESS_EN);
}

int vi_set_lvds_flow(int fd, CVI_BOOL is_lvds_flow_on)
{
    S_CTRL_VALUE(is_lvds_flow_on, VI_IOCTL_SUBLVDS_PATH);
}

int vi_set_clk(int fd, CVI_BOOL clk_on)
{
    S_CTRL_VALUE(clk_on, VI_IOCTL_CLK_CTRL);
}

#ifdef ARCH_CV182X
int vi_set_rgbir(int fd, CVI_BOOL is_rgbir)
{
    S_CTRL_VALUE(is_rgbir, VI_IOCTL_RGBIR);
}
#endif

int vi_get_ip_dump_list(int fd, struct ip_info *ip_info_list)
{
    G_CTRL_PTR(ip_info_list, VI_IOCTL_GET_IP_INFO);
}

int vi_set_trig_preraw(int fd, CVI_U32 dev_num)
{
    S_CTRL_VALUE(dev_num, VI_IOCTL_TRIG_PRERAW);
}

int vi_set_online2sc(int fd, struct cvi_isp_sc_online *online)
{
    S_CTRL_PTR(online, VI_IOCTL_SC_ONLINE);
}

int vi_get_tun_addr(int fd, struct isp_tuning_cfg *tun_buf_info)
{
    G_CTRL_PTR(tun_buf_info, VI_IOCTL_GET_TUN_ADDR);
}

int vi_get_dma_size(int fd, CVI_U32 *size)
{
    G_CTRL_VALUE(size, VI_IOCTL_GET_BUF_SIZE);
}

int vi_set_dma_buf_info(int fd, struct cvi_vi_dma_buf_info *param)
{
    S_CTRL_PTR(param, VI_IOCTL_SET_DMA_BUF_INFO);
}

int vi_set_enq_buf(int fd, struct vi_buffer *buf)
{
    S_CTRL_PTR(buf, VI_IOCTL_ENQ_BUF);
}

int vi_set_start_streaming(int fd)
{
    S_CTRL_VALUE(1, VI_IOCTL_START_STREAMING);
}

int vi_set_stop_streaming(int fd)
{
    S_CTRL_VALUE(1, VI_IOCTL_STOP_STREAMING);
}

int vi_set_tuning_dis(int fd, int *ptr)
{
    S_CTRL_PTR(ptr, VI_IOCTL_TUNING_PARAM);
}

int vi_sdk_set_dev_attr(int fd, int dev, VI_DEV_ATTR_S *pstDevAttr)
{
    SDK_CTRL_SET_CFG(pstDevAttr, VI_SDK_SET_DEVATTR, dev, -1, -1, -1);
}

int vi_sdk_get_dev_attr(int fd, int dev, VI_DEV_ATTR_S *pstDevAttr)
{
    SDK_CTRL_SET_CFG(pstDevAttr, VI_SDK_GET_DEVATTR, dev, -1, -1, -1);
}

int vi_sdk_enable_dev(int fd, int dev)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_ENABLE_DEV, dev, -1, -1, -1);
}

int vi_sdk_create_pipe(int fd, int pipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
    SDK_CTRL_SET_CFG(pstPipeAttr, VI_SDK_CREATE_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_start_pipe(int fd, int pipe)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_START_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_stop_pipe(int fd, int pipe)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_STOP_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_set_chn_attr(int fd, int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr)
{
    SDK_CTRL_SET_CFG(pstChnAttr, VI_SDK_SET_CHNATTR, -1, pipe, chn, -1);
}

int vi_sdk_enable_chn(int fd, int pipe, int chn)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_ENABLE_CHN, -1, pipe, chn, -1);
}

int vi_sdk_disable_chn(int fd, int pipe, int chn)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_DISABLE_CHN, -1, pipe, chn, -1);
}

int vi_sdk_set_motion_lv(int fd, struct mlv_info_s *pmlv_i)
{
    SDK_CTRL_SET_CFG(pmlv_i, VI_SDK_SET_MOTION_LV, -1, -1, -1, -1);
}

int vi_sdk_enable_dis(int fd, int pipe)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_ENABLE_DIS, -1, pipe, -1, -1);
}

int vi_sdk_disable_dis(int fd, int pipe)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_DISABLE_DIS, -1, pipe, -1, -1);
}

int vi_sdk_set_dis_info(int fd, struct dis_info_s *pdis_i)
{
    SDK_CTRL_SET_CFG(pdis_i, VI_SDK_SET_DIS_INFO, -1, -1, -1, -1);
}

int vi_sdk_set_dev_timing_attr(int fd, int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr)
{
    SDK_CTRL_SET_CFG(pstDevTimingAttr, VI_SDK_SET_DEV_TIMING_ATTR, dev, -1, -1, -1);
}

int vi_sdk_set_pipe_frm_src(int fd, int pipe, VI_PIPE_FRAME_SOURCE_E *source)
{
    SDK_CTRL_SET_CFG(source, VI_SDK_SET_PIPE_FRM_SRC, -1, pipe, -1, -1);
}

int vi_sdk_send_pipe_raw(int fd, int pipe, VIDEO_FRAME_INFO_S *sVideoFrm)
{
    SDK_CTRL_SET_CFG(sVideoFrm, VI_SDK_SEND_PIPE_RAW, -1, pipe, -1, -1);
}

int vi_sdk_get_chn_frame(int fd, int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo,
                         CVI_S32 s32MilliSec)
{
    SDK_CTRL_SET_CFG(pstFrameInfo, VI_SDK_GET_CHN_FRAME, -1, pipe, chn, s32MilliSec);
}

int vi_sdk_release_chn_frame(int fd, int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    SDK_CTRL_SET_CFG(pstFrameInfo, VI_SDK_RELEASE_CHN_FRAME, -1, pipe, chn, -1);
}

int vi_sdk_set_chn_crop(int fd, int pipe, int chn, VI_CROP_INFO_S *pstCropInfo)
{
    SDK_CTRL_SET_CFG(pstCropInfo, VI_SDK_SET_CHN_CROP, -1, pipe, chn, -1);
}

int vi_sdk_get_chn_crop(int fd, int pipe, int chn, VI_CROP_INFO_S *pstCropInfo)
{
    SDK_CTRL_SET_CFG(pstCropInfo, VI_SDK_GET_CHN_CROP, -1, pipe, chn, -1);
}

int vi_sdk_get_pipe_frame(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
    SDK_CTRL_SET_CFG(pstFrameInfo, VI_SDK_GET_PIPE_FRAME, -1, pipe, -1, s32MilliSec);
}

int vi_sdk_release_pipe_frame(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    SDK_CTRL_SET_CFG(pstFrameInfo, VI_SDK_RELEASE_PIPE_FRAME, -1, pipe, -1, -1);
}

int vi_sdk_start_smooth_rawdump(int fd, int pipe,
                                struct cvi_vip_isp_smooth_raw_param *smooth_raw_param)
{
    SDK_CTRL_SET_CFG(smooth_raw_param, VI_SDK_START_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_stop_smooth_rawdump(int fd, int pipe,
                               struct cvi_vip_isp_smooth_raw_param *smooth_raw_param)
{
    SDK_CTRL_SET_CFG(smooth_raw_param, VI_SDK_STOP_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_get_smooth_rawdump(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo,
                              CVI_S32 s32MilliSec)
{
    SDK_CTRL_SET_CFG(pstFrameInfo, VI_SDK_GET_SMOOTH_RAWDUMP, -1, pipe, -1, s32MilliSec);
}

int vi_sdk_put_smooth_rawdump(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    SDK_CTRL_SET_CFG(pstFrameInfo, VI_SDK_PUT_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_set_chn_rotation(int fd, const struct vi_chn_rot_cfg *cfg)
{
    SDK_CTRL_SET_CFG(cfg, VI_SDK_SET_CHN_ROTATION, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_set_chn_ldc(int fd, const struct vi_chn_ldc_cfg *cfg)
{
    SDK_CTRL_SET_CFG(cfg, VI_SDK_SET_CHN_LDC, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_enable_pattern(int fd, int pattern)
{
    S_CTRL_VALUE(pattern, VI_IOCTL_ENABLE_PATTERN);
}

int vi_sdk_reg_sync_task(int fd, int pipe, void *sync_task)
{
    SDK_CTRL_SET_CFG(sync_task, VI_SDK_REG_SYNC_TASK, -1, pipe, -1, -1);
}

int vi_sdk_unreg_sync_task(int fd, int pipe, void *sync_task)
{
    SDK_CTRL_SET_CFG(sync_task, VI_SDK_UNREG_SYNC_TASK, -1, pipe, -1, -1);
}

int vi_sdk_disable_frame_valid(int fd, int dev)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_DISABLE_FRAME_VALID, dev, -1, -1, -1);
}

int vi_sdk_enable_frame_valid(int fd, int dev)
{
    SDK_CTRL_SET_CFG(NULL, VI_SDK_ENABLE_FRAME_VALID, dev, -1, -1, -1);
}

int vi_sdk_attach_vbpool(int fd, const struct vi_vb_pool_cfg *cfg)
{
    SDK_CTRL_SET_CFG(cfg, VI_SDK_ATTACH_VB_POOL, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_detach_vbpool(int fd, const struct vi_vb_pool_cfg *cfg)
{
    SDK_CTRL_SET_CFG(cfg, VI_SDK_DETACH_VB_POOL, -1, -1, cfg->ViChn, -1);
}