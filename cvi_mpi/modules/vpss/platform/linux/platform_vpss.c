#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <math.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/mman.h>

#include "cvi_buffer.h"
#include "cvi_errno.h"
#include "cvi_comm_vb.h"
#include "cvi_sys.h"
#include "gdc_mesh.h"
#include "vpss_ioctl.h"
#include "sys_internal.h"

struct cvi_gdc_mesh mesh[VPSS_MAX_GRP_NUM][VPSS_MAX_CHN_NUM];

static CVI_S32 vpss_fd = -1;
static pthread_mutex_t vpss_fd_lock = PTHREAD_MUTEX_INITIALIZER;

static CVI_S32 vpss_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&vpss_fd_lock);
	close_device(&vpss_fd);
	pthread_mutex_unlock(&vpss_fd_lock);

	return CVI_SUCCESS;
}

static CVI_S32 get_vpss_fd(CVI_VOID)
{
	pthread_mutex_lock(&vpss_fd_lock);
	if (vpss_fd <= 0) {
		if (open_device(VPSS_DEV_NAME, &vpss_fd) == -1) {
			perror("VPSS open fail\n");
			vpss_fd = -1;
		}
	}
	pthread_mutex_unlock(&vpss_fd_lock);

	return vpss_fd;
}

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
	CVI_S32 fd = get_vpss_fd();
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];
	struct vpss_chn_rot_cfg cfg;

	UNUSED(u32Width);
	UNUSED(u32Height);
	// TODO: dummy settings
	pmesh->paddr = DEFAULT_MESH_PADDR;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enRotation = enRotation;
	return vpss_set_chn_rotation(fd, &cfg);
}

static CVI_S32 _vpss_update_ldc_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U64 paddr = 0;
	CVI_VOID *vaddr;
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];
	CVI_S32 s32Ret;
	char mesh_name[128];

	if (!pstLDCAttr->bEnable) {
		if (enRotation != ROTATION_0)
			return _vpss_update_rotation_mesh(VpssGrp, VpssChn, enRotation,
				u32Width, u32Height);
		else {
			CVI_S32 fd = get_vpss_fd();
			struct vpss_chn_ldc_cfg cfg;

			cfg.VpssGrp = VpssGrp;
			cfg.VpssChn = VpssChn;
			cfg.enRotation = enRotation;
			cfg.stLDCAttr = *pstLDCAttr;
			cfg.meshHandle = paddr;
			return vpss_set_chn_ldc(fd, &cfg);
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

	pthread_mutex_lock(&pmesh->lock);
	pmesh->paddr = paddr;
	pmesh->vaddr = vaddr;
	pthread_mutex_unlock(&pmesh->lock);

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "Grp(%d) Chn(%d) mesh base(%#"PRIx64") vaddr(%p)\n"
		      , VpssGrp, VpssChn, paddr, vaddr);

	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_ldc_cfg cfg;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enRotation = enRotation;
	cfg.stLDCAttr = *pstLDCAttr;
	cfg.meshHandle = paddr;
	return vpss_set_chn_ldc(fd, &cfg);
}

static void _vpss_proamp_2_csc(struct vpss_grp_csc_cfg *csc_cfg)
{
	// for grp proc-amp.
	CVI_S32 *proc_amp = csc_cfg->proc_amp;
	float h = (float)(proc_amp[PROC_AMP_HUE] - 50) * PI / 360;
	float b_off = (proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
	float C_gain = 1 + (proc_amp[PROC_AMP_CONTRAST] - 50) * 0.02;
	float S = 1 + (proc_amp[PROC_AMP_SATURATION] - 50) * 0.02;
	float A = cos(h) * C_gain * S;
	float B = sin(h) * C_gain * S;
	float C_diff, c_off, tmp;
	CVI_U8 sub_0_l, add_0_l, add_1_l, add_2_l;

	if (proc_amp[PROC_AMP_CONTRAST] > 50)
		C_diff = 256 / C_gain;
	else
		C_diff = 256 * C_gain;
	c_off = 128 - (C_diff/2);

	if (b_off < 0) {
		sub_0_l = abs(proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
		add_0_l = 0;
		add_1_l = 0;
		add_2_l = 0;
	} else {
		sub_0_l = 0;
		if ((C_gain * b_off) > 255) {
			add_0_l = 255;
		} else {
			add_0_l = C_gain * b_off;
		}
		add_1_l = add_0_l;
		add_2_l = add_0_l;
	}

	if (proc_amp[PROC_AMP_CONTRAST] > 50) {
		csc_cfg->sub[0] = sub_0_l + c_off;
		csc_cfg->add[0] = add_0_l;
		csc_cfg->add[1] = add_1_l;
		csc_cfg->add[2] = add_2_l;
	} else {
		csc_cfg->sub[0] = sub_0_l;
		csc_cfg->add[0] = add_0_l + c_off;
		csc_cfg->add[1] = add_1_l + c_off;
		csc_cfg->add[2] = add_2_l + c_off;
	}
	csc_cfg->sub[1] = 128;
	csc_cfg->sub[2] = 128;

	csc_cfg->coef[0][0] = C_gain * BIT(10);
	tmp = B * -1.402;
	csc_cfg->coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = A * 1.402;
	csc_cfg->coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	csc_cfg->coef[1][0] = C_gain * BIT(10);
	tmp = A * -0.344 + B * 0.714;
	csc_cfg->coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = B * -0.344 + A * -0.714;
	csc_cfg->coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	csc_cfg->coef[2][0] = C_gain * BIT(10);
	tmp = A * 1.772;
	csc_cfg->coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = B * 1.772;
	csc_cfg->coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[0][0]: %#4x coef[0][1]: %#4x coef[0][2]: %#4x\n"
		, csc_cfg->coef[0][0], csc_cfg->coef[0][1]
		, csc_cfg->coef[0][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[1][0]: %#4x coef[1][1]: %#4x coef[1][2]: %#4x\n"
		, csc_cfg->coef[1][0], csc_cfg->coef[1][1]
		, csc_cfg->coef[1][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[2][0]: %#4x coef[2][1]: %#4x coef[2][2]: %#4x\n"
		, csc_cfg->coef[2][0], csc_cfg->coef[2][1]
		, csc_cfg->coef[2][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "sub[0]: %3d sub[1]: %3d sub[2]: %3d\n"
		, csc_cfg->sub[0], csc_cfg->sub[1], csc_cfg->sub[2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "add[0]: %3d add[1]: %3d add[2]: %3d\n"
		, csc_cfg->add[0], csc_cfg->add[1], csc_cfg->add[2]);
}

static CVI_VOID _vpss_check_normalize(VPSS_CHN_ATTR_S *pstChnAttr)
{
	if (pstChnAttr->stNormalize.bEnable) {
		for (CVI_U8 i = 0; i < 3; ++i) {
			if (pstChnAttr->stNormalize.factor[i] >= 1.0f) {
				pstChnAttr->stNormalize.factor[i] = 1.0f - 1.0f/8192;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with max value 8191/8192\n", i);
			}
			if (pstChnAttr->stNormalize.factor[i] < (1.0f/8192)) {
				pstChnAttr->stNormalize.factor[i] = (1.0f/8192);
				CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with min value 1/8192\n", i);
			}
			if (pstChnAttr->stNormalize.mean[i] > 255.0f) {
				pstChnAttr->stNormalize.mean[i] = 255.0f;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with max value 255\n", i);
			}
			if (pstChnAttr->stNormalize.mean[i] < 0) {
				pstChnAttr->stNormalize.mean[i] = 0;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with min value 0\n", i);
			}
		}
	}
}


/************************vpss Settings**********************************/
CVI_S32 platform_vpss_setmode(const VPSS_MODE_S *pstVPSSMode)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;

	ret = vpss_set_mode(fd, pstVPSSMode);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "vpss_set_mode fail\n");
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getmode(VPSS_MODE_S *pstVPSSMode)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	VPSS_MODE_S stMode;

	ret = vpss_get_mode(fd, &stMode);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "vpss_get_mode fail\n");
		return ret;
	}
	*pstVPSSMode = stMode;

	return CVI_SUCCESS;
}

/************************grp Settings**********************************/
CVI_S32 platform_vpss_creategrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_grp_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	cfg.VpssGrp = VpssGrp;
	memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

	ret = vpss_create_grp(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) create group fail\n", VpssGrp);
		return ret;
	}
	// for chn rotation, ldc mesh gen
	for (CVI_U8 i = 0; i < VPSS_MAX_CHN_NUM; ++i)
		pthread_mutex_init(&mesh[VpssGrp][i].lock, NULL);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_destroygrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_destroy_grp(fd, VpssGrp);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) destroy group fail\n", VpssGrp);
		return ret;
	}
	for (CVI_U8 i = 0; i < VPSS_MAX_CHN_NUM; ++i)
		pthread_mutex_destroy(&mesh[VpssGrp][i].lock);

	return CVI_SUCCESS;
}

VPSS_GRP platform_vpss_getavailablegrp(CVI_VOID)
{
	CVI_S32 fd = get_vpss_fd();
	VPSS_GRP grp = VPSS_INVALID_GRP;

	vpss_get_available_grp(fd, &grp);

	return grp;
}

CVI_S32 platform_vpss_startgrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_start_grp(fd, VpssGrp);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) start group fail\n",
				VpssGrp);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_stopgrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_stop_grp(fd, VpssGrp);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) stop group fail\n", VpssGrp);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_resetgrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_reset_grp(fd, VpssGrp);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) reset group fail\n", VpssGrp);
		return ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpattr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_grp_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;

	ret = vpss_get_grp_attr(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get grp attr fail\n", VpssGrp);
		return ret;
	}

	memcpy(pstGrpAttr, &cfg.stGrpAttr, sizeof(*pstGrpAttr));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpattr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_grp_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

	ret = vpss_set_grp_attr(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set grp attr fail\n",
				VpssGrp);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpcrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_grp_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;

	ret = vpss_get_grp_crop(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get crop fail\n", VpssGrp);
		return ret;
	}

	*pstCropInfo = cfg.stCropInfo;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpcrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_grp_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.stCropInfo = *pstCropInfo;

	ret = vpss_set_grp_crop(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set crop fail\n", VpssGrp);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_sendframe(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_snd_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	cfg.VpssGrp = VpssGrp;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
	cfg.s32MilliSec = s32MilliSec;

	ret = vpss_send_frame(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) send frame fail\n", VpssGrp);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpprocampctrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_proc_amp_ctrl_cfg cfg = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, ctrl);

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	cfg.type = type;
	ret = vpss_get_proc_amp_ctrl(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp ctrl fail\n", VpssGrp);
		return ret;
	}
	*ctrl = cfg.ctrl;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
	CVI_S32 ret;
	CVI_S32 fd = get_vpss_fd();
	struct vpss_proc_amp_cfg cfg = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, value);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	cfg.VpssGrp = VpssGrp;
	ret = vpss_get_proc_amp(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp fail\n", VpssGrp);
		return ret;
	}
	*value = cfg.proc_amp[type];
	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setgrpprocamp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 value)
{
	CVI_S32 ret;
	PROC_AMP_CTRL_S ctrl;
	struct vpss_grp_csc_cfg csc_cfg;
	struct vpss_proc_amp_cfg amp_cfg;
	CVI_S32 fd = get_vpss_fd();

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	platform_vpss_getgrpprocampctrl(VpssGrp, type, &ctrl);
	if ((value > ctrl.maximum) || (value < ctrl.minimum)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) new value(%d) out of range(%d ~ %d).\n"
			, VpssGrp, value, ctrl.minimum, ctrl.maximum);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	amp_cfg.VpssGrp = VpssGrp;
	ret = vpss_get_proc_amp(fd, &amp_cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp fail\n", VpssGrp);
		return ret;
	}
	amp_cfg.proc_amp[type] = value;

	memset(&csc_cfg, 0, sizeof(csc_cfg));
	csc_cfg.VpssGrp = VpssGrp;
	memcpy(csc_cfg.proc_amp, amp_cfg.proc_amp, sizeof(csc_cfg.proc_amp));
	_vpss_proamp_2_csc(&csc_cfg);

	ret = vpss_set_grp_csc(fd, &csc_cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set group csc fail\n", VpssGrp);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getallprocamp(VPSS_ALL_PROC_AMP_S *pstProcAmp)
{
	CVI_S32 ret;
	CVI_S32 fd = get_vpss_fd();
	VPSS_ALL_PROC_AMP_S cfg = {0};

	ret = vpss_get_all_proc_amp(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VPSS get all proc amp fail\n");
		return ret;
	}

	memcpy(pstProcAmp, &cfg, sizeof(*pstProcAmp));

	return CVI_SUCCESS;
}

/* platform_vpss_setgrpparamfrombin: Apply the settings of scene from bin
 *
 * @param VpssGrp: the vpss grp to apply
 * @param bin_data: VPSS_BIN_DATA pointer from the caller
 * @return: result of the API
 */
CVI_S32 platform_vpss_setgrpparamfrombin(VPSS_GRP VpssGrp, VPSS_BIN_DATA *bin_data)
{
	CVI_S32 ret;
	struct vpss_grp_csc_cfg csc_cfg = {0};
	CVI_S32 fd = get_vpss_fd();

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;

	csc_cfg.VpssGrp = VpssGrp;
	memcpy(csc_cfg.proc_amp, bin_data->proc_amp, sizeof(csc_cfg.proc_amp));
	_vpss_proamp_2_csc(&csc_cfg);

	ret = vpss_set_grp_csc(fd, &csc_cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set group csc fail\n", VpssGrp);
		return ret;
	}

	CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is exist, vpss grp param use pqbin value !!\n");

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getbinscene(VPSS_GRP VpssGrp, CVI_U8 *scene)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_scene cfg = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, scene);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.VpssGrp = VpssGrp;
	s32Ret = vpss_get_binscene(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp fail\n", VpssGrp);
		return s32Ret;
	}

	*scene = cfg.scene;

	return CVI_SUCCESS;
}

/* Chn Settings */
CVI_S32 platform_vpss_setchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_cfg attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memcpy(&attr.stChnAttr,  pstChnAttr, sizeof(attr.stChnAttr));
	// Handle float poing in user space
	_vpss_check_normalize(&attr.stChnAttr);

	ret = vpss_set_chn_attr(fd, &attr);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_cfg attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_get_chn_attr(fd, &attr);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*pstChnAttr = attr.stChnAttr;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_enablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_enable_chn(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) enable fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_disablechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	CVI_CHAR mesh_name[128];

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_disable_chn(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) disable fail\n", VpssGrp, VpssChn);
		return ret;
	}

	snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
	gdc_free_cur_tsk_mesh(mesh_name);
	mesh[VpssGrp][VpssChn].paddr = CVI_NULL;
	mesh[VpssGrp][VpssChn].vaddr = CVI_NULL;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stCropInfo = *pstCropInfo;

	ret = vpss_set_chn_crop(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn crop fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchncrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_chn_crop(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn crop fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*pstCropInfo = cfg.stCropInfo;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
	CVI_S32 ret;
	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_cfg attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	struct vpss_chn_ldc_cfg ldc_cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_get_chn_attr(fd, &attr);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	if (!GDC_SUPPORT_FMT(attr.stChnAttr.enPixelFormat)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for GDC.\n",
			VpssGrp, VpssChn, attr.stChnAttr.enPixelFormat);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	ret = vpss_get_chn_ldc(fd, &ldc_cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn LDC attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	if (enRotation == ROTATION_180) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VI_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid rotation(%d).\n"
			, VpssGrp, VpssChn, enRotation);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	if (ldc_cfg.stLDCAttr.bEnable) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn rotation fail, please add rotation to ldc.\n",
			VpssGrp, VpssChn);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	} else
		return _vpss_update_rotation_mesh(VpssGrp, VpssChn, enRotation,
			attr.stChnAttr.u32Width, attr.stChnAttr.u32Height);
	return ret;
}

CVI_S32 platform_vpss_getchnrotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_rot_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penRotation);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_chn_rotation(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*penRotation = cfg.enRotation;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_rot_cfg rot_cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	struct vpss_chn_cfg attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_get_chn_attr(fd, &attr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	s32Ret = CHECK_VPSS_GDC_FMT(VpssGrp, VpssChn, attr.stChnAttr.enPixelFormat);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_get_chn_rotation(fd, &rot_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return _vpss_update_ldc_mesh(VpssGrp, VpssChn, pstLDCAttr, rot_cfg.enRotation,
				attr.stChnAttr.u32Width, attr.stChnAttr.u32Height);
}

CVI_S32 platform_vpss_getchnldcattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_ldc_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_chn_ldc(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn LDC attr fail\n", VpssGrp, VpssChn);
		return ret;
	}

	memcpy(pstLDCAttr, &cfg.stLDCAttr, sizeof(*pstLDCAttr));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_sendchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
	cfg.s32MilliSec = s32MilliSec;

	ret = vpss_send_chn_frame(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) send chn frame fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
			CVI_S32 s32MilliSec)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstFrameInfo);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.s32MilliSec = s32MilliSec;

	ret = vpss_get_chn_frame(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn frame fail\n", VpssGrp, VpssChn);
		return ret;
	}
	memcpy(pstFrameInfo, &cfg.stVideoFrame, sizeof(*pstFrameInfo));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_releasechnframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));

	ret = vpss_release_chn_frame(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) release chn frame fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_triggersnapframe(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32FrameCnt)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_snap_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn, .frame_cnt = u32FrameCnt};

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = vpss_trigger_snap_frame(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) Trigger Snap Frame fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_attachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_vb_pool_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.hVbPool = hVbPool;
	return vpss_attach_vbpool(fd, &cfg);
}

CVI_S32 platform_vpss_detachvbpool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_vb_pool_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	return vpss_detach_vbpool(fd, &cfg);
}

CVI_S32 platform_vpss_setchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_align_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.u32Align = u32Align;

	ret = vpss_set_chn_align(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn align fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnalign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_align_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pu32Align);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_chn_align(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn align fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*pu32Align = cfg.u32Align;

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
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_yratio_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.YRatio = (CVI_U32)(YRatio * 100);

	ret = vpss_set_chn_yratio(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn Y Ratio fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnyratio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_yratio_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pYRatio);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_chn_yratio(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn Y Ratio fail\n", VpssGrp, VpssChn);
		return ret;
	}
	*pYRatio = (1.0f * cfg.YRatio) / 100.0;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_coef_level_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enCoef = enCoef;

	ret = vpss_set_coef_level(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn ScaleCoefLevel fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnscalecoeflevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_coef_level_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penCoef);
	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_coef_level(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn ScaleCoefLevel fail\n", VpssGrp, VpssChn);
		return ret;
	}

	*penCoef = cfg.enCoef;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_DRAW_RECT_S *pstDrawRect)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_draw_rect_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stDrawRect = *pstDrawRect;

	ret = vpss_set_draw_rect(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set draw rect fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchndrawrect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_DRAW_RECT_S *pstDrawRect)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_draw_rect_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_draw_rect(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get draw rect fail\n", VpssGrp, VpssChn);
		return ret;
	}
	*pstDrawRect = cfg.stDrawRect;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_convert_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stConvert = *pstConvert;

	ret = vpss_set_convert(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set convert fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnconvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_chn_convert_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_get_convert(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get convert fail\n", VpssGrp, VpssChn);
		return ret;
	}
	*pstConvert = cfg.stConvert;

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_wrap_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.wrap = *pstVpssChnBufWrap;

	s32Ret = vpss_set_chn_wrap(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn wrap fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnbufwrapattr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
		VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_wrap_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_chn_wrap(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn Wrap Attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	memcpy(pstVpssChnBufWrap, &cfg.wrap, sizeof(*pstVpssChnBufWrap));

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_showchn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_show_chn(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) show chn fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_hidechn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;
	struct vpss_en_chn_cfg cfg;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	ret = vpss_hide_chn(fd, &cfg);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) hide chn fail\n", VpssGrp, VpssChn);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getchnfd(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 ret;

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	return fd;
}

CVI_S32 platform_vpss_closefd(CVI_VOID)
{
	vpss_dev_close();
	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_createstitch(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	UNUSED(VpssGrp);
	UNUSED(pstStitchAttr);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_destroystitch(VPSS_GRP VpssGrp)
{
	UNUSED(VpssGrp);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_setstitchattr(VPSS_GRP VpssGrp, const CVI_STITCH_ATTR_S *pstStitchAttr)
{
	UNUSED(VpssGrp);
	UNUSED(pstStitchAttr);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_getstitchattr(VPSS_GRP VpssGrp, CVI_STITCH_ATTR_S *pstStitchAttr)
{
	UNUSED(VpssGrp);
	UNUSED(pstStitchAttr);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_startstitch(VPSS_GRP VpssGrp)
{
	UNUSED(VpssGrp);

	return CVI_SUCCESS;
}

CVI_S32 platform_vpss_stopstitch(VPSS_GRP VpssGrp)
{
	UNUSED(VpssGrp);

	return CVI_SUCCESS;
}

