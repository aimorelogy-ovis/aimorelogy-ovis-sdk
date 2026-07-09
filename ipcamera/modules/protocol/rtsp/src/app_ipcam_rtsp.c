#include <sys/prctl.h>
#include <stdlib.h>
#include <string.h>

#include "app_ipcam_rtsp.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_venc.h"
#include "cvi_mbuf.h"

#ifdef AUDIO_SUPPORT
#include "app_ipcam_audio.h"
#endif

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_RTSP_T stRtspCtx;
static APP_PARAM_RTSP_T *pstRtspCtx = &stRtspCtx;
static pthread_mutex_t RtspMutex = PTHREAD_MUTEX_INITIALIZER;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
APP_PARAM_RTSP_T *app_ipcam_Rtsp_Param_Get(void)
{
    return pstRtspCtx;
}

static CVI_S32 app_ipcam_RtspAttr_Init(VENC_CHN vencChn, CVI_S32 session_id
    , RTSP_SER_ATTR_S *pstAttr)
{
    /* update vidoe streaming codec type from video attr */
    APP_VENC_CHN_CFG_S *pstVencChnCfg =
        app_ipcam_VencChnCfg_Get(vencChn);
    if (pstVencChnCfg == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "venc[%d] config is null. \n", vencChn);
        return CVI_FAILURE;
    }

    switch (pstVencChnCfg->enType) {
        case PT_H264:
            pstAttr->video_codec = RTSP_VIDEO_H264;
        break;
        case PT_H265:
            pstAttr->video_codec = RTSP_VIDEO_H265;
        break;
        case PT_MJPEG:
            pstAttr->video_codec = RTSP_VIDEO_MJPEG;
        break;
        default:
            pstAttr->video_codec = RTSP_VIDEO_BUTT;
            APP_PROF_LOG_PRINT(LEVEL_INFO, "No support type:%d.\n"
                , pstVencChnCfg->enType);
        break;
    }

    pstAttr->framerate = pstVencChnCfg->u32DstFrameRate;

#if defined(AUDIO_SUPPORT) && defined(RTSP_AUDIO_ENABLE)
    pstAttr->audio_en = CVI_TRUE;
    APP_PARAM_AUDIO_CFG_T *pstAudioCfg = app_ipcam_Audio_Param_Get();
    if (pstAudioCfg == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "audio config is null. \n");
        return CVI_FAILURE;
    }
    if (pstAudioCfg->bInit) {
        pstAttr->audio_codec       = RTSP_AUDIO_PCM;
        pstAttr->audio_sample_rate = pstVencChnCfg->u32DstFrameRate * 320;
        pstAttr->audio_channels    = 1;
        pstAttr->audio_pernum      = pstAudioCfg->astAudioCfg.u32PtNumPerFrm;
    }
#else
    pstAttr->audio_en          = CVI_FALSE;
    pstAttr->audio_codec       = RTSP_AUDIO_BUTT;
    pstAttr->audio_sample_rate = 0;
    pstAttr->audio_channels    = 0;
    pstAttr->audio_pernum      = 0;
#endif

    pstAttr->timeout = 120;

    APP_PROF_LOG_PRINT(LEVEL_INFO,
        "VencChn_%d attach to Session_%d with CodecType=%d, fps:%f."
        "audio en:%d, codec:%d, rate:%d, chns:%d, pernum:%d, timeout:%d.\n"
        , vencChn
        , session_id
        , pstAttr->video_codec
        , pstAttr->framerate
        , pstAttr->audio_en
        , pstAttr->audio_codec
        , pstAttr->audio_sample_rate
        , pstAttr->audio_channels
        , pstAttr->audio_pernum
        , pstAttr->timeout);

    return CVI_SUCCESS;
}

static void rtsp_service_media_task(void *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    RTSP_FRAME_S frame = {0};
    RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)arg;

    if (ctx == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Input param is null. \n");
        return ;
    }

    CVI_CHAR TaskName[64] = {'\0'};
    snprintf(TaskName, sizeof(TaskName), "app_rtsp_%d_thread", ctx->attr.id);
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s running\n", TaskName);

    CVI_S32 mbufId = ctx->attr.id;
    CVI_MBUF_HANDLE* readerId = app_ipcam_Mbuf_CreateReader(mbufId, 1);

    CVI_MEDIA_FRAME_INFO_T stReadFrameInfo;
    memset(&stReadFrameInfo.frameParam, 0, sizeof(stReadFrameInfo.frameParam));
    stReadFrameInfo.frameBuf = malloc(CVI_MBUF_STREAM_MAX_SIZE);
    if (NULL == stReadFrameInfo.frameBuf) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "frameBuf malloc fail\n");
        return ;
    }

    // Ensure that the first frame is I-frame
    ctx->i_frame_flag = 1;
    while(ctx->RtspThread.bRun_flag) {
        stReadFrameInfo.frameBufLen = CVI_MBUF_STREAM_MAX_SIZE;
        s32Ret = app_ipcam_Mbuf_ReadFrame(readerId, ctx->i_frame_flag
                                        , &stReadFrameInfo, 100);
        if (s32Ret < 0) {
            APP_PROF_LOG_PRINT(LEVEL_INFO
                , "app_ipcam_Mbuf_ReadFrame failed. s32Ret:%d.\n", s32Ret);
            continue;
        }
        if ((ctx->i_frame_flag)
            && (stReadFrameInfo.frameParam.frameType != CVI_MEDIA_VFRAME_I)) {
            // Wait for the I-frame ...
            continue;
        }
        ctx->i_frame_flag = 0;

        if (stReadFrameInfo.frameParam.frameLen > 0) {
            frame.data[0] = stReadFrameInfo.frameBuf;
            frame.iskey[0] = 1;
            frame.len[0] = stReadFrameInfo.frameParam.frameLen;
            OSAL_TIME_GetBootTimeUs(&frame.vi_pts[0]);
            // send video data
            if ((stReadFrameInfo.frameParam.frameType == CVI_MEDIA_VFRAME_P) ||
                (stReadFrameInfo.frameParam.frameType == CVI_MEDIA_VFRAME_I)) {
                frame.type = FRAME_TYPE_VIDEO;
                s32Ret = RTSP_SendFrame(ctx->rtsp_ser, &frame);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTSP_WriteFrame failed\n");
                }
            } else if (stReadFrameInfo.frameParam.frameType == CVI_MEDIA_AFRAME_A) {
#if defined(AUDIO_SUPPORT) && defined(RTSP_AUDIO_ENABLE)
                // send audio data
                frame.type = FRAME_TYPE_AUDIO;
                s32Ret = RTSP_SendFrame(ctx->rtsp_ser, &frame);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTSP_WriteFrame failed\n");
                }
#else
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "drop audio frame in rtsp.\n");
#endif
            } else {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "No support frameType:%d. \n"
                    , stReadFrameInfo.frameParam.frameType);
            }
        }
    }

    if (readerId) {
        app_ipcam_Mbuf_DestoryReader(readerId);
        readerId = NULL;
    }

    if (stReadFrameInfo.frameBuf) {
        free(stReadFrameInfo.frameBuf);
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s exit. \n", TaskName);
}

static void rtsp_service_start_media_by_name(char *name)
{
    if (name == NULL || strlen(name) <= 0) {
        return;
    }

    pthread_mutex_lock(&RtspMutex);
    for (CVI_S32 i = 0; i < RTSP_INSTANCE_NUM; i++) {
        RTSP_SERVICE_CONTEXT_S *c = pstRtspCtx->rtsp_ctx[i];
        if (c) {
            OSAL_MUTEX_Lock(c->mutex);
            if (strcmp(c->attr.rtsp_name, name) == 0) {
                if (c->ref == 0) {
                    c->RtspThread.bRun_flag = 1;
                    OSAL_TASK_ATTR_S video;
                    static char v_name[64] = {0};
                    snprintf(v_name, sizeof(v_name), "m_%s", name);
                    video.name = v_name;
                    video.entry = rtsp_service_media_task;
                    video.param = (void *)c;
                    video.priority = OSAL_TASK_PRI_NORMAL;
                    video.detached = false;
                    video.stack_size = 128 * 1024;
                    OSAL_TASK_Create(&video, &c->media_task);
                    APP_PROF_LOG_PRINT(LEVEL_INFO
                        , "Create thread(%s) is success for rtsp_service_media_task.\n"
                        , name);
                }
                c->ref++;
                APP_PROF_LOG_PRINT(LEVEL_INFO, "session %s start play %d. \n"
                    , name, c->ref);
                OSAL_MUTEX_Unlock(c->mutex);
                break;
            }
            OSAL_MUTEX_Unlock(c->mutex);
        }
    }

    pthread_mutex_unlock(&RtspMutex);
}

static void rtsp_service_stop_media_by_name(char *name)
{
    if (name == NULL || strlen(name) <= 0) {
        return;
    }
    pthread_mutex_lock(&RtspMutex);
    for (CVI_S32 i = 0; i < RTSP_INSTANCE_NUM; i++) {
        RTSP_SERVICE_CONTEXT_S *c = pstRtspCtx->rtsp_ctx[i];
        if (c) {
            OSAL_MUTEX_Lock(c->mutex);
            if (strcmp(c->attr.rtsp_name, name) == 0) {
                c->ref--;
                if (c->ref == 0) {
                    c->RtspThread.bRun_flag = 0;
                    OSAL_TASK_Join(c->media_task);
                    OSAL_TASK_Destroy(&c->media_task);
                    APP_PROF_LOG_PRINT(LEVEL_INFO
                        , "Destroy thread(%s) is success for rtsp_service_media_task.\n"
                        , name);
                }
                OSAL_MUTEX_Unlock(c->mutex);
                break;
            }
            OSAL_MUTEX_Unlock(c->mutex);
        }
    }

    pthread_mutex_unlock(&RtspMutex);
}

static void rtsp_service_event_cb(RTSP_EVENT_S *e)
{
    if (e) {
        switch (e->e) {
        case RTSP_EVENT_CLI_CONNECT: {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service: "
                "recv %s CONNECT %s Event. \n", e->rtsp_name, e->cli_ipaddr);
            rtsp_service_start_media_by_name(e->rtsp_name);
        } break;
        case RTSP_EVENT_CLI_DISCONNECT: {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service: "
                "recv %s DISCONNECT %s Event. \n", e->rtsp_name, e->cli_ipaddr);
            rtsp_service_stop_media_by_name(e->rtsp_name);
        } break;
        default:
            break;
        }
    }
}

static CVI_S32 rtsp_service_create(RTSP_SERVICE_CONTEXT_S **rtsp_ctx
    , RTSP_SER_ATTR_S *attr)
{
    if (pstRtspCtx->rtsp_ctx[attr->id] != NULL) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s is exist", attr->rtsp_name);
        return CVI_FAILURE;
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service init start %s\n"
        , attr->rtsp_name);
    RTSP_SERVICE_CONTEXT_S *ctx =
        (RTSP_SERVICE_CONTEXT_S *)malloc(sizeof(RTSP_SERVICE_CONTEXT_S));
    if (ctx == NULL) {
        return CVI_FAILURE;
    }
    memset(ctx, 0x0, sizeof(RTSP_SERVICE_CONTEXT_S));
    memcpy(&ctx->attr, attr, sizeof(RTSP_SER_ATTR_S));
    ctx->mute = ((attr->audio_en == 1) ? 0 : 1);
    OSAL_MUTEX_ATTR_S mutex_attr;
    mutex_attr.name = "rtsp_service_mutex";
    mutex_attr.type = PTHREAD_MUTEX_NORMAL;
    OSAL_MUTEX_Create(&mutex_attr, &ctx->mutex);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service init mute %d\n", ctx->mute);

    RTSP_INFO_S rtsp_info;
    memset(&rtsp_info, 0x0, sizeof(RTSP_INFO_S));
    RTSP_MEDIA_INFO_S media_info;
    memset(&media_info, 0x0, sizeof(RTSP_MEDIA_INFO_S));

    strcpy(rtsp_info.username, attr->username);
    strcpy(rtsp_info.password, attr->password);
    rtsp_info.max_conn = attr->max_conn;
    rtsp_info.timeout = attr->timeout;
    rtsp_info.port = attr->port;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service init rtsp_info %d %d %d. \n"
        , attr->port, attr->max_conn, attr->timeout);

    media_info.id = attr->id;
    strcpy(media_info.rtsp_name, attr->rtsp_name);
    media_info.framerate = attr->framerate;
    media_info.bitrate_kbps = attr->bitrate_kbps;
    media_info.video_codec = attr->video_codec;
    media_info.audio_codec = attr->audio_codec;
    media_info.audio_channels = attr->audio_channels;
    media_info.audio_pernum = attr->audio_pernum;
    media_info.audio_sample_rate = attr->audio_sample_rate;
    media_info.rtp_port = 10000;
    media_info.video_port = 20000;
    media_info.audio_port = 21000;
    media_info.cb = rtsp_service_event_cb;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service init media_info %f %d %d\n"
        , attr->framerate, attr->video_codec, attr->audio_pernum);

    RTSP_Create(&ctx->rtsp_ser, &rtsp_info, &media_info);
    if (ctx->rtsp_ser == NULL) {
        OSAL_MUTEX_Destroy(ctx->mutex);
        free(ctx);
        return CVI_FAILURE;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp id:%d name:%s create success\n"
        , attr->id, ctx->attr.rtsp_name);
    pstRtspCtx->rtsp_ctx[attr->id] = ctx;
    *rtsp_ctx = ctx;

    return CVI_SUCCESS;
}

static void rtsp_service_destroy(RTSP_SERVICE_CONTEXT_S *rtsp_ctx)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service_destroy start.\n");

    if (rtsp_ctx == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s is no exist"
            , rtsp_ctx->attr.rtsp_name);
        return;
    }

    s32Ret = RTSP_Destroy(rtsp_ctx->rtsp_ser);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "RTSP_Destroy failed. s32Ret:%d.\n"
            , s32Ret);
        return;
    }

    while (rtsp_ctx->ref > 0) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service: recv %s DISCONNECT. \n", rtsp_ctx->attr.rtsp_name);
        rtsp_service_stop_media_by_name(rtsp_ctx->attr.rtsp_name);
    }
    OSAL_MUTEX_Destroy(rtsp_ctx->mutex);
    free(rtsp_ctx);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp_service_destroy done.\n");
}

CVI_S32 app_ipcam_Rtsp_Server_Create(CVI_VOID)
{
    CVI_S32 s32Ret = 0;
    RTSP_SER_ATTR_S stAttr[RTSP_INSTANCE_NUM];
    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Rtsp_Server_Create start.\n");

    for (CVI_S32 i = 0; i < pstRtspCtx->session_cnt; i++) {
        s32Ret = app_ipcam_RtspAttr_Init(pstRtspCtx->VencChn[i], i, &stAttr[i]);
        if (s32Ret != CVI_SUCCESS) {
            return CVI_FAILURE;
        }

        strcpy(stAttr[i].username, "test");   // unused
        strcpy(stAttr[i].password, "123456"); // unused
        stAttr[i].id = i;
        stAttr[i].max_conn = pstRtspCtx->session_cnt;
        snprintf(stAttr[i].rtsp_name, MAX_RTSP_NAME_LEN, "live%d", i);
        stAttr[i].bitrate_kbps = pstRtspCtx->bitrate_kbps[i];
        stAttr[i].port = pstRtspCtx->port;

        s32Ret = rtsp_service_create(&pstRtspCtx->rtsp_ctx[i], &stAttr[i]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "rtsp_service_create %d failed", i);
            return CVI_FAILURE;
        }
        pstRtspCtx->rtsp_ctx[i]->attr.bitrate_kbps = pstRtspCtx->bitrate_kbps[i];
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_Rtsp_Server_Create done.\n");
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_rtsp_Server_Destroy(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_rtsp_Server_Destroy start.\n");

    for (CVI_U32 i = 0; i < RTSP_INSTANCE_NUM; i++) {
        if (pstRtspCtx->rtsp_ctx[i]) {
            rtsp_service_destroy(pstRtspCtx->rtsp_ctx[i]);
            pstRtspCtx->rtsp_ctx[i] = NULL;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_rtsp_Server_Destroy done.\n");
    return CVI_SUCCESS;
}
