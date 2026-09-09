#ifndef __APP_IPCAM_OSD_H__
#define __APP_IPCAM_OSD_H__

#include <stdbool.h>
#include "cvi_type.h"
#include "cvi_comm_region.h"
#include "app_ipcam_mq.h"
#include "cvi_region.h"






#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_FILE_LEN    32
#define DEBUG_STR_LEN   32
#define APP_OSD_STR_LEN_MAX     64
#define OSDC_OBJS_MAX 128
#define OSDC_NUM_MAX 3
#define OSDC_AI_STR_MAX 20

#define APP_OSD_COLOR_WHITE_RGB       0xFFFFFF
#define APP_OSD_COLOR_CYAN_RGB        0x00D9FF
#define APP_OSD_COLOR_AMBER_RGB       0xFFB000
#define APP_OSD_COLOR_READY_RGB       0xFFC247
#define APP_OSD_COLOR_LOST_RGB        0xFF3030

#define COLOR_WHITE(FORMATE)  ((FORMATE) ? 0xFFFFFFFF : 0xFFFF)
#define COLOR_BLACK(FORMATE)  ((FORMATE) ? 0xFF000000 : 0x8000)
#define COLOR_BLUE(FORMATE)   ((FORMATE) ? 0xFF0000FF : 0x801F)
#define COLOR_GREEN(FORMATE)  ((FORMATE) ? 0xFF00FF00 : 0x83E0)
#define COLOR_RED(FORMATE)    ((FORMATE) ? 0xFFFF0000 : 0xFC00)
#define COLOR_CYAN(FORMATE)   ((FORMATE) ? 0xFF00FFFF : 0x83FF)
#define COLOR_YELLOW(FORMATE) ((FORMATE) ? 0xFFFFFF00 : 0xFFE0)
#define COLOR_PINK(FORMATE)   ((FORMATE) ? 0xFFFF00FF : 0xFC1F)

#define IDX_TO_COLOR(FORMATE, IDX, COLOR)  do {         \
    if ((IDX) == 0) (COLOR) = COLOR_WHITE(FORMATE);       \
    else if ((IDX) == 1) (COLOR) = COLOR_WHITE(FORMATE);  \
    else if ((IDX) == 2) (COLOR) = COLOR_BLACK(FORMATE);  \
    else if ((IDX) == 3) (COLOR) = COLOR_BLUE(FORMATE);  \
    else if ((IDX) == 4) (COLOR) = COLOR_GREEN(FORMATE);  \
    else if ((IDX) == 5) (COLOR) = COLOR_RED(FORMATE);  \
    else if ((IDX) == 6) (COLOR) = COLOR_CYAN(FORMATE);  \
    else if ((IDX) == 7) (COLOR) = COLOR_YELLOW(FORMATE);  \
    else if ((IDX) == 8) (COLOR) = COLOR_PINK(FORMATE);  \
} while(0)

#define COLOR_TO_IDX(FORMATE, IDX, COLOR)  do {         \
    if ((COLOR) == COLOR_WHITE(FORMATE)) (IDX) = 0;       \
    else if ((COLOR) == COLOR_WHITE(FORMATE)) (IDX) = 1;  \
    else if ((COLOR) == COLOR_BLACK(FORMATE)) (IDX) = 2;  \
    else if ((COLOR) == COLOR_BLUE(FORMATE)) (IDX) = 3;  \
    else if ((COLOR) == COLOR_GREEN(FORMATE)) (IDX) = 4;  \
    else if ((COLOR) == COLOR_RED(FORMATE)) (IDX) = 5;  \
    else if ((COLOR) == COLOR_CYAN(FORMATE)) (IDX) = 6;  \
    else if ((COLOR) == COLOR_YELLOW(FORMATE)) (IDX) = 7;  \
    else if ((COLOR) == COLOR_PINK(FORMATE)) (IDX) = 8;  \
} while(0)

typedef enum OSD_TYPE_T {
    TYPE_PICTURE,
    TYPE_STRING,
    TYPE_TIME,
    TYPE_DEBUG,
    TYPE_END
} OSD_TYPE_E;

typedef enum APP_OSD_COLOR_MODE_T {
    APP_OSD_COLOR_MODE_FIXED = 0,
    APP_OSD_COLOR_MODE_MODEL,
    APP_OSD_COLOR_MODE_BUTT
} APP_OSD_COLOR_MODE_E;

typedef enum APP_OSD_LABEL_MODE_T {
    APP_OSD_LABEL_MODE_NONE = 0,
    APP_OSD_LABEL_MODE_CLASS,
    APP_OSD_LABEL_MODE_CLASS_SCORE,
    APP_OSD_LABEL_MODE_BUTT
} APP_OSD_LABEL_MODE_E;

typedef enum APP_OSD_RETICLE_TEMPLATE_T {
    APP_OSD_RETICLE_RECTANGLE = 0,
    APP_OSD_RETICLE_CORNERS,
    APP_OSD_RETICLE_CROSSHAIR,
    APP_OSD_RETICLE_CROSSHAIR_DOT,
    APP_OSD_RETICLE_BRACKET_CROSS,
    APP_OSD_RETICLE_CIRCLE,
    APP_OSD_RETICLE_TEMPLATE_BUTT
} APP_OSD_RETICLE_TEMPLATE_E;

typedef enum APP_OSD_TEXT_POSITION_T {
    APP_OSD_TEXT_POSITION_CUSTOM = 0,
    APP_OSD_TEXT_POSITION_TOP_LEFT,
    APP_OSD_TEXT_POSITION_TOP_RIGHT,
    APP_OSD_TEXT_POSITION_BOTTOM_LEFT,
    APP_OSD_TEXT_POSITION_BOTTOM_RIGHT,
    APP_OSD_TEXT_POSITION_BUTT
} APP_OSD_TEXT_POSITION_E;

typedef struct APP_OSD_STYLE_CFG_T {
    CVI_BOOL bDetectionEnable;
    APP_OSD_COLOR_MODE_E enDetectionColorMode;
    CVI_U32 u32DetectionColor;
    CVI_U32 u32DetectionThickness;
    APP_OSD_LABEL_MODE_E enDetectionLabelMode;
    CVI_BOOL bTrackingEnable;
    CVI_U32 u32TrackingColor;
    CVI_U32 u32TrackingLostColor;
    CVI_U32 u32TrackingThickness;
    CVI_BOOL bTrackingCorners;
    CVI_BOOL bTrackingHideWhenLost;
    CVI_BOOL bReticleEnable;
    APP_OSD_RETICLE_TEMPLATE_E enReticleTemplate;
    CVI_U32 u32ReticleIdleColor;
    CVI_U32 u32ReticleReadyColor;
    CVI_U32 u32ReticleThickness;
    CVI_BOOL bReticleShowWhileTracking;
    APP_OSD_TEXT_POSITION_E enTextPosition;
} APP_OSD_STYLE_CFG_S;

typedef struct APP_OSDC_OBJS_AI_STR_INFO_T
{
    CVI_U64 u64BitmapPhyAddr[OSDC_AI_STR_MAX];
    CVI_VOID *pBitmapVirAddr[OSDC_AI_STR_MAX];
    CVI_S32 maxlen[OSDC_AI_STR_MAX];
    CVI_U32 ai_str_num;
}APP_OSDC_OBJS_AI_STR_INFO_S;

typedef struct APP_OSDC_OBJS_INFO_T {
    CVI_BOOL bShow;
    RGN_CMPR_TYPE_E type;
    CVI_U32 color;
    CVI_U32 x1;
    CVI_U32 y1;
    CVI_U32 x2;
    CVI_U32 y2;
    CVI_U32 width;
    CVI_U32 height;
    CVI_BOOL filled;
    CVI_S32 thickness;
    OSD_TYPE_E enType;
    CVI_S32 maxlen;
    union {
        char filename[MAX_FILE_LEN];
        char str[APP_OSD_STR_LEN_MAX];
    };
    CVI_U64 u64BitmapPhyAddr;
    CVI_VOID *pBitmapVirAddr;
} APP_OSDC_OBJS_INFO_S;

typedef struct APP_PARAM_OSDC_CFG_T {
    CVI_BOOL enable;
    APP_OSD_STYLE_CFG_S stStyle;
    RGN_HANDLE handle[OSDC_NUM_MAX];
    MMF_CHN_S  mmfChn[OSDC_NUM_MAX];
    CVI_BOOL bShow[OSDC_NUM_MAX];
    CVI_U32 VpssGrp[OSDC_NUM_MAX];
    CVI_U32 VpssChn[OSDC_NUM_MAX];
    CVI_U32 CompressedSize[OSDC_NUM_MAX];
    PIXEL_FORMAT_E format[OSDC_NUM_MAX];
    CVI_BOOL bShowPdRect[OSDC_NUM_MAX];
    CVI_BOOL bShowMdRect[OSDC_NUM_MAX];
    CVI_BOOL bShowFdRect[OSDC_NUM_MAX];
    CVI_BOOL bShowTrackRect[OSDC_NUM_MAX];
    CVI_BOOL bShowHumanKeypointRect[OSDC_NUM_MAX];
    CVI_U32 osdcObjNum[OSDC_NUM_MAX];
    APP_OSDC_OBJS_INFO_S osdcObj[OSDC_NUM_MAX][OSDC_OBJS_MAX];
} APP_PARAM_OSDC_CFG_S;



APP_PARAM_OSDC_CFG_S *app_ipcam_Osdc_Param_Get(void);
int app_ipcam_Osdc_Init(void);
int app_ipcam_Osdc_DeInit(void);

#ifdef OBJECT_TRACK_SUPPORT
CVI_VOID app_ipcam_Osdc_ObjectTrackRect_Publish(
    CVI_BOOL bShow, CVI_BOOL bLost, CVI_FLOAT fX1, CVI_FLOAT fY1,
    CVI_FLOAT fX2, CVI_FLOAT fY2,
    CVI_U32 u32SourceWidth, CVI_U32 u32SourceHeight);
#endif

#ifdef PD_SUPPORT
CVI_S32 app_ipcam_Osdc_PdRect_EnsureEventMode(CVI_VOID);
CVI_VOID app_ipcam_Osdc_PdRect_Publish(CVI_VOID);
#endif

#ifdef WEB_SOCKET
APP_OSDC_OBJS_INFO_S *app_ipcam_OsdcPrivacy_Param_Get(void);
#endif
/*****************************************************************
 *  The following API for command test used             S
 * **************************************************************/
void app_ipcam_Osdc_Status(APP_PARAM_OSDC_CFG_S *pstOsdcCfg);
CVI_S32 app_ipcam_Osdc_Reload(const char *pszConfigPath);
int app_ipcam_CmdTask_Rect_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
/*****************************************************************
 *  The above API for command test used                 E
 * **************************************************************/

#ifdef __cplusplus
}
#endif

#endif
