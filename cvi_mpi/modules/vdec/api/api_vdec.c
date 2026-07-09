#include "cvi_debug.h"
#include "cvi_errno.h"
#include "platform_vdec.h"
#include "cvi_vdec.h"

CVI_S32 CVI_VDEC_CreateChn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
	return platform_vdec_create_chn(VdChn, pstAttr);
}

CVI_S32 CVI_VDEC_DestroyChn(VDEC_CHN VdChn)
{
	return platform_vdec_destroy_chn(VdChn);
}

CVI_S32 CVI_VDEC_GetChnAttr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr)
{
	return platform_vdec_get_chn_attr(VdChn, pstAttr);
}

CVI_S32 CVI_VDEC_SetChnAttr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
	return platform_vdec_set_chn_attr(VdChn, pstAttr);
}

CVI_S32 CVI_VDEC_StartRecvStream(VDEC_CHN VdChn)
{
	return platform_vdec_start_recv_stream(VdChn);
}

CVI_S32 CVI_VDEC_StopRecvStream(VDEC_CHN VdChn)
{
	return platform_vdec_stop_recv_stream(VdChn);
}

CVI_S32 CVI_VDEC_QueryStatus(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus)
{
	return platform_vdec_query_status(VdChn, pstStatus);
}

CVI_S32 CVI_VDEC_GetFd(VDEC_CHN VdChn)
{
	return platform_vdec_get_fd(VdChn);
}

CVI_S32 CVI_VDEC_CloseFd(VDEC_CHN VdChn)
{
	return platform_vdec_close_fd(VdChn);
}

CVI_S32 CVI_VDEC_ResetChn(VDEC_CHN VdChn)
{
	return platform_vdec_reset_chn(VdChn);
}

CVI_S32 CVI_VDEC_SetChnParam(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam)
{
	return platform_vdec_set_chn_param(VdChn, pstParam);
}

CVI_S32 CVI_VDEC_GetChnParam(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam)
{
	return platform_vdec_get_chn_param(VdChn, pstParam);
}

/* s32MilliSec: -1 is block,0 is no block,other positive number is timeout */
CVI_S32 CVI_VDEC_SendStream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, CVI_S32 s32MilliSec)
{
	return platform_vdec_send_stream(VdChn, pstStream, s32MilliSec);;
}

CVI_S32 CVI_VDEC_GetFrame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return platform_vdec_get_frame(VdChn, pstFrameInfo, s32MilliSec);
}

CVI_S32 CVI_VDEC_ReleaseFrame(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	return platform_vdec_release_frame(VdChn, pstFrameInfo);
}

CVI_S32 CVI_VDEC_AttachVbPool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool)
{
	return platform_vdec_attach_vb_pool(VdChn, pstPool);
}

CVI_S32 CVI_VDEC_DetachVbPool(VDEC_CHN VdChn)
{
	return platform_vdec_detach_vb_pool(VdChn);
}

CVI_S32 CVI_VDEC_SetModParam(const VDEC_MOD_PARAM_S *pstModParam)
{
	return platform_vdec_set_mod_param(pstModParam);
}

CVI_S32 CVI_VDEC_GetModParam(VDEC_MOD_PARAM_S *pstModParam)
{
	return platform_vdec_get_mod_param(pstModParam);
}