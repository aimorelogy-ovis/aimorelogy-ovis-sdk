/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_bin_param_header.h
 * Description:
 *
 */

#ifndef _CVI_BIN_PARAM_HEADER_H_
#define _CVI_BIN_PARAM_HEADER_H_

#include "cvi_comm_isp.h"
#include "cvi_comm_3a.h"
#include "cvi_comm_vpss.h"
#include "cvi_comm_vo.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct {
	//Pub attr
	ISP_PUB_ATTR_S pub_attr;

	// Pre-Raw
	ISP_BLACK_LEVEL_ATTR_S blc;
	ISP_LBLC_ATTR_S lblc;
	ISP_LBLC_LUT_ATTR_S lblcLut;
	ISP_MESH_SHADING_ATTR_S mlsc;
	ISP_MESH_SHADING_GAIN_LUT_ATTR_S mlscLUT;
	ISP_STATISTICS_CFG_S StatCfg;

	// Raw-top
	ISP_FSHDR_ATTR_S fshdr;
	ISP_DP_DYNAMIC_ATTR_S dpc_dynamic;
	ISP_CROSSTALK_ATTR_S crosstalk;
	ISP_BNR_ATTR_S bnr;
	ISP_BNR_FILTER_ATTR_S bnr_filter;
	ISP_RADIAL_SHADING_ATTR_S rlsc;
	ISP_RADIAL_SHADING_GAIN_LUT_ATTR_S rlscLUT;
	ISP_DRC_ATTR_S drc;
	ISP_DEMOSAIC_ATTR_S demosaic;
	ISP_DEMOSAIC_DEMOIRE_ATTR_S demosaic_demoire;

	// RGB-top
	ISP_PFR_ATTR_S pfr;
	ISP_SATURATION_ATTR_S Saturation;
	ISP_CCM_ATTR_S ccm;
	ISP_CCM_SATURATION_ATTR_S ccm_saturation;
	ISP_COLOR_TONE_ATTR_S colortone;
	ISP_GAMMA_ATTR_S gamma;
	ISP_CLUT_ATTR_S clut;
	ISP_CLUT_HSL_ATTR_S clut_hsl;
	ISP_CSC_ATTR_S csc;

	// YUV-top
	ISP_LDCI_ATTR_S ldci;
	ISP_DCI_ATTR_S dci;
	ISP_DCI_AUTO_GAMMA_ATTR_S dciAutoGamma;
	ISP_PRESHARPEN_ATTR_S presharpen;
	ISP_PRESHARPEN_REFINE_ATTR_S presharpen_refine;
	ISP_PRESHARPEN_EDGE_EXT_ATTR_S presharpen_edge_ext;
	ISP_TNR_ATTR_S tnr;
	ISP_TNR_MV_ATTR_S tnr_mv;
	ISP_TNR_PS_ATTR_S tnr_ps;
	ISP_TNR_NR_ATTR_S tnr_nr;
	ISP_CNR_ATTR_S cnr;
	ISP_CNR_FILTER_ATTR_S cnr_filter;
	ISP_SHARPEN_ATTR_S sharpen;
	ISP_CA_ATTR_S ca;
	ISP_CA2_ATTR_S ca2;
	ISP_YCONTRAST_ATTR_S ycontrast;

	// TEAISP
	TEAISP_BNR_ATTR_S teaisp_bnr;
	TEAISP_BNR_NP_S teaisp_bnr_np;

	// other
	ISP_CMOS_NOISE_CALIBRATION_S np;
	ISP_MONO_ATTR_S mono;

	// ISP_3A_Parameter_Structures
	ISP_WDR_EXPOSURE_ATTR_S WDRExpAttr;
	ISP_EXPOSURE_ATTR_S ExpAttr;
	ISP_AE_ROUTE_S AeRouteAttr;
	ISP_AE_ROUTE_EX_S AeRouteAttrEx;
	ISP_SMART_EXPOSURE_ATTR_S AeSmartExposureAttr;
	ISP_IRIS_ATTR_S AeIrisAttr;
	ISP_DCIRIS_ATTR_S AeDcirisAttr;
	ISP_AE_ROUTE_S AeRouteSFAttr;
	ISP_AE_ROUTE_EX_S AeRouteSFAttrEx;

	ISP_WB_ATTR_S WBAttr;
	ISP_AWB_ATTR_EX_S AWBAttrEx;
	ISP_AWB_Calibration_Gain_S WBCalib;
	ISP_AWB_Calibration_Gain_S_EX WBCalibEx;

	ISP_FOCUS_ATTR_S FocusAttr;
} ISP_Parameter_Structures;

typedef struct {
	VPSS_BIN_DATA vpss_bin_data[VPSS_MAX_GRP_NUM];
} VPSS_Parameter_Structures;

typedef struct {
	VO_BIN_INFO_S vo_bin_data;
} VO_Parameter_Structures;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // _CVI_BIN_PARAM_HEADER_H_
