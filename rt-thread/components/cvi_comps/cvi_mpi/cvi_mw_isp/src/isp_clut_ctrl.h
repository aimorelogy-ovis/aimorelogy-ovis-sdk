/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_clut_ctrl.h
 * Description:
 *
 */

#ifndef _ISP_CLUT_CTRL_H_
#define _ISP_CLUT_CTRL_H_

#include "cvi_comm_isp.h"
#include "isp_algo_clut.h"
#include "isp_feature_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

struct isp_clut_partial_update {
    // CLUT table
    CVI_BOOL bTableUpdate;
    CVI_U32  u32TableUpdateStep;
    CVI_U32  u32lastTblIdx;
    CVI_U32  updateFailCnt;

    // Saturation LUT
    CVI_BOOL         bSatLutUpdate;
    CVI_U32          u32SatLutUUpdateLen;
    CVI_U32          u32SatLutUUpdateStep;
    CLUT_UPDATE_LIST stUpdateList;
};

struct isp_clut_ctrl_runtime {
    struct clut_param_in  clut_param_in;
    struct clut_param_out clut_param_out;

    struct isp_clut_partial_update clut_partial_update;

    CVI_BOOL bTableUpdateFullMode;

    CVI_BOOL preprocess_table_updated;
    CVI_BOOL preprocess_table_updating;
    CVI_BOOL preprocess_lut_updated;
    CVI_BOOL process_updated;
    CVI_BOOL postprocess_updated;
    CVI_BOOL is_module_bypass;
    CVI_BOOL bEnStatus;
};

CVI_S32 isp_clut_ctrl_init(VI_PIPE ViPipe);
CVI_S32 isp_clut_ctrl_uninit(VI_PIPE ViPipe);
CVI_S32 isp_clut_ctrl_suspend(VI_PIPE ViPipe);
CVI_S32 isp_clut_ctrl_resume(VI_PIPE ViPipe);
CVI_S32 isp_clut_ctrl_ctrl(VI_PIPE ViPipe, enum isp_module_cmd cmd, CVI_VOID *input);

CVI_S32 isp_clut_ctrl_get_clut_attr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S **pstCLUTAttr);
CVI_S32 isp_clut_ctrl_set_clut_attr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S *pstCLUTAttr);
CVI_S32 isp_clut_ctrl_get_clut_saturation_attr(
    VI_PIPE ViPipe, const ISP_CLUT_SATURATION_ATTR_S **pstClutSaturationAttr);
CVI_S32 isp_clut_ctrl_set_clut_saturation_attr(
    VI_PIPE ViPipe, const ISP_CLUT_SATURATION_ATTR_S *pstClutSaturationAttr);

extern const struct isp_module_ctrl clut_mod;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // _ISP_CLUT_CTRL_H_
