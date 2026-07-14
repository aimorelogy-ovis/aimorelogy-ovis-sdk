#include "app_ipcam_module.h"
#include <stdlib.h>

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_MODULE_CFG_S g_stModuleCfg = {0};
APP_PARAM_MODULE_CFG_S * g_pstModuleCfg = &g_stModuleCfg;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
APP_PARAM_MODULE_CFG_S *app_ipcam_Module_Param_Get(void)
{
    return g_pstModuleCfg;
}