#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

#include "vpss_ioctl.h"

CVI_S32 vpss_send_frame(struct vpss_snd_frm_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SEND_FRAME, (void *)cfg);
}

CVI_S32 vpss_send_chn_frame(struct vpss_chn_frm_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SEND_CHN_FRAME, (void *)cfg);
}

CVI_S32 vpss_release_chn_frame(const struct vpss_chn_frm_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_RELEASE_CHN_RAME, (void *)cfg);
}

CVI_S32 vpss_create_grp(struct vpss_crt_grp_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_CREATE_GROUP, (void *)cfg);
}

CVI_S32 vpss_destroy_grp(VPSS_GRP VpssGrp)
{
    return vpss_ioctl(CVI_VPSS_DESTROY_GROUP, (void *)&VpssGrp);
}

CVI_S32 vpss_get_available_grp(VPSS_GRP *pVpssGrp)
{
    return vpss_ioctl(CVI_VPSS_GET_AVAIL_GROUP, (void *)pVpssGrp);
}

CVI_S32 vpss_start_grp(struct vpss_str_grp_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_START_GROUP, (void *)cfg);
}

CVI_S32 vpss_stop_grp(VPSS_GRP VpssGrp)
{
    return vpss_ioctl(CVI_VPSS_STOP_GROUP, (void *)&VpssGrp);
}

CVI_S32 vpss_reset_grp(VPSS_GRP VpssGrp)
{
    return vpss_ioctl(CVI_VPSS_RESET_GROUP, (void *)&VpssGrp);
}

CVI_S32 vpss_set_grp_attr(const struct vpss_grp_attr *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_GRP_ATTR, (void *)cfg);
}

CVI_S32 vpss_get_grp_attr(struct vpss_grp_attr *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_GRP_ATTR, (void *)cfg);
}

CVI_S32 vpss_set_grp_crop(const struct vpss_grp_crop_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_GRP_CROP, (void *)cfg);
}

CVI_S32 vpss_get_grp_crop(struct vpss_grp_crop_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_GRP_CROP, (void *)cfg);
}

CVI_VOID _vpss_pack_fixed_point_norm(VPSS_NORMALIZE_S *pstNormalize)
{
    CVI_U16                    sc_frac[3];
    CVI_U8                     sub[3];
    CVI_U16                    sub_frac[3];
    CVI_DOUBLE                 sub_int;
    VPSS_NORMALIZE_S           stNormalize;
    struct vpss_int_normalize *int_norm;

    if (!pstNormalize->bEnable)
        return;

    memcpy(&stNormalize, pstNormalize, sizeof(stNormalize));

    for (CVI_U8 i = 0; i < 3; ++i)
    {
        sc_frac[i]  = pstNormalize->factor[i] * 8192;
        sub_frac[i] = modf(pstNormalize->mean[i], &sub_int) * 1024;
        sub[i]      = sub_int;
    }

    int_norm           = (struct vpss_int_normalize *)pstNormalize;
    int_norm->enable   = 1;
    int_norm->rounding = pstNormalize->rounding;

    for (CVI_U8 i = 0; i < 3; ++i)
    {
        int_norm->sc_frac[i]  = sc_frac[i];
        int_norm->sub[i]      = sub[i];
        int_norm->sub_frac[i] = sub_frac[i];
    }
}

CVI_S32 vpss_set_chn_attr(struct vpss_chn_attr *attr)
{
    if (sizeof(struct vpss_int_normalize) > sizeof(VPSS_NORMALIZE_S))
    {
        fprintf(stderr, "Kernel cannnot use float\n");
        return CVI_FAILURE;
    }

    _vpss_pack_fixed_point_norm(&attr->stChnAttr.stNormalize);

    return vpss_ioctl(CVI_VPSS_SET_CHN_ATTR, (void *)attr);
}

CVI_S32 vpss_get_chn_attr(struct vpss_chn_attr *attr)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_ATTR, (void *)attr);
}

CVI_S32 vpss_show_chn(struct vpss_en_chn_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SHOW_CHN, (void *)cfg);
}

CVI_S32 vpss_hide_chn(struct vpss_en_chn_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_HIDE_CHN, (void *)cfg);
}

CVI_S32 vpss_enable_chn(struct vpss_en_chn_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_ENABLE_CHN, (void *)cfg);
}

CVI_S32 vpss_disable_chn(struct vpss_en_chn_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_DISABLE_CHN, (void *)cfg);
}

CVI_S32 vpss_get_chn_frame(struct vpss_chn_frm_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_FRAME, (void *)cfg);
}

CVI_S32 vpss_set_chn_wrap(const struct vpss_chn_wrap_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_CHN_BUF_WRAP, (void *)cfg);
}

CVI_S32 vpss_get_chn_wrap(struct vpss_chn_wrap_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_BUF_WRAP, (void *)cfg);
}

CVI_S32 vpss_set_chn_rotation(const struct vpss_chn_rot_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_CHN_ROTATION, (void *)cfg);
}

CVI_S32 vpss_get_chn_rotation(struct vpss_chn_rot_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_ROTATION, (void *)cfg);
}

CVI_S32 vpss_set_chn_align(const struct vpss_chn_align_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_CHN_ALIGN, (void *)cfg);
}

CVI_S32 vpss_get_chn_align(struct vpss_chn_align_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_ALIGN, (void *)cfg);
}

CVI_S32 vpss_set_chn_ldc(const struct vpss_chn_ldc_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_CHN_LDC, (void *)cfg);
}

CVI_S32 vpss_get_chn_ldc(struct vpss_chn_ldc_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_LDC, (void *)cfg);
}

CVI_S32 vpss_set_chn_crop(const struct vpss_chn_crop_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_CHN_CROP, (void *)cfg);
}

CVI_S32 vpss_get_chn_crop(struct vpss_chn_crop_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_CROP, (void *)cfg);
}

CVI_S32 vpss_set_coef_level(const struct vpss_chn_coef_level_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_CHN_SCALE_COEFF_LEVEL, (void *)cfg);
}

CVI_S32 vpss_get_coef_level(struct vpss_chn_coef_level_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_SCALE_COEFF_LEVEL, (void *)cfg);
}

CVI_S32 vpss_set_chn_yratio(const struct vpss_chn_yratio_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_CHN_YRATIO, (void *)cfg);
}

CVI_S32 vpss_get_chn_yratio(struct vpss_chn_yratio_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_GET_CHN_YRATIO, (void *)cfg);
}

CVI_S32 vpss_set_grp_csc(const struct vpss_grp_csc_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_SET_GRP_CSC_CFG, (void *)cfg);
}

CVI_S32 vpss_attach_vbpool(const struct vpss_vb_pool_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_ATTACH_VB_POOL, (void *)cfg);
}

CVI_S32 vpss_detach_vbpool(const struct vpss_vb_pool_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_DETACH_VB_POOL, (void *)cfg);
}

CVI_S32 vpss_trigger_snap_frame(struct vpss_snap_cfg *cfg)
{
    return vpss_ioctl(CVI_VPSS_TRIGGER_SNAP_FRAME, (void *)cfg);
}
