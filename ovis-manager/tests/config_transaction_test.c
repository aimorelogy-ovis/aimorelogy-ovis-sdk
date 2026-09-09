#include "ovis_manager.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int fail_service_calls;
static int service_calls;
static int fail_usb_reboot_calls;
static int usb_reboot_calls;

int service_run_action(enum service_action action, char *output, size_t size)
{
	(void)action;
	service_calls++;
	if (fail_service_calls > 0) {
		fail_service_calls--;
		snprintf(output, size, "simulated restart failure");
		return 1;
	}
	snprintf(output, size, "simulated restart success");
	return 0;
}

int usb_schedule_output_reboot(char *output, size_t size)
{
	usb_reboot_calls++;
	if (fail_usb_reboot_calls > 0) {
		fail_usb_reboot_calls--;
		snprintf(output, size, "simulated USB output reboot failure");
		return 1;
	}
	snprintf(output, size, "simulated USB output reboot success");
	return 0;
}

int service_reload_overlay(char *output, size_t size)
{
	snprintf(output, size, "simulated overlay reload success");
	return 0;
}

void audit_log(const char *operation, const char *result)
{
	(void)operation;
	(void)result;
}

static void fail(const char *message)
{
	fprintf(stderr, "config transaction test failed: %s\n", message);
	exit(1);
}

static void clean_test_directory(void)
{
	unlink("/tmp/ovis-manager-config-test/active.ini");
	unlink("/tmp/ovis-manager-config-test/active.ini.corrupt");
	unlink("/tmp/ovis-manager-config-test/active.ini.tmp");
	unlink("/tmp/ovis-manager-config-test/active.ini.migrated");
	unlink("/tmp/ovis-manager-config-test/active.ini.migrated.tmp");
	unlink("/tmp/ovis-manager-config-test/active.ini.display.tmp");
	unlink("/tmp/ovis-manager-config-test/pending.ini");
	unlink("/tmp/ovis-manager-config-test/pending.ini.tmp");
	unlink("/tmp/ovis-manager-config-test/backup.ini");
	unlink("/tmp/ovis-manager-config-test/backup.ini.tmp");
	rmdir("/tmp/ovis-manager-config-test");
}

static cJSON *read_document(char *revision, size_t revision_size)
{
	char json[8192];
	cJSON *root;
	cJSON *item;

	if (config_read_json(json, sizeof(json)) != 0)
		fail("unable to read configuration document");
	root = cJSON_Parse(json);
	item = cJSON_GetObjectItemCaseSensitive(root, "revision");
	if (!cJSON_IsString(item))
		fail("configuration revision is missing");
	snprintf(revision, revision_size, "%s", item->valuestring);
	return root;
}

static int active_config_contains(const char *text)
{
	char line[1024];
	FILE *file = fopen("/tmp/ovis-manager-config-test/active.ini", "r");

	if (file == NULL)
		return 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		if (strstr(line, text) != NULL) {
			fclose(file);
			return 1;
		}
	}
	fclose(file);
	return 0;
}

static int active_config_value_equals(const char *wanted_section,
	const char *wanted_key, const char *wanted_value)
{
	char line[1024];
	char section[64] = "";
	FILE *file = fopen("/tmp/ovis-manager-config-test/active.ini", "r");

	if (file == NULL)
		return 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *text = line;
		char *equals;
		char *end;

		while (*text == ' ' || *text == '\t')
			text++;
		if (*text == '[') {
			end = strchr(text, ']');
			if (end != NULL) {
				*end = '\0';
				snprintf(section, sizeof(section), "%s", text + 1);
			}
			continue;
		}
		equals = strchr(text, '=');
		if (equals == NULL || strcmp(section, wanted_section) != 0)
			continue;
		*equals = '\0';
		end = equals;
		while (end > text && (end[-1] == ' ' || end[-1] == '\t'))
			*--end = '\0';
		if (strcmp(text, wanted_key) != 0)
			continue;
		text = equals + 1;
		while (*text == ' ' || *text == '\t')
			text++;
		end = text + strcspn(text, ";\r\n");
		while (end > text && (end[-1] == ' ' || end[-1] == '\t'))
			end--;
		*end = '\0';
		fclose(file);
		return strcmp(text, wanted_value) == 0;
	}
	fclose(file);
	return 0;
}

static char *make_payload(cJSON *document, int fps, int bitrate, int sensitivity,
	int sub_enabled, int rtsp_enabled, int uvc_enabled, int motion_enabled,
	int object_tracking_enabled)
{
	cJSON *payload = cJSON_CreateObject();
	cJSON *revision = cJSON_GetObjectItemCaseSensitive(document, "revision");
	cJSON *values = cJSON_Duplicate(
		cJSON_GetObjectItemCaseSensitive(document, "values"), 1);
	cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
	cJSON *outputs = cJSON_GetObjectItemCaseSensitive(values, "outputs");
	cJSON *rtsp = cJSON_GetObjectItemCaseSensitive(outputs, "rtsp");
	cJSON *uvc = cJSON_GetObjectItemCaseSensitive(outputs, "uvc");
	cJSON *main_stream = cJSON_GetObjectItemCaseSensitive(video, "main");
	cJSON *sub_stream = cJSON_GetObjectItemCaseSensitive(video, "sub");
	cJSON *detection = cJSON_GetObjectItemCaseSensitive(values, "detection");
	cJSON *motion = cJSON_GetObjectItemCaseSensitive(detection, "motion");
	cJSON *tracking = cJSON_GetObjectItemCaseSensitive(values, "tracking");
	cJSON *object_tracking = cJSON_GetObjectItemCaseSensitive(
		tracking, "single_object");
	char *json;

	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(main_stream, "fps"), fps);
	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(main_stream, "bitrate_kbps"),
		bitrate);
	cJSON_ReplaceItemInObjectCaseSensitive(sub_stream, "enabled",
		cJSON_CreateBool(sub_enabled));
	cJSON_ReplaceItemInObjectCaseSensitive(rtsp, "enabled",
		cJSON_CreateBool(rtsp_enabled));
	cJSON_ReplaceItemInObjectCaseSensitive(uvc, "enabled",
		cJSON_CreateBool(uvc_enabled));
	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(motion, "sensitivity"),
		sensitivity);
	cJSON_ReplaceItemInObjectCaseSensitive(motion, "enabled",
		cJSON_CreateBool(motion_enabled));
	cJSON_ReplaceItemInObjectCaseSensitive(object_tracking, "enabled",
		cJSON_CreateBool(object_tracking_enabled));
	cJSON_AddItemToObject(payload, "revision", cJSON_Duplicate(revision, 1));
	cJSON_AddItemToObject(payload, "values", values);
	json = cJSON_PrintUnformatted(payload);
	cJSON_Delete(payload);
	return json;
}

static char *make_frontend_payload(cJSON *document, int fps,
	int object_tracking_enabled)
{
	char *json = make_payload(document, fps, 10000, 50, 0, 0, 1, 0,
		object_tracking_enabled);
	cJSON *payload = cJSON_Parse(json);
	cJSON *values = cJSON_GetObjectItemCaseSensitive(payload, "values");
	cJSON *detection = cJSON_GetObjectItemCaseSensitive(values, "detection");
	cJSON *object = cJSON_GetObjectItemCaseSensitive(detection, "object");

	free(json);
	cJSON_ReplaceItemInObjectCaseSensitive(object, "model",
		cJSON_CreateString("builtin.person_detection"));
	json = cJSON_PrintUnformatted(payload);
	cJSON_Delete(payload);
	return json;
}

static char *make_ai_conflict_payload(cJSON *document)
{
	cJSON *payload = cJSON_CreateObject();
	cJSON *revision = cJSON_GetObjectItemCaseSensitive(document, "revision");
	cJSON *values = cJSON_Duplicate(
		cJSON_GetObjectItemCaseSensitive(document, "values"), 1);
	cJSON *detection = cJSON_GetObjectItemCaseSensitive(values, "detection");
	cJSON *object = cJSON_GetObjectItemCaseSensitive(detection, "object");
	cJSON *face = cJSON_GetObjectItemCaseSensitive(detection, "face");
	char *json;

	cJSON_ReplaceItemInObjectCaseSensitive(object, "enabled", cJSON_CreateBool(1));
	cJSON_ReplaceItemInObjectCaseSensitive(face, "enabled", cJSON_CreateBool(1));
	cJSON_AddItemToObject(payload, "revision", cJSON_Duplicate(revision, 1));
	cJSON_AddItemToObject(payload, "values", values);
	json = cJSON_PrintUnformatted(payload);
	cJSON_Delete(payload);
	return json;
}

static char *make_object_frame_payload(cJSON *document, int width, int height)
{
	cJSON *payload = cJSON_CreateObject();
	cJSON *revision = cJSON_GetObjectItemCaseSensitive(document, "revision");
	cJSON *values = cJSON_Duplicate(
		cJSON_GetObjectItemCaseSensitive(document, "values"), 1);
	cJSON *detection = cJSON_GetObjectItemCaseSensitive(values, "detection");
	cJSON *object = cJSON_GetObjectItemCaseSensitive(detection, "object");
	cJSON *size = cJSON_GetObjectItemCaseSensitive(object, "processing_size");
	char *json;

	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(size, "width"), width);
	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(size, "height"), height);
	cJSON_AddItemToObject(payload, "revision", cJSON_Duplicate(revision, 1));
	cJSON_AddItemToObject(payload, "values", values);
	json = cJSON_PrintUnformatted(payload);
	cJSON_Delete(payload);
	return json;
}

static char *make_display_payload(cJSON *document, int enabled, const char *mode)
{
	cJSON *payload = cJSON_CreateObject();
	cJSON *revision = cJSON_GetObjectItemCaseSensitive(document, "revision");
	cJSON *values = cJSON_Duplicate(
		cJSON_GetObjectItemCaseSensitive(document, "values"), 1);
	cJSON *outputs = cJSON_GetObjectItemCaseSensitive(values, "outputs");
	cJSON *display = cJSON_GetObjectItemCaseSensitive(outputs, "display");
	char *json;

	cJSON_ReplaceItemInObjectCaseSensitive(display, "enabled",
		cJSON_CreateBool(enabled));
	cJSON_ReplaceItemInObjectCaseSensitive(display, "mode",
		cJSON_CreateString(mode));
	cJSON_AddItemToObject(payload, "revision", cJSON_Duplicate(revision, 1));
	cJSON_AddItemToObject(payload, "values", values);
	json = cJSON_PrintUnformatted(payload);
	cJSON_Delete(payload);
	return json;
}

static void stage_display_and_apply(int enabled)
{
	char current_revision[33];
	char staged_revision[33];
	char validation[4096];
	char response[4096];
	char error[256];
	char message[512];
	cJSON *document = read_document(current_revision, sizeof(current_revision));
	cJSON *saved;
	cJSON *item;
	char *payload = make_display_payload(document, enabled, OVIS_DISPLAY_MODE);
	int rolled_back = 0;

	cJSON_Delete(document);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 0 ||
	    config_stage_json(payload, response, sizeof(response), error,
			sizeof(error)) != 0)
		fail("display configuration could not be staged");
	free(payload);
	saved = cJSON_Parse(response);
	item = cJSON_GetObjectItemCaseSensitive(saved, "revision");
	if (!cJSON_IsString(item))
		fail("display staged revision is missing");
	snprintf(staged_revision, sizeof(staged_revision), "%s", item->valuestring);
	cJSON_Delete(saved);
	if (config_apply_staged(staged_revision, message, sizeof(message),
			&rolled_back) != 0 || rolled_back)
		fail("display configuration could not be applied");
}

static void stage_and_apply(int fps, int bitrate, int sensitivity, int sub_enabled,
	int rtsp_enabled, int uvc_enabled, int motion_enabled,
	int object_tracking_enabled, int fail_first_restart, int expect_success,
	int expect_rollback)
{
	char current_revision[33];
	char staged_revision[33];
	char validation[4096];
	char response[4096];
	char error[256];
	char message[512];
	cJSON *document = read_document(current_revision, sizeof(current_revision));
	cJSON *saved;
	cJSON *item;
	char *payload = make_payload(document, fps, bitrate, sensitivity, sub_enabled,
		rtsp_enabled, uvc_enabled, motion_enabled, object_tracking_enabled);
	int rolled_back = 0;
	int result;

	cJSON_Delete(document);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 0)
		fail("valid payload was rejected");
	if (config_stage_json(payload, response, sizeof(response), error,
			sizeof(error)) != 0)
		fail("valid payload could not be staged");
	free(payload);
	saved = cJSON_Parse(response);
	item = cJSON_GetObjectItemCaseSensitive(saved, "revision");
	if (!cJSON_IsString(item))
		fail("staged revision is missing");
	snprintf(staged_revision, sizeof(staged_revision), "%s", item->valuestring);
	cJSON_Delete(saved);
	fail_service_calls = fail_first_restart;
	result = config_apply_staged(staged_revision, message, sizeof(message), &rolled_back);
	if ((result == 0) != expect_success || rolled_back != expect_rollback)
		fail("unexpected apply or rollback result");
}

static void test_tracking_options(void)
{
	char revision[33];
	char validation[4096];
	char response[4096];
	char error[256];
	char message[512];
	int rolled_back;
	int step;

	for (step = 0; step < 3; step++) {
		cJSON *document = read_document(revision, sizeof(revision));
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *tracking = cJSON_GetObjectItemCaseSensitive(
			cJSON_GetObjectItemCaseSensitive(values, "tracking"), "single_object");
		cJSON *style = cJSON_GetObjectItemCaseSensitive(
			cJSON_GetObjectItemCaseSensitive(values, "overlay"), "tracking");
		cJSON *saved;
		char *payload;
		int before = service_calls;
		const char *selection = step == 2 ? "point" : "reticle";
		const char *initial = step == 2 ? "target" : "fixed_80";
		const char *shape = step == 2 ? "rectangle" : "corners";

		if (step == 1) {
			/* An older client edits a color without knowing the new fields. */
			cJSON_DeleteItemFromObjectCaseSensitive(tracking, "selection_mode");
			cJSON_DeleteItemFromObjectCaseSensitive(tracking, "initial_box_mode");
			cJSON_DeleteItemFromObjectCaseSensitive(style, "boxStyle");
			cJSON_DeleteItemFromObjectCaseSensitive(style, "hideWhenLost");
			cJSON_ReplaceItemInObjectCaseSensitive(style, "color", cJSON_CreateString("#FFFFFF"));
		} else {
			cJSON_ReplaceItemInObjectCaseSensitive(tracking, "selection_mode", cJSON_CreateString(selection));
			cJSON_ReplaceItemInObjectCaseSensitive(tracking, "initial_box_mode", cJSON_CreateString(initial));
			cJSON_ReplaceItemInObjectCaseSensitive(style, "boxStyle", cJSON_CreateString(shape));
			cJSON_ReplaceItemInObjectCaseSensitive(style, "hideWhenLost", cJSON_CreateBool(step != 2));
		}
		payload = cJSON_PrintUnformatted(document);
		cJSON_Delete(document);
		if (config_validate_json(payload, validation, sizeof(validation), error, sizeof(error)) != 0)
			fail("tracking options validation failed");
		if (step == 1 && strstr(validation, "overlay_reload") == NULL)
			fail("style-only update unexpectedly requires a restart");
		if (config_stage_json(payload, response, sizeof(response), error, sizeof(error)) != 0)
			fail("tracking options staging failed");
		free(payload);
		saved = cJSON_Parse(response);
		snprintf(revision, sizeof(revision), "%s",
			cJSON_GetObjectItemCaseSensitive(saved, "revision")->valuestring);
		cJSON_Delete(saved);
		if (config_apply_staged(revision, message, sizeof(message), &rolled_back) != 0 || rolled_back)
			fail("tracking options apply failed");
		if (service_calls - before != (step == 1 ? 0 : 1))
			fail("tracking options restart behavior is incorrect");
		if (!active_config_value_equals("ai_object_track_config", "selection_mode", selection) ||
		    !active_config_value_equals("ai_object_track_config", "initial_box_mode", initial) ||
		    !active_config_value_equals("osd_style", "tracking_box_style", shape) ||
		    !active_config_value_equals("osd_style", "tracking_hide_when_lost", step == 2 ? "0" : "1"))
			fail("tracking options did not round-trip or were reset by an older client");
	}
	{
		cJSON *document = read_document(revision, sizeof(revision));
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *tracking = cJSON_GetObjectItemCaseSensitive(
			cJSON_GetObjectItemCaseSensitive(values, "tracking"), "single_object");
		char *payload;

		cJSON_ReplaceItemInObjectCaseSensitive(tracking, "selection_mode", cJSON_CreateString("invalid"));
		payload = cJSON_PrintUnformatted(document);
		cJSON_Delete(document);
		if (config_validate_json(payload, validation, sizeof(validation), error, sizeof(error)) == 0)
			fail("invalid tracking selection mode was accepted");
		free(payload);
	}
}

int main(void)
{
	char capabilities[8192];
	char error[256];
	char revision_before[33];
	char revision_after[33];
	char validation[4096];
	char message[512];
	cJSON *document;
	cJSON *motion;
	char *payload;
	int rolled_back = 0;

	clean_test_directory();
	if (mkdir("/tmp/ovis-manager-config-test", 0755) != 0)
		fail("unable to create test directory");
	if (config_ensure_runtime(error, sizeof(error)) != 0)
		fail(error);
	if (!active_config_value_equals("vi_cfg_isp0", "teaisp_bnr_enable", "0"))
		fail("AI BNR migration did not preserve the default disabled state");
	if (config_capabilities_json(capabilities, sizeof(capabilities)) != 0 ||
		    strstr(capabilities, "\"schema_version\":8") == NULL ||
		    strstr(capabilities, "\"display\":{\"supported\":true") == NULL ||
		    strstr(capabilities, "\"id\":\"720x480_60\"") == NULL ||
	    strstr(capabilities, "\"overlay\":{\"supported\":true") == NULL ||
	    strstr(capabilities, "\"reticleTemplates\"") == NULL ||
	    strstr(capabilities, "\"reticle_templates\"") == NULL ||
	    strstr(capabilities, "\"ai_isp\"") == NULL ||
	    strstr(capabilities, "\"required_main_fps\":30") == NULL ||
	    strstr(capabilities, "\"exclusive_with\":[\"object\",\"face\",\"motion\","
		    "\"human_pose\",\"single_object_tracking\"]") == NULL ||
	    strstr(capabilities, "\"supported\":false") == NULL)
		fail("AI BNR capability contract is invalid");
	if (!active_config_value_equals("ai_object_track_config", "sot_vpss_grp", "0") ||
	    !active_config_value_equals("ai_object_track_config", "sot_vpss_chn", "2") ||
	    !active_config_value_equals("ai_object_track_config",
		    "sot_refine_selected_det", "0") ||
	    !active_config_value_equals("vpssgrp0.chn0", "depth", "0") ||
	    !active_config_value_equals("vpssgrp0.chn2", "width", "640") ||
	    !active_config_value_equals("vpssgrp0.chn2", "height", "384") ||
	    !active_config_value_equals("vpssgrp0.chn2", "chn_pixel_fmt",
		    "PIXEL_FORMAT_UINT8_C3_PLANAR") ||
	    !active_config_value_equals("vpssgrp0.chn2", "chn_enable", "0") ||
	    !active_config_value_equals("vpssgrp0.chn2", "depth", "1") ||
	    !active_config_value_equals("vpssgrp0.chn2", "attach_en", "1") ||
	    !active_config_value_equals("vpssgrp0.chn2", "attach_pool", "1") ||
	    !active_config_value_equals("vpssgrp0.chn2", "dst_framerate", "-1") ||
		    !active_config_value_equals("vb_config", "vb_pool_cnt", "10") ||
	    !active_config_value_equals("vb_pool_1", "frame_width", "640") ||
	    !active_config_value_equals("vb_pool_1", "frame_height", "384") ||
	    !active_config_value_equals("vb_pool_1", "frame_fmt",
		    "PIXEL_FORMAT_UINT8_C3_PLANAR") ||
	    !active_config_value_equals("vb_pool_5", "frame_width", "1920") ||
	    !active_config_value_equals("vb_pool_5", "frame_height", "1080") ||
	    !active_config_value_equals("vb_pool_5", "frame_fmt", "PIXEL_FORMAT_NV12") ||
	    !active_config_value_equals("vb_pool_5", "blk_cnt", "4") ||
	    !active_config_value_equals("vb_pool_7", "frame_width", "1920") ||
	    !active_config_value_equals("vb_pool_7", "frame_height", "1080") ||
	    !active_config_value_equals("vb_pool_7", "frame_fmt", "PIXEL_FORMAT_NV12") ||
	    !active_config_value_equals("vb_pool_7", "blk_cnt", "4") ||
	    !active_config_value_equals("vb_pool_6", "blk_cnt", "4") ||
	    !active_config_value_equals("vb_pool_6", "bEnable", "0") ||
	    !active_config_value_equals("vb_pool_8", "frame_width", "1920") ||
	    !active_config_value_equals("vb_pool_8", "frame_height", "1080") ||
	    !active_config_value_equals("vb_pool_8", "blk_cnt", "4") ||
	    !active_config_value_equals("vb_pool_8", "bEnable", "0") ||
		    !active_config_value_equals("vpss_config", "vpss_grp", "8") ||
	    !active_config_value_equals("vpssgrp1", "chn_cnt", "1") ||
	    !active_config_value_equals("vpssgrp6", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp6", "src_dev_id", "0") ||
	    !active_config_value_equals("vpssgrp6", "dst_dev_id", "6") ||
	    !active_config_value_equals("vpssgrp6.chn0", "chn_enable", "0") ||
	    !active_config_value_equals("vpssgrp6.chn0", "src_framerate", "30") ||
	    !active_config_value_equals("vpssgrp6.chn0", "dst_framerate", "30") ||
	    !active_config_value_equals("vpssgrp6.chn0", "attach_pool", "8") ||
	    !active_config_value_equals("vencchn3", "src_dev_id", "0") ||
	    !active_config_value_equals("vencchn3", "src_chn_id", "0") ||
	    !active_config_value_equals("vencchn3", "vpss_grp", "0") ||
	    !active_config_value_equals("vencchn3", "vpss_chn", "0") ||
	    !active_config_value_equals("vencchn3", "src_framerate", "30") ||
	    !active_config_value_equals("vencchn3", "dst_framerate", "30") ||
	    !active_config_value_equals("vencchn3", "rc_mode",
		    "VENC_RC_MODE_MJPEGCBR") ||
	    !active_config_value_equals("vencchn3", "bit_rate", "50000") ||
	    !active_config_value_equals("vencchn3", "max_bitrate", "50000") ||
	    !active_config_value_equals("vpssgrp2", "max_w", "1920") ||
	    !active_config_value_equals("vpssgrp2", "max_h", "1080") ||
	    !active_config_value_equals("vpssgrp2", "src_chn_id", "0") ||
	    !active_config_value_equals("vpssgrp2", "src_framerate", "-1") ||
	    !active_config_value_equals("vpssgrp2", "dst_framerate", "-1") ||
	    !active_config_value_equals("vpssgrp2", "pixel_fmt", "PIXEL_FORMAT_NV12") ||
	    !active_config_value_equals("vpssgrp2.chn0", "chn_pixel_fmt",
		    "PIXEL_FORMAT_NV12") ||
	    !active_config_value_equals("vb_pool_2", "frame_fmt", "PIXEL_FORMAT_NV12") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0") ||
		    !active_config_value_equals("output_config", "rtsp_enable", "0") ||
		    !active_config_value_equals("output_config", "uvc_enable", "1") ||
		    !active_config_value_equals("display_config", "vo_cnt", "0") ||
		    !active_config_value_equals("display_config_0", "panel_type",
			    OVIS_DISPLAY_PANEL_TYPE) ||
		    !active_config_value_equals("display_config_0", "src_dev_id", "7") ||
		    !active_config_value_equals("vb_pool_9", "bEnable", "0") ||
		    !active_config_value_equals("vb_pool_9", "frame_width", "720") ||
		    !active_config_value_equals("vb_pool_9", "frame_height", "480") ||
		    !active_config_value_equals("vpssgrp7", "grp_enable", "0") ||
		    !active_config_value_equals("vpssgrp7", "dst_dev_id", "7") ||
		    !active_config_value_equals("vpssgrp7.chn0", "attach_pool", "9"))
		fail("ObjectTrack VPSS topology migration failed");
	document = read_document(revision_before, sizeof(revision_before));
	payload = make_frontend_payload(document, 60, 1);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 0)
		fail("frontend string model payload was rejected");
	free(payload);
	cJSON_Delete(document);

	stage_and_apply(30, 9000, 80, 0, 1, 0, 1, 0, 0, 1, 0);
	if (!active_config_value_equals("vpssgrp2", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp3", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp4", "grp_enable", "1") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp0.chn2", "chn_enable", "0") ||
	    !active_config_value_equals("vb_pool_4", "bEnable", "1"))
		fail("VPSS feature groups did not follow the AI switches");
	if (!active_config_value_equals("vpssgrp0.chn1", "chn_enable", "0") ||
	    !active_config_value_equals("vencchn2", "bEnable", "0") ||
	    !active_config_value_equals("osdc_config1", "bShow", "0"))
		fail("disabled sub stream left dependent channels enabled");
	document = read_document(revision_before, sizeof(revision_before));
	{
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
		cJSON *main_stream = cJSON_GetObjectItemCaseSensitive(video, "main");
		cJSON *sub_stream = cJSON_GetObjectItemCaseSensitive(video, "sub");
		if (cJSON_GetObjectItemCaseSensitive(main_stream, "bitrate_kbps")->valueint != 9000)
			fail("main bitrate did not round-trip");
		if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(sub_stream, "enabled")))
			fail("sub stream disable did not round-trip");
	}
	motion = cJSON_GetObjectItemCaseSensitive(
		cJSON_GetObjectItemCaseSensitive(
			cJSON_GetObjectItemCaseSensitive(document, "values"), "detection"),
		"motion");
	if (cJSON_GetObjectItemCaseSensitive(motion, "sensitivity")->valueint != 80)
		fail("motion sensitivity did not round-trip");
	payload = make_payload(document, 30, 20000, 80, 0, 1, 0, 1, 0);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 || strstr(validation, "OUT_OF_RANGE") == NULL)
		fail("out-of-range bitrate was not rejected");
	free(payload);
	payload = make_ai_conflict_payload(document);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 || strstr(validation, "AI_FEATURE_CONFLICT") == NULL)
		fail("conflicting TPU features were not rejected");
	free(payload);
	payload = make_object_frame_payload(document, 1920, 1080);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 0)
		fail("maximum object AI input frame size was rejected");
	free(payload);
	payload = make_object_frame_payload(document, 1922, 1080);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 || strstr(validation, "OUT_OF_RANGE") == NULL)
		fail("out-of-range object AI input frame size was not rejected");
	free(payload);
	cJSON_Delete(document);

	stage_and_apply(30, 8500, 60, 1, 1, 0, 0, 0, 1, 0, 1);
	document = read_document(revision_after, sizeof(revision_after));
	if (strcmp(revision_before, revision_after) != 0)
		fail("rollback did not restore the previous file");
	cJSON_Delete(document);

	stage_and_apply(60, 8800, 60, 1, 0, 1, 0, 0, 0, 1, 0);
	if (!active_config_value_equals("vpssgrp4", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp0.chn2", "chn_enable", "0"))
		fail("disabled motion detection left its VPSS group enabled");
	if (!active_config_value_equals("vpssgrp0.chn1", "chn_enable", "0") ||
	    !active_config_value_equals("vencchn2", "bEnable", "0") ||
	    !active_config_value_equals("osdc_config1", "bShow", "0"))
		fail("60 fps UVC mode did not release sub stream resources");
	document = read_document(revision_after, sizeof(revision_after));
	{
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
		cJSON *main_stream = cJSON_GetObjectItemCaseSensitive(video, "main");
		cJSON *sub_stream = cJSON_GetObjectItemCaseSensitive(video, "sub");
		if (cJSON_GetObjectItemCaseSensitive(main_stream, "fps")->valueint != 60)
			fail("60 fps did not round-trip");
		if (!cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(sub_stream, "enabled")))
			fail("sub stream enable did not round-trip");
	}
	if (!active_config_contains(OVIS_SC235HAI_60FPS_SNS_TYPE))
		fail("60 fps sensor type was not persisted");
	if (!active_config_value_equals("vpssgrp6.chn0", "src_framerate", "60") ||
	    !active_config_value_equals("vpssgrp6.chn0", "dst_framerate", "60") ||
	    !active_config_value_equals("vencchn3", "src_framerate", "60") ||
	    !active_config_value_equals("vencchn3", "dst_framerate", "60") ||
	    !active_config_value_equals("vpssgrp2", "src_framerate", "-1") ||
	    !active_config_value_equals("vpssgrp2", "dst_framerate", "-1"))
		fail("60 fps did not update the UVC or AI VPSS frame rates");
	cJSON_Delete(document);
	stage_and_apply(60, 8800, 60, 1, 0, 1, 0, 1, 0, 1, 0);
	if (!active_config_value_equals("ai_object_track_config",
			"object_track_enable", "1") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp0.chn2", "chn_enable", "1"))
		fail("ObjectTrack enabled the retired VPSS group");

	document = read_document(revision_after, sizeof(revision_after));
	payload = make_payload(document, 60, 8800, 60, 1, 0, 0, 0, 1);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 || strstr(validation, "OUTPUT_MODE_CONFLICT") == NULL)
		fail("disabled UVC and RTSP outputs were not rejected");
	free(payload);
	payload = make_payload(document, 60, 8800, 60, 1, 1, 1, 0, 1);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 || strstr(validation, "OUTPUT_MODE_CONFLICT") == NULL)
		fail("simultaneous UVC and RTSP outputs were not rejected");
	free(payload);
	cJSON_Delete(document);

	service_calls = 0;
	usb_reboot_calls = 0;
	stage_and_apply(60, 8800, 60, 1, 1, 0, 0, 1, 0, 1, 0);
	if (service_calls != 0 || usb_reboot_calls != 1 ||
	    !active_config_value_equals("output_config", "rtsp_enable", "1") ||
	    !active_config_value_equals("output_config", "uvc_enable", "0") ||
	    !active_config_value_equals("output_config", "sub_enable", "1") ||
	    !active_config_value_equals("vb_pool_6", "bEnable", "1") ||
	    !active_config_value_equals("vpssgrp1", "grp_enable", "1") ||
	    !active_config_value_equals("vencchn0", "bEnable", "1") ||
	    !active_config_value_equals("vencchn1", "bEnable", "1") ||
	    !active_config_value_equals("vencchn2", "bEnable", "1") ||
	    !active_config_value_equals("vb_pool_8", "bEnable", "0") ||
	    !active_config_value_equals("vpssgrp6", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp6.chn0", "chn_enable", "0") ||
	    !active_config_value_equals("vencchn3", "bEnable", "0") ||
	    !active_config_value_equals("rtsp_config", "rtsp_cnt", "2"))
		fail("RTSP output did not enable its processing resources");
	document = read_document(revision_after, sizeof(revision_after));
	{
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *outputs = cJSON_GetObjectItemCaseSensitive(values, "outputs");
		cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
		if (!cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(
				cJSON_GetObjectItemCaseSensitive(outputs, "rtsp"), "enabled")) ||
		    cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(
				cJSON_GetObjectItemCaseSensitive(outputs, "uvc"), "enabled")) ||
		    !cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(
				cJSON_GetObjectItemCaseSensitive(video, "sub"), "enabled")))
			fail("output switches or desired sub stream did not round-trip");
	}
	cJSON_Delete(document);
	fail_usb_reboot_calls = 1;
	service_calls = 0;
	usb_reboot_calls = 0;
	stage_and_apply(60, 8800, 60, 1, 0, 1, 0, 1, 0, 0, 1);
	if (service_calls != 0 || usb_reboot_calls != 1 ||
	    !active_config_value_equals("output_config", "uvc_enable", "0"))
		fail("failed UVC reboot scheduling changed the running service or config");

	document = read_document(revision_after, sizeof(revision_after));
	payload = make_display_payload(document, 1, "unsupported");
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 ||
	    strstr(validation, "UNSUPPORTED_DISPLAY_MODE") == NULL)
		fail("unsupported display mode was not rejected");
	free(payload);
	cJSON_Delete(document);
	service_calls = 0;
	usb_reboot_calls = 0;
	stage_display_and_apply(1);
	if (service_calls != 1 || usb_reboot_calls != 0 ||
	    !active_config_value_equals("display_config", "vo_cnt", "1") ||
	    !active_config_value_equals("vpssgrp7", "grp_enable", "1") ||
	    !active_config_value_equals("vb_pool_9", "bEnable", "1"))
		fail("display output did not enable its processing resources");
	document = read_document(revision_after, sizeof(revision_after));
	{
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *outputs = cJSON_GetObjectItemCaseSensitive(values, "outputs");
		cJSON *display = cJSON_GetObjectItemCaseSensitive(outputs, "display");
		if (!cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(display, "enabled")) ||
		    strcmp(cJSON_GetObjectItemCaseSensitive(display, "mode")->valuestring,
			    OVIS_DISPLAY_MODE) != 0)
			fail("display output did not round-trip");
	}
	cJSON_Delete(document);
	service_calls = 0;
	usb_reboot_calls = 0;
	stage_display_and_apply(0);
	if (service_calls != 1 || usb_reboot_calls != 0 ||
	    !active_config_value_equals("display_config", "vo_cnt", "0") ||
	    !active_config_value_equals("vpssgrp7", "grp_enable", "0") ||
	    !active_config_value_equals("vb_pool_9", "bEnable", "0"))
		fail("display output did not release its processing resources");

	fail_service_calls = 0;
	if (config_apply_defaults(message, sizeof(message), &rolled_back) != 0)
		fail("default configuration could not be applied");
	document = read_document(revision_after, sizeof(revision_after));
	{
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
		cJSON *main_stream = cJSON_GetObjectItemCaseSensitive(video, "main");
		if (cJSON_GetObjectItemCaseSensitive(main_stream, "bitrate_kbps")->valueint != 10000)
			fail("reset did not restore the default bitrate");
		if (cJSON_GetObjectItemCaseSensitive(main_stream, "fps")->valueint != 30)
			fail("reset did not restore the default frame rate");
	}
	if (!active_config_contains(OVIS_SC235HAI_30FPS_SNS_TYPE))
		fail("reset did not restore the 30 fps sensor type");
	if (!active_config_value_equals("vpssgrp6.chn0", "src_framerate", "30") ||
	    !active_config_value_equals("vpssgrp6.chn0", "dst_framerate", "30") ||
	    !active_config_value_equals("vencchn3", "src_framerate", "30") ||
	    !active_config_value_equals("vencchn3", "dst_framerate", "30") ||
	    !active_config_value_equals("vpssgrp2", "src_framerate", "-1") ||
	    !active_config_value_equals("vpssgrp2", "dst_framerate", "-1"))
		fail("reset did not restore the UVC or AI VPSS frame rates");
	if (!active_config_value_equals("vpssgrp2", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp3", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp4", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp0.chn2", "chn_enable", "0"))
		fail("reset did not disable unused VPSS feature groups");
	if (!active_config_value_equals("output_config", "rtsp_enable", "0") ||
	    !active_config_value_equals("output_config", "uvc_enable", "1") ||
	    !active_config_value_equals("display_config", "vo_cnt", "0") ||
	    !active_config_value_equals("vpssgrp7", "grp_enable", "0") ||
	    !active_config_value_equals("vb_pool_9", "bEnable", "0") ||
	    !active_config_value_equals("vpssgrp0.chn1", "chn_enable", "0") ||
	    !active_config_value_equals("vencchn2", "bEnable", "0") ||
	    !active_config_value_equals("osdc_config1", "bShow", "0"))
		fail("reset did not restore the default UVC-only output mode");
	cJSON_Delete(document);
	test_tracking_options();
	clean_test_directory();
	puts("config transaction test passed");
	return 0;
}
