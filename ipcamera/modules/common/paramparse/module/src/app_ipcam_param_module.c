#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Module(const char * file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading module config ------------------> start \n");
    APP_PARAM_MODULE_CFG_S *Module = app_ipcam_Module_Param_Get();

    Module->alios_sys_mode = ini_getl("module_config", "alios_sys_mode", 0, file);
    Module->alios_vi_mode = ini_getl("module_config", "alios_vi_mode", 0, file);
    Module->alios_vpss_mode = ini_getl("module_config", "alios_vpss_mode", 0, file);
    Module->alios_venc_mode = ini_getl("module_config", "alios_venc_mode", 0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO, \
                    "alios_sys_mode:%d, alios_vi_mode:%d, alios_vpss_mode:%d, alios_venc_mode:%d\n", \
                    Module->alios_sys_mode, Module->alios_vi_mode, \
                    Module->alios_vpss_mode, Module->alios_venc_mode);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading module config ------------------> done \n\n");
    return CVI_SUCCESS;
}