#include <cvi_base.h>
#include <cvi_base_ctx.h>
#include <cvi_defines.h>
#include <cvi_common.h>
#include <cvi_vip.h>
#include <cvi_buffer.h>

#include <vo.h>
#include <vo_cb.h>

static CVI_S32 _check_vo_status(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CVI_S32 ret = CVI_SUCCESS;

	ret = CHECK_VO_CHN_VALID(VoLayer, VoChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = CHECK_VO_CHN_ENABLE(VoLayer, VoChn);
	if (ret != CVI_SUCCESS)
		return ret;
	return ret;
}

extern struct cvi_vo_ctx *gVoCtx;

CVI_S32 vo_cb_get_rgn_hdls(VO_LAYER VoLayer, VO_CHN VoChn, RGN_HANDLE hdls[])
{
	CVI_S32 ret, i;

	ret = MOD_CHECK_NULL_PTR(CVI_ID_VO, hdls);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = _check_vo_status(VoLayer, VoChn);
	if (ret != CVI_SUCCESS)
		return ret;

	for (i = 0; i < RGN_MAX_NUM_VO; ++i)
		hdls[i] = gVoCtx->rgn_handle[i];

	return CVI_SUCCESS;
}

CVI_S32 vo_cb_set_rgn_hdls(VO_LAYER VoLayer, VO_CHN VoChn, RGN_HANDLE hdls[])
{
	CVI_S32 ret, i;

	ret = MOD_CHECK_NULL_PTR(CVI_ID_VO, hdls);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = _check_vo_status(VoLayer, VoChn);
	if (ret != CVI_SUCCESS)
		return ret;

	for (i = 0; i < RGN_MAX_NUM_VPSS; ++i)
		gVoCtx->rgn_handle[i] = hdls[i];

	return CVI_SUCCESS;
}

CVI_S32 vo_cb_set_rgn_cfg(VO_LAYER VoLayer, VO_CHN VoChn, struct cvi_rgn_cfg *cfg)
{
	CVI_S32 ret;

	ret = MOD_CHECK_NULL_PTR(CVI_ID_VO, cfg);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = _check_vo_status(VoLayer, VoChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memcpy(&gVoCtx->rgn_cfg, cfg, sizeof(*cfg));

	return CVI_SUCCESS;
}

CVI_S32 vo_cb_get_chn_size(VO_LAYER VoLayer, VO_CHN VoChn, RECT_S *rect)
{
	CVI_S32 ret;

	ret = MOD_CHECK_NULL_PTR(CVI_ID_VO, rect);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = _check_vo_status(VoLayer, VoChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memcpy(rect, &gVoCtx->stChnAttr.stRect, sizeof(*rect));

	return CVI_SUCCESS;
}
