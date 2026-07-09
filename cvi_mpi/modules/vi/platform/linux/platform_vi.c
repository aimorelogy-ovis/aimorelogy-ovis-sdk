#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <inttypes.h>

#include "cvi_sns_ctrl.h"
#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "gdc_mesh.h"
#include "vi_ioctl.h"
#include "sys_internal.h"
#include "platform_vi.h"

#define GDC_SUPPORT_FMT(fmt)							\
	((fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||		\
	 (fmt == PIXEL_FORMAT_YUV_400))

typedef CVI_VOID(*pfnChnMirrorFlip)(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eChnMirrorFlip);
static pfnChnMirrorFlip s_pfnDevMirrorFlip[VI_MAX_DEV_NUM];

struct vi_pm_s {
	VI_PM_OPS_S	stOps;
	CVI_VOID	*pvData;
};

struct cvi_gdc_mesh g_vi_mesh[VI_MAX_CHN_NUM];

struct vi_dbg_th_info_s {
	CVI_U8    th_enable;
	pthread_t vi_dbg_thread;
};
struct vi_dbg_th_info_s gViDbgTH;

static CVI_S32 vi_fd = -1;
static pthread_mutex_t vi_fd_lock = PTHREAD_MUTEX_INITIALIZER;

static CVI_S32 vi_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&vi_fd_lock);
	close_device(&vi_fd);
	pthread_mutex_unlock(&vi_fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_vi_fd(CVI_VOID)
{
	pthread_mutex_lock(&vi_fd_lock);
	if (vi_fd <= 0) {
		if (open_device(VI_DEV_NAME, &vi_fd) == -1) {
			perror("VI open fail\n");
			vi_fd = -1;
		}
	}
	pthread_mutex_unlock(&vi_fd_lock);

	return vi_fd;
}

CVI_S32 vi_is_closed(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_FALSE;

	pthread_mutex_lock(&vi_fd_lock);
	if (vi_fd <= 0)
		s32Ret = CVI_TRUE;
	pthread_mutex_unlock(&vi_fd_lock);

	return s32Ret;
}

/**************************************************************************
 *   Internal APIs for vi only
 **************************************************************************/

static CVI_VOID *vi_dbg_handler(CVI_VOID *data)
{
	CVI_S32 fd = -1;
	fd_set rfds;
	CVI_S32 ret = CVI_SUCCESS;

	UNUSED(data);

	prctl(PR_SET_NAME, "vi_dbg_handler");

	fd = get_vi_fd();

	gViDbgTH.th_enable = CVI_TRUE;

	while (gViDbgTH.th_enable) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		ret = select(fd + 1, &rfds, NULL, NULL, NULL);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_dbg_thread select error\n");
			break;
		}

		//Cat vi_dbg/mipi_rx if error
		if (FD_ISSET(fd, &rfds) && gViDbgTH.th_enable) {
			system("cat /proc/soph/mipi-rx");
			system("cat /proc/soph/vi_dbg");
		}
	}
	CVI_TRACE_VI(CVI_DBG_INFO, "-\n");

	pthread_exit(NULL);
}

static CVI_S32 _vi_chn_enable_mirror_flip(VI_PIPE ViPipe, VI_CHN ViChn, bool bFlip, bool bMirror)
{
	ISP_SNS_MIRRORFLIP_TYPE_E eChnMirrorFlip;

	if (bMirror && bFlip)
		eChnMirrorFlip = ISP_SNS_MIRROR_FLIP;
	else if (bMirror)
		eChnMirrorFlip = ISP_SNS_MIRROR;
	else if (bFlip)
		eChnMirrorFlip = ISP_SNS_FLIP;
	else
		eChnMirrorFlip = ISP_SNS_NORMAL;

	if (eChnMirrorFlip != ISP_SNS_NORMAL && !s_pfnDevMirrorFlip[ViChn]) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI chn mirror/flip do not support this sensor.");
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	if (s_pfnDevMirrorFlip[ViPipe])
		s_pfnDevMirrorFlip[ViPipe](ViPipe, eChnMirrorFlip);

	return CVI_SUCCESS;
}

static CVI_S32 _vi_update_rotation_mesh(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E enRotation)
{
	struct vi_chn_rot_cfg cfg;
	CVI_S32 fd = get_vi_fd();

	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.enRotation = enRotation;
	if (vi_sdk_set_chn_rotation(fd, &cfg) != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI Set Chn(%d) Rotation(%d) fail\n", ViChn, enRotation);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _vi_update_ldc_mesh(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr,
	ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U64 paddr, paddr_old;
	CVI_VOID *vaddr, *vaddr_old;
	struct cvi_gdc_mesh *pmesh = &g_vi_mesh[ViChn];
	CVI_S32 s32Ret;
	char mesh_name[128];

	if (!pstLDCAttr->bEnable) {
		if (enRotation != ROTATION_0)
			return _vi_update_rotation_mesh(ViPipe, ViChn, enRotation);
		else
			return CVI_SUCCESS;
	}

	snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
	s32Ret = gdc_gen_ldcmesh(u32Width, u32Height, &pstLDCAttr->stAttr,
				mesh_name, &paddr, &vaddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Chn(%d) gen mesh fail\n", ViChn);
		return s32Ret;
	}

	pthread_mutex_lock(&pmesh->lock);
	if (pmesh->paddr) {
		paddr_old = pmesh->paddr;
		vaddr_old = pmesh->vaddr;
	} else {
		paddr_old = 0;
		vaddr_old = NULL;
	}
	pmesh->paddr = paddr;
	pmesh->vaddr = vaddr;
	pthread_mutex_unlock(&pmesh->lock);

	if (paddr_old && paddr_old != DEFAULT_MESH_PADDR)
		CVI_SYS_IonFree(paddr_old, vaddr_old);

	CVI_TRACE_VI(CVI_DBG_DEBUG, "ViPipe(%d) ViChn(%d) mesh base(%#"PRIx64") vaddr(%p)\n"
		, ViPipe, ViChn, paddr, vaddr);

	CVI_S32	fd = get_vi_fd();
	struct vi_chn_ldc_cfg cfg;

	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.enRotation = enRotation;
	cfg.stLDCAttr = *pstLDCAttr;
	cfg.meshHandle = paddr;
	if (vi_sdk_set_chn_ldc(fd, &cfg) != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI Set Chn(%d) LDC fail\n", ViChn);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Internal APIs for other modules
 **************************************************************************/

CVI_S32 platform_vi_suspend(void)
{
	return CVI_SUCCESS;
}

CVI_S32 platform_vi_resume(void)
{
	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setbypassfrm(CVI_U32 snr_num, CVI_U8 bypass_num)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();

	s32Ret = vi_sdk_set_bypass_frm(fd, snr_num, bypass_num);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_bypass_frm ioctl failed errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 platform_vi_setdevnum(CVI_U32 devNum)
{
	UNUSED(devNum);

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevnum(CVI_U32 *devNum)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vi_sdk_get_dev_num(fd, devNum);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_num ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_querydevstatus(VI_PIPE ViPipe)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_BOOL bStatus;

	s32Ret = vi_sdk_get_dev_status(fd, ViPipe, &bStatus);
	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	return bStatus == CVI_TRUE ? CVI_SUCCESS : CVI_FAILURE;
}

CVI_S32 platform_vi_enablepatgen(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	s32Ret = vi_sdk_enable_pagten(fd, ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enable_pagten ioctl failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setdevattr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();

	VI_DEV_ATTR_S devAttr = *pstDevAttr;

	s32Ret = vi_sdk_set_dev_attr(fd, ViDev, &devAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "setDevAttr ioctl failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevattr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vi_sdk_get_dev_attr(fd, ViDev, pstDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_attr ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setdevattrex(VI_DEV ViDev, const VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();

	VI_DEV_ATTR_EX_S devAttrEx = *pstDevAttrEx;

	s32Ret = vi_sdk_set_dev_attr_ex(fd, ViDev, &devAttrEx);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "setDevAttr ioctl failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevattrex(VI_DEV ViDev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vi_sdk_get_dev_attr_ex(fd, ViDev, pstDevAttrEx);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_attr ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setdevbindattr(VI_DEV ViDev, const VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_DEV_BIND_PIPE_S devBindAttr;

	fd = get_vi_fd();

	devBindAttr = *pstDevBindAttr;

	s32Ret = vi_sdk_set_dev_bind_attr(fd, ViDev, &devBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "setDevBindAttr ioctl failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevbindattr(VI_DEV ViDev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_dev_bind_attr(fd, ViDev, pstDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_bind_attr ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setdevunbindattr(VI_DEV ViDev)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_set_dev_unbind_attr(fd, ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_dev_unbind_attr ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_enabledev(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_enable_dev(fd, ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enable_dev ioctl failed errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_disabledev(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_disable_dev(fd, ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "disable_dev ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_setdevtimingattr(VI_DEV ViDev, const VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_DEV_TIMING_ATTR_S stTimingAttr;

	fd = get_vi_fd();

	stTimingAttr = *pstTimingAttr;
	s32Ret = vi_sdk_set_dev_timing_attr(fd, ViDev, &stTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevtimingattr(VI_DEV ViDev, VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_dev_timing_attr(fd, ViDev, pstTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/* 2 for vi pipe */
CVI_S32 platform_vi_createpipe(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_PIPE_ATTR_S stPipeAttr;

	fd = get_vi_fd();

	stPipeAttr = *pstPipeAttr;

	s32Ret = vi_sdk_create_pipe(fd, ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_create_pipe ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_destroypipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	s32Ret = vi_sdk_destroy_pipe(fd, ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_destroy_pipe ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_startpipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	s32Ret = vi_sdk_start_pipe(fd, ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_start_pipe ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_stoppipe(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setpipeattr(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	if (pstPipeAttr->u32MaxW > VI_PIPE_ONLINE_MAX_WIDTH || pstPipeAttr->u32MaxH > VI_PIPE_ONLINE_MAX_HEIGHT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "u32MaxW(%d) or u32MaxH(%d) too large\n",
			pstPipeAttr->u32MaxW, pstPipeAttr->u32MaxH);
		return CVI_ERR_VI_INVALID_PARA;
	}

	CVI_S32 fd = get_vi_fd();
	VI_PIPE_ATTR_S pipeAttr = *pstPipeAttr;

	CVI_S32 s32Ret = vi_sdk_set_pipe_attr(fd, ViPipe, &pipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipeattr(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_pipe_attr(fd, ViPipe, pstPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setpipedumpattr(VI_PIPE ViPipe, const VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DUMP_ATTR_S dumpAttr;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	dumpAttr = *pstDumpAttr;
	s32Ret = vi_sdk_set_pipe_dump_attr(fd, ViPipe, &dumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_dump_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipedumpattr(VI_PIPE ViPipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_pipe_dump_attr(fd, ViPipe, pstDumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_dump_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

// get bayer from preraw
CVI_S32 platform_vi_getpipeframe(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_pipe_frame(fd, ViPipe, pstFrameInfo, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_releasepipeframe(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	stVideoFrame = *pstFrameInfo;
	s32Ret = vi_sdk_release_pipe_frame(fd, ViPipe, &stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_release_pipe_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_startsmoothrawdump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	VI_PIPE ViPipe = pstDumpInfo->ViPipe;
	CVI_U8 frm_num = 0;
	CVI_U64 phy_addr = 0, raw_blk_phy;
	struct sop_vip_isp_smooth_raw_param param;
	struct sop_vip_isp_raw_blk *raw_blk = CVI_NULL;
	VI_DEV_ATTR_S stDevAttr;

	fd = get_vi_fd();

	if (pstDumpInfo->u8BlkCnt < 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Need two ring buffer at least, now is %d\n", pstDumpInfo->u8BlkCnt);
		return CVI_ERR_VI_INVALID_PARA;
	}

	s32Ret = vi_sdk_get_dev_attr(fd, ViPipe, &stDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "sys busy\n");
		return s32Ret;
	}

	if ((stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ||
		(stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME) ||
		(stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME_FULL_RATE)) {
		frm_num = 2;
	} else {
		frm_num = 1;
	}

	frm_num = (pstDumpInfo->u8BlkCnt) * frm_num;

	if (CVI_SYS_IonAlloc(&raw_blk_phy, (void **)&raw_blk, "smooth_dump",
				frm_num * sizeof(struct sop_vip_isp_raw_blk)) != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "ion malloc raw_blk NG.\n");
		return CVI_ERR_VI_NOMEM;
	}

	for (CVI_U8 i = 0; i < frm_num; i++) {
		phy_addr = *(pstDumpInfo->phy_addr_list + i);
		if (phy_addr == 0) {
			if (raw_blk != CVI_NULL) {
				CVI_SYS_IonFree(raw_blk_phy, raw_blk);
				raw_blk = CVI_NULL;
			}
			CVI_TRACE_VI(CVI_DBG_ERR, "phy_addr is invalid\n");
			return CVI_ERR_VI_INVALID_PARA;
		}

		(raw_blk + i)->raw_dump.phy_addr = phy_addr;
		// CVI_TRACE_VI(CVI_DBG_DEBUG, "i=%d, phy_paddr(%#"PRIx64")\n", i, (raw_blk+i)->raw_dump.phy_addr);

		// set rawdump crop info
		(raw_blk + i)->crop_x = pstDumpInfo->stCropRect.s32X;
		(raw_blk + i)->crop_y = pstDumpInfo->stCropRect.s32Y;
		(raw_blk + i)->src_w = pstDumpInfo->stCropRect.u32Width;
		(raw_blk + i)->src_h = pstDumpInfo->stCropRect.u32Height;
		//CVI_TRACE_VI(CVI_DBG_DEBUG, "i (%d), crop_x(%d), crop_y(%d), src_w(%d), src_h(%d)\n",
		//	i, (raw_blk + i)->crop_x, (raw_blk + i)->crop_y,
		//	(raw_blk + i)->src_w, (raw_blk + i)->src_h);
	}

	param.raw_num = ViPipe;
	param.frm_num = frm_num;
	param.raw_blk_phyaddr = raw_blk_phy;

	s32Ret = vi_sdk_start_smooth_rawdump(fd, ViPipe, &param);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_start_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		CVI_SYS_IonFree(raw_blk_phy, raw_blk);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	if (raw_blk != CVI_NULL) {
		CVI_SYS_IonFree(raw_blk_phy, raw_blk);
		raw_blk = CVI_NULL;
	}

	return s32Ret;
}

CVI_S32 platform_vi_stopsmoothrawdump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	VI_PIPE ViPipe = pstDumpInfo->ViPipe;
	struct sop_vip_isp_smooth_raw_param param;

	fd = get_vi_fd();

	param.raw_num = ViPipe;

	s32Ret = vi_sdk_stop_smooth_rawdump(fd, ViPipe, &param);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_stop_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getsmoothrawdump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_smooth_rawdump(fd, ViPipe, pstVideoFrame, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_putsmoothrawdump(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame[2];

	fd = get_vi_fd();

	stVideoFrame[0] = pstVideoFrame[0];
	stVideoFrame[1] = pstVideoFrame[1];
	s32Ret = vi_sdk_put_smooth_rawdump(fd, ViPipe, stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_put_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_querypipestatus(VI_PIPE ViPipe, VI_PIPE_STATUS_S *pstStatus)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vi_sdk_get_pipe_status(fd, ViPipe, pstStatus);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_status ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setpipeframesource(VI_PIPE ViPipe, const VI_PIPE_FRAME_SOURCE_E enSource)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE_FRAME_SOURCE_E src = enSource;

	if (enSource < 0 || enSource >= VI_PIPE_FRAME_SOURCE_BUTT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enSource(%d)is invalid\n", enSource);
		return CVI_ERR_VI_INVALID_PARA;
	}

	fd = get_vi_fd();

	s32Ret = vi_sdk_set_pipe_frm_src(fd, ViPipe, &src);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_frm_src ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipeframesource(VI_PIPE ViPipe, VI_PIPE_FRAME_SOURCE_E *penSource)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_pipe_frm_src(fd, ViPipe, penSource);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_frm_src ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_sendpiperaw(CVI_U32 u32PipeNum, VI_PIPE PipeId[], const VIDEO_FRAME_INFO_S *pstVideoFrame[],
			   CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	UNUSED(s32MilliSec);

	fd = get_vi_fd();

	for (CVI_U32 i = 0; i < u32PipeNum; ++i) {
		VI_PIPE pipeid = PipeId[i];

		VIDEO_FRAME_INFO_S stVideoFrm = *pstVideoFrame[i];

		s32Ret = vi_sdk_send_pipe_raw(fd, pipeid, &stVideoFrm);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_send_pipe_raw ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipefd(VI_PIPE ViPipe)
{
	CVI_S32 fd = -1;

	UNUSED(ViPipe);

	fd = get_vi_fd();

	if (fd <= 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe fd fail\n");
		return CVI_FAILURE;
	}

	return fd;
}

CVI_S32 platform_vi_closefd(void)
{
	vi_dev_close();
	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setpipecrop(VI_PIPE ViPipe, const CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();
	CROP_INFO_S stCropInfo = *pstCropInfo;

	if (pstCropInfo->bEnable == CVI_TRUE) {
		s32Ret = vi_sdk_set_pipe_crop(fd, ViPipe, &stCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_crop ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 platform_vi_getpipecrop(VI_PIPE ViPipe, CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_pipe_crop(fd, ViPipe, pstCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_crop ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_attachvbpool(VI_PIPE ViPipe, VI_CHN ViChn, VB_POOL VbPool)
{
	CVI_S32 fd = get_vi_fd();
	struct vi_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.VbPool = VbPool;

	return vi_sdk_attach_vbpool(fd, &cfg);
}

CVI_S32 platform_vi_detachvbpool(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 fd = get_vi_fd();
	struct vi_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;

	return vi_sdk_detach_vbpool(fd, &cfg);
}

CVI_S32 platform_vi_setchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	if (pstChnAttr->stFrameRate.s32SrcFrameRate != pstChnAttr->stFrameRate.s32DstFrameRate)
		CVI_TRACE_VI(CVI_DBG_WARN, "FrameRate ctrl, src(%d) dst(%d), not support yet.\n",
					  pstChnAttr->stFrameRate.s32SrcFrameRate,
					  pstChnAttr->stFrameRate.s32DstFrameRate);

	if (pstChnAttr->enPixelFormat != PIXEL_FORMAT_NV21 &&
		pstChnAttr->enPixelFormat != PIXEL_FORMAT_NV12 &&
		(pstChnAttr->enPixelFormat < PIXEL_FORMAT_YUYV || pstChnAttr->enPixelFormat > PIXEL_FORMAT_VYUY)) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support %d\n", pstChnAttr->enPixelFormat);
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = vi_sdk_set_chn_attr(fd, ViPipe, ViChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_enablechn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CHN_ATTR_S stChnAttr;

	if (ViChn >= (VI_MAX_CHN_NUM + VI_MAX_EXT_CHN_NUM) || ViChn < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, " invalid chn-id(%d)\n", ViChn);
		return CVI_ERR_VI_INVALID_CHNID;
	}



	s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	if (stChnAttr.stSize.u32Width == 0 &&
		stChnAttr.stSize.u32Height == 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, " Call SetChnAttr first(%d)\n", ViChn);
		return CVI_ERR_VI_FAILED_NOTCONFIG;
	}

	if (ViChn >= VI_EXT_CHN_START) {
		CVI_TRACE_VI(CVI_DBG_ERR, " not support ext chn(%d)\n", ViChn);
	} else {
		if (ViChn < VI_MAX_CHN_NUM)
			_vi_chn_enable_mirror_flip(ViPipe, ViChn,
				stChnAttr.bFlip, stChnAttr.bMirror);

		s32Ret = vi_sdk_enable_chn(fd, ViPipe, ViChn);
		if (s32Ret < 0) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_enable_chn ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		if (s32Ret == CVI_SUCCESS_ALL_CHN) {
			struct sched_param param;
			pthread_attr_t attr;

			param.sched_priority = 85;

			pthread_attr_init(&attr);
			pthread_attr_setschedpolicy(&attr, SCHED_RR);
			pthread_attr_setschedparam(&attr, &param);
			pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

			pthread_create(&gViDbgTH.vi_dbg_thread, &attr, (void *)vi_dbg_handler, NULL);
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_disablechn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR mesh_name[128];

	if (ViChn >= (VI_MAX_CHN_NUM + VI_MAX_EXT_CHN_NUM)) {
		CVI_TRACE_VI(CVI_DBG_ERR, " invalid chn-id(%d)\n", ViChn);
		return CVI_ERR_VI_INVALID_CHNID;
	}

	if (ViChn < VI_MAX_PHY_CHN_NUM) {
		gViDbgTH.th_enable = CVI_FALSE;
		s32Ret = vi_sdk_disable_chn(fd, ViPipe, ViChn);
		if (s32Ret < 0) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_disable_chn ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		if (s32Ret == CVI_SUCCESS_ALL_CHN) {
			pthread_join(gViDbgTH.vi_dbg_thread, NULL);
		}
	} else if (ViChn >= VI_EXT_CHN_START) {
		CVI_TRACE_VI(CVI_DBG_ERR, " not support ext chn(%d)\n", ViChn);
	}

	snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
	gdc_free_cur_tsk_mesh(mesh_name);

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setchncrop(VI_PIPE ViPipe, VI_CHN ViChn, const VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();
	VI_CROP_INFO_S cropInfo;

	if (pstCropInfo->stCropRect.s32X % 2 || pstCropInfo->stCropRect.s32Y % 2 ||
		pstCropInfo->stCropRect.u32Width % 2 || pstCropInfo->stCropRect.u32Height % 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d)_w(%d)_h(%d) must be multiple of 2.\n",
					pstCropInfo->stCropRect.s32X,
					pstCropInfo->stCropRect.s32Y,
					pstCropInfo->stCropRect.u32Width,
					pstCropInfo->stCropRect.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstCropInfo->stCropRect.s32X < 0 || pstCropInfo->stCropRect.s32Y < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d) is invalid.\n",
					pstCropInfo->stCropRect.s32X,
					pstCropInfo->stCropRect.s32Y);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstCropInfo->bEnable == CVI_TRUE) {
		cropInfo = *pstCropInfo;
		s32Ret = vi_sdk_set_chn_crop(fd, ViPipe, ViChn, &cropInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_chn_crop ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getchncrop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_crop(fd, ViPipe, ViChn, pstCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_crop ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getchnframe(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vi_sdk_get_chn_frame(fd, ViPipe, ViChn, pstFrameInfo, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_releasechnframe(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stFrameInfo;

	stFrameInfo = *pstFrameInfo;
	s32Ret = vi_sdk_release_chn_frame(fd, ViPipe, ViChn, &stFrameInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_release_chn_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_querychnstatus(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_STATUS_S *pstChnStatus)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vi_sdk_get_chn_status(fd, ViPipe, ViChn, pstChnStatus);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_status ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setchnrotation(VI_PIPE ViPipe, VI_CHN ViChn, const ROTATION_E enRotation)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct sop_isp_sc_online online;
	VI_CHN_ATTR_S stChnAttr;
	struct vi_chn_ldc_cfg ldc_cfg;

	online.raw_num = ViPipe;
	s32Ret = vi_get_online2sc(fd, &online);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "sys busy.\n");
		return CVI_ERR_VI_BUSY;
	}

	if (online.is_sc_online) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI Rotation not support online2sc.\n");
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = vi_sdk_get_chn_ldc(fd, ViPipe, ViChn, &ldc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_ldc ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	if (!GDC_SUPPORT_FMT(stChnAttr.enPixelFormat)) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support format(%d).\n", stChnAttr.enPixelFormat);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (enRotation == ROTATION_180) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VI_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VI(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (ldc_cfg.stLDCAttr.bEnable) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set rotation fail, please add rotation to ldc.\n");
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	} else
		return _vi_update_rotation_mesh(ViPipe, ViChn, enRotation);
	return s32Ret;
}

CVI_S32 platform_vi_getchnrotation(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E *penRotation)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32	fd = get_vi_fd();
	struct vi_chn_rot_cfg rotCfg;

	rotCfg.ViPipe = ViPipe;
	rotCfg.ViChn = ViChn;
	s32Ret = vi_sdk_get_chn_rotation(fd, &rotCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_rotation ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	*penRotation = rotCfg.enRotation;

	return s32Ret;
}

CVI_S32 platform_vi_setchnldcattr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CHN_ATTR_S stChnAttr;
	struct vi_chn_rot_cfg rotCfg;
	CVI_S32	fd = get_vi_fd();
	struct sop_isp_sc_online online;

	online.raw_num = ViPipe;
	s32Ret = vi_get_online2sc(fd, &online);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "sys busy.\n");
		return CVI_ERR_VI_BUSY;
	}

	if (online.is_sc_online) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI Rotation not support online2sc.\n");
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	if (!GDC_SUPPORT_FMT(stChnAttr.enPixelFormat)) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support format(%d).\n", stChnAttr.enPixelFormat);
		return CVI_ERR_VI_INVALID_PARA;
	}

	rotCfg.ViPipe = ViPipe;
	rotCfg.ViChn = ViChn;

	s32Ret = vi_sdk_get_chn_rotation(fd, &rotCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_rotation ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return _vi_update_ldc_mesh(ViPipe, ViChn, pstLDCAttr, rotCfg.enRotation,
		stChnAttr.stSize.u32Width, stChnAttr.stSize.u32Height);
}

CVI_S32 platform_vi_getchnldcattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	struct vi_chn_ldc_cfg ldcCfg;
	CVI_S32	fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_ldc(fd, ViPipe, ViChn, &ldcCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "ViPipe(%d) ViChn(%d) get chn LDC attr fail\n", ViPipe, ViChn);
		return s32Ret;
	}

	memcpy(pstLDCAttr, &ldcCfg.stLDCAttr, sizeof(*pstLDCAttr));

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_regchnflipmirrorcallback(VI_PIPE ViPipe, VI_DEV ViDev, void *pvData)
{
	UNUSED(ViPipe);

	s_pfnDevMirrorFlip[ViDev] = (pfnChnMirrorFlip)pvData;

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_unregchnflipmirrorcallback(VI_PIPE ViPipe, VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(ViPipe);

	s_pfnDevMirrorFlip[ViDev] = CVI_NULL;

	return s32Ret;
}

CVI_S32 platform_vi_setchnflipmirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL bFlip, CVI_BOOL bMirror)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	struct vi_chn_flip_mirror_cfg cfg;

	fd = get_vi_fd();

	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.bFlip = bFlip;
	cfg.bMirror = bMirror;
	s32Ret = vi_sdk_set_chn_flip_mirror(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_chn_flip_mirror ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return _vi_chn_enable_mirror_flip(ViPipe, ViChn, bFlip, bMirror);
}

CVI_S32 platform_vi_getchnflipmirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL *pbFlip, CVI_BOOL *pbMirror)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	struct vi_chn_flip_mirror_cfg cfg;

	fd = get_vi_fd();

	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	s32Ret = vi_sdk_get_chn_flip_mirror(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_flip_mirror ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	*pbFlip = cfg.bFlip;
	*pbMirror = cfg.bMirror;

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_dumphwregistertofile(VI_PIPE ViPipe, FILE *fp, VI_DUMP_REGISTER_TABLE_S *pstRegTbl)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U64 json_phy = 0;
	void *json_vir = NULL;
	CVI_S32 fd = -1;
	struct ip_info ip_info;

	UNUSED(pstRegTbl);

	fd = get_vi_fd();

#define DUMP_SIZE 524288

	s32Ret = CVI_SYS_IonAlloc_Cached(&json_phy, &json_vir, "VI_DMA_BUF", DUMP_SIZE);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI ion alloc size(%d) failed.\n", DUMP_SIZE);
		return CVI_ERR_VI_NOMEM;
	}

	ip_info.phy_addr = json_phy;
	ip_info.size = DUMP_SIZE;

	s32Ret = vi_sdk_dump_register(fd, ViPipe, &ip_info);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "dump_register fail\n");
		goto free_json;
	}

	fwrite(json_vir, 1, ip_info.size, fp);

free_json:
	CVI_SYS_IonFree(json_phy, json_vir);

	return s32Ret;
}

CVI_S32 platform_vi_setdevrxframecount(VI_DEV ViDev, CVI_U32 u32RxFrameCount)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	s32Ret = vi_sdk_set_dev_rx_frame_count(fd, ViDev, u32RxFrameCount);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_dev_rx_frame_count fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_getdevrxframecount(VI_DEV ViDev, CVI_U32 *pu32FrameCount)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_dev_rx_frame_count(fd, ViDev, pu32FrameCount);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_rx_frame_count fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_aiispcfg(VI_AI_ISP_CFG_S *pstAiIspCfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	if (pstAiIspCfg == CVI_NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "pstAiIspCfg is NULL\n");
		return CVI_ERR_VI_INVALID_NULL_PTR;
	}

	s32Ret = vi_sdk_set_ai_isp_cfg(fd, pstAiIspCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_ai_isp_cfg failed, ViPipe:%d, ret:0x%x\n",
			pstAiIspCfg->viPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_aiispinfo(VI_AI_ISP_INFO_WRAP_S *pstAiIspWrapInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	fd = get_vi_fd();

	if (pstAiIspWrapInfo == CVI_NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "pstAiIspWrapInfo is NULL\n");
		return CVI_ERR_VI_INVALID_NULL_PTR;
	}

	if (pstAiIspWrapInfo->enCmd == VI_CMD_GET_AI_ISP_RAW) {
		s32Ret = vi_sdk_get_ai_isp_raw(fd, pstAiIspWrapInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_ai_isp_raw failed, ViPipe:%d, ret:0x%x\n",
				pstAiIspWrapInfo->viPipe, s32Ret);
			return s32Ret;
		}

	} else {
		s32Ret = vi_sdk_put_ai_isp_raw(fd, pstAiIspWrapInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_put_ai_isp_raw failed, ViPipe:%d, ret:0x%x\n",
				pstAiIspWrapInfo->viPipe, s32Ret);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

/**
 * @deprecated
 */
CVI_S32 platform_vi_regpmcallback(VI_DEV ViDev, VI_PM_OPS_S *pstPmOps, void *pvData)
{
	UNUSED(ViDev);
	UNUSED(pstPmOps);
	UNUSED(pvData);

	CVI_TRACE_VI(CVI_DBG_ERR, "not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 platform_vi_unregpmcallback(VI_DEV ViDev)
{
	UNUSED(ViDev);

	CVI_TRACE_VI(CVI_DBG_ERR, "not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 platform_vi_trig_ahd(VI_PIPE ViPipe, CVI_U8 u8AHDSignal)
{
	UNUSED(ViPipe);
	UNUSED(u8AHDSignal);

	CVI_TRACE_VI(CVI_DBG_ERR, "not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 platform_vi_setextchnattr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	UNUSED(pstExtChnAttr);

	CVI_TRACE_VI(CVI_DBG_ERR, "not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 platform_vi_getextchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	UNUSED(ViPipe);
	UNUSED(ViChn);
	UNUSED(pstExtChnAttr);

	CVI_TRACE_VI(CVI_DBG_ERR, "not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 platform_vi_setmipibinddev(VI_DEV ViDev, MIPI_DEV MipiDev)
{
	UNUSED(ViDev);
	UNUSED(MipiDev);

	CVI_TRACE_VI(CVI_DBG_ERR, "not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 platform_vi_getmipibinddev(VI_DEV ViDev, MIPI_DEV *pMipiDev)
{
	UNUSED(ViDev);
	UNUSED(pMipiDev);

	CVI_TRACE_VI(CVI_DBG_ERR, "not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}
