#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Ai_IRFD(const char * file)
{
    APP_PARAM_AI_IR_FD_CFG_S * Ai = app_ipcam_Ai_IR_FD_Param_Get();
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI IR FD config ------------------> start \n");
    char tmp_buff[128] = {0};
    Ai->bEnable          = ini_getl("ai_irfd_config", "enable", 0, file);
    Ai->VpssGrp          = ini_getl("ai_irfd_config", "vpss_grp", 0, file);
    Ai->VpssChn          = ini_getl("ai_irfd_config", "vpss_chn", 0, file);
    Ai->bVpssPreProcSkip = ini_getl("ai_irfd_config", "vpssPreProcSkip", 0, file);
    Ai->attachPoolId     = ini_getl("ai_irfd_config", "attach_pool", -1, file);

    ini_gets("ai_irfd_config", "fd_model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_fd, tmp_buff, 128);
    ini_gets("ai_irfd_config", "ln_model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_ln, tmp_buff, 128);
    ini_gets("ai_irfd_config", "fr_model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_fr, tmp_buff, 128);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI IR FD config ------------------> done \n\n");
    return CVI_SUCCESS;
}
