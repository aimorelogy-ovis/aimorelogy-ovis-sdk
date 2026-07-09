#ifndef __TDE_IOCTL_H__
#define __TDE_IOCTL_H__

#include <sys/ioctl.h>
#include "tde_uapi.h"

/* Configured from user  */
CVI_S32 tde_begin_job(CVI_S32 fd, struct tde_begin_job_cfg *cfg);
CVI_S32 tde_end_job(CVI_S32 fd, struct tde_end_job_cfg *cfg);
CVI_S32 tde_wait_all_done(CVI_S32 fd);
CVI_S32 tde_cancel_job(CVI_S32 fd, struct tde_cancel_job_cfg *cfg);
CVI_S32 tde_rotate(CVI_S32 fd, struct tde_rotate_cfg *cfg);
CVI_S32 tde_draw_line(CVI_S32 fd, struct tde_draw_line_cfg *cfg);
CVI_S32 tde_quick_copy(CVI_S32 fd, struct tde_quick_copy_cfg *cfg);


#endif
