#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <pthread.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <cvi_base.h>
#include <cvi_mw_base.h>

#include "cvi_buffer.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include "cvi_gdc.h"
#include "gdc_mesh.h"
#include "vpss_ioctl.h"
#include "vpss_ctx.h"

#define CHECK_VPSS_GRP_VALID(grp)                                                                \
    do {                                                                                         \
        if ((grp >= VPSS_MAX_GRP_NUM) || (grp < 0))                                              \
        {                                                                                        \
            CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssGrp(%d) exceeds Max(%d)\n", grp, VPSS_MAX_GRP_NUM); \
            return CVI_ERR_VPSS_ILLEGAL_PARAM;                                                   \
        }                                                                                        \
    } while (0)

#define CHECK_VPSS_GRP_CREATED(grp)                                           \
    do {                                                                      \
        if (!vpssCtx[grp] || !vpssCtx[grp]->isCreated)                        \
        {                                                                     \
            CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) isn't created yet.\n", grp); \
            return CVI_ERR_VPSS_UNEXIST;                                      \
        }                                                                     \
    } while (0)

#define CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn)                                              \
    do {                                                                                    \
        if (CVI_SYS_GetVPSSMode() == VPSS_MODE_SINGLE)                                      \
        {                                                                                   \
            if ((VpssChn >= VPSS_MAX_CHN_NUM) || (VpssChn < 0))                             \
            {                                                                               \
                CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid for VPSS-Single.\n",   \
                               VpssGrp, VpssChn);                                           \
                return CVI_ERR_VPSS_ILLEGAL_PARAM;                                          \
            }                                                                               \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            if ((VpssChn >= vpssCtx[VpssGrp]->chnNum) || (VpssChn < 0))                     \
            {                                                                               \
                CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid for VPSS-Dual(%d).\n", \
                               VpssGrp, VpssChn, vpssCtx[VpssGrp]->stGrpAttr.u8VpssDev);    \
                return CVI_ERR_VPSS_ILLEGAL_PARAM;                                          \
            }                                                                               \
        }                                                                                   \
    } while (0)

#define CHECK_YUV_PARAM(fmt, w, h)                                                      \
    do {                                                                                \
        if (fmt == PIXEL_FORMAT_YUV_PLANAR_422)                                         \
        {                                                                               \
            if (w & 0x01)                                                               \
            {                                                                           \
                CVI_TRACE_VPSS(CVI_DBG_ERR, "YUV_422 width(%d) should be even.\n", w);  \
                return CVI_ERR_VPSS_ILLEGAL_PARAM;                                      \
            }                                                                           \
        }                                                                               \
        else if ((fmt == PIXEL_FORMAT_YUV_PLANAR_420)                                   \
                 || (fmt == PIXEL_FORMAT_NV12)                                          \
                 || (fmt == PIXEL_FORMAT_NV21))                                         \
        {                                                                               \
            if (w & 0x01)                                                               \
            {                                                                           \
                CVI_TRACE_VPSS(CVI_DBG_ERR, "YUV_420 width(%d) should be even.\n", w);  \
                return CVI_ERR_VPSS_ILLEGAL_PARAM;                                      \
            }                                                                           \
            if (h & 0x01)                                                               \
            {                                                                           \
                CVI_TRACE_VPSS(CVI_DBG_ERR, "YUV_420 height(%d) should be even.\n", h); \
                return CVI_ERR_VPSS_ILLEGAL_PARAM;                                      \
            }                                                                           \
        }                                                                               \
    } while (0)

#define CHECK_VPSS_GRP_FMT(grp, fmt)                                                          \
    do {                                                                                      \
        if (!VPSS_GRP_SUPPORT_FMT(fmt))                                                       \
        {                                                                                     \
            CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) enPixelFormat(%d) unsupported\n", grp, fmt); \
            return CVI_ERR_VPSS_ILLEGAL_PARAM;                                                \
        }                                                                                     \
    } while (0)

#define CHECK_VPSS_CHN_FMT(grp, chn, fmt)                                                  \
    do {                                                                                   \
        if (!VPSS_CHN_SUPPORT_FMT(fmt))                                                    \
        {                                                                                  \
            CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) enPixelFormat(%d) unsupported\n", \
                           grp, chn, fmt);                                                 \
            return CVI_ERR_VPSS_ILLEGAL_PARAM;                                             \
        }                                                                                  \
    } while (0)

#define CHECK_VPSS_GDC_FMT(grp, chn, fmt)                                                   \
    do {                                                                                    \
        if (!GDC_SUPPORT_FMT(fmt))                                                          \
        {                                                                                   \
            CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for GDC.\n", \
                           grp, chn, (fmt));                                                \
            return CVI_ERR_VPSS_ILLEGAL_PARAM;                                              \
        }                                                                                   \
    } while (0)

#define FRC_INVALID(ctx, VpssChn) \
    (ctx->stChnCfgs[VpssChn].stChnAttr.stFrameRate.s32DstFrameRate <= 0 || ctx->stChnCfgs[VpssChn].stChnAttr.stFrameRate.s32SrcFrameRate <= 0 || ctx->stChnCfgs[VpssChn].stChnAttr.stFrameRate.s32DstFrameRate >= ctx->stChnCfgs[VpssChn].stChnAttr.stFrameRate.s32SrcFrameRate)

struct _vpss_gdc_cb_param
{
    MMF_CHN_S      chn;
    enum GDC_USAGE usage;
};

struct _vpss_rgnex_job_info
{
    MMF_CHN_S             chn;
    struct cvi_rgn_ex_cfg rgn_ex_cfg;
    PIXEL_FORMAT_E        enPixelFormat;
    CVI_U32               bytesperline[2];
};

static struct cvi_vpss_ctx *vpssCtx[VPSS_MAX_GRP_NUM] = {[0 ... VPSS_MAX_GRP_NUM - 1] = NULL};

struct cvi_gdc_mesh mesh[VPSS_MAX_GRP_NUM][VPSS_MAX_CHN_NUM];

static PROC_AMP_CTRL_S procamp_ctrls[PROC_AMP_MAX] = {
    {.minimum = 0, .maximum = 100, .step = 1, .default_value = 50},
    {.minimum = 0, .maximum = 100, .step = 1, .default_value = 50},
    {.minimum = 0, .maximum = 100, .step = 1, .default_value = 50},
    {.minimum = 0, .maximum = 100, .step = 1, .default_value = 50},
};

static VPSS_BIN_DATA vpss_bin_data[VPSS_MAX_GRP_NUM];
static CVI_BOOL      g_bLoadBinDone = CVI_FALSE;

VPSS_BIN_DATA *get_vpssbindata_addr(void)
{
    return vpss_bin_data;
}

void set_loadbin_state(CVI_BOOL bstate)
{
    g_bLoadBinDone = bstate;
}

struct cvi_vpss_ctx **vpss_get_ctx(void)
{
    return vpssCtx;
}

/**************************************************************************
 *   Job related APIs.
 **************************************************************************/
void _vpss_GrpParamInit(VPSS_GRP VpssGrp)
{
    PROC_AMP_CTRL_S ctrl;

    for (CVI_U8 i = PROC_AMP_BRIGHTNESS; i < PROC_AMP_MAX; ++i)
    {
        CVI_VPSS_GetGrpProcAmpCtrl(VpssGrp, i, &ctrl);
        vpssCtx[VpssGrp]->proc_amp[i] = ctrl.default_value;
    }
}

static CVI_S32 _vpss_update_rotation_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
    struct cvi_gdc_mesh    *pmesh = &mesh[VpssGrp][VpssChn];
    struct vpss_chn_rot_cfg cfg;

    // TODO: dummy settings
    pmesh->paddr                                    = DEFAULT_MESH_PADDR;
    vpssCtx[VpssGrp]->stChnCfgs[VpssChn].enRotation = enRotation;

    cfg.VpssGrp    = VpssGrp;
    cfg.VpssChn    = VpssChn;
    cfg.enRotation = enRotation;
    return vpss_set_chn_rotation(&cfg);

    return CVI_SUCCESS;
}

static CVI_S32 _vpss_update_ldc_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
                                     const VPSS_LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation)
{
    CVI_U64              paddr = CVI_NULL, paddr_old;
    CVI_VOID            *vaddr, *vaddr_old;
    struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];
    CVI_S32              s32Ret;
    (void)vaddr_old;

    if (!pstLDCAttr->bEnable)
    {
        pthread_mutex_lock(&pmesh->lock);
        vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stLDCAttr = *pstLDCAttr;
        pthread_mutex_unlock(&pmesh->lock);

        if (vpssCtx[VpssGrp]->stChnCfgs[VpssChn].enRotation != ROTATION_0)
            return _vpss_update_rotation_mesh(VpssGrp, VpssChn,
                                              vpssCtx[VpssGrp]->stChnCfgs[VpssChn].enRotation);
        else
        {
            struct vpss_chn_ldc_cfg cfg;

            cfg.VpssGrp    = VpssGrp;
            cfg.VpssChn    = VpssChn;
            cfg.enRotation = enRotation;
            cfg.stLDCAttr  = *pstLDCAttr;
            cfg.meshHandle = paddr;
            return vpss_set_chn_ldc(&cfg);
        }
    }

    s32Ret = CVI_GDC_GenLDCMesh(ALIGN(vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.u32Width, DEFAULT_ALIGN),
                                ALIGN(vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.u32Height, DEFAULT_ALIGN),
                                &pstLDCAttr->stAttr, "vpss_mesh", &paddr, &vaddr);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) gen mesh fail\n",
                       VpssGrp, VpssChn);
        return s32Ret;
    }

    pthread_mutex_lock(&pmesh->lock);
    if (pmesh->paddr)
    {
        paddr_old = pmesh->paddr;
        vaddr_old = pmesh->vaddr;
    }
    else
    {
        paddr_old = 0;
        vaddr_old = NULL;
    }
    pmesh->paddr                                    = paddr;
    pmesh->vaddr                                    = vaddr;
    vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stLDCAttr  = *pstLDCAttr;
    vpssCtx[VpssGrp]->stChnCfgs[VpssChn].enRotation = enRotation;
    pthread_mutex_unlock(&pmesh->lock);
    if (paddr_old && paddr_old != DEFAULT_MESH_PADDR)
        rt_free_align((void *)paddr_old);

    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "Grp(%d) Chn(%d) mesh base(%llx) vaddr(%p)\n",
                   VpssGrp, VpssChn, paddr, vaddr);

    struct vpss_chn_ldc_cfg cfg;

    cfg.VpssGrp    = VpssGrp;
    cfg.VpssChn    = VpssChn;
    cfg.enRotation = enRotation;
    cfg.stLDCAttr  = *pstLDCAttr;
    cfg.meshHandle = paddr;
    return vpss_set_chn_ldc(&cfg);
}

CVI_BOOL bVpssOpen = CVI_FALSE;

CVI_S32 vpss_dev_open(CVI_VOID)
{
    if (bVpssOpen == CVI_FALSE)
    {
        vpss_open();
        bVpssOpen = CVI_TRUE;
    }

    return CVI_SUCCESS;
}

CVI_S32 vpss_dev_close(CVI_VOID)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (bVpssOpen == CVI_TRUE)
    {
        s32Ret = vpss_close();
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_TRACE_VPSS(CVI_DBG_ERR, "VPSS close failed\n");
            s32Ret = CVI_FAILURE;
            return s32Ret;
        }
        bVpssOpen = CVI_FALSE;
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_Suspend(void)
{
    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "+\n");

    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "-\n");
    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_Resume(void)
{
    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "+\n");

    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "-\n");
    return CVI_SUCCESS;
}
/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_VPSS_CreateGrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
    struct vpss_crt_grp_cfg cfg;

    vpss_dev_open();

    CHECK_VPSS_GRP_VALID(VpssGrp);
    if (vpssCtx[VpssGrp])
    {
        CVI_TRACE_VPSS(CVI_DBG_WARN, "Grp(%d) is occupied\n", VpssGrp);
        return CVI_ERR_VPSS_EXIST;
    }

    cfg.VpssGrp = VpssGrp;
    memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

    if (vpss_create_grp(&cfg) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) VpssDev(%d) create group fail\n",
                       VpssGrp, pstGrpAttr->u8VpssDev);
        return CVI_FAILURE;
    }

    vpssCtx[VpssGrp] = calloc(sizeof(struct cvi_vpss_ctx), 1);
    if (!vpssCtx[VpssGrp])
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssCtx malloc failed\n");
        vpss_destroy_grp(VpssGrp);
        return CVI_ERR_VPSS_NOMEM;
    }

    // for chn rotation, ldc mesh gen
    vpssCtx[VpssGrp]->isCreated = CVI_TRUE;
    vpssCtx[VpssGrp]->chnNum    = (CVI_SYS_GetVPSSMode() == VPSS_MODE_SINGLE) ? VPSS_MAX_CHN_NUM : (pstGrpAttr->u8VpssDev == 0) ? 1
                                                                                                                                : VPSS_MAX_CHN_NUM - 1;
    _vpss_GrpParamInit(VpssGrp);

    for (CVI_U8 i = 0; i < vpssCtx[VpssGrp]->chnNum; ++i)
    {
        memset(&vpssCtx[VpssGrp]->stChnCfgs[i], 0, sizeof(vpssCtx[VpssGrp]->stChnCfgs[i]));
        pthread_mutex_init(&mesh[VpssGrp][i].lock, NULL);
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DestroyGrp(VPSS_GRP VpssGrp)
{
    CHECK_VPSS_GRP_VALID(VpssGrp);
    if (!vpssCtx[VpssGrp])
    {
        CVI_TRACE_VPSS(CVI_DBG_WARN, "Grp(%d) has been destroyed\n", VpssGrp);
        return CVI_SUCCESS;
    }

    if (vpss_destroy_grp(VpssGrp) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) destroy group fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }

    for (CVI_U8 i = 0; i < vpssCtx[VpssGrp]->chnNum; ++i)
    {
        if (mesh[VpssGrp][i].paddr)
        {
            if (mesh[VpssGrp][i].paddr != DEFAULT_MESH_PADDR)
                rt_free_align((void *)mesh[VpssGrp][i].paddr);
            mesh[VpssGrp][i].paddr = 0;
            mesh[VpssGrp][i].vaddr = 0;
        }
        pthread_mutex_destroy(&mesh[VpssGrp][i].lock);
    }

    vpssCtx[VpssGrp]->isCreated = CVI_FALSE;
    free(vpssCtx[VpssGrp]);
    vpssCtx[VpssGrp] = NULL;

    return CVI_SUCCESS;
}

VPSS_GRP CVI_VPSS_GetAvailableGrp(void)
{
    VPSS_GRP grp = VPSS_INVALID_GRP;

    vpss_get_available_grp(&grp);

    return grp;
}

CVI_S32 CVI_VPSS_StartGrp(VPSS_GRP VpssGrp)
{
    struct vpss_str_grp_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    if (vpssCtx[VpssGrp]->isStarted)
    {
        CVI_TRACE_VPSS(CVI_DBG_WARN, "Grp(%d) already started.\n", VpssGrp);
        return CVI_SUCCESS;
    }

    cfg.VpssGrp = VpssGrp;
    if (vpss_start_grp(&cfg) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) start group fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }
    vpssCtx[VpssGrp]->isStarted = CVI_TRUE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_StopGrp(VPSS_GRP VpssGrp)
{
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    if (!vpssCtx[VpssGrp]->isStarted)
    {
        CVI_TRACE_VPSS(CVI_DBG_WARN, "Grp(%d) has been stopped\n", VpssGrp);
        return CVI_SUCCESS;
    }

    if (vpss_stop_grp(VpssGrp) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) stop group fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }

    vpssCtx[VpssGrp]->isStarted = CVI_FALSE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ResetGrp(VPSS_GRP VpssGrp)
{
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    if (vpss_reset_grp(VpssGrp) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) reset group fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }
    vpssCtx[VpssGrp]->isStarted = CVI_FALSE;
    _vpss_GrpParamInit(VpssGrp);

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpAttr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr)
{
    struct vpss_grp_attr cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;

    if (vpss_get_grp_attr(&cfg) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get grp attr fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }

    memcpy(pstGrpAttr, &cfg.stGrpAttr, sizeof(*pstGrpAttr));

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpAttr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
    struct vpss_grp_attr cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

    if (vpss_set_grp_attr(&cfg) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set grp attr fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }

    vpssCtx[VpssGrp]->chnNum = (CVI_SYS_GetVPSSMode() == VPSS_MODE_SINGLE) ? VPSS_MAX_CHN_NUM : (pstGrpAttr->u8VpssDev == 0) ? 1
                                                                                                                             : VPSS_MAX_CHN_NUM - 1;
    CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) VpssDev(%d) u32MaxW(%d) u32MaxH(%d) PixelFmt(%d)\n",
                   VpssGrp, pstGrpAttr->u8VpssDev, pstGrpAttr->u32MaxW, pstGrpAttr->u32MaxH, pstGrpAttr->enPixelFormat);

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpProcAmpCtrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, ctrl);
    CHECK_VPSS_GRP_VALID(VpssGrp);

    if (type >= PROC_AMP_MAX)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
        return CVI_ERR_VPSS_ILLEGAL_PARAM;
    }

    *ctrl = procamp_ctrls[type];
    return CVI_SUCCESS;
}

static void _vpss_proamp_2_csc(const CVI_S32 proc_amp[PROC_AMP_MAX], struct vpss_grp_csc_cfg *csc_cfg)
{
    // for grp proc-amp.
    float  h      = (float)(proc_amp[PROC_AMP_HUE] - 50) * PI / 360;
    float  b_off  = (proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
    float  C_gain = 1 + (proc_amp[PROC_AMP_CONTRAST] - 50) * 0.02;
    float  S      = 1 + (proc_amp[PROC_AMP_SATURATION] - 50) * 0.02;
    float  A      = cos(h) * C_gain * S;
    float  B      = sin(h) * C_gain * S;
    float  C_diff, c_off, tmp;
    CVI_U8 sub_0_l, add_0_l, add_1_l, add_2_l;

    if (proc_amp[PROC_AMP_CONTRAST] > 50)
        C_diff = 256 / C_gain;
    else
        C_diff = 256 * C_gain;
    c_off = 128 - (C_diff / 2);

    if (b_off < 0)
    {
        sub_0_l = abs(proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
        add_0_l = 0;
        add_1_l = 0;
        add_2_l = 0;
    }
    else
    {
        sub_0_l = 0;
        add_0_l = C_gain * b_off;
        add_1_l = add_0_l;
        add_2_l = add_0_l;
    }

    if (proc_amp[PROC_AMP_CONTRAST] > 50)
    {
        csc_cfg->sub[0] = sub_0_l + c_off;
        csc_cfg->add[0] = add_0_l;
        csc_cfg->add[1] = add_1_l;
        csc_cfg->add[2] = add_2_l;
    }
    else
    {
        csc_cfg->sub[0] = sub_0_l;
        csc_cfg->add[0] = add_0_l + c_off;
        csc_cfg->add[1] = add_1_l + c_off;
        csc_cfg->add[2] = add_2_l + c_off;
    }
    csc_cfg->sub[1] = 128;
    csc_cfg->sub[2] = 128;

    csc_cfg->coef[0][0] = C_gain * BIT(10);
    tmp                 = B * -1.402;
    csc_cfg->coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
    tmp                 = A * 1.402;
    csc_cfg->coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
    csc_cfg->coef[1][0] = C_gain * BIT(10);
    tmp                 = A * -0.344 + B * 0.714;
    csc_cfg->coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
    tmp                 = B * -0.344 + A * -0.714;
    csc_cfg->coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
    csc_cfg->coef[2][0] = C_gain * BIT(10);
    tmp                 = A * 1.772;
    csc_cfg->coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
    tmp                 = B * 1.772;
    csc_cfg->coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[0][0]: %#4x coef[0][1]: %#4x coef[0][2]: %#4x\n",
                   csc_cfg->coef[0][0], csc_cfg->coef[0][1], csc_cfg->coef[0][2]);
    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[1][0]: %#4x coef[1][1]: %#4x coef[1][2]: %#4x\n",
                   csc_cfg->coef[1][0], csc_cfg->coef[1][1], csc_cfg->coef[1][2]);
    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[2][0]: %#4x coef[2][1]: %#4x coef[2][2]: %#4x\n",
                   csc_cfg->coef[2][0], csc_cfg->coef[2][1], csc_cfg->coef[2][2]);
    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "sub[0]: %3d sub[1]: %3d sub[2]: %3d\n",
                   csc_cfg->sub[0], csc_cfg->sub[1], csc_cfg->sub[2]);
    CVI_TRACE_VPSS(CVI_DBG_DEBUG, "add[0]: %3d add[1]: %3d add[2]: %3d\n",
                   csc_cfg->add[0], csc_cfg->add[1], csc_cfg->add[2]);
}

CVI_S32 CVI_VPSS_GetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, value);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    if (type >= PROC_AMP_MAX)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
        return CVI_ERR_VPSS_ILLEGAL_PARAM;
    }

    *value = vpssCtx[VpssGrp]->proc_amp[type];
    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, const CVI_S32 value)
{
    PROC_AMP_CTRL_S ctrl;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    if (type >= PROC_AMP_MAX)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
        return CVI_ERR_VPSS_ILLEGAL_PARAM;
    }

    CVI_VPSS_GetGrpProcAmpCtrl(VpssGrp, type, &ctrl);
    if ((value > ctrl.maximum) || (value < ctrl.minimum))
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) new value(%d) out of range(%d ~ %d).\n",
                       VpssGrp, value, ctrl.minimum, ctrl.maximum);
        return CVI_ERR_VPSS_ILLEGAL_PARAM;
    }

    vpssCtx[VpssGrp]->proc_amp[type] = value;
    //#if defined(__CV181X__)

    struct vpss_grp_csc_cfg csc_cfg;

    memset(&csc_cfg, 0, sizeof(csc_cfg));
    csc_cfg.VpssGrp = VpssGrp;
    _vpss_proamp_2_csc(vpssCtx[VpssGrp]->proc_amp, &csc_cfg);

    if (vpss_set_grp_csc(&csc_cfg) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) set group csc fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }
    //#endif

    return CVI_SUCCESS;
}

static CVI_VOID _vpss_check_normalize(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
    if (pstChnAttr->stNormalize.bEnable)
    {
        for (CVI_U8 i = 0; i < 3; ++i)
        {
            if (pstChnAttr->stNormalize.factor[i] >= 1.0f)
            {
                vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.stNormalize.factor[i] = 1.0f - 1.0f / 8192;
                CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with max value 8191/8192\n", i);
            }
            if (pstChnAttr->stNormalize.factor[i] < (1.0f / 8192))
            {
                vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.stNormalize.factor[i] = (1.0f / 8192);
                CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with min value 1/8192\n", i);
            }
            if (pstChnAttr->stNormalize.mean[i] > 255.0f)
            {
                vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.stNormalize.mean[i] = 255.0f;
                CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with max value 255\n", i);
            }
            if (pstChnAttr->stNormalize.mean[i] < 0)
            {
                vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.stNormalize.mean[i] = 0;
                CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with min value 0\n", i);
            }
        }
    }
}

/* Chn Settings */
CVI_S32 CVI_VPSS_SetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
    struct vpss_chn_attr attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    // Handle float poing in user space
    _vpss_check_normalize(VpssGrp, VpssChn, pstChnAttr);

    // for chn rotation, mesh gen
    vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr = *pstChnAttr;
    memcpy(&attr.stChnAttr, pstChnAttr, sizeof(attr.stChnAttr));

    if (vpss_set_chn_attr(&attr) != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) set chn attr fail\n",
                       VpssGrp);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
    struct vpss_chn_attr attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    if (vpss_get_chn_attr(&attr) != CVI_SUCCESS)
        return CVI_FAILURE;

    *pstChnAttr = attr.stChnAttr;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

    // for chn rotation, mesh gen
    struct VPSS_CHN_CFG *chn_cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    chn_cfg            = &vpssCtx[VpssGrp]->stChnCfgs[VpssChn];
    chn_cfg->isEnabled = CVI_TRUE;

    if (vpss_enable_chn(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    if (vpss_disable_chn(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo)
{
    struct vpss_chn_crop_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp    = VpssGrp;
    cfg.VpssChn    = VpssChn;
    cfg.stCropInfo = *pstCropInfo;

    if (vpss_set_chn_crop(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo)
{
    struct vpss_chn_crop_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_get_chn_crop(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    *pstCropInfo = cfg.stCropInfo;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ShowChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    struct vpss_en_chn_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_show_chn(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) Chn(%d)\n", VpssGrp, VpssChn);

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_HideChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    struct vpss_en_chn_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_hide_chn(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    CVI_TRACE_VPSS(CVI_DBG_INFO, "Grp(%d) Chn(%d)\n", VpssGrp, VpssChn);

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpCrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo)
{
    struct vpss_grp_crop_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;

    if (vpss_get_grp_crop(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    *pstCropInfo = cfg.stCropInfo;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpCrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo)
{
    struct vpss_grp_crop_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp    = VpssGrp;
    cfg.stCropInfo = *pstCropInfo;

    if (vpss_set_grp_crop(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

//TBD
CVI_S32 CVI_VPSS_GetGrpFrame(VPSS_GRP VpssGrp, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    CVI_TRACE_VPSS(CVI_DBG_ERR, "Not support get group frame\n");
    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ReleaseGrpFrame(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SendFrame(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
    struct vpss_snd_frm_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);

    cfg.VpssGrp = VpssGrp;
    memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
    cfg.s32MilliSec = s32MilliSec;

    if (vpss_send_frame(&cfg) != CVI_SUCCESS)
        return CVI_ERR_VPSS_BUSY;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SendChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
    struct vpss_chn_frm_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
    cfg.s32MilliSec = s32MilliSec;

    if (vpss_send_chn_frame(&cfg) != CVI_SUCCESS)
        return CVI_ERR_VPSS_BUSY;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
                             CVI_S32 s32MilliSec)
{
    struct vpss_chn_frm_cfg cfg;
    CVI_S32                 ret;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstFrameInfo);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp     = VpssGrp;
    cfg.VpssChn     = VpssChn;
    cfg.s32MilliSec = s32MilliSec;

    ret = vpss_get_chn_frame(&cfg);
    if (ret == CVI_SUCCESS)
        memcpy(pstFrameInfo, &cfg.stVideoFrame, sizeof(*pstFrameInfo));

    return ret;
}

CVI_S32 CVI_VPSS_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    struct vpss_chn_frm_cfg cfg;
    CVI_S32                 ret;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));

    ret = vpss_release_chn_frame(&cfg);
    if (ret != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) release chn frame fail\n", VpssGrp, VpssChn);
        return ret;
    }

    return CVI_SUCCESS;
}


CVI_S32 CVI_VPSS_SetModParam(const VPSS_MOD_PARAM_S *pstModParam)
{
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstModParam);

    //todo: how to use vpssmod param...
    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetModParam(VPSS_MOD_PARAM_S *pstModParam)
{
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstModParam);

    pstModParam->u32VpssSplitNodeNum = 1;
    pstModParam->u32VpssVbSource     = 0; //vb from common vb pool
    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);
    CHECK_VPSS_GDC_FMT(VpssGrp, VpssChn, vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.enPixelFormat);

    struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];

    if (enRotation >= ROTATION_MAX)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid rotation(%d).\n", VpssGrp, VpssChn, enRotation);
        return CVI_ERR_VPSS_ILLEGAL_PARAM;
    }
    else if (enRotation == vpssCtx[VpssGrp]->stChnCfgs[VpssChn].enRotation)
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "rotation(%d) not changed.\n", enRotation);
        return CVI_SUCCESS;
    }
    else if (!vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stLDCAttr.bEnable && enRotation == ROTATION_0)
    {
        pthread_mutex_lock(&pmesh->lock);
        vpssCtx[VpssGrp]->stChnCfgs[VpssChn].enRotation = enRotation;
        pthread_mutex_unlock(&pmesh->lock);
        //return CVI_SUCCESS;
    }

    if (vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stLDCAttr.bEnable)
        return _vpss_update_ldc_mesh(VpssGrp, VpssChn,
                                     &vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stLDCAttr, enRotation);
    else
        return _vpss_update_rotation_mesh(VpssGrp, VpssChn, enRotation);
}

CVI_S32 CVI_VPSS_GetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation)
{
    struct vpss_chn_rot_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_get_chn_rotation(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    *penRotation = cfg.enRotation;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align)
{
    struct vpss_chn_align_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp  = VpssGrp;
    cfg.VpssChn  = VpssChn;
    cfg.u32Align = u32Align;

    if (vpss_set_chn_align(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align)
{
    struct vpss_chn_align_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pu32Align);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_get_chn_align(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    *pu32Align = cfg.u32Align;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef)
{
    struct vpss_chn_coef_level_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    cfg.enCoef  = enCoef;

    if (vpss_set_coef_level(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef)
{
    struct vpss_chn_coef_level_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penCoef);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_get_coef_level(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    *penCoef = cfg.enCoef;

    return CVI_SUCCESS;
}

/* CVI_VPSS_SetChnYRatio: Modify the y ratio of chn output. Only work for yuv format.
 *
 * @param VpssGrp: The Vpss Grp to work.
 * @param VpssChn: The Vpss Chn to work.
 * @param YRatio: Output's Y will be sacled by this ratio.
 * @return: CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VPSS_SetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio)
{
    struct vpss_chn_yratio_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    cfg.YRatio  = (CVI_U32)(YRatio * 100);

    if (vpss_set_chn_yratio(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio)
{
    struct vpss_chn_yratio_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pYRatio);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_get_chn_yratio(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    *pYRatio = (1.0f * cfg.YRatio) / 100.0;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);
    CHECK_VPSS_GDC_FMT(VpssGrp, VpssChn, vpssCtx[VpssGrp]->stChnCfgs[VpssChn].stChnAttr.enPixelFormat);

    return _vpss_update_ldc_mesh(VpssGrp, VpssChn, pstLDCAttr, vpssCtx[VpssGrp]->stChnCfgs[VpssChn].enRotation);
}

CVI_S32 CVI_VPSS_GetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
    struct vpss_chn_ldc_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;

    if (vpss_get_chn_ldc(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    memcpy(pstLDCAttr, &cfg.stLDCAttr, sizeof(*pstLDCAttr));

    return CVI_SUCCESS;
}

/* CVI_VPSS_SetGrpParamfromBin: Apply the settings of scene from bin
 *
 * @param VpssGrp: the vpss grp to apply
 * @param scene: the scene of settings stored in bin to use
 * @return: result of the API
 */
CVI_S32 CVI_VPSS_SetGrpParamfromBin(VPSS_GRP VpssGrp, CVI_U8 scene)
{
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    if (scene > VPSS_MAX_GRP_NUM)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "scene(%d) is over max(%d)\n", scene, VPSS_MAX_GRP_NUM);
        return CVI_ERR_VPSS_ILLEGAL_PARAM;
    }
    if (g_bLoadBinDone)
    {
        memcpy(vpssCtx[VpssGrp]->proc_amp, vpss_bin_data[scene].proc_amp, sizeof(vpssCtx[VpssGrp]->proc_amp));
        CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is exist, vpss grp param use pqbin value !!\n");
    }
    else
    {
        CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is not find, vpss grp param use default !!\n");
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_AttachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool)
{
    struct vpss_vb_pool_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    cfg.hVbPool = hVbPool;
    return vpss_attach_vbpool(&cfg);
}

CVI_S32 CVI_VPSS_DetachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    struct vpss_vb_pool_cfg cfg;

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    memset(&cfg, 0, sizeof(cfg));
    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    return vpss_detach_vbpool(&cfg);
}

CVI_S32 CVI_VPSS_GetRegionLuma(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_REGION_INFO_S *pstRegionInfo,
                               CVI_U64 *pu64LumaData, CVI_S32 s32MilliSec)
{
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstRegionInfo);
    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pu64LumaData);

    CVI_S32            ret = 0;
    VIDEO_FRAME_INFO_S stVideoFrame;
    CVI_U8            *pstVirAddr;
    SIZE_S             stSize;
    CVI_U32            u32X, u32Y, u32XStep, u32YStep, u32Num;
    CVI_U32            u32MainStride;
    CVI_S32            s32StartX, s32StartY;

    s32StartX        = pstRegionInfo->pstRegion->s32X;
    s32StartY        = pstRegionInfo->pstRegion->s32Y;
    stSize.u32Width  = pstRegionInfo->pstRegion->u32Width;
    stSize.u32Height = pstRegionInfo->pstRegion->u32Height;
    if ((s32StartX < 0) || (s32StartY < 0))
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "region info(%d %d %d %d) invalid.\n", s32StartX, s32StartY, stSize.u32Width, stSize.u32Height);
        return CVI_ERR_VPSS_ILLEGAL_PARAM;
    }

    if (!vpssCtx[VpssGrp]->isStarted)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) not yet started.\n", VpssGrp);
        return CVI_ERR_VPSS_NOTREADY;
    }
    if (!vpssCtx[VpssGrp]->stChnCfgs[VpssChn].isEnabled)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) not yet enabled.\n", VpssGrp, VpssChn);
        return CVI_ERR_VPSS_NOTREADY;
    }

    ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, s32MilliSec);
    if (ret != CVI_SUCCESS)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get buf fail\n", VpssGrp, VpssChn);
        return CVI_ERR_VPSS_BUF_EMPTY;
    }

    if ((s32StartX + stSize.u32Width > stVideoFrame.stVFrame.u32Width) || (s32StartY + stSize.u32Height > stVideoFrame.stVFrame.u32Height) || ((CVI_U32)s32StartX >= stVideoFrame.stVFrame.u32Width) || ((CVI_U32)s32StartY >= stVideoFrame.stVFrame.u32Height) || (stSize.u32Width > stVideoFrame.stVFrame.u32Width) || (stSize.u32Height > stVideoFrame.stVFrame.u32Height))
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "size(%d %d %d %d) out of range.\n", s32StartX, s32StartY, stSize.u32Width, stSize.u32Height);
        ret = CVI_ERR_VPSS_ILLEGAL_PARAM;
        goto release_blk;
    }

    if (!IS_FMT_YUV(stVideoFrame.stVFrame.enPixelFormat))
    {
        ret = CVI_ERR_VPSS_NOT_SUPPORT;
        CVI_TRACE_VPSS(CVI_DBG_ERR, "only support yuv-fmt(%d).\n", stVideoFrame.stVFrame.enPixelFormat);
        goto release_blk;
    }

    size_t Luma_size = stVideoFrame.stVFrame.u32Length[0];

    pstVirAddr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], Luma_size);
    if (pstVirAddr == NULL)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "mmap for stVideoFrame failed.\n");
        ret = CVI_FAILURE;
        goto release_blk;
    }

    u32MainStride = stVideoFrame.stVFrame.u32Stride[0];

    u32Num        = 0;
    *pu64LumaData = 0;
    u32XStep      = stSize.u32Width > 9 ? stSize.u32Width / 9 : 1;
    u32YStep      = stSize.u32Height > 9 ? stSize.u32Height / 9 : 1;

    for (u32Y = s32StartY; u32Y < s32StartY + stSize.u32Height; u32Y += u32YStep)
    {
        for (u32X = s32StartX; u32X < (s32StartX + stSize.u32Width); u32X += u32XStep)
        {
            *pu64LumaData += *(pstVirAddr + u32X + u32Y * u32MainStride);
            u32Num++;
        }
    }

    for (u32X = s32StartX + u32XStep / 2; u32X < (s32StartX + stSize.u32Width); u32X += u32XStep)
    {
        for (u32Y = s32StartY + u32YStep / 2; u32Y < (s32StartY + stSize.u32Height); u32Y += u32YStep)
        {
            *pu64LumaData += *(pstVirAddr + u32X + u32Y * u32MainStride);
            u32Num++;
        }
    }

    *pu64LumaData = *pu64LumaData / u32Num;

    CVI_SYS_Munmap(pstVirAddr, Luma_size);
release_blk:
    if (CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame) != CVI_SUCCESS)
        return CVI_FAILURE;
    return ret;
}

CVI_S32 CVI_VPSS_TriggerSnapFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32FrameCnt)
{
    struct vpss_snap_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn, .frame_cnt = u32FrameCnt};

    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    if (vpss_trigger_snap_frame(&cfg) != CVI_SUCCESS)
        return CVI_FAILURE;

    return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnBufWrapAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
    struct vpss_chn_wrap_cfg cfg;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    cfg.wrap    = *pstVpssChnBufWrap;
    return vpss_set_chn_wrap(&cfg);
}

CVI_S32 CVI_VPSS_GetChnBufWrapAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap)
{
    struct vpss_chn_wrap_cfg cfg;
    CVI_S32                  ret;

    MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVpssChnBufWrap);
    CHECK_VPSS_GRP_VALID(VpssGrp);
    CHECK_VPSS_GRP_CREATED(VpssGrp);
    CHECK_VPSS_CHN_VALID(VpssGrp, VpssChn);

    cfg.VpssGrp = VpssGrp;
    cfg.VpssChn = VpssChn;
    cfg.wrap    = *pstVpssChnBufWrap;

    ret = vpss_get_chn_wrap(&cfg);
    if (ret == CVI_SUCCESS)
        memcpy(pstVpssChnBufWrap, &cfg.wrap, sizeof(*pstVpssChnBufWrap));

    return ret;
}

CVI_U32 CVI_VPSS_GetWrapBufferSize(CVI_U32 u32Width, CVI_U32 u32Height, PIXEL_FORMAT_E enPixelFormat,
                                   CVI_U32 u32BufLine, CVI_U32 u32BufDepth)
{
    CVI_U32         u32BufSize;
    VB_CAL_CONFIG_S stCalConfig;

    if (u32Width < 64 || u32Height < 64)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "width(%d) or height(%d) too small\n", u32Width, u32Height);
        return 0;
    }
    if (u32BufLine != 64 && u32BufLine != 128)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "u32BufLine(%d) invalid, only 64 or 128 lines\n",
                       u32BufLine);
        return 0;
    }
    if (u32BufDepth < 2 || u32BufDepth > 32)
    {
        CVI_TRACE_VPSS(CVI_DBG_ERR, "u32BufDepth(%d) invalid, 2 ~ 32\n",
                       u32BufDepth);
        return 0;
    }

    COMMON_GetPicBufferConfig(u32Width, u32Height, enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stCalConfig);

    u32BufSize  = stCalConfig.u32VBSize / u32Height;
    u32BufSize *= u32BufLine * u32BufDepth;
    CVI_TRACE_VPSS(CVI_DBG_INFO, "width(%d), height(%d), u32BufSize=%d\n",
                   u32Width, u32Height, u32BufSize);

    return u32BufSize;
}
