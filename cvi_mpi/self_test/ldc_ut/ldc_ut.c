#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <pthread.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_gdc.h"

#include "ldc_ut_comm.h"

//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB

#define GDC_FILE_IN_ROT                         "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_ROT0                       "res/ldc/output/1920x1080_rot0.yuv"
#define GDC_FILE_OUT_ROT90                      "res/ldc/output/1920x1080_rot90.yuv"
#define GDC_FILE_OUT_ROT270                     "res/ldc/output/1920x1080_rot270.yuv"
#define GDC_MD5_PEF_ROT0                        "d5e0df06d3021b863bcacbf4d9e033de"
#define GDC_MD5_PEF_ROT90                       "4109aa62845f228f3245ff6671c3f339"
#define GDC_MD5_PEF_ROT270                      "f068fe7ae591470dad5a795eacdd0061"
#define GDC_FILE_IN_ROT_4M                      "res/ldc/input/2560x1440.yuv"
#define GDC_FILE_IN_ROT_5M                      "res/ldc/input/2880x1620.yuv"
#define GDC_FILE_IN_ROT_8M                      "res/ldc/input/3840x2160.yuv"
#define GDC_FILE_OUT_ROT0_4M                    "res/ldc/output/2560x1440_rot0.yuv"
#define GDC_FILE_OUT_ROT90_5M                   "res/ldc/output/2880x1620_rot90.yuv"
#define GDC_FILE_OUT_ROT270_8M                  "res/ldc/output/3840x2160_rot270.yuv"
#define GDC_MD5_PEF_ROT0_4M                     "1b24c960025f17a726b8812c099c1ee8"
#define GDC_MD5_PEF_ROT90_5M                    "37a7dd5a4f14c0b26bafb2d2c481b5ec"
#define GDC_MD5_PEF_ROT270_8M                   "7b5dbcb0a495f480b6295bea4e690e63"

#define GDC_FILE_IN_ROT_1                         "res/ldc/input/128x128.yuv"
#define GDC_FILE_OUT_ROT0_1                       "res/ldc/output/128x128_rot0.yuv"
#define GDC_MD5_PEF_ROT0_1                        "dda7dac214c83d6f3a6795858b1adfe4"
#define GDC_FILE_OUT_ROT90_1                      "res/ldc/output/128x128_rot90.yuv"
#define GDC_MD5_PEF_ROT90_1                       "4fbd90df5370968e0cec1693f992395f"
#define GDC_FILE_OUT_ROT270_1                     "res/ldc/output/128x128_rot270.yuv"
#define GDC_MD5_PEF_ROT270_1                      "f14498fa55a3cdacde33c6280b55bc64"

#define GDC_FILE_IN_ROT_2                         "res/ldc/input/4096x4096.yuv"
#define GDC_FILE_OUT_ROT0_2                       "res/ldc/output/4096x4096_rot0.yuv"
#define GDC_MD5_PEF_ROT0_2                        "d20a34f92ca036c9b8019e664bfb7aaf"
#define GDC_FILE_OUT_ROT90_2                      "res/ldc/output/4096x4096_rot90.yuv"
#define GDC_MD5_PEF_ROT90_2                       "d422e3d8b7d7bea7b93ee4c12c0e3e8c"
#define GDC_FILE_OUT_ROT270_2                     "res/ldc/output/4096x4096_rot270.yuv"
#define GDC_MD5_PEF_ROT270_2                      "8d947e1a073a6ba98a3c8cc3fd41ff13"
#define GDC_FILE_OUT_ROT_XYFLIP_2                 "res/ldc/output/4096x4096_rot_xy_flip.yuv"
#define GDC_MD5_PEF_ROT_XYFLIP_2                  "b4a9ca3d57bc8ca99d33daed37b85ecb"

#define GDC_FILE_IN_LDC_BARREL_0P3              "res/ldc/input/1920x1080_barrel_0.3.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_0           "res/ldc/output/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_1           "res/ldc/output/1920x1080_barrel_0.3_r50_ofst_0_0_d-200.yuv"
#define GDC_MD5_PEF_LDC_BARREL_0P3_0            "9858d320f4ae52cade10d20ea61473c4"
#define GDC_MD5_PEF_LDC_BARREL_0P3_1            "87216f0d34af9cf54507ca6b9d892906"
#define GDC_FILE_IN_LDC_PINCUSHION_0P3          "res/ldc/input/1920x1080_pincushion_0.3.yuv"
#define GDC_FILE_OUT_LDC_PINCUSHION_0P3_0       "res/ldc/output/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.yuv"
#define GDC_FILE_OUT_LDC_PINCUSHION_0P3_1       "res/ldc/output/1920x1080_pincushion_0.3_r50_ofst_0_0_d400.yuv"
#define GDC_MD5_PEF_LDC_PINCUSHION_0P3_0        "78acfb91bd08f159623a85fe03a01169"
#define GDC_MD5_PEF_LDC_PINCUSHION_0P3_1        "efa1c0738db28f8864b6181dedcdba60"

#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_0       "res/ldc/input/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.mesh"
#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_1       "res/ldc/input/1920x1080_barrel_0.3_r50_ofst_0_0_d-200.mesh"
#define GDC_FILE_IN_LDC_PINCUSHION_0P3_MESH_0   "res/ldc/input/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.mesh"
#define GDC_FILE_IN_LDC_PINCUSHION_0P3_MESH_1   "res/ldc/input/1920x1080_pincushion_0.3_r50_ofst_0_0_d400.mesh"

#define GDC_FILE_IN_LDC_BARREL_0P3_4M           "res/ldc/input/2560x1440_barrel_0.3.yuv"
#define GDC_FILE_IN_LDC_BARREL_0P3_5M           "res/ldc/input/2880x1620_barrel_0.3.yuv"
#define GDC_FILE_IN_LDC_BARREL_0P3_8M           "res/ldc/input/3840x2160_barrel_0.3.yuv"
#define GDC_FILE_IN_LDC_BARREL_0P3_MAX          "res/ldc/input/4096x4096_barrel_0.3.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_4M          "res/ldc/output/2560x1440_barrel_0.3_r100_ofst_0_0_d-300.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_5M          "res/ldc/output/2880x1620_barrel_0.3_r50_ofst_0_0_d-200.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_8M          "res/ldc/output/3840x2160_barrel_0.3_r50_ofst_0_0_d-100.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_MAX         "res/ldc/output/4096x4096_barrel_0.3_r100_ofst_0_0_d-300.yuv"
#define GDC_MD5_PEF_LDC_BARREL_0P3_4M       "b0104f23e3f758e46ffbad77990319b7"
#define GDC_MD5_PEF_LDC_BARREL_0P3_5M       "4649aafb33d16543b804cfa2765fbf74"
#define GDC_MD5_PEF_LDC_BARREL_0P3_8M       "6bd0f1a997070027f97730c347dbf5c6"
#define GDC_MD5_PEF_LDC_BARREL_0P3_MAX      "c07b34fbbf319b06f93879825a829f90"

#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_4M      "res/ldc/input/2560x1440_barrel_0.3_r100_ofst_0_0_d-300.mesh"
#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_5M      "res/ldc/input/2880x1620_barrel_0.3_r50_ofst_0_0_d-200.mesh"
#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_8M      "res/ldc/input/3840x2160_barrel_0.3_r50_ofst_0_0_d-100.mesh"
#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_MAX     "res/ldc/input/4096x4096_barrel_0.3_r100_ofst_0_0_d-300.mesh"

#define GDC_FILE_IN_FMT_0                      "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_FMT_0                     "res/ldc/output/1920x1080.yuv"
#define GDC_MD5_PEF_FMT_0                      "d5e0df06d3021b863bcacbf4d9e033de"
#define GDC_FILE_IN_FMT_1                      "res/ldc/input/1920x1080_yonly.yuv"
#define GDC_FILE_OUT_FMT_1                     "res/ldc/output/1920x1080_yonly.yuv"
#define GDC_MD5_PEF_FMT_1                      "79c3833d44605f4993201b062ec1c42c"

#define GDC_FILE_IN_NOT_ALIGN                  "res/ldc/input/666x666_yuv400.yuv"
#define GDC_FILE_OUT_NOT_ALIGN                 "res/ldc/output/666x666_yuv400.yuv"

#define GDC_FILE_IN_CMDQ                       "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ                      "res/ldc/output/1920x1080_cmdq.yuv"
#define GDC_MD5_PEF_CMDQ                       "9a14e17162de418ac3e84b6fb5197a2b"
#define GDC_FILE_IN_CMDQ_1TO2                  "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ_1TO2_0               "res/ldc/output/1920x1080_cmdq_1to2_0.yuv"
#define GDC_MD5_PEF_CMDQ_1TO2_0                "d5e0df06d3021b863bcacbf4d9e033de"
#define GDC_FILE_IN_CMDQ_1TO2_MAX              "res/ldc/input/4096x4096.yuv"
#define GDC_FILE_OUT_CMDQ_1TO2_0_MAX           "res/ldc/output/4096x4096_cmdq_1to2_0.yuv"
#define GDC_MD5_PEF_CMDQ_1TO2_0_MAX            "d20a34f92ca036c9b8019e664bfb7aaf"

#define GDC_FILE_IN_LDC_GRID_INFO              "res/ldc/input/1280x768.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO             "res/ldc/output/1280x768_grid_info.yuv"
#define GDC_FILE_IN_LDC_GRID                   "res/ldc/input/grid_info_79_44_3476_80_45_1280x720.dat"

#define GDC_MAX_W    4096
#define GDC_MAX_H    4096
#define GDC_MIN_W    64
#define GDC_MIN_H    64

#ifndef FPGA_PORTING
#define GDC_REPEAT_TIMES 6
#else
#define GDC_REPEAT_TIMES 2
#endif

typedef CVI_S32 (*p_func)(void);

#define GDC_DEFAULT_PIXEL_FMT PIXEL_FORMAT_NV21
#define MAX_FUNC_CNT 100

static CVI_BOOL b_gdc_save_file;
static CVI_BOOL needSuspend;

typedef enum _GDC_TEST_OP {
	GDC_TEST_ROT = 0,
	GDC_TEST_ROT1,
	GDC_TEST_ROT2,
	GDC_TEST_LDC,
	GDC_TEST_LDC_LOAD_MESH,
	GDC_TEST_MAX_SIZE,
	GDC_TEST_FMT,
	GDC_TEST_SIZE_NO_ALIGN,
	GDC_TEST_CMDQ,
	GDC_TEST_CMDQ_1TO2,
	GDC_TEST_CMDQ_1TO2_MAX,
	GDC_TEST_ONLINE,
	GDC_TEST_ASYNC,
	GDC_TEST_MULTI_THREAD,
	GDC_TEST_PEF,
	GDC_TEST_LOAD_GRID_INFO,
	GDC_TEST_RST,
	GDC_TEST_SUSPEND,
	GDC_TEST_RESUME,
	GDC_TEST_RUN_SUSPEND,
	GDC_TEST_PRESURE_SIZE_FOR_EACH = 98,
	GDC_TEST_AUTO_REGRESSION = 99,
	GDC_TEST_USER_CONFIG = 100,
} GDC_TEST_OP;

typedef struct _GDC_BASIC_TEST_PARAM {
	SIZE_S size_in;
	SIZE_S size_out;
	CVI_CHAR filename_in[128];
	CVI_CHAR filename_out[128];
	CVI_CHAR aszMD5Sum[33];
	ROTATION_E enRotation;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	VB_BLK inBlk, outBlk;
	PIXEL_FORMAT_E enPixelFormat;
	GDC_HANDLE hHandle;
	GDC_TASK_ATTR_S stTask;
	GDC_IDENTITY_ATTR_S identity;
	GDC_TEST_OP op;
} GDC_BASIC_TEST_PARAM;

void gdc_ut_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_VB_Exit();
		CVI_SYS_Exit();
		UT_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

static CVI_S32 gdc_basic_add_tsk(GDC_BASIC_TEST_PARAM *param, void *ptr)
{
	LDC_ATTR_S *LDCAttr;
	ROTATION_E enRotation;
	CVI_CHAR *meshName;
	CVI_U64 u64PhyAddr;
	CVI_VOID *pVirAddr;

	CVI_S32 s32Ret = CVI_FAILURE;

	if (!param) {
		UT_PRT("gdc_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	switch (param->op) {
	case GDC_TEST_ROT:
		enRotation = (ROTATION_E)(uintptr_t)ptr;

		s32Ret = CVI_GDC_AddRotationTask(param->hHandle, &param->stTask, enRotation);
		if (s32Ret) {
			UT_PRT("CVI_GDC_AddRotationTask failed!\n");
		}
		break;
	case GDC_TEST_LDC:
		LDCAttr = (LDC_ATTR_S *)ptr;

		s32Ret = CVI_GDC_GenLDCMesh(param->size_in.u32Width, param->size_in.u32Height, LDCAttr
			, param->stTask.name, &u64PhyAddr, &pVirAddr);
		if (s32Ret) {
			UT_PRT("gen LDC mesh(%s) fail\n", param->stTask.name);
			break;
		}

		param->stTask.au64privateData[0] = u64PhyAddr;
		s32Ret = CVI_GDC_AddLDCTask(param->hHandle, &param->stTask, LDCAttr, param->enRotation);
		if (s32Ret) {
			UT_PRT("CVI_GDC_AddLDCTask failed!\n");
		}
		break;
	case GDC_TEST_LDC_LOAD_MESH:
		meshName = (CVI_CHAR *)ptr;
		LDC_ATTR_S stLDCAttr = {0};

		s32Ret = CVI_GDC_LoadLDCMesh(param->size_out.u32Width, param->size_out.u32Height
			, meshName, param->stTask.name, &u64PhyAddr, &pVirAddr);
		if (s32Ret) {
			UT_PRT("gen LDC mesh(%s) fail for tsk(%s)\n", meshName, param->stTask.name);
			break;
		}

		param->stTask.au64privateData[0] = u64PhyAddr;
		s32Ret = CVI_GDC_AddLDCTask(param->hHandle, &param->stTask, &stLDCAttr, ROTATION_0);
		if (s32Ret) {
			UT_PRT("CVI_GDC_AddLDCTask failed!\n");
		}
		break;
	default:
		UT_PRT("not allow this op(%d) fail\n", param->op);
		break;
	}

	return s32Ret;
}

static CVI_S32 gdc_basic(GDC_BASIC_TEST_PARAM *param, void *ptr)
{
	CVI_S32 times = GDC_REPEAT_TIMES;
	VB_CONFIG_S stVbConf;
	CVI_S32 s32Ret;
	CVI_U32 BlkSize;
	GDC_HANDLE hHandle = 0;

	if (!param) {
		UT_PRT("gdc_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	param->u32BlkSizeIn = COMMON_GetPicBufferSize(ALIGN(param->size_in.u32Width, DEFAULT_ALIGN)
		, ALIGN(param->size_in.u32Height, DEFAULT_ALIGN), param->enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	param->u32BlkSizeOut = COMMON_GetPicBufferSize(ALIGN(param->size_out.u32Width, DEFAULT_ALIGN)
		, ALIGN(param->size_out.u32Height, DEFAULT_ALIGN), param->enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	BlkSize = (param->u32BlkSizeIn > param->u32BlkSizeOut) ? param->u32BlkSizeIn : param->u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_NOCACHE;

	UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}
	/************************************************
	 * step2:  Init GDC
	 ************************************************/
	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param->size_in, param->enPixelFormat,
			param->filename_in, &param->stVideoFrameIn);
		if (s32Ret) {
			UT_PRT("GDCFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
		if (s32Ret) {
			UT_PRT("GDC_COMM_PrepareFrame failed!\n");
			goto exit2;
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param->hHandle);
		if (s32Ret) {
			UT_PRT("CVI_GDC_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = gdc_basic_add_tsk(param, ptr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_GDC_EndJob(param->hHandle);
		if (s32Ret) {
			UT_PRT("CVI_GDC_EndJob failed!\n");
			goto exit2;
		}

		UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[1], param->stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[1], param->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		s32Ret = CVI_GDC_GetWorkJob(&hHandle);
		if (s32Ret) {
			UT_PRT("CVI_GDC_GetWorkJob failed!\n");
			goto exit2;
		}

		if (param->aszMD5Sum[0]) {
			if (CompareWithMD5(param->aszMD5Sum, &param->stVideoFrameOut)) {
				b_gdc_save_file = CVI_TRUE;
				s32Ret = CVI_FAILURE;
				UT_PRT("Compare MD5 fail, MD5:%s\n", param->aszMD5Sum);
			} else {
				b_gdc_save_file = CVI_FALSE;
			}
		} else {
			b_gdc_save_file = CVI_TRUE;
		}

		if (b_gdc_save_file) {
			if (FrameFullSaveToFile(param->filename_out, &param->stVideoFrameOut) != CVI_SUCCESS) {
				UT_PRT("FrameFullSaveToFile. s32Ret: 0x%x !\n", s32Ret);
			}
			UT_PRT("-------------------times:(%d)----------------------\n", times);
			UT_PRT("output file:%s\n", param->filename_out);
			goto exit2;
		}
		param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param->inBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param->inBlk);
			param->inBlk = VB_INVALID_HANDLE;
		}

		param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param->outBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param->outBlk);
			param->outBlk = VB_INVALID_HANDLE;
		}

		if (s32Ret) {
			UT_PRT("release VB fail.\n");
			goto exit2;
		}
	} while (times--);

exit2:
	if (s32Ret)
		if (param->hHandle)
			s32Ret |= CVI_GDC_CancelJob(param->hHandle);
	CVI_GDC_FreeCurTaskMesh(param->stTask.name);
	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		UT_PRT("CVI_GDC_DeInit fail.\n");
	}
	param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
	if (param->inBlk != VB_INVALID_HANDLE) {
		s32Ret |= CVI_VB_ReleaseBlock(param->inBlk);
		param->inBlk = VB_INVALID_HANDLE;
	}
	param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
	if (param->outBlk != VB_INVALID_HANDLE) {
		s32Ret |= CVI_VB_ReleaseBlock(param->outBlk);
		param->outBlk = VB_INVALID_HANDLE;
	}
exit1:
	s32Ret |= CVI_VB_Exit();
exit0:
	s32Ret |= CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 gdc_test_rot(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[3] = {ROTATION_0, ROTATION_90, ROTATION_270};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 3;
	CVI_CHAR *filename_in[3] = {GDC_FILE_IN_ROT, GDC_FILE_IN_ROT, GDC_FILE_IN_ROT};
	CVI_CHAR *filename_out[3] = {GDC_FILE_OUT_ROT0, GDC_FILE_OUT_ROT90, GDC_FILE_OUT_ROT270};
	CVI_CHAR *aszMD5Sum[3] = {GDC_MD5_PEF_ROT0, GDC_MD5_PEF_ROT90, GDC_MD5_PEF_ROT270};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.aszMD5Sum, aszMD5Sum[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		if (i) {
			param.size_out.u32Width = 1088;
			param.size_out.u32Height = 1920;
		} else {
			param.size_out.u32Width = 1920;
			param.size_out.u32Height = 1088;
		}
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_ROT;

		s32Ret = gdc_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_rot_scaling(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[3] = {ROTATION_0, ROTATION_90, ROTATION_270};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 3;
	CVI_CHAR *filename_in[3] = {GDC_FILE_IN_ROT_4M, GDC_FILE_IN_ROT_5M, GDC_FILE_IN_ROT_8M};
	CVI_CHAR *filename_out[3] = {GDC_FILE_OUT_ROT0_4M, GDC_FILE_OUT_ROT90_5M, GDC_FILE_OUT_ROT270_8M};
	CVI_CHAR *aszMD5Sum[3] = {GDC_MD5_PEF_ROT0_4M, GDC_MD5_PEF_ROT90_5M, GDC_MD5_PEF_ROT270_8M};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.aszMD5Sum, aszMD5Sum[i]);
		if (i == 0) {
			param.size_in.u32Width = 2560;
			param.size_in.u32Height = 1440;
			param.size_out.u32Width = 2560;
			param.size_out.u32Height = 1472;
		} else if (i == 1) {
			param.size_in.u32Width = 2880;
			param.size_in.u32Height = 1620;
			param.size_out.u32Width = 1664;
			param.size_out.u32Height = 2880;
		} else {
			param.size_in.u32Width = 3840;
			param.size_in.u32Height = 2160;
			param.size_out.u32Width = 2176;
			param.size_out.u32Height = 3840;
		}

		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_4m_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_4m_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_ROT;

		s32Ret = gdc_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_rot_small(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[3] = {ROTATION_0, ROTATION_90, ROTATION_270};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 3;
	CVI_CHAR *filename_in[3] = {GDC_FILE_IN_ROT_1, GDC_FILE_IN_ROT_1, GDC_FILE_IN_ROT_1};
	CVI_CHAR *filename_out[3] = {GDC_FILE_OUT_ROT0_1, GDC_FILE_OUT_ROT90_1, GDC_FILE_OUT_ROT270_1};
	CVI_CHAR *aszMD5Sum[3] = {GDC_MD5_PEF_ROT0_1, GDC_MD5_PEF_ROT90_1, GDC_MD5_PEF_ROT270_1};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.aszMD5Sum, aszMD5Sum[i]);
		param.size_in.u32Width = 128;
		param.size_in.u32Height = 128;
		param.size_out.u32Width = 128;
		param.size_out.u32Height = 128;

		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_1_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_1_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_ROT;

		s32Ret = gdc_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_ldc(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr[5] = {
		{CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 50, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 0, 0, 0, 400, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 50, 0, 0, 400, {0}, 0, 0},
	};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 4;
	CVI_CHAR *filename_in[4] = {
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_PINCUSHION_0P3,
		GDC_FILE_IN_LDC_PINCUSHION_0P3,
	};
	CVI_CHAR *filename_out[4] = {
		GDC_FILE_OUT_LDC_BARREL_0P3_0,
		GDC_FILE_OUT_LDC_BARREL_0P3_1,
		GDC_FILE_OUT_LDC_PINCUSHION_0P3_0,
		GDC_FILE_OUT_LDC_PINCUSHION_0P3_1,
	};
	CVI_CHAR *aszMD5Sum[4] = {
		GDC_MD5_PEF_LDC_BARREL_0P3_0,
		GDC_MD5_PEF_LDC_BARREL_0P3_1,
		GDC_MD5_PEF_LDC_PINCUSHION_0P3_0,
		GDC_MD5_PEF_LDC_PINCUSHION_0P3_1,
	};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.aszMD5Sum, aszMD5Sum[i]);

		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;

		param.size_out.u32Width = 1920;
		param.size_out.u32Height = ALIGN(1080, DEFAULT_ALIGN);

		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_LDC;

		s32Ret = gdc_basic(&param, (void *)&stLdcAttr[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_ldc_load_mesh_scaling(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR *mesh_file_name[5] = {
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_0,
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_4M,
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_5M,
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_8M,
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_MAX,
	};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 5;
	CVI_CHAR *filename_in[5] = {
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_BARREL_0P3_4M,
		GDC_FILE_IN_LDC_BARREL_0P3_5M,
		GDC_FILE_IN_LDC_BARREL_0P3_8M,
		GDC_FILE_IN_LDC_BARREL_0P3_MAX,
	};
	CVI_CHAR *filename_out[5] = {
		GDC_FILE_OUT_LDC_BARREL_0P3_0,
		GDC_FILE_OUT_LDC_BARREL_0P3_4M,
		GDC_FILE_OUT_LDC_BARREL_0P3_5M,
		GDC_FILE_OUT_LDC_BARREL_0P3_8M,
		GDC_FILE_OUT_LDC_BARREL_0P3_MAX,
	};
	CVI_CHAR *aszMD5Sum[5] = {
		GDC_MD5_PEF_LDC_BARREL_0P3_0,
		GDC_MD5_PEF_LDC_BARREL_0P3_4M,
		GDC_MD5_PEF_LDC_BARREL_0P3_5M,
		GDC_MD5_PEF_LDC_BARREL_0P3_8M,
		GDC_MD5_PEF_LDC_BARREL_0P3_MAX,
	};

	for (CVI_U8 i = 1; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.aszMD5Sum, aszMD5Sum[i]);

		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		param.size_out.u32Width = 1920;
		param.size_out.u32Height = ALIGN(1080, DEFAULT_ALIGN);

		if (i == 0) {
			param.size_in.u32Width = 1920;
			param.size_in.u32Height = 1080;
			param.size_out.u32Width = 1920;
			param.size_out.u32Height = 1088;
		} else if (i == 1) {
			param.size_in.u32Width = 2560;
			param.size_in.u32Height = 1440;
			param.size_out.u32Width = 2560;
			param.size_out.u32Height = 1472;
		} else if (i == 2) {
			param.size_in.u32Width = 2880;
			param.size_in.u32Height = 1620;
			param.size_out.u32Width = 2880;
			param.size_out.u32Height = 1664;
		} else if (i == 3){
			param.size_in.u32Width = 3840;
			param.size_in.u32Height = 2160;
			param.size_out.u32Width = 3840;
			param.size_out.u32Height = 2176;
		} else {
			param.size_in.u32Width = 4096;
			param.size_in.u32Height = 4096;
			param.size_out.u32Width = 4096;
			param.size_out.u32Height = 4096;
		}
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_LDC_LOAD_MESH;

		s32Ret = gdc_basic(&param, (void *)mesh_file_name[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_rot_maxsize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[4] = {ROTATION_0, ROTATION_90, ROTATION_270, ROTATION_XY_FLIP};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 4;
	CVI_CHAR *filename_in[4] = {GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2};
	CVI_CHAR *filename_out[4] = {
		GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT90_2, GDC_FILE_OUT_ROT270_2, GDC_FILE_OUT_ROT_XYFLIP_2
	};
	CVI_CHAR *aszMD5Sum[4] = {
		GDC_MD5_PEF_ROT0_2, GDC_MD5_PEF_ROT90_2, GDC_MD5_PEF_ROT270_2, GDC_MD5_PEF_ROT_XYFLIP_2
	};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.aszMD5Sum, aszMD5Sum[i]);
		param.size_in.u32Width = GDC_MAX_W;
		param.size_in.u32Height = GDC_MAX_H;
		param.size_out.u32Width = GDC_MAX_W;
		param.size_out.u32Height = GDC_MAX_H;

		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_2_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_2_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_ROT;

		s32Ret = gdc_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_fmt(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 2;
	CVI_CHAR *filename_in[2] = {GDC_FILE_IN_FMT_0, GDC_FILE_IN_FMT_1};
	CVI_CHAR *filename_out[2] = {GDC_FILE_OUT_FMT_0, GDC_FILE_OUT_FMT_1};
	CVI_CHAR *aszMD5Sum[2] = {GDC_MD5_PEF_FMT_0, GDC_MD5_PEF_FMT_1};
	PIXEL_FORMAT_E enPixelFormat[2] = {
		GDC_DEFAULT_PIXEL_FMT,
		PIXEL_FORMAT_YUV_400,
	};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.aszMD5Sum, aszMD5Sum[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		param.size_out.u32Width = 1920;
		param.size_out.u32Height = 1088;

		param.enPixelFormat = enPixelFormat[i];
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_fmt_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_fmt_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_ROT;

		s32Ret = gdc_basic(&param, (void *)ROTATION_0);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_not_align(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_CHAR *filename_in = GDC_FILE_IN_NOT_ALIGN;
	CVI_CHAR *filename_out = GDC_FILE_OUT_NOT_ALIGN;
	VB_CONFIG_S stVbConf;
	CVI_U32 BlkSize;


	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	strcpy(param.filename_in, filename_in);
	strcpy(param.filename_out, filename_out);
	param.size_in.u32Width = 666;
	param.size_in.u32Height = 666;
	param.size_out.u32Width = 666;
	param.size_out.u32Height = 666;

	param.enPixelFormat = PIXEL_FORMAT_YUV_400;
	snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_not_align");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_not_align");
	param.identity.syncIo = CVI_TRUE;
	param.op = GDC_TEST_ROT;

	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width
		, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width
		, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_NOCACHE;

	UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	param.hHandle = 0;
	memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
	s32Ret = GDCFileToFrame2(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
	if (s32Ret) {
		UT_PRT("GDCFileToFrame failed!\n");
		goto exit2;
	}

	memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
	s32Ret = GDC_COMM_PrepareFrame2(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
	if (s32Ret) {
		UT_PRT("GDC_COMM_PrepareFrame failed!\n");
		goto exit2;
	}

	memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
	memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
	memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

	s32Ret = CVI_GDC_BeginJob(&param.hHandle);
	if (s32Ret) {
		UT_PRT("CVI_GDC_BeginJob failed!\n");
		goto exit2;
	}

	s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
	if (s32Ret) {
		UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
		goto exit2;
	}

	s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
	if (s32Ret) {
		UT_PRT("CVI_GDC_AddRotationTask failed!\n");
	}

	s32Ret = CVI_GDC_EndJob(param.hHandle);
	if (s32Ret) {
		UT_PRT("CVI_GDC_EndJob failed!\n");
		goto exit2;
	}

	UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
		, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
	UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
		, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

exit2:
	if (s32Ret)
		if (param.hHandle)
			s32Ret |= CVI_GDC_CancelJob(param.hHandle);
	CVI_GDC_FreeCurTaskMesh(param.stTask.name);
	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		UT_PRT("CVI_GDC_DeInit fail.\n");
	}

	param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
	if (param.inBlk != VB_INVALID_HANDLE) {
		s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
		param.inBlk = VB_INVALID_HANDLE;
	}
	param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
	if (param.outBlk != VB_INVALID_HANDLE) {
		s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			param.outBlk = VB_INVALID_HANDLE;
	}
exit1:
	s32Ret |= CVI_VB_Exit();
exit0:
	s32Ret |= CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_reset(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 BlkSize;
	CVI_S32 times = 4, err_times = 4;

	do {
		for (CVI_U8 cnt = 0; cnt < times; cnt++) {
			s32Ret = CVI_SYS_Init();
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_SYS_Init failed!\n");
				return s32Ret;
			}
			param.op = GDC_TEST_ROT;
			param.size_in.u32Width =   1920;
			param.size_in.u32Height =  1080;
			param.size_out.u32Width =  1920;
			param.size_out.u32Height = 1080;
			param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
			snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_reset_%d", cnt);
			param.identity.enModId = CVI_ID_USER;
			param.identity.u32ID = 0;
			snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_reset_%d", cnt);
			param.identity.syncIo = CVI_TRUE;
			param.op = GDC_TEST_ROT;
			param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width
				, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
				, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
			param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width
				, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
				, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

			BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

			stVbConf.u32MaxPoolCnt				= 1;
			stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
			stVbConf.astCommPool[0].u32BlkCnt	= 3;

			UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

			s32Ret = CVI_VB_SetConfig(&stVbConf);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VB_SetConf failed!\n");
				goto exit0;
			}

			s32Ret = CVI_VB_Init();
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VB_Init failed!\n");
				goto exit0;
			}

			s32Ret = CVI_GDC_Init();
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_GDC_Init failed!\n");
				goto exit1;
			}
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_in, param.enPixelFormat, &param.stVideoFrameIn);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame in failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame out failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			if (cnt == 0) {
				param.stTask.stImgIn.stVFrame.u32Width = 0x3fff;
				param.stTask.stImgIn.stVFrame.u32Height = 0x3fff;
				param.stTask.stImgOut.stVFrame.u32Width = 0x3fff;
				param.stTask.stImgOut.stVFrame.u32Height = 0x3fff;
			}

			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				UT_PRT("CVI_GDC_AddRotationTask failed!\n");
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
				param.outBlk = VB_INVALID_HANDLE;
			}

			if (s32Ret) {
				UT_PRT("release VB fail.\n");
				goto exit2;
			}
		exit2:
			if (s32Ret)
				if (param.hHandle)
					s32Ret |= CVI_GDC_CancelJob(param.hHandle);
			CVI_GDC_FreeCurTaskMesh(param.stTask.name);
			s32Ret |= CVI_GDC_DeInit();
			if (s32Ret) {
				UT_PRT("CVI_GDC_DeInit fail.\n");
			}

			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
					param.outBlk = VB_INVALID_HANDLE;
			}
		exit1:
			s32Ret |= CVI_VB_Exit();
		exit0:
			s32Ret |= CVI_SYS_Exit();

			if (cnt == 0 && s32Ret == CVI_SUCCESS) {
				UT_PRT("unknown error, expect NG, but OK occur\n");
				return CVI_FAILURE;
			}
		}
	} while (err_times--);
	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_cmdq(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	CVI_CHAR *filename_in[1] = {GDC_FILE_IN_CMDQ};
	CVI_CHAR *filename_out[1] = {GDC_FILE_OUT_CMDQ};
	CVI_CHAR *aszMD5Sum[1] = {GDC_MD5_PEF_CMDQ};
	GDC_TASK_ATTR_S stTask_1st;
	GDC_TASK_ATTR_S stTask_2nd;
	SIZE_S size_1st_out, size_2nd_out;
	CVI_S32 times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;

	for (CVI_U8 cnt = 0; cnt < 1; cnt ++) {
		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_Init failed!\n");
			return s32Ret;
		}

		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.aszMD5Sum, aszMD5Sum[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1088;
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width
			, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width
			, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

		BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

		stVbConf.u32MaxPoolCnt				= 1;
		stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt	= 3;

		UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VB_SetConf failed!\n");
			goto exit0;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VB_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_GDC_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_GDC_Init failed!\n");
			goto exit1;
		}

		times = GDC_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat
				, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame out failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			//1st rot90
			memset(&stTask_1st.stImgIn, 0, sizeof(stTask_1st.stImgIn));
			memcpy(&stTask_1st.stImgIn, &param.stVideoFrameIn, sizeof(stTask_1st.stImgIn));

			memset(&stTask_1st.stImgOut, 0, sizeof(stTask_1st.stImgOut));
			size_1st_out.u32Width = ALIGN(stTask_1st.stImgIn.stVFrame.u32Height, DEFAULT_ALIGN);
			size_1st_out.u32Height = ALIGN(stTask_1st.stImgIn.stVFrame.u32Width, DEFAULT_ALIGN);
			snprintf(stTask_1st.name, sizeof(param.stTask.name), "%.*s_1st_%d",
					(int)(sizeof(param.stTask.name) - 7), param.stTask.name, times);

			s32Ret = GDC_COMM_PrepareFrame(&size_1st_out, param.enPixelFormat, &stTask_1st.stImgOut);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame 1st out failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_1st, ROTATION_90);
			if (s32Ret) {
				UT_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//2nd rot90 again
			memset(&stTask_2nd.stImgIn, 0, sizeof(stTask_2nd.stImgIn));
			memcpy(&stTask_2nd.stImgIn, &stTask_1st.stImgOut, sizeof(stTask_2nd.stImgIn));

			memset(&stTask_2nd.stImgOut, 0, sizeof(stTask_2nd.stImgOut));
			memcpy(&stTask_2nd.stImgOut, &param.stVideoFrameOut, sizeof(stTask_2nd.stImgIn));
			size_2nd_out.u32Width = ALIGN(stTask_2nd.stImgIn.stVFrame.u32Height, DEFAULT_ALIGN);
			size_2nd_out.u32Height = ALIGN(stTask_2nd.stImgIn.stVFrame.u32Width, DEFAULT_ALIGN);
			stTask_2nd.stImgOut.stVFrame.u32Width = size_2nd_out.u32Width;
			stTask_2nd.stImgOut.stVFrame.u32Height = size_2nd_out.u32Height;
			snprintf(stTask_2nd.name, sizeof(param.stTask.name), "%.*s_2nd_%d",
					(int)(sizeof(param.stTask.name) - 7), param.stTask.name, times);

			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_2nd, ROTATION_90);
			if (s32Ret) {
				UT_PRT("CVI_GDC_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (param.aszMD5Sum[0]) {
				if (CompareWithMD5(param.aszMD5Sum, &param.stVideoFrameOut)) {
					b_gdc_save_file = CVI_TRUE;
					s32Ret = CVI_FAILURE;
					UT_PRT("Compare MD5 fail, MD5:%s\n", param.aszMD5Sum);
				} else {
					b_gdc_save_file = CVI_FALSE;
				}
			} else {
				b_gdc_save_file = CVI_TRUE;
			}
			if (b_gdc_save_file) {
				if (FrameFullSaveToFile(param.filename_out, &stTask_2nd.stImgOut) != CVI_SUCCESS) {
					UT_PRT("FrameFullSaveToFile s32Ret: 0x%x !\n", s32Ret);
				}
				UT_PRT("output file:%s\n", param.filename_out);
				goto exit2;
			}
			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
				param.outBlk = VB_INVALID_HANDLE;
			}
			Blk = CVI_VB_PhysAddr2Handle(stTask_1st.stImgOut.stVFrame.u64PhyAddr[0]);
			if (Blk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(Blk);
				Blk = VB_INVALID_HANDLE;
			}

			if (s32Ret) {
				UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		CVI_GDC_FreeCurTaskMesh(stTask_1st.name);
		CVI_GDC_FreeCurTaskMesh(stTask_2nd.name);
		s32Ret |= CVI_GDC_DeInit();
		if (s32Ret) {
			UT_PRT("CVI_GDC_DeInit fail.\n");
		}

		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
			param.inBlk = VB_INVALID_HANDLE;
		}
		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			param.outBlk = VB_INVALID_HANDLE;
		}
		Blk = CVI_VB_PhysAddr2Handle(stTask_1st.stImgOut.stVFrame.u64PhyAddr[0]);
		if (Blk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(Blk);
			Blk = VB_INVALID_HANDLE;
		}
	exit1:
		s32Ret |= CVI_VB_Exit();
	exit0:
		s32Ret |= CVI_SYS_Exit();

		if (s32Ret)
			goto err;
	}

err:
	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_cmdq_1to2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	CVI_CHAR *filename_in[1] = {GDC_FILE_IN_CMDQ_1TO2};
	CVI_CHAR *filename_out[1] = {GDC_FILE_OUT_CMDQ_1TO2_0};
	CVI_CHAR *aszMD5Sum[1] = {GDC_MD5_PEF_CMDQ_1TO2_0};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	CVI_S32 times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 2; cnt ++) {
		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_Init failed!\n");
			return s32Ret;
		}
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.aszMD5Sum, aszMD5Sum[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1088;
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width
			, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width
			, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

		BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

		stVbConf.u32MaxPoolCnt				= 1;
		stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt	= 3;

		UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VB_SetConf failed!\n");
			goto exit0;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VB_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_GDC_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_GDC_Init failed!\n");
			goto exit1;
		}

		times = GDC_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat
				, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame out 1st failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			strcpy(name, param.stTask.name);
			snprintf(param.stTask.name, sizeof(param.stTask.name), "%.*s_1st_%d",
					(int)(sizeof(param.stTask.name) - 7), name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				UT_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//1to2
			memset(&stVideoFrameOut_tmp, 0, sizeof(stVideoFrameOut_tmp));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame out 2nd failed!\n");
				goto exit2;
			}

			memset(&stTask_tmp, 0, sizeof(stTask_tmp));
			memcpy(&stTask_tmp.stImgIn, &param.stVideoFrameIn, sizeof(VIDEO_FRAME_INFO_S));
			memcpy(&stTask_tmp.stImgOut, &stVideoFrameOut_tmp, sizeof(VIDEO_FRAME_INFO_S));
			snprintf(stTask_tmp.name, sizeof(param.stTask.name), "%.*s_2nd_%d",
					(int)(sizeof(param.stTask.name) - 7), name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_tmp, ROTATION_0);
			if (s32Ret) {
				UT_PRT("CVI_GDC_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (param.aszMD5Sum[0]) {
				if (CompareWithMD5(param.aszMD5Sum, &param.stVideoFrameOut) &&
					CompareWithMD5(param.aszMD5Sum, &stTask_tmp.stImgOut)) {
					b_gdc_save_file = CVI_TRUE;
					s32Ret = CVI_FAILURE;
					UT_PRT("Compare MD5 fail, MD5:%s\n", param.aszMD5Sum);
				} else {
					b_gdc_save_file = CVI_FALSE;
				}
			} else {
				b_gdc_save_file = CVI_TRUE;
			}
			if (b_gdc_save_file) {
				if (FrameFullSaveToFile(param.filename_out, &param.stVideoFrameOut) != CVI_SUCCESS) {
					UT_PRT("FrameFullSaveToFile s32Ret: 0x%x !\n", s32Ret);
				}
				UT_PRT("output file:%s\n", param.filename_out);
				goto exit2;
			}
			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
				param.outBlk = VB_INVALID_HANDLE;
			}
			Blk = CVI_VB_PhysAddr2Handle(stTask_tmp.stImgOut.stVFrame.u64PhyAddr[0]);
			if (Blk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(Blk);
				Blk = VB_INVALID_HANDLE;
			}

			if (s32Ret) {
				UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		CVI_GDC_FreeCurTaskMesh(param.stTask.name);
		CVI_GDC_FreeCurTaskMesh(stTask_tmp.name);
		s32Ret |= CVI_GDC_DeInit();
		if (s32Ret) {
			UT_PRT("CVI_GDC_DeInit fail.\n");
		}

		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
			param.inBlk = VB_INVALID_HANDLE;
		}
		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			param.outBlk = VB_INVALID_HANDLE;
		}
		Blk = CVI_VB_PhysAddr2Handle(stTask_tmp.stImgOut.stVFrame.u64PhyAddr[0]);
		if (Blk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(Blk);
			Blk = VB_INVALID_HANDLE;
		}
	exit1:
		s32Ret |= CVI_VB_Exit();
	exit0:
		s32Ret |= CVI_SYS_Exit();

		if (s32Ret)
			goto err;
	}

err:
	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_cmdq_1to2_maxsize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	CVI_CHAR *filename_in[1] = {GDC_FILE_IN_CMDQ_1TO2_MAX};
	CVI_CHAR *filename_out[1] = {GDC_FILE_OUT_CMDQ_1TO2_0_MAX};
	CVI_CHAR *aszMD5Sum[1] = {GDC_MD5_PEF_CMDQ_1TO2_0_MAX};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	CVI_S32 times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 1; cnt ++) {
		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_Init failed!\n");
			return s32Ret;
		}
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.aszMD5Sum, aszMD5Sum[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   GDC_MAX_W;
		param.size_in.u32Height =  GDC_MAX_H;
		param.size_out.u32Width =  GDC_MAX_W;
		param.size_out.u32Height = GDC_MAX_H;
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width
			, param.size_in.u32Height, GDC_DEFAULT_PIXEL_FMT
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width
			, param.size_out.u32Height, GDC_DEFAULT_PIXEL_FMT
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

		BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

		stVbConf.u32MaxPoolCnt				= 1;
		stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt	= 3;

		UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VB_SetConf failed!\n");
			goto exit0;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VB_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_GDC_Init();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_GDC_Init failed!\n");
			goto exit1;
		}

		times = GDC_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat
				, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame out 1st failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			strcpy(name, param.stTask.name);
			snprintf(param.stTask.name, sizeof(param.stTask.name), "%.*s_1st_%d",
				(int)(sizeof(param.stTask.name) - 7), name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				UT_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//1to2
			memset(&stVideoFrameOut_tmp, 0, sizeof(stVideoFrameOut_tmp));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp);
			if (s32Ret) {
				UT_PRT("GDC_COMM_PrepareFrame out 2nd failed!\n");
				goto exit2;
			}

			memset(&stTask_tmp, 0, sizeof(stTask_tmp));
			memcpy(&stTask_tmp.stImgIn, &param.stVideoFrameIn, sizeof(VIDEO_FRAME_INFO_S));
			memcpy(&stTask_tmp.stImgOut, &stVideoFrameOut_tmp, sizeof(VIDEO_FRAME_INFO_S));
			snprintf(stTask_tmp.name, sizeof(param.stTask.name), "%.*s_2nd_%d",
					(int)(sizeof(param.stTask.name) - 7), name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_tmp, ROTATION_0);
			if (s32Ret) {
				UT_PRT("CVI_GDC_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (param.aszMD5Sum[0]) {
				if (CompareWithMD5(param.aszMD5Sum, &param.stVideoFrameOut) &&
					CompareWithMD5(param.aszMD5Sum, &stTask_tmp.stImgOut)) {
					b_gdc_save_file = CVI_TRUE;
					s32Ret = CVI_FAILURE;
					UT_PRT("Compare MD5 fail, MD5:%s\n", param.aszMD5Sum);
				} else {
					b_gdc_save_file = CVI_FALSE;
				}
			} else {
				b_gdc_save_file = CVI_TRUE;
			}
			if (b_gdc_save_file) {
				if (FrameFullSaveToFile(param.filename_out, &param.stVideoFrameOut) != CVI_SUCCESS) {
					UT_PRT("FrameFullSaveToFile s32Ret: 0x%x !\n", s32Ret);
				}
				UT_PRT("output file:%s\n", param.filename_out);
				goto exit2;
			}
			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
				param.outBlk = VB_INVALID_HANDLE;
			}
			Blk = CVI_VB_PhysAddr2Handle(stTask_tmp.stImgOut.stVFrame.u64PhyAddr[0]);
			if (Blk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(Blk);
				Blk = VB_INVALID_HANDLE;
			}

			if (s32Ret) {
				UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		CVI_GDC_FreeCurTaskMesh(param.stTask.name);
		CVI_GDC_FreeCurTaskMesh(stTask_tmp.name);
		s32Ret |= CVI_GDC_DeInit();
		if (s32Ret) {
			UT_PRT("CVI_GDC_DeInit fail.\n");
		}

		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
			param.inBlk = VB_INVALID_HANDLE;
		}
		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			param.outBlk = VB_INVALID_HANDLE;
		}
		Blk = CVI_VB_PhysAddr2Handle(stTask_tmp.stImgOut.stVFrame.u64PhyAddr[0]);
		if (Blk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(Blk);
			Blk = VB_INVALID_HANDLE;
		}
	exit1:
		s32Ret |= CVI_VB_Exit();
	exit0:
		s32Ret |= CVI_SYS_Exit();

		if (s32Ret)
			goto err;
	}
err:
	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_async(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_CHAR *filename_in[1] = {GDC_FILE_IN_ROT};
	CVI_CHAR *filename_out[1] = {GDC_FILE_OUT_ROT0};
	CVI_CHAR *aszMD5Sum[1] = {GDC_MD5_PEF_ROT0};
	CVI_U8 times = GDC_REPEAT_TIMES * 10;
	CVI_U32 BlkSize;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	strcpy(param.filename_in, filename_in[0]);
	strcpy(param.filename_out, filename_out[0]);
	strcpy(param.aszMD5Sum, aszMD5Sum[0]);
	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1088;
	param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
	snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_async");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_async");
	param.identity.syncIo = CVI_FALSE;
	param.op = GDC_TEST_ROT;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width
		, param.size_in.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width
		, param.size_out.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;

	UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	do {
		param.hHandle = 0;
		memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat
			, param.filename_in, &param.stVideoFrameIn);
		if (s32Ret) {
			UT_PRT("GDCFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat
			, &param.stVideoFrameOut);
		if (s32Ret) {
			UT_PRT("GDC_COMM_PrepareFrame failed!\n");
			goto exit2;
		}

		memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
		memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
		memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param.hHandle);
		if (s32Ret) {
			UT_PRT("CVI_GDC_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
		if (s32Ret) {
			UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
		if (s32Ret) {
			UT_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_EndJob(param.hHandle);
		if (s32Ret) {
			UT_PRT("CVI_GDC_EndJob failed!\n");
			goto exit2;
		}

		if (needSuspend) {
			s32Ret = CVI_GDC_Suspend();
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_GDC_Suspend fail. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}
			s32Ret = CVI_GDC_Resume();
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_GDC_Resume fail. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}
		}

		usleep(1000*500);

		if ((s32Ret = CVI_GDC_GetChnFrame(&param.identity, &param.stVideoFrameOut, 5000)) != CVI_SUCCESS)
			break;

		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);

		if (s32Ret) {
			UT_PRT("release VB fail.\n");
			goto exit2;
		}
	} while (times--);

exit2:
	if (s32Ret)
		if (param.hHandle)
			s32Ret |= CVI_GDC_CancelJob(param.hHandle);
	CVI_GDC_FreeCurTaskMesh(param.stTask.name);
	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		UT_PRT("CVI_GDC_DeInit fail.\n");
	}

	if (param.stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
	}
	if (param.stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
	}
exit1:
	s32Ret |= CVI_VB_Exit();
exit0:
	s32Ret |= CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_online(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_BOOL LdcAsyncDoneFlag;
void *test_gdc_async_thread(void *data)
{
	CVI_S32 s32MilliSec = 5000;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	CVI_S32 s32Ret;

	if (!param)
		goto EXIT;

	while (!LdcAsyncDoneFlag) {
		s32Ret = CVI_GDC_GetChnFrame(&param->identity, &param->stVideoFrameOut, s32MilliSec);
		if (s32Ret) {
			usleep(1000* 5);
			continue;
		}

		UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[1]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[1]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		//if (b_gdc_save_file) {
		if (0) {
			s32Ret = FrameFullSaveToFile(param->filename_out, &param->stVideoFrameOut);
			if (s32Ret) {
				UT_PRT("FrameFullSaveToFile fail\n");
			}
		}

		param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param->inBlk != VB_INVALID_HANDLE)
			CVI_VB_ReleaseBlock(param->inBlk);

		param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param->outBlk != VB_INVALID_HANDLE)
			CVI_VB_ReleaseBlock(param->outBlk);
	}
EXIT:
	pthread_exit(0);
}

static CVI_S32 gdc_basic_do_job(GDC_BASIC_TEST_PARAM *param, CVI_S32 times, void *op_ptr)
{
	CVI_S32 s32Ret;

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param->size_in, param->enPixelFormat
			, param->filename_in, &param->stVideoFrameIn);
		if (s32Ret) {
			UT_PRT("GDCFileToFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
		if (s32Ret) {
			UT_PRT("GDC_COMM_PrepareFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param->hHandle);
		if (s32Ret) {
			UT_PRT("CVI_GDC_BeginJob failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = gdc_basic_add_tsk(param, op_ptr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_GDC_EndJob(param->hHandle);
		if (s32Ret) {
			UT_PRT("CVI_GDC_EndJob failed!\n");
			return CVI_FAILURE;
		}

		param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param->inBlk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(param->inBlk);
			param->inBlk = VB_INVALID_HANDLE;
		}
		param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param->outBlk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(param->outBlk);
			param->outBlk = VB_INVALID_HANDLE;
		}
	} while (times--);

	return s32Ret;
}

void *gdc_basic_thread_func0(void *data)
{
	CVI_S32 s32Ret;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	CVI_S32 times = GDC_REPEAT_TIMES * 10;
	void *ptr = (void *)ROTATION_0;

	s32Ret = gdc_basic_do_job(param, times, ptr);
	if (s32Ret) {
		UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			CVI_GDC_CancelJob(param->hHandle);
	CVI_GDC_FreeCurTaskMesh(param->stTask.name);

	pthread_exit(0);
}

static CVI_S32 gdc_test_multi_thread(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param[8] = {0};
	CVI_U32 BlkSize;
	CVI_S32 i, rc, threads;
	CVI_CHAR *filename_in[4] = {
		GDC_FILE_IN_ROT_1,
		GDC_FILE_IN_ROT_1,
		GDC_FILE_IN_ROT_1,
		GDC_FILE_IN_ROT_1};
	CVI_CHAR *filename_out[4] = {
		GDC_FILE_OUT_ROT0_1,
		GDC_FILE_OUT_ROT0_1,
		GDC_FILE_OUT_ROT0_1,
		GDC_FILE_OUT_ROT0_1};
	CVI_CHAR *aszMD5Sum[4] = {GDC_MD5_PEF_ROT0_1, GDC_MD5_PEF_ROT0_1, GDC_MD5_PEF_ROT0_1, GDC_MD5_PEF_ROT0_1};
	GDC_TEST_OP op[4] = {GDC_TEST_ROT, GDC_TEST_ROT, GDC_TEST_ROT, GDC_TEST_ROT};
	CVI_U32 WidthIn[4] = {128, 128, 128, 128};
	CVI_U32 HeightIn[4] = {128, 128, 128, 128};
	CVI_U32 WidthOut[4] = {128, 128, 128, 128};
	CVI_U32 HeightOut[4] = {128, 128, 128, 128};
	pthread_t thread[4] = {[0 ... 3] = 0};

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	param[0].size_in.u32Width = WidthIn[0];
	param[0].size_in.u32Height = HeightIn[0];
	param[0].size_out.u32Width = WidthOut[0];
	param[0].size_out.u32Height = HeightOut[0];
	param[0].enPixelFormat = GDC_DEFAULT_PIXEL_FMT;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param[0].u32BlkSizeIn = COMMON_GetPicBufferSize(param[0].size_in.u32Width
		, param[0].size_in.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	param[0].u32BlkSizeOut = COMMON_GetPicBufferSize(param[0].size_out.u32Width
		, param[0].size_out.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	BlkSize = (param->u32BlkSizeIn > param->u32BlkSizeOut) ? param->u32BlkSizeIn : param->u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 10;

	UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	pthread_attr_t attr;
	struct sched_param schedParam;

	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	schedParam.sched_priority = 99;
	pthread_attr_setschedparam(&attr, &schedParam);

	threads = 4;
	for (i = 0; i < threads; i++) {
		param[i].enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		param[i].identity.syncIo = CVI_TRUE;
		strcpy(param[i].filename_in, filename_in[i]);
		strcpy(param[i].filename_out, filename_out[i]);
		strcpy(param[i].aszMD5Sum, aszMD5Sum[i]);
		param[i].size_in.u32Width = WidthIn[i];
		param[i].size_in.u32Height = HeightIn[i];
		param[i].size_out.u32Width = WidthOut[i];
		param[i].size_out.u32Height = HeightOut[i];
		param[i].op = op[i];
		snprintf(param[i].stTask.name, sizeof(param[i].stTask.name), "tsk_multi_th_%d", i);
		param[i].identity.enModId = CVI_ID_USER;
		param[i].identity.u32ID = i;
		snprintf(param[i].identity.Name, sizeof(param[i].identity.Name), "job_multi_th_%d", i);

		rc = pthread_create(&thread[i], &attr, gdc_basic_thread_func0, (void *)&param[i]);
		if (rc) {
			UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	pthread_attr_destroy(&attr);

	for (i = 0; i < threads; i++)
		pthread_join(thread[i], NULL);

	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		UT_PRT("CVI_GDC_DeInit failed!\n");
	}
exit1:
	s32Ret |= CVI_VB_Exit();
exit0:
	s32Ret |= CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_presure_size_for_each(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 times = GDC_REPEAT_TIMES;
	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param = {0};
	PIXEL_FORMAT_E enPixelFormat[2] = {
		GDC_DEFAULT_PIXEL_FMT,
		PIXEL_FORMAT_YUV_400,
	};
	CVI_U32 BlkSize;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	for (CVI_S32 w = GDC_MIN_W; w < GDC_MAX_W; w += GDC_MIN_W) {
		for (CVI_S32 h = GDC_MIN_H; h < GDC_MAX_H; h += GDC_MIN_H) {
			for (CVI_U8 fmt_idx = 0; fmt_idx < 2; fmt_idx ++) {
				s32Ret = CVI_SYS_Init();
				if (s32Ret != CVI_SUCCESS) {
					UT_PRT("CVI_SYS_Init failed!\n");
					return s32Ret;
				}

				param.size_in.u32Width = w;
				param.size_in.u32Height = h;
				param.size_out.u32Width = w;
				param.size_out.u32Height = h;
				param.enPixelFormat = enPixelFormat[fmt_idx];
				snprintf(param.stTask.name, sizeof(param.stTask.name)
					, "tsk_fmt%d_w%d_h%d", fmt_idx, w, h);
				param.identity.enModId = CVI_ID_USER;
				param.identity.u32ID = (w + h);
				snprintf(param.identity.Name, sizeof(param.identity.Name)
					, "job_fmt%d_w%d_h%d", fmt_idx, w, h);
				param.identity.syncIo = CVI_TRUE;
				param.op = GDC_TEST_ROT;
				param.u32BlkSizeIn = COMMON_GetPicBufferSize(GDC_MAX_W, GDC_MAX_H, PIXEL_FORMAT_YUV_PLANAR_444
					, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
				param.u32BlkSizeOut = COMMON_GetPicBufferSize(GDC_MAX_W, GDC_MAX_H, PIXEL_FORMAT_YUV_PLANAR_444
					, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

				BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

				stVbConf.u32MaxPoolCnt				= 1;
				stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
				stVbConf.astCommPool[0].u32BlkCnt	= 2;

				UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

				s32Ret = CVI_VB_SetConfig(&stVbConf);
				if (s32Ret != CVI_SUCCESS) {
					UT_PRT("CVI_VB_SetConf failed!\n");
					goto exit0;
				}

				s32Ret = CVI_VB_Init();
				if (s32Ret != CVI_SUCCESS) {
					UT_PRT("CVI_VB_Init failed!\n");
					goto exit0;
				}

				s32Ret = CVI_GDC_Init();
				if (s32Ret != CVI_SUCCESS) {
					UT_PRT("CVI_GDC_Init failed!\n");
					goto exit1;
				}

				times = GDC_REPEAT_TIMES;
				do {
					memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
					s32Ret = GDC_COMM_PrepareFrame(&param.size_in
						, param.enPixelFormat, &param.stVideoFrameIn);
					if (s32Ret) {
						UT_PRT("GDC_COMM_PrepareFrame in failed!\n");
						goto exit2;
					}

					memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
					s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat
						, &param.stVideoFrameOut);
					if (s32Ret) {
						UT_PRT("GDC_COMM_PrepareFrame out failed!\n");
						goto exit2;
					}

					memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
					memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
					memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

					s32Ret = CVI_GDC_BeginJob(&param.hHandle);
					if (s32Ret) {
						UT_PRT("CVI_GDC_BeginJob failed!\n");
						goto exit2;
					}

					s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
					if (s32Ret) {
						UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
						goto exit2;
					}

					s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
					if (s32Ret) {
						UT_PRT("CVI_GDC_AddRotationTask failed!\n");
					}

					s32Ret = CVI_GDC_EndJob(param.hHandle);
					if (s32Ret) {
						UT_PRT("CVI_GDC_EndJob failed!\n");
						goto exit2;
					}

					UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
						, param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
						, param.stVideoFrameIn.stVFrame.u64PhyAddr[1]
						, param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
					UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
						, param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
						, param.stVideoFrameOut.stVFrame.u64PhyAddr[1]
						, param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);
					UT_PRT("-------------------times:(%d)----------------------\n", times);

					param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
					if (param.inBlk != VB_INVALID_HANDLE)
						s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

					param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
					if (param.outBlk != VB_INVALID_HANDLE)
						s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);

					if (s32Ret) {
						UT_PRT("release VB fail.\n");
						break;
					}
				} while (times--);
			exit2:
				if (s32Ret && param.hHandle)
					s32Ret |= CVI_GDC_CancelJob(param.hHandle);
				CVI_GDC_FreeCurTaskMesh(param.stTask.name);
				s32Ret |= CVI_GDC_DeInit();
				if (s32Ret) {
					UT_PRT("CVI_GDC_DeInit fail.\n");
				}

				param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
				if (param.inBlk != VB_INVALID_HANDLE)
					s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

				param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
				if (param.outBlk != VB_INVALID_HANDLE)
					s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			exit1:
				s32Ret |= CVI_VB_Exit();
			exit0:
				s32Ret |= CVI_SYS_Exit();

				if (s32Ret)
					goto err;
			}
		}
	}

err:
	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_pef(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	p_func test_func[4] = {gdc_test_rot, gdc_test_rot_scaling,
		gdc_test_rot_maxsize, gdc_test_ldc_load_mesh_scaling};

	for (CVI_S32 i = 0; i < 4; i++)
		s32Ret |= test_func[i]();

	return s32Ret;
}

static CVI_S32 gdc_test_ldc_grid_info(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr = {0};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_CHAR *filename_in = GDC_FILE_IN_LDC_GRID_INFO;
	CVI_CHAR *filename_out = GDC_FILE_OUT_LDC_GRID_INFO;
	CVI_CHAR *filename_grid = GDC_FILE_IN_LDC_GRID;

	strcpy(param.filename_in, filename_in);
	strcpy(param.filename_out, filename_out);

	param.size_in.u32Width = 1280;
	param.size_in.u32Height = ALIGN(720, DEFAULT_ALIGN);
	param.size_out.u32Width = 1280;
	param.size_out.u32Height = ALIGN(720, DEFAULT_ALIGN);
	param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
	snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_grid_0");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_grid_0");
	param.identity.syncIo = CVI_TRUE;

	param.op = GDC_TEST_LDC;

	stLdcAttr.stGridInfoAttr.Enable = CVI_TRUE;
	strcpy(stLdcAttr.stGridInfoAttr.gridFileName, filename_grid);
	strcpy(stLdcAttr.stGridInfoAttr.gridBindName, param.stTask.name);

	s32Ret = gdc_basic(&param, (void *)&stLdcAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("Test failed.\n");
		return s32Ret;
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_suspend(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_GDC_Suspend();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_Suspend failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 gdc_test_resume(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_GDC_Resume();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_Resume failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 gdc_test_running_suspend(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	needSuspend = CVI_TRUE;
	s32Ret = gdc_test_async();
	needSuspend = CVI_FALSE;
	return s32Ret;
}

static CVI_S32 gdc_test_auto_regression(CVI_VOID)
{
	CVI_S32 s32Ret[100] = {[0 ... 99] = CVI_SUCCESS};
	CVI_S32 Ret = CVI_SUCCESS;
	p_func test_func[MAX_FUNC_CNT] = {
		gdc_test_rot,
		gdc_test_rot_small,
		gdc_test_rot_scaling,
		gdc_test_ldc,
		gdc_test_ldc_load_mesh_scaling,
		gdc_test_rot_maxsize,
		gdc_test_fmt,
		gdc_test_not_align,
		gdc_test_cmdq,
		gdc_test_cmdq_1to2,
		gdc_test_cmdq_1to2_maxsize,
		gdc_test_async,
		gdc_test_online,
		gdc_test_multi_thread,
		gdc_test_pef,
		gdc_test_ldc_grid_info,
		gdc_test_reset,
		//gdc_test_presure_size_for_each();//it takes too long time
	};

	for (CVI_S32 i = 0; i < MAX_FUNC_CNT; i++) {
		if (test_func[i]) {
			s32Ret[i] = test_func[i]();
			Ret |= s32Ret[i];
		}
	}

	for (CVI_S32 i = 0; i < MAX_FUNC_CNT; i++) {
		if (s32Ret[i])
			UT_PRT("op[%d] fail, ret[%d]\n", i, s32Ret[i]);
	}

	UT_CHECK_CASE_RET(Ret);
	return Ret;
}

static CVI_S32 gdc_test_user_config(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32WidthIn, u32HeightIn;
	CVI_U32 u32WidthOut, u32HeightOut;
	CVI_S32 fmt;
	CVI_S32 op, tmp;
	CVI_U8 i;
	GDC_BASIC_TEST_PARAM stTestParam = {0};

	memset(&stTestParam, 0, sizeof(stTestParam));
	printf("\n---gdc config---\n");
	printf("input width:");
	scanf("%d", &u32WidthIn);
	printf("input height:");
	scanf("%d", &u32HeightIn);

	printf("output width:");
	scanf("%d", &u32WidthOut);
	printf("output height:");
	scanf("%d", &u32HeightOut);

	printf("format list:\n");
	for (i = PIXEL_FORMAT_RGB_888; i < PIXEL_FORMAT_MAX; i++) {
		if (strncmp(GetFmtName(i), "unknown", sizeof("unknown")))
			printf("%2d : %s\n", i, GetFmtName(i));
	}
	printf("input format:");
	scanf("%d", &fmt);

	printf("input file:");
	scanf("%s", stTestParam.filename_in);

	stTestParam.size_in.u32Width = u32WidthIn;
	stTestParam.size_in.u32Height = u32HeightIn;
	stTestParam.size_out.u32Width = u32WidthOut;
	stTestParam.size_out.u32Height = u32HeightOut;
	stTestParam.enPixelFormat = (PIXEL_FORMAT_E)fmt;
	stTestParam.identity.syncIo = CVI_TRUE;
	snprintf(stTestParam.filename_out, 128, "res/output/%s_%d_%d_%s.bin", __func__,
		stTestParam.size_out.u32Width,
		stTestParam.size_out.u32Height,
		GetFmtName(stTestParam.enPixelFormat));

	printf("input rot or ldc:(rot:0,ldc:1) ");
	scanf("%d", &op);
	printf("\n");
	if (op == 0) {
		ROTATION_E rot;
		printf("input rot:(rot0:0, rot90:1, rot180:2, rot270:3) ");
		scanf("%d", &tmp);
		printf("\n");

		rot = (ROTATION_E)tmp;
		stTestParam.op = GDC_TEST_ROT;
		s32Ret = gdc_basic(&stTestParam, (void *)rot);
	} else {

		LDC_ATTR_S stLDCAttr = {0};

		printf("Keep AspectRatio 1(Y)/0(N): ");
		scanf("%d", &tmp);
		stLDCAttr.bAspect = tmp;
		if (stLDCAttr.bAspect) {
			printf("Ratio (0 ~ 100): ");
			scanf("%d", &tmp);
			stLDCAttr.s32XYRatio = tmp;
		} else {
			printf("XRatio (0 ~ 100): ");
			scanf("%d", &stLDCAttr.s32XRatio);
			printf("YRatio (0 ~ 100): ");
			scanf("%d", &stLDCAttr.s32YRatio);
		}
		printf("XOffset (-511 ~ 511): ");
		scanf("%d", &stLDCAttr.s32CenterXOffset);
		printf("YOffset (-511 ~ 511): ");
		scanf("%d", &stLDCAttr.s32CenterYOffset);
		printf("DistortionRatio (-300 ~ 500): ");
		scanf("%d", &stLDCAttr.s32DistortionRatio);

		stTestParam.op = GDC_TEST_LDC;
		s32Ret = gdc_basic(&stTestParam, (void *)&stLDCAttr);
	}

	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("Test failed.\n");
		return s32Ret;
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 _gdc_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case GDC_TEST_ROT:
		s32Ret = gdc_test_rot();
		break;
	case GDC_TEST_ROT1:
		s32Ret = gdc_test_rot_small();
		break;
	case GDC_TEST_ROT2:
		s32Ret = gdc_test_rot_scaling();
		break;
	case GDC_TEST_LDC:
		s32Ret = gdc_test_ldc();
		break;
	case GDC_TEST_LDC_LOAD_MESH:
		s32Ret = gdc_test_ldc_load_mesh_scaling();
		break;
	case GDC_TEST_MAX_SIZE:
		s32Ret = gdc_test_rot_maxsize();
		break;
	case GDC_TEST_FMT:
		s32Ret = gdc_test_fmt();
		break;
	case GDC_TEST_SIZE_NO_ALIGN:
		s32Ret = gdc_test_not_align();
		break;
	case GDC_TEST_CMDQ:
		s32Ret = gdc_test_cmdq();
		break;
	case GDC_TEST_CMDQ_1TO2:
		s32Ret = gdc_test_cmdq_1to2();
		break;
	case GDC_TEST_CMDQ_1TO2_MAX:
		s32Ret = gdc_test_cmdq_1to2_maxsize();
		break;
	case GDC_TEST_ONLINE:
		s32Ret = gdc_test_online();
		break;
	case GDC_TEST_ASYNC:
		s32Ret = gdc_test_async();
		break;
	case GDC_TEST_MULTI_THREAD:
		s32Ret = gdc_test_multi_thread();
		break;
	case GDC_TEST_PEF:
		s32Ret = gdc_test_pef();
		break;
	case GDC_TEST_LOAD_GRID_INFO:
		s32Ret = gdc_test_ldc_grid_info();
		break;
	case GDC_TEST_RST:
		s32Ret = gdc_test_reset();
		break;
	case GDC_TEST_SUSPEND:
		s32Ret = gdc_test_suspend();
		break;
	case GDC_TEST_RESUME:
		s32Ret = gdc_test_resume();
		break;
	case GDC_TEST_RUN_SUSPEND:
		s32Ret = gdc_test_running_suspend();
		break;
	case GDC_TEST_PRESURE_SIZE_FOR_EACH:
		s32Ret = gdc_test_presure_size_for_each();
		break;
	case GDC_TEST_AUTO_REGRESSION:
		s32Ret = gdc_test_auto_regression();
		break;
	case GDC_TEST_USER_CONFIG:
		s32Ret = gdc_test_user_config();
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static void gdc_show_help(void)
{
	UT_PRT("%4d: gdc basic test rot\n", GDC_TEST_ROT);
	UT_PRT("%4d: gdc basic test rot small\n", GDC_TEST_ROT1);
	UT_PRT("%4d: gdc basic test rot img scaling 4m 5m 8m\n", GDC_TEST_ROT2);
	UT_PRT("%4d: gdc basic test ldc\n", GDC_TEST_LDC);
	UT_PRT("%4d: gdc basic test ldc load mesh\n", GDC_TEST_LDC_LOAD_MESH);
	UT_PRT("%4d: gdc basic test maxsize\n", GDC_TEST_MAX_SIZE);
	UT_PRT("%4d: gdc basic test fmt\n", GDC_TEST_FMT);
	UT_PRT("%4d: gdc basic test size not align\n", GDC_TEST_SIZE_NO_ALIGN);
	UT_PRT("%4d: gdc basic test cmdq\n", GDC_TEST_CMDQ);
	UT_PRT("%4d: gdc basic test cmdq_1to2\n", GDC_TEST_CMDQ_1TO2);
	UT_PRT("%4d: gdc basic test cmdq_1to2 maxsize\n", GDC_TEST_CMDQ_1TO2_MAX);
	UT_PRT("%4d: gdc basic test online\n", GDC_TEST_ONLINE);
	UT_PRT("%4d: gdc basic test async\n", GDC_TEST_ASYNC);
	UT_PRT("%4d: gdc basic test multi thread\n", GDC_TEST_MULTI_THREAD);
	UT_PRT("%4d: gdc basic test pef\n", GDC_TEST_PEF);
	UT_PRT("%4d: gdc basic test grid_info\n", GDC_TEST_LOAD_GRID_INFO);
	UT_PRT("%4d: gdc basic test reset\n", GDC_TEST_RST);
	UT_PRT("%4d: gdc basic test suspend\n", GDC_TEST_SUSPEND);
	UT_PRT("%4d: gdc basic test resume\n", GDC_TEST_RESUME);
	UT_PRT("%4d: gdc basic test running suspend\n", GDC_TEST_RUN_SUSPEND);
	UT_PRT("%4d: gdc test presure size for each\n", GDC_TEST_PRESURE_SIZE_FOR_EACH);
	UT_PRT("%4d: gdc test auto regression\n", GDC_TEST_AUTO_REGRESSION);
	UT_PRT("%4d: gdc user cofig test\n", GDC_TEST_USER_CONFIG);

	UT_PRT("255: exit\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;

	system("stty erase ^H");

	signal(SIGINT, gdc_ut_HandleSig);
	signal(SIGTERM, gdc_ut_HandleSig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		b_gdc_save_file = (CVI_BOOL)atoi(argv[2]);

		s32Ret = _gdc_handle_op(op);
		UT_PRT("gdc ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		b_gdc_save_file = CVI_TRUE;
		do {
			gdc_show_help();
			scanf("%d", &op);

			s32Ret = _gdc_handle_op(op);
			if (op != 255)
				UT_PRT("gdc ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}
