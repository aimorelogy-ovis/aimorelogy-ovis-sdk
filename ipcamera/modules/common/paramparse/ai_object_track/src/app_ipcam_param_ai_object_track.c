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
    float legacy_tracking_score_threshold = 0.12f;
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
    Ai->SotVpssGrp              = ini_getl(tmp_section, "sot_vpss_grp", Ai->VpssGrp, file);
    Ai->SotVpssChn              = ini_getl(tmp_section, "sot_vpss_chn", Ai->VpssChn, file);
    Ai->u32SotGrpWidth          = ini_getl(tmp_section, "sot_grp_width", Ai->u32GrpWidth, file);
    Ai->u32SotGrpHeight         = ini_getl(tmp_section, "sot_grp_height", Ai->u32GrpHeight, file);
    Ai->bSotRefineSelectedDet   = ini_getl(tmp_section, "sot_refine_selected_det", 0, file);
    Ai->threshold_occluded      = ini_getf(tmp_section, "threshold_occluded", 0.1, file);
    Ai->threshold_reappear      = ini_getf(tmp_section, "threshold_reappear", 2.0, file);
    Ai->search_type             = ini_getl(tmp_section, "search_type", 3, file);
    ini_gets(tmp_section, "selection_mode", "point", tmp_buff, sizeof(tmp_buff), file);
    if (strcmp(tmp_buff, "point") != 0 && strcmp(tmp_buff, "reticle") != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "invalid tracking selection_mode: %s\n", tmp_buff);
        return CVI_FAILURE;
    }
    Ai->bSelectAtReticle = strcmp(tmp_buff, "reticle") == 0;
    ini_gets(tmp_section, "initial_box_mode", "target", tmp_buff, sizeof(tmp_buff), file);
    if (strcmp(tmp_buff, "target") != 0 && strcmp(tmp_buff, "fixed_80") != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "invalid tracking initial_box_mode: %s\n", tmp_buff);
        return CVI_FAILURE;
    }
    Ai->bFixedInitBox = strcmp(tmp_buff, "fixed_80") == 0;
    Ai->use_kalman              = ini_getl(tmp_section, "use_kalman", 1, file);
    Ai->sot_gmc_enable          = ini_getl(tmp_section, "sot_gmc_enable", 0, file);
    Ai->sot_gmc_interval        = ini_getl(tmp_section, "sot_gmc_interval", 4, file);
    if (Ai->sot_gmc_interval < 1 || Ai->sot_gmc_interval > 8) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "[%s][sot_gmc_interval] invalid value %u, fallback to 4\n",
            tmp_section, Ai->sot_gmc_interval);
        Ai->sot_gmc_interval = 4;
    }
    legacy_tracking_score_threshold = ini_getf(
        tmp_section, "tracking_score_threshold", 0.12, file);
    Ai->sot_min_observed_score = ini_getf(
        tmp_section, "sot_min_observed_score",
        legacy_tracking_score_threshold, file);
    if (Ai->sot_min_observed_score < 0.0f || Ai->sot_min_observed_score > 1.0f) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "[%s][sot_min_observed_score] invalid value %.3f, fallback to 0.12\n",
            tmp_section, Ai->sot_min_observed_score);
        Ai->sot_min_observed_score = 0.12f;
    }
    Ai->debug_log_enable        = ini_getl(tmp_section, "debug_log_enable", 0, file);

    ini_gets(tmp_section, "model_id_sot", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_sot] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_sot] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_sot = enum_num;
    }

    ini_gets(tmp_section, "model_path_sot", " ", tmp_buff, 128, file);
    app_ipcam_Param_CopyString(Ai->model_path_sot, sizeof(Ai->model_path_sot), tmp_buff);

    ini_gets(tmp_section, "model_path_sam", " ", tmp_buff, 128, file);
    app_ipcam_Param_CopyString(Ai->model_path_sam, sizeof(Ai->model_path_sam), tmp_buff);

    ini_gets(tmp_section, "model_path_cfg", " ", tmp_buff, 128, file);
    app_ipcam_Param_CopyString(Ai->model_path_cfg, sizeof(Ai->model_path_cfg), tmp_buff);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI object track config ------------------> done \n\n");

    return CVI_SUCCESS;
}
