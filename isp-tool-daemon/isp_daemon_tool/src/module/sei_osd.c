
#include <sys/prctl.h>
#include <string.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_INFO
#define CLOG_TAG "sei_osd"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_venc.h"

#define SEI_QUEUE_SIZE (3)
#define SEI_MAX_INFO_LEN (1024)
#define SEI_UUID_SIZE (16)

// UUID for custom SEI data
static const uint8_t SEI_ISP_INFO_UUID[SEI_UUID_SIZE] = {
	0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
};

typedef enum {
	CODEC_H264,
	CODEC_H265,
	CODEC_UNKNOWN
} codec_type_e;

typedef struct {
	VENC_STREAM_S stream;
	VENC_STREAM_S *old_stream;
	uint8_t has_sei_info;
} sei_osd_data_t;

typedef struct {
	char isp_info_str[SEI_MAX_INFO_LEN];
	codec_type_e codec;
} sei_ctx_t;

static void sei_get_isp_info_string(VI_PIPE viPipe, char *str, uint32_t strLen)
{
	ISP_EXP_INFO_S expInfo;
	ISP_WB_INFO_S wbInfo;
	uint32_t frame_id = 0;

	memset(&expInfo, 0, sizeof(ISP_EXP_INFO_S));
	memset(&wbInfo, 0, sizeof(ISP_WB_INFO_S));

	CVI_ISP_QueryExposureInfo(viPipe, &expInfo);
	CVI_ISP_QueryWBInfo(viPipe, &wbInfo);
	CVI_ISP_GetFrameID(viPipe, &frame_id);

	snprintf(str, strLen,
		"FID=%u|LT=%u|ST=%u|LAG=%u|LDG=%u|LIG=%u|SAG=%u|SDG=%u|SIG=%u|"
		"Lum=%u|HE=%d|FPS=%u|LISO=%u|SISO=%u|WER=%u|LV=%d|RG=%u|BG=%u|CT=%u",
		frame_id, expInfo.u32ExpTime, expInfo.u32ShortExpTime,
		expInfo.u32AGain, expInfo.u32DGain, expInfo.u32ISPDGain,
		expInfo.u32AGainSF, expInfo.u32DGainSF, expInfo.u32ISPDGainSF,
		expInfo.u8AveLum, expInfo.s16HistError, expInfo.u32Fps,
		expInfo.u32ISO, expInfo.u32ISOSF, expInfo.u32WDRExpRatio,
		(int)(expInfo.fLightValue * 100), wbInfo.u16Rgain,
		wbInfo.u16Bgain, wbInfo.u16ColorTemp);
}

static uint8_t *build_h264_sei_nal(const char *payload, uint32_t *out_size)
{
	uint32_t payload_len = strlen(payload);
	uint32_t sei_payload_size = SEI_UUID_SIZE + payload_len;

	// NAL header(4) + NAL type(1) + SEI type(1) + size bytes + UUID + payload + trailing(1)
	uint32_t size_bytes = 1;
	uint32_t temp_size = sei_payload_size;
	while (temp_size >= 0xFF) {
		size_bytes++;
		temp_size -= 0xFF;
	}

	uint32_t buffer_size = 4 + 1 + 1 + size_bytes + SEI_UUID_SIZE + payload_len + 1;
	uint8_t *buffer = (uint8_t *)malloc(buffer_size);
	if (buffer == NULL) {
		return NULL;
	}

	uint8_t *p = buffer;

	// NAL header: 0x00 0x00 0x00 0x01
	*p++ = 0x00;
	*p++ = 0x00;
	*p++ = 0x00;
	*p++ = 0x01;

	// NAL unit type: SEI (6) with forbidden_zero_bit=0, nal_ref_idc=0
	*p++ = 0x06; // (0 << 7) | (0 << 5) | 6

	// SEI payload type: user_data_unregistered (5)
	*p++ = 5;

	// write payload szie (may need multiple bytes)
	sei_payload_size = SEI_UUID_SIZE + payload_len;
	while (sei_payload_size >= 0xFF) {
		*p++ = 0xFF;
		sei_payload_size -= 0xFF;
	}
	*p++ = (uint8_t)sei_payload_size;

	memcpy(p, SEI_ISP_INFO_UUID, SEI_UUID_SIZE);
	p += SEI_UUID_SIZE;

	memcpy(p, payload, payload_len);
	p += payload_len;

	// RBSP trailing bits: 0x80
	*p++ = 0x80;

	*out_size = p - buffer;

	return buffer;
}

static uint8_t *build_h265_sei_nal(const char *payload, uint32_t *out_size)
{
	uint32_t payload_len = strlen(payload);
	uint32_t sei_payload_size = SEI_UUID_SIZE + payload_len;

	// NAL header(4) + NAL type(2) + SEI type(1) + size bytes + UUID + payload + trailing(1)
	uint32_t size_bytes = 1;
	uint32_t temp_size = sei_payload_size;
	while (temp_size >= 0xFF) {
		size_bytes++;
		temp_size -= 0xFF;
	}

	uint32_t buffer_size = 4 + 2 + 1 + size_bytes + SEI_UUID_SIZE + payload_len + 1;
	uint8_t *buffer = (uint8_t *)malloc(buffer_size);
	if (buffer == NULL) {
		return NULL;
	}

	uint8_t *p = buffer;

	// NAL header: 0x00 0x00 0x00 0x01
	*p++ = 0x00;
	*p++ = 0x00;
	*p++ = 0x00;
	*p++ = 0x01;

	// NAL unit type: PREFIX_SEI_NUT (39)
	// forbidden_zero_bit(1) + nal_unit_type(6) + nuh_layer_id(6) + nuh_temporal_id_plus1(3)
	*p++ = (39 << 1); // First byte: type 39
	*p++ = 0x01;      // Second byte: layer_id=0, temporal_id_plus1=1

	// SEI payload type: user_data_unregistered (5)
	*p++ = 5;

	// write payload szie (may need multiple bytes)
	sei_payload_size = SEI_UUID_SIZE + payload_len;
	while (sei_payload_size >= 0xFF) {
		*p++ = 0xFF;
		sei_payload_size -= 0xFF;
	}
	*p++ = (uint8_t)sei_payload_size;

	memcpy(p, SEI_ISP_INFO_UUID, SEI_UUID_SIZE);
	p += SEI_UUID_SIZE;

	memcpy(p, payload, payload_len);
	p += payload_len;

	// RBSP trailing bits: 0x80
	*p++ = 0x80;

	*out_size = p - buffer;

	return buffer;
}

static int insert_sei_to_stream(sei_ctx_t *ctx, int pipe_id, sei_osd_data_t *sei_data)
{
	uint32_t sei_nal_size = 0;
	uint8_t *sei_buffer = NULL;

	sei_get_isp_info_string(pipe_id, ctx->isp_info_str, sizeof(ctx->isp_info_str));

	if (ctx->codec == CODEC_H264) {
		sei_buffer = build_h264_sei_nal(ctx->isp_info_str, &sei_nal_size);
	} else if (ctx->codec == CODEC_H265) {
		sei_buffer = build_h265_sei_nal(ctx->isp_info_str, &sei_nal_size);
	} else {
		clog_w("Unsupported codec, skip SEI insertion\n");
		return -1;
	}

	if (sei_buffer == NULL) {
		clog_e("Failed to build SEI NAL unit\n");
		return -1;
	}

	VENC_PACK_S *old_packs = sei_data->stream.pstPack;
	uint32_t old_pack_count = sei_data->stream.u32PackCount;

	sei_data->stream.pstPack = (VENC_PACK_S *)calloc(old_pack_count + 1, sizeof(VENC_PACK_S));
	if (sei_data->stream.pstPack == NULL) {
		clog_e("Failed to allocate memory for SEI pack array\n");
		sei_data->stream.pstPack = old_packs;
		free(sei_buffer);
		sei_buffer = NULL;
		return -1;
	}

	memcpy(sei_data->stream.pstPack, old_packs, old_pack_count * sizeof(VENC_PACK_S));

	VENC_PACK_S *sei_pack = &sei_data->stream.pstPack[old_pack_count];

	memset(sei_pack, 0, sizeof(VENC_PACK_S));
	sei_pack->pu8Addr = sei_buffer;
	sei_pack->u32Len = sei_nal_size;
	sei_pack->u32Offset = 0;
	sei_pack->u64PTS = old_packs[0].u64PTS;
	sei_pack->bFrameEnd = CVI_TRUE;

	if (old_pack_count > 0) {
		sei_data->stream.pstPack[old_pack_count - 1].bFrameEnd = CVI_FALSE;
	}

	sei_data->stream.u32PackCount = old_pack_count + 1;
	clog_d("SEI inserted: size=%u, total_packs=%u\n",
		sei_nal_size, sei_data->stream.u32PackCount);
	return 0;
}

static int release_sei_data(struct module_t *src_module, sei_osd_data_t *sei_data)
{
	int ret = 0;

	if (sei_data->has_sei_info) {
		VENC_PACK_S *last_pack =
			&sei_data->stream.pstPack[sei_data->stream.u32PackCount - 1];

		if (last_pack->pu8Addr) {
			free(last_pack->pu8Addr);
			last_pack->pu8Addr = NULL;
		}

		free(sei_data->stream.pstPack);
		sei_data->stream.pstPack = NULL;
		sei_data->stream.u32PackCount = 0;
	}

	ret = src_module->fun.put(src_module, sei_data->old_stream);
	free(sei_data);
	return ret;
}

static int init(struct module_t *thiz)
{
	clog_i("pipe_id: %d\n", thiz->pipe_id);

	sei_ctx_t *ctx = (sei_ctx_t *)calloc(1, sizeof(sei_ctx_t));
	if (!ctx) {
		clog_e("calloc failed\n");
		return -1;
	}

	module_sei_osd_cfg_t *cfg = (module_sei_osd_cfg_t *)thiz->module_cfg;

	if (strcmp(cfg->codec, "264") == 0) {
		ctx->codec = CODEC_H264;
	} else if (strcmp(cfg->codec, "265") == 0) {
		ctx->codec = CODEC_H265;
	} else {
		clog_w("Unknown codec: %s, using UNKNOWN\n", cfg->codec);
		ctx->codec = CODEC_UNKNOWN;
	}

	thiz->module_ctx = ctx;

	module_queue_init(&thiz->queue, SEI_QUEUE_SIZE);

	clog_i("SEI module initialized for codec: %s\n", cfg->codec);

	return 0;
}

static int deinit(struct module_t *thiz)
{
	clog_i("pipe_id: %d\n", thiz->pipe_id);

	sei_ctx_t *ctx = (sei_ctx_t *)thiz->module_ctx;

	if (ctx) {
		free(ctx);
		thiz->module_ctx = NULL;
	}

	if (thiz->module_cfg) {
		free(thiz->module_cfg);
		thiz->module_cfg = NULL;
	}

	module_queue_deinit(&thiz->queue);

	return 0;
}

static void *worker(void *arg)
{
	int ret = 0;
	struct module_t *thiz = (struct module_t *)arg;
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);
	sei_ctx_t *ctx = (sei_ctx_t *)thiz->module_ctx;
	int pipe_id = thiz->pipe_id;

	clog_i("run, pipe_id: %d\n", pipe_id);
	prctl(PR_SET_NAME, "sei_osd", 0, 0, 0);

	VENC_STREAM_S *pstream = NULL;
	sei_osd_data_t *sei_data = NULL;

	while (thiz->thread_run) {
		ret = src_module->fun.get(src_module, (void **)&pstream);
		if (ret != 0) {
			clog_e("src module get fail...\n");
			continue;
		}

		sei_data = (sei_osd_data_t *)calloc(1, sizeof(sei_osd_data_t));
		if (!sei_data) {
			clog_e("calloc sei_osd_data_t failed\n");
			src_module->fun.put(src_module, pstream);
			continue;
		}

		sei_data->stream = *pstream;
		sei_data->old_stream = pstream;

		sei_data->has_sei_info = 1;
		ret = insert_sei_to_stream(ctx, pipe_id, sei_data);
		if (ret != 0) {
			clog_e("Failed to insert SEI\n");
			sei_data->has_sei_info = 0;
		}

		ret = module_queue_push(&thiz->queue, sei_data, DAEMON_TIMEOUT_MS);
		if (ret != 0) {
			clog_e("module_queue_push failed with %#x\n", ret);
			release_sei_data(src_module, sei_data);
			continue;
		}
	}

	while (module_queue_size(&thiz->queue) > 0) {
		ret = module_queue_pop(&thiz->queue, (void **)&sei_data, DAEMON_TIMEOUT_MS);
		if (ret == 0 && sei_data) {
			release_sei_data(src_module, sei_data);
		}
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
	struct module_t *src_module =
		&(GET_MODULE_PIPE_NODE_PTR(thiz)->prev->module);
	sei_osd_data_t *sei_data = (sei_osd_data_t *)data;

	return release_sei_data(src_module, sei_data);
}

struct module_fun_t sei_osd_fun = {
	.init = init,
	.deinit = deinit,
	.start = start,
	.stop = stop,
	.get = get,
	.put = put,
};
