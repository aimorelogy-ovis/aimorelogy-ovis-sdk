#include "ovis_manager.h"
#include "cJSON.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

void audit_log(const char *operation, const char *result)
{
	(void)operation;
	(void)result;
}

int task_is_busy(void)
{
	return 0;
}

unsigned long config_task_submit_apply(const char *revision)
{
	(void)revision;
	return 1;
}

int config_stage_detection_model(const char *model_id, const char *model_path,
	const char *model_config_path, double threshold, int processing_width,
	int processing_height, int enabled,
	char revision[17], char *error, size_t error_size)
{
	(void)model_id;
	(void)model_path;
	(void)model_config_path;
	(void)threshold;
	(void)processing_width;
	(void)processing_height;
	(void)enabled;
	(void)error;
	(void)error_size;
	snprintf(revision, 17, "0000000000000001");
	return 0;
}

int config_rebase_backup_away_from_model(const char *id,
	char *error, size_t error_size)
{
	(void)id;
	(void)error;
	(void)error_size;
	return 0;
}

static int expect(int condition, const char *message)
{
	if (condition)
		return 0;
	fprintf(stderr, "FAIL: %s\n", message);
	return -1;
}

int main(void)
{
	static const char create_body[] =
		"{\"importerId\":\"detection.yolov5\",\"schemaVersion\":1,"
		"\"name\":\"helmet\",\"fileSize\":64,\"metadata\":{"
		"\"labels\":[\"person\",\"helmet\"],\"anchors\":["
		"[[10,13],[16,30],[33,23]],[[30,61],[62,45],[59,119]],"
		"[[116,90],[156,198],[373,326]]]}}";
	unsigned char model[64] = { 0xee, 0xaa, 0x55, 0xff };
	char json[65536];
	char error[256];
	char id[17];
	char path[512];
	char factory[2048];
	cJSON *root;
	cJSON *item;
	FILE *file;
	size_t length;

	if (model_importers_json(json, sizeof(json)) != 0 ||
	    expect(strstr(json, "\"strategy\":\"single-request\"") != NULL,
		"importer upload strategy") != 0 ||
	    expect(strstr(json, "\"metadataSchema\"") != NULL,
		"importer metadata schema") != 0)
		return 1;

	if (model_import_create(create_body, json, sizeof(json), error, sizeof(error)) != 0)
		return expect(0, error);
	root = cJSON_Parse(json);
	item = cJSON_GetObjectItemCaseSensitive(root, "id");
	if (expect(cJSON_IsString(item) && strlen(item->valuestring) == 16,
			"create response id") != 0) {
		cJSON_Delete(root);
		return 1;
	}
	snprintf(id, sizeof(id), "%s", item->valuestring);
	cJSON_Delete(root);

	if (model_import_write_content(id, -1, model, sizeof(model), sizeof(model),
			json, sizeof(json), error, sizeof(error)) != 0)
		return expect(0, error);
	if (model_import_commit(id, json, sizeof(json), error, sizeof(error)) != 0)
		return expect(0, error);

	snprintf(path, sizeof(path), "%s/%s/factory.json", OVIS_MODEL_STORE_DIR, id);
	file = fopen(path, "r");
	if (expect(file != NULL, "factory config exists") != 0)
		return 1;
	length = fread(factory, 1, sizeof(factory) - 1, file);
	factory[length] = '\0';
	fclose(file);
	if (expect(strstr(factory, "\"YOLOV5\"") != NULL,
			"factory model type") != 0 ||
	    expect(strstr(factory, "\"anchors\":\"10,13,16,30") != NULL,
			"factory anchors") != 0)
		return 1;
	if (model_get_json(id, json, sizeof(json), error, sizeof(error)) != 0 ||
	    expect(strstr(json, "\"deployable\":true") != NULL,
		"model detail deployable") != 0)
		return 1;
	if (model_deployment_get_json(id, json, sizeof(json), error, sizeof(error)) != 0 ||
	    expect(strstr(json, "\"threshold\":0.5") != NULL,
		"default deployment threshold") != 0 ||
	    expect(strstr(json, "\"processingSize\":{\"width\":448,\"height\":256}") != NULL,
		"default AI input frame size") != 0 ||
	    expect(strstr(json, "\"maxWidth\":1920") != NULL,
		"AI input frame maximum width") != 0 ||
	    expect(strstr(json, "\"maxHeight\":1080") != NULL,
		"AI input frame maximum height") != 0)
		return 1;
	if (model_deployment_update(id, "{\"threshold\":0.65,"
		"\"processingSize\":{\"width\":320,\"height\":192}}", json,
			sizeof(json), error, sizeof(error)) != 0 ||
	    expect(strstr(json, "\"threshold\":0.65") != NULL,
		"updated deployment threshold") != 0 ||
	    expect(strstr(json, "\"width\":320") != NULL,
		"updated deployment processing width") != 0)
		return 1;
	if (model_deployment_update(id, "{\"threshold\":0.6,"
		"\"processingSize\":{\"width\":1920,\"height\":1080}}", json,
			sizeof(json), error, sizeof(error)) != 0 ||
	    expect(strstr(json, "\"processingSize\":{\"width\":1920,\"height\":1080}") != NULL,
		"maximum AI input frame size") != 0)
		return 1;
	if (model_deployment_update(id, "{\"threshold\":0.6,"
		"\"processingSize\":{\"width\":1922,\"height\":1080}}", json,
			sizeof(json), error, sizeof(error)) == 0 ||
	    expect(strstr(error, "AI 输入帧尺寸") != NULL,
		"invalid AI input frame size error") != 0)
		return 1;
	if (model_list_json(json, sizeof(json)) != 0 ||
	    expect(strstr(json, id) != NULL, "model list contains model") != 0)
		return 1;
	if (model_delete(id, error, sizeof(error)) != 0)
		return expect(0, error);
	rmdir(OVIS_MODEL_IMPORT_DIR);
	rmdir(OVIS_MODEL_STORE_DIR);
	rmdir(OVIS_MODEL_DIR);
	puts("model store tests passed");
	return 0;
}
