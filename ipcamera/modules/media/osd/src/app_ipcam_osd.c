
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <time.h>
#include <math.h>
#include "cvi_errno.h"
#include "cvi_comm_video.h"
#include "app_ipcam_osd.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_loadbmp.h"
#include "app_ipcam_fontmod.h"
#include "app_ipcam_vpss.h"
#include "errno.h"
#ifdef AI_SUPPORT
#include "app_ipcam_ai.h"
#endif
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define OSD_LIB_FONT_W      24
#define OSD_LIB_FONT_H      24

#define NOASCII_CHARACTER_BYTES 2
#define BYTE_BITS               8
#define ISASCII(a)              (((a) >= 0x00 && (a) <= 0x7F) ? 1 : 0)
#define APP_OSDC_REFRESH_US      100000
#define APP_OSDC_FIRST_CANVAS_TIMEOUT_MS 500
#define APP_OSDC_FIRST_CANVAS_RETRY_US 10000
#define APP_OSDC_PD_RECT_HANDLE (RGN_MAX_NUM - 1)
#define APP_OSDC_PD_RECT_LAYER  1
#define APP_OSDC_PD_IDLE_REFRESH_US 1000000
#define APP_OSDC_STYLE_THICKNESS_MIN 1
#define APP_OSDC_STYLE_THICKNESS_MAX 4
#define APP_OSDC_PI 3.14159265358979323846f

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef enum APP_AI_RECT_SHOW_T {
    APP_AI_ALL_RECT_HIDDEN = 0x0,
    APP_AI_PD_RECT_SHOW    = 0x01,
    APP_AI_MD_RECT_SHOW    = 0x02,
    APP_AI_FD_RECT_SHOW    = 0x04,
    APP_AI_TRACK_RECT_SHOW = 0x08,
    APP_AI_ALL_RECT_SHOW   = 0x0F
} APP_AI_RECT_SHOW_E;

typedef struct APP_OSDC_CANVAS_CFG_T {
    CVI_BOOL createCanvas;
    CVI_U16 drawFlag;
    BITMAP_S rgnBitmap;
} APP_OSDC_CANVAS_CFG_S;

#ifdef AI_SUPPORT
typedef struct APP_OSDC_AI_RECT_RATIO_T {
    CVI_S32 VpssChn_W;
    CVI_S32 VpssChn_H;
    float ScaleX;
    float ScaleY;
} APP_OSDC_AI_RECT_RATIO_S;
#endif
/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_OSDC_CFG_S g_stOsdcCfg, *g_pstOsdcCfg = &g_stOsdcCfg;

static CVI_BOOL g_bOsdcThreadRun;
static pthread_t g_pthOsdcRgn;
static pthread_mutex_t OsdcMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_OsdcWakeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_OsdcWakeCond = PTHREAD_COND_INITIALIZER;
static CVI_U64 g_u64OsdcWakeGeneration;
static CVI_BOOL g_abOsdcCanvasReady[OSDC_NUM_MAX];

#ifdef PD_SUPPORT
static CVI_BOOL g_bOsdcPdRectThreadRun;
static CVI_BOOL g_bOsdcPdRectThreadReady;
static CVI_BOOL g_bOsdcPdRectRegionReady;
static pthread_t g_pthOsdcPdRect;
static pthread_mutex_t g_OsdcPdRectLifecycleMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_OsdcPdRectMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_OsdcPdRectCond = PTHREAD_COND_INITIALIZER;
static CVI_U64 g_u64OsdcPdRectGeneration;
static MMF_CHN_S g_stOsdcPdRectChn = {0};
static CVI_U32 g_u32OsdcPdRectWidth;
static CVI_U32 g_u32OsdcPdRectHeight;
static CVI_U32 g_u32OsdcPdRectCompressedSize;
static PIXEL_FORMAT_E g_enOsdcPdRectFormat;
#endif

#ifdef OBJECT_TRACK_SUPPORT
typedef struct APP_OSDC_TRACK_RECT_STATE_T {
    CVI_BOOL bConfigEnabled;
    CVI_BOOL bStyleEnabled;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    CVI_U32 u32OutputWidth;
    CVI_U32 u32OutputHeight;
    CVI_BOOL bShow;
    CVI_BOOL bLost;
    CVI_FLOAT fX1;
    CVI_FLOAT fY1;
    CVI_FLOAT fX2;
    CVI_FLOAT fY2;
    CVI_U32 u32SourceWidth;
    CVI_U32 u32SourceHeight;
    CVI_U32 u32Color;
    CVI_U32 u32LostColor;
    CVI_U32 u32Thickness;
    CVI_U64 u64Generation;
} APP_OSDC_TRACK_RECT_STATE_S;

static CVI_BOOL g_bOsdcTrackRectThreadRun;
static pthread_t g_pthOsdcTrackRect;
static pthread_mutex_t g_OsdcTrackRectMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_OsdcTrackRectCond = PTHREAD_COND_INITIALIZER;
static APP_OSDC_TRACK_RECT_STATE_S g_stOsdcTrackRectState = {0};
#endif

static APP_OSDC_CANVAS_CFG_S g_stOsdcCanvasCfg = {0};
// static OSDC_DRAW_OBJ_S g_ObjsVec[OSDC_OBJS_MAX] = {0};
static APP_OSDC_OBJS_AI_STR_INFO_S g_objStrAi = {0};
#ifdef PD_SUPPORT
static APP_OSDC_OBJS_AI_STR_INFO_S g_objStrPd = {0};
#endif

#ifdef AI_SUPPORT
APP_OSDC_AI_RECT_RATIO_S g_stPdRectRatio = {0};
APP_OSDC_AI_RECT_RATIO_S g_stMdRectRatio = {0};
APP_OSDC_AI_RECT_RATIO_S g_stFdRectRatio = {0};
APP_OSDC_AI_RECT_RATIO_S g_stHumanKeypointRectRatio = {0};
APP_OSDC_AI_RECT_RATIO_S g_stObjectTrackRectRatio = {0};

#ifdef PD_SUPPORT
static TDLObject g_objMetaPd = {0};
#endif
#ifdef MD_SUPPORT
static TDLObject g_objMetaMd = {0};
#endif
#ifdef FACE_SUPPORT
static TDLFace g_objMetaFd = {0};
#endif
#ifdef HUMAN_KEYPOINT_SUPPORT
static TDLObject g_objMetaHumanKeypoint = {0};
#endif
#ifdef OBJECT_TRACK_SUPPORT
static TDLObject g_objMetaObjectTrack = {0};
#endif
#endif

#ifdef WEB_SOCKET
static APP_OSDC_OBJS_INFO_S g_stOsdcPrivacy[4];
static APP_OSDC_OBJS_INFO_S *g_pstOsdcPrivacy = &g_stOsdcPrivacy[0];
#endif
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/


#ifdef WEB_SOCKET
APP_OSDC_OBJS_INFO_S *app_ipcam_OsdcPrivacy_Param_Get(void)
{
    return g_pstOsdcPrivacy;
}
#endif

static CVI_U32 app_ipcam_Osdc_Thickness(CVI_U32 u32Thickness)
{
    if (u32Thickness < APP_OSDC_STYLE_THICKNESS_MIN) {
        return APP_OSDC_STYLE_THICKNESS_MIN;
    }
    if (u32Thickness > APP_OSDC_STYLE_THICKNESS_MAX) {
        return APP_OSDC_STYLE_THICKNESS_MAX;
    }
    return u32Thickness;
}

static CVI_U32 app_ipcam_Osdc_RgbToArgb1555(CVI_U32 u32Rgb)
{
    CVI_U32 u32Red = (u32Rgb >> 16) & 0xff;
    CVI_U32 u32Green = (u32Rgb >> 8) & 0xff;
    CVI_U32 u32Blue = u32Rgb & 0xff;

    return 0x8000 | ((u32Red >> 3) << 10) |
        ((u32Green >> 3) << 5) | (u32Blue >> 3);
}

static CVI_U32 app_ipcam_Osdc_ColorToArgb1555(CVI_U32 u32Color)
{
    if (u32Color <= 0xffff && (u32Color & 0x8000) != 0) {
        return u32Color;
    }
    return app_ipcam_Osdc_RgbToArgb1555(u32Color);
}

static CVI_U32 app_ipcam_Osdc_ModelColor(CVI_S32 s32ClassId)
{
    static const CVI_U32 au32Palette[] = {
        0x00D9FF, 0xFFB000, 0x5CE65C, 0xFF5C8A,
        0xB980FF, 0x4D8DFF, 0xFFD84D, 0x4DE1B8
    };
    CVI_U32 u32Index = s32ClassId >= 0 ? (CVI_U32)s32ClassId : 0;

    return au32Palette[u32Index %
        (sizeof(au32Palette) / sizeof(au32Palette[0]))];
}

static CVI_U32 app_ipcam_Osdc_DetectionColor(
    const APP_OSD_STYLE_CFG_S *pstStyle, CVI_S32 s32ClassId)
{
    CVI_U32 u32Rgb = pstStyle->u32DetectionColor;

    if (pstStyle->enDetectionColorMode == APP_OSD_COLOR_MODE_MODEL) {
        u32Rgb = app_ipcam_Osdc_ModelColor(s32ClassId);
    }
    return app_ipcam_Osdc_RgbToArgb1555(u32Rgb);
}

static CVI_VOID app_ipcam_Osdc_Wake(CVI_VOID)
{
    pthread_mutex_lock(&g_OsdcWakeMutex);
    g_u64OsdcWakeGeneration++;
    pthread_cond_signal(&g_OsdcWakeCond);
    pthread_mutex_unlock(&g_OsdcWakeMutex);
}

#ifdef OBJECT_TRACK_SUPPORT
static CVI_VOID app_ipcam_Osdc_ObjectTrackRect_ConfigUpdate(
    const APP_PARAM_OSDC_CFG_S *pstOsdcCfg)
{
    CVI_BOOL bEnabled = CVI_FALSE;
    VPSS_GRP VpssGrp = VPSS_INVALID_GRP;
    VPSS_CHN VpssChn = VPSS_INVALID_CHN;
    CVI_U32 u32Width = 0;
    CVI_U32 u32Height = 0;

    if (pstOsdcCfg != NULL && pstOsdcCfg->enable &&
        pstOsdcCfg->bShow[0] && pstOsdcCfg->bShowTrackRect[0] &&
        pstOsdcCfg->mmfChn[0].enModId == CVI_ID_VPSS &&
        pstOsdcCfg->mmfChn[0].s32DevId >= 0 &&
        pstOsdcCfg->mmfChn[0].s32DevId < CVI_MAX_VPSS_GRP &&
        pstOsdcCfg->mmfChn[0].s32ChnId >= 0 &&
        pstOsdcCfg->mmfChn[0].s32ChnId < VPSS_MAX_PHY_CHN_NUM) {
        APP_VPSS_GRP_CFG_T *pstVpssCfg = &app_ipcam_Vpss_Param_Get()->
            astVpssGrpCfg[pstOsdcCfg->mmfChn[0].s32DevId];

        VpssGrp = pstOsdcCfg->mmfChn[0].s32DevId;
        VpssChn = pstOsdcCfg->mmfChn[0].s32ChnId;
        u32Width = pstVpssCfg->astVpssChnAttr[VpssChn].u32Width;
        u32Height = pstVpssCfg->astVpssChnAttr[VpssChn].u32Height;
        bEnabled = u32Width > 0 && u32Height > 0;
    }

    pthread_mutex_lock(&g_OsdcTrackRectMutex);
    g_stOsdcTrackRectState.bConfigEnabled = bEnabled;
    g_stOsdcTrackRectState.bStyleEnabled =
        pstOsdcCfg != NULL && pstOsdcCfg->stStyle.bTrackingEnable;
    g_stOsdcTrackRectState.VpssGrp = VpssGrp;
    g_stOsdcTrackRectState.VpssChn = VpssChn;
    g_stOsdcTrackRectState.u32OutputWidth = u32Width;
    g_stOsdcTrackRectState.u32OutputHeight = u32Height;
    g_stOsdcTrackRectState.u32Color = pstOsdcCfg != NULL ?
        pstOsdcCfg->stStyle.u32TrackingColor : APP_OSD_COLOR_AMBER_RGB;
    g_stOsdcTrackRectState.u32LostColor = pstOsdcCfg != NULL ?
        pstOsdcCfg->stStyle.u32TrackingLostColor : APP_OSD_COLOR_LOST_RGB;
    g_stOsdcTrackRectState.u32Thickness = pstOsdcCfg != NULL ?
        app_ipcam_Osdc_Thickness(pstOsdcCfg->stStyle.u32TrackingThickness) : 3;
    g_stOsdcTrackRectState.u64Generation++;
    pthread_cond_signal(&g_OsdcTrackRectCond);
    pthread_mutex_unlock(&g_OsdcTrackRectMutex);
}

CVI_VOID app_ipcam_Osdc_ObjectTrackRect_Publish(
    CVI_BOOL bShow, CVI_BOOL bLost, CVI_FLOAT fX1, CVI_FLOAT fY1,
    CVI_FLOAT fX2, CVI_FLOAT fY2,
    CVI_U32 u32SourceWidth, CVI_U32 u32SourceHeight)
{
    pthread_mutex_lock(&g_OsdcTrackRectMutex);
    g_stOsdcTrackRectState.bShow = bShow;
    g_stOsdcTrackRectState.bLost = bLost;
    g_stOsdcTrackRectState.fX1 = fX1;
    g_stOsdcTrackRectState.fY1 = fY1;
    g_stOsdcTrackRectState.fX2 = fX2;
    g_stOsdcTrackRectState.fY2 = fY2;
    g_stOsdcTrackRectState.u32SourceWidth = u32SourceWidth;
    g_stOsdcTrackRectState.u32SourceHeight = u32SourceHeight;
    g_stOsdcTrackRectState.u64Generation++;
    pthread_cond_signal(&g_OsdcTrackRectCond);
    pthread_mutex_unlock(&g_OsdcTrackRectMutex);
}
#endif

#ifdef PD_SUPPORT
static CVI_BOOL app_ipcam_Osdc_PdRect_UnifiedReady(CVI_VOID)
{
    return g_stOsdcCanvasCfg.createCanvas && g_pstOsdcCfg->enable &&
        g_pstOsdcCfg->bShow[0] &&
        g_pstOsdcCfg->bShowPdRect[0] &&
        g_pstOsdcCfg->mmfChn[0].enModId == CVI_ID_VPSS &&
        g_pstOsdcCfg->mmfChn[0].s32DevId >= 0 &&
        g_pstOsdcCfg->mmfChn[0].s32DevId < CVI_MAX_VPSS_GRP &&
        g_pstOsdcCfg->mmfChn[0].s32ChnId >= 0 &&
        g_pstOsdcCfg->mmfChn[0].s32ChnId < VPSS_MAX_PHY_CHN_NUM;
}

CVI_VOID app_ipcam_Osdc_PdRect_Publish(CVI_VOID)
{
    pthread_mutex_lock(&g_OsdcPdRectMutex);
    if (g_bOsdcPdRectThreadRun) {
        g_u64OsdcPdRectGeneration++;
        pthread_cond_signal(&g_OsdcPdRectCond);
    }
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    app_ipcam_Osdc_Wake();
}

#endif

static CVI_S32 GetNonASCNum(char *string, CVI_S32 len)
{
    CVI_S32 i;
    CVI_S32 n = 0;

    for (i = 0; i < len; i++) {
        if (string[i] == '\0')
            break;
        if (!ISASCII(string[i])) {
            i++;
            n++;
        }
    }

    return n;
}

static CVI_S32 GetFontMod(char *Character, uint8_t **FontMod, CVI_S32 *FontModLen)
{
    CVI_U32 offset = 0;
    CVI_U32 areacode = 0;
    CVI_U32 bitcode = 0;

    if (ISASCII(Character[0])) {
        areacode = 3;
        bitcode = (CVI_U32)((uint8_t)Character[0] - 0x20);
    } else {
        areacode = (CVI_U32)((uint8_t)Character[0] - 0xA0);
        bitcode = (CVI_U32)((uint8_t)Character[1] - 0xA0);
    }
    offset = (94 * (areacode - 1) + (bitcode - 1)) * (OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8);
    *FontMod = (uint8_t *)g_fontLib + offset;
    *FontModLen = OSD_LIB_FONT_W*OSD_LIB_FONT_H / 8;
    return CVI_SUCCESS;
}

static CVI_VOID GetTimeStr(const struct tm *pstTime, char *pazStr, CVI_S32 s32MaxLen)
{
    time_t nowTime;
    struct tm stTime = {0};

    if (!pstTime) {
        time(&nowTime);
        localtime_r(&nowTime, &stTime);
        pstTime = &stTime;
    }

    snprintf(pazStr, s32MaxLen, "%04d-%02d-%02d %02d:%02d:%02d",
        pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday,
        pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
}

// #ifdef AI_SUPPORT
// static CVI_VOID GetDebugStr(char *pazStr, CVI_S32 s32MaxLen)
// {
//     if (NULL == pazStr || 0 >= s32MaxLen) {
//         APP_PROF_LOG_PRINT(LEVEL_ERROR, "szStr is NULL or s32MaxLen:%d invalid!\n", s32MaxLen);
//         return ;
//     }

// #ifdef FACE_SUPPORT
//     snprintf(pazStr, s32MaxLen, "MD[%02dFPS]PD[%02dFPS]FD[%02dFPS]",
//             app_ipcam_Ai_MD_ProcFps_Get(), app_ipcam_Ai_PD_ProcFps_Get(), app_ipcam_Ai_FD_ProcFps_Get());
// #else
//     snprintf(pazStr, s32MaxLen, "MD[%02dFPS]PD[%02dFPS]",
//             app_ipcam_Ai_MD_ProcFps_Get(), app_ipcam_Ai_PD_ProcFps_Get());
// #endif
// }
// #endif

#ifdef AI_SUPPORT
#ifdef OBJECT_TRACK_SUPPORT
static int app_ipcam_Osd_Ai_Bitmap_Update(char *szStr, BITMAP_S *pstBitmap, CVI_U32 color)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    if (NULL == szStr || NULL == pstBitmap)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "szStr/pstBitmap is NULL\n");
        return s32Ret;
    }
    CVI_U32 u32CanvasWidth, u32CanvasHeight, u32BgColor, u32Color;
    SIZE_S stFontSize;
    CVI_S32 s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
    CVI_S32 NonASCNum = GetNonASCNum(szStr, s32StrLen);

    u32CanvasWidth = OSD_LIB_FONT_W * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
    u32CanvasHeight = OSD_LIB_FONT_H;
    stFontSize.u32Width = OSD_LIB_FONT_W;
    stFontSize.u32Height = OSD_LIB_FONT_H;
    u32BgColor = 0x7fff;
    u32Color = color;

    if (szStr == NULL) {
       APP_PROF_LOG_PRINT(LEVEL_ERROR, "szStr NULL pointer!\n");
       return CVI_FAILURE;
    }

    uint16_t *puBmData = (uint16_t *)pstBitmap->pData;
    CVI_U32 u32BmRow, u32BmCol;

    for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
        CVI_S32 NonASCShow = 0;

        for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
            CVI_S32 s32BmDataIdx = u32BmRow * pstBitmap->u32Width + u32BmCol;
            CVI_S32 s32CharIdx = u32BmCol / stFontSize.u32Width;
            CVI_S32 s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);

            if (NonASCNum > 0 && s32CharIdx > 0) {
                NonASCShow = GetNonASCNum(szStr, s32StringIdx);
                s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
            }
            CVI_S32 s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
                            stFontSize.u32Width;
            CVI_S32 s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
            CVI_S32 s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
            CVI_S32 s32BitOffset = s32CharCol % BYTE_BITS;
            uint8_t *FontMod = NULL;
            CVI_S32 FontModLen = 0;

            if (GetFontMod(&szStr[s32StringIdx], &FontMod, &FontModLen) == CVI_SUCCESS) {
                if (FontMod != NULL && s32HexOffset < FontModLen) {
                    uint8_t temp = FontMod[s32HexOffset];

                    if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1)
                        puBmData[s32BmDataIdx] = (uint16_t)u32Color;
                    else
                        puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
                    continue;
                }
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "GetFontMod Fail\n");
            return CVI_FAILURE;
        }
    }

    return s32Ret;
}

static int app_ipcam_ObjsRectInfo_Add_AiStr(
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr, CVI_U32 OsdcObjsNum,
    TDLObject *ai_obj, CVI_U32 ai_num,
    APP_OSD_LABEL_MODE_E enLabelMode, CVI_U32 u32Color)
{
    CVI_S32 s32Ret = 0;
    BITMAP_S stBitmap;
    CVI_S32 s32StrLen = 0;
    char *pszStr = NULL;
    char szStr[APP_OSD_STR_LEN_MAX];
    CVI_S32 s32DataLen = 0;

    memset(&stBitmap, 0, sizeof(BITMAP_S));
    memset(szStr, 0, sizeof(szStr));
    pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_BIT_MAP;
    const char *pszName = ai_obj->info[ai_num].name[0] != '\0' ?
        ai_obj->info[ai_num].name : "object";
    if (enLabelMode == APP_OSD_LABEL_MODE_CLASS_SCORE) {
        snprintf(szStr, APP_OSD_STR_LEN_MAX, "%.31s %.2f",
            pszName, ai_obj->info[ai_num].score);
    } else {
        snprintf(szStr, APP_OSD_STR_LEN_MAX, "%.31s", pszName);
    }
    pszStr = szStr;
    //s32StrLen = strlen(pszStr) - GetNonASCNum(pszStr, strlen(pszStr));
    s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
    stBitmap.u32Width = OSD_LIB_FONT_W * s32StrLen;
    stBitmap.u32Height = OSD_LIB_FONT_H;
    // APP_PROF_LOG_PRINT(LEVEL_ERROR, "wyx check hd name:%s!\n",pszStr);

    s32DataLen = 2 * (stBitmap.u32Width) * (stBitmap.u32Height);
    if (s32DataLen == 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "s32DataLen invalid!\n");
        return -1;
    }

    stBitmap.pData = malloc(s32DataLen);
    if (stBitmap.pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memroy err!\n");
        return -1;
    }
    memset(stBitmap.pData, 0, s32DataLen);
    s32Ret = app_ipcam_Osd_Ai_Bitmap_Update(pszStr, &stBitmap, u32Color);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Ai_Bitmap_Update failed!\n");
        free(stBitmap.pData);
        return -1;
    }

    if (s32DataLen > g_objStrAi.maxlen[ai_num]) {
        if(g_objStrAi.maxlen[ai_num]){
            CVI_SYS_IonFree(g_objStrAi.u64BitmapPhyAddr[ai_num], g_objStrAi.pBitmapVirAddr[ai_num]);
            g_objStrAi.u64BitmapPhyAddr[ai_num] = (CVI_U64)0;
            g_objStrAi.pBitmapVirAddr[ai_num] = NULL;
        }
        s32Ret = CVI_SYS_IonAlloc(&g_objStrAi.u64BitmapPhyAddr[ai_num], (CVI_VOID **)&g_objStrAi.pBitmapVirAddr[ai_num],
            "rgn_cmpr_bitmap2", s32DataLen);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
            free(stBitmap.pData);
            return -1;
        }
        g_objStrAi.maxlen[ai_num] = s32DataLen;
   }
    memcpy(g_objStrAi.pBitmapVirAddr[ai_num], stBitmap.pData, s32DataLen);
    pstObjAttr[OsdcObjsNum].stBitmap.stRect.s32X =
        (int)(g_stObjectTrackRectRatio.ScaleX * ai_obj->info[ai_num].box.x1);
    pstObjAttr[OsdcObjsNum].stBitmap.stRect.s32Y = fmax(0,
        (int)(g_stObjectTrackRectRatio.ScaleY *
            ai_obj->info[ai_num].box.y1 - OSD_LIB_FONT_H));
    pstObjAttr[OsdcObjsNum].stBitmap.stRect.u32Width = stBitmap.u32Width;
    pstObjAttr[OsdcObjsNum].stBitmap.stRect.u32Height = stBitmap.u32Height;
    pstObjAttr[OsdcObjsNum].stBitmap.u64BitmapPAddr = (CVI_U32)g_objStrAi.u64BitmapPhyAddr[ai_num];
    free(stBitmap.pData);
    return CVI_SUCCESS;
}

static CVI_BOOL app_ipcam_Osd_ObjectTrack_HitCenterBox(const TDLObject *pstAiObj, const int32_t box[4])
{
    CVI_U32 i = 0;

    if (pstAiObj == NULL || pstAiObj->info == NULL || pstAiObj->size == 0 || box == NULL) {
        return CVI_FALSE;
    }

    for (i = 0; i < pstAiObj->size; i++) {
        float center_x = (pstAiObj->info[i].box.x1 + pstAiObj->info[i].box.x2) / 2.0f;
        float center_y = (pstAiObj->info[i].box.y1 + pstAiObj->info[i].box.y2) / 2.0f;

        if (center_x >= box[0] && center_x <= box[2] &&
            center_y >= box[1] && center_y <= box[3]) {
            return CVI_TRUE;
        }
    }

    return CVI_FALSE;
}

static CVI_S32 app_ipcam_Osd_ObjectTrack_ReticleSegment_Add(
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr, CVI_U32 *pu32OsdcObjsNum,
    CVI_S32 s32X, CVI_S32 s32Y, CVI_U32 u32Width, CVI_U32 u32Height,
    CVI_U32 u32Color)
{
    RGN_CMPR_OBJ_ATTR_S *pstSegment;

    if (*pu32OsdcObjsNum >= OSDC_OBJS_MAX ||
        u32Width == 0 || u32Height == 0) {
        return CVI_FAILURE;
    }
    pstSegment = &pstObjAttr[*pu32OsdcObjsNum];
    pstSegment->enObjType = RGN_CMPR_RECT;
    pstSegment->stRgnRect.stRect.s32X = s32X;
    pstSegment->stRgnRect.stRect.s32Y = s32Y;
    pstSegment->stRgnRect.stRect.u32Width = u32Width;
    pstSegment->stRgnRect.stRect.u32Height = u32Height;
    pstSegment->stRgnRect.u32Thick = 1;
    pstSegment->stRgnRect.u32Color = u32Color;
    pstSegment->stRgnRect.u32IsFill = CVI_TRUE;
    (*pu32OsdcObjsNum)++;
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Osd_ObjectTrack_ReticleCorners_Add(
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr, CVI_U32 *pu32OsdcObjsNum,
    CVI_S32 s32X, CVI_S32 s32Y, CVI_U32 u32Width, CVI_U32 u32Height,
    CVI_U32 u32Thick, CVI_U32 u32Color)
{
    CVI_U32 u32Arm = fmax(u32Thick * 3,
        fmin(u32Width, u32Height) / 4);
    CVI_S32 s32Right = s32X + u32Width - u32Thick;
    CVI_S32 s32Bottom = s32Y + u32Height - u32Thick;

#define ADD_RETICLE_SEGMENT(x, y, w, h) \
    do { \
        if (app_ipcam_Osd_ObjectTrack_ReticleSegment_Add( \
                pstObjAttr, pu32OsdcObjsNum, (x), (y), (w), (h), \
                u32Color) != CVI_SUCCESS) { \
            return CVI_FAILURE; \
        } \
    } while (0)

    ADD_RETICLE_SEGMENT(s32X, s32Y, u32Arm, u32Thick);
    ADD_RETICLE_SEGMENT(s32X, s32Y, u32Thick, u32Arm);
    ADD_RETICLE_SEGMENT(s32Right - u32Arm + u32Thick, s32Y,
        u32Arm, u32Thick);
    ADD_RETICLE_SEGMENT(s32Right, s32Y, u32Thick, u32Arm);
    ADD_RETICLE_SEGMENT(s32X, s32Bottom, u32Arm, u32Thick);
    ADD_RETICLE_SEGMENT(s32X, s32Bottom - u32Arm + u32Thick,
        u32Thick, u32Arm);
    ADD_RETICLE_SEGMENT(s32Right - u32Arm + u32Thick, s32Bottom,
        u32Arm, u32Thick);
    ADD_RETICLE_SEGMENT(s32Right, s32Bottom - u32Arm + u32Thick,
        u32Thick, u32Arm);
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Osd_ObjectTrack_ReticleCross_Add(
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr, CVI_U32 *pu32OsdcObjsNum,
    CVI_S32 s32X, CVI_S32 s32Y, CVI_U32 u32Width, CVI_U32 u32Height,
    CVI_U32 u32Thick, CVI_U32 u32Color, CVI_BOOL bDot)
{
    CVI_S32 s32CenterX = s32X + u32Width / 2;
    CVI_S32 s32CenterY = s32Y + u32Height / 2;
    CVI_U32 u32Arm = fmax(u32Thick * 3,
        fmin(u32Width, u32Height) / 5);
    CVI_U32 u32Gap = u32Thick * 2;

    ADD_RETICLE_SEGMENT(s32CenterX - u32Gap - u32Arm,
        s32CenterY - u32Thick / 2, u32Arm, u32Thick);
    ADD_RETICLE_SEGMENT(s32CenterX + u32Gap,
        s32CenterY - u32Thick / 2, u32Arm, u32Thick);
    ADD_RETICLE_SEGMENT(s32CenterX - u32Thick / 2,
        s32CenterY - u32Gap - u32Arm, u32Thick, u32Arm);
    ADD_RETICLE_SEGMENT(s32CenterX - u32Thick / 2,
        s32CenterY + u32Gap, u32Thick, u32Arm);
    if (bDot) {
        ADD_RETICLE_SEGMENT(s32CenterX - u32Thick,
            s32CenterY - u32Thick, u32Thick * 2, u32Thick * 2);
    }
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Osd_ObjectTrack_ReticleCircle_Add(
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr, CVI_U32 *pu32OsdcObjsNum,
    CVI_S32 s32X, CVI_S32 s32Y, CVI_U32 u32Width, CVI_U32 u32Height,
    CVI_U32 u32Thick, CVI_U32 u32Color)
{
    CVI_S32 s32CenterX = s32X + u32Width / 2;
    CVI_S32 s32CenterY = s32Y + u32Height / 2;
    CVI_FLOAT fRadiusX = u32Width / 2.0f;
    CVI_FLOAT fRadiusY = u32Height / 2.0f;
    CVI_U32 u32DotSize = u32Thick * 2;
    CVI_U32 i;

    for (i = 0; i < 16; i++) {
        CVI_FLOAT fAngle = 2.0f * APP_OSDC_PI * i / 16.0f;
        CVI_S32 s32DotX = lroundf(s32CenterX + cosf(fAngle) *
            (fRadiusX - u32DotSize / 2.0f) - u32DotSize / 2.0f);
        CVI_S32 s32DotY = lroundf(s32CenterY + sinf(fAngle) *
            (fRadiusY - u32DotSize / 2.0f) - u32DotSize / 2.0f);

        ADD_RETICLE_SEGMENT(s32DotX, s32DotY, u32DotSize, u32DotSize);
    }
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Osd_ObjectTrack_CenterBox_Add(
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr,
    CVI_U32 *pu32OsdcObjsNum,
    const TDLObject *pstAiObj,
    const APP_OSD_STYLE_CFG_S *pstStyle)
{
    int32_t box[4] = {0};
    CVI_U32 u32Color;
    CVI_U32 u32Thick;
    CVI_S32 s32X;
    CVI_S32 s32Y;
    CVI_U32 u32Width;
    CVI_U32 u32Height;
    APP_PARAM_OBJECT_TRACK_MODE mode = app_ipcam_Ai_Object_Track_Mode_Get();

    if (pstObjAttr == NULL || pu32OsdcObjsNum == NULL ||
        pstStyle == NULL || !pstStyle->bReticleEnable ||
        (app_ipcam_Ai_Object_Track_ProcStatus_Get() &&
         mode == TRACKING && !pstStyle->bReticleShowWhileTracking)) {
        return CVI_SUCCESS;
    }
    if (*pu32OsdcObjsNum >= OSDC_OBJS_MAX) {
        return CVI_FAILURE;
    }
    app_ipcam_Ai_Object_Track_DefaultBox_Get(box);
    if (app_ipcam_Ai_Object_Track_ProcStatus_Get() && mode == TRACKING) {
        u32Color = app_ipcam_Osdc_RgbToArgb1555(
            pstStyle->u32TrackingColor);
    } else if (app_ipcam_Osd_ObjectTrack_HitCenterBox(pstAiObj, box)) {
        u32Color = app_ipcam_Osdc_RgbToArgb1555(
            pstStyle->u32ReticleReadyColor);
    } else {
        u32Color = app_ipcam_Osdc_RgbToArgb1555(
            pstStyle->u32ReticleIdleColor);
    }
    u32Thick = app_ipcam_Osdc_Thickness(pstStyle->u32ReticleThickness);
    s32X = lroundf(g_stObjectTrackRectRatio.ScaleX * box[0]);
    s32Y = lroundf(g_stObjectTrackRectRatio.ScaleY * box[1]);
    u32Width = lroundf(g_stObjectTrackRectRatio.ScaleX *
        (box[2] - box[0]));
    u32Height = lroundf(g_stObjectTrackRectRatio.ScaleY *
        (box[3] - box[1]));

    if (u32Width < u32Thick * 4 || u32Height < u32Thick * 4) {
        return CVI_FAILURE;
    }
    switch (pstStyle->enReticleTemplate) {
        case APP_OSD_RETICLE_RECTANGLE:
            pstObjAttr[*pu32OsdcObjsNum].enObjType = RGN_CMPR_RECT;
            pstObjAttr[*pu32OsdcObjsNum].stRgnRect.stRect.s32X = s32X;
            pstObjAttr[*pu32OsdcObjsNum].stRgnRect.stRect.s32Y = s32Y;
            pstObjAttr[*pu32OsdcObjsNum].stRgnRect.stRect.u32Width = u32Width;
            pstObjAttr[*pu32OsdcObjsNum].stRgnRect.stRect.u32Height = u32Height;
            pstObjAttr[*pu32OsdcObjsNum].stRgnRect.u32Thick = u32Thick;
            pstObjAttr[*pu32OsdcObjsNum].stRgnRect.u32Color = u32Color;
            pstObjAttr[*pu32OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
            (*pu32OsdcObjsNum)++;
            return CVI_SUCCESS;
        case APP_OSD_RETICLE_CORNERS:
            return app_ipcam_Osd_ObjectTrack_ReticleCorners_Add(
                pstObjAttr, pu32OsdcObjsNum, s32X, s32Y,
                u32Width, u32Height, u32Thick, u32Color);
        case APP_OSD_RETICLE_CROSSHAIR:
            return app_ipcam_Osd_ObjectTrack_ReticleCross_Add(
                pstObjAttr, pu32OsdcObjsNum, s32X, s32Y,
                u32Width, u32Height, u32Thick, u32Color, CVI_FALSE);
        case APP_OSD_RETICLE_CROSSHAIR_DOT:
            return app_ipcam_Osd_ObjectTrack_ReticleCross_Add(
                pstObjAttr, pu32OsdcObjsNum, s32X, s32Y,
                u32Width, u32Height, u32Thick, u32Color, CVI_TRUE);
        case APP_OSD_RETICLE_BRACKET_CROSS:
            if (app_ipcam_Osd_ObjectTrack_ReticleCorners_Add(
                    pstObjAttr, pu32OsdcObjsNum, s32X, s32Y,
                    u32Width, u32Height, u32Thick, u32Color) != CVI_SUCCESS) {
                return CVI_FAILURE;
            }
            return app_ipcam_Osd_ObjectTrack_ReticleCross_Add(
                pstObjAttr, pu32OsdcObjsNum, s32X, s32Y,
                u32Width, u32Height, u32Thick, u32Color, CVI_FALSE);
        case APP_OSD_RETICLE_CIRCLE:
            return app_ipcam_Osd_ObjectTrack_ReticleCircle_Add(
                pstObjAttr, pu32OsdcObjsNum, s32X, s32Y,
                u32Width, u32Height, u32Thick, u32Color);
        default:
            return CVI_FAILURE;
    }
}
#undef ADD_RETICLE_SEGMENT
#endif
#endif

static int app_ipcam_Osd_Bitmap_Update(
    char *szStr, BITMAP_S *pstBitmap, CVI_U32 u32TextColor)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    if (NULL == szStr || NULL == pstBitmap)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "szStr/pstBitmap is NULL\n");
        return s32Ret;
    }
    CVI_U32 u32CanvasWidth, u32CanvasHeight, u32BgColor, u32Color;
    SIZE_S stFontSize;
    CVI_S32 s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
    CVI_S32 NonASCNum = GetNonASCNum(szStr, s32StrLen);

    u32CanvasWidth = OSD_LIB_FONT_W * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
    u32CanvasHeight = OSD_LIB_FONT_H;
    stFontSize.u32Width = OSD_LIB_FONT_W;
    stFontSize.u32Height = OSD_LIB_FONT_H;
    u32BgColor = 0x7fff;
    u32Color = u32TextColor;

    if (szStr == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "szStr NULL pointer!\n");
        return CVI_FAILURE;
    }

    uint16_t *puBmData = (uint16_t *)pstBitmap->pData;
    CVI_U32 u32BmRow, u32BmCol;

    for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
        CVI_S32 NonASCShow = 0;

        for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
            CVI_S32 s32BmDataIdx = u32BmRow * pstBitmap->u32Width + u32BmCol;
            CVI_S32 s32CharIdx = u32BmCol / stFontSize.u32Width;
            CVI_S32 s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);

            if (NonASCNum > 0 && s32CharIdx > 0) {
                NonASCShow = GetNonASCNum(szStr, s32StringIdx);
                s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
            }
            CVI_S32 s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
                            stFontSize.u32Width;
            CVI_S32 s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
            CVI_S32 s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
            CVI_S32 s32BitOffset = s32CharCol % BYTE_BITS;
            uint8_t *FontMod = NULL;
            CVI_S32 FontModLen = 0;

            if (GetFontMod(&szStr[s32StringIdx], &FontMod, &FontModLen) == CVI_SUCCESS) {
                if (FontMod != NULL && s32HexOffset < FontModLen) {
                    uint8_t temp = FontMod[s32HexOffset];

                    if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1)
                        puBmData[s32BmDataIdx] = (uint16_t)u32Color;
                    else
                        puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
                    continue;
                }
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "GetFontMod Fail\n");
            return CVI_FAILURE;
        }
    }

    return s32Ret;
}

CVI_S32 app_ipcam_Rgn_Mst_LoadBmp(
    const char *filename,
    BITMAP_S *pstBitmap,
    CVI_BOOL bFil,
    CVI_U32 u16FilColor,
    PIXEL_FORMAT_E enPixelFormat)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    CVI_S32 Bpp;
    CVI_U32 nColors;
    CVI_U32 u32PdataSize;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "GetBmpInfo err!\n");
        return CVI_FAILURE;
    }

    Bpp = bmpInfo.bmiHeader.biBitCount / 8;
    nColors = 0;
    if (Bpp == 1) {
        if (bmpInfo.bmiHeader.biClrUsed == 0)
            nColors = 1 << bmpInfo.bmiHeader.biBitCount;
        else
            nColors = bmpInfo.bmiHeader.biClrUsed;

        if (nColors > 256) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Number of indexed palette is over 256.");
            return CVI_FAILURE;
        }
    }

    if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
    } else if (enPixelFormat == PIXEL_FORMAT_8BIT_MODE) {
        Surface.enColorFmt = OSD_COLOR_FMT_8BIT_MODE;
    }  else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Pixel format is not support!\n");
        return CVI_FAILURE;
    }

    u32PdataSize = Bpp * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight);
    pstBitmap->pData = malloc(u32PdataSize);
    if (pstBitmap->pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memory err!\n");
        return CVI_FAILURE;
    }

   if (0 != CreateSurfaceByBitMap(filename, &Surface, (CVI_U8 *)(pstBitmap->pData))) {
       APP_PROF_LOG_PRINT(LEVEL_ERROR, "CreateSurfaceByBitMap failed!\n");
       return CVI_FAILURE;
   }

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = enPixelFormat;

    // if pixel value match color, make it transparent.
    // Only works for ARGB1555
    if (bFil) {
        CVI_U32 i, j;
        CVI_U16 *pu16Temp;

        pu16Temp = (CVI_U16 *)pstBitmap->pData;
        for (i = 0; i < pstBitmap->u32Height; i++) {
            for (j = 0; j < pstBitmap->u32Width; j++) {
                if (u16FilColor == *pu16Temp)
                    *pu16Temp &= 0x7FFF;

                pu16Temp++;
            }
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Rgn_Mst_Canvas_Update(
    const char *filename,
    BITMAP_S *pstBitmap,
    CVI_BOOL bFil,
    CVI_U32 u16FilColor,
    SIZE_S *pstSize,
    CVI_U32 u32Stride,
    PIXEL_FORMAT_E enPixelFormat)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,"GetBmpInfo err!\n");
        return CVI_FAILURE;
    }

    if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Pixel format is not support!\n");
        return CVI_FAILURE;
    }

    if (pstBitmap->pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"malloc osd memroy err!\n");
        return CVI_FAILURE;
    }

    CreateSurfaceByCanvas(filename, &Surface, (CVI_U8 *)(pstBitmap->pData)
                , pstSize->u32Width, pstSize->u32Height, u32Stride);

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = enPixelFormat;

    // if pixel value match color, make it transparent.
    // Only works for ARGB1555
    if (bFil) {
        CVI_U32 i, j;
        CVI_U16 *pu16Temp;

        pu16Temp = (CVI_U16 *)pstBitmap->pData;
        for (i = 0; i < pstBitmap->u32Height; i++) {
            for (j = 0; j < pstBitmap->u32Width; j++) {
                if (u16FilColor == *pu16Temp)
                    *pu16Temp &= 0x7FFF;

                pu16Temp++;
            }
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Rgn_Canvas_Update(RGN_HANDLE Handle, const char *filename)
{
    CVI_S32 s32Ret;
    SIZE_S stSize;
    BITMAP_S stBitmap;
    RGN_CANVAS_INFO_S stCanvasInfo;

    s32Ret = CVI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    // stBitmap.pData = stCanvasInfo.pu8VirtAddr;
    stSize.u32Width = stCanvasInfo.stSize.u32Width;
    stSize.u32Height = stCanvasInfo.stSize.u32Height;

    stBitmap.pData = CVI_SYS_Mmap(stCanvasInfo.u64PhyAddr, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
    if (stBitmap.pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "stBitmap.pData == NULL!\n");
    }

    app_ipcam_Rgn_Mst_Canvas_Update(filename, &stBitmap, CVI_FALSE, 0,
            &stSize, stCanvasInfo.u32Stride, PIXEL_FORMAT_ARGB_1555);

    s32Ret = CVI_RGN_UpdateCanvas(Handle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
        CVI_SYS_Munmap(stBitmap.pData, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
        return CVI_FAILURE;
    }

    CVI_SYS_Munmap(stBitmap.pData, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);

    return s32Ret;
}


APP_PARAM_OSDC_CFG_S *app_ipcam_Osdc_Param_Get(void)
{
    return g_pstOsdcCfg;
}

#ifdef PD_SUPPORT
static CVI_S32 app_ipcam_Osdc_PdRectRegion_Create(CVI_VOID)
{
    RGN_ATTR_S stRegionAttr = {0};
    RGN_CHN_ATTR_S stChnAttr = {0};
    APP_VPSS_GRP_CFG_T *pstVpssCfg;
    CVI_S32 s32Ret;

    if (!g_pstOsdcCfg->bShow[0] ||
        g_pstOsdcCfg->mmfChn[0].enModId != CVI_ID_VPSS ||
        g_pstOsdcCfg->mmfChn[0].s32DevId < 0 ||
        g_pstOsdcCfg->mmfChn[0].s32DevId >= CVI_MAX_VPSS_GRP ||
        g_pstOsdcCfg->mmfChn[0].s32ChnId < 0 ||
        g_pstOsdcCfg->mmfChn[0].s32ChnId >= VPSS_MAX_PHY_CHN_NUM) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "DET event OSD invalid target: show=%d mod=%d dev=%d chn=%d\n",
            g_pstOsdcCfg->bShow[0],
            g_pstOsdcCfg->mmfChn[0].enModId,
            g_pstOsdcCfg->mmfChn[0].s32DevId,
            g_pstOsdcCfg->mmfChn[0].s32ChnId);
        return CVI_FAILURE;
    }
    for (CVI_U32 i = 0; i < OSDC_NUM_MAX; i++) {
        if (g_pstOsdcCfg->bShow[i] &&
            g_pstOsdcCfg->handle[i] == APP_OSDC_PD_RECT_HANDLE) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "DET OSD handle %u conflicts with configured OSD region\n",
                APP_OSDC_PD_RECT_HANDLE);
            return CVI_FAILURE;
        }
    }

    g_stOsdcPdRectChn = g_pstOsdcCfg->mmfChn[0];
    pstVpssCfg = &app_ipcam_Vpss_Param_Get()->
        astVpssGrpCfg[g_stOsdcPdRectChn.s32DevId];
    g_u32OsdcPdRectWidth = pstVpssCfg->
        astVpssChnAttr[g_stOsdcPdRectChn.s32ChnId].u32Width;
    g_u32OsdcPdRectHeight = pstVpssCfg->
        astVpssChnAttr[g_stOsdcPdRectChn.s32ChnId].u32Height;
    g_u32OsdcPdRectCompressedSize =
        g_pstOsdcCfg->CompressedSize[0] > RGN_CMPR_MIN_SIZE ?
        g_pstOsdcCfg->CompressedSize[0] : RGN_CMPR_MIN_SIZE;
    g_enOsdcPdRectFormat = g_pstOsdcCfg->format[0];
    if (g_u32OsdcPdRectWidth == 0 || g_u32OsdcPdRectHeight == 0) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "DET event OSD invalid size: %ux%u\n",
            g_u32OsdcPdRectWidth, g_u32OsdcPdRectHeight);
        return CVI_FAILURE;
    }

    stRegionAttr.enType = OVERLAY_RGN;
    stRegionAttr.unAttr.stOverlay.enPixelFormat = g_enOsdcPdRectFormat;
    stRegionAttr.unAttr.stOverlay.stSize.u32Width = g_u32OsdcPdRectWidth;
    stRegionAttr.unAttr.stOverlay.stSize.u32Height = g_u32OsdcPdRectHeight;
    stRegionAttr.unAttr.stOverlay.u32BgColor = 0x00000000;
    stRegionAttr.unAttr.stOverlay.u32CanvasNum = RGN_MAX_BUF_NUM;
    stRegionAttr.unAttr.stOverlay.stCompressInfo.enOSDCompressMode =
        OSD_COMPRESS_MODE_HW;
    stRegionAttr.unAttr.stOverlay.stCompressInfo.u32CompressedSize =
        g_u32OsdcPdRectCompressedSize;
    s32Ret = CVI_RGN_Create(APP_OSDC_PD_RECT_HANDLE, &stRegionAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "create DET OSD region failed with %#x, handle=%u\n",
            s32Ret, APP_OSDC_PD_RECT_HANDLE);
        return s32Ret;
    }

    stChnAttr.bShow = CVI_TRUE;
    stChnAttr.enType = OVERLAY_RGN;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
    stChnAttr.unChnAttr.stOverlayChn.u32Layer = APP_OSDC_PD_RECT_LAYER;
    s32Ret = CVI_RGN_AttachToChn(
        APP_OSDC_PD_RECT_HANDLE, &g_stOsdcPdRectChn, &stChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "attach DET OSD region failed with %#x\n", s32Ret);
        CVI_RGN_Destroy(APP_OSDC_PD_RECT_HANDLE);
        return s32Ret;
    }
    g_bOsdcPdRectRegionReady = CVI_TRUE;
    return CVI_SUCCESS;
}

static CVI_VOID app_ipcam_Osdc_PdRectRegion_Destroy(CVI_VOID)
{
    CVI_U32 i;

    if (!g_bOsdcPdRectRegionReady) {
        return;
    }
    CVI_RGN_DetachFromChn(APP_OSDC_PD_RECT_HANDLE, &g_stOsdcPdRectChn);
    CVI_RGN_Destroy(APP_OSDC_PD_RECT_HANDLE);
    g_bOsdcPdRectRegionReady = CVI_FALSE;
    for (i = 0; i < OSDC_AI_STR_MAX; i++) {
        if (g_objStrPd.maxlen[i] > 0) {
            CVI_SYS_IonFree(g_objStrPd.u64BitmapPhyAddr[i],
                g_objStrPd.pBitmapVirAddr[i]);
            g_objStrPd.u64BitmapPhyAddr[i] = 0;
            g_objStrPd.pBitmapVirAddr[i] = NULL;
            g_objStrPd.maxlen[i] = 0;
        }
    }
    g_objStrPd.ai_str_num = 0;
}

static CVI_S32 app_ipcam_Osdc_PdLabel_Add(
    RGN_CMPR_OBJ_ATTR_S *pstObjectAttr, CVI_U32 u32ObjectCount,
    const TDLObjectInfo *pstObject, CVI_U32 u32ObjectIndex,
    APP_OSD_LABEL_MODE_E enLabelMode, CVI_U32 u32Color,
    CVI_S32 s32X, CVI_S32 s32Y)
{
    BITMAP_S stBitmap = {0};
    char szText[APP_OSD_STR_LEN_MAX] = {0};
    const char *pszName;
    CVI_S32 s32DataLen;
    CVI_S32 s32Ret;

    if (pstObject == NULL || u32ObjectIndex >= OSDC_AI_STR_MAX) {
        return CVI_FAILURE;
    }
    pszName = pstObject->name[0] != '\0' ? pstObject->name : "object";
    if (enLabelMode == APP_OSD_LABEL_MODE_CLASS_SCORE) {
        snprintf(szText, sizeof(szText), "%.31s %.2f", pszName,
            pstObject->score);
    } else {
        snprintf(szText, sizeof(szText), "%.31s", pszName);
    }
    stBitmap.u32Width = OSD_LIB_FONT_W * strnlen(szText, sizeof(szText));
    stBitmap.u32Height = OSD_LIB_FONT_H;
    s32DataLen = 2 * stBitmap.u32Width * stBitmap.u32Height;
    if (s32DataLen <= 0) {
        return CVI_FAILURE;
    }
    if (s32DataLen > g_objStrPd.maxlen[u32ObjectIndex]) {
        if (g_objStrPd.maxlen[u32ObjectIndex] > 0) {
            CVI_SYS_IonFree(g_objStrPd.u64BitmapPhyAddr[u32ObjectIndex],
                g_objStrPd.pBitmapVirAddr[u32ObjectIndex]);
        }
        s32Ret = CVI_SYS_IonAlloc(
            &g_objStrPd.u64BitmapPhyAddr[u32ObjectIndex],
            (CVI_VOID **)&g_objStrPd.pBitmapVirAddr[u32ObjectIndex],
            "pd_label", s32DataLen);
        if (s32Ret != CVI_SUCCESS) {
            g_objStrPd.u64BitmapPhyAddr[u32ObjectIndex] = 0;
            g_objStrPd.pBitmapVirAddr[u32ObjectIndex] = NULL;
            g_objStrPd.maxlen[u32ObjectIndex] = 0;
            return s32Ret;
        }
        g_objStrPd.maxlen[u32ObjectIndex] = s32DataLen;
    }
    stBitmap.pData = g_objStrPd.pBitmapVirAddr[u32ObjectIndex];
    memset(stBitmap.pData, 0, s32DataLen);
    s32Ret = app_ipcam_Osd_Bitmap_Update(szText, &stBitmap, u32Color);
    if (s32Ret != CVI_SUCCESS) {
        return s32Ret;
    }
    pstObjectAttr[u32ObjectCount].enObjType = RGN_CMPR_BIT_MAP;
    pstObjectAttr[u32ObjectCount].stBitmap.stRect.s32X = s32X;
    pstObjectAttr[u32ObjectCount].stBitmap.stRect.s32Y =
        fmax(0, s32Y - OSD_LIB_FONT_H);
    pstObjectAttr[u32ObjectCount].stBitmap.stRect.u32Width = stBitmap.u32Width;
    pstObjectAttr[u32ObjectCount].stBitmap.stRect.u32Height = stBitmap.u32Height;
    pstObjectAttr[u32ObjectCount].stBitmap.u64BitmapPAddr =
        (CVI_U32)g_objStrPd.u64BitmapPhyAddr[u32ObjectIndex];
    if (g_objStrPd.ai_str_num <= u32ObjectIndex) {
        g_objStrPd.ai_str_num = u32ObjectIndex + 1;
    }
    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Osdc_PdRectRegion_Update(const TDLObject *pstObjects)
{
    RGN_CANVAS_INFO_S stCanvasInfo = {0};
    RGN_CANVAS_CMPR_ATTR_S *pstCanvasAttr;
    RGN_CMPR_OBJ_ATTR_S *pstObjectAttr;
    CVI_U32 u32ObjectCount = 0;
    CVI_U32 u32SourceWidth;
    CVI_U32 u32SourceHeight;
    APP_OSD_STYLE_CFG_S stStyle;
    CVI_S32 s32Ret;

    if (!g_bOsdcPdRectRegionReady || pstObjects == NULL) {
        return CVI_FAILURE;
    }
    s32Ret = CVI_RGN_GetCanvasInfo(
        APP_OSDC_PD_RECT_HANDLE, &stCanvasInfo);
    if (s32Ret != CVI_SUCCESS || stCanvasInfo.pstCanvasCmprAttr == NULL ||
        stCanvasInfo.pstObjAttr == NULL) {
        return s32Ret != CVI_SUCCESS ? s32Ret : CVI_FAILURE;
    }

    pstCanvasAttr = stCanvasInfo.pstCanvasCmprAttr;
    pstObjectAttr = stCanvasInfo.pstObjAttr;
    pthread_mutex_lock(&OsdcMutex);
    stStyle = g_pstOsdcCfg->stStyle;
    pthread_mutex_unlock(&OsdcMutex);
    u32SourceWidth = pstObjects->width > 0 ? pstObjects->width :
        app_ipcam_Ai_PD_Param_Get()->u32GrpWidth;
    u32SourceHeight = pstObjects->height > 0 ? pstObjects->height :
        app_ipcam_Ai_PD_Param_Get()->u32GrpHeight;

    if (stStyle.bDetectionEnable &&
        u32SourceWidth > 0 && u32SourceHeight > 0 &&
        pstObjects->info != NULL) {
        CVI_U32 u32InputCount = pstObjects->size < OSDC_OBJS_MAX ?
            pstObjects->size : OSDC_OBJS_MAX;
        for (CVI_U32 i = 0; i < u32InputCount; i++) {
            const TDLObjectInfo *pstObject = &pstObjects->info[i];
            CVI_S32 s32X1;
            CVI_S32 s32Y1;
            CVI_S32 s32X2;
            CVI_S32 s32Y2;

            if (u32ObjectCount >= OSDC_OBJS_MAX) {
                break;
            }

            if (!isfinite(pstObject->box.x1) ||
                !isfinite(pstObject->box.y1) ||
                !isfinite(pstObject->box.x2) ||
                !isfinite(pstObject->box.y2)) {
                continue;
            }
            s32X1 = lroundf(pstObject->box.x1 *
                g_u32OsdcPdRectWidth / u32SourceWidth);
            s32Y1 = lroundf(pstObject->box.y1 *
                g_u32OsdcPdRectHeight / u32SourceHeight);
            s32X2 = lroundf(pstObject->box.x2 *
                g_u32OsdcPdRectWidth / u32SourceWidth);
            s32Y2 = lroundf(pstObject->box.y2 *
                g_u32OsdcPdRectHeight / u32SourceHeight);
            s32X1 = fmax(0, fmin(s32X1,
                (CVI_S32)g_u32OsdcPdRectWidth - 1));
            s32Y1 = fmax(0, fmin(s32Y1,
                (CVI_S32)g_u32OsdcPdRectHeight - 1));
            s32X2 = fmax(s32X1 + 1, fmin(s32X2,
                (CVI_S32)g_u32OsdcPdRectWidth));
            s32Y2 = fmax(s32Y1 + 1, fmin(s32Y2,
                (CVI_S32)g_u32OsdcPdRectHeight));

            pstObjectAttr[u32ObjectCount].enObjType = RGN_CMPR_RECT;
            pstObjectAttr[u32ObjectCount].stRgnRect.stRect.s32X = s32X1;
            pstObjectAttr[u32ObjectCount].stRgnRect.stRect.s32Y = s32Y1;
            pstObjectAttr[u32ObjectCount].stRgnRect.stRect.u32Width =
                s32X2 - s32X1;
            pstObjectAttr[u32ObjectCount].stRgnRect.stRect.u32Height =
                s32Y2 - s32Y1;
            pstObjectAttr[u32ObjectCount].stRgnRect.u32Thick =
                app_ipcam_Osdc_Thickness(stStyle.u32DetectionThickness);
            pstObjectAttr[u32ObjectCount].stRgnRect.u32Color =
                app_ipcam_Osdc_DetectionColor(&stStyle,
                    pstObject->class_id);
            pstObjectAttr[u32ObjectCount].stRgnRect.u32IsFill = CVI_FALSE;
            u32ObjectCount++;
            if (stStyle.enDetectionLabelMode != APP_OSD_LABEL_MODE_NONE &&
                i < OSDC_AI_STR_MAX && u32ObjectCount < OSDC_OBJS_MAX &&
                app_ipcam_Osdc_PdLabel_Add(pstObjectAttr, u32ObjectCount,
                    pstObject, i, stStyle.enDetectionLabelMode,
                    pstObjectAttr[u32ObjectCount - 1].stRgnRect.u32Color,
                    s32X1, s32Y1) == CVI_SUCCESS) {
                u32ObjectCount++;
            }
        }
    }

    pstCanvasAttr->u32Width = g_u32OsdcPdRectWidth;
    pstCanvasAttr->u32Height = g_u32OsdcPdRectHeight;
    pstCanvasAttr->u32BgColor = 0x00000000;
    pstCanvasAttr->enPixelFormat = g_enOsdcPdRectFormat;
    pstCanvasAttr->u32BsSize = g_u32OsdcPdRectCompressedSize;
    pstCanvasAttr->u32ObjNum = u32ObjectCount;
    return CVI_RGN_UpdateCanvas(APP_OSDC_PD_RECT_HANDLE);
}

static CVI_VOID *Thread_Osdc_PdRect_Draw(CVI_VOID *arg)
{
    CVI_U64 u64LastGeneration = (CVI_U64)-1;
    TDLObject stObjects = {0};

    (void)arg;
    pthread_mutex_lock(&g_OsdcPdRectMutex);
    g_bOsdcPdRectThreadReady = CVI_TRUE;
    pthread_cond_broadcast(&g_OsdcPdRectCond);
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    prctl(PR_SET_NAME, "OSDC_PD_RECT", 0, 0, 0);
    while (CVI_TRUE) {
        CVI_S32 s32Ret;

        pthread_mutex_lock(&g_OsdcPdRectMutex);
        while (g_bOsdcPdRectThreadRun &&
               u64LastGeneration == g_u64OsdcPdRectGeneration) {
            pthread_cond_wait(
                &g_OsdcPdRectCond, &g_OsdcPdRectMutex);
        }
        if (!g_bOsdcPdRectThreadRun) {
            pthread_mutex_unlock(&g_OsdcPdRectMutex);
            break;
        }
        u64LastGeneration = g_u64OsdcPdRectGeneration;
        pthread_mutex_unlock(&g_OsdcPdRectMutex);

        if (!g_pstOsdcCfg->bShowPdRect[0] ||
            !app_ipcam_Ai_PD_ProcStatus_Get()) {
            stObjects.size = 0;
        } else if (app_ipcam_Ai_PD_ObjDrawInfo_Get(
                       &stObjects) != CVI_SUCCESS) {
            stObjects.size = 0;
        }
        s32Ret = app_ipcam_Osdc_PdRectRegion_Update(&stObjects);
        if (s32Ret != CVI_SUCCESS && s32Ret != CVI_ERR_RGN_BUSY) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "update DET event OSD region failed with %#x\n", s32Ret);
        }
    }
    stObjects.size = 0;
    app_ipcam_Osdc_PdRectRegion_Update(&stObjects);
    free(stObjects.info);
    pthread_mutex_lock(&g_OsdcPdRectMutex);
    g_bOsdcPdRectThreadReady = CVI_FALSE;
    pthread_cond_broadcast(&g_OsdcPdRectCond);
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    return NULL;
}

static CVI_VOID app_ipcam_Osdc_PdRect_StopEventMode(CVI_VOID)
{
    pthread_mutex_lock(&g_OsdcPdRectLifecycleMutex);
    pthread_mutex_lock(&g_OsdcPdRectMutex);
    g_bOsdcPdRectThreadRun = CVI_FALSE;
    g_u64OsdcPdRectGeneration++;
    pthread_cond_broadcast(&g_OsdcPdRectCond);
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    if (g_pthOsdcPdRect > (pthread_t)0) {
        pthread_join(g_pthOsdcPdRect, NULL);
        g_pthOsdcPdRect = 0;
    }
    pthread_mutex_lock(&g_OsdcPdRectMutex);
    g_bOsdcPdRectThreadReady = CVI_FALSE;
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    app_ipcam_Osdc_PdRectRegion_Destroy();
    pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
}

CVI_S32 app_ipcam_Osdc_PdRect_EnsureEventMode(CVI_VOID)
{
    CVI_S32 s32Ret;
    CVI_BOOL bThreadReady;

    pthread_mutex_lock(&g_OsdcPdRectLifecycleMutex);
    if (app_ipcam_Osdc_PdRect_UnifiedReady()) {
        pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
        return CVI_SUCCESS;
    }
    pthread_mutex_lock(&g_OsdcPdRectMutex);
    bThreadReady = g_bOsdcPdRectThreadRun &&
        g_bOsdcPdRectThreadReady;
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    if (bThreadReady && g_bOsdcPdRectRegionReady) {
        pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
        return CVI_SUCCESS;
    }
    if (!g_pstOsdcCfg->enable || !g_pstOsdcCfg->bShow[0] ||
        !g_pstOsdcCfg->bShowPdRect[0]) {
        pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
        return CVI_FAILURE;
    }

    pthread_mutex_lock(&g_OsdcPdRectMutex);
    g_bOsdcPdRectThreadRun = CVI_FALSE;
    pthread_cond_broadcast(&g_OsdcPdRectCond);
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    if (g_pthOsdcPdRect > (pthread_t)0) {
        pthread_join(g_pthOsdcPdRect, NULL);
        g_pthOsdcPdRect = 0;
    }
    pthread_mutex_lock(&g_OsdcPdRectMutex);
    g_bOsdcPdRectThreadReady = CVI_FALSE;
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    app_ipcam_Osdc_PdRectRegion_Destroy();

    s32Ret = app_ipcam_Osdc_PdRectRegion_Create();
    if (s32Ret != CVI_SUCCESS) {
        pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
        return s32Ret;
    }

    pthread_mutex_lock(&g_OsdcPdRectMutex);
    g_bOsdcPdRectThreadRun = CVI_TRUE;
    g_bOsdcPdRectThreadReady = CVI_FALSE;
    g_u64OsdcPdRectGeneration++;
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    s32Ret = pthread_create(
        &g_pthOsdcPdRect, NULL, Thread_Osdc_PdRect_Draw, NULL);
    if (s32Ret != CVI_SUCCESS) {
        pthread_mutex_lock(&g_OsdcPdRectMutex);
        g_bOsdcPdRectThreadRun = CVI_FALSE;
        pthread_mutex_unlock(&g_OsdcPdRectMutex);
        app_ipcam_Osdc_PdRectRegion_Destroy();
        g_pthOsdcPdRect = 0;
        pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "create DET event OSD thread failed with %#x\n", s32Ret);
        return s32Ret;
    }

    pthread_mutex_lock(&g_OsdcPdRectMutex);
    while (g_bOsdcPdRectThreadRun && !g_bOsdcPdRectThreadReady) {
        pthread_cond_wait(&g_OsdcPdRectCond, &g_OsdcPdRectMutex);
    }
    if (!g_bOsdcPdRectThreadReady) {
        pthread_mutex_unlock(&g_OsdcPdRectMutex);
        pthread_join(g_pthOsdcPdRect, NULL);
        g_pthOsdcPdRect = 0;
        app_ipcam_Osdc_PdRectRegion_Destroy();
        pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
        return CVI_FAILURE;
    }
    pthread_mutex_unlock(&g_OsdcPdRectMutex);
    pthread_mutex_unlock(&g_OsdcPdRectLifecycleMutex);
    return CVI_SUCCESS;
}
#endif

CVI_S32 app_ipcam_OSDCRgn_Create(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    int iOsdcIndex = 0;
    for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
        if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
            MMF_CHN_S *mmfChn = &g_pstOsdcCfg->mmfChn[iOsdcIndex];
            RGN_HANDLE handle = g_pstOsdcCfg->handle[iOsdcIndex];
            CVI_U32 u32CpsSize = g_pstOsdcCfg->CompressedSize[iOsdcIndex];

            VPSS_CHN_ATTR_S *pVpssChnAttr = &app_ipcam_Vpss_Param_Get()->astVpssGrpCfg[mmfChn->s32DevId].astVpssChnAttr[mmfChn->s32ChnId];
            CVI_U32 u32Width = pVpssChnAttr->u32Width;
            CVI_U32 u32Height = pVpssChnAttr->u32Height;
            APP_PROF_LOG_PRINT(LEVEL_INFO, "OSDC RGN handle(%d) RGN size W:%d H:%d\n", handle, u32Width, u32Height);

            for (CVI_U32 i = 0; i < g_pstOsdcCfg->osdcObjNum[iOsdcIndex]; i++) {
                if (g_pstOsdcCfg->osdcObj[iOsdcIndex][i].type == RGN_CMPR_BIT_MAP) {
                    g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr = (CVI_U64)0;
                    g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr = NULL;
                    g_pstOsdcCfg->osdcObj[iOsdcIndex][i].maxlen = 0;
                }
            }

             g_objStrAi.ai_str_num = 0;
            for (CVI_U32 i = 0; i < OSDC_AI_STR_MAX; i++) {
                g_objStrAi.u64BitmapPhyAddr[i] = (CVI_U64)0;
                g_objStrAi.pBitmapVirAddr[i] = NULL;
                g_objStrAi.maxlen[i] = 0;
            }

            RGN_ATTR_S regAttr;
            memset(&regAttr, 0, sizeof(regAttr));
            regAttr.enType = OVERLAY_RGN;
            regAttr.unAttr.stOverlay.enPixelFormat = g_pstOsdcCfg->format[iOsdcIndex];
            regAttr.unAttr.stOverlay.stSize.u32Width = u32Width;
            regAttr.unAttr.stOverlay.stSize.u32Height = u32Height;
            regAttr.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
            regAttr.unAttr.stOverlay.u32CanvasNum = 2;
#if defined(AI_SUPPORT) && defined(PD_SUPPORT)
            if (iOsdcIndex == 0 &&
                g_pstOsdcCfg->bShowPdRect[iOsdcIndex]) {
                regAttr.unAttr.stOverlay.u32CanvasNum = RGN_MAX_BUF_NUM;
            }
#endif
            regAttr.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
            regAttr.unAttr.stOverlay.stCompressInfo.u32CompressedSize = u32CpsSize;
            s32Ret = CVI_RGN_Create(handle, &regAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_RGN_Create failed with %#x, hdl(%d)\n", s32Ret, handle);
                return s32Ret;
            }

            CVI_BOOL bShow = g_pstOsdcCfg->bShow[iOsdcIndex];
            RGN_CHN_ATTR_S regChnAttr;
            memset(&regChnAttr, 0, sizeof(regChnAttr));
            regChnAttr.bShow = bShow;
            regChnAttr.enType = OVERLAY_RGN;
            regChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
            regChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
            regChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
            regChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
            s32Ret = CVI_RGN_AttachToChn(handle, mmfChn, &regChnAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_RGN_AttachToChn failed with %#x, hdl(%d), chn(%d %d %d)\n",
                           s32Ret, handle, mmfChn->enModId, mmfChn->s32DevId, mmfChn->s32ChnId);
                CVI_RGN_Destroy(handle);
            }

        }
    }

    return s32Ret;
}

CVI_S32 app_ipcam_OSDCRgn_Destory(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    int iOsdcIndex = 0;
    for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
        if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
            RGN_HANDLE handle = g_pstOsdcCfg->handle[iOsdcIndex];
            MMF_CHN_S *mmfChn = &g_pstOsdcCfg->mmfChn[iOsdcIndex];

            s32Ret = CVI_RGN_DetachFromChn(handle, mmfChn);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_RGN_DetachFromChn failedwith %#x, hdl(%d), chn(%d %d %d)\n",
                           s32Ret, handle, mmfChn->enModId, mmfChn->s32DevId, mmfChn->s32ChnId);
                return CVI_FAILURE;
            }

            s32Ret = CVI_RGN_Destroy(handle);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"app_ipcam_OSDCRgn_Destory failed with %#x, hdl(%d)\n", s32Ret, handle);
                return CVI_FAILURE;
            }
        }
        for (CVI_U32 i = 0; i < g_pstOsdcCfg->osdcObjNum[iOsdcIndex]; i++) {
            if (g_pstOsdcCfg->osdcObj[iOsdcIndex][i].maxlen) {
                CVI_SYS_IonFree(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr, g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr);
                g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr = (CVI_U64)0;
                g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr = NULL;
                g_pstOsdcCfg->osdcObj[iOsdcIndex][i].maxlen = 0;
            }
        }

        for (CVI_U32 i = 0; i < g_objStrAi.ai_str_num; i++) {
            if (g_objStrAi.maxlen[i]) {
                CVI_SYS_IonFree(g_objStrAi.u64BitmapPhyAddr[i], g_objStrAi.pBitmapVirAddr[i]);
                g_objStrAi.u64BitmapPhyAddr[i] = (CVI_U64)0;
                g_objStrAi.pBitmapVirAddr[i] = NULL;
                g_objStrAi.maxlen[i] = 0;
            }
        }
        g_objStrAi.ai_str_num = 0;
    }

    return s32Ret;
}

/* Compressed OSD objects use ARGB1555 colors. */
static int app_ipcam_ObjsRectInfo_Update(RGN_HANDLE OsdcHandle, int iOsdcIndex)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U32 i = 0;
    CVI_U32 OsdcObjsNum = 0;
    CVI_S32 s32StrLen = 0;
    CVI_S32 s32DataLen = 0;
    char *pszStr = NULL;
    char szStr[APP_OSD_STR_LEN_MAX] = {0};
    float s32Ratio = 1;
    BITMAP_S stBitmap;
    RGN_CANVAS_INFO_S stCanvasInfo = {0};
    s32Ret = CVI_RGN_GetCanvasInfo(OsdcHandle, &stCanvasInfo);
    if (s32Ret != CVI_SUCCESS) {
#if defined(AI_SUPPORT) && defined(PD_SUPPORT)
        if (s32Ret == CVI_ERR_RGN_BUSY && iOsdcIndex == 0) {
            return s32Ret;
        }
#endif
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "CVI RGN GetCanvasInfo failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr = stCanvasInfo.pstObjAttr;

    APP_PARAM_OSDC_CFG_S *stOsdcCfg = app_ipcam_Osdc_Param_Get();
    MMF_CHN_S *pstChn = &stOsdcCfg->mmfChn[iOsdcIndex];
    APP_PARAM_CHN_CFG_T *pastChnInfo = &app_ipcam_Vi_Param_Get()->astChnInfo[0];
    VPSS_CHN_ATTR_S *pVpssChnAttr = &app_ipcam_Vpss_Param_Get()->astVpssGrpCfg[pstChn->s32DevId].astVpssChnAttr[pstChn->s32ChnId];
    s32Ratio = fmax(((float)pVpssChnAttr->u32Width / (float)pastChnInfo->u32Width), ((float)pVpssChnAttr->u32Height / (float)pastChnInfo->u32Height));
    #ifdef AI_SUPPORT
    #ifdef PD_SUPPORT
    if (iOsdcIndex == 0 &&
        g_pstOsdcCfg->bShowPdRect[iOsdcIndex] &&
        g_pstOsdcCfg->stStyle.bDetectionEnable &&
        app_ipcam_Ai_PD_ProcStatus_Get()) {
        app_ipcam_Ai_PD_ObjDrawInfo_Get(&g_objMetaPd);
        if (g_objMetaPd.size > 0 && g_objMetaPd.info != NULL) {
            for (i = 0; i < g_objMetaPd.size; i++) {
                if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                    break;
                }
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stPdRectRatio.ScaleX * g_objMetaPd.info[i].box.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stPdRectRatio.ScaleY * g_objMetaPd.info[i].box.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_stPdRectRatio.ScaleX * (g_objMetaPd.info[i].box.x2 - g_objMetaPd.info[i].box.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_stPdRectRatio.ScaleY * (g_objMetaPd.info[i].box.y2 - g_objMetaPd.info[i].box.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick =
                    app_ipcam_Osdc_Thickness(
                        g_pstOsdcCfg->stStyle.u32DetectionThickness);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Color =
                    app_ipcam_Osdc_DetectionColor(&g_pstOsdcCfg->stStyle,
                        g_objMetaPd.info[i].class_id);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
                pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

                OsdcObjsNum++;
            }
        }
    }
    #endif
    #ifdef MD_SUPPORT
    if (iOsdcIndex == 0 && g_pstOsdcCfg->bShowMdRect[iOsdcIndex]) {
        app_ipcam_Ai_MD_ObjDrawInfo_Get(&g_objMetaMd);
        if (g_objMetaMd.size > 0 && g_objMetaMd.info != NULL) {
            for (i = 0; i < g_objMetaMd.size; i++) {
                if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                    break;
                }

                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stMdRectRatio.ScaleX * g_objMetaMd.info[i].box.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stMdRectRatio.ScaleY * g_objMetaMd.info[i].box.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_stMdRectRatio.ScaleX * (g_objMetaMd.info[i].box.x2 - g_objMetaMd.info[i].box.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_stMdRectRatio.ScaleY * (g_objMetaMd.info[i].box.y2 - g_objMetaMd.info[i].box.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = 4;
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = COLOR_YELLOW(0);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
                pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

                OsdcObjsNum++;
            }
            TDL_ReleaseObjectMeta(&g_objMetaMd);
        }
    }
    #endif

#ifdef FACE_SUPPORT
    if (iOsdcIndex == 0 && g_pstOsdcCfg->bShowFdRect[iOsdcIndex]) {
        app_ipcam_Ai_FD_ObjDrawInfo_Get(&g_objMetaFd);
        if (g_objMetaFd.size > 0 && g_objMetaFd.info != NULL) {
            for (i = 0; i < g_objMetaFd.size; i++) {
                if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                    return CVI_FAILURE;
                }

                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stFdRectRatio.ScaleX * g_objMetaFd.info[i].box.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stFdRectRatio.ScaleY * g_objMetaFd.info[i].box.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_stFdRectRatio.ScaleX * (g_objMetaFd.info[i].box.x2 - g_objMetaFd.info[i].box.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_stFdRectRatio.ScaleY * (g_objMetaFd.info[i].box.y2 - g_objMetaFd.info[i].box.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = 4;
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = COLOR_RED(0);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
                pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

                OsdcObjsNum++;
            }
            TDL_ReleaseFaceMeta(&g_objMetaFd);
        }
    }
#endif
#ifdef HUMAN_KEYPOINT_SUPPORT
    if (iOsdcIndex == 0 && g_pstOsdcCfg->bShowHumanKeypointRect[iOsdcIndex]) {
        app_ipcam_Ai_Human_Keypoint_ObjDrawInfo_Get(&g_objMetaHumanKeypoint);
        if (g_objMetaHumanKeypoint.size > 0 && g_objMetaHumanKeypoint.info != NULL) {
            // printf("obj_meta.size:%d\n", g_objMetaHumanKeypoint.size);
            for (i = 0; i < g_objMetaHumanKeypoint.size; i++) {
                for (int j = 0; j < 17; j++) {
                    if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                        break;
                    }
                    pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stHumanKeypointRectRatio.ScaleX * g_objMetaHumanKeypoint.info[i].landmark_properity[j].x) - 8;
                    pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stHumanKeypointRectRatio.ScaleY * (g_objMetaHumanKeypoint.info[i].landmark_properity[j].y-4)) - 8;
                    pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = 16 ;
                    pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = 16 ;
                    pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = 8;
                    pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = COLOR_RED(0);
                    pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_TRUE;
                    pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

                    OsdcObjsNum++;
                }
            }
            TDL_ReleaseObjectMeta(&g_objMetaHumanKeypoint);
        }
    }
#endif
#ifdef OBJECT_TRACK_SUPPORT
if (iOsdcIndex == 0) {
    CVI_BOOL bTrackingReady = app_ipcam_Ai_Object_Track_ProcStatus_Get();

    if (bTrackingReady && app_ipcam_Ai_Object_Track_Mode_Get() != TRACKING) {
        app_ipcam_Ai_Object_Track_ObjDrawInfo_Get(&g_objMetaObjectTrack);
    }
    s32Ret = app_ipcam_Osd_ObjectTrack_CenterBox_Add(
        pstObjAttr, &OsdcObjsNum,
        bTrackingReady ? &g_objMetaObjectTrack : NULL,
        &g_pstOsdcCfg->stStyle);
    if (s32Ret != CVI_SUCCESS) {
        return s32Ret;
    }
}
if (iOsdcIndex == 0 &&
    g_pstOsdcCfg->bShowTrackRect[iOsdcIndex] &&
    app_ipcam_Ai_Object_Track_ProcStatus_Get()) {
    APP_PARAM_OBJECT_TRACK_MODE enTrackMode =
        app_ipcam_Ai_Object_Track_Mode_Get();

    if (enTrackMode != TRACKING &&
        g_pstOsdcCfg->stStyle.bDetectionEnable &&
        g_objMetaObjectTrack.size > 0 &&
        g_objMetaObjectTrack.info != NULL) {
        for (i = 0; i < g_objMetaObjectTrack.size; i++) {
            if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                return CVI_FAILURE;
            }

            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stObjectTrackRectRatio.ScaleX * g_objMetaObjectTrack.info[i].box.x1);
            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stObjectTrackRectRatio.ScaleY * g_objMetaObjectTrack.info[i].box.y1);
            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_stObjectTrackRectRatio.ScaleX * (g_objMetaObjectTrack.info[i].box.x2 - g_objMetaObjectTrack.info[i].box.x1);
            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_stObjectTrackRectRatio.ScaleY * (g_objMetaObjectTrack.info[i].box.y2 - g_objMetaObjectTrack.info[i].box.y1);
            CVI_U32 u32ObjectColor = app_ipcam_Osdc_DetectionColor(
                &g_pstOsdcCfg->stStyle,
                g_objMetaObjectTrack.info[i].class_id);
            pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick =
                app_ipcam_Osdc_Thickness(
                    g_pstOsdcCfg->stStyle.u32DetectionThickness);
            pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = u32ObjectColor;
            pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
            pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

            OsdcObjsNum++;
            if (i < OSDC_AI_STR_MAX &&
                g_pstOsdcCfg->stStyle.enDetectionLabelMode !=
                    APP_OSD_LABEL_MODE_NONE)
            {
                s32Ret = app_ipcam_ObjsRectInfo_Add_AiStr(
                    pstObjAttr, OsdcObjsNum, &g_objMetaObjectTrack, i,
                    g_pstOsdcCfg->stStyle.enDetectionLabelMode,
                    u32ObjectColor);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"app_ipcam_ObjsRectInfo_Add_AiStr failed with %#x!\n", s32Ret);
                    if(g_objStrAi.ai_str_num < i)
                        g_objStrAi.ai_str_num = i;
                    break;
                }
                OsdcObjsNum++;
            }
			if(g_objStrAi.ai_str_num < g_objMetaObjectTrack.size){
                g_objStrAi.ai_str_num = fmin(g_objMetaObjectTrack.size, OSDC_AI_STR_MAX);
            }
        }
    }
}
#endif
#endif

    for (i = 0; i < g_pstOsdcCfg->osdcObjNum[iOsdcIndex]; i++) {
        if (OsdcObjsNum >= OSDC_OBJS_MAX) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
            break;
        }
        if (!g_pstOsdcCfg->osdcObj[iOsdcIndex][i].bShow) {
            continue;
        }

        pstObjAttr[OsdcObjsNum].enObjType = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].type;
        OSD_TYPE_E enType = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].enType;
        if (pstObjAttr[OsdcObjsNum].enObjType == RGN_CMPR_BIT_MAP) {
            memset(&stBitmap, 0, sizeof(BITMAP_S));
            switch (enType) {
                case TYPE_PICTURE:
                    s32Ret = app_ipcam_Rgn_Mst_LoadBmp(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].filename, &stBitmap, CVI_FALSE, 0, PIXEL_FORMAT_ARGB_1555);
                    if (s32Ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "app_ipcam_Rgn_Mst_LoadBmp failed with %#x!\n", s32Ret);
                        continue;
                    }
                break;

                case TYPE_STRING:
                    s32StrLen = strlen(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str) - GetNonASCNum(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str, strlen(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str));
                    pszStr = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str;
                    stBitmap.u32Width = OSD_LIB_FONT_W * s32StrLen;
                    stBitmap.u32Height = OSD_LIB_FONT_H;
                break;

                case TYPE_TIME:
                    memset(szStr, 0, APP_OSD_STR_LEN_MAX);
                    GetTimeStr(NULL, szStr, APP_OSD_STR_LEN_MAX);
                    s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
                    pszStr = szStr;
                    stBitmap.u32Width = OSD_LIB_FONT_W * s32StrLen;
                    stBitmap.u32Height = OSD_LIB_FONT_H;
                break;

                case TYPE_DEBUG:
                    // #ifdef AI_SUPPORT
                    //     memset(szStr, 0, APP_OSD_STR_LEN_MAX);
                    //     GetDebugStr(szStr, APP_OSD_STR_LEN_MAX);
                    //     s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
                    //     pszStr = szStr;
                    //     stBitmap.u32Width = OSD_LIB_FONT_W * s32StrLen;
                    //     stBitmap.u32Height = OSD_LIB_FONT_H;
                    // #else
                    //     g_pstOsdcCfg->osdcObj[iOsdcIndex][i].bShow = 0;
                    // #endif
                break;

                default:
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OSDC enType = %d invalid \n", enType);
                break;
            }

            s32DataLen = 2 * (stBitmap.u32Width) * (stBitmap.u32Height);
            if (s32DataLen == 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "enType %d s32DataLen invalid!\n", enType);
                continue;
            }

            if (TYPE_PICTURE != enType) {
                stBitmap.pData = malloc(s32DataLen);
                if (stBitmap.pData == NULL) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memroy err!\n");
                    return -1;
                }
                memset(stBitmap.pData, 0, s32DataLen);
                s32Ret = app_ipcam_Osd_Bitmap_Update(pszStr, &stBitmap,
                    app_ipcam_Osdc_ColorToArgb1555(
                        g_pstOsdcCfg->osdcObj[iOsdcIndex][i].color));
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Bitmap_Update failed!\n");
                    free(stBitmap.pData);
                    return -1;
                }
            }
            if (s32DataLen > g_pstOsdcCfg->osdcObj[iOsdcIndex][i].maxlen) {
                if(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].maxlen){
                    CVI_SYS_IonFree(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr, g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr);
                    g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr = (CVI_U64)0;
                    g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr = NULL;
                }
                s32Ret = CVI_SYS_IonAlloc(&g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr, (CVI_VOID **)&g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr,
                    "rgn_cmpr_bitmap2", s32DataLen);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
                    free(stBitmap.pData);
                    return -1;
                }
                g_pstOsdcCfg->osdcObj[iOsdcIndex][i].maxlen = s32DataLen;
            }
            memcpy(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr, stBitmap.pData, s32DataLen);
            CVI_S32 s32X = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x1 * s32Ratio;
            CVI_S32 s32Y = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y1 * s32Ratio;
            if (enType == TYPE_STRING) {
                CVI_S32 s32OffsetX = s32X;
                CVI_S32 s32OffsetY = s32Y;

                switch (g_pstOsdcCfg->stStyle.enTextPosition) {
                    case APP_OSD_TEXT_POSITION_TOP_RIGHT:
                        s32X = stCanvasInfo.stSize.u32Width -
                            stBitmap.u32Width - s32OffsetX;
                        break;
                    case APP_OSD_TEXT_POSITION_BOTTOM_LEFT:
                        s32Y = stCanvasInfo.stSize.u32Height -
                            stBitmap.u32Height - s32OffsetY;
                        break;
                    case APP_OSD_TEXT_POSITION_BOTTOM_RIGHT:
                        s32X = stCanvasInfo.stSize.u32Width -
                            stBitmap.u32Width - s32OffsetX;
                        s32Y = stCanvasInfo.stSize.u32Height -
                            stBitmap.u32Height - s32OffsetY;
                        break;
                    case APP_OSD_TEXT_POSITION_CUSTOM:
                    case APP_OSD_TEXT_POSITION_TOP_LEFT:
                    default:
                        break;
                }
                s32X = fmax(0, fmin(s32X,
                    (CVI_S32)stCanvasInfo.stSize.u32Width -
                    (CVI_S32)stBitmap.u32Width));
                s32Y = fmax(0, fmin(s32Y,
                    (CVI_S32)stCanvasInfo.stSize.u32Height -
                    (CVI_S32)stBitmap.u32Height));
            }
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.s32X = s32X;
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.s32Y = s32Y;
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.u32Width = stBitmap.u32Width;
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.u32Height = stBitmap.u32Height;
            pstObjAttr[OsdcObjsNum].stBitmap.u64BitmapPAddr = (CVI_U32)g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr;

            free(stBitmap.pData);
        } else {
            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x1 * s32Ratio;;
            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y1 * s32Ratio;;
            if (pstObjAttr[OsdcObjsNum].enObjType == RGN_CMPR_LINE) {
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x2 * s32Ratio;;
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y2 * s32Ratio;;
            } else if (pstObjAttr[OsdcObjsNum].enObjType == RGN_CMPR_RECT) {
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].width * s32Ratio;
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].height * s32Ratio;
            }
            pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].thickness;
            pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].color;
            pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].filled;
#ifdef WEB_SOCKET
            if (g_pstOsdcCfg->osdcObj[iOsdcIndex][i].filled) {
                g_pstOsdcPrivacy->bShow = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].bShow;
                g_pstOsdcPrivacy->x1 = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x1 * s32Ratio;
                g_pstOsdcPrivacy->y1 = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y1 * s32Ratio;
                g_pstOsdcPrivacy->width = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].width * s32Ratio;
                g_pstOsdcPrivacy->height = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].height * s32Ratio;
                COLOR_TO_IDX(0, g_pstOsdcPrivacy->color, g_pstOsdcCfg->osdcObj[iOsdcIndex][i].color);
            }
#endif
        }
        OsdcObjsNum++;
    }

    RGN_ATTR_S stRegion;
    s32Ret = CVI_RGN_GetAttr(OsdcHandle, &stRegion);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI RGN GetAttr failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    pstCanvasCmprAttr->u32Width      = stRegion.unAttr.stOverlay.stSize.u32Width;
    pstCanvasCmprAttr->u32Height     = stRegion.unAttr.stOverlay.stSize.u32Height;
    pstCanvasCmprAttr->u32BgColor    = stRegion.unAttr.stOverlay.u32BgColor;
    pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
    pstCanvasCmprAttr->u32BsSize     = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
    pstCanvasCmprAttr->u32ObjNum     = OsdcObjsNum;
    s32Ret = CVI_RGN_UpdateCanvas(OsdcHandle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI RGN UpdateCanvas failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    g_abOsdcCanvasReady[iOsdcIndex] = CVI_TRUE;
    return CVI_SUCCESS;
}

static CVI_U64 app_ipcam_Osdc_MonotonicMs(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return (CVI_U64)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static int app_ipcam_Osdc_FirstCanvas(void)
{
    CVI_U64 start = app_ipcam_Osdc_MonotonicMs();
    CVI_U32 busyCount = 0;

    memset(g_abOsdcCanvasReady, 0, sizeof(g_abOsdcCanvasReady));
    for (;;) {
        CVI_BOOL ready = CVI_TRUE;

        for (int i = 0; i < OSDC_NUM_MAX; i++) {
            if (!g_pstOsdcCfg->bShow[i] || g_abOsdcCanvasReady[i]) {
                continue;
            }
            CVI_S32 ret = app_ipcam_ObjsRectInfo_Update(g_pstOsdcCfg->handle[i], i);
            if (ret == CVI_ERR_RGN_BUSY) {
                busyCount++;
                ready = CVI_FALSE;
            } else if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,
                    "OSD first canvas failed: handle=%u ret=%#x busy=%u\n",
                    g_pstOsdcCfg->handle[i], ret, busyCount);
                return ret;
            }
        }
        CVI_U64 elapsed = app_ipcam_Osdc_MonotonicMs() - start;
        if (ready) {
            APP_PROF_LOG_PRINT(LEVEL_INFO,
                "[boot %llu ms] OSD first canvas ready: elapsed=%llu ms busy=%u\n",
                (unsigned long long)app_ipcam_Osdc_MonotonicMs(),
                (unsigned long long)elapsed, busyCount);
            return CVI_SUCCESS;
        }
        if (elapsed >= APP_OSDC_FIRST_CANVAS_TIMEOUT_MS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "OSD first canvas timeout: elapsed=%llu ms busy=%u\n",
                (unsigned long long)elapsed, busyCount);
            return CVI_ERR_RGN_BUSY;
        }
        usleep(APP_OSDC_FIRST_CANVAS_RETRY_US);
    }
}

static int app_ipcam_ObjRectRatio_Set(void)
{
    /* get main-streaming VPSS Grp0Chn0 size */
    APP_VPSS_GRP_CFG_T *pstVpssCfg = &app_ipcam_Vpss_Param_Get()->astVpssGrpCfg[0];
    _NULL_POINTER_CHECK_(pstVpssCfg, -1);

    /* OSD Codec size form main streaming (vpss group_0 channel_0) */
    SIZE_S stOdecSize;
    stOdecSize.u32Width = pstVpssCfg->astVpssChnAttr[0].u32Width;
    stOdecSize.u32Height = pstVpssCfg->astVpssChnAttr[0].u32Height;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "draw canvas size W=%d H=%d\n", stOdecSize.u32Width, stOdecSize.u32Height);

    #ifdef PD_SUPPORT
    APP_PARAM_AI_PD_CFG_S *pstPdCfg = app_ipcam_Ai_PD_Param_Get();
    _NULL_POINTER_CHECK_(pstPdCfg, -1);
    /* set AI PD rect-ratio */
    g_stPdRectRatio.VpssChn_W = pstPdCfg->u32GrpWidth;
    g_stPdRectRatio.VpssChn_H = pstPdCfg->u32GrpHeight;
    g_stPdRectRatio.ScaleX = (float)stOdecSize.u32Width /
        (float)g_stPdRectRatio.VpssChn_W;
    g_stPdRectRatio.ScaleY = (float)stOdecSize.u32Height /
        (float)g_stPdRectRatio.VpssChn_H;
    #endif
    #ifdef MD_SUPPORT
    APP_PARAM_AI_MD_CFG_S *pstMdCfg = app_ipcam_Ai_MD_Param_Get();
    _NULL_POINTER_CHECK_(pstMdCfg, -1);
    /* set AI MD rect-ratio */
    g_stMdRectRatio.VpssChn_W = pstMdCfg->u32GrpWidth;
    g_stMdRectRatio.VpssChn_H = pstMdCfg->u32GrpHeight;
    g_stMdRectRatio.ScaleX = (float)stOdecSize.u32Width / (float)g_stMdRectRatio.VpssChn_W;
    g_stMdRectRatio.ScaleY = (float)stOdecSize.u32Height / (float)g_stMdRectRatio.VpssChn_H;
    #endif
    /* set AI FD rect-ratio */
    #ifdef FACE_SUPPORT
    APP_PARAM_AI_FD_CFG_S *pstFdCfg = app_ipcam_Ai_FD_Param_Get();
    _NULL_POINTER_CHECK_(pstFdCfg, -1);
    /* set AI FD rect-ratio */
    g_stFdRectRatio.VpssChn_W = pstFdCfg->u32GrpWidth;
    g_stFdRectRatio.VpssChn_H = pstFdCfg->u32GrpHeight;
    g_stFdRectRatio.ScaleX = (float)stOdecSize.u32Width / (float)g_stFdRectRatio.VpssChn_W;
    g_stFdRectRatio.ScaleY = (float)stOdecSize.u32Height / (float)g_stFdRectRatio.VpssChn_H;
    #endif
    #ifdef HUMAN_KEYPOINT_SUPPORT
    APP_PARAM_AI_HUMAN_KEYPOINT_CFG_S *pstHumanKeypointCfg = app_ipcam_Ai_Human_Keypoint_Param_Get();
    _NULL_POINTER_CHECK_(pstHumanKeypointCfg, -1);
    /* set AI Human Keypoint Detect rect-ratio */
    g_stHumanKeypointRectRatio.ScaleX = g_stHumanKeypointRectRatio.ScaleY =
        fmax(((float)stOdecSize.u32Width / (float)pstHumanKeypointCfg->model_size_w), ((float)stOdecSize.u32Height / (float)pstHumanKeypointCfg->model_size_h));
    #endif
    #ifdef OBJECT_TRACK_SUPPORT
    APP_PARAM_AI_OBJECT_TRACK_CFG_S *pstObjTrackCfg = app_ipcam_Ai_Object_Track_Param_Get();
    _NULL_POINTER_CHECK_(pstObjTrackCfg, -1);
    g_stObjectTrackRectRatio.VpssChn_W = pstObjTrackCfg->u32GrpWidth ?
        pstObjTrackCfg->u32GrpWidth : 640;
    g_stObjectTrackRectRatio.VpssChn_H = pstObjTrackCfg->u32GrpHeight ?
        pstObjTrackCfg->u32GrpHeight : 384;
    g_stObjectTrackRectRatio.ScaleX = (float)stOdecSize.u32Width / (float)g_stObjectTrackRectRatio.VpssChn_W;
    g_stObjectTrackRectRatio.ScaleY = (float)stOdecSize.u32Height / (float)g_stObjectTrackRectRatio.VpssChn_H;
    #endif
    return CVI_SUCCESS;
}

static void app_ipcam_AiRectShow_Set(int status)
{
    int showRect = status;
    if (showRect > APP_AI_ALL_RECT_SHOW) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "showRect(%d) > (%d) not legal and draw all rect default\n", showRect, APP_AI_ALL_RECT_SHOW);
        showRect = APP_AI_ALL_RECT_SHOW;
    }

    int iOsdcIndex = 0;

    pthread_mutex_lock(&OsdcMutex);
    for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
        if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
            g_pstOsdcCfg->bShowPdRect[iOsdcIndex] = (APP_AI_PD_RECT_SHOW & showRect);
            g_pstOsdcCfg->bShowMdRect[iOsdcIndex] = (APP_AI_MD_RECT_SHOW & showRect);
            g_pstOsdcCfg->bShowFdRect[iOsdcIndex] = (APP_AI_FD_RECT_SHOW & showRect);
            g_pstOsdcCfg->bShowTrackRect[iOsdcIndex] = (APP_AI_TRACK_RECT_SHOW & showRect);
        }
    }
#ifdef OBJECT_TRACK_SUPPORT
    app_ipcam_Osdc_ObjectTrackRect_ConfigUpdate(g_pstOsdcCfg);
#endif
    pthread_mutex_unlock(&OsdcMutex);
#ifdef PD_SUPPORT
    app_ipcam_Osdc_PdRect_EnsureEventMode();
    app_ipcam_Osdc_PdRect_Publish();
#endif
}

#ifdef OBJECT_TRACK_SUPPORT
static CVI_BOOL app_ipcam_Osdc_ObjectTrackRect_Build(
    const APP_OSDC_TRACK_RECT_STATE_S *pstState,
    VPSS_DRAW_RECT_S *pstDrawRect,
    VPSS_GRP *pVpssGrp, VPSS_CHN *pVpssChn)
{
    CVI_S32 s32X1 = 0;
    CVI_S32 s32Y1 = 0;
    CVI_S32 s32X2 = 0;
    CVI_S32 s32Y2 = 0;
    CVI_U32 u32MinSize = 0;
    CVI_U16 u16Thick = pstState->u32Thickness;

    memset(pstDrawRect, 0, sizeof(*pstDrawRect));
    *pVpssGrp = VPSS_INVALID_GRP;
    *pVpssChn = VPSS_INVALID_CHN;

    if (!pstState->bConfigEnabled) {
        return CVI_FALSE;
    }
    *pVpssGrp = pstState->VpssGrp;
    *pVpssChn = pstState->VpssChn;
    if (!pstState->bStyleEnabled || !pstState->bShow ||
        pstState->u32SourceWidth == 0 || pstState->u32SourceHeight == 0 ||
        pstState->u32OutputWidth == 0 || pstState->u32OutputHeight == 0) {
        return CVI_TRUE;
    }
    if (!isfinite(pstState->fX1) || !isfinite(pstState->fY1) ||
        !isfinite(pstState->fX2) || !isfinite(pstState->fY2)) {
        return CVI_TRUE;
    }

    s32X1 = (CVI_S32)lroundf(pstState->fX1 * pstState->u32OutputWidth /
                             pstState->u32SourceWidth);
    s32Y1 = (CVI_S32)lroundf(pstState->fY1 * pstState->u32OutputHeight /
                             pstState->u32SourceHeight);
    s32X2 = (CVI_S32)lroundf(pstState->fX2 * pstState->u32OutputWidth /
                             pstState->u32SourceWidth);
    s32Y2 = (CVI_S32)lroundf(pstState->fY2 * pstState->u32OutputHeight /
                             pstState->u32SourceHeight);

    s32X1 = fmax(0, fmin(s32X1,
        (CVI_S32)pstState->u32OutputWidth - 1));
    s32Y1 = fmax(0, fmin(s32Y1,
        (CVI_S32)pstState->u32OutputHeight - 1));
    s32X2 = fmax(s32X1 + 1, fmin(s32X2,
        (CVI_S32)pstState->u32OutputWidth));
    s32Y2 = fmax(s32Y1 + 1, fmin(s32Y2,
        (CVI_S32)pstState->u32OutputHeight));

    u32MinSize = fmin(s32X2 - s32X1, s32Y2 - s32Y1);
    if (u32MinSize < 2) {
        return CVI_TRUE;
    }
    if (u32MinSize < 2 * u16Thick) {
        u16Thick = u32MinSize / 2;
    }

    pstDrawRect->astRect[0].bEnable = CVI_TRUE;
    pstDrawRect->astRect[0].u16Thick = u16Thick;
    pstDrawRect->astRect[0].u32BgColor = (pstState->bLost ?
        pstState->u32LostColor : pstState->u32Color) & 0x00ffffff;
    pstDrawRect->astRect[0].stRect.s32X = s32X1;
    pstDrawRect->astRect[0].stRect.s32Y = s32Y1;
    pstDrawRect->astRect[0].stRect.u32Width = s32X2 - s32X1;
    pstDrawRect->astRect[0].stRect.u32Height = s32Y2 - s32Y1;
    return CVI_TRUE;
}

static void *Thread_Osdc_ObjectTrackRect_Draw(void *arg)
{
    APP_OSDC_TRACK_RECT_STATE_S stState = {0};
    VPSS_DRAW_RECT_S stDrawRect = {0};
    VPSS_DRAW_RECT_S stLastDrawRect = {0};
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    VPSS_GRP LastVpssGrp = VPSS_INVALID_GRP;
    VPSS_CHN LastVpssChn = VPSS_INVALID_CHN;
    CVI_U64 u64LastGeneration = (CVI_U64)-1;
    CVI_BOOL bTargetValid = CVI_FALSE;

    (void)arg;
    prctl(PR_SET_NAME, "OSDC_TRACK_RECT", 0, 0, 0);

    while (CVI_TRUE) {
        pthread_mutex_lock(&g_OsdcTrackRectMutex);
        while (g_bOsdcTrackRectThreadRun &&
               u64LastGeneration == g_stOsdcTrackRectState.u64Generation) {
            pthread_cond_wait(&g_OsdcTrackRectCond, &g_OsdcTrackRectMutex);
        }
        if (!g_bOsdcTrackRectThreadRun) {
            pthread_mutex_unlock(&g_OsdcTrackRectMutex);
            break;
        }
        stState = g_stOsdcTrackRectState;
        u64LastGeneration = stState.u64Generation;
        pthread_mutex_unlock(&g_OsdcTrackRectMutex);

        bTargetValid = app_ipcam_Osdc_ObjectTrackRect_Build(
            &stState, &stDrawRect, &VpssGrp, &VpssChn);
        if (!bTargetValid) {
            if (LastVpssGrp != VPSS_INVALID_GRP &&
                LastVpssChn != VPSS_INVALID_CHN &&
                stLastDrawRect.astRect[0].bEnable) {
                memset(&stLastDrawRect, 0, sizeof(stLastDrawRect));
                CVI_VPSS_SetChnDrawRect(
                    LastVpssGrp, LastVpssChn, &stLastDrawRect);
            }
            continue;
        }
        if (VpssGrp == LastVpssGrp && VpssChn == LastVpssChn &&
            memcmp(&stDrawRect, &stLastDrawRect, sizeof(stDrawRect)) == 0) {
            continue;
        }

        if (CVI_VPSS_SetChnDrawRect(VpssGrp, VpssChn, &stDrawRect) !=
            CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,
                "ObjectTrack hardware OSD update failed, grp=%d chn=%d\n",
                VpssGrp, VpssChn);
            continue;
        }
        stLastDrawRect = stDrawRect;
        LastVpssGrp = VpssGrp;
        LastVpssChn = VpssChn;
    }

    memset(&stDrawRect, 0, sizeof(stDrawRect));
    if (LastVpssGrp != VPSS_INVALID_GRP && LastVpssChn != VPSS_INVALID_CHN) {
        CVI_VPSS_SetChnDrawRect(LastVpssGrp, LastVpssChn, &stDrawRect);
    }
    return NULL;
}
#endif

void *Thread_Osdc_Draw(void *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U64 u64LastGeneration = (CVI_U64)-1;
    prctl(PR_SET_NAME, "OSDC_DRAW", 0, 0, 0);
    int iOsdcIndex = 0;

    while (CVI_TRUE) {
        int iWaitRet = 0;

        pthread_mutex_lock(&g_OsdcWakeMutex);
        while (g_bOsdcThreadRun &&
               u64LastGeneration == g_u64OsdcWakeGeneration &&
               iWaitRet != ETIMEDOUT) {
            struct timespec stDeadline;
            CVI_U32 u32RefreshUs = APP_OSDC_REFRESH_US;

#if defined(AI_SUPPORT) && defined(PD_SUPPORT)
            if (app_ipcam_Ai_PD_ProcStatus_Get()) {
                u32RefreshUs = APP_OSDC_PD_IDLE_REFRESH_US;
            }
#endif

            clock_gettime(CLOCK_REALTIME, &stDeadline);
            stDeadline.tv_nsec += u32RefreshUs * 1000L;
            if (stDeadline.tv_nsec >= 1000000000L) {
                stDeadline.tv_sec += stDeadline.tv_nsec / 1000000000L;
                stDeadline.tv_nsec %= 1000000000L;
            }
            iWaitRet = pthread_cond_timedwait(
                &g_OsdcWakeCond, &g_OsdcWakeMutex, &stDeadline);
        }
        if (!g_bOsdcThreadRun) {
            pthread_mutex_unlock(&g_OsdcWakeMutex);
            break;
        }
        u64LastGeneration = g_u64OsdcWakeGeneration;
        pthread_mutex_unlock(&g_OsdcWakeMutex);

        pthread_mutex_lock(&OsdcMutex);
        for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
            if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
                RGN_HANDLE OsdcHandle = g_pstOsdcCfg->handle[iOsdcIndex];
                s32Ret = app_ipcam_ObjsRectInfo_Update(OsdcHandle, iOsdcIndex);
                if (s32Ret != CVI_SUCCESS &&
                    s32Ret != CVI_ERR_RGN_BUSY) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"app_ipcam_ObjsRectInfo_Update failed with %#x!\n", s32Ret);
                }
            }
        }
        pthread_mutex_unlock(&OsdcMutex);
    }

    return NULL;
}

int app_ipcam_Osdc_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

#ifdef PD_SUPPORT
    app_ipcam_Osdc_PdRect_StopEventMode();
#endif

    if (!g_pstOsdcCfg->enable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "draw Osdc thread not enable!\n");
        return CVI_SUCCESS;
    }

    if (g_stOsdcCanvasCfg.createCanvas == CVI_FALSE) {
        s32Ret = app_ipcam_OSDCRgn_Create();
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"app_ipcam_OSDCRgn_Create failed with %#x!\n", s32Ret);
            return CVI_FAILURE;
        }
        g_stOsdcCanvasCfg.createCanvas = CVI_TRUE;
        APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_OSDCRgn_Created!\n");
    }

    /* calculate AI Rect ratio betwen streaming and AI size */
    APP_IPCAM_CHECK_RET(app_ipcam_ObjRectRatio_Set(), "OSDC OBJ RATIO RECT SET");

    /* Submit static overlays before VENC starts consuming VPSS frames. */
    s32Ret = app_ipcam_Osdc_FirstCanvas();
    if (s32Ret != CVI_SUCCESS) {
        app_ipcam_OSDCRgn_Destory();
        g_stOsdcCanvasCfg.createCanvas = CVI_FALSE;
        return s32Ret;
    }

    pthread_mutex_lock(&g_OsdcWakeMutex);
    g_bOsdcThreadRun = CVI_TRUE;
    g_u64OsdcWakeGeneration++;
    pthread_mutex_unlock(&g_OsdcWakeMutex);
    s32Ret = pthread_create(
                &g_pthOsdcRgn,
                NULL,
                Thread_Osdc_Draw,
                (CVI_VOID *)g_pstOsdcCfg);
    if (s32Ret != 0) {
        pthread_mutex_lock(&g_OsdcWakeMutex);
        g_bOsdcThreadRun = CVI_FALSE;
        pthread_mutex_unlock(&g_OsdcWakeMutex);
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pthread_create failed!\n");
        return CVI_FAILURE;
    }

#ifdef PD_SUPPORT
    if (app_ipcam_Osdc_PdRect_EnsureEventMode() != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,
            "DET event OSD unavailable, use canvas fallback\n");
    }
#endif

#ifdef OBJECT_TRACK_SUPPORT
    app_ipcam_Osdc_ObjectTrackRect_ConfigUpdate(g_pstOsdcCfg);
    pthread_mutex_lock(&g_OsdcTrackRectMutex);
    g_bOsdcTrackRectThreadRun = CVI_TRUE;
    g_stOsdcTrackRectState.u64Generation++;
    pthread_mutex_unlock(&g_OsdcTrackRectMutex);
    s32Ret = pthread_create(
                &g_pthOsdcTrackRect,
                NULL,
                Thread_Osdc_ObjectTrackRect_Draw,
                NULL);
    if (s32Ret != 0) {
        pthread_mutex_lock(&g_OsdcTrackRectMutex);
        g_bOsdcTrackRectThreadRun = CVI_FALSE;
        pthread_mutex_unlock(&g_OsdcTrackRectMutex);
#ifdef PD_SUPPORT
        app_ipcam_Osdc_PdRect_StopEventMode();
#endif
        pthread_mutex_lock(&g_OsdcWakeMutex);
        g_bOsdcThreadRun = CVI_FALSE;
        g_u64OsdcWakeGeneration++;
        pthread_cond_broadcast(&g_OsdcWakeCond);
        pthread_mutex_unlock(&g_OsdcWakeMutex);
        pthread_join(g_pthOsdcRgn, NULL);
        g_pthOsdcRgn = 0;
        app_ipcam_OSDCRgn_Destory();
        g_stOsdcCanvasCfg.createCanvas = CVI_FALSE;
        APP_PROF_LOG_PRINT(LEVEL_ERROR,
            "create ObjectTrack hardware OSD thread failed!\n");
        return CVI_FAILURE;
    }
#endif

    return CVI_SUCCESS;
}

int app_ipcam_Osdc_DeInit(void)
{
    // CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 iTime = GetCurTimeInMsec();

#ifdef PD_SUPPORT
    app_ipcam_Osdc_PdRect_StopEventMode();
#endif

    if (!g_pstOsdcCfg->enable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "draw Osdc thread not enable!\n");
        return CVI_SUCCESS;
    }

    if (!g_stOsdcCanvasCfg.createCanvas) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "draw Osdc Canvas not create!\n");
        return CVI_SUCCESS;
    }

#ifdef OBJECT_TRACK_SUPPORT
    pthread_mutex_lock(&g_OsdcTrackRectMutex);
    g_bOsdcTrackRectThreadRun = CVI_FALSE;
    g_stOsdcTrackRectState.bShow = CVI_FALSE;
    g_stOsdcTrackRectState.u64Generation++;
    pthread_cond_signal(&g_OsdcTrackRectCond);
    pthread_mutex_unlock(&g_OsdcTrackRectMutex);
    if (g_pthOsdcTrackRect > (pthread_t)0) {
        pthread_join(g_pthOsdcTrackRect, NULL);
        g_pthOsdcTrackRect = 0;
    }
#endif

    pthread_mutex_lock(&g_OsdcWakeMutex);
    g_bOsdcThreadRun = CVI_FALSE;
    g_u64OsdcWakeGeneration++;
    pthread_cond_broadcast(&g_OsdcWakeCond);
    pthread_mutex_unlock(&g_OsdcWakeMutex);
    if (g_pthOsdcRgn > (pthread_t)0) {
        pthread_join(g_pthOsdcRgn, NULL);
        g_pthOsdcRgn = 0;
    }
    APP_PROF_LOG_PRINT(LEVEL_WARN, "waiting osdc thread exit takes %u ms \n", (GetCurTimeInMsec() - iTime));

    app_ipcam_OSDCRgn_Destory();

    g_stOsdcCanvasCfg.createCanvas = CVI_FALSE;

#ifdef OBJECT_TRACK_SUPPORT
    free(g_objMetaObjectTrack.info);
    g_objMetaObjectTrack.info = NULL;
#endif
#ifdef PD_SUPPORT
    free(g_objMetaPd.info);
    g_objMetaPd.info = NULL;
    g_objMetaPd.size = 0;
#endif

    return CVI_SUCCESS;
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/

void app_ipcam_Osdc_Status(APP_PARAM_OSDC_CFG_S *pstOsdcCfg)
{
    CVI_BOOL bOldEnable;
    CVI_U64 au64BitmapPhyAddr[OSDC_NUM_MAX][OSDC_OBJS_MAX] = {{0}};
    CVI_VOID *apBitmapVirAddr[OSDC_NUM_MAX][OSDC_OBJS_MAX] = {{0}};
    CVI_U32 au32BitmapMaxLen[OSDC_NUM_MAX][OSDC_OBJS_MAX] = {{0}};
    CVI_U32 i;
    CVI_U32 j;

    if (NULL == pstOsdcCfg) {
        return ;
    }
    bOldEnable = g_stOsdcCfg.enable;
    if (bOldEnable != pstOsdcCfg->enable) {
        if (bOldEnable) {
            app_ipcam_Osdc_DeInit();
        }
        pthread_mutex_lock(&OsdcMutex);
        memcpy(&g_stOsdcCfg, pstOsdcCfg, sizeof(g_stOsdcCfg));
        pthread_mutex_unlock(&OsdcMutex);
        if (g_stOsdcCfg.enable) {
            app_ipcam_Osdc_Init();
        }
        return;
    }

    pthread_mutex_lock(&OsdcMutex);
    if (bOldEnable) {
        for (i = 0; i < OSDC_NUM_MAX; i++) {
            for (j = 0; j < OSDC_OBJS_MAX; j++) {
                au64BitmapPhyAddr[i][j] =
                    g_stOsdcCfg.osdcObj[i][j].u64BitmapPhyAddr;
                apBitmapVirAddr[i][j] =
                    g_stOsdcCfg.osdcObj[i][j].pBitmapVirAddr;
                au32BitmapMaxLen[i][j] =
                    g_stOsdcCfg.osdcObj[i][j].maxlen;
            }
        }
    }
    memcpy(&g_stOsdcCfg, pstOsdcCfg, sizeof(g_stOsdcCfg));
    if (bOldEnable) {
        for (i = 0; i < OSDC_NUM_MAX; i++) {
            for (j = 0; j < OSDC_OBJS_MAX; j++) {
                g_stOsdcCfg.osdcObj[i][j].u64BitmapPhyAddr =
                    au64BitmapPhyAddr[i][j];
                g_stOsdcCfg.osdcObj[i][j].pBitmapVirAddr =
                    apBitmapVirAddr[i][j];
                g_stOsdcCfg.osdcObj[i][j].maxlen =
                    au32BitmapMaxLen[i][j];
            }
        }
#ifdef OBJECT_TRACK_SUPPORT
        app_ipcam_Osdc_ObjectTrackRect_ConfigUpdate(&g_stOsdcCfg);
#endif
    }
    pthread_mutex_unlock(&OsdcMutex);
    if (bOldEnable) {
#ifdef PD_SUPPORT
        app_ipcam_Osdc_PdRect_EnsureEventMode();
        app_ipcam_Osdc_PdRect_Publish();
#else
        app_ipcam_Osdc_Wake();
#endif
    }
}

CVI_S32 app_ipcam_Osdc_Reload(const char *pszConfigPath)
{
    APP_PARAM_OSDC_CFG_S stOsdcCfg;

    if (pszConfigPath == NULL ||
        Load_Param_Osdc_Config(pszConfigPath, &stOsdcCfg) != CVI_SUCCESS) {
        return CVI_FAILURE;
    }
    app_ipcam_Osdc_Status(&stOsdcCfg);
    return CVI_SUCCESS;
}



int app_ipcam_CmdTask_Rect_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    CVI_CHAR param[512] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s arg2=%d\n", __FUNCTION__, param, msg->arg2);

    CVI_CHAR *temp = strtok(param, ":");

    APP_PROF_LOG_PRINT(LEVEL_WARN, "%s temp=%c \n", __FUNCTION__, *temp);
    while(NULL != temp) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp) {
            case 's':
                {
                    temp = strtok(NULL, "/");
                    int flag = atoi(temp);
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "%s flag=0x%x \n", __FUNCTION__, flag);
                    app_ipcam_AiRectShow_Set(flag);
                }
                break;

            default:
                return 0;
                break;
        }

        temp = strtok(NULL, ":");
    }

    return CVI_SUCCESS;
}
/*****************************************************************
 *  The above API for command test used                 End
 * **************************************************************/
