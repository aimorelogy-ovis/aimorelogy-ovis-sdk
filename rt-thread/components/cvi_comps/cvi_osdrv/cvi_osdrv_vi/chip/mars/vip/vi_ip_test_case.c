#include <vip/vi_drv.h>

/****************************************************************************
 * Global parameters
 ****************************************************************************/
enum VI_TEST_CASE {
	VI_TEST_RGBGAMMA_ENABLE = 1,
	VI_TEST_RGBGAMMA_ENABLE_TBL_INVERSE,
	VI_TEST_CNR_ENABLE,
	VI_TEST_CNR_SET_LUT,
	VI_TEST_LCAC_ENABLE_MEDIUM_STRENGTH,
	VI_TEST_LCAC_ENABLE_STRONG_STRENGTH,
	VI_TEST_BNR_ENABLE_WEAK_STRENGTH,
	VI_TEST_BNR_ENABLE_STRONG_STRENGTH,
	VI_TEST_YNR_ENABLE,
	VI_TEST_DHZ_ENABLE,
	VI_TEST_BLC_GAIN_800,
	VI_TEST_BLC_GAIN_B00,
	VI_TEST_BLC_OFFSET_1FF,
	VI_TEST_BLC_GAIN_800_OFFSET_1FF,
	VI_TEST_BLC_GAIN_800_OFFSET_1FF_2ND_OFFSET_1FF,
	VI_TEST_WBG_GAIN_800,
	VI_TEST_DPC_ENABLE,
	VI_TEST_DPC_ENABLE_STATIC_ONLY,
	VI_TEST_DPC_ENABLE_DYNAMIC_ONLY,
	VI_TEST_PREEE_ENABLE,
	VI_TEST_EE_ENABLE,
	VI_TEST_CCM_ENABLE,
	VI_TEST_YCURVE_ENABLE_TBL_INVERSE,
	VI_TEST_YGAMMA_ENABLE,
	VI_TEST_YGAMMA_ENABLE_TBL_INVERSE,
	VI_TEST_YGAMMA_LUT_MEM0_CHECK,
	VI_TEST_YGAMMA_LUT_MEM1_CHECK,
	VI_TEST_3DNR_ENABLE_MOTION_MAP_OUT,
	VI_TEST_FUSION_LE_OUTPUT,
	VI_TEST_FUSION_SE_OUTPUT,
	VI_TEST_AE_HIST_ENABLE,
	VI_TEST_HIST_V_ENABLE_LUMA_MODE,
	VI_TEST_HIST_V_ENABLE,
	VI_TEST_HIST_V_ENABLE_OFFX64_OFFY32,
	VI_TEST_DCI_ENABLE,
	VI_TEST_DCI_ENABLE_DEMO_MODE,
	VI_TEST_LTM_CHECK_GLOBAL_TONE,
	VI_TEST_LTM_CHECK_DARK_TONE,
	VI_TEST_LTM_CHECK_BRIGHT_TONE,
	VI_TEST_LTM_CHECK_ALL_TONE_EE_ENABLE,
	VI_TEST_GMS_ENABLE,
	VI_TEST_AF_ENABLE,
	VI_TEST_LSC_ENABLE,
	VI_TEST_LDCI_ENABLE,
	VI_TEST_LDCI_ENABLE_TONE_CURVE_LUT_P_1023,
};

extern int vi_ip_test_case;
extern struct isp_ccm_cfg ccm_hw_cfg;
extern uint16_t ygamma_data[];
extern uint16_t gamma_data[];
extern uint16_t ycur_data[];
extern u16 dci_map_lut_50[];
extern uint16_t ltm_d_lut[];
extern uint16_t ltm_b_lut[];
extern uint16_t ltm_g_lut[];

/*******************************************************************************
 *	IPs test case config
 ******************************************************************************/
void vi_ip_test_cases(struct isp_ctx *ctx)
{
	switch (vi_ip_test_case) {
	case VI_TEST_RGBGAMMA_ENABLE: //rgbgamma enable
	{
		vi_pr(VI_INFO, "RGBgamma enable\n");

		ispblk_gamma_config(ctx, false, 0, gamma_data, 0);
		ispblk_gamma_enable(ctx, true);
		break;
	}
	case VI_TEST_RGBGAMMA_ENABLE_TBL_INVERSE: //2
	{
		vi_pr(VI_INFO, "RGBgamma enable, tbl inverse\n");

		ispblk_gamma_config(ctx, false, 0, gamma_data, 1);
		ispblk_gamma_enable(ctx, true);
		break;
	}
	case VI_TEST_CNR_ENABLE: //3
	{
		vi_pr(VI_INFO, "CNR enable\n");

		ispblk_cnr_config(ctx, true, true, 255, 0);
		break;
	}
	case VI_TEST_CNR_SET_LUT: //4
	{
		vi_pr(VI_INFO, "CNR set lut\n");

		ispblk_cnr_config(ctx, true, true, 255, 1);
		break;
	}
	case VI_TEST_LCAC_ENABLE_MEDIUM_STRENGTH: //5
	{
		vi_pr(VI_INFO, "LCAC enable, default medium strength setting\n");

		ispblk_lcac_config(ctx, true, 0);
		break;
	}
	case VI_TEST_LCAC_ENABLE_STRONG_STRENGTH: //6
	{
		vi_pr(VI_INFO, "LCAC enable, strong strength setting\n");

		ispblk_lcac_config(ctx, true, 1);
		break;
	}
	case VI_TEST_BNR_ENABLE_WEAK_STRENGTH: //7
	{
		vi_pr(VI_INFO, "BNR enable, weak strength\n");

		ispblk_bnr_config(ctx, ISP_BNR_OUT_B_OUT, false, 0, 0);
		break;
	}
	case VI_TEST_BNR_ENABLE_STRONG_STRENGTH: //8
	{
		vi_pr(VI_INFO, "BNR enable, strong strength\n");

		ispblk_bnr_config(ctx, ISP_BNR_OUT_B_OUT, false, 0, 255);
		break;
	}
	case VI_TEST_YNR_ENABLE: //9
	{
		vi_pr(VI_INFO, "YNR enable\n");

		ispblk_ynr_config(ctx, ISP_YNR_OUT_Y_OUT, 128);
		break;
	}
	case VI_TEST_DHZ_ENABLE: //10
	{
		vi_pr(VI_INFO, "DHZ enable\n");

		ispblk_dhz_config(ctx, true);
		break;
	}
	case VI_TEST_BLC_GAIN_800: //11
	{
		vi_pr(VI_INFO, "BLC be_le enable, gain 0x800\n");

		ispblk_blc_set_gain(ctx, ISP_BLC_ID_BE_LE, 0x800, 0x800, 0x800, 0x800);
		ispblk_blc_enable(ctx, ISP_BLC_ID_BE_LE, true, false);
		break;
	}
	case VI_TEST_BLC_GAIN_B00: //12
	{
		vi_pr(VI_INFO, "BLC be_le enable, gain 0xB00\n");

		ispblk_blc_set_gain(ctx, ISP_BLC_ID_BE_LE, 0xB00, 0xB00, 0xB00, 0xB00);
		ispblk_blc_enable(ctx, ISP_BLC_ID_BE_LE, true, false);
		break;
	}
	case VI_TEST_BLC_OFFSET_1FF: //13
	{
		vi_pr(VI_INFO, "BLC be_le enable, offset 0x1ff\n");

		ispblk_blc_set_offset(ctx, ISP_BLC_ID_BE_LE, 0x1FF, 0x1FF, 0x1FF, 0x1FF);
		ispblk_blc_enable(ctx, ISP_BLC_ID_BE_LE, true, false);
		break;
	}
	case VI_TEST_BLC_GAIN_800_OFFSET_1FF: //14
	{
		vi_pr(VI_INFO, "BLC be_le enable, gain 0x800 offset 0x1ff\n");

		ispblk_blc_set_offset(ctx, ISP_BLC_ID_BE_LE, 0x1FF, 0x1FF, 0x1FF, 0x1FF);
		ispblk_blc_set_gain(ctx, ISP_BLC_ID_BE_LE, 0x800, 0x800, 0x800, 0x800);
		ispblk_blc_enable(ctx, ISP_BLC_ID_BE_LE, true, false);
		break;
	}
	case VI_TEST_BLC_GAIN_800_OFFSET_1FF_2ND_OFFSET_1FF: //15
	{
		vi_pr(VI_INFO, "BLC be_le enable, gain 0x800 offset 0x1ff\n");

		ispblk_blc_set_offset(ctx, ISP_BLC_ID_BE_LE, 0x1FF, 0x1FF, 0x1FF, 0x1FF);
		ispblk_blc_set_gain(ctx, ISP_BLC_ID_BE_LE, 0x800, 0x800, 0x800, 0x800);
		ispblk_blc_set_2ndoffset(ctx, ISP_BLC_ID_BE_LE, 0x1FF, 0x1FF, 0x1FF, 0x1FF);
		ispblk_blc_enable(ctx, ISP_BLC_ID_BE_LE, true, false);
		break;
	}
	case VI_TEST_WBG_GAIN_800: //16
	{
		vi_pr(VI_INFO, "WBG rawtop_le enable, gain 0x800\n");

		ispblk_wbg_config(ctx, ISP_WBG_ID_RAW_TOP_LE, 0x800, 0x800, 0x800);
		ispblk_wbg_enable(ctx, ISP_WBG_ID_RAW_TOP_LE, true, false);
		break;
	}
	case VI_TEST_DPC_ENABLE: //17
	{
		vi_pr(VI_INFO, "DPC enable\n");
		ispblk_dpc_config(ctx, ISP_RAW_PATH_LE, true, 0);
		break;
	}
	case VI_TEST_DPC_ENABLE_STATIC_ONLY: //18
	{
		vi_pr(VI_INFO, "DPC enable static only\n");
		ispblk_dpc_config(ctx, ISP_RAW_PATH_LE, true, 1);
		break;
	}
	case VI_TEST_DPC_ENABLE_DYNAMIC_ONLY: //19
	{
		vi_pr(VI_INFO, "DPC enable dynamic only\n");
		ispblk_dpc_config(ctx, ISP_RAW_PATH_LE, true, 2);
		break;
	}
	case VI_TEST_PREEE_ENABLE: //20
	{
		vi_pr(VI_INFO, "PREEE enable\n");
		ispblk_pre_ee_config(ctx, true);
		break;
	}
	case VI_TEST_EE_ENABLE: //21
	{
		vi_pr(VI_INFO, "EE enable\n");
		ispblk_ee_config(ctx, true);
		break;
	}
	case VI_TEST_CCM_ENABLE: //22
	{
		vi_pr(VI_INFO, "CCM enable\n");
		ispblk_ccm_config(ctx, ISP_BLK_ID_CCM0, true, &ccm_hw_cfg);
		break;
	}
	case VI_TEST_YCURVE_ENABLE_TBL_INVERSE: //23
	{
		vi_pr(VI_INFO, "YCURVE enable, tbl inverse\n");
		ispblk_ycur_config(ctx, true, 0, ycur_data);
		ispblk_ycur_enable(ctx, true, 0);
		break;
	}
	case VI_TEST_YGAMMA_ENABLE: //24
	{
		vi_pr(VI_INFO, "Ygamma enable\n");
		ispblk_ygamma_config(ctx, false, 0, ygamma_data, 0, 0);
		ispblk_ygamma_enable(ctx, true);
		break;
	}
	case VI_TEST_YGAMMA_ENABLE_TBL_INVERSE: //25
	{
		vi_pr(VI_INFO, "Ygamma enable, tbl inverse\n");
		ispblk_ygamma_config(ctx, false, 0, ygamma_data, 1, 0);
		ispblk_ygamma_enable(ctx, true);
		break;
	}
	case VI_TEST_YGAMMA_LUT_MEM0_CHECK: //26
	{
		vi_pr(VI_INFO, "Ygamma LUT MEM0 Check\n");
		ispblk_ygamma_config(ctx, false, 0, ygamma_data, 0, 1);
		break;
	}
	case VI_TEST_YGAMMA_LUT_MEM1_CHECK: //27
	{
		vi_pr(VI_INFO, "Ygamma LUT MEM1 Check\n");
		ispblk_ygamma_config(ctx, false, 1, ygamma_data, 0, 1);
		break;
	}
	case VI_TEST_3DNR_ENABLE_MOTION_MAP_OUT: //28
	{
		vi_pr(VI_INFO, "TNR enable, motion map output\n");
		ispblk_tnr_config(ctx, ctx->is_3dnr_on, 1);
		break;
	}
	case VI_TEST_FUSION_LE_OUTPUT: //29
	{
		vi_pr(VI_INFO, "ltm disable, le frame output\n");
		ispblk_ltm_config(ctx, false, false, false, false);
		ispblk_fusion_config(ctx, true, true, ISP_FS_OUT_LONG);
		break;
	}
	case VI_TEST_FUSION_SE_OUTPUT: //30
	{
		vi_pr(VI_INFO, "ltm disable, se frame output\n");
		ispblk_ltm_config(ctx, false, false, false, false);
		ispblk_fusion_config(ctx, true, true, ISP_FS_OUT_SHORT);
		break;
	}
	case VI_TEST_AE_HIST_ENABLE: //31
	{
		vi_pr(VI_INFO, "AE_HIST enable\n");

		ispblk_aehist_config(ctx, ISP_BLK_ID_AEHIST0, true);
		if (ctx->is_hdr_on)
			ispblk_aehist_config(ctx, ISP_BLK_ID_AEHIST1, true);
		else
			ispblk_aehist_config(ctx, ISP_BLK_ID_AEHIST1, false);

		break;
	}
	case VI_TEST_HIST_V_ENABLE_LUMA_MODE: //32
	{
		vi_pr(VI_INFO, "HIST_V enable, luma mode\n");
		ispblk_hist_v_config(ctx, true, 0);
		break;
	}
	case VI_TEST_HIST_V_ENABLE: //33
	{
		vi_pr(VI_INFO, "HIST_V enable\n");
		ispblk_hist_v_config(ctx, true, 1);
		break;
	}
	case VI_TEST_HIST_V_ENABLE_OFFX64_OFFY32: //34
	{
		vi_pr(VI_INFO, "HIST_V enable, Offx64_Offy32\n");
		ispblk_hist_v_config(ctx, true, 2);
		break;
	}
	case VI_TEST_DCI_ENABLE: //35
	{
		vi_pr(VI_INFO, "DCI enable\n");
		ispblk_dci_config(ctx, true, ctx->gamma_tbl_idx, dci_map_lut_50, 0);
		break;
	}
	case VI_TEST_DCI_ENABLE_DEMO_MODE: //36
	{
		vi_pr(VI_INFO, "DCI enable, demo_mode\n");
		ispblk_dci_config(ctx, true, ctx->gamma_tbl_idx, dci_map_lut_50, 1);
		break;
	}
	case VI_TEST_LTM_CHECK_GLOBAL_TONE: //37
	{
		vi_pr(VI_INFO, "ltm enable, dark_tone/bright_tone/ee_en disable\n");
		ispblk_ltm_b_lut(ctx, 0, ltm_b_lut);
		ispblk_ltm_d_lut(ctx, 0, ltm_d_lut);
		ispblk_ltm_g_lut(ctx, 0, ltm_g_lut);
		ispblk_ltm_config(ctx, true, false, false, false);
		break;
	}
	case VI_TEST_LTM_CHECK_DARK_TONE: //38
	{
		vi_pr(VI_INFO, "ltm enable, bright_tone/ee_en disable\n");
		ispblk_ltm_b_lut(ctx, 0, ltm_b_lut);
		ispblk_ltm_d_lut(ctx, 0, ltm_d_lut);
		ispblk_ltm_g_lut(ctx, 0, ltm_g_lut);
		ispblk_ltm_config(ctx, true, true, false, false);
		break;
	}
	case VI_TEST_LTM_CHECK_BRIGHT_TONE: //39
	{
		vi_pr(VI_INFO, "ltm enable, bright_tone/ee_en disable\n");
		ispblk_ltm_b_lut(ctx, 0, ltm_b_lut);
		ispblk_ltm_d_lut(ctx, 0, ltm_d_lut);
		ispblk_ltm_g_lut(ctx, 0, ltm_g_lut);
		ispblk_ltm_config(ctx, true, false, true, false);
		break;
	}
	case VI_TEST_LTM_CHECK_ALL_TONE_EE_ENABLE: //40
	{
		vi_pr(VI_INFO, "ltm enable, all enable\n");
		ispblk_ltm_b_lut(ctx, 0, ltm_b_lut);
		ispblk_ltm_d_lut(ctx, 0, ltm_d_lut);
		ispblk_ltm_g_lut(ctx, 0, ltm_g_lut);
		ispblk_ltm_config(ctx, true, true, true, true);
		break;
	}
	case VI_TEST_GMS_ENABLE: //41
	{
		vi_pr(VI_INFO, "gms enable\n");
		ispblk_gms_config(ctx, true);
		break;
	}
	case VI_TEST_AF_ENABLE: //42
	{
		vi_pr(VI_INFO, "af enable\n");
		ispblk_af_config(ctx, true);
		break;
	}
	case VI_TEST_LSC_ENABLE: //43
	{
		vi_pr(VI_INFO, "lsc enable\n");
		ispblk_lsc_config(ctx, true);
		break;
	}
	case VI_TEST_LDCI_ENABLE: //44
	{
		vi_pr(VI_INFO, "ldci enable\n");
		ispblk_ldci_config(ctx, true, 0);
		break;
	}
	case VI_TEST_LDCI_ENABLE_TONE_CURVE_LUT_P_1023: //45
	{
		vi_pr(VI_INFO, "ldci enable, TONE_CURVE_LUT_P_1023\n");
		ispblk_ldci_config(ctx, true, 1);
		break;
	}
	default:
		break;
	}

	vi_ip_test_case = 0;
}
