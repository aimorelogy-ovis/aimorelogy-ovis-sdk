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

int Load_Param_Osdc(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Osdc config ------------------> start \n");
    unsigned int i = 0;
    unsigned int j = 0;
    int enum_num = 0;
    int ret = 0;
    char tmp_buff[APP_OSD_STR_LEN_MAX] = {0};
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_OSDC_CFG_S * Osdc = app_ipcam_Osdc_Param_Get();
    const char ** mode_id = app_ipcam_Param_get_mode_id();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();

    memset(Osdc, 0, sizeof(APP_PARAM_OSDC_CFG_S));
    Osdc->enable = ini_getl("osdc_config", "enable", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "osdc enable: %d\n", Osdc->enable);
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
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][format] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][format] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Osdc->format[j] = enum_num;
            }

            ini_gets(tmp_section, "mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Osdc->mmfChn[j].enModId = enum_num;
            }

            Osdc->mmfChn[j].s32DevId = ini_getl(tmp_section, "dev_id", 0, file);
            Osdc->mmfChn[j].s32ChnId = ini_getl(tmp_section, "chn_id", 0, file);
            Osdc->bShowPdRect[j]     = ini_getl(tmp_section, "show_pd_rect", 0, file);
            Osdc->bShowMdRect[j]     = ini_getl(tmp_section, "show_md_rect", 0, file);
            Osdc->bShowFdRect[j]     = ini_getl(tmp_section, "show_fd_rect", 0, file);
            Osdc->bShowHumanKeypointRect[j] = ini_getl(tmp_section, "show_human_keypoint_rect", 0, file);
            Osdc->bShowTrackRect[j] = ini_getl(tmp_section, "show_track_rect", 0, file);
            Osdc->osdcObjNum[j]      = ini_getl(tmp_section, "cnt", 0, file);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "handle=%d bShow=%d Format=0x%x cpsSize=%d ModeId=%d DevId=%d ChnId=%d PdRect=%d MdRect=%d FdRect=%d osdcObjNum=%d\n",
                Osdc->handle[j], Osdc->bShow[j], Osdc->format[j], Osdc->CompressedSize[j], Osdc->mmfChn[j].enModId,
                Osdc->mmfChn[j].s32DevId, Osdc->mmfChn[j].s32ChnId, Osdc->bShowPdRect[j], Osdc->bShowMdRect[j], Osdc->bShowFdRect[j], Osdc->osdcObjNum[j]);

            for (i = 0; i < Osdc->osdcObjNum[j]; i++) {
                memset(tmp_section, 0, sizeof(tmp_section));
                snprintf(tmp_section, sizeof(tmp_section), "osdc%d_obj_info%d", j, i);
                Osdc->osdcObj[j][i].bShow      = ini_getl(tmp_section, "bShow", 0, file);
                ini_gets(tmp_section, "type", " ", str_name, PARAM_STRING_NAME_LEN, file);
                ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, rgn_cmpr_type, RGN_CMPR_BUTT, &enum_num);
                if (ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][type] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][type] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                    Osdc->osdcObj[j][i].type = enum_num;
                }

                Osdc->osdcObj[j][i].color = ini_getl(tmp_section, "color", 0, file);
                Osdc->osdcObj[j][i].x1    = ini_getl(tmp_section, "x1", 0, file);
                Osdc->osdcObj[j][i].y1    = ini_getl(tmp_section, "y1", 0, file);
                if (RGN_CMPR_BIT_MAP == Osdc->osdcObj[j][i].type) {
                    ini_gets(tmp_section, "entype", " ", str_name, PARAM_STRING_NAME_LEN, file);
                    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, osd_type, TYPE_END, &enum_num);
                    if (ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][entype] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
                    } else {
                        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][entype] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                        Osdc->osdcObj[j][i].enType = enum_num;
                    }

                    if (TYPE_STRING == Osdc->osdcObj[j][i].enType) {
                        ini_gets(tmp_section, "str", " ", tmp_buff, APP_OSD_STR_LEN_MAX, file);
                        strncpy(Osdc->osdcObj[j][i].str, tmp_buff, APP_OSD_STR_LEN_MAX);
                    } else if (TYPE_PICTURE == Osdc->osdcObj[j][i].enType) {
                        ini_gets(tmp_section, "file_name", " ", tmp_buff, APP_OSD_STR_LEN_MAX, file);
                        strncpy(Osdc->osdcObj[j][i].filename, tmp_buff, APP_OSD_STR_LEN_MAX);
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

                APP_PROF_LOG_PRINT(LEVEL_INFO, "type=%d color=0x%x x1=%d y1=%d x2=%d y2=%d width=%d height=%d filled=%d thickness=%d \n",
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
