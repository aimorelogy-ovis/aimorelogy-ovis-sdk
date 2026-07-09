
#include <sys/prctl.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "rtsp"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "cvi_isp.h"
#include "cvi_venc.h"
#include "rtsp.h"

static int rtsp_server_cnt;
static CVI_RTSP_CTX *prtsp_server_ctx;

static void *worker(void *arg);

static void rtsp_connect(const char *ip, void *arg)
{
	(void)arg;
	clog_i("rtsp connect: %s\n", ip);
}

static void rtsp_disconnect(const char *ip, void *arg)
{
	(void)arg;
	clog_i("rtsp disconnect: %s\n", ip);
}

static int init(struct module_t *thiz)
{
	module_rtsp_cfg_t *cfg = (module_rtsp_cfg_t *)thiz->module_cfg;

	clog_i("chn_id: %d\n", thiz->pipe_id);

	if (rtsp_server_cnt == 0) {
		CVI_RTSP_CONFIG config;

		memset(&config, 0, sizeof(CVI_RTSP_CONFIG));

		config.packetLen = cfg->bitrate;
		config.port = cfg->rtsp_port;
		config.tcpBufSize = cfg->max_buf_size;

		if (CVI_RTSP_Create(&prtsp_server_ctx, &config) < 0) {
			clog_e("create rtsp server fail...\n");
			return -1;
		}

		CVI_RTSP_SetOutPckBuf_MaxSize(cfg->max_buf_size);

		CVI_RTSP_STATE_LISTENER listener;

		memset(&listener, 0, sizeof(CVI_RTSP_STATE_LISTENER));
		listener.onConnect = rtsp_connect;
		listener.argConn = thiz;
		listener.onDisconnect = rtsp_disconnect;
		listener.argDisconn = thiz;
		CVI_RTSP_SetListener(prtsp_server_ctx, &listener);

		if (CVI_RTSP_Start(prtsp_server_ctx) < 0) {
			clog_e("fail to rtsp start\n");
			return -1;
		}
	}

	rtsp_server_cnt++;
	return 0;
}

static int deinit(struct module_t *thiz)
{
	clog_i("chn_id: %d\n", thiz->pipe_id);

	rtsp_server_cnt--;
	if (rtsp_server_cnt == 0) {
		CVI_RTSP_Stop(prtsp_server_ctx);
		CVI_RTSP_Destroy(&prtsp_server_ctx);
	}

	if (thiz->module_cfg) {
		free(thiz->module_cfg);
		thiz->module_cfg = NULL;
	}

	if (thiz->module_ctx) {
		free(thiz->module_ctx);
		thiz->module_ctx = NULL;
	}

	return 0;
}

static void rtsp_play(int references, void *arg)
{
	clog_i("rtsp play, references: %d\n", references);
	int *session_play_enable = (int *)arg;
	*session_play_enable = 1;
}

static void rtsp_teardown(int references, void *arg)
{
	clog_i("rtsp teardown, references: %d\n", references);
	int *session_play_enable = (int *)arg;
	*session_play_enable = 0;
}

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);
	module_rtsp_cfg_t *cfg = (module_rtsp_cfg_t *)thiz->module_cfg;
	int chn = thiz->pipe_id;

	clog_i("run, chn_id: %d\n", chn);
	prctl(PR_SET_NAME, "rtsp", 0, 0, 0);

	CVI_RTSP_SESSION *session;
	CVI_RTSP_SESSION_ATTR attr;
	CVI_RTSP_VIDEO_CODEC codec = RTSP_VIDEO_H264;
	int session_play_enable = 0;
	int find_key_frame = 0;

	if (strcmp(cfg->codec, "264") == 0) {
		codec = RTSP_VIDEO_H264;
	} else if (strcmp(cfg->codec, "265") == 0) {
		codec = RTSP_VIDEO_H265;
	} else if (strcmp(cfg->codec, "mjpeg") == 0) {
		codec = RTSP_VIDEO_JPEG;
	} else {
		clog_e("codec: %s not support!\n",
		       cfg->codec);
		return NULL;
	}

	memset(&attr, 0, sizeof(CVI_RTSP_SESSION_ATTR));
	attr.video.codec = codec;
	attr.video.bitrate = cfg->bitrate * 2; // !!!
	attr.video.play = rtsp_play;
	attr.video.playArg = &session_play_enable;
	attr.video.teardown = rtsp_teardown;
	attr.video.teardownArg = &session_play_enable;
	sprintf(attr.name, "stream%d", chn);

	ret = CVI_RTSP_CreateSession(prtsp_server_ctx, &attr, &session);
	if (ret != 0) {
		clog_e("CVI_RTSP_CreateSession failed with %#x\n", ret);
		return (void *)-1;
	}

	VENC_STREAM_S *pstream = NULL;
	VENC_PACK_S *ppack = NULL;
	CVI_RTSP_DATA data;

	while (thiz->thread_run) {
		if (pstream != NULL) {
			src_module->fun.put(src_module, pstream);
			pstream = NULL;
		}

		ret = src_module->fun.get(src_module, (void **)&pstream);
		if (ret != 0) {
			clog_e("src module get fail...\n");
			continue;
		}

		if (session_play_enable == 0) {
			find_key_frame = 0;
			continue;
		}

		if (pstream->u32PackCount >= CVI_RTSP_DATA_MAX_BLOCK) {
			clog_e("out of max block count, %d\n",
			       pstream->u32PackCount);
			continue;
		}

		if (find_key_frame == 0) {
			for (CVI_U32 i = 0; i < pstream->u32PackCount; i++) {
				ppack = &pstream->pstPack[i];
				if (codec == RTSP_VIDEO_H264) {
					if (ppack->DataType.enH264EType > H264E_NALU_PSLICE) {
						find_key_frame = 1;
					} else {
						continue;
					}
				} else if (codec == RTSP_VIDEO_H265) {
					if (ppack->DataType.enH265EType > H265E_NALU_PSLICE) {
						find_key_frame = 1;
					} else {
						continue;
					}
				} else {
					find_key_frame = 1;
				}
			}
		}

		memset(&data, 0, sizeof(CVI_RTSP_DATA));

		data.blockCnt = pstream->u32PackCount;
		for (CVI_U32 i = 0; i < pstream->u32PackCount; i++) {
			ppack = &pstream->pstPack[i];
			data.dataPtr[i] = ppack->pu8Addr + ppack->u32Offset;
			data.dataLen[i] = ppack->u32Len - ppack->u32Offset;
		}

		ret = CVI_RTSP_WriteFrame(prtsp_server_ctx, session->video,
					  &data);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_RTSP_WriteFrame failed, ret = %d\n", ret);
			continue;
		}
	}

	if (pstream != NULL) {
		src_module->fun.put(src_module, pstream);
		pstream = NULL;
	}

	ret = CVI_RTSP_DestroySession(prtsp_server_ctx, session);
	if (ret != 0) {
		clog_e("CVI_RTSP_DestroySession failed with %#x\n", ret);
	}

	return NULL;
}

static int start(struct module_t *thiz)
{
	clog_i("chn_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 1;
	pthread_create(&thiz->thread_id, NULL, worker, thiz);
	return 0;
}

static int stop(struct module_t *thiz)
{
	clog_i("chn_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 0;
	pthread_join(thiz->thread_id, NULL);
	return 0;
}

struct module_fun_t cvi_rtsp_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = NULL,
	.put = NULL,
};
