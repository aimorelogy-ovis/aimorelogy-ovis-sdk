#include <vpss_cb.h>
#include "vpss_debug.h"
#include "vpss.h"
#include "vpss_os_ioctl.h"

int vpss_ioctl(unsigned int cmd, void *arg)
{
    int ret = 0;

    switch (cmd)
    {
    case CVI_VPSS_CREATE_GROUP: {
        struct vpss_crt_grp_cfg *cfg =
            (struct vpss_crt_grp_cfg *)arg;

        ret = _os_vpss_create_grp(cfg->VpssGrp, &cfg->stGrpAttr);
        break;
    }

    case CVI_VPSS_DESTROY_GROUP: {
        VPSS_GRP VpssGrp = *((VPSS_GRP *)arg);

        ret = _os_vpss_destroy_grp(VpssGrp);
        break;
    }

    case CVI_VPSS_START_GROUP: {
        struct vpss_str_grp_cfg *cfg = (struct vpss_str_grp_cfg *)arg;

        ret = _os_vpss_start_grp(cfg->VpssGrp);
        break;
    }

    case CVI_VPSS_STOP_GROUP: {
        VPSS_GRP VpssGrp = *((VPSS_GRP *)arg);

        ret = _os_vpss_stop_grp(VpssGrp);
        break;
    }

    case CVI_VPSS_RESET_GROUP: {
        VPSS_GRP VpssGrp = *((VPSS_GRP *)arg);

        ret = _os_vpss_reset_grp(VpssGrp);
        break;
    }

    case CVI_VPSS_GET_AVAIL_GROUP: {
        VPSS_GRP VpssGrp;

        VpssGrp            = _os_vpss_get_available_grp();
        *((VPSS_GRP *)arg) = VpssGrp;
        ret                = 0;
        break;
    }

    case CVI_VPSS_SET_GRP_ATTR: {
        struct vpss_grp_attr *cfg     = (struct vpss_grp_attr *)arg;
        VPSS_GRP              VpssGrp = cfg->VpssGrp;

        const VPSS_GRP_ATTR_S *pstGrpAttr = &cfg->stGrpAttr;

        ret = _os_vpss_set_grp_attr(VpssGrp, pstGrpAttr);
        break;
    }

    case CVI_VPSS_GET_GRP_ATTR: {
        struct vpss_grp_attr *cfg        = (struct vpss_grp_attr *)arg;
        VPSS_GRP              VpssGrp    = cfg->VpssGrp;
        VPSS_GRP_ATTR_S      *pstGrpAttr = &cfg->stGrpAttr;

        ret = _os_vpss_get_grp_attr(VpssGrp, pstGrpAttr);
        break;
    }

    case CVI_VPSS_SET_GRP_CROP: {
        struct vpss_grp_crop_cfg *cfg     = (struct vpss_grp_crop_cfg *)arg;
        VPSS_GRP                  VpssGrp = cfg->VpssGrp;

        const VPSS_CROP_INFO_S *pstCropInfo = &cfg->stCropInfo;

        ret = _os_vpss_set_grp_crop(VpssGrp, pstCropInfo);
        break;
    }

    case CVI_VPSS_GET_GRP_CROP: {
        struct vpss_grp_crop_cfg *cfg         = (struct vpss_grp_crop_cfg *)arg;
        VPSS_GRP                  VpssGrp     = cfg->VpssGrp;
        VPSS_CROP_INFO_S         *pstCropInfo = &cfg->stCropInfo;

        ret = _os_vpss_get_grp_crop(VpssGrp, pstCropInfo);
        break;
    }

    case CVI_VPSS_GET_GRP_FRAME: {
        struct vpss_grp_frame_cfg *cfg           = (struct vpss_grp_frame_cfg *)arg;
        VPSS_GRP                   VpssGrp       = cfg->VpssGrp;
        VIDEO_FRAME_INFO_S        *pstVideoFrame = &cfg->stVideoFrame;

        ret = vpss_get_grp_frame(VpssGrp, pstVideoFrame);
        break;
    }

    case CVI_VPSS_SET_RELEASE_GRP_FRAME: {
        struct vpss_grp_frame_cfg *cfg           = (struct vpss_grp_frame_cfg *)arg;
        VPSS_GRP                   VpssGrp       = cfg->VpssGrp;
        const VIDEO_FRAME_INFO_S  *pstVideoFrame = &cfg->stVideoFrame;

        ret = vpss_release_grp_frame(VpssGrp, pstVideoFrame);
        break;
    }

    case CVI_VPSS_SEND_FRAME: {
        struct vpss_snd_frm_cfg  *cfg           = (struct vpss_snd_frm_cfg *)arg;
        VPSS_GRP                  VpssGrp       = cfg->VpssGrp;
        const VIDEO_FRAME_INFO_S *pstVideoFrame = &cfg->stVideoFrame;
        CVI_S32                   s32MilliSec   = cfg->s32MilliSec;

        ret = _os_vpss_send_frame(VpssGrp, pstVideoFrame, s32MilliSec);
        break;
    }

    case CVI_VPSS_SET_GRP_CSC_CFG: {
        struct vpss_grp_csc_cfg *cfg = (struct vpss_grp_csc_cfg *)arg;

        ret = _os_vpss_set_grp_csc(cfg);
        break;
    }

    case CVI_VPSS_SEND_CHN_FRAME: {
        struct vpss_chn_frm_cfg  *cfg           = (struct vpss_chn_frm_cfg *)arg;
        VPSS_GRP                  VpssGrp       = cfg->VpssGrp;
        VPSS_CHN                  VpssChn       = cfg->VpssChn;
        const VIDEO_FRAME_INFO_S *pstVideoFrame = &cfg->stVideoFrame;
        CVI_S32                   s32MilliSec   = cfg->s32MilliSec;

        ret = _os_vpss_send_chn_frame(VpssGrp, VpssChn, pstVideoFrame, s32MilliSec);
        break;
    }

    case CVI_VPSS_SET_CHN_ATTR: {
        struct vpss_chn_attr *attr    = (struct vpss_chn_attr *)arg;
        VPSS_GRP              VpssGrp = attr->VpssGrp;
        VPSS_CHN              VpssChn = attr->VpssChn;

        const VPSS_CHN_ATTR_S *pstChnAttr = &attr->stChnAttr;

        ret = _os_vpss_set_chn_attr(VpssGrp, VpssChn, pstChnAttr);
        break;
    }

    case CVI_VPSS_GET_CHN_ATTR: {
        struct vpss_chn_attr *attr       = (struct vpss_chn_attr *)arg;
        VPSS_GRP              VpssGrp    = attr->VpssGrp;
        VPSS_CHN              VpssChn    = attr->VpssChn;
        VPSS_CHN_ATTR_S      *pstChnAttr = &attr->stChnAttr;

        ret = _os_vpss_get_chn_attr(VpssGrp, VpssChn, pstChnAttr);
        break;
    }

    case CVI_VPSS_ENABLE_CHN: {
        struct vpss_en_chn_cfg *cfg     = (struct vpss_en_chn_cfg *)arg;
        VPSS_GRP                VpssGrp = cfg->VpssGrp;
        VPSS_CHN                VpssChn = cfg->VpssChn;

        ret = _os_vpss_enable_chn(VpssGrp, VpssChn);
        break;
    }

    case CVI_VPSS_DISABLE_CHN: {
        struct vpss_en_chn_cfg *cfg = (struct vpss_en_chn_cfg *)arg;

        ret = _os_vpss_disable_chn(cfg->VpssGrp, cfg->VpssChn);
        break;
    }

    case CVI_VPSS_SET_CHN_CROP: {
        struct vpss_chn_crop_cfg *cfg         = (struct vpss_chn_crop_cfg *)arg;
        VPSS_GRP                  VpssGrp     = cfg->VpssGrp;
        VPSS_CHN                  VpssChn     = cfg->VpssChn;
        const VPSS_CROP_INFO_S   *pstCropInfo = &cfg->stCropInfo;

        ret = _os_vpss_set_chn_crop(VpssGrp, VpssChn, pstCropInfo);
        break;
    }

    case CVI_VPSS_GET_CHN_CROP: {
        struct vpss_chn_crop_cfg *cfg         = (struct vpss_chn_crop_cfg *)arg;
        VPSS_GRP                  VpssGrp     = cfg->VpssGrp;
        VPSS_CHN                  VpssChn     = cfg->VpssChn;
        VPSS_CROP_INFO_S         *pstCropInfo = &cfg->stCropInfo;

        ret = _os_vpss_get_chn_crop(VpssGrp, VpssChn, pstCropInfo);
        break;
    }

    case CVI_VPSS_SET_CHN_ROTATION: {
        struct cvi_vpss_chn_rot_cfg *cfg        = (struct cvi_vpss_chn_rot_cfg *)arg;
        VPSS_GRP                     VpssGrp    = cfg->VpssGrp;
        VPSS_CHN                     VpssChn    = cfg->VpssChn;
        ROTATION_E                   enRotation = cfg->enRotation;

        ret = _os_vpss_set_chn_rotation(VpssGrp, VpssChn, enRotation);
        break;
    }

    case CVI_VPSS_GET_CHN_ROTATION: {
        struct cvi_vpss_chn_rot_cfg *cfg         = (struct cvi_vpss_chn_rot_cfg *)arg;
        VPSS_GRP                     VpssGrp     = cfg->VpssGrp;
        VPSS_CHN                     VpssChn     = cfg->VpssChn;
        ROTATION_E                  *penRotation = &cfg->enRotation;

        ret = _os_vpss_get_chn_rotation(VpssGrp, VpssChn, penRotation);
        break;
    }

    case CVI_VPSS_SET_CHN_LDC: {
        struct cvi_vpss_chn_ldc_cfg *cfg        = (struct cvi_vpss_chn_ldc_cfg *)arg;
        VPSS_GRP                     VpssGrp    = cfg->VpssGrp;
        VPSS_CHN                     VpssChn    = cfg->VpssChn;
        ROTATION_E                   enRotation = cfg->enRotation;
        CVI_U64                      mesh_addr  = cfg->meshHandle;

        const VPSS_LDC_ATTR_S *pstLDCAttr = &cfg->stLDCAttr;

        ret = vpss_set_chn_ldc_attr(VpssGrp, VpssChn, enRotation, pstLDCAttr, mesh_addr);
        break;
    }

    case CVI_VPSS_GET_CHN_LDC: {
        struct cvi_vpss_chn_ldc_cfg *cfg        = (struct cvi_vpss_chn_ldc_cfg *)arg;
        VPSS_GRP                     VpssGrp    = cfg->VpssGrp;
        VPSS_CHN                     VpssChn    = cfg->VpssChn;
        VPSS_LDC_ATTR_S             *pstLDCAttr = &cfg->stLDCAttr;

        ret = vpss_get_chn_ldc_attr(VpssGrp, VpssChn, pstLDCAttr);
        break;
    }

    case CVI_VPSS_GET_CHN_FRAME: {
        struct vpss_chn_frm_cfg *cfg          = (struct vpss_chn_frm_cfg *)arg;
        VPSS_GRP                 VpssGrp      = cfg->VpssGrp;
        VPSS_CHN                 VpssChn      = cfg->VpssChn;
        VIDEO_FRAME_INFO_S      *pstFrameInfo = &cfg->stVideoFrame;
        CVI_S32                  s32MilliSec  = cfg->s32MilliSec;

        ret = _os_vpss_get_chn_frame(VpssGrp, VpssChn, pstFrameInfo, s32MilliSec);
        break;
    }

    case CVI_VPSS_RELEASE_CHN_RAME: {
        struct vpss_chn_frm_cfg  *cfg           = (struct vpss_chn_frm_cfg *)arg;
        VPSS_GRP                  VpssGrp       = cfg->VpssGrp;
        VPSS_CHN                  VpssChn       = cfg->VpssChn;
        const VIDEO_FRAME_INFO_S *pstVideoFrame = &cfg->stVideoFrame;

        ret = _os_vpss_release_chn_frame(VpssGrp, VpssChn, pstVideoFrame);
        break;
    }

    case CVI_VPSS_SET_CHN_ALIGN: {
        struct vpss_chn_align_cfg *cfg      = (struct vpss_chn_align_cfg *)arg;
        VPSS_GRP                   VpssGrp  = cfg->VpssGrp;
        VPSS_CHN                   VpssChn  = cfg->VpssChn;
        CVI_U32                    u32Align = cfg->u32Align;

        ret = _os_vpss_set_chn_align(VpssGrp, VpssChn, u32Align);
        break;
    }

    case CVI_VPSS_GET_CHN_ALIGN: {
        struct vpss_chn_align_cfg *cfg       = (struct vpss_chn_align_cfg *)arg;
        VPSS_GRP                   VpssGrp   = cfg->VpssGrp;
        VPSS_CHN                   VpssChn   = cfg->VpssChn;
        CVI_U32                   *pu32Align = &cfg->u32Align;

        ret = _os_vpss_get_chn_align(VpssGrp, VpssChn, pu32Align);
        break;
    }

    case CVI_VPSS_SET_CHN_YRATIO: {
        struct vpss_chn_yratio_cfg *cfg     = (struct vpss_chn_yratio_cfg *)arg;
        VPSS_GRP                    VpssGrp = cfg->VpssGrp;
        VPSS_CHN                    VpssChn = cfg->VpssChn;
        CVI_U32                     YRatio  = cfg->YRatio;

        ret = _os_vpss_set_chn_yratio(VpssGrp, VpssChn, YRatio);
        break;
    }

    case CVI_VPSS_GET_CHN_YRATIO: {
        struct vpss_chn_yratio_cfg *cfg     = (struct vpss_chn_yratio_cfg *)arg;
        VPSS_GRP                    VpssGrp = cfg->VpssGrp;
        VPSS_CHN                    VpssChn = cfg->VpssChn;
        CVI_U32                    *pYRatio = &cfg->YRatio;

        ret = _os_vpss_get_chn_yratio(VpssGrp, VpssChn, pYRatio);
        break;
    }

    case CVI_VPSS_SET_CHN_SCALE_COEFF_LEVEL: {
        struct vpss_chn_coef_level_cfg *cfg     = (struct vpss_chn_coef_level_cfg *)arg;
        VPSS_GRP                        VpssGrp = cfg->VpssGrp;
        VPSS_CHN                        VpssChn = cfg->VpssChn;
        VPSS_SCALE_COEF_E               enCoef  = cfg->enCoef;

        ret = vpss_set_chn_scale_coef_level(VpssGrp, VpssChn, enCoef);
        break;
    }

    case CVI_VPSS_GET_CHN_SCALE_COEFF_LEVEL: {
        struct vpss_chn_coef_level_cfg *cfg     = (struct vpss_chn_coef_level_cfg *)arg;
        VPSS_GRP                        VpssGrp = cfg->VpssGrp;
        VPSS_CHN                        VpssChn = cfg->VpssChn;
        VPSS_SCALE_COEF_E              *penCoef = &cfg->enCoef;

        ret = vpss_get_chn_scale_coef_level(VpssGrp, VpssChn, penCoef);
        break;
    }

    case CVI_VPSS_SHOW_CHN: {
        struct vpss_en_chn_cfg *cfg     = (struct vpss_en_chn_cfg *)arg;
        VPSS_GRP                VpssGrp = cfg->VpssGrp;
        VPSS_CHN                VpssChn = cfg->VpssChn;

        ret = _os_vpss_show_chn(VpssGrp, VpssChn);
        break;
    }

    case CVI_VPSS_HIDE_CHN: {
        struct vpss_en_chn_cfg *cfg     = (struct vpss_en_chn_cfg *)arg;
        VPSS_GRP                VpssGrp = cfg->VpssGrp;
        VPSS_CHN                VpssChn = cfg->VpssChn;

        ret = _os_vpss_hide_chn(VpssGrp, VpssChn);
        break;
    }

    case CVI_VPSS_SET_CHN_BUF_WRAP: {
        struct vpss_chn_wrap_cfg  *cfg               = (struct vpss_chn_wrap_cfg *)arg;
        VPSS_GRP                   VpssGrp           = cfg->VpssGrp;
        VPSS_CHN                   VpssChn           = cfg->VpssChn;
        const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap = &cfg->wrap;

        ret = vpss_set_chn_bufwrap_attr(VpssGrp, VpssChn, pstVpssChnBufWrap);
        break;
    }

    case CVI_VPSS_GET_CHN_BUF_WRAP: {
        struct vpss_chn_wrap_cfg *cfg               = (struct vpss_chn_wrap_cfg *)arg;
        VPSS_GRP                  VpssGrp           = cfg->VpssGrp;
        VPSS_CHN                  VpssChn           = cfg->VpssChn;
        VPSS_CHN_BUF_WRAP_S      *pstVpssChnBufWrap = &cfg->wrap;

        ret = vpss_get_chn_bufwrap_attr(VpssGrp, VpssChn, pstVpssChnBufWrap);
        break;
    }

    case CVI_VPSS_ATTACH_VB_POOL: {
        struct vpss_vb_pool_cfg *cfg     = (struct vpss_vb_pool_cfg *)arg;
        VPSS_GRP                 VpssGrp = cfg->VpssGrp;
        VPSS_CHN                 VpssChn = cfg->VpssChn;
        VB_POOL                  hVbPool = (VB_POOL)cfg->hVbPool;

        ret = _os_vpss_attach_vb_pool(VpssGrp, VpssChn, hVbPool);
        break;
    }

    case CVI_VPSS_DETACH_VB_POOL: {
        struct vpss_vb_pool_cfg *cfg     = (struct vpss_vb_pool_cfg *)arg;
        VPSS_GRP                 VpssGrp = cfg->VpssGrp;
        VPSS_CHN                 VpssChn = cfg->VpssChn;

        ret = _os_vpss_detach_vb_pool(VpssGrp, VpssChn);
        break;
    }

    case CVI_VPSS_TRIGGER_SNAP_FRAME: {
        struct vpss_snap_cfg *cfg       = (struct vpss_snap_cfg *)arg;
        VPSS_GRP              VpssGrp   = cfg->VpssGrp;
        VPSS_CHN              VpssChn   = cfg->VpssChn;
        CVI_U32               frame_cnt = cfg->frame_cnt;

        ret = _os_vpss_trigger_snap_frame(VpssGrp, VpssChn, frame_cnt);
        break;
    }

    case CVI_VPSS_SET_BLD_CFG: {
        struct vpss_bld_cfg *cfg = (struct vpss_bld_cfg *)arg;
        struct sclr_bld_cfg  bld_cfg;

        bld_cfg.ctrl.raw            = 0;
        bld_cfg.ctrl.b.enable       = cfg->enable;
        bld_cfg.ctrl.b.fix_alpha    = cfg->fix_alpha;
        bld_cfg.ctrl.b.blend_y      = cfg->blend_y;
        bld_cfg.ctrl.b.y2r_enable   = cfg->y2r_enable;
        bld_cfg.ctrl.b.alpha_factor = cfg->alpha_factor;
        bld_cfg.width               = cfg->wd;
        sclr_top_bld_set_cfg(&bld_cfg);
        break;
    }

    default:
        CVI_TRACE_VPSS(CVI_DBG_DEBUG, "unknown cmd(0x%x)\n", cmd);
        break;
    }

    return ret;
}
