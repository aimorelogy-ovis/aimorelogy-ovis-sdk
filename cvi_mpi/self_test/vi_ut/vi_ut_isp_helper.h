#pragma once
#ifndef CV184X_FPGA_RAW_REPLAY
#include "vi_ut_comm.h"

typedef struct _SAMPLE_ISP_RAW_REPLAY_CONFIG_S {
	int width;
	int height;
	int wdr_mode;
	int frame_rate;
	int bayer_id;
	int is_dpcm;
} SAMPLE_ISP_RAW_REPLAY_CONFIG_S;

typedef struct _SAMPLE_ISP_CONFIG_S {
	int sensor_type;
	int vi_num;
	int is_raw_replay_mode;
	SAMPLE_ISP_RAW_REPLAY_CONFIG_S *isp_raw_replay_config;
} SAMPLE_ISP_CONFIG_S;

int SAMPLE_ISP_Init(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config);

int SAMPLE_ISP_Exit(VI_PIPE pipe);


#endif
