#ifndef __APP_IPCAM_TEAISP_BNR_H__
#define __APP_IPCAM_TEAISP_BNR_H__

#include "cvi_common.h"
#include "cvi_comm_vi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_TEAISP_BNR_MODEL_PATH \
    "/usr/share/ipcamera/cv184x/bmodel_0326_blc1_patch2.bmodel"

CVI_BOOL app_ipcam_TeaispBnr_IsEnabled(VI_PIPE ViPipe);
CVI_BOOL app_ipcam_TeaispBnr_IsCertified(void);
CVI_S32 app_ipcam_TeaispBnr_ValidateConfiguration(void);
CVI_S32 app_ipcam_TeaispBnr_ValidateAiExclusion(void);
CVI_S32 app_ipcam_TeaispBnr_DriverInit(VI_PIPE ViPipe);
CVI_S32 app_ipcam_TeaispBnr_ValidatePq(VI_PIPE ViPipe);
CVI_S32 app_ipcam_TeaispBnr_LoadModel(VI_PIPE ViPipe);
CVI_S32 app_ipcam_TeaispBnr_DriverDeInit(VI_PIPE ViPipe);

#ifdef __cplusplus
}
#endif

#endif
