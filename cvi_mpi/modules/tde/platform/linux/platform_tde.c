#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>		/* low-level i/o */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "cvi_errno.h"
#include "cvi_debug.h"
#include "tde_ioctl.h"

#define TDE_DEV_NAME  "/dev/soph-tde"

#define TDE_CHECK_NULL_PTR(ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_TDE(CVI_DBG_ERR, "NULL pointer.\n"); \
			return CVI_ERR_TDE_NULL_PTR; \
		} \
	} while (0)

#define TDE_CHECK_FD(fd) \
	do { \
		if ((fd) == -1) { \
			CVI_TRACE_TDE(CVI_DBG_ERR, "fd=-1, Not open device.\n"); \
			return CVI_ERR_TDE_NOTREADY; \
		} \
	} while (0)

static CVI_S32 tde_fd = -1;
static pthread_mutex_t tde_fd_lock = PTHREAD_MUTEX_INITIALIZER;


static int tde_open_device(const char *dev_name, CVI_S32 *fd)
{
	struct stat st;

	*fd = open(dev_name, O_RDWR /* required */  | O_NONBLOCK | O_CLOEXEC, 0);
	if (-1 == *fd) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return -1;
	}

	if (-1 == fstat(*fd, &st)) {
		close(*fd);
		CVI_TRACE_TDE(CVI_DBG_ERR, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		close(*fd);
		CVI_TRACE_TDE(CVI_DBG_ERR, "%s is no device\n", dev_name);
		return -ENODEV;
	}
	return 0;
}

static CVI_S32 tde_close_device(CVI_S32 *fd)
{
	if (*fd == -1)
		return -1;

	if (-1 == close(*fd)) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "%s: fd(%d) failure\n", __func__, *fd);
		return -1;
	}

	*fd = -1;

	return CVI_SUCCESS;
}

static CVI_S32 get_tde_fd(CVI_VOID)
{
	pthread_mutex_lock(&tde_fd_lock);
	if (tde_fd <= 0) {
		if (tde_open_device(TDE_DEV_NAME, &tde_fd) == -1) {
			perror("TDE open fail\n");
			tde_fd = -1;
		}
	}
	pthread_mutex_unlock(&tde_fd_lock);

	return tde_fd;
}


/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 platform_tde_open(CVI_VOID)
{
	CVI_S32 fd = get_tde_fd();

	if (fd == -1) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "tde open fail\n");
		return CVI_ERR_TDE_NOTREADY;
	}

	return CVI_SUCCESS;
}

CVI_VOID platform_tde_close(CVI_VOID)
{
	tde_close_device(&tde_fd);
}

TDE_HANDLE platform_tde_begin_job(CVI_VOID)
{
	CVI_S32 fd = get_tde_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct tde_begin_job_cfg cfg = {0};

	TDE_CHECK_FD(fd);

	s32Ret = tde_begin_job(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "tde_begin_job fail.\n");
		return TDE_INVALID_HANDLE;
	}

	return cfg.handle;
}

CVI_S32 platform_tde_end_job(TDE_HANDLE s32Handle, CVI_BOOL bSync, CVI_BOOL bBlock, CVI_U32 u32TimeOut)
{
	CVI_S32 fd = get_tde_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct tde_end_job_cfg cfg = {0};

	TDE_CHECK_FD(fd);

	cfg.handle = s32Handle;
	cfg.bSync = bSync;
	cfg.bBlock = bBlock;
	cfg.u32TimeOut = u32TimeOut;

	s32Ret = tde_end_job(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "tde_end_job fail.\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_tde_wait_all_done(CVI_VOID)
{
	CVI_S32 fd = get_tde_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	TDE_CHECK_FD(fd);

	s32Ret = tde_wait_all_done(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "tde_wait_all_done fail.\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_tde_cancel_job(TDE_HANDLE s32Handle)
{
	CVI_S32 fd = get_tde_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct tde_cancel_job_cfg cfg = {0};

	TDE_CHECK_FD(fd);
	cfg.handle = s32Handle;

	s32Ret = tde_cancel_job(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "tde_cancel_job fail.\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_tde_rotate(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_ROTATE_ANGLE_E enRotateAngle)
{
	CVI_S32 fd = get_tde_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct tde_rotate_cfg cfg = {0};

	TDE_CHECK_FD(fd);
	TDE_CHECK_NULL_PTR(pstSrc);
	TDE_CHECK_NULL_PTR(pstDst);

	cfg.handle = s32Handle;
	cfg.src = *pstSrc;
	cfg.dst = *pstDst;
	cfg.angle = enRotateAngle;

	s32Ret = tde_rotate(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "tde_rotation fail.\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_tde_draw_line(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_LINE_S *pstLine)
{
		CVI_S32 fd = get_tde_fd();
		CVI_S32 s32Ret = CVI_SUCCESS;
		struct tde_draw_line_cfg cfg = {0};

		TDE_CHECK_FD(fd);
		TDE_CHECK_NULL_PTR(pstSrc);
		TDE_CHECK_NULL_PTR(pstDst);
		TDE_CHECK_NULL_PTR(pstLine);

		cfg.handle = s32Handle;
		cfg.src = *pstSrc;
		cfg.dst = *pstDst;
		cfg.line = *pstLine;

		s32Ret = tde_draw_line(fd, &cfg);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_TDE(CVI_DBG_ERR, "tde_draw_line fail.\n");
			return s32Ret;
		}

		return s32Ret;
}

CVI_S32 platform_tde_quick_copy(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc, TDE_SURFACE_S *pstDst)
{
	CVI_S32 fd = get_tde_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct tde_quick_copy_cfg cfg = {0};

	TDE_CHECK_FD(fd);
	TDE_CHECK_NULL_PTR(pstSrc);
	TDE_CHECK_NULL_PTR(pstDst);

	cfg.handle = s32Handle;
	cfg.src = *pstSrc;
	cfg.dst = *pstDst;

	s32Ret = tde_quick_copy(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_TDE(CVI_DBG_ERR, "tde_quick_copy fail.\n");
		return s32Ret;
	}

	return s32Ret;
}

