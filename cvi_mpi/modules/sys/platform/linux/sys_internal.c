#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

#include <getopt.h>		/* getopt_long() */

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/prctl.h>

#include "cvi_buffer.h"
#include "cvi_errno.h"
#include "base_uapi.h"
#include "sys_internal.h"

//struct vdev dev_isp, dev_vpss, dev_disp, dev_ldc, dev_rgn,dev_dpu,dev_hdmi;

static CVI_S32 base_fd = -1, sys_fd = -1;
static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

static void *shared_mem;
static int shared_mem_usr_cnt;
static pthread_mutex_t shared_mem_lock = PTHREAD_MUTEX_INITIALIZER;


int open_device(const char *dev_name, CVI_S32 *fd)
{
	struct stat st;

	*fd = open(dev_name, O_RDWR /* required */  | O_NONBLOCK | O_CLOEXEC, 0);
	if (-1 == *fd) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return -1;
	}

	if (-1 == fstat(*fd, &st)) {
		close(*fd);
		CVI_TRACE_SYS(CVI_DBG_ERR, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		close(*fd);
		CVI_TRACE_SYS(CVI_DBG_ERR, "%s is no device\n", dev_name);
		return -ENODEV;
	}
	return 0;
}

CVI_S32 close_device(CVI_S32 *fd)
{
	if (*fd == -1)
		return -1;

	if (-1 == close(*fd)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "%s: fd(%d) failure\n", __func__, *fd);
		return -1;
	}

	*fd = -1;

	return CVI_SUCCESS;
}

CVI_S32 base_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&fd_lock);
	close_device(&base_fd);
	pthread_mutex_unlock(&fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_base_fd(CVI_VOID)
{
	pthread_mutex_lock(&fd_lock);
	if (base_fd <= 0) {
		if (open_device(BASE_DEV_NAME, &base_fd) == -1) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "base open fail\n");
			base_fd = -1;
		}
	}
	pthread_mutex_unlock(&fd_lock);

	return base_fd;
}

CVI_S32 sys_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&fd_lock);
	close_device(&sys_fd);
	pthread_mutex_unlock(&fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_sys_fd(CVI_VOID)
{
	pthread_mutex_lock(&fd_lock);
	if (sys_fd <= 0) {
		if (open_device(SYS_DEV_NAME, &sys_fd) == -1) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "SYS open fail\n");
			sys_fd = -1;
		}
	}
	pthread_mutex_unlock(&fd_lock);

	return sys_fd;
}

long get_diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec-t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

void *base_get_shm(void)
{
	CVI_S32 fd = get_base_fd();

	if (fd  == -1)
		return NULL;

	pthread_mutex_lock(&shared_mem_lock);
	if (shared_mem == NULL) {
		shared_mem = mmap(NULL, BASE_SHARE_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (shared_mem == MAP_FAILED) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "base dev mmap fail!\n");
			pthread_mutex_unlock(&shared_mem_lock);
			return NULL;
		}
	}

	shared_mem_usr_cnt++;
	pthread_mutex_unlock(&shared_mem_lock);

	return shared_mem;
}

void base_release_shm(void)
{
	pthread_mutex_lock(&shared_mem_lock);
	shared_mem_usr_cnt--;
	if (shared_mem_usr_cnt == 0 && shared_mem != NULL) {
		munmap((void *)shared_mem, BASE_SHARE_MEM_SIZE);
		shared_mem = NULL;
	}
	pthread_mutex_unlock(&shared_mem_lock);
}
