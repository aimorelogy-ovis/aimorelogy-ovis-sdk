
#include "daemon_base.h"
#include "daemon_cfg.h"
#include "cvi_json.h"
#include "cvi_comm_video.h"

#define GET_VC_KEY_VAL(p_vc_cfg, sub_cfg, key, sub_key)\
{\
	if (cvi_json_object_object_get_ex(json_obj, key, &json_val)) {\
		if (cvi_json_object_object_get_ex(json_val, "items", &items_val)) {\
			arr_len = cvi_json_object_array_length(items_val);\
			for (int i = 0; i < arr_len; ++i) {\
				item_obj = cvi_json_object_array_get_idx(items_val, i);\
				if (cvi_json_object_object_get_ex(item_obj, "key", &item_key_val)) {\
					key_val_str = cvi_json_object_get_string(item_key_val);\
					if (strcmp(key_val_str, #sub_key) == 0) {\
						if (cvi_json_object_object_get_ex(item_obj, "value", &value)) {\
							p_vc_cfg->sub_cfg.sub_key = cvi_json_object_get_int(value);\
						} \
					} \
				} \
			} \
		} \
	} \
} \

#define PR_VC_KEY_VAL(p_vc_cfg, sub_cfg, key, sub_key)\
{\
	printf("\t"key": "#sub_key": %d\n", p_vc_cfg->sub_cfg.sub_key);\
} \

static int print_daemon_pipe_cfg(daemon_pipe_cfg_t *p_cfg);
static int init_vc_from_json(const char *json_path, vc_cfg_t *p_vc_cfg);
static int print_vc_key_val(vc_cfg_t *p_vc_cfg);
static int init_raw_replay_param_from_json(const char *json_path, raw_replay_cfg_t *p_cfg);
static void print_raw_replay_param(raw_replay_cfg_t *p_cfg);
static int init_stitch_param_from_json(const char *json_path, daemon_pipe_cfg_t *p_cfg, int dev_num);
static void print_stitch_param(ISP_STITCH_ATTR_S *p_attr, int chn);

static int get_json_object_from_file(const char *json_path, struct cvi_json_object **json_obj)
{
	FILE *fp = fopen(json_path, "r");

	if (!fp) {
		clog_e("Error opening file: %s.\n", json_path);
		return -1;
	}

	fseek(fp, 0, SEEK_END);

	unsigned long long file_size = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	char *buffer = (char *)malloc(file_size + 1);

	if (!buffer) {
		clog_e("Memory allocation error.\n");
		fclose(fp);
		return -1;
	}

	fread(buffer, 1, file_size, fp);
	fclose(fp);
	buffer[file_size] = '\0';

	*json_obj = cvi_json_tokener_parse(buffer);

	if (*json_obj == NULL) {
		clog_e("Error parsing JSON: %s.\n", json_path);
		free(buffer);
		return -1;
	}

	free(buffer);

	return 0;
}

static COMPRESS_MODE_E get_compress_mode(const char *mode)
{
	if (strcmp(mode, "none") == 0) {
		return COMPRESS_MODE_NONE;
	} else if (strcmp(mode, "tile") == 0) {
		return COMPRESS_MODE_TILE;
	} else if (strcmp(mode, "line") == 0) {
		return COMPRESS_MODE_LINE;
	} else if (strcmp(mode, "frame") == 0) {
		return COMPRESS_MODE_FRAME;
	} else {
		return COMPRESS_MODE_NONE;
	}
}

int daemon_pipe_cfg_init(const char *json_path, daemon_pipe_cfg_t **p_pipe_cfg)
{
	int dev_num = 0;
	daemon_pipe_cfg_t *p_cfg = NULL;
	struct cvi_json_object *json_obj = NULL;

	if (get_json_object_from_file(json_path, &json_obj) != 0) {
		clog_e("parse json: %s fail!\n", json_path);
		return -1;
	}

	struct cvi_json_object *val_json_object = NULL;

	if (cvi_json_object_object_get_ex(json_obj, "dev-num", &val_json_object)) {
		dev_num = cvi_json_object_get_int(val_json_object);
	} else {
		clog_e("parse dev-num fail!\n");
		return -1;
	}

	clog_i("dev-num: %d\n", dev_num);
	p_cfg = (daemon_pipe_cfg_t *) calloc(dev_num, sizeof(daemon_pipe_cfg_t));
	if (p_cfg == NULL) {
		clog_e("calloc pipe cfg fail!\n");
		return -1;
	}

	p_cfg->dev_num = dev_num;

	if (cvi_json_object_object_get_ex(json_obj, "rtsp-port", &val_json_object)) {
		p_cfg->rtsp_port = cvi_json_object_get_int(val_json_object);
	}

	if (cvi_json_object_object_get_ex(json_obj, "rtsp-max-buf-size", &val_json_object)) {
		p_cfg->rtsp_max_buf_size = cvi_json_object_get_int(val_json_object);
	}

	if (cvi_json_object_object_get_ex(json_obj, "rtsp-server-select", &val_json_object)) {
		p_cfg->rtsp_server_select = cvi_json_object_get_int(val_json_object);
	}

	if (cvi_json_object_object_get_ex(json_obj, "vi-vpss-mode", &val_json_object)) {
		p_cfg->vi_vpss_mode = cvi_json_object_get_int(val_json_object);
	}

	p_cfg->max_use_tpu_num = 1;
	if (cvi_json_object_object_get_ex(json_obj, "max-use-tpu-num", &val_json_object)) {
		p_cfg->max_use_tpu_num = cvi_json_object_get_int(val_json_object);
	}

	if (cvi_json_object_object_get_ex(json_obj, "teaisp-faceae-model", &val_json_object)) {
		snprintf(p_cfg->teaisp_faceae_model_path, MAX_PATH_LEN, "%s",
			cvi_json_object_get_string(val_json_object));
	}

	if (cvi_json_object_object_get_ex(json_obj, "teaisp-pq-model", &val_json_object)) {
		snprintf(p_cfg->teaisp_pq_model_path, MAX_PATH_LEN, "%s",
			cvi_json_object_get_string(val_json_object));
	}

	if (cvi_json_object_object_get_ex(json_obj, "cvi-bin-path", &val_json_object)) {
		snprintf(p_cfg->cvi_bin_path, MAX_PATH_LEN, "%s",
			cvi_json_object_get_string(val_json_object));
	}

	if (cvi_json_object_object_get_ex(json_obj, "sensor-cfg-ini", &val_json_object)) {
		snprintf(p_cfg->sns_cfg_ini, MAX_PATH_LEN, "%s",
			cvi_json_object_get_string(val_json_object));
	}

	if (cvi_json_object_object_get_ex(json_obj, "is_fastboot_mode", &val_json_object)) {
		const char *tmp_str = cvi_json_object_get_string(val_json_object);

		if (strcmp(tmp_str, "true") == 0) {
			p_cfg->is_fastboot_mode = 1;
		} else {
			p_cfg->is_fastboot_mode = 0;
		}
	}

	if (cvi_json_object_object_get_ex(json_obj, "enable_dump_boot_video", &val_json_object)) {
		const char *tmp_str = cvi_json_object_get_string(val_json_object);

		if (strcmp(tmp_str, "true") == 0) {
			p_cfg->enable_dump_boot_video = 1;
		} else {
			p_cfg->enable_dump_boot_video = 0;
		}
	}

	if (cvi_json_object_object_get_ex(json_obj, "replay-mode", &val_json_object)) {
		const char *tmp_str = cvi_json_object_get_string(val_json_object);

		if (strcmp(tmp_str, "true") == 0) {
			p_cfg->raw_replay_enable = 1;
		} else {
			p_cfg->raw_replay_enable = 0;
		}
	}

	if (p_cfg->raw_replay_enable) {
		if (cvi_json_object_object_get_ex(json_obj, "replay-param", &val_json_object)) {
			const char *tmp_str = cvi_json_object_get_string(val_json_object);

			if (init_raw_replay_param_from_json(tmp_str, &p_cfg->raw_replay_cfg) != 0) {
				clog_e("init raw replay param fail!\n");
				free(p_cfg);
				return -1;
			} else {
				print_raw_replay_param(&p_cfg->raw_replay_cfg);
			}
		}
	}

	// parse enable-stitch and stitch-param
	if (cvi_json_object_object_get_ex(json_obj, "enable-stitch", &val_json_object)) {
		const char *tmp_str = cvi_json_object_get_string(val_json_object);

		if (strcmp(tmp_str, "true") == 0) {
			p_cfg->enable_stitch = 1;
		} else {
			p_cfg->enable_stitch = 0;
		}
	}

	if (p_cfg->enable_stitch) {
		if (cvi_json_object_object_get_ex(json_obj, "stitch-param", &val_json_object)) {
			const char *tmp_str = cvi_json_object_get_string(val_json_object);

			if (init_stitch_param_from_json(tmp_str, p_cfg, dev_num) != 0) {
				clog_e("init stitch param fail!\n");
				free(p_cfg);
				return -1;
			}
		}
	}

	// video src info
	if (cvi_json_object_object_get_ex(json_obj, "video-src-info", &val_json_object)) {
		int ch_num = cvi_json_object_array_length(val_json_object);

		if (dev_num > ch_num) {
			clog_e("rtsp cfg error: dev_num: %d must greater than video src info array length: %d!\n",
				dev_num, ch_num);
			return -1;
		}

		struct cvi_json_object *array_ele = NULL;
		struct cvi_json_object *arr_val_json_object = NULL;

		for (int i = 1; i < dev_num; ++i) {
			p_cfg[i].dev_num = p_cfg[0].dev_num;
			p_cfg[i].rtsp_port = p_cfg[0].rtsp_port;
			p_cfg[i].rtsp_max_buf_size = p_cfg[0].rtsp_max_buf_size;
			p_cfg[i].rtsp_server_select = p_cfg[0].rtsp_server_select;
			p_cfg[i].vi_vpss_mode = p_cfg[0].vi_vpss_mode;
			p_cfg[i].raw_replay_enable = p_cfg[0].raw_replay_enable;
			p_cfg[i].is_fastboot_mode = p_cfg[0].is_fastboot_mode;
			p_cfg[i].enable_dump_boot_video = p_cfg[0].enable_dump_boot_video;
			p_cfg[i].max_use_tpu_num = p_cfg[0].max_use_tpu_num;
			memcpy(p_cfg[i].teaisp_faceae_model_path, p_cfg[0].teaisp_faceae_model_path, MAX_PATH_LEN);
			memcpy(p_cfg[i].teaisp_pq_model_path, p_cfg[0].teaisp_pq_model_path, MAX_PATH_LEN);
			memcpy(p_cfg[i].cvi_bin_path, p_cfg[0].cvi_bin_path, MAX_PATH_LEN);
			memcpy(p_cfg[i].sns_cfg_ini, p_cfg[0].sns_cfg_ini, MAX_PATH_LEN);
		}

		for (int i = 0; i < dev_num; ++i) {
			array_ele = cvi_json_object_array_get_idx(val_json_object, i);
			p_cfg[i].video_pipe_cfg.chn = i;

			if (cvi_json_object_object_get_ex(array_ele, "buf-blk-cnt", &arr_val_json_object)) {
				p_cfg[i].video_pipe_cfg.buf_blk_cnt = cvi_json_object_get_int(arr_val_json_object);
			}

			if (cvi_json_object_object_get_ex(array_ele, "is_wdr_mode", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				if (strcmp(tmp_str, "true") == 0) {
					p_cfg[i].video_pipe_cfg.is_wdr_mode = 1;
				} else {
					p_cfg[i].video_pipe_cfg.is_wdr_mode = 0;
				}
			}

			if (cvi_json_object_object_get_ex(array_ele, "enable-isp-info-osd", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				if (strcmp(tmp_str, "true") == 0) {
					p_cfg[i].video_pipe_cfg.enable_isp_info_osd = 1;
				} else {
					p_cfg[i].video_pipe_cfg.enable_isp_info_osd = 0;
				}
			}

			p_cfg[i].video_pipe_cfg.tpu_device_id = 0;
			if (cvi_json_object_object_get_ex(json_obj, "tpu-device-id", &arr_val_json_object)) {
				p_cfg[i].video_pipe_cfg.tpu_device_id = cvi_json_object_get_int(arr_val_json_object);
				if (p_cfg[i].video_pipe_cfg.tpu_device_id >= p_cfg->max_use_tpu_num)
					p_cfg[i].video_pipe_cfg.tpu_device_id = p_cfg->max_use_tpu_num - 1;
			}

			if (cvi_json_object_object_get_ex(array_ele, "enable-faceae", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				if (strcmp(tmp_str, "true") == 0) {
					p_cfg[i].video_pipe_cfg.enable_face_ae = 1;
				} else {
					p_cfg[i].video_pipe_cfg.enable_face_ae = 0;
				}
			}

			if (cvi_json_object_object_get_ex(array_ele, "enable-teaisp-pq", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				if (strcmp(tmp_str, "true") == 0) {
					p_cfg[i].video_pipe_cfg.enable_teaisp_pq = 1;
				} else {
					p_cfg[i].video_pipe_cfg.enable_teaisp_pq = 0;
				}
			}

			if (cvi_json_object_object_get_ex(array_ele, "enable-teaisp-bnr", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				if (strcmp(tmp_str, "true") == 0) {
					p_cfg[i].video_pipe_cfg.enable_teaisp_bnr = 1;
				} else {
					p_cfg[i].video_pipe_cfg.enable_teaisp_bnr = 0;
				}
			}

			if (cvi_json_object_object_get_ex(array_ele, "teaisp_model_list", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				snprintf(p_cfg[i].video_pipe_cfg.bnr_model_list, MAX_PATH_LEN,
				 "%s",  tmp_str);
			}

			if (cvi_json_object_object_get_ex(array_ele, "venc_json", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				snprintf(p_cfg[i].video_pipe_cfg.venc_json, MAX_PATH_LEN,
				 "%s",  tmp_str);
			}

			if (cvi_json_object_object_get_ex(array_ele, "codec", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				snprintf(p_cfg[i].video_pipe_cfg.codec, MAX_CODEC_LEN, "%s", tmp_str);
			}

			if (cvi_json_object_object_get_ex(array_ele, "compress-mode", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				p_cfg[i].video_pipe_cfg.compress_mode = get_compress_mode(tmp_str);
			}

			if (cvi_json_object_object_get_ex(array_ele, "enable-patgen", &arr_val_json_object)) {
				const char *tmp_str = cvi_json_object_get_string(arr_val_json_object);

				if (strcmp(tmp_str, "true") == 0) {
					p_cfg[i].video_pipe_cfg.enable_patgen = 1;
				} else {
					p_cfg[i].video_pipe_cfg.enable_patgen = 0;
				}
			}
		}
		print_daemon_pipe_cfg(p_cfg);

		// get the vc param
		for (int i = 0; i < dev_num; ++i) {
			init_vc_from_json(p_cfg->video_pipe_cfg.venc_json, &p_cfg[i].video_pipe_cfg.st_vc_cfg);
			print_vc_key_val(&p_cfg->video_pipe_cfg.st_vc_cfg);
		}
	}

	// free
	cvi_json_object_put(json_obj);

	*p_pipe_cfg = p_cfg;

	printf("----------------------------------------------------------------------------\n");
	return 0;
}

int daemon_pipe_cfg_deinit(daemon_pipe_cfg_t **p_cfg)
{
	if (p_cfg && *p_cfg) {
		free(*p_cfg);
		*p_cfg = NULL;
	}
	return 0;
}

static int print_daemon_pipe_cfg(daemon_pipe_cfg_t *p_cfg)
{
	for (int i = 0; i < p_cfg[0].dev_num; ++i) {
		printf("------------------------daemon pipe chn: %d, pipe info--------------------------\n", i);
		printf("\trtsp-port:%d\n", p_cfg[i].rtsp_port);
		printf("\trtsp-max-buf-size:%llu\n", p_cfg[i].rtsp_max_buf_size);
		printf("\trtsp-server-select: %d\n", p_cfg[i].rtsp_server_select);
		printf("\tvi-vpss-mode: %d\n", p_cfg[i].vi_vpss_mode);
		printf("\tteaisp-faceae-model: %s\n", p_cfg[i].teaisp_faceae_model_path);
		printf("\tteaisp-pq-model: %s\n", p_cfg[i].teaisp_pq_model_path);
		printf("\treplay-mode: %d\n", p_cfg[i].raw_replay_enable);
		printf("\tis_fastboot_mode: %d\n", p_cfg[i].is_fastboot_mode);
		printf("\tenable_dump_boot_video: %d\n", p_cfg[i].enable_dump_boot_video);
		printf("\tchn: %d\n", p_cfg[i].video_pipe_cfg.chn);
		printf("\tbuf-blk-cnt: %d\n", p_cfg[i].video_pipe_cfg.buf_blk_cnt);
		printf("\tis_wdr_mode: %d\n", p_cfg[i].video_pipe_cfg.is_wdr_mode);
		printf("\tenable-teaisp-bnr: %d\n", p_cfg[i].video_pipe_cfg.enable_teaisp_bnr);
		printf("\tvenc_json: %s\n", p_cfg[i].video_pipe_cfg.venc_json);
		printf("\tcodec: %s\n", p_cfg[i].video_pipe_cfg.codec);
		printf("\tcompress-mode: %d\n", p_cfg[i].video_pipe_cfg.compress_mode);
		printf("\tenable-patgen: %d\n", p_cfg[i].video_pipe_cfg.enable_patgen);
		printf("\tmax-use-tpu-num: %d\n", p_cfg[i].max_use_tpu_num);
		printf("\ttpu-device-id: %d\n", p_cfg[i].video_pipe_cfg.tpu_device_id);
	}

	return 0;
}

static int init_vc_from_json(const char *json_path, vc_cfg_t *p_vc_cfg)
{
	struct cvi_json_object *json_obj = NULL;
	struct cvi_json_object *json_val = NULL;
	struct cvi_json_object *items_val = NULL;
	struct cvi_json_object *item_obj = NULL;
	struct cvi_json_object *item_key_val = NULL;
	struct cvi_json_object *value = NULL;
	const char *key_val_str;
	int arr_len = 0;

	if (get_json_object_from_file(json_path, &json_obj) != 0) {
		clog_e("parse json: %s fail!\n", json_path);
		return -1;
	}

	memset(p_vc_cfg, 0, sizeof(vc_cfg_t));

	// Coding Param
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", FrmLostOpen);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", LostMode);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", FrmLostBpsThr);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", EncFrmGaps);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", IntraCost);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", aspectRatioInfoPresentFlag);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", overscanInfoPresentFlag);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", videoSignalTypePresentFlag);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", videoFormat);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", videoFullRangeFlag);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", colourDescriptionPresentFlag);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", ChromaQpOffset);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", CbQpOffset);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", CrQpOffset);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", TimingInfoPresentFlag);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", FixedFrameRateFlag);
	GET_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", NumUnitsInTick);

	// Gop Mode
	GET_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", GopMode);
	GET_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", IPQpDelta);
	GET_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", BgInterval);
	GET_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", BgQpDelta);
	GET_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", ViQpDelta);
	// RC Attr
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", RcMode);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", Gop);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", VariableFPS);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", SrcFrmRate);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", DstFrmRate);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", StatTime);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", BitRate);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", MaxBitrate);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", IQP);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", PQP);
	// RC Param
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", RowQpDelta);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", ThrdLv);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", FirstFrameStartQp);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", InitialDelay);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinIprop);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxIprop);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxQp);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinQp);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxIQp);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinIQp);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", ChangePos);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinStillPercent);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxStillQP);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MotionSensitivity);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", PureStillThr);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", AvbrFrmLostOpen);
	GET_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", AvbrFrmGap);

	// free
	cvi_json_object_put(json_obj);

	return 0;
}

static int print_vc_key_val(vc_cfg_t *p_vc_cfg)
{
	printf("-------------------------------------- VC Param --------------------------------------\n");
	// Coding Param
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", FrmLostOpen);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", LostMode);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", FrmLostBpsThr);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", EncFrmGaps);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", IntraCost);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", aspectRatioInfoPresentFlag);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", overscanInfoPresentFlag);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", videoSignalTypePresentFlag);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", videoFormat);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", videoFullRangeFlag);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", colourDescriptionPresentFlag);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", ChromaQpOffset);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", CbQpOffset);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", CrQpOffset);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", TimingInfoPresentFlag);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", FixedFrameRateFlag);
	PR_VC_KEY_VAL(p_vc_cfg, st_coding_param, "Coding Param", NumUnitsInTick);
	// Gop Mode
	PR_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", GopMode);
	PR_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", IPQpDelta);
	PR_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", BgInterval);
	PR_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", BgQpDelta);
	PR_VC_KEY_VAL(p_vc_cfg, st_gop_mode, "Gop Mode", ViQpDelta);
	// RC Attr
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", RcMode);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", Gop);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", VariableFPS);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", SrcFrmRate);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", DstFrmRate);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", StatTime);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", BitRate);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", MaxBitrate);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", IQP);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_attr, "RC Attr", PQP);
	// RC Param
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", RowQpDelta);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", ThrdLv);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", FirstFrameStartQp);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", InitialDelay);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinIprop);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxIprop);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxQp);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinQp);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxIQp);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinIQp);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", ChangePos);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MinStillPercent);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MaxStillQP);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", MotionSensitivity);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", PureStillThr);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", AvbrFrmLostOpen);
	PR_VC_KEY_VAL(p_vc_cfg, st_rc_param, "RC Param", AvbrFrmGap);

	return 0;
}

static int init_raw_replay_param_from_json(const char *json_path, raw_replay_cfg_t *p_cfg)
{
	struct cvi_json_object *json_obj = NULL;
	struct cvi_json_object *json_val = NULL;

	if (get_json_object_from_file(json_path, &json_obj) != 0) {
		clog_e("parse json: %s fail!\n", json_path);
		return -1;
	}

	memset(p_cfg, 0, sizeof(raw_replay_cfg_t));

	if (cvi_json_object_object_get_ex(json_obj, "ReplayScene", &json_val)) {
		p_cfg->replay_scene = cvi_json_object_get_int(json_val);
	}

	if (cvi_json_object_object_get_ex(json_obj, "PixelFormat", &json_val)) {
		p_cfg->pixel_format = cvi_json_object_get_int(json_val);
	}

	if (cvi_json_object_object_get_ex(json_obj, "Width", &json_val)) {
		p_cfg->width = cvi_json_object_get_int(json_val);
	}

	if (cvi_json_object_object_get_ex(json_obj, "Height", &json_val)) {
		p_cfg->height = cvi_json_object_get_int(json_val);
	}

	if (cvi_json_object_object_get_ex(json_obj, "TimingEnable", &json_val)) {
		const char *tmp_str = cvi_json_object_get_string(json_val);

		if (strcmp(tmp_str, "true") == 0) {
			p_cfg->timing_enable = 1;
		} else {
			p_cfg->timing_enable = 0;
		}
	}

	if (cvi_json_object_object_get_ex(json_obj, "FrameRate", &json_val)) {
		p_cfg->frame_rate = cvi_json_object_get_int(json_val);
	}

	if (cvi_json_object_object_get_ex(json_obj, "WDRMode", &json_val)) {
		p_cfg->wdr_mode = cvi_json_object_get_int(json_val);
	}

	if (cvi_json_object_object_get_ex(json_obj, "BayerFormat", &json_val)) {
		p_cfg->bayer_format = cvi_json_object_get_int(json_val);
	}

	if (cvi_json_object_object_get_ex(json_obj, "CompressMode", &json_val)) {
		const char *tmp_str = cvi_json_object_get_string(json_val);

		p_cfg->compress_mode = get_compress_mode(tmp_str);
	}

	if (cvi_json_object_object_get_ex(json_obj, "OfflineRawDir", &json_val)) {
		const char *tmp_str = cvi_json_object_get_string(json_val);

		snprintf(p_cfg->offline_raw_dir, MAX_PATH_LEN, "%s", tmp_str);
	}

	// free
	cvi_json_object_put(json_obj);

	return 0;
}

static void print_raw_replay_param(raw_replay_cfg_t *p_cfg)
{
	printf("--------------------------------Raw Replay Param--------------------------------\n");
	printf("\tReplayScene: %d\n", p_cfg->replay_scene);
	printf("\tPixelFormat: %d\n", p_cfg->pixel_format);
	printf("\tWidth: %d\n", p_cfg->width);
	printf("\tHeight: %d\n", p_cfg->height);
	printf("\tTimingEnable: %d\n", p_cfg->timing_enable);
	printf("\tFrameRate: %d\n", p_cfg->frame_rate);
	printf("\tWDRMode: %d\n", p_cfg->wdr_mode);
	printf("\tBayerFormat: %d\n", p_cfg->bayer_format);
	printf("\tCompressMode: %d\n", p_cfg->compress_mode);
	printf("\tOfflineRawDir: %s\n", p_cfg->offline_raw_dir);
}

static int init_stitch_param_from_json(const char *json_path, daemon_pipe_cfg_t *p_cfg, int dev_num)
{
	struct cvi_json_object *json_obj = NULL;

	if (get_json_object_from_file(json_path, &json_obj) != 0) {
		clog_e("parse stitch json: %s fail!\n", json_path);
		return -1;
	}

	struct cvi_json_object *stitch_attr_array = NULL;

	if (!cvi_json_object_object_get_ex(json_obj, "isp_stitch_attr", &stitch_attr_array)) {
		clog_e("get isp_stitch_attr from json fail!\n");
		cvi_json_object_put(json_obj);
		return -1;
	}

	int arr_len = cvi_json_object_array_length(stitch_attr_array);

	if (arr_len > dev_num) {
		clog_w("stitch attr array length %d is greater than dev_num %d, only parse first %d items\n",
			arr_len, dev_num, dev_num);
		arr_len = dev_num;
	}

	for (int i = 0; i < arr_len; ++i) {
		struct cvi_json_object *item_obj = cvi_json_object_array_get_idx(stitch_attr_array, i);
		struct cvi_json_object *json_val = NULL;
		ISP_STITCH_ATTR_S *p_stitch = &p_cfg[i].video_pipe_cfg.stitch_attr;

		if (cvi_json_object_object_get_ex(item_obj, "enable", &json_val)) {
			const char *tmp_str = cvi_json_object_get_string(json_val);

			if (strcmp(tmp_str, "true") == 0) {
				p_stitch->enable = 1;
			} else {
				p_stitch->enable = 0;
			}
		}

		if (cvi_json_object_object_get_ex(item_obj, "bMainPipe", &json_val)) {
			const char *tmp_str = cvi_json_object_get_string(json_val);

			if (strcmp(tmp_str, "true") == 0) {
				p_stitch->bMainPipe = 1;
			} else {
				p_stitch->bMainPipe = 0;
			}
		}

		if (cvi_json_object_object_get_ex(item_obj, "bCalibEnable", &json_val)) {
			const char *tmp_str = cvi_json_object_get_string(json_val);

			if (strcmp(tmp_str, "true") == 0) {
				p_stitch->bCalibEnable = 1;
			} else {
				p_stitch->bCalibEnable = 0;
			}
		}

		if (cvi_json_object_object_get_ex(item_obj, "bCombineSts", &json_val)) {
			const char *tmp_str = cvi_json_object_get_string(json_val);

			if (strcmp(tmp_str, "true") == 0) {
				p_stitch->bCombineSts = 1;
			} else {
				p_stitch->bCombineSts = 0;
			}
		}

		if (cvi_json_object_object_get_ex(item_obj, "u8CombChn", &json_val)) {
			p_stitch->u8CombChn = cvi_json_object_get_int(json_val);
		}

		if (cvi_json_object_object_get_ex(item_obj, "u8CombChnSum", &json_val)) {
			p_stitch->u8CombChnSum = cvi_json_object_get_int(json_val);
		}

		if (cvi_json_object_object_get_ex(item_obj, "u8Group", &json_val)) {
			p_stitch->u8Group = cvi_json_object_get_int(json_val);
		}

		if (cvi_json_object_object_get_ex(item_obj, "u32CalibLumaRatio", &json_val)) {
			p_stitch->u32CalibLumaRatio = cvi_json_object_get_int(json_val);
		}

		if (cvi_json_object_object_get_ex(item_obj, "u32CalibRGainRatio", &json_val)) {
			p_stitch->u32CalibRGainRatio = cvi_json_object_get_int(json_val);
		}

		if (cvi_json_object_object_get_ex(item_obj, "u32CalibBGainRatio", &json_val)) {
			p_stitch->u32CalibBGainRatio = cvi_json_object_get_int(json_val);
		}

		print_stitch_param(p_stitch, i);
	}

	cvi_json_object_put(json_obj);
	return 0;
}

static void print_stitch_param(ISP_STITCH_ATTR_S *p_attr, int chn)
{
	printf("--------------------------------Stitch Param Chn %d--------------------------------\n", chn);
	printf("\tenable: %d\n", p_attr->enable);
	printf("\tbMainPipe: %d\n", p_attr->bMainPipe);
	printf("\tbCalibEnable: %d\n", p_attr->bCalibEnable);
	printf("\tbCombineSts: %d\n", p_attr->bCombineSts);
	printf("\tu8CombChn: %d\n", p_attr->u8CombChn);
	printf("\tu8CombChnSum: %d\n", p_attr->u8CombChnSum);
	printf("\tu8Group: %d\n", p_attr->u8Group);
	printf("\tu32CalibLumaRatio: %u\n", p_attr->u32CalibLumaRatio);
	printf("\tu32CalibRGainRatio: %u\n", p_attr->u32CalibRGainRatio);
	printf("\tu32CalibBGainRatio: %u\n", p_attr->u32CalibBGainRatio);
}
