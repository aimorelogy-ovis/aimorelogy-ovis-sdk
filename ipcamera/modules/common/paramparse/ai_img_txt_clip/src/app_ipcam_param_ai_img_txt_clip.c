#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"


int Load_Param_Ai_Img_Txt_Clip(const char * file)
{
    APP_PARAM_AI_IMG_TXT_CLIP_S * Ai = app_ipcam_Ai_Img_Txt_Clip_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI img txt clip config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_img_txt_clip_config");

    Ai->bEnable          = ini_getl(tmp_section, "clip_enable", 0, file);
    Ai->VpssGrp          = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn          = ini_getl(tmp_section, "vpss_chn", 0, file);
    Ai->u32GrpWidth      = ini_getl(tmp_section, "grp_width", 0, file);
    Ai->u32GrpHeight     = ini_getl(tmp_section, "grp_height", 0, file);

    ini_gets(tmp_section, "model_path_img", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_img, tmp_buff, 128);

    ini_gets(tmp_section, "model_path_txt", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_txt, tmp_buff, 128);

    ini_gets(tmp_section, "model_path_cfg", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_cfg, tmp_buff, 128);

    ini_gets(tmp_section, "txt_dir", " ", tmp_buff, 128, file);
    strncpy(Ai->txt_dir, tmp_buff, 128);

    ini_gets(tmp_section, "model_id_img", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_img] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_img] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_img = enum_num;
    }

    ini_gets(tmp_section, "model_id_txt", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_txt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_txt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_txt = enum_num;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_img=%d model_path_img=%s\n",
        Ai->model_id_img, Ai->model_path_img);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id_txt=%d model_path_txt=%s\n",
        Ai->model_id_txt, Ai->model_path_txt);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_path_txt=%s\n", Ai->model_path_cfg);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d txt_dir=%s\n",
        Ai->bEnable, Ai->VpssGrp, Ai->VpssChn, Ai->u32GrpWidth, Ai->u32GrpHeight, Ai->txt_dir);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI image txt clip config ------------------> done \n\n");

    return CVI_SUCCESS;
}
