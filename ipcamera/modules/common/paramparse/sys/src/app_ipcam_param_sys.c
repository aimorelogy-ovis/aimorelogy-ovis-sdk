#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Sys(const char *file)
{
    CVI_U32 i = 0;
    CVI_U32 vbpoolnum = 0;
    CVI_S32 enum_num = 0;
    CVI_S32 ret = 0;
    char tmp_section[16] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_SYS_CFG_S *Sys = app_ipcam_Sys_Param_Get();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** data_bitwidth = app_ipcam_Param_get_data_bitwidth();
    const char ** compress_mode = app_ipcam_Param_get_compress_mode();
    const char ** vi_vpss_mode = app_ipcam_Param_get_vi_vpss_mode();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading systerm config ------------------> start \n");

    Sys->u8SbmCnt = ini_getl("slice_config", "slice_cnt", 0, file);
    if ((Sys->u8SbmCnt > 0) && (Sys->u8SbmCnt <= APP_IPCAM_SBM_MAX_NUM)) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "SBM Enable !\n");
        for (i = 0; i < Sys->u8SbmCnt; i++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            sprintf(tmp_section, "slice_buff_%d", i);
            Sys->stSbmCfg[i].bEnable = (bool)ini_getl(tmp_section, "bEnable", 0, file);
            Sys->stSbmCfg[i].s32SbmGrp = ini_getl(tmp_section, "s32SbmGrp", 0, file);
            Sys->stSbmCfg[i].s32SbmChn = ini_getl(tmp_section, "s32SbmChn", 0, file);
            Sys->stSbmCfg[i].s32WrapBufLine = ini_getl(tmp_section, "s32WrapBufLine", 0, file);
            Sys->stSbmCfg[i].s32WrapBufSize = ini_getl(tmp_section, "s32WrapBufSize", 0, file);
        }
    }

    Sys->vb_pool_num = ini_getl("vb_config", "vb_pool_cnt", 0, file);

    for (i = 0; i < Sys->vb_pool_num; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        sprintf(tmp_section, "vb_pool_%d", i);

        Sys->vb_pool[i].bEnable = ini_getl(tmp_section, "bEnable", 1, file);
        if (!Sys->vb_pool[i].bEnable)
            continue;

        Sys->vb_pool[vbpoolnum].width = ini_getl(tmp_section, "frame_width", 0, file);
        Sys->vb_pool[vbpoolnum].height = ini_getl(tmp_section, "frame_height", 0, file);
        ini_gets(tmp_section, "frame_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][frame_fmt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][frame_fmt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Sys->vb_pool[vbpoolnum].fmt = enum_num;
        }

        ini_gets(tmp_section, "data_bitwidth", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, data_bitwidth, DATA_BITWIDTH_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][data_bitwidth] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][data_bitwidth] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Sys->vb_pool[vbpoolnum].enBitWidth = enum_num;
        }

        ini_gets(tmp_section, "compress_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, compress_mode, COMPRESS_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][compress_mode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][compress_mode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Sys->vb_pool[vbpoolnum].enCmpMode = enum_num;
        }

        Sys->vb_pool[vbpoolnum].vb_blk_num = ini_getl(tmp_section, "blk_cnt", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "vb_pool[%d] w=%4d h=%4d count=%d fmt=%d\n", vbpoolnum, Sys->vb_pool[vbpoolnum].width,
            Sys->vb_pool[vbpoolnum].height, Sys->vb_pool[vbpoolnum].vb_blk_num, Sys->vb_pool[vbpoolnum].fmt);

        vbpoolnum++;
    }

    Sys->vb_pool_num = vbpoolnum;

    for(i = 0; i < VI_MAX_PIPE_NUM; i++){
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_vpss_mode_%d", i);

        ini_gets(tmp_section, "enMode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vi_vpss_mode, VI_VPSS_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enMode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enMode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Sys->stVIVPSSMode.aenMode[i] = enum_num;
        }
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading systerm config ------------------>done \n\n");

    return CVI_SUCCESS;
}



