/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_ive_interface.c
 * Description: ive kernel space driver entry related code

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
// #include <aos/kernel.h>
#include <io.h>
// #include "aos/aos.h"
#include "semaphore.h"
#include "asm/barrier.h"
#include "drv/list.h"
#include "drv/cvi_irq.h"
#include <csi_kernel.h>
#include <k_api.h>
#include "sys/prctl.h"
#include "cvi_ive_interface.h"
#include "cvi_ive_platform.h"
#include <drv/tick.h>

struct cvi_ive_device *ive_dev;

static uint32_t get_duration_us(uint64_t t1, uint64_t t2)
{
	uint32_t diff_time;

	if(t1 > t2) {
		diff_time = t1 - t2;
	} else {
		diff_time = t2 - t1;
	}

	return diff_time;
}


//static char g_kdata[512];
static uint32_t g_enable_usage_profiling;
static struct ive_profiling_info *g_time_infos;

// proc_operations function
//static int ive_proc_write(int user_input_param);
// file_operations function
int cvi_ive_open(void);
int cvi_ive_close(void);
int cvi_ive_ioctl(unsigned int cmd, void *arg);

static void start_ioctl_time(struct ive_profiling_info *pinfo, char *name)
{
	int i = 0;

	if (g_enable_usage_profiling) {
		strcpy(pinfo->op_name, name);
		pinfo->time_ioctl_diff_us = 0;
		for (i = 0; i < 6; i++) {
			pinfo->time_vld_diff_us[i] = 0;
		}
		pinfo->time_tile_diff_us = 0;

		pinfo->time_ioctl_start = csi_tick_get_us();
	}
}

static void stop_ioctl_time(struct ive_profiling_info *pinfo)
{
	if (g_enable_usage_profiling) {

		pinfo->time_ioctl_end =  csi_tick_get_us();

		pinfo->time_ioctl_diff_us =
			get_duration_us(pinfo->time_ioctl_start, pinfo->time_ioctl_end);
	}
}

void start_vld_time(int optype)
{
	if (g_enable_usage_profiling && optype < MOD_ALL &&
		optype >= MOD_BYP &&
		strlen(g_time_infos[optype].op_name) > 0) {

		g_time_infos[optype].time_vld_start =  csi_tick_get_us();

	}
}

void stop_vld_time(int optype, int tile_num)
{
	if (tile_num > 6)
		return;
	if (g_enable_usage_profiling && optype < MOD_ALL &&
		optype >= MOD_BYP &&
		strlen(g_time_infos[optype].op_name) > 0) {

		g_time_infos[optype].time_vld_end =  csi_tick_get_us();

		g_time_infos[optype].time_vld_diff_us[tile_num] =
			get_duration_us(g_time_infos[optype].time_vld_start,
			g_time_infos[optype].time_vld_end);
	}
	g_time_infos[optype].time_tile_diff_us +=
		g_time_infos[optype].time_vld_diff_us[tile_num];
}

void cvi_ive_irq_handler(int irq, void *data)
{
	struct cvi_ive_device *ndev = data;

	/*clear to do*/
	pthread_mutex_lock(&ndev->close_lock);
	// ive_printf(IVE_INFO, "[IVE] ive use_count %d\n", ndev->use_count);
	if (ndev->use_count != 0) {
		platform_ive_irq(ndev);
	}
	pthread_mutex_unlock(&ndev->close_lock);
}

#if 0
static int ive_proc_show(void)
{
	int i = 0, tile = 0;

	if (g_enable_usage_profiling) {
		char const *row_name[] = {"op name", "start(s)", "ioctl(us)",
							"tile0(us)", "tile1(us)", "tile2(us)", "tile3(us)",
							"tile4(us)", "tile5(us)", "tileSum(us)"};
		int row_space[] = { -15, 10, 10, 10, 10, 10, 10, 10, 10, 10};
		int table[] = { 20, 21, 22, 23, 24, 3, 2, 25, 26, 27,
						28, 31, 33, 35, 1, 29, 30, 4, 6, 7,
						8, 9, 10, 11, 15, 16, 17, 19, 18, 36,
						12, 34, 13, 14, 32, 5};

		printf("[IVE] ive time profiling\n");
		printf("%*s| %*s| %*s| %*s| %*s| %*s| %*s| %*s| %*s| %*s\n",
		row_space[0], row_name[0], row_space[1], row_name[1],
		row_space[2], row_name[2], row_space[3], row_name[3],
		row_space[4], row_name[4], row_space[5], row_name[5],
		row_space[6], row_name[6], row_space[7], row_name[7],
		row_space[8], row_name[8], row_space[9], row_name[9]);

		for (i = 0; i < 36; i++) {
			uint32_t second_vld_time[6] = {0};
			uint32_t second_tile_time = 0;
			uint32_t id = table[i];

			if (strlen(g_time_infos[id].op_name) > 0) {
				if (id == 10) {
					for (tile = 0; tile < 6; tile++) {
						second_vld_time[tile] = g_time_infos[5].time_vld_diff_us[tile];
					}
					second_tile_time = g_time_infos[5].time_tile_diff_us;
				} else if (id == 5) {
					continue;
				}
				printf(
					"%*s| %*ld| %*u| %*d| %*d| %*d| %*d| %*d| %*d| %*d\n",
					row_space[0],
					g_time_infos[id].op_name,
					row_space[1],
					g_time_infos[id].time_ioctl_start,
					row_space[2],
					g_time_infos[id].time_ioctl_diff_us,
					row_space[3],
					g_time_infos[id].time_vld_diff_us[0] + second_vld_time[0],
					row_space[4],
					g_time_infos[id].time_vld_diff_us[1] + second_vld_time[1],
					row_space[5],
					g_time_infos[id].time_vld_diff_us[2] + second_vld_time[2],
					row_space[6],
					g_time_infos[id].time_vld_diff_us[3] + second_vld_time[3],
					row_space[7],
					g_time_infos[id].time_vld_diff_us[4] + second_vld_time[4],
					row_space[8],
					g_time_infos[id].time_vld_diff_us[5] + second_vld_time[5],
					row_space[9],
					g_time_infos[id].time_tile_diff_us + second_tile_time);
			}
		}
	} else {
		printf( "[IVE] ive time profiling is disabled\n");
	}
	return 0;
}

static int ive_proc_write(int user_input_param)
{
	int count, i;

	// reset related info
	if (user_input_param == 0) {
		g_enable_usage_profiling = 0;
		printf("\n[IVE] Time profiling is ended\n");
	} else if (user_input_param == 1) {
		for (i = 0; i < MOD_ALL; i++) {
			memset(&g_time_infos[i], 0,
			       sizeof(struct ive_profiling_info));
		}
		g_enable_usage_profiling = 1;
		printf("\n[IVE] Time profiling is started\n");
	} else if (user_input_param == 2) {
		cvi_ive_set_reg_dump(true);
		printf("\n[IVE] Enable dump reg state\n");
	} else if (user_input_param == 3) {
		cvi_ive_set_reg_dump(false);
		printf("\n[IVE] Disable dump reg state\n");
	} else if (user_input_param == 4) {
		cvi_ive_set_dma_dump(true);
		printf("\n[IVE] Enable dump dma phy addr\n");
	} else if (user_input_param == 5) {
		cvi_ive_set_dma_dump(false);
		printf("\n[IVE] Disable dump dma phy addr\n");
	} else if (user_input_param == 6) {
		cvi_ive_set_img_dump(true);
		printf("\n[IVE] Enable dump IVE_IMAGE_S, IVE_DATA_S, IVE_MEM_INFO_S\n");
	} else if (user_input_param == 7) {
		cvi_ive_set_img_dump(false);
		printf("\n[IVE] Disable dump IVE_IMAGE_S, IVE_DATA_S, IVE_MEM_INFO_S\n");
	} else if (user_input_param == 10) {
		cvi_ive_dump_op1_op2_info();
	} else if (user_input_param == 11) {
		cvi_ive_dump_hw_flow();
	} else {
		printf("\nIVE Command List:\n"
				"\t0: Set time profiling stop\n"
				"\t1: Set time profiling start\n"
				"\t2: Enable print reg state\n"
				"\t3: Disable print reg state\n"
				"\t4: Enable print dma phy addr\n"
				"\t5: Disable print dma phy addr\n"
				"\t6: Enable print IVE_IMAGE_S, IVE_DATA_S, IVE_MEM_INFO_S\n"
				"\t7: Disable print IVE_IMAGE_S, IVE_DATA_S, IVE_MEM_INFO_S\n"
				"\t10: Dump ive op1/op2 mode info\n"
				"\t11: Dump hardware info\n");
	}
	return count;
}
#endif

int cvi_ive_ioctl(unsigned int cmd, void *g_kdata)
{
	struct cvi_ive_device *ndev = ive_dev;
	CVI_S32 ret = -1;

	if (ndev == NULL) {
		ive_printf(IVE_ERR, "IVE not init\n");
		return -1;
	}

	if (g_kdata || cmd == CVI_IVE_IOC_DUMP || cmd == CVI_IVE_IOC_CMDQ) {

        //memset((void *)g_kdata, 0, sizeof(g_kdata));

		//if (arg) {
     		//memcpy(g_kdata, arg, sizeof(g_kdata));
		//}

		switch (cmd) {
		case CVI_IVE_IOC_QUERY: {
			CVI_BOOL bFinish;
			struct cvi_ive_query_arg *val =
					(struct cvi_ive_query_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_QUERY], "QUERY");
			memcpy(&bFinish, val->pbFinish, sizeof(bool));
			ret = cvi_ive_Query(ndev, &bFinish, val->bBlock);
			memcpy(val->pbFinish, &bFinish, sizeof(bool));
			stop_ioctl_time(&g_time_infos[MOD_QUERY]);
		} break;
		case CVI_IVE_IOC_RESET: {
			start_ioctl_time(&g_time_infos[MOD_RESET], "RESET");
			ret = cvi_ive_reset(ndev, *((int *) g_kdata));
			stop_ioctl_time(&g_time_infos[MOD_RESET]);
		} break;
		case CVI_IVE_IOC_DUMP: {
			start_ioctl_time(&g_time_infos[MOD_DUMP], "DUMP");
			ret = cvi_ive_dump_reg_state(true);
			stop_ioctl_time(&g_time_infos[MOD_DUMP]);
		} break;
		case CVI_IVE_IOC_TEST: {
			struct cvi_ive_test_arg *val =
					(struct cvi_ive_test_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_TEST], "Test");
			ret = cvi_ive_test(ndev, val->pAddr, &val->u16Width,
						&val->u16Height);
			stop_ioctl_time(&g_time_infos[MOD_TEST]);
		} break;
		case CVI_IVE_IOC_DMA: {
			struct cvi_ive_ioctl_dma_arg *val =
					(struct cvi_ive_ioctl_dma_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_DMA], "DMA");
			ret = cvi_ive_DMA(ndev, &val->stSrc, &val->stDst,
						&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_DMA]);
		} break;
		case CVI_IVE_IOC_And: {
			struct cvi_ive_ioctl_and_arg *val =
					(struct cvi_ive_ioctl_and_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_AND], "And");
			ret = cvi_ive_And(ndev, &val->stSrc1, &val->stSrc2,
						&val->stDst, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_AND]);
		} break;
		case CVI_IVE_IOC_Or: {
			struct cvi_ive_ioctl_or_arg *val =
					(struct cvi_ive_ioctl_or_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_OR], "Or");
			ret = cvi_ive_Or(ndev, &val->stSrc1, &val->stSrc2,
						&val->stDst, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_OR]);
		} break;
		case CVI_IVE_IOC_Xor: {
			struct cvi_ive_ioctl_xor_arg *val =
					(struct cvi_ive_ioctl_xor_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_XOR], "Xor");
			ret = cvi_ive_Xor(ndev, &val->stSrc1, &val->stSrc2,
						&val->stDst, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_XOR]);
		} break;
		case CVI_IVE_IOC_Add: {
			struct cvi_ive_ioctl_add_arg *val =
					(struct cvi_ive_ioctl_add_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_ADD], "Add");
			ret = cvi_ive_Add(ndev, &val->stSrc1, &val->stSrc2,
						&val->stDst, &val->pstCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_ADD]);
		} break;
		case CVI_IVE_IOC_Sub: {
			struct cvi_ive_ioctl_sub_arg *val =
					(struct cvi_ive_ioctl_sub_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_SUB], "Sub");
			ret = cvi_ive_Sub(ndev, &val->stSrc1, &val->stSrc2,
						&val->stDst, &val->stCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_SUB]);
		} break;
		case CVI_IVE_IOC_Thresh: {
			struct cvi_ive_ioctl_thresh_arg *val =
					(struct cvi_ive_ioctl_thresh_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_THRESH], "Thresh");
			ret = cvi_ive_Thresh(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_THRESH]);
		} break;
		case CVI_IVE_IOC_Dilate: {
			struct cvi_ive_ioctl_dilate_arg *val =
					(struct cvi_ive_ioctl_dilate_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_DILA], "Dilate");
			ret = cvi_ive_Dilate(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_DILA]);
		} break;
		case CVI_IVE_IOC_Erode: {
			struct cvi_ive_ioctl_erode_arg *val =
					(struct cvi_ive_ioctl_erode_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_ERO], "Erode");
			ret = cvi_ive_Erode(ndev, &val->stSrc, &val->stDst,
						&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_ERO]);
		} break;
		case CVI_IVE_IOC_MatchBgModel: {
			struct cvi_ive_ioctl_match_bgmodel_arg *val =
					(struct cvi_ive_ioctl_match_bgmodel_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_BGM], "MatchBgModel");
			ret = cvi_ive_MatchBgModel(ndev, &val->stCurImg,
							&val->stBgModel,
							&val->stFgFlag, &val->stDiffFg,
							&val->stStatData, &val->stCtrl,
							val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_BGM]);
		} break;
		case CVI_IVE_IOC_UpdateBgModel: {
			struct cvi_ive_ioctl_update_bgmodel_arg *val =
					(struct cvi_ive_ioctl_update_bgmodel_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_BGU], "UpdateBgModel");
			ret = cvi_ive_UpdateBgModel(ndev, &val->stBgModel,
							&val->stFgFlag, &val->stBgImg,
							&val->stChgSta,
							&val->stStatData,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_BGU]);
		} break;
		case CVI_IVE_IOC_GMM: {
			struct cvi_ive_ioctl_gmm_arg *val =
					(struct cvi_ive_ioctl_gmm_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_GMM], "GMM");
			ret = cvi_ive_GMM(ndev, &val->stSrc, &val->stFg,
						&val->stBg, &val->stModel, &val->stCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_GMM]);
		} break;
		case CVI_IVE_IOC_GMM2: {
			struct cvi_ive_ioctl_gmm2_arg *val =
					(struct cvi_ive_ioctl_gmm2_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_GMM2], "GMM2");
			ret = cvi_ive_GMM2(ndev, &val->stSrc, &val->stFactor,
						&val->stFg, &val->stBg, &val->stInfo,
						&val->stModel, &val->stCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_GMM2]);
		} break;

		case CVI_IVE_IOC_Bernsen: {
			struct cvi_ive_ioctl_bernsen_arg *val =
					(struct cvi_ive_ioctl_bernsen_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_BERNSEN], "Bernsen");
			ret = cvi_ive_Bernsen(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_BERNSEN]);
		} break;
		case CVI_IVE_IOC_Filter: {
			struct cvi_ive_ioctl_filter_arg *val =
					(struct cvi_ive_ioctl_filter_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_FILTER3CH], "Filter");
			ret = cvi_ive_Filter(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_FILTER3CH]);
		} break;
		case CVI_IVE_IOC_Sobel: {
			struct cvi_ive_ioctl_sobel_arg *val =
					(struct cvi_ive_ioctl_sobel_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_SOBEL], "Sobel");
			ret = cvi_ive_Sobel(ndev, &val->stSrc, &val->stDstH,
						&val->stDstV, &val->stCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_SOBEL]);
		} break;
		case CVI_IVE_IOC_MagAndAng: {
			struct cvi_ive_ioctl_maganang_arg *val =
					(struct cvi_ive_ioctl_maganang_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_MAG], "MagAndAng");
			ret = cvi_ive_MagAndAng(ndev, &val->stSrc, &val->stDstMag,
						&val->stDstAng, &val->stCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_MAG]);
		} break;
		case CVI_IVE_IOC_CSC: {
			struct cvi_ive_ioctl_csc_arg *val =
					(struct cvi_ive_ioctl_csc_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_CSC], "CSC");
			ret = cvi_ive_CSC(ndev, &val->stSrc, &val->stDst,
						&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_CSC]);
		} break;
		case CVI_IVE_IOC_Hist: {
			struct cvi_ive_ioctl_hist_arg *val =
					(struct cvi_ive_ioctl_hist_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_HIST], "Hist");
			ret = cvi_ive_Hist(ndev, &val->stSrc, &val->stDst,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_HIST]);
		} break;
		case CVI_IVE_IOC_FilterAndCSC: {
			struct cvi_ive_ioctl_filter_and_csc_arg *val =
					(struct cvi_ive_ioctl_filter_and_csc_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_FILTERCSC], "FilterAndCSC");
			ret = cvi_ive_FilterAndCSC(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_FILTERCSC]);
		} break;
		case CVI_IVE_IOC_Map: {
			struct cvi_ive_ioctl_map_arg *val =
					(struct cvi_ive_ioctl_map_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_MAP], "Map");
			ret = cvi_ive_Map(ndev, &val->stSrc, &val->stMap,
						&val->stDst, &val->stCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_MAP]);
		} break;
		case CVI_IVE_IOC_NCC: {
			struct cvi_ive_ioctl_ncc_arg *val =
					(struct cvi_ive_ioctl_ncc_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_NCC], "NCC");
			ret = cvi_ive_NCC(ndev, &val->stSrc1, &val->stSrc2,
						&val->stDst, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_NCC]);
		} break;
		case CVI_IVE_IOC_Integ: {
			struct cvi_ive_ioctl_integ_arg *val =
					(struct cvi_ive_ioctl_integ_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_INTEG], "Integ");
			ret = cvi_ive_Integ(ndev, &val->stSrc, &val->stDst,
						&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_INTEG]);
		} break;
		case CVI_IVE_IOC_LBP: {
			struct cvi_ive_ioctl_lbp_arg *val =
					(struct cvi_ive_ioctl_lbp_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_LBP], "LBP");
			ret = cvi_ive_LBP(ndev, &val->stSrc, &val->stDst,
						&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_LBP]);
		} break;
		case CVI_IVE_IOC_Thresh_S16: {
			struct cvi_ive_ioctl_thresh_s16_arg *val =
					(struct cvi_ive_ioctl_thresh_s16_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_THRS16], "Thresh_S16");
			ret = cvi_ive_Thresh_S16(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_THRS16]);
		} break;
		case CVI_IVE_IOC_Thresh_U16: {
			struct cvi_ive_ioctl_thres_su16_arg *val =
					(struct cvi_ive_ioctl_thres_su16_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_THRU16], "Thresh_U16");
			ret = cvi_ive_Thresh_U16(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_THRU16]);
		} break;
		case CVI_IVE_IOC_16BitTo8Bit: {
			struct cvi_ive_ioctl_16bit_to_8bit_arg *val =
					(struct cvi_ive_ioctl_16bit_to_8bit_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_16To8], "16BitTo8Bit");
			ret = cvi_ive_16BitTo8Bit(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_16To8]);
		} break;
		case CVI_IVE_IOC_OrdStatFilter: {
			struct cvi_ive_ioctl_ord_stat_filter_arg *val =
					(struct cvi_ive_ioctl_ord_stat_filter_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_ORDSTAFTR], "OrdStatFilter");
			ret = cvi_ive_OrdStatFilter(ndev, &val->stSrc,
							&val->stDst, &val->stCtrl,
							val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_ORDSTAFTR]);
		} break;
		case CVI_IVE_IOC_CannyHysEdge: {
			struct cvi_ive_ioctl_canny_hys_edge_arg *val =
					(struct cvi_ive_ioctl_canny_hys_edge_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_CANNY], "CannyHysEdge");
			ret = cvi_ive_CannyHysEdge(ndev, &val->stSrc, &val->stDst,
							&val->stStack, &val->stCtrl,
							val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_CANNY]);
		} break;
		case CVI_IVE_IOC_NormGrad: {
			struct cvi_ive_ioctl_norm_grad_arg *val =
					(struct cvi_ive_ioctl_norm_grad_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_NORMG], "NormGrad");
			ret = cvi_ive_NormGrad(ndev, &val->stSrc, &val->stDstH,
							&val->stDstV, &val->stDstHV,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_NORMG]);
		} break;
		case CVI_IVE_IOC_GradFg: {
			struct cvi_ive_ioctl_grad_fg_arg *val =
					(struct cvi_ive_ioctl_grad_fg_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_GRADFG], "GradFg");
			ret = cvi_ive_GradFg(ndev, &val->stBgDiffFg,
							&val->stCurGrad, &val->stBgGrad,
							&val->stGradFg, &val->stCtrl,
							val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_GRADFG]);
		} break;
		case CVI_IVE_IOC_SAD: {
			struct cvi_ive_ioctl_sad_arg *val =
					(struct cvi_ive_ioctl_sad_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_SAD], "SAD");
			ret = cvi_ive_SAD(ndev, &val->stSrc1, &val->stSrc2,
						&val->stSad, &val->stThr, &val->stCtrl,
						val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_SAD]);
		} break;
		case CVI_IVE_IOC_Resize: {
			int i = 0;
			struct cvi_ive_ioctl_resize_arg *val =
					(struct cvi_ive_ioctl_resize_arg *) g_kdata;
			IVE_SRC_IMAGE_S *Src;
			IVE_DST_IMAGE_S *Dst;

			start_ioctl_time(&g_time_infos[MOD_RESIZE], "Resize");
			Src = calloc(1, sizeof(IVE_IMAGE_S) * val->stCtrl.u16Num);
			Dst = calloc(1, sizeof(IVE_IMAGE_S) * val->stCtrl.u16Num);
			for (i = 0; i < val->stCtrl.u16Num; i++) {
				memcpy(&Src[i], &val->astSrc[i], sizeof(Src[i]));
				memcpy(&Dst[i], &val->astDst[i], sizeof(Dst[i]));
			}
			ret = cvi_ive_Resize(ndev, Src, Dst, &val->stCtrl,
							val->bInstant);
			free(Src);
			free(Dst);
			stop_ioctl_time(&g_time_infos[MOD_RESIZE]);
		} break;
		case CVI_IVE_IOC_imgInToOdma: {
			struct cvi_ive_ioctl_filter_arg *val =
					(struct cvi_ive_ioctl_filter_arg *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_BYP], "imgInToOdma");
			ret = cvi_ive_imgInToOdma(ndev, &val->stSrc, &val->stDst,
							&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_BYP]);
		} break;
		case CVI_IVE_IOC_rgbPToYuvToErodeToDilate: {
			struct cvi_ive_ioctl_rgbPToYuvToErodeToDilate *val =
					(struct cvi_ive_ioctl_rgbPToYuvToErodeToDilate *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_ED],
					"rgbPToYuvToErodeToDilate");
			ret = cvi_ive_rgbPToYuvToErodeToDilate(
				ndev, &val->stSrc, &val->stDst1, &val->stDst2,
				&val->stCtrl, val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_ED]);
		} break;
		case CVI_IVE_IOC_STCandiCorner: {
			struct cvi_ive_ioctl_stcandicorner *val =
					(struct cvi_ive_ioctl_stcandicorner *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_STCANDI], "STCandiCorner");
			start_ioctl_time(&g_time_infos[MOD_STBOX], "STBox");
			ret = cvi_ive_STCandiCorner(ndev, &val->stSrc,
							&val->stDst, &val->stCtrl,
							val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_STCANDI]);
			stop_ioctl_time(&g_time_infos[MOD_STBOX]);
		} break;
		case CVI_IVE_IOC_MD: {
			struct cvi_ive_ioctl_md *val = (struct cvi_ive_ioctl_md *) g_kdata;

			start_ioctl_time(&g_time_infos[MOD_MD], "FrameDiffDetect");
			ret = cvi_ive_FrameDiffMotion(ndev, &val->stSrc1,
								&val->stSrc2, &val->stDst,
								&val->stCtrl,
								val->bInstant);
			stop_ioctl_time(&g_time_infos[MOD_MD]);
		} break;
		case CVI_IVE_IOC_CMDQ: {
			start_ioctl_time(&g_time_infos[MOD_CMDQ], "CmdQ");
			ret = cvi_ive_CmdQ(ndev);
			stop_ioctl_time(&g_time_infos[MOD_CMDQ]);
		} break;
		default:
			ive_printf(IVE_ERR, "this api no support now\n");
			return -1;
		}
	}

		if (ret) {
			ive_printf(IVE_ERR, "[IVE] ioctl _IOC_NR(%d) fail\n", cmd);
			return ret;
		}

	return ret;
}

int cvi_ive_open(void)
{
	if (ive_dev == NULL) {
		ive_printf(IVE_ERR, "IVE Device not init\n");
		return -1;
	}
	pthread_mutex_lock(&ive_dev->close_lock);
	ive_dev->use_count++;
	pthread_mutex_unlock(&ive_dev->close_lock);

	return ive_dev->use_count;
}

int cvi_ive_close(void)
{
	pthread_mutex_lock(&ive_dev->close_lock);
	ive_dev->use_count--;
	pthread_mutex_unlock(&ive_dev->close_lock);

	return 0;
}

static int cvi_ive_probe(void)
{
	ive_printf(IVE_DBG, "IVE start\n");

	ive_dev = calloc(1, sizeof(struct cvi_ive_device));
	if (!ive_dev) {
		ive_printf(IVE_ERR, "Failed to allocate resource\n");
		return -1;
	}

	ive_dev->ive_base = IVE_TOP_PHY_REG_BASE;

	if (assign_ive_block_addr(ive_dev->ive_base)) {
		ive_printf(IVE_ERR, "IVE Set Reg fail\n");
		return -1;
	}

	ive_dev->ive_irq = IVE_IRQ_NUM;

	ive_dev->use_count = 0;

	pthread_mutex_init(&ive_dev->close_lock, NULL);
	sem_init(&ive_dev->frame_done, 0, 0);
	sem_init(&ive_dev->op_done, 0, 0);
	// 注册中断
	if (request_irq(ive_dev->ive_irq, cvi_ive_irq_handler, 0, "cvi-ive", ive_dev)) {
		ive_printf(IVE_ERR, "Unable to request ive IRQ(%d)\r\n", ive_dev->ive_irq);
		return -1;
	}

	g_time_infos = calloc(MOD_ALL * sizeof(struct ive_profiling_info), 1);

	g_enable_usage_profiling = 0;

	// stcandicorner_workaround(ive_dev);
	return 0;
}

static int cvi_ive_remove(void)
{
	pthread_mutex_destroy(&ive_dev->close_lock);
	sem_destroy(&ive_dev->frame_done);
	sem_destroy(&ive_dev->op_done);
	free(ive_dev);
	free(g_time_infos);

	return 0;
}

int cvi_ive_init(void)
{
	int ret = 0;
	ret = cvi_ive_probe();
	if (ret != 0) {
		printf("ive drv init failed\n");
	}
	return 0;
}

int cvi_ive_deinit(void)
{
	cvi_ive_remove();
	return 0;
}


