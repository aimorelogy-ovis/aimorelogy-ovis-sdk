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
	{ "main_fps", "vencchn0", "dst_framerate", VALUE_INTEGER, 15, 30 },
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

	if (strcmp(id, "main_fps") != 0 && strcmp(id, "sub_fps") != 0)
		return 1;
	return fps == 15 || fps == 25 || fps == 30;
}

int config_validate_file(const char *path, char *error, size_t error_size)
{
	int found[sizeof(required_fields) / sizeof(required_fields[0])] = {0};
	char line[1024];
	char section[64] = "";
	FILE *file;
	size_t i;

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
	return 0;
}
