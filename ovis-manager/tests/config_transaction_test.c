#include "ovis_manager.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int fail_service_calls;

int service_run_action(enum service_action action, char *output, size_t size)
{
	(void)action;
	if (fail_service_calls > 0) {
		fail_service_calls--;
		snprintf(output, size, "simulated restart failure");
		return 1;
	}
	snprintf(output, size, "simulated restart success");
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
	unlink("/tmp/ovis-manager-config-test/pending.ini");
	unlink("/tmp/ovis-manager-config-test/pending.ini.tmp");
	unlink("/tmp/ovis-manager-config-test/backup.ini");
	unlink("/tmp/ovis-manager-config-test/backup.ini.tmp");
	rmdir("/tmp/ovis-manager-config-test");
}

static cJSON *read_document(char *revision, size_t revision_size)
{
	char json[4096];
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
	int sub_enabled, int motion_enabled, int object_tracking_enabled)
{
	cJSON *payload = cJSON_CreateObject();
	cJSON *revision = cJSON_GetObjectItemCaseSensitive(document, "revision");
	cJSON *values = cJSON_Duplicate(
		cJSON_GetObjectItemCaseSensitive(document, "values"), 1);
	cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
	cJSON *main_stream = cJSON_GetObjectItemCaseSensitive(video, "main");
	cJSON *sub_stream = cJSON_GetObjectItemCaseSensitive(video, "sub");
	cJSON *detection = cJSON_GetObjectItemCaseSensitive(values, "detection");
	cJSON *motion = cJSON_GetObjectItemCaseSensitive(detection, "motion");
	cJSON *object_tracking = cJSON_GetObjectItemCaseSensitive(
		detection, "object_tracking");
	char *json;

	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(main_stream, "fps"), fps);
	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(main_stream, "bitrate_kbps"),
		bitrate);
	cJSON_ReplaceItemInObjectCaseSensitive(sub_stream, "enabled",
		cJSON_CreateBool(sub_enabled));
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

static char *make_ai_conflict_payload(cJSON *document)
{
	cJSON *payload = cJSON_CreateObject();
	cJSON *revision = cJSON_GetObjectItemCaseSensitive(document, "revision");
	cJSON *values = cJSON_Duplicate(
		cJSON_GetObjectItemCaseSensitive(document, "values"), 1);
	cJSON *detection = cJSON_GetObjectItemCaseSensitive(values, "detection");
	cJSON *person = cJSON_GetObjectItemCaseSensitive(detection, "person");
	cJSON *face = cJSON_GetObjectItemCaseSensitive(detection, "face");
	char *json;

	cJSON_ReplaceItemInObjectCaseSensitive(person, "enabled", cJSON_CreateBool(1));
	cJSON_ReplaceItemInObjectCaseSensitive(face, "enabled", cJSON_CreateBool(1));
	cJSON_AddItemToObject(payload, "revision", cJSON_Duplicate(revision, 1));
	cJSON_AddItemToObject(payload, "values", values);
	json = cJSON_PrintUnformatted(payload);
	cJSON_Delete(payload);
	return json;
}

static void stage_and_apply(int fps, int bitrate, int sensitivity, int sub_enabled,
	int motion_enabled, int object_tracking_enabled, int fail_first_restart,
	int expect_success, int expect_rollback)
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
		motion_enabled, object_tracking_enabled);
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

int main(void)
{
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
	if (!active_config_value_equals("ai_object_track_config", "sot_vpss_grp", "0") ||
	    !active_config_value_equals("ai_object_track_config",
		    "sot_refine_selected_det", "0") ||
	    !active_config_value_equals("vpssgrp0.chn0", "depth", "0") ||
	    !active_config_value_equals("vpssgrp0.chn2", "dst_framerate", "-1") ||
	    !active_config_value_equals("vb_config", "vb_pool_cnt", "7") ||
	    !active_config_value_equals("vb_pool_5", "blk_cnt", "8") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0"))
		fail("ObjectTrack VPSS topology migration failed");

	stage_and_apply(30, 9000, 80, 0, 1, 0, 0, 1, 0);
	if (!active_config_value_equals("vpssgrp2", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp3", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp4", "grp_enable", "1") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0"))
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
	payload = make_payload(document, 30, 20000, 80, 0, 1, 0);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 || strstr(validation, "OUT_OF_RANGE") == NULL)
		fail("out-of-range bitrate was not rejected");
	free(payload);
	payload = make_ai_conflict_payload(document);
	if (config_validate_json(payload, validation, sizeof(validation), error,
			sizeof(error)) != 1 || strstr(validation, "AI_FEATURE_CONFLICT") == NULL)
		fail("conflicting TPU features were not rejected");
	free(payload);
	cJSON_Delete(document);

	stage_and_apply(30, 8500, 60, 1, 0, 0, 1, 0, 1);
	document = read_document(revision_after, sizeof(revision_after));
	if (strcmp(revision_before, revision_after) != 0)
		fail("rollback did not restore the previous file");
	cJSON_Delete(document);

	stage_and_apply(60, 8800, 60, 1, 0, 0, 0, 1, 0);
	if (!active_config_value_equals("vpssgrp4", "grp_enable", "0"))
		fail("disabled motion detection left its VPSS group enabled");
	if (!active_config_value_equals("vpssgrp0.chn1", "chn_enable", "1") ||
	    !active_config_value_equals("vencchn2", "bEnable", "1") ||
	    !active_config_value_equals("osdc_config1", "bShow", "1"))
		fail("enabled sub stream did not restore dependent channels");
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
	cJSON_Delete(document);
	stage_and_apply(60, 8800, 60, 1, 0, 1, 0, 1, 0);
	if (!active_config_value_equals("ai_object_track_config",
			"object_track_enable", "1") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0"))
		fail("ObjectTrack enabled the retired VPSS group");

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
	if (!active_config_value_equals("vpssgrp2", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp3", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp4", "grp_enable", "0") ||
	    !active_config_value_equals("vpssgrp5", "grp_enable", "0"))
		fail("reset did not disable unused VPSS feature groups");
	if (!active_config_value_equals("vpssgrp0.chn1", "chn_enable", "1") ||
	    !active_config_value_equals("vencchn2", "bEnable", "1") ||
	    !active_config_value_equals("osdc_config1", "bShow", "1"))
		fail("reset did not restore sub stream dependent channels");
	cJSON_Delete(document);
	clean_test_directory();
	puts("config transaction test passed");
	return 0;
}
