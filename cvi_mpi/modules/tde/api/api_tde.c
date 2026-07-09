#include "platform_tde.h"


CVI_S32 CVI_TDE_Open(CVI_VOID)
{
	return platform_tde_open();
}

CVI_VOID CVI_TDE_Close(CVI_VOID)
{
	return platform_tde_close();
}

TDE_HANDLE CVI_TDE_BeginJob(CVI_VOID)
{
	return platform_tde_begin_job();
}

CVI_S32 CVI_TDE_EndJob(TDE_HANDLE s32Handle, CVI_BOOL bSync, CVI_BOOL bBlock, CVI_U32 u32TimeOut)
{
	return platform_tde_end_job(s32Handle, bSync, bBlock, u32TimeOut);
}

CVI_S32 CVI_TDE_WaitAllDone(CVI_VOID)
{
	return platform_tde_wait_all_done();
}

CVI_S32 CVI_TDE_CancelJob(TDE_HANDLE s32Handle)
{
	return platform_tde_cancel_job(s32Handle);
}

CVI_S32 CVI_TDE_Rotate(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_ROTATE_ANGLE_E enRotateAngle)
{
	return platform_tde_rotate(s32Handle, pstSrc, pstDst, enRotateAngle);
}

CVI_S32 CVI_TDE_DrawLine(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_LINE_S *pstLine)
{
	return platform_tde_draw_line(s32Handle, pstSrc, pstDst, pstLine);
}

CVI_S32 CVI_TDE_QuickCopy(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc, TDE_SURFACE_S *pstDst)
{
	return platform_tde_quick_copy(s32Handle, pstSrc, pstDst);
}

