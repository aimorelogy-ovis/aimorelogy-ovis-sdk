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
#include "cvi_vpss.h"
#include "cvi_gdc.h"
#include "vpss_ut_comm.h"

#define DEFAULT_W 1920
#define DEFAULT_H 1080
#define DEFAULT_FBCTABLE_LENGTH 69632

#define THREAD_CNT 4

#ifndef FPGA_PORTING
#define UT_TIMEOUT_MS 1000
#define TEST_CNT0 1000
#define TEST_CNT1 1000
#else
#define UT_TIMEOUT_MS 60000
#define TEST_CNT0 100
#define TEST_CNT1 100
#endif

#define RANDOM(min, max) ((rand() % ((max) - (min) + 1)) + (min))

//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB
#define VPSS_DEFAULT_FILE_IN      "res/1080p.yuv420"
#define VPSS_4K_FILE_IN           "res/3840_2160.rgb"
#define VPSS_RGB_FILE_IN          "res/1080p.rgb"
#define VPSS_ROT_FILE_IN          "res/ldc/input/1920x1080.yuv"
#define VPSS_LDC_FILE_IN          "res/ldc/input/1920x1080_barrel_0.3.yuv"
#define VPSS_LDC_MESH_FILE        "res/vpss_mesh_MW.bin"
#define VPSS_LDC_DUMP_MESH_FILE   "res/dump_vpss_mesh_MW.bin"

#define MD5_BASIC             "80c8c14564ccae2c44b7f3d0c735e29c"
#define MD5_1_TO_2_CHN0       "80c8c14564ccae2c44b7f3d0c735e29c"
#define MD5_1_TO_2_CHN1       "baa7313b776dace762f8de90f8a64b75"
#define MD5_1_TO_3_CHN0       "29a45e41ff051d910f53dabd8c65e2c5"
#define MD5_1_TO_3_CHN1       "f028e69e4f31f05038f3b282dd0b9e98"
#define MD5_1_TO_3_CHN2       "7fbf0e71649cccc3122f6f90d9c6739d"
#define MD5_1_TO_4_CHN0       "29a45e41ff051d910f53dabd8c65e2c5"
#define MD5_1_TO_4_CHN1       "a1195e1dfd26d69bb45841bd6790af5c"
#define MD5_1_TO_4_CHN2       "08c60fd79857ddc020d4ba2aaa40b4a5"
#define MD5_1_TO_4_CHN3       "7fbf0e71649cccc3122f6f90d9c6739d"
#define MD5_MAX_RES           "efc8f6085dc1ceee22b337364ff8f397"
#define MD5_MIRROR            "911638a8e60676502a735479ca1c2bd7"
#define MD5_FLIP              "3884b64d0a54dc16f56f5721de8e5976"
#define MD5_MIRROR_FLIP       "bd812274c3345802d5d00c6464a2ac86"
#define MD5_ASPECT_RATIO1     "5a1ab5d03b756a46b043bcab27dd5992"
#define MD5_ASPECT_RATIO2     "5cad2931d027ea54a68c5a19264c823b"
#define MD5_ASPECT_RATIO3     "43b2c0658542ae2225807ded42fa8e1f"
#define MD5_BYTE_ALIGN        "6be8e0485840a247a0ee5151537584ab"
#define MD5_DRAW_RECT         "713375731d912d0ca2f6c607972ff346"
#define MD5_AMP_BRIGHTNESS    "d5bec14ea6cb14a7aaa142f700d420ae"
#define MD5_AMP_CONTRAST      "67e41eb6e5a6822f859a7d52c5633520"
#define MD5_AMP_SATURATION    "33ed93964e726cff6086bb2b975f186f"
#define MD5_AMP_HUE           "7e19a144493d2ecf9fbb5a78ca7a36a3"
#define MD5_NORMALIZE         "47c16ab4426300b684bc16fe0e8f6091"
#define MD5_CONVERT           "3cf37a3a754c484bb6a0068fddb99daa"
#define MD5_SCALE_COEF1       "4a7e481f7b99f26dca4c71480f7204a9"
#define MD5_SCALE_COEF2       "d491434afb334bf884492b2f8f965023"
#define MD5_SCALE_COEF3       "2790ec4598b2af053f70f5a5bd11e86e"
#define MD5_SCALE_COEF4       "8044202c7a0f8c6d5d4639cb76ba6036"
#define MD5_Y_RATIO           "6aae1a315aea80464d0bc9173c109de6"
#define MD5_HIDE              "f07d0b060e007107c339f63652210453"

#define OUT_FILE_PREFIX           "./out"

typedef struct _VPSS_BASIC_TEST_PARAM {
	VPSS_GRP VpssGrp;
	SIZE_S stSizeIn;
	SIZE_S stSizeOut;
	CVI_BOOL bMirror;
	CVI_BOOL bFlip;
	CVI_BOOL bHide;
	PIXEL_FORMAT_E enFormatIn;
	PIXEL_FORMAT_E enFormatOut;
	ASPECT_RATIO_S stAspectRatio;
	VPSS_NORMALIZE_S stNormalize;
	VPSS_CROP_INFO_S stGrpCropInfo;
	VPSS_CROP_INFO_S stChnCropInfo;
	VPSS_DRAW_RECT_S stDrawRect;
	VPSS_CONVERT_S stConvert;
	VPSS_LDC_ATTR_S stLDCAttr;
	CVI_BOOL bUseLoadMesh;
	ROTATION_E enRotation;
	VPSS_SCALE_COEF_E enCoef;
	CVI_U32 u32ChnAlign;
	CVI_FLOAT YRatio;
	CVI_U32 u32CheckSum;
	CVI_U32 u32FbcTableLength;
	CVI_CHAR aszMD5Sum[33];
	CVI_CHAR aszFileNameIn[64];
	CVI_CHAR aszFileNameFbcIn[4][64];
	CVI_CHAR aszFileNameOut[64];
	CVI_CHAR aszFileNameRef[64];
} VPSS_BASIC_TEST_PARAM;

struct VPSS_CHN_PARAM {
	CVI_BOOL bEnable;
	VPSS_CHN VpssChn;
	SIZE_S stSizeOut;
	CVI_BOOL bMirror;
	CVI_BOOL bFlip;
	PIXEL_FORMAT_E enFormatOut;
	ASPECT_RATIO_S stAspectRatio;
	VPSS_NORMALIZE_S stNormalize;
	CVI_U32 u32CheckSum;
	CVI_CHAR aszMD5Sum[33];
	CVI_CHAR aszFileNameOut[64];
	CVI_CHAR aszFileNameRef[64];
};

typedef struct _VPSS_MULTI_TEST_PARAM {
	VPSS_GRP VpssGrp;
	SIZE_S stSizeIn;
	PIXEL_FORMAT_E enFormatIn;
	CVI_CHAR aszFileNameIn[64];
	struct VPSS_CHN_PARAM astChnParam[VPSS_MAX_CHN_NUM];
} VPSS_MULTI_TEST_PARAM;

typedef enum _VPSS_TEST_OP {
	VPSS_TEST_BASIC = 0,
	VPSS_TEST_1_to_2,
	VPSS_TEST_1_to_3,
	VPSS_TEST_1_to_4,
	VPSS_TEST_DUAL_MODE,
	VPSS_TEST_MAX_RES, // 5
	VPSS_TEST_MIRROR,
	VPSS_TEST_FLIP,
	VPSS_TEST_MIRROR_FLIP,
	VPSS_TEST_ASPECT_RATIO,
	VPSS_TEST_MULTI_GRP, // 10
	VPSS_TEST_MULTI_THREAD,
	VPSS_TEST_BYTE_ALIGN,
	VPSS_TEST_RESIZE,
	VPSS_TEST_MAX_SCALING,
	VPSS_TEST_DRAW_RECT, // 15
	VPSS_TEST_FORMAT,
	VPSS_TEST_GRP_CROP,
	VPSS_TEST_CHN_CROP,
	VPSS_TEST_AMP_CTRL,
	VPSS_TEST_NORMALIZE, // 20
	VPSS_TEST_CONVERT_TO,
	VPSS_TEST_SCALE_COEF,
	VPSS_TEST_Y_RATIO,
	VPSS_TEST_HIDE,
	VPSS_TEST_ROT, // 25
	VPSS_TEST_LDC,
	VPSS_TEST_LDC_LOAD_MESH,
	VPSS_TEST_PRESSURE,
	VPSS_TEST_PERF,
	VPSS_TEST_MP_GET_CHN_FRM, // 30
	VPSS_TEST_C_MODEL,
	VPSS_TEST_AUTO = 99,
	VPSS_TEST_USER_CONFIG = 100,
} VPSS_TEST_OP;

static pthread_mutex_t s_SyncMutex = PTHREAD_MUTEX_INITIALIZER;
static CVI_U32 s_u32Flag;

static CVI_VOID vpss_ut_handle_sig(CVI_S32 signo)
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

static CVI_S32 basic(const VPSS_BASIC_TEST_PARAM *pTestParam)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = pTestParam->VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_BOOL bFlag = CVI_FALSE;
	CVI_BOOL bSaveFile = CVI_TRUE;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};
	MESH_DUMP_ATTR_S MeshDumpAttr = {0};
	VI_VPSS_MODE_S stViVpssMode = {0};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(pTestParam->stSizeIn.u32Width, pTestParam->stSizeIn.u32Height,
		pTestParam->enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(pTestParam->stSizeOut.u32Width, pTestParam->stSizeOut.u32Height,
		pTestParam->enFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1 + ((pTestParam->stLDCAttr.bEnable) ? 1 : 0);
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1 + (pTestParam->stLDCAttr.bEnable || pTestParam->enRotation ? 1 : 0);
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

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

	stViVpssMode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE;
	stViVpssMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE;
	s32Ret = CVI_SYS_SetVIVPSSMode(&stViVpssMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_SetVIVPSSMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pTestParam->enFormatIn;
	stVpssGrpAttr.u32MaxW			     = pTestParam->stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = pTestParam->stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	if (pTestParam->stLDCAttr.bEnable && pTestParam->stLDCAttr.stAttr.enRotation != 0) {
		stVpssChnAttr.u32Width		    = ALIGN(pTestParam->stSizeIn.u32Width, DEFAULT_ALIGN);
		stVpssChnAttr.u32Height		    = ALIGN(pTestParam->stSizeIn.u32Height, DEFAULT_ALIGN);
	} else {
		stVpssChnAttr.u32Width		    = pTestParam->stSizeOut.u32Width;
		stVpssChnAttr.u32Height		    = pTestParam->stSizeOut.u32Height;
	}

	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = pTestParam->enFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= pTestParam->bMirror;
	stVpssChnAttr.bFlip				= pTestParam->bFlip;
	stVpssChnAttr.stAspectRatio		= pTestParam->stAspectRatio;
	stVpssChnAttr.stNormalize		= pTestParam->stNormalize;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//grp crop
	if (pTestParam->stGrpCropInfo.bEnable) {
		s32Ret = CVI_VPSS_SetGrpCrop(VpssGrp, &pTestParam->stGrpCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetGrpCrop failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn crop
	if (pTestParam->stChnCropInfo.bEnable) {
		s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &pTestParam->stChnCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnCrop failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn Draw rect
	bFlag = CVI_FALSE;
	for (i = 0; i < VPSS_RECT_NUM; i++)
		if (pTestParam->stDrawRect.astRect[i].bEnable)
			bFlag = CVI_TRUE;
	if (bFlag) {
		s32Ret = CVI_VPSS_SetChnDrawRect(VpssGrp, VpssChn, &pTestParam->stDrawRect);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnDrawRect failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn Convert to
	if (pTestParam->stConvert.bEnable) {
		s32Ret = CVI_VPSS_SetChnConvert(VpssGrp, VpssChn, &pTestParam->stConvert);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnConvert failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn LDC
	if (pTestParam->stLDCAttr.bEnable) {
		if (pTestParam->bUseLoadMesh) {
			strcpy(MeshDumpAttr.binFileName, VPSS_LDC_MESH_FILE);
			MeshDumpAttr.enModId = CVI_ID_VPSS;
			MeshDumpAttr.vpssMeshAttr.grp = VpssGrp;
			MeshDumpAttr.vpssMeshAttr.chn = VpssChn;

			s32Ret = CVI_GDC_LoadMesh(&MeshDumpAttr, &pTestParam->stLDCAttr.stAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_GDC_DumpMesh failed with %#x\n", s32Ret);
				goto exit4;
			}
		} else {
			s32Ret = CVI_VPSS_SetChnLDCAttr(VpssGrp, VpssChn, &pTestParam->stLDCAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_SetChnLDCAttr failed with %#x\n", s32Ret);
				goto exit4;
			}

			strcpy(MeshDumpAttr.binFileName, VPSS_LDC_DUMP_MESH_FILE);
			MeshDumpAttr.enModId = CVI_ID_VPSS;
			MeshDumpAttr.vpssMeshAttr.grp = VpssGrp;
			MeshDumpAttr.vpssMeshAttr.chn = VpssChn;

			s32Ret = CVI_GDC_DumpMesh(&MeshDumpAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_GDC_DumpMesh failed with %#x\n", s32Ret);
				goto exit4;
			}
		}
	}

	//chn rotation
	if (pTestParam->enRotation != ROTATION_0) {
		s32Ret = CVI_VPSS_SetChnRotation(VpssGrp, VpssChn, pTestParam->enRotation);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnRotation failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn coef
	if (pTestParam->enCoef != VPSS_SCALE_COEF_BICUBIC) {
		s32Ret = CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, pTestParam->enCoef);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnScaleCoefLevel failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn align
	if (pTestParam->u32ChnAlign > 0) {
		s32Ret = CVI_VPSS_SetChnAlign(VpssGrp, VpssChn, pTestParam->u32ChnAlign);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAlign failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn YRatio
	if (pTestParam->YRatio > 0) {
		s32Ret = CVI_VPSS_SetChnYRatio(VpssGrp, VpssChn, pTestParam->YRatio);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnYRatio failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn hide
	if (pTestParam->bHide) {
		s32Ret = CVI_VPSS_HideChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_HideChn failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &pTestParam->stSizeIn,
		pTestParam->enFormatIn, pTestParam->aszFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	UT_PRT("***CVI_VPSS_GetChnFrame Success***\n");

	if (pTestParam->aszMD5Sum[0]) {
		if (CompareWithMD5(pTestParam->aszMD5Sum, &stVideoFrame)) {
			bSaveFile = CVI_TRUE;
			s32Ret = CVI_FAILURE;
			UT_PRT("Compare MD5 fail, MD5:%s\n", pTestParam->aszMD5Sum);
		} else {
			bSaveFile = CVI_FALSE;
		}
	}

	if (bSaveFile && pTestParam->aszFileNameOut[0]) {
		if (FrameSaveToFile(pTestParam->aszFileNameOut, &stVideoFrame)) {
			UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
			goto exit4;
		}
		UT_PRT("output file:%s\n", pTestParam->aszFileNameOut);
	}

	if (pTestParam->aszFileNameRef[0]) {
		if (CompareWithFile(pTestParam->aszFileNameRef, &stVideoFrame)) {
			UT_PRT("CompareWithFile fail.\n");
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
			s32Ret = CVI_FAILURE;
			goto exit4;
		}
	}

	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 basic_mutli_chn(VPSS_MULTI_TEST_PARAM *pTestParam)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, n;
	VPSS_GRP VpssGrp = pTestParam->VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame;
	struct VPSS_CHN_PARAM *pstChnParam;
	CVI_BOOL bSaveFile = CVI_TRUE;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(pTestParam->stSizeIn.u32Width, pTestParam->stSizeIn.u32Height,
		pTestParam->enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	n = 1;
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		u32BlkSizeOut = COMMON_GetPicBufferSize(pstChnParam->stSizeOut.u32Width,
					pstChnParam->stSizeOut.u32Height, pstChnParam->enFormatOut,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		stVbConf.astCommPool[n].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[n].u32BlkCnt	= 1;
		stVbConf.astCommPool[n].enRemapMode	= VB_REMAP_MODE_CACHED;
		UT_PRT("common pool[%d] BlkSize %d\n", n, u32BlkSizeOut);
		n++;
	}
	stVbConf.u32MaxPoolCnt = n;

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pTestParam->enFormatIn;
	stVpssGrpAttr.u32MaxW			     = pTestParam->stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = pTestParam->stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	n = 1;
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		VpssChn = i;
		stVpssChnAttr.u32Width		    = pstChnParam->stSizeOut.u32Width;
		stVpssChnAttr.u32Height		    = pstChnParam->stSizeOut.u32Height;
		stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
		stVpssChnAttr.enPixelFormat		    = pstChnParam->enFormatOut;
		stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
		stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
		stVpssChnAttr.u32Depth			= 1;
		stVpssChnAttr.bMirror			= pstChnParam->bMirror;
		stVpssChnAttr.bFlip				= pstChnParam->bFlip;
		stVpssChnAttr.stAspectRatio		= pstChnParam->stAspectRatio;
		stVpssChnAttr.stNormalize		= pstChnParam->stNormalize;
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAttr failed with %#x, chn(%d)\n", s32Ret, i);
			goto exit2;
		}

		s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, n);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_EnableChn failed with %#x, chn(%d)\n", s32Ret, i);
			goto exit2;
		}
		n++;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit2;
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &pTestParam->stSizeIn,
		pTestParam->enFormatIn, pTestParam->aszFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit3;
	}

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		VpssChn = i;
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Grp(%d) Chn(%d), CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
			goto exit3;
		}
		UT_PRT("***Grp(%d) Chn(%d) CVI_VPSS_GetChnFrame Success***\n", VpssGrp, VpssChn);

		if (pstChnParam->aszMD5Sum[0]) {
			if (CompareWithMD5(pstChnParam->aszMD5Sum, &stVideoFrame)) {
				bSaveFile = CVI_TRUE;
				s32Ret = CVI_FAILURE;
				UT_PRT("chn%d Compare MD5 fail, MD5:%s\n",
					i, pstChnParam->aszMD5Sum);
			} else {
				bSaveFile = CVI_FALSE;
			}
		}

		if (bSaveFile && pstChnParam->aszFileNameOut[0]) {
			if (FrameSaveToFile(pstChnParam->aszFileNameOut, &stVideoFrame)) {
				UT_PRT("Grp(%d) Chn(%d),FrameSaveToFile. s32Ret: 0x%x !\n",
					VpssGrp, VpssChn, s32Ret);
			}
			UT_PRT("output file:%s\n", pstChnParam->aszFileNameOut);
		}

		if (pstChnParam->aszFileNameRef[0]) {
			if (CompareWithFile(pstChnParam->aszFileNameRef, &stVideoFrame)) {
				UT_PRT("Grp(%d) Chn(%d),CompareWithFile fail.\n", VpssGrp, VpssChn);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
				s32Ret = CVI_FAILURE;
				goto exit3;
			}
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);

		if (s32Ret)
			break;
	}

exit3:
	CVI_VPSS_StopGrp(VpssGrp);
exit2:
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		if (!pTestParam->astChnParam[i].bEnable)
			continue;
		CVI_VPSS_DisableChn(VpssGrp, i);
	}
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_basic(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x13030706;
	strncpy(stTestParam.aszMD5Sum, MD5_BASIC, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_1_to_2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32ChnNum = 2;
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));

	for (i = 0; i < s32ChnNum; i++) {
		stTestParam.astChnParam[i].bEnable = CVI_TRUE;
		stTestParam.astChnParam[i].VpssChn = i;
		stTestParam.astChnParam[i].stSizeOut.u32Width = DEFAULT_W;
		stTestParam.astChnParam[i].stSizeOut.u32Height = DEFAULT_H;
		stTestParam.astChnParam[i].bMirror = CVI_FALSE;
		stTestParam.astChnParam[i].bFlip = CVI_FALSE;
		stTestParam.astChnParam[i].enFormatOut = (i == 0) ? PIXEL_FORMAT_NV21 : PIXEL_FORMAT_RGB_888;
		stTestParam.astChnParam[i].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stTestParam.astChnParam[i].stNormalize.bEnable = CVI_FALSE;
		snprintf(stTestParam.astChnParam[i].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
			OUT_FILE_PREFIX, __func__, i,
			stTestParam.astChnParam[i].stSizeOut.u32Width,
			stTestParam.astChnParam[i].stSizeOut.u32Height,
			GetFmtName(stTestParam.astChnParam[i].enFormatOut));
	}

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, MD5_1_TO_2_CHN0,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, MD5_1_TO_2_CHN1,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	stTestParam.astChnParam[0].u32CheckSum = 0x202f50f7;
	stTestParam.astChnParam[1].u32CheckSum = 0x4d474279;
	s32Ret = basic_mutli_chn(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_1_to_3(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32ChnNum = 3;
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));

	stTestParam.astChnParam[0].enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.astChnParam[1].enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.astChnParam[2].enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.astChnParam[0].stSizeOut.u32Width = DEFAULT_W;
	stTestParam.astChnParam[0].stSizeOut.u32Height = DEFAULT_H;
	stTestParam.astChnParam[1].stSizeOut.u32Width = 1280;
	stTestParam.astChnParam[1].stSizeOut.u32Height = 720;
	stTestParam.astChnParam[2].stSizeOut.u32Width = 640;
	stTestParam.astChnParam[2].stSizeOut.u32Height = 480;

	for (i = 0; i < s32ChnNum; i++) {
		stTestParam.astChnParam[i].bEnable = CVI_TRUE;
		stTestParam.astChnParam[i].VpssChn = i;
		stTestParam.astChnParam[i].bMirror = CVI_FALSE;
		stTestParam.astChnParam[i].bFlip = CVI_FALSE;
		stTestParam.astChnParam[i].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stTestParam.astChnParam[i].stNormalize.bEnable = CVI_FALSE;
		snprintf(stTestParam.astChnParam[i].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
			OUT_FILE_PREFIX, __func__, i,
			stTestParam.astChnParam[i].stSizeOut.u32Width,
			stTestParam.astChnParam[i].stSizeOut.u32Height,
			GetFmtName(stTestParam.astChnParam[i].enFormatOut));
	}

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, MD5_1_TO_3_CHN0,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, MD5_1_TO_3_CHN1,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	strncpy(stTestParam.astChnParam[2].aszMD5Sum, MD5_1_TO_3_CHN2,
		sizeof(stTestParam.astChnParam[2].aszMD5Sum));

	stTestParam.astChnParam[0].u32CheckSum = 0x66aa69de;
	stTestParam.astChnParam[1].u32CheckSum = 0xabb84d27;
	stTestParam.astChnParam[2].u32CheckSum = 0x9ea313ad;
	s32Ret = basic_mutli_chn(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_1_to_4(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32ChnNum = 4;
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));

	stTestParam.astChnParam[0].enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.astChnParam[1].enFormatOut = PIXEL_FORMAT_HSV_888;
	stTestParam.astChnParam[2].enFormatOut = PIXEL_FORMAT_YUV_400;
	stTestParam.astChnParam[3].enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.astChnParam[0].stSizeOut.u32Width = DEFAULT_W;
	stTestParam.astChnParam[0].stSizeOut.u32Height = DEFAULT_H;
	stTestParam.astChnParam[1].stSizeOut.u32Width = DEFAULT_W;
	stTestParam.astChnParam[1].stSizeOut.u32Height = DEFAULT_H;
	stTestParam.astChnParam[2].stSizeOut.u32Width = 1280;
	stTestParam.astChnParam[2].stSizeOut.u32Height = 720;
	stTestParam.astChnParam[3].stSizeOut.u32Width = 640;
	stTestParam.astChnParam[3].stSizeOut.u32Height = 480;

	for (i = 0; i < s32ChnNum; i++) {
		stTestParam.astChnParam[i].bEnable = CVI_TRUE;
		stTestParam.astChnParam[i].VpssChn = i;
		stTestParam.astChnParam[i].bMirror = CVI_FALSE;
		stTestParam.astChnParam[i].bFlip = CVI_FALSE;
		stTestParam.astChnParam[i].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stTestParam.astChnParam[i].stNormalize.bEnable = CVI_FALSE;
		snprintf(stTestParam.astChnParam[i].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
			OUT_FILE_PREFIX, __func__, i,
			stTestParam.astChnParam[i].stSizeOut.u32Width,
			stTestParam.astChnParam[i].stSizeOut.u32Height,
			GetFmtName(stTestParam.astChnParam[i].enFormatOut));
	}

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, MD5_1_TO_4_CHN0,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, MD5_1_TO_4_CHN1,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	strncpy(stTestParam.astChnParam[2].aszMD5Sum, MD5_1_TO_4_CHN2,
		sizeof(stTestParam.astChnParam[2].aszMD5Sum));
	strncpy(stTestParam.astChnParam[3].aszMD5Sum, MD5_1_TO_4_CHN3,
		sizeof(stTestParam.astChnParam[3].aszMD5Sum));
	stTestParam.astChnParam[0].u32CheckSum = 0x66aa69de;
	stTestParam.astChnParam[1].u32CheckSum = 0x93fb6e1a;
	stTestParam.astChnParam[2].u32CheckSum = 0xafde8336;
	stTestParam.astChnParam[3].u32CheckSum = 0x9ea313ad;
	s32Ret = basic_mutli_chn(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_dual_mode(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp0 = 0;
	VPSS_GRP VpssGrp1 = 1;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VPSS_MODE_S stVPSSMode;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {DEFAULT_W, DEFAULT_H};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pFileNameIn = VPSS_DEFAULT_FILE_IN;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 6;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

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
	 * step2:  Init VPSS
	 ************************************************/
	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit0;
	}

	//grp0
	s32Ret = CVI_VPSS_CreateGrp(VpssGrp0, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp0, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp0, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp0, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	//grp1
	stVpssGrpAttr.u8VpssDev = 1;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp1, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp1, s32Ret);
		goto exit2;
	}

	for (VpssChn = 0; VpssChn < 3; VpssChn++) {
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp1, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit3;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp1, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit3;
		}
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp0);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	s32Ret = CVI_VPSS_StartGrp(VpssGrp1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	VIDEO_FRAME_INFO_S stVideoFrameIn;
	VIDEO_FRAME_INFO_S stVideoFrameOut;

	s32Ret = FileToFrame(&stSize, enPixelFormat, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit3;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp0, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SendFrame failed with %#x\n", s32Ret);
		goto exit4;
	}
	s32Ret = CVI_VPSS_SendFrame(VpssGrp1, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SendFrame failed with %#x\n", s32Ret);
		goto exit4;
	}

	VpssChn = 0;
	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp0, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("Grp(%d) Chn(%d), CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n",
			VpssGrp0, VpssChn, s32Ret);
		goto exit4;
	}
	UT_PRT("***Grp(%d) Chn(%d) CVI_VPSS_GetChnFrame Success***\n", VpssGrp0, VpssChn);
	CVI_VPSS_ReleaseChnFrame(VpssGrp0, VpssChn, &stVideoFrameOut);

	for (VpssChn = 0; VpssChn < 3; VpssChn++) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp1, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("Grp(%d) Chn(%d), CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n",
				VpssGrp1, VpssChn, s32Ret);
			goto exit4;
		}
		UT_PRT("***Grp(%d) Chn(%d) CVI_VPSS_GetChnFrame Success***\n", VpssGrp1, VpssChn);
		CVI_VPSS_ReleaseChnFrame(VpssGrp1, VpssChn, &stVideoFrameOut);
	}

exit4:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit3:
	CVI_VPSS_StopGrp(VpssGrp1);
	CVI_VPSS_DisableChn(VpssGrp1, 2);
	CVI_VPSS_DisableChn(VpssGrp1, 1);
	CVI_VPSS_DisableChn(VpssGrp1, 0);
	CVI_VPSS_DestroyGrp(VpssGrp1);
exit2:
	CVI_VPSS_StopGrp(VpssGrp0);
	CVI_VPSS_DisableChn(VpssGrp0, 0);
	CVI_VPSS_DestroyGrp(VpssGrp0);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_max_resolution(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = 3840;
	stTestParam.stSizeIn.u32Height = 2160;
	stTestParam.stSizeOut.u32Width = VPSS_MAX_IMAGE_WIDTH;
	stTestParam.stSizeOut.u32Height = VPSS_MAX_IMAGE_HEIGHT;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_RGB_888;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_400;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x63b7be2c;
	strncpy(stTestParam.aszMD5Sum, MD5_MAX_RES, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_4K_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_mirror(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_TRUE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0xca233e75;
	strncpy(stTestParam.aszMD5Sum, MD5_MIRROR, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_flip(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_TRUE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x6832c8c6;
	strncpy(stTestParam.aszMD5Sum, MD5_FLIP, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_mirror_flip(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_TRUE;
	stTestParam.bFlip = CVI_TRUE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x8a0831cd;
	strncpy(stTestParam.aszMD5Sum, MD5_MIRROR_FLIP, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_aspect_ratio(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
	stTestParam.stAspectRatio.bEnableBgColor = CVI_TRUE;
	stTestParam.stAspectRatio.u32BgColor = 0;
	stTestParam.stAspectRatio.stVideoRect.s32X = 64;
	stTestParam.stAspectRatio.stVideoRect.s32Y = 64;
	stTestParam.stAspectRatio.stVideoRect.u32Width = 1280;
	stTestParam.stAspectRatio.stVideoRect.u32Height = 720;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x1e28594b;
	strncpy(stTestParam.aszMD5Sum, MD5_ASPECT_RATIO1, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_1_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	//x,y odd
	stTestParam.stAspectRatio.stVideoRect.s32X = 65;
	stTestParam.stAspectRatio.stVideoRect.s32Y = 31;
	stTestParam.stAspectRatio.stVideoRect.u32Width = 1280;
	stTestParam.stAspectRatio.stVideoRect.u32Height = 720;
	stTestParam.u32CheckSum = 0x30186f67;
	strncpy(stTestParam.aszMD5Sum, MD5_ASPECT_RATIO2, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_2_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	//w,h odd
	stTestParam.stAspectRatio.stVideoRect.s32X = 64;
	stTestParam.stAspectRatio.stVideoRect.s32Y = 30;
	stTestParam.stAspectRatio.stVideoRect.u32Width = 1281;
	stTestParam.stAspectRatio.stVideoRect.u32Height = 721;
	stTestParam.u32CheckSum = 0xd6b83235;
	strncpy(stTestParam.aszMD5Sum, MD5_ASPECT_RATIO3, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_3_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_multi_grp(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	SIZE_S stSizeOut = {640, 480};
	PIXEL_FORMAT_E enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_NV21;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	CVI_S32 s32GrpNum = 16;
	CVI_CHAR *pFileNameIn = VPSS_DEFAULT_FILE_IN;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height,
		enFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= s32GrpNum;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSizeOut.u32Width;
	stVpssChnAttr.u32Height		    = stSizeOut.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode	= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable	= CVI_FALSE;

	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
			goto exit1;
		}

		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit1;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit1;
		}

		/*start vpss*/
		s32Ret = CVI_VPSS_StartGrp(VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
			goto exit1;
		}
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit1;
	}
	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SendFrame fail.\n");
			CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
			goto exit1;
		}
	}
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));

	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit1;
		}
		UT_PRT("***Grp(%d) Chn(%d), CVI_VPSS_GetChnFrame Success***\n", VpssGrp, VpssChn);

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_ReleaseChnFrame for Grp(%d) Chn(%d). s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
			goto exit1;
		}
	}

exit1:
	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		CVI_VPSS_StopGrp(VpssGrp);
		CVI_VPSS_DisableChn(VpssGrp, VpssChn);
		CVI_VPSS_DestroyGrp(VpssGrp);
	}
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_VOID *multi_thread_run(CVI_VOID *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32Repeat = TEST_CNT0, s32AvgFrameRate;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VIDEO_FRAME_INFO_S stVideoFrameOut, stVideoFrameIn;
	PIXEL_FORMAT_E enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_NV21;
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	CVI_CHAR *pFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_U64 u64PTS, u64StartPTS, u64EndPTS, u64CurPTS1, u64CurPTS2, u64CostTime;
	CVI_U32 u32FrameCnt;
	CVI_U64 u64MinCostTime = 10000, u64MaxCostTime = 0;

	UNUSED(arg);

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = enFormatIn;
	stVpssGrpAttr.u32MaxW = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width = DEFAULT_W;
	stVpssChnAttr.u32Height = DEFAULT_H;
	stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat = enFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth = 1;
	stVpssChnAttr.bMirror = CVI_FALSE;
	stVpssChnAttr.bFlip = CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable = CVI_FALSE;

	VpssGrp = CVI_VPSS_GetAvailableGrp();
	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return NULL;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit0;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit0;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit0;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit1;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	CVI_SYS_GetCurPTS(&u64PTS);
	u32FrameCnt = 0;
	u64StartPTS = u64PTS;

	for (i = 0; i < s32Repeat; i++) {
		CVI_SYS_GetCurPTS(&u64CurPTS1);
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit3;
		}

		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit3;
		}
		CVI_SYS_GetCurPTS(&u64CurPTS2);

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit3;
		}

		u32FrameCnt++;
		u64CostTime = u64CurPTS2 - u64CurPTS1;
		u64MinCostTime = u64CostTime < u64MinCostTime ? u64CostTime : u64MinCostTime;
		u64MaxCostTime = u64CostTime > u64MaxCostTime ? u64CostTime : u64MaxCostTime;

		if ((u64CurPTS2 - u64PTS) >= 1000000) {
			UT_PRT("[VpssGrp%d] FrameRate:%d fps\n", VpssGrp, u32FrameCnt);
			u32FrameCnt = 0;
			u64PTS = u64CurPTS2;
		}

	}
	CVI_SYS_GetCurPTS(&u64EndPTS);
	s32AvgFrameRate = s32Repeat / ((u64EndPTS - u64StartPTS) / 1000000);
	UT_PRT("\n[VpssGrp%d] 1080P cost time: Min-Max: (%"PRIu64", %"PRIu64")us,"
		"offset=%"PRIu64"\n", VpssGrp,
		u64MinCostTime, u64MaxCostTime, u64MaxCostTime - u64MinCostTime);
	UT_PRT("\n[VpssGrp%d] average FrameRate:%d fps\n", VpssGrp, s32AvgFrameRate);

exit3:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit2:
	CVI_VPSS_StopGrp(VpssGrp);
exit1:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit0:
	CVI_VPSS_DestroyGrp(VpssGrp);

	if (s32Ret == CVI_SUCCESS) {
		pthread_mutex_lock(&s_SyncMutex);
		s_u32Flag |= BIT(VpssGrp);
		pthread_mutex_unlock(&s_SyncMutex);
	}

	return NULL;
}

static CVI_S32 vpss_test_multi_thread(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	CVI_S32 s32ThreadNum = THREAD_CNT;
	pthread_t thread[THREAD_CNT] = {[0 ... THREAD_CNT - 1] = 0};
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(DEFAULT_W, DEFAULT_H, PIXEL_FORMAT_YUV_PLANAR_420,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(DEFAULT_W, DEFAULT_H, PIXEL_FORMAT_NV21,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt				= 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= s32ThreadNum;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= s32ThreadNum;
	stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

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

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	s_u32Flag = 0;

	for (i = 0; i < s32ThreadNum; i++) {
		s32Ret = pthread_create(&thread[i], NULL, multi_thread_run, NULL);
		if (s32Ret < 0) {
			UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < s32ThreadNum; i++)
		if (thread[i] != 0)
			pthread_join(thread[i], NULL);

	sleep(1);

	for (i = 0; i < s32ThreadNum; i++)
		if (!(s_u32Flag & BIT(i)))
			s32Ret = CVI_FAILURE;

exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_byte_align(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = 642;
	stTestParam.stSizeOut.u32Height = 480;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32ChnAlign = 1;
	stTestParam.u32CheckSum = 0x6f16a1b1;
	strncpy(stTestParam.aszMD5Sum, MD5_BYTE_ALIGN, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_resize(CVI_VOID)
{
	CVI_S32 i, j, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSizeIn = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};
	CVI_S32 s32MinWidth = 16;
	CVI_S32 s32MaxWidth = 3840;
	CVI_S32 s32MinHeight = 16;
	CVI_S32 s32MaxHeight = 2160;
#ifndef FPGA_PORTING
	CVI_S32 s32Step = 32;
#else
	CVI_S32 s32Step = 256;
#endif
	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(3840, 3840,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSizeIn.u32Width;
	stVpssChnAttr.u32Height		    = stSizeIn.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	for (i = s32MinWidth; i <= s32MaxWidth; i += s32Step) {
		for (j = s32MinHeight; j <= s32MaxHeight; j += s32Step) {
			stVpssChnAttr.u32Width = i;
			stVpssChnAttr.u32Height = j;
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				goto exit5;
			}
			s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_SendFrame fail.\n");
				goto exit5;
			}
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
				UT_PRT("output: w=%d h=%d fail\n", i, i);
				goto exit5;
			}
			s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
				goto exit5;
			}
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_max_scaling(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame1, stVideoFrame2, stVideoFrame3;
	SIZE_S stSizeIn = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_RGB_888;
	CVI_CHAR *pstFileNameIn = VPSS_RGB_FILE_IN;
	CVI_S32 s32MaxWidth = 3840;
	CVI_S32 s32MaxHeight = 2160;
	CVI_S32 s32ScalingRatio = 128;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(s32MaxWidth, s32MaxHeight,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = s32MaxWidth;
	stVpssChnAttr.u32Height		    = s32MaxHeight;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &stSizeIn, enPixelFormat, pstFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame1, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	//1/128 scaling down
	stVpssGrpAttr.u32MaxW = s32MaxWidth;
	stVpssGrpAttr.u32MaxH = s32MaxHeight;
	s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetGrpAttr(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);
		goto exit4;
	}
	stVpssChnAttr.u32Width = (s32MaxWidth + s32ScalingRatio - 1) / s32ScalingRatio;
	stVpssChnAttr.u32Height = (s32MaxHeight + s32ScalingRatio - 1) / s32ScalingRatio;
	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);
		goto exit4;
	}
	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame1, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SendFrame fail.\n");
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);
		goto exit4;
	}
	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame2, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	UT_PRT("128 scaling down successful.\n");

	//128 scaling up
	stVpssGrpAttr.u32MaxW = (s32MaxWidth + s32ScalingRatio - 1) / s32ScalingRatio;
	stVpssGrpAttr.u32MaxH = (s32MaxHeight + s32ScalingRatio - 1) / s32ScalingRatio;
	s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetGrpAttr(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);
		goto exit4;
	}
	stVpssChnAttr.u32Width = s32MaxWidth;
	stVpssChnAttr.u32Height = s32MaxHeight;
	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);
		goto exit4;
	}
	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame2, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SendFrame fail.\n");
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);
		goto exit4;
	}
	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame3, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame3);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	UT_PRT("128 scaling up successful.\n");

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_draw_rect(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.stDrawRect.astRect[0].bEnable = CVI_TRUE;
	stTestParam.stDrawRect.astRect[0].u32BgColor = 0xffff;
	stTestParam.stDrawRect.astRect[0].u16Thick = 6;
	stTestParam.stDrawRect.astRect[0].stRect.s32X = 96;
	stTestParam.stDrawRect.astRect[0].stRect.s32Y = 96;
	stTestParam.stDrawRect.astRect[0].stRect.u32Width = 308;
	stTestParam.stDrawRect.astRect[0].stRect.u32Height = 208;
	for (CVI_S32 i = 1; i < VPSS_RECT_NUM; i++) {
		memcpy(&stTestParam.stDrawRect.astRect[i], &stTestParam.stDrawRect.astRect[0],
			sizeof(stTestParam.stDrawRect.astRect[0]));
		stTestParam.stDrawRect.astRect[i].stRect.s32X += i * 200;
		stTestParam.stDrawRect.astRect[i].stRect.s32Y += i * 64;
	}
	stTestParam.u32CheckSum = 0x143f04f7;
	strncpy(stTestParam.aszMD5Sum, MD5_DRAW_RECT, sizeof(stTestParam.aszMD5Sum));

	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_format(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 i, j;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut0, stVideoFrameOut1;
	SIZE_S stSize = {1920, 1080};
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	PIXEL_FORMAT_E fmt_in[] = {
		PIXEL_FORMAT_RGB_888,
		PIXEL_FORMAT_BGR_888,
		PIXEL_FORMAT_RGB_888_PLANAR,
		PIXEL_FORMAT_BGR_888_PLANAR,
		PIXEL_FORMAT_NV12,
		PIXEL_FORMAT_NV21,
		PIXEL_FORMAT_NV16,
		PIXEL_FORMAT_NV61,
		PIXEL_FORMAT_YUYV,
		PIXEL_FORMAT_YVYU,
		PIXEL_FORMAT_UYVY,
		PIXEL_FORMAT_VYUY,
		PIXEL_FORMAT_YUV_444,
		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_PLANAR_422,
		PIXEL_FORMAT_YUV_PLANAR_444,
		PIXEL_FORMAT_YUV_400,
	};
	PIXEL_FORMAT_E fmt_out[] = {
		PIXEL_FORMAT_RGB_888,
		PIXEL_FORMAT_BGR_888,
		PIXEL_FORMAT_RGB_888_PLANAR,
		PIXEL_FORMAT_BGR_888_PLANAR,
		PIXEL_FORMAT_NV12,
		PIXEL_FORMAT_NV21,
		PIXEL_FORMAT_NV16,
		PIXEL_FORMAT_NV61,
		PIXEL_FORMAT_YUYV,
		PIXEL_FORMAT_YVYU,
		PIXEL_FORMAT_UYVY,
		PIXEL_FORMAT_VYUY,
		PIXEL_FORMAT_YUV_444,
		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_PLANAR_422,
		PIXEL_FORMAT_YUV_PLANAR_444,
		PIXEL_FORMAT_YUV_400,
		PIXEL_FORMAT_HSV_888,
		PIXEL_FORMAT_HSV_888_PLANAR,
		PIXEL_FORMAT_FP32_C3_PLANAR,
		PIXEL_FORMAT_FP16_C3_PLANAR,
		PIXEL_FORMAT_BF16_C3_PLANAR,
		PIXEL_FORMAT_INT8_C3_PLANAR,
		PIXEL_FORMAT_UINT8_C3_PLANAR
	};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		PIXEL_FORMAT_FP32_C3_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSize, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	for (i = 0; i < ARRAY_SIZE(fmt_in); i++) {
		stVpssGrpAttr.enPixelFormat = enPixelFormat;
		s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetGrpAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		stVpssChnAttr.enPixelFormat = fmt_in[i];
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			UT_PRT("output fmt: %d\n", i);
			goto exit5;
		}

		stVpssGrpAttr.enPixelFormat = fmt_in[i];
		s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetGrpAttr failed with %#x\n", s32Ret);
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
			goto exit5;
		}
		for (j = 0; j < ARRAY_SIZE(fmt_out); j++) {
			stVpssChnAttr.enPixelFormat = fmt_out[j];
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameOut0, 1000);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_SendFrame fail.\n");
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut1, UT_TIMEOUT_MS);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
				UT_PRT("output fmt: %d\n", i);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut1);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 test_crop(CVI_BOOL isChn)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSizeIn = {1920, 1080};
	SIZE_S stSizeOut = {640, 480};
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enPixelFormatOut = PIXEL_FORMAT_RGB_888;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	VPSS_CROP_INFO_S stCropInfo;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height,
		enPixelFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSizeOut.u32Width;
	stVpssChnAttr.u32Height		    = stSizeOut.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enPixelFormatIn, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	srand((CVI_U32)time(NULL));

	for (i = 0; i <= 50; i++) {
		stCropInfo.stCropRect.s32X = RANDOM(0, stSizeIn.u32Width - 16);
		stCropInfo.stCropRect.s32Y = RANDOM(0, stSizeIn.u32Height - 16);
		stCropInfo.stCropRect.u32Width = RANDOM(16, stSizeIn.u32Width) & ~(0x1);
		stCropInfo.stCropRect.u32Height = RANDOM(16, stSizeIn.u32Height) & ~(0x1);
		if (((stCropInfo.stCropRect.s32X + stCropInfo.stCropRect.u32Width)
			> stSizeIn.u32Width)
			|| ((stCropInfo.stCropRect.s32Y + stCropInfo.stCropRect.u32Height)
			> stSizeIn.u32Height)) {
			i--;
			continue;
		}

		if (isChn)
			s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropInfo);
		else
			s32Ret = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			UT_PRT("crop fail,x=%d y=%d w=%d h=%d\n",
				stCropInfo.stCropRect.s32X, stCropInfo.stCropRect.s32Y,
				stCropInfo.stCropRect.u32Width, stCropInfo.stCropRect.u32Height);
			goto exit5;
		}
		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_grp_crop(CVI_VOID)
{
	CVI_S32 s32Ret = test_crop(CVI_FALSE);

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_chn_crop(CVI_VOID)
{
	CVI_S32 s32Ret = test_crop(CVI_TRUE);

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_amp_ctrl(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSize, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	PROC_AMP_E type;
	CVI_S32 value = 80;
	PROC_AMP_CTRL_S ctrl;
	CVI_CHAR FileName[64];
	CVI_CHAR *pSuffix[PROC_AMP_MAX] = {"brightness", "contrast", "saturation", "hue"};
	CVI_CHAR *pszMd5[PROC_AMP_MAX] = {MD5_AMP_BRIGHTNESS, MD5_AMP_CONTRAST,
		MD5_AMP_SATURATION, MD5_AMP_HUE};
	//CVI_U32 au32CheckSum[PROC_AMP_MAX] = {0x596d1930, 0xc7b3f358, 0x9b199b91, 0xf85907af};

	for (type = PROC_AMP_BRIGHTNESS; type < PROC_AMP_MAX; type++) {
		CVI_VPSS_GetGrpProcAmpCtrl(VpssGrp, type, &ctrl);
		if ((ctrl.minimum != 0) || (ctrl.maximum != 100)
			|| (ctrl.step != 1) || (ctrl.default_value != 50)) {
			UT_PRT("CVI_VPSS_GetGrpProcAmpCtrl fail!\n");
			s32Ret = CVI_FAILURE;
			goto exit5;
		}
		s32Ret = CVI_VPSS_SetGrpProcAmp(VpssGrp, type, value);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetGrpProcAmp failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}

		UT_PRT("***CVI_VPSS_GetChnFrame Success***\n");

		if (CompareWithMD5(pszMd5[type], &stVideoFrameOut)) {
			snprintf(FileName, 64, "%s/%s_%s_%d_%d_%s.bin",
				OUT_FILE_PREFIX, __func__, pSuffix[type],
				stSize.u32Width,
				stSize.u32Height,
				GetFmtName(enPixelFormat));
			FrameSaveToFile(FileName, &stVideoFrameOut);
			s32Ret = CVI_FAILURE;
			UT_PRT("Compare MD5 fail, MD5:%s\n", pszMd5[type]);
			UT_PRT("output file:%s\n", FileName);
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		CVI_VPSS_SetGrpProcAmp(VpssGrp, type, ctrl.default_value);

		//if (s32Ret)
		//	break;
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

//normalize: x*factor - mean
static CVI_S32 vpss_test_normalize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_RGB_888;
	stTestParam.enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_TRUE;
	stTestParam.stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	stTestParam.stNormalize.factor[0] = 0.5;
	stTestParam.stNormalize.factor[1] = 0.5;
	stTestParam.stNormalize.factor[2] = 0.5;
	stTestParam.stNormalize.mean[0] = 10;
	stTestParam.stNormalize.mean[1] = 10;
	stTestParam.stNormalize.mean[2] = 10;
	stTestParam.u32CheckSum = 0x891e2d0b;
	strncpy(stTestParam.aszMD5Sum, MD5_NORMALIZE, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_RGB_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

//convert: ax + b
static CVI_S32 vpss_test_convert(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_RGB_888;
	stTestParam.enFormatOut = PIXEL_FORMAT_UINT8_C3_PLANAR;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.stConvert.bEnable = CVI_TRUE;
	stTestParam.stConvert.u32aFactor[0] = 2 * 8192;
	stTestParam.stConvert.u32aFactor[1] = 2 * 8192;
	stTestParam.stConvert.u32aFactor[2] = 2 * 8192;
	stTestParam.stConvert.u32bFactor[0] = 5 * 8192;
	stTestParam.stConvert.u32bFactor[1] = 5 * 8192;
	stTestParam.stConvert.u32bFactor[2] = 5 * 8192;
	stTestParam.u32CheckSum = 0xda899972;
	strncpy(stTestParam.aszMD5Sum, MD5_CONVERT, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_RGB_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_scale_coef(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = 1280;
	stTestParam.stSizeOut.u32Height = 720;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.enCoef = VPSS_SCALE_COEF_BICUBIC;
	stTestParam.u32CheckSum = 0xc3cf194b;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF1, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_bicubic.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	//bilinear
	stTestParam.enCoef = VPSS_SCALE_COEF_BILINEAR;
	stTestParam.u32CheckSum = 0x5583d343;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF2, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_bilinear.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	//nearest
	stTestParam.enCoef = VPSS_SCALE_COEF_NEAREST;
	stTestParam.u32CheckSum = 0x39768945;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF3, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_nearest.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	//opencv bicubic
	stTestParam.enCoef = VPSS_SCALE_COEF_BICUBIC_OPENCV;
	stTestParam.u32CheckSum = 0xbdc97502;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF4, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_bicubic_cv.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_y_ratio(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.YRatio = 0.5;
	stTestParam.u32CheckSum = 0x4e5ca43c;
	strncpy(stTestParam.aszMD5Sum, MD5_Y_RATIO, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_hide(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.bHide = CVI_TRUE;
	stTestParam.u32CheckSum = 0x3c3a4000;
	strncpy(stTestParam.aszMD5Sum, MD5_HIDE, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_rotation(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = ALIGN(DEFAULT_W, DEFAULT_ALIGN);
	stTestParam.stSizeOut.u32Height = ALIGN(DEFAULT_H, DEFAULT_ALIGN);
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_NV21;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.enRotation = ROTATION_90;
	strncpy(stTestParam.aszFileNameIn, VPSS_ROT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_ldc(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;
	VPSS_LDC_ATTR_S *pstLDCAttr;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = ALIGN(DEFAULT_W, DEFAULT_ALIGN);
	stTestParam.stSizeOut.u32Height = ALIGN(DEFAULT_H, DEFAULT_ALIGN);
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_NV21;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.enRotation = ROTATION_0;

	pstLDCAttr = &stTestParam.stLDCAttr;
	pstLDCAttr->bEnable = CVI_TRUE;
	pstLDCAttr->stAttr.bAspect = CVI_TRUE;
	pstLDCAttr->stAttr.s32XYRatio = 100;
	pstLDCAttr->stAttr.s32CenterXOffset = 0;
	pstLDCAttr->stAttr.s32CenterYOffset = 0;
	pstLDCAttr->stAttr.s32DistortionRatio = -200;

	strncpy(stTestParam.aszFileNameIn, VPSS_LDC_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_ldc_load_mesh(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;
	VPSS_LDC_ATTR_S *pstLDCAttr;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = ALIGN(DEFAULT_W, DEFAULT_ALIGN);
	stTestParam.stSizeOut.u32Height = ALIGN(DEFAULT_H, DEFAULT_ALIGN);
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_NV21;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.enRotation = ROTATION_0;

	pstLDCAttr = &stTestParam.stLDCAttr;
	pstLDCAttr->bEnable = CVI_TRUE;
	stTestParam.bUseLoadMesh = CVI_TRUE;

	strncpy(stTestParam.aszFileNameIn, VPSS_LDC_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_VOID *pressure_thread_run(CVI_VOID *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, j, s32Repeat = TEST_CNT0;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VIDEO_FRAME_INFO_S stVideoFrameOut, stVideoFrameIn;
	PIXEL_FORMAT_E enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_NV21;
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	SIZE_S astSizeOut[VPSS_MAX_CHN_NUM] = {{DEFAULT_W, DEFAULT_H}, {1280, 720}, {640, 360}, {320, 180}};
	CVI_BOOL abChnEnable[VPSS_MAX_CHN_NUM];
	CVI_CHAR *pFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_U32 u32ChnMask;

	UNUSED(arg);

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = enFormatIn;
	stVpssGrpAttr.u32MaxW = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	VpssGrp = CVI_VPSS_GetAvailableGrp();
	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return NULL;
	}

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		VpssChn = i;
		stVpssChnAttr.u32Width = astSizeOut[i].u32Width;
		stVpssChnAttr.u32Height = astSizeOut[i].u32Height;
		stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
		stVpssChnAttr.enPixelFormat = enFormatOut;
		stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
		stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
		stVpssChnAttr.u32Depth = 1;
		stVpssChnAttr.bMirror = CVI_FALSE;
		stVpssChnAttr.bFlip = CVI_FALSE;
		stVpssChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stVpssChnAttr.stNormalize.bEnable = CVI_FALSE;

		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit0;
		}

		s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1 + i);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
			goto exit0;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit0;
		}
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit1;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	for (i = 0; i < s32Repeat; i++) {
		u32ChnMask = 0;

		for (j = 0; j < VPSS_MAX_CHN_NUM; j++) {
			abChnEnable[j] = rand() % 2 ? CVI_TRUE : CVI_FALSE;
			if (abChnEnable[j]) {
				CVI_VPSS_EnableChn(VpssGrp, j);
				u32ChnMask |= BIT(j);
			} else
				CVI_VPSS_DisableChn(VpssGrp, j);
		}

		if (!u32ChnMask)
			continue;

		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit3;
		}

		for (j = 0; j < VPSS_MAX_CHN_NUM; j++) {
			if (!abChnEnable[j])
				continue;
			VpssChn = j;
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}

			s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}
		}
	}

exit3:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit2:
	CVI_VPSS_StopGrp(VpssGrp);
exit1:
	for (j = 0; j < VPSS_MAX_CHN_NUM; j++)
		CVI_VPSS_DisableChn(VpssGrp, j);
exit0:
	CVI_VPSS_DestroyGrp(VpssGrp);

	if (s32Ret == CVI_SUCCESS) {
		pthread_mutex_lock(&s_SyncMutex);
		s_u32Flag |= BIT(VpssGrp);
		pthread_mutex_unlock(&s_SyncMutex);
	}

	return NULL;
}

static CVI_S32 vpss_test_pressure(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	CVI_S32 s32ThreadNum = 4;
	pthread_t thread[10] = {[0 ... 9] = 0};
	SIZE_S astSizeOut[VPSS_MAX_CHN_NUM] = {{DEFAULT_W, DEFAULT_H}, {1280, 720}, {640, 360}, {320, 180}};
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt				= 1 + VPSS_MAX_CHN_NUM;

	u32BlkSizeIn = COMMON_GetPicBufferSize(DEFAULT_W, DEFAULT_H, PIXEL_FORMAT_YUV_PLANAR_420,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= s32ThreadNum;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		u32BlkSizeOut = COMMON_GetPicBufferSize(astSizeOut[i].u32Width, astSizeOut[i].u32Height,
			PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

		stVbConf.astCommPool[1 + i].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[1 + i].u32BlkCnt	= s32ThreadNum;
		stVbConf.astCommPool[1 + i].enRemapMode = VB_REMAP_MODE_CACHED;
		UT_PRT("common pool[%d] BlkSize %d\n", 1 + i, u32BlkSizeOut);
	}

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

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	srand((CVI_U32)time(NULL));
	s_u32Flag = 0;

	for (i = 0; i < s32ThreadNum; i++) {
		s32Ret = pthread_create(&thread[i], NULL, pressure_thread_run, NULL);
		if (s32Ret < 0) {
			UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < s32ThreadNum; i++)
		if (thread[i] != 0)
			pthread_join(thread[i], NULL);

	for (i = 0; i < s32ThreadNum; i++)
		if (!(s_u32Flag & BIT(i)))
			s32Ret = CVI_FAILURE;

exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_perf(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_U64 u64CurPTS1, u64CurPTS2, u64CostTime;
	CVI_U64 u64MinCostTime = 10000, u64MaxCostTime = 0;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSize, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	for (i = 0; i < TEST_CNT0; i++) {
		CVI_SYS_GetCurPTS(&u64CurPTS1);
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
		CVI_SYS_GetCurPTS(&u64CurPTS2);
		u64CostTime = u64CurPTS2 - u64CurPTS1;
		u64MinCostTime = u64CostTime < u64MinCostTime ? u64CostTime : u64MinCostTime;
		u64MaxCostTime = u64CostTime > u64MaxCostTime ? u64CostTime : u64MaxCostTime;

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
		//system("cat /proc/soph/vpss");
	}
	if ((u64MaxCostTime - u64MinCostTime) > 500) {
		s32Ret = -1;
		UT_PRT("Time fluctuation anomaly !!!\n");
	}
	UT_PRT("1080P cost time: Min-Max: (%"PRIu64", %"PRIu64")us, offset=%"PRIu64"\n",
		u64MinCostTime, u64MaxCostTime, u64MaxCostTime - u64MinCostTime);

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_mp_get_chn_frm_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {DEFAULT_W, DEFAULT_H};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &stSize, enPixelFormat, pstFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = system("./vpss_ut_client 0");
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("client fail\n");
	}

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_csc_rgb2yuv(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_RGB_888;
	PIXEL_FORMAT_E enPixelFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_RGB_FILE_IN;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
				PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8,
				COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//RGB2YUV
	s32Ret = FileToFrame(&stSize, enPixelFormatIn, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SendFrame fail.\n");
		goto exit5;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_GetChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	s32Ret = CompareCmodelRgb2Yuv(&stVideoFrameIn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CompareCmodelRgb2yuv fail!\n");
	} else {
		UT_PRT("RGB2YUV OK!\n");
	}

	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_csc_yuv2rgb(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enPixelFormatOut = PIXEL_FORMAT_RGB_888;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	VPSS_MODE_S stVPSSMode = {.enMode = VPSS_MODE_SINGLE, .aenInput[0] = VPSS_INPUT_MEM};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
				PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8,
				COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;
	stVpssGrpAttr.u8VpssDev = 0;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//YUV2RGB
	s32Ret = FileToFrame(&stSize, enPixelFormatIn, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SendFrame fail.\n");
		goto exit5;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_GetChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	s32Ret = CompareCmodelYuv2rgb(&stVideoFrameIn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CompareCmodelYuv2rgb fail!\n");
	} else {
		UT_PRT("YUV2RGB OK!\n");
	}

	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_c_model(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vpss_test_csc_rgb2yuv();
	s32Ret |= vpss_test_csc_yuv2rgb();

	return s32Ret;
}

static CVI_S32 vpss_test_user_config(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	SIZE_S stSizeIn, stSizeOut;
	PIXEL_FORMAT_E enPixelFormatIn;
	PIXEL_FORMAT_E enPixelFormatOut;
	CVI_CHAR aszFileNameIn[128];
	CVI_CHAR aszFileNameOut[128];
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	VPSS_MODE_S stVPSSMode;
	VPSS_CROP_INFO_S stCropInfo;
	CVI_S32 s32Tmp;

	//input param
	printf("\n---vpss config---\n");
	printf("vpss mode: 0(single) 1(dual):");
	scanf("%d", &s32Tmp);
	stVPSSMode.enMode = s32Tmp;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;

	if (stVPSSMode.enMode == VPSS_MODE_DUAL) {
		printf("device id: 0(dev0) 1(dev1):");
		scanf("%d", &s32Tmp);
		stVpssGrpAttr.u8VpssDev = s32Tmp;
	} else {
		stVpssGrpAttr.u8VpssDev = 0;
	}
	printf("chn id:");
	scanf("%d", &VpssChn);

	printf("input width:");
	scanf("%d", &stSizeIn.u32Width);
	printf("input height:");
	scanf("%d", &stSizeIn.u32Height);

	printf("output width:");
	scanf("%d", &stSizeOut.u32Width);
	printf("output height:");
	scanf("%d", &stSizeOut.u32Height);

	printf("chn crop: 0(disable) 1(enable)");
	scanf("%d", &s32Tmp);
	stCropInfo.bEnable = s32Tmp;

	if (stCropInfo.bEnable) {
		printf("chn crop x:");
		scanf("%d", &stCropInfo.stCropRect.s32X);
		printf("chn crop y:");
		scanf("%d", &stCropInfo.stCropRect.s32Y);
		printf("chn crop width:");
		scanf("%d", &stCropInfo.stCropRect.u32Width);
		printf("chn crop height:");
		scanf("%d", &stCropInfo.stCropRect.u32Height);

		stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	}

	printf("format list:\n");
	for (i = PIXEL_FORMAT_RGB_888; i < PIXEL_FORMAT_MAX; i++) {
		if (strncmp(GetFmtName(i), "unknown", sizeof("unknown")))
			printf("%2d : %s\n", i, GetFmtName(i));
	}
	printf("input format:");
	scanf("%d", &s32Tmp);
	enPixelFormatIn = s32Tmp;

	printf("output format:");
	scanf("%d", &s32Tmp);
	enPixelFormatOut = s32Tmp;

	printf("input file:");
	scanf("%s", aszFileNameIn);

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height,
		enPixelFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= (u32BlkSizeIn > u32BlkSizeOut) ? u32BlkSizeIn : u32BlkSizeOut;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);

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
	 * step2:  Init VPSS
	 ************************************************/
	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetMode failed with %#x!\n", s32Ret);
		goto exit1;
	}

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;

	stVpssChnAttr.u32Width		    = stSizeOut.u32Width;
	stVpssChnAttr.u32Height		    = stSizeOut.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_SetChnCrop failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//YUV2RGB
	s32Ret = FileSendToVpss(VpssGrp, &stSizeIn, enPixelFormatIn, aszFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VPSS_GetChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	snprintf(aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stSizeOut.u32Width,
		stSizeOut.u32Height,
		GetFmtName(enPixelFormatOut));

	FrameFullSaveToFile(aszFileNameOut, &stVideoFrameOut);
	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_VB_Exit();
exit0:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_auto(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret |= vpss_test_basic();
	s32Ret |= vpss_test_1_to_2();
	s32Ret |= vpss_test_1_to_3();
	s32Ret |= vpss_test_1_to_4();
	s32Ret |= vpss_test_dual_mode();
	s32Ret |= vpss_test_max_resolution();
	s32Ret |= vpss_test_mirror();
	s32Ret |= vpss_test_flip();
	s32Ret |= vpss_test_mirror_flip();
	s32Ret |= vpss_test_aspect_ratio();
	s32Ret |= vpss_test_multi_grp();
	s32Ret |= vpss_test_multi_thread();
	s32Ret |= vpss_test_byte_align();
	s32Ret |= vpss_test_resize();
	s32Ret |= vpss_test_max_scaling();
	s32Ret |= vpss_test_draw_rect();
	s32Ret |= vpss_test_format();
	s32Ret |= vpss_test_grp_crop();
	s32Ret |= vpss_test_chn_crop();
	s32Ret |= vpss_test_amp_ctrl();
	s32Ret |= vpss_test_normalize();
	s32Ret |= vpss_test_convert();
	s32Ret |= vpss_test_scale_coef();
	s32Ret |= vpss_test_y_ratio();
	s32Ret |= vpss_test_hide();
	s32Ret |= vpss_test_rotation();
	s32Ret |= vpss_test_ldc();
	s32Ret |= vpss_test_pressure();
	//s32Ret |= vpss_test_perf();
	s32Ret |= vpss_mp_get_chn_frm_test();
	s32Ret |= vpss_test_c_model();

	return s32Ret;
}

static CVI_S32 _vpss_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case VPSS_TEST_BASIC:
		s32Ret = vpss_test_basic();
		break;
	case VPSS_TEST_1_to_2:
		s32Ret = vpss_test_1_to_2();
		break;
	case VPSS_TEST_1_to_3:
		s32Ret = vpss_test_1_to_3();
		break;
	case VPSS_TEST_1_to_4:
		s32Ret = vpss_test_1_to_4();
		break;
	case VPSS_TEST_DUAL_MODE:
		s32Ret = vpss_test_dual_mode();
		break;
	case VPSS_TEST_MAX_RES:
		s32Ret = vpss_test_max_resolution();
		break;
	case VPSS_TEST_MIRROR:
		s32Ret = vpss_test_mirror();
		break;
	case VPSS_TEST_FLIP:
		s32Ret = vpss_test_flip();
		break;
	case VPSS_TEST_MIRROR_FLIP:
		s32Ret = vpss_test_mirror_flip();
		break;
	case VPSS_TEST_ASPECT_RATIO:
		s32Ret = vpss_test_aspect_ratio();
		break;
	case VPSS_TEST_MULTI_GRP:
		s32Ret = vpss_test_multi_grp();
		break;
	case VPSS_TEST_MULTI_THREAD:
		s32Ret = vpss_test_multi_thread();
		break;
	case VPSS_TEST_BYTE_ALIGN:
		s32Ret = vpss_test_byte_align();
		break;
	case VPSS_TEST_RESIZE:
		s32Ret = vpss_test_resize();
		break;
	case VPSS_TEST_MAX_SCALING:
		s32Ret = vpss_test_max_scaling();
		break;
	case VPSS_TEST_DRAW_RECT:
		s32Ret = vpss_test_draw_rect();
		break;
	case VPSS_TEST_FORMAT:
		s32Ret = vpss_test_format();
		break;
	case VPSS_TEST_GRP_CROP:
		s32Ret = vpss_test_grp_crop();
		break;
	case VPSS_TEST_CHN_CROP:
		s32Ret = vpss_test_chn_crop();
		break;
	case VPSS_TEST_AMP_CTRL:
		s32Ret = vpss_test_amp_ctrl();
		break;
	case VPSS_TEST_NORMALIZE:
		s32Ret = vpss_test_normalize();
		break;
	case VPSS_TEST_CONVERT_TO:
		s32Ret = vpss_test_convert();
		break;
	case VPSS_TEST_SCALE_COEF:
		s32Ret = vpss_test_scale_coef();
		break;
	case VPSS_TEST_Y_RATIO:
		s32Ret = vpss_test_y_ratio();
		break;
	case VPSS_TEST_HIDE:
		s32Ret = vpss_test_hide();
		break;
	case VPSS_TEST_ROT:
		s32Ret = vpss_test_rotation();
		break;
	case VPSS_TEST_LDC:
		s32Ret = vpss_test_ldc();
		break;
	case VPSS_TEST_LDC_LOAD_MESH:
		s32Ret = vpss_test_ldc_load_mesh();
		break;
	case VPSS_TEST_PRESSURE:
		s32Ret = vpss_test_pressure();
		break;
	case VPSS_TEST_PERF:
		s32Ret = vpss_test_perf();
		break;
	case VPSS_TEST_MP_GET_CHN_FRM:
		s32Ret = vpss_mp_get_chn_frm_test();
		break;
	case VPSS_TEST_C_MODEL:
		s32Ret = vpss_test_c_model();
		break;
	case VPSS_TEST_AUTO:
		s32Ret = vpss_test_auto();
		break;
	case VPSS_TEST_USER_CONFIG:
		s32Ret = vpss_test_user_config();
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static CVI_VOID vpss_show_help(CVI_VOID)
{
	UT_PRT("%4d: vpss basic test\n", VPSS_TEST_BASIC);
	UT_PRT("%4d: vpss 2 chn\n", VPSS_TEST_1_to_2);
	UT_PRT("%4d: vpss 3 chn\n", VPSS_TEST_1_to_3);
	UT_PRT("%4d: vpss 4 chn\n", VPSS_TEST_1_to_4);
	UT_PRT("%4d: vpss dual mode\n", VPSS_TEST_DUAL_MODE);
	UT_PRT("%4d: Maximum resolution\n", VPSS_TEST_MAX_RES);
	UT_PRT("%4d: mirror\n", VPSS_TEST_MIRROR);
	UT_PRT("%4d: flip\n", VPSS_TEST_FLIP);
	UT_PRT("%4d: rotation 180(mirror + flip)\n", VPSS_TEST_MIRROR_FLIP);
	UT_PRT("%4d: aspect ratio\n", VPSS_TEST_ASPECT_RATIO);
	UT_PRT("%4d: multi grp\n", VPSS_TEST_MULTI_GRP);
	UT_PRT("%4d: multi thread\n", VPSS_TEST_MULTI_THREAD);
	UT_PRT("%4d: byte align\n", VPSS_TEST_BYTE_ALIGN);
	UT_PRT("%4d: resize\n", VPSS_TEST_RESIZE);
	UT_PRT("%4d: 128 scaling\n", VPSS_TEST_MAX_SCALING);
	UT_PRT("%4d: draw rectangle\n", VPSS_TEST_DRAW_RECT);
	UT_PRT("%4d: pixel format\n", VPSS_TEST_FORMAT);
	UT_PRT("%4d: group crop\n", VPSS_TEST_GRP_CROP);
	UT_PRT("%4d: channel crop\n", VPSS_TEST_CHN_CROP);
	UT_PRT("%4d: amp ctrl\n", VPSS_TEST_AMP_CTRL);
	UT_PRT("%4d: normalize\n", VPSS_TEST_NORMALIZE);
	UT_PRT("%4d: convert\n", VPSS_TEST_CONVERT_TO);
	UT_PRT("%4d: scale coef\n", VPSS_TEST_SCALE_COEF);
	UT_PRT("%4d: y ratio\n", VPSS_TEST_Y_RATIO);
	UT_PRT("%4d: hide\n", VPSS_TEST_HIDE);
	UT_PRT("%4d: rotation\n", VPSS_TEST_ROT);
	UT_PRT("%4d: ldc\n", VPSS_TEST_LDC);
	UT_PRT("%4d: ldc lod mesh\n", VPSS_TEST_LDC_LOAD_MESH);
	UT_PRT("%4d: pressure test\n", VPSS_TEST_PRESSURE);
	UT_PRT("%4d: perf test(1920x1080)\n", VPSS_TEST_PERF);
	UT_PRT("%4d: multi-process get chn frame test\n", VPSS_TEST_MP_GET_CHN_FRM);
	UT_PRT("%4d: c-model test\n", VPSS_TEST_C_MODEL);
	UT_PRT("%4d: auto test\n", VPSS_TEST_AUTO);
	UT_PRT("%4d: user config\n", VPSS_TEST_USER_CONFIG);

	UT_PRT(" 255: exit\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;
	CVI_CHAR mkdir_cmd[64] = {0};

	UT_PRT("Create Output Directory %s !\n", OUT_FILE_PREFIX);
	snprintf(mkdir_cmd, 63, "mkdir -p %s", OUT_FILE_PREFIX);
	system(mkdir_cmd);

	system("stty erase ^H");

	signal(SIGINT, vpss_ut_handle_sig);
	signal(SIGTERM, vpss_ut_handle_sig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = _vpss_handle_op(op);
		UT_PRT("vpss ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			vpss_show_help();
			scanf("%d", &op);

			s32Ret = _vpss_handle_op(op);
			if (op != 255)
				UT_PRT("vpss ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}

