#pragma once

#include "cvi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _RAW_REPLAY_INFO {
	CVI_S32 isValid;
	CVI_S32 numFrame;
	CVI_S32 curFrame;
	CVI_S32 width;
	CVI_S32 height;
	CVI_S32 bayerID;
	CVI_S32 enWDR;
	CVI_S32 ISO;
	CVI_FLOAT lightValue;
	CVI_S32 colorTemp;
	CVI_S32 ispDGain;
	CVI_S32 exposureRatio;
	CVI_S32 exposureAGain;
	CVI_S32 exposureDGain;
	CVI_S32 longExposure;
	CVI_S32 shortExposure;
	CVI_S32 WB_RGain;
	CVI_S32 WB_GGain;
	CVI_S32 WB_BGain;
	CVI_S32 CCM[9];
	CVI_S32 BLC_Offset[4];
	CVI_S32 BLC_Gain[4];
	CVI_S32 size;

	CVI_S32 roiFrameNum;
	RECT_S stRoiRect;
	CVI_S32 roiFrameSize;

	CVI_S32 op_mode;
	CVI_S32 AGainSF;
	CVI_S32 DGainSF;
	CVI_S32 ispDGainSF;

	CVI_S32 pixFormat;
	CVI_S32 is_dpcm;
} RAW_REPLAY_INFO;

/*
typedef struct _OFFLINE_3A_ALGO_RET {
	int iso;
	float lv;
	int isp_dgain;
	int color_tmp;
	// exp informaton
	int le_exp_time;
	int se_exp_time;
	int exp_ratio;
	int exp_again;
	int exp_dgain;
	// wbg
	int wbg_rgain;
	int wbg_bgain;
	int wbg_ggain;
	// blc
	int blc_offset_r;
	int blc_offset_gr;
	int blc_offset_gb;
	int blc_offset_b;
	int blc_gain_r;
	int blc_gain_gr;
	int blc_gain_gb;
	int blc_gain_b;
	// ccm
	int ccm[9];
} OFFLINE_3A_ALGO_RET;
*/

typedef struct _ISP_ALGO_RESULT_S {
	CVI_U32 u32FrameIdx;
	CVI_U32 u32IspPostDgain;
	CVI_U32 u32IspPreDgain;
	CVI_U32 u32IspPostDgainSE;
	CVI_U32 u32IspPreDgainSE;
	CVI_FLOAT afAEEVRatio[ISP_CHANNEL_MAX_NUM];
	CVI_U32 u32PreIso;
	CVI_U32 u32PostIso;
	CVI_U32 u32PreBlcIso;
	CVI_U32 u32PostBlcIso;
	CVI_U32 u32PostNpIso;
	CVI_U32 u32PreNpIso;
	CVI_U32 au32ExpRatio[3];
	CVI_S16  currentLV;
	CVI_U32  u32AvgLuma;
	WDR_MODE_E enFSWDRMode;
	CVI_U32 u32ColorTemp;
	CVI_U32 au32WhiteBalanceGain[ISP_BAYER_CHN_NUM];
	CVI_U32 au32WhiteBalanceGainPre[ISP_BAYER_CHN_NUM];
} ISP_ALGO_RESULT_S;

typedef enum _RAWPLAY_OP_MODE {
	RAW_OP_MODE_NORMAL = 0,
	RAW_OP_MODE_AE_SIM = 1,
	RAW_OP_MODE_AWB_SIM = 2,
	RAW_OP_MODE_AF_SIM = 3,
} RAWPLAY_OP_MODE;

typedef enum {
	BB,
	GB,
	GR,
	RR,
	BAYER_CHANNEL_SIZE,
} BAYER_CHANNEL;

typedef struct _OFFLINE_CTRL_CMD {
	int raw_replay_start;
	int raw_replay_stop;
	int raw_replay_reset;
	int raw_replay_start_idx;
	int raw_replay_end_idx;
	int raw_replay_run_mode; // 0: loop; 1: loop and reset
	char yuv_save_idx[1024];
	char stop_wait_idx[1024];
	int raw_replay_wait_continue;
	int raw_replay_max_frame_id;
} OFFLINE_CTRL_CMD;

#define DISABLE_AWB_UPDATE_CTRL       (1 << 0)

CVI_S32 raw_replay_offline_init(char *boardPath, VI_USR_PIC_INFO_S *pic_info);
void raw_replay_offline_uninit(void);
CVI_S32 start_raw_replay_offline(VI_PIPE ViPipe);
CVI_S32 stop_raw_replay_offline(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
