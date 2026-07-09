/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_debug.c
 * Description:
 *
 */

#include "isp_debug.h"

// #include "isp_main_local.h"
// #include "isp_ioctl.h"
// #include "isp_defines.h"

// #include "cvi_sys.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
// #include "devmem.h"

// #include "isp_tun_buf_ctrl.h"
#include "isp_gamma_ctrl.h"

int gDebugLevel = LOG_INFO;
int gExportToStdout;

void CVI_DEBUG_SetDebugLevel(int level)
{
    gDebugLevel = level;
    ISP_LOG_DEBUG("gDebugLevel = %d\n", gDebugLevel);
}

CVI_S32 isp_dbg_dumpFrameRawInfoToBuffer(VI_PIPE ViPipe, CVI_U8 *buf, CVI_S32 buf_size)
{
#define SNPRINTF_LOCAL(offset, buf, buf_size, fmt, ...)                                \
    do {                                                                               \
        int ret;                                                                       \
        ret = snprintf((char *)(buf + offset), buf_size - offset, fmt, ##__VA_ARGS__); \
        if (ret > 0 && ret < buf_size - offset) {                                      \
            offset += ret;                                                             \
        } else if (ret <= 0) {                                                         \
            ISP_LOG_ERR("snprintf fail\n");                                            \
            return CVI_FAILURE;                                                        \
        } else {                                                                       \
            ISP_LOG_ERR("snprintf overflow\n");                                        \
            return CVI_FAILURE;                                                        \
        }                                                                              \
    } while (0)

    ISP_EXP_INFO_S                   stExpInfo        = {};
    ISP_WB_INFO_S                    stWBInfo         = {};
    ISP_INNER_STATE_INFO_S           stInnerStateInfo = {};
    ISP_MESH_SHADING_GAIN_LUT_ATTR_S stMLSC           = {};
    ISP_DRC_ATTR_S                   stDrc            = {};
    CVI_U16                         *pu16GammaLut     = NULL;
    CVI_S32                          offset           = 0;
    if (CVI_ISP_QueryExposureInfo(ViPipe, &stExpInfo) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    if (CVI_ISP_QueryWBInfo(ViPipe, &stWBInfo) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    if (CVI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    if (isp_gamma_ctrl_get_real_gamma_lut(ViPipe, &pu16GammaLut) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    if (CVI_ISP_GetMeshShadingGainLutAttr(ViPipe, &stMLSC) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    if (CVI_ISP_GetDRCAttr(ViPipe, &stDrc) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }

    SNPRINTF_LOCAL(offset, buf, buf_size, "ISO = %u\n", stExpInfo.u32ISO);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Light Value = %.2f\n", stExpInfo.fLightValue);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Color Temp. = %u\n", stWBInfo.u16ColorTemp);
    SNPRINTF_LOCAL(offset, buf, buf_size, "ISP DGain = %u\n", stExpInfo.u32ISPDGain);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure Time = %u\n", stExpInfo.u32ExpTime);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Long Exposure = %u\n", stExpInfo.u32LongExpTime);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Short Exposure = %u\n", stExpInfo.u32ShortExpTime);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure Ratio = %u\n", stExpInfo.u32WDRExpRatio);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure AGain = %u\n", stExpInfo.u32AGain);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure DGain = %u\n", stExpInfo.u32DGain);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure AGainSF = %u\n", stExpInfo.u32AGainSF);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure DGainSF = %u\n", stExpInfo.u32DGainSF);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure ISPDGainSF = %u\n", stExpInfo.u32ISPDGainSF);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Exposure ISOSF = %u\n", stExpInfo.u32ISOSF);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_wbg_rgain = %u\n", stWBInfo.u16Rgain);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_wbg_bgain = %u\n", stWBInfo.u16Bgain);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_wbg_grgain = %u\n", stWBInfo.u16Grgain);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_wbg_gbgain = %u\n", stWBInfo.u16Gbgain);

    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_00 = %d\n", (CVI_S16)stInnerStateInfo.ccm[0]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_01 = %d\n", (CVI_S16)stInnerStateInfo.ccm[1]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_02 = %d\n", (CVI_S16)stInnerStateInfo.ccm[2]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_10 = %d\n", (CVI_S16)stInnerStateInfo.ccm[3]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_11 = %d\n", (CVI_S16)stInnerStateInfo.ccm[4]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_12 = %d\n", (CVI_S16)stInnerStateInfo.ccm[5]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_20 = %d\n", (CVI_S16)stInnerStateInfo.ccm[6]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_21 = %d\n", (CVI_S16)stInnerStateInfo.ccm[7]);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_ccm_22 = %d\n", (CVI_S16)stInnerStateInfo.ccm[8]);

    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_offset_r = %u\n", stInnerStateInfo.blcOffsetR);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_offset_gr = %u\n", stInnerStateInfo.blcOffsetGr);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_offset_gb = %u\n", stInnerStateInfo.blcOffsetGb);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_offset_b = %u\n", stInnerStateInfo.blcOffsetB);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_gain_r = %u\n", stInnerStateInfo.blcGainR);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_gain_gr = %u\n", stInnerStateInfo.blcGainGr);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_gain_gb = %u\n", stInnerStateInfo.blcGainGb);
    SNPRINTF_LOCAL(offset, buf, buf_size, "reg_blc_gain_b = %u\n", stInnerStateInfo.blcGainB);

    // DRC group 1
    SNPRINTF_LOCAL(offset, buf, buf_size, "DRC Enable = %u\n", stDrc.Enable);
    SNPRINTF_LOCAL(offset, buf, buf_size, "DRC LocalToneEn = %u\n", stDrc.LocalToneEn);
    SNPRINTF_LOCAL(offset, buf, buf_size, "DRC ToneCurveSelect = %u\n", stDrc.ToneCurveSelect);

    // DRC group 2
    SNPRINTF_LOCAL(offset, buf, buf_size, "Manual.TargetYScale = %u\n",
                   stDrc.stManual.TargetYScale);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Auto.TargetYScale\n");
    for (CVI_U32 u32Idx = 0; u32Idx < (ISP_AUTO_LV_NUM - 1); ++u32Idx) {
        SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ", stDrc.stAuto.TargetYScale[u32Idx]);
    }
    SNPRINTF_LOCAL(offset, buf, buf_size, "%d\n", stDrc.stAuto.TargetYScale[ISP_AUTO_LV_NUM - 1]);

    // DRC group 3
    SNPRINTF_LOCAL(offset, buf, buf_size, "Manual.HdrStrength = %u\n", stDrc.stManual.HdrStrength);
    SNPRINTF_LOCAL(offset, buf, buf_size, "Auto.HdrStrength\n");
    for (CVI_U32 u32Idx = 0; u32Idx < (ISP_AUTO_LV_NUM - 1); ++u32Idx) {
        SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ", stDrc.stAuto.HdrStrength[u32Idx]);
    }
    SNPRINTF_LOCAL(offset, buf, buf_size, "%d\n", stDrc.stAuto.HdrStrength[ISP_AUTO_LV_NUM - 1]);

    // DRC group 4 (Linear DRC)
    // DRC Curve User Define (DRC_GLOBAL_USER_DEFINE_NUM)
    SNPRINTF_LOCAL(offset, buf, buf_size, "DRC CurveUserDefine =");
    for (CVI_U32 u32PntIdx = 0; u32PntIdx < DRC_GLOBAL_USER_DEFINE_NUM; ++u32PntIdx) {
        if ((u32PntIdx % 32) == 0) {
            SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
        }
        SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ", stDrc.CurveUserDefine[u32PntIdx]);
    }
    SNPRINTF_LOCAL(offset, buf, buf_size, "\n");

    // DRC Dark User Define (DRC_DARK_USER_DEFINE_NUM)
    SNPRINTF_LOCAL(offset, buf, buf_size, "DRC DarkUserDefine =");
    for (CVI_U32 u32PntIdx = 0; u32PntIdx < DRC_DARK_USER_DEFINE_NUM; ++u32PntIdx) {
        if ((u32PntIdx % 32) == 0) {
            SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
        }
        SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ", stDrc.DarkUserDefine[u32PntIdx]);
    }
    SNPRINTF_LOCAL(offset, buf, buf_size, "\n");

    // DRC Bright User Define (DRC_BRIGHT_USER_DEFINE_NUM)
    SNPRINTF_LOCAL(offset, buf, buf_size, "DRC BrightUserDefine =");
    for (CVI_U32 u32PntIdx = 0; u32PntIdx < DRC_BRIGHT_USER_DEFINE_NUM; ++u32PntIdx) {
        if ((u32PntIdx % 32) == 0) {
            SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
        }
        SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ", stDrc.BrightUserDefine[u32PntIdx]);
    }
    SNPRINTF_LOCAL(offset, buf, buf_size, "\n");

    // Gamma Log
    // In DRC7-4 the gamma lut is not fix to SRGB.
    // In DRC7-6 we can fix the table to SRGB lut.
    if (pu16GammaLut) {
        SNPRINTF_LOCAL(offset, buf, buf_size, "gamma =");
        for (CVI_U32 u32PntIdx = 0; u32PntIdx < GAMMA_NODE_NUM; ++u32PntIdx) {
            if ((u32PntIdx % 32) == 0) {
                SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
            }
            SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ", pu16GammaLut[u32PntIdx]);
        }
        SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
    }

    if (stMLSC.Size > 0) {
        CVI_U32 u32LutIndex = stMLSC.Size - 1;

        // LSC RGain Log
        SNPRINTF_LOCAL(offset, buf, buf_size, "lsc_r_gain =");
        for (CVI_U32 u32PntIdx = 0; u32PntIdx < CVI_ISP_LSC_GRID_POINTS; ++u32PntIdx) {
            if ((u32PntIdx % 32) == 0) {
                SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
            }
            SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ",
                           stMLSC.LscGainLut[u32LutIndex].RGain[u32PntIdx]);
        }
        SNPRINTF_LOCAL(offset, buf, buf_size, "\n");

        // LSC GGain Log
        SNPRINTF_LOCAL(offset, buf, buf_size, "lsc_g_gain =");
        for (CVI_U32 u32PntIdx = 0; u32PntIdx < CVI_ISP_LSC_GRID_POINTS; ++u32PntIdx) {
            if ((u32PntIdx % 32) == 0) {
                SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
            }
            SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ",
                           stMLSC.LscGainLut[u32LutIndex].GGain[u32PntIdx]);
        }
        SNPRINTF_LOCAL(offset, buf, buf_size, "\n");

        // LSC BGain Log
        SNPRINTF_LOCAL(offset, buf, buf_size, "lsc_b_gain =");
        for (CVI_U32 u32PntIdx = 0; u32PntIdx < CVI_ISP_LSC_GRID_POINTS; ++u32PntIdx) {
            if ((u32PntIdx % 32) == 0) {
                SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
            }
            SNPRINTF_LOCAL(offset, buf, buf_size, "%d, ",
                           stMLSC.LscGainLut[u32LutIndex].BGain[u32PntIdx]);
        }
        SNPRINTF_LOCAL(offset, buf, buf_size, "\n");
    }

    return CVI_SUCCESS;
}
