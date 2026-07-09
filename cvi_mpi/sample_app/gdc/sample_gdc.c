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

#include "sample_gdc_comm.h"

//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB

#define GDC_FILE_IN_ROT                         "res/1920x1080.yuv"
#define GDC_FILE_OUT_ROT0                       "res/output/1920x1080_rot0.yuv"
#define GDC_FILE_OUT_ROT90                      "res/output/1920x1080_rot90.yuv"
#define GDC_FILE_OUT_ROT270                     "res/output/1920x1080_rot270.yuv"

#define GDC_FILE_IN_LDC_BARREL_0P3              "res/1920x1080_barrel_0.3.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_0           "res/output/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_1           "res/output/1920x1080_barrel_0.3_r50_ofst_0_0_d-200.yuv"
#define GDC_FILE_IN_LDC_PINCUSHION_0P3          "res/1920x1080_pincushion_0.3.yuv"
#define GDC_FILE_OUT_LDC_PINCUSHION_0P3_0       "res/output/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.yuv"
#define GDC_FILE_OUT_LDC_PINCUSHION_0P3_1       "res/output/1920x1080_pincushion_0.3_r50_ofst_0_0_d400.yuv"

#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_0       "res/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.mesh"

#define GDC_FILE_IN_CMDQ                       "res/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ                      "res/output/1920x1080_cmdq.yuv"
#define GDC_FILE_IN_CMDQ_1TO2                  "res/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ_1TO2_0               "res/output/1920x1080_cmdq_1to2_0.yuv"

#define GDC_FILE_IN_LDC_GRID_INFO              "res/1280x768.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO             "res/output/1280x768_grid_info.yuv"
#define GDC_FILE_IN_LDC_GRID                   "res/grid_info_79_44_3476_80_45_1280x720.dat"

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
	GDC_TEST_LDC,
	GDC_TEST_LDC_LOAD_MESH,
	GDC_TEST_CMDQ,
	GDC_TEST_CMDQ_1TO2,
	GDC_TEST_ASYNC,
	GDC_TEST_LOAD_GRID_INFO,
	GDC_TEST_RUN_SUSPEND,
} GDC_TEST_OP;

typedef struct _GDC_BASIC_TEST_PARAM {
	SIZE_S size_in;
	SIZE_S size_out;
	CVI_CHAR filename_in[128];
	CVI_CHAR filename_out[128];
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

void SAMPLE_GDC_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_VB_Exit();
		CVI_SYS_Exit();
		SAMPLE_PRT("Program termination abnormally\n");
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
		SAMPLE_PRT("gdc_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	switch (param->op) {
	case GDC_TEST_ROT:
		enRotation = (ROTATION_E)(uintptr_t)ptr;

		s32Ret = CVI_GDC_AddRotationTask(param->hHandle, &param->stTask, enRotation);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_AddRotationTask failed!\n");
		}
		break;
	case GDC_TEST_LDC:
		LDCAttr = (LDC_ATTR_S *)ptr;

		s32Ret = CVI_GDC_GenLDCMesh(param->size_in.u32Width, param->size_in.u32Height, LDCAttr
			, param->stTask.name, &u64PhyAddr, &pVirAddr);
		if (s32Ret) {
			SAMPLE_PRT("gen LDC mesh(%s) fail\n", param->stTask.name);
			break;
		}

		param->stTask.au64privateData[0] = u64PhyAddr;
		s32Ret = CVI_GDC_AddLDCTask(param->hHandle, &param->stTask, LDCAttr, param->enRotation);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_AddLDCTask failed!\n");
		}
		break;
	case GDC_TEST_LDC_LOAD_MESH:
		meshName = (CVI_CHAR *)ptr;
		LDC_ATTR_S stLDCAttr = {0};

		s32Ret = CVI_GDC_LoadLDCMesh(param->size_out.u32Width, param->size_out.u32Height
			, meshName, param->stTask.name, &u64PhyAddr, &pVirAddr);
		if (s32Ret) {
			SAMPLE_PRT("gen LDC mesh(%s) fail for tsk(%s)\n", meshName, param->stTask.name);
			break;
		}

		param->stTask.au64privateData[0] = u64PhyAddr;
		s32Ret = CVI_GDC_AddLDCTask(param->hHandle, &param->stTask, &stLDCAttr, ROTATION_0);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_AddLDCTask failed!\n");
		}
		break;
	default:
		SAMPLE_PRT("not allow this op(%d) fail\n", param->op);
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
		SAMPLE_PRT("gdc_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
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

	SAMPLE_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}
	/************************************************
	 * step2:  Init GDC
	 ************************************************/
	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param->size_in, param->enPixelFormat,
			param->filename_in, &param->stVideoFrameIn);
		if (s32Ret) {
			SAMPLE_PRT("GDCFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
		if (s32Ret) {
			SAMPLE_PRT("GDC_COMM_PrepareFrame failed!\n");
			goto exit2;
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param->hHandle);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = gdc_basic_add_tsk(param, ptr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_GDC_EndJob(param->hHandle);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_EndJob failed!\n");
			goto exit2;
		}

		SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[1], param->stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[1], param->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		s32Ret = CVI_GDC_GetWorkJob(&hHandle);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_GetWorkJob failed!\n");
			goto exit2;
		}

		if (b_gdc_save_file) {
			if (SAMPLE_COMM_FRAME_SaveToFile(param->filename_out, &param->stVideoFrameOut) != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile. s32Ret: 0x%x !\n", s32Ret);
			}
			SAMPLE_PRT("-------------------times:(%d)----------------------\n", times);
			SAMPLE_PRT("output file:%s\n", param->filename_out);
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
			SAMPLE_PRT("release VB fail.\n");
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
		SAMPLE_PRT("CVI_GDC_DeInit fail.\n");
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

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
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
			SAMPLE_PRT("Test failed.\n");
			return s32Ret;
		}
	}

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

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);

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
			SAMPLE_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	return s32Ret;
}

static CVI_S32 gdc_test_ldc_load_mesh_scaling(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR *mesh_file_name[1] = {
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_0,
	};
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_S32 cnt = 1;
	CVI_CHAR *filename_in[1] = {
		GDC_FILE_IN_LDC_BARREL_0P3,
	};
	CVI_CHAR *filename_out[1] = {
		GDC_FILE_OUT_LDC_BARREL_0P3_0,
	};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);

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

		param.op = GDC_TEST_LDC_LOAD_MESH;

		s32Ret = gdc_basic(&param, (void *)mesh_file_name[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	return s32Ret;
}

static CVI_S32 gdc_test_cmdq(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	CVI_CHAR *filename_in[1] = {GDC_FILE_IN_CMDQ};
	CVI_CHAR *filename_out[1] = {GDC_FILE_OUT_CMDQ};
	GDC_TASK_ATTR_S stTask_1st;
	GDC_TASK_ATTR_S stTask_2nd;
	SIZE_S size_1st_out, size_2nd_out;
	CVI_S32 times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;

	for (CVI_U8 cnt = 0; cnt < 1; cnt ++) {
		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_SYS_Init failed!\n");
			return s32Ret;
		}

		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);

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

		SAMPLE_PRT("common pool[0] BlkSize %d\n", BlkSize);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VB_SetConf failed!\n");
			goto exit0;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VB_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_GDC_Init();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_GDC_Init failed!\n");
			goto exit1;
		}

		times = GDC_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat
				, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				SAMPLE_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				SAMPLE_PRT("GDC_COMM_PrepareFrame out failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_SetJobIdentity failed!\n");
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
				SAMPLE_PRT("GDC_COMM_PrepareFrame 1st out failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_1st, ROTATION_90);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
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
				SAMPLE_PRT("CVI_GDC_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (b_gdc_save_file) {
				if (SAMPLE_COMM_FRAME_SaveToFile(param.filename_out, &stTask_2nd.stImgOut) != CVI_SUCCESS) {
					SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile s32Ret: 0x%x !\n", s32Ret);
				}
				SAMPLE_PRT("output file:%s\n", param.filename_out);
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
				SAMPLE_PRT("release VB fail.\n");
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
			SAMPLE_PRT("CVI_GDC_DeInit fail.\n");
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
	return s32Ret;
}

static CVI_S32 gdc_test_cmdq_1to2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	CVI_CHAR *filename_in[1] = {GDC_FILE_IN_CMDQ_1TO2};
	CVI_CHAR *filename_out[1] = {GDC_FILE_OUT_CMDQ_1TO2_0};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	CVI_S32 times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 2; cnt ++) {
		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_SYS_Init failed!\n");
			return s32Ret;
		}
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);

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

		SAMPLE_PRT("common pool[0] BlkSize %d\n", BlkSize);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VB_SetConf failed!\n");
			goto exit0;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VB_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_GDC_Init();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_GDC_Init failed!\n");
			goto exit1;
		}

		times = GDC_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat
				, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				SAMPLE_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				SAMPLE_PRT("GDC_COMM_PrepareFrame out 1st failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			strcpy(name, param.stTask.name);
			snprintf(param.stTask.name, sizeof(param.stTask.name), "%.*s_1st_%d",
					(int)(sizeof(param.stTask.name) - 7), name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//1to2
			memset(&stVideoFrameOut_tmp, 0, sizeof(stVideoFrameOut_tmp));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp);
			if (s32Ret) {
				SAMPLE_PRT("GDC_COMM_PrepareFrame out 2nd failed!\n");
				goto exit2;
			}

			memset(&stTask_tmp, 0, sizeof(stTask_tmp));
			memcpy(&stTask_tmp.stImgIn, &param.stVideoFrameIn, sizeof(VIDEO_FRAME_INFO_S));
			memcpy(&stTask_tmp.stImgOut, &stVideoFrameOut_tmp, sizeof(VIDEO_FRAME_INFO_S));
			snprintf(stTask_tmp.name, sizeof(param.stTask.name), "%.*s_2nd_%d",
					(int)(sizeof(param.stTask.name) - 7), name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_tmp, ROTATION_0);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				SAMPLE_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (b_gdc_save_file) {
				if (SAMPLE_COMM_FRAME_SaveToFile(param.filename_out, &param.stVideoFrameOut) != CVI_SUCCESS) {
					SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile s32Ret: 0x%x !\n", s32Ret);
				}
				SAMPLE_PRT("output file:%s\n", param.filename_out);
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
				SAMPLE_PRT("release VB fail.\n");
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
			SAMPLE_PRT("CVI_GDC_DeInit fail.\n");
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
	return s32Ret;
}

static CVI_S32 gdc_test_async(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param = {0};
	CVI_CHAR *filename_in[1] = {GDC_FILE_IN_ROT};
	CVI_CHAR *filename_out[1] = {GDC_FILE_OUT_ROT0};
	CVI_U8 times = GDC_REPEAT_TIMES * 10;
	CVI_U32 BlkSize;

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	strcpy(param.filename_in, filename_in[0]);
	strcpy(param.filename_out, filename_out[0]);
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

	SAMPLE_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		goto exit0;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	do {
		param.hHandle = 0;
		memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat
			, param.filename_in, &param.stVideoFrameIn);
		if (s32Ret) {
			SAMPLE_PRT("GDCFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat
			, &param.stVideoFrameOut);
		if (s32Ret) {
			SAMPLE_PRT("GDC_COMM_PrepareFrame failed!\n");
			goto exit2;
		}

		memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
		memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
		memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param.hHandle);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_EndJob(param.hHandle);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_EndJob failed!\n");
			goto exit2;
		}

		if (needSuspend) {
			s32Ret = CVI_GDC_Suspend();
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_GDC_Suspend fail. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}
			s32Ret = CVI_GDC_Resume();
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_GDC_Resume fail. s32Ret: 0x%x !\n", s32Ret);
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
			SAMPLE_PRT("release VB fail.\n");
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
		SAMPLE_PRT("CVI_GDC_DeInit fail.\n");
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

		SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[1]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n"
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[1]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		//if (b_gdc_save_file) {
		if (0) {
			s32Ret = SAMPLE_COMM_FRAME_SaveToFile(param->filename_out, &param->stVideoFrameOut);
			if (s32Ret) {
				SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile fail\n");
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
			SAMPLE_PRT("GDCFileToFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
		if (s32Ret) {
			SAMPLE_PRT("GDC_COMM_PrepareFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param->hHandle);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_BeginJob failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_SetJobIdentity failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = gdc_basic_add_tsk(param, op_ptr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_GDC_EndJob(param->hHandle);
		if (s32Ret) {
			SAMPLE_PRT("CVI_GDC_EndJob failed!\n");
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
		SAMPLE_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			CVI_GDC_CancelJob(param->hHandle);
	CVI_GDC_FreeCurTaskMesh(param->stTask.name);

	pthread_exit(0);
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
		SAMPLE_PRT("Test failed.\n");
		return s32Ret;
	}

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

static CVI_S32 _gdc_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case GDC_TEST_ROT:
		s32Ret = gdc_test_rot();
		break;
	case GDC_TEST_LDC:
		s32Ret = gdc_test_ldc();
		break;
	case GDC_TEST_LDC_LOAD_MESH:
		s32Ret = gdc_test_ldc_load_mesh_scaling();
		break;
	case GDC_TEST_CMDQ:
		s32Ret = gdc_test_cmdq();
		break;
	case GDC_TEST_CMDQ_1TO2:
		s32Ret = gdc_test_cmdq_1to2();
		break;
	case GDC_TEST_ASYNC:
		s32Ret = gdc_test_async();
		break;
	case GDC_TEST_LOAD_GRID_INFO:
		s32Ret = gdc_test_ldc_grid_info();
		break;
	case GDC_TEST_RUN_SUSPEND:
		s32Ret = gdc_test_running_suspend();
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static void gdc_show_help(void)
{
	SAMPLE_PRT("%4d: gdc basic test rot\n", GDC_TEST_ROT);
	SAMPLE_PRT("%4d: gdc basic test ldc\n", GDC_TEST_LDC);
	SAMPLE_PRT("%4d: gdc basic test ldc load mesh\n", GDC_TEST_LDC_LOAD_MESH);
	SAMPLE_PRT("%4d: gdc basic test cmdq\n", GDC_TEST_CMDQ);
	SAMPLE_PRT("%4d: gdc basic test cmdq_1to2\n", GDC_TEST_CMDQ_1TO2);
	SAMPLE_PRT("%4d: gdc basic test async\n", GDC_TEST_ASYNC);
	SAMPLE_PRT("%4d: gdc basic test grid_info\n", GDC_TEST_LOAD_GRID_INFO);
	SAMPLE_PRT("%4d: gdc basic test running suspend\n", GDC_TEST_RUN_SUSPEND);

	SAMPLE_PRT("255: exit\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR **argv)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 op = 255;
	CVI_S32 MAX_OP = 7;
	b_gdc_save_file = CVI_TRUE;

	system("stty erase ^H");

	signal(SIGINT, SAMPLE_GDC_HandleSig);
	signal(SIGTERM, SAMPLE_GDC_HandleSig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);

		if (op < 0 || op > MAX_OP) {
            SAMPLE_PRT("Error: invalid operation code %d. Valid range is 0 to %d.\n", op, MAX_OP);
            return CVI_FAILURE;
        }

		s32Ret = _gdc_handle_op(op);
		SAMPLE_PRT("gdc ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			gdc_show_help();
			scanf("%d", &op);

			if (op != 255 && (op < 0 || op > MAX_OP)) {
				SAMPLE_PRT("Error: invalid operation code %d. Valid range is 0 to %d.\n", op, MAX_OP);
				continue;
        	}

			s32Ret = _gdc_handle_op(op);
			if (op != 255)
				SAMPLE_PRT("gdc ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}
