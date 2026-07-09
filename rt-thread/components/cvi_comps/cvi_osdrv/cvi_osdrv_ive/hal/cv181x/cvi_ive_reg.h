/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: reg.h
 * Description: driver register access control header file
 */

#ifndef CVI_IVE_REG
#define CVI_IVE_REG

// include register
//#include "ive_ccl_reg.h"
//#include "ive_dma_reg.h"
//#include "ive_filterop_reg.h"
//#include "ive_gmm_reg.h"
//#include "ive_map_reg.h"
//#include "ive_match_bg_reg.h"
//#include "ive_sad_reg.h"
//#include "ive_top_reg.h"
//#include "ive_update_bg_reg.h"
// include regBuilder
#include "mmio.h"

#include "builder/reg_ive_ccl.h"
#include "builder/reg_ive_ccl.struct.h"
#include "builder/reg_ive_dma.h"
#include "builder/reg_ive_dma.struct.h"
#include "builder/reg_ive_filterop.h"
#include "builder/reg_ive_filterop.struct.h"
#include "builder/reg_ive_gmm.h"
#include "builder/reg_ive_gmm.struct.h"
#include "builder/reg_ive_map.h"
#include "builder/reg_ive_map.struct.h"
#include "builder/reg_ive_match_bg.h"
#include "builder/reg_ive_match_bg.struct.h"
#include "builder/reg_ive_sad.h"
#include "builder/reg_ive_sad.struct.h"
#include "builder/reg_ive_top.h"
#include "builder/reg_ive_top.struct.h"
#include "builder/reg_ive_update_bg.h"
#include "builder/reg_ive_update_bg.struct.h"
#include "builder/reg_ive_ncc.h"
#include "builder/reg_ive_ncc.struct.h"
#include "builder/reg_cmdq.h"
#include "builder/reg_cmdq.struct.h"

#include "builder/reg_isp_dma_ctl.h"
#include "builder/reg_isp_dma_ctl.struct.h"
#include "builder/reg_isp_rdma.h"
#include "builder/reg_isp_rdma.struct.h"
#include "builder/reg_isp_wdma.h"
#include "builder/reg_isp_wdma.struct.h"

#include "builder/reg_img_in.h"
#include "builder/reg_img_in.struct.h"
#include "builder/reg_ive_hist.h"
#include "builder/reg_ive_hist.struct.h"
#include "builder/reg_sc_odma.h"
#include "builder/reg_sc_odma.struct.h"
#include "builder/reg_ive_intg.h"
#include "builder/reg_ive_intg.struct.h"


#ifdef __cplusplus
extern "C" {
#endif

// ip address
#define IVE_IRQ_NUM (97)
#define IVE_INTR_NUM (138)
#define CSIMAC0_INTR_NUM (155)
#define CSIMAC1_INTR_NUM (156)
#define IVE_TOP_PHY_REG_BASE (0x0A0A0000)
#define DRAM_PHY_BASE (0x80000000)
#define IVE_BLK_REGS_BITW (12)
#define IVE_BLK_ID_BITW (8)
#define IVE_PRERAW_INST_NUM (2)

#define VREG_SIZE (sizeof(struct VREG_RESV))
#define ADMA_DESC_SIZE (sizeof(struct IVECQ_ADMA_DESC_T))

/* IVE REG FIELD DEFINE */

/* IVE BLOCK ADDR OFFSET DEFINE */
//#define IVE_BLK_BA_IVE_TOP 0XA0C_6000
#define IVE_BLK_BA_IVE_TOP (0X00000000)
#define IVE_BLK_BA_IMG_IN (0X00000400)
#define IVE_BLK_BA_RDMA_IMG1 (0X00000500)
#define IVE_BLK_BA_MAP (0X00000600)
#define IVE_BLK_BA_HIST (0X00000700)
#define IVE_BLK_BA_INTG (0X00000800)
#define IVE_BLK_BA_NCC (0X00000900)
#define IVE_BLK_BA_SAD (0X00000A00)
#define IVE_BLK_BA_SAD_WDMA (0X00000A80)
#define IVE_BLK_BA_SAD_WDMA_THR (0X00000B00)
#define IVE_BLK_BA_GMM_MODEL_RDMA_0 (0X00001000)
#define IVE_BLK_BA_GMM_MODEL_RDMA_1 (0X00001040)
#define IVE_BLK_BA_GMM_MODEL_RDMA_2 (0X00001080)
#define IVE_BLK_BA_GMM_MODEL_RDMA_3 (0X000010C0)
#define IVE_BLK_BA_GMM_MODEL_RDMA_4 (0X00001100)
#define IVE_BLK_BA_GMM_MODEL_WDMA_0 (0X00001140)
#define IVE_BLK_BA_GMM_MODEL_WDMA_1 (0X00001180)
#define IVE_BLK_BA_GMM_MODEL_WDMA_2 (0X000011C0)
#define IVE_BLK_BA_GMM_MODEL_WDMA_3 (0X00001200)
#define IVE_BLK_BA_GMM_MODEL_WDMA_4 (0X00001240)
#define IVE_BLK_BA_GMM_MATCH_WDMA (0X00001280)
#define IVE_BLK_BA_GMM (0X000012C0)
#define IVE_BLK_BA_BG_MATCH (0X00001400)
#define IVE_BLK_BA_BG_UPDATE (0X00001600)
#define IVE_BLK_BA_FILTEROP (0X00002000)
#define IVE_BLK_BA_CCL (0X00002400)
#define IVE_BLK_BA_CCL_SRC_RDMA (0X00002440)
#define IVE_BLK_BA_CCL_DST_WDMA (0X00002480)
#define IVE_BLK_BA_CCL_REGION_WDMA (0X000024C0)
#define IVE_BLK_BA_DMAF (0X00002600)
#define IVE_BLK_BA_LK (0X00002700)
#define IVE_BLK_BA_RDMA_EIGVAL (0X00002800)
#define IVE_BLK_BA_WDMA (0X00002900) // (W AXI MASTER)
#define IVE_BLK_BA_RDMA (0X00002A00) // (R AXI MASTER)
//#define IVE_BLK_BA_WDMA_Y          (IVE_BLK_BA_FILTEROP - 0x1C0)
#define IVE_BLK_BA_CMDQ (0X00003000)

#define MAP_IVE_BLOCK_ID(_ba)                                                  \
	(((_ba) >> IVE_BLK_REGS_BITW) & ((1 << IVE_BLK_ID_BITW) - 1))

enum IVE_BLK_ID_T {
	IVE_BLK_ID_IVE_TOP = MAP_IVE_BLOCK_ID(IVE_BLK_BA_IVE_TOP),
	IVE_BLK_ID_IMG_IN = MAP_IVE_BLOCK_ID(IVE_BLK_BA_IMG_IN),
	IVE_BLK_ID_RDMA_IMG1 = MAP_IVE_BLOCK_ID(IVE_BLK_BA_RDMA_IMG1),
	IVE_BLK_ID_MAP = MAP_IVE_BLOCK_ID(IVE_BLK_BA_MAP),
	IVE_BLK_ID_HIST = MAP_IVE_BLOCK_ID(IVE_BLK_BA_HIST),
	IVE_BLK_ID_INTG = MAP_IVE_BLOCK_ID(IVE_BLK_BA_INTG),
	IVE_BLK_ID_SAD = MAP_IVE_BLOCK_ID(IVE_BLK_BA_SAD),
	IVE_BLK_ID_SAD_WDMA = MAP_IVE_BLOCK_ID(IVE_BLK_BA_SAD_WDMA),
	IVE_BLK_ID_SAD_WDMA_THR = MAP_IVE_BLOCK_ID(IVE_BLK_BA_SAD_WDMA_THR),
	IVE_BLK_ID_NCC = MAP_IVE_BLOCK_ID(IVE_BLK_BA_NCC),
	IVE_BLK_ID_GMM_MODEL_RDMA_0 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_0),
	IVE_BLK_ID_GMM_MODEL_RDMA_1 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_1),
	IVE_BLK_ID_GMM_MODEL_RDMA_2 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_2),
	IVE_BLK_ID_GMM_MODEL_RDMA_3 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_3),
	IVE_BLK_ID_GMM_MODEL_RDMA_4 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_4),
	IVE_BLK_ID_GMM_MODEL_WDMA_0 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_0),
	IVE_BLK_ID_GMM_MODEL_WDMA_1 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_1),
	IVE_BLK_ID_GMM_MODEL_WDMA_2 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_2),
	IVE_BLK_ID_GMM_MODEL_WDMA_3 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_3),
	IVE_BLK_ID_GMM_MODEL_WDMA_4 =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_4),
	IVE_BLK_ID_GMM_MATCH_WDMA = MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MATCH_WDMA),
	IVE_BLK_ID_GMM = MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM),
	IVE_BLK_ID_BG_MATCH = MAP_IVE_BLOCK_ID(IVE_BLK_BA_BG_MATCH),
	IVE_BLK_ID_BG_UPDATE = MAP_IVE_BLOCK_ID(IVE_BLK_BA_BG_UPDATE),
	IVE_BLK_ID_FILTEROP = MAP_IVE_BLOCK_ID(IVE_BLK_BA_FILTEROP),
	IVE_BLK_ID_CCL = MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL),
	IVE_BLK_ID_CCL_SRC_RDMA = MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL_SRC_RDMA),
	IVE_BLK_ID_CCL_DST_WDMA = MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL_DST_WDMA),
	IVE_BLK_ID_CCL_REGION_WDMA =
		MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL_REGION_WDMA),
	IVE_BLK_ID_DMAF = MAP_IVE_BLOCK_ID(IVE_BLK_BA_DMAF),
	IVE_BLK_ID_LK = MAP_IVE_BLOCK_ID(IVE_BLK_BA_LK),
	IVE_BLK_ID_RDMA_EIGVAL = MAP_IVE_BLOCK_ID(IVE_BLK_BA_RDMA_EIGVAL),
	IVE_BLK_ID_WDMA = MAP_IVE_BLOCK_ID(IVE_BLK_BA_WDMA),
	IVE_BLK_ID_RDMA = MAP_IVE_BLOCK_ID(IVE_BLK_BA_RDMA),
	IVE_BLK_ID_MAX
};

#ifdef __cplusplus
}
#endif

#define ive_reg_read(addr) mmio_read_32(addr)
#define ive_reg_write(data, addr) mmio_write_32(addr, data)

#endif //_CVI_REG_H_
