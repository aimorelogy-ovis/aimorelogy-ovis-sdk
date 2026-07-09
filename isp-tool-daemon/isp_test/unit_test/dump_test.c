
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_buffer.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_isp.h"

typedef int (*dump_test_fn)(VI_PIPE ViPipe);

#define TEST_FAIL(fmt, arg...) printf("[ISP_TEST_FAIL] %s,%d: " fmt, __FUNCTION__, __LINE__, ##arg)

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define DUMP_TEST_MAX_LOOP (10)
#define MAX_FRAME_NUM (4)

static int dump_yuv_test(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);

	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32	VpssGrp = 0;
	CVI_S32	VpssChn = 0;
	VIDEO_FRAME_INFO_S stFrame;

	printf("Run dump yuv test case\n");

	memset(&stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VPSS_GetChnFrame failed with %#x!\n", s32Ret);
		return -1;
	}

	usleep(50 * 1000);

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VPSS_ReleaseChnFrame failed with %#x!\n", s32Ret);
		return -1;
	}

	return 0;
}

static int dump_raw_test(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DUMP_ATTR_S stDumpAttr;
	VIDEO_FRAME_INFO_S stFrame[MAX_FRAME_NUM];

	printf("Run dump raw test case\n");

	memset(&stDumpAttr, 0, sizeof(VI_DUMP_ATTR_S));

	stDumpAttr.bEnable = CVI_TRUE;
	stDumpAttr.u32Depth = 0;
	stDumpAttr.enDumpType = VI_DUMP_TYPE_RAW;
	s32Ret = CVI_VI_SetPipeDumpAttr(ViPipe, &stDumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_SetPipeDumpAttr failed with %#x!\n", s32Ret);
		return -1;
	}

	memset(stFrame, 0, sizeof(VIDEO_FRAME_INFO_S) * MAX_FRAME_NUM);
	s32Ret = CVI_VI_GetPipeFrame(ViPipe, stFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_GetPipeFrame failed with %#x!\n", s32Ret);
		return -1;
	}

	usleep(50 * 1000);

	if (stFrame[0].stVFrame.u64PhyAddr[0] == 0) {
		TEST_FAIL("Get invalid raw frame phy addr!\n");
		return -1;
	}

	s32Ret = CVI_VI_ReleasePipeFrame(ViPipe, stFrame);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_ReleasePipeFrame failed with %#x!\n", s32Ret);
		return -1;
	}

	return 0;
}

static int dump_raw_use_vb_test(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DEV_ATTR_S stDevAttr;
	VI_CHN_ATTR_S stChnAttr;
	VB_POOL_CONFIG_S stVbPoolCfg;
	VB_POOL VbPool;
	VB_BLK VbBlk;
	VI_DUMP_ATTR_S stDumpAttr;
	VIDEO_FRAME_INFO_S stFrame[MAX_FRAME_NUM];

	printf("Run dump raw use vb test case\n");

	memset(&stDevAttr, 0, sizeof(VI_DEV_ATTR_S));
	s32Ret = CVI_VI_GetDevAttr(ViPipe, &stDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_GetDevAttr failed with %#x!\n", s32Ret);
		return -1;
	}

	CVI_U8 u8MaxFrameNum = 0;

	switch (stDevAttr.stWDRAttr.enWDRMode) {
	case WDR_MODE_4To1_LINE:
	case WDR_MODE_4To1_FRAME:
	case WDR_MODE_4To1_FRAME_FULL_RATE:
		u8MaxFrameNum = 4;
		break;
	case WDR_MODE_3To1_LINE:
	case WDR_MODE_3To1_FRAME:
	case WDR_MODE_3To1_FRAME_FULL_RATE:
		u8MaxFrameNum = 3;
		break;
	case WDR_MODE_2To1_LINE:
	case WDR_MODE_2To1_FRAME:
	case WDR_MODE_2To1_FRAME_FULL_RATE:
		u8MaxFrameNum = 2;
		break;
	default:
		u8MaxFrameNum = 1;
		break;
	}

	memset(&stChnAttr, 0, sizeof(VI_CHN_ATTR_S));
	s32Ret = CVI_VI_GetChnAttr(ViPipe, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_GetChnAttr failed with %#x!\n", s32Ret);
		return -1;
	}

	CVI_U32 u32BlkSize = VI_GetRawBufferSize(stChnAttr.stSize.u32Width,
							stChnAttr.stSize.u32Height,
							stChnAttr.enPixelFormat,
							stChnAttr.enCompressMode,
							DEFAULT_ALIGN,
							CVI_FALSE);

	memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
	stVbPoolCfg.u32BlkCnt = 1;
	stVbPoolCfg.u32BlkSize = u32BlkSize * u8MaxFrameNum;
	VbPool = CVI_VB_CreatePool(&stVbPoolCfg);
	if (VbPool == VB_INVALID_POOLID) {
		TEST_FAIL("CVI_VB_CreatePool failed!\n");
		return -1;
	}

	VbBlk = CVI_VB_GetBlock(VbPool, stVbPoolCfg.u32BlkSize);
	if (VbBlk == VB_INVALID_HANDLE) {
		TEST_FAIL("CVI_VB_GetBlock failed!\n");
		return -1;
	}

	memset(&stFrame, 0, sizeof(VIDEO_FRAME_INFO_S) * MAX_FRAME_NUM);
	stFrame[0].stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(VbBlk);
	stFrame[0].stVFrame.u32Length[0] = stVbPoolCfg.u32BlkSize / u8MaxFrameNum;

	if (u8MaxFrameNum > 1) {
		CVI_U32 u32PerFrameSize = stFrame[0].stVFrame.u32Length[0];

		for (CVI_U32 u32Idx = 1; u32Idx < u8MaxFrameNum; ++u32Idx) {
			stFrame[u32Idx].stVFrame.u64PhyAddr[0]
				= stFrame[0].stVFrame.u64PhyAddr[0] + (u32PerFrameSize * u32Idx);
			stFrame[u32Idx].stVFrame.u32Length[0] = u32PerFrameSize;
		}
	}

	memset(&stDumpAttr, 0, sizeof(VI_DUMP_ATTR_S));

	stDumpAttr.bEnable = CVI_TRUE;
	stDumpAttr.u32Depth = 0;
	stDumpAttr.enDumpType = VI_DUMP_TYPE_RAW;
	s32Ret = CVI_VI_SetPipeDumpAttr(ViPipe, &stDumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_SetPipeDumpAttr failed with %#x!\n", s32Ret);
		return -1;
	}

	s32Ret = CVI_VI_GetPipeFrame(ViPipe, stFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_GetPipeFrame failed with %#x!\n", s32Ret);
		return -1;
	}

	usleep(50 * 1000);

	if (stFrame[0].stVFrame.u64PhyAddr[0] != CVI_VB_Handle2PhysAddr(VbBlk)) {
		TEST_FAIL("Get invalid raw frame phy addr!\n");
		return -1;
	}

	s32Ret = CVI_VI_ReleasePipeFrame(ViPipe, stFrame);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_ReleasePipeFrame failed with %#x!\n", s32Ret);
		return -1;
	}

	s32Ret = CVI_VB_ReleaseBlock(VbBlk);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VB_ReleaseBlock failed with %#x!\n", s32Ret);
		return -1;
	}

	s32Ret = CVI_VB_DestroyPool(VbPool);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VB_DestroyPool failed with %#x!\n", s32Ret);
		return -1;
	}

	return 0;
}

static int dump_smooth_raw_test(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DEV_ATTR_S stDevAttr;
	VI_CHN_ATTR_S stChnAttr;
	VB_POOL_CONFIG_S stVbPoolCfg;
	VB_POOL VbPool;
	VB_BLK VbBlk;
	VIDEO_FRAME_INFO_S stFrame[MAX_FRAME_NUM];
	CVI_U8 fullSizeBlkCnt = 3;
	CVI_U64 u64FullVbPhyAddr = 0;
	CVI_U64 *pFullVbPhyAddrList = NULL;
	VI_SMOOTH_RAW_DUMP_INFO_S stDumpInfo;

	printf("Run dump smooth raw test case\n");

	memset(&stDevAttr, 0, sizeof(VI_DEV_ATTR_S));
	s32Ret = CVI_VI_GetDevAttr(ViPipe, &stDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_GetDevAttr failed with %#x!\n", s32Ret);
		return -1;
	}

	CVI_U8 u8MaxFrameNum = 0;

	switch (stDevAttr.stWDRAttr.enWDRMode) {
	case WDR_MODE_4To1_LINE:
	case WDR_MODE_4To1_FRAME:
	case WDR_MODE_4To1_FRAME_FULL_RATE:
		u8MaxFrameNum = 4;
		break;
	case WDR_MODE_3To1_LINE:
	case WDR_MODE_3To1_FRAME:
	case WDR_MODE_3To1_FRAME_FULL_RATE:
		u8MaxFrameNum = 3;
		break;
	case WDR_MODE_2To1_LINE:
	case WDR_MODE_2To1_FRAME:
	case WDR_MODE_2To1_FRAME_FULL_RATE:
		u8MaxFrameNum = 2;
		break;
	default:
		u8MaxFrameNum = 1;
		break;
	}

	memset(&stChnAttr, 0, sizeof(VI_CHN_ATTR_S));
	s32Ret = CVI_VI_GetChnAttr(ViPipe, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_GetChnAttr failed with %#x!\n", s32Ret);
		return -1;
	}

	CVI_U32 u32BlkSize = VI_GetRawBufferSize(stChnAttr.stSize.u32Width,
							stChnAttr.stSize.u32Height,
							stChnAttr.enPixelFormat,
							stChnAttr.enCompressMode,
							DEFAULT_ALIGN,
							CVI_FALSE);

	memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
	stVbPoolCfg.u32BlkCnt = u8MaxFrameNum * fullSizeBlkCnt;
	stVbPoolCfg.u32BlkSize = u32BlkSize;
	VbPool = CVI_VB_CreatePool(&stVbPoolCfg);
	if (VbPool == VB_INVALID_POOLID) {
		TEST_FAIL("CVI_VB_CreatePool failed!\n");
		return -1;
	}

	pFullVbPhyAddrList = calloc(1, sizeof(*pFullVbPhyAddrList) * stVbPoolCfg.u32BlkCnt);

	for (CVI_U8 i = 0; i < fullSizeBlkCnt * u8MaxFrameNum; i++) {
		VbBlk = CVI_VB_GetBlock(VbPool, u32BlkSize);
		if (VbBlk == VB_INVALID_HANDLE) {
			TEST_FAIL("CVI_VB_GetBlock failed!\n");
			return -1;
		}
		u64FullVbPhyAddr = CVI_VB_Handle2PhysAddr(VbBlk);
		*(pFullVbPhyAddrList + i) = u64FullVbPhyAddr;
	}

	memset(&stDumpInfo, 0, sizeof(stDumpInfo));

	stDumpInfo.ViPipe = ViPipe;
	stDumpInfo.u8BlkCnt = fullSizeBlkCnt;
	stDumpInfo.phy_addr_list = pFullVbPhyAddrList;

	s32Ret = CVI_VI_StartSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_StartSmoothRawDump failed with %#x!\n", s32Ret);
		return -1;
	}

	for (CVI_U8 i = 0; i < DUMP_TEST_MAX_LOOP; i++) {
		memset(stFrame, 0, sizeof(VIDEO_FRAME_INFO_S) * MAX_FRAME_NUM);
		s32Ret = CVI_VI_GetSmoothRawDump(ViPipe, stFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			TEST_FAIL("CVI_VI_GetSmoothRawDump failed with %#x!\n", s32Ret);
			return -1;
		}
		usleep(5 * 1000);
		s32Ret = CVI_VI_PutSmoothRawDump(ViPipe, stFrame);
		if (s32Ret != CVI_SUCCESS) {
			TEST_FAIL("CVI_VI_PutSmoothRawDump failed with %#x!\n", s32Ret);
			return -1;
		}
	}

	s32Ret = CVI_VI_StopSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_StopSmoothRawDump failed with %#x!\n", s32Ret);
		return -1;
	}

	for (CVI_U8 i = 0; i < fullSizeBlkCnt * u8MaxFrameNum; i++) {
		u64FullVbPhyAddr = *(pFullVbPhyAddrList + i);
		VbBlk = CVI_VB_PhysAddr2Handle(u64FullVbPhyAddr);
		if (VbBlk != VB_INVALID_HANDLE) {
			s32Ret = CVI_VB_ReleaseBlock(VbBlk);
			if (s32Ret != CVI_SUCCESS) {
				TEST_FAIL("CVI_VB_ReleaseBlock failed with %#x!\n", s32Ret);
				return -1;
			}
		} else {
			TEST_FAIL("CVI_VB_PhysAddr2Handle failed!\n");
			return -1;
		}
	}
	free(pFullVbPhyAddrList);
	pFullVbPhyAddrList = NULL;

	s32Ret = CVI_VB_DestroyPool(VbPool);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VB_DestroyPool failed with %#x!\n", s32Ret);
		return -1;
	}

	return 0;
}

static int dump_reg_test(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *pfFile = NULL;
	VI_DUMP_REGISTER_TABLE_S stDumpRegTable;
	ISP_INNER_STATE_INFO_S stInnerStateInfo;

	printf("Run dump reg test case\n");

	memset(&stDumpRegTable, 0, sizeof(VI_DUMP_REGISTER_TABLE_S));
	memset(&stInnerStateInfo, 0, sizeof(ISP_INNER_STATE_INFO_S));

	s32Ret = CVI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_ISP_QueryInnerStateInfo failed with %#x!\n", s32Ret);
		return -1;
	}

	stDumpRegTable.MlscGainLut.RGain = stInnerStateInfo.mlscGainTable.RGain;
	stDumpRegTable.MlscGainLut.GGain = stInnerStateInfo.mlscGainTable.GGain;
	stDumpRegTable.MlscGainLut.BGain = stInnerStateInfo.mlscGainTable.BGain;

	pfFile = fopen("/tmp/dump_reg.reg", "w");
	if (pfFile == NULL) {
		TEST_FAIL("open /tmp/dump_reg.reg failed!\n");
		return -1;
	}
	s32Ret = CVI_VI_DumpHwRegisterToFile(ViPipe, pfFile, &stDumpRegTable);
	if (s32Ret != CVI_SUCCESS) {
		TEST_FAIL("CVI_VI_DumpHwRegisterToFile failed with %#x!\n", s32Ret);
		fclose(pfFile);
		return -1;
	}
	fclose(pfFile);
	remove("/tmp/dump_reg.reg");

	return 0;
}

static dump_test_fn g_dump_test_fn_arr[] = {
	dump_yuv_test,
	dump_raw_test,
	dump_raw_use_vb_test,
	dump_smooth_raw_test,
	dump_reg_test
};

int dump_test_main(VI_PIPE ViPipe)
{
	int ret = 0;
	int passed_cnt = 0;
	int failed_cnt = 0;

	int g_dump_test_fn_cnt = sizeof(g_dump_test_fn_arr) / sizeof(dump_test_fn);

	for (int i = 0; i < g_dump_test_fn_cnt; i++) {
		for (int j = 0; j < DUMP_TEST_MAX_LOOP; j++) {
			ret = g_dump_test_fn_arr[i](ViPipe);
			if (ret != 0) {
				break;
			}
			usleep(50 * 1000);
		}
		if (ret != 0) {
			failed_cnt++;
		} else {
			passed_cnt++;
		}
	}

	printf("ISP_DUMP_TEST finished, total: %d, passed: %d, failed: %d, passed_ratio: %.2f%%\n",
		   g_dump_test_fn_cnt, passed_cnt, failed_cnt,
		   (float)passed_cnt / g_dump_test_fn_cnt * 100);

	if (failed_cnt > 0) {
		ret = -1;
		printf("ISP_DUMP_TEST pipe: %d failed!\n", ViPipe);
	} else {
		ret = 0;
		printf("ISP_DUMP_TEST pipe: %d success!\n", ViPipe);
	}

	return ret;
}
