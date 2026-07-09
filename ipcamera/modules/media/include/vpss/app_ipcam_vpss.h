#ifndef __APP_IPCAM_VPSS_H__
#define __APP_IPCAM_VPSS_H__
#include "cvi_vpss.h"
#include "cvi_comm_video.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vpss.h"
#include "app_ipcam_mq.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define VPSS_GRP_ID_MAX     10
#define CVI_MAX_VPSS_GRP    16

typedef struct APP_VPSS_GRP_CFG_S {
    VPSS_GRP VpssGrp;
    CVI_BOOL bEnable;   /* update by ini file */
    CVI_BOOL bCreate;   /* update by coding */
    VI_DEV   s32BindVidev;  /* get Grp from vi if not set -1*/
    CVI_U8   u8ViRotation; // vi rotation
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CROP_INFO_S stVpssGrpCropInfo;
    CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM];   /* update by ini file */
    CVI_BOOL abChnCreate[VPSS_MAX_PHY_CHN_NUM];   /* update by coding */
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    VPSS_CROP_INFO_S stVpssChnCropInfo[VPSS_MAX_PHY_CHN_NUM];
    CVI_U32 aAttachEn[VPSS_MAX_PHY_CHN_NUM];
    CVI_U32	aAttachPool[VPSS_MAX_PHY_CHN_NUM];
    CVI_BOOL bBindMode;
    MMF_CHN_S astChn[2];
    VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap[VPSS_MAX_PHY_CHN_NUM];
} APP_VPSS_GRP_CFG_T;

typedef struct APP_VPSS_STITCH_PAIR_S {
    VPSS_GRP srcGrp;
    VPSS_CHN srcChn;
    VPSS_GRP dstGrp;
    VPSS_CHN dstChn;
} APP_VPSS_STITCH_PAIR_T;

typedef struct APP_PARAM_VPSS_CFG_S {
    VPSS_MODE_S stVPSSMode;
    CVI_U32 u32GrpCnt;
    APP_VPSS_GRP_CFG_T astVpssGrpCfg[CVI_MAX_VPSS_GRP];
    /* stitch / forward config parsed from ini */
    CVI_BOOL bStitchEnable;
    CVI_U32 u32StitchPairCnt;
    APP_VPSS_STITCH_PAIR_T astStitchPair[CVI_MAX_VPSS_GRP];
} APP_PARAM_VPSS_CFG_T;

APP_PARAM_VPSS_CFG_T *app_ipcam_Vpss_Param_Get(void);
int app_ipcam_Vpss_Init(void);
int app_ipcam_Vpss_DeInit(void);
int app_ipcam_Vpss_Create(VPSS_GRP VpssGrp);
int app_ipcam_Vpss_Destroy(VPSS_GRP VpssGrp);
int app_ipcam_Vpss_Bind(VPSS_GRP VpssGrp);
int app_ipcam_Vpss_Unbind(VPSS_GRP VpssGrp);
int app_ipcam_CmdTask_Rotate_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);

#ifdef __cplusplus
}
#endif

#endif
