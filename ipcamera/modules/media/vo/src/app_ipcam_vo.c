#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#include "cvi_defines.h"
#include "cvi_math.h"
#include "cvi_comm_mipi_tx.h"
#include "cvi_comm_video.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_vo.h"


#define VO_SOURCE_FLAG "/tmp/vdec"

static pthread_t g_pthVo;
static CVI_BOOL g_vo_thread_created;

CVI_S32 app_ipcam_Vo_Start_Preproc(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Start_Postproc(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Stop_Preproc(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Stop_Postproc(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Dev_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Dev_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Layer_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Layer_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Chn_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_Chn_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg);
CVI_S32 app_ipcam_Vo_LayerDispWHFrm_Get(const VO_INTF_SYNC_E* const penIntfSync
            , CVI_U32* const pu32W, CVI_U32* const pu32H, CVI_U32* const pu32Frm);

static CVI_VOID *pfunThreadVo(CVI_VOID *pvArg);
static CVI_BOOL thread_enable_flag;

CVI_S32 app_ipcam_Vo_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);
    thread_enable_flag = true;
    APP_CHK_RET(app_ipcam_Vo_Start_Preproc(pstVoCfg), "app_ipcam_Vo_Start_Preproc");
    APP_CHK_RET(app_ipcam_Vo_Dev_Start(pstVoCfg), "app_ipcam_Vo_Dev_Start");

    APP_FUNC_RET_CALLBACK(app_ipcam_Vo_Layer_Start(pstVoCfg), {}, {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[app_ipcam_Vo_Layer_Start] didn't return success!\n");

        APP_CHK_RET(app_ipcam_Vo_Dev_Stop(pstVoCfg), "app_ipcam_Vo_Dev_Stop");

        return CVI_FAILURE;
    });

    APP_FUNC_RET_CALLBACK(app_ipcam_Vo_Chn_Start(pstVoCfg), {}, {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "[app_ipcam_Vo_Chn_Start] didn't return success!\n");

        APP_CHK_RET(app_ipcam_Vo_Layer_Stop(pstVoCfg), "app_ipcam_Vo_Layer_Stop");
        APP_CHK_RET(app_ipcam_Vo_Dev_Stop(pstVoCfg), "app_ipcam_Vo_Dev_Stop");

        return CVI_FAILURE;
    });

    APP_CHK_RET(app_ipcam_Vo_Start_Postproc(pstVoCfg), "app_ipcam_Vo_Start_Postproc");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    APP_CHK_RET(app_ipcam_Vo_Stop_Preproc(pstVoCfg), "app_ipcam_Vo_Stop_Preproc");
    APP_CHK_RET(app_ipcam_Vo_Chn_Stop(pstVoCfg), "app_ipcam_Vo_Chn_Stop");
    APP_CHK_RET(app_ipcam_Vo_Layer_Stop(pstVoCfg), "app_ipcam_Vo_Layer_Stop");
    APP_CHK_RET(app_ipcam_Vo_Dev_Stop(pstVoCfg), "app_ipcam_Vo_Dev_Stop");
    APP_CHK_RET(app_ipcam_Vo_Stop_Postproc(pstVoCfg), "app_ipcam_Vo_Stop_Postproc");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Start_Preproc(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    if (pstVoCfg->bBindMode) {
        APP_CHK_RET(CVI_SYS_Bind(&pstVoCfg->stSrcChn, &pstVoCfg->stDstChn), "CVI_SYS_Bind");
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Start_Postproc(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    if (pstVoCfg->enRotation != ROTATION_0) {
        APP_CHK_RET(CVI_VO_SetChnRotation(pstVoCfg->stDstChn.s32DevId, pstVoCfg->stDstChn.s32ChnId
            , pstVoCfg->enRotation), "CVI_VO_SetChnRotation");
    }

    if (pstVoCfg->bBindMode != 1){
        if (pthread_create(&g_pthVo, CVI_NULL, pfunThreadVo, (CVI_VOID*)pstVoCfg) == 0) {
            g_vo_thread_created = CVI_TRUE;
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Thread VO is created successfully.\n");
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Thread VO is created failed.\n");
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Stop_Preproc(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);
    thread_enable_flag = false;
    if (pstVoCfg->bBindMode != 1 && g_vo_thread_created) {
        pthread_join(g_pthVo, CVI_NULL);
        g_vo_thread_created = CVI_FALSE;
    }
    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Stop_Postproc(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    if (pstVoCfg->bBindMode) {
        APP_CHK_RET(CVI_SYS_UnBind(&pstVoCfg->stSrcChn, &pstVoCfg->stDstChn), "CVI_SYS_UnBind");
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Dev_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    APP_CHK_RET(CVI_VO_SetPubAttr(pstVoCfg->s32VoDev, &pstVoCfg->stVoPubAttr), "CVI_VO_SetPubAttr");
    APP_CHK_RET(CVI_VO_Enable(pstVoCfg->s32VoDev), "CVI_VO_Enable");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Dev_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    APP_CHK_RET(CVI_VO_Disable(pstVoCfg->s32VoDev), "CVI_VO_Disable");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Layer_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    if (pstVoCfg->u32DisBufLen < 3) {
        APP_PROF_LOG_PRINT(LEVEL_WARN
            , "The display buffer length of VO is less than 3, and it will be set to default value.\n");
    } else {
        APP_CHK_RET(CVI_VO_SetDisplayBufLen(pstVoCfg->stDstChn.s32DevId, pstVoCfg->u32DisBufLen)
            , "CVI_VO_SetDisplayBufLen");
    }

    APP_CHK_RET(CVI_VO_SetVideoLayerAttr(pstVoCfg->stDstChn.s32DevId, &pstVoCfg->stLayerAttr)
        , "CVI_VO_SetVideoLayerAttr");
    APP_CHK_RET(CVI_VO_EnableVideoLayer(pstVoCfg->stDstChn.s32DevId), "CVI_VO_EnableVideoLayer");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Layer_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    APP_CHK_RET(CVI_VO_DisableVideoLayer(pstVoCfg->stDstChn.s32DevId), "CVI_VO_DisableVideoLayer");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Chn_Start(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    CVI_U32 u32Width = pstVoCfg->stLayerAttr.stImageSize.u32Width;
    CVI_U32 u32Height = pstVoCfg->stLayerAttr.stImageSize.u32Height;
    CVI_U32 u32WndNum = 0;
    CVI_U32 u32Square = 0;
    CVI_U32 u32Row = 0;
    CVI_U32 u32Col = 0;
    VO_CHN_ATTR_S stChnAttr = {.u32Priority = 0, .stRect = {.s32X = 0, .s32Y = 0, .u32Width = u32Width
        , .u32Height = u32Height}};

    switch (pstVoCfg->enVoMode) {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            u32Square = 1;
            break;

        case VO_MODE_2MUX:
            u32WndNum = 2;
            u32Square = 2;
            break;

        case VO_MODE_4MUX:
            u32WndNum = 4;
            u32Square = 2;
            break;

        case VO_MODE_8MUX:
            u32WndNum = 8;
            u32Square = 3;
            break;

        case VO_MODE_9MUX:
            u32WndNum = 9;
            u32Square = 3;
            break;

        case VO_MODE_16MUX:
            u32WndNum = 16;
            u32Square = 4;
            break;

        case VO_MODE_25MUX:
            u32WndNum = 25;
            u32Square = 5;
            break;

        case VO_MODE_36MUX:
            u32WndNum = 36;
            u32Square = 6;
            break;

        case VO_MODE_49MUX:
            u32WndNum = 49;
            u32Square = 7;
            break;

        case VO_MODE_64MUX:
            u32WndNum = 64;
            u32Square = 8;
            break;

        case VO_MODE_2X4:
            u32WndNum = 8;
            u32Square = 3;
            u32Row = 4;
            u32Col = 2;
            break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Unsupported VO mode!\n");

            return CVI_FAILURE;
            break;
    }

    APP_PROF_LOG_PRINT(
        LEVEL_INFO,
        "[%s] u32Width: %d, u32Height: %d, u32WndNum: %d, u32Square: %d.\n",
        __FUNCTION__, u32Width, u32Height, u32WndNum, u32Square
    );

    for (CVI_U32 i = 0; i < u32WndNum; i++) {
        if (
            pstVoCfg->enVoMode == VO_MODE_1MUX || pstVoCfg->enVoMode == VO_MODE_2MUX ||
            pstVoCfg->enVoMode == VO_MODE_4MUX || pstVoCfg->enVoMode == VO_MODE_8MUX ||
            pstVoCfg->enVoMode == VO_MODE_9MUX || pstVoCfg->enVoMode == VO_MODE_16MUX ||
            pstVoCfg->enVoMode == VO_MODE_25MUX || pstVoCfg->enVoMode == VO_MODE_36MUX ||
            pstVoCfg->enVoMode == VO_MODE_49MUX || pstVoCfg->enVoMode == VO_MODE_64MUX
        ) {
            stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
            stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
            stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Square, 2);
            stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Square, 2);
            stChnAttr.u32Priority = 0;
        } else if (pstVoCfg->enVoMode == VO_MODE_2X4) {
            stChnAttr.stRect.s32X = ALIGN_DOWN((u32Width / u32Col) * (i % u32Col), 2);
            stChnAttr.stRect.s32Y = ALIGN_DOWN((u32Height / u32Row) * (i / u32Col), 2);
            stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Col, 2);
            stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Row, 2);
            stChnAttr.u32Priority = 0;
        }

        APP_CHK_RET(CVI_VO_SetChnAttr(pstVoCfg->stDstChn.s32DevId, i, &stChnAttr), "CVI_VO_SetChnAttr");
        APP_CHK_RET(CVI_VO_EnableChn(pstVoCfg->stDstChn.s32DevId, i), "CVI_VO_EnableChn");
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_Chn_Stop(const APP_PARAM_VO_CFG_T* const pstVoCfg)
{
    _NULL_POINTER_CHECK_(pstVoCfg, CVI_FAILURE);

    CVI_U32 u32WndNum = 0;

    switch (pstVoCfg->enVoMode) {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            break;

        case VO_MODE_2MUX:
            u32WndNum = 2;
            break;

        case VO_MODE_4MUX:
            u32WndNum = 4;
            break;

        case VO_MODE_8MUX:
            u32WndNum = 8;
            break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Unsupported VO mode!\n");

            return CVI_FAILURE;
            break;
    }

    for (CVI_U32 i = 0; i < u32WndNum; i++) {
        APP_CHK_RET(CVI_VO_ClearChnBuf(pstVoCfg->stDstChn.s32DevId, i, CVI_TRUE), "CVI_VO_ClearChnBuf");
        APP_CHK_RET(CVI_VO_DisableChn(pstVoCfg->stDstChn.s32DevId, i), "CVI_VO_DisableChn");
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Vo_LayerDispWHFrm_Get(const VO_INTF_SYNC_E* const penIntfSync, CVI_U32* const pu32W
            , CVI_U32* const pu32H, CVI_U32* const pu32Frm)
{
    _NULL_POINTER_CHECK_(penIntfSync, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pu32W, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pu32H, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pu32Frm, CVI_FAILURE);

    switch (*penIntfSync) {
        case VO_OUTPUT_PAL:
            *pu32W = 720;
            *pu32H = 576;
            *pu32Frm = 25;
            break;

        case VO_OUTPUT_NTSC:
            *pu32W = 720;
            *pu32H = 480;
            *pu32Frm = 30;
            break;

        case VO_OUTPUT_1080P24:
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 24;
            break;

        case VO_OUTPUT_1080P25:
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 25;
            break;

        case VO_OUTPUT_1080P30:
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 30;
            break;

        case VO_OUTPUT_720P50:
            *pu32W = 1280;
            *pu32H = 720;
            *pu32Frm = 50;
            break;

        case VO_OUTPUT_720P60:
            *pu32W = 1280;
            *pu32H = 720;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1080P50:
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 50;
            break;

        case VO_OUTPUT_1080P60:
            *pu32W = 1920;
            *pu32H = 1080;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_576P50:
            *pu32W = 720;
            *pu32H = 576;
            *pu32Frm = 50;
            break;

        case VO_OUTPUT_480P60:
            *pu32W = 720;
            *pu32H = 480;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_800x600_60:
            *pu32W = 800;
            *pu32H = 600;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1024x768_60:
            *pu32W = 1024;
            *pu32H = 768;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1280x1024_60:
            *pu32W = 1280;
            *pu32H = 1024;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1366x768_60:
            *pu32W = 1366;
            *pu32H = 768;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1440x900_60:
            *pu32W = 1440;
            *pu32H = 900;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1280x800_60:
            *pu32W = 1280;
            *pu32H = 800;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1600x1200_60:
            *pu32W = 11600;
            *pu32H = 1200;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1680x1050_60:
            *pu32W = 1680;
            *pu32H = 1050;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1920x1200_60:
            *pu32W = 1920;
            *pu32H = 1200;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_640x480_60:
            *pu32W = 640;
            *pu32H = 480;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_720x1280_60:
            *pu32W = 720;
            *pu32H = 1280;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_1080x1920_60:
            *pu32W = 1080;
            *pu32H = 1920;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_480x800_60:
            *pu32W = 480;
            *pu32H = 800;
            *pu32Frm = 60;
            break;

        case VO_OUTPUT_USER:
            *pu32W = 720;
            *pu32H = 576;
            *pu32Frm = 25;
            break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Unsupported VO interface synchronization!\n");

            return CVI_FAILURE;
            break;
    }

    return CVI_SUCCESS;
}

static CVI_VOID *pfunThreadVo(CVI_VOID *pvArg)
{
    const APP_PARAM_VO_CFG_T* const pstVoCfg = (APP_PARAM_VO_CFG_T*)pvArg;
    VIDEO_FRAME_INFO_S stVoFrame = {0};

    while (thread_enable_flag) {
        usleep(1000);

        if (access(VO_SOURCE_FLAG, F_OK) == 0) {
            // If the VO_SOURCE_FLAG exists, the VO will play video through VDEC.
            // View the relevant code in the VDEC module.
        } else {
            APP_FUNC_RET_CALLBACK(CVI_VPSS_GetChnFrame(pstVoCfg->stSrcChn.s32DevId, pstVoCfg->stSrcChn.s32ChnId
                , &stVoFrame, 3000), {}, {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "CVI_VPSS_GetChnFrame didn't return success!\n");
                continue;
            });

            APP_FUNC_RET_CALLBACK(CVI_VO_SendFrame(pstVoCfg->stDstChn.s32DevId, pstVoCfg->stDstChn.s32ChnId
                , &stVoFrame, 3000), {}, {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "CVI_VO_SendFrame didn't return success!\n");
            });

            APP_FUNC_RET_CALLBACK(CVI_VPSS_ReleaseChnFrame(pstVoCfg->stSrcChn.s32DevId, pstVoCfg->stSrcChn.s32ChnId
                , &stVoFrame), {}, {
                APP_PROF_LOG_PRINT(LEVEL_WARN, "CVI_VPSS_ReleaseChnFrame didn't return success!\n");
            });

        }
    }

    return CVI_NULL;
}
