#ifndef __APP_IPCAM_SYS_H__
#define __APP_IPCAM_SYS_H__

#include "cvi_comm_video.h"
#include "cvi_comm_sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CVI_MAX_BIND_NUM    10

typedef struct _APP_PARAM_SBM_CFG_S {
    CVI_BOOL bEnable;
    CVI_S32 s32SbmGrp;
    CVI_S32 s32SbmChn;
    CVI_U32 s32WrapBufLine;
    CVI_U32 s32WrapBufSize;
} APP_PARAM_SBM_CFG_S;

typedef struct APP_PARAM_VB_CFG_T {
    CVI_BOOL            bEnable;
    uint32_t            size;
    uint32_t            width;
    uint32_t            height;
    PIXEL_FORMAT_E      fmt;
    DATA_BITWIDTH_E     enBitWidth;
    COMPRESS_MODE_E     enCmpMode;
    uint32_t            vb_blk_num;
} APP_PARAM_VB_CFG_S;

#define APP_IPCAM_VB_POOL_MAX_NUM (16)
#define APP_IPCAM_SBM_MAX_NUM (3)

typedef struct APP_PARAM_SYS_CFG_T {
    APP_PARAM_VB_CFG_S vb_pool[APP_IPCAM_VB_POOL_MAX_NUM];
    uint32_t vb_pool_num;
    VI_VPSS_MODE_S stVIVPSSMode;
    CVI_U8 u8SbmCnt;
    APP_PARAM_SBM_CFG_S stSbmCfg[APP_IPCAM_SBM_MAX_NUM];
} APP_PARAM_SYS_CFG_S;

APP_PARAM_SYS_CFG_S *app_ipcam_Sys_Param_Get(void);
int app_ipcam_Sys_EnableFastBoot(void);
int app_ipcam_Sys_Init(void);
int app_ipcam_Sys_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif
