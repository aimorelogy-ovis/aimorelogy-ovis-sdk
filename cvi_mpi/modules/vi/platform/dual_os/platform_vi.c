#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <inttypes.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "gdc_mesh.h"
#include "cvi_comm_vi.h"
#include "cvi_debug.h"
#include "msg_vi.h"
#include "cvi_msg_client.h"

#define GDC_SUPPORT_FMT(fmt)							\
	((fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||		\
	 (fmt == PIXEL_FORMAT_YUV_400))

/**************************************************************************
 *   Internal APIs for other modules
 **************************************************************************/
CVI_S32 platform_vi_suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SUSPEND, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_Suspend fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_RESUME, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_Resume fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_setbypassfrm(CVI_U32 snr_num, CVI_U8 bypass_num)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	VI_PIPE pipe = snr_num;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, pipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_BYPASS_FRM, &bypass_num,
				sizeof(CVI_U8), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set bypass frm fail, ViPipe:%d,s32Ret:%x\n", pipe, s32Ret);
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
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_NUM, (CVI_VOID *)devNum,
				sizeof(CVI_U32), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_GetDevNum fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_querydevstatus(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS, bStatus = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_QUERY_DEV_STATUS, (CVI_VOID *)&bStatus,
				sizeof(bStatus), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_QueryDevStatus fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return bStatus;
}

CVI_S32 platform_vi_enablepatgen(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS, bStatus = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_ENABLE_PATTERN, (CVI_VOID *)&bStatus,
				sizeof(bStatus), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_EnablePatt fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return bStatus;
}

CVI_S32 platform_vi_setdevattrex(VI_DEV ViDev, const VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_DEV_ATTR_EX, (CVI_VOID *)pstDevAttrEx,
				sizeof(VI_DEV_ATTR_EX_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set dev attr ex fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevattrex(VI_DEV ViDev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_ATTR_EX, pstDevAttrEx, sizeof(VI_DEV_ATTR_EX_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get dev attr ex fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_setdevattr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_DEV_ATTR, (CVI_VOID *)pstDevAttr,
				sizeof(VI_DEV_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set dev attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevattr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_ATTR, pstDevAttr, sizeof(VI_DEV_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get dev attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_setdevbindattr(VI_DEV ViDev, const VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DEV_BIND_PIPE_S devBindAttr;

	devBindAttr = *pstDevBindAttr;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_DEV_BIND_ATTR, &devBindAttr,
				  sizeof(VI_DEV_BIND_PIPE_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set dev bind attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevbindattr(VI_DEV ViDev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_BIND_ATTR, pstDevBindAttr,
				  sizeof(VI_DEV_BIND_PIPE_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get dev bind attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

//TODO maybe should be avoid
CVI_S32 platform_vi_setdevunbindattr(VI_DEV ViDev)
{
	UNUSED(ViDev);

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_enabledev(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_ENABLE_DEV, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enable dev fail,ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_disabledev(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DISABLE_DEV, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "diable dev fail,ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_setdevtimingattr(VI_DEV ViDev, const VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DEV_TIMING_ATTR_S stTimingAttr;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	stTimingAttr = *pstTimingAttr;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_DEV_TIMING_ATTR, (CVI_VOID *)&stTimingAttr,
				sizeof(VI_DEV_TIMING_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set dev timing attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}


	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevtimingattr(VI_DEV ViDev, VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViDev, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_DEV_TIMING_ATTR, (CVI_VOID *)pstTimingAttr,
				sizeof(VI_DEV_TIMING_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get dev timing attr fail, ViDev:%d,s32Ret:%x\n", ViDev, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/* 2 for vi pipe */
CVI_S32 platform_vi_createpipe(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_CREATE_PIPE, (CVI_VOID *)pstPipeAttr,
				sizeof(VI_PIPE_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "create pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_destroypipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DESTROY_PIPE, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "destroy pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_startpipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_START_PIPE, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "start pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_stoppipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_STOP_PIPE, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "stop pipe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setpipeattr(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_ATTR, (CVI_VOID *)pstPipeAttr,
				sizeof(VI_PIPE_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set pipe attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipeattr(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_ATTR, pstPipeAttr,
				  sizeof(VI_PIPE_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setpipedumpattr(VI_PIPE ViPipe, const VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 s32Ret;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_DUMP_ATTR, (CVI_VOID *)pstDumpAttr,
				sizeof(VI_DUMP_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set pipe dump attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipedumpattr(VI_PIPE ViPipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_DUMP_ATTR, pstDumpAttr, sizeof(VI_DUMP_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe dump attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

// get bayer from preraw
CVI_S32 platform_vi_getpipeframe(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VI_GET_PIPE_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S) * 2, &stPrivData, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get VI Pipe frame fail,VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_releasepipeframe(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_RELEASE_PIPE_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Release VI Pipe frame fail,VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_startsmoothrawdump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, 0, 0, 1);
	CVI_U64 pAddr = 0;
	CVI_VOID *vir_addr = NULL;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 size = 0;

	size = sizeof(CVI_U64) * pstDumpInfo->u8BlkCnt * 2;
	s32Ret = CVI_SYS_IonAlloc_Cached(&pAddr, NULL, "SMOOTH_DUMP_MSG", size);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI ion alloc size(%u) failed.\n", size);
		return CVI_ERR_VI_NOMEM;
	}

	vir_addr = CVI_SYS_Mmap(pAddr, size);
	if (vir_addr == NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI mmap failed.\n");
		CVI_SYS_IonFree(pAddr, vir_addr);
		return CVI_ERR_VI_NOMEM;
	}

	memcpy(vir_addr, pstDumpInfo->phy_addr_list, size);

	CVI_SYS_IonFlushCache(pAddr, vir_addr, size);

	stPrivData.as32PrivData[0] = pAddr & 0xFFFFFFFF;
	stPrivData.as32PrivData[1] = (pAddr >> 32) & 0xFFFFFFFF;
	stPrivData.as32PrivData[2] = size;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_START_SMOOTH_RAWDUMP, (CVI_VOID *)pstDumpInfo,
				sizeof(VI_SMOOTH_RAW_DUMP_INFO_S), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "StartSmoothRawDump fail, s32Ret:%x\n", s32Ret);
		CVI_SYS_Munmap(vir_addr, size);
		CVI_SYS_IonFree(pAddr, vir_addr);
		return s32Ret;
	}

	CVI_SYS_Munmap(vir_addr, size);
	s32Ret = CVI_SYS_IonFree(pAddr, vir_addr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "CVI_SYS_IonFree fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_stopsmoothrawdump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_STOP_SMOOTH_RAWDUMP, (CVI_VOID *)pstDumpInfo,
				sizeof(VI_SMOOTH_RAW_DUMP_INFO_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "StopSmoothRawDump fail, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getsmoothrawdump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, 0, 1);

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_SMOOTH_RAWDUMP, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S) * 2, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "GetSmoothRawDump fail, VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_putsmoothrawdump(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, 0, 1);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_PUT_SMOOTH_RAWDUMP, (CVI_VOID *)pstVideoFrame,
				sizeof(VIDEO_FRAME_INFO_S) * 2, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "PutSmoothRawDump fail, VIpipe:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_querypipestatus(VI_PIPE ViPipe, VI_PIPE_STATUS_S *pstStatus)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_STATUS, pstStatus, sizeof(VI_PIPE_STATUS_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "query pipe status fail, ViPipe:%d, s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setpipeframesource(VI_PIPE ViPipe, const VI_PIPE_FRAME_SOURCE_E enSource)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	VI_PIPE_FRAME_SOURCE_E source = enSource;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_FRM_SRC, &source,
				sizeof(VI_PIPE_FRAME_SOURCE_E), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "SetPipeFrameSource fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipeframesource(VI_PIPE ViPipe, VI_PIPE_FRAME_SOURCE_E *penSource)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_FRM_SRC, (CVI_VOID *)penSource,
				sizeof(VI_PIPE_FRAME_SOURCE_E), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "GetPipeFrameSource fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_sendpiperaw(CVI_U32 u32PipeNum, VI_PIPE PipeId[], const VIDEO_FRAME_INFO_S *pstVideoFrame[],
			   CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	VIDEO_FRAME_INFO_S stVideoFrame[VI_MAX_PIPE_NUM] = {0};
	CVI_U32 i = 0;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, 0, 0, 1);

	stPrivData.as32PrivData[0] = u32PipeNum;

	for (; i < u32PipeNum; ++i) {
		stPrivData.as32PrivData[i + 1] = PipeId[i];
		stVideoFrame[i] = *pstVideoFrame[i];
	}

	stPrivData.as32PrivData[i + 1] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SEND_PIPE_RAW, (CVI_VOID *)&stVideoFrame,
		sizeof(stVideoFrame[VI_MAX_PIPE_NUM]), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "SendPipeRaw fail, VIpipe:%d, s32Ret:%x\n",
			u32PipeNum, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

//ALIOS not support
CVI_S32 platform_vi_getpipefd(VI_PIPE ViPipe)
{
	UNUSED(ViPipe);

	CVI_TRACE_VI(CVI_DBG_ERR, "dual os not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

//ALIOS not support
CVI_S32 platform_vi_closefd(void)
{

	CVI_TRACE_VI(CVI_DBG_ERR, "dual os not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

CVI_S32 platform_vi_setpipecrop(VI_PIPE ViPipe, const CROP_INFO_S *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_PIPE_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(CROP_INFO_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set pipe crop fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getpipecrop(VI_PIPE ViPipe, CROP_INFO_S *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_PIPE_CROP, pstCropInfo,
				sizeof(CROP_INFO_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe crop fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_attachvbpool(VI_PIPE ViPipe, VI_CHN ViChn, VB_POOL VbPool)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_ATTACH_VB_POOL, &VbPool, sizeof(VB_POOL), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe crop fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_detachvbpool(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DETACH_VB_POOL, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe crop fail, ViPipe:%d s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_setchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pstChnAttr->stFrameRate.s32SrcFrameRate != pstChnAttr->stFrameRate.s32DstFrameRate) {
		CVI_TRACE_VI(CVI_DBG_WARN, "FrameRate ctrl, src(%d) dst(%d), not support yet.\n"
				, pstChnAttr->stFrameRate.s32SrcFrameRate, pstChnAttr->stFrameRate.s32DstFrameRate);
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VI_CHN_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set chn attr fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getchnattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_ATTR, pstChnAttr, sizeof(VI_CHN_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get chn attr fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_enablechn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_ENABLE_CHN, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enable chn fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_disablechn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR mesh_name[128];

	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, ViChn, 1);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DISABLE_CHN, NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "disable chn fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
	gdc_free_cur_tsk_mesh(mesh_name);

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setchncrop(VI_PIPE ViPipe, VI_CHN ViChn, const VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_CROP, (CVI_VOID *)pstCropInfo,
				sizeof(VI_CROP_INFO_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set chn crop fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getchncrop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_CROP, pstCropInfo,
				sizeof(VI_CROP_INFO_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get chn crop fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getchnframe(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, ViChn, 1);

	stPrivData.as32PrivData[0] = s32MilliSec;

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VI_GET_CHN_FRAME, (CVI_VOID *)pstFrameInfo,
			sizeof(VIDEO_FRAME_INFO_S), &stPrivData, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get VI chn frame fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
			ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_releasechnframe(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_RELEASE_CHN_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Release VI chn frame fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
			ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_querychnstatus(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_STATUS_S *pstChnStatus)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_STATUS, pstChnStatus, sizeof(VI_CHN_STATUS_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "query chn status fail, ViPipe:%d, ViChn:%d s32Ret:%x\n",
				ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setchnrotation(VI_PIPE ViPipe, VI_CHN ViChn, const ROTATION_E enRotation)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);
	ROTATION_E rotation = enRotation;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_ROTATION, &rotation,
				sizeof(ROTATION_E), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_SetChnRotation fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_getchnrotation(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E *penRotation)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_ROTATION, (CVI_VOID *)penRotation,
				sizeof(ROTATION_E), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_GetChnRotation fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _vi_update_ldc_mesh(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr,
	ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);
	CVI_U64 paddr = 0;
	CVI_VOID *vaddr;
	MSG_PRIV_DATA_S stPrivData = {0};
	CVI_S32 s32Ret;
	char mesh_name[128];

	if (!pstLDCAttr->bEnable) {
		if (enRotation != ROTATION_0) {
			s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_ROTATION, &enRotation, sizeof(ROTATION_E), NULL);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VI(CVI_DBG_ERR, "VI_SetChnRotation fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
			}
		} else {
			s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VI_SET_CHN_LDC_ATTR, (CVI_VOID *)pstLDCAttr,
				sizeof(VI_LDC_ATTR_S), &stPrivData, -1);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VI(CVI_DBG_ERR, "Set VI chn LDC ATTR fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
					ViPipe, ViChn, s32Ret);
			}
		}
		return s32Ret;
	}

	snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
	s32Ret = gdc_gen_ldcmesh(u32Width, u32Height, &pstLDCAttr->stAttr,
				mesh_name, &paddr, &vaddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Chn(%d) gen mesh fail\n", ViChn);
		return s32Ret;
	}

	CVI_TRACE_VI(CVI_DBG_DEBUG, "ViPipe(%d) ViChn(%d) mesh base(%#"PRIx64") vaddr(%p)\n"
		, ViPipe, ViChn, paddr, vaddr);

	stPrivData.as32PrivData[0] = (CVI_S32)(paddr & 0xFFFFFFF);
	stPrivData.as32PrivData[1] = (CVI_S32)((paddr >> 28) & 0xFFFFFFF);
	stPrivData.as32PrivData[2] = (CVI_S32)((paddr >> 56) & 0xFF);

	s32Ret = CVI_MSG_SendSync4(u32ModFd, MSG_CMD_VI_SET_CHN_LDC_ATTR, (CVI_VOID *)pstLDCAttr,
		sizeof(VI_LDC_ATTR_S), &stPrivData, -1);
	return CVI_SUCCESS;
}
//TODO need refactor
CVI_S32 platform_vi_setchnldcattr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd;
	VI_CHN_ATTR_S stChnAttr;
	ROTATION_E enRotation;

	u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_ATTR, &stChnAttr, sizeof(VI_CHN_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get chn attr fail, ViPipe:%d ViChn:%d,s32Ret:%x\n", ViPipe, ViChn, s32Ret);
		return s32Ret;
	}

	if (!GDC_SUPPORT_FMT(stChnAttr.enPixelFormat)) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support format(%d).\n", stChnAttr.enPixelFormat);
		return CVI_ERR_VI_INVALID_PARA;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_ROTATION, &enRotation,
				sizeof(ROTATION_E), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI_GetChnRotation fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return _vi_update_ldc_mesh(ViPipe, ViChn, pstLDCAttr, enRotation,
		stChnAttr.stSize.u32Width, stChnAttr.stSize.u32Height);
}

CVI_S32 platform_vi_getchnldcattr(VI_PIPE ViPipe, VI_CHN ViChn, VI_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VI, ViPipe, ViChn, 1);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_LDC_ATTR, (CVI_VOID *)pstLDCAttr,
		sizeof(VI_LDC_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get VI chn LDC ATTR fail,VIpipe:%d, VIChn:%d, s32Ret:%x\n",
			ViPipe, ViChn, s32Ret);
	}

	return s32Ret;
}

CVI_S32 platform_vi_regchnflipmirrorcallback(VI_PIPE ViPipe, VI_DEV ViDev, void *pvData)
{
	UNUSED(ViPipe);
	UNUSED(ViDev);
	UNUSED(pvData);

	CVI_TRACE_VI(CVI_DBG_ERR, "dual os not support\n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

CVI_S32 platform_vi_unregchnflipmirrorcallback(VI_PIPE ViPipe, VI_DEV ViDev)
{
	UNUSED(ViPipe);
	UNUSED(ViDev);

	CVI_TRACE_VI(CVI_DBG_ERR, "dual os not support\n");

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_setchnflipmirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL bFlip, CVI_BOOL bMirror)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);

	stPrivData.as32PrivData[0] = bFlip;
	stPrivData.as32PrivData[1] = bMirror;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_SET_CHN_FLIP_MIRROR, NULL,
				0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "SetChnFlipMirror fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_vi_getchnflipmirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL *pbFlip, CVI_BOOL *pbMirror)
{
	CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, ViChn);
	CVI_U32 bMirrFilp[2] = {0, 0};

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_GET_CHN_FLIP_MIRROR, bMirrFilp,
				sizeof(bMirrFilp), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "GetChnFlipMirror fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}
	*pbFlip = bMirrFilp[0];
	*pbMirror = bMirrFilp[1];

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_dumphwregistertofile(VI_PIPE ViPipe, FILE *fp, VI_DUMP_REGISTER_TABLE_S *pstRegTbl)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U64 json_phy = 0;
	MSG_PRIV_DATA_S stPrivData;
	void *json_vir = NULL;
	CVI_U32 size = 0;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, ViPipe, 0);

	UNUSED(pstRegTbl);

#define DUMP_SIZE 524288

	s32Ret = CVI_SYS_IonAlloc_Cached(&json_phy, &json_vir, "VI_DMA_BUF", DUMP_SIZE);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI ion alloc size(%d) failed.\n", DUMP_SIZE);
		return CVI_ERR_VI_NOMEM;
	}

	stPrivData.as32PrivData[0] = json_phy;
	stPrivData.as32PrivData[1] = DUMP_SIZE;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_DUMP_HW_REG_TO_FILE, NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "dump hw register fail:%d, s32Ret:%x\n",
			ViPipe, s32Ret);
		goto free_json;
	}

	size = stPrivData.as32PrivData[1];

	fwrite(json_vir, 1, size, fp);
	fflush(fp);

free_json:
	CVI_SYS_IonFree(json_phy, json_vir);

	return s32Ret;
}

CVI_S32 platform_vi_setdevrxframecount(VI_DEV ViDev, CVI_U32 u32RxFrameCount)
{
	UNUSED(ViDev);
	UNUSED(u32RxFrameCount);

	CVI_TRACE_VI(CVI_DBG_ERR, "dual os not support\n");

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_getdevrxframecount(VI_DEV ViDev, CVI_U32 *pu32FrameCount)
{
	UNUSED(ViDev);
	UNUSED(pu32FrameCount);

	CVI_TRACE_VI(CVI_DBG_ERR, "dual os not support\n");

	*pu32FrameCount = 0;

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

CVI_S32 platform_vi_aiispcfg(VI_AI_ISP_CFG_S *pstAiIspCfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	if (pstAiIspCfg == CVI_NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "pstAiIspCfg is NULL\n");
		return CVI_ERR_VI_INVALID_NULL_PTR;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_AI_ISP_CFG,
				  pstAiIspCfg, sizeof(*pstAiIspCfg), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "AI_ISP_CFG fail, ViPipe:%d, ret:0x%x\n",
			pstAiIspCfg->viPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_vi_aiispinfo(VI_AI_ISP_INFO_WRAP_S *pstAiIspInfoWrap)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VI, 0, 0);

	if (pstAiIspInfoWrap == CVI_NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "pstAiIspInfoWrap is NULL\n");
		return CVI_ERR_VI_INVALID_NULL_PTR;
	}

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VI_AI_ISP_INFO,
				  pstAiIspInfoWrap, sizeof(*pstAiIspInfoWrap), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "AI_ISP_INFO fail, ViPipe:%d, ret:0x%x\n",
			pstAiIspInfoWrap->viPipe, s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}
