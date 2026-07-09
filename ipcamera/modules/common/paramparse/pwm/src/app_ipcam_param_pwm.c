#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_pwm.h"

int Load_Param_Pwm(const char * file)
{
    // CVI_U32 GRP, CHN;
    // APP_PROF_LOG_PRINT(LEVEL_INFO, "loading PWM config ------------------> start \n");
    // APP_PARAM_PWM_CFG_S *Pwm = app_ipcam_Pwm_Param_Get();

    // GRP             = ini_getl("pwm_config", "grp", 0, file);
    // CHN             = ini_getl("pwm_config", "chn", 0, file);
    // Pwm->period     = ini_getl("pwm_config", "period", 0, file);
    // Pwm->duty_cycle = ini_getl("pwm_config", "duty_cycle", 0, file);
    // Pwm->enabled    = ini_getl("pwm_config", "enable", 0, file);

    // if ((GRP != 0) && (GRP != 4) && (GRP != 8) && (GRP != 12)) {
    //     APP_PROF_LOG_PRINT(LEVEL_ERROR, "PWM group invalid and Optional ID is [0/4/8/12]\n");
    //     return -1;
    // }

    // if (!(CHN <= 3)) {
    //     APP_PROF_LOG_PRINT(LEVEL_ERROR, "PWM channel invalid and Optional ID is [0~3]\n");
    //     return -1;
    // }
    // Pwm->grp = GRP;
    // Pwm->chn = CHN;

    // APP_PROF_LOG_PRINT(LEVEL_INFO, "GRP=%d  CHN=%d  PERIOD=%d  DUTY_CYCLE=%d  ENABLE=%d\n",Pwm->grp,Pwm->chn,Pwm->period,Pwm->duty_cycle,Pwm->enabled);

    // APP_PROF_LOG_PRINT(LEVEL_INFO, "loading PWM config ------------------> done \n\n");

    return CVI_SUCCESS;
}