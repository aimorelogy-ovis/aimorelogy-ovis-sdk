#pragma once

typedef struct _REPLAY_PARAM {
	CVI_U8 pixelFormat;
	CVI_U16 width;
	CVI_U16 height;
	CVI_BOOL timingEnable;
	CVI_BOOL enableTEAISPBnr;
	CVI_U32 frameRate;
	CVI_U8 WDRMode;
	CVI_U8 bayerFormat;
	COMPRESS_MODE_E compressMode;
} REPLAY_PARAM;

CVI_S32 set_replay(REPLAY_PARAM *pstParamPtr, const CVI_CHAR *pszJsonPath);
