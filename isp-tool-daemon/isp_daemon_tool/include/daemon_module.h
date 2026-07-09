
#ifndef __DAEMON_MODULE_H__
#define __DAEMON_MODULE_H__

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "cvi_comm_sys.h"
#include "cvi_vpss.h"
#include "cvi_bin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct module_video_src_cfg_t {
	uint8_t enable_teaisp_bnr;
	uint8_t enable_patgen;
	uint8_t enable_raw_replay;
	uint8_t enable_mirror;
	uint8_t enable_flip;
	uint8_t enable_isp_stitch;
	ISP_STITCH_ATTR_S stitch_attr;
	int bypass_src_frame_num;
	VI_VPSS_MODE_E vi_vpss_mode;
	COMPRESS_MODE_E compress_mode;
	char cvi_bin_path[MAX_PATH_LEN];
	char bnr_model_list[MAX_PATH_LEN];
	raw_replay_cfg_t raw_replay_cfg;
} module_video_src_cfg_t;
extern struct module_fun_t video_src_fun;

typedef struct module_venc_cfg_t {
	uint32_t width;
	uint32_t height;
	PIXEL_FORMAT_E pixel_format;
	char codec[MAX_CODEC_LEN];
	vc_cfg_t vc_cfg;
	int enable_dump_boot_video;
} module_venc_cfg_t;
extern struct module_fun_t venc_fun;

typedef struct module_venc_fastboot_cfg_t {
	int dev_num;
	char codec[MAX_CODEC_LEN];
} module_venc_fastboot_cfg_t;
extern struct module_fun_t venc_fun_fastboot;

typedef struct module_rtsp_cfg_t {
	int rtsp_port;
	int max_buf_size;
	int bitrate;
	char codec[MAX_CODEC_LEN];
} module_rtsp_cfg_t;
extern struct module_fun_t cvi_rtsp_fun;

typedef struct module_rtsp3_cfg_t {
	int rtsp_port;
	uint32_t width;
	uint32_t height;
	char codec[MAX_CODEC_LEN];
} module_rtsp3_cfg_t;
extern struct module_fun_t cvi_rtsp3_fun;

typedef struct module_sei_osd_cfg_t {
	char codec[MAX_CODEC_LEN];
} module_sei_osd_cfg_t;
extern struct module_fun_t sei_osd_fun;

extern struct module_fun_t osd_fun;

typedef struct module_vpss_cfg_t {
	int8_t grp_id; // -1 for auto assign
	uint8_t chn_id;
	uint8_t chn_num;
	uint8_t enable_vi_vpss_online;
	uint8_t enable_bypass_src_frame;
	VPSS_GRP_ATTR_S st_vpss_grp_attr;
	uint8_t chn_vb_cnt[VPSS_MAX_CHN_NUM];
	VPSS_CHN_ATTR_S st_vpss_chn_attr[VPSS_MAX_CHN_NUM];
} module_vpss_cfg_t;
extern struct module_fun_t vpss_fun;

typedef struct module_vpss_chn_cfg_t {
	uint8_t src_grp_id;
	uint8_t chn_id;
} module_vpss_chn_cfg_t;
extern struct module_fun_t vpss_chn_fun;

typedef struct module_face_ae_cfg_t {
	char model_path[MAX_PATH_LEN];
} module_face_ae_cfg_t;
extern struct module_fun_t face_ae_fun;

typedef struct module_teaisp_pq_cfg_t {
	char model_path[MAX_PATH_LEN];
} module_teaisp_pq_cfg_t;
extern struct module_fun_t teaisp_pq_fun;

extern struct module_fun_t empty_module_fun;

#ifdef __cplusplus
}
#endif
#endif
