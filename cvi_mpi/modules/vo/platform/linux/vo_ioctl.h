#ifndef __VO_IOCTL_H__
#define __VO_IOCTL_H__

#include "vo_uapi.h"

inline int vo_s_ctrl_value(int _fd, int _cfg, unsigned int _ioctl)
{
	struct vo_ext_control ec1;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.value = _cfg;
	return ioctl(_fd, VO_IOC_S_CTRL, &ec1);
}

static inline int vo_s_ctrl_ptr(int _fd, void *_cfg, unsigned int _ioctl)
{
	struct vo_ext_control ec1;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = (void *)_cfg;
	return ioctl(_fd, VO_IOC_S_CTRL, &ec1);
}

static inline int vo_s_ctrl_reserve_value(int _fd, int _cfg, unsigned int _reserve, unsigned int _ioctl)
{
	struct vo_ext_control ec1;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.value = _cfg;
	ec1.reserved[0] = _reserve;
	return ioctl(_fd, VO_IOC_S_CTRL, &ec1);
}

static inline int vo_s_ctrl_reserve_ptr(int _fd, void *_cfg, unsigned int size, unsigned int _reserved, unsigned int _ioctl)
{
	struct vo_ext_control ec1;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = (void *)_cfg;
	ec1.reserved[0] = _reserved;
	if (_cfg != NULL)
		ec1.size = size;
	return ioctl(_fd, VO_IOC_S_CTRL, &ec1);
}

inline int vo_g_ctrl_ptr(int _fd, void *_cfg, unsigned int _ioctl)
{
	struct vo_ext_control ec1;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = (void *)_cfg;
	return ioctl(_fd, VO_IOC_G_CTRL, &ec1);
}

static inline int vo_g_ctrl_reserve_ptr(int _fd, void *_cfg, unsigned int _reserve, unsigned int _ioctl)
{
	struct vo_ext_control ec1;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.ptr = (void *)_cfg;
	ec1.reserved[0] = _reserve;
	return ioctl(_fd, VO_IOC_G_CTRL, &ec1);
}

static inline int vo_sdk_ctrl_ptr(int _fd, void *_cfg, unsigned int size, unsigned int _ioctl, unsigned int _sdk_id)
{
	struct vo_ext_control ec1;

	memset(&ec1, 0, sizeof(ec1));
	ec1.id = _ioctl;
	ec1.sdk_id = _sdk_id;
	ec1.ptr = (void *)_cfg;
	if (_cfg != NULL)
		ec1.size = size;
	return ioctl(_fd, VO_IOC_S_CTRL, &ec1);
}

//self ioctl cmd test
int vo_set_pattern(int fd, VO_PATTERN_MODE pattern, unsigned int vodev);
int vo_set_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev);
int vo_get_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev);
int vo_set_csc(int fd, struct disp_csc_matrix *cfg, unsigned int volayer);

//vo sdk layer apis
int vo_sdk_enable(int fd, struct vo_dev_cfg *cfg);
int vo_sdk_disable(int fd, struct vo_dev_cfg *cfg);
int vo_sdk_isenable(int fd, struct vo_dev_cfg *cfg);
int vo_sdk_get_panelstatue(int fd, struct vo_panel_status_cfg *cfg);
int vo_sdk_get_pubattr(int fd, struct vo_pub_attr_cfg *cfg);
int vo_sdk_set_pubattr(int fd, struct vo_pub_attr_cfg *cfg);
int vo_sdk_set_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg);
int vo_sdk_get_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg);
int vo_sdk_set_btparam(int fd, struct vo_bt_param_cfg *cfg);
int vo_sdk_get_btparam(int fd, struct vo_bt_param_cfg *cfg);
int vo_sdk_get_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg);
int vo_sdk_set_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg);
int vo_sdk_set_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg);
int vo_sdk_get_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg);
int vo_sdk_set_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg);
int vo_sdk_get_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg);
int vo_sdk_set_layer_csc(int fd, struct vo_layer_csc_cfg *cfg);
int vo_sdk_get_layer_csc(int fd, struct vo_layer_csc_cfg *cfg);
int vo_sdk_enable_videolayer(int fd, struct vo_video_layer_cfg *cfg);
int vo_sdk_disable_videolayer(int fd, struct vo_video_layer_cfg *cfg);
int vo_sdk_enable_chn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_disable_chn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_send_frame(int fd, struct vo_snd_frm_cfg *cfg);
int vo_sdk_clearchnbuf(int fd, struct vo_clear_chn_buf_cfg *cfg);
int vo_sdk_set_chnattr(int fd, struct vo_chn_attr_cfg *cfg);
int vo_sdk_get_chnattr(int fd, struct vo_chn_attr_cfg *cfg);
int vo_sdk_set_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg);
int vo_sdk_get_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg);
int vo_sdk_get_chn_pts(int fd, struct vo_chn_pts_cfg *cfg);
int vo_sdk_get_chn_status(int fd, struct vo_chn_status_cfg *cfg);
int vo_sdk_set_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg);
int vo_sdk_get_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg);
int vo_sdk_showchn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_hidechn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_pausechn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_resumechn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_suspend(int fd);
int vo_sdk_resume(int fd);

#endif
