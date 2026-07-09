/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: app_ipcam_uvc_host.h
 * Description:
 *   uvc host module interface declaration
 */

#ifndef __APP_IPCAM_UVC_HOST_H__
#define __APP_IPCAM_UVC_HOST_H__

#include <stddef.h>
#include <pthread.h>
#include "cvi_common.h"
#include "cvi_comm_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define APP_IPCAM_UVC_HOST_DEV_PATH_LEN   (64)
#define APP_IPCAM_UVC_HOST_PIXFMT_LEN     (8)

typedef struct tagAPP_PARAM_UVC_HOST_CFG_S {
    CVI_BOOL bEnable;
    char szDevPath[APP_IPCAM_UVC_HOST_DEV_PATH_LEN];
    CVI_U32 u32Width;
    CVI_U32 u32Height;
    char szPixFmt[APP_IPCAM_UVC_HOST_PIXFMT_LEN];
    CVI_U32 u32SkipFrame;
    CVI_BOOL bFeedVpss;
    CVI_U32 u32VpssGrp;
} APP_PARAM_UVC_HOST_CFG_S;

typedef struct tagAPP_IPCAM_UVC_HOST_BUFFER_S {
    void *start;
    size_t length;
} APP_IPCAM_UVC_HOST_BUFFER_S;

typedef struct tagAPP_IPCAM_UVC_HOST_CONTEXT_S {
    CVI_BOOL bRun;
    CVI_BOOL bStreamOn;
    CVI_BOOL bThreadCreated;
    pthread_t tskId;
    CVI_S32 s32Fd;

    APP_PARAM_UVC_HOST_CFG_S *pstHostCfg;
    CVI_U32 u32ActiveWidth;
    CVI_U32 u32ActiveHeight;
    CVI_U32 u32ActivePixFmt;
    CVI_U32 u32SrcStride;
    CVI_U32 u32BufferCount;

    CVI_U64 u64FrameSeq;
    CVI_U64 u64LastFrameLogSeq;
    CVI_U64 u64FeedOkCnt;
    CVI_U64 u64FeedDropCnt;

    CVI_U32 u32FeedYStride;
    CVI_U32 u32FeedYSize;
    CVI_U32 u32FeedBufSize;
    CVI_U64 u64FeedPhyAddr;
    CVI_U8 *pu8FeedVirAddr;
    VIDEO_FRAME_INFO_S stFeedFrame;

    APP_IPCAM_UVC_HOST_BUFFER_S *pstBuffers;
} APP_IPCAM_UVC_HOST_CONTEXT_S;

APP_PARAM_UVC_HOST_CFG_S *app_ipcam_UvcHost_Param_Get(void);

CVI_S32 app_ipcam_UvcHost_Init(void);
CVI_S32 app_ipcam_UvcHost_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __APP_IPCAM_UVC_HOST_H__ */
