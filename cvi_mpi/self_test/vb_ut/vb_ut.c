#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_buffer.h"

#include "ut_comm.h"

#define COMMON_POOL0_BLK_SIZE (0x300000) // 3M
#define COMMON_POOL1_BLK_SIZE (0x100000) // 1M
#define COMMON_POOL0_BLK_CNT (3)
#define COMMON_POOL1_BLK_CNT (3)

typedef enum _VB_TEST_OP {
	VB_TEST_MMAP_POOL = 0,
	VB_TEST_GET_BLOCK,
	VB_TEST_CREATE_POOL,
	VB_TEST_GET_VB_CFG,
	VB_TEST_GET_BLK_INFO,
	VB_TEST_VB_BUF_SIZE,
	VB_TEST_CREATE_EX_POOL,
	VB_TEST_AUTO = 99,
} VB_TEST_OP;

static VB_CONFIG_S stVbConf;

static CVI_VOID vb_memcpy_test(CVI_VOID *dst, CVI_VOID *src, size_t size, CVI_U32 *duration)
{
	struct timespec time[2];

	clock_gettime(CLOCK_MONOTONIC, &time[0]);
	memcpy(dst, src, size);
	clock_gettime(CLOCK_MONOTONIC, &time[1]);
	*duration = GetTimeDiffInUs(time[0], time[1]);
}

static CVI_S32 vb_test_compare_config(VB_CAL_CONFIG_S *pstVbCfgNew, VB_CAL_CONFIG_S *pstVbCfgGolden)
{
	return  (pstVbCfgGolden->u32VBSize != pstVbCfgNew->u32VBSize) ||
		(pstVbCfgGolden->u32MainStride != pstVbCfgNew->u32MainStride) ||
		(pstVbCfgGolden->u32CStride != pstVbCfgNew->u32CStride) ||
		(pstVbCfgGolden->u32MainSize != pstVbCfgNew->u32MainSize) ||
		(pstVbCfgGolden->u32MainYSize != pstVbCfgNew->u32MainYSize) ||
		(pstVbCfgGolden->u32MainCSize != pstVbCfgNew->u32MainCSize) ||
		(pstVbCfgGolden->u32VBSize != pstVbCfgNew->u32VBSize) ||
		(pstVbCfgGolden->plane_num != pstVbCfgNew->plane_num);
}

static CVI_VOID vb_test_print_config(VB_CAL_CONFIG_S *pstVbCalConfig)
{
	UT_PRT(".u32VBSize = %d,\n", pstVbCalConfig->u32VBSize);
	UT_PRT(".u32MainStride = %d,\n", pstVbCalConfig->u32MainStride);
	UT_PRT(".u32CStride = %d,\n", pstVbCalConfig->u32CStride);
	UT_PRT(".u32MainSize = %d,\n", pstVbCalConfig->u32MainSize);
	UT_PRT(".u32MainYSize = %d,\n", pstVbCalConfig->u32MainYSize);
	UT_PRT(".u32MainCSize = %d,\n", pstVbCalConfig->u32MainCSize);
	UT_PRT(".u16AddrAlign = %d,\n", pstVbCalConfig->u16AddrAlign);
	UT_PRT(".plane_num = %d,\n", pstVbCalConfig->plane_num);
}

#ifndef CONFIG_DUAL_OS
static CVI_S32 vb_ut_mmap_pool_test(CVI_VOID)
{
	VB_POOL PoolId = 0;
	VB_BLK blk = VB_INVALID_HANDLE;
	CVI_S32 s32Ret = CVI_SUCCESS;
	const CVI_U32 bufLen = 0x2F0000;
	CVI_VOID *pVirAddr = NULL;
	CVI_U8 *pu8TestBuf = NULL;
	CVI_U32 duration1, duration2;

	s32Ret = CVI_VB_MmapPool(PoolId);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_MmapPool NG\n");
		goto MMAP_POOL_TEST_FAIL;
	}

	blk = CVI_VB_GetBlock(PoolId, bufLen);
	if (blk == VB_INVALID_HANDLE) {
		UT_PRT("CVI_VB_GetBlock NG\n");
		goto MMAP_POOL_TEST_FAIL;
	}

	s32Ret = CVI_VB_GetBlockVirAddr(PoolId, blk, &pVirAddr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_GetBlockVirAddr NG\n");
		goto MMAP_POOL_TEST_FAIL;
	}
	memset(pVirAddr, 0xab, bufLen);

	pu8TestBuf = malloc(bufLen);
	memset(pu8TestBuf, 0, bufLen);
	vb_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
	vb_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration2);
	if (memcmp(pu8TestBuf, pVirAddr, bufLen) != 0) {
		UT_PRT("memcpy test fail\n");
		goto MMAP_POOL_TEST_FAIL;
	}
	free(pu8TestBuf);
	UT_PRT("*map pool memcpy(%d bytes)  %d - %d\n", bufLen, duration1, duration2);

	CVI_VB_ReleaseBlock(blk);

	s32Ret = CVI_VB_MunmapPool(PoolId);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_MunmapPool NG\n");
		goto MMAP_POOL_TEST_FAIL;
	}
	return CVI_SUCCESS;

MMAP_POOL_TEST_FAIL:
	if (pu8TestBuf)
		free(pu8TestBuf);

	if (blk != VB_INVALID_HANDLE)
		CVI_VB_ReleaseBlock(blk);
	CVI_VB_MunmapPool(PoolId);
	return CVI_FAILURE;
}
#endif

static CVI_S32 vb_ut_get_block_test(CVI_VOID)
{
	VB_BLK blk = VB_INVALID_HANDLE;
	VB_POOL pool;
	CVI_U32 blk_size[2] = {COMMON_POOL0_BLK_SIZE, COMMON_POOL1_BLK_SIZE};
	CVI_U64 u64PhyAddr = 0;
	CVI_VOID *pVirAddr = NULL;
	CVI_U8 *pu8TestBuf = NULL;
	CVI_U32 duration1, duration2;

	for (CVI_S32 i = 0; i < 2; ++i) {
		blk = CVI_VB_GetBlock(VB_INVALID_POOLID, blk_size[i]);
		if (blk == VB_INVALID_HANDLE) {
			UT_PRT("CVI_VB_GetBlock NG\n");
			goto GET_BLOCK_TEST_FAIL;
		}

		pool = CVI_VB_Handle2PoolId(blk);
		if (pool != (CVI_U32)i) {
			UT_PRT("get blk from unexpected pool(%d)\n", pool);
			goto GET_BLOCK_TEST_FAIL;
		}

		u64PhyAddr = CVI_VB_Handle2PhysAddr(blk);
		if (!u64PhyAddr) {
			UT_PRT("CVI_VB_Handle2PhysAddr NG\n");
			goto GET_BLOCK_TEST_FAIL;
		}

		pVirAddr = CVI_SYS_MmapCache(u64PhyAddr, blk_size[i]);
		if (pVirAddr == CVI_NULL) {
			UT_PRT("CVI_SYS_MmapCache NG.\n");
			goto GET_BLOCK_TEST_FAIL;
		}
		memset(pVirAddr, 0xcd, blk_size[i]);

		pu8TestBuf = malloc(blk_size[i]);
		vb_memcpy_test(pu8TestBuf, pVirAddr, blk_size[i], &duration1);
		vb_memcpy_test(pu8TestBuf, pVirAddr, blk_size[i], &duration2);
		if (memcmp(pu8TestBuf, pVirAddr, blk_size[i]) != 0) {
			UT_PRT("memcpy test fail\n");
			goto GET_BLOCK_TEST_FAIL;
		}
		free(pu8TestBuf);
		pu8TestBuf = NULL;
		UT_PRT("*vb%d memcpy(%d bytes)		%d - %d\n",
				  i, blk_size[i], duration1, duration2);

		if (CVI_SYS_Munmap(pVirAddr, blk_size[i]) != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_Munmap NG.\n");
			goto GET_BLOCK_TEST_FAIL;
		}
		CVI_VB_ReleaseBlock(blk);
	}
	return CVI_SUCCESS;

GET_BLOCK_TEST_FAIL:
	if (pu8TestBuf)
		free(pu8TestBuf);
	if (blk != VB_INVALID_HANDLE)
		CVI_VB_ReleaseBlock(blk);
	return CVI_FAILURE;
}

static CVI_S32 vb_ut_create_pool_test(CVI_VOID)
{
	VB_POOL_CONFIG_S cfg;
	VB_POOL pool = VB_INVALID_POOLID, tmpPool;
	VB_BLK blk = VB_INVALID_HANDLE;
	CVI_S32 s32Ret = CVI_SUCCESS;
	const CVI_U32 bufLen = 0x80000;
	CVI_VOID *pVirAddr = NULL;
	CVI_U8 *pu8TestBuf = NULL;
	CVI_U32 duration1, duration2;
	CVI_U64 u64PhyAddr;

	memset(&cfg, 0, sizeof(cfg));
	cfg.u32BlkSize = 0x100000;
	cfg.u32BlkCnt = 2;
	cfg.enRemapMode = VB_REMAP_MODE_CACHED;
	strncpy(cfg.acName, "vb_ut", sizeof(cfg.acName));
	pool = CVI_VB_CreatePool(&cfg);
	if (pool == VB_INVALID_POOLID) {
		UT_PRT("CVI_VB_CreatePool NG.\n");
		goto CREATE_POOL_TEST_FAIL;
	}
	UT_PRT("create pool:%d\n", pool);

	blk = CVI_VB_GetBlock(pool, bufLen);
	if (blk == VB_INVALID_HANDLE) {
		UT_PRT("CVI_VB_GetBlock NG\n");
		goto CREATE_POOL_TEST_FAIL;
	}
	CVI_VB_PrintPool(pool);

	tmpPool = CVI_VB_Handle2PoolId(blk);
	if (tmpPool != pool) {
		UT_PRT("get blk from unexpected pool(%d)\n", tmpPool);
		goto CREATE_POOL_TEST_FAIL;
	}

	u64PhyAddr = CVI_VB_Handle2PhysAddr(blk);
	if (!u64PhyAddr) {
		UT_PRT("CVI_VB_Handle2PhysAddr NG\n");
		goto CREATE_POOL_TEST_FAIL;
	}

	pVirAddr = CVI_SYS_MmapCache(u64PhyAddr, bufLen);
	if (pVirAddr == CVI_NULL) {
		UT_PRT("CVI_SYS_MmapCache NG.\n");
		goto CREATE_POOL_TEST_FAIL;
	}
	memset(pVirAddr, 0xab, bufLen);

	pu8TestBuf = malloc(bufLen);
	memset(pu8TestBuf, 0, bufLen);
	vb_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
	vb_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration2);
	if (memcmp(pu8TestBuf, pVirAddr, bufLen) != 0) {
		UT_PRT("memcpy test fail\n");
		goto CREATE_POOL_TEST_FAIL;
	}
	free(pu8TestBuf);
	UT_PRT("*map pool memcpy(%d bytes)  %d - %d\n", bufLen, duration1, duration2);

	CVI_SYS_Munmap(pVirAddr, bufLen);
	CVI_VB_ReleaseBlock(blk);

	s32Ret = CVI_VB_DestroyPool(pool);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_DestroyPool NG\n");
		goto CREATE_POOL_TEST_FAIL;
	}
	return CVI_SUCCESS;

CREATE_POOL_TEST_FAIL:
	if (pu8TestBuf)
		free(pu8TestBuf);

	if (pVirAddr)
		CVI_SYS_Munmap(pVirAddr, bufLen);
	if (blk != VB_INVALID_HANDLE)
		CVI_VB_ReleaseBlock(blk);
	if (pool != VB_INVALID_POOLID) {
		CVI_VB_DestroyPool(pool);
	}
	return CVI_FAILURE;
}

static CVI_S32 vb_ut_get_config_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S vb_config;
	CVI_U8 i;

	s32Ret = CVI_VB_GetConfig(&vb_config);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_GetConfig NG\n");
		return CVI_FAILURE;
	}

	// Compare vb config
	if (vb_config.u32MaxPoolCnt != stVbConf.u32MaxPoolCnt) {
		UT_PRT("u32MaxPoolCnt compare NG\n");
		return CVI_FAILURE;
	}
	for (i = 0; i < stVbConf.u32MaxPoolCnt; ++i) {
		if (vb_config.astCommPool[i].u32BlkSize != stVbConf.astCommPool[i].u32BlkSize
			|| vb_config.astCommPool[i].u32BlkCnt != stVbConf.astCommPool[i].u32BlkCnt
			|| vb_config.astCommPool[i].enRemapMode != stVbConf.astCommPool[i].enRemapMode) {
			UT_PRT("pool(%d) cfg compare NG\n", i);
			return CVI_FAILURE;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 vb_ut_get_blk_info_test(CVI_VOID)
{
	VB_POOL PoolId = 1, pool_tmp;
	VB_BLK blk = VB_INVALID_HANDLE, tmp_blk;
	CVI_S32 s32Ret = CVI_SUCCESS;
	const CVI_U32 bufLen = COMMON_POOL1_BLK_SIZE;
	CVI_U32 cnt = 0;
	CVI_U64 u64PhyAddr = 0;

	blk = CVI_VB_GetBlock(PoolId, bufLen);
	if (blk == VB_INVALID_HANDLE) {
		UT_PRT("CVI_VB_GetBlock NG\n");
		goto GET_BLK_INFO_TEST_FAIL;
	}

	s32Ret = CVI_VB_InquireUserCnt(blk, &cnt);
	if (s32Ret != CVI_SUCCESS || cnt != 1) {
		UT_PRT("CVI_VB_InquireUserCnt NG, cnt(%d), ret(%d)\n",
			cnt, s32Ret);
		goto GET_BLK_INFO_TEST_FAIL;
	}

	pool_tmp = CVI_VB_Handle2PoolId(blk);
	if (pool_tmp != PoolId) {
		UT_PRT("get blk from unexpected pool(%d)\n", pool_tmp);
		goto GET_BLK_INFO_TEST_FAIL;
	}

	u64PhyAddr = CVI_VB_Handle2PhysAddr(blk);
	if (!u64PhyAddr) {
		UT_PRT("CVI_VB_Handle2PhysAddr fail\n");
		goto GET_BLK_INFO_TEST_FAIL;
	}

	tmp_blk = CVI_VB_PhysAddr2Handle(u64PhyAddr);
	if (tmp_blk != blk) {
		UT_PRT("CVI_VB_PhysAddr2Handle fail\n");
		goto GET_BLK_INFO_TEST_FAIL;
	}

	CVI_VB_ReleaseBlock(blk);
	s32Ret = CVI_VB_InquireUserCnt(blk, &cnt);
	if (s32Ret != CVI_SUCCESS || cnt != 0) {
		UT_PRT("CVI_VB_InquireUserCnt NG, cnt(%d), ret(%d)\n",
			cnt, s32Ret);
		goto GET_BLK_INFO_TEST_FAIL;
	}
	return CVI_SUCCESS;

GET_BLK_INFO_TEST_FAIL:
	if (blk != VB_INVALID_HANDLE)
		CVI_VB_ReleaseBlock(blk);
	return CVI_FAILURE;
}

static CVI_S32 vb_ut_fmt_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CAL_CONFIG_S stVbCalConfig;

	VB_CAL_CONFIG_S stVbCalConfig_YUV420_1920x1080 = {
		.u32VBSize = 3110400,
		.u32MainStride = 1920,
		.u32CStride = 960,
		.u32MainSize = 3110400,
		.u32MainYSize = 2073600,
		.u32MainCSize = 518400,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUV422_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 1920,
		.u32CStride = 960,
		.u32MainSize = 4147200,
		.u32MainYSize = 2073600,
		.u32MainCSize = 1036800,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_NV21_1920x1080 = {
		.u32VBSize = 3110400,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 3110400,
		.u32MainYSize = 2073600,
		.u32MainCSize = 1036800,
		.u16AddrAlign = 64,
		.plane_num = 2,
	};
	VB_CAL_CONFIG_S stVbCalConfig_NV16_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 4147200,
		.u32MainYSize = 2073600,
		.u32MainCSize = 2073600,
		.u16AddrAlign = 64,
		.plane_num = 2,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUYV_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 3840,
		.u32CStride = 0,
		.u32MainSize = 4147200,
		.u32MainYSize = 4147200,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_RGB_1920x1080 = {
		.u32VBSize = 6220800,
		.u32MainStride = 5760,
		.u32CStride = 0,
		.u32MainSize = 6220800,
		.u32MainYSize = 6220800,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_BGRP_1920x1080 = {
		.u32VBSize = 6220800,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 6220800,
		.u32MainYSize = 2073600,
		.u32MainCSize = 2073600,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUV420_800x600 = {
		.u32VBSize = 806400,
		.u32MainStride = 896,
		.u32CStride = 448,
		.u32MainSize = 806400,
		.u32MainYSize = 537600,
		.u32MainCSize = 134400,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_HSVP_1100x400 = {
		.u32VBSize = 1382400,
		.u32MainStride = 1152,
		.u32CStride = 1152,
		.u32MainSize = 1382400,
		.u32MainYSize = 460800,
		.u32MainCSize = 460800,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_ARGB1555_400x300 = {
		.u32VBSize = 249600,
		.u32MainStride = 832,
		.u32CStride = 0,
		.u32MainSize = 249600,
		.u32MainYSize = 249600,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_ARGB4444_200x200 = {
		.u32VBSize = 89600,
		.u32MainStride = 448,
		.u32CStride = 0,
		.u32MainSize = 89600,
		.u32MainYSize = 89600,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_YUV420_1920x1080)) {
		UT_PRT("YUV420 1920x1080 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_YUV420_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_YUV_PLANAR_422, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_YUV422_1920x1080)) {
		UT_PRT("YUV422 1920x1080 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_YUV422_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_NV21_1920x1080)) {
		UT_PRT("NV21 1920x1080 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_NV21_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_NV16, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_NV16_1920x1080)) {
		UT_PRT("NV16 1920x1080 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_NV16_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_YUYV, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_YUYV_1920x1080)) {
		UT_PRT("YUYV 1920x1080 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_YUYV_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_RGB_888, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_RGB_1920x1080)) {
		UT_PRT("BGR 1920x1080 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_RGB_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_BGR_888_PLANAR, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_BGRP_1920x1080)) {
		UT_PRT("BGR_Planar 1920x1080 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_BGRP_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(800, 600, PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_YUV420_800x600)) {
		UT_PRT("YUV420 800x600 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_YUV420_800x600);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1100, 400, PIXEL_FORMAT_HSV_888_PLANAR, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_HSVP_1100x400)) {
		UT_PRT("HSV Planar 1100x400 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_HSVP_1100x400);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(400, 300, PIXEL_FORMAT_ARGB_1555, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_ARGB1555_400x300)) {
		UT_PRT("ARGB1555 400x300 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_ARGB1555_400x300);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(200, 200, PIXEL_FORMAT_ARGB_4444, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (vb_test_compare_config(&stVbCalConfig, &stVbCalConfig_ARGB4444_200x200)) {
		UT_PRT("ARGB4444_00x200 vb config NG:\n");
		vb_test_print_config(&stVbCalConfig);
		UT_PRT("Expected config values:\n");
		vb_test_print_config(&stVbCalConfig_ARGB4444_200x200);
		s32Ret = CVI_FAILURE;
	}

	return s32Ret;
}

static CVI_S32 vb_ut_create_ex_pool_test(CVI_VOID)
{
	CVI_U32 u32BlkSize;
	CVI_U32 u32RotBlkSize;
	VB_BLK blk = VB_INVALID_HANDLE;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U64 u64PhyAddr = 0;
	CVI_VOID *pVirAddr = NULL;
	VB_POOL_CONFIG_EX_S stExconfig;
	CVI_S32 whproduct;
	VB_POOL pool = VB_INVALID_POOLID, tmpPool;

	u32BlkSize = COMMON_GetPicBufferSize(384, 256,
					PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32RotBlkSize = COMMON_GetPicBufferSize(256, 384,
					PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize	= u32BlkSize > u32RotBlkSize ? u32BlkSize : u32RotBlkSize;

	CVI_SYS_IonAlloc(&u64PhyAddr, &pVirAddr,
					"vb_memory", 2 * u32BlkSize);

	memset(&stExconfig, 0, sizeof(stExconfig));
	stExconfig.u32BlkCnt =  1;
	whproduct = 384 * 256;
	stExconfig.astUserBlk[0].au64PhyAddr[0] = u64PhyAddr; //stride 0
	stExconfig.astUserBlk[0].au64PhyAddr[1] = u64PhyAddr + (2 * whproduct);//stride 1
	stExconfig.astUserBlk[0].au64PhyAddr[2] = u64PhyAddr + (2 * whproduct) + (whproduct / 4);//stride 1

	pool = CVI_VB_CreateExPool(&stExconfig);
	if (pool == VB_INVALID_POOLID) {
		UT_PRT("CVI_VB_CreateExPool NG.\n");
		return CVI_FAILURE;
	}
	UT_PRT("create ex pool:%d\n", pool);

	blk = CVI_VB_GetBlock(pool, u32BlkSize);
	if (blk == VB_INVALID_HANDLE) {
		UT_PRT("CVI_VB_GetBlock NG\n");
		goto CREATE_EX_POOL_TEST_FAIL;
	}
	CVI_VB_PrintPool(pool);

	tmpPool = CVI_VB_Handle2PoolId(blk);
	if (tmpPool != pool) {
		UT_PRT("get blk from unexpected pool(%d)\n", tmpPool);
		goto CREATE_EX_POOL_TEST_FAIL;
	}
	s32Ret = CVI_VB_ReleaseBlock(blk);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_DestroyPool NG\n");
		goto CREATE_EX_POOL_TEST_FAIL;
	}
	CVI_VB_PrintPool(pool);

	s32Ret = CVI_VB_DestroyPool(pool);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_DestroyPool NG\n");
		goto CREATE_EX_POOL_TEST_FAIL;
	}

	s32Ret = CVI_SYS_IonFree(u64PhyAddr, pVirAddr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonFree NG\n");
		goto CREATE_EX_POOL_TEST_FAIL;
	}

	return CVI_SUCCESS;

CREATE_EX_POOL_TEST_FAIL:
	if (blk != VB_INVALID_HANDLE)
		CVI_VB_ReleaseBlock(blk);

	if (pool != VB_INVALID_POOLID)
		CVI_VB_DestroyPool(pool);

	if (u64PhyAddr && pVirAddr)
		CVI_SYS_IonFree(u64PhyAddr, pVirAddr);

	return CVI_FAILURE;
}

static CVI_S32 vb_auto_test()
{
	CVI_S32 s32Ret = CVI_SUCCESS;
#ifndef CONFIG_DUAL_OS
	s32Ret |= vb_ut_mmap_pool_test();
#endif
	s32Ret |= vb_ut_get_block_test();
	s32Ret |= vb_ut_create_pool_test();
	s32Ret |= vb_ut_get_config_test();
	s32Ret |= vb_ut_get_blk_info_test();
	s32Ret |= vb_ut_fmt_test();
	s32Ret |= vb_ut_create_ex_pool_test();
	return s32Ret;
}

static CVI_S32 vb_ut_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case VB_TEST_MMAP_POOL: {
#if defined(CONFIG_DUAL_OS)
		s32Ret = CVI_SUCCESS;
#else
		s32Ret = vb_ut_mmap_pool_test();
#endif
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_ut_mmap_pool_test success\n");
		break;
	}
	case VB_TEST_GET_BLOCK: {
		s32Ret = vb_ut_get_block_test();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_ut_get_block_test success\n");
		break;
	}
	case VB_TEST_CREATE_POOL: {
		s32Ret = vb_ut_create_pool_test();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_ut_create_pool_test success\n");
		break;
	}
	case VB_TEST_GET_VB_CFG: {
		s32Ret = vb_ut_get_config_test();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_ut_get_config_test success\n");
		break;
	}
	case VB_TEST_GET_BLK_INFO: {
		s32Ret = vb_ut_get_blk_info_test();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_ut_get_blk_info_test success\n");
		break;
	}
	case VB_TEST_VB_BUF_SIZE: {
		s32Ret = vb_ut_fmt_test();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_ut_fmt_test success\n");
		break;
	}
	case VB_TEST_CREATE_EX_POOL: {
		s32Ret = vb_ut_create_ex_pool_test();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_ut_create_ex_pool_test success\n");
		break;
	}
	case VB_TEST_AUTO: {
		s32Ret = vb_auto_test();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("vb_auto_test success\n");
		break;
	}
	default:
		break;
	}

	return s32Ret;
}

static CVI_VOID vb_ut_handleSig(CVI_S32 nSignal, siginfo_t *si, CVI_VOID *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	CVI_VB_Exit();
	CVI_SYS_Exit();

	exit(1);
}

static CVI_S32 vb_ut_init(CVI_VOID)
{
	CVI_S32 s32Ret;
	struct sigaction sa = {};

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = vb_ut_handleSig;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND;	// Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	CVI_VB_Exit();
	CVI_SYS_Exit();

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 2;
	stVbConf.astCommPool[0].u32BlkSize = COMMON_POOL0_BLK_SIZE;
	stVbConf.astCommPool[0].u32BlkCnt = COMMON_POOL0_BLK_CNT;
	UT_PRT("common pool[0] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL0_BLK_SIZE, COMMON_POOL0_BLK_CNT);
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize = COMMON_POOL1_BLK_SIZE;
	stVbConf.astCommPool[1].u32BlkCnt = COMMON_POOL1_BLK_CNT;
	UT_PRT("common pool[1] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL1_BLK_SIZE, COMMON_POOL1_BLK_CNT);
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 vb_ut_deinit(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_VB_Exit();
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_VOID vb_show_help(CVI_VOID)
{
	UT_PRT("%4d: Mmap pool test\n", VB_TEST_MMAP_POOL);
	UT_PRT("%4d: Get block test\n", VB_TEST_GET_BLOCK);
	UT_PRT("%4d: Create pool test\n", VB_TEST_CREATE_POOL);
	UT_PRT("%4d: Get vb config test\n", VB_TEST_GET_VB_CFG);
	UT_PRT("%4d: Get blk info test\n", VB_TEST_GET_BLK_INFO);
	UT_PRT("%4d: vb buf size test\n", VB_TEST_VB_BUF_SIZE);
	UT_PRT("%4d: Create ex pool test\n", VB_TEST_CREATE_EX_POOL);
	UT_PRT("%4d: auto test\n", VB_TEST_AUTO);
	UT_PRT("255: exit vb ut\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 op;
	CVI_S32 s32Ret;

	s32Ret = vb_ut_init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vb_ut_init fail, ret=%d\n", s32Ret);
		return -1;
	}

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = vb_ut_handle_op(op);
		UT_PRT("vb ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		system("stty erase ^H");
		do {
			vb_show_help();
			scanf("%d", &op);
			s32Ret = vb_ut_handle_op(op);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
		} while (op != 255);
	}

	vb_ut_deinit();
	return s32Ret;
}
