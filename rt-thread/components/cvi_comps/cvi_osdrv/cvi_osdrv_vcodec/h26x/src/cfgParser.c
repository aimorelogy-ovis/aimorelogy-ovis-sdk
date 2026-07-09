//------------------------------------------------------------------------------
// File: Mixer.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#include"errno.h"
#include "vpuapi.h"
#include "vpuapifunc.h"
#include "main_helper.h"
//------------------------------------------------------------------------------
// ENCODE PARAMETER PARSE FUNCSIONS
//------------------------------------------------------------------------------

#define NUM_GOP_PRESET (3)
const int32_t GOP_SIZE_PRESET[NUM_GOP_PRESET + 1] = { 0, 1, 2, 4};

static const int32_t SVC_T_ENC_GOP_PRESET_1[5] = { 1, 0, 0, 0, 0 };
static const int32_t SVC_T_ENC_GOP_PRESET_2[10] = {
	1, 2, 0, 2, 0, 2, 0, 0, 1, 0
};
static const int32_t SVC_T_ENC_GOP_PRESET_3[20] = {
	1, 4, 0, 3, 0, 2, 2, 0, 2, 0, 3, 4, 2, 3, 0, 4, 0, 0, 1, 0
};

static const int32_t *AVC_GOP_PRESET[NUM_GOP_PRESET + 1] = {
	NULL,
	SVC_T_ENC_GOP_PRESET_1,
	SVC_T_ENC_GOP_PRESET_2,
	SVC_T_ENC_GOP_PRESET_3,
};

void set_gop_info(ENC_CFG *pEncCfg)
{
	int i, j;
	const int32_t *src_gop = AVC_GOP_PRESET[pEncCfg->GopPreset];

	pEncCfg->set_dqp_pic_num = GOP_SIZE_PRESET[pEncCfg->GopPreset];

	for (i = 0, j = 0; i < pEncCfg->set_dqp_pic_num; i++) {
		pEncCfg->gop_entry[i].curr_poc = src_gop[j++];
		pEncCfg->gop_entry[i].qp_offset = src_gop[j++];
		pEncCfg->gop_entry[i].ref_poc = src_gop[j++];
		pEncCfg->gop_entry[i].temporal_id = src_gop[j++];
		pEncCfg->gop_entry[i].ref_long_term = src_gop[j++];
	}
}
