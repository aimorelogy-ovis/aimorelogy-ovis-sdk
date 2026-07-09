#include "cvi_buffer.h"
#include "cvi_errno.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_vpss.h"
#include "cvi_sys.h"
#include "gdc_mesh.h"
// #include "vpss_ioctl.h"
// #include "sys_internal.h"
#include "cvi_debug.h"
// #include "vpss_uapi.h"
#include "msg_vpss.h"
#include "cvi_msg_client.h"

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)

#define GDC_SUPPORT_FMT(fmt)                                                   \
	((fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||           \
	 (fmt == PIXEL_FORMAT_YUV_400))

static inline CVI_S32 CHECK_VPSS_GRP_VALID(VPSS_GRP grp)
{
	if ((grp >= VPSS_MAX_GRP_NUM) || (grp < 0)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssGrp(%d) exceeds Max(%d)\n", grp, VPSS_MAX_GRP_NUM);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_VPSS_CHN_VALID(VPSS_CHN VpssChn)
{
	if ((VpssChn >= VPSS_MAX_CHN_NUM) || (VpssChn < 0)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Chn(%d) invalid.\n", VpssChn);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_VPSS_GDC_FMT(VPSS_GRP grp, VPSS_CHN chn, PIXEL_FORMAT_E fmt)
{
	if (!GDC_SUPPORT_FMT(fmt)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for GDC.\n",
				grp, chn, fmt);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static CVI_S32 _vpss_update_rotation_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
	MSG_PRIV_DATA_S stPrivData;

	UNUSED(u32Width);
	UNUSED(u32Height);

	stPrivData.as32PrivData[0] = enRotation;

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_ROTATION, CVI_NULL, 0, &stPrivData, 30000);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d ChnRotation fail s32Ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

static CVI_S32 _vpss_update_ldc_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U64 paddr = 0;
	CVI_VOID *vaddr;
	CVI_S32 s32Ret;
	char mesh_name[128];
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
	MSG_PRIV_DATA_S stPrivData = {0};

	if (!pstLDCAttr->bEnable) {
		if (enRotation != ROTATION_0)
			return _vpss_update_rotation_mesh(VpssGrp, VpssChn, enRotation,
				u32Width, u32Height);
		else {
			s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_LDCATTR, (CVI_VOID *)pstLDCAttr,
						sizeof(VPSS_LDC_ATTR_S), &stPrivData, 30000);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d ChnLdcAttr fail s32Ret:%d\n",
					VpssGrp, VpssChn, s32Ret);
			}
			return s32Ret;
		}
	}

	snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
	s32Ret = gdc_gen_ldcmesh(u32Width, u32Height, &pstLDCAttr->stAttr,
				mesh_name, &paddr, &vaddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) gen mesh fail\n",
				VpssGrp, VpssChn);
		return s32Ret;
	}

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "Grp(%d) Chn(%d) mesh base(%p) vaddr(%p)\n"
		      , VpssGrp, VpssChn, (void *)(uintptr_t)paddr, (void *)vaddr);

	stPrivData.as32PrivData[0] = (CVI_S32)(paddr & 0xFFFFFFF);
	stPrivData.as32PrivData[1] = (CVI_S32)((paddr >> 28) & 0xFFFFFFF);
	stPrivData.as32PrivData[2] = (CVI_S32)((paddr >> 56) & 0xFF);

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_LDCATTR, (CVI_VOID *)pstLDCAttr,
				   sizeof(VPSS_LDC_ATTR_S), &stPrivData, 30000);
	return s32Ret;
}

/************************vpss Settings**********************************/
CVI_S32 platform_vpss_setmode(const VPSS_MODE_S *pstVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_MODE, (CVI_VOID *)pstVPSSMode,
				sizeof(VPSS_MODE_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set mode fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getmode(VPSS_MODE_S *pstVPSSMode)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVPSSMode);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_MODE, pstVPSSMode, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get mode fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/************************grp Settings**********************************/
CVI_S32 platform_vpss_creategrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_CREATE, (CVI_VOID *)pstGrpAttr,
				sizeof(VPSS_GRP_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Create group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_destroygrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DESTROY, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Destroy group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

VPSS_GRP platform_vpss_getavailablegrp(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_AVAILABLE_GRP, CVI_NULL, 0, CVI_NULL);

	return s32Ret;
}

CVI_S32 platform_vpss_startgrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_START, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Start group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_stopgrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_STOP, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Stop group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_resetgrp(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_RESET, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Reset group fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpattr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_ATTR, pstGrpAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get grp attr fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpattr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_ATTR, (CVI_VOID *)pstGrpAttr,
				sizeof(VPSS_GRP_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set grp attr fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpcrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_CROP, pstCropInfo, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get grp crop fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpcrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(VPSS_CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set grp crop fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}


	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_sendframe(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);

	stPrivData.as32PrivData[0] = s32MilliSec;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SEND_FRAME, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Send chn frame fail,VpssGrp:%d, s32Ret:%x\n",
			VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpprocampctrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, ctrl);

	stPrivData.as32PrivData[0] = type;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_PROCAMPCTRL, (CVI_VOID *)ctrl, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get Grp ProcAmpCtrl fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, value);

	stPrivData.as32PrivData[0] = type;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_PROCAMP, value, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get VpssGrp:%d ProcAmp fail s32Ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 value)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	stPrivData.as32PrivData[0] = type;
	stPrivData.as32PrivData[1] = value;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_PROCAMP, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d ProcAmp fail s32Ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getallprocamp(VPSS_ALL_PROC_AMP_S *pstProcAmp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, 0, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstProcAmp);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_ALL_GRP_PROCAMP, pstProcAmp, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR,  "VPSS get all proc amp fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/* platform_vpss_setgrpparamfrombin: Apply the settings from bin
 *
 * @param VpssGrp: the vpss grp to apply
 * @param bin_data: VPSS_BIN_DATA pointer from the caller
 * @return: result of the API
 */
CVI_S32 platform_vpss_setgrpparamfrombin(VPSS_GRP VpssGrp, VPSS_BIN_DATA *bin_data)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_GRP_PQBIN, (CVI_VOID *)bin_data,
				sizeof(VPSS_BIN_DATA), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set GRP PQBin fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}
	CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is exist, vpss grp param use pqbin value in Linux !!\n");

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getbinscene(VPSS_GRP VpssGrp, CVI_U8 *scene)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, scene);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_GRP_SCENE, scene, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get BinScene fail,s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/* Chn Settings */
CVI_S32 platform_vpss_setchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VPSS_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d attr fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ATTR, pstChnAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn attr fail,VpssGrp:%d Chn:%d,s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_enablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_ENABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Enable Grp:%d Chn:%d Fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_disablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
	CVI_CHAR mesh_name[128];

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DISABLE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Disable Grp:%d Chn:%d Fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
	gdc_free_cur_tsk_mesh(mesh_name);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(VPSS_CROP_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d Crop fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_CROP, pstCropInfo, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get VpssGrp:%d Chn:%d Crop fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (enRotation == ROTATION_180) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VI_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid rotation(%d).\n"
			, VpssGrp, VpssChn, enRotation);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	stPrivData.as32PrivData[0] = enRotation;

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VPSS_SET_CHN_ROTATION, CVI_NULL, 0, &stPrivData, 30000);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d ChnRotation fail s32Ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vpss_getchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penRotation);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ROTATION, penRotation, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get VpssGrp:%d Chn:%d ChnRotation fail s32Ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);
	VPSS_CHN_ATTR_S stChnAttr;
	ROTATION_E enRotation;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ATTR, &stChnAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn attr fail,VpssGrp:%d Chn:%d,s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	s32Ret = CHECK_VPSS_GDC_FMT(VpssGrp, VpssChn, stChnAttr.enPixelFormat);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ROTATION, &enRotation, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get VpssGrp:%d Chn:%d ChnRotation fail s32Ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return _vpss_update_ldc_mesh(VpssGrp, VpssChn, pstLDCAttr, enRotation,
				stChnAttr.u32Width, stChnAttr.u32Height);
}

CVI_S32 platform_vpss_getchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_LDCATTR, pstLDCAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get grp attr fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_sendchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	stPrivData.as32PrivData[0] = s32MilliSec;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SEND_CHN_FRAME, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Send chn frame fail,VpssGrp:%d, VpssChn:%d, s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
			CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VPSS, VpssGrp, VpssChn, 1);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstFrameInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (s32MilliSec == -1) {
		stPrivData.as32PrivData[0] = CVI_IPCMSG_SEND_SYNC_TIMEOUT - 100;
	} else {
		stPrivData.as32PrivData[0] = s32MilliSec;
	}

	do {
		s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_FRAME, (CVI_VOID *)pstFrameInfo,
					0, &stPrivData);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn frame timeout,VpssGrp:%d, VpssChn:%d, s32Ret:%x\n",
				VpssGrp, VpssChn, s32Ret);
		} else {
			break;
		}
	} while (s32MilliSec == -1);

	return s32Ret;
}

CVI_S32 platform_vpss_releasechnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
 	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_RELEASE_CHN_FRAME, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Release chn frame fail,VpssGrp:%d, VpssChn:%d, s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_triggersnapframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32FrameCnt)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	stPrivData.as32PrivData[0] = u32FrameCnt;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_TRIGGER_SNAP_FRAME, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssTriggerSnapFrame Grp:%d Chn:%d fail ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_attachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	stPrivData.as32PrivData[0] = hVbPool;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_ATTACH_VBPOOL, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssAttachVbPool Grp:%d Chn:%d fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_detachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DETACH_VBPOOL, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssDetachVbPool Grp:%d Chn:%d fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align)
{
	CVI_S32 s32Ret;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	stPrivData.as32PrivData[0] = u32Align;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_ALIGN, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssSetChnAlign Grp:%d Chn:%d fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pu32Align);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_ALIGN, pu32Align, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn Align fail,VpssGrp:%d chn:%d s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/* platform_vpss_SetChnYRatio: Modify the y ratio of chn output. Only work for yuv format.
 *
 * @param VpssGrp: The Vpss Grp to work.
 * @param VpssChn: The Vpss Chn to work.
 * @param YRatio: Output's Y will be sacled by this ratio.
 * @return: CVI_SUCCESS if OK.
 */
CVI_S32 platform_vpss_setchnyratio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_YRATIO, (CVI_VOID *)&YRatio,
		sizeof(CVI_FLOAT), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssSetChnYRatio Grp:%d Chn:%d fail s32Ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnyratio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pYRatio);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_YRATIO, pYRatio, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn YRatio fail,VpssGrp:%d chn:%d s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_SCALECOEF, (CVI_VOID *)&enCoef,
		sizeof(VPSS_SCALE_COEF_E), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssSetChnScaleCoefLevel Grp:%d Chn:%d fail ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penCoef);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_SCALECOEF, penCoef, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn ScaleCoefLevel fail,VpssGrp:%d chn:%d s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_DRAW_RECT_S *pstDrawRect)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstDrawRect);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_DRAW_RECT, (CVI_VOID *)pstDrawRect,
				sizeof(VPSS_DRAW_RECT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set chn draw rect fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_DRAW_RECT_S *pstDrawRect)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstDrawRect);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_DRAW_RECT, pstDrawRect, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn draw rect fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstConvert);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_CONVERT, (CVI_VOID *)pstConvert,
				sizeof(VPSS_CONVERT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set chn convert fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstConvert);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_CONVERT, pstConvert, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn convert fail,VpssGrp:%d,s32Ret:%x\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_CHN_BUFWRAPATTR, (CVI_VOID *)pstVpssChnBufWrap,
				sizeof(VPSS_CHN_BUF_WRAP_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set VpssGrp:%d Chn:%d BufWrapAttr fail ret:%d\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_CHN_BUFWRAPATTR, pstVpssChnBufWrap, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get chn BufWrapAttr fail,VpssGrp:%d Chn:%d,s32Ret:%x\n",
			VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_showchn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SHOW_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssShowChn Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_hidechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, VpssChn);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_HIDE_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssHideChn Grp:%d Chn:%d fail ret:%d\n", VpssGrp, VpssChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnfd(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	UNUSED(VpssGrp);
	UNUSED(VpssChn);
	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_closefd(CVI_VOID)
{
	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_createstitch(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstStitchAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_CREATE_STITCH, (CVI_VOID *)pstStitchAttr,
		sizeof(CVI_STITCH_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "CreateStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_destroystitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_DESTROY_STITCH, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "DestroyStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setstitchattr(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstStitchAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_SET_STITCH, (CVI_VOID *)pstStitchAttr,
		sizeof(CVI_STITCH_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "SetStitchAttr Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getstitchattr(VPSS_GRP VpssGrp, CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstStitchAttr);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_GET_STITCH, pstStitchAttr, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "GetStitchAttr Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_startstitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_START_STITCH, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "StartStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_stopstitch(VPSS_GRP VpssGrp)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VPSS, VpssGrp, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VPSS_STOP_STITCH, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "StopStitch Grp:%d ret:%d\n", VpssGrp, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

