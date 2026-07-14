#ifndef __APP_IPCAM_RTSP_H__
#define __APP_IPCAM_RTSP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include "cvi_type.h"
#include "cvi_common.h"
#include "app_ipcam_comm.h"
#include "rtsp.h"
#include "osal.h"

typedef struct RTSP_SER_ATTR {
    CVI_S32 id;
    CVI_CHAR rtsp_name[MAX_RTSP_NAME_LEN];
    CVI_S32 max_conn;
    CVI_S32 timeout;
    CVI_S32 port;
    CVI_S32 auth_en;
    CVI_CHAR username[MAX_RTSP_NAME_LEN];
    CVI_CHAR password[MAX_RTSP_NAME_LEN];
    RTSP_VIDEO_FORMAT_E video_codec;
    RTSP_AUDIO_FORMAT_E audio_codec;
    CVI_S32 audio_en;
    CVI_FLOAT framerate;
    CVI_S32 bitrate_kbps;
    CVI_S32 audio_sample_rate;
    CVI_S32 audio_channels;
    CVI_S32 audio_pernum;
} RTSP_SER_ATTR_S;

typedef struct RTSP_SERVICE_CONTEXT {
    CVI_S32 ref;
    CVI_S32 mute;
    CVI_CHAR *mute_data;
    RTSP_SER_ATTR_S attr;
    OSAL_MUTEX_HANDLE_S mutex;
    OSAL_TASK_HANDLE_S media_task;
    CVI_S32 video_fd;
    CVI_S32 audio_fd;
    CVI_S32 video_exit;
    CVI_S32 audio_exit;
    CVI_VOID *rtsp_ser;
    RUN_THREAD_PARAM RtspThread;
    CVI_S32 i_frame_flag;
} RTSP_SERVICE_CONTEXT_S;

typedef struct APP_PARAM_RTSP_S {
    CVI_S32 session_cnt;
    CVI_S32 port;
    VENC_CHN VencChn[RTSP_INSTANCE_NUM];
    CVI_S32 bitrate_kbps[RTSP_INSTANCE_NUM];
    RTSP_SERVICE_CONTEXT_S *rtsp_ctx[RTSP_INSTANCE_NUM];
} APP_PARAM_RTSP_T;

APP_PARAM_RTSP_T *app_ipcam_Rtsp_Param_Get(CVI_VOID);
int app_ipcam_Rtsp_Server_Create(CVI_VOID);
int app_ipcam_rtsp_Server_Destroy(CVI_VOID);

#ifdef __cplusplus
}
#endif

#endif
