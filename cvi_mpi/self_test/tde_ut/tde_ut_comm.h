#ifndef __TDE_UT_COMM_H__
#define __TDE_UT_COMM_H__

#include "ut_comm.h"

CVI_S32 TDECompareWithMD5(const CVI_CHAR *md5sum, CVI_VOID *buffer, TDE_SURFACE_S *pstDst);
CVI_S32 TDEFrameSaveToFile(const CVI_CHAR *filename, CVI_VOID *buffer, TDE_SURFACE_S *pstDst);
CVI_S32 TDEFileToBuffer(const CVI_CHAR *filename, CVI_VOID *buffer, SIZE_S *pstSize);

#endif
