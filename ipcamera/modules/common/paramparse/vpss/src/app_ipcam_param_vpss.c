#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

//priavete attirebute
#define VPSS_CROP_COORDINATE_MAX 2

const char *vpss_crop_coordinate[VPSS_CROP_COORDINATE_MAX] = {
    [VPSS_CROP_RATIO_COOR] = "VPSS_CROP_RATIO_COOR",
    [VPSS_CROP_ABS_COOR] = "VPSS_CROP_ABS_COOR"
};

const char *aspect_ratio[ASPECT_RATIO_MAX] = {
    [ASPECT_RATIO_NONE] = "ASPECT_RATIO_NONE",
    [ASPECT_RATIO_AUTO] = "ASPECT_RATIO_AUTO",
    [ASPECT_RATIO_MANUAL] = "ASPECT_RATIO_MANUAL"
};

const char *vpss_mode[VPSS_MODE_BUTT] = {
    [VPSS_MODE_SINGLE] = "VPSS_MODE_SINGLE",
    [VPSS_MODE_DUAL] = "VPSS_MODE_DUAL"
};

const char *vpss_input[VPSS_INPUT_BUTT] = {
    [VPSS_INPUT_MEM] = "VPSS_INPUT_MEM",
    [VPSS_INPUT_ISP] = "VPSS_INPUT_ISP"
};

int Load_Param_Vpss(const char *file)
{
    CVI_U32 grp_idx = 0;
    CVI_U32 chn_idx = 0;
    CVI_S32 enum_num = 0;
    CVI_S32 ret = 0;
    char tmp_section[32] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_VPSS_CFG_T *Vpss = app_ipcam_Vpss_Param_Get();
    const char ** mode_id = app_ipcam_Param_get_mode_id();
    const char ** pixel_format = app_ipcam_Param_get_pixel_format();
    const char ** video_format = app_ipcam_Param_get_video_format();


    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vpss config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "vpss_config");

    Vpss->u32GrpCnt = ini_getl(tmp_section, "vpss_grp", 0, file);

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "vpss_mode");

    ini_gets(tmp_section, "enMode", " ", str_name, PARAM_STRING_NAME_LEN, file);
    ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vpss_mode, VPSS_MODE_BUTT, &enum_num);
    if (ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enMode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enMode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
        Vpss->stVPSSMode.enMode = enum_num;
    }

    CVI_U32 dev_cnt = ini_getl("vpss_dev", "dev_cnt", 0, file);
    for (CVI_U32 i = 0; i < dev_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vpss_dev%d", i);

        ini_gets(tmp_section, "aenInput", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vpss_input, VPSS_INPUT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enMode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][enMode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Vpss->stVPSSMode.aenInput[i] = enum_num;
        }
    }

    for (grp_idx = 0; grp_idx < Vpss->u32GrpCnt; grp_idx++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vpssgrp%d", grp_idx);
        Vpss->astVpssGrpCfg[grp_idx].bEnable = ini_getl(tmp_section, "grp_enable", 1, file);
        if (!Vpss->astVpssGrpCfg[grp_idx].bEnable)
            continue;
        Vpss->astVpssGrpCfg[grp_idx].VpssGrp = grp_idx;
        Vpss->astVpssGrpCfg[grp_idx].bBindMode = ini_getl(tmp_section, "bind_mode", 0, file);
        if (Vpss->astVpssGrpCfg[grp_idx].bBindMode) {
            ini_gets(tmp_section, "src_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Vpss->astVpssGrpCfg[grp_idx].astChn[0].enModId = enum_num;
            }

            Vpss->astVpssGrpCfg[grp_idx].astChn[0].s32DevId = ini_getl(tmp_section, "src_dev_id", 0, file);
            Vpss->astVpssGrpCfg[grp_idx].astChn[0].s32ChnId = ini_getl(tmp_section, "src_chn_id", 0, file);

            ini_gets(tmp_section, "dst_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Vpss->astVpssGrpCfg[grp_idx].astChn[1].enModId = enum_num;
            }

            Vpss->astVpssGrpCfg[grp_idx].astChn[1].s32DevId = ini_getl(tmp_section, "dst_dev_id", 0, file);
            Vpss->astVpssGrpCfg[grp_idx].astChn[1].s32ChnId = ini_getl(tmp_section, "dst_chn_id", 0, file);
        }
        APP_PROF_LOG_PRINT(LEVEL_INFO, "vpss grp_idx=%d\n", grp_idx);
        VPSS_GRP_ATTR_S *pstVpssGrpAttr = &Vpss->astVpssGrpCfg[grp_idx].stVpssGrpAttr;

        ini_gets(tmp_section, "pixel_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][pixel_fmt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            pstVpssGrpAttr->enPixelFormat = enum_num;
        }

        pstVpssGrpAttr->stFrameRate.s32SrcFrameRate = ini_getl(tmp_section, "src_framerate", 0, file);
        pstVpssGrpAttr->stFrameRate.s32DstFrameRate = ini_getl(tmp_section, "dst_framerate", 0, file);
        pstVpssGrpAttr->u32MaxW                     = ini_getl(tmp_section, "max_w", 0, file);
        pstVpssGrpAttr->u32MaxH                     = ini_getl(tmp_section, "max_h", 0, file);
        pstVpssGrpAttr->u8VpssDev                   = ini_getl(tmp_section, "vpss_dev", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Group_ID_%d Config: pix_fmt=%2d sfr=%2d dfr=%2d Dev=%d W=%4d H=%4d\n",
            grp_idx, pstVpssGrpAttr->enPixelFormat, pstVpssGrpAttr->stFrameRate.s32SrcFrameRate, pstVpssGrpAttr->stFrameRate.s32DstFrameRate,
            pstVpssGrpAttr->u8VpssDev, pstVpssGrpAttr->u32MaxW, pstVpssGrpAttr->u32MaxH);


        VPSS_CROP_INFO_S *pstVpssGrpCropInfo = &Vpss->astVpssGrpCfg[grp_idx].stVpssGrpCropInfo;
        pstVpssGrpCropInfo->bEnable = ini_getl(tmp_section, "crop_en", 0, file);
        if (pstVpssGrpCropInfo->bEnable){
            ini_gets(tmp_section, "crop_coor", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vpss_crop_coordinate, VPSS_CROP_COORDINATE_MAX, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][crop_coor] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][crop_coor] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                pstVpssGrpCropInfo->enCropCoordinate = enum_num;
            }

            pstVpssGrpCropInfo->stCropRect.s32X      = ini_getl(tmp_section, "crop_rect_x", 0, file);
            pstVpssGrpCropInfo->stCropRect.s32Y      = ini_getl(tmp_section, "crop_rect_y", 0, file);
            pstVpssGrpCropInfo->stCropRect.u32Width  = ini_getl(tmp_section, "crop_rect_w", 0, file);
            pstVpssGrpCropInfo->stCropRect.u32Height = ini_getl(tmp_section, "crop_rect_h", 0, file);
        }

        CVI_U32 grp_chn_cnt = ini_getl(tmp_section, "chn_cnt", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Group_ID_%d channel count = %d\n", grp_idx, grp_chn_cnt);
        /* load vpss group channel config */
        for (chn_idx = 0; chn_idx < grp_chn_cnt; chn_idx++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "vpssgrp%d.chn%d", grp_idx, chn_idx);
            Vpss->astVpssGrpCfg[grp_idx].abChnEnable[chn_idx] = ini_getl(tmp_section, "chn_enable", 0, file);
            if (!Vpss->astVpssGrpCfg[grp_idx].abChnEnable[chn_idx])
                continue;

            VPSS_CHN_ATTR_S *pastVpssChnAttr = &Vpss->astVpssGrpCfg[grp_idx].astVpssChnAttr[chn_idx];
            pastVpssChnAttr->u32Width = ini_getl(tmp_section, "width", 0, file);
            pastVpssChnAttr->u32Height = ini_getl(tmp_section, "height", 0, file);

            ini_gets(tmp_section, "video_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, video_format, VIDEO_FORMAT_MAX, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][video_fmt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][video_fmt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                pastVpssChnAttr->enVideoFormat = enum_num;
            }

            ini_gets(tmp_section, "chn_pixel_fmt", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, pixel_format, PIXEL_FORMAT_MAX, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][chn_pixel_fmt] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][chn_pixel_fmt] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                pastVpssChnAttr->enPixelFormat = enum_num;
            }

            pastVpssChnAttr->stFrameRate.s32SrcFrameRate = ini_getl(tmp_section, "src_framerate", 0, file);
            pastVpssChnAttr->stFrameRate.s32DstFrameRate = ini_getl(tmp_section, "dst_framerate", 0, file);
            pastVpssChnAttr->u32Depth                    = ini_getl(tmp_section, "depth", 0, file);
            pastVpssChnAttr->bMirror                     = ini_getl(tmp_section, "mirror", 0, file);
            pastVpssChnAttr->bFlip                       = ini_getl(tmp_section, "flip", 0, file);

            ini_gets(tmp_section, "aspectratio", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, aspect_ratio, ASPECT_RATIO_MAX, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][aspectratio] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][aspectratio] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                pastVpssChnAttr->stAspectRatio.enMode = enum_num;
            }

            if (pastVpssChnAttr->stAspectRatio.enMode == ASPECT_RATIO_MANUAL) { /*ASPECT_RATIO_MANUAL*/
                pastVpssChnAttr->stAspectRatio.stVideoRect.s32X      = ini_getl(tmp_section, "s32x", 0, file);
                pastVpssChnAttr->stAspectRatio.stVideoRect.s32Y      = ini_getl(tmp_section, "s32y", 0, file);
                pastVpssChnAttr->stAspectRatio.stVideoRect.u32Width  = ini_getl(tmp_section, "rec_width", 0, file);
                pastVpssChnAttr->stAspectRatio.stVideoRect.u32Height = ini_getl(tmp_section, "rec_heigh", 0, file);
            }

            pastVpssChnAttr->stAspectRatio.bEnableBgColor = ini_getl(tmp_section, "en_color", 0, file);
            if (pastVpssChnAttr->stAspectRatio.bEnableBgColor != 0)
                pastVpssChnAttr->stAspectRatio.u32BgColor = ini_getl(tmp_section, "color", 0, file);
            pastVpssChnAttr->stNormalize.bEnable = ini_getl(tmp_section, "normalize", 0, file);

            VPSS_CROP_INFO_S *pstVpssChnCropInfo = &Vpss->astVpssGrpCfg[grp_idx].stVpssChnCropInfo[chn_idx];
            pstVpssChnCropInfo->bEnable = ini_getl(tmp_section, "crop_en", 0, file);
            if (pstVpssChnCropInfo->bEnable) {
                ini_gets(tmp_section, "crop_coor", " ", str_name, PARAM_STRING_NAME_LEN, file);
                ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, vpss_crop_coordinate, VPSS_CROP_COORDINATE_MAX, &enum_num);
                if (ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "Fail to convert string name [%s] to enum number!\n", str_name);
                } else {
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][crop_coor] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                    pstVpssChnCropInfo->enCropCoordinate = enum_num;
                }

                pstVpssChnCropInfo->stCropRect.s32X      = ini_getl(tmp_section, "crop_rect_x", 0, file);
                pstVpssChnCropInfo->stCropRect.s32Y      = ini_getl(tmp_section, "crop_rect_y", 0, file);
                pstVpssChnCropInfo->stCropRect.u32Width  = ini_getl(tmp_section, "crop_rect_w", 0, file);
                pstVpssChnCropInfo->stCropRect.u32Height = ini_getl(tmp_section, "crop_rect_h", 0, file);
            }

            Vpss->astVpssGrpCfg[grp_idx].aAttachEn[chn_idx] = ini_getl(tmp_section, "attach_en", 0, file);
            if (Vpss->astVpssGrpCfg[grp_idx].aAttachEn[chn_idx]) {
                Vpss->astVpssGrpCfg[grp_idx].aAttachPool[chn_idx] = ini_getl(tmp_section, "attach_pool", 0, file);
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Chn_ID_%d config: sft=%2d dfr=%2d W=%4d H=%4d Depth=%d Mirror=%d Flip=%d V_fmt=%2d P_fmt=%2d\n",
                chn_idx, pastVpssChnAttr->stFrameRate.s32SrcFrameRate, pastVpssChnAttr->stFrameRate.s32DstFrameRate,
                pastVpssChnAttr->u32Width, pastVpssChnAttr->u32Height, pastVpssChnAttr->u32Depth, pastVpssChnAttr->bMirror, pastVpssChnAttr->bFlip,
                pastVpssChnAttr->enVideoFormat, pastVpssChnAttr->enPixelFormat);
        }
    }

    /* 读取 stitch / forward 配置 */
    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "vpss_stitch");
    Vpss->bStitchEnable = ini_getl(tmp_section, "stitch_en", 0, file);
    if (Vpss->bStitchEnable) {
        CVI_U32 pair_cnt = ini_getl(tmp_section, "stitch_pair_cnt", 0, file);
        if (pair_cnt > CVI_MAX_VPSS_GRP) {
            APP_PROF_LOG_PRINT(LEVEL_WARN, "[%s] stitch_pair_cnt(%d) > max(%d), truncate\n",
                                tmp_section, pair_cnt, CVI_MAX_VPSS_GRP);
            pair_cnt = CVI_MAX_VPSS_GRP;
        }
        Vpss->u32StitchPairCnt = pair_cnt;
        for (CVI_U32 i = 0; i < pair_cnt; i++) {
            char sec[32] = {0};
            snprintf(sec, sizeof(sec), "vpss_stitch%d", i);
            Vpss->astStitchPair[i].srcGrp = ini_getl(sec, "src_grp", 0, file);
            Vpss->astStitchPair[i].srcChn = ini_getl(sec, "src_chn", 0, file);
            Vpss->astStitchPair[i].dstGrp = ini_getl(sec, "dst_grp", 0, file);
            Vpss->astStitchPair[i].dstChn = ini_getl(sec, "dst_chn", 0, file);
            APP_PROF_LOG_PRINT(LEVEL_INFO,
                                "[%s] pair%d: src(%d,%d) -> dst(%d,%d)\n",
                                tmp_section, i,
                                Vpss->astStitchPair[i].srcGrp, Vpss->astStitchPair[i].srcChn,
                                Vpss->astStitchPair[i].dstGrp, Vpss->astStitchPair[i].dstChn);
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vpss config ------------------> done \n\n");
    return CVI_SUCCESS;
}
