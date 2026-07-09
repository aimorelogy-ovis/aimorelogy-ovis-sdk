#include <cvi_comm_tde.h>


CVI_S32 platform_tde_open(CVI_VOID);

CVI_VOID platform_tde_close(CVI_VOID);

TDE_HANDLE platform_tde_begin_job(CVI_VOID);

CVI_S32 platform_tde_end_job(TDE_HANDLE s32Handle, CVI_BOOL bSync, CVI_BOOL bBlock, CVI_U32 u32TimeOut);

CVI_S32 platform_tde_wait_all_done(CVI_VOID);

CVI_S32 platform_tde_cancel_job(TDE_HANDLE s32Handle);

CVI_S32 platform_tde_rotate(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_ROTATE_ANGLE_E enRotateAngle);

CVI_S32 platform_tde_draw_line(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_LINE_S *pstLine);

CVI_S32 platform_tde_quick_copy(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc, TDE_SURFACE_S *pstDst);

