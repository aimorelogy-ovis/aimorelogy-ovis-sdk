#include "app_ipcam_vpss.h"
#include "app_ipcam_sys.h"
#include "app_ipcam_paramparse.h"
#include "cvi_sys.h"
#include <pthread.h>
#include <sys/prctl.h>

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_VPSS_CFG_T g_stVpssCfg;
APP_PARAM_VPSS_CFG_T *g_pstVpssCfg = &g_stVpssCfg;

/* vpss stitch thread: configurable grp/chn stitching */
static CVI_BOOL g_VpssStitchRunning = CVI_FALSE;
static pthread_t g_VpssStitchThread = 0;

static void *app_ipcam_Vpss_Stitch_Proc(void *arg)
{
    APP_PARAM_VPSS_CFG_T *pstVpssCfg = app_ipcam_Vpss_Param_Get();
    (void)arg;

    prctl(PR_SET_NAME, "vpss_stitch", 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "vpss_stitch running\n");

    while (g_VpssStitchRunning) {
        for (CVI_U32 i = 0; i < pstVpssCfg->u32StitchPairCnt; i++) {
            CVI_S32 s32Ret = CVI_SUCCESS;
            VIDEO_FRAME_INFO_S stFrame = {0};
            VPSS_GRP srcGrp = pstVpssCfg->astStitchPair[i].srcGrp;
            VPSS_CHN srcChn = pstVpssCfg->astStitchPair[i].srcChn;
            VPSS_GRP dstGrp = pstVpssCfg->astStitchPair[i].dstGrp;
            VPSS_CHN dstChn = pstVpssCfg->astStitchPair[i].dstChn;

            s32Ret = CVI_VPSS_GetChnFrame(srcGrp, srcChn, &stFrame, 3000);
            if (s32Ret == CVI_SUCCESS) {
                /* 使用 SendChnFrame，把 srcGrp/srcChn 的内容叠加/送到 dstGrp/dstChn */
                s32Ret = CVI_VPSS_SendChnFrame(dstGrp, dstChn, &stFrame, 3000);
                CVI_VPSS_ReleaseChnFrame(srcGrp, srcChn, &stFrame);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,
                        "stitch[%d]: SendChnFrame %d/%d -> %d/%d fail, ret=0x%x\n",
                        i, srcGrp, srcChn, dstGrp, dstChn, s32Ret);
                }
            } else {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "stitch[%d]: GetChnFrame %d/%d fail, ret=0x%x\n",
                    i, srcGrp, srcChn, s32Ret);
            }
        }
    }

    return NULL;
}

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_VPSS_CFG_T *app_ipcam_Vpss_Param_Get(void)
{
    return g_pstVpssCfg;
}

int app_ipcam_Vpss_Mode_Set(void)
{
    CVI_S32 s32Ret;
    APP_PARAM_MODULE_CFG_S *pModuleCfg = app_ipcam_Module_Param_Get();

    if (pModuleCfg->alios_vpss_mode)
        return CVI_SUCCESS;

    s32Ret = CVI_SYS_SetVIVPSSMode(&app_ipcam_Sys_Param_Get()->stVIVPSSMode);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_SYS_SetVIVPSSMode failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VPSS_SetMode(&g_pstVpssCfg->stVPSSMode);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "VI-VPSS modes configured before VI initialization\n");
    return CVI_SUCCESS;
}

int app_ipcam_Vpss_Destroy(VPSS_GRP VpssGrp)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_VPSS_GRP_CFG_T *pstVpssGrpCfg = &g_pstVpssCfg->astVpssGrpCfg[VpssGrp];
    if (!pstVpssGrpCfg->bCreate) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "VpssGrp(%d) not create yet!\n", VpssGrp);
        return CVI_SUCCESS;
    }

    for (VPSS_CHN VpssChn = 0; VpssChn < VPSS_MAX_PHY_CHN_NUM; VpssChn++) {
        if (pstVpssGrpCfg->abChnCreate[VpssChn]) {
            s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_DisableChn failed with %#x!\n", s32Ret);
                return CVI_FAILURE;
            }
            pstVpssGrpCfg->abChnCreate[VpssChn] = CVI_FALSE;
        }
    }

    s32Ret = CVI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_StopGrp failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_DestroyGrp failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    pstVpssGrpCfg->bCreate = CVI_FALSE;

    return CVI_SUCCESS;
}

int app_ipcam_Vpss_Create(VPSS_GRP VpssGrp)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PARAM_SYS_CFG_S * pstSysCfg = app_ipcam_Sys_Param_Get();
    APP_VPSS_GRP_CFG_T *pstVpssGrpCfg = &g_pstVpssCfg->astVpssGrpCfg[VpssGrp];
    if (!pstVpssGrpCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "VpssGrp(%d) not Enable!\n", VpssGrp);
        return CVI_SUCCESS;
    }

    if (pstVpssGrpCfg->bCreate) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "VpssGrp(%d) have been created!\n", VpssGrp);
        return CVI_SUCCESS;
    }

    s32Ret = CVI_VPSS_CreateGrp(pstVpssGrpCfg->VpssGrp, &pstVpssGrpCfg->stVpssGrpAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", pstVpssGrpCfg->VpssGrp, s32Ret);
        goto VPSS_EXIT;
    }

    s32Ret = CVI_VPSS_ResetGrp(pstVpssGrpCfg->VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_ResetGrp(grp:%d) failed with %#x!\n", pstVpssGrpCfg->VpssGrp, s32Ret);
        goto VPSS_EXIT;
    }

    for (VPSS_CHN VpssChn = 0; VpssChn < VPSS_MAX_PHY_CHN_NUM; VpssChn++) {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "\tChnID=%d isEnable=%d\n", VpssChn, pstVpssGrpCfg->abChnEnable[VpssChn]);
        if (pstVpssGrpCfg->abChnEnable[VpssChn]) {

            s32Ret = CVI_VPSS_SetChnAttr(
                pstVpssGrpCfg->VpssGrp, VpssChn,
                &pstVpssGrpCfg->astVpssChnAttr[VpssChn]);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "CVI_VPSS_SetChnAttr(%d) failed with %#x\n",
                    VpssChn, s32Ret);
                goto VPSS_EXIT;
            }

            s32Ret = CVI_VPSS_SetChnCrop(
                pstVpssGrpCfg->VpssGrp, VpssChn,
                &pstVpssGrpCfg->stVpssChnCropInfo[VpssChn]);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "CVI_VPSS_SetChnCrop(%d) failed with %#x\n",
                    VpssChn, s32Ret);
                goto VPSS_EXIT;
            }

            VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap = {0};
            for (int idx = 0; idx < pstSysCfg->u8SbmCnt; ++idx) {
                if ((VpssGrp == pstSysCfg->stSbmCfg[idx].s32SbmGrp) && (VpssChn == pstSysCfg->stSbmCfg[idx].s32SbmChn)) {
                    stVpssChnBufWrap.bEnable = CVI_TRUE;
                    stVpssChnBufWrap.u32BufLine = pstSysCfg->stSbmCfg[idx].s32WrapBufLine;
                    stVpssChnBufWrap.u32WrapBufferSize = pstSysCfg->stSbmCfg[idx].s32WrapBufSize;
                    s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, &stVpssChnBufWrap);
                    if (s32Ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_SetChnBufWrapAttr failed with %#x\n", s32Ret);
                        goto VPSS_EXIT;
                    }
                    break;
                }
            }

            s32Ret = CVI_VPSS_EnableChn(pstVpssGrpCfg->VpssGrp, VpssChn);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "CVI_VPSS_EnableChn(%d) failed with %#x\n",
                    VpssChn, s32Ret);
                goto VPSS_EXIT;
            }

            pstVpssGrpCfg->abChnCreate[VpssChn] = CVI_TRUE;

            if (pstVpssGrpCfg->aAttachEn[VpssChn]) {
                s32Ret = CVI_VPSS_AttachVbPool(
                    pstVpssGrpCfg->VpssGrp, VpssChn,
                    pstVpssGrpCfg->aAttachPool[VpssChn]);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,
                        "CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
                    goto VPSS_EXIT;
                }
            }
        }
    }

    s32Ret = CVI_VPSS_StartGrp(pstVpssGrpCfg->VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    pstVpssGrpCfg->bCreate = CVI_TRUE;

    return CVI_SUCCESS;

VPSS_EXIT:

    return s32Ret;
}

int app_ipcam_Vpss_Unbind(VPSS_GRP VpssGrp)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_VPSS_GRP_CFG_T *pstVpssGrpCfg = &g_pstVpssCfg->astVpssGrpCfg[VpssGrp];
    if (!pstVpssGrpCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "VpssGrp(%d) not Enable!\n", VpssGrp);
        return CVI_SUCCESS;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "VpssGrp=%d bindMode:%d\n", VpssGrp, pstVpssGrpCfg->bBindMode);

    if (pstVpssGrpCfg->bBindMode && pstVpssGrpCfg->bBound) {
        s32Ret = CVI_SYS_UnBind(&pstVpssGrpCfg->astChn[0], &pstVpssGrpCfg->astChn[1]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_UnBind failed with %#x\n", s32Ret);
            return s32Ret;
        }
        pstVpssGrpCfg->bBound = CVI_FALSE;
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vpss_Bind(VPSS_GRP VpssGrp)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_VPSS_GRP_CFG_T *pstVpssGrpCfg = &g_pstVpssCfg->astVpssGrpCfg[VpssGrp];
    if (!pstVpssGrpCfg->bEnable) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "VpssGrp(%d) not Enable!\n", VpssGrp);
        return CVI_SUCCESS;
    }

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "GrpID=%d isEnable=%d\n", pstVpssGrpCfg->VpssGrp, pstVpssGrpCfg->bEnable);

    if (pstVpssGrpCfg->bBindMode && !pstVpssGrpCfg->bBound) {
        s32Ret = CVI_SYS_Bind(&pstVpssGrpCfg->astChn[0], &pstVpssGrpCfg->astChn[1]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_Bind failed with %#x\n", s32Ret);
            return s32Ret;
        }
        pstVpssGrpCfg->bBound = CVI_TRUE;
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vpss_Chn_SetEnabled(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
                                  CVI_BOOL bEnable)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_VPSS_GRP_CFG_T *pstVpssGrpCfg = NULL;

    if (VpssGrp < 0 || VpssGrp >= CVI_MAX_VPSS_GRP ||
        VpssChn < 0 || VpssChn >= VPSS_MAX_PHY_CHN_NUM) {
        return CVI_FAILURE;
    }

    pstVpssGrpCfg = &g_pstVpssCfg->astVpssGrpCfg[VpssGrp];
    if (!pstVpssGrpCfg->bCreate ||
        !pstVpssGrpCfg->abChnEnable[VpssChn]) {
        return CVI_FAILURE;
    }

    if (bEnable == pstVpssGrpCfg->abChnCreate[VpssChn]) {
        return CVI_SUCCESS;
    }

    if (bEnable) {
        s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
    } else {
        s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
    }
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_VPSS_%sChn(%d, %d) failed with %#x\n",
            bEnable ? "Enable" : "Disable", VpssGrp, VpssChn, s32Ret);
        return s32Ret;
    }

    pstVpssGrpCfg->abChnCreate[VpssChn] = bEnable;
    return CVI_SUCCESS;
}

int app_ipcam_Vpss_Chn_Reconfigure(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
                                   const VPSS_CHN_ATTR_S *pstChnAttr,
                                   CVI_BOOL bAttach, VB_POOL VbPool)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 s32OriginalRet = CVI_SUCCESS;
    CVI_BOOL bOldAttach = CVI_FALSE;
    CVI_BOOL bNewPoolAttached = CVI_FALSE;
    VB_POOL OldVbPool = VB_INVALID_POOLID;
    VPSS_CHN_ATTR_S stOldChnAttr = {0};
    APP_VPSS_GRP_CFG_T *pstVpssGrpCfg = NULL;

    if (VpssGrp < 0 || VpssGrp >= CVI_MAX_VPSS_GRP ||
        VpssChn < 0 || VpssChn >= VPSS_MAX_PHY_CHN_NUM ||
        pstChnAttr == NULL || (bAttach && VbPool == VB_INVALID_POOLID)) {
        return CVI_FAILURE;
    }

    pstVpssGrpCfg = &g_pstVpssCfg->astVpssGrpCfg[VpssGrp];
    if (!pstVpssGrpCfg->bCreate ||
        !pstVpssGrpCfg->abChnEnable[VpssChn] ||
        !pstVpssGrpCfg->abChnCreate[VpssChn]) {
        return CVI_FAILURE;
    }

    stOldChnAttr = pstVpssGrpCfg->astVpssChnAttr[VpssChn];
    bOldAttach = pstVpssGrpCfg->aAttachEn[VpssChn] ? CVI_TRUE : CVI_FALSE;
    if (bOldAttach) {
        OldVbPool = pstVpssGrpCfg->aAttachPool[VpssChn];
    }

    s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_VPSS_DisableChn(%d, %d) failed with %#x\n",
            VpssGrp, VpssChn, s32Ret);
        return s32Ret;
    }
    pstVpssGrpCfg->abChnCreate[VpssChn] = CVI_FALSE;

    if (bOldAttach) {
        s32Ret = CVI_VPSS_DetachVbPool(VpssGrp, VpssChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "CVI_VPSS_DetachVbPool(%d, %d) failed with %#x\n",
                VpssGrp, VpssChn, s32Ret);
            s32OriginalRet = s32Ret;
            goto restore_enabled_channel;
        }
    }

    s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, pstChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_VPSS_SetChnAttr(%d, %d) failed with %#x\n",
            VpssGrp, VpssChn, s32Ret);
        s32OriginalRet = s32Ret;
        goto restore_old_channel;
    }

    if (bAttach) {
        s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, VbPool);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "CVI_VPSS_AttachVbPool(%d, %d, %u) failed with %#x\n",
                VpssGrp, VpssChn, VbPool, s32Ret);
            s32OriginalRet = s32Ret;
            goto restore_old_channel;
        }
        bNewPoolAttached = CVI_TRUE;
    }

    s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI_VPSS_EnableChn(%d, %d) failed with %#x\n",
            VpssGrp, VpssChn, s32Ret);
        s32OriginalRet = s32Ret;
        goto restore_old_channel;
    }

    pstVpssGrpCfg->astVpssChnAttr[VpssChn] = *pstChnAttr;
    pstVpssGrpCfg->aAttachEn[VpssChn] = bAttach;
    pstVpssGrpCfg->aAttachPool[VpssChn] =
        bAttach ? VbPool : VB_INVALID_POOLID;
    pstVpssGrpCfg->abChnCreate[VpssChn] = CVI_TRUE;
    return CVI_SUCCESS;

restore_old_channel:
    if (bNewPoolAttached) {
        s32Ret = CVI_VPSS_DetachVbPool(VpssGrp, VpssChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "rollback CVI_VPSS_DetachVbPool(%d, %d) failed with %#x\n",
                VpssGrp, VpssChn, s32Ret);
        }
    }
    s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stOldChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "rollback CVI_VPSS_SetChnAttr(%d, %d) failed with %#x\n",
            VpssGrp, VpssChn, s32Ret);
    }
    if (bOldAttach) {
        s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, OldVbPool);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "rollback CVI_VPSS_AttachVbPool(%d, %d, %u) failed with %#x\n",
                VpssGrp, VpssChn, OldVbPool, s32Ret);
        }
    }

restore_enabled_channel:
    s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret == CVI_SUCCESS) {
        pstVpssGrpCfg->abChnCreate[VpssChn] = CVI_TRUE;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "rollback CVI_VPSS_EnableChn(%d, %d) failed with %#x\n",
            VpssGrp, VpssChn, s32Ret);
    }
    return s32OriginalRet;
}

int app_ipcam_Vpss_DeInit(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PARAM_MODULE_CFG_S * pModuleCfg = app_ipcam_Module_Param_Get();
    /* stop vpss stitch thread before destroy grp */
    if (g_VpssStitchRunning) {
        g_VpssStitchRunning = CVI_FALSE;
        if (g_VpssStitchThread != 0) {
            pthread_join(g_VpssStitchThread, NULL);
            g_VpssStitchThread = 0;
        }
    }

    if(!pModuleCfg->alios_vpss_mode){
        for (CVI_U32 VpssGrp = 0; VpssGrp < g_pstVpssCfg->u32GrpCnt; VpssGrp++) {
            s32Ret = app_ipcam_Vpss_Destroy(VpssGrp);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Vpss grp(%d) Destroy failed with 0x%x!\n", VpssGrp, s32Ret);
                return s32Ret;
            }

            s32Ret = app_ipcam_Vpss_Unbind(VpssGrp);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Vpss grp(%d) Unbind failed with 0x%x!\n", VpssGrp, s32Ret);
                return s32Ret;
            }
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Vpss_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PARAM_MODULE_CFG_S * pModuleCfg = app_ipcam_Module_Param_Get();
    pthread_attr_t attr;

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "vpss init ------------------> start \n");

    if(!pModuleCfg->alios_vpss_mode){
        for (CVI_U32 VpssGrp = 0; VpssGrp < g_pstVpssCfg->u32GrpCnt; VpssGrp++) {
            s32Ret = app_ipcam_Vpss_Create(VpssGrp);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Vpss grp(%d) Create failed with 0x%x!\n", VpssGrp, s32Ret);
                return s32Ret;
            }
            s32Ret = app_ipcam_Vpss_Bind(VpssGrp);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Vpss grp(%d) Bind failed with 0x%x!\n", VpssGrp, s32Ret);
                return s32Ret;
            }
        }

        /* 在 VPSS 所有 grp 创建并绑定完成后，如配置启用 stitch，则启动拼接线程 */
        if (g_pstVpssCfg->bStitchEnable && !g_VpssStitchRunning) {
            g_VpssStitchRunning = CVI_TRUE;

            pthread_attr_init(&attr);
            pthread_attr_setstacksize(&attr, 8192);
            if (pthread_create(&g_VpssStitchThread, &attr, app_ipcam_Vpss_Stitch_Proc, NULL) != 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "pthread_create(vpss_stitch) failed!\n");
                g_VpssStitchRunning = CVI_FALSE;
                pthread_attr_destroy(&attr);
                return CVI_FAILURE;
            }
            pthread_attr_destroy(&attr);
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "vpss init ------------------> end \n");

    return CVI_SUCCESS;
}
