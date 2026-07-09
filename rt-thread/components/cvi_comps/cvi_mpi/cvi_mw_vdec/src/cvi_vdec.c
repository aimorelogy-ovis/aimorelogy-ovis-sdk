#include "cvi_comm_math.h"
#include "cvi_vdec.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_buffer.h"
#include "cvi_base.h"
#include "cvi_comm_vdec.h"
#include "cvi_defines.h"

#include "cvi_comm_vb.h"
#include "base_ctx.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "pthread.h"
#include "vdec.h"
#include "vip_atomic.h"

#define CVI_VDEC_NO_INPUT  -10
#define CVI_VDEC_INPUT_ERR -11

#define VDEC_TIME_BLOCK_MODE    (-1)
#define VDEC_RET_TIMEOUT        (-2)
#define VDEC_TIME_TRY_MODE      (0)
#define VDEC_DEFAULT_MUTEX_MODE VDEC_TIME_BLOCK_MODE
#define CHECK_MUTEX_RET(X, FUNC, LINE)                                            \
    do {                                                                          \
        if (X != 0)                                                               \
        {                                                                         \
            if ((X == ETIMEDOUT) || (X == EBUSY))                                 \
            {                                                                     \
                CVI_VDEC_TRACE("mutex timeout and retry[%s]:[%d]\n", FUNC, LINE); \
                return CVI_ERR_VDEC_BUSY;                                         \
            }                                                                     \
            else                                                                  \
            {                                                                     \
                CVI_VDEC_ERR("vdec mutex error[%d],[%s]:[%d]\n", X, FUNC, LINE);  \
                return CVI_ERR_VDEC_ERR_VDEC_MUTEX;                               \
            }                                                                     \
        }                                                                         \
    } while (0)

// below should align to cv183x_vcodec.h
#define VPU_MISCDEV_NAME           "/dev/cvi-vcodec"
#define VCODEC_VENC_SHARE_MEM_SIZE (0x30000) // 192k
#define VCODEC_VDEC_SHARE_MEM_SIZE (0x8000)  // 32k
#define VCODEC_SHARE_MEM_SIZE \
    (VCODEC_VENC_SHARE_MEM_SIZE + VCODEC_VDEC_SHARE_MEM_SIZE)
#define SEC_TO_MS (1000)

typedef struct _vdec_proc_info_t
{
    VDEC_CHN_ATTR_S   chnAttr;
    VDEC_CHN_PARAM_S  stChnParam;
    VIDEO_FRAME_S     stFrame;
    VCODEC_PERF_FPS_S stFPS;
    uint16_t          u16ChnNo;
    uint8_t           u8ChnUsed;
} vdec_proc_info_t;

typedef struct _proc_debug_config_t
{
    uint32_t u32DbgMask;
    uint32_t u32StartFrmIdx;
    uint32_t u32EndFrmIdx;
    char     cDumpPath[CVI_VDEC_STR_LEN];
    uint32_t u32NoDataTimeout;
} proc_debug_config_t;

#define IF_WANNA_DISABLE_BIND_MODE() \
    ((pVbCtx->currBindMode == CVI_TRUE) && (pVbCtx->enable_bind_mode == CVI_FALSE))
#define IF_WANNA_ENABLE_BIND_MODE() \
    ((pVbCtx->currBindMode == CVI_FALSE) && (pVbCtx->enable_bind_mode == CVI_TRUE))

extern int     pthread_mutex_timedlock(pthread_mutex_t       *mutex,
                                       const struct timespec *at);
extern int32_t vb_done_handler(MMF_CHN_S chn, enum CHN_TYPE_E chn_type,
                               VB_BLK blk);
extern VB_BLK  CVI_VB_GetBlock(VB_POOL Pool, CVI_U32 u32BlkSize);

vdec_dbg vdecDbg = {
    .currMask = -1,
};
vdec_context  *vdec_handle;
static void   *vpu_comm_shared_mem;
static void   *vdec_vpu_shared_mem;
static void   *vdec_proc_shared_mem;
static CVI_U32 vdec_shared_mem_usr_cnt;

static pthread_mutex_t g_vdec_handle_mutex = PTHREAD_MUTEX_INITIALIZER;

static CVI_VOID cviChangeVdecMask(CVI_S32 frameIdx);
static CVI_S32  allocate_vdec_frame(vdec_chn_context *pChnHandle, CVI_U32 idx);

static CVI_S32  free_vdec_frame(VDEC_CHN_ATTR_S    *pstAttr,
                                VIDEO_FRAME_INFO_S *pstVideoFrame);
static CVI_S32  cvi_jpeg_decode(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream,
                                VIDEO_FRAME_INFO_S *pstVideoFrame,
                                CVI_S32 s32MilliSec, CVI_U64 *pu64DecHwTime);
static CVI_S32  cvi_h264_decode(void *pHandle, PIXEL_FORMAT_E enPixelFormat,
                                const VDEC_STREAM_S *pstStream,
                                CVI_S32              s32MilliSec);
static CVI_S32  cvi_h265_decode(void *pHandle, PIXEL_FORMAT_E enPixelFormat,
                                const VDEC_STREAM_S *pstStream,
                                CVI_S32              s32MilliSec);
static void     cviSetVideoFrameInfo(VIDEO_FRAME_INFO_S *pstVideoFrame,
                                     cviDispFrameCfg    *pdfc);
static CVI_S32  cviSetVideoChnAttrToProc(VDEC_CHN            VdChn,
                                         VIDEO_FRAME_INFO_S *psFrameInfo,
                                         CVI_U64             u64DecHwTime);
static CVI_VOID cviGetDebugConfigFromDecProc(void);
static CVI_S32  cviSetVdecFpsToProc(VDEC_CHN VdChn, CVI_BOOL bSendStream);
static CVI_S32  cviVdec_Mutex_Lock(pthread_mutex_t  *__restrict __mutex,
                                   CVI_S32 s32MilliSec, CVI_S32 *s32CostTime);
static CVI_S32  cviVdec_Mutex_Unlock(pthread_mutex_t *__mutex);

static CVI_U64 get_current_time(CVI_VOID)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000; // in ms
}

static void *vdec_vpu_get_shm(void)
{
    if (vpu_comm_shared_mem == NULL)
    {
        vpu_comm_shared_mem = malloc(VCODEC_VENC_SHARE_MEM_SIZE);
        if (vpu_comm_shared_mem == NULL)
        {
            CVI_VDEC_ERR("vpu_comm_shared_mem malloc fail!\n");
            return NULL;
        }
    }
    return (vpu_comm_shared_mem);
}

static void vdec_vpu_release_shm(void)
{
    vdec_shared_mem_usr_cnt--;

    if (vdec_shared_mem_usr_cnt == 0 && vpu_comm_shared_mem != NULL)
    {
        free(vpu_comm_shared_mem);
        vdec_vpu_shared_mem = vdec_proc_shared_mem = NULL;
        vpu_comm_shared_mem                        = NULL;
    }
}

static CVI_S32 _VDEC_MMAP(void)
{
    CVI_S32 ret = CVI_SUCCESS;

    vdec_shared_mem_usr_cnt++;

    if (vpu_comm_shared_mem != NULL)
    {
        CVI_VDEC_INFO("already done mmap\n");
        return CVI_SUCCESS;
    }

    vdec_vpu_shared_mem = vdec_vpu_get_shm();
    if (vdec_vpu_shared_mem == NULL)
    {
        CVI_VDEC_ERR("base_get_shm failed!\n");
        return CVI_ERR_VDEC_NOMEM;
    }
    vdec_proc_shared_mem = vdec_vpu_shared_mem + sizeof(proc_debug_config_t) + sizeof(VDEC_MOD_PARAM_S);

    return ret;
}

static CVI_S32 _VDEC_UNMAP(void)
{
    if (vpu_comm_shared_mem == NULL)
    {
        CVI_VDEC_INFO("No need to unmap\n");
        return CVI_SUCCESS;
    }

    vdec_vpu_release_shm();
    return CVI_SUCCESS;
}

static CVI_S32 cviVdec_Mutex_Unlock(pthread_mutex_t *__mutex)
{
    return pthread_mutex_unlock(__mutex);
}

static CVI_S32 cviVdec_Mutex_Lock(pthread_mutex_t * __restrict __mutex,
                                  CVI_S32 s32MilliSec, CVI_S32 *s32CostTime)
{
    CVI_S32 s32RetCostTime;
    CVI_S32 s32RetCheck;
    CVI_U64 u64StartTime, u64EndTime;

    s32CostTime = NULL;
    s32MilliSec = -1;
    // u64StartTime = get_current_time(); to do?
    if (s32MilliSec < 0)
    {
        // block mode
        s32RetCheck = pthread_mutex_lock(__mutex);
        // u64EndTime = get_current_time(); to do?
        // s32RetCostTime = u64EndTime - u64StartTime; to do?
    }
    else if (s32MilliSec == 0)
    {
        // trylock
        s32RetCheck    = pthread_mutex_trylock(__mutex);
        s32RetCostTime = 0;
    }
    else
    {
        // timelock
        struct timespec tout;
        unsigned long   time_interval;
        unsigned long   time_ns = 0;

        clock_gettime(CLOCK_REALTIME, &tout);
        time_interval  = (unsigned long)s32MilliSec;
        tout.tv_sec   += (time_interval / 1000);
        time_ns        = tout.tv_nsec + (time_interval % 1000) * 1000000;
        if (time_ns >= (1000000000))
        {
            tout.tv_sec++;
            tout.tv_nsec = time_ns - 1000000000;
        }
        else
        {
            // no need update second again
            tout.tv_nsec = time_ns;
        }
        s32RetCheck    = pthread_mutex_timedlock(__mutex, &tout);
        u64EndTime     = get_current_time();
        s32RetCostTime = u64EndTime - u64StartTime;
    }
    // calculate cost lock time

    // check the mutex validity
    if (s32RetCheck != 0)
    {
        if (s32RetCheck == ETIMEDOUT)
            CVI_VDEC_TRACE("lock timeout\n");
        else if (s32RetCheck == EBUSY)
            CVI_VDEC_TRACE("try lock timeout\n");
        else if (s32RetCheck == EAGAIN)
            CVI_VDEC_ERR("maximum number of recursive locks for mutex\n");
        else if (s32RetCheck == EINVAL)
            CVI_VDEC_ERR("invalid mutex/ not init or been destroyed\n");
        else
            CVI_VDEC_ERR("unexpected error in mutex lock[%d]\n", s32RetCheck);
    }
    else
    {
        // lock valid
        CVI_VDEC_TRACE("lock valid\n");
    }

    if (s32CostTime != NULL) *s32CostTime = s32RetCostTime;
    return s32RetCheck;
}

static CVI_VOID cviChangeVdecMask(CVI_S32 frameIdx)
{
    vdec_dbg *pDbg = &vdecDbg;

    pDbg->currMask = pDbg->dbgMask;
    if (pDbg->startFn >= 0)
    {
        if (frameIdx >= pDbg->startFn && frameIdx <= pDbg->endFn)
            pDbg->currMask = pDbg->dbgMask;
        else
            pDbg->currMask = CVI_VDEC_MASK_ERR;
    }

    CVI_VDEC_TRACE("currMask = 0x%X\n", pDbg->currMask);
}

static CVI_S32 check_vdec_chn_handle(VDEC_CHN VdChn)
{
    if (vdec_handle == NULL)
    {
        CVI_VDEC_ERR("Call VDEC Destroy before Create, failed\n");
        return CVI_ERR_VDEC_UNEXIST;
    }

    if (vdec_handle->chn_handle[VdChn] == NULL)
    {
        CVI_VDEC_ERR("VDEC Chn #%d haven't created !\n", VdChn);
        return CVI_ERR_VDEC_INVALID_CHNID;
    }

    return CVI_SUCCESS;
}

#if 0
static CVI_U32 vdec_dump_queue_status(VDEC_CHN VdChn)
{

	vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

	if (vdec_handle == NULL) {
		CVI_VDEC_ERR("Call VENC Destroy before Create, failed\n");
		return CVI_ERR_VDEC_UNEXIST;
	}

	if (vdec_handle->chn_handle[VdChn] == NULL) {
		CVI_VDEC_ERR("VDEC Chn #%d haven't created !\n", VdChn);
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	for (int i = 0; i < pChnHandle->ChnAttr.u32FrameBufCnt; i++) {
		CVI_VDEC_INFO("FbArray %d, flag %d\n", i, pChnHandle->VideoFrameArray[i].stVFrame.u32FrameFlag);
	}

	for (int i = 0; i < DISPLAY_QUEUE_SIZE; i++) {
		CVI_VDEC_INFO("DisQueue %d, idx %d\n", i, pChnHandle->display_queue[i]);
	}

	return CVI_SUCCESS;

}
#endif

static CVI_S32 get_avail_fb(VDEC_CHN            VdChn,
                            VIDEO_FRAME_INFO_S *VideoFrameArray)
{
    CVI_S32 s32Ret       = check_vdec_chn_handle(VdChn);
    CVI_S32 avail_fb_idx = -1;

    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return avail_fb_idx;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    for (CVI_U32 i = 0; i < pChnHandle->VideoFrameArrayNum; i++)
    {
        if (VideoFrameArray[i].stVFrame.u32FrameFlag == 0)
        {
            avail_fb_idx = i;
            break;
        }
    }

    return avail_fb_idx;
}

static CVI_S32 insert_display_queue(CVI_U32 w_idx, CVI_S32 fb_idx,
                                    CVI_S8 *display_queue)
{
    if (display_queue[w_idx] != -1) return CVI_ERR_VDEC_ILLEGAL_PARAM;
    CVI_VDEC_INFO("insert_display_queue, w_idx %d, fb_idx %d\n", w_idx, fb_idx);
    display_queue[w_idx] = fb_idx;

    return CVI_SUCCESS;
}

static CVI_S32 allocate_vdec_frame(vdec_chn_context *pChnHandle, CVI_U32 idx)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    VDEC_CHN_ATTR_S  *pstAttr  = &pChnHandle->ChnAttr;
    VDEC_CHN_PARAM_S *pstParam = &pChnHandle->ChnParam;

    VIDEO_FRAME_S  *pstVFrame = &(pChnHandle->VideoFrameArray[idx].stVFrame);
    VB_CAL_CONFIG_S stVbCfg;
    PIXEL_FORMAT_E  enPixelFormat =
        (pstAttr->enType == PT_JPEG || pstAttr->enType == PT_MJPEG)
             ? pstParam->enPixelFormat
             : PIXEL_FORMAT_YUV_PLANAR_420;

    memset(&stVbCfg, 0, sizeof(stVbCfg));
    VDEC_GetPicBufferConfig(pstAttr->enType, pstAttr->u32PicWidth,
                            pstAttr->u32PicHeight, enPixelFormat, DATA_BITWIDTH_8,
                            COMPRESS_MODE_NONE, &stVbCfg);

    pstVFrame->enCompressMode = COMPRESS_MODE_NONE;
    pstVFrame->enPixelFormat  = enPixelFormat;
    pstVFrame->enVideoFormat  = VIDEO_FORMAT_LINEAR;
    pstVFrame->enColorGamut   = COLOR_GAMUT_BT709;
    pstVFrame->u32Width       = pstAttr->u32PicWidth;
    pstVFrame->u32Height      = pstAttr->u32PicHeight;
    pstVFrame->u32Stride[0]   = stVbCfg.u32MainStride;
    pstVFrame->u32Stride[1]   = (stVbCfg.plane_num > 1) ? stVbCfg.u32CStride : 0;
    pstVFrame->u32Stride[2]   = (stVbCfg.plane_num > 2) ? stVbCfg.u32CStride : 0;
    pstVFrame->u32TimeRef     = 0;
    pstVFrame->u64PTS         = 0;
    pstVFrame->enDynamicRange = DYNAMIC_RANGE_SDR8;
    pstVFrame->u32FrameFlag   = 0;

    CVI_VDEC_DISP("u32Stride[0] = %d, u32Stride[1] = %d, u32Stride[2] = %d\n",
                  pstVFrame->u32Stride[0], pstVFrame->u32Stride[1],
                  pstVFrame->u32Stride[2]);

    if (pstVFrame->u32Width % DEFAULT_ALIGN)
    {
        CVI_VDEC_INFO("u32Width is not algined to %d\n", DEFAULT_ALIGN);
    }

    if (pstAttr->enType == PT_JPEG || pstAttr->enType == PT_MJPEG)
    {
        VB_POOL hPicVbPool = VB_INVALID_POOLID;
        VB_BLK  blk;

        if (vdec_handle->g_stModParam.enVdecVBSource != VB_SOURCE_USER)
        {
            blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCfg.u32VBSize);
        }
        else
        {
            hPicVbPool = pChnHandle->vbPool.hPicVbPool;
            blk        = CVI_VB_GetBlock(hPicVbPool, stVbCfg.u32VBSize);
        }

        if (blk == VB_INVALID_HANDLE)
        {
            CVI_VDEC_ERR("Can't acquire vb block\n");
            return CVI_ERR_VDEC_NOMEM;
        }

        pChnHandle->vbBLK[idx] = blk;

        CVI_VDEC_INFO("alloc size %d, %d x %d\n", stVbCfg.u32VBSize,
                      pstAttr->u32PicWidth, pstAttr->u32PicHeight);

        pChnHandle->VideoFrameArray[idx].u32PoolId = CVI_VB_Handle2PoolId(blk);

        pstVFrame->u32Length[0]  = stVbCfg.u32MainYSize;
        pstVFrame->u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
        pstVFrame->pu8VirAddr[0] = (CVI_U8 *)pstVFrame->u64PhyAddr[0];

        if (stVbCfg.plane_num > 1)
        {
            pstVFrame->u32Length[1] = stVbCfg.u32MainCSize;
            pstVFrame->u64PhyAddr[1] =
                pstVFrame->u64PhyAddr[0] + ALIGN(stVbCfg.u32MainYSize, stVbCfg.u16AddrAlign);
            pstVFrame->pu8VirAddr[1] = (CVI_U8 *)pstVFrame->u64PhyAddr[1];
        }
        else
        {
            pstVFrame->u32Length[1]  = 0;
            pstVFrame->u64PhyAddr[1] = 0;
            pstVFrame->pu8VirAddr[1] = NULL;
        }

        if (stVbCfg.plane_num > 2)
        {
            pstVFrame->u32Length[2] = stVbCfg.u32MainCSize;
            pstVFrame->u64PhyAddr[2] =
                pstVFrame->u64PhyAddr[1] + ALIGN(stVbCfg.u32MainCSize, stVbCfg.u16AddrAlign);
            pstVFrame->pu8VirAddr[2] = (CVI_U8 *)pstVFrame->u64PhyAddr[2];
        }
        else
        {
            pstVFrame->u32Length[2]  = 0;
            pstVFrame->u64PhyAddr[2] = 0;
            pstVFrame->pu8VirAddr[2] = NULL;
        }

        CVI_VDEC_DISP("phy addr(%llx, %llx, %llx\n", pstVFrame->u64PhyAddr[0],
                      pstVFrame->u64PhyAddr[1], pstVFrame->u64PhyAddr[2]);

        CVI_VDEC_DISP("vir addr(%p, %p, %p\n", pstVFrame->pu8VirAddr[0],
                      pstVFrame->pu8VirAddr[1], pstVFrame->pu8VirAddr[2]);

        pChnHandle->FrmArray[idx].phyAddr  = pstVFrame->u64PhyAddr[0];
        pChnHandle->FrmArray[idx].size     = stVbCfg.u32VBSize;
        pChnHandle->FrmArray[idx].virtAddr = pstVFrame->pu8VirAddr[0];
        pChnHandle->FrmNum++;
    }

    return s32Ret;
}

CVI_S32 _CVI_VDEC_InitHandle(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    pthread_mutex_lock(&g_vdec_handle_mutex);

    if (vdec_handle == NULL)
    {
        vdec_handle = calloc(1, sizeof(vdec_context));

        if (vdec_handle == NULL)
        {
            pthread_mutex_unlock(&g_vdec_handle_mutex);
            return CVI_ERR_VDEC_NOMEM;
        }

        memset(vdec_handle, 0x00, sizeof(vdec_context));
        vdec_handle->g_stModParam.enVdecVBSource =
            VB_SOURCE_COMMON; // Default is VB_SOURCE_COMMON
        vdec_handle->g_stModParam.u32MiniBufMode                    = 0;
        vdec_handle->g_stModParam.u32ParallelMode                   = 0;
        vdec_handle->g_stModParam.stVideoModParam.u32MaxPicHeight   = 2160;
        vdec_handle->g_stModParam.stVideoModParam.u32MaxPicWidth    = 4096;
        vdec_handle->g_stModParam.stVideoModParam.u32MaxSliceNum    = 200;
        vdec_handle->g_stModParam.stPictureModParam.u32MaxPicHeight = 8192;
        vdec_handle->g_stModParam.stPictureModParam.u32MaxPicWidth  = 8192;
    }

    pthread_mutex_unlock(&g_vdec_handle_mutex);

    return s32Ret;
}

static CVI_S32 _cvi_vdec_AllocVbBuf(vdec_chn_context *pChnHandle, void *arg)
{
    stCviCb_HostAllocFB *pHostAllocFB = (stCviCb_HostAllocFB *)arg;

    if (vdec_handle->g_stModParam.enVdecVBSource == VB_SOURCE_PRIVATE)
    {
        if (pChnHandle->bHasVbPool == false)
        {
            VB_POOL_CONFIG_S stVbPoolCfg;

            stVbPoolCfg.u32BlkSize  = pHostAllocFB->iFrmBufSize;
            stVbPoolCfg.u32BlkCnt   = pHostAllocFB->iFrmBufNum;
            stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;

            pChnHandle->vbPool.hPicVbPool = CVI_VB_CreatePool(&stVbPoolCfg);

            if (pChnHandle->vbPool.hPicVbPool == VB_INVALID_POOLID)
            {
                CVI_VDEC_ERR("CVI_VB_CreatePool failed !\n");
                return 0;
            }

            pChnHandle->bHasVbPool = true;

            CVI_VDEC_TRACE(
                "Create Private Pool: %d, u32BlkSize=0x%x,  u32BlkCnt=%d\n",
                pChnHandle->vbPool.hPicVbPool, stVbPoolCfg.u32BlkSize,
                stVbPoolCfg.u32BlkCnt);
        }
    }

    if (pChnHandle->bHasVbPool == false)
    {
        CVI_VDEC_ERR("pChnHandle->bHasVbPool == false\n");
        return 0;
    }

    if (pHostAllocFB->iFrmBufNum > MAX_VDEC_FRM_NUM)
    {
        CVI_VDEC_ERR("iFrmBufNum > %d\n", MAX_VDEC_FRM_NUM);
        return 0;
    }

    for (int j = 0; j < pHostAllocFB->iFrmBufNum; j++)
    {
        CVI_U64 u64PhyAddr;

        pChnHandle->vbBLK[j] = CVI_VB_GetBlock(pChnHandle->vbPool.hPicVbPool,
                                               pHostAllocFB->iFrmBufSize);
        if (pChnHandle->vbBLK[j] == VB_INVALID_HANDLE)
        {
            CVI_VDEC_ERR("CVI_VB_GetBlockwithID failed !\n");
            CVI_VDEC_ERR(
                "Frame isn't enough. Need %d frames, size 0x%x. Now only %d frames\n",
                pHostAllocFB->iFrmBufNum, pHostAllocFB->iFrmBufSize, j);
            return 0;
        }

        u64PhyAddr = CVI_VB_Handle2PhysAddr(pChnHandle->vbBLK[j]);

        pChnHandle->FrmArray[j].phyAddr  = u64PhyAddr;
        pChnHandle->FrmArray[j].size     = pHostAllocFB->iFrmBufSize;
        pChnHandle->FrmArray[j].virtAddr = (void *)u64PhyAddr;

        CVI_VDEC_TRACE(
            "CVI_VB_GET_BLOCK, VbBlk = %d, u64PhyAddr = 0x%llx, virtAddr = %p\n",
            (CVI_S32)pChnHandle->vbBLK[j], (long long)u64PhyAddr,
            pChnHandle->FrmArray[j].virtAddr);
    }

    pChnHandle->FrmNum = pHostAllocFB->iFrmBufNum;

    cviVDecAttachFrmBuf((void *)pChnHandle->pHandle, (void *)pChnHandle->FrmArray,
                        pHostAllocFB->iFrmBufNum);

    return 1;
}

static CVI_S32 _cvi_vdec_FreeVbBuf(vdec_chn_context *pChnHandle)
{
    if (vdec_handle->g_stModParam.enVdecVBSource == VB_SOURCE_PRIVATE)
    {
        CVI_VDEC_ERR("not support VB_SOURCE_PRIVATE yet\n");
        return 0;
    }

    CVI_VDEC_TRACE("\n");

    if (pChnHandle->ChnAttr.enType == PT_H264 || pChnHandle->ChnAttr.enType == PT_H265)
    {
        if (pChnHandle->bHasVbPool == true)
        {
            VB_BLK blk;

            for (CVI_U32 i = 0; i < pChnHandle->FrmNum; i++)
            {
                blk = CVI_VB_PhysAddr2Handle(pChnHandle->FrmArray[i].phyAddr);

                if (blk != VB_INVALID_HANDLE)
                {
                    CVI_VB_ReleaseBlock(blk);
                }
            }
        }
        else
        {
            CVI_VDEC_ERR("bHasVbPool = false\n");
            return 0;
        }
    }

    return 1;
}

static CVI_S32 _cvi_vdec_GetDispQ_Count(vdec_chn_context *pChnHandle)
{
    CVI_S32 s32Count = 0;

    CVI_U32 i = 0;

    for (i = 0; i < pChnHandle->VideoFrameArrayNum; i++)
    {
        VIDEO_FRAME_S *pstCurFrame;

        pstCurFrame = &pChnHandle->VideoFrameArray[i].stVFrame;

        if (pstCurFrame->u32FrameFlag == 1)
        {
            s32Count++;
        }
    }

    CVI_VDEC_TRACE("s32Count=%d\n", s32Count);

    return s32Count;
}

static CVI_S32 _cvi_vdec_CallbackFunc(unsigned int VdChn, unsigned int CbType,
                                      void *arg)
{
    CVI_S32           s32Ret = 0, s32Check = CVI_SUCCESS;
    vdec_chn_context *pChnHandle = NULL;

    s32Check = check_vdec_chn_handle(VdChn);
    if (s32Check != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    CVI_VDEC_TRACE("VdChn = %d, CbType= %d\n", VdChn, CbType);

    pChnHandle = vdec_handle->chn_handle[VdChn];

    switch (CbType)
    {
    case CVI_H26X_DEC_CB_AllocFB:
        if (arg == NULL)
        {
            CVI_VDEC_ERR("arg == NULL\n");
            return s32Ret;
        }

        if (_cvi_vdec_AllocVbBuf(pChnHandle, arg) != 1)
        {
            return s32Ret;
        }
        s32Ret = 1;
        break;
    case CVI_H26X_DEC_CB_FreeFB:
        if (_cvi_vdec_FreeVbBuf(pChnHandle) != 1)
        {
            return s32Ret;
        }
        s32Ret = 1;
        break;
    case CVI_H26X_DEC_CB_GET_DISPQ_COUNT:
        s32Ret = _cvi_vdec_GetDispQ_Count(pChnHandle);
        break;
    default:
        CVI_VDEC_ERR("Unsupported cbType: %d\n", CbType);
        break;
    }

    return s32Ret;
}

#define VDEC_NO_FRAME_IDX 0xFFFFFFFF

CVI_BOOL _cvi_vdec_FindBlkInfo(vdec_chn_context *pChnHandle, CVI_U64 u64PhyAddr,
                               VB_BLK *pVbBLK, CVI_U32 *pFrmIdx)
{
    CVI_U32 i = 0;

    if ((pChnHandle == NULL) || (pVbBLK == NULL) || (pFrmIdx == NULL))
        return false;

    *pVbBLK  = VB_INVALID_HANDLE;
    *pFrmIdx = VDEC_NO_FRAME_IDX;

    for (i = 0; i < pChnHandle->FrmNum; i++)
    {
        if ((pChnHandle->FrmArray[i].phyAddr <= u64PhyAddr) && (pChnHandle->FrmArray[i].phyAddr + pChnHandle->FrmArray[i].size) > u64PhyAddr)
        {
            *pVbBLK  = pChnHandle->vbBLK[i];
            *pFrmIdx = i;
            break;
        }
    }

    if (i == pChnHandle->FrmNum)
    {
        CVI_VDEC_ERR("Can't find BLK !\n");
        return false;
    }

    return true;
}

CVI_S32 _cvi_vdec_FindFrameIdx(vdec_chn_context         *pChnHandle,
                               const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    CVI_U32 i = 0;

    if ((pChnHandle == NULL) || (pstFrameInfo == NULL)) return -1;

    for (i = 0; i < pChnHandle->VideoFrameArrayNum; i++)
    {
        VIDEO_FRAME_S *pstCurFrame;

        pstCurFrame = &pChnHandle->VideoFrameArray[i].stVFrame;

        if ((pstCurFrame->u32FrameFlag == 1) && (pstCurFrame->u64PhyAddr[0] == pstFrameInfo->stVFrame.u64PhyAddr[0]) && (pstCurFrame->u64PhyAddr[1] == pstFrameInfo->stVFrame.u64PhyAddr[1]) && (pstCurFrame->u64PhyAddr[2] == pstFrameInfo->stVFrame.u64PhyAddr[2]))
        {
            return i;
        }
    }

    return -1;
}

typedef struct _VDEC_BIND_FRM_S
{
    VIDEO_FRAME_INFO_S stFrameInfo;
    VB_BLK             vbBLK;
    CVI_BOOL           bIsFrmOutput;
} VDEC_BIND_FRM_S;

CVI_VOID *vdec_event_handler(CVI_VOID *data)
{
    vdec_chn_context       *pChnHandle = (vdec_chn_context *)data;
    VDEC_CHN                VdecChn;
    VIDEO_FRAME_INFO_S      stVFrame;
    CVI_S32                 s32Ret = CVI_SUCCESS;
    CVI_S32                 s32Cnt = 0;
    CVI_U32                 frmIdx = 0;
    struct cvi_vdec_vb_ctx *pVbCtx;
    VB_BLK                  vbBLK = VB_INVALID_HANDLE;
    CVI_U32                 i     = 0;
    VDEC_BIND_FRM_S        *pBindFrmQueue;
    CVI_CHAR                cThreadName[16];

    if (pChnHandle == NULL) return (CVI_VOID *)CVI_ERR_VDEC_NULL_PTR;

    VdecChn = pChnHandle->VdChn;
    pVbCtx  = pChnHandle->pVbCtx;

    pBindFrmQueue =
        (VDEC_BIND_FRM_S *)malloc(sizeof(VDEC_BIND_FRM_S) * MAX_VDEC_FRM_NUM);

    if (pBindFrmQueue == NULL)
    {
        CVI_VDEC_ERR("no memory for pBindFrmQueue\n");
        return (CVI_VOID *)CVI_FAILURE;
    }

    memset(pBindFrmQueue, 0, sizeof(VDEC_BIND_FRM_S) * MAX_VDEC_FRM_NUM);

    snprintf(cThreadName, sizeof(cThreadName), "vdec_handler-%d", VdecChn);
    // prctl(PR_SET_NAME, cThreadName);

    while (1)
    {
        if (IF_WANNA_DISABLE_BIND_MODE())
        {
            break;
        }

        // Check if frame free
        CVI_U32 u32Cnt = 0;

        for (i = 0; i < MAX_VDEC_FRM_NUM; i++)
        {
            if (pBindFrmQueue[i].bIsFrmOutput == true)
            {
                CVI_VB_InquireUserCnt(pBindFrmQueue[i].vbBLK, &u32Cnt);
                if (u32Cnt == 1)
                {
                    CVI_VDEC_ReleaseFrame(VdecChn, &(pBindFrmQueue[i].stFrameInfo));
                    pBindFrmQueue[i].bIsFrmOutput = false;
                    break;
                }
            }
        }

        s32Ret = CVI_VDEC_GetFrame(VdecChn, &stVFrame, -1);

        if (s32Ret == CVI_SUCCESS)
        {
            s32Cnt++;
            if (_cvi_vdec_FindBlkInfo(pChnHandle, stVFrame.stVFrame.u64PhyAddr[0],
                                      &vbBLK, &frmIdx)
                == false)
            {
                return (CVI_VOID *)CVI_NULL;
            }

            MMF_CHN_S    chn = {.enModId = CVI_ID_VDEC, .s32DevId = 0, .s32ChnId = 0};
            struct vb_s *vb  = (struct vb_s *)vbBLK;

            // atomic_fetch_add(&vb->usr_cnt, 1); to do?
            // rhino_atomic_add(&vb->usr_cnt, 1); to do?
            // rt_hw_atomic_add(&vb->usr_cnt, 1); to do?
            atomic_inc(&vb->usr_cnt);

            chn.s32ChnId            = VdecChn;
            vb->buf.phy_addr[0]     = stVFrame.stVFrame.u64PhyAddr[0];
            vb->buf.phy_addr[1]     = stVFrame.stVFrame.u64PhyAddr[1];
            vb->buf.phy_addr[2]     = stVFrame.stVFrame.u64PhyAddr[2];
            vb->buf.stride[0]       = stVFrame.stVFrame.u32Stride[0];
            vb->buf.stride[1]       = stVFrame.stVFrame.u32Stride[1];
            vb->buf.stride[2]       = stVFrame.stVFrame.u32Stride[2];
            vb->buf.length[0]       = stVFrame.stVFrame.u32Length[0];
            vb->buf.length[1]       = stVFrame.stVFrame.u32Length[1];
            vb->buf.length[2]       = stVFrame.stVFrame.u32Length[2];
            vb->buf.size.u32Height  = stVFrame.stVFrame.u32Height;
            vb->buf.size.u32Width   = stVFrame.stVFrame.u32Width;
            vb->buf.s16OffsetLeft   = stVFrame.stVFrame.s16OffsetLeft;
            vb->buf.s16OffsetBottom = stVFrame.stVFrame.s16OffsetBottom;
            vb->buf.s16OffsetRight  = stVFrame.stVFrame.s16OffsetRight;
            vb->buf.s16OffsetTop    = stVFrame.stVFrame.s16OffsetTop;

            vb->buf.enPixelFormat = stVFrame.stVFrame.enPixelFormat;

            VB_CAL_CONFIG_S stVbCalConfig;

            COMMON_GetPicBufferConfig(
                stVFrame.stVFrame.u32Width, stVFrame.stVFrame.u32Height,
                stVFrame.stVFrame.enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,
                DEFAULT_ALIGN, &stVbCalConfig);

            vb_done_handler(chn, CHN_TYPE_OUT, vbBLK);

            for (i = 0; i < MAX_VDEC_FRM_NUM; i++)
            {
                if (pBindFrmQueue[i].bIsFrmOutput == false)
                {
                    pBindFrmQueue[i].vbBLK        = vbBLK;
                    pBindFrmQueue[i].stFrameInfo  = stVFrame;
                    pBindFrmQueue[i].bIsFrmOutput = true;
                    break;
                }
            }
        }
        else
        {
            usleep(1000);
        }
    }

    if (pBindFrmQueue != NULL)
    {
        free(pBindFrmQueue);
    }

    return (CVI_VOID *)CVI_SUCCESS;
}

static CVI_S32 CVI_VDEC_Init(void)
{
    CVI_S32 ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    ret = _VDEC_MMAP();
    if (ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("_VDEC_MMAP failed.\n");
        return ret;
    }

    cviGetDebugConfigFromDecProc();

    return ret;
}

#define MAX_VDEC_DISPLAYQ_NUM 32

CVI_S32 CVI_VDEC_CreateChn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
    CVI_S32             s32Ret = CVI_SUCCESS;
    pthread_mutexattr_t ma;

    CVI_VDEC_API("VdChn = %d\n", VdChn);

    CVI_VDEC_Init();

    s32Ret = _CVI_VDEC_InitHandle();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("_CVI_VDEC_InitHandle failed !\n");
        return s32Ret;
    }

    vdec_handle->chn_handle[VdChn] = calloc(1, sizeof(vdec_chn_context));
    if (vdec_handle->chn_handle[VdChn] == NULL)
    {
        CVI_VDEC_ERR("Allocate chn_handle memory failed !\n");
        return CVI_ERR_VDEC_NOMEM;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];
    pChnHandle->VdChn            = VdChn;
    pChnHandle->pVbCtx           = &vdec_vb_ctx[VdChn];

    memcpy(&pChnHandle->ChnAttr, pstAttr, sizeof(VDEC_CHN_ATTR_S));

    if ((pChnHandle->ChnAttr.enType == PT_H264) || (pChnHandle->ChnAttr.enType == PT_H265))
    {
        pChnHandle->VideoFrameArrayNum = MAX_VDEC_DISPLAYQ_NUM;
    }
    else
    {
        pChnHandle->VideoFrameArrayNum = pChnHandle->ChnAttr.u32FrameBufCnt;
    }

    pChnHandle->VideoFrameArray =
        calloc(pChnHandle->VideoFrameArrayNum, sizeof(VIDEO_FRAME_INFO_S));

    if (pChnHandle->VideoFrameArray == NULL)
    {
        CVI_VDEC_ERR("Allocate VideoFrameArray memory failed !\n");
        return CVI_ERR_VDEC_NOMEM;
    }

    CVI_VDEC_TRACE("u32FrameBufCnt = %d\n", pChnHandle->ChnAttr.u32FrameBufCnt);
    CVI_VDEC_TRACE("VideoFrameArrayNum = %d\n", pChnHandle->VideoFrameArrayNum);

    memset(pChnHandle->display_queue, -1, DISPLAY_QUEUE_SIZE);

    pthread_mutex_init(&pChnHandle->display_queue_lock, 0);
    pthread_mutex_init(&pChnHandle->status_lock, 0);

    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
    // pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&pChnHandle->chnShmMutex, &ma);

    if (pChnHandle->ChnAttr.enType == PT_JPEG || pChnHandle->ChnAttr.enType == PT_MJPEG)
    {
    }
    else if (pChnHandle->ChnAttr.enType == PT_H264 || pChnHandle->ChnAttr.enType == PT_H265)
    {
        cviInitDecConfig initDecCfg, *pInitDecCfg;

        pInitDecCfg = &initDecCfg;
        memset(pInitDecCfg, 0, sizeof(cviInitDecConfig));
        pInitDecCfg->codec =
            (pChnHandle->ChnAttr.enType == PT_H265) ? CODEC_H265 : CODEC_H264;
        pInitDecCfg->cviApiMode = API_MODE_SDK;

        pInitDecCfg->vbSrcMode    = vdec_handle->g_stModParam.enVdecVBSource;
        pInitDecCfg->chnNum       = VdChn;
        pInitDecCfg->bsBufferSize = pChnHandle->ChnAttr.u32StreamBufSize;

        s32Ret = cviVDecOpen(pInitDecCfg, &pChnHandle->pHandle);
        if (s32Ret < 0)
        {
            CVI_VDEC_ERR("cviVDecOpen, %d\n", s32Ret);
            return s32Ret;
        }

        if (vdec_handle->g_stModParam.enVdecVBSource != VB_SOURCE_COMMON)
        {
            cviVDecAttachCallBack(_cvi_vdec_CallbackFunc);
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_VDEC_DestroyChn(VDEC_CHN VdChn)
{
    CVI_S32                 s32Ret = CVI_SUCCESS;
    struct cvi_vdec_vb_ctx *pVbCtx = NULL;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];
    pVbCtx                       = pChnHandle->pVbCtx;

    if (IF_WANNA_DISABLE_BIND_MODE())
    {
        pthread_join(pVbCtx->thread, NULL);
        pVbCtx->currBindMode = CVI_FALSE;
    }

    s32Ret = _VDEC_UNMAP();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("_VDEC_UNMAP failed.\n");
        return s32Ret;
    }

    if (pChnHandle->ChnAttr.enType == PT_H264 || pChnHandle->ChnAttr.enType == PT_H265)
    {
        s32Ret = cviVDecClose(pChnHandle->pHandle);

        if (pChnHandle->bHasVbPool == true)
        {
            VB_BLK blk;

            for (CVI_U32 i = 0; i < pChnHandle->FrmNum; i++)
            {
                blk = CVI_VB_PhysAddr2Handle(pChnHandle->FrmArray[i].phyAddr);

                if (blk != VB_INVALID_HANDLE)
                {
                    CVI_VB_ReleaseBlock(blk);
                }
            }

            if (vdec_handle->g_stModParam.enVdecVBSource == VB_SOURCE_PRIVATE)
            {
                CVI_VB_DestroyPool(pChnHandle->vbPool.hPicVbPool);
                CVI_VDEC_TRACE("CVI_VB_DestroyPool: %d\n",
                               pChnHandle->vbPool.hPicVbPool);
            }
        }
    }

    pthread_mutex_destroy(&pChnHandle->display_queue_lock);
    pthread_mutex_destroy(&pChnHandle->status_lock);
    pthread_mutex_destroy(&pChnHandle->chnShmMutex);

    if (pChnHandle->VideoFrameArray)
    {
        free(pChnHandle->VideoFrameArray);
        pChnHandle->VideoFrameArray = NULL;
    }

    if (vdec_handle->chn_handle[VdChn])
    {
        free(vdec_handle->chn_handle[VdChn]);
        vdec_handle->chn_handle[VdChn] = NULL;
    }

    return s32Ret;
}

static CVI_S32 free_vdec_frame(VDEC_CHN_ATTR_S    *pstAttr,
                               VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
    VB_BLK         blk;

    if (pstAttr->enType == PT_JPEG || pstAttr->enType == PT_MJPEG)
    {
        blk = CVI_VB_PhysAddr2Handle(pstVFrame->u64PhyAddr[0]);
        if (blk != VB_INVALID_HANDLE)
        {
            CVI_VB_ReleaseBlock(blk);
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_VDEC_SetChnParam(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    memcpy(&pChnHandle->ChnParam, pstParam, sizeof(VDEC_CHN_PARAM_S));

    return s32Ret;
}

CVI_S32 CVI_VDEC_GetChnParam(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    memcpy(pstParam, &pChnHandle->ChnParam, sizeof(VDEC_CHN_PARAM_S));

    return s32Ret;
}

static pthread_mutex_t jpdLock = PTHREAD_MUTEX_INITIALIZER;

CVI_S32 CVI_VDEC_SendStream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream,
                            CVI_S32 s32MilliSec)
{
    CVI_S32                 s32Ret = CVI_SUCCESS;
    CVI_S32                 fb_idx;
    VIDEO_FRAME_INFO_S     *pstVideoFrame;
    CVI_BOOL                bNoFbWithDisp = CVI_FALSE;
    CVI_BOOL                bGetDispFrm   = CVI_FALSE;
    CVI_BOOL                bFlushDecodeQ = CVI_FALSE;
    CVI_S32                 s32TimeCost;
    CVI_S32                 s32TimeOutMs = s32MilliSec;
    CVI_U64                 u64DecHwTime = 0;
    struct cvi_vdec_vb_ctx *pVbCtx       = NULL;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];
    pVbCtx                       = pChnHandle->pVbCtx;

    if ((pstStream->u32Len == 0) && (pChnHandle->ChnAttr.enType == PT_JPEG || pChnHandle->ChnAttr.enType == PT_MJPEG))
        return CVI_SUCCESS;

    if ((pstStream->u32Len == 0) && (pstStream->bEndOfStream == CVI_FALSE))
        return CVI_SUCCESS;

    if ((pstStream->bEndOfStream == CVI_TRUE) && (pChnHandle->ChnAttr.enType != PT_JPEG && pChnHandle->ChnAttr.enType != PT_MJPEG))
    {
        bFlushDecodeQ = CVI_TRUE;
    }

    pChnHandle->u32SendStreamCnt++;
    s32Ret = cviSetVdecFpsToProc(VdChn, CVI_TRUE);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("(chn %d) cviSetVdecFpsToProc fail\n", VdChn);
        return s32Ret;
    }

    cviGetDebugConfigFromDecProc();

    // to do?
    // if (IF_WANNA_ENABLE_BIND_MODE()) {
    // 	struct sched_param param;
    // 	pthread_attr_t attr;

    // 	pVbCtx->currBindMode = CVI_TRUE;

    // 	param.sched_priority = 80;
    // 	pthread_attr_init(&attr);
    // 	pthread_attr_setschedpolicy(&attr, SCHED_RR);
    // 	pthread_attr_setschedparam(&attr, &param);
    // 	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    // 	pthread_create(&pVbCtx->thread,
    // 			&attr, vdec_event_handler, (CVI_VOID *) pChnHandle);
    // }

    while (1)
    {
        bNoFbWithDisp = CVI_FALSE;
        bGetDispFrm   = CVI_FALSE;

        fb_idx = get_avail_fb(VdChn, pChnHandle->VideoFrameArray);
        if (fb_idx < 0)
        {
            CVI_VDEC_WARN("cannot get any fb in VideoFrameArray\n");
            return CVI_ERR_VDEC_BUF_FULL;
        }

        pstVideoFrame = &pChnHandle->VideoFrameArray[fb_idx];
        CVI_VDEC_INFO("VdChn = %d, pts %llu, addr %p, len %d, eof %d, eos %d\n",
                      VdChn, pstStream->u64PTS, pstStream->pu8Addr,
                      pstStream->u32Len, pstStream->bEndOfFrame,
                      pstStream->bEndOfStream);

        pChnHandle->stStatus.u32LeftStreamBytes += pstStream->u32Len;

        CVI_U64 startTime = get_current_time();

        if (pChnHandle->ChnAttr.enType == PT_JPEG || pChnHandle->ChnAttr.enType == PT_MJPEG)
        {
            s32Ret = cviVdec_Mutex_Lock(&jpdLock, s32TimeOutMs, &s32TimeCost);
            if ((s32Ret == ETIMEDOUT) || (s32Ret == EBUSY))
            {
                pChnHandle->stStatus.u32LeftStreamBytes -= pstStream->u32Len;
            }
            CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
            s32Ret = cvi_jpeg_decode(VdChn, pstStream, pstVideoFrame, s32TimeOutMs,
                                     &u64DecHwTime);
            cviVdec_Mutex_Unlock(&jpdLock);
            if (s32Ret != CVI_SUCCESS)
            {
                if (s32Ret == CVI_ERR_VDEC_BUSY)
                {
                    pChnHandle->stStatus.u32LeftStreamBytes -= pstStream->u32Len;
                    CVI_VDEC_TRACE("jpeg timeout in nonblock mode[%d]\n", s32TimeOutMs);
                    return s32Ret;
                }
                CVI_VDEC_ERR("cvi_jpeg_decode error\n");
                goto ERR_CVI_VDEC_SEND_STREAM;
            }

            bGetDispFrm = CVI_TRUE;
        }
        else if ((pChnHandle->ChnAttr.enType == PT_H264) || (pChnHandle->ChnAttr.enType == PT_H265))
        {
            cviDispFrameCfg dispFrameCfg, *pdfc = &dispFrameCfg;

            if (pChnHandle->ChnAttr.enType == PT_H264)
            {
                s32Ret = cvi_h264_decode(pChnHandle->pHandle,
                                         pChnHandle->ChnParam.enPixelFormat, pstStream,
                                         s32TimeOutMs);
            }
            else
            {
                s32Ret = cvi_h265_decode(pChnHandle->pHandle,
                                         pChnHandle->ChnParam.enPixelFormat, pstStream,
                                         s32TimeOutMs);
            }

            if (bFlushDecodeQ == CVI_TRUE)
            {
                if (s32Ret == CVI_VDEC_RET_NO_FB)
                {
                    continue;
                }
                else if (s32Ret == CVI_VDEC_RET_LAST_FRM)
                {
                    s32Ret = CVI_SUCCESS;
                    break;
                }
            }
            else
            {
                if (s32Ret == CVI_VDEC_RET_DEC_ERR)
                {
                    s32Ret = CVI_SUCCESS;
                    CVI_VDEC_ERR("cvi_h26x_decode error\n");
                    pChnHandle->stStatus.u32LeftStreamBytes -= pstStream->u32Len;
                    goto ERR_CVI_VDEC_SEND_STREAM;
                }
                else if (s32Ret == CVI_VDEC_RET_NO_FB)
                {
                    s32Ret                                   = CVI_ERR_VDEC_BUF_FULL;
                    pChnHandle->stStatus.u32LeftStreamBytes -= pstStream->u32Len;
                    goto ERR_CVI_VDEC_SEND_STREAM;
                }
            }

            if (s32Ret == CVI_VDEC_RET_LOCK_TIMEOUT)
            {
                pChnHandle->stStatus.u32LeftStreamBytes -= pstStream->u32Len;
                CVI_VDEC_TRACE("26x dec timeout[%d] chn[%d]\n", s32TimeOutMs, VdChn);
                s32Ret = CVI_ERR_VDEC_BUSY;
                goto ERR_CVI_VDEC_SEND_STREAM;
            }

            if (s32Ret == CVI_VDEC_RET_NO_FB_WITH_DISP)
            {
                bNoFbWithDisp = CVI_TRUE;
                CVI_VDEC_TRACE("chn[%d] no fb for decode but disp frame is available\n",
                               VdChn);
            }

            if (s32Ret == CVI_VDEC_RET_FRM_DONE || s32Ret == CVI_VDEC_RET_NO_FB_WITH_DISP)
            {
                s32Ret = cviVDecGetFrame(pChnHandle->pHandle, pdfc);
                if (s32Ret >= 0)
                {
                    u64DecHwTime = pdfc->decHwTime;
                    cviSetVideoFrameInfo(pstVideoFrame, pdfc);
                    bGetDispFrm = CVI_TRUE;
                }

                if (!bGetDispFrm) bNoFbWithDisp = CVI_FALSE;

                s32Ret = CVI_SUCCESS;
            }
        }
        else
        {
            CVI_VDEC_ERR("enType = %d\n", pChnHandle->ChnAttr.enType);
            s32Ret = EN_ERR_NOT_SUPPORT;
            goto ERR_CVI_VDEC_SEND_STREAM;
        }

        pChnHandle->stStatus.u32LeftStreamBytes -= pstStream->u32Len;

        if (bGetDispFrm == CVI_TRUE)
        {
            s32Ret = cviSetVideoChnAttrToProc(VdChn, pstVideoFrame, u64DecHwTime);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VDEC_ERR("cviSetVideoChnAttrToProc fail");
                return s32Ret;
            }

            s32Ret = cviVdec_Mutex_Lock(&pChnHandle->display_queue_lock,
                                        VDEC_TIME_BLOCK_MODE, &s32TimeCost);
            CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
            pstVideoFrame->stVFrame.u32FrameFlag = 1;
            pstVideoFrame->stVFrame.u64PTS       = pstStream->u64PTS;
            s32Ret                               = insert_display_queue(pChnHandle->w_idx, fb_idx,
                                                                        pChnHandle->display_queue);
            cviVdec_Mutex_Unlock(&pChnHandle->display_queue_lock);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VDEC_ERR("insert_display_queue fail, w_idx %d, fb_idx %d\n",
                             pChnHandle->w_idx, fb_idx);
                return s32Ret;
            }

            pChnHandle->w_idx++;
            if (pChnHandle->w_idx == DISPLAY_QUEUE_SIZE) pChnHandle->w_idx = 0;
            s32Ret = cviVdec_Mutex_Lock(&pChnHandle->status_lock,
                                        VDEC_TIME_BLOCK_MODE, &s32TimeCost);
            CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
            pChnHandle->stStatus.u32LeftPics++;
            cviVdec_Mutex_Unlock(&pChnHandle->status_lock);
        }
        else
        {
            s32Ret = cviVdec_Mutex_Lock(&pChnHandle->display_queue_lock,
                                        VDEC_TIME_BLOCK_MODE, &s32TimeCost);
            CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
            // Can't get display frame now.
            // Release pstVideoFrame which get from get_avail_fb()
            pstVideoFrame->stVFrame.u32FrameFlag = 0;
            cviVdec_Mutex_Unlock(&pChnHandle->display_queue_lock);
            if (bFlushDecodeQ == CVI_TRUE)
            {
                CVI_VDEC_TRACE("---->chn[%d] Decode ENd\n", VdChn);
                CVI_VDEC_TRACE("leftstreambytes[%d], u32LeftPics[%d]\n",
                               pChnHandle->stStatus.u32LeftStreamBytes,
                               pChnHandle->stStatus.u32LeftPics);
                break;
            }
        }

        CVI_U64 endTime = get_current_time();

        pChnHandle->totalDecTime += (endTime - startTime);
        CVI_VDEC_PERF(
            "SendStream timestamp = %llu , dec time = %llu ms, total = %llu ms\n",
            (unsigned long long)(startTime),
            (unsigned long long)(endTime - startTime),
            (unsigned long long)(pChnHandle->totalDecTime));

        if (bNoFbWithDisp) continue;

        if (bFlushDecodeQ == CVI_FALSE) break;

        if ((pstStream->bEndOfStream == CVI_TRUE) && (s32MilliSec > 0))
        {
            CVI_VDEC_TRACE("force flush in EndOfStream in nonblock mode flush dec\n");
            s32TimeOutMs = -1;
        }
    }

ERR_CVI_VDEC_SEND_STREAM:
    return s32Ret;
}

static void cvi_jpeg_set_frame_info(VIDEO_FRAME_S *pVideoFrame,
                                    CVIFRAMEBUF   *pCVIFrameBuf)
{
    int c_h_shift = 0; // chroma height shift

    pVideoFrame->u32Width        = pCVIFrameBuf->width;
    pVideoFrame->u32Height       = pCVIFrameBuf->height;
    pVideoFrame->u32Stride[0]    = pCVIFrameBuf->strideY;
    pVideoFrame->u32Stride[1]    = pCVIFrameBuf->strideC;
    pVideoFrame->u32Stride[2]    = pCVIFrameBuf->strideC;
    pVideoFrame->s16OffsetTop    = 0;
    pVideoFrame->s16OffsetBottom = 0;
    pVideoFrame->s16OffsetLeft   = 0;
    pVideoFrame->s16OffsetRight  = 0;

    switch (pCVIFrameBuf->format)
    {
    case CVI_FORMAT_400:
        pVideoFrame->enPixelFormat = PIXEL_FORMAT_YUV_400;
        pVideoFrame->u32Stride[2]  = pCVIFrameBuf->strideC;
        break;
    case CVI_FORMAT_422:
        pVideoFrame->enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;
        pVideoFrame->u32Stride[2]  = pCVIFrameBuf->strideC;
        break;
    case CVI_FORMAT_444:
        pVideoFrame->enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_444;
        pVideoFrame->u32Stride[2]  = pCVIFrameBuf->strideC;
        break;
    case CVI_FORMAT_420:
    default:
        c_h_shift = 1;
        if (pCVIFrameBuf->chromaInterleave == CVI_CBCR_INTERLEAVE)
        {
            pVideoFrame->enPixelFormat = PIXEL_FORMAT_NV12;
            pVideoFrame->u32Stride[2]  = 0;
        }
        else if (pCVIFrameBuf->chromaInterleave == CVI_CRCB_INTERLEAVE)
        {
            pVideoFrame->enPixelFormat = PIXEL_FORMAT_NV21;
            pVideoFrame->u32Stride[2]  = 0;
        }
        else
        { // CVI_CBCR_SEPARATED
            pVideoFrame->enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
            pVideoFrame->u32Stride[2]  = pCVIFrameBuf->strideC;
        }
        break;
    }

    pVideoFrame->u32Length[0] = ALIGN(
        pVideoFrame->u32Stride[0] * pVideoFrame->u32Height, JPEGD_ALIGN_FRM);
    pVideoFrame->u32Length[1] =
        ALIGN(pVideoFrame->u32Stride[1] * (pVideoFrame->u32Height >> c_h_shift),
              JPEGD_ALIGN_FRM);
    pVideoFrame->u32Length[2] =
        ALIGN(pVideoFrame->u32Stride[2] * (pVideoFrame->u32Height >> c_h_shift),
              JPEGD_ALIGN_FRM);

    CVI_VDEC_INFO("jpeg dec %dx%d, strideY %d, strideC %d, sizeY %d, sizeC %d\n",
                  pVideoFrame->u32Width, pVideoFrame->u32Height,
                  pVideoFrame->u32Stride[0], pVideoFrame->u32Stride[1],
                  pVideoFrame->u32Length[0], pVideoFrame->u32Length[1]);
}

static CVI_S32 cvi_jpeg_decode(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream,
                               VIDEO_FRAME_INFO_S *pstVideoFrame,
                               CVI_S32 s32MilliSec, CVI_U64 *pu64DecHwTime)
{
    int               ret                = CVI_SUCCESS;
    CVIPackedFormat   enPackedFormat     = CVI_PACKED_FORMAT_NONE;
    CVICbCrInterLeave enChromaInterLeave = CVI_CBCR_SEPARATED;
    int               iRotAngle          = 0;
    int               iMirDir            = 0;
    int               iROIEnable         = 0;
    int               iROIOffsetX        = 0;
    int               iROIOffsetY        = 0;
    int               iROIWidth          = 0;
    int               iROIHeight         = 0;
    int               iVDownSampling     = 0;
    int               iHDownSampling     = 0;

    CVIJpgHandle      jpgHandle = {0};
    CVIDecConfigParam decConfig;
    CVIJpgConfig      config;

    memset(&decConfig, 0, sizeof(CVIDecConfigParam));
    memset(&config, 0, sizeof(CVIJpgConfig));

    UNUSED(VdChn);

    /* initial JPU core */
    ret = CVIJpgInit();
    if (ret != 0)
    {
        CVI_VDEC_ERR("\nFailed to CVIJpgInit!!!\n");
        return CVI_ERR_VDEC_ERR_INIT;
    }

    /* read source file data */
    unsigned char *srcBuf  = pstStream->pu8Addr;
    int            readLen = pstStream->u32Len;

    config.type = CVIJPGCOD_DEC;

    if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12)
        enChromaInterLeave = CVI_CBCR_INTERLEAVE;
    else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)
        enChromaInterLeave = CVI_CRCB_INTERLEAVE;

    // 2'b00: Normal
    // 2'b10: CbCr interleave (e.g. NV12 in 4:2:0 or NV16 in 4:2:2)
    // 2'b11: CrCb interleave (e.g. NV21 in 4:2:0)
    memset(&(config.u.dec), 0, sizeof(CVIDecConfigParam));
    config.u.dec.dec_buf.packedFormat     = enPackedFormat;
    config.u.dec.dec_buf.chromaInterleave = enChromaInterLeave;
    // ROI param
    config.u.dec.roiEnable  = iROIEnable;
    config.u.dec.roiWidth   = iROIWidth;
    config.u.dec.roiHeight  = iROIHeight;
    config.u.dec.roiOffsetX = iROIOffsetX;
    config.u.dec.roiOffsetY = iROIOffsetY;
    // Frame Partial Mode (DON'T SUPPORT)
    config.u.dec.usePartialMode = 0;
    // Rotation Angle (0, 90, 180, 270)
    config.u.dec.rotAngle = iRotAngle;
    // mirror direction (0-no mirror, 1-vertical, 2-horizontal,
    // 3-both)

    config.u.dec.mirDir                 = iMirDir;
    config.u.dec.iHorScaleMode          = iHDownSampling;
    config.u.dec.iVerScaleMode          = iVDownSampling;
    config.u.dec.iDataLen               = readLen;
    config.u.dec.dst_type               = JPEG_MEM_EXTERNAL;
    config.u.dec.dec_buf.vbY.phys_addr  = pstVideoFrame->stVFrame.u64PhyAddr[0];
    config.u.dec.dec_buf.vbCb.phys_addr = pstVideoFrame->stVFrame.u64PhyAddr[1];
    config.u.dec.dec_buf.vbCr.phys_addr = pstVideoFrame->stVFrame.u64PhyAddr[2];

    CVI_VDEC_INFO("iDataLen = %d\n", config.u.dec.iDataLen);

    /* Open JPU Devices */
    jpgHandle = CVIJpgOpen(config);
    if (jpgHandle == NULL)
    {
        CVI_VDEC_ERR("\nFailed to CVIJpgOpen\n");
        ret = CVI_ERR_VDEC_NULL_PTR;
        goto FIND_ERROR;
    }

    /* send jpeg data for decode or encode operator */
    ret = CVIJpgSendFrameData(jpgHandle, (void *)srcBuf, readLen, s32MilliSec);

    if (ret != 0)
    {
        if ((ret == VDEC_RET_TIMEOUT) && (s32MilliSec >= 0))
        {
            CVI_VDEC_TRACE("CVIJpgSendFrameData ret timeout\n");
            return CVI_ERR_VDEC_BUSY;
        }
        else
        {
            CVI_VDEC_ERR("\nFailed to CVIJpgSendFrameData, ret = %d\n", ret);
            ret = CVI_ERR_VDEC_ERR_SEND_FAILED;
            goto FIND_ERROR;
        }
    }

    CVIFRAMEBUF cviFrameBuf;

    memset(&cviFrameBuf, 0, sizeof(CVIFRAMEBUF));

    ret = CVIJpgGetFrameData(jpgHandle, (unsigned char *)&cviFrameBuf,
                             sizeof(CVIFRAMEBUF),
                             (unsigned long int *)pu64DecHwTime);
    if (ret != 0)
    {
        CVI_VDEC_ERR("\nFailed to CVIJpgGetFrameData, ret = %d\n", ret);
        ret = CVI_ERR_VDEC_ERR_GET_FAILED;
    }

    cvi_jpeg_set_frame_info(&pstVideoFrame->stVFrame, &cviFrameBuf);

    CVIJpgReleaseFrameData(jpgHandle);
FIND_ERROR:
    if (jpgHandle != NULL)
    {
        CVIJpgClose(jpgHandle);
        jpgHandle = NULL;
    }

    CVIJpgUninit();
    return ret;
}

static void cvi_h26x_set_output_format(cviDecOnePicCfg *pdopc,
                                       PIXEL_FORMAT_E   enPixelFormat)
{
    if (pdopc == NULL) return;

    switch (enPixelFormat)
    {
    case PIXEL_FORMAT_NV12:
        pdopc->cbcrInterleave = 1;
        pdopc->nv21           = 0;
        break;
    case PIXEL_FORMAT_NV21:
        pdopc->cbcrInterleave = 1;
        pdopc->nv21           = 1;
        break;
    default:
        pdopc->cbcrInterleave = 0;
        pdopc->nv21           = 0;
        break;
    }
}

static CVI_S32 cvi_h264_decode(void *pHandle, PIXEL_FORMAT_E enPixelFormat,
                               const VDEC_STREAM_S *pstStream,
                               CVI_S32              s32MilliSec)
{
    cviDecOnePicCfg dopc, *pdopc = &dopc;
    int             decStatus = 0;

    pdopc->bsAddr       = pstStream->pu8Addr;
    pdopc->bsLen        = pstStream->u32Len;
    pdopc->bEndOfStream = pstStream->bEndOfStream;
    cvi_h26x_set_output_format(pdopc, enPixelFormat);

    decStatus = cviVDecDecOnePic(pHandle, pdopc, s32MilliSec);

    return decStatus;
}

static CVI_S32 cvi_h265_decode(void *pHandle, PIXEL_FORMAT_E enPixelFormat,
                               const VDEC_STREAM_S *pstStream,
                               CVI_S32              s32MilliSec)
{
    cviDecOnePicCfg dopc, *pdopc = &dopc;
    int             decStatus = 0;

    pdopc->bsAddr       = pstStream->pu8Addr;
    pdopc->bsLen        = pstStream->u32Len;
    pdopc->bEndOfStream = pstStream->bEndOfStream;
    cvi_h26x_set_output_format(pdopc, enPixelFormat);

    decStatus = cviVDecDecOnePic(pHandle, pdopc, s32MilliSec);

    return decStatus;
}

static void cviSetVideoFrameInfo(VIDEO_FRAME_INFO_S *pstVideoFrame,
                                 cviDispFrameCfg    *pdfc)
{
    VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;

    pstVFrame->enPixelFormat =
        (pdfc->cbcrInterleave)
            ? ((pdfc->nv21) ? PIXEL_FORMAT_NV21 : PIXEL_FORMAT_NV12)
            : PIXEL_FORMAT_YUV_PLANAR_420;

    pstVFrame->pu8VirAddr[0] = pdfc->addrY;
    pstVFrame->pu8VirAddr[1] = pdfc->addrCb;
    pstVFrame->pu8VirAddr[2] = pdfc->addrCr;

    pstVFrame->u64PhyAddr[0] = pdfc->phyAddrY;
    pstVFrame->u64PhyAddr[1] = pdfc->phyAddrCb;
    pstVFrame->u64PhyAddr[2] = pdfc->phyAddrCr;

    pstVFrame->u32Width  = pdfc->width;
    pstVFrame->u32Height = pdfc->height;

    pstVFrame->u32Stride[0] = pdfc->strideY;
    pstVFrame->u32Stride[1] = pdfc->strideC;
    pstVFrame->u32Stride[2] = pdfc->cbcrInterleave ? 0 : pdfc->strideC;

    pstVFrame->u32Length[0] = pstVFrame->u32Stride[0] * pstVFrame->u32Height;
    pstVFrame->u32Length[1] = pstVFrame->u32Stride[1] * pstVFrame->u32Height / 2;
    pstVFrame->u32Length[2] = pstVFrame->u32Stride[2] * pstVFrame->u32Height / 2;

    pstVFrame->s16OffsetTop    = 0;
    pstVFrame->s16OffsetBottom = 0;
    pstVFrame->s16OffsetLeft   = 0;
    pstVFrame->s16OffsetRight  = 0;

    pstVFrame->pPrivateData = (void *)(uintptr_t)pdfc->indexFrameDisplay;
}

static CVI_S32 cviSetVideoChnAttrToProc(VDEC_CHN            VdChn,
                                        VIDEO_FRAME_INFO_S *psFrameInfo,
                                        CVI_U64             u64DecHwTime)
{
    CVI_S32           s32Ret = CVI_SUCCESS;
    VDEC_MOD_PARAM_S *pVdecModParam =
        (VDEC_MOD_PARAM_S *)(vdec_vpu_shared_mem + sizeof(proc_debug_config_t));
    vdec_proc_info_t *pProcInfoShareMem =
        (vdec_proc_info_t *)vdec_proc_shared_mem;
    vdec_proc_info_t *pProcSharedMem;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pProcInfoShareMem == NULL || pVdecModParam == NULL)
    {
        CVI_VDEC_ERR("invalid shared mem addr, Mod %p, ProcInfo %p\n",
                     pVdecModParam, pProcInfoShareMem);
        return CVI_ERR_VDEC_NULL_PTR;
    }
    if (psFrameInfo == NULL)
    {
        CVI_VDEC_ERR("psFrameInfo is NULL\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];
    memcpy(pVdecModParam, &vdec_handle->g_stModParam, sizeof(VDEC_MOD_PARAM_S));
    memcpy(&pChnHandle->stVideoFrameInfo, psFrameInfo,
           sizeof(VIDEO_FRAME_INFO_S));
    pChnHandle->stFPS.u64HwTime = u64DecHwTime;

    pProcSharedMem = (pProcInfoShareMem + VdChn);

    pthread_mutex_lock(&pChnHandle->chnShmMutex);

    memcpy(&pProcSharedMem->chnAttr, &pChnHandle->ChnAttr,
           sizeof(VDEC_CHN_ATTR_S));
    memcpy(&pProcSharedMem->stChnParam, &pChnHandle->ChnParam,
           sizeof(VDEC_CHN_PARAM_S));
    memcpy(&pProcSharedMem->stFrame, &psFrameInfo->stVFrame,
           sizeof(VIDEO_FRAME_S));
    pProcSharedMem->stFPS.u64HwTime = u64DecHwTime;
    pProcSharedMem->u16ChnNo        = VdChn;
    pProcSharedMem->u8ChnUsed       = 1;

    pthread_mutex_unlock(&pChnHandle->chnShmMutex);

    return s32Ret;
}

static CVI_VOID cviGetDebugConfigFromDecProc(void)
{
    proc_debug_config_t *ptDecProcDebugLevel =
        (proc_debug_config_t *)vdec_vpu_shared_mem;
    vdec_dbg *pDbg = &vdecDbg;

    CVI_VDEC_TRACE("\n");

    if (ptDecProcDebugLevel == NULL)
    {
        CVI_VDEC_ERR("ptDecProcDebugLevel is NULL\n");
        return;
    }

    memset(pDbg, 0, sizeof(vdec_dbg));

    pDbg->dbgMask = ptDecProcDebugLevel->u32DbgMask;
    if (pDbg->dbgMask == CVI_VDEC_NO_INPUT || pDbg->dbgMask == CVI_VDEC_INPUT_ERR)
        pDbg->dbgMask = CVI_VDEC_MASK_ERR;
    else
        pDbg->dbgMask |= CVI_VDEC_MASK_ERR;

    pDbg->currMask = pDbg->dbgMask;
    pDbg->startFn  = ptDecProcDebugLevel->u32StartFrmIdx;
    pDbg->endFn    = ptDecProcDebugLevel->u32EndFrmIdx;
    strcpy(pDbg->dbgDir, ptDecProcDebugLevel->cDumpPath);

    //	CVI_FUNC_COND(CVI_VDEC_MASK_DUMP_YUV, cviOpenDumpYuv());
    //	CVI_FUNC_COND(CVI_VDEC_MASK_DUMP_BS, cviOpenDumpBs());

    cviChangeVdecMask(0);
}

static CVI_S32 cviSetVdecFpsToProc(VDEC_CHN VdChn, CVI_BOOL bSendStream)
{
    CVI_S32           s32Ret = CVI_SUCCESS;
    VDEC_MOD_PARAM_S *pVdecModParam =
        (VDEC_MOD_PARAM_S *)(vdec_vpu_shared_mem + sizeof(proc_debug_config_t));
    vdec_proc_info_t *pProcInfoShareMem =
        (vdec_proc_info_t *)vdec_proc_shared_mem;
    vdec_proc_info_t *pProcSharedMem;
    CVI_U64           u64CurTime = get_current_time();

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pProcInfoShareMem == NULL || pVdecModParam == NULL)
    {
        CVI_VDEC_ERR("invalid shared mem addr, Mod %p, ProcInfo %p\n",
                     pVdecModParam, pProcInfoShareMem);
        return CVI_FAILURE;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    pProcSharedMem = (pProcInfoShareMem + VdChn);

    pthread_mutex_lock(&pChnHandle->chnShmMutex);
    if (bSendStream)
    {
        if ((u64CurTime - pChnHandle->u64LastSendStreamTimeStamp) > SEC_TO_MS)
        {
            pProcSharedMem->stFPS.u32InFPS         = (CVI_U32)((pChnHandle->u32SendStreamCnt * SEC_TO_MS) / ((CVI_U32)(u64CurTime - pChnHandle->u64LastSendStreamTimeStamp)));
            pChnHandle->u64LastSendStreamTimeStamp = u64CurTime;
            pChnHandle->u32SendStreamCnt           = 0;
        }
    }
    else
    {
        if ((u64CurTime - pChnHandle->u64LastGetFrameTimeStamp) > SEC_TO_MS)
        {
            pProcSharedMem->stFPS.u32OutFPS      = (CVI_U32)((pChnHandle->u32GetFrameCnt * SEC_TO_MS) / ((CVI_U32)(u64CurTime - pChnHandle->u64LastGetFrameTimeStamp)));
            pChnHandle->u64LastGetFrameTimeStamp = u64CurTime;
            pChnHandle->u32GetFrameCnt           = 0;
        }
    }
    pthread_mutex_unlock(&pChnHandle->chnShmMutex);

    return s32Ret;
}

CVI_S32 CVI_VDEC_QueryStatus(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    memcpy(pstStatus, &pChnHandle->stStatus, sizeof(VDEC_CHN_STATUS_S));

    return s32Ret;
}

CVI_S32 CVI_VDEC_ResetChn(VDEC_CHN VdChn)
{
    CVI_S32          s32Ret = CVI_SUCCESS;
    VDEC_CHN_ATTR_S *pstChnAttr;
    CVI_U32          u32FrameBufCnt;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    pstChnAttr = &pChnHandle->ChnAttr;

    if (pstChnAttr->enType == PT_H264 || pstChnAttr->enType == PT_H265)
    {
        s32Ret = cviVDecReset(pChnHandle->pHandle);
        if (s32Ret < 0)
        {
            CVI_VDEC_ERR("cviVDecReset, %d\n", s32Ret);
            return CVI_ERR_VDEC_ERR_INVALID_RET;
        }

        if (pChnHandle->VideoFrameArray != NULL)
        {
            u32FrameBufCnt = pChnHandle->ChnAttr.u32FrameBufCnt;
            memset(pChnHandle->VideoFrameArray, 0,
                   sizeof(VIDEO_FRAME_INFO_S) * u32FrameBufCnt);
        }
    }

    return s32Ret;
}

CVI_S32 CVI_VDEC_StartRecvStream(VDEC_CHN VdChn)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    if (pChnHandle->bStartRecv == CVI_FALSE)
    {
        // create VB buffer
        for (CVI_U32 i = 0; i < pChnHandle->ChnAttr.u32FrameBufCnt; i++)
        {
            s32Ret = allocate_vdec_frame(pChnHandle, i);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VDEC_ERR("allocate_vdec_frame, %d\n", s32Ret);
                return s32Ret;
            }
        }
    }

    pChnHandle->bStartRecv = CVI_TRUE;
    return s32Ret;
}

CVI_S32 CVI_VDEC_StopRecvStream(VDEC_CHN VdChn)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    if (pChnHandle->bStartRecv == CVI_TRUE)
    {
        if (pChnHandle->ChnAttr.enType == PT_JPEG || pChnHandle->ChnAttr.enType == PT_MJPEG)
        {
            for (CVI_U32 i = 0; i < pChnHandle->ChnAttr.u32FrameBufCnt; i++)
            {
                s32Ret = free_vdec_frame(&pChnHandle->ChnAttr,
                                         &pChnHandle->VideoFrameArray[i]);
                if (s32Ret != CVI_SUCCESS)
                {
                    CVI_VDEC_ERR("free_vdec_frame, %ud\n", s32Ret);
                    return s32Ret;
                }
            }
        }
    }

    pChnHandle->bStartRecv = CVI_FALSE;
    return s32Ret;
}

CVI_S32 CVI_VDEC_GetFrame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
                          CVI_S32 s32MilliSec)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 s32TimeCost;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];
    CVI_S32           fb_idx;

    s32Ret = cviVdec_Mutex_Lock(&pChnHandle->display_queue_lock, s32MilliSec,
                                &s32TimeCost);
    CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
    fb_idx = pChnHandle->display_queue[pChnHandle->r_idx];
    cviVdec_Mutex_Unlock(&pChnHandle->display_queue_lock);
    if (fb_idx < 0)
    {
        CVI_VDEC_WARN("get display queue fail, r_idx %d, fb_idx %d\n",
                      pChnHandle->r_idx, fb_idx);
        return CVI_ERR_VDEC_ERR_INVALID_RET;
    }

    pChnHandle->u32GetFrameCnt++;
    s32Ret = cviSetVdecFpsToProc(VdChn, CVI_FALSE);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("(chn %d) cviSetVdecFpsToProc fail\n", VdChn);
        return s32Ret;
    }

    memcpy(pstFrameInfo, &pChnHandle->VideoFrameArray[fb_idx],
           sizeof(VIDEO_FRAME_INFO_S));

    if (pChnHandle->stStatus.u32LeftPics <= 0)
    {
        CVI_VDEC_ERR("u32LeftPics %d\n", pChnHandle->stStatus.u32LeftPics);
        return CVI_ERR_VDEC_BUF_EMPTY;
    }

    s32Ret = cviVdec_Mutex_Lock(&pChnHandle->status_lock, VDEC_TIME_BLOCK_MODE,
                                &s32TimeCost);
    CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
    pChnHandle->stStatus.u32LeftPics--;
    pstFrameInfo->stVFrame.u32TimeRef             = pChnHandle->seqNum;
    pChnHandle->seqNum                           += 2;
    pChnHandle->display_queue[pChnHandle->r_idx]  = -1;
    pChnHandle->r_idx++;
    if (pChnHandle->r_idx == DISPLAY_QUEUE_SIZE) pChnHandle->r_idx = 0;
    cviVdec_Mutex_Unlock(&pChnHandle->status_lock);

    return s32Ret;
}

CVI_S32 CVI_VDEC_ReleaseFrame(VDEC_CHN                  VdChn,
                              const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    CVI_S32           s32Ret = CVI_SUCCESS;
    vdec_chn_context *pChnHandle;
    CVI_S32           fb_idx;
    VIDEO_FRAME_S    *pstVFrame;

    UNUSED(pstFrameInfo);

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = vdec_handle->chn_handle[VdChn];

    s32Ret = cviVdec_Mutex_Lock(&pChnHandle->display_queue_lock,
                                VDEC_DEFAULT_MUTEX_MODE, NULL);
    CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
    fb_idx = _cvi_vdec_FindFrameIdx(pChnHandle, pstFrameInfo);
    if (fb_idx < 0)
    {
        CVI_VDEC_ERR("Can't find video frame in VideoFrameArray!\n");
        cviVdec_Mutex_Unlock(&pChnHandle->display_queue_lock);
        return fb_idx;
    }

    pstVFrame = &pChnHandle->VideoFrameArray[fb_idx].stVFrame;
    if ((pChnHandle->ChnAttr.enType == PT_H264) || (pChnHandle->ChnAttr.enType == PT_H265))
    {
        cviVDecReleaseFrame(pChnHandle->pHandle, pstVFrame->pPrivateData);
    }

    CVI_VDEC_INFO("release fb_idx %d\n", fb_idx);
    pstVFrame->u32FrameFlag = 0;
    cviVdec_Mutex_Unlock(&pChnHandle->display_queue_lock);

    return CVI_SUCCESS;
}

CVI_S32 CVI_VDEC_SetChnAttr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    memcpy(&pChnHandle->ChnAttr, pstAttr, sizeof(VDEC_CHN_ATTR_S));

    return s32Ret;
}

CVI_S32 CVI_VDEC_GetChnAttr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    vdec_chn_context *pChnHandle = vdec_handle->chn_handle[VdChn];

    memcpy(pstAttr, &pChnHandle->ChnAttr, sizeof(VDEC_CHN_ATTR_S));

    return s32Ret;
}

CVI_S32 CVI_VDEC_AttachVbPool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    vdec_chn_context *pChnHandle = NULL;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstPool == NULL)
    {
        return CVI_ERR_VDEC_NULL_PTR;
    }

    if (vdec_handle->g_stModParam.enVdecVBSource != VB_SOURCE_USER)
    {
        CVI_VDEC_ERR("Not support enVdecVBSource:%d\n",
                     vdec_handle->g_stModParam.enVdecVBSource);
        return CVI_ERR_VDEC_NOT_SUPPORT;
    }

    pChnHandle = vdec_handle->chn_handle[VdChn];

    pChnHandle->vbPool     = *pstPool;
    pChnHandle->bHasVbPool = true;

    return s32Ret;
}

CVI_S32 CVI_VDEC_DetachVbPool(VDEC_CHN VdChn)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    vdec_chn_context *pChnHandle = NULL;

    CVI_VDEC_API("\n");

    s32Ret = check_vdec_chn_handle(VdChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (vdec_handle->g_stModParam.enVdecVBSource != VB_SOURCE_USER)
    {
        CVI_VDEC_ERR("Invalid detachVb in ChnId[%d] VBSource:[%d]\n", VdChn,
                     vdec_handle->g_stModParam.enVdecVBSource);
        return CVI_ERR_VDEC_NOT_SUPPORT;
    }

    pChnHandle = vdec_handle->chn_handle[VdChn];
    if (pChnHandle->bStartRecv != CVI_FALSE)
    {
        CVI_VDEC_ERR("Cannot detach vdec vb before StopRecvStream\n");
        return CVI_ERR_VDEC_ERR_SEQ_OPER;
    }

    if (pChnHandle->bHasVbPool == false)
    {
        CVI_VDEC_ERR("ChnId[%d] Null VB\n", VdChn);
        return CVI_SUCCESS;
    }
    else
    {
        if (pChnHandle->ChnAttr.enType == PT_H264 || pChnHandle->ChnAttr.enType == PT_H265)
        {
            CVI_VDEC_API("26x detach\n");
            if (pChnHandle->bHasVbPool == true)
            {
                VB_BLK blk;

                for (CVI_U32 i = 0; i < pChnHandle->FrmNum; i++)
                {
                    // CVI_SYS_Munmap(pChnHandle->FrmArray[i].virtAddr,
                    //	pChnHandle->FrmArray[i].size);
                    blk = CVI_VB_PhysAddr2Handle(pChnHandle->FrmArray[i].phyAddr);

                    if (blk != VB_INVALID_HANDLE) CVI_VB_ReleaseBlock(blk);
                }
            }
        }
        pChnHandle->bHasVbPool = false;
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_VDEC_SetModParam(const VDEC_MOD_PARAM_S *pstModParam)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (pstModParam == NULL)
    {
        return CVI_ERR_VDEC_ILLEGAL_PARAM;
    }

    s32Ret = _CVI_VDEC_InitHandle();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("_CVI_VDEC_InitHandle failed !\n");
        return s32Ret;
    }

    s32Ret =
        cviVdec_Mutex_Lock(&g_vdec_handle_mutex, VDEC_DEFAULT_MUTEX_MODE, NULL);
    CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
    memcpy(&vdec_handle->g_stModParam, pstModParam, sizeof(VDEC_MOD_PARAM_S));
    cviVdec_Mutex_Unlock(&g_vdec_handle_mutex);

    return s32Ret;
}

CVI_S32 CVI_VDEC_GetModParam(VDEC_MOD_PARAM_S *pstModParam)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (pstModParam == NULL)
    {
        return CVI_ERR_VDEC_ILLEGAL_PARAM;
    }

    s32Ret = _CVI_VDEC_InitHandle();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VDEC_ERR("_CVI_VDEC_InitHandle failed !\n");
        return s32Ret;
    }

    s32Ret =
        cviVdec_Mutex_Lock(&g_vdec_handle_mutex, VDEC_DEFAULT_MUTEX_MODE, NULL);
    CHECK_MUTEX_RET(s32Ret, __func__, __LINE__);
    memcpy(pstModParam, &vdec_handle->g_stModParam, sizeof(VDEC_MOD_PARAM_S));
    cviVdec_Mutex_Unlock(&g_vdec_handle_mutex);

    return s32Ret;
}
