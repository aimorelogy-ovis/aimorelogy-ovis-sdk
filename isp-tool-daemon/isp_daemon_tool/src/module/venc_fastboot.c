
#include <sys/prctl.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "venc_fastboot"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "venc_cfg.h"
#include "cvi_isp.h"
#include "cvi_venc.h"
#include "cvi_sys.h"

// touch /tmp/venc_dump
#define ENABLE_VENC_DUMP_DEBUG (1)
#define VENC_QUEUE_SIZE (3)

static int init(struct module_t *thiz)
{
	int ret = CVI_SUCCESS;

	clog_i("pipe_id: %d\n", thiz->pipe_id);
	module_queue_init(&thiz->queue, VENC_QUEUE_SIZE);

	module_venc_fastboot_cfg_t *cfg =
		(module_venc_fastboot_cfg_t *)thiz->module_cfg;
	int dev_num = cfg->dev_num;

	for (int i = 0; i < dev_num; ++i) {
		ret = CVI_ISP_MemInit(i);
		if (ret != CVI_SUCCESS) {
			clog_e("pipe: %d, CVI_ISP_MemInit fail!\n", i);
			return -1;
		}
	}

	return 0;
}

static int deinit(struct module_t *thiz)
{
	int ret = 0;

	clog_i("pipe_id: %d\n", thiz->pipe_id);
	module_queue_deinit(&thiz->queue);

	if (thiz->module_cfg) {
		free(thiz->module_cfg);
		thiz->module_cfg = NULL;
	}

	if (thiz->module_ctx) {
		free(thiz->module_ctx);
		thiz->module_ctx = NULL;
	}

	return ret;
}

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	int chn = thiz->pipe_id;

	clog_i("run, pipe_id: %d\n", thiz->pipe_id);
	prctl(PR_SET_NAME, "venc", 0, 0, 0);

	module_venc_fastboot_cfg_t *cfg =
		(module_venc_fastboot_cfg_t *)thiz->module_cfg;
	VENC_CHN_STATUS_S stStat;
	VENC_RECV_PIC_PARAM_S stRecvParam = {0};

	stRecvParam.s32RecvPicNum = -1;

	ret = CVI_VENC_StartRecvFrame(chn, &stRecvParam);

	if (ret != CVI_SUCCESS) {
		clog_e("venc start recv frame fail!\n");
	}

	FILE *fp = NULL;

	while (thiz->thread_run) {
		memset(&stStat, 0, sizeof(VENC_CHN_STATUS_S));
		ret = CVI_VENC_QueryStatus(chn, &stStat);
		if (ret != CVI_SUCCESS) {
			clog_e("venc chn: %d, query status fail: %#x...\n", chn, ret);
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
			clog_e("venc chn: %d, get stream fail: %#x...\n", chn, ret);
			free(pstStream->pstPack);
			free(pstStream);
			continue;
		}

#ifdef ENABLE_VENC_DUMP_DEBUG
		if (access("/tmp/venc_dump", F_OK) == 0) {
			if (fp == NULL && strcmp(cfg->codec,
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
	CVI_VENC_ReleaseStream(thiz->pipe_id, (VENC_STREAM_S *)data);
	free(((VENC_STREAM_S *)data)->pstPack);
	free(data);
	return 0;
}

struct module_fun_t venc_fun_fastboot = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
