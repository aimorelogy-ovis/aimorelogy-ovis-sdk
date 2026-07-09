#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"


int Load_Param_Ai_FD(const char * file)
{
    APP_PARAM_AI_FD_CFG_S * Ai = app_ipcam_Ai_FD_Param_Get();
    int enum_num = 0;
    int ret = 0;
    char tmp_section[16] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** ai_supported_model = app_ipcam_Param_get_ai_supported_model();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI FD config ------------------> start \n");

    char tmp_buff[128] = {0};

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "ai_fd_config");

    Ai->FD_bEnable       = ini_getl(tmp_section, "fd_enable", 0, file);
    Ai->VpssGrp          = ini_getl(tmp_section, "vpss_grp", 0, file);
    Ai->VpssChn          = ini_getl(tmp_section, "vpss_chn", 0, file);
    Ai->u32GrpWidth      = ini_getl(tmp_section, "grp_width", 0, file);
    Ai->u32GrpHeight     = ini_getl(tmp_section, "grp_height", 0, file);

    Ai->threshold_fd     = ini_getf(tmp_section, "threshold_fd", 0, file);

    ini_gets(tmp_section, "model_path_fd", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_fd, tmp_buff, 128);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Ai->model_id_fd, Ai->model_path_fd);

    ini_gets(tmp_section, "model_id_fd", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, ai_supported_model, TDL_MODEL_MAX, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_fd] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][model_id_fd] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Ai->model_id_fd = enum_num;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d threshold=%f\n",
        Ai->FD_bEnable, Ai->VpssGrp, Ai->VpssChn, Ai->u32GrpWidth, Ai->u32GrpHeight, Ai->threshold_fd);


    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI FD config ------------------> done \n\n");

    return CVI_SUCCESS;
}
