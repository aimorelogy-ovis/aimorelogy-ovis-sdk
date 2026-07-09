#ifndef MODULES_VPU_INCLUDE_VI_IOCTL_H_
#define MODULES_VPU_INCLUDE_VI_IOCTL_H_

#include "cvi_comm_vi.h"
#include "vi_isp.h"
#include "vi_uapi.h"

int vi_set_hdr(int fd, CVI_BOOL is_hdr_on);
int vi_get_ip_dump_list(int fd, struct ip_info *ip_info_list);
int vi_get_online2sc(int fd, struct sop_isp_sc_online *online);

int vi_sdk_set_dev_num(int fd, CVI_U32 devNum);
int vi_sdk_get_dev_num(int fd, CVI_U32 *devNum);
int vi_sdk_enable_pagten(int fd, int dev);

int vi_sdk_get_dev_status(int fd, int dev, CVI_BOOL *pbStatus);
int vi_sdk_set_dev_attr(int fd, int dev, VI_DEV_ATTR_S *pstDevAttr);
int vi_sdk_get_dev_attr(int fd, int dev, VI_DEV_ATTR_S *pstDevAttr);
int vi_sdk_set_dev_attr_ex(int fd, int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx);
int vi_sdk_get_dev_attr_ex(int fd, int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx);
int vi_sdk_set_dev_bind_attr(int fd, int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr);
int vi_sdk_get_dev_bind_attr(int fd, int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr);
int vi_sdk_set_dev_unbind_attr(int fd, int dev);
int vi_sdk_get_pipe_status(int fd, int pipe, VI_PIPE_STATUS_S *pstStatus);
int vi_sdk_get_chn_status(int fd, int pipe, int chn, VI_CHN_STATUS_S *pstStatus);
int vi_sdk_enable_dev(int fd, int dev);
int vi_sdk_disable_dev(int fd, int dev);
int vi_sdk_create_pipe(int fd, int pipe, VI_PIPE_ATTR_S *pstPipeAttr);
int vi_sdk_start_pipe(int fd, int pipe);
int vi_sdk_destroy_pipe(int fd, int pipe);
int vi_sdk_set_chn_attr(int fd, int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr);
int vi_sdk_get_chn_attr(int fd, int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr);
int vi_sdk_set_pipe_attr(int fd, int pipe, VI_PIPE_ATTR_S *pstPipeAttr);
int vi_sdk_get_pipe_attr(int fd, int pipe, VI_PIPE_ATTR_S *pstPipeAttr);
int vi_sdk_get_pipe_dump_attr(int fd, int pipe, VI_DUMP_ATTR_S *pstDumpAttr);
int vi_sdk_set_pipe_dump_attr(int fd, int pipe, VI_DUMP_ATTR_S *pstDumpAttr);
int vi_sdk_enable_chn(int fd, int pipe, int chn);
int vi_sdk_disable_chn(int fd, int pipe, int chn);
int vi_sdk_set_motion_lv(int fd, struct mlv_info_s *mlv_i);
int vi_sdk_set_bypass_frm(int fd, CVI_U32 snr_num, CVI_U8 bypass_num);
int vi_sdk_set_pipe_frm_src(int fd, int pipe, VI_PIPE_FRAME_SOURCE_E *source);
int vi_sdk_get_pipe_frm_src(int fd, int pipe, VI_PIPE_FRAME_SOURCE_E *source);
int vi_sdk_send_pipe_raw(int fd, int pipe, VIDEO_FRAME_INFO_S *sVideoFrm);
int vi_sdk_set_dev_timing_attr(int fd, int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr);
int vi_sdk_get_chn_frame(int fd, int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
int vi_sdk_get_dev_timing_attr(int fd, int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr);
int vi_sdk_release_chn_frame(int fd, int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo);
int vi_sdk_set_chn_crop(int fd, int pipe, int chn, VI_CROP_INFO_S *pstCropInfo);
int vi_sdk_get_chn_crop(int fd, int pipe, int chn, VI_CROP_INFO_S *pstCropInfo);
int vi_sdk_set_pipe_crop(int fd, int pipe, CROP_INFO_S *pstCropInfo);
int vi_sdk_get_pipe_crop(int fd, int pipe, CROP_INFO_S *pstCropInfo);
int vi_sdk_get_pipe_frame(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
int vi_sdk_release_pipe_frame(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo);
int vi_sdk_start_smooth_rawdump(int fd, int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param);
int vi_sdk_stop_smooth_rawdump(int fd, int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param);
int vi_sdk_get_smooth_rawdump(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
int vi_sdk_put_smooth_rawdump(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo);
int vi_sdk_set_chn_rotation(int fd, struct vi_chn_rot_cfg *cfg);
int vi_sdk_get_chn_rotation(int fd, struct vi_chn_rot_cfg *cfg);
int vi_sdk_set_chn_ldc(int fd, struct vi_chn_ldc_cfg *cfg);
int vi_sdk_get_chn_ldc(int fd, int pipe, int chn, struct vi_chn_ldc_cfg *cfg);
int vi_sdk_set_chn_flip_mirror(int fd, struct vi_chn_flip_mirror_cfg *cfg);
int vi_sdk_get_chn_flip_mirror(int fd, struct vi_chn_flip_mirror_cfg *cfg);
int vi_sdk_attach_vbpool(int fd, struct vi_vb_pool_cfg *cfg);
int vi_sdk_detach_vbpool(int fd, struct vi_vb_pool_cfg *cfg);
int vi_sdk_dump_register(int fd, int pipe, struct ip_info *ip_info);
int vi_sdk_set_dev_rx_frame_count(int fd, int dev, uint32_t count);
int vi_sdk_get_dev_rx_frame_count(int fd, int dev, uint32_t *count);

int vi_sdk_set_ai_isp_cfg(int fd, VI_AI_ISP_CFG_S *pstAiIspCfg);
int vi_sdk_get_ai_isp_raw(int fd, VI_AI_ISP_INFO_WRAP_S *pstAiIspInfoWrap);
int vi_sdk_put_ai_isp_raw(int fd, VI_AI_ISP_INFO_WRAP_S *pstAiIspInfoWrap);

#endif // MODULES_VPU_INCLUDE_VI_IOCTL_H_
