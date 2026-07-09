/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_clut_ctrl.c
 * Description:
 *
 */

#include "isp_clut_ctrl.h"

#include "cvi_comm_isp.h"
#include "cvi_sys.h"
#include "isp_debug.h"
#include "isp_defines.h"
#include "isp_interpolate.h"
#include "isp_ioctl.h"
#include "isp_main_local.h"
#include "isp_mgr_buf.h"
#include "isp_proc_local.h"
#include "isp_tun_buf_ctrl.h"

#define CLUT_CHANNEL_SIZE               (17)  // HW dependent
#define CLUT_PARTIAL_UPDATE_SIZE        (256)
#define CLUT_OFFLINE_FULL_UPDATE_SYMBOL (0xFF)

const struct isp_module_ctrl clut_mod = {.init    = isp_clut_ctrl_init,
                                         .uninit  = isp_clut_ctrl_uninit,
                                         .suspend = isp_clut_ctrl_suspend,
                                         .resume  = isp_clut_ctrl_resume,
                                         .ctrl    = isp_clut_ctrl_ctrl};

static CVI_S32 isp_clut_ctrl_post_eof(VI_PIPE ViPipe, ISP_ALGO_RESULT_S *algoResult);
static CVI_S32 isp_clut_ctrl_preprocess(VI_PIPE ViPipe, ISP_ALGO_RESULT_S *algoResult);
static CVI_S32 isp_clut_ctrl_process(VI_PIPE ViPipe);
static CVI_S32 isp_clut_ctrl_postprocess(VI_PIPE ViPipe);
static CVI_S32 isp_clut_ctrl_check_clut_attr_valid(const ISP_CLUT_ATTR_S *pstCLUTAttr);
static CVI_S32 isp_clut_ctrl_check_clut_saturation_attr_valid(
    const ISP_CLUT_SATURATION_ATTR_S *pstClutSaturationAttr);

static CVI_S32 set_clut_proc_info(VI_PIPE ViPipe);

static struct isp_clut_ctrl_runtime *_get_clut_ctrl_runtime(VI_PIPE ViPipe);

CVI_S32 isp_clut_ctrl_init(VI_PIPE ViPipe)
{
    ISP_LOG_DEBUG("+\n");
    CVI_S32                       ret     = CVI_SUCCESS;
    struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    isp_algo_clut_init(ViPipe);

    runtime->bTableUpdateFullMode = CVI_TRUE;

#if defined(__CV181X__) || defined(__CV180X__)
    // Force partial update in CV181X
    runtime->bTableUpdateFullMode = CVI_FALSE;
#endif  // CHIP_ARCH

    runtime->clut_partial_update.bTableUpdate         = CVI_FALSE;
    runtime->clut_partial_update.u32TableUpdateStep   = 0;
    runtime->clut_partial_update.updateFailCnt        = 0;
    runtime->clut_partial_update.u32lastTblIdx        = 1;
    runtime->clut_partial_update.bSatLutUpdate        = CVI_FALSE;
    runtime->clut_partial_update.u32SatLutUUpdateLen  = 0;
    runtime->clut_partial_update.u32SatLutUUpdateStep = 0;

    runtime->preprocess_table_updated  = CVI_TRUE;
    runtime->preprocess_table_updating = CVI_TRUE;
    runtime->preprocess_lut_updated    = CVI_TRUE;
    runtime->process_updated           = CVI_FALSE;
    runtime->postprocess_updated       = CVI_FALSE;
    runtime->is_module_bypass          = CVI_FALSE;

    return ret;
}

CVI_S32 isp_clut_ctrl_uninit(VI_PIPE ViPipe)
{
    ISP_LOG_DEBUG("+\n");
    CVI_S32 ret = CVI_SUCCESS;

    isp_algo_clut_uninit(ViPipe);

    return ret;
}

CVI_S32 isp_clut_ctrl_suspend(VI_PIPE ViPipe)
{
    ISP_LOG_DEBUG("+\n");
    CVI_S32 ret = CVI_SUCCESS;

    UNUSED(ViPipe);

    return ret;
}

CVI_S32 isp_clut_ctrl_resume(VI_PIPE ViPipe)
{
    ISP_LOG_DEBUG("+\n");
    CVI_S32 ret = CVI_SUCCESS;

    UNUSED(ViPipe);

    return ret;
}

CVI_S32 isp_clut_ctrl_ctrl(VI_PIPE ViPipe, enum isp_module_cmd cmd, CVI_VOID *input)
{
    ISP_LOG_DEBUG("+\n");
    CVI_S32                       ret     = CVI_SUCCESS;
    struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    switch (cmd) {
    case MOD_CMD_POST_EOF:
        isp_clut_ctrl_post_eof(ViPipe, (ISP_ALGO_RESULT_S *)input);
        break;
    case MOD_CMD_SET_MODCTRL:
        runtime->is_module_bypass = ((ISP_MODULE_CTRL_U *)input)->bitBypassClut;
        break;
    case MOD_CMD_GET_MODCTRL:
        ((ISP_MODULE_CTRL_U *)input)->bitBypassClut = runtime->is_module_bypass;
        break;
    default:
        break;
    }

    return ret;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_clut_ctrl_post_eof(VI_PIPE ViPipe, ISP_ALGO_RESULT_S *algoResult)
{
    CVI_S32 ret = CVI_SUCCESS;

    ret = isp_clut_ctrl_preprocess(ViPipe, algoResult);
    if (ret != CVI_SUCCESS) return ret;

    ret = isp_clut_ctrl_process(ViPipe);
    if (ret != CVI_SUCCESS) return ret;

    ret = isp_clut_ctrl_postprocess(ViPipe);
    if (ret != CVI_SUCCESS) return ret;

    set_clut_proc_info(ViPipe);

    return ret;
}

static CVI_S32 isp_clut_ctrl_preprocess(VI_PIPE ViPipe, ISP_ALGO_RESULT_S *algoResult)
{
    CVI_S32                       ret     = CVI_SUCCESS;
    struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    const ISP_CLUT_ATTR_S            *clut_attr;
    const ISP_CLUT_SATURATION_ATTR_S *clut_saturation_attr = NULL;

    isp_clut_ctrl_get_clut_attr(ViPipe, &clut_attr);
    isp_clut_ctrl_get_clut_saturation_attr(ViPipe, &clut_saturation_attr);

    CVI_BOOL is_preprocess_update = CVI_FALSE;
    CVI_U8   intvl                = MAX(clut_attr->UpdateInterval, 1);

    is_preprocess_update =
        ((runtime->preprocess_table_updated) || (runtime->preprocess_table_updating) ||
         (runtime->preprocess_lut_updated) || ((algoResult->u32FrameIdx % intvl) == 0));

    // No need to update status
    if (is_preprocess_update == CVI_FALSE) return ret;

    // runtime->preprocess_table_updated = CVI_FALSE;
    runtime->postprocess_updated = CVI_TRUE;

    // No need to update parameters if disable. Because its meaningless
    if (!clut_attr->Enable || runtime->is_module_bypass) return ret;

    // ParamIn
    runtime->clut_param_in.is_table_update   = runtime->preprocess_table_updated;
    runtime->clut_param_in.is_table_updating = runtime->preprocess_table_updating;
    runtime->clut_param_in.is_lut_update     = ISP_PTR_CAST_PTR(&runtime->preprocess_lut_updated);

    runtime->clut_param_in.iso             = algoResult->u32PostIso;
    runtime->clut_param_in.ClutR           = ISP_PTR_CAST_PTR(clut_attr->ClutR);
    runtime->clut_param_in.ClutG           = ISP_PTR_CAST_PTR(clut_attr->ClutG);
    runtime->clut_param_in.ClutB           = ISP_PTR_CAST_PTR(clut_attr->ClutB);
    runtime->clut_param_in.saturation_attr = ISP_PTR_CAST_PTR(clut_saturation_attr);

    runtime->process_updated = CVI_TRUE;

    return ret;
}

static CVI_S32 isp_clut_ctrl_process(VI_PIPE ViPipe)
{
    CVI_S32                       ret     = CVI_SUCCESS;
    struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    if (runtime->process_updated == CVI_FALSE) return ret;

    ret = isp_algo_clut_main(ViPipe, (struct clut_param_in *)&runtime->clut_param_in,
                             (struct clut_param_out *)&runtime->clut_param_out);

    runtime->process_updated = CVI_FALSE;

    return ret;
}

static CVI_S32 isp_clut_ctrl_postprocess(VI_PIPE ViPipe)
{
    CVI_S32                       ret     = CVI_SUCCESS;
    struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    struct cvi_vip_isp_post_cfg *post_addr = get_post_tuning_buf_addr(ViPipe);
    CVI_U8                       tun_idx   = 0;  // get_tuning_buf_idx(ViPipe);

    struct cvi_vip_isp_clut_config *clut_cfg =
        (struct cvi_vip_isp_clut_config *)&(post_addr->tun_cfg[tun_idx].clut_cfg);

    struct isp_clut_partial_update *ptPartialUp = &(runtime->clut_partial_update);

    const ISP_CLUT_ATTR_S            *clut_attr            = NULL;
    const ISP_CLUT_SATURATION_ATTR_S *clut_saturation_attr = NULL;
    const CVI_BOOL                    bIsMultiCam          = IS_MULTI_CAM();

    isp_clut_ctrl_get_clut_attr(ViPipe, &clut_attr);
    isp_clut_ctrl_get_clut_saturation_attr(ViPipe, &clut_saturation_attr);

    CVI_BOOL is_postprocess_update = ((runtime->postprocess_updated == CVI_TRUE) || (bIsMultiCam));

#ifdef __CV181X__
    // Force update full mode when in multiple sensor situation.
    // TODO: Need to verify when multiple sensor environment is ready.
    if (bIsMultiCam) {
        runtime->bTableUpdateFullMode = CVI_TRUE;
    }
#endif  // __CV181X__

    clut_cfg->enable = clut_attr->Enable && !runtime->is_module_bypass;

    if ((runtime->bTableUpdateFullMode) && (runtime->preprocess_table_updated)) {
        CVI_U32 u32CLutTblIdx;

        G_EXT_CTRLS_VALUE(VI_IOCTL_GET_CLUT_TBL_IDX, ViPipe, &u32CLutTblIdx);
        if (u32CLutTblIdx == CLUT_OFFLINE_FULL_UPDATE_SYMBOL) {
            runtime->preprocess_table_updated  = CVI_FALSE;
            runtime->preprocess_table_updating = CVI_FALSE;
        }
    }

    if (is_postprocess_update == CVI_TRUE) {
        if (runtime->bTableUpdateFullMode) {
            if (runtime->preprocess_table_updated) {
                clut_cfg->update            = 1;
                clut_cfg->is_update_partial = 0;
                clut_cfg->tbl_idx           = CLUT_OFFLINE_FULL_UPDATE_SYMBOL;
                memcpy(clut_cfg->r_lut, clut_attr->ClutR, sizeof(uint16_t) * ISP_CLUT_LUT_LENGTH);
                memcpy(clut_cfg->g_lut, clut_attr->ClutG, sizeof(uint16_t) * ISP_CLUT_LUT_LENGTH);
                memcpy(clut_cfg->b_lut, clut_attr->ClutB, sizeof(uint16_t) * ISP_CLUT_LUT_LENGTH);
            } else if (clut_saturation_attr->Enable) {
                CVI_U32 len = runtime->clut_param_out.updateList.length;

                if (len > 0) {
                    clut_cfg->update            = 1;
                    clut_cfg->is_update_partial = 1;
                    clut_cfg->update_length     = len;
                    memcpy(clut_cfg->lut, runtime->clut_param_out.updateList.items,
                           sizeof(CVI_U32) * 2 * len);
                }
            }
        } else {
            if (runtime->preprocess_table_updated) {
                ptPartialUp->bTableUpdate       = CVI_TRUE;
                ptPartialUp->u32TableUpdateStep = 0;

                runtime->preprocess_table_updated = CVI_FALSE;
            } else if (clut_saturation_attr->Enable) {
                if (ptPartialUp->bSatLutUpdate != CVI_TRUE) {
                    ptPartialUp->bSatLutUpdate        = CVI_TRUE;
                    ptPartialUp->u32SatLutUUpdateStep = 0;
                    ptPartialUp->u32SatLutUUpdateLen  = ptPartialUp->stUpdateList.length =
                        runtime->clut_param_out.updateList.length;

                    memcpy(ptPartialUp->stUpdateList.items,
                           runtime->clut_param_out.updateList.items,
                           sizeof(CVI_U32) * 2 * ptPartialUp->stUpdateList.length);
                }
            }
        }
    }

    // Partial Update
    if (!runtime->bTableUpdateFullMode) {
        if (ptPartialUp->bTableUpdate) {
            CVI_U32  u32DataCount = 0;
            CVI_U32  u32LutIdx;
            CVI_U32  b_idx, g_idx, r_idx;
            CVI_U32  b_value, g_value, r_value;
            CVI_U32  u32TableUpdateIndex;
            CVI_U32  u32CLutTblIdx;
            CVI_BOOL bPartialUpdateFail = 0;

            G_EXT_CTRLS_VALUE(VI_IOCTL_GET_CLUT_TBL_IDX, ViPipe, &u32CLutTblIdx);
            u32TableUpdateIndex = MIN(ptPartialUp->u32TableUpdateStep, u32CLutTblIdx);
            if (u32CLutTblIdx == ptPartialUp->u32lastTblIdx) {
                if (ptPartialUp->updateFailCnt++ > 10) {
                    u32TableUpdateIndex        = 0;
                    ptPartialUp->updateFailCnt = 0;
                    ISP_LOG_DEBUG("ViPipe = %d clut partial update fail reset update index.\n",
                                  ViPipe);
                } else {
                    bPartialUpdateFail = 1;
                }
            } else {
                ptPartialUp->u32lastTblIdx = u32CLutTblIdx;
                ptPartialUp->updateFailCnt = 0;
            }
            u32LutIdx = u32TableUpdateIndex * CLUT_PARTIAL_UPDATE_SIZE;

            if (!bPartialUpdateFail) {
                for (uint32_t idx = 0;
                     ((idx < CLUT_PARTIAL_UPDATE_SIZE) && (u32LutIdx < ISP_CLUT_LUT_LENGTH));
                     ++idx, ++u32LutIdx) {
                    b_idx = u32LutIdx / (CLUT_CHANNEL_SIZE * CLUT_CHANNEL_SIZE);
                    g_idx = (u32LutIdx / CLUT_CHANNEL_SIZE) % CLUT_CHANNEL_SIZE;
                    r_idx = u32LutIdx % CLUT_CHANNEL_SIZE;

                    b_value = clut_attr->ClutB[u32LutIdx];
                    g_value = clut_attr->ClutG[u32LutIdx];
                    r_value = clut_attr->ClutR[u32LutIdx];

                    // 0: addr, 1: value
                    clut_cfg->lut[idx][0] = (b_idx << 16) + (g_idx << 8) + r_idx;
                    clut_cfg->lut[idx][1] =
                        ((r_value & 0x3FF) << 20) + ((g_value & 0x3FF) << 10) + (b_value & 0x3FF);
                    u32DataCount++;
                }
                u32TableUpdateIndex++;
                ptPartialUp->u32TableUpdateStep = u32TableUpdateIndex;

                clut_cfg->update_length = u32DataCount;

                if (u32DataCount > 0) {
                    clut_cfg->update            = 1;
                    clut_cfg->is_update_partial = 1;
                    clut_cfg->tbl_idx           = ptPartialUp->u32TableUpdateStep;
                } else {
                    clut_cfg->update                = 0;
                    clut_cfg->is_update_partial     = 0;
                    clut_cfg->tbl_idx               = 0;
                    ptPartialUp->bTableUpdate       = CVI_FALSE;
                    ptPartialUp->u32TableUpdateStep = 0;

                    runtime->preprocess_table_updating = CVI_FALSE;
                }
            }
            ptPartialUp->bSatLutUpdate        = CVI_FALSE;
            ptPartialUp->u32SatLutUUpdateStep = 0;
            ptPartialUp->u32SatLutUUpdateLen  = 0;
        } else if (ptPartialUp->bSatLutUpdate) {
            CVI_U32 u32SatLutStartIdx, u32SatLutCount;
            CVI_U32 u32TableUpdateIndex;
            CVI_U32 u32CLutTblIdx;

            G_EXT_CTRLS_VALUE(VI_IOCTL_GET_CLUT_TBL_IDX, ViPipe, &u32CLutTblIdx);
            u32TableUpdateIndex = MIN(ptPartialUp->u32SatLutUUpdateStep, u32CLutTblIdx);
            u32SatLutStartIdx   = u32TableUpdateIndex * CLUT_PARTIAL_UPDATE_SIZE;

            if (ptPartialUp->u32SatLutUUpdateLen > u32SatLutStartIdx) {
                u32SatLutCount = ptPartialUp->u32SatLutUUpdateLen - u32SatLutStartIdx;
                u32SatLutCount = MIN(u32SatLutCount, CLUT_PARTIAL_UPDATE_SIZE);

                memcpy(clut_cfg->lut, ptPartialUp->stUpdateList.items + u32SatLutStartIdx,
                       sizeof(CVI_U32) * 2 * u32SatLutCount);

                u32TableUpdateIndex++;
                ptPartialUp->u32SatLutUUpdateStep = u32TableUpdateIndex;

                clut_cfg->update            = 1;
                clut_cfg->is_update_partial = 1;
                clut_cfg->tbl_idx           = ptPartialUp->u32SatLutUUpdateStep;
                clut_cfg->update_length     = u32SatLutCount;
            } else {
                clut_cfg->update            = 0;
                clut_cfg->is_update_partial = 0;
                clut_cfg->tbl_idx           = 0;
                clut_cfg->update_length     = 0;

                ptPartialUp->bSatLutUpdate        = CVI_FALSE;
                ptPartialUp->u32SatLutUUpdateLen  = 0;
                ptPartialUp->u32SatLutUUpdateStep = 0;
            }
        }
    }

    runtime->postprocess_updated = CVI_FALSE;

    return ret;
}

static CVI_S32 set_clut_proc_info(VI_PIPE ViPipe)
{
    CVI_S32 ret = CVI_SUCCESS;
#ifndef ARCH_RTOS_CV181X
    if (ISP_GET_PROC_ACTION(ViPipe, PROC_LEVEL_1)) {
        const ISP_CLUT_ATTR_S *clut_attr = NULL;
        ISP_DEBUGINFO_PROC_S  *pProcST   = NULL;

        isp_clut_ctrl_get_clut_attr(ViPipe, &clut_attr);
        ISP_GET_PROC_INFO(ViPipe, pProcST);

        // common
        pProcST->CLutEnable = clut_attr->Enable;
    }
#else
    UNUSED(ViPipe);
#endif
    return ret;
}

static struct isp_clut_ctrl_runtime *_get_clut_ctrl_runtime(VI_PIPE ViPipe)
{
    CVI_BOOL isVipipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

    if (!isVipipeValid) {
        ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
        return NULL;
    }

    struct isp_clut_shared_buffer *shared_buf = CVI_NULL;

    isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CLUT, (CVI_VOID *)&shared_buf);

    return &shared_buf->runtime;
}

//-----------------------------------------------------------------------------
//  private functions
//-----------------------------------------------------------------------------
static CVI_S32 isp_clut_ctrl_check_clut_attr_valid(const ISP_CLUT_ATTR_S *pstCLUTAttr)
{
    CVI_S32 ret = CVI_SUCCESS;

    // CHECK_VALID_CONST(pstCLUTAttr, Enable, CVI_FALSE, CVI_TRUE);
    // CHECK_VALID_CONST(pstCLUTAttr, UpdateInterval, 0, 0xff);
    CHECK_VALID_ARRAY_1D(pstCLUTAttr, ClutR, ISP_CLUT_LUT_LENGTH, 0x0, 0x3ff);
    CHECK_VALID_ARRAY_1D(pstCLUTAttr, ClutG, ISP_CLUT_LUT_LENGTH, 0x0, 0x3ff);
    CHECK_VALID_ARRAY_1D(pstCLUTAttr, ClutB, ISP_CLUT_LUT_LENGTH, 0x0, 0x3ff);

    return ret;
}

static CVI_S32 isp_clut_ctrl_check_clut_saturation_attr_valid(
    const ISP_CLUT_SATURATION_ATTR_S *pstClutSaturationAttr)
{
    CVI_S32 ret = CVI_SUCCESS;

    // CHECK_VALID_CONST(pstClutSaturationAttr, Enable, CVI_FALSE, CVI_TRUE);
    CHECK_VALID_AUTO_ISO_2D(pstClutSaturationAttr, SatIn, 4, 0x0, 0x2000);
    CHECK_VALID_AUTO_ISO_2D(pstClutSaturationAttr, SatOut, 4, 0x0, 0x2000);

    return ret;
}

//-----------------------------------------------------------------------------
//  public functions, set or get param
//-----------------------------------------------------------------------------
CVI_S32 isp_clut_ctrl_get_clut_attr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S **pstCLUTAttr)
{
    if (pstCLUTAttr == CVI_NULL) {
        return CVI_FAILURE;
    }

    CVI_S32                        ret           = CVI_SUCCESS;
    struct isp_clut_shared_buffer *shared_buffer = CVI_NULL;
    const ISP_CLUT_ATTR_S         *clut_attr;
    struct isp_clut_ctrl_runtime  *runtime = _get_clut_ctrl_runtime(ViPipe);

    isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CLUT, (CVI_VOID *)&shared_buffer);
    *pstCLUTAttr = &shared_buffer->stCLUTAttr;
    clut_attr    = *pstCLUTAttr;
    if (runtime->bEnStatus != clut_attr->Enable) {
        runtime->clut_partial_update.updateFailCnt = 0;
        runtime->bEnStatus                         = clut_attr->Enable;
    }
    return ret;
}

CVI_S32 isp_clut_ctrl_set_clut_attr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S *pstCLUTAttr)
{
    if (pstCLUTAttr == CVI_NULL) {
        return CVI_FAILURE;
    }

    CVI_S32                       ret     = CVI_SUCCESS;
    struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    ret = isp_clut_ctrl_check_clut_attr_valid(pstCLUTAttr);
    if (ret != CVI_SUCCESS) return ret;

    const ISP_CLUT_ATTR_S *p = CVI_NULL;

    isp_clut_ctrl_get_clut_attr(ViPipe, &p);
    memcpy((CVI_VOID *)p, pstCLUTAttr, sizeof(*pstCLUTAttr));

    runtime->preprocess_table_updated  = CVI_TRUE;
    runtime->preprocess_table_updating = CVI_TRUE;

    return CVI_SUCCESS;
}

CVI_S32 isp_clut_ctrl_get_clut_saturation_attr(
    VI_PIPE ViPipe, const ISP_CLUT_SATURATION_ATTR_S **pstClutSaturationAttr)
{
    if (pstClutSaturationAttr == CVI_NULL) {
        return CVI_FAILURE;
    }

    CVI_S32                        ret           = CVI_SUCCESS;
    struct isp_clut_shared_buffer *shared_buffer = CVI_NULL;

    isp_mgr_buf_get_addr(ViPipe, ISP_IQ_BLOCK_CLUT, (CVI_VOID *)&shared_buffer);
    *pstClutSaturationAttr = &shared_buffer->stClutSaturationAttr;

    return ret;
}

CVI_S32 isp_clut_ctrl_set_clut_saturation_attr(
    VI_PIPE ViPipe, const ISP_CLUT_SATURATION_ATTR_S *pstClutSaturationAttr)
{
    if (pstClutSaturationAttr == CVI_NULL) {
        return CVI_FAILURE;
    }

    CVI_S32                       ret     = CVI_SUCCESS;
    struct isp_clut_ctrl_runtime *runtime = _get_clut_ctrl_runtime(ViPipe);

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    ret = isp_clut_ctrl_check_clut_saturation_attr_valid(pstClutSaturationAttr);
    if (ret != CVI_SUCCESS) return ret;

    const ISP_CLUT_SATURATION_ATTR_S *p = CVI_NULL;

    isp_clut_ctrl_get_clut_saturation_attr(ViPipe, &p);
    memcpy((CVI_VOID *)p, pstClutSaturationAttr, sizeof(*pstClutSaturationAttr));

    runtime->preprocess_lut_updated = CVI_TRUE;

    return CVI_SUCCESS;
}
