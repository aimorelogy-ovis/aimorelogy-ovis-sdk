#include <sys/prctl.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "vpss_chn_src"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"

#define VPSS_CHN_QUEUE_SIZE (1)

static int init(struct module_t *thiz)
{
	int ret = 0;
	module_vpss_chn_cfg_t *cfg = (module_vpss_chn_cfg_t *)thiz->module_cfg;

	clog_i("grp: %d, chn: %d\n", cfg->src_grp_id, cfg->chn_id);
	module_queue_init(&thiz->queue, VPSS_CHN_QUEUE_SIZE);

	return ret;
}

static int deinit(struct module_t *thiz)
{
	int ret = 0;
	module_vpss_chn_cfg_t *cfg = (module_vpss_chn_cfg_t *)thiz->module_cfg;

	clog_i("grp: %d, chn: %d\n", cfg->src_grp_id, cfg->chn_id);

	if (thiz->module_cfg != NULL) {
		free(thiz->module_cfg);
		thiz->module_cfg = NULL;
	}
	if (thiz->module_ctx != NULL) {
		free(thiz->module_ctx);
		thiz->module_ctx = NULL;
	}
	module_queue_deinit(&thiz->queue);
	return ret;
}

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	module_vpss_chn_cfg_t *cfg = (module_vpss_chn_cfg_t *)thiz->module_cfg;

	clog_i("grp: %d, chn: %d\n", cfg->src_grp_id, cfg->chn_id);
	prctl(PR_SET_NAME, "vpss_chn_src", 0, 0, 0);

	VIDEO_FRAME_INFO_S *dst_frame = NULL;

	while (thiz->thread_run) {
		dst_frame = (VIDEO_FRAME_INFO_S *)calloc(1, sizeof(VIDEO_FRAME_INFO_S));
		if (dst_frame == NULL) {
			clog_e("calloc failed\n");
			usleep(10000); // sleep 10ms to avoid busy loop
			continue;
		}

		ret = CVI_VPSS_GetChnFrame(cfg->src_grp_id, cfg->chn_id,
					   dst_frame, DAEMON_TIMEOUT_MS);
		if (ret != CVI_SUCCESS) {
			clog_e("vpss grp %d, chn %d, get frame fail: %#x\n",
			       cfg->src_grp_id, cfg->chn_id, ret);
			free(dst_frame);
			continue;
		}

		ret = module_queue_push(&thiz->queue, dst_frame, DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_e("module_queue_push failed with %#x\n", ret);
			CVI_VPSS_ReleaseChnFrame(cfg->src_grp_id, cfg->chn_id, dst_frame);
			free(dst_frame);
			continue;
		}
	}

	while (module_queue_size(&thiz->queue) > 0) {
		VIDEO_FRAME_INFO_S *frame = NULL;

		module_queue_pop(&thiz->queue, (void **)&frame, DAEMON_TIMEOUT_MS);
		if (frame != NULL) {
			CVI_VPSS_ReleaseChnFrame(cfg->src_grp_id, cfg->chn_id, frame);
			free(frame);
		}
	}

	clog_i("exit, grp: %d, chn: %d\n", cfg->src_grp_id, cfg->chn_id);
	return NULL;
}

static int start(struct module_t *thiz)
{
	module_vpss_chn_cfg_t *cfg = (module_vpss_chn_cfg_t *)thiz->module_cfg;

	clog_i("grp: %d, chn: %d\n", cfg->src_grp_id, cfg->chn_id);
	thiz->thread_run = 1;
	pthread_create(&thiz->thread_id, NULL, worker, thiz);
	return 0;
}

static int stop(struct module_t *thiz)
{
	module_vpss_chn_cfg_t *cfg = (module_vpss_chn_cfg_t *)thiz->module_cfg;

	clog_i("grp: %d, chn: %d\n", cfg->src_grp_id, cfg->chn_id);
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
	int ret = 0;
	module_vpss_chn_cfg_t *cfg = (module_vpss_chn_cfg_t *)thiz->module_cfg;

	ret = CVI_VPSS_ReleaseChnFrame(cfg->src_grp_id, cfg->chn_id,
				       (VIDEO_FRAME_INFO_S *)data);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VPSS_ReleaseChnFrame: %d, %d failed with %#x\n",
			cfg->src_grp_id, cfg->chn_id, ret);
		return -1;
	}
	free(data);
	return 0;
}

struct module_fun_t vpss_chn_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
