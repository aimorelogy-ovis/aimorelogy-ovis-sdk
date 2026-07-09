#ifndef __PLATFORM_VO_H__
#define __PLATFORM_VO_H__

#include "cvi_comm_vo.h"

CVI_S32 Platform_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr);
CVI_S32 Platform_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);
CVI_S32 Platform_VO_SetLVDSParam(VO_DEV VoDev, const VO_LVDS_ATTR_S *pstLVDSParam);
CVI_S32 Platform_VO_GetLVDSParam(VO_DEV VoDev, VO_LVDS_ATTR_S *pstLVDSParam);
CVI_S32 Platform_VO_SetBTParam(VO_DEV VoDev, const VO_BT_ATTR_S *pstBTParam);
CVI_S32 Platform_VO_GetBTParam(VO_DEV VoDev, VO_BT_ATTR_S *pstBTParam);
CVI_S32 Platform_VO_Enable(VO_DEV VoDev);
CVI_S32 Platform_VO_Disable(VO_DEV VoDev);
CVI_S32 Platform_VO_EnableVideoLayer(VO_LAYER VoLayer);
CVI_S32 Platform_VO_DisableVideoLayer(VO_LAYER VoLayer);
CVI_S32 Platform_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);
CVI_S32 Platform_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);
CVI_S32 Platform_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl);
CVI_S32 Platform_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value);
CVI_S32 Platform_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value);
CVI_S32 Platform_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr);
CVI_S32 Platform_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr);
CVI_S32 Platform_VO_SetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 s32ChnFrmRate);
CVI_S32 Platform_VO_GetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 *ps32ChnFrmRate);
CVI_S32 Platform_VO_GetChnPTS(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U64 *pu64ChnPTS);
CVI_S32 Platform_VO_QueryChnStatus(VO_LAYER VoLayer, VO_CHN VoChn, VO_QUERY_STATUS_S *pstStatus);
CVI_S32 Platform_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen);
CVI_S32 Platform_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen);
CVI_S32 Platform_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 Platform_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 Platform_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation);
CVI_S32 Platform_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation);
CVI_S32 Platform_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);
CVI_S32 Platform_VO_ShowPattern(VO_DEV VoDev, VO_PATTERN_MODE PatternId);
CVI_S32 Platform_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll);
CVI_S32 Platform_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 Platform_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 Platform_VO_CloseFd(void);
CVI_S32 Platform_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 Platform_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 Platform_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init);
CVI_S32 Platform_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData);
CVI_S32 Platform_VO_UnRegPmCallBack(VO_DEV VoDev);
CVI_BOOL Platform_VO_IsEnabled(VO_DEV VoDev);
CVI_S32 Platform_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo);
CVI_S32 Platform_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo);
CVI_S32 Platform_VO_Suspend(void);
CVI_S32 Platform_VO_Resume(void);

#endif
