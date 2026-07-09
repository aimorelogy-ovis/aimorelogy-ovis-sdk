#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include "app_ipcam_uvc_host.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_comm.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include "cvi_buffer.h"
#include "cvi_comm_vb.h"

#define UVC_HOST_BUFFER_COUNT           4
#define UVC_HOST_SELECT_TIMEOUT_SEC     2
#define UVC_HOST_LOG_INTERVAL           500
#define UVC_HOST_ION_NAME               "uvc_host"

static CVI_BOOL s_bUvcHostInit = CVI_FALSE;
static APP_IPCAM_UVC_HOST_CONTEXT_S s_stUvcHostCtx = {0};
static APP_PARAM_UVC_HOST_CFG_S s_stUvcHostCfg = {0};

APP_PARAM_UVC_HOST_CFG_S *app_ipcam_UvcHost_Param_Get(void)
{
    return &s_stUvcHostCfg;
}

static CVI_S32 app_ipcam_uvc_host_xioctl(CVI_S32 fd, unsigned long request, void *arg)
{
    CVI_S32 s32Ret;

    do {
        s32Ret = ioctl(fd, request, arg);
    } while (s32Ret == -1 && errno == EINTR);

    return s32Ret;
}

static CVI_U32 app_ipcam_uvc_host_pixfmt_from_cfg(const APP_PARAM_UVC_HOST_CFG_S *pstHostCfg)
{
    CVI_U32 u32PixFmt = V4L2_PIX_FMT_YUYV;

    if (pstHostCfg == NULL || strlen(pstHostCfg->szPixFmt) != 4) {
        return u32PixFmt;
    }

    u32PixFmt = v4l2_fourcc(pstHostCfg->szPixFmt[0],
                            pstHostCfg->szPixFmt[1],
                            pstHostCfg->szPixFmt[2],
                            pstHostCfg->szPixFmt[3]);

    return u32PixFmt;
}

static void app_ipcam_uvc_host_copy_yuyv_rows(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx,
                                               const CVI_U8 *pu8Data,
                                               CVI_U32 u32SrcStride)
{
    CVI_U32 row;
    CVI_U32 lineBytes = pstHostCtx->u32ActiveWidth * 2;

    for (row = 0; row < pstHostCtx->u32ActiveHeight; ++row) {
        CVI_U8 *pu8Dst = pstHostCtx->stFeedFrame.stVFrame.pu8VirAddr[0] + row * pstHostCtx->u32FeedYStride;
        const CVI_U8 *pu8Src = pu8Data + row * u32SrcStride;
        memcpy(pu8Dst, pu8Src, lineBytes);
    }
}

static void app_ipcam_uvc_host_release_feed_buffer(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    if (pstHostCtx->pu8FeedVirAddr != NULL) {
        CVI_SYS_IonFree(pstHostCtx->u64FeedPhyAddr, pstHostCtx->pu8FeedVirAddr);
        pstHostCtx->pu8FeedVirAddr = NULL;
        pstHostCtx->u64FeedPhyAddr = 0;
    }

    memset(&pstHostCtx->stFeedFrame, 0, sizeof(pstHostCtx->stFeedFrame));
    pstHostCtx->u32FeedYStride = 0;
    pstHostCtx->u32FeedYSize = 0;
    pstHostCtx->u32FeedBufSize = 0;
}

/**
 * What changed: Host feed buffer uses host-only config structure and removes duplicated default config macros.
 * Previous behavior: Host file had an independent set of UVC_HOST_DEFAULT_xxx and duplicated config copy context.
 * Impact: Runtime behavior is driven by host-only ini-derived config source, reducing duplicated maintenance points.
 *
 * Examples:
 * - WN2 640x512 YUYV: lineBytes = 640 * 2 = 1280, expected payload >= 1280 * 512 = 655360.
 * - Buffer size follows COMMON_GetPicBufferConfig(active_width, active_height, PIXEL_FORMAT_YUYV, ...).
 */
static CVI_S32 app_ipcam_uvc_host_prepare_feed_buffer(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VB_CAL_CONFIG_S stCalConfig;

    if (pstHostCtx->u32ActivePixFmt != V4L2_PIX_FMT_YUYV) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "uvc host feed_vpss only supports YUYV, fmt=%c%c%c%c\n",
                           (pstHostCtx->u32ActivePixFmt & 0xFF),
                           ((pstHostCtx->u32ActivePixFmt >> 8) & 0xFF),
                           ((pstHostCtx->u32ActivePixFmt >> 16) & 0xFF),
                           ((pstHostCtx->u32ActivePixFmt >> 24) & 0xFF));
        return CVI_FAILURE;
    }

    if ((pstHostCtx->u32ActiveWidth & 0x1) || (pstHostCtx->u32ActiveHeight & 0x1)) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "uvc host feed_vpss requires even size, got %ux%u\n",
                           pstHostCtx->u32ActiveWidth,
                           pstHostCtx->u32ActiveHeight);
        return CVI_FAILURE;
    }

    memset(&stCalConfig, 0, sizeof(stCalConfig));
    COMMON_GetPicBufferConfig(pstHostCtx->u32ActiveWidth,
                              pstHostCtx->u32ActiveHeight,
                              PIXEL_FORMAT_YUYV,
                              DATA_BITWIDTH_8,
                              COMPRESS_MODE_NONE,
                              DEFAULT_ALIGN,
                              &stCalConfig);

    s32Ret = CVI_SYS_IonAlloc_Cached(&pstHostCtx->u64FeedPhyAddr,
                                     (CVI_VOID **)&pstHostCtx->pu8FeedVirAddr,
                                     UVC_HOST_ION_NAME,
                                     stCalConfig.u32VBSize);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "CVI_SYS_IonAlloc_Cached failed, ret=0x%x\n",
                           s32Ret);
        return s32Ret;
    }

    pstHostCtx->u32FeedYStride = stCalConfig.u32MainStride;
    pstHostCtx->u32FeedYSize = stCalConfig.u32MainYSize;
    pstHostCtx->u32FeedBufSize = stCalConfig.u32VBSize;

    memset(&pstHostCtx->stFeedFrame, 0, sizeof(pstHostCtx->stFeedFrame));
    pstHostCtx->stFeedFrame.u32PoolId = VB_INVALID_POOLID;
    pstHostCtx->stFeedFrame.stVFrame.u32Width = pstHostCtx->u32ActiveWidth;
    pstHostCtx->stFeedFrame.stVFrame.u32Height = pstHostCtx->u32ActiveHeight;
    pstHostCtx->stFeedFrame.stVFrame.enPixelFormat = PIXEL_FORMAT_YUYV;
    pstHostCtx->stFeedFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    pstHostCtx->stFeedFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    pstHostCtx->stFeedFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;
    pstHostCtx->stFeedFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
    pstHostCtx->stFeedFrame.stVFrame.u32Stride[0] = pstHostCtx->u32FeedYStride;
    pstHostCtx->stFeedFrame.stVFrame.u64PhyAddr[0] = pstHostCtx->u64FeedPhyAddr;
    pstHostCtx->stFeedFrame.stVFrame.pu8VirAddr[0] = pstHostCtx->pu8FeedVirAddr;
    pstHostCtx->stFeedFrame.stVFrame.u32Length[0] = pstHostCtx->u32FeedYSize;

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_uvc_host_feed_vpss(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx,
                                            const CVI_U8 *pu8Data,
                                            CVI_U32 u32Bytes)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U64 u64Pts = 0;
    CVI_U32 lineBytes;
    CVI_U32 srcStride;
    CVI_U32 minBytes;
    CVI_U32 srcNeedBytes;
    CVI_U64 totalFeed;
    APP_PARAM_UVC_HOST_CFG_S *pstHostCfg = pstHostCtx->pstHostCfg;

    if (!pstHostCfg->bFeedVpss) {
        return CVI_SUCCESS;
    }

    if (pstHostCtx->pu8FeedVirAddr == NULL || pstHostCtx->u32FeedBufSize == 0) {
        return CVI_FAILURE;
    }

    lineBytes = pstHostCtx->u32ActiveWidth * 2;
    srcStride = pstHostCtx->u32SrcStride;
    minBytes = lineBytes * pstHostCtx->u32ActiveHeight;

    if (u32Bytes < minBytes) {
        pstHostCtx->u64FeedDropCnt++;
        return CVI_FAILURE;
    }

    srcNeedBytes = srcStride * (pstHostCtx->u32ActiveHeight - 1) + lineBytes;
    if (u32Bytes < srcNeedBytes) {
        srcStride = lineBytes;
    }

    app_ipcam_uvc_host_copy_yuyv_rows(pstHostCtx, pu8Data, srcStride);

    CVI_SYS_GetCurPTS(&u64Pts);
    pstHostCtx->stFeedFrame.stVFrame.u64PTS = u64Pts;
    pstHostCtx->stFeedFrame.stVFrame.u64DTS = u64Pts;
    pstHostCtx->stFeedFrame.stVFrame.u32SeqenceNo = (CVI_U32)(pstHostCtx->u64FrameSeq & 0xFFFFFFFF);
    pstHostCtx->stFeedFrame.stVFrame.u32TimeRef = (CVI_U32)(pstHostCtx->u64FrameSeq & 0xFFFFFFFF);

    s32Ret = CVI_SYS_IonFlushCache(pstHostCtx->u64FeedPhyAddr,
                                   pstHostCtx->pu8FeedVirAddr,
                                   pstHostCtx->u32FeedBufSize);
    if (s32Ret != CVI_SUCCESS) {
        pstHostCtx->u64FeedDropCnt++;
        return s32Ret;
    }

    s32Ret = CVI_VPSS_SendFrame(pstHostCfg->u32VpssGrp,
                                &pstHostCtx->stFeedFrame,
                                3000);
    if (s32Ret == CVI_SUCCESS) {
        pstHostCtx->u64FeedOkCnt++;
    } else {
        pstHostCtx->u64FeedDropCnt++;
    }

    totalFeed = pstHostCtx->u64FeedOkCnt + pstHostCtx->u64FeedDropCnt;
    if ((totalFeed % UVC_HOST_LOG_INTERVAL) == 0) {
        APP_PROF_LOG_PRINT(LEVEL_TRACE,
                           "uvc host: vpss feed grp=%u ok=%llu drop=%llu last_ret=0x%x\n",
                           pstHostCfg->u32VpssGrp,
                           (unsigned long long)pstHostCtx->u64FeedOkCnt,
                           (unsigned long long)pstHostCtx->u64FeedDropCnt,
                           s32Ret);
    }

    return s32Ret;
}

static CVI_S32 app_ipcam_uvc_host_query_capability(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    struct v4l2_capability cap;

    memset(&cap, 0, sizeof(cap));
    if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_QUERYCAP, &cap) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "VIDIOC_QUERYCAP failed, %s(%d)\n",
                           strerror(errno),
                           errno);
        return CVI_FAILURE;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "%s is not VIDEO_CAPTURE\n",
                           pstHostCtx->pstHostCfg->szDevPath);
        return CVI_FAILURE;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "%s is not STREAMING\n",
                           pstHostCtx->pstHostCfg->szDevPath);
        return CVI_FAILURE;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "uvc host: driver=%s card=%s bus=%s\n",
                       cap.driver,
                       cap.card,
                       cap.bus_info);

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_uvc_host_set_format(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    struct v4l2_format fmt;
    APP_PARAM_UVC_HOST_CFG_S *pstHostCfg = pstHostCtx->pstHostCfg;

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = pstHostCfg->u32Width;
    fmt.fmt.pix.height = pstHostCfg->u32Height;
    fmt.fmt.pix.pixelformat = app_ipcam_uvc_host_pixfmt_from_cfg(pstHostCfg);
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_S_FMT, &fmt) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "VIDIOC_S_FMT failed, %s(%d)\n",
                           strerror(errno),
                           errno);
        return CVI_FAILURE;
    }

    pstHostCtx->u32ActiveWidth = fmt.fmt.pix.width;
    pstHostCtx->u32ActiveHeight = fmt.fmt.pix.height;
    pstHostCtx->u32ActivePixFmt = fmt.fmt.pix.pixelformat;
    pstHostCtx->u32SrcStride = fmt.fmt.pix.bytesperline;
    if (pstHostCtx->u32SrcStride < (pstHostCtx->u32ActiveWidth * 2)) {
        pstHostCtx->u32SrcStride = pstHostCtx->u32ActiveWidth * 2;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "uvc host: active format=%ux%u %c%c%c%c stride=%u\n",
                       pstHostCtx->u32ActiveWidth,
                       pstHostCtx->u32ActiveHeight,
                       (pstHostCtx->u32ActivePixFmt & 0xFF),
                       ((pstHostCtx->u32ActivePixFmt >> 8) & 0xFF),
                       ((pstHostCtx->u32ActivePixFmt >> 16) & 0xFF),
                       ((pstHostCtx->u32ActivePixFmt >> 24) & 0xFF),
                       pstHostCtx->u32SrcStride);

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_uvc_host_init_mmap(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    struct v4l2_requestbuffers req;
    CVI_U32 i;

    memset(&req, 0, sizeof(req));
    req.count = UVC_HOST_BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_REQBUFS, &req) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "VIDIOC_REQBUFS failed, %s(%d)\n",
                           strerror(errno),
                           errno);
        return CVI_FAILURE;
    }

    if (req.count < 2) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "insufficient buffers, count=%u\n",
                           req.count);
        return CVI_FAILURE;
    }

    pstHostCtx->pstBuffers = calloc(req.count, sizeof(*pstHostCtx->pstBuffers));
    if (pstHostCtx->pstBuffers == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "calloc host buffers failed\n");
        return CVI_FAILURE;
    }

    for (i = 0; i < req.count; ++i) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_QUERYBUF, &buf) < 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "VIDIOC_QUERYBUF failed, %s(%d)\n",
                               strerror(errno),
                               errno);
            return CVI_FAILURE;
        }

        pstHostCtx->pstBuffers[i].length = buf.length;
        pstHostCtx->pstBuffers[i].start = mmap(NULL,
                                               buf.length,
                                               PROT_READ | PROT_WRITE,
                                               MAP_SHARED,
                                               pstHostCtx->s32Fd,
                                               buf.m.offset);
        if (pstHostCtx->pstBuffers[i].start == MAP_FAILED) {
            pstHostCtx->pstBuffers[i].start = NULL;
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "mmap failed, %s(%d)\n",
                               strerror(errno),
                               errno);
            return CVI_FAILURE;
        }
    }

    pstHostCtx->u32BufferCount = req.count;
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_uvc_host_queue_all(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    CVI_U32 i;

    for (i = 0; i < pstHostCtx->u32BufferCount; ++i) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_QBUF, &buf) < 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "VIDIOC_QBUF failed, %s(%d)\n",
                               strerror(errno),
                               errno);
            return CVI_FAILURE;
        }
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_uvc_host_stream_on(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_STREAMON, &type) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "VIDIOC_STREAMON failed, %s(%d)\n",
                           strerror(errno),
                           errno);
        return CVI_FAILURE;
    }

    pstHostCtx->bStreamOn = CVI_TRUE;
    return CVI_SUCCESS;
}

static void app_ipcam_uvc_host_stream_off(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (!pstHostCtx->bStreamOn) {
        return;
    }

    app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_STREAMOFF, &type);
    pstHostCtx->bStreamOn = CVI_FALSE;
}

static void app_ipcam_uvc_host_cleanup_mmap(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    CVI_U32 i;

    if (pstHostCtx->pstBuffers == NULL) {
        return;
    }

    for (i = 0; i < pstHostCtx->u32BufferCount; ++i) {
        if (pstHostCtx->pstBuffers[i].start != NULL) {
            munmap(pstHostCtx->pstBuffers[i].start, pstHostCtx->pstBuffers[i].length);
        }
    }

    free(pstHostCtx->pstBuffers);
    pstHostCtx->pstBuffers = NULL;
    pstHostCtx->u32BufferCount = 0;
}

static void app_ipcam_uvc_host_reset_context(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx)
{
    pstHostCtx->bRun = CVI_FALSE;

    if (pstHostCtx->bThreadCreated) {
        pthread_join(pstHostCtx->tskId, NULL);
        pstHostCtx->bThreadCreated = CVI_FALSE;
    }

    app_ipcam_uvc_host_stream_off(pstHostCtx);
    app_ipcam_uvc_host_cleanup_mmap(pstHostCtx);
    app_ipcam_uvc_host_release_feed_buffer(pstHostCtx);

    if (pstHostCtx->s32Fd >= 0) {
        close(pstHostCtx->s32Fd);
        pstHostCtx->s32Fd = -1;
    }

    pstHostCtx->bStreamOn = CVI_FALSE;
    pstHostCtx->tskId = (pthread_t)-1;
    pstHostCtx->u32ActiveWidth = 0;
    pstHostCtx->u32ActiveHeight = 0;
    pstHostCtx->u32ActivePixFmt = V4L2_PIX_FMT_YUYV;
    pstHostCtx->u32SrcStride = 0;
    pstHostCtx->u64FrameSeq = 0;
    pstHostCtx->u64LastFrameLogSeq = 0;
    pstHostCtx->u64FeedOkCnt = 0;
    pstHostCtx->u64FeedDropCnt = 0;
}

static void app_ipcam_uvc_host_handle_frame(APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx,
                                             const CVI_U8 *pu8Data,
                                             CVI_U32 u32Bytes)
{
    APP_PARAM_UVC_HOST_CFG_S *pstHostCfg = pstHostCtx->pstHostCfg;

    if (pstHostCfg->bFeedVpss) {
        app_ipcam_uvc_host_feed_vpss(pstHostCtx, pu8Data, u32Bytes);
    }

    if ((pstHostCtx->u64FrameSeq - pstHostCtx->u64LastFrameLogSeq) >= UVC_HOST_LOG_INTERVAL) {
        APP_PROF_LOG_PRINT(LEVEL_TRACE,
                           "uvc host: frame_seq=%llu bytes=%u fmt=%c%c%c%c %ux%u\n",
                           (unsigned long long)pstHostCtx->u64FrameSeq,
                           u32Bytes,
                           (pstHostCtx->u32ActivePixFmt & 0xFF),
                           ((pstHostCtx->u32ActivePixFmt >> 8) & 0xFF),
                           ((pstHostCtx->u32ActivePixFmt >> 16) & 0xFF),
                           ((pstHostCtx->u32ActivePixFmt >> 24) & 0xFF),
                           pstHostCtx->u32ActiveWidth,
                           pstHostCtx->u32ActiveHeight);
        pstHostCtx->u64LastFrameLogSeq = pstHostCtx->u64FrameSeq;
    }
}

/* ***
* What changed: Add key-function block comment for host capture thread main loop.
* Previous behavior: The thread had no high-level flow note, making maintenance harder.
* Impact: Clarifies blocking point, error handling, and frame data path to VPSS.
* 1) Block on select() to wait MMAP buffer ready from V4L2 camera.
* 2) DQBUF to fetch one frame, validate index/bytes, then feed VPSS pipeline.
* 3) QBUF to return buffer back to driver, keep streaming loop continuous.
*/
static void *app_ipcam_uvc_host_capture_task(void *arg)
{
    APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx = (APP_IPCAM_UVC_HOST_CONTEXT_S *)arg;

    prctl(PR_SET_NAME, "cvitask_uvc_host", 0, 0, 0);

    while (pstHostCtx->bRun) {
        fd_set fds;
        struct timeval timeout;
        CVI_S32 s32Ret;
        struct v4l2_buffer buf;

        FD_ZERO(&fds);
        FD_SET(pstHostCtx->s32Fd, &fds);
        timeout.tv_sec = UVC_HOST_SELECT_TIMEOUT_SEC;
        timeout.tv_usec = 0;

        s32Ret = select(pstHostCtx->s32Fd + 1, &fds, NULL, NULL, &timeout);
        if (s32Ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "select failed, %s(%d)\n",
                               strerror(errno),
                               errno);
            break;
        }

        if (s32Ret == 0) {
            continue;
        }

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) {
                continue;
            }
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "VIDIOC_DQBUF failed, %s(%d)\n",
                               strerror(errno),
                               errno);
            break;
        }

        if (buf.index >= pstHostCtx->u32BufferCount) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "invalid buffer index=%u, max=%u\n",
                               buf.index,
                               pstHostCtx->u32BufferCount);
            break;
        }

        /*
         * What changed: Add explicit note for warm-up(skip) frames and effective(valid) frames.
         * Previous behavior: The skip gate was present but the fixed-frame and valid-frame intent was undocumented.
         * Impact: Frame policy is now clear: seq <= skip_frame are warm-up frames, seq > skip_frame are valid frames.
         */
        pstHostCtx->u64FrameSeq++;
        if (pstHostCtx->u64FrameSeq > pstHostCtx->pstHostCfg->u32SkipFrame) {
            app_ipcam_uvc_host_handle_frame(pstHostCtx,
                                            (const CVI_U8 *)pstHostCtx->pstBuffers[buf.index].start,
                                            buf.bytesused);
        }

        if (app_ipcam_uvc_host_xioctl(pstHostCtx->s32Fd, VIDIOC_QBUF, &buf) < 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                               "VIDIOC_QBUF(requeue) failed, %s(%d)\n",
                               strerror(errno),
                               errno);
            break;
        }
    }

    return NULL;
}

CVI_S32 app_ipcam_UvcHost_Init(void)
{
    APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx = &s_stUvcHostCtx;
    APP_PARAM_UVC_HOST_CFG_S *pstHostCfg;

    if (s_bUvcHostInit == CVI_TRUE) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "uvc host already initialized\n");
        return CVI_SUCCESS;
    }

    app_ipcam_uvc_host_reset_context(pstHostCtx);
    pstHostCtx->pstHostCfg = app_ipcam_UvcHost_Param_Get();
    pstHostCfg = pstHostCtx->pstHostCfg;

    if (pstHostCfg->bEnable == CVI_FALSE) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "uvc host disabled by ini, skip init\n");
        return CVI_SUCCESS;
    }

    if (pstHostCfg->szDevPath[0] == '\0') {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "invalid host_dev, please configure [uvc_config].host_dev\n");
        goto failed;
    }
    if (pstHostCfg->u32Width == 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "invalid host_width, please configure [uvc_config].host_width\n");
        goto failed;
    }
    if (pstHostCfg->u32Height == 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "invalid host_height, please configure [uvc_config].host_height\n");
        goto failed;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "uvc host config dev=%s, %ux%u fmt=%s skip=%u feed_vpss=%u vpss_grp=%u\n",
                       pstHostCfg->szDevPath,
                       pstHostCfg->u32Width,
                       pstHostCfg->u32Height,
                       pstHostCfg->szPixFmt,
                       pstHostCfg->u32SkipFrame,
                       (unsigned int)pstHostCfg->bFeedVpss,
                       pstHostCfg->u32VpssGrp);

    pstHostCtx->s32Fd = open(pstHostCfg->szDevPath, O_RDWR | O_NONBLOCK, 0);
    if (pstHostCtx->s32Fd < 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
                           "open %s failed, %s(%d)\n",
                           pstHostCfg->szDevPath,
                           strerror(errno),
                           errno);
        goto failed;
    }

    if (app_ipcam_uvc_host_query_capability(pstHostCtx) != CVI_SUCCESS) {
        goto failed;
    }

    if (app_ipcam_uvc_host_set_format(pstHostCtx) != CVI_SUCCESS) {
        goto failed;
    }

    if (app_ipcam_uvc_host_prepare_feed_buffer(pstHostCtx) != CVI_SUCCESS) {
        goto failed;
    }

    if (app_ipcam_uvc_host_init_mmap(pstHostCtx) != CVI_SUCCESS) {
        goto failed;
    }

    if (app_ipcam_uvc_host_queue_all(pstHostCtx) != CVI_SUCCESS) {
        goto failed;
    }

    if (app_ipcam_uvc_host_stream_on(pstHostCtx) != CVI_SUCCESS) {
        goto failed;
    }

    pstHostCtx->bRun = CVI_TRUE;
    if (pthread_create(&pstHostCtx->tskId, NULL, app_ipcam_uvc_host_capture_task, pstHostCtx) != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "create host capture thread failed\n");
        pstHostCtx->bRun = CVI_FALSE;
        goto failed;
    }
    pstHostCtx->bThreadCreated = CVI_TRUE;

    APP_PROF_LOG_PRINT(LEVEL_INFO,
                       "uvc host start capture on %s, active=%ux%u fmt=%c%c%c%c skip=%u feed_vpss=%u grp=%u\n",
                       pstHostCfg->szDevPath,
                       pstHostCtx->u32ActiveWidth,
                       pstHostCtx->u32ActiveHeight,
                       (pstHostCtx->u32ActivePixFmt & 0xFF),
                       ((pstHostCtx->u32ActivePixFmt >> 8) & 0xFF),
                       ((pstHostCtx->u32ActivePixFmt >> 16) & 0xFF),
                       ((pstHostCtx->u32ActivePixFmt >> 24) & 0xFF),
                       pstHostCfg->u32SkipFrame,
                       (unsigned int)pstHostCfg->bFeedVpss,
                       pstHostCfg->u32VpssGrp);

    s_bUvcHostInit = CVI_TRUE;
    return CVI_SUCCESS;

failed:
    app_ipcam_uvc_host_reset_context(pstHostCtx);
    s_bUvcHostInit = CVI_FALSE;
    return CVI_FAILURE;
}

CVI_S32 app_ipcam_UvcHost_DeInit(void)
{
    APP_IPCAM_UVC_HOST_CONTEXT_S *pstHostCtx = &s_stUvcHostCtx;
    APP_PARAM_UVC_HOST_CFG_S *pstHostCfg = app_ipcam_UvcHost_Param_Get();

    if (pstHostCfg->bEnable == CVI_FALSE) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "uvc host disabled by ini, skip exit\n");
        return CVI_SUCCESS;
    }

    if (s_bUvcHostInit == CVI_FALSE && pstHostCtx->s32Fd < 0 && pstHostCtx->bThreadCreated == CVI_FALSE) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "uvc host not initialized\n");
        return CVI_SUCCESS;
    }

    app_ipcam_uvc_host_reset_context(pstHostCtx);
    s_bUvcHostInit = CVI_FALSE;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "uvc host capture stopped\n");
    return CVI_SUCCESS;
}
