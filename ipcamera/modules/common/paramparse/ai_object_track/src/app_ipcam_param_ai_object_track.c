#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_OBJECT_TRACK(const char * file)
{
    APP_PARAM_AI_OBJECT_TRACK_CFG_S *Ai = app_ipcam_Ai_Object_Track_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[32] = {0};
    char tmp_buff[128] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI object track config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_object_track_config");

    Ai->bEnable                 = ini_getl(tmp_section, "object_track_enable", 0, file);
    Ai->VpssGrp                 = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn                 = ini_getl(tmp_section, "vpss_chn", 0, file);
    Ai->u32GrpWidth             = ini_getl(tmp_section, "grp_width", 0, file);
    Ai->u32GrpHeight            = ini_getl(tmp_section, "grp_height", 0, file);
    Ai->threshold_occluded      = ini_getf(tmp_section, "threshold_occluded", 0.1, file);
    Ai->threshold_reappear      = ini_getf(tmp_section, "threshold_reappear", 2.0, file);
    Ai->search_type             = ini_getl(tmp_section, "search_type", 2, file);
    Ai->use_kalman              = ini_getl(tmp_section, "use_kalman", 1, file);
    Ai->tracking_score_threshold = ini_getf(tmp_section, "tracking_score_threshold", 0.5, file);
    Ai->debug_log_enable        = ini_getl(tmp_section, "debug_log_enable", 0, file);

    ini_gets(tmp_section, "model_id_det", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_det] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_det] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_det = enum_num;
    }

    ini_gets(tmp_section, "model_id_sot", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_sot] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_sot] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_sot = enum_num;
    }

    ini_gets(tmp_section, "model_path_det", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_det, tmp_buff, 128);

    ini_gets(tmp_section, "model_path_sot", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_sot, tmp_buff, 128);

    ini_gets(tmp_section, "model_path_sam", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_sam, tmp_buff, 128);

    ini_gets(tmp_section, "model_path_cfg", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_cfg, tmp_buff, 128);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI object track config ------------------> done \n\n");

    return CVI_SUCCESS;
}
