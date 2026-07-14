
#ifndef _RTSP_SER_API_H_
#define _RTSP_SER_API_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RTSP_VIDEO_CODEC_H264 = 0,
    RTSP_VIDEO_CODEC_H265,
    RTSP_VIDEO_CODEC_MJPEG,
    RTSP_VIDEO_CODEC_BUTT
} RTSP_VIDEO_CODEC_E;

typedef enum {
    RTSP_AUDIO_CODEC_NONE,
    RTSP_AUDIO_CODEC_PCM,
    RTSP_AUDIO_CODEC_AAC,
    RTSP_AUDIO_CODEC_BUTT
} RTSP_AUDIO_CODEC_E;

typedef struct RTSP_SERVICE_PARAM {
    int32_t rtsp_id;
    char rtsp_name[32];
    int32_t max_conn;
    int32_t timeout;
    int32_t port;
    RTSP_VIDEO_CODEC_E video_codec;
    RTSP_AUDIO_CODEC_E audio_codec;

    uint32_t width;
    uint32_t height;
    float framerate;
    int32_t bitrate_kbps;
    int32_t audio_sample_rate;
    int32_t audio_channels;
    int32_t audio_pernum;

} RTSP_SERVICE_PARAM_S;

typedef void *RTSP_SERVICE_HANDLE_T;

#ifdef __cplusplus
}
#endif

#endif
