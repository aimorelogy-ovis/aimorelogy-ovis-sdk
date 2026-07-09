//--=========================================================================--
//  This file is a part of QC Tool project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2004 - 2011   CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--
#include "main_helper.h"
//#ifdef SUPPORT_ENCODE_CUSTOM_HEADER
#include "header_struct.h"
#include "pbu.h"
//#endif
#include "malloc.h"

// include in the ffmpeg header
typedef struct {
	CodStd codStd;
	Uint32 mp4Class;
	Uint32 codecId;
	Uint32 fourcc;
} CodStdTab;

#ifndef MKTAG
#define MKTAG(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))
#endif


//////////////////// DRAM Read/Write helper Function
Uint32 seiEncode(CodStd format, Uint8 *pSrc, Uint32 srcLen, Uint8 *pBuffer, Uint32 bufferSize)
{
	spp_enc_context spp;
	Uint32 code;

	Uint32 put_bit_byte_size;
	Uint32 i;

	const Uint32 nuh_layer_id = 0;
	const Uint32 nuh_temporal_id_plus1 = 1;

	spp = spp_enc_init(pBuffer, bufferSize, 1);

	// put start code
	spp_enc_put_nal_byte(spp, 1, 4);

	// put nal header
	if (format == STD_AVC) {
		code = AVC_NUT_SEI;
		spp_enc_put_nal_byte(spp, code, 1);
	} else if (format == STD_HEVC) {
		code = ((SNT_PREFIX_SEI & 0x3f) << 9) |
		       ((nuh_layer_id & 0x3f) << 3) |
		       ((nuh_temporal_id_plus1 & 0x3) << 0);
		spp_enc_put_nal_byte(spp, code, 2);
	}

	// put payload type
	spp_enc_put_nal_byte(spp, 0x05, 1);

	// put payload size
	for (i = 0; i < (srcLen + 16) / 0xff; i++)
		spp_enc_put_nal_byte(spp, 0xff, 1);
	spp_enc_put_nal_byte(spp, (srcLen + 16) % 0xff, 1);

	// put uuid(16 byte nouse)
	for (i = 0; i < 8; i++)
		spp_enc_put_nal_byte(spp, 0x03, 2);

	//put userdata
	for (i = 0; i < srcLen; i++)
		spp_enc_put_bits(spp, pSrc[i], 8);

	spp_enc_put_byte_align(spp, 1);
	spp_enc_flush(spp);

	put_bit_byte_size = spp_enc_get_nal_cnt(spp);

	spp_enc_deinit(spp);
	return put_bit_byte_size;
}

static int get_stop_one_bit_idx(Uint8 byte)
{
	int i;

	for (i = 0; i < 7; i++) {
		if ((byte >> i) & 0x1)
			break;
	}

	return i;
}

static void encodeVuiAspectRatioInfo(spp_enc_context spp, cviVuiAspectRatio *pari)
{
	spp_enc_put_bits(spp, pari->aspect_ratio_info_present_flag ? 1 : 0, 1);
	if (pari->aspect_ratio_info_present_flag) {
		spp_enc_put_bits(spp, pari->aspect_ratio_idc, 8);
		if (pari->aspect_ratio_idc == EXTENDED_SAR) {
			spp_enc_put_bits(spp, pari->sar_width, 16);
			spp_enc_put_bits(spp, pari->sar_height, 16);
		}
	}
}

static void encodeVuiOverscanInfo(spp_enc_context spp, cviVuiAspectRatio *pari)
{
	spp_enc_put_bits(spp, pari->overscan_info_present_flag ? 1 : 0, 1);
	if (pari->overscan_info_present_flag) {
		spp_enc_put_bits(spp, pari->overscan_appropriate_flag ? 1 : 0, 1);
	}
}

static void encodeVuiVideoSignalType(spp_enc_context spp, cviVuiVideoSignalType *pvst)
{
	spp_enc_put_bits(spp, pvst->video_signal_type_present_flag ? 1 : 0, 1);
	if (pvst->video_signal_type_present_flag) {
		spp_enc_put_bits(spp, pvst->video_format, 3);
		spp_enc_put_bits(spp, pvst->video_full_range_flag, 1);
		spp_enc_put_bits(spp, pvst->colour_description_present_flag ? 1 : 0, 1);
		if (pvst->colour_description_present_flag) {
			spp_enc_put_bits(spp, pvst->colour_primaries, 8);
			spp_enc_put_bits(spp, pvst->transfer_characteristics, 8);
			spp_enc_put_bits(spp, pvst->matrix_coefficients, 8);
		}
	}
}

static void encodeVuiH264TimingInfo(spp_enc_context spp, cviVuiH264TimingInfo *pti)
{
	spp_enc_put_bits(spp, pti->timing_info_present_flag ? 1 : 0, 1);
	if (pti->timing_info_present_flag) {
		spp_enc_put_bits(spp, pti->num_units_in_tick, 32);
		spp_enc_put_bits(spp, pti->time_scale, 32);
		spp_enc_put_bits(spp, pti->fixed_frame_rate_flag, 1);
	}
}

static void encodeVuiH265TimingInfo(spp_enc_context spp, cviVuiH265TimingInfo *pti)
{
	spp_enc_put_bits(spp, pti->timing_info_present_flag ? 1 : 0, 1);
	if (pti->timing_info_present_flag) {
		spp_enc_put_bits(spp, pti->num_units_in_tick, 32);
		spp_enc_put_bits(spp, pti->time_scale, 32);
		spp_enc_put_bits(spp, 0, 1); // vui_poc_proportional_to_timing_flag = 0;
		spp_enc_put_bits(spp, 0, 1); // vui_hrd_parameters_present_flag = 0;
	}
}

BOOL H264SpsAddVui(cviH264Vui *pVui, void **ppBuffer, Int32 *pBufferSize)
{
	int i;
	int stop_one_bit_idx = 0;
	int keep_bytes = 0;
	int keep_bits = 0;

	int dst_size = *pBufferSize + 64;
	unsigned char *dst_buf = NULL;
	unsigned char *sps = (unsigned char *)(*ppBuffer);

	spp_enc_context spp;

	if (!pVui->aspect_ratio_info.aspect_ratio_info_present_flag &&
	    !pVui->aspect_ratio_info.overscan_info_present_flag &&
	    !pVui->video_signal_type.video_signal_type_present_flag &&
	    !pVui->timing_info.timing_info_present_flag) {
		// none of the supported flags are present. do nothing.
		return TRUE;
	}

	dst_buf = malloc(dst_size);

	// find location of vui_parameters_present_flag
	// which immediately precedes the stop_one_bit
	stop_one_bit_idx = get_stop_one_bit_idx(sps[*pBufferSize - 1]);

	// keep everything before vui_parameters_present_flag
	if (stop_one_bit_idx == 7) {
		keep_bytes = *pBufferSize - 2;
		keep_bits = 7;
	} else {
		keep_bytes = *pBufferSize - 1;
		keep_bits = 6 - stop_one_bit_idx;
	}

	memcpy(dst_buf, *ppBuffer, keep_bytes - 2);

	spp = spp_enc_init(dst_buf + keep_bytes - 2, dst_size - (keep_bytes - 2), 1);

	for (i = 0; i < 2; i++) {
		spp_enc_put_bits(spp, sps[keep_bytes - 2 + i], 8);
	}

	for (i = 0; i < keep_bits; i++) {
		int bit = (sps[keep_bytes] >> (7 - i)) & 0x1;
		spp_enc_put_bits(spp, bit, 1);
	}

	// vui_parameters_present_flag = 1
	spp_enc_put_bits(spp, 1, 1);

	encodeVuiAspectRatioInfo(spp, &pVui->aspect_ratio_info);
	encodeVuiOverscanInfo(spp, &pVui->aspect_ratio_info);
	encodeVuiVideoSignalType(spp, &pVui->video_signal_type);

	// chroma_loc_info_present_flag = 0
	spp_enc_put_bits(spp, 0, 1);

	encodeVuiH264TimingInfo(spp, &pVui->timing_info);

	// nal_hrd_parameters_present_flag = 0
	// vcl_hrd_parameters_present_flag = 0
	// pic_struct_present_flag = 0
	// bitstream_restriction_flag = 0
	spp_enc_put_bits(spp, 0, 4);

	spp_enc_put_byte_align(spp, 1);
	spp_enc_flush(spp);

	// update new sps length
	*pBufferSize = spp_enc_get_nal_cnt(spp) + keep_bytes - 2;
	spp_enc_deinit(spp);

	// replace sps
	free(*ppBuffer);
	*ppBuffer = dst_buf;

	return TRUE;
}

BOOL H265SpsAddVui(cviH265Vui *pVui, void **ppBuffer, Int32 *pBufferSize)
{
	int i;
	int stop_one_bit_idx = 0;
	int keep_bytes = 0;
	int keep_bits = 0;
	int dst_size = *pBufferSize + 64;
	unsigned char *dst_buf = NULL;
	unsigned char *sps = (unsigned char *)(*ppBuffer);

	spp_enc_context spp;

	if (!pVui->aspect_ratio_info.aspect_ratio_info_present_flag &&
	    !pVui->aspect_ratio_info.overscan_info_present_flag &&
	    !pVui->video_signal_type.video_signal_type_present_flag &&
	    !pVui->timing_info.timing_info_present_flag) {
		// none of the supported flags are present. do nothing.
		return TRUE;
	}

	dst_buf = malloc(dst_size);

	// find location of vui_parameters_present_flag
	// which is 2 bits ahead of the stop_one_bit
	stop_one_bit_idx = get_stop_one_bit_idx(sps[*pBufferSize - 1]);

	// keep everything before vui_parameters_present_flag
	if (stop_one_bit_idx == 7) {
		keep_bytes = *pBufferSize - 2;
		keep_bits = 6;
	} else if (stop_one_bit_idx == 6) {
		keep_bytes = *pBufferSize - 2;
		keep_bits = 7;
	} else {
		keep_bytes = *pBufferSize - 1;
		keep_bits = 5 - stop_one_bit_idx;
	}

	memcpy(dst_buf, *ppBuffer, keep_bytes - 2);

	spp = spp_enc_init(dst_buf + keep_bytes - 2, dst_size - (keep_bytes - 2), 1);

	for (i = 0; i < 2; i++) {
		spp_enc_put_bits(spp, sps[keep_bytes - 2 + i], 8);
	}

	for (i = 0; i < keep_bits; i++) {
		int bit = (sps[keep_bytes] >> (7 - i)) & 0x1;
		spp_enc_put_bits(spp, bit, 1);
	}

	// vui_parameters_present_flag = 1
	spp_enc_put_bits(spp, 1, 1);

	encodeVuiAspectRatioInfo(spp, &pVui->aspect_ratio_info);
	encodeVuiOverscanInfo(spp, &pVui->aspect_ratio_info);
	encodeVuiVideoSignalType(spp, &pVui->video_signal_type);

	// chroma_loc_info_present_flag = 0
	// neutral_chroma_indication_flag = 0
	// field_seq_flag = 0
	// frame_field_info_present_flag = 0
	// default_display_window_flag = 0
	spp_enc_put_bits(spp, 0, 5);

	encodeVuiH265TimingInfo(spp, &pVui->timing_info);

	// bitstream_restriction_flag = 0
	// sps_extension_present_flag = 0
	spp_enc_put_bits(spp, 0, 2);

	spp_enc_put_byte_align(spp, 1);
	spp_enc_flush(spp);

	// update new sps length
	*pBufferSize = spp_enc_get_nal_cnt(spp) + keep_bytes - 2;
	spp_enc_deinit(spp);

	// replace sps
	free(*ppBuffer);
	*ppBuffer = dst_buf;

	return TRUE;
}
