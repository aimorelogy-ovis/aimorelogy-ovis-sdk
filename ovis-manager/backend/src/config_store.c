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
};

struct ini_update {
	const char *section;
	const char *key;
	char value[32];
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
	    read_int(path, "ai_md_config", "threshold", &motion_threshold) != 0)
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
	if (motion == NULL) {
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
	return root;
}

int config_capabilities_json(char *json, size_t size)
{
	static const char capabilities[] =
		"{\"schema_version\":1,\"video\":{"
		"\"main\":{\"profiles\":[{\"id\":\"1080p\",\"width\":1920,\"height\":1080,"
		"\"fps_options\":[15,25,30],\"bitrate_min\":512,\"bitrate_max\":15000}]},"
		"\"sub\":{\"profiles\":[{\"id\":\"768x572\",\"width\":768,\"height\":572,"
		"\"fps_options\":[15,25,30],\"bitrate_min\":128,\"bitrate_max\":4000}]}},"
		"\"features\":{\"osd\":true,\"person_detection\":true,"
		"\"face_detection\":true,\"motion_detection\":true}}";

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
	const char *revision_text;
	const char *main_profile;
	const char *sub_profile;
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
	    int_item(motion, "sensitivity", &values->motion_sensitivity) != 0)
		goto done;
	if (strcmp(main_profile, "1080p") != 0 || strcmp(sub_profile, "768x572") != 0) {
		snprintf(error, error_size, "配置包含设备不支持的分辨率预设");
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

static int fps_supported(int fps)
{
	return fps == 15 || fps == 25 || fps == 30;
}

static cJSON *validate_values(const struct config_values *values)
{
	cJSON *errors = cJSON_CreateArray();

	if (!fps_supported(values->main_fps))
		add_issue(errors, "video.main.fps", "UNSUPPORTED_FPS", "主码流不支持此帧率");
	if (values->main_bitrate < 512 || values->main_bitrate > 15000)
		add_issue(errors, "video.main.bitrate_kbps", "OUT_OF_RANGE", "主码流码率范围为 512-15000 Kbps");
	if (!fps_supported(values->sub_fps))
		add_issue(errors, "video.sub.fps", "UNSUPPORTED_FPS", "子码流不支持此帧率");
	if (values->sub_bitrate < 128 || values->sub_bitrate > 4000)
		add_issue(errors, "video.sub.bitrate_kbps", "OUT_OF_RANGE", "子码流码率范围为 128-4000 Kbps");
	if (values->person_threshold < 0 || values->person_threshold > 1)
		add_issue(errors, "detection.person.threshold", "OUT_OF_RANGE", "人员检测阈值必须在 0 到 1 之间");
	if (values->face_threshold < 0 || values->face_threshold > 1)
		add_issue(errors, "detection.face.threshold", "OUT_OF_RANGE", "人脸检测阈值必须在 0 到 1 之间");
	if (values->motion_sensitivity < 0 || values->motion_sensitivity > 100)
		add_issue(errors, "detection.motion.sensitivity", "OUT_OF_RANGE", "移动检测灵敏度必须在 0 到 100 之间");
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
	};
	char validation_error[256];

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
	ensure_dir("/mnt/cfg");
	if (ensure_dir(OVIS_CONFIG_DIR) != 0) {
		snprintf(error, error_size, "无法创建运行配置目录");
		return -1;
	}
	unlink(OVIS_CONFIG_PENDING);
	if (config_validate_file(OVIS_CONFIG_FILE, error, error_size) == 0)
		return 0;
	if (access(OVIS_CONFIG_FILE, F_OK) == 0)
		rename(OVIS_CONFIG_FILE, OVIS_CONFIG_FILE ".corrupt");
	if (config_validate_file(OVIS_CONFIG_BACKUP, error, error_size) == 0 &&
	    atomic_copy(OVIS_CONFIG_BACKUP, OVIS_CONFIG_FILE) == 0)
		return 0;
	if (config_validate_file(OVIS_DEFAULT_CONFIG, error, error_size) == 0 &&
	    atomic_copy(OVIS_DEFAULT_CONFIG, OVIS_CONFIG_FILE) == 0)
		return 0;
	snprintf(error, error_size, "没有可用的默认或备份配置");
	return -1;
}
