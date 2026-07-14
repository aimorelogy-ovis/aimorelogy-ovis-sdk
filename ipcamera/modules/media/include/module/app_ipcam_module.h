#ifndef __APP_IPCAM_MODULE_H__
#define __APP_IPCAM_MODULE_H__

#include "cvi_comm_video.h"
#include "cvi_comm_sys.h"

typedef struct APP_PARAM_MODULE_CFG_T {
    CVI_S32 alios_sys_mode;
    CVI_S32 alios_vi_mode;
    CVI_S32 alios_vpss_mode;
    CVI_S32 alios_venc_mode;
} APP_PARAM_MODULE_CFG_S;

APP_PARAM_MODULE_CFG_S *app_ipcam_Module_Param_Get(void);

#endif