
#ifdef ENABLE_TEAISP_PQ
#include <sys/prctl.h>
#include "cvi_sys.h"

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "teaisp_pq"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "tdl_sdk_api.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_awb.h"

#define TEAISP_PQ_SCENE_MAX_TYPE (5)
static const char *teaisppq_scene_str[TEAISP_PQ_SCENE_MAX_TYPE] = {
	"SNOW", "FOG", "BACKLIGHT", "GRASS", "COMMON"
};

typedef struct {
	tdl_sdk_api_t *api;
	TDLHandle handle;
} teaisp_pq_ctx_t;

static int init(struct module_t *thiz)
{
	int ret = 0;

	clog_i("pipe_id: %d\n", thiz->pipe_id);

	module_teaisp_pq_cfg_t *cfg =
		(module_teaisp_pq_cfg_t *)thiz->module_cfg;

	load_tdl_sdk_lib();

	teaisp_pq_ctx_t *ctx =
		(teaisp_pq_ctx_t *)malloc(sizeof(teaisp_pq_ctx_t));

	if (ctx == NULL) {
		clog_e("malloc teaisp_pq_ctx_t failed\n");
		return -1;
	}
	thiz->module_ctx = ctx;
	ctx->api = get_tdl_sdk_api();

	ctx->handle = ctx->api->create_handle(0);
	if (ctx->handle == NULL) {
		clog_e("Create teaisppq handle failed!\n");
		return -1;
	}

	ret = ctx->api->open_model(ctx->handle,
		TDL_SUPPORTED_MODEL_CLASSIFICATION,
		cfg->model_path,
		NULL);
	if (ret != 0) {
		clog_e("open_model failed, ret: %d, model: %s\n", ret,
		       cfg->model_path);
		return -1;
	}

	return 0;
}

static int deinit(struct module_t *thiz)
{
	teaisp_pq_ctx_t *ctx = (teaisp_pq_ctx_t *)thiz->module_ctx;

	ctx->api->close_model(ctx->handle, TDL_SUPPORTED_MODEL_CLASSIFICATION);
	ctx->api->destroy_handle(ctx->handle);
	unload_tdl_sdk_lib();
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

static int get_vi_raw(int pipe, VIDEO_FRAME_INFO_S frame[])
{
	int ret = 0;
	VI_DUMP_ATTR_S attr;

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;
	ret = CVI_VI_SetPipeDumpAttr(pipe, &attr);
	if (ret != CVI_SUCCESS) {
		clog_e("set vi dump attribute failed, ret: %d\n", ret);
		return ret;
	}

	ret = CVI_VI_GetPipeFrame(pipe, frame, DAEMON_TIMEOUT_MS);
	if (ret != CVI_SUCCESS) {
		clog_e("get vi frame failed, ret: %d\n", ret);
		return ret;
	}

	return 0;
}

static int put_vi_raw(int pipe, VIDEO_FRAME_INFO_S frame[])
{
	int ret = 0;

	ret = CVI_VI_ReleasePipeFrame(pipe, frame);
	if (ret != CVI_SUCCESS) {
		clog_e("release vi frame failed, ret: %d\n", ret);
		return ret;
	}

	return 0;
}

static int update_isp_meta(int pipe, TDLIspMeta *isp_meta)
{
	int ret = 0;
	ISP_WB_Q_INFO_S awb_info;
	ISP_INNER_STATE_INFO_S inner_state_info;

	ret = CVI_AWB_QueryInfo(pipe, &awb_info);
	if (ret != 0) {
		clog_e("query awb info failed, ret: %d\n", ret);
		return ret;
	}

	ret = CVI_ISP_QueryInnerStateInfo(pipe, &inner_state_info);
	if (ret != 0) {
		clog_e("query inner state info failed, ret: %d\n", ret);
		return ret;
	}

	isp_meta->awb[0] = awb_info.u16Rgain;
	isp_meta->awb[1] = awb_info.u16Grgain;
	isp_meta->awb[2] = awb_info.u16Bgain;

	for (int i = 0; i < 9; i++) {
		isp_meta->ccm[i] = inner_state_info.ccm[i];
	}

	isp_meta->blc = inner_state_info.blcOffsetR;
	isp_meta->blc += inner_state_info.blcOffsetGr;
	isp_meta->blc += inner_state_info.blcOffsetGb;
	isp_meta->blc += inner_state_info.blcOffsetB;
	isp_meta->blc /= 4;

	return 0;
}

static int update_isp_scene(int pipe, TDLClass *cls_meta)
{
#if 1 // TODO
	UNUSED(pipe);
	UNUSED(cls_meta);
	return 0;
#else
	TEAISP_PQ_SCENE_INFO scene_info;

	switch (cls_meta.info[0].class_id) {
	case 0:
		scene_info.scene = SCENE_SNOW;
		break;
	case 1:
		scene_info.scene = SCENE_FOG;
		break;
	case 2:
		scene_info.scene = SCENE_BACKLIGHT;
		break;
	case 3:
		scene_info.scene = SCENE_GRASS;
		break;
	default:
		scene_info.scene = SCENE_COMMON;
		break;
	}

	float score = cls_meta.info[0].score;

	scene_info.scene_score = score * 100;
	CVI_TEAISP_PQ_SetSceneInfo(pipe, &scene_info);
	return 0;
#endif
}

static void teaisppq_put_text(int pipe, teaisp_pq_ctx_t *ctx,
			      VIDEO_FRAME_INFO_S *pstVideoFrame)
{
#if 1 // TODO
	UNUSED(pipe);
	UNUSED(ctx);
	UNUSED(pstVideoFrame);
#else
	char scene_text[128];
	int pos_x, pos_y;
	float c_r, c_g, c_b;

	pos_x = 50;
	pos_y = 100;
	c_r = 255;
	c_g = 0;
	c_b = 255;

	TEAISP_PQ_ATTR_S stTEAISPPQAttr;
	TEAISP_PQ_SCENE_INFO scene_info;
	int scene_id;

	CVI_TEAISP_PQ_GetAttr(pipe, &stTEAISPPQAttr);
	CVI_TEAISP_PQ_GetDetectSceneInfo(pipe, &scene_info);

	if (stTEAISPPQAttr.TuningMode != 0) {
		scene_info.scene_score = 100;
		scene_id = (stTEAISPPQAttr.TuningMode - 1) % TEAISP_SCENE_NUM;
		snprintf(scene_text, sizeof(scene_text),
			 "Tuning SCENE: %s, score: 100",
			 teaisppq_scene_str[scene_id]);
	} else {
		scene_id = scene_info.scene;
		snprintf(scene_text, sizeof(scene_text),
			 "Detect SCENE: %s, score: %d",
			 teaisppq_scene_str[scene_id], scene_info.scene_score);
	}

	if (stTEAISPPQAttr.Enable &&
	    stTEAISPPQAttr.SceneBypass[scene_id] == 0 &&
	    (scene_info.scene_score >=
	     stTEAISPPQAttr.SceneConfThres[scene_id])) {
		strncat(scene_text, " (accpet)",
			sizeof(scene_text) - strlen(scene_text) - 1);
	} else {
		strncat(scene_text, " (bypass)",
			sizeof(scene_text) - strlen(scene_text) - 1);
		if (!stTEAISPPQAttr.Enable) {
			strncat(scene_text, " [disable]",
				sizeof(scene_text) - strlen(scene_text) - 1);
		} else if (stTEAISPPQAttr.SceneBypass[scene_id] != 0) {
			strncat(scene_text, " [scene bypass]",
				sizeof(scene_text) - strlen(scene_text) - 1);
		} else {
			strncat(scene_text, " [Low score]",
				sizeof(scene_text) - strlen(scene_text) - 1);
		}
	}

	ctx->api->service_object_write_text(scene_text, pos_x, pos_y,
					    pstVideoFrame, c_r, c_g, c_b);
#endif
}

#define TEAISP_PQ_RUN_INTERVAL_FRAME 5

static void *worker(void *arg)
{
	int ret = 0;
	int run_interval_frame = 0;
	struct module_t *thiz = (struct module_t *)arg;
	teaisp_pq_ctx_t *ctx = (teaisp_pq_ctx_t *)thiz->module_ctx;

	clog_i("run, pipe_id: %d\n", thiz->pipe_id);
	prctl(PR_SET_NAME, "teaisp_pq", 0, 0, 0);

	TDLIspMeta isp_meta;
	TDLClass cls_meta;
	VIDEO_FRAME_INFO_S stVideoFrame[2] = {};

	while (thiz->thread_run) {
		ret = CVI_ISP_GetVDTimeOut(thiz->pipe_id, ISP_VD_BE_END,
					   DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_w("get vd timeout, go on wait...\n");
			continue;
		}
		run_interval_frame++;
		if (run_interval_frame < TEAISP_PQ_RUN_INTERVAL_FRAME) {
			continue;
		}
		run_interval_frame = 0;

		ret = update_isp_meta(thiz->pipe_id, &isp_meta);
		if (ret != 0) {
			clog_e("update isp meta failed, ret: %d\n", ret);
			continue;
		}

		memset(stVideoFrame, 0, sizeof(stVideoFrame));
		ret = get_vi_raw(thiz->pipe_id, stVideoFrame);
		if (ret != 0) {
			clog_e("get vi raw failed, ret: %d\n", ret);
			continue;
		}

		PIXEL_FORMAT_E pixelFormat = stVideoFrame[0].stVFrame.enPixelFormat;
		CVI_U32 u32Stride = stVideoFrame[0].stVFrame.u32Stride[0];

		stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_888;
		stVideoFrame[0].stVFrame.u32Width = stVideoFrame[0].stVFrame.u32Width * 3 / 2;

		stVideoFrame[0].stVFrame.u32Stride[0] = stVideoFrame[0].stVFrame.u32Width;
		stVideoFrame[0].stVFrame.pu8VirAddr[0] = CVI_SYS_MmapCache(
												stVideoFrame[0].stVFrame.u64PhyAddr[0],
												stVideoFrame[0].stVFrame.u32Length[0]);
		CVI_SYS_IonInvalidateCache(stVideoFrame[0].stVFrame.u64PhyAddr[0],
									stVideoFrame[0].stVFrame.pu8VirAddr[0],
									stVideoFrame[0].stVFrame.u32Length[0]);

		memset(&cls_meta, 0, sizeof(cls_meta));
		cls_meta.size = topK;
		cls_meta.info = (TDLClassInfo *) malloc(topK * sizeof(TDLClassInfo));
		TDLImage vpss_image = ctx->api->wrap_vpss_frame(stVideoFrame, false);
		ret = ctx->api->isp_image_classification(ctx->handle, TDL_SUPPORTED_MODEL_CLASSIFICATION,
																vpss_image, &isp_meta, &cls_meta);
		if (ret != 0) {
			clog_e("isp image classification failed, ret: %d\n", ret);
		}

		stVideoFrame[0].stVFrame.u32Stride[0] = u32Stride;
		stVideoFrame[0].stVFrame.enPixelFormat = pixelFormat;
		stVideoFrame[0].stVFrame.u32Width = stVideoFrame[0].stVFrame.u32Width * 2 / 3;
		CVI_SYS_Munmap(stVideoFrame[0].stVFrame.pu8VirAddr[0], stVideoFrame[0].stVFrame.u32Length[0]);

		if (getenv("TEAISP_PQ_DEBUG")) {
			for (int i = 0; i < TEAISP_PQ_SCENE_MAX_TYPE; i++) {
				clog_i("scene: %s, score: %f\n",
				       teaisppq_scene_str[i],
				       cls_meta.info[i].score);
			}
		}
		put_vi_raw(thiz->pipe_id, stVideoFrame);
		if (ret == 0)
			update_isp_scene(thiz->pipe_id, &cls_meta);

		ctx->api->free_vpss_frame(vpss_image);
		ctx->api->free_class_meta(&cls_meta);
	}

	return NULL;
}

static int start(struct module_t *thiz)
{
	clog_i("pipe_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 1;
	pthread_create(&thiz->thread_id, NULL, worker, thiz);
	return 0;
}

static int stop(struct module_t *thiz)
{
	clog_i("pipe_id: %d\n", thiz->pipe_id);
	thiz->thread_run = 0;
	pthread_join(thiz->thread_id, NULL);
	return 0;
}

static int get(struct module_t *thiz, void **data)
{
	int ret = 0;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);

	ret = src_module->fun.get(src_module, data);
	if (ret != 0) {
		return ret;
	}

	teaisp_pq_ctx_t *ctx = (teaisp_pq_ctx_t *)thiz->module_ctx;

	teaisppq_put_text(thiz->pipe_id, ctx, (VIDEO_FRAME_INFO_S *)*data);
	return 0;
}

static int put(struct module_t *thiz, void *data)
{
	int ret = 0;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);

	ret = src_module->fun.put(src_module, data);
	if (ret != 0) {
		return ret;
	}
	return 0;
}

struct module_fun_t teaisp_pq_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
#endif