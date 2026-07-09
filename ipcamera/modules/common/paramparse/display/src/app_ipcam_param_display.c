#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

#define VO_INTF_TYPE_MAX ((0x01L << 16) + 1)
const char *panel_type[PANEL_MAX] = {
    [PANEL_DSI_3AML069LP01G] = "PANEL_DSI_3AML069LP01G",
    [PANEL_DSI_GM8775C] = "PANEL_DSI_GM8775C",
    [PANEL_DSI_HX8394_EVB] = "PANEL_DSI_HX8394_EVB",
    [PANEL_DSI_HX8399_1080P] = "PANEL_DSI_HX8399_1080P",
    [PANEL_DSI_ICN9707] = "PANEL_DSI_ICN9707",
    [PANEL_DSI_ILI9881C] = "PANEL_DSI_ILI9881C",
    [PANEL_DSI_ILI9881D] = "PANEL_DSI_ILI9881D",
    [PANEL_DSI_JD9366AB] = "PANEL_DSI_JD9366AB",
    [PANEL_DSI_JD9852] = "PANEL_DSI_JD9852",
    [PANEL_DSI_LT9611_1024x768_60] = "PANEL_DSI_LT9611_1024x768_60",
    [PANEL_DSI_LT9611_1280x720_60] = "PANEL_DSI_LT9611_1280x720_60",
    [PANEL_DSI_LT9611_1280x1024_60] = "PANEL_DSI_LT9611_1280x1024_60",
    [PANEL_DSI_LT9611_1600x1200_60] = "PANEL_DSI_LT9611_1600x1200_60",
    [PANEL_DSI_LT9611_1920x1080_30] = "PANEL_DSI_LT9611_1920x1080_30",
    [PANEL_DSI_LT9611_1920x1080_60] = "PANEL_DSI_LT9611_1920x1080_60",
    [PANEL_DSI_NT35521] = "PANEL_DSI_NT35521",
    [PANEL_DSI_OTA7290B] = "PANEL_DSI_OTA7290B",
    [PANEL_DSI_OTA7290B_1920] = "PANEL_DSI_OTA7290B_1920",
    [PANEL_DSI_ST7701] = "PANEL_DSI_ST7701",
    [PANEL_DSI_ST7785M] = "PANEL_DSI_ST7785M",
    [PANEL_DSI_ST7796S] = "PANEL_DSI_ST7796S",
    [PANEL_BT656_TP2803] = "PANEL_BT656_TP2803",
    [PANEL_BT656_MS7024_720x480_60] = "PANEL_BT656_MS7024_720x480_60",
    [PANEL_BT656_MS7024_720x480_30] = "PANEL_BT656_MS7024_720x480_30",
    [PANEL_BT656_MS7024_720x576_25] = "PANEL_BT656_MS7024_720x576_25",
    [PANEL_BT656_MS7024_720x576_50] = "PANEL_BT656_MS7024_720x576_50",
    [PANEL_LVDS_LCM185X56] = "PANEL_LVDS_LCM185X56",
    [PANEL_I80_ST7789V] = "PANEL_I80_ST7789V"
};

const char *vo_intf_type[VO_INTF_TYPE_MAX] = {
    [VO_INTF_BT656] = "VO_INTF_BT656",
    [VO_INTF_BT1120] = "VO_INTF_BT1120",
    [VO_INTF_MIPI] = "VO_INTF_MIPI",
};

const char *vo_intf_sync[VO_OUTPUT_BUTT] = {
    [VO_OUTPUT_PAL] = "VO_OUTPUT_PAL",
    [VO_OUTPUT_NTSC] = "VO_OUTPUT_NTSC",
    [VO_OUTPUT_1080P24] = "VO_OUTPUT_1080P24",
    [VO_OUTPUT_1080P25] = "VO_OUTPUT_1080P25",
    [VO_OUTPUT_1080P30] = "VO_OUTPUT_1080P30",
    [VO_OUTPUT_720P50] = "VO_OUTPUT_720P50",
    [VO_OUTPUT_720P60] = "VO_OUTPUT_720P60",
    [VO_OUTPUT_1080P50] = "VO_OUTPUT_1080P50",
    [VO_OUTPUT_1080P60] = "VO_OUTPUT_1080P60",
    [VO_OUTPUT_576P50] = "VO_OUTPUT_576P50",
    [VO_OUTPUT_480P60] = "VO_OUTPUT_480P60",
    [VO_OUTPUT_800x600_60] = "VO_OUTPUT_800x600_60",
    [VO_OUTPUT_1024x768_60] = "VO_OUTPUT_1024x768_60",
    [VO_OUTPUT_1280x1024_60] = "VO_OUTPUT_1280x1024_60",
    [VO_OUTPUT_1366x768_60] = "VO_OUTPUT_1366x768_60",
    [VO_OUTPUT_1440x900_60] = "VO_OUTPUT_1440x900_60",
    [VO_OUTPUT_1280x800_60] = "VO_OUTPUT_1280x800_60",
    [VO_OUTPUT_1600x1200_60] = "VO_OUTPUT_1600x1200_60",
    [VO_OUTPUT_1680x1050_60] = "VO_OUTPUT_1680x1050_60",
    [VO_OUTPUT_1920x1200_60] = "VO_OUTPUT_1920x1200_60",
    [VO_OUTPUT_640x480_60] = "VO_OUTPUT_640x480_60",
    [VO_OUTPUT_720x1280_60] = "VO_OUTPUT_720x1280_60",
    [VO_OUTPUT_1080x1920_60] = "VO_OUTPUT_1080x1920_60",
    [VO_OUTPUT_USER] = "VO_OUTPUT_USER"
};

const char *rotation[ROTATION_MAX] = {
    [ROTATION_0] = "ROTATION_0",
    [ROTATION_90] ="ROTATION_90",
    [ROTATION_180] = "ROTATION_180",
    [ROTATION_270] = "ROTATION_270",
    [ROTATION_XY_FLIP] = "ROTATION_XY_FLIP"
};

const char *vo_mode[VO_MODE_BUTT] = {
    [VO_MODE_1MUX] = "VO_MODE_1MUX",
    [VO_MODE_2MUX] = "VO_MODE_2MUX",
    [VO_MODE_4MUX] = "VO_MODE_4MUX",
    [VO_MODE_8MUX] = "VO_MODE_8MUX",
    [VO_MODE_9MUX] = "VO_MODE_9MUX",
    [VO_MODE_16MUX] = "VO_MODE_16MUX",
    [VO_MODE_25MUX] = "VO_MODE_25MUX",
    [VO_MODE_36MUX] = "VO_MODE_36MUX",
    [VO_MODE_49MUX] = "VO_MODE_49MUX",
    [VO_MODE_64MUX] = "VO_MODE_64MUX",
    [VO_MODE_2X4] = "VO_MODE_2X4"
};

int Load_Param_Display(const char * file)
{
    int ret = 0;
    int enum_num = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_MULTI_DISPLAY_CFG_T * pstDisplayCfg = app_ipcam_Display_Param_Get();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** mode_id = app_ipcam_Param_get_mode_id();


    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading display config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    pstDisplayCfg->vo_num = ini_getl("display_config", "vo_cnt", 0, file);

    for(unsigned i = 0; i < pstDisplayCfg->vo_num; i++){
        memset(tmp_section, 0, sizeof(tmp_section));
        sprintf(tmp_section, "display_config_%d", i);

        ini_gets(tmp_section, "panel_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, panel_type, PANEL_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][panel_type] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][panel_type] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].enPanelType = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stPanelI2cCfg.s32I2cDev = ini_getl(tmp_section, "i2c_dev", 0, file);
        pstDisplayCfg->vo_cfg[i].stPanelI2cCfg.s32I2cAddr = ini_getl(tmp_section, "i2c_addr", 0, file);

        pstDisplayCfg->vo_cfg[i].stVoCfg.s32VoDev = ini_getl(tmp_section, "vo_dev", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.u32BgColor = ini_getl(tmp_section, "bg_color", 0, file);

        ini_gets(tmp_section, "intf_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_intf_type, VO_INTF_TYPE_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_type] Fail to convert string name [%s] to number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_type] Convert string name [%s] to number [%d].\n", tmp_section
                , str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfType = enum_num;
        }

        ini_gets(tmp_section, "intf_sync", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_intf_sync, VO_OUTPUT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_sync] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][intf_sync] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfSync = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32X = ini_getl(tmp_section, "dis_x", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32Y = ini_getl(tmp_section, "dis_y", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Width = ini_getl(tmp_section, "dis_width", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Height = ini_getl(tmp_section, "dis_height", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Width = ini_getl(tmp_section, "img_width", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Height = ini_getl(tmp_section, "img_height", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.u32DispFrmRt = ini_getl(tmp_section, "dis_framerate", 0, file);

        ini_gets(tmp_section, "pixel_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.enPixFormat = enum_num;
        }

        ini_gets(tmp_section, "mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vo_mode, VO_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mode] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][mode] Convert string name [%s] to enum number [%d].\n", tmp_section
                , str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.enVoMode = enum_num;
        }

        ini_gets(tmp_section, "rotation", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, rotation, ROTATION_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rotation] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.enRotation = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.u32DisBufLen = ini_getl(tmp_section, "dis_buf_len", 0, file);

        pstDisplayCfg->vo_cfg[i].stVoCfg.bBindMode = ini_getl(tmp_section, "bind_mode", 0, file);

        ini_gets(tmp_section, "src_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.enModId = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32DevId = ini_getl(tmp_section, "src_dev_id", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32ChnId = ini_getl(tmp_section, "src_chn_id", 0, file);

        ini_gets(tmp_section, "dst_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Fail to convert string name [%s] to enum number!\n"
                , tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Convert string name [%s] to enum number [%d].\n"
                , tmp_section, str_name, enum_num);
            pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.enModId = enum_num;
        }

        pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32DevId = ini_getl(tmp_section, "dst_dev_id", 0, file);
        pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32ChnId = ini_getl(tmp_section, "dst_chn_id", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO,
            "enPanelType:%d, s32VoDev:%d, u32BgColor:%d, enIntfType:%d, enIntfSync:%d,\n"
            "x:%d, y:%d, w:%d, h:%d, imgw:%d, imgh:%d,\n"
            "u32DispFrmRt:%d, enPixFormat:%d, enVoMode:%d, enRotation:%d, u32DisBufLen:%d,\n"
            "bBindMode:%d, src: modid=%d, devid=%d, chnid=%d, dst: modid=%d, devid=%d, chnid=%d,\n"
            "i2c_dev:%d, i2c_addr:%d.\n"
            , pstDisplayCfg->vo_cfg[i].enPanelType
            , pstDisplayCfg->vo_cfg[i].stVoCfg.s32VoDev
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.u32BgColor
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfType
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stVoPubAttr.enIntfSync
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32X
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.s32Y
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Width
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stDispRect.u32Height
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Width
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.stImageSize.u32Height
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.u32DispFrmRt
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stLayerAttr.enPixFormat
            , pstDisplayCfg->vo_cfg[i].stVoCfg.enVoMode
            , pstDisplayCfg->vo_cfg[i].stVoCfg.enRotation
            , pstDisplayCfg->vo_cfg[i].stVoCfg.u32DisBufLen
            , pstDisplayCfg->vo_cfg[i].stVoCfg.bBindMode
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.enModId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32DevId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stSrcChn.s32ChnId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.enModId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32DevId
            , pstDisplayCfg->vo_cfg[i].stVoCfg.stDstChn.s32ChnId
            , pstDisplayCfg->vo_cfg[i].stPanelI2cCfg.s32I2cDev
            , pstDisplayCfg->vo_cfg[i].stPanelI2cCfg.s32I2cAddr);
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading display config ------------------> done \n\n");

    return CVI_SUCCESS;
}
