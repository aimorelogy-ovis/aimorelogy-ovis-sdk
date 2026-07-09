#pragma once

typedef struct _SAMPLE_ISP_RAW_REPLAY_CONFIG_S {
	CVI_S32 width;
	CVI_S32 height;
	CVI_S32 wdr_mode;
	CVI_S32 frame_rate;
	CVI_S32 bayer_id;
	CVI_S32 is_dpcm;
} SAMPLE_ISP_RAW_REPLAY_CONFIG_S;

typedef struct _SAMPLE_ISP_CONFIG_S {
	CVI_S32 sensor_type;
	CVI_S32 vi_num;
	CVI_S32 is_raw_replay_mode;
	SAMPLE_ISP_RAW_REPLAY_CONFIG_S *isp_raw_replay_config;
} SAMPLE_ISP_CONFIG_S;

CVI_S32 isp_init(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config)
CVI_S32 isp_exit(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *pstSampleIspConfig);
