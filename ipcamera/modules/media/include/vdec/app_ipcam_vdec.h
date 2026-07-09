#ifndef __APP_IPCAM_VDEC_H__
#define __APP_IPCAM_VDEC_H__

#include "cvi_type.h"
#include <cvi_common.h>
#include "cvi_comm_vdec.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define VDEC_CHN_MAX 4

typedef struct APP_VDEC_CHN_CFG_T {
    CVI_BOOL    bEnable;
    VDEC_CHN    VdecChn;
    CVI_U32     u32Width;
    CVI_U32     u32Height;
    CVI_CHAR    decode_file_name[128];
    RECT_S      astDispRect;
    VDEC_CHN_ATTR_S astChnAttr;
    VDEC_CHN_PARAM_S astChnParam;

} APP_VDEC_CHN_CFG_S;

typedef struct APP_PARAM_VDEC_CTX_T {
    CVI_BOOL    bInit;
    CVI_S32     s32VdecChnCnt;
    VB_POOL     PicVbPool;
    pthread_t   send_to_vdec_thread;
    pthread_t   send_to_vpss_thread;
    CVI_BOOL    thread_enable_flag;
    CVI_BOOL    bBindMode;
    MMF_CHN_S   astChn[2];
    VPSS_GRP    VpssGrp;
    VPSS_CHN    VpssChn;
    APP_VDEC_CHN_CFG_S astVdecChnCfg;

} APP_PARAM_VDEC_CFG_S;

APP_PARAM_VDEC_CFG_S *app_ipcam_Vdec_Param_Get(void);
APP_VDEC_CHN_CFG_S *app_ipcam_VdecChnCfg_Get(void);

int app_ipcam_Vdec_Start(void);
int app_ipcam_Vdec_Stop(void);
int app_ipcam_Vdec_Init(void);

#ifdef __cplusplus
}
#endif

#endif
