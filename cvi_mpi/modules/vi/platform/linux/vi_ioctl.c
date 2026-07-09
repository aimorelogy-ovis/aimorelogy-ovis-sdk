#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <cvi_comm_video.h>
#include "vi_ioctl.h"
#include "vi_uapi.h"

static inline int S_CTRL_VALUE(int fd, int cfg, enum VI_IOCTL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.value = cfg;

	rc = ioctl(fd, VI_IOC_S_CTRL, &ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

static inline int G_CTRL_PTR(int fd, void *cfg, int size, enum VI_IOCTL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = cfg;
	ec1.size = size;

	rc = ioctl(fd, VI_IOC_G_CTRL, &ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_G_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

static inline int S_CTRL_PTR(int fd, void *cfg, int size, enum VI_IOCTL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = cfg;
	ec1.size = size;

	rc = ioctl(fd, VI_IOC_S_CTRL, &ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

static inline int SDK_CTRL_SET_VALUE(int fd, enum VI_SDK_CTRL _ioctl
					, int dev, int pipe, int chn, int val)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.sdk_cfg.dev = dev;
	ec1.sdk_cfg.pipe = pipe;
	ec1.sdk_cfg.chn = chn;
	ec1.sdk_cfg.value = val;

	rc = ioctl(fd, VI_IOC_SDK_CTRL, &ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

static inline int SDK_CTRL_GET_VALUE(int fd, uint32_t *out, enum VI_SDK_CTRL _ioctl)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.value = 0;

	rc = ioctl(fd, VI_IOC_SDK_CTRL, &ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_SDK_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	*out = ec1.value;
	return rc;
}

static inline int SDK_CTRL_SET_CFG(int fd, void *cfg, int size, enum VI_SDK_CTRL _ioctl
					, int dev, int pipe, int chn, int val)
{
	struct vi_ext_control ec1;
	int rc = 0;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.sdk_cfg.dev = dev;
	ec1.sdk_cfg.pipe = pipe;
	ec1.sdk_cfg.chn = chn;
	ec1.sdk_cfg.ptr = cfg;
	ec1.sdk_cfg.size = size;
	ec1.sdk_cfg.reserved[0] = val;

	rc = ioctl(fd, VI_IOC_SDK_CTRL, &ec1);
	if (rc < 0) {
		fprintf(stderr, "VI_SDK_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));
	}
	return rc;
}

int vi_set_hdr(int fd, CVI_BOOL is_hdr_on)
{
	return S_CTRL_VALUE(fd, is_hdr_on, VI_IOCTL_HDR);
}

int vi_get_online2sc(int fd, struct sop_isp_sc_online *online)
{
	return G_CTRL_PTR(fd, online, sizeof(struct sop_isp_sc_online), VI_IOCTL_GET_SC_ONLINE);
}

int vi_sdk_get_dev_num(int fd, CVI_U32 *devNum)
{
	return SDK_CTRL_GET_VALUE(fd, devNum, VI_SDK_GET_DEV_NUM);
}

int vi_sdk_enable_pagten(int fd, int dev)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_ENABLE_PATGEN, dev, -1, -1, -1);
}

int vi_sdk_set_dev_attr(int fd, int dev, VI_DEV_ATTR_S *pstDevAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDevAttr, sizeof(VI_DEV_ATTR_S), VI_SDK_SET_DEV_ATTR, dev, -1, -1, -1);
}

int vi_sdk_get_dev_attr(int fd, int dev, VI_DEV_ATTR_S *pstDevAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDevAttr, sizeof(VI_DEV_ATTR_S), VI_SDK_GET_DEV_ATTR, dev, -1, -1, -1);
}

int vi_sdk_set_dev_attr_ex(int fd, int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	return SDK_CTRL_SET_CFG(fd, pstDevAttrEx, sizeof(VI_DEV_ATTR_EX_S), VI_SDK_SET_DEV_ATTR_EX, dev, -1, -1, -1);
}

int vi_sdk_get_dev_attr_ex(int fd, int dev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	return SDK_CTRL_SET_CFG(fd, pstDevAttrEx, sizeof(VI_DEV_ATTR_EX_S), VI_SDK_GET_DEV_ATTR_EX, dev, -1, -1, -1);
}

int vi_sdk_set_dev_bind_attr(int fd, int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDevBindAttr, sizeof(VI_DEV_BIND_PIPE_S), VI_SDK_SET_DEV_BIND_ATTR, dev, -1, -1,
				-1);
}

int vi_sdk_get_dev_bind_attr(int fd, int dev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDevBindAttr, sizeof(VI_DEV_BIND_PIPE_S), VI_SDK_GET_DEV_BIND_ATTR, dev, -1, -1,
				-1);
}

int vi_sdk_set_dev_unbind_attr(int fd, int dev)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_SET_DEV_UNBIND_ATTR, dev, -1, -1, -1);
}

int vi_sdk_enable_dev(int fd, int dev)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_ENABLE_DEV, dev, -1, -1, -1);
}

int vi_sdk_disable_dev(int fd, int dev)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_DISABLE_DEV, dev, -1, -1, -1);
}

int vi_sdk_create_pipe(int fd, int pipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstPipeAttr, sizeof(VI_PIPE_ATTR_S), VI_SDK_CREATE_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_destroy_pipe(int fd, int pipe)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_DESTROY_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_attr(int fd, int pipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstPipeAttr, sizeof(VI_PIPE_ATTR_S), VI_SDK_GET_PIPE_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_set_pipe_attr(int fd, int pipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstPipeAttr, sizeof(VI_PIPE_ATTR_S), VI_SDK_SET_PIPE_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_start_pipe(int fd, int pipe)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_START_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_stop_pipe(int fd, int pipe)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_STOP_PIPE, -1, pipe, -1, -1);
}

int vi_sdk_set_chn_attr(int fd, int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstChnAttr, sizeof(VI_CHN_ATTR_S), VI_SDK_SET_CHN_ATTR, -1, pipe, chn, -1);
}

int vi_sdk_get_chn_attr(int fd, int pipe, int chn, VI_CHN_ATTR_S *pstChnAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstChnAttr, sizeof(VI_CHN_ATTR_S), VI_SDK_GET_CHN_ATTR, -1, pipe, chn, -1);
}

int vi_sdk_enable_chn(int fd, int pipe, int chn)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_ENABLE_CHN, -1, pipe, chn, -1);
}

int vi_sdk_disable_chn(int fd, int pipe, int chn)
{
	return SDK_CTRL_SET_CFG(fd, NULL, 0, VI_SDK_DISABLE_CHN, -1, pipe, chn, -1);
}

int vi_sdk_set_motion_lv(int fd, struct mlv_info_s *pmlv_i)
{
	return SDK_CTRL_SET_CFG(fd, pmlv_i, sizeof(struct mlv_info_s), VI_SDK_SET_MOTION_LV, -1, -1, -1, -1);
}

int vi_sdk_set_bypass_frm(int fd, CVI_U32 snr_num, CVI_U8 bypass_num)
{
	return SDK_CTRL_SET_VALUE(fd, VI_SDK_SET_BYPASS_FRM, -1, snr_num, -1, bypass_num);
}

int vi_sdk_set_pipe_frm_src(int fd, int pipe, VI_PIPE_FRAME_SOURCE_E *source)
{
	return SDK_CTRL_SET_CFG(fd, source, sizeof(VI_PIPE_FRAME_SOURCE_E), VI_SDK_SET_PIPE_FRM_SRC, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_frm_src(int fd, int pipe, VI_PIPE_FRAME_SOURCE_E *source)
{
	return SDK_CTRL_SET_CFG(fd, source, sizeof(VI_PIPE_FRAME_SOURCE_E), VI_SDK_GET_PIPE_FRM_SRC, -1, pipe, -1, -1);
}

int vi_sdk_set_dev_timing_attr(int fd, int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDevTimingAttr, sizeof(VI_DEV_TIMING_ATTR_S), VI_SDK_SET_DEV_TIMING_ATTR, dev, -1,
				-1, -1);
}

int vi_sdk_get_dev_timing_attr(int fd, int dev, VI_DEV_TIMING_ATTR_S *pstDevTimingAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDevTimingAttr, sizeof(VI_DEV_TIMING_ATTR_S), VI_SDK_GET_DEV_TIMING_ATTR, dev, -1,
				-1, -1);
}

int vi_sdk_send_pipe_raw(int fd, int pipe, VIDEO_FRAME_INFO_S *sVideoFrm)
{
	return SDK_CTRL_SET_CFG(fd, sVideoFrm, sizeof(VIDEO_FRAME_INFO_S), VI_SDK_SEND_PIPE_RAW, -1, pipe, -1, -1);
}

int vi_sdk_get_dev_status(int fd, int dev, CVI_BOOL *pbStatus)
{
	return SDK_CTRL_SET_CFG(fd, pbStatus, sizeof(CVI_BOOL), VI_SDK_GET_DEV_STATUS, dev, -1, -1, -1);
}

int vi_sdk_get_pipe_status(int fd, int pipe, VI_PIPE_STATUS_S *pstStatus)
{
	return SDK_CTRL_SET_CFG(fd, pstStatus, sizeof(VI_PIPE_STATUS_S), VI_SDK_GET_PIPE_STATUS, -1, pipe, -1, -1);
}

int vi_sdk_get_chn_status(int fd, int pipe, int chn, VI_CHN_STATUS_S *pstStatus)
{
	return SDK_CTRL_SET_CFG(fd, pstStatus, sizeof(VI_CHN_STATUS_S), VI_SDK_GET_CHN_STATUS, -1, pipe, chn, -1);
}

int vi_sdk_get_chn_frame(int fd, int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return SDK_CTRL_SET_CFG(fd, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S), VI_SDK_GET_CHN_FRAME, -1, pipe, chn,
				s32MilliSec);
}

int vi_sdk_release_chn_frame(int fd, int pipe, int chn, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	return SDK_CTRL_SET_CFG(fd, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S), VI_SDK_RELEASE_CHN_FRAME, -1, pipe, chn,
				-1);
}

int vi_sdk_set_chn_crop(int fd, int pipe, int chn, VI_CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(fd, pstCropInfo, sizeof(VI_CROP_INFO_S), VI_SDK_SET_CHN_CROP, -1, pipe, chn, -1);
}

int vi_sdk_get_chn_crop(int fd, int pipe, int chn, VI_CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(fd, pstCropInfo, sizeof(VI_CROP_INFO_S), VI_SDK_GET_CHN_CROP, -1, pipe, chn, -1);
}

int vi_sdk_set_pipe_crop(int fd, int pipe, CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(fd, pstCropInfo, sizeof(CROP_INFO_S), VI_SDK_SET_PIPE_CROP, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_crop(int fd, int pipe, CROP_INFO_S *pstCropInfo)
{
	return SDK_CTRL_SET_CFG(fd, pstCropInfo, sizeof(CROP_INFO_S), VI_SDK_GET_PIPE_CROP, -1, pipe, -1, -1);
}

int vi_sdk_get_pipe_frame(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return SDK_CTRL_SET_CFG(fd, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2, VI_SDK_GET_PIPE_FRAME, -1, pipe, -1,
				s32MilliSec);
}

int vi_sdk_release_pipe_frame(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	return SDK_CTRL_SET_CFG(fd, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2, VI_SDK_RELEASE_PIPE_FRAME, -1, pipe,
				-1, -1);
}

int vi_sdk_get_pipe_dump_attr(int fd, int pipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDumpAttr, sizeof(VI_DUMP_ATTR_S), VI_SDK_GET_PIPE_DUMP_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_set_pipe_dump_attr(int fd, int pipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	return SDK_CTRL_SET_CFG(fd, pstDumpAttr, sizeof(VI_DUMP_ATTR_S), VI_SDK_SET_PIPE_DUMP_ATTR, -1, pipe, -1, -1);
}

int vi_sdk_start_smooth_rawdump(int fd, int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param)
{
	return SDK_CTRL_SET_CFG(fd, smooth_raw_param, sizeof(struct sop_vip_isp_smooth_raw_param),
				VI_SDK_START_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_stop_smooth_rawdump(int fd, int pipe, struct sop_vip_isp_smooth_raw_param *smooth_raw_param)
{
	return SDK_CTRL_SET_CFG(fd, smooth_raw_param, sizeof(struct sop_vip_isp_smooth_raw_param),
				VI_SDK_STOP_SMOOTH_RAWDUMP, -1, pipe, -1, -1);
}

int vi_sdk_get_smooth_rawdump(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return SDK_CTRL_SET_CFG(fd, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2, VI_SDK_GET_SMOOTH_RAWDUMP, -1, pipe,
				-1, s32MilliSec);
}

int vi_sdk_put_smooth_rawdump(int fd, int pipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	return SDK_CTRL_SET_CFG(fd, pstFrameInfo, sizeof(VIDEO_FRAME_INFO_S) * 2, VI_SDK_PUT_SMOOTH_RAWDUMP, -1, pipe,
				-1, -1);
}

int vi_sdk_set_chn_rotation(int fd, struct vi_chn_rot_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_chn_rot_cfg), VI_SDK_SET_CHN_ROTATION, -1, cfg->ViPipe,
				cfg->ViChn, -1);
}

int vi_sdk_get_chn_rotation(int fd, struct vi_chn_rot_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_chn_rot_cfg), VI_SDK_GET_CHN_ROTATION, -1, cfg->ViPipe,
				cfg->ViChn, -1);
}

int vi_sdk_set_chn_ldc(int fd, struct vi_chn_ldc_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_chn_ldc_cfg), VI_SDK_SET_CHN_LDC, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_get_chn_ldc(int fd, int pipe, int chn, struct vi_chn_ldc_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_chn_ldc_cfg), VI_SDK_GET_CHN_LDC, -1, pipe, chn, -1);
}

int vi_sdk_set_chn_flip_mirror(int fd, struct vi_chn_flip_mirror_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_chn_flip_mirror_cfg), VI_SDK_SET_CHN_FLIP_MIRROR, -1,
				cfg->ViPipe, cfg->ViChn, -1);
}

int vi_sdk_get_chn_flip_mirror(int fd, struct vi_chn_flip_mirror_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_chn_flip_mirror_cfg), VI_SDK_GET_CHN_FLIP_MIRROR, -1,
				cfg->ViPipe, cfg->ViChn, -1);
}

int vi_sdk_attach_vbpool(int fd, struct vi_vb_pool_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_vb_pool_cfg), VI_SDK_ATTACH_VB_POOL, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_detach_vbpool(int fd, struct vi_vb_pool_cfg *cfg)
{
	return SDK_CTRL_SET_CFG(fd, cfg, sizeof(struct vi_vb_pool_cfg), VI_SDK_DETACH_VB_POOL, -1, -1, cfg->ViChn, -1);
}

int vi_sdk_dump_register(int fd, int pipe, struct ip_info *ip_info)
{
	return SDK_CTRL_SET_CFG(fd, ip_info, sizeof(struct ip_info), VI_SDK_DUMP_REGISTER, -1, pipe, -1, -1);
}

int vi_sdk_set_dev_rx_frame_count(int fd, int dev, uint32_t count)
{
	return SDK_CTRL_SET_CFG(fd, &count, sizeof(uint32_t), VI_SDK_SET_DEV_RX_FRAME_COUNT, dev, -1, -1, -1);
}

int vi_sdk_get_dev_rx_frame_count(int fd, int dev, uint32_t *count)
{
	return SDK_CTRL_SET_CFG(fd, count, sizeof(uint32_t), VI_SDK_GET_DEV_RX_FRAME_COUNT, dev, -1, -1, -1);
}

int vi_sdk_set_ai_isp_cfg(int fd, VI_AI_ISP_CFG_S *pstAiIspCfg)
{
	struct vi_ai_isp_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.ViPipe = pstAiIspCfg->viPipe;
	cfg.ViAiISPType = (CVI_U8)pstAiIspCfg->enAiIspType;
	cfg.Reserved[0] = pstAiIspCfg->reserved[0];
	cfg.Reserved[1] = pstAiIspCfg->reserved[1];

	return S_CTRL_PTR(fd, &cfg, sizeof(struct vi_ai_isp_cfg), VI_IOCTL_AI_ISP_CFG);
}

int vi_sdk_get_ai_isp_raw(int fd, VI_AI_ISP_INFO_WRAP_S *infoWrap)
{
	int ret = 0;
	struct vi_ai_isp_info info;

	memset(&info, 0, sizeof(info));

	info.ViPipe = infoWrap->stIspInfo.viPipe;
	info.InputAddr[0] = infoWrap->stIspInfo.inputAddr[0];
	info.InputAddr[1] = infoWrap->stIspInfo.inputAddr[1];
	info.OutputAddr[0] = infoWrap->stIspInfo.outputAddr[0];
	info.OutputAddr[1] = infoWrap->stIspInfo.outputAddr[1];
	info.Size = infoWrap->stIspInfo.size;
	info.Reserved[0] = infoWrap->stIspInfo.reserved[0];

	ret = G_CTRL_PTR(fd, &info, sizeof(struct vi_ai_isp_info), VI_IOCTL_GET_AI_ISP_RAW);
	if (ret < 0) {
		fprintf(stderr, "VI_IOCTL_GET_AI_ISP_RAW - %s NG, %s\n", __func__, strerror(errno));
		return ret;
	}

	infoWrap->stIspInfo.inputAddr[0] = info.InputAddr[0];
	infoWrap->stIspInfo.inputAddr[1] = info.InputAddr[1];
	infoWrap->stIspInfo.outputAddr[0] = info.OutputAddr[0];
	infoWrap->stIspInfo.outputAddr[1] = info.OutputAddr[1];
	infoWrap->stIspInfo.size = info.Size;
	infoWrap->stIspInfo.reserved[0] = info.Reserved[0];

	return ret;
}

int vi_sdk_put_ai_isp_raw(int fd, VI_AI_ISP_INFO_WRAP_S *infoWrap)
{
	struct vi_ai_isp_info info;

	memset(&info, 0, sizeof(info));

	info.ViPipe = infoWrap->stIspInfo.viPipe;
	info.InputAddr[0] = infoWrap->stIspInfo.inputAddr[0];
	info.InputAddr[1] = infoWrap->stIspInfo.inputAddr[1];
	info.OutputAddr[0] = infoWrap->stIspInfo.outputAddr[0];
	info.OutputAddr[1] = infoWrap->stIspInfo.outputAddr[1];
	info.Size = infoWrap->stIspInfo.size;
	info.Reserved[0] = infoWrap->stIspInfo.reserved[0];

	return S_CTRL_PTR(fd, &info, sizeof(struct vi_ai_isp_info), VI_IOCTL_PUT_AI_ISP_RAW);
}