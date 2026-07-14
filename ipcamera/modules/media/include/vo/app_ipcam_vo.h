#ifndef __APP_IPCAM_VO_H__
#define __APP_IPCAM_VO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <stdio.h>
#include "cvi_defines.h"
#include "cvi_comm_vo.h"

typedef enum _VO_MODE_E {
    VO_MODE_1MUX,
    VO_MODE_2MUX,
    VO_MODE_4MUX,
    VO_MODE_8MUX,
    VO_MODE_9MUX,
    VO_MODE_16MUX,
    VO_MODE_25MUX,
    VO_MODE_36MUX,
    VO_MODE_49MUX,
    VO_MODE_64MUX,
    VO_MODE_2X4,
    VO_MODE_BUTT
} VO_MODE_E;

typedef struct APP_PARAM_VO_CFG_S {
    VO_DEV s32VoDev;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_MODE_E enVoMode;
    ROTATION_E enRotation;
    CVI_U32 u32DisBufLen;
    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDstChn;
    CVI_BOOL bBindMode;
} APP_PARAM_VO_CFG_T;

CVI_S32 app_ipcam_Vo_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
