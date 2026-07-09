#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"
#include "tdl_model_def.h"
#include "app_ipcam_ai.h"

int Load_Param_Ai_CRY(const char * file)
{
    APP_PARAM_AI_CRY_CFG_S * Ai = app_ipcam_Ai_Cry_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[16] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI CRY config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_cry_config");

    Ai->bEnable = ini_getl(tmp_section, "cry_enable", 0, file);
    Ai->model_id = ini_getl(tmp_section, "model_id", 0, file);
    Ai->application_scene = ini_getl(tmp_section, "application_scene", 0, file);
    ini_gets(tmp_section, "model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path, tmp_buff, 128);

    ini_gets(tmp_section, "model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Convert string name [%s] to enum number [%d]!\n", tmp_section, str_name, enum_num);
        Ai->model_id = enum_num;
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Ai->model_id, Ai->model_path);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI Cry config ------------------> done \n\n");

    return CVI_SUCCESS;
}
