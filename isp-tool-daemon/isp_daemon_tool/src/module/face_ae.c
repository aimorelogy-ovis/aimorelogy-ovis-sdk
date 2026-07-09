
#ifdef ENABLE_FACE_AE
#include <sys/prctl.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "face_ae"

#include <math.h>
#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "tdl_sdk_api.h"
#include "cvi_isp.h"

#define FACE_AE_QUEUE_SIZE (1)

typedef struct {
	tdl_sdk_api_t *api;
	TDLHandle handle;
} face_ae_ctx_t;

static int init(struct module_t *thiz)
{
	int ret = 0;

	clog_i("pipe_id: %d\n", thiz->pipe_id);
	module_queue_init(&thiz->queue, FACE_AE_QUEUE_SIZE);

	module_face_ae_cfg_t *cfg = (module_face_ae_cfg_t *)thiz->module_cfg;

	load_tdl_sdk_lib();

	face_ae_ctx_t *ctx = (face_ae_ctx_t *)malloc(sizeof(face_ae_ctx_t));

	if (ctx == NULL) {
		clog_e("malloc face_ae_ctx_t failed\n");
		return -1;
	}
	thiz->module_ctx = ctx;
	ctx->api = get_tdl_sdk_api();

	ctx->handle = ctx->api->create_handle(0);
	if (ctx->handle == NULL) {
		clog_e("Create face_ae handle failed!\n");
		return -1;
	}

	ret = ctx->api->open_model(ctx->handle,
				   TDL_SUPPORTED_MODEL_FACE,
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
	face_ae_ctx_t *ctx = (face_ae_ctx_t *)thiz->module_ctx;

	ctx->api->close_model(ctx->handle, TDL_SUPPORTED_MODEL_FACE);
	ctx->api->destroy_handle(ctx->handle);

	unload_tdl_sdk_lib();
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

static void update_face_ae(TDLFace *face, CVI_U32 width, CVI_U32 height,
			   VI_PIPE ViPipe)
{
	ISP_SMART_INFO_S stFaceInfo;

	memset(&stFaceInfo, 0, sizeof(ISP_SMART_INFO_S));
	stFaceInfo.stROI[0].bEnable = 1;
	stFaceInfo.stROI[0].bAvailable = 1;
	if (face->size > 0) {
		float max_area = 0.0;
		size_t max_idx = 0;

		for (size_t i = 0; i < face->size; i++) {
			TDLBox bbox = face->info[i].box;
			float area = fabs(bbox.x2 - bbox.x1) *
				     fabs(bbox.y2 - bbox.y1);
			if (area > max_area) {
				max_area = area;
				max_idx = i;
			}
		}
		TDLBox max_bbox = face->info[max_idx].box;

		stFaceInfo.stROI[0].u8Num = 1;
		stFaceInfo.stROI[0].u16PosX[0] = max_bbox.x1;
		stFaceInfo.stROI[0].u16PosY[0] = max_bbox.y1;
		stFaceInfo.stROI[0].u16Width[0] =
			fabs(max_bbox.x2 - max_bbox.x1);
		stFaceInfo.stROI[0].u16Height[0] =
			fabs(max_bbox.y2 - max_bbox.y1);
		stFaceInfo.stROI[0].u16FrameWidth = width;
		stFaceInfo.stROI[0].u16FrameHeight = height;
		if (getenv("FACE_AE_DEBUG")) {
			clog_i("CVI_ISP_SetFaceAeInfo, ViPipe:%d, u8FDNum:%d\n",
			       ViPipe, stFaceInfo.stROI[0].u8Num);
			clog_i("posx: %d, posy: %d, width: %d, height: %d\n",
			       stFaceInfo.stROI[0].u16PosX[0],
			       stFaceInfo.stROI[0].u16PosY[0],
			       stFaceInfo.stROI[0].u16Width[0],
			       stFaceInfo.stROI[0].u16Height[0]);
		}
		CVI_ISP_SetSmartInfo(ViPipe, &stFaceInfo, 0);
	} else {
		CVI_ISP_SetSmartInfo(ViPipe, &stFaceInfo, 0);
	}
}

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);

	clog_i("run, pipe_id: %d\n", thiz->pipe_id);
	prctl(PR_SET_NAME, "face_ae", 0, 0, 0);

	face_ae_ctx_t *ctx = (face_ae_ctx_t *)thiz->module_ctx;

	VIDEO_FRAME_INFO_S *src_frame = NULL;

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

		TDLFace face_meta;
		TDLBrush brush = {0};

		brush.color.b = 53.f;
		brush.color.g = 208.f;
		brush.color.r = 217.f;
		brush.size = 4;

		TDLImage vpss_image = ctx->api->wrap_vpss_frame(src_frame, false);

		memset(&face_meta, 0, sizeof(TDLFace));
		ret = ctx->api->face_detection(
					ctx->handle, TDL_SUPPORTED_MODEL_FACE,
					vpss_image, &face_meta);
		if (ret != 0) {
			clog_e("face_detection failed, ret: %d\n", ret);
			continue;
		}

		if (face_meta.size > 0) {
			ctx->api->service_face_draw_rect(&face_meta, (void *)src_frame, true, brush);
		}

		update_face_ae(&face_meta, src_frame->stVFrame.u32Width,
						src_frame->stVFrame.u32Height, thiz->pipe_id);

		ctx->api->free_face_meta(&face_meta);
		ctx->api->free_vpss_frame(vpss_image);

		ret = module_queue_push(&thiz->queue, src_frame,
					DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_e("module_queue_push failed with %#x\n", ret);
			continue;
		}
		src_frame = NULL;
	}

	while (module_queue_size(&thiz->queue) > 0) {
		VIDEO_FRAME_INFO_S *src_frame = NULL;

		module_queue_pop(&thiz->queue, (void **)&src_frame,
				 DAEMON_TIMEOUT_MS);
		src_module->fun.put(src_module, src_frame);
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

	ret = src_module->fun.put(src_module, data);
	if (ret != 0) {
		clog_e("src module put fail..., ret: %d\n", ret);
		return -1;
	}

	return 0;
}

struct module_fun_t face_ae_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
#endif