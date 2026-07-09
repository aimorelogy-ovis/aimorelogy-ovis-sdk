
#include <sys/prctl.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "vpss"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"

#define VPSS_QUEUE_SIZE (1)

typedef struct {
	VB_POOL chn_vb_pool[VPSS_MAX_CHN_NUM];
} vpss_ctx_t;

static int init(struct module_t *thiz)
{
	int ret = 0;
	module_vpss_cfg_t *vpss_cfg = (module_vpss_cfg_t *)thiz->module_cfg;

	if (vpss_cfg->grp_id >= 0) {
		clog_i("use assigned vpss grp id: %d\n", vpss_cfg->grp_id);
	} else {
		vpss_cfg->grp_id = CVI_VPSS_GetAvailableGrp();
		if (vpss_cfg->grp_id < 0) {
			clog_e("CVI_VPSS_GetAvailableGrp failed with %#x\n",
			       vpss_cfg->grp_id);
			return vpss_cfg->grp_id;
		}
		clog_i("auto assign vpss grp id: %d\n", vpss_cfg->grp_id);
	}

	module_queue_init(&thiz->queue, VPSS_QUEUE_SIZE);

	vpss_ctx_t *ctx = (vpss_ctx_t *)calloc(1, sizeof(vpss_ctx_t));

	if (ctx == NULL) {
		clog_e("calloc failed\n");
		return -1;
	}

	thiz->module_ctx = ctx;

	clog_i("vpss grp %d attr: maxw=%u, maxh=%u, pixfmt=%d\n",
		vpss_cfg->grp_id,
		vpss_cfg->st_vpss_grp_attr.u32MaxW,
		vpss_cfg->st_vpss_grp_attr.u32MaxH,
		vpss_cfg->st_vpss_grp_attr.enPixelFormat);

	ret = CVI_VPSS_CreateGrp(vpss_cfg->grp_id, &vpss_cfg->st_vpss_grp_attr);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VPSS_CreateGrp: %d failed with %#x\n", vpss_cfg->grp_id, ret);
		return ret;
	}

	if (vpss_cfg->chn_num == 0 || vpss_cfg->chn_num > VPSS_MAX_CHN_NUM) {
		clog_e("vpss chn num invalid: %d\n", vpss_cfg->chn_num);
		return -1;
	}

	// alloc vb pool
	for (int vpss_chn = 0; vpss_chn < vpss_cfg->chn_num; vpss_chn++) {
		if (vpss_cfg->chn_vb_cnt[vpss_chn] == 0) {
			clog_i("vpss grp %d chn %d use vi vb, no need create vpss vb pool\n",
				vpss_cfg->grp_id, vpss_chn);
			ctx->chn_vb_pool[vpss_chn] = VB_INVALID_POOLID;
			continue;
		}

		VB_POOL_CONFIG_S vb_pool_cfg;

		memset(&vb_pool_cfg, 0, sizeof(VB_POOL_CONFIG_S));
		vb_pool_cfg.u32BlkSize = COMMON_GetPicBufferSize(
			vpss_cfg->st_vpss_chn_attr[vpss_chn].u32Width,
			vpss_cfg->st_vpss_chn_attr[vpss_chn].u32Height,
			vpss_cfg->st_vpss_chn_attr[vpss_chn].enPixelFormat,
			DATA_BITWIDTH_8,
			COMPRESS_MODE_NONE,
			DEFAULT_ALIGN);
		vb_pool_cfg.u32BlkCnt = vpss_cfg->chn_vb_cnt[vpss_chn];
		vb_pool_cfg.enRemapMode = VB_REMAP_MODE_CACHED;
		snprintf(vb_pool_cfg.acName, MAX_VB_POOL_NAME_LEN,
			 "VPSS_GRP_%d_CHN_%d", vpss_cfg->grp_id, vpss_chn);
		clog_i("vpss grp %d chn %d vb_pool blksize=%u, cnt=%u\n",
			vpss_cfg->grp_id, vpss_chn, vb_pool_cfg.u32BlkSize, vb_pool_cfg.u32BlkCnt);
		ctx->chn_vb_pool[vpss_chn] = CVI_VB_CreatePool(&vb_pool_cfg);
		if (ctx->chn_vb_pool[vpss_chn] == VB_INVALID_POOLID) {
			clog_e("CVI_VB_CreatePool failed for chn %d\n", vpss_chn);
		}
	}

	for (int vpss_chn = 0; vpss_chn < vpss_cfg->chn_num; vpss_chn++) {
		ret = CVI_VPSS_SetChnAttr(vpss_cfg->grp_id, vpss_chn,
					  &vpss_cfg->st_vpss_chn_attr[vpss_chn]);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_VPSS_SetChnAttr: %d failed with %#x\n", vpss_chn, ret);
			return ret;
		}

		if (ctx->chn_vb_pool[vpss_chn] != VB_INVALID_POOLID) {
			ret = CVI_VPSS_AttachVbPool(vpss_cfg->grp_id, vpss_chn,
							      ctx->chn_vb_pool[vpss_chn]);
			if (ret != CVI_SUCCESS) {
				clog_e("CVI_VPSS_AttachVbPool: %d failed with %#x\n", vpss_chn, ret);
				return ret;
			}
		}

		ret = CVI_VPSS_EnableChn(vpss_cfg->grp_id, vpss_chn);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_VPSS_EnableChn: %d failed with %#x\n", vpss_chn, ret);
			return ret;
		}
	}

	ret = CVI_VPSS_StartGrp(vpss_cfg->grp_id);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VPSS_StartGrp: %d failed with %#x\n", vpss_cfg->grp_id, ret);
		return ret;
	}

	return ret;
}

static int deinit(struct module_t *thiz)
{
	int ret = 0;
	vpss_ctx_t *ctx = (vpss_ctx_t *)thiz->module_ctx;
	module_vpss_cfg_t *vpss_cfg = (module_vpss_cfg_t *)thiz->module_cfg;

	for (int vpss_chn = 0; vpss_chn < vpss_cfg->chn_num; vpss_chn++) {

		if (ctx->chn_vb_pool[vpss_chn] != VB_INVALID_POOLID) {
			ret = CVI_VPSS_DetachVbPool(vpss_cfg->grp_id, vpss_chn);
			if (ret != CVI_SUCCESS) {
				clog_e("CVI_VPSS_DetachVbPool: %d failed with %#x\n", vpss_chn, ret);
				//return ret;
			}
		}

		ret = CVI_VPSS_DisableChn(vpss_cfg->grp_id, vpss_chn);
		if (ret != CVI_SUCCESS) {
			clog_e("CVI_VPSS_DisableChn: %d failed with %#x\n", vpss_chn, ret);
			//return ret;
		}

		if (ctx->chn_vb_pool[vpss_chn] != VB_INVALID_POOLID) {
			CVI_VB_DestroyPool(ctx->chn_vb_pool[vpss_chn]);
			ctx->chn_vb_pool[vpss_chn] = VB_INVALID_POOLID;
		}
	}

	ret = CVI_VPSS_StopGrp(vpss_cfg->grp_id);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VPSS_StopGrp: %d failed with %#x\n", vpss_cfg->grp_id, ret);
		//return ret;
	}

	ret = CVI_VPSS_DestroyGrp(vpss_cfg->grp_id);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VPSS_DestroyGrp: %d failed with %#x\n", vpss_cfg->grp_id, ret);
		//return ret;
	}

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

static void *vpss_online_worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	module_vpss_cfg_t *vpss_cfg = (module_vpss_cfg_t *)thiz->module_cfg;

	clog_i("run vpss online, grp_id: %d\n", vpss_cfg->grp_id);
	prctl(PR_SET_NAME, "vpss_online", 0, 0, 0);

	while (thiz->thread_run) {
		VIDEO_FRAME_INFO_S *pframe =
			calloc(1, sizeof(VIDEO_FRAME_INFO_S));
		if (pframe == NULL) {
			clog_e("calloc failed\n");
			return (void *)-1;
		}

		ret = CVI_VPSS_GetChnFrame(vpss_cfg->grp_id, vpss_cfg->chn_id,
					   pframe, DAEMON_TIMEOUT_MS);
		if (ret != CVI_SUCCESS) {
			clog_e("get vpss chn frame failed with %#x\n", ret);
			free(pframe);
			continue;
		}

		ret = module_queue_push(&thiz->queue, pframe,
					DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_e("module_queue_push failed with %#x\n", ret);
			CVI_VPSS_ReleaseChnFrame(vpss_cfg->grp_id, vpss_cfg->chn_id,
					 pframe);
			free(pframe);
			continue;
		}
	}

	while (module_queue_size(&thiz->queue) > 0) {
		VIDEO_FRAME_INFO_S *pframe = NULL;

		module_queue_pop(&thiz->queue, (void **)&pframe,
				 DAEMON_TIMEOUT_MS);
		CVI_VPSS_ReleaseChnFrame(vpss_cfg->grp_id, vpss_cfg->chn_id,
					 pframe);
		free(pframe);
	}

	return NULL;
}

static void *vpss_offline_worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);
	module_vpss_cfg_t *vpss_cfg = (module_vpss_cfg_t *)thiz->module_cfg;

	clog_i("run vpss offline, grp_id: %d\n", vpss_cfg->grp_id);
	prctl(PR_SET_NAME, "vpss_offline", 0, 0, 0);

	VIDEO_FRAME_INFO_S *src_frame = NULL;
	VIDEO_FRAME_INFO_S *dst_frame = NULL;

	while (thiz->thread_run) {
		if (src_frame != NULL) {
			src_module->fun.put(src_module, src_frame);
			src_frame = NULL;
		}

		ret = src_module->fun.get(src_module, (void **)&src_frame);
		if (ret != 0) {
			clog_e("src module get fail...\n");
			continue;
		}

		ret = CVI_VPSS_SendFrame(vpss_cfg->grp_id, src_frame,
					 DAEMON_TIMEOUT_MS);
		if (ret != CVI_SUCCESS) {
			clog_e("vpss grp %d, send frame fail...\n",
			       vpss_cfg->grp_id);
			continue;
		}

		if (vpss_cfg->enable_bypass_src_frame) {
			ret = module_queue_push(&thiz->queue, src_frame,
						DAEMON_TIMEOUT_MS);
			if (ret != 0) {
				clog_e("module_queue_push failed with %#x\n", ret);
				src_module->fun.put(src_module, src_frame);
				continue;
			}
			src_frame = NULL;
			continue;
		}

		dst_frame = (VIDEO_FRAME_INFO_S *)calloc(
			1, sizeof(VIDEO_FRAME_INFO_S));
		if (dst_frame == NULL) {
			clog_e("calloc failed\n");
			continue;
		}

		ret = CVI_VPSS_GetChnFrame(vpss_cfg->grp_id, vpss_cfg->chn_id,
					   dst_frame, DAEMON_TIMEOUT_MS);
		if (ret != CVI_SUCCESS) {
			clog_e("vpss grp %d, chn %d, get frame fail...\n",
			       vpss_cfg->grp_id, vpss_cfg->chn_id);
			continue;
		}

		ret = module_queue_push(&thiz->queue, dst_frame,
					DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_e("module_queue_push failed with %#x\n", ret);
			CVI_VPSS_ReleaseChnFrame(vpss_cfg->grp_id, vpss_cfg->chn_id,
						 dst_frame);
			continue;
		}
	}

	while (module_queue_size(&thiz->queue) > 0) {
		VIDEO_FRAME_INFO_S *dst_frame = NULL;

		module_queue_pop(&thiz->queue, (void **)&dst_frame,
				 DAEMON_TIMEOUT_MS);
		if (vpss_cfg->enable_bypass_src_frame) {
			src_module->fun.put(src_module, dst_frame);
			continue;
		}
		CVI_VPSS_ReleaseChnFrame(vpss_cfg->grp_id, vpss_cfg->chn_id,
					 dst_frame);
		free(dst_frame);
	}

	return NULL;
}

static int start(struct module_t *thiz)
{
	module_vpss_cfg_t *vpss_cfg = (module_vpss_cfg_t *)thiz->module_cfg;

	clog_i("vpss grp_id: %d\n", vpss_cfg->grp_id);
	thiz->thread_run = 1;
	if (vpss_cfg->enable_vi_vpss_online) {
		pthread_create(&thiz->thread_id, NULL, vpss_online_worker, thiz);
	} else {
		pthread_create(&thiz->thread_id, NULL, vpss_offline_worker, thiz);
	}
	return 0;
}

static int stop(struct module_t *thiz)
{
	module_vpss_cfg_t *vpss_cfg = (module_vpss_cfg_t *)thiz->module_cfg;

	clog_i("vpss grp_id: %d\n", vpss_cfg->grp_id);
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
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);
	module_vpss_cfg_t *vpss_cfg = (module_vpss_cfg_t *)thiz->module_cfg;

	if (vpss_cfg->enable_bypass_src_frame) {
		return src_module->fun.put(src_module, data);
	}

	ret = CVI_VPSS_ReleaseChnFrame(vpss_cfg->grp_id, vpss_cfg->chn_id,
				       (VIDEO_FRAME_INFO_S *)data);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_VPSS_ReleaseChnFrame failed with %#x\n", ret);
		return -1;
	}
	free(data);
	return 0;
}

struct module_fun_t vpss_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
