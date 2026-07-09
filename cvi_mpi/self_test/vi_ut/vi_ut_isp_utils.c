#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvi_json.h"
#include <cvi_comm_video.h>

#include "vi_ut_isp_utils.h"

#define GET_RP_KEY_VAL_INT(json_obj, key, param)\
{\
	if (cvi_json_object_object_get_ex(json_obj, key, &valJsonObject)) {\
		param = cvi_json_object_get_int(valJsonObject);\
	} \
} \

#define GET_RP_KEY_VAL_STR(json_obj, key, param)\
{\
	if (cvi_json_object_object_get_ex(json_obj, key, &valJsonObject)) {\
		param = cvi_json_object_get_string(valJsonObject);\
	} \
} \

static CVI_S32 get_json_object_from_file(const CVI_U8 *jsonPath, struct cvi_json_object **jsonObj)
{
	FILE *filePtr = fopen(jsonPath, "r");
	CVI_U64 fileSize;
	CVI_U8 *bufferPtr;

	if (!filePtr) {
		fprintf(stderr, "Error opening file.\n");
		return -1;
	}

	fseek(filePtr, 0, SEEK_END);

	fileSize = ftell(filePtr);

	fseek(filePtr, 0, SEEK_SET);

	bufferPtr = (CVI_U8 *)malloc(fileSize + 1);

	if (!bufferPtr) {
		fprintf(stderr, "Memory allocation error.\n");
		fclose(filePtr);
		return -1;
	}

	fread(bufferPtr, 1, fileSize, filePtr);
	fclose(filePtr);
	bufferPtr[fileSize] = '\0';

	*jsonObj = cvi_json_tokener_parse(bufferPtr);

	if (*jsonObj == NULL) {
		fprintf(stderr, "Error parsing JSON.\n");
		free(bufferPtr);
		return -1;
	}

	free(bufferPtr);

	return 0;
}

static COMPRESS_MODE_E get_compress_mode(const CVI_U8 *mode)
{
	if (mode == NULL)
		return COMPRESS_MODE_NONE;
	if (strcmp(mode, "none") == 0) {
		return COMPRESS_MODE_NONE;
	} else if (strcmp(mode, "tile") == 0) {
		return COMPRESS_MODE_TILE;
	} else if (strcmp(mode, "line") == 0) {
		return COMPRESS_MODE_LINE;
	} else if (strcmp(mode, "frame") == 0) {
		return COMPRESS_MODE_FRAME;
	} else {
		return COMPRESS_MODE_NONE;
	}
}

CVI_S32 set_replay(REPLAY_PARAM *paramPtr, const CVI_U8 *jsonPath)
{
	struct cvi_json_object *jsonObj = NULL;
	struct cvi_json_object *valJsonObject = cvi_json_object_new_object();
	const CVI_U8 *modeStr = NULL;

	if (paramPtr == NULL || jsonPath == NULL)
		return -1;

	paramPtr->enableTEAISPBnr = false;

	if (get_json_object_from_file(jsonPath, &jsonObj) != 0) {
		UT_PRT("fail to parse replay config, use default param!");
		paramPtr->pixelFormat = 0;
		paramPtr->width = 2560;
		paramPtr->height = 1440;
		paramPtr->timingEnable = false;
		paramPtr->frameRate = 25;
		paramPtr->WDRMode = 0;
		paramPtr->bayerFormat = 0;
		paramPtr->compressMode = COMPRESS_MODE_NONE;
	} else {
		GET_RP_KEY_VAL_INT(jsonObj, "PixelFormat", paramPtr->pixelFormat);
		GET_RP_KEY_VAL_INT(jsonObj, "width", paramPtr->width);
		GET_RP_KEY_VAL_INT(jsonObj, "height", paramPtr->height);
		GET_RP_KEY_VAL_INT(jsonObj, "TimingEnable", paramPtr->timingEnable);
		GET_RP_KEY_VAL_INT(jsonObj, "FrameRate", paramPtr->frameRate);
		GET_RP_KEY_VAL_INT(jsonObj, "WDRMode", paramPtr->WDRMode);
		GET_RP_KEY_VAL_INT(jsonObj, "BayerFormat", paramPtr->bayerFormat);
		GET_RP_KEY_VAL_STR(jsonObj, "CompressMode", modeStr);

		paramPtr->compressMode = get_compress_mode(modeStr);

		UT_PRT("PixelFormat:%d\n", paramPtr->pixelFormat);
		UT_PRT("width:%d\n", paramPtr->width);
		UT_PRT("height:%d\n", paramPtr->height);
		UT_PRT("TimingEnable:%d\n", paramPtr->timingEnable);
		UT_PRT("FrameRate:%d\n", paramPtr->frameRate);
		UT_PRT("WDRMode:%d\n", paramPtr->WDRMode);
		UT_PRT("BayerFormat:%d\n", paramPtr->bayerFormat);
		UT_PRT("CompressMode:%s\n", modeStr);
	}
	cvi_json_object_put(jsonObj);

	return 0;
}
