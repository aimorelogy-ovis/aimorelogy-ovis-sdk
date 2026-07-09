/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_uvc.h
 * Description:
 *   uvc module interface decalration
 */

#ifndef __CVI_UVC_H__
#define __CVI_UVC_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** uvc stream format enum */
typedef enum cviUVC_STREAM_FORMAT_E
{
    CVI_UVC_STREAM_FORMAT_YUV420 = 0,
    CVI_UVC_STREAM_FORMAT_MJPEG,
    CVI_UVC_STREAM_FORMAT_H264,
    CVI_UVC_STREAM_FORMAT_H265,
    CVI_UVC_STREAM_FORMAT_BUTT
} CVI_UVC_STREAM_FORMAT_E;

typedef enum cviUVC_VIDEOMODE_E
{
    CVI_UVC_VIDEOMODE_VGA30 = 0, /**<640x360   30fps */
    CVI_UVC_VIDEOMODE_720P30,    /**<1280x720  30fps */
    CVI_UVC_VIDEOMODE_1080P30,   /**<1920x1080 30fps */
    CVI_UVC_VIDEOMODE_4K30,      /**<3840x2160 30fps */
    CVI_UVC_VIDEOMODE_BUTT
} CVI_UVC_VIDEOMODE_E;

typedef struct cviUVC_VIDEOATTR_S
{
    uint32_t u32BitRate;
    CVI_UVC_VIDEOMODE_E enMode;
} CVI_UVC_VIDEOATTR_S;

typedef struct cviUVC_FORMAT_CAP_S
{
    uint32_t u32Cnt;
    CVI_UVC_VIDEOMODE_E enDefMode;
    CVI_UVC_VIDEOATTR_S astModes[CVI_UVC_VIDEOMODE_BUTT];
} CVI_UVC_FORMAT_CAP_S;

/** uvc device capabilities, including all format */
typedef struct cviUVC_DEVICE_CAP_S
{
    CVI_UVC_FORMAT_CAP_S astFmtCaps[CVI_UVC_STREAM_FORMAT_BUTT];
} CVI_UVC_DEVICE_CAP_S;

/** uvc stream attribute */
typedef struct tagUVC_STREAM_ATTR_S {
    CVI_UVC_STREAM_FORMAT_E enFormat;
    uint32_t u32Width;
    uint32_t u32Height;
    uint32_t u32Fps;
    uint32_t u32BitRate;
} UVC_STREAM_ATTR_S;

/* UVC Context */
typedef struct tagUVC_CONTEXT_S {
    char szDevPath[64];
    bool bRun;
    bool bPCConnect;
    pthread_t TskId;
    pthread_t Tsk2Id;
} UVC_CONTEXT_S;

typedef struct cviUVC_DATA_SOURCE_S
{
    uint32_t VcapHdl; /**<vcap handle */
    uint32_t VprocHdl; /**<vproc handle */
    uint32_t VencHdl;  /**<venc handle */
    uint32_t AcapHdl;  /**<audio handle */
    uint32_t VprocChnId; /**<vproc chn id */
} CVI_UVC_DATA_SOURCE_S;

typedef struct cviUVC_BUFFER_CFG_S
{
    uint32_t u32BufCnt;
    uint32_t u32BufSize;
} CVI_UVC_BUFFER_CFG_S;


int32_t UVC_Init(const CVI_UVC_DEVICE_CAP_S *pstCap, const CVI_UVC_DATA_SOURCE_S *pstDataSrc,
                 CVI_UVC_BUFFER_CFG_S *pstBufferCfg);
int32_t UVC_Deinit(void);
int32_t UVC_Start(const char *pDevPath);
int32_t UVC_Stop(void);

int32_t UVC_STREAM_ReqIDR(void);

UVC_CONTEXT_S *UVC_GetCtx(void);

void app_uvc_exit(void);
int app_uvc_init(void);

int cvi_uvc_stream_send_data(void *data);

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#endif /* __CVI_UVC_H__ */