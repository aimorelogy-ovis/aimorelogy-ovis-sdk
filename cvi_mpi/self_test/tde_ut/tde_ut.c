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
#include "cvi_tde.h"

#include "tde_ut_comm.h"

#define TDE_DEFAULT_PIXEL_FMT PIXEL_FORMAT_ARGB_8888

#define TEST_CNT 1000
#if defined(CONFIG_DUAL_OS)
#define THREAD_CNT 2
#else
#define THREAD_CNT 5
#endif


#define TDE_DEFAULT_FILE_IN           "res/tde/640_480_bgra.bin"

#define TDE_MD5_PEF_ROT90             "d3ba59299dcaeeb0fd8a28b22fb8bb8e"
#define TDE_MD5_PEF_ROT270            "5c138d9809c53209e19b86cd397b2fe1"
#define TDE_MD5_PEF_DRAWLINE          "5e7b07224eecc3ca8a843cfed6a1ecd2"
#define TDE_MD5_PEF_QUICKCOPY         "31eaf7d92b280c0abc9e4f4232efb832"
#define TDE_MD5_PEF_ROT90_DRAWLINE    "c7ef382eeb5913c42da863af13506ae7"

static CVI_U32 s_u32Flag;
static pthread_mutex_t s_SyncMutex = PTHREAD_MUTEX_INITIALIZER;

typedef enum _TDE_TEST_OP {
	TDE_TEST_ROT90 = 0,
	TDE_TEST_ROT270,
	TDE_TEST_DRAWLINE,
	TDE_TEST_QUICK_COPY,
	TDE_TEST_MULTI_TASK,
	TDE_TEST_MULTI_JOB,
	TDE_TEST_MISC,
	TDE_TEST_MIN_RES,
	TDE_TEST_MAX_RES,
	TDE_TEST_PERF,
	TDE_TEST_AUTO = 99
} TDE_TEST_OP;

typedef struct _TDE_BASIC_TEST_PARAM {
	TDE_TEST_OP op;
	SIZE_S stSizeIn;
	CVI_CHAR filename_in[128];
	CVI_CHAR filename_out[128];
	CVI_CHAR aszMD5Sum[33];
	TDE_LINE_S stLine;
} TDE_BASIC_TEST_PARAM;

static CVI_VOID tde_ut_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_TDE_Close();
		CVI_SYS_Exit();
		CVI_VB_Exit();
		UT_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

static CVI_S32 tde_basic(TDE_BASIC_TEST_PARAM *pTestParam)
{
	CVI_S32 s32Ret;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc;
	TDE_SURFACE_S stDst;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		UT_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc.u32Width = ALIGN(pTestParam->stSizeIn.u32Width, TDE_ALIGN);
	stSrc.u32Height = ALIGN(pTestParam->stSizeIn.u32Height, TDE_ALIGN);
	stSrc.u32Stride = stSrc.u32Width * s32PixelSize;
	stDst.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	switch (pTestParam->op) {
	case TDE_TEST_ROT90:
	case TDE_TEST_ROT270:
		stDst.u32Width = stSrc.u32Height;
		stDst.u32Height = stSrc.u32Width;
		stDst.u32Stride = stDst.u32Width * s32PixelSize;
		break;
	case TDE_TEST_DRAWLINE:
	case TDE_TEST_QUICK_COPY:
	default:
		stDst.u32Width = stSrc.u32Width;
		stDst.u32Height = stSrc.u32Height;
		stDst.u32Stride = stDst.u32Width * s32PixelSize;
		break;
	}

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc.u32Stride * stSrc.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst.u32Stride * stDst.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	memset(pVirAddrSrc, 0, stSrc.u32Stride * stSrc.u32Height);
	memset(pVirAddrDst, 0, stDst.u32Stride * stDst.u32Height);
	s32Ret = TDEFileToBuffer(pTestParam->filename_in, pVirAddrSrc, &pTestParam->stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("TDEFileToBuffer failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc.u32Stride * stSrc.u32Height);
	CVI_SYS_IonFlushCache(u64PhyAddrDst, pVirAddrDst, stDst.u32Stride * stDst.u32Height);

	stSrc.u64PhyAddr = u64PhyAddrSrc;
	stDst.u64PhyAddr = u64PhyAddrDst;

	switch (pTestParam->op) {
	case TDE_TEST_ROT90:
		s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc, &stDst, TDE_ROTATE_90);
		if (s32Ret) {
			UT_PRT("CVI_TDE_Rotate[rot90] failed!\n");
		}
		break;
	case TDE_TEST_ROT270:
		s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc, &stDst, TDE_ROTATE_270);
		if (s32Ret) {
			UT_PRT("CVI_TDE_Rotate[rot270] failed!\n");
		}
		break;
	case TDE_TEST_DRAWLINE:
		s32Ret = CVI_TDE_DrawLine(s32Handle, &stSrc, &stDst, &pTestParam->stLine);
		if (s32Ret) {
			UT_PRT("CVI_TDE_AddDrawlineTask failed!\n");
		}
		break;
	case TDE_TEST_QUICK_COPY:
		s32Ret = CVI_TDE_QuickCopy(s32Handle, &stSrc, &stDst);
		if (s32Ret) {
			UT_PRT("CVI_TDE_QuickCopy failed!\n");
		}
		break;

	default:
		UT_PRT("not allow this op(%d) fail\n", pTestParam->op);
		s32Ret = -1;
		break;
	}

	if (s32Ret != CVI_SUCCESS) {
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_EndJob failed!\n");
		goto exit3;
	}

	CVI_SYS_IonInvalidateCache(u64PhyAddrDst, pVirAddrDst, stDst.u32Stride * stDst.u32Height);
	if (TDECompareWithMD5(pTestParam->aszMD5Sum, pVirAddrDst, &stDst)) {
		TDEFrameSaveToFile(pTestParam->filename_out, pVirAddrDst, &stDst);
		UT_PRT("***Check MD5 fail***\n");
		UT_PRT("SaveToFile, file:%s\n", pTestParam->filename_out);
		s32Ret = -1;
	} else {
		UT_PRT("***JOB DONE***\n");
		s32Ret = CVI_SUCCESS;
	}

exit3:
	if (u64PhyAddrSrc)
		CVI_SYS_IonFree(u64PhyAddrSrc, pVirAddrSrc);
	if (u64PhyAddrDst)
		CVI_SYS_IonFree(u64PhyAddrDst, pVirAddrDst);
exit2:
	CVI_TDE_Close();
exit1:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 tde_test_rot90(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	TDE_BASIC_TEST_PARAM stTestParam = {0};

	stTestParam.op = TDE_TEST_ROT90;
	stTestParam.stSizeIn.u32Width = 640;
	stTestParam.stSizeIn.u32Height = 480;
	strcpy(stTestParam.filename_in, TDE_DEFAULT_FILE_IN);
	strcpy(stTestParam.filename_out, "res/tde/output_rot90.bin");
	strcpy(stTestParam.aszMD5Sum, TDE_MD5_PEF_ROT90);

	s32Ret = tde_basic(&stTestParam);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 tde_test_rot270(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	TDE_BASIC_TEST_PARAM stTestParam = {0};

	stTestParam.op = TDE_TEST_ROT270;
	stTestParam.stSizeIn.u32Width = 640;
	stTestParam.stSizeIn.u32Height = 480;
	strcpy(stTestParam.filename_in, TDE_DEFAULT_FILE_IN);
	strcpy(stTestParam.filename_out, "res/tde/output_rot270.bin");
	strcpy(stTestParam.aszMD5Sum, TDE_MD5_PEF_ROT270);

	s32Ret = tde_basic(&stTestParam);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 tde_test_drawline(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	TDE_BASIC_TEST_PARAM stTestParam = {0};

	stTestParam.op = TDE_TEST_DRAWLINE;
	stTestParam.stSizeIn.u32Width = 640;
	stTestParam.stSizeIn.u32Height = 480;
	stTestParam.stLine.s32StartX = 100;
	stTestParam.stLine.s32StartY = 100;
	stTestParam.stLine.s32EndX = 100;
	stTestParam.stLine.s32EndY = 300;
	stTestParam.stLine.u32Color = 0xff0000ff;
	stTestParam.stLine.u32Thick = 16;
	strcpy(stTestParam.filename_in, TDE_DEFAULT_FILE_IN);
	strcpy(stTestParam.filename_out, "res/tde/output_drawline.bin");
	strcpy(stTestParam.aszMD5Sum, TDE_MD5_PEF_DRAWLINE);

	s32Ret = tde_basic(&stTestParam);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 tde_test_quickcopy(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	TDE_BASIC_TEST_PARAM stTestParam = {0};

	stTestParam.op = TDE_TEST_QUICK_COPY;
	stTestParam.stSizeIn.u32Width = 640;
	stTestParam.stSizeIn.u32Height = 480;
	strcpy(stTestParam.filename_in, TDE_DEFAULT_FILE_IN);
	strcpy(stTestParam.filename_out, "res/tde/output_copy.bin");
	strcpy(stTestParam.aszMD5Sum, TDE_MD5_PEF_QUICKCOPY);

	s32Ret = tde_basic(&stTestParam);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 tde_test_multi_task(CVI_VOID)
{
	CVI_S32 s32Ret;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc1, stSrc2;
	TDE_SURFACE_S stDst1, stDst2;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;
	TDE_BASIC_TEST_PARAM stTestParam = {0};

	stTestParam.stSizeIn.u32Width = 640;
	stTestParam.stSizeIn.u32Height = 480;
	stTestParam.stLine.s32StartX = 100;
	stTestParam.stLine.s32StartY = 100;
	stTestParam.stLine.s32EndX = 100;
	stTestParam.stLine.s32EndY = 300;
	stTestParam.stLine.u32Color = 0xff0000ff;
	stTestParam.stLine.u32Thick = 16;
	strcpy(stTestParam.filename_in, TDE_DEFAULT_FILE_IN);
	strcpy(stTestParam.filename_out, "res/tde/output_rot90_drawline.bin");
	strcpy(stTestParam.aszMD5Sum, TDE_MD5_PEF_ROT90_DRAWLINE);

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		UT_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc1.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc1.u32Width = ALIGN(stTestParam.stSizeIn.u32Width, TDE_ALIGN);
	stSrc1.u32Height = ALIGN(stTestParam.stSizeIn.u32Height, TDE_ALIGN);
	stSrc1.u32Stride = stSrc1.u32Width * s32PixelSize;
	stDst1.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stDst1.u32Width = stSrc1.u32Height;
	stDst1.u32Height = stSrc1.u32Width;
	stDst1.u32Stride = stDst1.u32Width * s32PixelSize;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc1.u32Stride * stSrc1.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst1.u32Stride * stDst1.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	memset(pVirAddrSrc, 0, stSrc1.u32Stride * stSrc1.u32Height);
	memset(pVirAddrDst, 0, stDst1.u32Stride * stDst1.u32Height);
	s32Ret = TDEFileToBuffer(stTestParam.filename_in, pVirAddrSrc, &stTestParam.stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("TDEFileToBuffer failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc1.u32Stride * stSrc1.u32Height);
	CVI_SYS_IonFlushCache(u64PhyAddrDst, pVirAddrDst, stDst1.u32Stride * stDst1.u32Height);

	stSrc1.u64PhyAddr = u64PhyAddrSrc;
	stDst1.u64PhyAddr = u64PhyAddrDst;
	s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc1, &stDst1, TDE_ROTATE_90);
	if (s32Ret) {
		UT_PRT("CVI_TDE_Rotate[rot90] failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	memcpy(&stSrc2, &stDst1, sizeof(stDst1));
	memcpy(&stDst2, &stSrc2, sizeof(stDst1));
	stDst2.u64PhyAddr = u64PhyAddrSrc;

	s32Ret = CVI_TDE_DrawLine(s32Handle, &stSrc2, &stDst2, &stTestParam.stLine);
	if (s32Ret) {
		UT_PRT("CVI_TDE_AddDrawlineTask failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_EndJob failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	CVI_SYS_IonInvalidateCache(u64PhyAddrSrc, pVirAddrSrc, stDst2.u32Stride * stDst2.u32Height);
	if (TDECompareWithMD5(stTestParam.aszMD5Sum, pVirAddrSrc, &stDst2)) {
		TDEFrameSaveToFile(stTestParam.filename_out, pVirAddrSrc, &stDst2);
		UT_PRT("***Check MD5 fail***\n");
		UT_PRT("SaveToFile, file:%s\n", stTestParam.filename_out);
		s32Ret = -1;
	} else {
		UT_PRT("***JOB DONE***\n");
		s32Ret = CVI_SUCCESS;
	}

exit3:
	if (u64PhyAddrSrc)
		CVI_SYS_IonFree(u64PhyAddrSrc, pVirAddrSrc);
	if (u64PhyAddrDst)
		CVI_SYS_IonFree(u64PhyAddrDst, pVirAddrDst);
exit2:
	CVI_TDE_Close();
exit1:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_VOID *tde_multi_thread_run(CVI_VOID *arg)
{
	CVI_S32 s32Ret, i;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc1, stSrc2;
	TDE_SURFACE_S stDst1, stDst2;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;
	TDE_BASIC_TEST_PARAM stTestParam = {0};

	UNUSED(arg);
	stTestParam.stSizeIn.u32Width = 640;
	stTestParam.stSizeIn.u32Height = 480;
	stTestParam.stLine.s32StartX = 100;
	stTestParam.stLine.s32StartY = 100;
	stTestParam.stLine.s32EndX = 100;
	stTestParam.stLine.s32EndY = 300;
	stTestParam.stLine.u32Color = 0xff0000ff;
	stTestParam.stLine.u32Thick = 16;

	stSrc1.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc1.u32Width = ALIGN(stTestParam.stSizeIn.u32Width, TDE_ALIGN);
	stSrc1.u32Height = ALIGN(stTestParam.stSizeIn.u32Height, TDE_ALIGN);
	stSrc1.u32Stride = stSrc1.u32Width * s32PixelSize;
	stDst1.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stDst1.u32Width = stSrc1.u32Height;
	stDst1.u32Height = stSrc1.u32Width;
	stDst1.u32Stride = stDst1.u32Width * s32PixelSize;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc1.u32Stride * stSrc1.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		return NULL;
	}
	stSrc1.u64PhyAddr = u64PhyAddrSrc;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst1.u32Stride * stDst1.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		goto exit;
	}
	stDst1.u64PhyAddr = u64PhyAddrDst;

	memcpy(&stSrc2, &stDst1, sizeof(stDst1));
	memcpy(&stDst2, &stSrc2, sizeof(stDst1));
	stDst2.u64PhyAddr = u64PhyAddrSrc;

	for (i = 0; i < TEST_CNT; i++) {
		s32Handle = CVI_TDE_BeginJob();
		if (s32Handle == TDE_INVALID_HANDLE) {
			UT_PRT("CVI_TDE_BeginJob failed!\n");
			goto exit;
		}

		s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc1, &stDst1, TDE_ROTATE_90);
		if (s32Ret) {
			UT_PRT("CVI_TDE_Rotate[rot90] failed!\n");
			CVI_TDE_CancelJob(s32Handle);
			goto exit;
		}

		s32Ret = CVI_TDE_DrawLine(s32Handle, &stSrc2, &stDst2, &stTestParam.stLine);
		if (s32Ret) {
			UT_PRT("CVI_TDE_AddDrawlineTask failed!\n");
			CVI_TDE_CancelJob(s32Handle);
			goto exit;
		}

		s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_TDE_EndJob failed!\n");
			CVI_TDE_CancelJob(s32Handle);
			goto exit;
		}
	}

exit:
	if (u64PhyAddrSrc)
		CVI_SYS_IonFree(u64PhyAddrSrc, pVirAddrSrc);
	if (u64PhyAddrDst)
		CVI_SYS_IonFree(u64PhyAddrDst, pVirAddrDst);

	if (s32Ret == CVI_SUCCESS) {
		pthread_mutex_lock(&s_SyncMutex);
		s_u32Flag++;
		pthread_mutex_unlock(&s_SyncMutex);
	}

	return NULL;
}

static CVI_S32 tde_test_multi_job(CVI_VOID)
{
	CVI_S32 s32Ret, i;
	CVI_S32 s32ThreadNum = THREAD_CNT;
	pthread_t thread[THREAD_CNT] = {[0 ... THREAD_CNT - 1] = 0};

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}
	s_u32Flag = 0;

	for (i = 0; i < s32ThreadNum; i++) {
		s32Ret = pthread_create(&thread[i], NULL, tde_multi_thread_run, NULL);
		if (s32Ret < 0) {
			UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < s32ThreadNum; i++)
		if (thread[i] != 0)
			pthread_join(thread[i], NULL);

	if (s_u32Flag != THREAD_CNT)
		s32Ret = CVI_FAILURE;

	CVI_TDE_Close();
exit1:
	CVI_SYS_Exit();

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 tde_test_multi_misc(CVI_VOID)
{
	CVI_S32 s32Ret;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc;
	TDE_SURFACE_S stDst;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;
	SIZE_S stSizeIn = {640, 480};

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		UT_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc.u32Width = ALIGN(stSizeIn.u32Width, TDE_ALIGN);
	stSrc.u32Height = ALIGN(stSizeIn.u32Height, TDE_ALIGN);
	stSrc.u32Stride = stSrc.u32Width * s32PixelSize;
	stDst.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stDst.u32Width = stSrc.u32Height;
	stDst.u32Height = stSrc.u32Width;
	stDst.u32Stride = stDst.u32Width * s32PixelSize;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc.u32Stride * stSrc.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst.u32Stride * stDst.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	stSrc.u64PhyAddr = u64PhyAddrSrc;
	stDst.u64PhyAddr = u64PhyAddrDst;

	s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc, &stDst, TDE_ROTATE_90);
	if (s32Ret) {
		UT_PRT("CVI_TDE_Rotate[rot90] failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	//test CVI_TDE_CancelJob
	s32Ret = CVI_TDE_CancelJob(s32Handle);
	if (s32Ret) {
		UT_PRT("CVI_TDE_CancelJob failed!\n");
		goto exit3;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		UT_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit3;
	}
	s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc, &stDst, TDE_ROTATE_90);
	if (s32Ret) {
		UT_PRT("CVI_TDE_Rotate[rot90] failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_FALSE, 0);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_EndJob failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	//test CVI_TDE_WaitAllDone
	s32Ret = CVI_TDE_WaitAllDone();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_WaitAllDone failed!\n");
		goto exit3;
	}

exit3:
	if (u64PhyAddrSrc)
		CVI_SYS_IonFree(u64PhyAddrSrc, pVirAddrSrc);
	if (u64PhyAddrDst)
		CVI_SYS_IonFree(u64PhyAddrDst, pVirAddrDst);
exit2:
	CVI_TDE_Close();
exit1:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 tde_test_resolution(SIZE_S *pstSize, CVI_U64 *pu64Time)
{
	CVI_S32 s32Ret;
	CVI_U32 w, h;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc, stDst;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;
	CVI_U32 *pu32Temp;
	CVI_U32 u32Pattern = 0xff00FF00; //green
	CVI_U64 u64Pts1, u64Pts2;

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		UT_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc.u32Width = ALIGN(pstSize->u32Width, TDE_ALIGN);
	stSrc.u32Height = ALIGN(pstSize->u32Height, TDE_ALIGN);
	stSrc.u32Stride = stSrc.u32Width * s32PixelSize;
	stDst.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stDst.u32Width = stSrc.u32Height;
	stDst.u32Height = stSrc.u32Width;
	stDst.u32Stride = stDst.u32Width * s32PixelSize;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc.u32Stride * stSrc.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst.u32Stride * stDst.u32Height);
	if (s32Ret) {
		UT_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	//fill pattern
	for (h = 0; h < stSrc.u32Height; h++) {
		pu32Temp = (CVI_U32 *)(pVirAddrSrc + (h * stSrc.u32Stride));
		for (w = 0; w < stSrc.u32Width; w++) {
			*pu32Temp = u32Pattern;
			pu32Temp++;
		}
	}

	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc.u32Stride * stSrc.u32Height);

	stSrc.u64PhyAddr = u64PhyAddrSrc;
	stDst.u64PhyAddr = u64PhyAddrDst;
	s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc, &stDst, TDE_ROTATE_90);
	if (s32Ret) {
		UT_PRT("CVI_TDE_Rotate[rot90] failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	CVI_SYS_GetCurPTS(&u64Pts1);
	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_TDE_EndJob failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_GetCurPTS(&u64Pts2);
	UT_PRT("w=%d h=%d, perf:%"PRIu64"us\n", pstSize->u32Width, pstSize->u32Height, u64Pts2 - u64Pts1);
	if (pu64Time)
		*pu64Time = u64Pts2 - u64Pts1;

	CVI_SYS_IonInvalidateCache(u64PhyAddrDst, pVirAddrDst, stDst.u32Stride * stDst.u32Height);
	//check dst
	for (h = 0; h < stDst.u32Height; h++) {
		pu32Temp = (CVI_U32 *)(pVirAddrDst + (h * stDst.u32Stride));
		for (w = 0; w < stDst.u32Width; w++) {
			if (*pu32Temp != u32Pattern) {
				UT_PRT("Data error, [w=%d][h=%d] = %x.\n", w, h, *pu32Temp);
				s32Ret = CVI_FAILURE;
				goto exit3;
			}
			pu32Temp++;
		}
	}

exit3:
	if (u64PhyAddrSrc)
		CVI_SYS_IonFree(u64PhyAddrSrc, pVirAddrSrc);
	if (u64PhyAddrDst)
		CVI_SYS_IonFree(u64PhyAddrDst, pVirAddrDst);
exit2:
	CVI_TDE_Close();
exit1:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 tde_test_min_resolution(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize = {16, 16};

	s32Ret = tde_test_resolution(&stSize, NULL);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 tde_test_max_resolution(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize = {4096, 2160};

	s32Ret = tde_test_resolution(&stSize, NULL);
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 tde_test_perf(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSize = {1280, 720};
	CVI_U64 u64Time = 0;
	CVI_U64 u64MaxTime = 13 * 1000; //us

	s32Ret = tde_test_resolution(&stSize, &u64Time);
	if (u64Time > u64MaxTime) {
		UT_PRT("Exceed expectations, [720P] cost time:%"PRIu64"us\n", u64Time);
		s32Ret = CVI_FAILURE;
	}
	UT_CHECK_CASE_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 tde_test_auto(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret |= tde_test_rot90();
	s32Ret |= tde_test_rot270();
	s32Ret |= tde_test_drawline();
	s32Ret |= tde_test_quickcopy();
	s32Ret |= tde_test_multi_task();
	s32Ret |= tde_test_multi_job();
	s32Ret |= tde_test_multi_misc();
	s32Ret |= tde_test_min_resolution();
	//s32Ret |= tde_test_max_resolution();
	s32Ret |= tde_test_perf();
	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 tde_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case TDE_TEST_ROT90:
		s32Ret = tde_test_rot90();
		break;
	case TDE_TEST_ROT270:
		s32Ret = tde_test_rot270();
		break;
	case TDE_TEST_DRAWLINE:
		s32Ret = tde_test_drawline();
		break;
	case TDE_TEST_QUICK_COPY:
		s32Ret = tde_test_quickcopy();
		break;
	case TDE_TEST_MULTI_TASK:
		s32Ret = tde_test_multi_task();
		break;
	case TDE_TEST_MULTI_JOB:
		s32Ret = tde_test_multi_job();
		break;
	case TDE_TEST_MISC:
		s32Ret = tde_test_multi_misc();
		break;
	case TDE_TEST_MIN_RES:
		s32Ret = tde_test_min_resolution();
		break;
	case TDE_TEST_MAX_RES:
		s32Ret = tde_test_max_resolution();
		break;
	case TDE_TEST_PERF:
		s32Ret = tde_test_perf();
		break;
	case TDE_TEST_AUTO:
		s32Ret = tde_test_auto();
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static CVI_VOID tde_show_help(CVI_VOID)
{
	UT_PRT("%4d: tde basic test rot90\n", TDE_TEST_ROT90);
	UT_PRT("%4d: tde basic test rot270\n", TDE_TEST_ROT270);
	UT_PRT("%4d: tde basic test drawline\n", TDE_TEST_DRAWLINE);
	UT_PRT("%4d: tde basic test quick copy\n", TDE_TEST_QUICK_COPY);
	UT_PRT("%4d: tde multi task,rot90 + drawline\n", TDE_TEST_MULTI_TASK);
	UT_PRT("%4d: tde multi job\n", TDE_TEST_MULTI_JOB);
	UT_PRT("%4d: tde misc test\n", TDE_TEST_MISC);
	UT_PRT("%4d: min resolution\n", TDE_TEST_MIN_RES);
	UT_PRT("%4d: max resolution\n", TDE_TEST_MAX_RES);
	UT_PRT("%4d: perf test(1920x1080)\n", TDE_TEST_PERF);
	UT_PRT("%4d: auto test\n", TDE_TEST_AUTO);
	UT_PRT("255: exit\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;

	s32Ret = 0;
	system("stty erase ^H");

	signal(SIGINT, tde_ut_HandleSig);
	signal(SIGTERM, tde_ut_HandleSig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);

		s32Ret = tde_handle_op(op);
		UT_PRT("tde ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			tde_show_help();
			scanf("%d", &op);

			s32Ret = tde_handle_op(op);
			if (op != 255)
				UT_PRT("tde ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}
