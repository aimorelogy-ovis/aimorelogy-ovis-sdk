#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"


//private attirebute
#define VENC_BIND_MAX 3

const char *venc_bind_mode[VENC_BIND_MAX] = {
    [VENC_BIND_DISABLE] = "VENC_BIND_DISABLE",
    [VENC_BIND_VI] = "VENC_BIND_VI",
    [VENC_BIND_VPSS] = "VENC_BIND_VPSS"
};

const char *venc_rc_mode[VENC_RC_MODE_BUTT] = {
    [VENC_RC_MODE_H264CBR] = "VENC_RC_MODE_H264CBR",
    [VENC_RC_MODE_H264VBR] = "VENC_RC_MODE_H264VBR",
    [VENC_RC_MODE_H264AVBR] = "VENC_RC_MODE_H264AVBR",
    [VENC_RC_MODE_H264QVBR] = "VENC_RC_MODE_H264QVBR",
    [VENC_RC_MODE_H264FIXQP] = "VENC_RC_MODE_H264FIXQP",
    [VENC_RC_MODE_H264QPMAP] = "VENC_RC_MODE_H264QPMAP",
    [VENC_RC_MODE_H264UBR] = "VENC_RC_MODE_H264UBR",
    [VENC_RC_MODE_MJPEGCBR] = "VENC_RC_MODE_MJPEGCBR",
    [VENC_RC_MODE_MJPEGVBR] = "VENC_RC_MODE_MJPEGVBR",
    [VENC_RC_MODE_MJPEGFIXQP] = "VENC_RC_MODE_MJPEGFIXQP",
    [VENC_RC_MODE_H265CBR] = "VENC_RC_MODE_H265CBR",
    [VENC_RC_MODE_H265VBR] = "VENC_RC_MODE_H265VBR",
    [VENC_RC_MODE_H265AVBR] = "VENC_RC_MODE_H265AVBR",
    [VENC_RC_MODE_H265QVBR] = "VENC_RC_MODE_H265QVBR",
    [VENC_RC_MODE_H265FIXQP] = "VENC_RC_MODE_H265FIXQP",
    [VENC_RC_MODE_H265QPMAP] = "VENC_RC_MODE_H265QPMAP",
    [VENC_RC_MODE_H265UBR] = "VENC_RC_MODE_H265UBR"
};

const char *venc_gop_mode[VENC_GOPMODE_BUTT] = {
    [VENC_GOPMODE_NORMALP] = "VENC_GOPMODE_NORMALP",
    [VENC_GOPMODE_DUALP] = "VENC_GOPMODE_DUALP",
    [VENC_GOPMODE_SMARTP] = "VENC_GOPMODE_SMARTP",
    [VENC_GOPMODE_ADVSMARTP] = "VENC_GOPMODE_ADVSMARTP",
    [VENC_GOPMODE_BIPREDB] = "VENC_GOPMODE_BIPREDB",
    [VENC_GOPMODE_LOWDELAYB] = "VENC_GOPMODE_LOWDELAYB"
};

int Load_Param_Venc(const char *file)
{
    int i = 0;
    long int chn_num = 0;
    int buffSize = 0;
    int enum_num = 0;
    int ret = 0;
    char tmp_section[32] = {0};
    char tmp_buff[PARAM_STRING_LEN] = {0};
    char str_name[PARAM_STRING_NAME_LEN] = {0};
    APP_PARAM_VENC_CTX_S *Venc = app_ipcam_Venc_Param_Get();
    const char ** mode_id = app_ipcam_Param_get_mode_id();
    const char ** payload_type = app_ipcam_Param_get_payload_type();

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading venc config ------------------> start \n");

    chn_num = ini_getl("venc_config", "chn_num", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "vpss_chn_num: %ld\n", chn_num);
    Venc->s32VencChnCnt = chn_num;

    for (i = 0; i < chn_num; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vencchn%d", i);
        Venc->astVencChnCfg[i].VencChn = i;

        Venc->astVencChnCfg[i].bEnable = ini_getl(tmp_section, "bEnable", 0, file);
        if (!Venc->astVencChnCfg[i].bEnable) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] not enable!\n", i);
            continue;
        }
        ini_gets(tmp_section, "en_type", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, payload_type, PT_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][en_type] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][en_type] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Venc->astVencChnCfg[i].enType = enum_num;
        }

        Venc->astVencChnCfg[i].StreamTo  = ini_getl(tmp_section, "send_to", 0, file);
        Venc->astVencChnCfg[i].VpssGrp   = ini_getl(tmp_section, "vpss_grp", 0, file);
        Venc->astVencChnCfg[i].VpssChn   = ini_getl(tmp_section, "vpss_chn", 0, file);
        Venc->astVencChnCfg[i].u32Width  = ini_getl(tmp_section, "width", 0, file);
        Venc->astVencChnCfg[i].u32Height = ini_getl(tmp_section, "height", 0, file);

        ini_gets(tmp_section, "bind_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, venc_bind_mode, VENC_BIND_MAX, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][bind_mode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][bind_mode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Venc->astVencChnCfg[i].enBindMode = enum_num;
        }

        if (Venc->astVencChnCfg[i].enBindMode != VENC_BIND_DISABLE) {
            ini_gets(tmp_section, "src_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][src_mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Venc->astVencChnCfg[i].astChn[0].enModId = enum_num;
            }

            Venc->astVencChnCfg[i].astChn[0].s32DevId = ini_getl(tmp_section, "src_dev_id", 0, file);
            Venc->astVencChnCfg[i].astChn[0].s32ChnId = ini_getl(tmp_section, "src_chn_id", 0, file);
            ini_gets(tmp_section, "dst_mod_id", " ", str_name, PARAM_STRING_NAME_LEN, file);
            ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, mode_id, CVI_ID_BUTT, &enum_num);
            if (ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
            } else {
                APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][dst_mod_id] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
                Venc->astVencChnCfg[i].astChn[1].enModId = enum_num;
            }

            Venc->astVencChnCfg[i].astChn[1].s32DevId = ini_getl(tmp_section, "dst_dev_id", 0, file);
            Venc->astVencChnCfg[i].astChn[1].s32ChnId = ini_getl(tmp_section, "dst_chn_id", 0, file);
        }
        ini_gets(tmp_section, "save_path", " ", tmp_buff, PARAM_STRING_LEN, file);
        strncpy(Venc->astVencChnCfg[i].SavePath, tmp_buff, PARAM_STRING_LEN);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] enType=%d StreamTo=%d VpssGrp=%d VpssChn=%d u32Width=%d u32Height=%d\n save path =%s\n",
            i, Venc->astVencChnCfg[i].enType, Venc->astVencChnCfg[i].StreamTo, Venc->astVencChnCfg[i].VpssGrp, Venc->astVencChnCfg[i].VpssChn,
            Venc->astVencChnCfg[i].u32Width, Venc->astVencChnCfg[i].u32Height, Venc->astVencChnCfg[i].SavePath);

        ini_gets(tmp_section, "rc_mode", " ", str_name, PARAM_STRING_NAME_LEN, file);
        ret = app_ipcam_Param_Convert_StrName_to_EnumNum(str_name, venc_rc_mode, VENC_RC_MODE_BUTT, &enum_num);
        if (ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rc_mode] Fail to convert string name [%s] to enum number!\n", tmp_section, str_name);
        } else {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "[%s][rc_mode] Convert string name [%s] to enum number [%d].\n", tmp_section, str_name, enum_num);
            Venc->astVencChnCfg[i].enRcMode = enum_num;
        }

        buffSize = ini_getl(tmp_section, "bitStreamBuf", 0, file);
        Venc->astVencChnCfg[i].u32StreamBufSize = (buffSize << 10);

        if (Venc->astVencChnCfg[i].enType != PT_JPEG) {

            Venc->astVencChnCfg[i].u32Profile      = ini_getl(tmp_section, "profile", 0, file);
            Venc->astVencChnCfg[i].u32SrcFrameRate = ini_getl(tmp_section, "src_framerate", 0, file);
            Venc->astVencChnCfg[i].u32DstFrameRate = ini_getl(tmp_section, "dst_framerate", 0, file);
            Venc->astVencChnCfg[i].enGopMode       = ini_getl(tmp_section, "gop_mode", 0, file);

            switch (Venc->astVencChnCfg[i].enGopMode) {
                case VENC_GOPMODE_NORMALP:
                    Venc->astVencChnCfg[i].unGopParam.stNormalP.s32IPQpDelta = ini_getl(tmp_section, "NormalP_IPQpDelta", 0, file);
                break;
                case VENC_GOPMODE_SMARTP:
                    Venc->astVencChnCfg[i].unGopParam.stSmartP.s32BgQpDelta  = ini_getl(tmp_section, "SmartP_BgQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stSmartP.s32ViQpDelta  = ini_getl(tmp_section, "SmartP_ViQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stSmartP.u32BgInterval = ini_getl(tmp_section, "SmartP_BgInterval", 0, file);
                break;
                case VENC_GOPMODE_DUALP:
                    Venc->astVencChnCfg[i].unGopParam.stDualP.s32IPQpDelta   = ini_getl(tmp_section, "DualP_IPQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stDualP.s32SPQpDelta   = ini_getl(tmp_section, "DualP_SPQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stDualP.u32SPInterval  = ini_getl(tmp_section, "DualP_SPInterval", 0, file);
                break;
                case VENC_GOPMODE_BIPREDB:
                    Venc->astVencChnCfg[i].unGopParam.stBipredB.s32BQpDelta  = ini_getl(tmp_section, "BipredB_BQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stBipredB.s32IPQpDelta = ini_getl(tmp_section, "BipredB_IPQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stBipredB.u32BFrmNum   = ini_getl(tmp_section, "BipredB_BFrmNum", 0, file);
                break;
                default:
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "venc_chn[%d] gop mode: %d invalid", i, Venc->astVencChnCfg[i].enGopMode);
                break;
            }

            Venc->astVencChnCfg[i].u32BitRate                     = ini_getl(tmp_section, "bit_rate", 0, file);
            Venc->astVencChnCfg[i].u32MaxBitRate                  = ini_getl(tmp_section, "max_bitrate", 0, file);
            Venc->astVencChnCfg[i].bSingleCore                    = ini_getl(tmp_section, "single_core", 0, file);
            Venc->astVencChnCfg[i].u32Gop                         = ini_getl(tmp_section, "gop", 0, file);
            Venc->astVencChnCfg[i].u32IQp                         = ini_getl(tmp_section, "fixed_IQP", 0, file);
            Venc->astVencChnCfg[i].u32PQp                         = ini_getl(tmp_section, "fixed_QPQ", 0, file);
            Venc->astVencChnCfg[i].statTime                       = ini_getl(tmp_section, "statTime", 0, file);
            Venc->astVencChnCfg[i].u32Duration                    = ini_getl(tmp_section, "file_duration", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32ThrdLv            = ini_getl(tmp_section, "ThrdLv", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32FirstFrameStartQp = ini_getl(tmp_section, "firstFrmstartQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32InitialDelay      = ini_getl(tmp_section, "initialDelay", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxIprop          = ini_getl(tmp_section, "MaxIprop", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinIprop          = ini_getl(tmp_section, "MinIprop", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxQp             = ini_getl(tmp_section, "MaxQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinQp             = ini_getl(tmp_section, "MinQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxIQp            = ini_getl(tmp_section, "MaxIQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinIQp            = ini_getl(tmp_section, "MinIQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32ChangePos         = ini_getl(tmp_section, "ChangePos", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32MinStillPercent   = ini_getl(tmp_section, "MinStillPercent", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxStillQP        = ini_getl(tmp_section, "MaxStillQP", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinStillPSNR      = ini_getl(tmp_section, "MinStillPSNR", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MotionSensitivity = ini_getl(tmp_section, "MotionSensitivity", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32AvbrFrmLostOpen   = ini_getl(tmp_section, "AvbrFrmLostOpen", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32AvbrFrmGap        = ini_getl(tmp_section, "AvbrFrmGap", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32AvbrPureStillThr  = ini_getl(tmp_section, "AvbrPureStillThr", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32MaxReEncodeTimes  = ini_getl(tmp_section, "MaxReEncodeTimes", 0, file);

            APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] enRcMode=%d u32BitRate=%d u32MaxBitRate=%d enBindMode=%d bSingleCore=%d\n",
                i, Venc->astVencChnCfg[i].enRcMode, Venc->astVencChnCfg[i].u32BitRate, Venc->astVencChnCfg[i].u32MaxBitRate,
                Venc->astVencChnCfg[i].enBindMode, Venc->astVencChnCfg[i].bSingleCore);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "u32Gop=%d statTime=%d u32ThrdLv=%d\n",
                Venc->astVencChnCfg[i].u32Gop, Venc->astVencChnCfg[i].statTime, Venc->astVencChnCfg[i].stRcParam.u32ThrdLv);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "u32MaxQp=%d u32MinQp=%d u32MaxIQp=%d u32MinIQp=%d s32ChangePos=%d s32InitialDelay=%d\n",
                Venc->astVencChnCfg[i].stRcParam.u32MaxQp, Venc->astVencChnCfg[i].stRcParam.u32MinQp, Venc->astVencChnCfg[i].stRcParam.u32MaxIQp,
                Venc->astVencChnCfg[i].stRcParam.u32MinIQp, Venc->astVencChnCfg[i].stRcParam.s32ChangePos, Venc->astVencChnCfg[i].stRcParam.s32InitialDelay);
        } else {
            Venc->astVencChnCfg[i].stJpegCodecParam.quality   = ini_getl(tmp_section, "quality", 0, file);
            Venc->astVencChnCfg[i].stJpegCodecParam.MCUPerECS = ini_getl(tmp_section, "MCUPerECS", 0, file);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] quality=%d\n", i, Venc->astVencChnCfg[i].stJpegCodecParam.quality);
        }
    }

    int roi_num = ini_getl("roi_config", "max_num", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "roi_max_num: %d\n", roi_num);

    for (i = 0; i < roi_num; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        sprintf(tmp_section, "roi_index%d", i);

        Venc->astRoiCfg[i].u32Index = i;
        Venc->astRoiCfg[i].bEnable = ini_getl(tmp_section, "bEnable", 0, file);
        if (!Venc->astRoiCfg[i].bEnable) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Roi[%d] not enable!\n", i);
            continue;
        }

        Venc->astRoiCfg[i].VencChn   = ini_getl(tmp_section, "venc", 0, file);
        Venc->astRoiCfg[i].bAbsQp    = ini_getl(tmp_section, "absqp", 0, file);
        Venc->astRoiCfg[i].u32Qp     = ini_getl(tmp_section, "qp", 0, file);
        Venc->astRoiCfg[i].u32X      = ini_getl(tmp_section, "x", 0, file);
        Venc->astRoiCfg[i].u32Y      = ini_getl(tmp_section, "y", 0, file);
        Venc->astRoiCfg[i].u32Width  = ini_getl(tmp_section, "width", 0, file);
        Venc->astRoiCfg[i].u32Height = ini_getl(tmp_section, "height", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] bAbsQp=%d u32Qp=%d xy=(%d,%d) wd=(%d,%d)\n",
            Venc->astRoiCfg[i].VencChn, Venc->astRoiCfg[i].bAbsQp, Venc->astRoiCfg[i].u32Qp,
            Venc->astRoiCfg[i].u32X, Venc->astRoiCfg[i].u32Y, Venc->astRoiCfg[i].u32Width,
            Venc->astRoiCfg[i].u32Height);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Roi config ------------------> done \n\n");

    return CVI_SUCCESS;
}

