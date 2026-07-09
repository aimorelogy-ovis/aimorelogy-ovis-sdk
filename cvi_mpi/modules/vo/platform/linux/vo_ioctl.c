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

#include <cvi_comm_vo.h>
#include "vo_uapi.h"
#include "vo_ioctl.h"

int vo_set_pattern(int fd, VO_PATTERN_MODE pattern, unsigned int vodev)
{
	return vo_s_ctrl_reserve_value(fd, pattern, vodev, VO_IOCTL_PATTERN);
}

int vo_set_frame_bgcolor(int fd, void *rgb, unsigned int vodev)
{
	return vo_s_ctrl_reserve_ptr(fd, rgb, sizeof(*rgb), vodev, VO_IOCTL_FRAME_BGCOLOR);
}

int vo_set_window_bgcolor(int fd, void *rgb, unsigned int vodev)
{
	return vo_s_ctrl_reserve_ptr(fd, rgb, sizeof(*rgb), vodev, VO_IOCTL_WINDOW_BGCOLOR);
}

int vo_enable_window_bgcolor(int fd, int enable, unsigned int vodev)
{
	return vo_s_ctrl_reserve_value(fd, enable, vodev, VO_IOCTL_ENABLE_WIN_BGCOLOR);
}

int vo_get_videolayer_size(int fd, SIZE_S *vsize, unsigned int vodev)
{
	return vo_g_ctrl_reserve_ptr(fd, vsize, vodev, VO_IOCTL_GET_VLAYER_SIZE);
}

int vo_get_intf_type(int fd, CVI_U32 *intf, unsigned int vodev)
{
	return vo_g_ctrl_reserve_ptr(fd, intf, vodev, VO_IOCTL_GET_INTF_TYPE);
}

int vo_set_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev)
{
	return vo_s_ctrl_reserve_ptr(fd, gamma_attr, sizeof(*gamma_attr), vodev, VO_IOCTL_GAMMA_LUT_UPDATE);
}

int vo_get_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev)
{
	return vo_g_ctrl_reserve_ptr(fd, gamma_attr, vodev, VO_IOCTL_GAMMA_LUT_READ);
}

int vo_set_tgt_compose(int fd, struct vo_rect *area, unsigned int vodev)
{
	return vo_s_ctrl_reserve_ptr(fd, area, sizeof(*area), vodev, VO_IOCTL_SEL_TGT_COMPOSE);
}

int vo_set_tgt_crop(int fd, struct vo_rect *area, unsigned int vodev)
{
	return vo_s_ctrl_reserve_ptr(fd, area, sizeof(*area), vodev, VO_IOCTL_SEL_TGT_CROP);
}

int vo_set_dv_timings(int fd, struct vo_dv_timings *timings, unsigned int vodev)
{
	return vo_s_ctrl_reserve_ptr(fd, timings, sizeof(*timings), vodev, VO_IOCTL_SET_DV_TIMINGS);
}

int vo_get_dv_timings(int fd, struct vo_dv_timings *timings, unsigned int vodev)
{
	return vo_g_ctrl_reserve_ptr(fd, timings, vodev, VO_IOCTL_GET_DV_TIMINGS);
}

int vo_set_stop_streaming(int fd, unsigned int vodev)
{
	return vo_s_ctrl_reserve_value(fd, 0, vodev, VO_IOCTL_STOP_STREAMING);
}

int vo_set_start_streaming(int fd, unsigned int vodev)
{
	return vo_s_ctrl_reserve_value(fd, 0, vodev, VO_IOCTL_START_STREAMING);
}

int vo_set_csc(int fd, struct disp_csc_matrix *cfg, unsigned int volayer)
{
	return vo_s_ctrl_reserve_ptr(fd, cfg, sizeof(*cfg), volayer, VO_IOCTL_SET_CUSTOM_CSC);
}

//vo sdk API list
int vo_sdk_clearchnbuf(int fd, struct vo_clear_chn_buf_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_CLEAR_CHNBUF);
}

int vo_sdk_send_frame(int fd, struct vo_snd_frm_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SEND_FRAME);
}

int vo_sdk_get_panelstatue(int fd, struct vo_panel_status_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_PANELSTATUE);
}

int vo_sdk_get_pubattr(int fd, struct vo_pub_attr_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_PUBATTR);
}

int vo_sdk_set_pubattr(int fd, struct vo_pub_attr_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_PUBATTR);
}

int vo_sdk_set_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_LVDSPARAM);
}

int vo_sdk_get_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_LVDSPARAM);
}

int vo_sdk_set_btparam(int fd, struct vo_bt_param_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_BTPARAM);
}

int vo_sdk_get_btparam(int fd, struct vo_bt_param_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_BTPARAM);
}

int vo_sdk_suspend(int fd)
{
	return vo_sdk_ctrl_ptr(fd, NULL, 0, VO_IOCTL_SDK_CTRL, VO_SDK_SUSPEND);
}

int vo_sdk_resume(int fd)
{
	return vo_sdk_ctrl_ptr(fd, NULL, 0, VO_IOCTL_SDK_CTRL, VO_SDK_RESUME);
}

int vo_sdk_get_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_DISPLAYBUFLEN);
}

int vo_sdk_set_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_DISPLAYBUFLEN);
}

int vo_sdk_set_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_VIDEOLAYERATTR);
}

int vo_sdk_get_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_VIDEOLAYERATTR);
}

int vo_sdk_set_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_LAYER_PROC_AMP);
}

int vo_sdk_get_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_LAYER_PROC_AMP);
}

int vo_sdk_set_layer_csc(int fd, struct vo_layer_csc_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_LAYERCSC);
}

int vo_sdk_get_layer_csc(int fd, struct vo_layer_csc_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_LAYERCSC);
}

int vo_sdk_enable_videolayer(int fd, struct vo_video_layer_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_ENABLE_VIDEOLAYER);
}

int vo_sdk_disable_videolayer(int fd, struct vo_video_layer_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_DISABLE_VIDEOLAYER);
}

int vo_sdk_set_chnattr(int fd, struct vo_chn_attr_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNATTR);
}

int vo_sdk_get_chnattr(int fd, struct vo_chn_attr_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNATTR);
}

int vo_sdk_set_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNFRAMERATE);
}

int vo_sdk_get_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNFRAMERATE);
}

int vo_sdk_get_chn_pts(int fd, struct vo_chn_pts_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNPTS);
}

int vo_sdk_get_chn_status(int fd, struct vo_chn_status_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNSTATUS);
}

int vo_sdk_set_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNROTATION);
}

int vo_sdk_get_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNROTATION);
}

int vo_sdk_enable_chn(int fd, struct vo_chn_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_ENABLE_CHN);
}

int vo_sdk_disable_chn(int fd, struct vo_chn_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_DISABLE_CHN);
}

int vo_sdk_enable(int fd, struct vo_dev_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_ENABLE);
}

int vo_sdk_disable(int fd, struct vo_dev_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_DISABLE);
}

int vo_sdk_isenable(int fd, struct vo_dev_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_ISENABLE);
}

int vo_sdk_showchn(int fd, struct vo_chn_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_SHOW_CHN);
}

int vo_sdk_hidechn(int fd, struct vo_chn_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_HIDE_CHN);
}

int vo_sdk_pausechn(int fd, struct vo_chn_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_PAUSE_CHN);
}

int vo_sdk_resumechn(int fd, struct vo_chn_cfg *cfg)
{
	return vo_sdk_ctrl_ptr(fd, cfg, sizeof(*cfg), VO_IOCTL_SDK_CTRL, VO_SDK_RESUME_CHN);
}
