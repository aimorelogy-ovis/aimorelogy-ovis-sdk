#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "tde_ioctl.h"


CVI_S32 tde_begin_job(CVI_S32 fd, struct tde_begin_job_cfg *cfg)
{
	return ioctl(fd, CVI_TDE_BEJIN_JOB, cfg);
}

CVI_S32 tde_end_job(CVI_S32 fd, struct tde_end_job_cfg *cfg)
{
	return ioctl(fd, CVI_TDE_END_JOB, cfg);
}

CVI_S32 tde_wait_all_done(CVI_S32 fd)
{
	return ioctl(fd, CVI_TDE_WAIT_ALL_DONE, NULL);
}

CVI_S32 tde_cancel_job(CVI_S32 fd, struct tde_cancel_job_cfg *cfg)
{
	return ioctl(fd, CVI_TDE_CANCEL_JOB, cfg);
}

CVI_S32 tde_rotate(CVI_S32 fd, struct tde_rotate_cfg *cfg)
{
	return ioctl(fd, CVI_TDE_ROTATE, cfg);
}

CVI_S32 tde_draw_line(CVI_S32 fd, struct tde_draw_line_cfg *cfg)
{
	return ioctl(fd, CVI_TDE_DRAW_LINE, cfg);
}

CVI_S32 tde_quick_copy(CVI_S32 fd, struct tde_quick_copy_cfg *cfg)
{
	return ioctl(fd, CVI_TDE_QUICK_COPY, cfg);
}

