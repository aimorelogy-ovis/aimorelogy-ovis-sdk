#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"


const char *rgn_cmpr_type[RGN_CMPR_BUTT] = {
    [RGN_CMPR_RECT] = "RGN_CMPR_RECT",
    [RGN_CMPR_BIT_MAP] = "RGN_CMPR_BIT_MAP",
    [RGN_CMPR_LINE] = "RGN_CMPR_LINE"
};

const char *osd_type[TYPE_END] = {
    [TYPE_PICTURE] = "TYPE_PICTURE",
    [TYPE_STRING] = "TYPE_STRING",
    [TYPE_TIME] = "TYPE_TIME",
    [TYPE_DEBUG] = "TYPE_DEBUG"
};

static const char *osd_color_mode[APP_OSD_COLOR_MODE_BUTT] = {
    [APP_OSD_COLOR_MODE_FIXED] = "fixed",
    [APP_OSD_COLOR_MODE_MODEL] = "model"
};

static const char *osd_label_mode[APP_OSD_LABEL_MODE_BUTT] = {
    [APP_OSD_LABEL_MODE_NONE] = "none",
    [APP_OSD_LABEL_MODE_CLASS] = "class",
    [APP_OSD_LABEL_MODE_CLASS_SCORE] = "class_score"
};

static const char *osd_reticle_template[APP_OSD_RETICLE_TEMPLATE_BUTT] = {
    [APP_OSD_RETICLE_RECTANGLE] = "rectangle",
    [APP_OSD_RETICLE_CORNERS] = "corners",
    [APP_OSD_RETICLE_CROSSHAIR] = "crosshair",
    [APP_OSD_RETICLE_CROSSHAIR_DOT] = "crosshair_dot",
    [APP_OSD_RETICLE_BRACKET_CROSS] = "bracket_cross",
    [APP_OSD_RETICLE_CIRCLE] = "circle"
};

static const char *osd_text_position[APP_OSD_TEXT_POSITION_BUTT] = {
    [APP_OSD_TEXT_POSITION_CUSTOM] = "custom",
    [APP_OSD_TEXT_POSITION_TOP_LEFT] = "top-left",
    [APP_OSD_TEXT_POSITION_TOP_RIGHT] = "top-right",
    [APP_OSD_TEXT_POSITION_BOTTOM_LEFT] = "bottom-left",
    [APP_OSD_TEXT_POSITION_BOTTOM_RIGHT] = "bottom-right"
};

static void app_ipcam_Osdc_Style_Default(APP_OSD_STYLE_CFG_S *Style)
{
    memset(Style, 0, sizeof(*Style));
    Style->bDetectionEnable = CVI_TRUE;
    Style->enDetectionColorMode = APP_OSD_COLOR_MODE_FIXED;
    Style->u32DetectionColor = APP_OSD_COLOR_CYAN_RGB;
    Style->u32DetectionThickness = 2;
    Style->enDetectionLabelMode = APP_OSD_LABEL_MODE_NONE;
    Style->bTrackingEnable = CVI_TRUE;
    Style->u32TrackingColor = APP_OSD_COLOR_AMBER_RGB;
    Style->u32TrackingLostColor = APP_OSD_COLOR_LOST_RGB;
    Style->u32TrackingThickness = 3;
    Style->bReticleEnable = CVI_TRUE;
    Style->enReticleTemplate = APP_OSD_RETICLE_CORNERS;
    Style->u32ReticleIdleColor = APP_OSD_COLOR_WHITE_RGB;
    Style->u32ReticleReadyColor = APP_OSD_COLOR_READY_RGB;
    Style->u32ReticleThickness = 2;
    Style->bReticleShowWhileTracking = CVI_FALSE;
    Style->enTextPosition = APP_OSD_TEXT_POSITION_TOP_LEFT;
}

static int app_ipcam_Osdc_ReadEnum(const char *file, const char *key,
    const char *default_value, const char *values[], int value_count,
    int default_result)
{
    char value[PARAM_STRING_NAME_LEN] = {0};
    int parsed = default_result;

    ini_gets("osd_style", key, default_value, value, sizeof(value), file);
    app_ipcam_Param_Convert_StrName_to_EnumNum(
        value, values, value_count, &parsed);
    return parsed;
}

int Load_Param_Osdc_Config(const char *file, APP_PARAM_OSDC_CFG_S *Osdc)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Osdc config ------------------> start \n");
    unsigned int i = 0;
    unsigned int j = 0;
    int enum_num = 0;
    int ret = 0;
    char tmp_buff[APP_OSD_STR_LEN_MAX] = {0};
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    const char ** mode_id = app_ipcam_Param_get_mode_id();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();

    if (file == NULL || Osdc == NULL) {
        return CVI_FAILURE;
    }

    memset(Osdc, 0, sizeof(APP_PARAM_OSDC_CFG_S));
    app_ipcam_Osdc_Style_Default(&Osdc->stStyle);
    Osdc->enable = ini_getl("osdc_config", "enable", 0, file);
    Osdc->stStyle.bDetectionEnable = ini_getl("osd_style", "detection_enabled", 1, file);
    Osdc->stStyle.enDetectionColorMode = app_ipcam_Osdc_ReadEnum(
        file, "detection_color_mode", "fixed", osd_color_mode,
        APP_OSD_COLOR_MODE_BUTT, APP_OSD_COLOR_MODE_FIXED);
    Osdc->stStyle.u32DetectionColor = ini_getl("osd_style", "detection_color",
        APP_OSD_COLOR_CYAN_RGB, file);
    Osdc->stStyle.u32DetectionThickness = ini_getl("osd_style", "detection_thickness", 2, file);
    Osdc->stStyle.enDetectionLabelMode = app_ipcam_Osdc_ReadEnum(
        file, "detection_label_mode", "none", osd_label_mode,
        APP_OSD_LABEL_MODE_BUTT, APP_OSD_LABEL_MODE_NONE);
    Osdc->stStyle.bTrackingEnable = ini_getl("osd_style", "tracking_enabled", 1, file);
    Osdc->stStyle.u32TrackingColor = ini_getl("osd_style", "tracking_color",
        APP_OSD_COLOR_AMBER_RGB, file);
    Osdc->stStyle.u32TrackingLostColor = ini_getl("osd_style", "tracking_lost_color",
        APP_OSD_COLOR_LOST_RGB, file);
    Osdc->stStyle.u32TrackingThickness = ini_getl("osd_style", "tracking_thickness", 3, file);
    Osdc->stStyle.bReticleEnable = ini_getl("osd_style", "reticle_enabled", 1, file);
    Osdc->stStyle.enReticleTemplate = app_ipcam_Osdc_ReadEnum(
        file, "reticle_template", "corners", osd_reticle_template,
        APP_OSD_RETICLE_TEMPLATE_BUTT, APP_OSD_RETICLE_CORNERS);
    Osdc->stStyle.u32ReticleIdleColor = ini_getl("osd_style", "reticle_idle_color",
        APP_OSD_COLOR_WHITE_RGB, file);
    Osdc->stStyle.u32ReticleReadyColor = ini_getl("osd_style", "reticle_ready_color",
        APP_OSD_COLOR_READY_RGB, file);
    Osdc->stStyle.u32ReticleThickness = ini_getl("osd_style", "reticle_thickness", 2, file);
    Osdc->stStyle.bReticleShowWhileTracking = ini_getl(
        "osd_style", "reticle_show_while_tracking", 0, file);
    Osdc->stStyle.enTextPosition = app_ipcam_Osdc_ReadEnum(
        file, "text_position", "top-left", osd_text_position,
        APP_OSD_TEXT_POSITION_BUTT, APP_OSD_TEXT_POSITION_TOP_LEFT);
    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "osdc enable: %d\n", Osdc->enable);
    if (Osdc->enable) {
        for (j = 0; j < OSDC_NUM_MAX; j++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "osdc_config%d", j);
            Osdc->bShow[j]          = ini_getl(tmp_section, "bShow", 0, file);
            Osdc->handle[j]         = ini_getl(tmp_section, "handle", 0, file);
            Osdc->VpssGrp[j]        = ini_getl(tmp_section, "vpss_grp", 0, file);
            Osdc->VpssChn[j]        = ini_getl(tmp_section, "vpss_chn", 0, file);
            Osdc->CompressedSize[j] = ini_getl(tmp_section, "compressedsize", 0, file);

            ini_gets(tmp_section, "format", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][format] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][format] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Osdc->format[j] = enum_num;
            }

            ini_gets(tmp_section, "mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Osdc->mmfChn[j].enModId = enum_num;
            }

            Osdc->mmfChn[j].s32DevId = ini_getl(tmp_section, "dev_id", 0, file);
            Osdc->mmfChn[j].s32ChnId = ini_getl(tmp_section, "chn_id", 0, file);
            Osdc->bShowPdRect[j]     = ini_getl(tmp_section, "show_pd_rect", 0, file);
            Osdc->bShowMdRect[j]     = ini_getl(tmp_section, "show_md_rect", 0, file);
            Osdc->bShowFdRect[j]     = ini_getl(tmp_section, "show_fd_rect", 0, file);
            Osdc->bShowHumanKeypointRect[j] = ini_getl(tmp_section, "show_human_keypoint_rect", j == 0 ? 1 : 0, file);
            Osdc->bShowTrackRect[j] = ini_getl(tmp_section, "show_track_rect", j == 0 ? 1 : 0, file);
            Osdc->osdcObjNum[j]      = ini_getl(tmp_section, "cnt", 0, file);
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "handle=%d bShow=%d Format=0x%x cpsSize=%d ModeId=%d DevId=%d ChnId=%d PdRect=%d MdRect=%d FdRect=%d osdcObjNum=%d\n",
                Osdc->handle[j], Osdc->bShow[j], Osdc->format[j], Osdc->CompressedSize[j], Osdc->mmfChn[j].enModId,
                Osdc->mmfChn[j].s32DevId, Osdc->mmfChn[j].s32ChnId, Osdc->bShowPdRect[j], Osdc->bShowMdRect[j], Osdc->bShowFdRect[j], Osdc->osdcObjNum[j]);

            for (i = 0; i < Osdc->osdcObjNum[j]; i++) {
                memset(tmp_section, 0, sizeof(tmp_section));
                snprintf(tmp_section, sizeof(tmp_section), "osdc%d_obj_info%d", j, i);
                Osdc->osdcObj[j][i].bShow      = ini_getl(tmp_section, "bShow", 0, file);
                ini_gets(tmp_section, "type", " ", str_name, PARAM_STRING_NAME_LEN, file);
                ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, rgn_cmpr_type, RGN_CMPR_BUTT, &enum_num);
                if (ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][type] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][type] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                    Osdc->osdcObj[j][i].type = enum_num;
                }

                Osdc->osdcObj[j][i].color = ini_getl(tmp_section, "color", 0, file);
                Osdc->osdcObj[j][i].x1    = ini_getl(tmp_section, "x1", 0, file);
                Osdc->osdcObj[j][i].y1    = ini_getl(tmp_section, "y1", 0, file);
                if (RGN_CMPR_BIT_MAP == Osdc->osdcObj[j][i].type) {
                    ini_gets(tmp_section, "entype", " ", str_name, PARAM_STRING_NAME_LEN, file);
                    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, osd_type, TYPE_END, &enum_num);
                    if (ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][entype] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
                    } else {
                        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%s][entype] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                        Osdc->osdcObj[j][i].enType = enum_num;
                    }

                    if (TYPE_STRING == Osdc->osdcObj[j][i].enType) {
                        ini_gets(tmp_section, "str", " ", tmp_buff, APP_OSD_STR_LEN_MAX, file);
                        app_ipcam_Param_CopyString(Osdc->osdcObj[j][i].str,
                            sizeof(Osdc->osdcObj[j][i].str), tmp_buff);
                    } else if (TYPE_PICTURE == Osdc->osdcObj[j][i].enType) {
                        ini_gets(tmp_section, "file_name", " ", tmp_buff, APP_OSD_STR_LEN_MAX, file);
                        app_ipcam_Param_CopyString(Osdc->osdcObj[j][i].filename,
                            sizeof(Osdc->osdcObj[j][i].filename), tmp_buff);
                    }
                } else {
                    if (RGN_CMPR_LINE == Osdc->osdcObj[j][i].type) {
                        Osdc->osdcObj[j][i].x2     = ini_getl(tmp_section, "x2", 0, file);
                        Osdc->osdcObj[j][i].y2     = ini_getl(tmp_section, "y2", 0, file);
                    } else if (RGN_CMPR_RECT == Osdc->osdcObj[j][i].type) {
                        Osdc->osdcObj[j][i].width  = ini_getl(tmp_section, "width", 0, file);
                        Osdc->osdcObj[j][i].height = ini_getl(tmp_section, "height", 0, file);
                    }
                    Osdc->osdcObj[j][i].filled     = ini_getl(tmp_section, "filled", 0, file);
                    Osdc->osdcObj[j][i].thickness  = ini_getl(tmp_section, "thickness", 0, file);
                }

                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "type=%d color=0x%x x1=%d y1=%d x2=%d y2=%d width=%d height=%d filled=%d thickness=%d \n",
                    Osdc->osdcObj[j][i].type, Osdc->osdcObj[j][i].color, Osdc->osdcObj[j][i].x1, Osdc->osdcObj[j][i].y1,
                    Osdc->osdcObj[j][i].x2, Osdc->osdcObj[j][i].y2, Osdc->osdcObj[j][i].width, Osdc->osdcObj[j][i].height,
                    Osdc->osdcObj[j][i].filled, Osdc->osdcObj[j][i].thickness);
            }
        }
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Osdc not enable!\n");
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Osdc config ------------------> done \n\n");

    return CVI_SUCCESS;
}

int Load_Param_Osdc(const char *file)
{
    return Load_Param_Osdc_Config(file, app_ipcam_Osdc_Param_Get());
}
