#include "ovis_manager.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum value_type { VALUE_INTEGER, VALUE_DECIMAL };

struct config_field {
	const char *id;
	const char *section;
	const char *key;
	enum value_type type;
	double minimum;
	double maximum;
};

static const struct config_field required_fields[] = {
	{ "rtsp_enabled", "output_config", "rtsp_enable", VALUE_INTEGER, 0, 1 },
	{ "uvc_enabled", "output_config", "uvc_enable", VALUE_INTEGER, 0, 1 },
	{ "desired_sub_enabled", "output_config", "sub_enable", VALUE_INTEGER, 0, 1 },
	{ "main_enabled", "vencchn0", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "main_width", "vencchn0", "width", VALUE_INTEGER, 1920, 1920 },
	{ "main_height", "vencchn0", "height", VALUE_INTEGER, 1080, 1080 },
	{ "main_fps", "vencchn0", "dst_framerate", VALUE_INTEGER, 15, 60 },
	{ "main_src_fps", "vencchn0", "src_framerate", VALUE_INTEGER, 30, 60 },
	{ "main_bitrate", "vencchn0", "bit_rate", VALUE_INTEGER, 512, 15000 },
	{ "main_max_bitrate", "vencchn0", "max_bitrate", VALUE_INTEGER, 512, 15000 },
	{ "sub_enabled", "vencchn1", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "sub_width", "vencchn1", "width", VALUE_INTEGER, 768, 768 },
	{ "sub_height", "vencchn1", "height", VALUE_INTEGER, 572, 572 },
	{ "sub_fps", "vencchn1", "dst_framerate", VALUE_INTEGER, 15, 30 },
	{ "sub_bitrate", "vencchn1", "bit_rate", VALUE_INTEGER, 128, 4000 },
	{ "sub_max_bitrate", "vencchn1", "max_bitrate", VALUE_INTEGER, 128, 4000 },
	{ "osd_enabled", "osdc_config", "enable", VALUE_INTEGER, 0, 1 },
	{ "person_enabled", "ai_pd_config", "pd_enable", VALUE_INTEGER, 0, 1 },
	{ "person_threshold", "ai_pd_config", "threshold", VALUE_DECIMAL, 0, 1 },
	{ "object_processing_width", "ai_pd_config", "grp_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_OBJECT_FRAME_MAX_WIDTH },
	{ "object_processing_height", "ai_pd_config", "grp_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_OBJECT_FRAME_MAX_HEIGHT },
	{ "object_channel_width", "vpssgrp2.chn0", "width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_OBJECT_FRAME_MAX_WIDTH },
	{ "object_channel_height", "vpssgrp2.chn0", "height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_OBJECT_FRAME_MAX_HEIGHT },
	{ "object_pool_width", "vb_pool_2", "frame_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_OBJECT_FRAME_MAX_WIDTH },
	{ "object_pool_height", "vb_pool_2", "frame_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_OBJECT_FRAME_MAX_HEIGHT },
	{ "object_pool_enabled", "vb_pool_2", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "object_group_max_width", "vpssgrp2", "max_w", VALUE_INTEGER, 1920, 1920 },
	{ "object_group_max_height", "vpssgrp2", "max_h", VALUE_INTEGER, 1080, 1080 },
	{ "object_group_source_channel", "vpssgrp2", "src_chn_id", VALUE_INTEGER, 0, 0 },
	{ "object_group_src_fps", "vpssgrp2", "src_framerate", VALUE_INTEGER, 30, 60 },
	{ "object_group_dst_fps", "vpssgrp2", "dst_framerate", VALUE_INTEGER, OVIS_AI_DETECTION_FPS, OVIS_AI_DETECTION_FPS },
	{ "face_enabled", "ai_fd_config", "fd_enable", VALUE_INTEGER, 0, 1 },
	{ "face_threshold", "ai_fd_config", "threshold_fd", VALUE_DECIMAL, 0, 1 },
	{ "face_processing_width", "ai_fd_config", "grp_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_FACE_MAX_WIDTH },
	{ "face_processing_height", "ai_fd_config", "grp_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_FACE_MAX_HEIGHT },
	{ "face_channel_width", "vpssgrp3.chn0", "width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_FACE_MAX_WIDTH },
	{ "face_channel_height", "vpssgrp3.chn0", "height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_FACE_MAX_HEIGHT },
	{ "face_pool_width", "vb_pool_3", "frame_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_FACE_MAX_WIDTH },
	{ "face_pool_height", "vb_pool_3", "frame_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_FACE_MAX_HEIGHT },
	{ "face_pool_enabled", "vb_pool_3", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "face_group_max_width", "vpssgrp3", "max_w", VALUE_INTEGER, 1920, 1920 },
	{ "face_group_max_height", "vpssgrp3", "max_h", VALUE_INTEGER, 1080, 1080 },
	{ "face_group_source_channel", "vpssgrp3", "src_chn_id", VALUE_INTEGER, 0, 0 },
	{ "motion_enabled", "ai_md_config", "md_enable", VALUE_INTEGER, 0, 1 },
	{ "motion_threshold", "ai_md_config", "threshold", VALUE_INTEGER, 0, 255 },
	{ "motion_processing_width", "ai_md_config", "grp_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_MOTION_MAX_WIDTH },
	{ "motion_processing_height", "ai_md_config", "grp_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_MOTION_MAX_HEIGHT },
	{ "motion_channel_width", "vpssgrp4.chn0", "width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_MOTION_MAX_WIDTH },
	{ "motion_channel_height", "vpssgrp4.chn0", "height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_MOTION_MAX_HEIGHT },
	{ "motion_pool_width", "vb_pool_4", "frame_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_MOTION_MAX_WIDTH },
	{ "motion_pool_height", "vb_pool_4", "frame_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_MOTION_MAX_HEIGHT },
	{ "motion_pool_enabled", "vb_pool_4", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "motion_group_max_width", "vpssgrp4", "max_w", VALUE_INTEGER, 1920, 1920 },
	{ "motion_group_max_height", "vpssgrp4", "max_h", VALUE_INTEGER, 1080, 1080 },
	{ "motion_group_source_channel", "vpssgrp4", "src_chn_id", VALUE_INTEGER, 0, 0 },
	{ "human_pose_enabled", "ai_human_keypoint_config", "human_keypoint_enable", VALUE_INTEGER, 0, 1 },
	{ "human_pose_threshold", "ai_human_keypoint_config", "threshold", VALUE_DECIMAL, 0, 1 },
	{ "human_pose_processing_width", "ai_human_keypoint_config", "model_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_HUMAN_POSE_MAX_WIDTH },
	{ "human_pose_processing_height", "ai_human_keypoint_config", "model_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_HUMAN_POSE_MAX_HEIGHT },
	{ "object_tracking_enabled", "ai_object_track_config", "object_track_enable", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_search_type", "ai_object_track_config", "search_type", VALUE_INTEGER, 2, 3 },
	{ "object_tracking_use_kalman", "ai_object_track_config", "use_kalman", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_score_threshold", "ai_object_track_config", "tracking_score_threshold", VALUE_DECIMAL, 0, 1 },
	{ "object_tracking_det_width", "ai_object_track_config", "grp_width", VALUE_INTEGER, 640, 640 },
	{ "object_tracking_det_height", "ai_object_track_config", "grp_height", VALUE_INTEGER, 384, 384 },
	{ "object_tracking_sot_group", "ai_object_track_config", "sot_vpss_grp", VALUE_INTEGER, 0, 0 },
	{ "object_tracking_sot_channel", "ai_object_track_config", "sot_vpss_chn", VALUE_INTEGER, 2, 2 },
	{ "object_tracking_sot_width", "ai_object_track_config", "sot_grp_width", VALUE_INTEGER, 1920, 1920 },
	{ "object_tracking_sot_height", "ai_object_track_config", "sot_grp_height", VALUE_INTEGER, 1080, 1080 },
	{ "object_tracking_preprocessed", "ai_object_track_config", "det_input_preprocessed", VALUE_INTEGER, 1, 1 },
	{ "object_tracking_det_refine", "ai_object_track_config", "sot_refine_selected_det", VALUE_INTEGER, 0, 0 },
	{ "object_tracking_source_depth", "vpssgrp0.chn0", "depth", VALUE_INTEGER, 0, 0 },
	{ "ai_source_channel_enabled", "vpssgrp0.chn2", "chn_enable", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_ai_channel_width", "vpssgrp0.chn2", "width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_TRACK_DET_WIDTH },
	{ "object_tracking_ai_channel_height", "vpssgrp0.chn2", "height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_TRACK_DET_HEIGHT },
	{ "object_tracking_ai_channel_depth", "vpssgrp0.chn2", "depth", VALUE_INTEGER, 1, 1 },
	{ "object_tracking_ai_channel_attach", "vpssgrp0.chn2", "attach_en", VALUE_INTEGER, 1, 1 },
	{ "object_tracking_ai_channel_pool", "vpssgrp0.chn2", "attach_pool", VALUE_INTEGER, 1, 1 },
	{ "object_tracking_det_src_fps", "vpssgrp0.chn2", "src_framerate", VALUE_INTEGER, -1, -1 },
	{ "object_tracking_det_dst_fps", "vpssgrp0.chn2", "dst_framerate", VALUE_INTEGER, -1, -1 },
	{ "object_tracking_vb_pool_count", "vb_config", "vb_pool_cnt", VALUE_INTEGER, 9, 9 },
	{ "object_tracking_det_pool_width", "vb_pool_1", "frame_width", VALUE_INTEGER, OVIS_AI_MIN_WIDTH, OVIS_AI_TRACK_DET_WIDTH },
	{ "object_tracking_det_pool_height", "vb_pool_1", "frame_height", VALUE_INTEGER, OVIS_AI_MIN_HEIGHT, OVIS_AI_TRACK_DET_HEIGHT },
	{ "object_tracking_det_pool_blocks", "vb_pool_1", "blk_cnt", VALUE_INTEGER, 4, 4 },
	{ "shared_ai_pool_enabled", "vb_pool_1", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_video_pool_width", "vb_pool_5", "frame_width", VALUE_INTEGER, 1920, 1920 },
	{ "object_tracking_video_pool_height", "vb_pool_5", "frame_height", VALUE_INTEGER, 1080, 1080 },
	{ "object_tracking_video_pool_blocks", "vb_pool_5", "blk_cnt", VALUE_INTEGER, 4, 4 },
	{ "object_tracking_sot_pool_width", "vb_pool_7", "frame_width", VALUE_INTEGER, 1920, 1920 },
	{ "object_tracking_sot_pool_height", "vb_pool_7", "frame_height", VALUE_INTEGER, 1080, 1080 },
	{ "object_tracking_sot_pool_blocks", "vb_pool_7", "blk_cnt", VALUE_INTEGER, 4, 4 },
	{ "object_tracking_sot_pool_enabled", "vb_pool_7", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "rtsp_pool_blocks", "vb_pool_6", "blk_cnt", VALUE_INTEGER, 4, 4 },
	{ "rtsp_pool_enabled", "vb_pool_6", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "uvc_pool_width", "vb_pool_8", "frame_width", VALUE_INTEGER, 1920, 1920 },
	{ "uvc_pool_height", "vb_pool_8", "frame_height", VALUE_INTEGER, 1080, 1080 },
	{ "uvc_pool_blocks", "vb_pool_8", "blk_cnt", VALUE_INTEGER, 4, 4 },
	{ "uvc_vpss_group_count", "vpss_config", "vpss_grp", VALUE_INTEGER, 7, 7 },
	{ "rtsp_channel_count", "vpssgrp1", "chn_cnt", VALUE_INTEGER, 1, 1 },
	{ "uvc_pool_enabled", "vb_pool_8", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "rtsp_group_enabled", "vpssgrp1", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "uvc_group_enabled", "vpssgrp6", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "uvc_group_device", "vpssgrp6", "vpss_dev", VALUE_INTEGER, 0, 0 },
	{ "uvc_group_channel_count", "vpssgrp6", "chn_cnt", VALUE_INTEGER, 1, 1 },
	{ "uvc_group_source_device", "vpssgrp6", "src_dev_id", VALUE_INTEGER, 0, 0 },
	{ "uvc_group_source_channel", "vpssgrp6", "src_chn_id", VALUE_INTEGER, 0, 0 },
	{ "uvc_group_destination", "vpssgrp6", "dst_dev_id", VALUE_INTEGER, 6, 6 },
	{ "uvc_channel_enabled", "vpssgrp6.chn0", "chn_enable", VALUE_INTEGER, 0, 1 },
	{ "uvc_channel_width", "vpssgrp6.chn0", "width", VALUE_INTEGER, 1920, 1920 },
	{ "uvc_channel_height", "vpssgrp6.chn0", "height", VALUE_INTEGER, 1080, 1080 },
	{ "uvc_channel_src_fps", "vpssgrp6.chn0", "src_framerate", VALUE_INTEGER, 30, 60 },
	{ "uvc_channel_dst_fps", "vpssgrp6.chn0", "dst_framerate", VALUE_INTEGER, 30, 60 },
	{ "uvc_channel_depth", "vpssgrp6.chn0", "depth", VALUE_INTEGER, 0, 0 },
	{ "uvc_channel_attach", "vpssgrp6.chn0", "attach_en", VALUE_INTEGER, 1, 1 },
	{ "uvc_channel_pool", "vpssgrp6.chn0", "attach_pool", VALUE_INTEGER, 8, 8 },
	{ "uvc_venc_source_group", "vencchn3", "src_dev_id", VALUE_INTEGER, 6, 6 },
	{ "uvc_venc_source_channel", "vencchn3", "src_chn_id", VALUE_INTEGER, 0, 0 },
	{ "uvc_venc_group", "vencchn3", "vpss_grp", VALUE_INTEGER, 6, 6 },
	{ "uvc_venc_channel", "vencchn3", "vpss_chn", VALUE_INTEGER, 0, 0 },
	{ "uvc_venc_src_fps", "vencchn3", "src_framerate", VALUE_INTEGER, 30, 60 },
	{ "uvc_venc_dst_fps", "vencchn3", "dst_framerate", VALUE_INTEGER, 30, 60 },
	{ "uvc_venc_bitrate", "vencchn3", "bit_rate", VALUE_INTEGER, 50000, 50000 },
	{ "uvc_venc_max_bitrate", "vencchn3", "max_bitrate", VALUE_INTEGER, 50000, 50000 },
	{ "uvc_venc_enabled", "vencchn3", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "rtsp_session_count", "rtsp_config", "rtsp_cnt", VALUE_INTEGER, 0, 2 },
	{ "person_vpss_enabled", "vpssgrp2", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "face_vpss_enabled", "vpssgrp3", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "motion_vpss_enabled", "vpssgrp4", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_legacy_vpss_disabled", "vpssgrp5", "grp_enable", VALUE_INTEGER, 0, 0 },
	{ "sub_vpss_enabled", "vpssgrp0.chn1", "chn_enable", VALUE_INTEGER, 0, 1 },
	{ "jpeg_enabled", "vencchn2", "bEnable", VALUE_INTEGER, 0, 1 },
	{ "sub_osd_enabled", "osdc_config1", "bShow", VALUE_INTEGER, 0, 1 },
};

static char *trim(char *text)
{
	char *end;

	while (isspace((unsigned char)*text))
		text++;
	end = text + strlen(text);
	while (end > text && isspace((unsigned char)end[-1]))
		*--end = '\0';
	return text;
}

static int validate_value(const struct config_field *field, const char *text)
{
	char *end;
	double value;

	errno = 0;
	value = strtod(text, &end);
	if (errno != 0 || end == text)
		return -1;
	while (isspace((unsigned char)*end))
		end++;
	if (*end != '\0' && *end != ';')
		return -1;
	if (field->type == VALUE_INTEGER && value != (long)value)
		return -1;
	return value >= field->minimum && value <= field->maximum ? 0 : -1;
}

static int fps_is_supported(const char *id, const char *text)
{
	long fps = strtol(text, NULL, 10);

	if (strcmp(id, "main_fps") == 0)
		return fps == 15 || fps == 25 || fps == 30 || fps == 60;
	if (strcmp(id, "main_src_fps") == 0)
		return fps == 30 || fps == 60;
	if (strcmp(id, "sub_fps") == 0)
		return fps == 15 || fps == 25 || fps == 30;
	return 1;
}

static int ai_dimension_is_supported(const char *id, const char *text)
{
	long value;

	if (strstr(id, "processing_width") == NULL &&
	    strstr(id, "processing_height") == NULL &&
	    strstr(id, "_channel_width") == NULL &&
	    strstr(id, "_channel_height") == NULL &&
	    strstr(id, "_pool_width") == NULL &&
	    strstr(id, "_pool_height") == NULL)
		return 1;
	value = strtol(text, NULL, 10);
	return (value % 2) == 0;
}

static int read_config_integer(const char *path, const char *wanted_section,
	const char *wanted_key, long *result)
{
	char line[1024];
	char section[64] = "";
	FILE *file = fopen(path, "r");

	if (file == NULL)
		return -1;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *key = trim(line);
		char *equals;

		if (*key == '[') {
			char *close = strchr(key, ']');
			if (close != NULL) {
				*close = '\0';
				snprintf(section, sizeof(section), "%s", key + 1);
			}
			continue;
		}
		equals = strchr(key, '=');
		if (equals == NULL)
			continue;
		*equals = '\0';
		if (strcmp(section, wanted_section) == 0 &&
		    strcmp(trim(key), wanted_key) == 0) {
			*result = strtol(trim(equals + 1), NULL, 10);
			fclose(file);
			return 0;
		}
	}
	fclose(file);
	return -1;
}

static int config_word_matches(const char *path, const char *wanted_section,
	const char *wanted_key, const char *expected)
{
	char line[1024];
	char section[64] = "";
	FILE *file = fopen(path, "r");

	if (file == NULL)
		return 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *key = trim(line);
		char *equals;
		char *value;
		char *end;

		if (*key == '[') {
			char *close = strchr(key, ']');
			if (close != NULL) {
				*close = '\0';
				snprintf(section, sizeof(section), "%s", key + 1);
			}
			continue;
		}
		equals = strchr(key, '=');
		if (equals == NULL)
			continue;
		*equals = '\0';
		if (strcmp(section, wanted_section) != 0 ||
		    strcmp(trim(key), wanted_key) != 0)
			continue;
		value = trim(equals + 1);
		end = value;
		while (*end != '\0' && !isspace((unsigned char)*end) && *end != ';')
			end++;
		*end = '\0';
		fclose(file);
		return strcmp(value, expected) == 0;
	}
	fclose(file);
	return 0;
}

static int dimensions_match(const char *path, const char *config_section,
	const char *config_width_key, const char *config_height_key,
	const char *channel_section, const char *pool_section)
{
	long config_width;
	long config_height;
	long channel_width;
	long channel_height;
	long pool_width;
	long pool_height;

	return read_config_integer(path, config_section, config_width_key,
			&config_width) == 0 &&
		read_config_integer(path, config_section, config_height_key,
			&config_height) == 0 &&
		read_config_integer(path, channel_section, "width", &channel_width) == 0 &&
		read_config_integer(path, channel_section, "height", &channel_height) == 0 &&
		read_config_integer(path, pool_section, "frame_width", &pool_width) == 0 &&
		read_config_integer(path, pool_section, "frame_height", &pool_height) == 0 &&
		config_width == channel_width && config_width == pool_width &&
		config_height == channel_height && config_height == pool_height;
}

int config_validate_file(const char *path, char *error, size_t error_size)
{
	int found[sizeof(required_fields) / sizeof(required_fields[0])] = {0};
	char line[1024];
	char section[64] = "";
	long main_fps = 0;
	long main_src_fps = 0;
	long uvc_src_fps = 0;
	long uvc_dst_fps = 0;
	long uvc_venc_src_fps = 0;
	long uvc_venc_dst_fps = 0;
	long object_group_src_fps = 0;
	long rtsp_enabled = 0;
	long uvc_enabled = 0;
	long desired_sub_enabled = 0;
	long main_enabled = 0;
	long sub_enabled = 0;
	long osd_enabled = 0;
	unsigned long sensor_type = 0;
	int sensor_type_found = 0;
	long person_enabled = 0;
	long face_enabled = 0;
	long motion_enabled = 0;
	long human_pose_enabled = 0;
	long object_tracking_enabled = 0;
	long ai_source_enabled = 0;
	long shared_ai_pool_enabled = 0;
	long object_pool_enabled = 0;
	long face_pool_enabled = 0;
	long motion_pool_enabled = 0;
	long object_tracking_sot_pool_enabled = 0;
	long person_vpss_enabled = 0;
	long face_vpss_enabled = 0;
	long motion_vpss_enabled = 0;
	long sub_vpss_enabled = 0;
	long jpeg_enabled = 0;
	long sub_osd_enabled = 0;
	long rtsp_pool_enabled = 0;
	long uvc_pool_enabled = 0;
	long rtsp_group_enabled = 0;
	long uvc_group_enabled = 0;
	long uvc_channel_enabled = 0;
	long uvc_venc_enabled = 0;
	long rtsp_session_count = 0;
	FILE *file;
	size_t i;
	int active_tpu_features = 0;

	file = fopen(path, "r");
	if (file == NULL) {
		snprintf(error, error_size, "无法打开配置文件");
		return -1;
	}
	while (fgets(line, sizeof(line), file) != NULL) {
		char *value;
		char *key = trim(line);
		char *equals;

		if (*key == '[') {
			char *close = strchr(key, ']');
			if (close != NULL) {
				*close = '\0';
				snprintf(section, sizeof(section), "%s", key + 1);
			}
			continue;
		}
		equals = strchr(key, '=');
		if (equals == NULL || *key == ';' || *key == '#')
			continue;
		*equals = '\0';
		value = trim(equals + 1);
		key = trim(key);
		if (strcmp(section, "sensor_config0") == 0 && strcmp(key, "sns_type") == 0) {
			char *end;

			errno = 0;
			sensor_type = strtoul(value, &end, 0);
			while (isspace((unsigned char)*end))
				end++;
			if (errno != 0 || end == value || (*end != '\0' && *end != ';')) {
				snprintf(error, error_size, "sensor_type 格式无效");
				fclose(file);
				return -1;
			}
			sensor_type_found = 1;
		}
		for (i = 0; i < sizeof(required_fields) / sizeof(required_fields[0]); i++) {
			if (strcmp(section, required_fields[i].section) == 0 &&
			    strcmp(key, required_fields[i].key) == 0) {
				if (validate_value(&required_fields[i], value) != 0 ||
				    !fps_is_supported(required_fields[i].id, value) ||
				    !ai_dimension_is_supported(required_fields[i].id, value)) {
					snprintf(error, error_size, "%s 超出允许范围", required_fields[i].id);
					fclose(file);
					return -1;
				}
				found[i] = 1;
				if (strcmp(required_fields[i].id, "rtsp_enabled") == 0)
					rtsp_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_enabled") == 0)
					uvc_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "desired_sub_enabled") == 0)
					desired_sub_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "main_enabled") == 0)
					main_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "main_fps") == 0)
					main_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "main_src_fps") == 0)
					main_src_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_channel_src_fps") == 0)
					uvc_src_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_channel_dst_fps") == 0)
					uvc_dst_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_venc_src_fps") == 0)
					uvc_venc_src_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_venc_dst_fps") == 0)
					uvc_venc_dst_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "object_group_src_fps") == 0)
					object_group_src_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "sub_enabled") == 0)
					sub_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "osd_enabled") == 0)
					osd_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "person_enabled") == 0)
					person_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "face_enabled") == 0)
					face_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "motion_enabled") == 0)
					motion_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "human_pose_enabled") == 0)
					human_pose_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "object_tracking_enabled") == 0)
					object_tracking_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "ai_source_channel_enabled") == 0)
					ai_source_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "shared_ai_pool_enabled") == 0)
					shared_ai_pool_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "object_pool_enabled") == 0)
					object_pool_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "face_pool_enabled") == 0)
					face_pool_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "motion_pool_enabled") == 0)
					motion_pool_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id,
						"object_tracking_sot_pool_enabled") == 0)
					object_tracking_sot_pool_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "person_vpss_enabled") == 0)
					person_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "face_vpss_enabled") == 0)
					face_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "motion_vpss_enabled") == 0)
					motion_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "sub_vpss_enabled") == 0)
					sub_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "jpeg_enabled") == 0)
					jpeg_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "sub_osd_enabled") == 0)
					sub_osd_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "rtsp_pool_enabled") == 0)
					rtsp_pool_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_pool_enabled") == 0)
					uvc_pool_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "rtsp_group_enabled") == 0)
					rtsp_group_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_group_enabled") == 0)
					uvc_group_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_channel_enabled") == 0)
					uvc_channel_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "uvc_venc_enabled") == 0)
					uvc_venc_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "rtsp_session_count") == 0)
					rtsp_session_count = strtol(value, NULL, 10);
				if ((strcmp(required_fields[i].id, "person_enabled") == 0 ||
				     strcmp(required_fields[i].id, "face_enabled") == 0 ||
				     strcmp(required_fields[i].id, "human_pose_enabled") == 0 ||
				     strcmp(required_fields[i].id, "object_tracking_enabled") == 0) &&
				    strtol(value, NULL, 10) == 1)
					active_tpu_features++;
			}
		}
	}
	fclose(file);
	for (i = 0; i < sizeof(required_fields) / sizeof(required_fields[0]); i++) {
		if (!found[i]) {
			snprintf(error, error_size, "缺少配置项 %s", required_fields[i].id);
			return -1;
		}
	}
	if (!sensor_type_found) {
		snprintf(error, error_size, "缺少配置项 sensor_type");
		return -1;
	}
	if (main_src_fps != (main_fps == 60 ? 60 : 30)) {
		snprintf(error, error_size, "主码流源帧率与输出帧率不匹配");
		return -1;
	}
	if (uvc_src_fps != main_src_fps || uvc_dst_fps != main_src_fps ||
	    uvc_venc_src_fps != main_src_fps || uvc_venc_dst_fps != main_src_fps) {
		snprintf(error, error_size, "UVC 帧率与 sensor 模式不匹配");
		return -1;
	}
	if (object_group_src_fps != main_src_fps) {
		snprintf(error, error_size, "目标检测 VPSS 源帧率与 sensor 模式不匹配");
		return -1;
	}
	if (sensor_type != strtoul(main_fps == 60 ? OVIS_SC235HAI_60FPS_SNS_TYPE :
			OVIS_SC235HAI_30FPS_SNS_TYPE, NULL, 0)) {
		snprintf(error, error_size, "SC235HAI 模式与主码流帧率不匹配");
		return -1;
	}
	if (active_tpu_features > 1) {
		snprintf(error, error_size, "TPU AI 功能最多只能启用一项");
		return -1;
	}
	if (person_enabled != person_vpss_enabled ||
	    face_enabled != face_vpss_enabled ||
	    motion_enabled != motion_vpss_enabled) {
		snprintf(error, error_size, "AI 功能与 VPSS 处理组开关不匹配");
		return -1;
	}
	if (ai_source_enabled != (human_pose_enabled || object_tracking_enabled)) {
		snprintf(error, error_size, "姿态或跟踪功能与共享 RGB VPSS 通道开关不匹配");
		return -1;
	}
	if (shared_ai_pool_enabled != (human_pose_enabled || object_tracking_enabled) ||
	    object_pool_enabled != person_enabled || face_pool_enabled != face_enabled ||
	    motion_pool_enabled != motion_enabled ||
	    object_tracking_sot_pool_enabled != object_tracking_enabled) {
		snprintf(error, error_size, "AI 功能与对应 VB 池开关不匹配");
		return -1;
	}
	if (main_enabled != rtsp_enabled || rtsp_pool_enabled != rtsp_enabled ||
	    rtsp_group_enabled != rtsp_enabled ||
	    rtsp_session_count != (rtsp_enabled ? 2 : 0)) {
		snprintf(error, error_size, "RTSP 开关与处理资源状态不匹配");
		return -1;
	}
	if (sub_enabled != (rtsp_enabled && desired_sub_enabled &&
			!(uvc_enabled && main_fps == 60)) ||
	    sub_vpss_enabled != sub_enabled || jpeg_enabled != sub_enabled ||
	    sub_osd_enabled != (sub_enabled && osd_enabled)) {
		snprintf(error, error_size, "子码流与依赖处理通道开关不匹配");
		return -1;
	}
	if (uvc_pool_enabled != uvc_enabled || uvc_group_enabled != uvc_enabled ||
	    uvc_channel_enabled != uvc_enabled || uvc_venc_enabled != uvc_enabled) {
		snprintf(error, error_size, "UVC 开关与处理资源状态不匹配");
		return -1;
	}
	if (!dimensions_match(path, "ai_pd_config", "grp_width", "grp_height",
			"vpssgrp2.chn0", "vb_pool_2") ||
	    !dimensions_match(path, "ai_fd_config", "grp_width", "grp_height",
			"vpssgrp3.chn0", "vb_pool_3") ||
	    !dimensions_match(path, "ai_md_config", "grp_width", "grp_height",
			"vpssgrp4.chn0", "vb_pool_4")) {
		snprintf(error, error_size, "AI 配置、VPSS 通道和 VB 池输入帧尺寸不匹配");
		return -1;
	}
	if (!config_word_matches(path, "vpssgrp2", "pixel_fmt", "PIXEL_FORMAT_NV12") ||
	    !config_word_matches(path, "vpssgrp2.chn0", "chn_pixel_fmt",
		    "PIXEL_FORMAT_NV12") ||
	    !config_word_matches(path, "vb_pool_2", "frame_fmt", "PIXEL_FORMAT_NV12") ||
	    !config_word_matches(path, "vpssgrp3", "pixel_fmt", "PIXEL_FORMAT_NV12") ||
	    !config_word_matches(path, "vpssgrp4", "pixel_fmt", "PIXEL_FORMAT_NV12")) {
		snprintf(error, error_size, "AI VPSS 上游链路必须统一使用 NV12 帧");
		return -1;
	}
	{
		long shared_width;
		long shared_height;
		long pool_width;
		long pool_height;
		long expected_width;
		long expected_height;
		const char *section = object_tracking_enabled ? "ai_object_track_config" :
			human_pose_enabled ? "ai_human_keypoint_config" : NULL;
		const char *width_key = object_tracking_enabled ? "grp_width" : "model_width";
		const char *height_key = object_tracking_enabled ? "grp_height" : "model_height";

		if (section != NULL) {
			if (read_config_integer(path, section, width_key, &expected_width) != 0 ||
			    read_config_integer(path, section, height_key, &expected_height) != 0) {
				snprintf(error, error_size, "无法读取共享 AI 通道输入尺寸");
				return -1;
			}
		} else {
			expected_width = OVIS_AI_TRACK_DET_WIDTH;
			expected_height = OVIS_AI_TRACK_DET_HEIGHT;
		}
		if (
		    read_config_integer(path, "vpssgrp0.chn2", "width", &shared_width) != 0 ||
		    read_config_integer(path, "vpssgrp0.chn2", "height", &shared_height) != 0 ||
		    read_config_integer(path, "vb_pool_1", "frame_width", &pool_width) != 0 ||
		    read_config_integer(path, "vb_pool_1", "frame_height", &pool_height) != 0 ||
		    shared_width != expected_width || shared_height != expected_height ||
		    pool_width != expected_width || pool_height != expected_height) {
			snprintf(error, error_size, "共享 AI 通道与当前输入帧尺寸不匹配");
			return -1;
		}
	}
	return 0;
}
