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

static char *make_payload(cJSON *document, int bitrate, int sensitivity)
{
	cJSON *payload = cJSON_CreateObject();
	cJSON *revision = cJSON_GetObjectItemCaseSensitive(document, "revision");
	cJSON *values = cJSON_Duplicate(
		cJSON_GetObjectItemCaseSensitive(document, "values"), 1);
	cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
	cJSON *main_stream = cJSON_GetObjectItemCaseSensitive(video, "main");
	cJSON *detection = cJSON_GetObjectItemCaseSensitive(values, "detection");
	cJSON *motion = cJSON_GetObjectItemCaseSensitive(detection, "motion");
	char *json;

	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(main_stream, "bitrate_kbps"),
		bitrate);
	cJSON_SetNumberValue(cJSON_GetObjectItemCaseSensitive(motion, "sensitivity"),
		sensitivity);
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

static void stage_and_apply(int bitrate, int sensitivity, int fail_first_restart,
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
	char *payload = make_payload(document, bitrate, sensitivity);
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

	stage_and_apply(9000, 80, 0, 1, 0);
	document = read_document(revision_before, sizeof(revision_before));
	{
		cJSON *values = cJSON_GetObjectItemCaseSensitive(document, "values");
		cJSON *video = cJSON_GetObjectItemCaseSensitive(values, "video");
		cJSON *main_stream = cJSON_GetObjectItemCaseSensitive(video, "main");
		if (cJSON_GetObjectItemCaseSensitive(main_stream, "bitrate_kbps")->valueint != 9000)
			fail("main bitrate did not round-trip");
	}
	motion = cJSON_GetObjectItemCaseSensitive(
		cJSON_GetObjectItemCaseSensitive(
			cJSON_GetObjectItemCaseSensitive(document, "values"), "detection"),
		"motion");
	if (cJSON_GetObjectItemCaseSensitive(motion, "sensitivity")->valueint != 80)
		fail("motion sensitivity did not round-trip");
	payload = make_payload(document, 20000, 80);
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

	stage_and_apply(8500, 60, 1, 0, 1);
	document = read_document(revision_after, sizeof(revision_after));
	if (strcmp(revision_before, revision_after) != 0)
		fail("rollback did not restore the previous file");
	cJSON_Delete(document);

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
	}
	cJSON_Delete(document);
	clean_test_directory();
	puts("config transaction test passed");
	return 0;
}
