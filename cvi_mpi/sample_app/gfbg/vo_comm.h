#ifndef __VO_COMM_H__
#define __VO_COMM_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vo.h"
#include "cvi_math.h"
#include "cvi_buffer.h"
#include "cvi_comm_video.h"
#include "sample_comm.h"
typedef struct vo_ut_FILE {
	SIZE_S stSize;
	PIXEL_FORMAT_E enPixelFormat;
	CVI_CHAR filename[30];
} VO_UT_FILE;

CVI_S32 vo_sys_init(CVI_VOID);
CVI_S32 vo_sys_deinit(CVI_VOID);
CVI_S32 vo_init_by_fmt(CVI_S32 fmt, VO_DEV VoDev);
CVI_S32 vo_send_frame(VO_UT_FILE *fileptr, VO_DEV VoDev);
CVI_S32 vo_deinit(CVI_VOID);

#endif /* End of #ifndef __VO_COMM_H__*/
