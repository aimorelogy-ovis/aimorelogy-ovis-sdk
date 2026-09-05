#include "ovis_manager.h"
#include "cJSON.h"
#include "ini_snapshot.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/file.h>
#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct config_values {
	int ai_bnr_enabled;
	int ai_bnr_present;
	int rtsp_enabled;
	int uvc_enabled;
	int display_enabled;
	int display_present;
	char display_mode[24];
	int main_fps;
	int main_bitrate;
	int sub_enabled;
	int sub_fps;
	int sub_bitrate;
	int osd_enabled;
	int overlay_text_present;
	int text_enabled;
	int text_main_enabled;
	int text_sub_enabled;
	char text_content[64];
	char text_position[24];
	int text_x;
	int text_y;
	uint32_t text_color;
	int overlay_detection_present;
	int detection_osd_enabled;
	char detection_color_mode[16];
	uint32_t detection_color;
	int detection_thickness;
	char detection_label_mode[24];
	int overlay_tracking_present;
	int tracking_osd_enabled;
	uint32_t tracking_color;
	uint32_t tracking_lost_color;
	int tracking_thickness;
	int overlay_reticle_present;
	int reticle_enabled;
	char reticle_template[24];
	uint32_t reticle_idle_color;
	uint32_t reticle_ready_color;
	int reticle_thickness;
	int reticle_show_while_tracking;
	int object_enabled;
	double object_threshold;
	int object_width;
	int object_height;
	int face_enabled;
	double face_threshold;
	int face_width;
	int face_height;
	int motion_enabled;
	int motion_sensitivity;
	int motion_width;
	int motion_height;
	int human_pose_enabled;
	double human_pose_threshold;
	int human_pose_width;
	int human_pose_height;
	int object_tracking_enabled;
	int object_tracking_search_type;
	int object_tracking_use_kalman;
	double object_tracking_score_threshold;
	int object_tracking_det_width;
	int object_tracking_det_height;
	int object_tracking_sot_width;
	int object_tracking_sot_height;
	char object_model_id[96];
	char object_model_path[512];
	int object_model_update;
};

struct ini_update {
	const char *section;
	const char *key;
	char value[160];
	int written;
};

static pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;
static __thread int config_snapshot_active;

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
	FILE *file;

	if (config_snapshot_active && ini_snapshot_begin(path) != 0)
		return -1;
	file = config_snapshot_active ? ini_snapshot_open(path) : fopen(path, "r");

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

static int read_uint32(const char *path, const char *section, const char *key,
	uint32_t *value)
{
	char text[64];
	char *end;
	unsigned long parsed;

	if (read_ini_value(path, section, key, text, sizeof(text)) != 0)
		return -1;
	errno = 0;
	parsed = strtoul(text, &end, 0);
	if (errno != 0 || end == text || *trim(end) != '\0' ||
	    parsed > UINT32_MAX)
		return -1;
	*value = (uint32_t)parsed;
	return 0;
}

static void unquote(char *text)
{
	size_t length = strlen(text);

	if (length >= 2 && text[0] == '"' && text[length - 1] == '"') {
		memmove(text, text + 1, length - 2);
		text[length - 2] = '\0';
	}
}

static void overlay_defaults(struct config_values *values)
{
	values->text_enabled = 0;
	values->text_main_enabled = 1;
	values->text_sub_enabled = 0;
	values->text_x = 20;
	values->text_y = 20;
	values->text_color = 0xffffff;
	snprintf(values->text_position, sizeof(values->text_position), "top-left");
	values->detection_osd_enabled = 1;
	snprintf(values->detection_color_mode,
		sizeof(values->detection_color_mode), "fixed");
	values->detection_color = 0x00d9ff;
	values->detection_thickness = 2;
	snprintf(values->detection_label_mode,
		sizeof(values->detection_label_mode), "none");
	values->tracking_osd_enabled = 1;
	values->tracking_color = 0xffb000;
	values->tracking_lost_color = 0xff3030;
	values->tracking_thickness = 3;
	values->reticle_enabled = 1;
	snprintf(values->reticle_template,
		sizeof(values->reticle_template), "corners");
	values->reticle_idle_color = 0xffffff;
	values->reticle_ready_color = 0xffc247;
	values->reticle_thickness = 2;
	values->reticle_show_while_tracking = 0;
}

static void load_overlay_values(const char *path, struct config_values *values)
{
	char text[160];

	overlay_defaults(values);
	if (read_int(path, "osd_style", "text_enabled",
			&values->text_enabled) != 0) {
		read_int(path, "osdc0_obj_info1", "bShow",
			&values->text_main_enabled);
		read_int(path, "osdc1_obj_info1", "bShow",
			&values->text_sub_enabled);
		values->text_enabled = values->text_main_enabled ||
			values->text_sub_enabled;
	} else {
		read_int(path, "osd_style", "text_main_enabled",
			&values->text_main_enabled);
		read_int(path, "osd_style", "text_sub_enabled",
			&values->text_sub_enabled);
	}
	if (read_ini_value(path, "osdc0_obj_info1", "str", text,
			sizeof(text)) == 0) {
		unquote(text);
		snprintf(values->text_content, sizeof(values->text_content), "%s", text);
	}
	read_int(path, "osdc0_obj_info1", "x1", &values->text_x);
	read_int(path, "osdc0_obj_info1", "y1", &values->text_y);
	read_uint32(path, "osdc0_obj_info1", "color", &values->text_color);
	if (read_ini_value(path, "osd_style", "text_position", text,
			sizeof(text)) == 0)
		snprintf(values->text_position, sizeof(values->text_position), "%s", text);
	read_int(path, "osd_style", "detection_enabled",
		&values->detection_osd_enabled);
	if (read_ini_value(path, "osd_style", "detection_color_mode", text,
			sizeof(text)) == 0)
		snprintf(values->detection_color_mode,
			sizeof(values->detection_color_mode), "%s", text);
	read_uint32(path, "osd_style", "detection_color", &values->detection_color);
	read_int(path, "osd_style", "detection_thickness",
		&values->detection_thickness);
	if (read_ini_value(path, "osd_style", "detection_label_mode", text,
			sizeof(text)) == 0)
		snprintf(values->detection_label_mode,
			sizeof(values->detection_label_mode), "%s", text);
	read_int(path, "osd_style", "tracking_enabled",
		&values->tracking_osd_enabled);
	read_uint32(path, "osd_style", "tracking_color", &values->tracking_color);
	read_uint32(path, "osd_style", "tracking_lost_color",
		&values->tracking_lost_color);
	read_int(path, "osd_style", "tracking_thickness",
		&values->tracking_thickness);
	read_int(path, "osd_style", "reticle_enabled", &values->reticle_enabled);
	if (read_ini_value(path, "osd_style", "reticle_template", text,
			sizeof(text)) == 0)
		snprintf(values->reticle_template,
			sizeof(values->reticle_template), "%s", text);
	read_uint32(path, "osd_style", "reticle_idle_color",
		&values->reticle_idle_color);
	read_uint32(path, "osd_style", "reticle_ready_color",
		&values->reticle_ready_color);
	read_int(path, "osd_style", "reticle_thickness",
		&values->reticle_thickness);
	read_int(path, "osd_style", "reticle_show_while_tracking",
		&values->reticle_show_while_tracking);
	values->overlay_text_present = 1;
	values->overlay_detection_present = 1;
	values->overlay_tracking_present = 1;
	values->overlay_reticle_present = 1;
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
	char display_panel[64];

	memset(values, 0, sizeof(*values));
	if (read_int(path, "vi_cfg_isp0", "teaisp_bnr_enable",
			&values->ai_bnr_enabled) != 0 ||
	    read_int(path, "output_config", "rtsp_enable", &values->rtsp_enabled) != 0 ||
	    read_int(path, "output_config", "uvc_enable", &values->uvc_enabled) != 0 ||
	    read_int(path, "display_config", "vo_cnt", &values->display_enabled) != 0 ||
	    read_ini_value(path, "display_config_0", "panel_type", display_panel,
			sizeof(display_panel)) != 0 ||
	    read_int(path, "vencchn0", "dst_framerate", &values->main_fps) != 0 ||
	    read_int(path, "vencchn0", "bit_rate", &values->main_bitrate) != 0 ||
	    read_int(path, "output_config", "sub_enable", &values->sub_enabled) != 0 ||
	    read_int(path, "vencchn1", "dst_framerate", &values->sub_fps) != 0 ||
	    read_int(path, "vencchn1", "bit_rate", &values->sub_bitrate) != 0 ||
	    read_int(path, "osdc_config", "enable", &values->osd_enabled) != 0 ||
	    read_int(path, "ai_pd_config", "pd_enable", &values->object_enabled) != 0 ||
	    read_double(path, "ai_pd_config", "threshold", &values->object_threshold) != 0 ||
	    read_int(path, "ai_pd_config", "grp_width", &values->object_width) != 0 ||
	    read_int(path, "ai_pd_config", "grp_height", &values->object_height) != 0 ||
	    read_int(path, "ai_fd_config", "fd_enable", &values->face_enabled) != 0 ||
	    read_double(path, "ai_fd_config", "threshold_fd", &values->face_threshold) != 0 ||
	    read_int(path, "ai_fd_config", "grp_width", &values->face_width) != 0 ||
	    read_int(path, "ai_fd_config", "grp_height", &values->face_height) != 0 ||
	    read_int(path, "ai_md_config", "md_enable", &values->motion_enabled) != 0 ||
	    read_int(path, "ai_md_config", "threshold", &motion_threshold) != 0 ||
	    read_int(path, "ai_md_config", "grp_width", &values->motion_width) != 0 ||
	    read_int(path, "ai_md_config", "grp_height", &values->motion_height) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "human_keypoint_enable",
		&values->human_pose_enabled) != 0 ||
	    read_double(path, "ai_human_keypoint_config", "threshold",
		&values->human_pose_threshold) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "model_width",
		&values->human_pose_width) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "model_height",
		&values->human_pose_height) != 0 ||
	    read_int(path, "ai_object_track_config", "object_track_enable",
		&values->object_tracking_enabled) != 0 ||
	    read_int(path, "ai_object_track_config", "search_type",
		&values->object_tracking_search_type) != 0 ||
	    read_int(path, "ai_object_track_config", "use_kalman",
		&values->object_tracking_use_kalman) != 0 ||
	    read_double(path, "ai_object_track_config", "sot_min_observed_score",
		&values->object_tracking_score_threshold) != 0 ||
	    read_int(path, "ai_object_track_config", "grp_width",
		&values->object_tracking_det_width) != 0 ||
	    read_int(path, "ai_object_track_config", "grp_height",
		&values->object_tracking_det_height) != 0 ||
	    read_int(path, "ai_object_track_config", "sot_grp_width",
		&values->object_tracking_sot_width) != 0 ||
	    read_int(path, "ai_object_track_config", "sot_grp_height",
		&values->object_tracking_sot_height) != 0 ||
	    read_ini_value(path, "ai_pd_config", "model_id", values->object_model_id,
		sizeof(values->object_model_id)) != 0 ||
	    read_ini_value(path, "ai_pd_config", "model_path", values->object_model_path,
		sizeof(values->object_model_path)) != 0)
		return -1;
	if ((values->display_enabled != 0 && values->display_enabled != 1) ||
	    strcmp(display_panel, OVIS_DISPLAY_PANEL_TYPE) != 0)
		return -1;
	values->display_present = 1;
	snprintf(values->display_mode, sizeof(values->display_mode), "%s",
		OVIS_DISPLAY_MODE);
	if (values->object_model_path[0] == '"') {
		size_t length = strlen(values->object_model_path);
		memmove(values->object_model_path, values->object_model_path + 1, length);
		if (length > 1 && values->object_model_path[length - 2] == '"')
			values->object_model_path[length - 2] = '\0';
	}
	values->motion_sensitivity = threshold_to_sensitivity(motion_threshold);
	values->ai_bnr_present = 1;
	load_overlay_values(path, values);
	return 0;
}

int config_ai_bnr_supported(void)
{
#if defined(OVIS_AI_BNR_SUPPORT) && defined(OVIS_AI_BNR_CERTIFIED)
	int default_enabled;

	return read_int(OVIS_DEFAULT_CONFIG, "vi_cfg_isp0", "teaisp_bnr_enable",
			&default_enabled) == 0 &&
		(default_enabled == 0 || default_enabled == 1) &&
		access(OVIS_AI_BNR_MODEL, R_OK) == 0 &&
		access(OVIS_AI_BNR_PQ_BIN, R_OK) == 0;
#else
	return 0;
#endif
}

int config_get_output_flags(int *rtsp_enabled, int *uvc_enabled)
{
	if (rtsp_enabled == NULL || uvc_enabled == NULL)
		return -1;
	if (read_int(OVIS_CONFIG_FILE, "output_config", "rtsp_enable",
			rtsp_enabled) != 0 ||
	    read_int(OVIS_CONFIG_FILE, "output_config", "uvc_enable",
			uvc_enabled) != 0) {
		*rtsp_enabled = 0;
		*uvc_enabled = 1;
		return -1;
	}
	if (!((*rtsp_enabled == 0 && *uvc_enabled == 1) ||
	      (*rtsp_enabled == 1 && *uvc_enabled == 0))) {
		*rtsp_enabled = 0;
		*uvc_enabled = 1;
		return -1;
	}
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

static cJSON *add_processing_size(cJSON *parent, const char *name,
	int width, int height)
{
	cJSON *size = cJSON_AddObjectToObject(parent, name);

	if (size == NULL)
		return NULL;
	cJSON_AddNumberToObject(size, "width", width);
	cJSON_AddNumberToObject(size, "height", height);
	return size;
}

static void add_detection_model(cJSON *parent, const struct config_values *values)
{
	cJSON *model = cJSON_AddObjectToObject(parent, "model");
	char prefix[512];
	const char *start;

	if (model == NULL)
		return;
	snprintf(prefix, sizeof(prefix), "%s/", OVIS_MODEL_STORE_DIR);
	start = strncmp(values->object_model_path, prefix, strlen(prefix)) == 0 ?
		values->object_model_path + strlen(prefix) : NULL;
	if (start != NULL && strlen(start) >= 16 && start[16] == '/') {
		char id[17];
		memcpy(id, start, 16);
		id[16] = '\0';
		cJSON_AddStringToObject(model, "source", "custom");
		cJSON_AddStringToObject(model, "id", id);
	} else if (strcmp(values->object_model_id,
			"TDL_MODEL_YOLOV8N_DET_PERSON_VEHICLE") == 0) {
		cJSON_AddStringToObject(model, "source", "builtin");
		cJSON_AddStringToObject(model, "id", "builtin.person-vehicle");
	} else {
		cJSON_AddStringToObject(model, "source", "builtin");
		cJSON_AddStringToObject(model, "id", "builtin.monitor-person");
	}
	cJSON_AddStringToObject(model, "runtime_model", values->object_model_id);
}

static void color_to_hex(uint32_t color, char text[8])
{
	snprintf(text, 8, "#%06X", color & 0xffffff);
}

static cJSON *values_to_json(const struct config_values *values)
{
	cJSON *root = cJSON_CreateObject();
	cJSON *outputs;
	cJSON *rtsp;
	cJSON *uvc;
	cJSON *display;
	cJSON *video;
	cJSON *main_stream;
	cJSON *sub_stream;
	cJSON *overlay;
	cJSON *texts;
	cJSON *text;
	cJSON *streams;
	cJSON *overlay_detection;
	cJSON *overlay_tracking;
	cJSON *reticle;
	cJSON *ai_isp;
	cJSON *bnr;
	cJSON *detection;
	cJSON *object;
	cJSON *face;
	cJSON *motion;
	cJSON *human_pose;
	cJSON *tracking;
	cJSON *single_object;
	char color[8];
	const char *search_method = values->object_tracking_search_type == 3 ?
		"fastsam" : values->object_tracking_search_type == 2 ?
		"color" : "box";

	if (root == NULL)
		return NULL;
	outputs = cJSON_AddObjectToObject(root, "outputs");
	rtsp = cJSON_AddObjectToObject(outputs, "rtsp");
	uvc = cJSON_AddObjectToObject(outputs, "uvc");
	display = cJSON_AddObjectToObject(outputs, "display");
	video = cJSON_AddObjectToObject(root, "video");
	main_stream = cJSON_AddObjectToObject(video, "main");
	sub_stream = cJSON_AddObjectToObject(video, "sub");
	overlay = cJSON_AddObjectToObject(root, "overlay");
	ai_isp = cJSON_AddObjectToObject(root, "ai_isp");
	bnr = cJSON_AddObjectToObject(ai_isp, "bnr");
	detection = cJSON_AddObjectToObject(root, "detection");
	object = cJSON_AddObjectToObject(detection, "object");
	face = cJSON_AddObjectToObject(detection, "face");
	motion = cJSON_AddObjectToObject(detection, "motion");
	human_pose = cJSON_AddObjectToObject(detection, "human_pose");
	tracking = cJSON_AddObjectToObject(root, "tracking");
	single_object = cJSON_AddObjectToObject(tracking, "single_object");
	if (uvc == NULL || display == NULL || bnr == NULL || single_object == NULL) {
		cJSON_Delete(root);
		return NULL;
	}
	cJSON_AddBoolToObject(rtsp, "enabled", values->rtsp_enabled);
	cJSON_AddBoolToObject(uvc, "enabled", values->uvc_enabled);
	cJSON_AddBoolToObject(display, "enabled", values->display_enabled);
	cJSON_AddStringToObject(display, "mode", values->display_mode);
	cJSON_AddStringToObject(main_stream, "profile", "1080p");
	cJSON_AddNumberToObject(main_stream, "fps", values->main_fps);
	cJSON_AddNumberToObject(main_stream, "bitrate_kbps", values->main_bitrate);
	cJSON_AddBoolToObject(sub_stream, "enabled", values->sub_enabled);
	cJSON_AddStringToObject(sub_stream, "profile", "768x572");
	cJSON_AddNumberToObject(sub_stream, "fps", values->sub_fps);
	cJSON_AddNumberToObject(sub_stream, "bitrate_kbps", values->sub_bitrate);
	cJSON_AddBoolToObject(overlay, "enabled", values->osd_enabled);
	texts = cJSON_AddArrayToObject(overlay, "texts");
	text = cJSON_CreateObject();
	streams = cJSON_AddArrayToObject(text, "streams");
	cJSON_AddStringToObject(text, "id", "primary");
	cJSON_AddBoolToObject(text, "enabled", values->text_enabled);
	cJSON_AddStringToObject(text, "content", values->text_content);
	if (values->text_main_enabled)
		cJSON_AddItemToArray(streams, cJSON_CreateString("main"));
	if (values->text_sub_enabled)
		cJSON_AddItemToArray(streams, cJSON_CreateString("sub"));
	cJSON_AddStringToObject(text, "position", values->text_position);
	cJSON_AddNumberToObject(text, "x", values->text_x);
	cJSON_AddNumberToObject(text, "y", values->text_y);
	color_to_hex(values->text_color, color);
	cJSON_AddStringToObject(text, "color", color);
	cJSON_AddItemToArray(texts, text);
	overlay_detection = cJSON_AddObjectToObject(overlay, "detection");
	cJSON_AddBoolToObject(overlay_detection, "enabled",
		values->detection_osd_enabled);
	cJSON_AddStringToObject(overlay_detection, "colorMode",
		values->detection_color_mode);
	color_to_hex(values->detection_color, color);
	cJSON_AddStringToObject(overlay_detection, "color", color);
	cJSON_AddNumberToObject(overlay_detection, "thickness",
		values->detection_thickness);
	cJSON_AddStringToObject(overlay_detection, "labelMode",
		values->detection_label_mode);
	overlay_tracking = cJSON_AddObjectToObject(overlay, "tracking");
	cJSON_AddBoolToObject(overlay_tracking, "enabled",
		values->tracking_osd_enabled);
	color_to_hex(values->tracking_color, color);
	cJSON_AddStringToObject(overlay_tracking, "color", color);
	color_to_hex(values->tracking_lost_color, color);
	cJSON_AddStringToObject(overlay_tracking, "lostColor", color);
	cJSON_AddNumberToObject(overlay_tracking, "thickness",
		values->tracking_thickness);
	reticle = cJSON_AddObjectToObject(overlay, "reticle");
	cJSON_AddBoolToObject(reticle, "enabled", values->reticle_enabled);
	cJSON_AddStringToObject(reticle, "template", values->reticle_template);
	color_to_hex(values->reticle_idle_color, color);
	cJSON_AddStringToObject(reticle, "idleColor", color);
	color_to_hex(values->reticle_ready_color, color);
	cJSON_AddStringToObject(reticle, "readyColor", color);
	cJSON_AddNumberToObject(reticle, "thickness",
		values->reticle_thickness);
	cJSON_AddBoolToObject(reticle, "showWhileTracking",
		values->reticle_show_while_tracking);
	cJSON_AddBoolToObject(bnr, "enabled", values->ai_bnr_enabled);
	cJSON_AddBoolToObject(object, "enabled", values->object_enabled);
	cJSON_AddNumberToObject(object, "threshold", values->object_threshold);
	add_processing_size(object, "processing_size", values->object_width,
		values->object_height);
	add_detection_model(object, values);
	cJSON_AddBoolToObject(face, "enabled", values->face_enabled);
	cJSON_AddNumberToObject(face, "threshold", values->face_threshold);
	add_processing_size(face, "processing_size", values->face_width,
		values->face_height);
	cJSON_AddBoolToObject(motion, "enabled", values->motion_enabled);
	cJSON_AddNumberToObject(motion, "sensitivity", values->motion_sensitivity);
	add_processing_size(motion, "processing_size", values->motion_width,
		values->motion_height);
	cJSON_AddBoolToObject(human_pose, "enabled", values->human_pose_enabled);
	cJSON_AddNumberToObject(human_pose, "threshold", values->human_pose_threshold);
	add_processing_size(human_pose, "processing_size", values->human_pose_width,
		values->human_pose_height);
	cJSON_AddBoolToObject(single_object, "enabled", values->object_tracking_enabled);
	cJSON_AddStringToObject(single_object, "default_target_source",
		values->object_enabled ? "detection" : search_method);
	cJSON_AddStringToObject(single_object, "fallback_target_source", search_method);
	cJSON_AddStringToObject(single_object, "search_method", search_method);
	cJSON_AddBoolToObject(single_object, "detection_linked",
		values->object_enabled && values->object_tracking_enabled);
	cJSON_AddBoolToObject(single_object, "use_kalman",
		values->object_tracking_use_kalman);
	cJSON_AddNumberToObject(single_object, "score_threshold",
		values->object_tracking_score_threshold);
	add_processing_size(single_object, "processing_size",
		values->object_tracking_sot_width, values->object_tracking_sot_height);
	return root;
}

int config_capabilities_json(char *json, size_t size)
{
	static const char capabilities[] =
		"{\"schema_version\":7,\"outputs\":{"
		"\"rtsp\":{\"supported\":true,\"default_enabled\":false},"
		"\"uvc\":{\"supported\":true,\"default_enabled\":true,"
		"\"profile\":{\"codec\":\"mjpeg\",\"width\":1920,\"height\":1080,\"fps\":30}},"
		"\"display\":{\"supported\":true,\"default_enabled\":false,"
		"\"apply_mode\":\"ipcamera_restart\",\"modes\":["
		"{\"id\":\"720x480_60\",\"width\":720,\"height\":480,\"fps\":60}]}},"
		"\"video\":{"
		"\"main\":{\"profiles\":[{\"id\":\"1080p\",\"width\":1920,\"height\":1080,"
		"\"fps_options\":[15,25,30,60],\"bitrate_min\":512,\"bitrate_max\":15000}]},"
		"\"sub\":{\"profiles\":[{\"id\":\"768x572\",\"width\":768,\"height\":572,"
		"\"fps_options\":[15,25,30],\"bitrate_min\":128,\"bitrate_max\":4000}]}},"
		"\"overlay\":{\"supported\":true,"
		"\"maxTexts\":1,\"textMaxBytes\":63,\"utf8Text\":false,"
		"\"max_texts\":1,\"text_max_bytes\":63,"
		"\"sub_text_max_bytes\":31,"
		"\"text_positions\":[\"custom\",\"top-left\",\"top-right\","
		"\"bottom-left\",\"bottom-right\"],\"color_modes\":[\"fixed\",\"model\"],"
		"\"label_modes\":[\"none\",\"class\",\"class_score\"],"
		"\"thickness\":{\"min\":1,\"max\":4},"
		"\"thickness_min\":1,\"thickness_max\":4,"
		"\"colorModes\":[\"fixed\",\"model\"],"
		"\"labelModes\":[\"none\",\"class\",\"class_score\"],"
		"\"reticleTemplates\":[\"rectangle\",\"corners\",\"crosshair\","
		"\"crosshair_dot\",\"bracket_cross\",\"circle\"],"
		"\"reticle_templates\":[\"rectangle\",\"corners\",\"crosshair\","
		"\"crosshair_dot\",\"bracket_cross\",\"circle\"],"
		"\"streams\":{\"main\":{\"text\":true,\"ai\":true},"
		"\"sub\":{\"text\":true,\"ai\":false}}},"
		"\"features\":{\"osd\":true,\"object_detection\":true,"
		"\"face_detection\":true,\"motion_detection\":true,"
		"\"human_pose\":true,\"object_tracking\":true,"
		"\"single_object_tracking\":true},"
		"\"ai\":{\"max_active_tpu_features\":2,"
		"\"allowed_tpu_combinations\":[[\"object\"],[\"single_object_tracking\"],"
		"[\"object\",\"single_object_tracking\"],[\"face\"],[\"human_pose\"]],"
		"\"features\":["
		"{\"id\":\"object\",\"name\":\"目标检测\",\"model_selectable\":true,"
		"\"builtin_models\":["
		"{\"id\":\"builtin.monitor-person\",\"name\":\"人员检测\"},"
		"{\"id\":\"builtin.person-vehicle\",\"name\":\"人员/车辆检测\"}],"
		"\"processing_size\":{\"description\":\"送入 AI 的 VPSS 图像帧尺寸，不改变 BModel Tensor 尺寸\","
		"\"min_width\":160,\"max_width\":1920,"
		"\"min_height\":96,\"max_height\":1080,\"step\":2,\"default\":{\"width\":448,\"height\":256}}},"
		"{\"id\":\"face\",\"name\":\"人脸检测\",\"model\":\"SCRFD\","
		"\"processing_size\":{\"description\":\"送入 AI 的 VPSS 图像帧尺寸，不改变 BModel Tensor 尺寸\","
		"\"min_width\":160,\"max_width\":768,"
		"\"min_height\":96,\"max_height\":432,\"step\":2,\"default\":{\"width\":768,\"height\":432}}},"
		"{\"id\":\"motion\",\"name\":\"移动检测\","
		"\"processing_size\":{\"description\":\"送入移动检测管线的 VPSS 图像帧尺寸\","
		"\"min_width\":160,\"max_width\":640,"
		"\"min_height\":96,\"max_height\":360,\"step\":2,\"default\":{\"width\":640,\"height\":360}}},"
		"{\"id\":\"human_pose\",\"name\":\"人体姿态\",\"model\":\"YOLOv8 Pose\","
		"\"processing_size\":{\"description\":\"送入 AI 的 VPSS 图像帧尺寸，不改变 BModel Tensor 尺寸\","
		"\"min_width\":160,\"max_width\":640,"
		"\"min_height\":96,\"max_height\":384,\"step\":2,\"default\":{\"width\":640,\"height\":384}}},"
		"{\"id\":\"single_object_tracking\",\"name\":\"单目标跟踪\","
		"\"model\":\"FearTrack\",\"detection_link\":true,"
		"\"target_sources\":[\"detection\",\"fastsam\",\"color\",\"box\"],"
		"\"fallback_target_sources\":[\"fastsam\",\"color\",\"box\"],"
		"\"processing_size\":{\"fixed\":true,\"width\":1920,\"height\":1080}}],"
		"\"motion_detection\":true}}";
	cJSON *root = cJSON_Parse(capabilities);
	cJSON *ai_isp;
	cJSON *bnr;
	cJSON *exclusive;
	static const char *features[] = {
		"object", "face", "motion", "human_pose", "single_object_tracking"
	};
	size_t index;

	if (root == NULL)
		return -1;
	ai_isp = cJSON_AddObjectToObject(root, "ai_isp");
	if (ai_isp == NULL) {
		cJSON_Delete(root);
		return -1;
	}
	bnr = cJSON_AddObjectToObject(ai_isp, "bnr");
	if (bnr == NULL) {
		cJSON_Delete(root);
		return -1;
	}
	exclusive = cJSON_AddArrayToObject(bnr, "exclusive_with");
	if (exclusive == NULL) {
		cJSON_Delete(root);
		return -1;
	}
	cJSON_AddBoolToObject(bnr, "supported", config_ai_bnr_supported());
	cJSON_AddStringToObject(bnr, "apply_mode", "ipcamera_restart");
	cJSON_AddNumberToObject(bnr, "required_main_fps", 30);
	for (index = 0; index < sizeof(features) / sizeof(features[0]); index++)
		cJSON_AddItemToArray(exclusive, cJSON_CreateString(features[index]));
	return json_print(root, json, size);
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

static int processing_size_item(cJSON *parent, const char *name,
	int *width, int *height)
{
	cJSON *size = object_item(parent, name);

	return int_item(size, "width", width) == 0 &&
		int_item(size, "height", height) == 0 ? 0 : -1;
}

static int select_builtin_detection_model(const char *id,
	struct config_values *values, char *error, size_t error_size)
{
	if (strcmp(id, "builtin.monitor-person") == 0 ||
	    strcmp(id, "builtin.object-detection") == 0 ||
	    strcmp(id, "builtin.object_detection") == 0 ||
	    strcmp(id, "builtin.person-detection") == 0 ||
	    strcmp(id, "builtin.person_detection") == 0) {
		snprintf(values->object_model_id, sizeof(values->object_model_id),
			"TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON");
		snprintf(values->object_model_path, sizeof(values->object_model_path),
			"/usr/share/ipcamera/cv184x/"
			"yolov8n_det_monitor_person_256_448_INT8_cv184x.bmodel");
	} else if (strcmp(id, "builtin.person-vehicle") == 0 ||
		   strcmp(id, "builtin.person-vehicle-detection") == 0 ||
		   strcmp(id, "builtin.person_vehicle_detection") == 0) {
		snprintf(values->object_model_id, sizeof(values->object_model_id),
			"TDL_MODEL_YOLOV8N_DET_PERSON_VEHICLE");
		snprintf(values->object_model_path, sizeof(values->object_model_path),
			"/usr/share/ipcamera/cv184x/"
			"yolov8n_det_person_vehicle_384_640_INT8_cv184x.bmodel");
	} else {
		snprintf(error, error_size, "不支持的内置目标检测模型");
		return -1;
	}
	values->object_model_update = 1;
	return 0;
}

static int parse_detection_model(cJSON *model, struct config_values *values,
	char *error, size_t error_size)
{
	const char *source;
	const char *id;

	if (cJSON_IsString(model) && model->valuestring != NULL &&
	    model->valuestring[0] != '\0') {
		id = model->valuestring;
		if (strncmp(id, "builtin.", strlen("builtin.")) != 0)
			return 0;
		return select_builtin_detection_model(id, values, error, error_size);
	}
	if (!cJSON_IsObject(model) ||
	    string_item(model, "source", &source) != 0 ||
	    string_item(model, "id", &id) != 0) {
		snprintf(error, error_size, "目标检测模型选择无效");
		return -1;
	}
	if (strcmp(source, "builtin") == 0)
		return select_builtin_detection_model(id, values, error, error_size);
	if (strcmp(source, "custom") == 0)
		return 0;
	snprintf(error, error_size, "目标检测模型来源无效");
	return -1;
}

static int parse_color(const char *text, uint32_t *color)
{
	char *end;
	unsigned long parsed;

	if (text == NULL || strlen(text) != 7 || text[0] != '#')
		return -1;
	errno = 0;
	parsed = strtoul(text + 1, &end, 16);
	if (errno != 0 || *end != '\0' || parsed > 0xffffff)
		return -1;
	*color = (uint32_t)parsed;
	return 0;
}

static int parse_overlay(cJSON *overlay, struct config_values *values,
	char *error, size_t error_size)
{
	cJSON *texts;
	cJSON *text;
	cJSON *streams;
	cJSON *stream;
	cJSON *detection;
	cJSON *tracking;
	cJSON *reticle;
	const char *content;
	const char *position;
	const char *color;
	const char *mode;
	int main_enabled = 0;
	int sub_enabled = 0;

	texts = cJSON_GetObjectItemCaseSensitive(overlay, "texts");
	if (texts != NULL) {
		if (!cJSON_IsArray(texts) || cJSON_GetArraySize(texts) > 1) {
			snprintf(error, error_size, "自定义文字最多支持一项");
			return -1;
		}
		values->overlay_text_present = 1;
		if (cJSON_GetArraySize(texts) == 1) {
			text = cJSON_GetArrayItem(texts, 0);
			streams = cJSON_GetObjectItemCaseSensitive(text, "streams");
			if (!cJSON_IsObject(text) ||
			    bool_item(text, "enabled", &values->text_enabled) != 0 ||
			    string_item(text, "content", &content) != 0 ||
			    string_item(text, "position", &position) != 0 ||
			    int_item(text, "x", &values->text_x) != 0 ||
			    int_item(text, "y", &values->text_y) != 0 ||
			    string_item(text, "color", &color) != 0 ||
			    !cJSON_IsArray(streams) || parse_color(color,
				&values->text_color) != 0) {
				snprintf(error, error_size, "自定义文字配置无效");
				return -1;
			}
			if (strlen(content) >= sizeof(values->text_content) ||
			    strlen(position) >= sizeof(values->text_position)) {
				snprintf(error, error_size, "自定义文字内容或位置过长");
				return -1;
			}
			cJSON_ArrayForEach(stream, streams) {
				if (!cJSON_IsString(stream) || stream->valuestring == NULL) {
					snprintf(error, error_size, "文字码流配置无效");
					return -1;
				}
				if (strcmp(stream->valuestring, "main") == 0)
					main_enabled = 1;
				else if (strcmp(stream->valuestring, "sub") == 0)
					sub_enabled = 1;
				else {
					snprintf(error, error_size, "文字码流仅支持 main 或 sub");
					return -1;
				}
			}
			values->text_main_enabled = main_enabled;
			values->text_sub_enabled = sub_enabled;
			snprintf(values->text_content, sizeof(values->text_content),
				"%s", content);
			snprintf(values->text_position, sizeof(values->text_position),
				"%s", position);
		} else {
			values->text_enabled = 0;
		}
	}

	detection = cJSON_GetObjectItemCaseSensitive(overlay, "detection");
	if (detection != NULL) {
		values->overlay_detection_present = 1;
		if (!cJSON_IsObject(detection) ||
		    bool_item(detection, "enabled", &values->detection_osd_enabled) != 0 ||
		    string_item(detection, "colorMode", &mode) != 0 ||
		    string_item(detection, "color", &color) != 0 ||
		    int_item(detection, "thickness", &values->detection_thickness) != 0 ||
		    string_item(detection, "labelMode", &position) != 0 ||
		    parse_color(color, &values->detection_color) != 0) {
			snprintf(error, error_size, "检测框样式配置无效");
			return -1;
		}
		snprintf(values->detection_color_mode,
			sizeof(values->detection_color_mode), "%s", mode);
		snprintf(values->detection_label_mode,
			sizeof(values->detection_label_mode), "%s", position);
	}

	tracking = cJSON_GetObjectItemCaseSensitive(overlay, "tracking");
	if (tracking != NULL) {
		const char *lost_color;

		values->overlay_tracking_present = 1;
		if (!cJSON_IsObject(tracking) ||
		    bool_item(tracking, "enabled", &values->tracking_osd_enabled) != 0 ||
		    string_item(tracking, "color", &color) != 0 ||
		    string_item(tracking, "lostColor", &lost_color) != 0 ||
		    int_item(tracking, "thickness", &values->tracking_thickness) != 0 ||
		    parse_color(color, &values->tracking_color) != 0 ||
		    parse_color(lost_color, &values->tracking_lost_color) != 0) {
			snprintf(error, error_size, "跟踪框样式配置无效");
			return -1;
		}
	}

	reticle = cJSON_GetObjectItemCaseSensitive(overlay, "reticle");
	if (reticle != NULL) {
		const char *idle_color;
		const char *ready_color;

		values->overlay_reticle_present = 1;
		if (!cJSON_IsObject(reticle) ||
		    bool_item(reticle, "enabled", &values->reticle_enabled) != 0 ||
		    string_item(reticle, "template", &mode) != 0 ||
		    string_item(reticle, "idleColor", &idle_color) != 0 ||
		    string_item(reticle, "readyColor", &ready_color) != 0 ||
		    int_item(reticle, "thickness", &values->reticle_thickness) != 0 ||
		    bool_item(reticle, "showWhileTracking",
			&values->reticle_show_while_tracking) != 0 ||
		    parse_color(idle_color, &values->reticle_idle_color) != 0 ||
		    parse_color(ready_color, &values->reticle_ready_color) != 0) {
			snprintf(error, error_size, "中心准星样式配置无效");
			return -1;
		}
		snprintf(values->reticle_template,
			sizeof(values->reticle_template), "%s", mode);
	}
	return 0;
}

static int parse_payload(const char *body, struct config_values *values,
	char revision[33], char *error, size_t error_size)
{
	cJSON *root = cJSON_Parse(body);
	cJSON *values_json;
	cJSON *outputs;
	cJSON *rtsp;
	cJSON *uvc;
	cJSON *display;
	cJSON *video;
	cJSON *main_stream;
	cJSON *sub_stream;
	cJSON *overlay;
	cJSON *ai_isp;
	cJSON *bnr;
	cJSON *detection;
	cJSON *object;
	cJSON *object_model;
	cJSON *face;
	cJSON *motion;
	cJSON *human_pose;
	cJSON *tracking;
	cJSON *single_object;
	cJSON *object_tracking;
	const char *revision_text;
	const char *main_profile;
	const char *sub_profile;
	const char *display_mode;
	const char *search_method;
	const char *default_target_source = NULL;
	int new_tracking_schema = 0;
	int result = -1;

	memset(values, 0, sizeof(*values));
	overlay_defaults(values);
	if (!cJSON_IsObject(root))
		goto done;
	values_json = object_item(root, "values");
	outputs = object_item(values_json, "outputs");
	rtsp = object_item(outputs, "rtsp");
	uvc = object_item(outputs, "uvc");
	display = outputs == NULL ? NULL :
		cJSON_GetObjectItemCaseSensitive(outputs, "display");
	if (display != NULL) {
		if (!cJSON_IsObject(display) ||
		    bool_item(display, "enabled", &values->display_enabled) != 0 ||
		    string_item(display, "mode", &display_mode) != 0 ||
		    strlen(display_mode) >= sizeof(values->display_mode))
			goto done;
		values->display_present = 1;
		snprintf(values->display_mode, sizeof(values->display_mode), "%s",
			display_mode);
	}
	video = object_item(values_json, "video");
	main_stream = object_item(video, "main");
	sub_stream = object_item(video, "sub");
	overlay = object_item(values_json, "overlay");
	ai_isp = values_json == NULL ? NULL :
		cJSON_GetObjectItemCaseSensitive(values_json, "ai_isp");
	bnr = NULL;
	if (ai_isp != NULL) {
		if (!cJSON_IsObject(ai_isp))
			goto done;
		bnr = object_item(ai_isp, "bnr");
		if (bool_item(bnr, "enabled", &values->ai_bnr_enabled) != 0)
			goto done;
		values->ai_bnr_present = 1;
	}
	detection = object_item(values_json, "detection");
	object = object_item(detection, "object");
	object_model = object == NULL ? NULL :
		cJSON_GetObjectItemCaseSensitive(object, "model");
	face = object_item(detection, "face");
	motion = object_item(detection, "motion");
	human_pose = object_item(detection, "human_pose");
	tracking = object_item(values_json, "tracking");
	single_object = object_item(tracking, "single_object");
	object_tracking = single_object != NULL ? single_object :
		object_item(detection, "object_tracking");
	new_tracking_schema = single_object != NULL;
	values->object_tracking_det_width = OVIS_AI_TRACK_DET_WIDTH;
	values->object_tracking_det_height = OVIS_AI_TRACK_DET_HEIGHT;
	if (parse_detection_model(object_model, values, error, error_size) != 0)
		goto done;
	if (parse_overlay(overlay, values, error, error_size) != 0)
		goto done;
	if (string_item(root, "revision", &revision_text) != 0 || strlen(revision_text) > 32 ||
	    bool_item(rtsp, "enabled", &values->rtsp_enabled) != 0 ||
	    bool_item(uvc, "enabled", &values->uvc_enabled) != 0 ||
	    string_item(main_stream, "profile", &main_profile) != 0 ||
	    int_item(main_stream, "fps", &values->main_fps) != 0 ||
	    int_item(main_stream, "bitrate_kbps", &values->main_bitrate) != 0 ||
	    bool_item(sub_stream, "enabled", &values->sub_enabled) != 0 ||
	    string_item(sub_stream, "profile", &sub_profile) != 0 ||
	    int_item(sub_stream, "fps", &values->sub_fps) != 0 ||
	    int_item(sub_stream, "bitrate_kbps", &values->sub_bitrate) != 0 ||
	    bool_item(overlay, "enabled", &values->osd_enabled) != 0 ||
	    bool_item(object, "enabled", &values->object_enabled) != 0 ||
	    double_item(object, "threshold", &values->object_threshold) != 0 ||
	    processing_size_item(object, "processing_size", &values->object_width,
		&values->object_height) != 0 ||
	    bool_item(face, "enabled", &values->face_enabled) != 0 ||
	    double_item(face, "threshold", &values->face_threshold) != 0 ||
	    processing_size_item(face, "processing_size", &values->face_width,
		&values->face_height) != 0 ||
	    bool_item(motion, "enabled", &values->motion_enabled) != 0 ||
	    int_item(motion, "sensitivity", &values->motion_sensitivity) != 0 ||
	    processing_size_item(motion, "processing_size", &values->motion_width,
		&values->motion_height) != 0 ||
	    bool_item(human_pose, "enabled", &values->human_pose_enabled) != 0 ||
	    double_item(human_pose, "threshold", &values->human_pose_threshold) != 0 ||
	    processing_size_item(human_pose, "processing_size", &values->human_pose_width,
		&values->human_pose_height) != 0 ||
	    bool_item(object_tracking, "enabled", &values->object_tracking_enabled) != 0 ||
	    string_item(object_tracking, new_tracking_schema ?
		"fallback_target_source" : "search_method", &search_method) != 0 ||
	    bool_item(object_tracking, "use_kalman", &values->object_tracking_use_kalman) != 0 ||
	    double_item(object_tracking, "score_threshold",
		&values->object_tracking_score_threshold) != 0)
		goto done;
	if (new_tracking_schema) {
		if (string_item(object_tracking, "default_target_source",
				&default_target_source) != 0 ||
		    processing_size_item(object_tracking, "processing_size",
				&values->object_tracking_sot_width,
				&values->object_tracking_sot_height) != 0)
			goto done;
		if (strcmp(default_target_source, "detection") != 0 &&
		    strcmp(default_target_source, "fastsam") != 0 &&
		    strcmp(default_target_source, "color") != 0 &&
		    strcmp(default_target_source, "box") != 0) {
			snprintf(error, error_size, "默认跟踪目标来源不受支持");
			goto done;
		}
	} else if (processing_size_item(object_tracking,
			"detection_processing_size",
			&values->object_tracking_det_width,
			&values->object_tracking_det_height) != 0 ||
		    processing_size_item(object_tracking, "tracking_processing_size",
			&values->object_tracking_sot_width,
			&values->object_tracking_sot_height) != 0) {
		goto done;
	}
	if (strcmp(main_profile, "1080p") != 0 || strcmp(sub_profile, "768x572") != 0) {
		snprintf(error, error_size, "配置包含设备不支持的分辨率预设");
		goto done;
	}
	if (strcmp(search_method, "color") == 0)
		values->object_tracking_search_type = 2;
	else if (strcmp(search_method, "fastsam") == 0)
		values->object_tracking_search_type = 3;
	else if (strcmp(search_method, "box") == 0)
		values->object_tracking_search_type = 0;
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

static int processing_size_valid(int width, int height, int max_width,
	int max_height)
{
	return width >= OVIS_AI_MIN_WIDTH && width <= max_width &&
		height >= OVIS_AI_MIN_HEIGHT && height <= max_height &&
		(width % 2) == 0 && (height % 2) == 0;
}

static int one_of(const char *value, const char *const options[], size_t count)
{
	size_t index;

	for (index = 0; index < count; index++) {
		if (strcmp(value, options[index]) == 0)
			return 1;
	}
	return 0;
}

static int text_content_valid(const char *text)
{
	size_t index;
	size_t length = strlen(text);

	if (length > 63)
		return 0;
	for (index = 0; index < length; index++) {
		unsigned char value = (unsigned char)text[index];
		if (value < 0x20 || value > 0x7e || value == '"' || value == ';')
			return 0;
	}
	return 1;
}

static void inherit_overlay_values(struct config_values *values,
	const struct config_values *active)
{
	if (!values->overlay_text_present) {
		values->text_enabled = active->text_enabled;
		values->text_main_enabled = active->text_main_enabled;
		values->text_sub_enabled = active->text_sub_enabled;
		values->text_x = active->text_x;
		values->text_y = active->text_y;
		values->text_color = active->text_color;
		snprintf(values->text_content, sizeof(values->text_content), "%s",
			active->text_content);
		snprintf(values->text_position, sizeof(values->text_position), "%s",
			active->text_position);
	}
	if (!values->overlay_detection_present) {
		values->detection_osd_enabled = active->detection_osd_enabled;
		values->detection_color = active->detection_color;
		values->detection_thickness = active->detection_thickness;
		snprintf(values->detection_color_mode,
			sizeof(values->detection_color_mode), "%s",
			active->detection_color_mode);
		snprintf(values->detection_label_mode,
			sizeof(values->detection_label_mode), "%s",
			active->detection_label_mode);
	}
	if (!values->overlay_tracking_present) {
		values->tracking_osd_enabled = active->tracking_osd_enabled;
		values->tracking_color = active->tracking_color;
		values->tracking_lost_color = active->tracking_lost_color;
		values->tracking_thickness = active->tracking_thickness;
	}
	if (!values->overlay_reticle_present) {
		values->reticle_enabled = active->reticle_enabled;
		values->reticle_idle_color = active->reticle_idle_color;
		values->reticle_ready_color = active->reticle_ready_color;
		values->reticle_thickness = active->reticle_thickness;
		values->reticle_show_while_tracking =
			active->reticle_show_while_tracking;
		snprintf(values->reticle_template,
			sizeof(values->reticle_template), "%s",
			active->reticle_template);
	}
}

static void inherit_display_values(struct config_values *values,
	const struct config_values *active)
{
	if (values->display_present)
		return;
	values->display_enabled = active->display_enabled;
	values->display_present = 1;
	snprintf(values->display_mode, sizeof(values->display_mode), "%s",
		active->display_mode);
}

static int non_overlay_values_equal(const struct config_values *left,
	const struct config_values *right)
{
	struct config_values left_copy = *left;
	struct config_values right_copy = *right;
	size_t overlay_offset = offsetof(struct config_values, osd_enabled);
	size_t overlay_size = offsetof(struct config_values, object_enabled) -
		overlay_offset;

	memset((char *)&left_copy + overlay_offset, 0, overlay_size);
	memset((char *)&right_copy + overlay_offset, 0, overlay_size);
	left_copy.ai_bnr_present = 0;
	right_copy.ai_bnr_present = 0;
	left_copy.display_present = 0;
	right_copy.display_present = 0;
	left_copy.object_model_update = 0;
	right_copy.object_model_update = 0;
	return memcmp(&left_copy, &right_copy, sizeof(left_copy)) == 0;
}

static cJSON *validate_values(const struct config_values *values)
{
	static const char *const text_positions[] = {
		"custom", "top-left", "top-right", "bottom-left", "bottom-right"
	};
	static const char *const color_modes[] = { "fixed", "model" };
	static const char *const label_modes[] = {
		"none", "class", "class_score"
	};
	static const char *const reticle_templates[] = {
		"rectangle", "corners", "crosshair", "crosshair_dot",
		"bracket_cross", "circle"
	};
	cJSON *errors = cJSON_CreateArray();
	int active_tpu_features =
		(values->object_enabled || values->object_tracking_enabled) +
		values->face_enabled + values->human_pose_enabled;
	int active_business_ai = values->object_enabled + values->face_enabled +
		values->motion_enabled + values->human_pose_enabled +
		values->object_tracking_enabled;

	if (values->ai_bnr_enabled && !config_ai_bnr_supported())
		add_issue(errors, "ai_isp.bnr.enabled", "AI_BNR_UNSUPPORTED",
			"当前固件的 AI BNR 模型或 PQ 资源尚未通过认证");
	if (values->ai_bnr_enabled && active_business_ai > 0)
		add_issue(errors, "ai_isp.bnr.enabled", "AI_BNR_FEATURE_CONFLICT",
			"AI BNR 不能与目标、人脸、移动、人体姿态或目标跟踪同时启用");
	if (values->rtsp_enabled == values->uvc_enabled)
		add_issue(errors, "outputs", "OUTPUT_MODE_CONFLICT",
			"UVC 和 RTSP 必须且只能启用一项");
	if (strcmp(values->display_mode, OVIS_DISPLAY_MODE) != 0)
		add_issue(errors, "outputs.display.mode", "UNSUPPORTED_DISPLAY_MODE",
			"当前固件仅支持 720x480@60 MS7024 输出模式");
	if (!main_fps_supported(values->main_fps))
		add_issue(errors, "video.main.fps", "UNSUPPORTED_FPS", "主码流不支持此帧率");
	if (values->main_bitrate < 512 || values->main_bitrate > 15000)
		add_issue(errors, "video.main.bitrate_kbps", "OUT_OF_RANGE", "主码流码率范围为 512-15000 Kbps");
	if (!sub_fps_supported(values->sub_fps))
		add_issue(errors, "video.sub.fps", "UNSUPPORTED_FPS", "子码流不支持此帧率");
	if (values->sub_bitrate < 128 || values->sub_bitrate > 4000)
		add_issue(errors, "video.sub.bitrate_kbps", "OUT_OF_RANGE", "子码流码率范围为 128-4000 Kbps");
	if (!text_content_valid(values->text_content))
		add_issue(errors, "overlay.texts[0].content", "INVALID_TEXT",
			"自定义文字仅支持不超过 63 字节的可打印 ASCII 字符");
	if (values->text_enabled && !values->text_main_enabled &&
	    !values->text_sub_enabled)
		add_issue(errors, "overlay.texts[0].streams", "EMPTY_STREAMS",
			"启用自定义文字时至少选择一个码流");
	if (values->text_enabled && values->text_sub_enabled &&
	    strlen(values->text_content) > 31)
		add_issue(errors, "overlay.texts[0].content", "SUB_STREAM_TEXT_TOO_LONG",
			"子码流文字最多支持 31 个 ASCII 字符");
	if (!one_of(values->text_position, text_positions,
			sizeof(text_positions) / sizeof(text_positions[0])))
		add_issue(errors, "overlay.texts[0].position", "UNSUPPORTED_VALUE",
			"不支持此文字位置");
	if (values->text_x < 0 || values->text_x > 1919 ||
	    values->text_y < 0 || values->text_y > 1079)
		add_issue(errors, "overlay.texts[0]", "OUT_OF_RANGE",
			"文字偏移必须位于 1920x1080 画布范围内");
	if (!one_of(values->detection_color_mode, color_modes,
			sizeof(color_modes) / sizeof(color_modes[0])))
		add_issue(errors, "overlay.detection.colorMode", "UNSUPPORTED_VALUE",
			"检测框颜色模式仅支持 fixed 或 model");
	if (!one_of(values->detection_label_mode, label_modes,
			sizeof(label_modes) / sizeof(label_modes[0])))
		add_issue(errors, "overlay.detection.labelMode", "UNSUPPORTED_VALUE",
			"不支持此检测标签模式");
	if (values->detection_thickness < 1 || values->detection_thickness > 4)
		add_issue(errors, "overlay.detection.thickness", "OUT_OF_RANGE",
			"检测框粗细范围为 1-4");
	if (values->tracking_thickness < 1 || values->tracking_thickness > 4)
		add_issue(errors, "overlay.tracking.thickness", "OUT_OF_RANGE",
			"跟踪框粗细范围为 1-4");
	if (!one_of(values->reticle_template, reticle_templates,
			sizeof(reticle_templates) / sizeof(reticle_templates[0])))
		add_issue(errors, "overlay.reticle.template", "UNSUPPORTED_VALUE",
			"不支持此中心准星模板");
	if (values->reticle_thickness < 1 || values->reticle_thickness > 4)
		add_issue(errors, "overlay.reticle.thickness", "OUT_OF_RANGE",
			"中心准星粗细范围为 1-4");
	if (values->object_threshold < 0 || values->object_threshold > 1)
		add_issue(errors, "detection.object.threshold", "OUT_OF_RANGE", "目标检测阈值必须在 0 到 1 之间");
	if (!processing_size_valid(values->object_width, values->object_height,
			OVIS_AI_OBJECT_FRAME_MAX_WIDTH, OVIS_AI_OBJECT_FRAME_MAX_HEIGHT))
		add_issue(errors, "detection.object.processing_size", "OUT_OF_RANGE",
			"目标检测 AI 输入帧尺寸必须为 160x96 到 1920x1080 范围内的偶数");
	if (values->face_threshold < 0 || values->face_threshold > 1)
		add_issue(errors, "detection.face.threshold", "OUT_OF_RANGE", "人脸检测阈值必须在 0 到 1 之间");
	if (!processing_size_valid(values->face_width, values->face_height,
			OVIS_AI_FACE_MAX_WIDTH, OVIS_AI_FACE_MAX_HEIGHT))
		add_issue(errors, "detection.face.processing_size", "OUT_OF_RANGE",
			"人脸检测 AI 输入帧尺寸必须为 160x96 到 768x432 范围内的偶数");
	if (values->motion_sensitivity < 0 || values->motion_sensitivity > 100)
		add_issue(errors, "detection.motion.sensitivity", "OUT_OF_RANGE", "移动检测灵敏度必须在 0 到 100 之间");
	if (!processing_size_valid(values->motion_width, values->motion_height,
			OVIS_AI_MOTION_MAX_WIDTH, OVIS_AI_MOTION_MAX_HEIGHT))
		add_issue(errors, "detection.motion.processing_size", "OUT_OF_RANGE",
			"移动检测 AI 输入帧尺寸必须为 160x96 到 640x360 范围内的偶数");
	if (values->human_pose_threshold < 0 || values->human_pose_threshold > 1)
		add_issue(errors, "detection.human_pose.threshold", "OUT_OF_RANGE", "人体姿态阈值必须在 0 到 1 之间");
	if (!processing_size_valid(values->human_pose_width, values->human_pose_height,
			OVIS_AI_HUMAN_POSE_MAX_WIDTH, OVIS_AI_HUMAN_POSE_MAX_HEIGHT))
		add_issue(errors, "detection.human_pose.processing_size", "OUT_OF_RANGE",
			"人体姿态 AI 输入帧尺寸必须为 160x96 到 640x384 范围内的偶数");
	if (values->object_tracking_score_threshold < 0 ||
	    values->object_tracking_score_threshold > 1)
		add_issue(errors, "tracking.single_object.score_threshold", "OUT_OF_RANGE", "目标跟踪分数阈值必须在 0 到 1 之间");
	if (values->object_tracking_sot_width != OVIS_AI_TRACK_SOT_WIDTH ||
	    values->object_tracking_sot_height != OVIS_AI_TRACK_SOT_HEIGHT)
		add_issue(errors, "tracking.single_object.processing_size",
			"FIXED_BY_MODEL", "目标跟踪搜索尺寸由当前运行管线固定为 1920x1080");
	if (active_tpu_features > 1)
		add_issue(errors, "detection", "AI_FEATURE_CONFLICT",
			"目标检测可与单目标跟踪联动，但不能与人脸或人体姿态同时启用");
	return errors;
}

int config_validate_json(const char *body, char *json, size_t size,
	char *error, size_t error_size)
{
	struct config_values values;
	struct config_values active_values;
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
	if (revision_for_file(OVIS_CONFIG_FILE, active_revision) != 0 ||
	    load_values(OVIS_CONFIG_FILE, &active_values) != 0) {
		snprintf(error, error_size, "无法读取当前配置版本");
		return -3;
	}
	if (strcmp(requested_revision, active_revision) != 0) {
		snprintf(error, error_size, "配置已被其他操作修改，请重新读取");
		return -2;
	}
	if (!values.ai_bnr_present)
		values.ai_bnr_enabled = active_values.ai_bnr_enabled;
	if (!values.object_model_update) {
		snprintf(values.object_model_id, sizeof(values.object_model_id), "%s",
			active_values.object_model_id);
		snprintf(values.object_model_path, sizeof(values.object_model_path), "%s",
			active_values.object_model_path);
	}
	inherit_display_values(&values, &active_values);
	inherit_overlay_values(&values, &active_values);
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
	if (valid) {
		if (non_overlay_values_equal(&values, &active_values)) {
			cJSON_AddItemToArray(requires,
				cJSON_CreateString("overlay_reload"));
		} else {
			cJSON_AddItemToArray(requires,
				cJSON_CreateString("ipcamera_restart"));
		}
		if (!non_overlay_values_equal(&values, &active_values) &&
		    values.uvc_enabled != active_values.uvc_enabled) {
			cJSON_AddItemToArray(requires, cJSON_CreateString("usb_gadget_restart"));
			cJSON_AddItemToArray(requires, cJSON_CreateString("management_reconnect"));
		}
	}
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

static int ensure_ini_key(const char *path, const char *wanted_section,
	const char *wanted_key, const char *default_value)
{
	char existing[160];
	char line[1024];
	char parse_line[1024];
	char section[64] = "";
	char temporary[512];
	FILE *input = NULL;
	FILE *output = NULL;
	int in_section = 0;
	int section_found = 0;
	int inserted = 0;
	int result = -1;

	if (read_ini_value(path, wanted_section, wanted_key, existing,
			sizeof(existing)) == 0)
		return 0;
	snprintf(temporary, sizeof(temporary), "%s.key.tmp", path);
	input = fopen(path, "r");
	output = fopen(temporary, "w");
	if (input == NULL || output == NULL)
		goto done;
	while (fgets(line, sizeof(line), input) != NULL) {
		char *text;

		snprintf(parse_line, sizeof(parse_line), "%s", line);
		text = trim(parse_line);
		if (*text == '[') {
			char *close = strchr(text, ']');
			if (in_section && !inserted) {
				fprintf(output, "%-16s = %s\n", wanted_key, default_value);
				inserted = 1;
			}
			if (close != NULL) {
				*close = '\0';
				snprintf(section, sizeof(section), "%s", text + 1);
				in_section = strcmp(section, wanted_section) == 0;
				if (in_section)
					section_found = 1;
			}
		}
		fputs(line, output);
	}
	if (in_section && !inserted) {
		fprintf(output, "%-16s = %s\n", wanted_key, default_value);
		inserted = 1;
	}
	if (!section_found || !inserted || fflush(output) != 0 ||
	    fsync(fileno(output)) != 0)
		goto done;
	if (fclose(output) != 0) {
		output = NULL;
		goto done;
	}
	output = NULL;
	if (rename(temporary, path) != 0)
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

static int set_update_int(struct ini_update *updates, size_t update_count,
	const char *section, const char *key, int value)
{
	size_t index;

	for (index = 0; index < update_count; index++) {
		if (strcmp(updates[index].section, section) == 0 &&
		    strcmp(updates[index].key, key) == 0) {
			snprintf(updates[index].value, sizeof(updates[index].value), "%d", value);
			return 0;
		}
	}
	return -1;
}

static int set_update_value(struct ini_update *updates, size_t update_count,
	const char *section, const char *key, const char *value)
{
	size_t index;

	for (index = 0; index < update_count; index++) {
		if (strcmp(updates[index].section, section) == 0 &&
		    strcmp(updates[index].key, key) == 0) {
			snprintf(updates[index].value, sizeof(updates[index].value),
				"%s", value);
			return 0;
		}
	}
	return -1;
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

static int append_missing_output_config(const char *path)
{
	int rtsp_enabled = 0;
	int uvc_enabled = 1;
	int sub_enabled = 0;
	FILE *file;

	if (has_section(path, "output_config"))
		return 0;
	read_int(path, "vencchn0", "bEnable", &rtsp_enabled);
	read_int(path, "vencchn3", "bEnable", &uvc_enabled);
	read_int(path, "vencchn1", "bEnable", &sub_enabled);
	if (!((rtsp_enabled == 0 && uvc_enabled == 1) ||
	      (rtsp_enabled == 1 && uvc_enabled == 0))) {
		rtsp_enabled = 0;
		uvc_enabled = 1;
	}
	file = fopen(path, "a");
	if (file == NULL)
		return -1;
	fprintf(file, "\n[output_config]\n"
		"rtsp_enable      = %d\n"
		"uvc_enable       = %d\n"
		"sub_enable       = %d\n",
		rtsp_enabled ? 1 : 0, uvc_enabled ? 1 : 0, sub_enabled ? 1 : 0);
	if (fflush(file) != 0 || fsync(fileno(file)) != 0) {
		fclose(file);
		return -1;
	}
	return fclose(file);
}

static int migrate_legacy_overlay(const char *path)
{
	struct ini_update updates[] = {
		{ "osdc0_obj_info0", "bShow", "0", 0 },
		{ "osdc0_obj_info1", "bShow", "0", 0 },
		{ "osdc0_obj_info1", "color", "0xffffff", 0 },
		{ "osdc0_obj_info1", "x1", "20", 0 },
		{ "osdc0_obj_info1", "y1", "20", 0 },
		{ "osdc0_obj_info1", "str", "\"\"", 0 },
		{ "osdc1_obj_info0", "bShow", "0", 0 },
		{ "osdc1_obj_info1", "bShow", "0", 0 },
		{ "osdc1_obj_info1", "color", "0xffffff", 0 },
		{ "osdc1_obj_info1", "x1", "20", 0 },
		{ "osdc1_obj_info1", "y1", "20", 0 },
		{ "osdc1_obj_info1", "str", "\"\"", 0 },
	};
	char migrated[512];
	FILE *file;

	if (has_section(path, "osd_style"))
		return 0;
	file = fopen(path, "a");
	if (file == NULL)
		return -1;
	fputs("\n[osd_style]\n"
		"config_version  = 1\n"
		"detection_enabled = 1\n"
		"detection_color_mode = fixed\n"
		"detection_color = 0x00d9ff\n"
		"detection_thickness = 2\n"
		"detection_label_mode = none\n"
		"tracking_enabled = 1\n"
		"tracking_color  = 0xffb000\n"
		"tracking_lost_color = 0xff3030\n"
		"tracking_thickness = 3\n"
		"reticle_enabled = 1\n"
		"reticle_template = corners\n"
		"reticle_idle_color = 0xffffff\n"
		"reticle_ready_color = 0xffc247\n"
		"reticle_thickness = 2\n"
		"reticle_show_while_tracking = 0\n"
		"text_enabled    = 0\n"
		"text_main_enabled = 1\n"
		"text_sub_enabled = 0\n"
		"text_position   = top-left\n", file);
	if (fflush(file) != 0 || fsync(fileno(file)) != 0) {
		fclose(file);
		return -1;
	}
	if (fclose(file) != 0)
		return -1;
	snprintf(migrated, sizeof(migrated), "%s.overlay.tmp", path);
	if (write_updates(path, migrated, updates,
			sizeof(updates) / sizeof(updates[0])) != 0)
		return -1;
	if (rename(migrated, path) != 0) {
		unlink(migrated);
		return -1;
	}
	return 0;
}

static int ensure_overlay_style_keys(const char *path)
{
	static const struct {
		const char *key;
		const char *value;
	} defaults[] = {
		{ "config_version", "1" },
		{ "detection_enabled", "1" },
		{ "detection_color_mode", "fixed" },
		{ "detection_color", "0x00d9ff" },
		{ "detection_thickness", "2" },
		{ "detection_label_mode", "none" },
		{ "tracking_enabled", "1" },
		{ "tracking_color", "0xffb000" },
		{ "tracking_lost_color", "0xff3030" },
		{ "tracking_thickness", "3" },
		{ "reticle_enabled", "1" },
		{ "reticle_template", "corners" },
		{ "reticle_idle_color", "0xffffff" },
		{ "reticle_ready_color", "0xffc247" },
		{ "reticle_thickness", "2" },
		{ "reticle_show_while_tracking", "0" },
		{ "text_enabled", "0" },
		{ "text_main_enabled", "1" },
		{ "text_sub_enabled", "0" },
		{ "text_position", "top-left" },
	};
	size_t index;

	for (index = 0; index < sizeof(defaults) / sizeof(defaults[0]); index++) {
		if (ensure_ini_key(path, "osd_style", defaults[index].key,
				defaults[index].value) != 0)
			return -1;
	}
	return 0;
}

static int append_missing_runtime_sections(const char *path)
{
	int need_human_pose = !has_section(path, "ai_human_keypoint_config");
	int need_object_tracking = !has_section(path, "ai_object_track_config");
	int need_sot_pool = !has_section(path, "vb_pool_7");
	int need_uvc_pool = !has_section(path, "vb_pool_8");
	int need_uvc_group = !has_section(path, "vpssgrp6");
	int need_uvc_channel = !has_section(path, "vpssgrp6.chn0");
	int need_display_pool = !has_section(path, "vb_pool_9");
	int need_display_group = !has_section(path, "vpssgrp7");
	int need_display_channel = !has_section(path, "vpssgrp7.chn0");
	int need_display_config = !has_section(path, "display_config");
	int need_display_device = !has_section(path, "display_config_0");
	FILE *file;

	if (!need_human_pose && !need_object_tracking && !need_sot_pool &&
	    !need_uvc_pool && !need_uvc_group && !need_uvc_channel &&
	    !need_display_pool && !need_display_group && !need_display_channel &&
	    !need_display_config && !need_display_device)
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
			"config_version     = 2\n"
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
			"sot_gmc_enable    = 1\n"
			"sot_gmc_interval  = 4\n"
			"sot_min_observed_score = 0.12\n"
			"tracking_score_threshold = 0.12\n"
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
			"bEnable         = 0\n"
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
			"grp_enable      = 0\n"
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
			"chn_enable      = 0\n"
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
	if (need_display_pool) {
		fputs("\n[vb_pool_9]\n"
			"bEnable         = 0\n"
			"frame_width     = 720\n"
			"frame_height    = 480\n"
			"frame_fmt       = PIXEL_FORMAT_NV12\n"
			"data_bitwidth   = DATA_BITWIDTH_8\n"
			"compress_mode   = COMPRESS_MODE_NONE\n"
			"blk_cnt         = 4\n"
			"mem_size        = 0\n", file);
	}
	if (need_display_group) {
		fputs("\n[vpssgrp7]\n"
			"group_id        = 7\n"
			"grp_enable      = 0\n"
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
			"dst_dev_id      = 7\n"
			"dst_chn_id      = 0\n", file);
	}
	if (need_display_channel) {
		fputs("\n[vpssgrp7.chn0]\n"
			"chn_enable      = 1\n"
			"width           = 720\n"
			"height          = 480\n"
			"video_fmt       = VIDEO_FORMAT_LINEAR\n"
			"chn_pixel_fmt   = PIXEL_FORMAT_NV12\n"
			"src_framerate   = -1\n"
			"dst_framerate   = -1\n"
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
			"attach_pool     = 9\n", file);
	}
	if (need_display_config)
		fputs("\n[display_config]\nvo_cnt = 0\n", file);
	if (need_display_device) {
		fputs("\n[display_config_0]\n"
			"panel_type      = PANEL_BT656_MS7024_720x480_60\n"
			"i2c_dev         = 2\n"
			"i2c_addr        = 0x76\n"
			"vo_dev          = 0\n"
			"bg_color        = 0x00000000\n"
			"intf_type       = VO_INTF_BT656\n"
			"intf_sync       = VO_OUTPUT_USER\n"
			"dis_x           = 0\n"
			"dis_y           = 0\n"
			"dis_width       = 720\n"
			"dis_height      = 480\n"
			"img_width       = 720\n"
			"img_height      = 480\n"
			"dis_framerate   = 60\n"
			"pixel_fmt       = PIXEL_FORMAT_NV12\n"
			"mode            = VO_MODE_1MUX\n"
			"rotation        = ROTATION_0\n"
			"dis_buf_len     = 3\n"
			"bind_mode       = 1\n"
			"src_mod_id      = CVI_ID_VPSS\n"
			"src_dev_id      = 7\n"
			"src_chn_id      = 0\n"
			"dst_mod_id      = CVI_ID_VO\n"
			"dst_dev_id      = 0\n"
			"dst_chn_id      = 0\n", file);
	}
	if (fflush(file) != 0 || fsync(fileno(file)) != 0) {
		fclose(file);
		return -1;
	}
	return fclose(file);
}

static int migrate_display_topology(const char *path)
{
	struct ini_update updates[] = {
		{ "vb_config", "vb_pool_cnt", "10", 0 },
		{ "vpss_config", "vpss_grp", "8", 0 },
		{ "vb_pool_9", "bEnable", "", 0 },
		{ "vb_pool_9", "frame_width", "720", 0 },
		{ "vb_pool_9", "frame_height", "480", 0 },
		{ "vb_pool_9", "frame_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vb_pool_9", "data_bitwidth", "DATA_BITWIDTH_8", 0 },
		{ "vb_pool_9", "compress_mode", "COMPRESS_MODE_NONE", 0 },
		{ "vb_pool_9", "blk_cnt", "4", 0 },
		{ "vb_pool_9", "mem_size", "0", 0 },
		{ "vpssgrp7", "group_id", "7", 0 },
		{ "vpssgrp7", "grp_enable", "", 0 },
		{ "vpssgrp7", "pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp7", "src_framerate", "-1", 0 },
		{ "vpssgrp7", "dst_framerate", "-1", 0 },
		{ "vpssgrp7", "vpss_dev", "0", 0 },
		{ "vpssgrp7", "max_w", "1920", 0 },
		{ "vpssgrp7", "max_h", "1080", 0 },
		{ "vpssgrp7", "chn_cnt", "1", 0 },
		{ "vpssgrp7", "crop_en", "0", 0 },
		{ "vpssgrp7", "crop_coor", "VPSS_CROP_RATIO_COOR", 0 },
		{ "vpssgrp7", "crop_rect_x", "0", 0 },
		{ "vpssgrp7", "crop_rect_y", "0", 0 },
		{ "vpssgrp7", "crop_rect_w", "0", 0 },
		{ "vpssgrp7", "crop_rect_h", "0", 0 },
		{ "vpssgrp7", "bind_mode", "1", 0 },
		{ "vpssgrp7", "src_mod_id", "CVI_ID_VPSS", 0 },
		{ "vpssgrp7", "src_dev_id", "0", 0 },
		{ "vpssgrp7", "src_chn_id", "0", 0 },
		{ "vpssgrp7", "dst_mod_id", "CVI_ID_VPSS", 0 },
		{ "vpssgrp7", "dst_dev_id", "7", 0 },
		{ "vpssgrp7", "dst_chn_id", "0", 0 },
		{ "vpssgrp7.chn0", "chn_enable", "1", 0 },
		{ "vpssgrp7.chn0", "width", "720", 0 },
		{ "vpssgrp7.chn0", "height", "480", 0 },
		{ "vpssgrp7.chn0", "video_fmt", "VIDEO_FORMAT_LINEAR", 0 },
		{ "vpssgrp7.chn0", "chn_pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp7.chn0", "src_framerate", "-1", 0 },
		{ "vpssgrp7.chn0", "dst_framerate", "-1", 0 },
		{ "vpssgrp7.chn0", "depth", "0", 0 },
		{ "vpssgrp7.chn0", "mirror", "0", 0 },
		{ "vpssgrp7.chn0", "filp", "0", 0 },
		{ "vpssgrp7.chn0", "aspectratio", "ASPECT_RATIO_NONE", 0 },
		{ "vpssgrp7.chn0", "s32x", "0", 0 },
		{ "vpssgrp7.chn0", "s32y", "0", 0 },
		{ "vpssgrp7.chn0", "rec_width", "0", 0 },
		{ "vpssgrp7.chn0", "rec_heigh", "0", 0 },
		{ "vpssgrp7.chn0", "en_color", "1", 0 },
		{ "vpssgrp7.chn0", "color", "0", 0 },
		{ "vpssgrp7.chn0", "normalize", "0", 0 },
		{ "vpssgrp7.chn0", "crop_en", "0", 0 },
		{ "vpssgrp7.chn0", "crop_coor", "VPSS_CROP_RATIO_COOR", 0 },
		{ "vpssgrp7.chn0", "crop_rect_x", "0", 0 },
		{ "vpssgrp7.chn0", "crop_rect_y", "0", 0 },
		{ "vpssgrp7.chn0", "crop_rect_w", "0", 0 },
		{ "vpssgrp7.chn0", "crop_rect_h", "0", 0 },
		{ "vpssgrp7.chn0", "attach_en", "1", 0 },
		{ "vpssgrp7.chn0", "attach_pool", "9", 0 },
		{ "display_config", "vo_cnt", "", 0 },
		{ "display_config_0", "panel_type", OVIS_DISPLAY_PANEL_TYPE, 0 },
		{ "display_config_0", "i2c_dev", "2", 0 },
		{ "display_config_0", "i2c_addr", "0x76", 0 },
		{ "display_config_0", "vo_dev", "0", 0 },
		{ "display_config_0", "bg_color", "0x00000000", 0 },
		{ "display_config_0", "intf_type", "VO_INTF_BT656", 0 },
		{ "display_config_0", "intf_sync", "VO_OUTPUT_USER", 0 },
		{ "display_config_0", "dis_x", "0", 0 },
		{ "display_config_0", "dis_y", "0", 0 },
		{ "display_config_0", "dis_width", "720", 0 },
		{ "display_config_0", "dis_height", "480", 0 },
		{ "display_config_0", "img_width", "720", 0 },
		{ "display_config_0", "img_height", "480", 0 },
		{ "display_config_0", "dis_framerate", "60", 0 },
		{ "display_config_0", "pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "display_config_0", "mode", "VO_MODE_1MUX", 0 },
		{ "display_config_0", "rotation", "ROTATION_0", 0 },
		{ "display_config_0", "dis_buf_len", "3", 0 },
		{ "display_config_0", "bind_mode", "1", 0 },
		{ "display_config_0", "src_mod_id", "CVI_ID_VPSS", 0 },
		{ "display_config_0", "src_dev_id", "7", 0 },
		{ "display_config_0", "src_chn_id", "0", 0 },
		{ "display_config_0", "dst_mod_id", "CVI_ID_VO", 0 },
		{ "display_config_0", "dst_dev_id", "0", 0 },
		{ "display_config_0", "dst_chn_id", "0", 0 },
	};
	char migrated[512];
	char value[160];
	int enabled = 0;
	size_t index;

	if (read_int(path, "display_config", "vo_cnt", &enabled) != 0 ||
	    (enabled != 0 && enabled != 1))
		enabled = 0;
	if (set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"display_config", "vo_cnt", enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp7", "grp_enable", enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_9", "bEnable", enabled) != 0)
		return -1;
	for (index = 0; index < sizeof(updates) / sizeof(updates[0]); index++) {
		if (ensure_ini_key(path, updates[index].section, updates[index].key,
				updates[index].value) != 0)
			return -1;
	}
	for (index = 0; index < sizeof(updates) / sizeof(updates[0]); index++) {
		if (read_ini_value(path, updates[index].section, updates[index].key,
				value, sizeof(value)) != 0 ||
		    strcmp(value, updates[index].value) != 0)
			break;
	}
	if (index == sizeof(updates) / sizeof(updates[0]))
		return 0;
	snprintf(migrated, sizeof(migrated), "%s.display.tmp", path);
	if (write_updates(path, migrated, updates,
			sizeof(updates) / sizeof(updates[0])) != 0)
		return -1;
	if (rename(migrated, path) != 0) {
		unlink(migrated);
		return -1;
	}
	return 0;
}

static int migrate_runtime_config_work(const char *path)
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
		{ "vb_config", "vb_pool_cnt", "10", 0 },
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
		{ "vb_pool_8", "bEnable", "0", 0 },
		{ "vb_pool_8", "frame_width", "1920", 0 },
		{ "vb_pool_8", "frame_height", "1080", 0 },
		{ "vb_pool_8", "frame_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vb_pool_8", "data_bitwidth", "DATA_BITWIDTH_8", 0 },
		{ "vb_pool_8", "compress_mode", "COMPRESS_MODE_NONE", 0 },
		{ "vb_pool_8", "blk_cnt", "4", 0 },
		{ "vb_pool_8", "mem_size", "0", 0 },
		{ "vpss_config", "vpss_grp", "8", 0 },
		{ "vpssgrp1", "chn_cnt", "1", 0 },
		{ "vpssgrp6", "group_id", "6", 0 },
		{ "vpssgrp6", "grp_enable", "0", 0 },
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
		{ "vpssgrp6.chn0", "chn_enable", "0", 0 },
		{ "vpssgrp6.chn0", "width", "1920", 0 },
		{ "vpssgrp6.chn0", "height", "1080", 0 },
		{ "vpssgrp6.chn0", "video_fmt", "VIDEO_FORMAT_LINEAR", 0 },
		{ "vpssgrp6.chn0", "chn_pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp6.chn0", "dst_framerate", "", 0 },
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
		{ "vencchn3", "src_dev_id", "0", 0 },
		{ "vencchn3", "src_chn_id", "0", 0 },
		{ "vencchn3", "vpss_grp", "0", 0 },
		{ "vencchn3", "vpss_chn", "0", 0 },
		{ "vencchn3", "src_framerate", "", 0 },
		{ "vencchn3", "dst_framerate", "", 0 },
		{ "vencchn3", "rc_mode", "VENC_RC_MODE_MJPEGCBR", 0 },
		{ "vencchn3", "bit_rate", "50000", 0 },
		{ "vencchn3", "max_bitrate", "50000", 0 },
		{ "vb_pool_6", "bEnable", "", 0 },
		{ "vpssgrp1", "grp_enable", "", 0 },
		{ "vencchn0", "bEnable", "", 0 },
		{ "vencchn1", "bEnable", "", 0 },
		{ "vencchn3", "bEnable", "", 0 },
		{ "rtsp_config", "rtsp_cnt", "", 0 },
		{ "vb_pool_1", "bEnable", "", 0 },
		{ "vb_pool_2", "bEnable", "", 0 },
		{ "vb_pool_3", "bEnable", "", 0 },
		{ "vb_pool_4", "bEnable", "", 0 },
		{ "vb_pool_7", "bEnable", "", 0 },
		{ "vpssgrp2", "max_w", "1920", 0 },
		{ "vpssgrp2", "max_h", "1080", 0 },
		{ "vpssgrp2", "pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp2", "src_chn_id", "0", 0 },
		{ "vpssgrp2", "src_framerate", "-1", 0 },
		{ "vpssgrp2", "dst_framerate", "-1", 0 },
		{ "vpssgrp2.chn0", "chn_pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vb_pool_2", "frame_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp3", "max_w", "1920", 0 },
		{ "vpssgrp3", "max_h", "1080", 0 },
		{ "vpssgrp3", "pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp3", "src_chn_id", "0", 0 },
		{ "vpssgrp4", "max_w", "1920", 0 },
		{ "vpssgrp4", "max_h", "1080", 0 },
		{ "vpssgrp4", "pixel_fmt", "PIXEL_FORMAT_NV12", 0 },
		{ "vpssgrp4", "src_chn_id", "0", 0 },
		{ "output_config", "rtsp_enable", "", 0 },
		{ "output_config", "uvc_enable", "", 0 },
		{ "ai_object_track_config", "config_version", "2", 0 },
		{ "ai_pd_config", "model_id", "", 0 },
	};
	char migrated[512];
	char value[160];
	int enabled[4] = {0};
	int motion_enabled = 0;
	int group_enabled[4];
	int sub_enabled = 0;
	int rtsp_enabled = 0;
	int uvc_enabled = 1;
	int osd_enabled = 0;
	int main_fps = 0;
	int runtime_sub_enabled = 0;
	int human_pose_width = OVIS_AI_HUMAN_POSE_MAX_WIDTH;
	int human_pose_height = OVIS_AI_HUMAN_POSE_MAX_HEIGHT;
	int shared_ai_source_enabled = 0;
	int tracking_config_version = 0;
	int legacy_tracking_config = 0;
	int sub_dependent_enabled[3];
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
		SHARED_CHANNEL_WIDTH_UPDATE_INDEX = 33,
		SHARED_CHANNEL_HEIGHT_UPDATE_INDEX = 34,
		SHARED_POOL_WIDTH_UPDATE_INDEX = 42,
		SHARED_POOL_HEIGHT_UPDATE_INDEX = 43,
	};
	static const int fixed_update_indexes[] = {
		1, 3, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23
	};

	if (access(path, F_OK) != 0)
		return -1;
	legacy_tracking_config = read_int(path, "ai_object_track_config",
		"config_version", &tracking_config_version) != 0 ||
		tracking_config_version < 2;
	if (ensure_ini_key(path, "ai_pd_config", "model_path_cfg",
			"\"/usr/share/ipcamera/model_factory.json\"") != 0)
		return -1;
	if (ensure_ini_key(path, "vi_cfg_isp0", "teaisp_bnr_enable", "0") != 0)
		return -1;
	if (read_ini_value(path, "ai_pd_config", "model_path", value,
			sizeof(value)) == 0 &&
	    (strstr(value, OVIS_MODEL_STORE_DIR "/") != NULL ||
	     strstr(value, "yolov8n_det_person_vehicle_384_640_INT8_cv184x.bmodel") != NULL))
		snprintf(updates[1].value, sizeof(updates[1].value), "%s", value);
	needs_update = !has_section(path, "output_config") ||
		!has_section(path, "ai_human_keypoint_config") ||
		!has_section(path, "ai_object_track_config") ||
		!has_section(path, "vb_pool_7") ||
		!has_section(path, "vb_pool_8") ||
		!has_section(path, "vpssgrp6") ||
		!has_section(path, "vpssgrp6.chn0") ||
		!has_section(path, "vb_pool_9") ||
		!has_section(path, "vpssgrp7") ||
		!has_section(path, "vpssgrp7.chn0") ||
		!has_section(path, "display_config") ||
		!has_section(path, "display_config_0");
	if (append_missing_output_config(path) != 0 ||
	    append_missing_runtime_sections(path) != 0 ||
	    migrate_display_topology(path) != 0 ||
	    migrate_legacy_overlay(path) != 0 ||
	    ensure_overlay_style_keys(path) != 0)
		return -1;
	if (ensure_ini_key(path, "ai_object_track_config", "config_version", "1") != 0)
		return -1;
	if (ensure_ini_key(path, "ai_object_track_config", "sot_gmc_enable", "1") != 0 ||
	    ensure_ini_key(path, "ai_object_track_config", "sot_gmc_interval", "4") != 0 ||
	    ensure_ini_key(path, "ai_object_track_config", "sot_min_observed_score", "0.12") != 0)
		return -1;
	if (read_int(path, "ai_pd_config", "pd_enable", &enabled[0]) != 0 ||
	    read_int(path, "ai_fd_config", "fd_enable", &enabled[1]) != 0 ||
	    read_int(path, "ai_md_config", "md_enable", &motion_enabled) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "human_keypoint_enable", &enabled[2]) != 0 ||
	    read_int(path, "ai_object_track_config", "object_track_enable", &enabled[3]) != 0 ||
	    read_int(path, "output_config", "rtsp_enable", &rtsp_enabled) != 0 ||
	    read_int(path, "output_config", "uvc_enable", &uvc_enabled) != 0 ||
	    read_int(path, "output_config", "sub_enable", &sub_enabled) != 0 ||
	    read_int(path, "vencchn0", "dst_framerate", &main_fps) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "model_width",
		&human_pose_width) != 0 ||
	    read_int(path, "ai_human_keypoint_config", "model_height",
		&human_pose_height) != 0 ||
	    read_int(path, "osdc_config", "enable", &osd_enabled) != 0)
		return -1;
	if (!((rtsp_enabled == 0 && uvc_enabled == 1) ||
	      (rtsp_enabled == 1 && uvc_enabled == 0))) {
		rtsp_enabled = 0;
		uvc_enabled = 1;
		needs_update = 1;
	}
	if (legacy_tracking_config && enabled[3] && !enabled[0]) {
		enabled[0] = 1;
		snprintf(updates[1].value, sizeof(updates[1].value),
			"\"/usr/share/ipcamera/cv184x/"
			"yolov8n_det_person_vehicle_384_640_INT8_cv184x.bmodel\"");
		snprintf(updates[sizeof(updates) / sizeof(updates[0]) - 1].value,
			sizeof(updates[sizeof(updates) / sizeof(updates[0]) - 1].value),
			"TDL_MODEL_YOLOV8N_DET_PERSON_VEHICLE");
		needs_update = 1;
	} else if (read_ini_value(path, "ai_pd_config", "model_id",
			updates[sizeof(updates) / sizeof(updates[0]) - 1].value,
			sizeof(updates[sizeof(updates) / sizeof(updates[0]) - 1].value)) != 0) {
		return -1;
	}
	runtime_sub_enabled = rtsp_enabled && sub_enabled &&
		!(uvc_enabled && main_fps == 60);
	if (set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"output_config", "rtsp_enable", rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"output_config", "uvc_enable", uvc_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_6", "bEnable", rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp1", "grp_enable", rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn0", "bEnable", rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn1", "bEnable", runtime_sub_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"rtsp_config", "rtsp_cnt", rtsp_enabled ? 2 : 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_8", "bEnable", 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp6", "grp_enable", 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp6.chn0", "chn_enable", 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn3", "bEnable", uvc_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp6.chn0", "dst_framerate",
			main_fps == 60 ? 60 : 30) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn3", "src_framerate",
			main_fps == 60 ? 60 : 30) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn3", "dst_framerate",
			main_fps == 60 ? 60 : 30) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp2", "src_framerate", OVIS_AI_FRAME_RATE_AUTO) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp2", "dst_framerate", OVIS_AI_FRAME_RATE_AUTO) != 0)
		return -1;
	if ((enabled[0] || enabled[3]) + enabled[1] + enabled[2] > 1)
		needs_update = 1;
	if (enabled[0]) {
		enabled[1] = 0;
		enabled[2] = 0;
	} else if (enabled[1]) {
		enabled[2] = 0;
		enabled[3] = 0;
	} else if (enabled[2]) {
		enabled[3] = 0;
	}
	shared_ai_source_enabled = enabled[2] || enabled[3];
	if (set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_1", "bEnable", shared_ai_source_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_2", "bEnable", enabled[0]) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_3", "bEnable", enabled[1]) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_4", "bEnable", motion_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_7", "bEnable", enabled[3]) != 0)
		return -1;
	snprintf(updates[UVC_SOURCE_FPS_UPDATE_INDEX].value,
		sizeof(updates[UVC_SOURCE_FPS_UPDATE_INDEX].value), "%d",
		main_fps == 60 ? 60 : 30);
	if (!enabled[3]) {
		int shared_width = enabled[2] ? human_pose_width : OVIS_AI_TRACK_DET_WIDTH;
		int shared_height = enabled[2] ? human_pose_height : OVIS_AI_TRACK_DET_HEIGHT;

		snprintf(updates[SHARED_CHANNEL_WIDTH_UPDATE_INDEX].value,
			sizeof(updates[SHARED_CHANNEL_WIDTH_UPDATE_INDEX].value), "%d",
			shared_width);
		snprintf(updates[SHARED_CHANNEL_HEIGHT_UPDATE_INDEX].value,
			sizeof(updates[SHARED_CHANNEL_HEIGHT_UPDATE_INDEX].value), "%d",
			shared_height);
		snprintf(updates[SHARED_POOL_WIDTH_UPDATE_INDEX].value,
			sizeof(updates[SHARED_POOL_WIDTH_UPDATE_INDEX].value), "%d",
			shared_width);
		snprintf(updates[SHARED_POOL_HEIGHT_UPDATE_INDEX].value,
			sizeof(updates[SHARED_POOL_HEIGHT_UPDATE_INDEX].value), "%d",
			shared_height);
	}
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
	group_enabled[0] = enabled[0];
	group_enabled[1] = enabled[1];
	group_enabled[2] = motion_enabled;
	group_enabled[3] = 0;
	for (index = 0; index < AI_GROUP_UPDATE_COUNT; index++) {
		int current;

		if (read_int(path, updates[AI_GROUP_UPDATE_BASE + index].section,
				updates[AI_GROUP_UPDATE_BASE + index].key, &current) != 0 ||
		    current != group_enabled[index]) {
			needs_update = 1;
			break;
		}
	}
	sub_dependent_enabled[0] = runtime_sub_enabled;
	sub_dependent_enabled[1] = runtime_sub_enabled;
	sub_dependent_enabled[2] = runtime_sub_enabled && osd_enabled;
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
		    current != shared_ai_source_enabled)
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
		shared_ai_source_enabled);
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
		{ "ai_object_track_config", "sot_min_observed_score", "", 0 },
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
		{ "output_config", "rtsp_enable", "", 0 },
		{ "output_config", "uvc_enable", "", 0 },
		{ "output_config", "sub_enable", "", 0 },
		{ "vb_pool_6", "bEnable", "", 0 },
		{ "vpssgrp1", "grp_enable", "", 0 },
		{ "vencchn0", "bEnable", "", 0 },
		{ "vb_pool_8", "bEnable", "", 0 },
		{ "vpssgrp6", "grp_enable", "", 0 },
		{ "vpssgrp6.chn0", "chn_enable", "", 0 },
		{ "vencchn3", "bEnable", "", 0 },
		{ "rtsp_config", "rtsp_cnt", "", 0 },
		{ "ai_pd_config", "grp_width", "", 0 },
		{ "ai_pd_config", "grp_height", "", 0 },
		{ "vpssgrp2.chn0", "width", "", 0 },
		{ "vpssgrp2.chn0", "height", "", 0 },
		{ "vb_pool_2", "frame_width", "", 0 },
		{ "vb_pool_2", "frame_height", "", 0 },
		{ "ai_fd_config", "grp_width", "", 0 },
		{ "ai_fd_config", "grp_height", "", 0 },
		{ "vpssgrp3.chn0", "width", "", 0 },
		{ "vpssgrp3.chn0", "height", "", 0 },
		{ "vb_pool_3", "frame_width", "", 0 },
		{ "vb_pool_3", "frame_height", "", 0 },
		{ "ai_md_config", "grp_width", "", 0 },
		{ "ai_md_config", "grp_height", "", 0 },
		{ "vpssgrp4.chn0", "width", "", 0 },
		{ "vpssgrp4.chn0", "height", "", 0 },
		{ "vb_pool_4", "frame_width", "", 0 },
		{ "vb_pool_4", "frame_height", "", 0 },
		{ "ai_human_keypoint_config", "model_width", "", 0 },
		{ "ai_human_keypoint_config", "model_height", "", 0 },
		{ "ai_object_track_config", "grp_width", "", 0 },
		{ "ai_object_track_config", "grp_height", "", 0 },
		{ "ai_object_track_config", "sot_grp_width", "", 0 },
		{ "ai_object_track_config", "sot_grp_height", "", 0 },
		{ "vpssgrp0.chn2", "width", "", 0 },
		{ "vpssgrp0.chn2", "height", "", 0 },
		{ "vb_pool_1", "frame_width", "", 0 },
		{ "vb_pool_1", "frame_height", "", 0 },
		{ "vb_pool_1", "bEnable", "", 0 },
		{ "vb_pool_2", "bEnable", "", 0 },
		{ "vb_pool_3", "bEnable", "", 0 },
		{ "vb_pool_4", "bEnable", "", 0 },
		{ "vb_pool_7", "bEnable", "", 0 },
		{ "vpssgrp6.chn0", "dst_framerate", "", 0 },
		{ "vencchn3", "src_framerate", "", 0 },
		{ "vencchn3", "dst_framerate", "", 0 },
		{ "vpssgrp2", "src_framerate", "", 0 },
		{ "vpssgrp2", "dst_framerate", "", 0 },
		{ "vencchn3", "src_dev_id", "0", 0 },
		{ "vencchn3", "vpss_grp", "0", 0 },
		{ "vi_cfg_isp0", "teaisp_bnr_enable", "", 0 },
		{ "osd_style", "detection_enabled", "", 0 },
		{ "osd_style", "detection_color_mode", "", 0 },
		{ "osd_style", "detection_color", "", 0 },
		{ "osd_style", "detection_thickness", "", 0 },
		{ "osd_style", "detection_label_mode", "", 0 },
		{ "osd_style", "tracking_enabled", "", 0 },
		{ "osd_style", "tracking_color", "", 0 },
		{ "osd_style", "tracking_lost_color", "", 0 },
		{ "osd_style", "tracking_thickness", "", 0 },
		{ "osd_style", "reticle_enabled", "", 0 },
		{ "osd_style", "reticle_template", "", 0 },
		{ "osd_style", "reticle_idle_color", "", 0 },
		{ "osd_style", "reticle_ready_color", "", 0 },
		{ "osd_style", "reticle_thickness", "", 0 },
		{ "osd_style", "reticle_show_while_tracking", "", 0 },
		{ "osd_style", "text_enabled", "", 0 },
		{ "osd_style", "text_main_enabled", "", 0 },
		{ "osd_style", "text_sub_enabled", "", 0 },
		{ "osd_style", "text_position", "", 0 },
		{ "osdc0_obj_info1", "bShow", "", 0 },
		{ "osdc0_obj_info1", "color", "", 0 },
		{ "osdc0_obj_info1", "x1", "", 0 },
		{ "osdc0_obj_info1", "y1", "", 0 },
		{ "osdc0_obj_info1", "str", "", 0 },
		{ "osdc1_obj_info1", "bShow", "", 0 },
		{ "osdc1_obj_info1", "color", "", 0 },
		{ "osdc1_obj_info1", "x1", "", 0 },
		{ "osdc1_obj_info1", "y1", "", 0 },
		{ "osdc1_obj_info1", "str", "", 0 },
		{ "display_config", "vo_cnt", "", 0 },
		{ "vpssgrp7", "grp_enable", "", 0 },
		{ "vb_pool_9", "bEnable", "", 0 },
		{ "ai_pd_config", "model_id", "", 0 },
		{ "ai_pd_config", "model_path", "", 0 },
	};
	char validation_error[256];
	char value[160];
	int runtime_sub_enabled;
	size_t model_update_index = sizeof(updates) / sizeof(updates[0]) - 2;
	enum {
		STAGE_AI_SOURCE_UPDATE_INDEX = 29,
		STAGE_UVC_SOURCE_FPS_UPDATE_INDEX = 30,
	};

	runtime_sub_enabled = values->rtsp_enabled && values->sub_enabled &&
		!(values->uvc_enabled && values->main_fps == 60);
	if (values->object_model_update &&
	    access(values->object_model_path, R_OK) != 0) {
		snprintf(error, error_size, "内置目标检测模型文件不可用");
		return -1;
	}

	snprintf(updates[0].value, sizeof(updates[0].value), "%d", values->main_fps);
	snprintf(updates[1].value, sizeof(updates[1].value), "%d", values->main_bitrate);
	snprintf(updates[2].value, sizeof(updates[2].value), "%d", values->main_bitrate);
	snprintf(updates[3].value, sizeof(updates[3].value), "%d",
		runtime_sub_enabled);
	snprintf(updates[4].value, sizeof(updates[4].value), "%d", values->sub_fps);
	snprintf(updates[5].value, sizeof(updates[5].value), "%d", values->sub_bitrate);
	snprintf(updates[6].value, sizeof(updates[6].value), "%d", values->sub_bitrate);
	snprintf(updates[7].value, sizeof(updates[7].value), "%d", values->osd_enabled);
	snprintf(updates[8].value, sizeof(updates[8].value), "%d", values->object_enabled);
	snprintf(updates[9].value, sizeof(updates[9].value), "%.6g", values->object_threshold);
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
	if (values->object_model_update) {
		snprintf(updates[model_update_index].value,
			sizeof(updates[model_update_index].value), "%s",
			values->object_model_id);
		snprintf(updates[model_update_index + 1].value,
			sizeof(updates[model_update_index + 1].value), "\"%s\"",
			values->object_model_path);
	} else if (read_ini_value(OVIS_CONFIG_FILE, "ai_pd_config", "model_id",
			updates[model_update_index].value,
			sizeof(updates[model_update_index].value)) != 0 ||
		   read_ini_value(OVIS_CONFIG_FILE, "ai_pd_config", "model_path",
			updates[model_update_index + 1].value,
			sizeof(updates[model_update_index + 1].value)) != 0) {
		snprintf(error, error_size, "无法读取当前目标检测模型配置");
		return -1;
	}
	snprintf(updates[20].value, sizeof(updates[20].value), "%s",
		values->main_fps == 60 ? OVIS_SC235HAI_60FPS_SNS_TYPE : OVIS_SC235HAI_30FPS_SNS_TYPE);
	snprintf(updates[21].value, sizeof(updates[21].value), "%d",
		values->main_fps == 60 ? 60 : 30);
	snprintf(updates[22].value, sizeof(updates[22].value), "%d", values->object_enabled);
	snprintf(updates[23].value, sizeof(updates[23].value), "%d", values->face_enabled);
	snprintf(updates[24].value, sizeof(updates[24].value), "%d", values->motion_enabled);
	snprintf(updates[25].value, sizeof(updates[25].value), "%d", 0);
	snprintf(updates[26].value, sizeof(updates[26].value), "%d",
		runtime_sub_enabled);
	snprintf(updates[27].value, sizeof(updates[27].value), "%d",
		runtime_sub_enabled);
	snprintf(updates[28].value, sizeof(updates[28].value), "%d",
		runtime_sub_enabled && values->osd_enabled);
	snprintf(updates[STAGE_AI_SOURCE_UPDATE_INDEX].value,
		sizeof(updates[STAGE_AI_SOURCE_UPDATE_INDEX].value), "%d",
		values->human_pose_enabled || values->object_tracking_enabled);
	snprintf(updates[STAGE_UVC_SOURCE_FPS_UPDATE_INDEX].value,
		sizeof(updates[STAGE_UVC_SOURCE_FPS_UPDATE_INDEX].value), "%d",
		values->main_fps == 60 ? 60 : 30);
	if (set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vi_cfg_isp0", "teaisp_bnr_enable", values->ai_bnr_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"output_config", "rtsp_enable", values->rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"output_config", "uvc_enable", values->uvc_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"display_config", "vo_cnt", values->display_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp7", "grp_enable", values->display_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_9", "bEnable", values->display_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"output_config", "sub_enable", values->sub_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_6", "bEnable", values->rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp1", "grp_enable", values->rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn0", "bEnable", values->rtsp_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_8", "bEnable", 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp6", "grp_enable", 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp6.chn0", "chn_enable", 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn3", "bEnable", values->uvc_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"rtsp_config", "rtsp_cnt", values->rtsp_enabled ? 2 : 0) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_1", "bEnable",
			values->human_pose_enabled || values->object_tracking_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_2", "bEnable", values->object_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_3", "bEnable", values->face_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_4", "bEnable", values->motion_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vb_pool_7", "bEnable", values->object_tracking_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp6.chn0", "dst_framerate",
			values->main_fps == 60 ? 60 : 30) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn3", "src_framerate",
			values->main_fps == 60 ? 60 : 30) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vencchn3", "dst_framerate",
			values->main_fps == 60 ? 60 : 30) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp2", "src_framerate", OVIS_AI_FRAME_RATE_AUTO) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"vpssgrp2", "dst_framerate", OVIS_AI_FRAME_RATE_AUTO) != 0) {
		snprintf(error, error_size, "无法生成输出服务或 AI 资源配置");
		return -1;
	}
	if (set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "detection_enabled",
			values->detection_osd_enabled) != 0 ||
	    set_update_value(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "detection_color_mode",
			values->detection_color_mode) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "detection_thickness",
			values->detection_thickness) != 0 ||
	    set_update_value(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "detection_label_mode",
			values->detection_label_mode) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "tracking_enabled",
			values->tracking_osd_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "tracking_thickness",
			values->tracking_thickness) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "reticle_enabled", values->reticle_enabled) != 0 ||
	    set_update_value(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "reticle_template",
			values->reticle_template) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "reticle_thickness",
			values->reticle_thickness) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "reticle_show_while_tracking",
			values->reticle_show_while_tracking) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "text_enabled", values->text_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "text_main_enabled",
			values->text_main_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "text_sub_enabled",
			values->text_sub_enabled) != 0 ||
	    set_update_value(updates, sizeof(updates) / sizeof(updates[0]),
			"osd_style", "text_position", values->text_position) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc0_obj_info1", "bShow",
			values->text_enabled && values->text_main_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc0_obj_info1", "x1", values->text_x) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc0_obj_info1", "y1", values->text_y) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc1_obj_info1", "bShow",
			values->text_enabled && values->text_sub_enabled) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc1_obj_info1", "x1", values->text_x) != 0 ||
	    set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc1_obj_info1", "y1", values->text_y) != 0) {
		snprintf(error, error_size, "无法生成 OSD 样式配置");
		return -1;
	}
#define SET_OSD_COLOR(section, key, color_value) \
	do { \
		snprintf(value, sizeof(value), "0x%06x", \
			(unsigned int)((color_value) & 0xffffff)); \
		if (set_update_value(updates, sizeof(updates) / sizeof(updates[0]), \
				(section), (key), value) != 0) { \
			snprintf(error, error_size, "无法生成 OSD 颜色配置"); \
			return -1; \
		} \
	} while (0)
	SET_OSD_COLOR("osd_style", "detection_color", values->detection_color);
	SET_OSD_COLOR("osd_style", "tracking_color", values->tracking_color);
	SET_OSD_COLOR("osd_style", "tracking_lost_color", values->tracking_lost_color);
	SET_OSD_COLOR("osd_style", "reticle_idle_color", values->reticle_idle_color);
	SET_OSD_COLOR("osd_style", "reticle_ready_color", values->reticle_ready_color);
	SET_OSD_COLOR("osdc0_obj_info1", "color", values->text_color);
	SET_OSD_COLOR("osdc1_obj_info1", "color", values->text_color);
#undef SET_OSD_COLOR
	snprintf(value, sizeof(value), "\"%s\"", values->text_content);
	if (set_update_value(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc0_obj_info1", "str", value) != 0 ||
	    set_update_value(updates, sizeof(updates) / sizeof(updates[0]),
			"osdc1_obj_info1", "str", value) != 0) {
		snprintf(error, error_size, "无法生成 OSD 文字配置");
		return -1;
	}
	{
		int shared_width = values->object_tracking_enabled ?
			values->object_tracking_det_width : values->human_pose_enabled ?
			values->human_pose_width : OVIS_AI_TRACK_DET_WIDTH;
		int shared_height = values->object_tracking_enabled ?
			values->object_tracking_det_height : values->human_pose_enabled ?
			values->human_pose_height : OVIS_AI_TRACK_DET_HEIGHT;
		struct {
			const char *section;
			const char *key;
			int value;
		} dimensions[] = {
			{ "ai_pd_config", "grp_width", values->object_width },
			{ "ai_pd_config", "grp_height", values->object_height },
			{ "vpssgrp2.chn0", "width", values->object_width },
			{ "vpssgrp2.chn0", "height", values->object_height },
			{ "vb_pool_2", "frame_width", values->object_width },
			{ "vb_pool_2", "frame_height", values->object_height },
			{ "ai_fd_config", "grp_width", values->face_width },
			{ "ai_fd_config", "grp_height", values->face_height },
			{ "vpssgrp3.chn0", "width", values->face_width },
			{ "vpssgrp3.chn0", "height", values->face_height },
			{ "vb_pool_3", "frame_width", values->face_width },
			{ "vb_pool_3", "frame_height", values->face_height },
			{ "ai_md_config", "grp_width", values->motion_width },
			{ "ai_md_config", "grp_height", values->motion_height },
			{ "vpssgrp4.chn0", "width", values->motion_width },
			{ "vpssgrp4.chn0", "height", values->motion_height },
			{ "vb_pool_4", "frame_width", values->motion_width },
			{ "vb_pool_4", "frame_height", values->motion_height },
			{ "ai_human_keypoint_config", "model_width", values->human_pose_width },
			{ "ai_human_keypoint_config", "model_height", values->human_pose_height },
			{ "ai_object_track_config", "grp_width", values->object_tracking_det_width },
			{ "ai_object_track_config", "grp_height", values->object_tracking_det_height },
			{ "ai_object_track_config", "sot_grp_width", values->object_tracking_sot_width },
			{ "ai_object_track_config", "sot_grp_height", values->object_tracking_sot_height },
			{ "vpssgrp0.chn2", "width", shared_width },
			{ "vpssgrp0.chn2", "height", shared_height },
			{ "vb_pool_1", "frame_width", shared_width },
			{ "vb_pool_1", "frame_height", shared_height },
		};
		size_t index;

		for (index = 0; index < sizeof(dimensions) / sizeof(dimensions[0]); index++) {
			if (set_update_int(updates, sizeof(updates) / sizeof(updates[0]),
					dimensions[index].section, dimensions[index].key,
					dimensions[index].value) != 0) {
				snprintf(error, error_size, "无法生成 AI 输入帧尺寸配置");
				return -1;
			}
		}
	}
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
	struct config_values active_values;
	char requested_revision[33];
	char active_revision[17];
	char staged_revision[17];
	cJSON *issues;
	int overlay_only;
	int result = -3;

	error[0] = '\0';
	if (parse_payload(body, &values, requested_revision, error, error_size) != 0)
		return -1;
	if (load_values(OVIS_CONFIG_FILE, &active_values) != 0) {
		snprintf(error, error_size, "无法读取当前配置");
		return -3;
	}
	if (!values.ai_bnr_present)
		values.ai_bnr_enabled = active_values.ai_bnr_enabled;
	if (!values.object_model_update) {
		snprintf(values.object_model_id, sizeof(values.object_model_id), "%s",
			active_values.object_model_id);
		snprintf(values.object_model_path, sizeof(values.object_model_path), "%s",
			active_values.object_model_path);
	}
	inherit_display_values(&values, &active_values);
	inherit_overlay_values(&values, &active_values);
	overlay_only = non_overlay_values_equal(&values, &active_values);
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
			"\"restart_required\":%s,\"reload_required\":%s}",
			staged_revision, overlay_only ? "false" : "true",
			overlay_only ? "true" : "false") >= (int)size) {
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

static int model_id_is_valid(const char *model_id)
{
	size_t index;

	if (model_id == NULL || strncmp(model_id, "TDL_MODEL_", 10) != 0 ||
	    strlen(model_id) >= 80)
		return 0;
	for (index = 10; model_id[index] != '\0'; index++) {
		if (!(model_id[index] >= 'A' && model_id[index] <= 'Z') &&
		    !(model_id[index] >= '0' && model_id[index] <= '9') &&
		    model_id[index] != '_')
			return 0;
	}
	return index > 10;
}

int config_stage_detection_model(const char *model_id, const char *model_path,
	const char *model_config_path, double threshold, int processing_width,
	int processing_height, int enabled,
	char revision[17], char *error, size_t error_size)
{
	struct ini_update updates[] = {
		{ "ai_pd_config", "model_id", "", 0 },
		{ "ai_pd_config", "model_path", "", 0 },
		{ "ai_pd_config", "model_path_cfg", "", 0 },
		{ "ai_pd_config", "threshold", "", 0 },
		{ "ai_pd_config", "pd_enable", "", 0 },
		{ "ai_fd_config", "fd_enable", "0", 0 },
		{ "ai_human_keypoint_config", "human_keypoint_enable", "0", 0 },
		{ "ai_object_track_config", "object_track_enable", "", 0 },
		{ "vpssgrp2", "grp_enable", "", 0 },
		{ "vpssgrp3", "grp_enable", "0", 0 },
		{ "vpssgrp0.chn2", "chn_enable", "", 0 },
		{ "ai_pd_config", "grp_width", "", 0 },
		{ "ai_pd_config", "grp_height", "", 0 },
		{ "vpssgrp2.chn0", "width", "", 0 },
		{ "vpssgrp2.chn0", "height", "", 0 },
		{ "vb_pool_2", "frame_width", "", 0 },
		{ "vb_pool_2", "frame_height", "", 0 },
		{ "vpssgrp0.chn2", "width", "", 0 },
		{ "vpssgrp0.chn2", "height", "", 0 },
		{ "vb_pool_1", "frame_width", "", 0 },
		{ "vb_pool_1", "frame_height", "", 0 },
		{ "vb_pool_1", "bEnable", "", 0 },
		{ "vb_pool_2", "bEnable", "", 0 },
		{ "vb_pool_3", "bEnable", "0", 0 },
		{ "vb_pool_4", "bEnable", "", 0 },
		{ "vb_pool_7", "bEnable", "", 0 },
	};
	char validation_error[256];
	int motion_enabled = 0;
	int object_tracking_enabled = 0;
	int custom_paths;
	int builtin_paths;
	int result = -1;

	error[0] = '\0';
	custom_paths = model_path != NULL && model_config_path != NULL &&
		strncmp(model_path, OVIS_MODEL_STORE_DIR "/", strlen(OVIS_MODEL_STORE_DIR) + 1) == 0 &&
		strncmp(model_config_path, OVIS_MODEL_STORE_DIR "/", strlen(OVIS_MODEL_STORE_DIR) + 1) == 0;
	builtin_paths = model_path != NULL && model_config_path != NULL &&
		(strcmp(model_path,
			"/usr/share/ipcamera/cv184x/yolov8n_det_monitor_person_256_448_INT8_cv184x.bmodel") == 0 ||
		 strcmp(model_path,
			"/usr/share/ipcamera/cv184x/yolov8n_det_person_vehicle_384_640_INT8_cv184x.bmodel") == 0) &&
		strcmp(model_config_path, "/usr/share/ipcamera/model_factory.json") == 0;
	if (!model_id_is_valid(model_id)) {
		snprintf(error, error_size, "检测模型运行类型无效");
		return -1;
	}
	if (threshold < 0 || threshold > 1) {
		snprintf(error, error_size, "检测阈值必须在 0 到 1 之间");
		return -1;
	}
	if (!processing_size_valid(processing_width, processing_height,
			OVIS_AI_OBJECT_FRAME_MAX_WIDTH, OVIS_AI_OBJECT_FRAME_MAX_HEIGHT)) {
		snprintf(error, error_size,
			"AI 输入帧尺寸必须为 160x96 到 1920x1080 范围内的偶数");
		return -1;
	}
	if (enabled != 0 && enabled != 1) {
		snprintf(error, error_size, "检测模型启用状态无效");
		return -1;
	}
	if (model_path == NULL || model_config_path == NULL ||
	    (!custom_paths && !builtin_paths) ||
	    strchr(model_path, '\n') != NULL || strchr(model_path, '"') != NULL ||
	    strchr(model_config_path, '\n') != NULL || strchr(model_config_path, '"') != NULL) {
		snprintf(error, error_size, "检测模型文件路径无效");
		return -1;
	}
	if (access(model_path, R_OK) != 0 || access(model_config_path, R_OK) != 0) {
		snprintf(error, error_size, "检测模型文件或配置文件不可用");
		return -1;
	}
	pthread_mutex_lock(&config_lock);
	if (ensure_ini_key(OVIS_CONFIG_FILE, "ai_pd_config", "model_path_cfg",
			"\"/usr/share/ipcamera/model_factory.json\"") != 0 ||
	    read_int(OVIS_CONFIG_FILE, "ai_md_config", "md_enable", &motion_enabled) != 0 ||
	    read_int(OVIS_CONFIG_FILE, "ai_object_track_config",
			"object_track_enable", &object_tracking_enabled) != 0) {
		snprintf(error, error_size, "无法准备检测模型运行配置");
		goto done;
	}
	snprintf(updates[0].value, sizeof(updates[0].value), "%s", model_id);
	snprintf(updates[1].value, sizeof(updates[1].value), "\"%s\"", model_path);
	snprintf(updates[2].value, sizeof(updates[2].value), "\"%s\"", model_config_path);
	snprintf(updates[3].value, sizeof(updates[3].value), "%.6g", threshold);
	snprintf(updates[4].value, sizeof(updates[4].value), "%d", enabled);
	snprintf(updates[7].value, sizeof(updates[7].value), "%d",
		object_tracking_enabled);
	snprintf(updates[8].value, sizeof(updates[8].value), "%d", enabled);
	snprintf(updates[10].value, sizeof(updates[10].value), "%d",
		object_tracking_enabled);
	snprintf(updates[11].value, sizeof(updates[11].value), "%d", processing_width);
	snprintf(updates[12].value, sizeof(updates[12].value), "%d", processing_height);
	snprintf(updates[13].value, sizeof(updates[13].value), "%d", processing_width);
	snprintf(updates[14].value, sizeof(updates[14].value), "%d", processing_height);
	snprintf(updates[15].value, sizeof(updates[15].value), "%d", processing_width);
	snprintf(updates[16].value, sizeof(updates[16].value), "%d", processing_height);
	snprintf(updates[17].value, sizeof(updates[17].value), "%d", OVIS_AI_TRACK_DET_WIDTH);
	snprintf(updates[18].value, sizeof(updates[18].value), "%d", OVIS_AI_TRACK_DET_HEIGHT);
	snprintf(updates[19].value, sizeof(updates[19].value), "%d", OVIS_AI_TRACK_DET_WIDTH);
	snprintf(updates[20].value, sizeof(updates[20].value), "%d", OVIS_AI_TRACK_DET_HEIGHT);
	snprintf(updates[21].value, sizeof(updates[21].value), "%d",
		object_tracking_enabled);
	snprintf(updates[22].value, sizeof(updates[22].value), "%d", enabled);
	snprintf(updates[24].value, sizeof(updates[24].value), "%d", motion_enabled);
	snprintf(updates[25].value, sizeof(updates[25].value), "%d",
		object_tracking_enabled);
	if (write_updates(OVIS_CONFIG_FILE, OVIS_CONFIG_PENDING, updates,
			sizeof(updates) / sizeof(updates[0])) != 0) {
		snprintf(error, error_size, "无法创建检测模型待应用配置");
		goto done;
	}
	if (config_validate_file(OVIS_CONFIG_PENDING, validation_error,
			sizeof(validation_error)) != 0) {
		unlink(OVIS_CONFIG_PENDING);
		snprintf(error, error_size, "检测模型配置校验失败: %s", validation_error);
		goto done;
	}
	if (revision_for_file(OVIS_CONFIG_PENDING, revision) != 0) {
		unlink(OVIS_CONFIG_PENDING);
		snprintf(error, error_size, "无法生成检测模型配置版本");
		goto done;
	}
	result = 0;
	audit_log(enabled ? "model.config.activate" : "model.config.deactivate", "success");
done:
	pthread_mutex_unlock(&config_lock);
	return result;
}

int config_rebase_backup_away_from_model(const char *id,
	char *error, size_t error_size)
{
	char path[512];
	char needle[512];
	int result = 0;

	if (id == NULL || strlen(id) != 16) {
		snprintf(error, error_size, "模型 ID 无效");
		return -1;
	}
	pthread_mutex_lock(&config_lock);
	if (access(OVIS_CONFIG_BACKUP, F_OK) == 0 &&
	    read_ini_value(OVIS_CONFIG_BACKUP, "ai_pd_config", "model_path",
			path, sizeof(path)) == 0) {
		snprintf(needle, sizeof(needle), "%s/%s/model.bmodel",
			OVIS_MODEL_STORE_DIR, id);
		if (strstr(path, needle) != NULL &&
		    atomic_copy(OVIS_CONFIG_FILE, OVIS_CONFIG_BACKUP) != 0) {
			snprintf(error, error_size, "无法更新模型关联的回滚配置");
			result = -1;
		}
	}
	pthread_mutex_unlock(&config_lock);
	return result;
}

static int apply_staged_locked(const char *revision, char *message,
	size_t message_size, int *rolled_back)
{
	struct config_values old_values;
	struct config_values new_values;
	char staged_revision[17];
	char validation_error[256];
	char service_output[512];
	char rollback_output[512];
	int old_uvc_enabled;
	int new_uvc_enabled;
	int uvc_changed;
	int overlay_only;
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
	if (load_values(OVIS_CONFIG_FILE, &old_values) != 0 ||
	    load_values(OVIS_CONFIG_PENDING, &new_values) != 0) {
		snprintf(message, message_size, "无法读取 UVC 输出状态");
		return -1;
	}
	old_uvc_enabled = old_values.uvc_enabled;
	new_uvc_enabled = new_values.uvc_enabled;
	uvc_changed = old_uvc_enabled != new_uvc_enabled;
	overlay_only = non_overlay_values_equal(&old_values, &new_values);
	if (atomic_copy(OVIS_CONFIG_FILE, OVIS_CONFIG_BACKUP) != 0) {
		snprintf(message, message_size, "备份当前配置失败");
		return -1;
	}
	if (atomic_copy(OVIS_CONFIG_PENDING, OVIS_CONFIG_FILE) != 0) {
		snprintf(message, message_size, "切换到新配置失败");
		return -1;
	}
	unlink(OVIS_CONFIG_PENDING);
	if (overlay_only) {
		restart_result = service_reload_overlay(service_output,
			sizeof(service_output));
	} else if (uvc_changed) {
		restart_result = usb_schedule_output_reboot(service_output,
			sizeof(service_output));
	} else {
		restart_result = service_run_action(SERVICE_RESTART, service_output,
			sizeof(service_output));
	}
	if (restart_result == 0) {
		snprintf(message, message_size, "%s", overlay_only ?
			"OSD 配置已实时应用" : uvc_changed ?
			"配置已保存，设备正在重启" : "配置应用成功");
		audit_log("config.apply", "success");
		return 0;
	}
	rollback_result = atomic_copy(OVIS_CONFIG_BACKUP, OVIS_CONFIG_FILE);
	if (rollback_result == 0 && !uvc_changed) {
		if (overlay_only) {
			rollback_result = service_reload_overlay(rollback_output,
				sizeof(rollback_output));
		} else {
			rollback_result = service_run_action(SERVICE_RESTART,
				rollback_output, sizeof(rollback_output));
		}
	}
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

static int files_equal(const char *left, const char *right)
{
	unsigned char a[4096], b[4096];
	FILE *first = fopen(left, "rb");
	FILE *second = fopen(right, "rb");
	int equal = 0;

	if (first == NULL || second == NULL)
		goto done;
	for (;;) {
		size_t size_a = fread(a, 1, sizeof(a), first);
		size_t size_b = fread(b, 1, sizeof(b), second);

		if (ferror(first) || ferror(second) || size_a != size_b ||
		    memcmp(a, b, size_a) != 0)
			break;
		if (size_a == 0) {
			equal = 1;
			break;
		}
	}
done:
	if (first != NULL) fclose(first);
	if (second != NULL) fclose(second);
	return equal;
}

static int migrate_runtime_config(const char *path)
{
	char directory[] = "/tmp/ovis-migrate-XXXXXX";
	char work[256];
	char entry_path[512];
	DIR *dir;
	struct dirent *entry;
	int result = -1;

	if (mkdtemp(directory) == NULL)
		return -1;
	snprintf(work, sizeof(work), "%s/config.ini", directory);
	/* Legacy migrations may rewrite individual keys. Keep those writes in RAM. */
	if (copy_file(path, work) == 0 && migrate_runtime_config_work(work) == 0)
		result = files_equal(path, work) ? 0 : atomic_copy(work, path);
	ini_snapshot_end();
	dir = opendir(directory);
	if (dir != NULL) {
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			snprintf(entry_path, sizeof(entry_path), "%s/%s", directory, entry->d_name);
			unlink(entry_path);
		}
		closedir(dir);
	}
	rmdir(directory);
	return result;
}

static int config_ensure_runtime_uncached(char *error, size_t error_size)
{
	char runtime_error[256] = "未找到运行配置";
	char backup_error[256] = "未找到备份配置";
	char default_error[256] = "未知错误";
	int migration_result = 0;

	ensure_dir("/mnt/cfg");
	if (ensure_dir(OVIS_CONFIG_DIR) != 0) {
		snprintf(error, error_size, "无法创建运行配置目录");
		return -1;
	}
	unlink(OVIS_CONFIG_PENDING);
	if (access(OVIS_CONFIG_FILE, F_OK) == 0) {
		migration_result = migrate_runtime_config(OVIS_CONFIG_FILE);
		if (migration_result != 0)
			snprintf(runtime_error, sizeof(runtime_error), "运行配置迁移失败");
		else if (config_validate_file(OVIS_CONFIG_FILE, runtime_error,
				sizeof(runtime_error)) == 0)
			return 0;
	}
	if (access(OVIS_CONFIG_FILE, F_OK) == 0)
		rename(OVIS_CONFIG_FILE, OVIS_CONFIG_FILE ".corrupt");
	if (access(OVIS_CONFIG_BACKUP, F_OK) == 0) {
		migration_result = migrate_runtime_config(OVIS_CONFIG_BACKUP);
		if (migration_result != 0) {
			snprintf(backup_error, sizeof(backup_error), "备份配置迁移失败");
		} else if (config_validate_file(OVIS_CONFIG_BACKUP, backup_error,
				sizeof(backup_error)) == 0) {
			if (atomic_copy(OVIS_CONFIG_BACKUP, OVIS_CONFIG_FILE) == 0)
				return 0;
			snprintf(backup_error, sizeof(backup_error), "无法恢复备份配置");
		}
	}
	if (config_validate_file(OVIS_DEFAULT_CONFIG, default_error,
			sizeof(default_error)) == 0) {
		if (atomic_copy(OVIS_DEFAULT_CONFIG, OVIS_CONFIG_FILE) == 0)
			return 0;
		snprintf(default_error, sizeof(default_error), "无法复制默认配置");
	}
	snprintf(error, error_size, "默认配置不可用: %s；运行配置: %s；备份配置: %s",
		default_error, runtime_error, backup_error);
	return -1;
}

#ifndef OVIS_CONFIG_CACHE_DIR
#define OVIS_CONFIG_CACHE_DIR "/run/ovis-config"
#endif
/* Bump the schema when migration rules change; builds also invalidate the cache. */
#define OVIS_CONFIG_CACHE_VERSION "1 " __DATE__ " " __TIME__

int config_ensure_runtime(char *error, size_t error_size)
{
	const char *stamp = OVIS_CONFIG_CACHE_DIR "/validated.ini";
	const char *version_path = OVIS_CONFIG_CACHE_DIR "/version";
	char version[128] = "";
	FILE *file;
	int lock_fd = -1;
	int result;

	pthread_mutex_lock(&config_lock);
	if (ensure_dir(OVIS_CONFIG_CACHE_DIR) == 0) {
		chmod(OVIS_CONFIG_CACHE_DIR, 0700);
		lock_fd = open(OVIS_CONFIG_CACHE_DIR "/prepare.lock", O_CREAT | O_RDWR, 0600);
	}
	if (lock_fd >= 0 && flock(lock_fd, LOCK_EX) != 0) {
		close(lock_fd);
		lock_fd = -1;
	}
	if (lock_fd >= 0) {
		file = fopen(version_path, "r");
		if (file != NULL) {
			if (fgets(version, sizeof(version), file) == NULL)
				version[0] = '\0';
			fclose(file);
		}
		if (strcmp(version, OVIS_CONFIG_CACHE_VERSION) == 0 &&
		    files_equal(OVIS_CONFIG_FILE, stamp)) {
			result = 0;
			goto done;
		}
		unlink(stamp);
	}
	config_snapshot_active = 1;
	result = config_ensure_runtime_uncached(error, error_size);
	if (result == 0 && lock_fd >= 0 && atomic_copy(OVIS_CONFIG_FILE, stamp) == 0) {
		file = fopen(version_path, "w");
		if (file != NULL) {
			fputs(OVIS_CONFIG_CACHE_VERSION, file);
			if (fclose(file) != 0)
				unlink(stamp);
		} else {
			unlink(stamp);
		}
	}
done:
	config_snapshot_active = 0;
	ini_snapshot_end();
	if (lock_fd >= 0) {
		flock(lock_fd, LOCK_UN);
		close(lock_fd);
	}
	pthread_mutex_unlock(&config_lock);
	return result;
}
