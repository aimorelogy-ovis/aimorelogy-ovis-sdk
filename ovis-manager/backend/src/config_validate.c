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
	{ "main_enabled", "vencchn0", "bEnable", VALUE_INTEGER, 1, 1 },
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
	{ "face_enabled", "ai_fd_config", "fd_enable", VALUE_INTEGER, 0, 1 },
	{ "face_threshold", "ai_fd_config", "threshold_fd", VALUE_DECIMAL, 0, 1 },
	{ "motion_enabled", "ai_md_config", "md_enable", VALUE_INTEGER, 0, 1 },
	{ "motion_threshold", "ai_md_config", "threshold", VALUE_INTEGER, 0, 255 },
	{ "human_pose_enabled", "ai_human_keypoint_config", "human_keypoint_enable", VALUE_INTEGER, 0, 1 },
	{ "human_pose_threshold", "ai_human_keypoint_config", "threshold", VALUE_DECIMAL, 0, 1 },
	{ "object_tracking_enabled", "ai_object_track_config", "object_track_enable", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_search_type", "ai_object_track_config", "search_type", VALUE_INTEGER, 2, 3 },
	{ "object_tracking_use_kalman", "ai_object_track_config", "use_kalman", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_score_threshold", "ai_object_track_config", "tracking_score_threshold", VALUE_DECIMAL, 0, 1 },
	{ "person_vpss_enabled", "vpssgrp2", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "face_vpss_enabled", "vpssgrp3", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "motion_vpss_enabled", "vpssgrp4", "grp_enable", VALUE_INTEGER, 0, 1 },
	{ "object_tracking_vpss_enabled", "vpssgrp5", "grp_enable", VALUE_INTEGER, 0, 1 },
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

int config_validate_file(const char *path, char *error, size_t error_size)
{
	int found[sizeof(required_fields) / sizeof(required_fields[0])] = {0};
	char line[1024];
	char section[64] = "";
	long main_fps = 0;
	long main_src_fps = 0;
	long sub_enabled = 0;
	long osd_enabled = 0;
	unsigned long sensor_type = 0;
	int sensor_type_found = 0;
	long person_enabled = 0;
	long face_enabled = 0;
	long motion_enabled = 0;
	long object_tracking_enabled = 0;
	long person_vpss_enabled = 0;
	long face_vpss_enabled = 0;
	long motion_vpss_enabled = 0;
	long object_tracking_vpss_enabled = 0;
	long sub_vpss_enabled = 0;
	long jpeg_enabled = 0;
	long sub_osd_enabled = 0;
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
				    !fps_is_supported(required_fields[i].id, value)) {
					snprintf(error, error_size, "%s 超出允许范围", required_fields[i].id);
					fclose(file);
					return -1;
				}
				found[i] = 1;
				if (strcmp(required_fields[i].id, "main_fps") == 0)
					main_fps = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "main_src_fps") == 0)
					main_src_fps = strtol(value, NULL, 10);
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
				else if (strcmp(required_fields[i].id, "object_tracking_enabled") == 0)
					object_tracking_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "person_vpss_enabled") == 0)
					person_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "face_vpss_enabled") == 0)
					face_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "motion_vpss_enabled") == 0)
					motion_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "object_tracking_vpss_enabled") == 0)
					object_tracking_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "sub_vpss_enabled") == 0)
					sub_vpss_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "jpeg_enabled") == 0)
					jpeg_enabled = strtol(value, NULL, 10);
				else if (strcmp(required_fields[i].id, "sub_osd_enabled") == 0)
					sub_osd_enabled = strtol(value, NULL, 10);
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
	    motion_enabled != motion_vpss_enabled ||
	    object_tracking_enabled != object_tracking_vpss_enabled) {
		snprintf(error, error_size, "AI 功能与 VPSS 处理组开关不匹配");
		return -1;
	}
	if (sub_vpss_enabled != sub_enabled || jpeg_enabled != sub_enabled ||
	    sub_osd_enabled != (sub_enabled && osd_enabled)) {
		snprintf(error, error_size, "子码流与依赖处理通道开关不匹配");
		return -1;
	}
	return 0;
}
