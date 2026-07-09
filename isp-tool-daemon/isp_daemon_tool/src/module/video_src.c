
#include <sys/prctl.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "video_src"

#include "daemon_base.h"
#include "daemon_module.h"
#include "daemon_cfg.h"
#include "daemon_utils.h"
#include "video_src_cfg.h"
#include "cvi_comm_vi.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_vpss.h"

// touch /tmp/dump_video_src
#define ENABLE_VIDEO_SRC_DUMP_DEBUG (1)
#define VIDEO_SRC_QUEUE_SIZE (1)

static int init(struct module_t *thiz)
{
	int ret = 0;
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)thiz->module_cfg;

	clog_i("pipe_id: %d\n", thiz->pipe_id);

	module_queue_init(&thiz->queue, VIDEO_SRC_QUEUE_SIZE);

	ret = module_video_src_init(thiz->pipe_id, cfg);
	return ret;
}

static int deinit(struct module_t *thiz)
{
	clog_i("pipe_id: %d\n", thiz->pipe_id);
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)thiz->module_cfg;

	module_video_src_deinit(thiz->pipe_id, cfg);
	module_queue_deinit(&thiz->queue);

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

static int get_frame(module_t *thiz, VIDEO_FRAME_INFO_S *pframe)
{
	int ret = 0;

	ret = CVI_VI_GetChnFrame(thiz->pipe_id, 0, pframe,
				   DAEMON_TIMEOUT_MS);

	if (ret != CVI_SUCCESS) {
		clog_e("get chn frame failed with %#x\n", ret);
	}

	return ret;
}

static int put_frame(module_t *thiz, VIDEO_FRAME_INFO_S *pframe)
{
	int ret = 0;

	ret = CVI_VI_ReleaseChnFrame(thiz->pipe_id, 0, pframe);

	if (ret != CVI_SUCCESS) {
		clog_e("release chn frame failed with %#x\n", ret);
	}

	return ret;
}

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;

	clog_i("run, pipe_id: %d\n", thiz->pipe_id);
	prctl(PR_SET_NAME, "video_src", 0, 0, 0);

	while (thiz->thread_run) {
		VIDEO_FRAME_INFO_S *pframe =
			calloc(1, sizeof(VIDEO_FRAME_INFO_S));
		if (pframe == NULL) {
			clog_e("calloc failed\n");
			return (void *)-1;
		}

		ret = get_frame(thiz, pframe);
		if (ret != CVI_SUCCESS) {
			clog_e("get vi chn frame failed with %#x\n", ret);
			free(pframe);
			continue;
		}

#ifdef ENABLE_VIDEO_SRC_DUMP_DEBUG
		if (access("/tmp/dump_video_src", F_OK) == 0) {
			char file_name[32];

			snprintf(file_name, sizeof(file_name),
				 "video_src_%d.yuv", thiz->pipe_id);

			FILE *fp = fopen(file_name, "wb");

			dump_video_frame(fp, pframe);
			fflush(fp);
			fclose(fp);
			clog_i("dump video src frame, %s\n", file_name);
			system("sync;rm -f /tmp/dump_video_src");
		}
#endif
		ret = module_queue_push(&thiz->queue, pframe,
					DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_e("module_queue_push failed with %#x\n", ret);
			put_frame(thiz, pframe);
			free(pframe);
			continue;
		}
	}

	while (module_queue_size(&thiz->queue) > 0) {
		VIDEO_FRAME_INFO_S *pframe = NULL;

		module_queue_pop(&thiz->queue, (void **)&pframe,
				 DAEMON_TIMEOUT_MS);
		put_frame(thiz, pframe);
		free(pframe);
	}

	return NULL;
}

static int start(struct module_t *thiz)
{
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)thiz->module_cfg;

	if (cfg->vi_vpss_mode == VI_ONLINE_VPSS_ONLINE ||
	    cfg->vi_vpss_mode == VI_OFFLINE_VPSS_ONLINE) {
		clog_i("pipe_id: %d vi vpss online mode, skip start thread\n", thiz->pipe_id);
		return 0;
	}

	clog_i("pipe_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 1;
	pthread_create(&thiz->thread_id, NULL, worker, thiz);
	return 0;
}

static int stop(struct module_t *thiz)
{
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)thiz->module_cfg;

	if (cfg->vi_vpss_mode == VI_ONLINE_VPSS_ONLINE ||
	    cfg->vi_vpss_mode == VI_OFFLINE_VPSS_ONLINE) {
		clog_i("pipe_id: %d vi vpss online mode, skip stop thread\n", thiz->pipe_id);
		return 0;
	}

	clog_i("pipe_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 0;
	pthread_join(thiz->thread_id, NULL);
	return 0;
}

static int get(struct module_t *thiz, void **data)
{
	int ret = 0;
	ret = module_queue_pop(&thiz->queue, data, DAEMON_TIMEOUT_MS);
	if (ret != 0) {
		clog_e("module_queue_pop failed with %#x\n", ret);
		return -1;
	}
	return 0;
}

static int put(struct module_t *thiz, void *data)
{
	put_frame(thiz, (VIDEO_FRAME_INFO_S *)data);
	free(data);
	return 0;
}

struct module_fun_t video_src_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
