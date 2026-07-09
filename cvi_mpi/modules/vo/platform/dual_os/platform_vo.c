#include "cvi_debug.h"
#include "cvi_errno.h"
#include "cvi_common.h"
#include "cvi_comm_vo.h"
#include "msg_vo.h"
#include "cvi_msg_client.h"

#define CHECK_VO_DEV_VALID(VoDev) do {									\
		if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {						\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);			\
			return CVI_ERR_VO_INVALID_DEVID;						\
		}											\
	} while (0)

#define CHECK_VO_LAYER_VALID(VoLayer) do {								\
		if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {					\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);			\
			return CVI_ERR_VO_INVALID_LAYERID;						\
		}											\
	} while (0)

#define CHECK_VO_CHN_VALID(VoLayer, VoChn) do {							\
		if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {					\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);			\
			return CVI_ERR_VO_INVALID_LAYERID;						\
		}											\
		if ((VoChn >= VO_MAX_CHN_NUM) || (VoChn < 0)) {						\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoChn(%d) invalid.\n", VoChn);			\
			return CVI_ERR_VO_INVALID_CHNID;						\
		}											\
	} while (0)

#define CHECK_VO_NULL_PTR(ptr)							\
	do {									\
		if (ptr == NULL) {						\
			CVI_TRACE_VO(CVI_DBG_ERR, " Invalid null pointer\n");	\
			return CVI_ERR_VO_NULL_PTR;				\
		}								\
	} while (0)

struct vo_pm_s {
	VO_PM_OPS_S	stOps;
	CVI_VOID	*pvData;
};
static struct vo_pm_s apstVoPm[VO_MAX_DEV_NUM] = { 0 };

static CVI_S32 _check_vo_exist(CVI_VOID)
{
	return CVI_SUCCESS;
}

/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/
CVI_S32 Platform_VO_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_Suspend fail\n");
		return s32Ret;
    	}

	for (VO_DEV VoDev = 0; VoDev < VO_MAX_DEV_NUM; ++VoDev) {
		if (apstVoPm[VoDev].stOps.pfnPanelSuspend) {
			s32Ret = apstVoPm[VoDev].stOps.pfnPanelSuspend(apstVoPm[VoDev].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VO(CVI_DBG_ERR, "Panel[%d] suspend failed with %#x!\n", VoDev, s32Ret);
				return s32Ret;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_Resume fail\n");
		return s32Ret;
   	}

	for (VO_DEV VoDev = 0; VoDev < VO_MAX_DEV_NUM; ++VoDev) {
		if (apstVoPm[VoDev].stOps.pfnPanelResume) {
			s32Ret = apstVoPm[VoDev].stOps.pfnPanelResume(apstVoPm[VoDev].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VO(CVI_DBG_ERR, "Panel[%d] resume failed with %#x!\n", VoDev, s32Ret);
				return s32Ret;
			}
		}
	}

	return CVI_SUCCESS;
}
/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 Platform_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr)
{
	CHECK_VO_NULL_PTR(pstPubAttr);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_PUBATTR, (CVI_VOID *)pstPubAttr, sizeof(*pstPubAttr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	CHECK_VO_NULL_PTR(pstPubAttr);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_PUBATTR, pstPubAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetLVDSParam(VO_DEV VoDev, const VO_LVDS_ATTR_S *pstLVDSParam)
{
	CHECK_VO_NULL_PTR(pstLVDSParam);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SETLVDSPARAM, (CVI_VOID *)pstLVDSParam, sizeof(*pstLVDSParam), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetLVDSParam(VO_DEV VoDev, VO_LVDS_ATTR_S *pstLVDSParam)
{
	CHECK_VO_NULL_PTR(pstLVDSParam);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GETLVDSPARAM, pstLVDSParam, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetBTParam(VO_DEV VoDev, const VO_BT_ATTR_S *pstBTParam)
{
	CHECK_VO_NULL_PTR(pstBTParam);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SETBTPARAM, (CVI_VOID *)pstBTParam, sizeof(*pstBTParam), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetBTParam(VO_DEV VoDev, VO_BT_ATTR_S *pstBTParam)
{
	CHECK_VO_NULL_PTR(pstBTParam);
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GETBTPARAM, pstBTParam, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetPubAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_Enable(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_ENABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_Enable fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_Disable(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DISABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_Disable fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_EnableVideoLayer(VO_LAYER VoLayer)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_ENABLE_VIDEOLAYER, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_EnableVideoLayer fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_DisableVideoLayer(VO_LAYER VoLayer)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DISABLE_VIDEOLAYER, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_DisableVideoLayer fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	CHECK_VO_NULL_PTR(pstLayerAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_VIDEOLAYERATTR, (CVI_VOID *)pstLayerAttr, sizeof(*pstLayerAttr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetVideoLayerAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	CHECK_VO_NULL_PTR(pstLayerAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_VIDEOLAYERATTR, pstLayerAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetVideoLayerAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr)
{
	CHECK_VO_NULL_PTR(pstChnAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_CHNATTR, (CVI_VOID *)pstChnAttr, sizeof(*pstChnAttr), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetChnAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr)
{
	CHECK_VO_NULL_PTR(pstChnAttr);
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_CHNATTR, pstChnAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetChnAttr fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 s32ChnFrmRate)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = s32ChnFrmRate;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SETCHNFRAMERATE, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetDisplayBufLen fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 *ps32ChnFrmRate)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GETCHNFRAMERATE, ps32ChnFrmRate, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetChnFrameRate fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetChnPTS(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U64 *pu64ChnPTS)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GETCHNPTS, pu64ChnPTS, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetChnPTS fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_QueryChnStatus(VO_LAYER VoLayer, VO_CHN VoChn, VO_QUERY_STATUS_S *pstStatus)
{
	CHECK_VO_NULL_PTR(pstStatus);
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_QUERYCHNSTATUS, pstStatus, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_QueryChnStatus fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);
	MSG_PRIV_DATA_S stPrivData;

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = u32BufLen;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_DISPLAYBUFLEN, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetDisplayBufLen fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_DISPLAYBUFLEN, pu32BufLen, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetDisplayBufLen fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_ENABLE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_EnableChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DISABLE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_DisableChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = enRotation;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SET_CHNROTATION, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetChnRotation fail ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GET_CHNROTATION, penRotation, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetChnRotation fail ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CHECK_VO_NULL_PTR(pstVideoFrame);
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);
	MSG_PRIV_DATA_S stPrivData;

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SEND_FRAME, (CVI_VOID *)pstVideoFrame, sizeof(*pstVideoFrame), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SendFrame fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = bClrAll;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_CLEAR_CHNBUF, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_ClearChnBuf fail ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SHOW_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_ShowChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_HIDE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_HideChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_CloseFd(void)
{
	CVI_TRACE_VO(CVI_DBG_ERR, "dual os not support close vo fd in linux.\n");
	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_PAUSE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_PauseChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, VoChn);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_RESUME_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_ResumeChn fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init)
{
	(void)VoLayer;
	(void)VoChn;
	(void)is_init;
	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData)
{
	(void)VoDev;
	(void)pstPmOps;
	(void)pvData;
	// CHECK_VO_DEV_VALID(VoDev);
	// CHECK_VO_NULL_PTR(pstPmOps);

	// apstVoPm[VoDev].stOps = *pstPmOps;
	// apstVoPm[VoDev].pvData = pvData;
	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_UnRegPmCallBack(VO_DEV VoDev)
{
	(void)VoDev;
	// CHECK_VO_DEV_VALID(VoDev);

	// memset(&apstVoPm[VoDev].stOps, 0, sizeof(apstVoPm[VoDev].stOps));
	// apstVoPm[VoDev].pvData = NULL;
	return CVI_SUCCESS;
}

CVI_BOOL Platform_VO_IsEnabled(VO_DEV VoDev)
{
	CVI_S32 s32Ret;
	CVI_BOOL bIsEnable;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);
		return CVI_FALSE;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_DEV_IS_ENABLE, &bIsEnable, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_IsEnabled fail,s32Ret:%x\n", s32Ret);
		return CVI_FALSE;
	}

	return bIsEnable;
}

CVI_S32 Platform_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	CHECK_VO_NULL_PTR(pinfo);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GAMMA_LUT_UPDATE, (CVI_VOID *)pinfo, sizeof(*pinfo), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetGammaInfo fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	CHECK_VO_NULL_PTR(pinfo);
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, 0, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GAMMA_LUT_READ, pinfo, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetGammaInfo fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_ShowPattern(VO_DEV VoDev, VO_PATTERN_MODE PatternId)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoDev, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = PatternId;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SHOW_PATTERN, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "ShowPattern fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = type;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GETLAYERPROCAMPCTRL, ctrl, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetLayerProcAmpCtrl fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = type;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_GETLAYERPROCAMP, value, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_GetLayerProcAmpCtrl fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 Platform_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value)
{
	CHECK_VO_LAYER_VALID(VoLayer);
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VO, VoLayer, 0);

	if (_check_vo_exist())
		return CVI_ERR_VO_NOT_SUPPORT;

	stPrivData.as32PrivData[0] = type;
	stPrivData.as32PrivData[1] = value;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VO_SETLAYERPROCAMP, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "CVI_VO_SetChnRotation fail ret:%d\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}
