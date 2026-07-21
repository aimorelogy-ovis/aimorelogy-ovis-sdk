#include "ovis_manager.h"
#include "cJSON.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>

#define OVIS_MODEL_MAX_METADATA_SIZE (192 * 1024)

enum model_metadata_kind {
	MODEL_METADATA_DETECTION,
	MODEL_METADATA_DETECTION_ANCHORS,
	MODEL_METADATA_CLASSIFICATION,
	MODEL_METADATA_POSE,
	MODEL_METADATA_SEGMENTATION,
	MODEL_METADATA_FEATURE,
	MODEL_METADATA_SOUND,
};

struct model_importer {
	const char *id;
	const char *name;
	const char *task;
	const char *model_type;
	enum model_metadata_kind metadata_kind;
	int deployable;
};

static const struct model_importer model_importers[] = {
	{ "detection.yolov5", "YOLOv5 目标检测", "object_detection", "YOLOV5",
		MODEL_METADATA_DETECTION_ANCHORS, 1 },
	{ "detection.yolov6", "YOLOv6 目标检测", "object_detection", "YOLOV6",
		MODEL_METADATA_DETECTION, 1 },
	{ "detection.yolov7", "YOLOv7 目标检测", "object_detection", "YOLOV7",
		MODEL_METADATA_DETECTION_ANCHORS, 1 },
	{ "detection.yolov8", "YOLOv8 目标检测", "object_detection", "YOLOV8",
		MODEL_METADATA_DETECTION, 1 },
	{ "detection.yolov10", "YOLOv10 目标检测", "object_detection", "YOLOV10",
		MODEL_METADATA_DETECTION, 1 },
	{ "detection.yolo26", "YOLO26 目标检测", "object_detection", "YOLO26",
		MODEL_METADATA_DETECTION, 1 },
	{ "detection.ppyoloe", "PP-YOLOE 目标检测", "object_detection", "PPYOLOE",
		MODEL_METADATA_DETECTION, 1 },
	{ "detection.yolox", "YOLOX 目标检测", "object_detection", "YOLOX",
		MODEL_METADATA_DETECTION, 1 },
	{ "classification.image", "图像分类", "image_classification", "CLS_IMG",
		MODEL_METADATA_CLASSIFICATION, 0 },
	{ "pose.yolov8", "YOLOv8 关键点检测", "keypoint_detection",
		"KEYPOINT_YOLOV8POSE", MODEL_METADATA_POSE, 0 },
	{ "segmentation.yolov8", "YOLOv8 实例分割", "instance_segmentation",
		"YOLOV8_SEG", MODEL_METADATA_SEGMENTATION, 0 },
	{ "feature.image", "图像特征提取", "image_feature", "FEATURE_IMG",
		MODEL_METADATA_FEATURE, 0 },
	{ "classification.sound_command", "语音命令分类", "sound_classification",
		"CLS_SOUND_COMMAND", MODEL_METADATA_SOUND, 0 },
};

static int ensure_dir(const char *path)
{
	return mkdir(path, 0755) == 0 || errno == EEXIST ? 0 : -1;
}

static int ensure_model_dirs(void)
{
	return ensure_dir(OVIS_MODEL_DIR) == 0 &&
		ensure_dir(OVIS_MODEL_IMPORT_DIR) == 0 &&
		ensure_dir(OVIS_MODEL_STORE_DIR) == 0 ? 0 : -1;
}

static const struct model_importer *find_importer(const char *id)
{
	size_t index;

	for (index = 0; index < sizeof(model_importers) / sizeof(model_importers[0]); index++) {
		if (strcmp(model_importers[index].id, id) == 0)
			return &model_importers[index];
	}
	return NULL;
}

static int unsupported_label_count(const struct model_importer *importer)
{
	if (strcmp(importer->id, "detection.yolov6") == 0 ||
	    strcmp(importer->id, "detection.yolo26") == 0 ||
	    strcmp(importer->id, "detection.ppyoloe") == 0)
		return 4;
	if (strcmp(importer->id, "detection.yolov8") == 0 ||
	    strcmp(importer->id, "detection.yolov10") == 0)
		return 64;
	return 0;
}

static int json_print(cJSON *root, char *json, size_t size)
{
	int result = cJSON_PrintPreallocated(root, json, (int)size, 0) ? 0 : -1;
	cJSON_Delete(root);
	return result;
}

static int id_is_valid(const char *id)
{
	size_t index;

	if (id == NULL || strlen(id) != 16)
		return 0;
	for (index = 0; index < 16; index++) {
		if (!isdigit((unsigned char)id[index]) &&
		    (id[index] < 'a' || id[index] > 'f'))
			return 0;
	}
	return 1;
}

static int text_is_valid(const char *text, size_t maximum)
{
	size_t index;
	size_t length;

	if (text == NULL || text[0] == '\0')
		return 0;
	length = strlen(text);
	if (length > maximum)
		return 0;
	for (index = 0; index < length; index++) {
		if ((unsigned char)text[index] < 0x20)
			return 0;
	}
	return 1;
}

static int object_has_only(cJSON *object, const char *const *names,
	size_t name_count)
{
	cJSON *item;

	cJSON_ArrayForEach(item, object) {
		size_t index;
		int found = 0;

		for (index = 0; index < name_count; index++) {
			if (strcmp(item->string, names[index]) == 0) {
				found = 1;
				break;
			}
		}
		if (!found)
			return 0;
	}
	return 1;
}

static int string_array_is_valid(cJSON *array, int minimum, int maximum)
{
	cJSON *item;
	int count;
	int index;

	if (!cJSON_IsArray(array))
		return 0;
	count = cJSON_GetArraySize(array);
	if (count < minimum || count > maximum)
		return 0;
	for (index = 0; index < count; index++) {
		int previous;

		item = cJSON_GetArrayItem(array, index);
		if (!cJSON_IsString(item) || !text_is_valid(item->valuestring, 96))
			return 0;
		for (previous = 0; previous < index; previous++) {
			cJSON *other = cJSON_GetArrayItem(array, previous);
			if (strcmp(item->valuestring, other->valuestring) == 0)
				return 0;
		}
	}
	return 1;
}

static int rgb_order_is_valid(cJSON *item)
{
	return cJSON_IsString(item) && item->valuestring != NULL &&
		(strcasecmp(item->valuestring, "rgb") == 0 ||
		 strcasecmp(item->valuestring, "bgr") == 0);
}

static int vector3_is_valid(cJSON *array, int reject_zero)
{
	int index;

	if (!cJSON_IsArray(array) || cJSON_GetArraySize(array) != 3)
		return 0;
	for (index = 0; index < 3; index++) {
		cJSON *item = cJSON_GetArrayItem(array, index);
		if (!cJSON_IsNumber(item) || !isfinite(item->valuedouble) ||
		    (reject_zero && item->valuedouble == 0.0))
			return 0;
	}
	return 1;
}

static int anchors_are_valid(cJSON *anchors)
{
	int head;

	if (!cJSON_IsArray(anchors) || cJSON_GetArraySize(anchors) != 3)
		return 0;
	for (head = 0; head < 3; head++) {
		cJSON *group = cJSON_GetArrayItem(anchors, head);
		int anchor;

		if (!cJSON_IsArray(group) || cJSON_GetArraySize(group) != 3)
			return 0;
		for (anchor = 0; anchor < 3; anchor++) {
			cJSON *pair = cJSON_GetArrayItem(group, anchor);
			int value;

			if (!cJSON_IsArray(pair) || cJSON_GetArraySize(pair) != 2)
				return 0;
			for (value = 0; value < 2; value++) {
				cJSON *number = cJSON_GetArrayItem(pair, value);
				if (!cJSON_IsNumber(number) || number->valuedouble != number->valueint ||
				    number->valueint <= 0 || number->valueint > 65535)
					return 0;
			}
		}
	}
	return 1;
}

static int skeleton_is_valid(cJSON *skeleton, int keypoint_count)
{
	cJSON *edge;

	if (skeleton == NULL)
		return 1;
	if (!cJSON_IsArray(skeleton) || cJSON_GetArraySize(skeleton) > 512)
		return 0;
	cJSON_ArrayForEach(edge, skeleton) {
		cJSON *from;
		cJSON *to;

		if (!cJSON_IsArray(edge) || cJSON_GetArraySize(edge) != 2)
			return 0;
		from = cJSON_GetArrayItem(edge, 0);
		to = cJSON_GetArrayItem(edge, 1);
		if (!cJSON_IsNumber(from) || !cJSON_IsNumber(to) ||
		    from->valuedouble != from->valueint || to->valuedouble != to->valueint ||
		    from->valueint < 0 || from->valueint >= keypoint_count ||
		    to->valueint < 0 || to->valueint >= keypoint_count ||
		    from->valueint == to->valueint)
			return 0;
	}
	return 1;
}

static int colors_are_valid(cJSON *colors, int label_count)
{
	cJSON *color;

	if (colors == NULL)
		return 1;
	if (!cJSON_IsArray(colors) || cJSON_GetArraySize(colors) != label_count)
		return 0;
	cJSON_ArrayForEach(color, colors) {
		const char *value;
		int index;

		if (!cJSON_IsString(color) || color->valuestring == NULL ||
		    strlen(color->valuestring) != 7 || color->valuestring[0] != '#')
			return 0;
		value = color->valuestring;
		for (index = 1; index < 7; index++) {
			if (!isxdigit((unsigned char)value[index]))
				return 0;
		}
	}
	return 1;
}

static int validate_metadata(const struct model_importer *importer,
	cJSON *metadata, char *error, size_t error_size)
{
	static const char *const detection_fields[] = { "labels" };
	static const char *const anchor_fields[] = { "labels", "anchors" };
	static const char *const classification_fields[] = {
		"labels", "rgbOrder", "mean", "std"
	};
	static const char *const pose_fields[] = {
		"labels", "keypoints", "rgbOrder", "skeleton"
	};
	static const char *const segmentation_fields[] = {
		"labels", "rgbOrder", "colors"
	};
	static const char *const feature_fields[] = { "rgbOrder", "mean", "std" };
	static const char *const sound_fields[] = {
		"labels", "sampleRate", "channels", "hopLength", "preprocessProfile"
	};
	cJSON *labels = cJSON_GetObjectItemCaseSensitive(metadata, "labels");
	cJSON *rgb_order = cJSON_GetObjectItemCaseSensitive(metadata, "rgbOrder");
	cJSON *mean = cJSON_GetObjectItemCaseSensitive(metadata, "mean");
	cJSON *std = cJSON_GetObjectItemCaseSensitive(metadata, "std");

	if (!cJSON_IsObject(metadata))
		goto invalid;
	switch (importer->metadata_kind) {
	case MODEL_METADATA_DETECTION:
		if (!object_has_only(metadata, detection_fields, 1) ||
		    !string_array_is_valid(labels, 1, 256))
			goto invalid;
		if (unsupported_label_count(importer) == cJSON_GetArraySize(labels)) {
			snprintf(error, error_size, "%s 不支持 %d 个类别",
				importer->name, cJSON_GetArraySize(labels));
			return -1;
		}
		break;
	case MODEL_METADATA_DETECTION_ANCHORS:
		if (!object_has_only(metadata, anchor_fields, 2) ||
		    !string_array_is_valid(labels, 1, 256) ||
		    !anchors_are_valid(cJSON_GetObjectItemCaseSensitive(metadata, "anchors")))
			goto invalid;
		break;
	case MODEL_METADATA_CLASSIFICATION:
		if (!object_has_only(metadata, classification_fields, 4) ||
		    !string_array_is_valid(labels, 2, 1024) || !rgb_order_is_valid(rgb_order) ||
		    !vector3_is_valid(mean, 0) || !vector3_is_valid(std, 1))
			goto invalid;
		break;
	case MODEL_METADATA_POSE: {
		cJSON *keypoints = cJSON_GetObjectItemCaseSensitive(metadata, "keypoints");
		if (!object_has_only(metadata, pose_fields, 4) ||
		    !string_array_is_valid(labels, 1, 256) ||
		    !string_array_is_valid(keypoints, 1, 256) || !rgb_order_is_valid(rgb_order) ||
		    !skeleton_is_valid(cJSON_GetObjectItemCaseSensitive(metadata, "skeleton"),
			cJSON_GetArraySize(keypoints)))
			goto invalid;
		break;
	}
	case MODEL_METADATA_SEGMENTATION:
		if (!object_has_only(metadata, segmentation_fields, 3) ||
		    !string_array_is_valid(labels, 1, 256) || !rgb_order_is_valid(rgb_order) ||
		    !colors_are_valid(cJSON_GetObjectItemCaseSensitive(metadata, "colors"),
			cJSON_GetArraySize(labels)))
			goto invalid;
		break;
	case MODEL_METADATA_FEATURE:
		if (!object_has_only(metadata, feature_fields, 3) ||
		    !rgb_order_is_valid(rgb_order) || !vector3_is_valid(mean, 0) ||
		    !vector3_is_valid(std, 1))
			goto invalid;
		break;
	case MODEL_METADATA_SOUND: {
		cJSON *sample_rate = cJSON_GetObjectItemCaseSensitive(metadata, "sampleRate");
		cJSON *channels = cJSON_GetObjectItemCaseSensitive(metadata, "channels");
		cJSON *hop_length = cJSON_GetObjectItemCaseSensitive(metadata, "hopLength");
		cJSON *profile = cJSON_GetObjectItemCaseSensitive(metadata, "preprocessProfile");
		if (!object_has_only(metadata, sound_fields, 5) ||
		    !string_array_is_valid(labels, 2, 256) || !cJSON_IsNumber(sample_rate) ||
		    sample_rate->valuedouble != sample_rate->valueint ||
		    (sample_rate->valueint != 8000 && sample_rate->valueint != 16000) ||
		    !cJSON_IsNumber(channels) || channels->valuedouble != channels->valueint ||
		    channels->valueint != 1 || !cJSON_IsNumber(hop_length) ||
		    hop_length->valuedouble != hop_length->valueint || hop_length->valueint < 1 ||
		    hop_length->valueint > 4096 || !cJSON_IsString(profile) ||
		    (strcmp(profile->valuestring, "common") != 0 &&
		     strcmp(profile->valuestring, "fixed") != 0))
			goto invalid;
		break;
	}
	}
	return 0;

invalid:
	snprintf(error, error_size, "%s 的模型参数无效", importer->name);
	return -1;
}

static int generate_id(char id[17])
{
	static unsigned int sequence;
	struct timespec timestamp;
	unsigned int attempt;

	if (clock_gettime(CLOCK_REALTIME, &timestamp) != 0)
		return -1;
	for (attempt = 0; attempt < 100; attempt++) {
		char path[512];
		char model_path[512];
		uint64_t value = ((uint64_t)timestamp.tv_sec << 24) ^
			(uint64_t)timestamp.tv_nsec ^ (uint64_t)getpid() ^ sequence++;

		snprintf(id, 17, "%016llx", (unsigned long long)value);
		snprintf(path, sizeof(path), "%s/%s.json", OVIS_MODEL_IMPORT_DIR, id);
		snprintf(model_path, sizeof(model_path), "%s/%s", OVIS_MODEL_STORE_DIR, id);
		if (access(path, F_OK) != 0 && access(model_path, F_OK) != 0)
			return 0;
	}
	return -1;
}

static int write_json_file(const char *path, cJSON *root)
{
	char temporary[512];
	char *text;
	FILE *file;
	int result = -1;

	text = cJSON_PrintUnformatted(root);
	if (text == NULL)
		return -1;
	snprintf(temporary, sizeof(temporary), "%s.tmp", path);
	file = fopen(temporary, "w");
	if (file == NULL || fputs(text, file) < 0 || fputc('\n', file) == EOF ||
	    fflush(file) != 0 || fsync(fileno(file)) != 0)
		goto done;
	if (fclose(file) != 0) {
		file = NULL;
		goto done;
	}
	file = NULL;
	if (rename(temporary, path) == 0)
		result = 0;
done:
	if (file != NULL)
		fclose(file);
	if (result != 0)
		unlink(temporary);
	free(text);
	return result;
}

static cJSON *read_json_file(const char *path)
{
	struct stat status;
	char *buffer;
	FILE *file;
	cJSON *root = NULL;

	if (stat(path, &status) != 0 || status.st_size <= 0 ||
	    status.st_size > OVIS_MODEL_MAX_METADATA_SIZE)
		return NULL;
	buffer = malloc((size_t)status.st_size + 1);
	if (buffer == NULL)
		return NULL;
	file = fopen(path, "r");
	if (file != NULL && fread(buffer, 1, (size_t)status.st_size, file) ==
	    (size_t)status.st_size) {
		buffer[status.st_size] = '\0';
		root = cJSON_Parse(buffer);
	}
	if (file != NULL)
		fclose(file);
	free(buffer);
	return root;
}

static void add_default_anchors(cJSON *importer, const int values[18])
{
	cJSON *defaults = cJSON_AddObjectToObject(importer, "defaults");
	cJSON *anchors = cJSON_AddArrayToObject(defaults, "anchors");
	int head;
	int offset = 0;

	for (head = 0; head < 3; head++) {
		cJSON *group = cJSON_CreateArray();
		int index;
		cJSON_AddItemToArray(anchors, group);
		for (index = 0; index < 3; index++) {
			cJSON *pair = cJSON_CreateArray();
			cJSON_AddItemToArray(pair, cJSON_CreateNumber(values[offset++]));
			cJSON_AddItemToArray(pair, cJSON_CreateNumber(values[offset++]));
			cJSON_AddItemToArray(group, pair);
		}
	}
}

static void add_default_sound_preprocess(cJSON *importer)
{
	cJSON *defaults = cJSON_AddObjectToObject(importer, "defaults");

	if (defaults == NULL)
		return;
	cJSON_AddNumberToObject(defaults, "sampleRate", 16000);
	cJSON_AddNumberToObject(defaults, "channels", 1);
	cJSON_AddNumberToObject(defaults, "hopLength", 128);
	cJSON_AddStringToObject(defaults, "preprocessProfile", "fixed");
}

static cJSON *add_schema_property(cJSON *properties, const char *name,
	const char *type)
{
	cJSON *property = cJSON_AddObjectToObject(properties, name);
	if (property != NULL)
		cJSON_AddStringToObject(property, "type", type);
	return property;
}

static void add_schema_required(cJSON *schema, const char *const *names,
	size_t count)
{
	cJSON *required = cJSON_AddArrayToObject(schema, "required");
	size_t index;

	for (index = 0; required != NULL && index < count; index++)
		cJSON_AddItemToArray(required, cJSON_CreateString(names[index]));
}

static cJSON *add_string_list_schema(cJSON *properties, const char *name,
	int minimum, int maximum)
{
	cJSON *array = add_schema_property(properties, name, "array");
	cJSON *items;

	if (array == NULL)
		return NULL;
	cJSON_AddNumberToObject(array, "minItems", minimum);
	cJSON_AddNumberToObject(array, "maxItems", maximum);
	cJSON_AddBoolToObject(array, "uniqueItems", 1);
	items = cJSON_AddObjectToObject(array, "items");
	if (items != NULL) {
		cJSON_AddStringToObject(items, "type", "string");
		cJSON_AddNumberToObject(items, "minLength", 1);
		cJSON_AddNumberToObject(items, "maxLength", 96);
	}
	return array;
}

static cJSON *add_fixed_vector_schema(cJSON *properties, const char *name,
	int nonzero)
{
	cJSON *array = add_schema_property(properties, name, "array");
	cJSON *items;

	if (array == NULL)
		return NULL;
	cJSON_AddNumberToObject(array, "minItems", 3);
	cJSON_AddNumberToObject(array, "maxItems", 3);
	if (nonzero)
		cJSON_AddBoolToObject(array, "x-nonZeroItems", 1);
	items = cJSON_AddObjectToObject(array, "items");
	if (items != NULL)
		cJSON_AddStringToObject(items, "type", "number");
	return array;
}

static cJSON *add_rgb_order_schema(cJSON *properties)
{
	cJSON *property = add_schema_property(properties, "rgbOrder", "string");
	cJSON *values;

	if (property == NULL)
		return NULL;
	values = cJSON_AddArrayToObject(property, "enum");
	if (values != NULL) {
		cJSON_AddItemToArray(values, cJSON_CreateString("RGB"));
		cJSON_AddItemToArray(values, cJSON_CreateString("BGR"));
	}
	return property;
}

static cJSON *add_anchors_schema(cJSON *properties)
{
	cJSON *outer = add_schema_property(properties, "anchors", "array");
	cJSON *group;
	cJSON *pair;
	cJSON *number;

	if (outer == NULL)
		return NULL;
	cJSON_AddNumberToObject(outer, "minItems", 3);
	cJSON_AddNumberToObject(outer, "maxItems", 3);
	group = cJSON_AddObjectToObject(outer, "items");
	if (group == NULL)
		return outer;
	cJSON_AddStringToObject(group, "type", "array");
	cJSON_AddNumberToObject(group, "minItems", 3);
	cJSON_AddNumberToObject(group, "maxItems", 3);
	pair = cJSON_AddObjectToObject(group, "items");
	if (pair == NULL)
		return outer;
	cJSON_AddStringToObject(pair, "type", "array");
	cJSON_AddNumberToObject(pair, "minItems", 2);
	cJSON_AddNumberToObject(pair, "maxItems", 2);
	number = cJSON_AddObjectToObject(pair, "items");
	if (number != NULL) {
		cJSON_AddStringToObject(number, "type", "integer");
		cJSON_AddNumberToObject(number, "minimum", 1);
		cJSON_AddNumberToObject(number, "maximum", 65535);
	}
	return outer;
}

static int add_metadata_schema(cJSON *item, const struct model_importer *importer)
{
	static const char *const detection_required[] = { "labels" };
	static const char *const anchors_required[] = { "labels", "anchors" };
	static const char *const classification_required[] = {
		"labels", "rgbOrder", "mean", "std"
	};
	static const char *const pose_required[] = {
		"labels", "keypoints", "rgbOrder"
	};
	static const char *const segmentation_required[] = { "labels", "rgbOrder" };
	static const char *const feature_required[] = { "rgbOrder", "mean", "std" };
	static const char *const sound_required[] = {
		"labels", "sampleRate", "channels", "hopLength", "preprocessProfile"
	};
	cJSON *schema = cJSON_AddObjectToObject(item, "metadataSchema");
	cJSON *properties;

	if (schema == NULL)
		return -1;
	cJSON_AddStringToObject(schema, "type", "object");
	cJSON_AddBoolToObject(schema, "additionalProperties", 0);
	properties = cJSON_AddObjectToObject(schema, "properties");
	if (properties == NULL)
		return -1;
	switch (importer->metadata_kind) {
	case MODEL_METADATA_DETECTION:
		add_schema_required(schema, detection_required, 1);
		add_string_list_schema(properties, "labels", 1, 256);
		break;
	case MODEL_METADATA_DETECTION_ANCHORS:
		add_schema_required(schema, anchors_required, 2);
		add_string_list_schema(properties, "labels", 1, 256);
		add_anchors_schema(properties);
		break;
	case MODEL_METADATA_CLASSIFICATION:
		add_schema_required(schema, classification_required, 4);
		add_string_list_schema(properties, "labels", 2, 1024);
		add_rgb_order_schema(properties);
		add_fixed_vector_schema(properties, "mean", 0);
		add_fixed_vector_schema(properties, "std", 1);
		break;
	case MODEL_METADATA_POSE: {
		cJSON *skeleton;
		cJSON *edge;
		cJSON *index;
		add_schema_required(schema, pose_required, 3);
		add_string_list_schema(properties, "labels", 1, 256);
		add_string_list_schema(properties, "keypoints", 1, 256);
		add_rgb_order_schema(properties);
		skeleton = add_schema_property(properties, "skeleton", "array");
		if (skeleton != NULL) {
			cJSON_AddNumberToObject(skeleton, "maxItems", 512);
			edge = cJSON_AddObjectToObject(skeleton, "items");
			if (edge != NULL) {
				cJSON_AddStringToObject(edge, "type", "array");
				cJSON_AddNumberToObject(edge, "minItems", 2);
				cJSON_AddNumberToObject(edge, "maxItems", 2);
				index = cJSON_AddObjectToObject(edge, "items");
				if (index != NULL) {
					cJSON_AddStringToObject(index, "type", "integer");
					cJSON_AddNumberToObject(index, "minimum", 0);
				}
			}
		}
		break;
	}
	case MODEL_METADATA_SEGMENTATION: {
		cJSON *colors;
		cJSON *color;
		add_schema_required(schema, segmentation_required, 2);
		add_string_list_schema(properties, "labels", 1, 256);
		add_rgb_order_schema(properties);
		colors = add_schema_property(properties, "colors", "array");
		if (colors != NULL) {
			cJSON_AddStringToObject(colors, "x-lengthEquals", "labels");
			color = cJSON_AddObjectToObject(colors, "items");
			if (color != NULL) {
				cJSON_AddStringToObject(color, "type", "string");
				cJSON_AddStringToObject(color, "pattern", "^#[0-9A-Fa-f]{6}$");
			}
		}
		break;
	}
	case MODEL_METADATA_FEATURE:
		add_schema_required(schema, feature_required, 3);
		add_rgb_order_schema(properties);
		add_fixed_vector_schema(properties, "mean", 0);
		add_fixed_vector_schema(properties, "std", 1);
		break;
	case MODEL_METADATA_SOUND: {
		cJSON *sample_rate;
		cJSON *channels;
		cJSON *hop_length;
		cJSON *profile;
		cJSON *values;
		add_schema_required(schema, sound_required, 5);
		add_string_list_schema(properties, "labels", 2, 256);
		sample_rate = add_schema_property(properties, "sampleRate", "integer");
		values = sample_rate == NULL ? NULL : cJSON_AddArrayToObject(sample_rate, "enum");
		if (values != NULL) {
			cJSON_AddItemToArray(values, cJSON_CreateNumber(8000));
			cJSON_AddItemToArray(values, cJSON_CreateNumber(16000));
		}
		channels = add_schema_property(properties, "channels", "integer");
		values = channels == NULL ? NULL : cJSON_AddArrayToObject(channels, "enum");
		if (values != NULL)
			cJSON_AddItemToArray(values, cJSON_CreateNumber(1));
		hop_length = add_schema_property(properties, "hopLength", "integer");
		if (hop_length != NULL) {
			cJSON_AddNumberToObject(hop_length, "minimum", 1);
			cJSON_AddNumberToObject(hop_length, "maximum", 4096);
		}
		profile = add_schema_property(properties, "preprocessProfile", "string");
		values = profile == NULL ? NULL : cJSON_AddArrayToObject(profile, "enum");
		if (values != NULL) {
			cJSON_AddItemToArray(values, cJSON_CreateString("common"));
			cJSON_AddItemToArray(values, cJSON_CreateString("fixed"));
		}
		break;
	}
	}
	return 0;
}

static void add_deployment_schema(cJSON *item, const struct model_importer *importer)
{
	cJSON *schema;
	cJSON *properties;
	cJSON *threshold;
	cJSON *processing_size;
	cJSON *size_properties;
	cJSON *required;

	if (!importer->deployable)
		return;
	schema = cJSON_AddObjectToObject(item, "deploymentSchema");
	if (schema == NULL)
		return;
	cJSON_AddStringToObject(schema, "type", "object");
	cJSON_AddBoolToObject(schema, "additionalProperties", 0);
	required = cJSON_AddArrayToObject(schema, "required");
	if (required != NULL) {
		cJSON_AddItemToArray(required, cJSON_CreateString("threshold"));
		cJSON_AddItemToArray(required, cJSON_CreateString("processingSize"));
	}
	properties = cJSON_AddObjectToObject(schema, "properties");
	threshold = properties == NULL ? NULL :
		add_schema_property(properties, "threshold", "number");
	if (threshold != NULL) {
		cJSON_AddNumberToObject(threshold, "minimum", 0);
		cJSON_AddNumberToObject(threshold, "maximum", 1);
		cJSON_AddNumberToObject(threshold, "default", 0.5);
		cJSON_AddNumberToObject(threshold, "step", 0.01);
	}
	processing_size = properties == NULL ? NULL :
		add_schema_property(properties, "processingSize", "object");
	if (processing_size != NULL) {
		cJSON *size_required = cJSON_AddArrayToObject(processing_size, "required");
		cJSON_AddStringToObject(processing_size, "description",
			"送入检测管线的 VPSS 图像帧尺寸，不改变 BModel Tensor 尺寸");
		cJSON_AddBoolToObject(processing_size, "additionalProperties", 0);
		if (size_required != NULL) {
			cJSON_AddItemToArray(size_required, cJSON_CreateString("width"));
			cJSON_AddItemToArray(size_required, cJSON_CreateString("height"));
		}
		size_properties = cJSON_AddObjectToObject(processing_size, "properties");
		if (size_properties != NULL) {
			cJSON *width = add_schema_property(size_properties, "width", "integer");
			cJSON *height = add_schema_property(size_properties, "height", "integer");
			cJSON_AddNumberToObject(width, "minimum", OVIS_AI_MIN_WIDTH);
			cJSON_AddNumberToObject(width, "maximum", OVIS_AI_OBJECT_FRAME_MAX_WIDTH);
			cJSON_AddNumberToObject(width, "multipleOf", 2);
			cJSON_AddNumberToObject(width, "default", OVIS_AI_OBJECT_DEFAULT_WIDTH);
			cJSON_AddNumberToObject(height, "minimum", OVIS_AI_MIN_HEIGHT);
			cJSON_AddNumberToObject(height, "maximum", OVIS_AI_OBJECT_FRAME_MAX_HEIGHT);
			cJSON_AddNumberToObject(height, "multipleOf", 2);
			cJSON_AddNumberToObject(height, "default", OVIS_AI_OBJECT_DEFAULT_HEIGHT);
		}
	}
}

int model_importers_json(char *json, size_t size)
{
	static const int yolov5_anchors[18] = {
		10, 13, 16, 30, 33, 23, 30, 61, 62, 45, 59, 119,
		116, 90, 156, 198, 373, 326
	};
	static const int yolov7_anchors[18] = {
		12, 16, 19, 36, 40, 28, 36, 75, 76, 55, 72, 146,
		142, 110, 192, 243, 459, 401
	};
	struct statvfs fs;
	cJSON *root = cJSON_CreateObject();
	cJSON *array;
	cJSON *upload;
	size_t index;
	unsigned long long available = 0;

	if (root == NULL)
		return -1;
	array = cJSON_AddArrayToObject(root, "importers");
	upload = cJSON_AddObjectToObject(root, "upload");
	if (upload != NULL) {
		cJSON_AddStringToObject(upload, "strategy", "single-request");
		cJSON_AddStringToObject(upload, "contentType", "application/octet-stream");
		cJSON_AddBoolToObject(upload, "contentLengthRequired", 1);
		cJSON_AddStringToObject(upload, "contentLengthSource", "body");
		cJSON_AddBoolToObject(upload, "contentRange", 0);
		cJSON_AddBoolToObject(upload, "resumable", 0);
		cJSON_AddNumberToObject(upload, "retryOffset", 0);
		cJSON_AddStringToObject(upload, "progressSource", "client");
	}
	if (ensure_model_dirs() == 0 && statvfs(OVIS_MODEL_DIR, &fs) == 0) {
		available = (unsigned long long)fs.f_bavail * fs.f_frsize;
		if (available > OVIS_MODEL_RESERVED_BYTES)
			available -= OVIS_MODEL_RESERVED_BYTES;
		else
			available = 0;
	}
	if (root == NULL || array == NULL) {
		cJSON_Delete(root);
		return -1;
	}
	for (index = 0; index < sizeof(model_importers) / sizeof(model_importers[0]); index++) {
		const struct model_importer *importer = &model_importers[index];
		cJSON *item = cJSON_CreateObject();
		cJSON_AddStringToObject(item, "id", importer->id);
		cJSON_AddNumberToObject(item, "schemaVersion", 1);
		cJSON_AddStringToObject(item, "name", importer->name);
		cJSON_AddStringToObject(item, "task", importer->task);
		cJSON_AddBoolToObject(item, "enabled", 1);
		cJSON_AddBoolToObject(item, "deployable", importer->deployable);
		cJSON_AddNumberToObject(item, "maxFileSize", OVIS_MODEL_MAX_FILE_SIZE);
		{
			cJSON *consumers = cJSON_AddArrayToObject(item, "runtimeConsumers");
			if (consumers != NULL && importer->deployable)
				cJSON_AddItemToArray(consumers,
					cJSON_CreateString("ipcamera.object_detection"));
		}
		if (add_metadata_schema(item, importer) != 0) {
			cJSON_Delete(item);
			cJSON_Delete(root);
			return -1;
		}
		add_deployment_schema(item, importer);
		if (unsupported_label_count(importer) != 0) {
			cJSON *constraints = cJSON_AddObjectToObject(item, "constraints");
			cJSON *counts = constraints == NULL ? NULL :
				cJSON_AddArrayToObject(constraints, "unsupportedLabelCounts");
			if (counts != NULL)
				cJSON_AddItemToArray(counts,
					cJSON_CreateNumber(unsupported_label_count(importer)));
		}
		if (strcmp(importer->id, "detection.yolov5") == 0)
			add_default_anchors(item, yolov5_anchors);
		else if (strcmp(importer->id, "detection.yolov7") == 0)
			add_default_anchors(item, yolov7_anchors);
		else if (strcmp(importer->id, "classification.sound_command") == 0)
			add_default_sound_preprocess(item);
		cJSON_AddItemToArray(array, item);
	}
	cJSON_AddNumberToObject(root, "availableBytes", (double)available);
	return json_print(root, json, size);
}

int model_import_create(const char *body, char *json, size_t size,
	char *error, size_t error_size)
{
	static const char *const fields[] = {
		"importerId", "schemaVersion", "name", "fileSize", "metadata"
	};
	cJSON *request = cJSON_Parse(body);
	cJSON *importer_id;
	cJSON *schema_version;
	cJSON *name;
	cJSON *file_size;
	cJSON *metadata;
	const struct model_importer *importer;
	cJSON *stored = NULL;
	cJSON *response = NULL;
	char id[17];
	char path[512];
	time_t now = time(NULL);
	int result = -1;

	error[0] = '\0';
	if (!cJSON_IsObject(request) || !object_has_only(request, fields, 5))
		goto invalid;
	importer_id = cJSON_GetObjectItemCaseSensitive(request, "importerId");
	schema_version = cJSON_GetObjectItemCaseSensitive(request, "schemaVersion");
	name = cJSON_GetObjectItemCaseSensitive(request, "name");
	file_size = cJSON_GetObjectItemCaseSensitive(request, "fileSize");
	metadata = cJSON_GetObjectItemCaseSensitive(request, "metadata");
	if (!cJSON_IsString(importer_id) || !cJSON_IsNumber(schema_version) ||
	    schema_version->valuedouble != schema_version->valueint ||
	    schema_version->valueint != 1 || !cJSON_IsString(name) ||
	    !text_is_valid(name->valuestring, 64) || !cJSON_IsNumber(file_size) ||
	    file_size->valuedouble != file_size->valueint || file_size->valueint <= 0 ||
	    (unsigned int)file_size->valueint > OVIS_MODEL_MAX_FILE_SIZE)
		goto invalid;
	importer = find_importer(importer_id->valuestring);
	if (importer == NULL) {
		snprintf(error, error_size, "模型导入器不存在或当前固件不支持");
		goto done;
	}
	if (validate_metadata(importer, metadata, error, error_size) != 0)
		goto done;
	if (ensure_model_dirs() != 0 || generate_id(id) != 0) {
		snprintf(error, error_size, "无法创建模型导入目录");
		goto done;
	}
	stored = cJSON_CreateObject();
	if (stored == NULL) {
		snprintf(error, error_size, "无法创建模型导入元数据");
		goto done;
	}
	cJSON_AddStringToObject(stored, "id", id);
	cJSON_AddStringToObject(stored, "status", "created");
	cJSON_AddStringToObject(stored, "importerId", importer->id);
	cJSON_AddNumberToObject(stored, "schemaVersion", 1);
	cJSON_AddStringToObject(stored, "name", name->valuestring);
	cJSON_AddNumberToObject(stored, "fileSize", file_size->valueint);
	cJSON_AddNumberToObject(stored, "uploadedBytes", 0);
	cJSON_AddNumberToObject(stored, "createdAt", (double)now);
	{
		cJSON *metadata_copy = cJSON_Duplicate(metadata, 1);
		if (metadata_copy == NULL) {
			snprintf(error, error_size, "无法复制模型参数");
			goto done;
		}
		cJSON_AddItemToObject(stored, "metadata", metadata_copy);
	}
	snprintf(path, sizeof(path), "%s/%s.json", OVIS_MODEL_IMPORT_DIR, id);
	if (write_json_file(path, stored) != 0) {
		snprintf(error, error_size, "无法保存模型导入任务");
		goto done;
	}
	response = cJSON_Duplicate(stored, 1);
	if (response == NULL || json_print(response, json, size) != 0) {
		response = NULL;
		unlink(path);
		snprintf(error, error_size, "模型导入响应过大");
		goto done;
	}
	response = NULL;
	result = 0;
	audit_log("model.import.create", "success");
	goto done;

invalid:
	snprintf(error, error_size, "模型导入请求结构无效");
done:
	cJSON_Delete(request);
	cJSON_Delete(stored);
	cJSON_Delete(response);
	return result;
}

static int replace_status(cJSON *root, const char *status)
{
	cJSON *item = cJSON_CreateString(status);
	int result;

	if (item == NULL)
		return -1;
	result = cJSON_ReplaceItemInObjectCaseSensitive(root, "status", item) ? 0 : -1;
	if (result != 0)
		cJSON_Delete(item);
	return result;
}

int model_import_status(const char *id, char *json, size_t size,
	char *error, size_t error_size)
{
	char path[512];
	cJSON *root;

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型导入任务 ID 无效");
		return -1;
	}
	snprintf(path, sizeof(path), "%s/%s.json", OVIS_MODEL_IMPORT_DIR, id);
	root = read_json_file(path);
	if (root == NULL) {
		snprintf(error, error_size, "模型导入任务不存在");
		return -2;
	}
	if (json_print(root, json, size) != 0) {
		snprintf(error, error_size, "模型导入状态响应过大");
		return -1;
	}
	return 0;
}

int model_import_write_content(const char *id, int fd, const void *initial,
	size_t initial_size, size_t content_length, char *json, size_t size,
	char *error, size_t error_size)
{
	char metadata_path[512];
	char content_path[512];
	char buffer[16384];
	cJSON *root;
	cJSON *expected;
	struct statvfs fs;
	struct stat partial_status;
	unsigned long long available;
	size_t received = 0;
	uint64_t checksum = UINT64_C(14695981039346656037);
	int output = -1;
	int result = -1;

	if (!id_is_valid(id) || initial_size > content_length) {
		snprintf(error, error_size, "模型导入任务 ID 或上传长度无效");
		return -1;
	}
	snprintf(metadata_path, sizeof(metadata_path), "%s/%s.json", OVIS_MODEL_IMPORT_DIR, id);
	snprintf(content_path, sizeof(content_path), "%s/%s.part", OVIS_MODEL_IMPORT_DIR, id);
	root = read_json_file(metadata_path);
	if (root == NULL) {
		snprintf(error, error_size, "模型导入任务不存在");
		return -2;
	}
	expected = cJSON_GetObjectItemCaseSensitive(root, "fileSize");
	if (!cJSON_IsNumber(expected) || (size_t)expected->valueint != content_length ||
	    content_length == 0 || content_length > OVIS_MODEL_MAX_FILE_SIZE) {
		snprintf(error, error_size, "上传长度与创建任务时声明的模型大小不一致");
		goto done;
	}
	cJSON_DeleteItemFromObjectCaseSensitive(root, "validationError");
	cJSON_DeleteItemFromObjectCaseSensitive(root, "checksum");
	if (statvfs(OVIS_MODEL_DIR, &fs) != 0) {
		snprintf(error, error_size, "无法读取模型存储空间");
		goto done;
	}
	available = (unsigned long long)fs.f_bavail * fs.f_frsize;
	if (stat(content_path, &partial_status) == 0 && partial_status.st_size > 0)
		available += (unsigned long long)partial_status.st_size;
	if (available <= OVIS_MODEL_RESERVED_BYTES ||
	    content_length > available - OVIS_MODEL_RESERVED_BYTES) {
		snprintf(error, error_size, "模型存储空间不足");
		goto done;
	}
	output = open(content_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (output < 0) {
		snprintf(error, error_size, "无法创建模型上传文件");
		goto done;
	}
	while (received < content_length) {
		const unsigned char *data;
		size_t count;
		ssize_t written;

		if (received < initial_size) {
			data = (const unsigned char *)initial + received;
			count = initial_size - received;
		} else {
			ssize_t fetched = recv(fd, buffer,
				content_length - received < sizeof(buffer) ?
				content_length - received : sizeof(buffer), 0);
			if (fetched < 0 && errno == EINTR)
				continue;
			if (fetched <= 0) {
				snprintf(error, error_size, "模型上传连接提前中断");
				goto done;
			}
			data = (const unsigned char *)buffer;
			count = (size_t)fetched;
		}
		{
			size_t offset = 0;
			while (offset < count) {
				written = write(output, data + offset, count - offset);
				if (written < 0 && errno == EINTR)
					continue;
				if (written <= 0) {
					snprintf(error, error_size, "写入模型文件失败");
					goto done;
				}
				offset += (size_t)written;
			}
		}
		for (written = 0; written < (ssize_t)count; written++) {
			checksum ^= data[written];
			checksum *= UINT64_C(1099511628211);
		}
		received += count;
	}
	if (fsync(output) != 0) {
		snprintf(error, error_size, "无法同步模型上传文件");
		goto done;
	}
	if (close(output) != 0) {
		output = -1;
		snprintf(error, error_size, "无法关闭模型上传文件");
		goto done;
	}
	output = -1;
	if (replace_status(root, "uploaded") != 0) {
		snprintf(error, error_size, "无法更新模型上传状态");
		goto done;
	}
	cJSON_DeleteItemFromObjectCaseSensitive(root, "error");
	cJSON_ReplaceItemInObjectCaseSensitive(root, "uploadedBytes",
		cJSON_CreateNumber((double)received));
	{
		char checksum_text[17];
		snprintf(checksum_text, sizeof(checksum_text), "%016llx",
			(unsigned long long)checksum);
		cJSON_AddStringToObject(root, "checksum", checksum_text);
	}
	if (write_json_file(metadata_path, root) != 0 ||
	    json_print(cJSON_Duplicate(root, 1), json, size) != 0) {
		snprintf(error, error_size, "无法保存模型上传状态");
		goto done;
	}
	result = 0;
	audit_log("model.import.upload", "success");
done:
	if (output >= 0)
		close(output);
	if (result != 0 && root != NULL) {
		replace_status(root, "failed");
		cJSON_ReplaceItemInObjectCaseSensitive(root, "uploadedBytes",
			cJSON_CreateNumber((double)received));
		cJSON_DeleteItemFromObjectCaseSensitive(root, "error");
		cJSON_AddStringToObject(root, "error",
			error[0] == '\0' ? "模型上传失败" : error);
		write_json_file(metadata_path, root);
		audit_log("model.import.upload", "failed");
	}
	cJSON_Delete(root);
	return result;
}

static void model_paths(const char *id, char *directory, size_t directory_size,
	char *model_path, size_t model_path_size, char *config_path,
	size_t config_path_size, char *metadata_path, size_t metadata_path_size)
{
	snprintf(directory, directory_size, "%s/%s", OVIS_MODEL_STORE_DIR, id);
	snprintf(model_path, model_path_size, "%s/%s/model.bmodel", OVIS_MODEL_STORE_DIR, id);
	snprintf(config_path, config_path_size, "%s/%s/factory.json", OVIS_MODEL_STORE_DIR, id);
	snprintf(metadata_path, metadata_path_size, "%s/%s/metadata.json", OVIS_MODEL_STORE_DIR, id);
}

static int bmodel_header_is_valid(const char *path)
{
	static const unsigned char magic[] = { 0xee, 0xaa, 0x55, 0xff };
	unsigned char header[sizeof(magic)];
	struct stat status;
	int fd;
	ssize_t count;

	if (stat(path, &status) != 0 || status.st_size < 64 ||
	    status.st_size > OVIS_MODEL_MAX_FILE_SIZE)
		return 0;
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	count = read(fd, header, sizeof(header));
	close(fd);
	return count == (ssize_t)sizeof(header) && memcmp(header, magic, sizeof(magic)) == 0;
}

static void add_rgb_order(cJSON *entry, cJSON *metadata)
{
	cJSON *order = cJSON_GetObjectItemCaseSensitive(metadata, "rgbOrder");
	if (cJSON_IsString(order))
		cJSON_AddStringToObject(entry, "rgb_order",
			strcasecmp(order->valuestring, "bgr") == 0 ? "bgr" : "rgb");
}

static int anchors_to_text(cJSON *anchors, char *text, size_t size)
{
	size_t used = 0;
	int head;

	for (head = 0; head < 3; head++) {
		int anchor;
		cJSON *group = cJSON_GetArrayItem(anchors, head);
		for (anchor = 0; anchor < 3; anchor++) {
			int value;
			cJSON *pair = cJSON_GetArrayItem(group, anchor);
			for (value = 0; value < 2; value++) {
				cJSON *number = cJSON_GetArrayItem(pair, value);
				int written = snprintf(text + used, size - used, "%s%d",
					used == 0 ? "" : ",", number->valueint);
				if (written < 0 || (size_t)written >= size - used)
					return -1;
				used += (size_t)written;
			}
		}
	}
	return 0;
}

static int create_factory_config(const struct model_importer *importer,
	cJSON *metadata, const char *path)
{
	cJSON *root = cJSON_CreateObject();
	cJSON *list;
	cJSON *entry;
	cJSON *labels = cJSON_GetObjectItemCaseSensitive(metadata, "labels");

	if (root == NULL)
		return -1;
	list = cJSON_AddObjectToObject(root, "model_list");
	entry = list == NULL ? NULL : cJSON_AddObjectToObject(list, importer->model_type);
	if (list == NULL || entry == NULL) {
		cJSON_Delete(root);
		return -1;
	}
	cJSON_AddStringToObject(entry, "file_name", "model.bmodel");
	if (cJSON_IsArray(labels))
		cJSON_AddItemToObject(entry, "types", cJSON_Duplicate(labels, 1));
	switch (importer->metadata_kind) {
	case MODEL_METADATA_DETECTION_ANCHORS: {
		char anchors[192];
		if (anchors_to_text(cJSON_GetObjectItemCaseSensitive(metadata, "anchors"),
				anchors, sizeof(anchors)) != 0) {
			cJSON_Delete(root);
			return -1;
		}
		cJSON_AddStringToObject(entry, "anchors", anchors);
		break;
	}
	case MODEL_METADATA_CLASSIFICATION:
	case MODEL_METADATA_FEATURE:
		add_rgb_order(entry, metadata);
		cJSON_AddItemToObject(entry, "mean", cJSON_Duplicate(
			cJSON_GetObjectItemCaseSensitive(metadata, "mean"), 1));
		cJSON_AddItemToObject(entry, "std", cJSON_Duplicate(
			cJSON_GetObjectItemCaseSensitive(metadata, "std"), 1));
		break;
	case MODEL_METADATA_POSE:
		add_rgb_order(entry, metadata);
		cJSON_AddNumberToObject(entry, "num_cls", cJSON_GetArraySize(labels));
		cJSON_AddNumberToObject(entry, "num_kpt", cJSON_GetArraySize(
			cJSON_GetObjectItemCaseSensitive(metadata, "keypoints")));
		break;
	case MODEL_METADATA_SEGMENTATION:
		add_rgb_order(entry, metadata);
		break;
	case MODEL_METADATA_SOUND: {
		cJSON *profile = cJSON_GetObjectItemCaseSensitive(metadata, "preprocessProfile");
		cJSON_AddStringToObject(entry, "rgb_order", "gray");
		cJSON_AddNumberToObject(entry, "hop_len",
			cJSON_GetObjectItemCaseSensitive(metadata, "hopLength")->valueint);
		cJSON_AddNumberToObject(entry, "fix",
			strcmp(profile->valuestring, "fixed") == 0 ? 1 : 0);
		break;
	}
	case MODEL_METADATA_DETECTION:
		break;
	}
	if (write_json_file(path, root) != 0) {
		cJSON_Delete(root);
		return -1;
	}
	cJSON_Delete(root);
	return 0;
}

int model_import_commit(const char *id, char *json, size_t size,
	char *error, size_t error_size)
{
	char import_metadata[512];
	char import_content[512];
	char final_directory[512];
	char final_model[512];
	char final_config[512];
	char final_metadata[512];
	char temporary_directory[512];
	char temporary_model[512];
	char temporary_config[512];
	char temporary_metadata[512];
	cJSON *root;
	cJSON *status;
	cJSON *importer_id;
	cJSON *metadata;
	const struct model_importer *importer;
	int result = -1;

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型导入任务 ID 无效");
		return -1;
	}
	snprintf(import_metadata, sizeof(import_metadata), "%s/%s.json", OVIS_MODEL_IMPORT_DIR, id);
	snprintf(import_content, sizeof(import_content), "%s/%s.part", OVIS_MODEL_IMPORT_DIR, id);
	root = read_json_file(import_metadata);
	if (root == NULL) {
		snprintf(error, error_size, "模型导入任务不存在");
		return -2;
	}
	status = cJSON_GetObjectItemCaseSensitive(root, "status");
	importer_id = cJSON_GetObjectItemCaseSensitive(root, "importerId");
	metadata = cJSON_GetObjectItemCaseSensitive(root, "metadata");
	if (!cJSON_IsString(status) || strcmp(status->valuestring, "uploaded") != 0 ||
	    !cJSON_IsString(importer_id) || !bmodel_header_is_valid(import_content)) {
		snprintf(error, error_size, "模型文件未上传完成或不是有效的 BModel");
		goto done;
	}
	importer = find_importer(importer_id->valuestring);
	if (importer == NULL || validate_metadata(importer, metadata, error, error_size) != 0)
		goto done;
	model_paths(id, final_directory, sizeof(final_directory), final_model,
		sizeof(final_model), final_config, sizeof(final_config), final_metadata,
		sizeof(final_metadata));
	snprintf(temporary_directory, sizeof(temporary_directory), "%s/.%s.tmp",
		OVIS_MODEL_STORE_DIR, id);
	snprintf(temporary_model, sizeof(temporary_model), "%s/model.bmodel", temporary_directory);
	snprintf(temporary_config, sizeof(temporary_config), "%s/factory.json", temporary_directory);
	snprintf(temporary_metadata, sizeof(temporary_metadata), "%s/metadata.json", temporary_directory);
	if (access(final_directory, F_OK) == 0 || mkdir(temporary_directory, 0755) != 0) {
		snprintf(error, error_size, "模型已经存在或无法创建模型目录");
		goto done;
	}
	if (rename(import_content, temporary_model) != 0) {
		snprintf(error, error_size, "无法提交模型文件");
		goto cleanup_directory;
	}
	if (create_factory_config(importer, metadata, temporary_config) != 0) {
		snprintf(error, error_size, "无法生成模型工厂配置");
		goto restore_content;
	}
	cJSON_DeleteItemFromObjectCaseSensitive(root, "modelType");
	cJSON_DeleteItemFromObjectCaseSensitive(root, "task");
	cJSON_DeleteItemFromObjectCaseSensitive(root, "deployable");
	cJSON_DeleteItemFromObjectCaseSensitive(root, "deployment");
	cJSON_DeleteItemFromObjectCaseSensitive(root, "committedAt");
	if (replace_status(root, "ready") != 0) {
		snprintf(error, error_size, "无法更新模型提交状态");
		goto restore_content;
	}
	cJSON_DeleteItemFromObjectCaseSensitive(root, "validationError");
	cJSON_AddStringToObject(root, "modelType", importer->model_type);
	cJSON_AddStringToObject(root, "task", importer->task);
	cJSON_AddBoolToObject(root, "deployable", importer->deployable);
	if (importer->deployable) {
		cJSON *deployment = cJSON_AddObjectToObject(root, "deployment");
		cJSON *processing_size;
		if (deployment == NULL) {
			snprintf(error, error_size, "无法创建模型部署参数");
			goto restore_content;
		}
		cJSON_AddNumberToObject(deployment, "threshold", 0.5);
		processing_size = cJSON_AddObjectToObject(deployment, "processingSize");
		if (processing_size == NULL) {
			snprintf(error, error_size, "无法创建模型 AI 输入帧尺寸");
			goto restore_content;
		}
		cJSON_AddNumberToObject(processing_size, "width", OVIS_AI_OBJECT_DEFAULT_WIDTH);
		cJSON_AddNumberToObject(processing_size, "height", OVIS_AI_OBJECT_DEFAULT_HEIGHT);
	}
	cJSON_AddNumberToObject(root, "committedAt", (double)time(NULL));
	if (write_json_file(temporary_metadata, root) != 0 ||
	    rename(temporary_directory, final_directory) != 0) {
		snprintf(error, error_size, "无法原子提交模型目录");
		goto restore_content;
	}
	unlink(import_metadata);
	result = 0;
	if (json_print(cJSON_Duplicate(root, 1), json, size) != 0) {
		if (snprintf(json, size, "{\"id\":\"%s\",\"status\":\"ready\"}", id) >=
		    (int)size)
			json[0] = '\0';
	}
	audit_log("model.import.commit", "success");
	goto done;

restore_content:
	unlink(temporary_config);
	unlink(temporary_metadata);
	rename(temporary_model, import_content);
cleanup_directory:
	rmdir(temporary_directory);
done:
	if (result != 0) {
		if (root != NULL && error[0] != '\0') {
			cJSON *current_status = cJSON_GetObjectItemCaseSensitive(root, "status");
			if (cJSON_IsString(current_status) &&
			    strcmp(current_status->valuestring, "ready") == 0)
				replace_status(root, "uploaded");
			cJSON_DeleteItemFromObjectCaseSensitive(root, "modelType");
			cJSON_DeleteItemFromObjectCaseSensitive(root, "task");
			cJSON_DeleteItemFromObjectCaseSensitive(root, "deployable");
			cJSON_DeleteItemFromObjectCaseSensitive(root, "deployment");
			cJSON_DeleteItemFromObjectCaseSensitive(root, "committedAt");
			cJSON_DeleteItemFromObjectCaseSensitive(root, "validationError");
			cJSON_AddStringToObject(root, "validationError", error);
			write_json_file(import_metadata, root);
		}
		audit_log("model.import.commit", "failed");
	}
	cJSON_Delete(root);
	return result;
}

static int detection_model_state_from_config(const char *config_path,
	int require_enabled, char id[17], double *threshold, int *processing_width,
	int *processing_height)
{
	char line[1024];
	char section[64] = "";
	char model_path[512] = "";
	int enabled = 0;
	double configured_threshold = 0.5;
	int configured_width = OVIS_AI_OBJECT_DEFAULT_WIDTH;
	int configured_height = OVIS_AI_OBJECT_DEFAULT_HEIGHT;
	FILE *file = fopen(config_path, "r");

	id[0] = '\0';
	if (threshold != NULL)
		*threshold = configured_threshold;
	if (processing_width != NULL)
		*processing_width = configured_width;
	if (processing_height != NULL)
		*processing_height = configured_height;
	if (file == NULL)
		return -1;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *text = line;
		char *equals;

		while (isspace((unsigned char)*text))
			text++;
		if (*text == '[') {
			char *close = strchr(text, ']');
			if (close != NULL) {
				*close = '\0';
				snprintf(section, sizeof(section), "%s", text + 1);
			}
			continue;
		}
		if (strcmp(section, "ai_pd_config") != 0 || (equals = strchr(text, '=')) == NULL)
			continue;
		*equals = '\0';
		{
			char *key = text;
			char *value = equals + 1;
			char *end;
			while (isspace((unsigned char)*key)) key++;
			end = key + strlen(key);
			while (end > key && isspace((unsigned char)end[-1])) *--end = '\0';
			while (isspace((unsigned char)*value)) value++;
			end = value + strlen(value);
			while (end > value && isspace((unsigned char)end[-1])) *--end = '\0';
			if (strcmp(key, "pd_enable") == 0)
				enabled = atoi(value);
			else if (strcmp(key, "threshold") == 0)
				configured_threshold = strtod(value, NULL);
			else if (strcmp(key, "grp_width") == 0)
				configured_width = atoi(value);
			else if (strcmp(key, "grp_height") == 0)
				configured_height = atoi(value);
			else if (strcmp(key, "model_path") == 0) {
				if (*value == '"') value++;
				end = value + strlen(value);
				if (end > value && end[-1] == '"') end[-1] = '\0';
				snprintf(model_path, sizeof(model_path), "%s", value);
			}
		}
	}
	fclose(file);
	if (threshold != NULL)
		*threshold = configured_threshold;
	if (processing_width != NULL)
		*processing_width = configured_width;
	if (processing_height != NULL)
		*processing_height = configured_height;
	if (require_enabled && !enabled)
		return 0;
	{
		char prefix[512];
		const char *start;
		snprintf(prefix, sizeof(prefix), "%s/", OVIS_MODEL_STORE_DIR);
		if (strncmp(model_path, prefix, strlen(prefix)) != 0)
			return 0;
		start = model_path + strlen(prefix);
		if (strlen(start) != 29 || strcmp(start + 16, "/model.bmodel") != 0)
			return 0;
		memcpy(id, start, 16);
		id[16] = '\0';
		if (!id_is_valid(id))
			id[0] = '\0';
	}
	return 0;
}

static int detection_model_from_config(const char *config_path,
	int require_enabled, char id[17])
{
	return detection_model_state_from_config(config_path, require_enabled, id,
		NULL, NULL, NULL);
}

static int active_detection_model(char id[17])
{
	return detection_model_from_config(OVIS_CONFIG_FILE, 1, id);
}

static cJSON *model_summary(cJSON *metadata, const char *active_id,
	const char *referenced_id)
{
	const char *fields[] = {
		"id", "status", "importerId", "schemaVersion", "name", "fileSize",
		"createdAt", "committedAt", "modelType", "task", "deployable",
		"deployment"
	};
	cJSON *summary = cJSON_CreateObject();
	cJSON *details;
	size_t index;
	cJSON *stored_metadata;

	if (summary == NULL)
		return NULL;
	details = cJSON_AddObjectToObject(summary, "metadataSummary");
	if (details == NULL) {
		cJSON_Delete(summary);
		return NULL;
	}
	for (index = 0; index < sizeof(fields) / sizeof(fields[0]); index++) {
		cJSON *item = cJSON_GetObjectItemCaseSensitive(metadata, fields[index]);
		if (item != NULL)
			cJSON_AddItemToObject(summary, fields[index], cJSON_Duplicate(item, 1));
	}
	stored_metadata = cJSON_GetObjectItemCaseSensitive(metadata, "metadata");
	if (cJSON_IsObject(stored_metadata)) {
		cJSON *labels = cJSON_GetObjectItemCaseSensitive(stored_metadata, "labels");
		cJSON *keypoints = cJSON_GetObjectItemCaseSensitive(stored_metadata, "keypoints");
		if (cJSON_IsArray(labels))
			cJSON_AddNumberToObject(details, "labelsCount", cJSON_GetArraySize(labels));
		if (cJSON_IsArray(keypoints))
			cJSON_AddNumberToObject(details, "keypointsCount", cJSON_GetArraySize(keypoints));
	}
	{
		cJSON *id = cJSON_GetObjectItemCaseSensitive(metadata, "id");
		cJSON_AddBoolToObject(summary, "active", cJSON_IsString(id) && active_id[0] != '\0' &&
			strcmp(id->valuestring, active_id) == 0);
		cJSON_AddBoolToObject(summary, "referenced", cJSON_IsString(id) &&
			referenced_id[0] != '\0' && strcmp(id->valuestring, referenced_id) == 0);
	}
	return summary;
}

int model_list_json(char *json, size_t size)
{
	cJSON *root = cJSON_CreateObject();
	cJSON *models;
	DIR *directory;
	struct dirent *entry;
	struct statvfs fs;
	char active_id[17];
	char referenced_id[17];
	unsigned long long total = 0;
	unsigned long long available = 0;
	int count = 0;

	if (root == NULL)
		return -1;
	models = cJSON_AddArrayToObject(root, "models");
	if (models == NULL || ensure_model_dirs() != 0) {
		cJSON_Delete(root);
		return -1;
	}
	active_detection_model(active_id);
	detection_model_from_config(OVIS_CONFIG_FILE, 0, referenced_id);
	directory = opendir(OVIS_MODEL_STORE_DIR);
	if (directory == NULL) {
		cJSON_Delete(root);
		return -1;
	}
	while ((entry = readdir(directory)) != NULL && count < 128) {
		char metadata_path[512];
		cJSON *metadata;
		cJSON *summary;

		if (!id_is_valid(entry->d_name))
			continue;
		snprintf(metadata_path, sizeof(metadata_path), "%s/%s/metadata.json",
			OVIS_MODEL_STORE_DIR, entry->d_name);
		metadata = read_json_file(metadata_path);
		if (metadata == NULL)
			continue;
		summary = model_summary(metadata, active_id, referenced_id);
		cJSON_Delete(metadata);
		if (summary != NULL) {
			cJSON_AddItemToArray(models, summary);
			count++;
		}
	}
	closedir(directory);
	if (statvfs(OVIS_MODEL_DIR, &fs) == 0) {
		total = (unsigned long long)fs.f_blocks * fs.f_frsize;
		available = (unsigned long long)fs.f_bavail * fs.f_frsize;
	}
	{
		cJSON *storage = cJSON_AddObjectToObject(root, "storage");
		cJSON_AddNumberToObject(storage, "totalBytes", (double)total);
		cJSON_AddNumberToObject(storage, "availableBytes", (double)available);
		cJSON_AddNumberToObject(storage, "reservedBytes", OVIS_MODEL_RESERVED_BYTES);
	}
	return json_print(root, json, size);
}

int model_get_json(const char *id, char *json, size_t size,
	char *error, size_t error_size)
{
	char directory[512];
	char model_path[512];
	char config_path[512];
	char metadata_path[512];
	char active_id[17];
	char referenced_id[17];
	cJSON *root;

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型 ID 无效");
		return -1;
	}
	model_paths(id, directory, sizeof(directory), model_path, sizeof(model_path),
		config_path, sizeof(config_path), metadata_path, sizeof(metadata_path));
	root = read_json_file(metadata_path);
	if (root == NULL) {
		snprintf(error, error_size, "模型不存在");
		return -2;
	}
	active_detection_model(active_id);
	detection_model_from_config(OVIS_CONFIG_FILE, 0, referenced_id);
	cJSON_AddBoolToObject(root, "active", active_id[0] != '\0' && strcmp(active_id, id) == 0);
	cJSON_AddBoolToObject(root, "referenced", referenced_id[0] != '\0' &&
		strcmp(referenced_id, id) == 0);
	if (json_print(root, json, size) != 0) {
		snprintf(error, error_size, "模型详情响应过大");
		return -1;
	}
	return 0;
}

int model_import_cancel(const char *id, char *error, size_t error_size)
{
	char metadata_path[512];
	char content_path[512];

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型导入任务 ID 无效");
		return -1;
	}
	snprintf(metadata_path, sizeof(metadata_path), "%s/%s.json", OVIS_MODEL_IMPORT_DIR, id);
	snprintf(content_path, sizeof(content_path), "%s/%s.part", OVIS_MODEL_IMPORT_DIR, id);
	if (access(metadata_path, F_OK) != 0) {
		snprintf(error, error_size, "模型导入任务不存在");
		return -2;
	}
	unlink(content_path);
	if (unlink(metadata_path) != 0) {
		snprintf(error, error_size, "无法取消模型导入任务");
		return -1;
	}
	audit_log("model.import.cancel", "success");
	return 0;
}

static int model_directory_is_clean(const char *directory)
{
	DIR *stream = opendir(directory);
	struct dirent *entry;
	int clean = 1;

	if (stream == NULL)
		return 0;
	while ((entry = readdir(stream)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
		    strcmp(entry->d_name, "model.bmodel") == 0 ||
		    strcmp(entry->d_name, "factory.json") == 0 ||
		    strcmp(entry->d_name, "metadata.json") == 0)
			continue;
		clean = 0;
		break;
	}
	closedir(stream);
	return clean;
}

int model_delete(const char *id, char *error, size_t error_size)
{
	char directory[512];
	char model_path[512];
	char config_path[512];
	char metadata_path[512];
	char referenced_id[17];
	char pending_id[17];

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型 ID 无效");
		return -1;
	}
	detection_model_from_config(OVIS_CONFIG_FILE, 0, referenced_id);
	detection_model_from_config(OVIS_CONFIG_PENDING, 0, pending_id);
	if ((referenced_id[0] != '\0' && strcmp(referenced_id, id) == 0) ||
	    (pending_id[0] != '\0' && strcmp(pending_id, id) == 0)) {
		snprintf(error, error_size, "运行中或等待应用的模型不能删除，请先停用并等待任务完成");
		return -3;
	}
	model_paths(id, directory, sizeof(directory), model_path, sizeof(model_path),
		config_path, sizeof(config_path), metadata_path, sizeof(metadata_path));
	if (access(metadata_path, F_OK) != 0) {
		snprintf(error, error_size, "模型不存在");
		return -2;
	}
	if (!model_directory_is_clean(directory)) {
		snprintf(error, error_size, "模型目录包含未知文件，拒绝删除");
		return -1;
	}
	if (config_rebase_backup_away_from_model(id, error, error_size) != 0)
		return -1;
	if (unlink(model_path) != 0 || unlink(config_path) != 0 ||
	    unlink(metadata_path) != 0) {
		snprintf(error, error_size, "无法删除模型文件");
		return -1;
	}
	if (rmdir(directory) != 0) {
		snprintf(error, error_size, "无法删除模型目录");
		return -1;
	}
	audit_log("model.delete", "success");
	return 0;
}

struct model_deployment_params {
	double threshold;
	int processing_width;
	int processing_height;
};

static int model_processing_size_valid(int width, int height)
{
	return width >= OVIS_AI_MIN_WIDTH && width <= OVIS_AI_OBJECT_FRAME_MAX_WIDTH &&
		height >= OVIS_AI_MIN_HEIGHT && height <= OVIS_AI_OBJECT_FRAME_MAX_HEIGHT &&
		(width % 2) == 0 && (height % 2) == 0;
}

static void model_saved_deployment(cJSON *metadata,
	struct model_deployment_params *params)
{
	cJSON *deployment = cJSON_GetObjectItemCaseSensitive(metadata, "deployment");
	cJSON *threshold = cJSON_IsObject(deployment) ?
		cJSON_GetObjectItemCaseSensitive(deployment, "threshold") : NULL;
	cJSON *size = cJSON_IsObject(deployment) ?
		cJSON_GetObjectItemCaseSensitive(deployment, "processingSize") : NULL;
	cJSON *width = cJSON_IsObject(size) ?
		cJSON_GetObjectItemCaseSensitive(size, "width") : NULL;
	cJSON *height = cJSON_IsObject(size) ?
		cJSON_GetObjectItemCaseSensitive(size, "height") : NULL;

	params->threshold = 0.5;
	params->processing_width = OVIS_AI_OBJECT_DEFAULT_WIDTH;
	params->processing_height = OVIS_AI_OBJECT_DEFAULT_HEIGHT;
	if (cJSON_IsNumber(threshold) && isfinite(threshold->valuedouble) &&
	    threshold->valuedouble >= 0 && threshold->valuedouble <= 1)
		params->threshold = threshold->valuedouble;
	if (cJSON_IsNumber(width) && cJSON_IsNumber(height) &&
	    model_processing_size_valid(width->valueint, height->valueint)) {
		params->processing_width = width->valueint;
		params->processing_height = height->valueint;
	}
}

static int parse_deployment(const char *body, int required,
	const struct model_deployment_params *fallback,
	struct model_deployment_params *params, char *error, size_t error_size)
{
	static const char *const fields[] = { "threshold", "processingSize" };
	static const char *const size_fields[] = { "width", "height" };
	cJSON *root;
	cJSON *threshold;
	cJSON *size;
	cJSON *width;
	cJSON *height;
	int result = -1;

	*params = *fallback;
	if (error != NULL && error_size > 0)
		error[0] = '\0';
	if (body == NULL || body[0] == '\0') {
		if (!required)
			return 0;
		snprintf(error, error_size, "部署参数不能为空");
		return -1;
	}
	root = cJSON_Parse(body);
	if (!cJSON_IsObject(root)) {
		snprintf(error, error_size, "部署参数必须是 JSON 对象");
		goto done;
	}
	if (!object_has_only(root, fields, 2)) {
		snprintf(error, error_size, "部署参数包含不支持的字段");
		goto done;
	}
	threshold = cJSON_GetObjectItemCaseSensitive(root, "threshold");
	size = cJSON_GetObjectItemCaseSensitive(root, "processingSize");
	if (required && (threshold == NULL || size == NULL)) {
		snprintf(error, error_size, "threshold 和 processingSize 均为必填项");
		goto done;
	}
	if (threshold != NULL) {
		if (!cJSON_IsNumber(threshold) || !isfinite(threshold->valuedouble) ||
		    threshold->valuedouble < 0 || threshold->valuedouble > 1) {
			snprintf(error, error_size, "threshold 必须是 0 到 1 之间的数值");
			goto done;
		}
		params->threshold = threshold->valuedouble;
	}
	if (size != NULL) {
		width = cJSON_GetObjectItemCaseSensitive(size, "width");
		height = cJSON_GetObjectItemCaseSensitive(size, "height");
		if (!cJSON_IsObject(size) || !object_has_only(size, size_fields, 2)) {
			snprintf(error, error_size,
				"processingSize 只能包含 width 和 height");
			goto done;
		}
		if (!cJSON_IsNumber(width) || !cJSON_IsNumber(height) ||
		    width->valuedouble != width->valueint ||
		    height->valuedouble != height->valueint) {
			snprintf(error, error_size,
				"AI 输入帧宽高必须是整数");
			goto done;
		}
		if (!model_processing_size_valid(width->valueint, height->valueint)) {
			snprintf(error, error_size,
				"AI 输入帧尺寸必须为 160x96 到 1920x1080 范围内的偶数");
			goto done;
		}
		params->processing_width = width->valueint;
		params->processing_height = height->valueint;
	}
	result = 0;
done:
	cJSON_Delete(root);
	return result;
}

static int model_deployment_response(const char *id, cJSON *metadata,
	char *json, size_t size, char *error, size_t error_size)
{
	char active_id[17];
	char referenced_id[17];
	struct model_deployment_params saved;
	struct model_deployment_params applied = {
		0.5, OVIS_AI_OBJECT_DEFAULT_WIDTH, OVIS_AI_OBJECT_DEFAULT_HEIGHT
	};
	cJSON *root = cJSON_CreateObject();
	cJSON *parameters;
	cJSON *schema;
	int referenced;

	if (root == NULL)
		goto failed;
	active_detection_model(active_id);
	model_saved_deployment(metadata, &saved);
	detection_model_state_from_config(OVIS_CONFIG_FILE, 0, referenced_id,
		&applied.threshold, &applied.processing_width,
		&applied.processing_height);
	referenced = referenced_id[0] != '\0' && strcmp(referenced_id, id) == 0;
	cJSON_AddStringToObject(root, "modelId", id);
	cJSON_AddNumberToObject(root, "schemaVersion", 1);
	cJSON_AddBoolToObject(root, "deployable", 1);
	cJSON_AddBoolToObject(root, "active", active_id[0] != '\0' &&
		strcmp(active_id, id) == 0);
	cJSON_AddBoolToObject(root, "referenced", referenced);
	parameters = cJSON_AddObjectToObject(root, "parameters");
	schema = cJSON_AddObjectToObject(root, "parameterSchema");
	if (parameters == NULL || schema == NULL)
		goto failed;
	cJSON_AddNumberToObject(parameters, "threshold", saved.threshold);
	{
		cJSON *processing_size = cJSON_AddObjectToObject(parameters, "processingSize");
		cJSON_AddNumberToObject(processing_size, "width", saved.processing_width);
		cJSON_AddNumberToObject(processing_size, "height", saved.processing_height);
	}
	if (referenced) {
		cJSON *applied_json = cJSON_AddObjectToObject(root, "appliedParameters");
		if (applied_json == NULL)
			goto failed;
		cJSON_AddNumberToObject(applied_json, "threshold", applied.threshold);
		{
			cJSON *processing_size = cJSON_AddObjectToObject(applied_json,
				"processingSize");
			cJSON_AddNumberToObject(processing_size, "width",
				applied.processing_width);
			cJSON_AddNumberToObject(processing_size, "height",
				applied.processing_height);
		}
	} else {
		cJSON_AddNullToObject(root, "appliedParameters");
	}
	{
		cJSON *threshold = cJSON_AddObjectToObject(schema, "threshold");
		if (threshold == NULL)
			goto failed;
		cJSON_AddStringToObject(threshold, "type", "number");
		cJSON_AddNumberToObject(threshold, "minimum", 0);
		cJSON_AddNumberToObject(threshold, "maximum", 1);
		cJSON_AddNumberToObject(threshold, "default", 0.5);
		cJSON_AddNumberToObject(threshold, "step", 0.01);
	}
	{
		cJSON *processing_size = cJSON_AddObjectToObject(schema, "processingSize");
		cJSON_AddStringToObject(processing_size, "type", "object");
		cJSON_AddStringToObject(processing_size, "description",
			"送入检测管线的 VPSS 图像帧尺寸，不改变 BModel Tensor 尺寸");
		cJSON_AddNumberToObject(processing_size, "minWidth", OVIS_AI_MIN_WIDTH);
		cJSON_AddNumberToObject(processing_size, "maxWidth", OVIS_AI_OBJECT_FRAME_MAX_WIDTH);
		cJSON_AddNumberToObject(processing_size, "minHeight", OVIS_AI_MIN_HEIGHT);
		cJSON_AddNumberToObject(processing_size, "maxHeight", OVIS_AI_OBJECT_FRAME_MAX_HEIGHT);
		cJSON_AddNumberToObject(processing_size, "step", 2);
	}
	if (json_print(root, json, size) == 0)
		return 0;
	root = NULL;
failed:
	cJSON_Delete(root);
	snprintf(error, error_size, "无法生成模型部署参数响应");
	return -1;
}

int model_deployment_get_json(const char *id, char *json, size_t size,
	char *error, size_t error_size)
{
	char directory[512];
	char model_path[512];
	char config_path[512];
	char metadata_path[512];
	cJSON *metadata;
	cJSON *deployable;
	int result;

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型 ID 无效");
		return -1;
	}
	model_paths(id, directory, sizeof(directory), model_path, sizeof(model_path),
		config_path, sizeof(config_path), metadata_path, sizeof(metadata_path));
	metadata = read_json_file(metadata_path);
	if (metadata == NULL) {
		snprintf(error, error_size, "模型不存在");
		return -2;
	}
	deployable = cJSON_GetObjectItemCaseSensitive(metadata, "deployable");
	if (!cJSON_IsTrue(deployable)) {
		cJSON_Delete(metadata);
		snprintf(error, error_size, "当前固件没有该模型类型的运行管线");
		return -3;
	}
	result = model_deployment_response(id, metadata, json, size, error, error_size);
	cJSON_Delete(metadata);
	return result;
}

int model_deployment_update(const char *id, const char *body,
	char *json, size_t size, char *error, size_t error_size)
{
	char directory[512];
	char model_path[512];
	char config_path[512];
	char metadata_path[512];
	cJSON *metadata;
	cJSON *deployable;
	cJSON *deployment;
	struct model_deployment_params fallback = {
		0.5, OVIS_AI_OBJECT_DEFAULT_WIDTH, OVIS_AI_OBJECT_DEFAULT_HEIGHT
	};
	struct model_deployment_params params;
	int result;

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型 ID 无效");
		return -1;
	}
	if (parse_deployment(body, 1, &fallback, &params, error, error_size) != 0)
		return -1;
	model_paths(id, directory, sizeof(directory), model_path, sizeof(model_path),
		config_path, sizeof(config_path), metadata_path, sizeof(metadata_path));
	metadata = read_json_file(metadata_path);
	if (metadata == NULL) {
		snprintf(error, error_size, "模型不存在");
		return -2;
	}
	deployable = cJSON_GetObjectItemCaseSensitive(metadata, "deployable");
	if (!cJSON_IsTrue(deployable)) {
		cJSON_Delete(metadata);
		snprintf(error, error_size, "当前固件没有该模型类型的运行管线");
		return -3;
	}
	deployment = cJSON_CreateObject();
	if (deployment == NULL) {
		cJSON_Delete(metadata);
		snprintf(error, error_size, "无法创建模型部署参数");
		return -1;
	}
	cJSON_AddNumberToObject(deployment, "threshold", params.threshold);
	{
		cJSON *processing_size = cJSON_AddObjectToObject(deployment, "processingSize");
		cJSON_AddNumberToObject(processing_size, "width", params.processing_width);
		cJSON_AddNumberToObject(processing_size, "height", params.processing_height);
	}
	if (cJSON_GetObjectItemCaseSensitive(metadata, "deployment") != NULL)
		cJSON_ReplaceItemInObjectCaseSensitive(metadata, "deployment", deployment);
	else
		cJSON_AddItemToObject(metadata, "deployment", deployment);
	if (write_json_file(metadata_path, metadata) != 0) {
		cJSON_Delete(metadata);
		snprintf(error, error_size, "无法保存模型部署参数");
		return -1;
	}
	result = model_deployment_response(id, metadata, json, size, error, error_size);
	cJSON_Delete(metadata);
	if (result == 0)
		audit_log("model.deployment.update", "success");
	return result;
}

int model_activate(const char *id, const char *body, char *json, size_t size,
	char *error, size_t error_size)
{
	char directory[512];
	char model_path[512];
	char config_path[512];
	char metadata_path[512];
	char revision[17];
	char tdl_model_id[96];
	cJSON *metadata;
	cJSON *deployable;
	cJSON *model_type;
	struct model_deployment_params saved;
	struct model_deployment_params params;
	unsigned long task_id;

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型 ID 无效");
		return -1;
	}
	model_paths(id, directory, sizeof(directory), model_path, sizeof(model_path),
		config_path, sizeof(config_path), metadata_path, sizeof(metadata_path));
	metadata = read_json_file(metadata_path);
	if (metadata == NULL) {
		snprintf(error, error_size, "模型不存在");
		return -2;
	}
	deployable = cJSON_GetObjectItemCaseSensitive(metadata, "deployable");
	model_type = cJSON_GetObjectItemCaseSensitive(metadata, "modelType");
	if (!cJSON_IsTrue(deployable) || !cJSON_IsString(model_type)) {
		cJSON_Delete(metadata);
		snprintf(error, error_size, "当前固件没有该模型类型的运行管线");
		return -3;
	}
	model_saved_deployment(metadata, &saved);
	if (parse_deployment(body, 0, &saved, &params, error, error_size) != 0) {
		cJSON_Delete(metadata);
		return -1;
	}
	snprintf(tdl_model_id, sizeof(tdl_model_id), "TDL_MODEL_%s", model_type->valuestring);
	cJSON_Delete(metadata);
	if (task_is_busy()) {
		snprintf(error, error_size, "已有配置任务正在执行");
		return -3;
	}
	if (config_stage_detection_model(tdl_model_id, model_path, config_path,
			params.threshold, params.processing_width, params.processing_height,
			1, revision, error, error_size) != 0)
		return -1;
	task_id = config_task_submit_apply(revision);
	if (task_id == 0) {
		unlink(OVIS_CONFIG_PENDING);
		snprintf(error, error_size, "已有配置任务正在执行");
		return -3;
	}
	if (snprintf(json, size, "{\"task_id\":%lu,\"revision\":\"%s\"}",
			task_id, revision) >= (int)size) {
		snprintf(error, error_size, "模型部署响应过大");
		return -1;
	}
	audit_log("model.activate", "accepted");
	return 0;
}

int model_deactivate(const char *id, char *json, size_t size,
	char *error, size_t error_size)
{
	char directory[512];
	char model_path[512];
	char config_path[512];
	char metadata_path[512];
	char referenced_id[17];
	char revision[17];
	cJSON *metadata;
	unsigned long task_id;

	if (!id_is_valid(id)) {
		snprintf(error, error_size, "模型 ID 无效");
		return -1;
	}
	detection_model_from_config(OVIS_CONFIG_FILE, 0, referenced_id);
	if (strcmp(referenced_id, id) != 0) {
		snprintf(error, error_size, "当前运行配置未引用该模型");
		return -3;
	}
	model_paths(id, directory, sizeof(directory), model_path, sizeof(model_path),
		config_path, sizeof(config_path), metadata_path, sizeof(metadata_path));
	metadata = read_json_file(metadata_path);
	if (metadata == NULL) {
		cJSON_Delete(metadata);
		snprintf(error, error_size, "模型元数据损坏");
		return -1;
	}
	cJSON_Delete(metadata);
	if (task_is_busy()) {
		snprintf(error, error_size, "已有配置任务正在执行");
		return -3;
	}
	if (config_stage_detection_model("TDL_MODEL_YOLOV8N_DET_MONITOR_PERSON",
			"/usr/share/ipcamera/cv184x/"
			"yolov8n_det_monitor_person_256_448_INT8_cv184x.bmodel",
			"/usr/share/ipcamera/model_factory.json", 0.7,
			OVIS_AI_OBJECT_DEFAULT_WIDTH, OVIS_AI_OBJECT_DEFAULT_HEIGHT, 0,
			revision, error, error_size) != 0)
		return -1;
	task_id = config_task_submit_apply(revision);
	if (task_id == 0) {
		unlink(OVIS_CONFIG_PENDING);
		snprintf(error, error_size, "已有配置任务正在执行");
		return -3;
	}
	if (snprintf(json, size, "{\"task_id\":%lu,\"revision\":\"%s\"}",
			task_id, revision) >= (int)size) {
		snprintf(error, error_size, "模型停用响应过大");
		return -1;
	}
	audit_log("model.deactivate", "accepted");
	return 0;
}
