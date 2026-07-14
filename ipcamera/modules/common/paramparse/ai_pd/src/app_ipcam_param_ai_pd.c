#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_PD(const char * file)
{
    APP_PARAM_AI_PD_CFG_S *Ai = app_ipcam_Ai_PD_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[16] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI PD config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_pd_config");

    Ai->bEnable           = ini_getl(tmp_section, "pd_enable", 0, file);
    Ai->VpssGrp           = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn           = ini_getl(tmp_section, "vpss_chn", 0, file);
    Ai->u32GrpWidth       = ini_getl(tmp_section, "grp_width", 0, file);
    Ai->u32GrpHeight      = ini_getl(tmp_section, "grp_height", 0, file);
    Ai->threshold         = ini_getf(tmp_section, "threshold", 0, file);
    ini_gets(tmp_section, "model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path, tmp_buff, 128);

    ini_gets(tmp_section, "model_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id = enum_num;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "pd_enable=%d vpss_grp=%d vpss_chn=%d GrpW=%d GrpH=%d threshold=%f\n", \
        Ai->bEnable, Ai->VpssGrp, Ai->VpssChn, Ai->u32GrpWidth, Ai->u32GrpHeight, Ai->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s \n", Ai->model_id, Ai->model_path);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI PD config ------------------> done \n\n");

    return CVI_SUCCESS;
}
