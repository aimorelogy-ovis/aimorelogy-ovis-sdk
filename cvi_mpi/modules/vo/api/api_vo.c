#include "cvi_buffer.h"
#include "cvi_debug.h"
#include "cvi_errno.h"

#include "platform_vo.h"

CVI_S32 CVI_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr)
{
	return Platform_VO_SetPubAttr(VoDev, pstPubAttr);
}

CVI_S32 CVI_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	return Platform_VO_GetPubAttr(VoDev, pstPubAttr);
}

CVI_S32 CVI_VO_SetLVDSParam(VO_DEV VoDev, const VO_LVDS_ATTR_S *pstLVDSParam)
{
	return Platform_VO_SetLVDSParam(VoDev, pstLVDSParam);
}

CVI_S32 CVI_VO_GetLVDSParam(VO_DEV VoDev, VO_LVDS_ATTR_S *pstLVDSParam)
{
	return Platform_VO_GetLVDSParam(VoDev, pstLVDSParam);
}

CVI_S32 CVI_VO_SetBTParam(VO_DEV VoDev, const VO_BT_ATTR_S *pstBTParam)
{
	return Platform_VO_SetBTParam(VoDev, pstBTParam);
}

CVI_S32 CVI_VO_GetBTParam(VO_DEV VoDev, VO_BT_ATTR_S *pstBTParam)
{
	return Platform_VO_GetBTParam(VoDev, pstBTParam);
}

CVI_S32 CVI_VO_Enable(VO_DEV VoDev)
{
	return Platform_VO_Enable(VoDev);
}

CVI_S32 CVI_VO_Disable(VO_DEV VoDev)
{
	return Platform_VO_Disable(VoDev);
}

CVI_S32 CVI_VO_EnableVideoLayer(VO_LAYER VoLayer)
{
	return Platform_VO_EnableVideoLayer(VoLayer);
}

CVI_S32 CVI_VO_DisableVideoLayer(VO_LAYER VoLayer)
{
	return Platform_VO_DisableVideoLayer(VoLayer);
}

CVI_S32 CVI_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	return Platform_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
}

CVI_S32 CVI_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	return Platform_VO_GetVideoLayerAttr(VoLayer, pstLayerAttr);
}

CVI_S32 CVI_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	return Platform_VO_GetLayerProcAmpCtrl(VoLayer, type, ctrl);
}

CVI_S32 CVI_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value)
{
	return Platform_VO_GetLayerProcAmp(VoLayer, type, value);
}

CVI_S32 CVI_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value)
{
	return Platform_VO_SetLayerProcAmp(VoLayer, type, value);
}

CVI_S32 CVI_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr)
{
	return Platform_VO_SetChnAttr(VoLayer, VoChn, pstChnAttr);
}

CVI_S32 CVI_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr)
{
	return Platform_VO_GetChnAttr(VoLayer, VoChn, pstChnAttr);
}

CVI_S32 CVI_VO_SetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 s32ChnFrmRate)
{
	return Platform_VO_SetChnFrameRate(VoLayer, VoChn, s32ChnFrmRate);
}

CVI_S32 CVI_VO_GetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 *ps32ChnFrmRate)
{
	return Platform_VO_GetChnFrameRate(VoLayer, VoChn, ps32ChnFrmRate);
}

CVI_S32 CVI_VO_GetChnPTS(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U64 *pu64ChnPTS)
{
	return Platform_VO_GetChnPTS(VoLayer, VoChn, pu64ChnPTS);
}

CVI_S32 CVI_VO_QueryChnStatus(VO_LAYER VoLayer, VO_CHN VoChn, VO_QUERY_STATUS_S *pstStatus)
{
	return Platform_VO_QueryChnStatus(VoLayer, VoChn, pstStatus);
}

CVI_S32 CVI_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen)
{
	return Platform_VO_SetDisplayBufLen(VoLayer, u32BufLen);
}

CVI_S32 CVI_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen)
{
	return Platform_VO_GetDisplayBufLen(VoLayer, pu32BufLen);
}

CVI_S32 CVI_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	return Platform_VO_EnableChn(VoLayer, VoChn);
}

CVI_S32 CVI_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	return Platform_VO_DisableChn(VoLayer, VoChn);
}

CVI_S32 CVI_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation)
{
	return Platform_VO_SetChnRotation(VoLayer, VoChn, enRotation);
}

CVI_S32 CVI_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation)
{
	return Platform_VO_GetChnRotation(VoLayer, VoChn, penRotation);
}

CVI_S32 CVI_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	return Platform_VO_SendFrame(VoLayer, VoChn, pstVideoFrame, s32MilliSec);
}

CVI_S32 CVI_VO_ShowPattern(VO_DEV VoDev, VO_PATTERN_MODE PatternId)
{
	return Platform_VO_ShowPattern(VoDev, PatternId);
}

CVI_S32 CVI_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll)
{
	return Platform_VO_ClearChnBuf(VoLayer, VoChn, bClrAll);
}

CVI_S32 CVI_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	return Platform_VO_ShowChn(VoLayer, VoChn);
}

CVI_S32 CVI_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	return Platform_VO_HideChn(VoLayer, VoChn);
}

CVI_S32 CVI_VO_CloseFd(void)
{
	return Platform_VO_CloseFd();
}

CVI_S32 CVI_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	return Platform_VO_PauseChn(VoLayer, VoChn);
}

CVI_S32 CVI_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	return Platform_VO_ResumeChn(VoLayer, VoChn);
}

CVI_S32 CVI_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init)
{
	return Platform_VO_Get_Panel_Status(VoLayer, VoChn, is_init);
}

CVI_S32 CVI_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData)
{
	return Platform_VO_RegPmCallBack(VoDev, pstPmOps, pvData);
}

CVI_S32 CVI_VO_UnRegPmCallBack(VO_DEV VoDev)
{
	return Platform_VO_UnRegPmCallBack(VoDev);
}

CVI_BOOL CVI_VO_IsEnabled(VO_DEV VoDev)
{
	return Platform_VO_IsEnabled(VoDev);
}

CVI_S32 CVI_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	return Platform_VO_SetGammaInfo(pinfo);
}

CVI_S32 CVI_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	return Platform_VO_GetGammaInfo(pinfo);
}

CVI_S32 CVI_VO_Suspend(void)
{
	return Platform_VO_Suspend();
}

CVI_S32 CVI_VO_Resume(void)
{
	return Platform_VO_Resume();
}