#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>

#include "cvi_math.h"
#include "cvi_sys.h"
#include "cvi_tde.h"

#include "tde_ut_comm.h"
#include "md5sum.h"

CVI_S32 TDECompareWithMD5(const CVI_CHAR *md5sum, CVI_VOID *buffer, TDE_SURFACE_S *pstDst)
{
	CVI_S32 i, result = CVI_SUCCESS;
	MD5_CTX md5_ctx;
	CVI_CHAR md[MD5_DIGEST_LENGTH];
	CVI_CHAR md_str[33];
	CVI_CHAR *p = md_str;
	CVI_S32 s32Index = 0;

	memset(md, 0, MD5_DIGEST_LENGTH);

	MD5_Init(&md5_ctx);
	MD5_Update(&md5_ctx, buffer, pstDst->u32Stride * pstDst->u32Height);
	MD5_Final((unsigned char *)md, &md5_ctx);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		s32Index += snprintf(p + s32Index, 33 - s32Index, "%02x", md[i]);

	if (strncmp(md5sum, md_str, 32)) {
		UT_PRT("md5sum error, frame md5sum:%s\n", md_str);
		result = CVI_FAILURE;
	}

	return result;
}

CVI_S32 TDEFrameSaveToFile(const CVI_CHAR *filename, CVI_VOID *buffer, TDE_SURFACE_S *pstDst)
{
	FILE *fp;
	CVI_U32 i;
	CVI_S32 result = CVI_SUCCESS, s32len;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		UT_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	for (i = 0; i < pstDst->u32Height; i++) {
		s32len = fwrite(buffer, pstDst->u32Width, 4, fp);
		if (s32len <= 0) {
			UT_PRT("fwrite data(%d) error\n", i);
			result = CVI_FAILURE;
			break;
		}
		buffer += pstDst->u32Stride;
	}

	fclose(fp);

	return result;
}

CVI_S32 TDEFileToBuffer(const CVI_CHAR *filename, CVI_VOID *buffer, SIZE_S *pstSize)
{
	FILE *fp;
	CVI_U32 i;
	CVI_S32 result = CVI_SUCCESS, s32len;

	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		UT_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	for (i = 0; i < pstSize->u32Height; i++) {
		s32len = fread(buffer, pstSize->u32Width, 4, fp);
		if (s32len <= 0) {
			UT_PRT("fread data(%d) error\n", i);
			result = CVI_FAILURE;
			break;
		}
		buffer += pstSize->u32Width * 4;
	}

	fclose(fp);

	return result;
}

