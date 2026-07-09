#ifndef _CVI_ISPD2_CALLBACK_FUNCS_DUMP_H_
#define _CVI_ISPD2_CALLBACK_FUNCS_DUMP_H_

#include "cvi_ispd2_local.h"

#define ENV_DUMP_ALL_FILE_PATH  "DUMP_FILE_PATH"
#define MAX_AWB_LOG_PATH_LENGTH (128)
#define MAX_LOG_FILENAME_LENGTH (MAX_AWB_LOG_PATH_LENGTH * 2 + 32)
#define FILE_NAME_LENGTH        (MAX_LOG_FILENAME_LENGTH + 20)

CVI_S32          CVI_ISPD2_Dump_Init(TISPDaemon2Info *ptObject);
CVI_S32          CVI_ISPD2_Dump_Start(VI_PIPE ViPipe);
TISPDaemon2Info *CVI_ISPD2_Dump_GetDaemon2Info(void);

#endif
