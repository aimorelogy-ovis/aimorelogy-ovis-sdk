#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

//private attribute
const char *vdec_demode[VIDEO_MODE_BUTT] = {
    [VIDEO_MODE_STREAM] = "VIDEO_MODE_STREAM",
    [VIDEO_MODE_FRAME]  = "VIDEO_MODE_FRAME",
    [VIDEO_MODE_COMPAT] = "VIDEO_MODE_COMPAT"
};

int Load_Param_Vdec(const char *file)
{
    int ret = 0;
    int i = 0;
    int enum_num = 0;
    long int vdec_chn_num = 0;
    char tmp_section[32] = {0};
    char tmp_buff[PARAM_STRING_LEN] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_VDEC_CFG_S *Vdec = app_ipcam_Vdec_Param_Get();
    const char ** payload_type = app_ipcam_Param_get_payload_type();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vdec config ------------------> start \n");

    vdec_chn_num = ini_getl("vdec_config", "chn_num", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "vdec_chn_num: %ld\n", vdec_chn_num);

    for (i = 0; i < vdec_chn_num; i++)
    {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vdecchn%d", i);
        Vdec->astVdecChnCfg.VdecChn = i;
        Vdec->astVdecChnCfg.bEnable = ini_getl(tmp_section, "bEnable", 0, file);
        if (!Vdec->astVdecChnCfg.bEnable)
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Vdec_chn[%d] not enable!\n", i);
            continue;
        }

        Vdec->astVdecChnCfg.u32Width  = ini_getl(tmp_section, "width", 640, file);
        Vdec->astVdecChnCfg.u32Height = ini_getl(tmp_section, "height", 360, file);

        ini_gets(tmp_section, "decode_filename", " ", tmp_buff, PARAM_STRING_LEN, file);
        strncpy(Vdec->astVdecChnCfg.decode_file_name, tmp_buff, PARAM_STRING_LEN);

        ini_gets(tmp_section, "de_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, payload_type, PT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][de_type] Fail to convert string name"
                    " [%s] to enum number!\n", tmp_section, str_name);
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][de_type] Convert string name "
                    "[%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Vdec->astVdecChnCfg.astChnAttr.enType = enum_num;
        }

        ini_gets(tmp_section, "de_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vdec_demode, VIDEO_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][de_mode] Fail to convert string name"
                    " [%s] to enum number!\n", tmp_section, str_name);
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][de_mode] Convert string name "
                    "[%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Vdec->astVdecChnCfg.astChnAttr.enMode = enum_num;
        }

        Vdec->astVdecChnCfg.astChnAttr.u32PicWidth      = ini_getl(tmp_section, "pic_width", 0, file);
        Vdec->astVdecChnCfg.astChnAttr.u32PicHeight     = ini_getl(tmp_section, "pic_height", 0, file);
        Vdec->astVdecChnCfg.astChnAttr.u32FrameBufCnt   = ini_getl(tmp_section, "frm_buf_cnt", 0, file);

        ini_gets(tmp_section, "vdec_pixfmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[%s][vdec_pixfmt] Fail to convert string name"
                    " [%s] to enum number!\n", tmp_section, str_name);
        }
        else
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][vdec_pixfmt] Convert string name "
                    "[%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Vdec->astVdecChnCfg.astChnParam.enPixelFormat = enum_num;
        }

        Vdec->astVdecChnCfg.astChnParam.u32DisplayFrameNum = ini_getl(tmp_section, "vdec_disp_frm_num", 0, file);
        Vdec->PicVbPool = ini_getl(tmp_section, "PicVbPool", 0, file);
        Vdec->VpssGrp   = ini_getl(tmp_section, "vpss_grp", 0, file);
        Vdec->VpssChn   = ini_getl(tmp_section, "vpss_chn", 0, file);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vdec config ------------------> done \n\n");

    return CVI_SUCCESS;
}
