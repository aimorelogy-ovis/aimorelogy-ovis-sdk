#include "ovis_manager.h"
#include "cJSON.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct config_values {
	int main_fps;
	int main_bitrate;
	int sub_enabled;
	int sub_fps;
	int sub_bitrate;
	int osd_enabled;
	int person_enabled;
	double person_threshold;
	int face_enabled;
	double face_threshold;
	int motion_enabled;
	int motion_sensitivity;
	int human_pose_enabled;
	double human_pose_threshold;
	int object_tracking_enabled;
	int object_tracking_search_type;
	int object_tracking_use_kalman;
	double object_tracking_score_threshold;
};

struct ini_update {
	const char *section;
	const char *key;
	char value[160];
	int written;
};

static pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;

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

static int ensure_dir(const char *path)
{
	return mkdir(path, 0755) == 0 || errno == EEXIST ? 0 : -1;
}

static int copy_file(const char *source, const char *target)
{
	char buffer[4096];
	ssize_t count;
	int input = open(source, O_RDONLY);
	int output;

	if (input < 0)
		return -1;
	output = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (output < 0) {
		close(input);
		return -1;
	}
	while ((count = read(input, buffer, sizeof(buffer))) > 0) {
		ssize_t offset = 0;
		while (offset < count) {
			ssize_t written = write(output, buffer + offset, (size_t)(count - offset));
			if (written <= 0) {
				close(input);
				close(output);
				return -1;
			}
			offset += written;
		}
	}
	if (count < 0 || fsync(output) != 0) {
		close(input);
		close(output);
		return -1;
	}
	close(input);
	return close(output);
}

static int atomic_copy(const char *source, const char *target)
{
	char temporary[512];

	snprintf(temporary, sizeof(temporary), "%s.tmp", target);
	if (copy_file(source, temporary) != 0 || rename(temporary, target) != 0) {
		unlink(temporary);
		return -1;
	}
	return 0;
}

static int read_ini_value(const char *path, const char *wanted_section,
	const char *wanted_key, char *value, size_t size)
{
	char line[1024];
	char section[64] = "";
	FILE *file = fopen(path, "r");

	if (file == NULL)
		return -1;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *text = trim(line);
		char *equals;

		if (*text == '[') {
			char *close = strchr(text, ']');
			if (close != NULL) {
				*close = '\0';
				snprintf(section, sizeof(section), "%s", text + 1);
			}
			continue;
		}
		equals = strchr(text, '=');
		if (equals == NULL)
			continue;
		*equals = '\0';
		if (strcmp(section, wanted_section) == 0 && strcmp(trim(text), wanted_key) == 0) {
			char *comment;
			text = trim(equals + 1);
			comment = strchr(text, ';');
			if (comment != NULL)
				*comment = '\0';
			snprintf(value, size, "%s", trim(text));
			fclose(file);
			return 0;
		}
	}
	fclose(file);
	return -1;
}

static int read_int(const char *path, const char *section, const char *key, int *value)
{
	char text[64];
	char *end;
	long parsed;

	if (read_ini_value(path, section, key, text, sizeof(text)) != 0)
		return -1;
	errno = 0;
	parsed = strtol(text, &end, 10);
	if (errno != 0 || end == text || *trim(end) != '\0')
		return -1;
	*value = (int)parsed;
	return 0;
}

static int read_double(const char *path, const char *section, const char *key,
	double *value)
{
	char text[64];
	char *end;

	if (read_ini_value(path, section, key, text, sizeof(text)) != 0)
		return -1;
	errno = 0;
	*value = strtod(text, &end);
	return errno == 0 && end != text && *trim(end) == '\0' ? 0 : -1;
}

static int threshold_to_sensitivity(int threshold)
{
	return ((255 - threshold) * 100 + 127) / 255;
}

static int sensitivity_to_threshold(int sensitivity)
{
	return ((100 - sensitivity) * 255 + 50) / 100;
}

static int load_values(const char *path, struct config_values *values)
{
	int motion_threshold;

	memset(values, 0, sizeof(*values));
	if (read_int(path, "vencchn0", "dst_framerate", &values->main_fps) != 0 ||
	    read_int(path, "vencchn0", "bit_rate", &values->main_bitrate) != 0 ||
	    read_int(path, "vencchn1", "bEnable", &values->sub_enabled) != 0 ||
	    read_int(path, "vencchn1", "dst_framerate", &values->sub_fps) != 0 ||
	    read_int(path, "vencchn1", "bit_rate", &values->sub_bitrate) != 0 ||
	    read_int(path, "osdc_config", "enable", &values->osd_enabled) != 0 ||
	    read_int(path, "ai_pd_config", "pd_enable", &values->person_enabled) != 0 ||
	    read_double(path, "ai_pd_config", "threshold", &values->person_threshold) != 0 ||
	    read_int(path, "ai_fd_config", "fd_enable", &values->face_enabled) != 0 ||
	    read_double(path, "ai_fd_config", "threshold_fd", &values->face_threshold) != 0 ||
	    read_int(path, "ai_md_config", "md_enable", &values->motion_enabled) != 0 ||
	    read_int(path, "ai_md_config", "threshold", &motion_threshold) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "human_keypoint_enable",
		&values->human_pose_enabled) != 0 ||
	    read_double(path, "ai_human_keypoint_config", "threshold",
		&values->human_pose_threshold) != 0 ||
	    read_int(path, "ai_object_track_config", "object_track_enable",
		&values->object_tracking_enabled) != 0 ||
	    read_int(path, "ai_object_track_config", "search_type",
		&values->object_tracking_search_type) != 0 ||
	    read_int(path, "ai_object_track_config", "use_kalman",
		&values->object_tracking_use_kalman) != 0 ||
	    read_double(path, "ai_object_track_config", "tracking_score_threshold",
		&values->object_tracking_score_threshold) != 0)
		return -1;
	values->motion_sensitivity = threshold_to_sensitivity(motion_threshold);
	return 0;
}

static int revision_for_file(const char *path, char revision[17])
{
	unsigned char buffer[4096];
	uint64_t hash = UINT64_C(14695981039346656037);
	size_t count;
	FILE *file = fopen(path, "rb");

	if (file == NULL)
		return -1;
	while ((count = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		size_t index;
		for (index = 0; index < count; index++) {
			hash ^= buffer[index];
			hash *= UINT64_C(1099511628211);
		}
	}
	if (ferror(file)) {
		fclose(file);
		return -1;
	}
	fclose(file);
	snprintf(revision, 17, "%016llx", (unsigned long long)hash);
	return 0;
}

static int json_print(cJSON *root, char *json, size_t size)
{
	int result = cJSON_PrintPreallocated(root, json, (int)size, 0) ? 0 : -1;
	cJSON_Delete(root);
	return result;
}

static cJSON *values_to_json(const struct config_values *values)
{
	cJSON *root = cJSON_CreateObject();
	cJSON *video;
	cJSON *main_stream;
	cJSON *sub_stream;
	cJSON *overlay;
	cJSON *detection;
	cJSON *person;
	cJSON *face;
	cJSON *motion;
	cJSON *human_pose;
	cJSON *object_tracking;

	if (root == NULL)
		return NULL;
	video = cJSON_AddObjectToObject(root, "video");
	main_stream = cJSON_AddObjectToObject(video, "main");
	sub_stream = cJSON_AddObjectToObject(video, "sub");
	overlay = cJSON_AddObjectToObject(root, "overlay");
	detection = cJSON_AddObjectToObject(root, "detection");
	person = cJSON_AddObjectToObject(detection, "person");
	face = cJSON_AddObjectToObject(detection, "face");
	motion = cJSON_AddObjectToObject(detection, "motion");
	human_pose = cJSON_AddObjectToObject(detection, "human_pose");
	object_tracking = cJSON_AddObjectToObject(detection, "object_tracking");
	if (object_tracking == NULL) {
		cJSON_Delete(root);
		return NULL;
	}
	cJSON_AddStringToObject(main_stream, "profile", "1080p");
	cJSON_AddNumberToObject(main_stream, "fps", values->main_fps);
	cJSON_AddNumberToObject(main_stream, "bitrate_kbps", values->main_bitrate);
	cJSON_AddBoolToObject(sub_stream, "enabled", values->sub_enabled);
	cJSON_AddStringToObject(sub_stream, "profile", "768x572");
	cJSON_AddNumberToObject(sub_stream, "fps", values->sub_fps);
	cJSON_AddNumberToObject(sub_stream, "bitrate_kbps", values->sub_bitrate);
	cJSON_AddBoolToObject(overlay, "enabled", values->osd_enabled);
	cJSON_AddBoolToObject(person, "enabled", values->person_enabled);
	cJSON_AddNumberToObject(person, "threshold", values->person_threshold);
	cJSON_AddBoolToObject(face, "enabled", values->face_enabled);
	cJSON_AddNumberToObject(face, "threshold", values->face_threshold);
	cJSON_AddBoolToObject(motion, "enabled", values->motion_enabled);
	cJSON_AddNumberToObject(motion, "sensitivity", values->motion_sensitivity);
	cJSON_AddBoolToObject(human_pose, "enabled", values->human_pose_enabled);
	cJSON_AddNumberToObject(human_pose, "threshold", values->human_pose_threshold);
	cJSON_AddBoolToObject(object_tracking, "enabled", values->object_tracking_enabled);
	cJSON_AddStringToObject(object_tracking, "search_method",
		values->object_tracking_search_type == 3 ? "fastsam" : "color");
	cJSON_AddBoolToObject(object_tracking, "use_kalman",
		values->object_tracking_use_kalman);
	cJSON_AddNumberToObject(object_tracking, "score_threshold",
		values->object_tracking_score_threshold);
	return root;
}

int config_capabilities_json(char *json, size_t size)
{
	static const char capabilities[] =
		"{\"schema_version\":2,\"video\":{"
		"\"main\":{\"profiles\":[{\"id\":\"1080p\",\"width\":1920,\"height\":1080,"
		"\"fps_options\":[15,25,30,60],\"bitrate_min\":512,\"bitrate_max\":15000}]},"
		"\"sub\":{\"profiles\":[{\"id\":\"768x572\",\"width\":768,\"height\":572,"
		"\"fps_options\":[15,25,30],\"bitrate_min\":128,\"bitrate_max\":4000}]}},"
		"\"features\":{\"osd\":true,\"person_detection\":true,"
		"\"face_detection\":true,\"motion_detection\":true,"
		"\"human_pose\":true,\"object_tracking\":true},"
		"\"ai\":{\"max_active_tpu_features\":1,\"features\":["
		"{\"id\":\"person\",\"name\":\"人员检测\",\"model\":\"YOLOv8n Monitor Person\"},"
		"{\"id\":\"face\",\"name\":\"人脸检测\",\"model\":\"SCRFD\"},"
		"{\"id\":\"human_pose\",\"name\":\"人体姿态\",\"model\":\"YOLOv8 Pose\"},"
		"{\"id\":\"object_tracking\",\"name\":\"目标检测与跟踪\","
		"\"model\":\"YOLOv8n + FearTrack\",\"search_methods\":[\"color\",\"fastsam\"]}],"
		"\"motion_detection\":true}}";

	if (strlen(capabilities) + 1 > size)
		return -1;
	snprintf(json, size, "%s", capabilities);
	return 0;
}

int config_read_json(char *json, size_t size)
{
	struct config_values values;
	char revision[17];
	char error[256];
	cJSON *root;
	cJSON *values_json;

	if (config_validate_file(OVIS_CONFIG_FILE, error, sizeof(error)) != 0 ||
	    load_values(OVIS_CONFIG_FILE, &values) != 0 ||
	    revision_for_file(OVIS_CONFIG_FILE, revision) != 0)
		return -1;
	root = cJSON_CreateObject();
	values_json = values_to_json(&values);
	if (root == NULL || values_json == NULL) {
		cJSON_Delete(root);
		cJSON_Delete(values_json);
		return -1;
	}
	cJSON_AddStringToObject(root, "revision", revision);
	cJSON_AddItemToObject(root, "values", values_json);
	return json_print(root, json, size);
}

static cJSON *object_item(cJSON *parent, const char *name)
{
	cJSON *item = parent == NULL ? NULL : cJSON_GetObjectItemCaseSensitive(parent, name);
	return cJSON_IsObject(item) ? item : NULL;
}

static int bool_item(cJSON *parent, const char *name, int *value)
{
	cJSON *item = parent == NULL ? NULL : cJSON_GetObjectItemCaseSensitive(parent, name);
	if (!cJSON_IsBool(item))
		return -1;
	*value = cJSON_IsTrue(item) ? 1 : 0;
	return 0;
}

static int int_item(cJSON *parent, const char *name, int *value)
{
	cJSON *item = parent == NULL ? NULL : cJSON_GetObjectItemCaseSensitive(parent, name);
	if (!cJSON_IsNumber(item) || item->valuedouble != (double)item->valueint)
		return -1;
	*value = item->valueint;
	return 0;
}

static int double_item(cJSON *parent, const char *name, double *value)
{
	cJSON *item = parent == NULL ? NULL : cJSON_GetObjectItemCaseSensitive(parent, name);
	if (!cJSON_IsNumber(item))
		return -1;
	*value = item->valuedouble;
	return 0;
}

static int string_item(cJSON *parent, const char *name, const char **value)
{
	cJSON *item = parent == NULL ? NULL : cJSON_GetObjectItemCaseSensitive(parent, name);
	if (!cJSON_IsString(item) || item->valuestring == NULL)
		return -1;
	*value = item->valuestring;
	return 0;
}

static int parse_payload(const char *body, struct config_values *values,
	char revision[33], char *error, size_t error_size)
{
	cJSON *root = cJSON_Parse(body);
	cJSON *values_json;
	cJSON *video;
	cJSON *main_stream;
	cJSON *sub_stream;
	cJSON *overlay;
	cJSON *detection;
	cJSON *person;
	cJSON *face;
	cJSON *motion;
	cJSON *human_pose;
	cJSON *object_tracking;
	const char *revision_text;
	const char *main_profile;
	const char *sub_profile;
	const char *search_method;
	int result = -1;

	memset(values, 0, sizeof(*values));
	if (!cJSON_IsObject(root))
		goto done;
	values_json = object_item(root, "values");
	video = object_item(values_json, "video");
	main_stream = object_item(video, "main");
	sub_stream = object_item(video, "sub");
	overlay = object_item(values_json, "overlay");
	detection = object_item(values_json, "detection");
	person = object_item(detection, "person");
	face = object_item(detection, "face");
	motion = object_item(detection, "motion");
	human_pose = object_item(detection, "human_pose");
	object_tracking = object_item(detection, "object_tracking");
	if (string_item(root, "revision", &revision_text) != 0 || strlen(revision_text) > 32 ||
	    string_item(main_stream, "profile", &main_profile) != 0 ||
	    int_item(main_stream, "fps", &values->main_fps) != 0 ||
	    int_item(main_stream, "bitrate_kbps", &values->main_bitrate) != 0 ||
	    bool_item(sub_stream, "enabled", &values->sub_enabled) != 0 ||
	    string_item(sub_stream, "profile", &sub_profile) != 0 ||
	    int_item(sub_stream, "fps", &values->sub_fps) != 0 ||
	    int_item(sub_stream, "bitrate_kbps", &values->sub_bitrate) != 0 ||
	    bool_item(overlay, "enabled", &values->osd_enabled) != 0 ||
	    bool_item(person, "enabled", &values->person_enabled) != 0 ||
	    double_item(person, "threshold", &values->person_threshold) != 0 ||
	    bool_item(face, "enabled", &values->face_enabled) != 0 ||
	    double_item(face, "threshold", &values->face_threshold) != 0 ||
	    bool_item(motion, "enabled", &values->motion_enabled) != 0 ||
	    int_item(motion, "sensitivity", &values->motion_sensitivity) != 0 ||
	    bool_item(human_pose, "enabled", &values->human_pose_enabled) != 0 ||
	    double_item(human_pose, "threshold", &values->human_pose_threshold) != 0 ||
	    bool_item(object_tracking, "enabled", &values->object_tracking_enabled) != 0 ||
	    string_item(object_tracking, "search_method", &search_method) != 0 ||
	    bool_item(object_tracking, "use_kalman", &values->object_tracking_use_kalman) != 0 ||
	    double_item(object_tracking, "score_threshold",
		&values->object_tracking_score_threshold) != 0)
		goto done;
	if (strcmp(main_profile, "1080p") != 0 || strcmp(sub_profile, "768x572") != 0) {
		snprintf(error, error_size, "配置包含设备不支持的分辨率预设");
		goto done;
	}
	if (strcmp(search_method, "color") == 0)
		values->object_tracking_search_type = 2;
	else if (strcmp(search_method, "fastsam") == 0)
		values->object_tracking_search_type = 3;
	else {
		snprintf(error, error_size, "目标跟踪搜索方式不受支持");
		goto done;
	}
	snprintf(revision, 33, "%s", revision_text);
	result = 0;
done:
	if (result != 0 && error[0] == '\0')
		snprintf(error, error_size, "配置请求结构无效");
	cJSON_Delete(root);
	return result;
}

static void add_issue(cJSON *issues, const char *field, const char *code,
	const char *message)
{
	cJSON *issue = cJSON_CreateObject();
	cJSON_AddStringToObject(issue, "field", field);
	cJSON_AddStringToObject(issue, "code", code);
	cJSON_AddStringToObject(issue, "message", message);
	cJSON_AddItemToArray(issues, issue);
}

static int main_fps_supported(int fps)
{
	return fps == 15 || fps == 25 || fps == 30 || fps == 60;
}

static int sub_fps_supported(int fps)
{
	return fps == 15 || fps == 25 || fps == 30;
}

static cJSON *validate_values(const struct config_values *values)
{
	cJSON *errors = cJSON_CreateArray();
	int active_tpu_features = values->person_enabled + values->face_enabled +
		values->human_pose_enabled + values->object_tracking_enabled;

	if (!main_fps_supported(values->main_fps))
		add_issue(errors, "video.main.fps", "UNSUPPORTED_FPS", "主码流不支持此帧率");
	if (values->main_bitrate < 512 || values->main_bitrate > 15000)
		add_issue(errors, "video.main.bitrate_kbps", "OUT_OF_RANGE", "主码流码率范围为 512-15000 Kbps");
	if (!sub_fps_supported(values->sub_fps))
		add_issue(errors, "video.sub.fps", "UNSUPPORTED_FPS", "子码流不支持此帧率");
	if (values->sub_bitrate < 128 || values->sub_bitrate > 4000)
		add_issue(errors, "video.sub.bitrate_kbps", "OUT_OF_RANGE", "子码流码率范围为 128-4000 Kbps");
	if (values->person_threshold < 0 || values->person_threshold > 1)
		add_issue(errors, "detection.person.threshold", "OUT_OF_RANGE", "人员检测阈值必须在 0 到 1 之间");
	if (values->face_threshold < 0 || values->face_threshold > 1)
		add_issue(errors, "detection.face.threshold", "OUT_OF_RANGE", "人脸检测阈值必须在 0 到 1 之间");
	if (values->motion_sensitivity < 0 || values->motion_sensitivity > 100)
		add_issue(errors, "detection.motion.sensitivity", "OUT_OF_RANGE", "移动检测灵敏度必须在 0 到 100 之间");
	if (values->human_pose_threshold < 0 || values->human_pose_threshold > 1)
		add_issue(errors, "detection.human_pose.threshold", "OUT_OF_RANGE", "人体姿态阈值必须在 0 到 1 之间");
	if (values->object_tracking_score_threshold < 0 ||
	    values->object_tracking_score_threshold > 1)
		add_issue(errors, "detection.object_tracking.score_threshold", "OUT_OF_RANGE", "目标跟踪分数阈值必须在 0 到 1 之间");
	if (active_tpu_features > 1)
		add_issue(errors, "detection", "AI_FEATURE_CONFLICT", "人员、人脸、人体姿态和目标跟踪最多只能启用一项");
	return errors;
}

int config_validate_json(const char *body, char *json, size_t size,
	char *error, size_t error_size)
{
	struct config_values values;
	char requested_revision[33];
	char active_revision[17];
	cJSON *root;
	cJSON *errors;
	cJSON *warnings;
	cJSON *requires;
	int valid;

	error[0] = '\0';
	if (parse_payload(body, &values, requested_revision, error, error_size) != 0)
		return -1;
	if (revision_for_file(OVIS_CONFIG_FILE, active_revision) != 0) {
		snprintf(error, error_size, "无法读取当前配置版本");
		return -3;
	}
	if (strcmp(requested_revision, active_revision) != 0) {
		snprintf(error, error_size, "配置已被其他操作修改，请重新读取");
		return -2;
	}
	root = cJSON_CreateObject();
	errors = validate_values(&values);
	warnings = cJSON_CreateArray();
	requires = cJSON_CreateArray();
	if (root == NULL || errors == NULL || warnings == NULL || requires == NULL) {
		cJSON_Delete(root);
		cJSON_Delete(errors);
		cJSON_Delete(warnings);
		cJSON_Delete(requires);
		snprintf(error, error_size, "内存不足");
		return -3;
	}
	valid = cJSON_GetArraySize(errors) == 0;
	cJSON_AddBoolToObject(root, "valid", valid);
	cJSON_AddItemToObject(root, "errors", errors);
	cJSON_AddItemToObject(root, "warnings", warnings);
	if (valid)
		cJSON_AddItemToArray(requires, cJSON_CreateString("ipcamera_restart"));
	cJSON_AddItemToObject(root, "requires", requires);
	if (json_print(root, json, size) != 0) {
		snprintf(error, error_size, "校验响应过大");
		return -3;
	}
	return valid ? 0 : 1;
}

static int write_updates(const char *source, const char *target,
	struct ini_update *updates, size_t update_count)
{
	char line[1024];
	char parse_line[1024];
	char section[64] = "";
	char temporary[512];
	FILE *input;
	FILE *output;
	size_t index;
	int result = -1;

	snprintf(temporary, sizeof(temporary), "%s.tmp", target);
	input = fopen(source, "r");
	output = fopen(temporary, "w");
	if (input == NULL || output == NULL)
		goto done;
	while (fgets(line, sizeof(line), input) != NULL) {
		char *text;
		char *equals;
		snprintf(parse_line, sizeof(parse_line), "%s", line);
		text = trim(parse_line);
		if (*text == '[') {
			char *close = strchr(text, ']');
			if (close != NULL) {
				*close = '\0';
				snprintf(section, sizeof(section), "%s", text + 1);
			}
		}
		equals = strchr(text, '=');
		if (equals != NULL) {
			*equals = '\0';
			text = trim(text);
			for (index = 0; index < update_count; index++) {
				if (strcmp(section, updates[index].section) == 0 &&
				    strcmp(text, updates[index].key) == 0) {
					fprintf(output, "%-16s = %s\n", updates[index].key, updates[index].value);
					updates[index].written = 1;
					goto next_line;
				}
			}
		}
		fputs(line, output);
	next_line: ;
	}
	for (index = 0; index < update_count; index++) {
		if (!updates[index].written)
			goto done;
	}
	if (fflush(output) != 0 || fsync(fileno(output)) != 0)
		goto done;
	if (fclose(output) != 0) {
		output = NULL;
		goto done;
	}
	output = NULL;
	if (rename(temporary, target) != 0)
		goto done;
	result = 0;
done:
	if (input != NULL)
		fclose(input);
	if (output != NULL)
		fclose(output);
	if (result != 0)
		unlink(temporary);
	return result;
}

static int has_section(const char *path, const char *wanted_section)
{
	char line[1024];
	FILE *file = fopen(path, "r");
	int found = 0;

	if (file == NULL)
		return 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *text = trim(line);
		if (*text == '[') {
			char *close = strchr(text, ']');
			if (close != NULL) {
				*close = '\0';
				if (strcmp(text + 1, wanted_section) == 0) {
					found = 1;
					break;
				}
			}
		}
	}
	fclose(file);
	return found;
}

static int append_missing_runtime_sections(const char *path)
{
	int need_human_pose = !has_section(path, "ai_human_keypoint_config");
	int need_object_tracking = !has_section(path, "ai_object_track_config");
	int need_sot_pool = !has_section(path, "vb_pool_7");
	int need_uvc_pool = !has_section(path, "vb_pool_8");
	int need_uvc_group = !has_section(path, "vpssgrp6");
	int need_uvc_channel = !has_section(path, "vpssgrp6.chn0");
	FILE *file;

	if (!need_human_pose && !need_object_tracking && !need_sot_pool &&
	    !need_uvc_pool && !need_uvc_group && !need_uvc_channel)
		return 0;
	file = fopen(path, "a");
	if (file == NULL)
		return -1;
	if (need_human_pose) {
		fputs("\n[ai_human_keypoint_config]\n"
			"human_keypoint_enable = 0\n"
			"vpss_grp          = 0\n"
			"vpss_chn          = 2\n"
			"model_width       = 640\n"
			"model_height      = 384\n"
			"model_id          = TDL_MODEL_KEYPOINT_YOLOV8POSE_PERSON17\n"
			"model_path        = \"/usr/share/ipcamera/cv184x/keypoint_yolov8pose_person17_384_640_INT8_cv184x.bmodel\"\n"
			"vpssPreProcSkip   = 0\n"
			"threshold         = 0.5\n", file);
	}
	if (need_object_tracking) {
		fputs("\n[ai_object_track_config]\n"
			"object_track_enable = 0\n"
			"vpss_grp          = 0\n"
			"vpss_chn          = 2\n"
			"grp_width         = 640\n"
			"grp_height        = 384\n"
			"sot_vpss_grp      = 0\n"
			"sot_vpss_chn      = 2\n"
			"sot_grp_width     = 1920\n"
			"sot_grp_height    = 1080\n"
			"det_input_preprocessed = 1\n"
			"sot_refine_selected_det = 0\n"
			"model_id_det      = TDL_MODEL_YOLOV8N_DET_PERSON_VEHICLE\n"
			"model_id_sot      = TDL_MODEL_TRACKING_FEARTRACK\n"
			"model_path_det    = \"/usr/share/ipcamera/cv184x/yolov8n_det_person_vehicle_384_640_INT8_cv184x.bmodel\"\n"
			"model_path_sot    = \"/usr/share/ipcamera/cv184x/tracking_feartrack_128_128_256_256_INT8_cv184x.bmodel\"\n"
			"model_path_sam    = \"/usr/share/ipcamera/cv184x/fastsam_seg_320_320_INT8_cv184x.bmodel\"\n"
			"model_path_cfg    = \"/usr/share/ipcamera/model_factory.json\"\n"
			"threshold_occluded = 0.1\n"
			"threshold_reappear = 2.0\n"
			"search_type       = 3\n"
			"use_kalman        = 1\n"
			"tracking_score_threshold = 0.5\n"
			"debug_log_enable  = 0\n", file);
	}
	if (need_sot_pool) {
		fputs("\n[vb_pool_7]\n"
			"bEnable         = 1\n"
			"frame_width     = 1920\n"
			"frame_height    = 1080\n"
			"frame_fmt       = PIXEL_FORMAT_NV12\n"
			"data_bitwidth   = DATA_BITWIDTH_8\n"
			"compress_mode   = COMPRESS_MODE_NONE\n"
			"blk_cnt         = 4\n"
			"mem_size        = 0\n", file);
	}
	if (need_uvc_pool) {
		fputs("\n[vb_pool_8]\n"
			"bEnable         = 1\n"
			"frame_width     = 1920\n"
			"frame_height    = 1080\n"
			"frame_fmt       = PIXEL_FORMAT_NV12\n"
			"data_bitwidth   = DATA_BITWIDTH_8\n"
			"compress_mode   = COMPRESS_MODE_NONE\n"
			"blk_cnt         = 4\n"
			"mem_size        = 0\n", file);
	}
	if (need_uvc_group) {
		fputs("\n[vpssgrp6]\n"
			"group_id        = 6\n"
			"grp_enable      = 1\n"
			"pixel_fmt       = PIXEL_FORMAT_NV12\n"
			"src_framerate   = -1\n"
			"dst_framerate   = -1\n"
			"vpss_dev        = 0\n"
			"max_w           = 1920\n"
			"max_h           = 1080\n"
			"chn_cnt         = 1\n"
			"crop_en         = 0\n"
			"crop_coor       = VPSS_CROP_RATIO_COOR\n"
			"crop_rect_x     = 0\n"
			"crop_rect_y     = 0\n"
			"crop_rect_w     = 0\n"
			"crop_rect_h     = 0\n"
			"bind_mode       = 1\n"
			"src_mod_id      = CVI_ID_VPSS\n"
			"src_dev_id      = 0\n"
			"src_chn_id      = 0\n"
			"dst_mod_id      = CVI_ID_VPSS\n"
			"dst_dev_id      = 6\n"
			"dst_chn_id      = 0\n", file);
	}
	if (need_uvc_channel) {
		fputs("\n[vpssgrp6.chn0]\n"
			"chn_enable      = 1\n"
			"width           = 1920\n"
			"height          = 1080\n"
			"video_fmt       = VIDEO_FORMAT_LINEAR\n"
			"chn_pixel_fmt   = PIXEL_FORMAT_NV12\n"
			"src_framerate   = 30\n"
			"dst_framerate   = 30\n"
			"depth           = 0\n"
			"mirror          = 0\n"
			"filp            = 0\n"
			"aspectratio     = ASPECT_RATIO_NONE\n"
			"s32x            = 0\n"
			"s32y            = 0\n"
			"rec_width       = 0\n"
			"rec_heigh       = 0\n"
			"en_color        = 1\n"
			"color           = 0\n"
			"normalize       = 0\n"
			"crop_en         = 0\n"
			"crop_coor       = VPSS_CROP_RATIO_COOR\n"
			"crop_rect_x     = 0\n"
			"crop_rect_y     = 0\n"
			"crop_rect_w     = 0\n"
			"crop_rect_h     = 0\n"
			"attach_en       = 1\n"
			"attach_pool     = 8\n", file);
	}
	if (fflush(file) != 0 || fsync(fileno(file)) != 0) {
		fclose(file);
		return -1;
	}
	return fclose(file);
}

static int migrate_runtime_config(const char *path)
{
	struct ini_update updates[] = {
		{ "ai_pd_config", "pd_enable", "", 0 },
		{ "ai_pd_config", "model_path", "\"/usr/share/ipcamera/cv184x/yolov8n_det_monitor_person_256_448_INT8_cv184x.bmodel\"", 0 },
		{ "ai_fd_config", "fd_enable", "", 0 },
		{ "ai_fd_config", "model_path_fd", "\"/usr/share/ipcamera/cv184x/scrfd_det_face_432_768_INT8_cv184x.bmodel\"", 0 },
		{ "ai_human_keypoint_config", "human_keypoint_enable", "", 0 },
		{ "ai_human_keypoint_config", "model_path", "\"/usr/share/ipcamera/cv184x/keypoint_yolov8pose_person17_384_640_INT8_cv184x.bmodel\"", 0 },
		{ "ai_object_track_config", "object_track_enable", "", 0 },
		{ "ai_object_track_config", "model_path_det", "\"/usr/share/ipcamera/cv184x/yolov8n_det_person_vehicle_384_640_INT8_cv184x.bmodel\"", 0 },
		{ "ai_object_track_config", "model_path_sot", "\"/usr/share/ipcamera/cv184x/tracking_feartrack_128_128_256_256_INT8_cv184x.bmodel\"", 0 },
		{ "ai_object_track_config", "model_path_sam", "\"/usr/share/ipcamera/cv184x/fastsam_seg_320_320_INT8_cv184x.bmodel\"", 0 },
		{ "ai_object_track_config", "model_path_cfg", "\"/usr/share/ipcamera/model_factory.json\"", 0 },
		{ "ai_object_track_config", "grp_width", "640", 0 },
		{ "ai_object_track_config", "grp_height", "384", 0 },
		{ "ai_object_track_config", "sot_vpss_grp", "0", 0 },
		{ "ai_object_track_config", "sot_vpss_chn", "2", 0 },
		{ "ai_object_track_config", "sot_grp_width", "1920", 0 },
		{ "ai_object_track_config", "sot_grp_height", "1080", 0 },
		{ "ai_object_track_config", "det_input_preprocessed", "1", 0 },
		{ "ai_object_track_config", "sot_refine_selected_det", "0", 0 },
		{ "vpssgrp0.chn0", "depth", "0", 0 },
		{ "vpssgrp0.chn2", "src_framerate", "-1", 0 },
		{ "vpssgrp0.chn2", "dst_framerate", "-1", 0 },
		{ "vb_config", "vb_pool_cnt", "9", 0 },
		{ "vb_pool_5", "blk_cnt", "4", 0 },
		{ "vpssgrp2", "grp_enable", "", 0 },
		{ "vpssgrp3", "grp_enable", "", 0 },
		{ "vpssgrp4", "grp_enable", "", 0 },
		{ "vpssgrp5", "grp_enable", "", 0 },
		{ "vpssgrp0.chn1", "chn_enable", "", 0 },
		{ "vencchn2", "bEnable", "", 0 },
		{ "osdc_config1", "bShow", "", 0 },
		{ "vpssgrp0.chn2", "chn_enable", "", 0 },
		{ "vpssgrp6.chn0", "src_framerate", "", 0 },
		{ "vpssgrp0.chn2", "width", "640", 0 },
		{ "vpssgrp0.chn2", "height", "384", 0 },
		{ "vpssgrp0.chn2", "chn_pixel_fmt", "PIXEL_FORMAT_UINT8_C3_PLANAR", 0 },
		{ "vpssgrp0.chn2", "depth", "1", 0 },
		{ "vpssgrp0.chn2", "attach_en", "1", 0 },
		{ "vpssgrp0.chn2", "attach_pool", "1", 0 },
		{ "vb_pool_5", "frame_width", "1920", 0 },
		{ "vb_pool_5", "frame_height", "1080", 0 },
		{ "vb_pool_5", "frame_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vb_pool_1", "frame_width", "640", 0 },
		{ "vb_pool_1", "frame_height", "384", 0 },
		{ "vb_pool_1", "frame_fmt", "PIXEL_FORMAT_UINT8_C3_PLANAR", 0 },
		{ "vb_pool_1", "blk_cnt", "4", 0 },
		{ "vb_pool_7", "frame_width", "1920", 0 },
		{ "vb_pool_7", "frame_height", "1080", 0 },
		{ "vb_pool_7", "frame_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vb_pool_7", "blk_cnt", "4", 0 },
		{ "vb_pool_6", "blk_cnt", "4", 0 },
		{ "vb_pool_8", "bEnable", "1", 0 },
		{ "vb_pool_8", "frame_width", "1920", 0 },
		{ "vb_pool_8", "frame_height", "1080", 0 },
		{ "vb_pool_8", "frame_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vb_pool_8", "data_bitwidth", "DATA_BITWIDTH_8", 0 },
		{ "vb_pool_8", "compress_mode", "COMPRESS_MODE_NONE", 0 },
		{ "vb_pool_8", "blk_cnt", "4", 0 },
		{ "vb_pool_8", "mem_size", "0", 0 },
		{ "vpss_config", "vpss_grp", "7", 0 },
		{ "vpssgrp1", "chn_cnt", "1", 0 },
		{ "vpssgrp6", "group_id", "6", 0 },
		{ "vpssgrp6", "grp_enable", "1", 0 },
		{ "vpssgrp6", "pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp6", "src_framerate", "-1", 0 },
		{ "vpssgrp6", "dst_framerate", "-1", 0 },
		{ "vpssgrp6", "vpss_dev", "0", 0 },
		{ "vpssgrp6", "max_w", "1920", 0 },
		{ "vpssgrp6", "max_h", "1080", 0 },
		{ "vpssgrp6", "chn_cnt", "1", 0 },
		{ "vpssgrp6", "crop_en", "0", 0 },
		{ "vpssgrp6", "crop_coor", "VPSS_CROP_RATIO_COOR", 0 },
		{ "vpssgrp6", "crop_rect_x", "0", 0 },
		{ "vpssgrp6", "crop_rect_y", "0", 0 },
		{ "vpssgrp6", "crop_rect_w", "0", 0 },
		{ "vpssgrp6", "crop_rect_h", "0", 0 },
		{ "vpssgrp6", "bind_mode", "1", 0 },
		{ "vpssgrp6", "src_mod_id", "CVI_ID_VPSS", 0 },
		{ "vpssgrp6", "src_dev_id", "0", 0 },
		{ "vpssgrp6", "src_chn_id", "0", 0 },
		{ "vpssgrp6", "dst_mod_id", "CVI_ID_VPSS", 0 },
		{ "vpssgrp6", "dst_dev_id", "6", 0 },
		{ "vpssgrp6", "dst_chn_id", "0", 0 },
		{ "vpssgrp6.chn0", "chn_enable", "1", 0 },
		{ "vpssgrp6.chn0", "width", "1920", 0 },
		{ "vpssgrp6.chn0", "height", "1080", 0 },
		{ "vpssgrp6.chn0", "video_fmt", "VIDEO_FORMAT_LINEAR", 0 },
		{ "vpssgrp6.chn0", "chn_pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp6.chn0", "dst_framerate", "30", 0 },
		{ "vpssgrp6.chn0", "depth", "0", 0 },
		{ "vpssgrp6.chn0", "mirror", "0", 0 },
		{ "vpssgrp6.chn0", "filp", "0", 0 },
		{ "vpssgrp6.chn0", "aspectratio", "ASPECT_RATIO_NONE", 0 },
		{ "vpssgrp6.chn0", "s32x", "0", 0 },
		{ "vpssgrp6.chn0", "s32y", "0", 0 },
		{ "vpssgrp6.chn0", "rec_width", "0", 0 },
		{ "vpssgrp6.chn0", "rec_heigh", "0", 0 },
		{ "vpssgrp6.chn0", "en_color", "1", 0 },
		{ "vpssgrp6.chn0", "color", "0", 0 },
		{ "vpssgrp6.chn0", "normalize", "0", 0 },
		{ "vpssgrp6.chn0", "crop_en", "0", 0 },
		{ "vpssgrp6.chn0", "crop_coor", "VPSS_CROP_RATIO_COOR", 0 },
		{ "vpssgrp6.chn0", "crop_rect_x", "0", 0 },
		{ "vpssgrp6.chn0", "crop_rect_y", "0", 0 },
		{ "vpssgrp6.chn0", "crop_rect_w", "0", 0 },
		{ "vpssgrp6.chn0", "crop_rect_h", "0", 0 },
		{ "vpssgrp6.chn0", "attach_en", "1", 0 },
		{ "vpssgrp6.chn0", "attach_pool", "8", 0 },
		{ "vencchn3", "src_dev_id", "6", 0 },
		{ "vencchn3", "src_chn_id", "0", 0 },
		{ "vencchn3", "vpss_grp", "6", 0 },
		{ "vencchn3", "vpss_chn", "0", 0 },
		{ "vencchn3", "src_framerate", "30", 0 },
		{ "vencchn3", "dst_framerate", "30", 0 },
	};
	char migrated[512];
	char value[160];
	int enabled[4] = {0};
	int motion_enabled = 0;
	int group_enabled[4];
	int sub_enabled = 0;
	int osd_enabled = 0;
	int main_fps = 0;
	int ai_source_enabled = 0;
	int sub_dependent_enabled[3];
	int keep = -1;
	int index;
	int needs_update;
	enum {
		AI_GROUP_UPDATE_BASE = 24,
		AI_GROUP_UPDATE_COUNT = 4,
		SUB_UPDATE_BASE = 28,
		SUB_UPDATE_COUNT = 3,
		AI_SOURCE_UPDATE_INDEX = 31,
		FIXED_TOPOLOGY_UPDATE_BASE = 32,
		UVC_SOURCE_FPS_UPDATE_INDEX = 32,
	};
	static const int fixed_update_indexes[] = {
		1, 3, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23
	};

	if (access(path, F_OK) != 0)
		return -1;
	needs_update = !has_section(path, "ai_human_keypoint_config") ||
		!has_section(path, "ai_object_track_config") ||
		!has_section(path, "vb_pool_7") ||
		!has_section(path, "vb_pool_8") ||
		!has_section(path, "vpssgrp6") ||
		!has_section(path, "vpssgrp6.chn0");
	if (append_missing_runtime_sections(path) != 0)
		return -1;
	if (read_int(path, "ai_pd_config", "pd_enable", &enabled[0]) != 0 ||
	    read_int(path, "ai_fd_config", "fd_enable", &enabled[1]) != 0 ||
	    read_int(path, "ai_md_config", "md_enable", &motion_enabled) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "human_keypoint_enable", &enabled[2]) != 0 ||
	    read_int(path, "ai_object_track_config", "object_track_enable", &enabled[3]) != 0 ||
	    read_int(path, "vencchn0", "dst_framerate", &main_fps) != 0 ||
	    read_int(path, "vencchn1", "bEnable", &sub_enabled) != 0 ||
	    read_int(path, "osdc_config", "enable", &osd_enabled) != 0)
		return -1;
	if (enabled[0] + enabled[1] + enabled[2] + enabled[3] > 1)
		needs_update = 1;
	snprintf(updates[UVC_SOURCE_FPS_UPDATE_INDEX].value,
		sizeof(updates[UVC_SOURCE_FPS_UPDATE_INDEX].value), "%d",
		main_fps == 60 ? 60 : 30);
	for (index = 0;
	     index < (int)(sizeof(fixed_update_indexes) /
		     sizeof(fixed_update_indexes[0]));
	     index++) {
		const struct ini_update *update = &updates[fixed_update_indexes[index]];
		if (read_ini_value(path, update->section, update->key,
				value, sizeof(value)) != 0 || strcmp(value, update->value) != 0) {
			needs_update = 1;
			break;
		}
	}
	for (index = FIXED_TOPOLOGY_UPDATE_BASE;
	     index < (int)(sizeof(updates) / sizeof(updates[0])); index++) {
		const struct ini_update *update =
			&updates[index];

		if (read_ini_value(path, update->section, update->key,
				value, sizeof(value)) != 0 ||
		    strcmp(value, update->value) != 0) {
			needs_update = 1;
			break;
		}
	}
	for (index = 0; index < 4; index++) {
		if (enabled[index] && keep < 0)
			keep = index;
		enabled[index] = enabled[index] && keep == index;
	}
	group_enabled[0] = enabled[0];
	group_enabled[1] = enabled[1];
	group_enabled[2] = motion_enabled;
	group_enabled[3] = 0;
	ai_source_enabled = enabled[0] || enabled[1] || motion_enabled ||
		enabled[2] || enabled[3];
	for (index = 0; index < AI_GROUP_UPDATE_COUNT; index++) {
		int current;

		if (read_int(path, updates[AI_GROUP_UPDATE_BASE + index].section,
				updates[AI_GROUP_UPDATE_BASE + index].key, &current) != 0 ||
		    current != group_enabled[index]) {
			needs_update = 1;
			break;
		}
	}
	sub_dependent_enabled[0] = sub_enabled;
	sub_dependent_enabled[1] = sub_enabled;
	sub_dependent_enabled[2] = sub_enabled && osd_enabled;
	for (index = 0; index < SUB_UPDATE_COUNT; index++) {
		int current;

		if (read_int(path, updates[SUB_UPDATE_BASE + index].section,
				updates[SUB_UPDATE_BASE + index].key, &current) != 0 ||
		    current != sub_dependent_enabled[index]) {
			needs_update = 1;
			break;
		}
	}
	{
		int current;

		if (read_int(path, updates[AI_SOURCE_UPDATE_INDEX].section,
				updates[AI_SOURCE_UPDATE_INDEX].key, &current) != 0 ||
		    current != ai_source_enabled)
			needs_update = 1;
	}
	if (!needs_update)
		return 0;
	snprintf(updates[0].value, sizeof(updates[0].value), "%d", enabled[0]);
	snprintf(updates[2].value, sizeof(updates[2].value), "%d", enabled[1]);
	snprintf(updates[4].value, sizeof(updates[4].value), "%d", enabled[2]);
	snprintf(updates[6].value, sizeof(updates[6].value), "%d", enabled[3]);
	for (index = 0; index < AI_GROUP_UPDATE_COUNT; index++)
		snprintf(updates[AI_GROUP_UPDATE_BASE + index].value,
			sizeof(updates[AI_GROUP_UPDATE_BASE + index].value),
			"%d", group_enabled[index]);
	for (index = 0; index < SUB_UPDATE_COUNT; index++)
		snprintf(updates[SUB_UPDATE_BASE + index].value,
			sizeof(updates[SUB_UPDATE_BASE + index].value),
			"%d", sub_dependent_enabled[index]);
	snprintf(updates[AI_SOURCE_UPDATE_INDEX].value,
		sizeof(updates[AI_SOURCE_UPDATE_INDEX].value), "%d",
		ai_source_enabled);
	snprintf(migrated, sizeof(migrated), "%s.migrated", path);
	if (write_updates(path, migrated, updates, sizeof(updates) / sizeof(updates[0])) != 0)
		return -1;
	if (rename(migrated, path) != 0) {
		unlink(migrated);
		return -1;
	}
	return 0;
}

static int stage_values(const struct config_values *values, char revision[17],
	char *error, size_t error_size)
{
	struct ini_update updates[] = {
		{ "vencchn0", "dst_framerate", "", 0 },
		{ "vencchn0", "bit_rate", "", 0 },
		{ "vencchn0", "max_bitrate", "", 0 },
		{ "vencchn1", "bEnable", "", 0 },
		{ "vencchn1", "dst_framerate", "", 0 },
		{ "vencchn1", "bit_rate", "", 0 },
		{ "vencchn1", "max_bitrate", "", 0 },
		{ "osdc_config", "enable", "", 0 },
		{ "ai_pd_config", "pd_enable", "", 0 },
		{ "ai_pd_config", "threshold", "", 0 },
		{ "ai_fd_config", "fd_enable", "", 0 },
		{ "ai_fd_config", "threshold_fd", "", 0 },
		{ "ai_md_config", "md_enable", "", 0 },
		{ "ai_md_config", "threshold", "", 0 },
		{ "ai_human_keypoint_config", "human_keypoint_enable", "", 0 },
		{ "ai_human_keypoint_config", "threshold", "", 0 },
		{ "ai_object_track_config", "object_track_enable", "", 0 },
		{ "ai_object_track_config", "search_type", "", 0 },
		{ "ai_object_track_config", "use_kalman", "", 0 },
		{ "ai_object_track_config", "tracking_score_threshold", "", 0 },
		{ "sensor_config0", "sns_type", "", 0 },
		{ "vencchn0", "src_framerate", "", 0 },
		{ "vpssgrp2", "grp_enable", "", 0 },
		{ "vpssgrp3", "grp_enable", "", 0 },
		{ "vpssgrp4", "grp_enable", "", 0 },
		{ "vpssgrp5", "grp_enable", "", 0 },
		{ "vpssgrp0.chn1", "chn_enable", "", 0 },
		{ "vencchn2", "bEnable", "", 0 },
		{ "osdc_config1", "bShow", "", 0 },
		{ "vpssgrp0.chn2", "chn_enable", "", 0 },
		{ "vpssgrp6.chn0", "src_framerate", "", 0 },
	};
	char validation_error[256];
	enum {
		STAGE_AI_SOURCE_UPDATE_INDEX = 29,
		STAGE_UVC_SOURCE_FPS_UPDATE_INDEX = 30,
	};

	snprintf(updates[0].value, sizeof(updates[0].value), "%d", values->main_fps);
	snprintf(updates[1].value, sizeof(updates[1].value), "%d", values->main_bitrate);
	snprintf(updates[2].value, sizeof(updates[2].value), "%d", values->main_bitrate);
	snprintf(updates[3].value, sizeof(updates[3].value), "%d", values->sub_enabled);
	snprintf(updates[4].value, sizeof(updates[4].value), "%d", values->sub_fps);
	snprintf(updates[5].value, sizeof(updates[5].value), "%d", values->sub_bitrate);
	snprintf(updates[6].value, sizeof(updates[6].value), "%d", values->sub_bitrate);
	snprintf(updates[7].value, sizeof(updates[7].value), "%d", values->osd_enabled);
	snprintf(updates[8].value, sizeof(updates[8].value), "%d", values->person_enabled);
	snprintf(updates[9].value, sizeof(updates[9].value), "%.6g", values->person_threshold);
	snprintf(updates[10].value, sizeof(updates[10].value), "%d", values->face_enabled);
	snprintf(updates[11].value, sizeof(updates[11].value), "%.6g", values->face_threshold);
	snprintf(updates[12].value, sizeof(updates[12].value), "%d", values->motion_enabled);
	snprintf(updates[13].value, sizeof(updates[13].value), "%d",
		sensitivity_to_threshold(values->motion_sensitivity));
	snprintf(updates[14].value, sizeof(updates[14].value), "%d", values->human_pose_enabled);
	snprintf(updates[15].value, sizeof(updates[15].value), "%.6g", values->human_pose_threshold);
	snprintf(updates[16].value, sizeof(updates[16].value), "%d", values->object_tracking_enabled);
	snprintf(updates[17].value, sizeof(updates[17].value), "%d", values->object_tracking_search_type);
	snprintf(updates[18].value, sizeof(updates[18].value), "%d", values->object_tracking_use_kalman);
	snprintf(updates[19].value, sizeof(updates[19].value), "%.6g",
		values->object_tracking_score_threshold);
	snprintf(updates[20].value, sizeof(updates[20].value), "%s",
		values->main_fps == 60 ? OVIS_SC235HAI_60FPS_SNS_TYPE : OVIS_SC235HAI_30FPS_SNS_TYPE);
	snprintf(updates[21].value, sizeof(updates[21].value), "%d",
		values->main_fps == 60 ? 60 : 30);
	snprintf(updates[22].value, sizeof(updates[22].value), "%d", values->person_enabled);
	snprintf(updates[23].value, sizeof(updates[23].value), "%d", values->face_enabled);
	snprintf(updates[24].value, sizeof(updates[24].value), "%d", values->motion_enabled);
	snprintf(updates[25].value, sizeof(updates[25].value), "%d", 0);
	snprintf(updates[26].value, sizeof(updates[26].value), "%d", values->sub_enabled);
	snprintf(updates[27].value, sizeof(updates[27].value), "%d", values->sub_enabled);
	snprintf(updates[28].value, sizeof(updates[28].value), "%d",
		values->sub_enabled && values->osd_enabled);
	snprintf(updates[STAGE_AI_SOURCE_UPDATE_INDEX].value,
		sizeof(updates[STAGE_AI_SOURCE_UPDATE_INDEX].value), "%d",
		values->person_enabled || values->face_enabled ||
		values->motion_enabled || values->human_pose_enabled ||
		values->object_tracking_enabled);
	snprintf(updates[STAGE_UVC_SOURCE_FPS_UPDATE_INDEX].value,
		sizeof(updates[STAGE_UVC_SOURCE_FPS_UPDATE_INDEX].value), "%d",
		values->main_fps == 60 ? 60 : 30);
	if (write_updates(OVIS_CONFIG_FILE, OVIS_CONFIG_PENDING, updates,
			sizeof(updates) / sizeof(updates[0])) != 0) {
		snprintf(error, error_size, "无法创建待应用配置");
		return -1;
	}
	if (config_validate_file(OVIS_CONFIG_PENDING, validation_error,
			sizeof(validation_error)) != 0) {
		unlink(OVIS_CONFIG_PENDING);
		snprintf(error, error_size, "待应用配置校验失败: %s", validation_error);
		return -1;
	}
	if (revision_for_file(OVIS_CONFIG_PENDING, revision) != 0) {
		unlink(OVIS_CONFIG_PENDING);
		snprintf(error, error_size, "无法生成待应用配置版本");
		return -1;
	}
	return 0;
}

int config_stage_json(const char *body, char *json, size_t size,
	char *error, size_t error_size)
{
	struct config_values values;
	char requested_revision[33];
	char active_revision[17];
	char staged_revision[17];
	cJSON *issues;
	int result = -3;

	error[0] = '\0';
	if (parse_payload(body, &values, requested_revision, error, error_size) != 0)
		return -1;
	issues = validate_values(&values);
	if (issues == NULL) {
		snprintf(error, error_size, "内存不足");
		return -3;
	}
	if (cJSON_GetArraySize(issues) != 0) {
		cJSON_Delete(issues);
		snprintf(error, error_size, "配置校验未通过");
		return -1;
	}
	cJSON_Delete(issues);
	pthread_mutex_lock(&config_lock);
	if (revision_for_file(OVIS_CONFIG_FILE, active_revision) != 0) {
		snprintf(error, error_size, "无法读取当前配置版本");
		goto done;
	}
	if (strcmp(requested_revision, active_revision) != 0) {
		snprintf(error, error_size, "配置已被其他操作修改，请重新读取");
		result = -2;
		goto done;
	}
	if (stage_values(&values, staged_revision, error, error_size) != 0)
		goto done;
	if (snprintf(json, size, "{\"saved\":true,\"revision\":\"%s\","
			"\"restart_required\":true}", staged_revision) >= (int)size) {
		snprintf(error, error_size, "保存响应过大");
		unlink(OVIS_CONFIG_PENDING);
		goto done;
	}
	result = 0;
	audit_log("config.stage", "success");
done:
	pthread_mutex_unlock(&config_lock);
	return result;
}

static int apply_staged_locked(const char *revision, char *message,
	size_t message_size, int *rolled_back)
{
	char staged_revision[17];
	char validation_error[256];
	char service_output[512];
	char rollback_output[512];
	int restart_result;
	int rollback_result;

	*rolled_back = 0;
	if (config_validate_file(OVIS_CONFIG_PENDING, validation_error,
			sizeof(validation_error)) != 0 ||
	    revision_for_file(OVIS_CONFIG_PENDING, staged_revision) != 0) {
		snprintf(message, message_size, "待应用配置不存在或已损坏");
		return -1;
	}
	if (strcmp(revision, staged_revision) != 0) {
		snprintf(message, message_size, "待应用配置版本不匹配");
		return -1;
	}
	if (atomic_copy(OVIS_CONFIG_FILE, OVIS_CONFIG_BACKUP) != 0) {
		snprintf(message, message_size, "备份当前配置失败");
		return -1;
	}
	if (atomic_copy(OVIS_CONFIG_PENDING, OVIS_CONFIG_FILE) != 0) {
		snprintf(message, message_size, "切换到新配置失败");
		return -1;
	}
	unlink(OVIS_CONFIG_PENDING);
	restart_result = service_run_action(SERVICE_RESTART, service_output,
		sizeof(service_output));
	if (restart_result == 0) {
		snprintf(message, message_size, "配置应用成功");
		audit_log("config.apply", "success");
		return 0;
	}
	rollback_result = atomic_copy(OVIS_CONFIG_BACKUP, OVIS_CONFIG_FILE);
	if (rollback_result == 0)
		rollback_result = service_run_action(SERVICE_RESTART, rollback_output,
			sizeof(rollback_output));
	if (rollback_result == 0) {
		*rolled_back = 1;
		snprintf(message, message_size, "新配置启动失败，已恢复原配置");
		audit_log("config.apply", "failed_rolled_back");
	} else {
		snprintf(message, message_size, "新配置启动失败，自动回滚也失败，请通过串口检查设备");
		audit_log("config.apply", "failed_rollback_failed");
	}
	return -1;
}

int config_apply_staged(const char *revision, char *message, size_t message_size,
	int *rolled_back)
{
	int result;
	pthread_mutex_lock(&config_lock);
	result = apply_staged_locked(revision, message, message_size, rolled_back);
	pthread_mutex_unlock(&config_lock);
	return result;
}

int config_apply_defaults(char *message, size_t message_size, int *rolled_back)
{
	char revision[17];
	char validation_error[256];
	int result;

	pthread_mutex_lock(&config_lock);
	if (config_validate_file(OVIS_DEFAULT_CONFIG, validation_error,
			sizeof(validation_error)) != 0) {
		snprintf(message, message_size, "默认配置不可用: %s", validation_error);
		pthread_mutex_unlock(&config_lock);
		return -1;
	}
	if (atomic_copy(OVIS_DEFAULT_CONFIG, OVIS_CONFIG_PENDING) != 0 ||
	    revision_for_file(OVIS_CONFIG_PENDING, revision) != 0) {
		unlink(OVIS_CONFIG_PENDING);
		snprintf(message, message_size, "无法准备默认配置");
		pthread_mutex_unlock(&config_lock);
		return -1;
	}
	result = apply_staged_locked(revision, message, message_size, rolled_back);
	if (result == 0)
		snprintf(message, message_size, "已恢复默认配置");
	pthread_mutex_unlock(&config_lock);
	return result;
}

int config_ensure_runtime(char *error, size_t error_size)
{
	int migration_result = 0;

	ensure_dir("/mnt/cfg");
	if (ensure_dir(OVIS_CONFIG_DIR) != 0) {
		snprintf(error, error_size, "无法创建运行配置目录");
		return -1;
	}
	unlink(OVIS_CONFIG_PENDING);
	if (access(OVIS_CONFIG_FILE, F_OK) == 0)
		migration_result = migrate_runtime_config(OVIS_CONFIG_FILE);
	if (migration_result == 0 &&
	    config_validate_file(OVIS_CONFIG_FILE, error, error_size) == 0)
		return 0;
	if (access(OVIS_CONFIG_FILE, F_OK) == 0)
		rename(OVIS_CONFIG_FILE, OVIS_CONFIG_FILE ".corrupt");
	if (access(OVIS_CONFIG_BACKUP, F_OK) == 0)
		migration_result = migrate_runtime_config(OVIS_CONFIG_BACKUP);
	else
		migration_result = -1;
	if (migration_result == 0 &&
	    config_validate_file(OVIS_CONFIG_BACKUP, error, error_size) == 0 &&
	    atomic_copy(OVIS_CONFIG_BACKUP, OVIS_CONFIG_FILE) == 0)
		return 0;
	if (config_validate_file(OVIS_DEFAULT_CONFIG, error, error_size) == 0 &&
	    atomic_copy(OVIS_DEFAULT_CONFIG, OVIS_CONFIG_FILE) == 0)
		return 0;
	snprintf(error, error_size, "没有可用的默认或备份配置");
	return -1;
}
