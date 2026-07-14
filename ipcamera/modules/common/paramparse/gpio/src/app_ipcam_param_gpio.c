#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_gpio.h"

int Load_Param_Gpio(const char * file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading GPIO config ------------------> start \n");
    APP_PARAM_GPIO_CFG_S *Gpio = app_ipcam_Gpio_Param_Get();

    Gpio->IR_CUT_A = ini_getl("gpio_config", "ir_cut_a", 0, file);
    Gpio->IR_CUT_B = ini_getl("gpio_config", "ir_cut_b", 0, file);
    Gpio->LED_WHITE = ini_getl("gpio_config", "led_white", 0, file);
    Gpio->LED_IR    = ini_getl("gpio_config", "led_ir", 0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "IR_CUT_A GPIO=%d IR_CUT_B GPIO=%d LED_WHITE GPIO=%d LED_RED GPIO=%d\n",Gpio->IR_CUT_A, Gpio->IR_CUT_B,Gpio->LED_WHITE,Gpio->LED_IR);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading GPIO config ------------------> done \n\n");

    return CVI_SUCCESS;
}
