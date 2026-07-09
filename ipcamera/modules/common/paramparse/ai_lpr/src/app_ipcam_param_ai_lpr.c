#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_LPR(const char * file)
{
    APP_PARAM_AI_LPR_CFG_S *Ai = app_ipcam_Ai_LPR_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[40] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI LPR config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_lpr_config");

    Ai->bEnable           = ini_getl(tmp_section, "lpr_enable", 0, file);
    Ai->VpssGrp           = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn           = ini_getl(tmp_section, "vpss_chn", 1, file);
    Ai->u32GrpWidth       = ini_getl(tmp_section, "grp_width", 640, file);
    Ai->u32GrpHeight      = ini_getl(tmp_section, "grp_height", 384, file);
    Ai->threshold         = ini_getf(tmp_section, "threshold", 0.5, file);

    /* 检测模型配置 */
    ini_gets(tmp_section, "model_path_det", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_det, tmp_buff, 128);
    memset(tmp_buff, 0, sizeof(tmp_buff));

    ini_gets(tmp_section, "model_id_det", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_det] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_det] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_det = enum_num;
    }

    /* 关键点模型配置 */
    ini_gets(tmp_section, "model_path_kp", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_kp, tmp_buff, 128);
    memset(tmp_buff, 0, sizeof(tmp_buff));

    memset(str_name, 0, sizeof(str_name));
    ini_gets(tmp_section, "model_id_kp", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_kp] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_kp] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_kp = enum_num;
    }

    /* 识别模型配置 */
    ini_gets(tmp_section, "model_path_rec", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_rec, tmp_buff, 128);

    memset(str_name, 0, sizeof(str_name));
    ini_gets(tmp_section, "model_id_rec", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_rec] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_rec] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_rec = enum_num;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "lpr_enable=%d vpss_grp=%d vpss_chn=%d GrpW=%d GrpH=%d threshold=%f\n",
        Ai->bEnable, Ai->VpssGrp, Ai->VpssChn, Ai->u32GrpWidth, Ai->u32GrpHeight, Ai->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_det=%d model_path_det=%s\n", Ai->model_id_det, Ai->model_path_det);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_kp=%d model_path_kp=%s\n", Ai->model_id_kp, Ai->model_path_kp);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_rec=%d model_path_rec=%s\n", Ai->model_id_rec, Ai->model_path_rec);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI LPR config ------------------> done \n\n");

    return CVI_SUCCESS;
}