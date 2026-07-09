#include "cvi_debug.h"
#include "cvi_errno.h"
#include "cvi_venc.h"
#include "platform_venc.h"

CVI_S32 CVI_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
	return platform_venc_create_chn(VeChn, pstAttr);
}

CVI_S32 CVI_VENC_DestroyChn(VENC_CHN VeChn)
{
	return platform_venc_destroy_chn(VeChn);
}

CVI_S32 CVI_VENC_ResetChn(VENC_CHN VeChn)
{
	return platform_venc_reset_chn(VeChn);
}

CVI_S32 CVI_VENC_StartRecvFrame(VENC_CHN VeChn, const VENC_RECV_PIC_PARAM_S *pstRecvParam)
{
	return platform_venc_start_recv_frame(VeChn, pstRecvParam);
}

CVI_S32 CVI_VENC_StopRecvFrame(VENC_CHN VeChn)
{
	return platform_venc_stop_recv_frame(VeChn);
}

CVI_S32 CVI_VENC_QueryStatus(VENC_CHN VeChn, VENC_CHN_STATUS_S *pstStatus)
{
	return platform_venc_query_status(VeChn, pstStatus);
}

CVI_S32 CVI_VENC_SetChnAttr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstChnAttr)
{
	return platform_venc_set_chn_attr(VeChn, pstChnAttr);
}

CVI_S32 CVI_VENC_GetChnAttr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstChnAttr)
{
	return platform_venc_get_chn_attr(VeChn, pstChnAttr);
}

CVI_S32 CVI_VENC_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, CVI_S32 S32MilliSec)
{
	return platform_venc_get_stream(VeChn, pstStream, S32MilliSec);
}

CVI_S32 CVI_VENC_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
	return platform_venc_release_stream(VeChn, pstStream);
}

CVI_S32 CVI_VENC_InsertUserData(VENC_CHN VeChn, CVI_U8 *pu8Data, CVI_U32 u32Len)
{
	return platform_venc_insert_user_data(VeChn, pu8Data, u32Len);
}

CVI_S32 CVI_VENC_SendFrame(VENC_CHN VeChn, const VIDEO_FRAME_INFO_S *pstFrame, CVI_S32 s32MilliSec)
{
	return platform_venc_send_frame(VeChn, pstFrame, s32MilliSec);
}

CVI_S32 CVI_VENC_SendFrameEx(VENC_CHN VeChn, const USER_FRAME_INFO_S *pstFrame, CVI_S32 s32MilliSec)
{
	return platform_venc_send_frame_ex(VeChn, pstFrame, s32MilliSec);
}

CVI_S32 CVI_VENC_RequestIDR(VENC_CHN VeChn, CVI_BOOL bInstant)
{
	return platform_venc_request_idr(VeChn, bInstant);
}

CVI_S32 CVI_VENC_EnableIDR(VENC_CHN VeChn, CVI_BOOL bInstant)
{
	return platform_venc_enable_idr(VeChn, bInstant);
}

CVI_S32 CVI_VENC_GetFd(VENC_CHN VeChn)
{
	return platform_venc_get_fd(VeChn);
}

CVI_S32 CVI_VENC_CloseFd(VENC_CHN VeChn)
{
	return platform_venc_close_fd(VeChn);
}

CVI_S32 CVI_VENC_SetRoiAttr(VENC_CHN VeChn, const VENC_ROI_ATTR_S *pstRoiAttr)
{
	return platform_venc_set_roi_attr(VeChn, pstRoiAttr);
}

CVI_S32 CVI_VENC_GetRoiAttr(VENC_CHN VeChn, CVI_U32 u32Index, VENC_ROI_ATTR_S *pstRoiAttr)
{
	return platform_venc_get_roi_attr(VeChn, u32Index, pstRoiAttr);
}

CVI_S32 CVI_VENC_SetH264Trans(VENC_CHN VeChn, const VENC_H264_TRANS_S *pstH264Trans)
{
	return platform_venc_set_h264_trans(VeChn, pstH264Trans);
}

CVI_S32 CVI_VENC_GetH264Trans(VENC_CHN VeChn, VENC_H264_TRANS_S *pstH264Trans)
{
	return platform_venc_get_h264_trans(VeChn, pstH264Trans);
}

CVI_S32 CVI_VENC_SetH264Entropy(VENC_CHN VeChn, const VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	return platform_venc_set_h264_entropy(VeChn, pstH264EntropyEnc);
}

CVI_S32 CVI_VENC_GetH264Entropy(VENC_CHN VeChn, VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	return platform_venc_get_h264_entropy(VeChn, pstH264EntropyEnc);
}

CVI_S32 CVI_VENC_SetH264Vui(VENC_CHN VeChn, const VENC_H264_VUI_S *pstH264Vui)
{
	return platform_venc_set_h264_vui(VeChn, pstH264Vui);
}

CVI_S32 CVI_VENC_GetH264Vui(VENC_CHN VeChn, VENC_H264_VUI_S *pstH264Vui)
{
	return platform_venc_get_h264_vui(VeChn, pstH264Vui);
}

CVI_S32 CVI_VENC_SetH265Vui(VENC_CHN VeChn, const VENC_H265_VUI_S *pstH265Vui)
{
	return platform_venc_set_h265_vui(VeChn, pstH265Vui);
}

CVI_S32 CVI_VENC_GetH265Vui(VENC_CHN VeChn, VENC_H265_VUI_S *pstH265Vui)
{
	return platform_venc_get_h265_vui(VeChn, pstH265Vui);
}

CVI_S32 CVI_VENC_SetJpegParam(VENC_CHN VeChn, const VENC_JPEG_PARAM_S *pstJpegParam)
{
	return platform_venc_set_jpeg_param(VeChn, pstJpegParam);
}

CVI_S32 CVI_VENC_GetJpegParam(VENC_CHN VeChn, VENC_JPEG_PARAM_S *pstJpegParam)
{
	return platform_venc_get_jpeg_param(VeChn, pstJpegParam);
}

CVI_S32 CVI_VENC_GetRcParam(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcParam)
{
	return platform_venc_get_rc_param(VeChn, pstRcParam);
}

CVI_S32 CVI_VENC_SetMjpegParam(VENC_CHN VeChn, const VENC_MJPEG_PARAM_S *pstMJpegParam)
{
	return platform_venc_set_mjpeg_param(VeChn, pstMJpegParam);
}

CVI_S32 CVI_VENC_GetMjpegParam(VENC_CHN VeChn, VENC_MJPEG_PARAM_S *pstMJpegParam)
{
	return platform_venc_get_mjpeg_param(VeChn, pstMJpegParam);
}

CVI_S32 CVI_VENC_SetRcParam(VENC_CHN VeChn, const VENC_RC_PARAM_S *pstRcParam)
{
	return platform_venc_set_rc_param(VeChn, pstRcParam);
}

CVI_S32 CVI_VENC_SetRefParam(VENC_CHN VeChn, const VENC_REF_PARAM_S *pstRefParam)
{
	return platform_venc_set_ref_param(VeChn, pstRefParam);
}

CVI_S32 CVI_VENC_GetRefParam(VENC_CHN VeChn, VENC_REF_PARAM_S *pstRefParam)
{
	return platform_venc_get_ref_param(VeChn, pstRefParam);
}

CVI_S32 CVI_VENC_SetH265PredUnit(VENC_CHN VeChn, const VENC_H265_PU_S *pstPredUnit)
{
	return platform_venc_set_h265_pred_unit(VeChn, pstPredUnit);

}

CVI_S32 CVI_VENC_GetH265PredUnit(VENC_CHN VeChn, VENC_H265_PU_S *pstPredUnit)
{
	return platform_venc_get_h265_pred_unit(VeChn, pstPredUnit);
}

CVI_S32 CVI_VENC_SetH265Trans(VENC_CHN VeChn, const VENC_H265_TRANS_S *pstH265Trans)
{
	return platform_venc_set_h265_trans(VeChn, pstH265Trans);
}

CVI_S32 CVI_VENC_GetH265Trans(VENC_CHN VeChn, VENC_H265_TRANS_S *pstH265Trans)
{
	return platform_venc_get_h265_trans(VeChn, pstH265Trans);
}

CVI_S32 CVI_VENC_SetFrameLostStrategy(VENC_CHN VeChn, const VENC_FRAMELOST_S *pstFrmLostParam)
{
	return platform_venc_set_frame_lost_strategy(VeChn, pstFrmLostParam);
}

CVI_S32 CVI_VENC_GetFrameLostStrategy(VENC_CHN VeChn, VENC_FRAMELOST_S *pstFrmLostParam)
{
	return platform_venc_get_frame_lost_strategy(VeChn, pstFrmLostParam);
}

CVI_S32 CVI_VENC_SetSuperFrameStrategy(VENC_CHN VeChn, const VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	return platform_venc_set_super_frame_strategy(VeChn, pstSuperFrmParam);
}

CVI_S32 CVI_VENC_GetSuperFrameStrategy(VENC_CHN VeChn, VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	return platform_venc_get_super_frame_strategy(VeChn, pstSuperFrmParam);
}

CVI_S32 CVI_VENC_SetChnParam(VENC_CHN VeChn, const VENC_CHN_PARAM_S *pstChnParam)
{
	return platform_venc_set_chn_param(VeChn, pstChnParam);
}

CVI_S32 CVI_VENC_GetChnParam(VENC_CHN VeChn, VENC_CHN_PARAM_S *pstChnParam)
{
	return platform_venc_get_chn_param(VeChn, pstChnParam);
}

CVI_S32 CVI_VENC_SetModParam(const VENC_PARAM_MOD_S *pstModParam)
{
	return platform_venc_set_mod_param(pstModParam);
}

CVI_S32 CVI_VENC_GetModParam(VENC_PARAM_MOD_S *pstModParam)
{
	return platform_venc_get_mod_param(pstModParam);
}

CVI_S32 CVI_VENC_AttachVbPool(VENC_CHN VeChn, const VENC_CHN_POOL_S *pstPool)
{
	return platform_venc_attach_vb_pool(VeChn, pstPool);
}

CVI_S32 CVI_VENC_DetachVbPool(VENC_CHN VeChn)
{
	return platform_venc_detach_vb_pool(VeChn);
}

CVI_S32 CVI_VENC_SetCuPrediction(VENC_CHN VeChn,
		const VENC_CU_PREDICTION_S *pstCuPrediction)
{
	return platform_venc_set_cu_prediction(VeChn, pstCuPrediction);
}

CVI_S32 CVI_VENC_GetCuPrediction(VENC_CHN VeChn, VENC_CU_PREDICTION_S *pstCuPrediction)
{
	return platform_venc_get_cu_prediction(VeChn, pstCuPrediction);
}

CVI_S32 CVI_VENC_CalcFrameParam(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	return platform_venc_calc_frame_param(VeChn, pstFrameParam);
}

CVI_S32 CVI_VENC_SetFrameParam(VENC_CHN VeChn, const VENC_FRAME_PARAM_S *pstFrameParam)
{
	return platform_venc_set_frame_param(VeChn, pstFrameParam);
}

CVI_S32 CVI_VENC_GetFrameParam(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	return platform_venc_get_frame_param(VeChn, pstFrameParam);
}

CVI_S32 CVI_VENC_SetH264Dblk(VENC_CHN VeChn, const VENC_H264_DBLK_S *pstH264Dblk)
{
	return platform_venc_set_h264_dblk(VeChn, pstH264Dblk);
}

CVI_S32 CVI_VENC_GetH264Dblk(VENC_CHN VeChn, VENC_H264_DBLK_S *pstH264Dblk)
{
	return platform_venc_get_h264_dblk(VeChn, pstH264Dblk);
}

CVI_S32 CVI_VENC_SetH265Dblk(VENC_CHN VeChn, const VENC_H265_DBLK_S *pstH265Dblk)
{
	return platform_venc_set_h265_dblk(VeChn, pstH265Dblk);
}

CVI_S32 CVI_VENC_GetH265Dblk(VENC_CHN VeChn, VENC_H265_DBLK_S *pstH265Dblk)
{
	return platform_venc_get_h265_dblk(VeChn, pstH265Dblk);
}

CVI_S32 CVI_VENC_SetH264IntraPred(VENC_CHN VeChn, const VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	return platform_venc_set_h264_intra_pred(VeChn, pstH264IntraPred);
}

CVI_S32 CVI_VENC_GetH264IntraPred(VENC_CHN VeChn, VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	return platform_venc_get_h264_intra_pred(VeChn, pstH264IntraPred);
}

CVI_S32 CVI_VENC_EnableSvc(VENC_CHN VeChn, CVI_BOOL enable)
{
	return platform_venc_enable_svc(VeChn, enable);
}

CVI_S32 CVI_VENC_SetSvcParam(VENC_CHN VeChn, const VENC_SVC_PARAM_S *pstSvcParam)
{

	return platform_venc_set_svc_param(VeChn, pstSvcParam);
}

CVI_S32 CVI_VENC_GetSvcParam(VENC_CHN VeChn, VENC_SVC_PARAM_S *pstSvcParam)
{
	return platform_venc_get_svc_param(VeChn, pstSvcParam);
}

CVI_S32 CVI_VENC_SetDebreathEffect(VENC_CHN VeChn, const VENC_DEBREATHEFFECT_S *pstDebreathEffect)
{
	return platform_venc_set_debreath_effect(VeChn, pstDebreathEffect);
}

CVI_S32 CVI_VENC_GetDebreathEffect(VENC_CHN VeChn, VENC_DEBREATHEFFECT_S *pstDebreathEffect)
{
	return platform_venc_get_debreath_effect(VeChn, pstDebreathEffect);

}