#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RTSP_PORT (8554)
#define RTSP_MAX_CLIENTS (8)
#define RTSP_SESSION_PREFIX "stream"

typedef enum {
	RTSP_CODEC_H264,
	RTSP_CODEC_H265,
	RTSP_CODEC_MJPEG, // not supported yet
	RTSP_CODEC_BUTT
} rtsp_codec_type_e;

typedef struct {
	uint8_t *data;
	int size;
	uint64_t ts; // pts
} cvi_rtsp_frame_t;

typedef struct {
	int width;
	int height;
	rtsp_codec_type_e codec_type;
} cvi_rtsp_video_info_t;

typedef struct {
	void *play_arg;
	void (*on_play)(int id, void *arg);
	void *teardown_arg;
	void (*on_teardown)(int id, void *arg);
} cvi_rtsp_event_callback_t;

typedef struct {
	cvi_rtsp_video_info_t video_info;
	cvi_rtsp_event_callback_t event_cb;
} cvi_rtsp_cfg_t;

int cvi_rtsp_send_frame(int id, cvi_rtsp_frame_t *frame);
int cvi_rtsp_init(int id, cvi_rtsp_cfg_t *cfg);
int cvi_rtsp_deinit(int id);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __RTSP_SERVER_H__
