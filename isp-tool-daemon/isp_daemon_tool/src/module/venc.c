
#include <sys/prctl.h>
#include <inttypes.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "venc"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "venc_cfg.h"
#include "cvi_isp.h"
#include "cvi_venc.h"

// touch /tmp/venc_dump
#define ENABLE_VENC_DUMP_DEBUG (1)
#define VENC_QUEUE_SIZE (3)
#define DUMP_BOOT_VIDEO_FRAME_MAX_COUNT (50)

typedef struct {
	bool enable_dump_boot_video;
	uint32_t dump_boot_video_frame_count;
	FILE *dump_boot_video_fp;
} venc_ctx_t;

static int init(struct module_t *thiz)
{
	int ret = 0;
	int chn_id = thiz->pipe_id;
	module_venc_cfg_t *venc_cfg = (module_venc_cfg_t *)thiz->module_cfg;
	venc_ctx_t *ctx = (venc_ctx_t *)calloc(1, sizeof(venc_ctx_t));

	if (!ctx) {
		clog_e("calloc failed\n");
		return -1;
	}

	thiz->module_ctx = ctx;

	clog_i("chn_id: %d, width: %d, height: %d, codec: %s\n",
			chn_id, venc_cfg->width, venc_cfg->height, venc_cfg->codec);

	module_queue_init(&thiz->queue, VENC_QUEUE_SIZE);

	ctx->enable_dump_boot_video = venc_cfg->enable_dump_boot_video;
	ret = module_venc_init(chn_id, thiz->module_cfg);
	if (ret != 0) {
		clog_a("module_venc_init: %d failed with %#x\n", chn_id, ret);
		return ret;
	}

	return 0;
}

static int deinit(struct module_t *thiz)
{
	int ret = 0;
	int chn_id = thiz->pipe_id;
	module_venc_cfg_t *venc_cfg = (module_venc_cfg_t *)thiz->module_cfg;

	clog_i("chn_id: %d, width: %d, height: %d, codec: %s\n",
			chn_id, venc_cfg->width, venc_cfg->height, venc_cfg->codec);
	ret = module_venc_deinit(chn_id);
	if (ret != CVI_SUCCESS) {
		clog_e("module_venc_deinit: %d failed with %#x\n", chn_id, ret);
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

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);
	int chn = thiz->pipe_id;

	clog_i("run, chn_id: %d\n", chn);
	prctl(PR_SET_NAME, "venc", 0, 0, 0);

	venc_ctx_t *ctx = (venc_ctx_t *)thiz->module_ctx;
	module_venc_cfg_t *venc_cfg = (module_venc_cfg_t *)thiz->module_cfg;
	VENC_CHN_STATUS_S stStat;
	VIDEO_FRAME_INFO_S *pframe = NULL;

	// for debug
	FILE *fp = NULL;

	while (thiz->thread_run) {
		if (pframe != NULL) {
			src_module->fun.put(src_module, pframe);
			pframe = NULL;
		}

		ret = src_module->fun.get(src_module, (void **)&pframe);
		if (ret != 0) {
			clog_e("src module get fail...\n");
			continue;
		}

		ret = CVI_VENC_SendFrame(chn, pframe, DAEMON_TIMEOUT_MS);
		if (ret != CVI_SUCCESS) {
			clog_e("venc chn %d, send frame fail...\n", chn);
			continue;
		}

		memset(&stStat, 0, sizeof(VENC_CHN_STATUS_S));
		ret = CVI_VENC_QueryStatus(chn, &stStat);
		if (ret != CVI_SUCCESS) {
			clog_e("venc chn: %d, query status fail...\n", chn);
			continue;
		}

		if (!stStat.u32CurPacks) {
			clog_e("venc chn: %d, u32CurPacks == 0, try again...\n",
			       chn);
			continue;
		}

		VENC_STREAM_S *pstStream =
			(VENC_STREAM_S *)calloc(1, sizeof(VENC_STREAM_S));
		if (pstStream == NULL) {
			clog_e("calloc fail...\n");
			continue;
		}

		pstStream->pstPack = (VENC_PACK_S *)calloc(stStat.u32CurPacks,
							   sizeof(VENC_PACK_S));
		if (pstStream->pstPack == NULL) {
			clog_e("calloc fail...\n");
			free(pstStream);
			continue;
		}

		ret = CVI_VENC_GetStream(chn, pstStream, DAEMON_TIMEOUT_MS);
		if (ret != CVI_SUCCESS) {
			clog_e("venc chn: %d, get stream fail...\n", chn);
			free(pstStream->pstPack);
			free(pstStream);
			continue;
		}

		if (ctx->enable_dump_boot_video &&
		    ctx->dump_boot_video_frame_count < DUMP_BOOT_VIDEO_FRAME_MAX_COUNT) {
			clog_i("dump boot video frame %d, pts: %" PRIu64 "\n",
			       pframe->stVFrame.u32TimeRef,
			       pframe->stVFrame.u64PTS);
			if (ctx->dump_boot_video_fp == NULL) {
				char dump_file_name[32] = { 0 };

				snprintf(dump_file_name, sizeof(dump_file_name),
					 "boot_video_%d_%" PRIu64 ".%s", chn, pframe->stVFrame.u64PTS,
					 strcmp(venc_cfg->codec,
						"264") == 0 ? "h264" : "h265");
				ctx->dump_boot_video_fp = fopen(dump_file_name, "wb");
				if (ctx->dump_boot_video_fp == NULL) {
					clog_e("fopen %s failed\n", dump_file_name);
					ctx->enable_dump_boot_video = false;
				} else {
					clog_i("start dump boot video to %s\n",
					       dump_file_name);
				}
			}
			if (ctx->dump_boot_video_fp != NULL) {
				for (uint32_t i = 0; i < pstStream->u32PackCount; i++) {
					VENC_PACK_S *ppack = &pstStream->pstPack[i];

					fwrite(ppack->pu8Addr + ppack->u32Offset,
					       ppack->u32Len - ppack->u32Offset, 1, ctx->dump_boot_video_fp);
				}
				ctx->dump_boot_video_frame_count++;
				if (ctx->dump_boot_video_frame_count >=
				    DUMP_BOOT_VIDEO_FRAME_MAX_COUNT) {
					clog_i("finish dump boot video, total %d frames\n",
					       ctx->dump_boot_video_frame_count);
					fflush(ctx->dump_boot_video_fp);
					fclose(ctx->dump_boot_video_fp);
					ctx->dump_boot_video_fp = NULL;
					ctx->enable_dump_boot_video = false;
				}
			}
		} // end

#ifdef ENABLE_VENC_DUMP_DEBUG
		if (access("/tmp/venc_dump", F_OK) == 0) {
			if (fp == NULL && strcmp(venc_cfg->codec,
						 "264") == 0) {
				char dump_file_name[32] = { 0 };

				snprintf(dump_file_name, sizeof(dump_file_name),
					 "rtsp_%d.h264", chn);
				fp = fopen(dump_file_name, "wb");
			} else if (fp == NULL) {
				char dump_file_name[32] = { 0 };

				snprintf(dump_file_name, sizeof(dump_file_name),
					 "rtsp_%d.h265", chn);
				fp = fopen(dump_file_name, "wb");
			}
			for (uint32_t i = 0; i < pstStream->u32PackCount; i++) {
				VENC_PACK_S *ppack = &pstStream->pstPack[i];

				fwrite(ppack->pu8Addr + ppack->u32Offset,
				       ppack->u32Len - ppack->u32Offset, 1, fp);
			}
		} else {
			if (fp != NULL) {
				fflush(fp);
				fclose(fp);
				fp = NULL;
			}
		}
#endif

		ret = module_queue_push(&thiz->queue, pstStream,
					DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_e("module_queue_push failed with %#x\n", ret);
			CVI_VENC_ReleaseStream(chn, pstStream);
			free(pstStream->pstPack);
			free(pstStream);
			continue;
		}
	}

	if (pframe != NULL) {
		src_module->fun.put(src_module, pframe);
		pframe = NULL;
	}

	while (module_queue_size(&thiz->queue) > 0) {
		VENC_STREAM_S *pstStream = NULL;

		module_queue_pop(&thiz->queue, (void **)&pstStream,
				 DAEMON_TIMEOUT_MS);
		CVI_VENC_ReleaseStream(chn, pstStream);
		free(pstStream->pstPack);
		free(pstStream);
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
	CVI_VENC_ReleaseStream(thiz->pipe_id, (VENC_STREAM_S *)data);
	free(((VENC_STREAM_S *)data)->pstPack);
	free(data);
	return 0;
}

struct module_fun_t venc_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
