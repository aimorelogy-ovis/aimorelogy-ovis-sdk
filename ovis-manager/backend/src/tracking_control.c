#include "ovis_manager.h"
#include "cJSON.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TRACK_REQUEST_PATH "/tmp/track"
#define TRACK_STATUS_PATH "/tmp/object_track_fps"
#define TRACK_COORDINATE_SCALE 1000000

static cJSON *object_item(cJSON *parent, const char *name)
{
	cJSON *item = parent == NULL ? NULL :
		cJSON_GetObjectItemCaseSensitive(parent, name);

	return cJSON_IsObject(item) ? item : NULL;
}

static int tracking_config_flags(int *tracking_enabled, int *detection_enabled)
{
	char json[64 * 1024];
	cJSON *root;
	cJSON *values;
	cJSON *tracking;
	cJSON *single_object;
	cJSON *detection;
	cJSON *object;
	cJSON *item;
	int result = -1;

	if (config_read_json(json, sizeof(json)) != 0)
		return -1;
	root = cJSON_Parse(json);
	values = object_item(root, "values");
	tracking = object_item(values, "tracking");
	single_object = object_item(tracking, "single_object");
	detection = object_item(values, "detection");
	object = object_item(detection, "object");
	item = single_object == NULL ? NULL :
		cJSON_GetObjectItemCaseSensitive(single_object, "enabled");
	if (!cJSON_IsBool(item))
		goto done;
	*tracking_enabled = cJSON_IsTrue(item);
	item = object == NULL ? NULL :
		cJSON_GetObjectItemCaseSensitive(object, "enabled");
	if (!cJSON_IsBool(item))
		goto done;
	*detection_enabled = cJSON_IsTrue(item);
	result = 0;
done:
	cJSON_Delete(root);
	return result;
}

static int write_track_request(const char *command, char *error,
	size_t error_size)
{
	char temporary[64];
	FILE *file;
	int write_failed;

	snprintf(temporary, sizeof(temporary), "%s.%ld.tmp",
		TRACK_REQUEST_PATH, (long)getpid());
	file = fopen(temporary, "w");
	if (file == NULL) {
		snprintf(error, error_size, "无法创建跟踪目标请求");
		return -1;
	}
	write_failed = fputs(command, file) == EOF || fflush(file) != 0 ||
		fsync(fileno(file)) != 0;
	if (fclose(file) != 0)
		write_failed = 1;
	if (write_failed) {
		unlink(temporary);
		snprintf(error, error_size, "无法写入跟踪目标请求");
		return -1;
	}
	if (rename(temporary, TRACK_REQUEST_PATH) != 0) {
		unlink(temporary);
		snprintf(error, error_size, "无法提交跟踪目标请求");
		return -1;
	}
	return 0;
}

static int normalized_number(cJSON *parent, const char *name, double *value)
{
	cJSON *item = parent == NULL ? NULL :
		cJSON_GetObjectItemCaseSensitive(parent, name);

	if (!cJSON_IsNumber(item) || item->valuedouble < 0.0 ||
	    item->valuedouble > 1.0)
		return -1;
	*value = item->valuedouble;
	return 0;
}

int tracking_status_json(char *json, size_t size)
{
	char line[128] = "";
	char mode[32] = "";
	const char *state;
	FILE *file;
	int tracking_enabled;
	int detection_enabled;

	if (json == NULL || size == 0 ||
	    tracking_config_flags(&tracking_enabled, &detection_enabled) != 0)
		return -1;
	if (!tracking_enabled) {
		state = "disabled";
	} else {
		file = fopen(TRACK_STATUS_PATH, "r");
		if (file != NULL) {
			while (fgets(line, sizeof(line), file) != NULL) {
				if (sscanf(line, "mode=%31s", mode) == 1)
					break;
			}
			fclose(file);
		}
		if (strcmp(mode, "tracking") == 0)
			state = "tracking";
		else
			state = "waiting_target";
	}
	if (snprintf(json, size,
			"{\"enabled\":%s,\"state\":\"%s\","
			"\"source\":null,\"target_valid\":%s,\"score\":null,"
			"\"detection_linked\":%s,\"detection_paused\":%s,"
			"\"error\":null}",
			tracking_enabled ? "true" : "false", state,
			strcmp(state, "tracking") == 0 ? "true" : "false",
			tracking_enabled && detection_enabled ? "true" : "false",
			strcmp(state, "tracking") == 0 && detection_enabled ?
				"true" : "false") >= (int)size)
		return -1;
	return 0;
}

int tracking_target_set(const char *body, char *error, size_t error_size)
{
	char command[256];
	const char *source;
	cJSON *root = body == NULL ? NULL : cJSON_Parse(body);
	cJSON *source_item;
	cJSON *point;
	cJSON *box;
	cJSON *id;
	double x;
	double y;
	double width;
	double height;
	int tracking_enabled;
	int detection_enabled;
	int result = -1;

	if (!cJSON_IsObject(root)) {
		snprintf(error, error_size, "跟踪目标请求结构无效");
		goto done;
	}
	if (tracking_config_flags(&tracking_enabled, &detection_enabled) != 0 ||
	    !tracking_enabled) {
		snprintf(error, error_size, "单目标跟踪未启用");
		goto done;
	}
	source_item = cJSON_GetObjectItemCaseSensitive(root, "source");
	if (!cJSON_IsString(source_item) || source_item->valuestring == NULL) {
		snprintf(error, error_size, "缺少跟踪目标来源");
		goto done;
	}
	source = source_item->valuestring;
	if (strcmp(source, "detection") == 0) {
		id = cJSON_GetObjectItemCaseSensitive(root, "detection_id");
		if (!detection_enabled) {
			snprintf(error, error_size, "目标检测未启用");
			goto done;
		}
		if (!cJSON_IsNumber(id) || id->valuedouble != id->valueint ||
		    id->valueint <= 0) {
			snprintf(error, error_size, "检测目标 ID 无效");
			goto done;
		}
		snprintf(command, sizeof(command), "id %d\n", id->valueint);
	} else if (strcmp(source, "fastsam") == 0) {
		point = object_item(root, "point");
		box = object_item(root, "box");
		if (point != NULL && normalized_number(point, "x", &x) == 0 &&
		    normalized_number(point, "y", &y) == 0) {
			snprintf(command, sizeof(command), "fastsam point %d %d %d %d\n",
				(int)(x * TRACK_COORDINATE_SCALE + 0.5),
				(int)(y * TRACK_COORDINATE_SCALE + 0.5),
				TRACK_COORDINATE_SCALE, TRACK_COORDINATE_SCALE);
		} else if (box != NULL && normalized_number(box, "x", &x) == 0 &&
			   normalized_number(box, "y", &y) == 0 &&
			   normalized_number(box, "width", &width) == 0 &&
			   normalized_number(box, "height", &height) == 0 &&
			   width > 0.0 && height > 0.0 && x + width <= 1.0 &&
			   y + height <= 1.0) {
			snprintf(command, sizeof(command),
				"fastsam box %d %d %d %d %d %d\n",
				(int)(x * TRACK_COORDINATE_SCALE + 0.5),
				(int)(y * TRACK_COORDINATE_SCALE + 0.5),
				(int)((x + width) * TRACK_COORDINATE_SCALE + 0.5),
				(int)((y + height) * TRACK_COORDINATE_SCALE + 0.5),
				TRACK_COORDINATE_SCALE, TRACK_COORDINATE_SCALE);
		} else {
			snprintf(error, error_size, "FastSAM 点或框坐标无效");
			goto done;
		}
	} else if (strcmp(source, "color") == 0 || strcmp(source, "box") == 0) {
		box = object_item(root, "box");
		if (normalized_number(box, "x", &x) != 0 ||
		    normalized_number(box, "y", &y) != 0 ||
		    normalized_number(box, "width", &width) != 0 ||
		    normalized_number(box, "height", &height) != 0 ||
		    width <= 0.0 || height <= 0.0 || x + width > 1.0 ||
		    y + height > 1.0) {
			snprintf(error, error_size, "跟踪目标框坐标无效");
			goto done;
		}
		snprintf(command, sizeof(command), "%s box %d %d %d %d %d %d\n",
			strcmp(source, "color") == 0 ? "color" : "direct",
			(int)(x * TRACK_COORDINATE_SCALE + 0.5),
			(int)(y * TRACK_COORDINATE_SCALE + 0.5),
			(int)((x + width) * TRACK_COORDINATE_SCALE + 0.5),
			(int)((y + height) * TRACK_COORDINATE_SCALE + 0.5),
			TRACK_COORDINATE_SCALE, TRACK_COORDINATE_SCALE);
	} else {
		snprintf(error, error_size, "跟踪目标来源不受支持");
		goto done;
	}
	result = write_track_request(command, error, error_size);
done:
	cJSON_Delete(root);
	return result;
}

int tracking_target_clear(char *error, size_t error_size)
{
	return write_track_request("stop\n", error, error_size);
}
