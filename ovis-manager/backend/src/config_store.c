#include "ovis_manager.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum value_type { VALUE_INTEGER, VALUE_DECIMAL };
struct config_field {
	const char *id;
	const char *section;
	const char *key;
	enum value_type type;
	double minimum;
	double maximum;
};
extern const struct config_field ovis_config_fields[];
extern const size_t ovis_config_field_count;
int config_validate_value(const struct config_field *field, const char *text);

static char *trim(char *text)
{
	char *end;
	while (isspace((unsigned char)*text)) text++;
	end = text + strlen(text);
	while (end > text && isspace((unsigned char)end[-1])) *--end = '\0';
	return text;
}

static int ensure_dir(const char *path)
{
	if (mkdir(path, 0755) == 0 || errno == EEXIST)
		return 0;
	return -1;
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
				close(input); close(output); return -1;
			}
			offset += written;
		}
	}
	if (count < 0 || fsync(output) != 0) {
		close(input); close(output); return -1;
	}
	close(input);
	return close(output);
}

static int atomic_copy(const char *source, const char *target)
{
	char temporary[256];
	snprintf(temporary, sizeof(temporary), "%s.tmp", target);
	if (copy_file(source, temporary) != 0 || rename(temporary, target) != 0) {
		unlink(temporary);
		return -1;
	}
	return 0;
}

static int find_json_value(const char *body, const char *id, char *value, size_t size)
{
	char needle[96];
	const char *start;
	const char *end;

	snprintf(needle, sizeof(needle), "\"%s\"", id);
	start = strstr(body, needle);
	if (start == NULL)
		return 0;
	start = strchr(start + strlen(needle), ':');
	if (start == NULL)
		return -1;
	start++;
	while (isspace((unsigned char)*start)) start++;
	end = start;
	while (*end && *end != ',' && *end != '}' && !isspace((unsigned char)*end)) end++;
	if (end == start || (size_t)(end - start) >= size)
		return -1;
	memcpy(value, start, (size_t)(end - start));
	value[end - start] = '\0';
	return 1;
}

static int validate_json_keys(const char *body, char *error, size_t error_size)
{
	const char *cursor = body;

	while ((cursor = strchr(cursor, '"')) != NULL) {
		const char *end = strchr(cursor + 1, '"');
		const char *after;
		size_t length;
		size_t i;
		int known = 0;

		if (end == NULL) { snprintf(error, error_size, "JSON 字段名未闭合"); return -1; }
		after = end + 1;
		while (isspace((unsigned char)*after)) after++;
		if (*after != ':') { snprintf(error, error_size, "JSON 对象格式无效"); return -1; }
		length = (size_t)(end - cursor - 1);
		for (i = 0; i < ovis_config_field_count; i++) {
			if (strlen(ovis_config_fields[i].id) == length &&
			    strncmp(cursor + 1, ovis_config_fields[i].id, length) == 0) {
				known = 1;
				break;
			}
		}
		if (!known) { snprintf(error, error_size, "包含不受支持的配置字段"); return -1; }
		cursor = end + 1;
	}
	return 0;
}

static int read_value(const char *path, const struct config_field *field, char *value, size_t size)
{
	char line[1024];
	char section[64] = "";
	FILE *file = fopen(path, "r");

	if (file == NULL) return -1;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *text = trim(line);
		char *equals;
		if (*text == '[') {
			char *close = strchr(text, ']');
			if (close) { *close = '\0'; snprintf(section, sizeof(section), "%s", text + 1); }
			continue;
		}
		equals = strchr(text, '=');
		if (!equals) continue;
		*equals = '\0';
		if (strcmp(section, field->section) == 0 && strcmp(trim(text), field->key) == 0) {
			char *end;
			text = trim(equals + 1);
			end = strchr(text, ';');
			if (end) *end = '\0';
			snprintf(value, size, "%s", trim(text));
			fclose(file);
			return 0;
		}
	}
	fclose(file);
	return -1;
}

int config_read_json(char *json, size_t size)
{
	size_t used = 0;
	size_t i;
	char value[64];

	if (config_validate_file(OVIS_CONFIG_FILE, value, sizeof(value)) != 0)
		return -1;
	used += (size_t)snprintf(json + used, size - used, "{\"restart_required\":true,\"values\":{");
	for (i = 0; i < ovis_config_field_count; i++) {
		if (read_value(OVIS_CONFIG_FILE, &ovis_config_fields[i], value, sizeof(value)) != 0)
			return -1;
		used += (size_t)snprintf(json + used, size - used, "%s\"%s\":%s",
			i ? "," : "", ovis_config_fields[i].id, value);
		if (used >= size) return -1;
	}
	snprintf(json + used, size - used, "}}");
	return 0;
}

int config_update_json(const char *body, char *error, size_t error_size)
{
	char values[32][64] = {{0}};
	int changed[32] = {0};
	char line[1024], parse_line[1024], section[64] = "";
	const char *source = OVIS_CONFIG_FILE;
	const char *temporary = OVIS_CONFIG_FILE ".new";
	FILE *input, *output;
	size_t i;
	int any = 0;

	if (body == NULL || *body != '{') { snprintf(error, error_size, "请求体不是 JSON 对象"); return -1; }
	if (validate_json_keys(body, error, error_size) != 0) return -1;
	for (i = 0; i < ovis_config_field_count; i++) {
		int rc = find_json_value(body, ovis_config_fields[i].id, values[i], sizeof(values[i]));
		if (rc < 0 || (rc > 0 && config_validate_value(&ovis_config_fields[i], values[i]) != 0)) {
			snprintf(error, error_size, "%s 的值无效", ovis_config_fields[i].id); return -1;
		}
		changed[i] = rc;
		any |= rc;
	}
	if (!any) { snprintf(error, error_size, "没有可更新的配置项"); return -1; }
	input = fopen(source, "r");
	output = fopen(temporary, "w");
	if (!input || !output) { if (input) fclose(input); if (output) fclose(output); snprintf(error, error_size, "无法创建临时配置"); return -1; }
	while (fgets(line, sizeof(line), input) != NULL) {
		char *text, *equals;
		snprintf(parse_line, sizeof(parse_line), "%s", line);
		text = trim(parse_line);
		if (*text == '[') {
			char *close = strchr(text, ']');
			if (close) { *close = '\0'; snprintf(section, sizeof(section), "%s", text + 1); }
		}
		equals = strchr(text, '=');
		if (equals) {
			*equals = '\0'; text = trim(text);
			for (i = 0; i < ovis_config_field_count; i++) {
				if (changed[i] && strcmp(section, ovis_config_fields[i].section) == 0 && strcmp(text, ovis_config_fields[i].key) == 0) {
					fprintf(output, "%-16s = %s\n", ovis_config_fields[i].key, values[i]);
					goto next_line;
				}
			}
		}
		fputs(line, output);
	next_line: ;
	}
	fclose(input);
	if (fflush(output) != 0 || fsync(fileno(output)) != 0) {
		fclose(output);
		unlink(temporary); snprintf(error, error_size, "临时配置落盘失败"); return -1;
	}
	if (fclose(output) != 0) { unlink(temporary); snprintf(error, error_size, "关闭临时配置失败"); return -1; }
	if (config_validate_file(temporary, error, error_size) != 0) { unlink(temporary); return -1; }
	if (atomic_copy(OVIS_CONFIG_FILE, OVIS_CONFIG_BACKUP) != 0 || rename(temporary, OVIS_CONFIG_FILE) != 0) {
		unlink(temporary); snprintf(error, error_size, "原子替换配置失败"); return -1;
	}
	audit_log("config.update", "success");
	return 0;
}

int config_reset(char *error, size_t error_size)
{
	if (config_validate_file(OVIS_DEFAULT_CONFIG, error, error_size) != 0)
		return -1;
	if (access(OVIS_CONFIG_FILE, F_OK) == 0 && atomic_copy(OVIS_CONFIG_FILE, OVIS_CONFIG_BACKUP) != 0) {
		snprintf(error, error_size, "备份当前配置失败"); return -1;
	}
	if (atomic_copy(OVIS_DEFAULT_CONFIG, OVIS_CONFIG_FILE) != 0) {
		snprintf(error, error_size, "恢复出厂配置失败"); return -1;
	}
	audit_log("config.reset", "success");
	return 0;
}

int config_ensure_runtime(char *error, size_t error_size)
{
	ensure_dir("/mnt/cfg");
	if (ensure_dir(OVIS_CONFIG_DIR) != 0) { snprintf(error, error_size, "无法创建运行配置目录"); return -1; }
	if (config_validate_file(OVIS_CONFIG_FILE, error, error_size) == 0)
		return 0;
	if (access(OVIS_CONFIG_FILE, F_OK) == 0)
		rename(OVIS_CONFIG_FILE, OVIS_CONFIG_FILE ".corrupt");
	if (config_validate_file(OVIS_CONFIG_BACKUP, error, error_size) == 0 && atomic_copy(OVIS_CONFIG_BACKUP, OVIS_CONFIG_FILE) == 0)
		return 0;
	if (config_validate_file(OVIS_DEFAULT_CONFIG, error, error_size) == 0 && atomic_copy(OVIS_DEFAULT_CONFIG, OVIS_CONFIG_FILE) == 0)
		return 0;
	snprintf(error, error_size, "没有可用的默认或备份配置");
	return -1;
}
