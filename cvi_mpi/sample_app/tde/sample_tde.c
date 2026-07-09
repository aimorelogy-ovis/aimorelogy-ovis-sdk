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
#include "sample_comm.h"


#define TDE_DEFAULT_FILE_IN           "res/640_480_bgra.bin"
#define TDE_DEFAULT_FILE_W (640)
#define TDE_DEFAULT_FILE_H (480)
#define TDE_DEFAULT_PIXEL_FMT (PIXEL_FORMAT_ARGB_8888)

CVI_VOID SAMPLE_TDE_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		//todo for release
		SAMPLE_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

CVI_VOID SAMPLE_TDE_Usage(CVI_CHAR *sPrgNm)
{
	SAMPLE_PRT("Usage : %s <index>\n", sPrgNm);
	SAMPLE_PRT("index:\n");
	SAMPLE_PRT("\t 0)tde test rotation 90\n");
	SAMPLE_PRT("\t 1)tde test rotation 270\n");
	SAMPLE_PRT("\t 2)tde test drawline\n");
	SAMPLE_PRT("\t 3)tde test quick copy\n");
	SAMPLE_PRT("\t 4)tde multi task,rot90 + drawline\n");
}

CVI_S32 TDEFileToBuffer(const CVI_CHAR *filename, CVI_VOID *buffer, SIZE_S *pstSize)
{
	FILE *fp;
	CVI_U32 i;
	CVI_S32 result = CVI_SUCCESS, s32len;

	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	for (i = 0; i < pstSize->u32Height; i++) {
		s32len = fread(buffer, pstSize->u32Width, 4, fp);
		if (s32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			result = CVI_FAILURE;
			break;
		}
		buffer += pstSize->u32Width * 4;
	}

	fclose(fp);

	return result;
}

CVI_S32 TDEFrameSaveToFile(const CVI_CHAR *filename, CVI_VOID *buffer, TDE_SURFACE_S *pstDst)
{
	FILE *fp;
	CVI_U32 i;
	CVI_S32 result = CVI_SUCCESS, s32len;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	for (i = 0; i < pstDst->u32Height; i++) {
		s32len = fwrite(buffer, pstDst->u32Width, 4, fp);
		if (s32len <= 0) {
			SAMPLE_PRT("fwrite data(%d) error\n", i);
			result = CVI_FAILURE;
			break;
		}
		buffer += pstDst->u32Stride;
	}

	fclose(fp);

	return result;
}

static CVI_S32 TDE_Rotate(TDE_ROTATE_ANGLE_E enRotateAngle)
{
	CVI_S32 s32Ret;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc;
	TDE_SURFACE_S stDst;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;
	SIZE_S stSizeIn = {TDE_DEFAULT_FILE_W, TDE_DEFAULT_FILE_H};
	CVI_CHAR *filename_in = TDE_DEFAULT_FILE_IN;
	CVI_CHAR *filename_out = NULL;

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		SAMPLE_PRT("CVI_TDE_BeginJob failed!\n");
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
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst.u32Stride * stDst.u32Height);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	memset(pVirAddrSrc, 0, stSrc.u32Stride * stSrc.u32Height);

	s32Ret = TDEFileToBuffer(filename_in, pVirAddrSrc, &stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("TDEFileToBuffer failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc.u32Stride * stSrc.u32Height);

	stSrc.u64PhyAddr = u64PhyAddrSrc;
	stDst.u64PhyAddr = u64PhyAddrDst;

	s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc, &stDst, enRotateAngle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_Rotate failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_EndJob failed!\n");
		goto exit3;
	}

	if (enRotateAngle == TDE_ROTATE_90) {
		filename_out = "480_640_bgra_rotate90.bin";
	} else if (enRotateAngle == TDE_ROTATE_270) {
		filename_out = "480_640_bgra_rotate270.bin";
	}

	SAMPLE_PRT("***JOB DONE***\n");
	SAMPLE_PRT("Successful, save output file:%s\n", filename_out);

	CVI_SYS_IonInvalidateCache(u64PhyAddrDst, pVirAddrDst, stDst.u32Stride * stDst.u32Height);
	TDEFrameSaveToFile(filename_out, pVirAddrDst, &stDst);


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

static CVI_S32 SAMPLE_TDE_Rotate90(CVI_VOID)
{
	return TDE_Rotate(TDE_ROTATE_90);
}

static CVI_S32 SAMPLE_TDE_Rotate270(CVI_VOID)
{
	return TDE_Rotate(TDE_ROTATE_270);
}

static CVI_S32 SAMPLE_TDE_DrawLine(CVI_VOID)
{
	CVI_S32 s32Ret;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc;
	TDE_SURFACE_S stDst;
	CVI_U64 u64PhyAddrSrc;
	CVI_VOID *pVirAddrSrc;
	CVI_S32 s32PixelSize = 4;
	SIZE_S stSizeIn = {TDE_DEFAULT_FILE_W, TDE_DEFAULT_FILE_H};
	CVI_CHAR *filename_in = TDE_DEFAULT_FILE_IN;
	CVI_CHAR *filename_out = "640_480_bgra_drawline.bin";
	TDE_LINE_S stLine;

	stLine.s32StartX = 100;
	stLine.s32StartY = 100;
	stLine.s32EndX = 100;
	stLine.s32EndY = 300;
	stLine.u32Color = 0xff0000ff;
	stLine.u32Thick = 16;

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		SAMPLE_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc.u32Width = ALIGN(stSizeIn.u32Width, TDE_ALIGN);
	stSrc.u32Height = ALIGN(stSizeIn.u32Height, TDE_ALIGN);
	stSrc.u32Stride = stSrc.u32Width * s32PixelSize;

	stDst.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stDst.u32Width = stSrc.u32Width;
	stDst.u32Height = stSrc.u32Height;
	stDst.u32Stride = stSrc.u32Stride;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_buffer", stSrc.u32Stride * stSrc.u32Height);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	memset(pVirAddrSrc, 0, stSrc.u32Stride * stSrc.u32Height);

	s32Ret = TDEFileToBuffer(filename_in, pVirAddrSrc, &stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("TDEFileToBuffer failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc.u32Stride * stSrc.u32Height);

	stSrc.u64PhyAddr = u64PhyAddrSrc;
	stDst.u64PhyAddr = u64PhyAddrSrc;

	s32Ret = CVI_TDE_DrawLine(s32Handle, &stSrc, &stDst, &stLine);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_DrawLine failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_EndJob failed!\n");
		goto exit3;
	}

	SAMPLE_PRT("***JOB DONE***\n");
	SAMPLE_PRT("Successful, save output file:%s\n", filename_out);

	CVI_SYS_IonInvalidateCache(u64PhyAddrSrc, pVirAddrSrc, stDst.u32Stride * stDst.u32Height);
	TDEFrameSaveToFile(filename_out, pVirAddrSrc, &stDst);


exit3:
	if (u64PhyAddrSrc)
		CVI_SYS_IonFree(u64PhyAddrSrc, pVirAddrSrc);
exit2:
	CVI_TDE_Close();
exit1:
	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 SAMPLE_TDE_QuickCopy(CVI_VOID)
{
	CVI_S32 s32Ret;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc;
	TDE_SURFACE_S stDst;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;
	SIZE_S stSizeIn = {TDE_DEFAULT_FILE_W, TDE_DEFAULT_FILE_H};
	CVI_CHAR *filename_in = TDE_DEFAULT_FILE_IN;
	CVI_CHAR *filename_out = "640_480_bgra_quick_copy.bin";

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		SAMPLE_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc.u32Width = ALIGN(stSizeIn.u32Width, TDE_ALIGN);
	stSrc.u32Height = ALIGN(stSizeIn.u32Height, TDE_ALIGN);
	stSrc.u32Stride = stSrc.u32Width * s32PixelSize;

	stDst.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stDst.u32Width = stSrc.u32Width;
	stDst.u32Height = stSrc.u32Height;
	stDst.u32Stride = stSrc.u32Stride;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc.u32Stride * stSrc.u32Height);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst.u32Stride * stDst.u32Height);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	memset(pVirAddrSrc, 0, stSrc.u32Stride * stSrc.u32Height);

	s32Ret = TDEFileToBuffer(filename_in, pVirAddrSrc, &stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("TDEFileToBuffer failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc.u32Stride * stSrc.u32Height);

	stSrc.u64PhyAddr = u64PhyAddrSrc;
	stDst.u64PhyAddr = u64PhyAddrDst;

	s32Ret = CVI_TDE_QuickCopy(s32Handle, &stSrc, &stDst);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_QuickCopy failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_EndJob failed!\n");
		goto exit3;
	}

	SAMPLE_PRT("***JOB DONE***\n");
	SAMPLE_PRT("Successful, save output file:%s\n", filename_out);

	CVI_SYS_IonInvalidateCache(u64PhyAddrDst, pVirAddrDst, stDst.u32Stride * stDst.u32Height);
	TDEFrameSaveToFile(filename_out, pVirAddrDst, &stDst);


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

static CVI_S32 SAMPLE_TDE_MultiTask(CVI_VOID)
{
	CVI_S32 s32Ret;
	TDE_HANDLE s32Handle;
	TDE_SURFACE_S stSrc1, stSrc2;
	TDE_SURFACE_S stDst1, stDst2;
	CVI_U64 u64PhyAddrSrc = 0, u64PhyAddrDst = 0;
	CVI_VOID *pVirAddrSrc, *pVirAddrDst;
	CVI_S32 s32PixelSize = 4;
	SIZE_S stSizeIn = {TDE_DEFAULT_FILE_W, TDE_DEFAULT_FILE_H};
	CVI_CHAR *filename_in = TDE_DEFAULT_FILE_IN;
	CVI_CHAR *filename_out = "480_640_bgra_rotate90_drawline.bin";
	TDE_LINE_S stLine;

	stLine.s32StartX = 100;
	stLine.s32StartY = 100;
	stLine.s32EndX = 100;
	stLine.s32EndY = 300;
	stLine.u32Color = 0xff00ffff;
	stLine.u32Thick = 16;

	/************************************************
	 * step1:  Init sys
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step2:  Init TDE
	 ************************************************/
	s32Ret = CVI_TDE_Open();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_Open failed!\n");
		goto exit1;
	}

	s32Handle = CVI_TDE_BeginJob();
	if (s32Handle == TDE_INVALID_HANDLE) {
		SAMPLE_PRT("CVI_TDE_BeginJob failed!\n");
		goto exit2;
	}
	stSrc1.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stSrc1.u32Width = ALIGN(stSizeIn.u32Width, TDE_ALIGN);
	stSrc1.u32Height = ALIGN(stSizeIn.u32Height, TDE_ALIGN);
	stSrc1.u32Stride = stSrc1.u32Width * s32PixelSize;

	stDst1.enColorFmt = TDE_DEFAULT_PIXEL_FMT;
	stDst1.u32Width = stSrc1.u32Height;
	stDst1.u32Height = stSrc1.u32Width;
	stDst1.u32Stride = stDst1.u32Width * s32PixelSize;

	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrSrc, &pVirAddrSrc, "TDE_src_buffer", stSrc1.u32Stride * stSrc1.u32Height);
	if (s32Ret) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64PhyAddrDst, &pVirAddrDst, "TDE_dst_buffer", stDst1.u32Stride * stDst1.u32Height);
	if (s32Ret) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	memset(pVirAddrSrc, 0, stSrc1.u32Stride * stSrc1.u32Height);
	s32Ret = TDEFileToBuffer(filename_in, pVirAddrSrc, &stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("TDEFileToBuffer failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}
	CVI_SYS_IonFlushCache(u64PhyAddrSrc, pVirAddrSrc, stSrc1.u32Stride * stSrc1.u32Height);

	stSrc1.u64PhyAddr = u64PhyAddrSrc;
	stDst1.u64PhyAddr = u64PhyAddrDst;
	s32Ret = CVI_TDE_Rotate(s32Handle, &stSrc1, &stDst1, TDE_ROTATE_90);
	if (s32Ret) {
		SAMPLE_PRT("CVI_TDE_Rotate[rot90] failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	memcpy(&stSrc2, &stDst1, sizeof(stDst1));
	memcpy(&stDst2, &stSrc2, sizeof(stSrc2));

	s32Ret = CVI_TDE_DrawLine(s32Handle, &stSrc2, &stDst2, &stLine);
	if (s32Ret) {
		SAMPLE_PRT("CVI_TDE_AddDrawlineTask failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	s32Ret = CVI_TDE_EndJob(s32Handle, CVI_TRUE, CVI_TRUE, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_TDE_EndJob failed!\n");
		CVI_TDE_CancelJob(s32Handle);
		goto exit3;
	}

	SAMPLE_PRT("***JOB DONE***\n");
	SAMPLE_PRT("Successful, save output file:%s\n", filename_out);

	CVI_SYS_IonInvalidateCache(u64PhyAddrDst, pVirAddrDst, stDst2.u32Stride * stDst2.u32Height);
	TDEFrameSaveToFile(filename_out, pVirAddrDst, &stDst2);

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

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32Index;

	if (argc < 2) {
		SAMPLE_TDE_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (!strncmp(argv[1], "-h", 2)) {
		SAMPLE_TDE_Usage(argv[0]);
		return CVI_SUCCESS;
	}

	signal(SIGINT, SAMPLE_TDE_HandleSig);
	signal(SIGTERM, SAMPLE_TDE_HandleSig);

	s32Index = atoi(argv[1]);
	switch (s32Index) {
	case 0:
		s32Ret = SAMPLE_TDE_Rotate90();
		break;
	case 1:
		s32Ret = SAMPLE_TDE_Rotate270();
		break;
	case 2:
		s32Ret = SAMPLE_TDE_DrawLine();
		break;
	case 3:
		s32Ret = SAMPLE_TDE_QuickCopy();
		break;
	case 4:
		s32Ret = SAMPLE_TDE_MultiTask();
		break;
	default:
		SAMPLE_PRT("the index %d is invaild!\n", s32Index);
		SAMPLE_TDE_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (s32Ret == CVI_SUCCESS)
		SAMPLE_PRT("sample_tde exit success!\n");
	else
		SAMPLE_PRT("sample_tde exit abnormally!\n");

	return s32Ret;
}

