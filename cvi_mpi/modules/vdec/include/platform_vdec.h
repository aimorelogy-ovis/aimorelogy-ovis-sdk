#ifndef __PLATFORM_VDEC_H__
#define __PLATFORM_VDEC_H__
#include "cvi_comm_vdec.h"

CVI_S32 platform_vdec_create_chn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr);
CVI_S32 platform_vdec_destroy_chn(VDEC_CHN VdChn);
CVI_S32 platform_vdec_get_chn_attr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr);
CVI_S32 platform_vdec_set_chn_attr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr);
CVI_S32 platform_vdec_start_recv_stream(VDEC_CHN VdChn);
CVI_S32 platform_vdec_stop_recv_stream(VDEC_CHN VdChn);
CVI_S32 platform_vdec_query_status(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus);
CVI_S32 platform_vdec_get_fd(VDEC_CHN VdChn);
CVI_S32 platform_vdec_close_fd(VDEC_CHN VdChn);
CVI_S32 platform_vdec_reset_chn(VDEC_CHN VdChn);
CVI_S32 platform_vdec_set_chn_param(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam);
CVI_S32 platform_vdec_get_chn_param(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam);
CVI_S32 platform_vdec_send_stream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, CVI_S32 s32MilliSec);
CVI_S32 platform_vdec_get_frame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);
CVI_S32 platform_vdec_release_frame(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrameInfo);
CVI_S32 platform_vdec_attach_vb_pool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool);
CVI_S32 platform_vdec_detach_vb_pool(VDEC_CHN VdChn);
CVI_S32 platform_vdec_set_mod_param(const VDEC_MOD_PARAM_S *pstModParam);
CVI_S32 platform_vdec_get_mod_param(VDEC_MOD_PARAM_S *pstModParam);
#endif