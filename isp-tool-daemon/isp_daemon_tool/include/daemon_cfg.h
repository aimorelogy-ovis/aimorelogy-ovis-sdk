
#ifndef __DAEMON_CFG_H_
#define __DAEMON_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cvi_json.h"
#include "cvi_comm_3a.h"

#define MAX_PATH_LEN 128
#define MAX_CODEC_LEN 16

typedef struct vc_coding_param_t {
	int FrmLostOpen;
	int LostMode;
	int FrmLostBpsThr;
	int EncFrmGaps;
	int IntraCost;
	int aspectRatioInfoPresentFlag;
	int overscanInfoPresentFlag;
	int videoSignalTypePresentFlag;
	int videoFormat;
	int videoFullRangeFlag;
	int colourDescriptionPresentFlag;
	int ChromaQpOffset;
	int CbQpOffset;
	int CrQpOffset;
	int TimingInfoPresentFlag;
	int FixedFrameRateFlag;
	int NumUnitsInTick;
} vc_coding_param_t;

typedef struct vc_gop_mode_t {
	int GopMode;
	int IPQpDelta;
	int BgInterval;
	int BgQpDelta;
	int ViQpDelta;
} vc_gop_mode_t;

typedef struct vc_rc_attr_t {
	int RcMode;
	int Gop;
	int VariableFPS;
	int SrcFrmRate;
	int DstFrmRate;
	int StatTime;
	int BitRate;
	int MaxBitrate;
	int IQP;
	int PQP;
} vc_rc_attr_t;

typedef struct vc_rc_param_t {
	int RowQpDelta;
	int FirstFrameStartQp;
	int InitialDelay;
	int ThrdLv;
	int MinIprop;
	int MaxIprop;
	int MaxQp;
	int MinQp;
	int MaxIQp;
	int MinIQp;
	int ChangePos;
	int MinStillPercent;
	int MaxStillQP;
	int MotionSensitivity;
	int PureStillThr;
	int AvbrFrmLostOpen;
	int AvbrFrmGap;
} vc_rc_param_t;

typedef struct venc_cfg_t {
	vc_coding_param_t st_coding_param;
	vc_gop_mode_t st_gop_mode;
	vc_rc_attr_t st_rc_attr;
	vc_rc_param_t st_rc_param;
} vc_cfg_t;

typedef struct video_pipe_cfg_t {
	int chn;
	int buf_blk_cnt;
	int is_wdr_mode;
	int tpu_device_id;
	int enable_isp_info_osd;
	int enable_face_ae;
	int enable_teaisp_pq;
	int enable_teaisp_bnr;
	char bnr_model_list[MAX_PATH_LEN];
	char venc_json[MAX_PATH_LEN];
	char codec[MAX_CODEC_LEN];
	int compress_mode;
	int enable_patgen;
	vc_cfg_t st_vc_cfg;
	ISP_STITCH_ATTR_S stitch_attr;
} video_pipe_cfg_t;

typedef struct raw_replay_cfg_t {
	int replay_scene;
	int pixel_format;
	int width;
	int height;
	int timing_enable;
	int frame_rate;
	int wdr_mode;
	int bayer_format;
	int compress_mode;
	char offline_raw_dir[MAX_PATH_LEN];
} raw_replay_cfg_t;

typedef struct daemon_pipe_cfg_t {
	int dev_num;
	int rtsp_port;
	unsigned long long rtsp_max_buf_size;
	int rtsp_server_select;
	int vi_vpss_mode;
	int raw_replay_enable;
	int is_fastboot_mode;
	int enable_dump_boot_video;
	int max_use_tpu_num;
	int enable_stitch;
	char teaisp_faceae_model_path[MAX_PATH_LEN];
	char teaisp_pq_model_path[MAX_PATH_LEN];
	char cvi_bin_path[MAX_PATH_LEN];
	char sns_cfg_ini[MAX_PATH_LEN];
	raw_replay_cfg_t raw_replay_cfg;
	video_pipe_cfg_t video_pipe_cfg;
} daemon_pipe_cfg_t;

int daemon_pipe_cfg_init(const char *json_path, daemon_pipe_cfg_t **p_cfg);
int daemon_pipe_cfg_deinit(daemon_pipe_cfg_t **p_cfg);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif // __DAEMON_CFG_H_
