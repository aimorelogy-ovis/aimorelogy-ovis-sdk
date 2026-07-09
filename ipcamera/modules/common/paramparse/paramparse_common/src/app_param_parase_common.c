#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_paramparse.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define PARAM_CFG_INI "/mnt/data/param_config.ini"

/**************************************************************************
 *                 V A R I A B L E    D E F I N I T I O N                 *
 **************************************************************************/
static char *input_file;
static char ParamCfgFile[64] = "/mnt/data/param_config.ini";

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
int app_ipcam_Param_Convert_StrName_to_EnumNum(const char *str_name, const char *str_enum[], const int enum_upper_bound, int * const enum_num)
{
    int i = 0;

    for (i = 0; i < enum_upper_bound; ++i) {
        if (str_enum[i] != NULL && strcmp(str_name, str_enum[i]) == 0) {
            *enum_num = i;

            break;
        }
    }

    if (i == enum_upper_bound) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "There is no enum type matching [%s]!\n", str_name);

        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

//weak hook function
__attribute__((weak)) int Load_Param_Module(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Sys(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Vpss(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Vi(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Venc(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Vdec(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Vdec_Soft(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Display(const char* const file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_FrmBuf(const char* const file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Audio(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Osdc(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Stitch(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_GDC(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_BlackLight(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Rtsp(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Gpio(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Pwm(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_MD(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_PD(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_IRFD(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_HD(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_Consumer_Counting(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_FD(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_CRY(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_HumanKeypoint(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_KeypointHandGesture(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_OBJECT_TRACK(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_Img_Txt_Clip(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Ai_LPR(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Record(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

__attribute__((weak)) int Load_Param_Uvc(const char *file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s defalut param \r\n", __func__);
    return 0;
}

//common attribute
const char *pixel_format[PIXEL_FORMAT_MAX] = {
    [PIXEL_FORMAT_RGB_888] = "PIXEL_FORMAT_RGB_888",
    [PIXEL_FORMAT_BGR_888] = "PIXEL_FORMAT_BGR_888",
    [PIXEL_FORMAT_RGB_888_PLANAR] = "PIXEL_FORMAT_RGB_888_PLANAR",
    [PIXEL_FORMAT_BGR_888_PLANAR] = "PIXEL_FORMAT_BGR_888_PLANAR",
    [PIXEL_FORMAT_ARGB_1555] = "PIXEL_FORMAT_ARGB_1555",
    [PIXEL_FORMAT_ARGB_4444] = "PIXEL_FORMAT_ARGB_4444",
    [PIXEL_FORMAT_ARGB_8888] = "PIXEL_FORMAT_ARGB_8888",
    [PIXEL_FORMAT_RGB_BAYER_8BPP] = "PIXEL_FORMAT_RGB_BAYER_8BPP",
    [PIXEL_FORMAT_RGB_BAYER_10BPP] = "PIXEL_FORMAT_RGB_BAYER_10BPP",
    [PIXEL_FORMAT_RGB_BAYER_12BPP] = "PIXEL_FORMAT_RGB_BAYER_12BPP",
    [PIXEL_FORMAT_RGB_BAYER_14BPP] = "PIXEL_FORMAT_RGB_BAYER_14BPP",
    [PIXEL_FORMAT_RGB_BAYER_16BPP] = "PIXEL_FORMAT_RGB_BAYER_16BPP",
    [PIXEL_FORMAT_YUV_PLANAR_422] = "PIXEL_FORMAT_YUV_PLANAR_422",
    [PIXEL_FORMAT_YUV_PLANAR_420] = "PIXEL_FORMAT_YUV_PLANAR_420",
    [PIXEL_FORMAT_YUV_PLANAR_444] = "PIXEL_FORMAT_YUV_PLANAR_444",
    [PIXEL_FORMAT_YUV_400] = "PIXEL_FORMAT_YUV_400",
    [PIXEL_FORMAT_HSV_888] = "PIXEL_FORMAT_HSV_888",
    [PIXEL_FORMAT_HSV_888_PLANAR] = "PIXEL_FORMAT_HSV_888_PLANAR",
    [PIXEL_FORMAT_NV12] = "PIXEL_FORMAT_NV12",
    [PIXEL_FORMAT_NV21] = "PIXEL_FORMAT_NV21",
    [PIXEL_FORMAT_NV16] = "PIXEL_FORMAT_NV16",
    [PIXEL_FORMAT_NV61] = "PIXEL_FORMAT_NV61",
    [PIXEL_FORMAT_YUYV] = "PIXEL_FORMAT_YUYV",
    [PIXEL_FORMAT_UYVY] = "PIXEL_FORMAT_UYVY",
    [PIXEL_FORMAT_YVYU] = "PIXEL_FORMAT_YVYU",
    [PIXEL_FORMAT_VYUY] = "PIXEL_FORMAT_VYUY",
    [PIXEL_FORMAT_FP32_C1] = "PIXEL_FORMAT_FP32_C1",
    [PIXEL_FORMAT_FP32_C3_PLANAR] = "PIXEL_FORMAT_FP32_C3_PLANAR",
    [PIXEL_FORMAT_INT32_C1] = "PIXEL_FORMAT_INT32_C1",
    [PIXEL_FORMAT_INT32_C3_PLANAR] = "PIXEL_FORMAT_INT32_C3_PLANAR",
    [PIXEL_FORMAT_UINT32_C1] = "PIXEL_FORMAT_UINT32_C1",
    [PIXEL_FORMAT_UINT32_C3_PLANAR] = "PIXEL_FORMAT_UINT32_C3_PLANAR",
    [PIXEL_FORMAT_BF16_C1] = "PIXEL_FORMAT_BF16_C1",
    [PIXEL_FORMAT_BF16_C3_PLANAR] = "PIXEL_FORMAT_BF16_C3_PLANAR",
    [PIXEL_FORMAT_INT16_C1] = "PIXEL_FORMAT_INT16_C1",
    [PIXEL_FORMAT_INT16_C3_PLANAR] = "PIXEL_FORMAT_INT16_C3_PLANAR",
    [PIXEL_FORMAT_UINT16_C1] = "PIXEL_FORMAT_UINT16_C1",
    [PIXEL_FORMAT_UINT16_C3_PLANAR] = "PIXEL_FORMAT_UINT16_C3_PLANAR",
    [PIXEL_FORMAT_INT8_C1] = "PIXEL_FORMAT_INT8_C1",
    [PIXEL_FORMAT_INT8_C3_PLANAR] = "PIXEL_FORMAT_INT8_C3_PLANAR",
    [PIXEL_FORMAT_UINT8_C1] = "PIXEL_FORMAT_UINT8_C1",
    [PIXEL_FORMAT_UINT8_C3_PLANAR] = "PIXEL_FORMAT_UINT8_C3_PLANAR",
    [PIXEL_FORMAT_8BIT_MODE] = "PIXEL_FORMAT_8BIT_MODE"
};

const char *data_bitwidth[DATA_BITWIDTH_MAX] = {
    [DATA_BITWIDTH_8] = "DATA_BITWIDTH_8",
    [DATA_BITWIDTH_10] = "DATA_BITWIDTH_10",
    [DATA_BITWIDTH_12] = "DATA_BITWIDTH_12",
    [DATA_BITWIDTH_14] = "DATA_BITWIDTH_14",
    [DATA_BITWIDTH_16] = "DATA_BITWIDTH_16"
};

const char *compress_mode[COMPRESS_MODE_BUTT] = {
    [COMPRESS_MODE_NONE] = "COMPRESS_MODE_NONE",
    [COMPRESS_MODE_TILE] = "COMPRESS_MODE_TILE",
    [COMPRESS_MODE_LINE] = "COMPRESS_MODE_LINE",
    [COMPRESS_MODE_FRAME] = "COMPRESS_MODE_FRAME"
};

const char *vi_vpss_mode[VI_VPSS_MODE_BUTT] = {
    [VI_OFFLINE_VPSS_OFFLINE] = "VI_OFFLINE_VPSS_OFFLINE",
    [VI_OFFLINE_VPSS_ONLINE] = "VI_OFFLINE_VPSS_ONLINE",
    [VI_SLICE_VPSS_OFFLINE] = "VI_SLICE_VPSS_OFFLINE",
    [VI_SLICE_VPSS_ONLINE] = "VI_SLICE_VPSS_ONLINE",
    [VI_ONLINE_VPSS_OFFLINE] = "VI_ONLINE_VPSS_OFFLINE",
    [VI_ONLINE_VPSS_ONLINE] = "VI_ONLINE_VPSS_ONLINE",
};

const char *mode_id[CVI_ID_BUTT] = {
    [CVI_ID_BASE] = "CVI_ID_BASE",
    [CVI_ID_VB] = "CVI_ID_VB",
    [CVI_ID_SYS] = "CVI_ID_SYS",
    [CVI_ID_RGN] = "CVI_ID_RGN",
    [CVI_ID_CHNL] = "CVI_ID_CHNL",
    [CVI_ID_VDEC] = "CVI_ID_VDEC",
    [CVI_ID_VPSS] = "CVI_ID_VPSS",
    [CVI_ID_VENC] = "CVI_ID_VENC",
    [CVI_ID_H264E] = "CVI_ID_H264E",
    [CVI_ID_JPEGE] = "CVI_ID_JPEGE",
    [CVI_ID_MPEG4E] = "CVI_ID_MPEG4E",
    [CVI_ID_H265E] = "CVI_ID_H265E",
    [CVI_ID_JPEGD] = "CVI_ID_JPEGD",
    [CVI_ID_VO] = "CVI_ID_VO",
    [CVI_ID_VI] = "CVI_ID_VI",
    [CVI_ID_DIS] = "CVI_ID_DIS",
    [CVI_ID_RC] = "CVI_ID_RC",
    [CVI_ID_AIO] = "CVI_ID_AIO",
    [CVI_ID_AI] = "CVI_ID_AI",
    [CVI_ID_AO] = "CVI_ID_AO",
    [CVI_ID_AENC] = "CVI_ID_AENC",
    [CVI_ID_ADEC] = "CVI_ID_ADEC",
    [CVI_ID_AUD] = "CVI_ID_AUD",
    [CVI_ID_VPU] = "CVI_ID_VPU",
    [CVI_ID_ISP] = "CVI_ID_ISP",
    [CVI_ID_IVE] = "CVI_ID_IVE",
    [CVI_ID_USER] = "CVI_ID_USER",
    [CVI_ID_PROC] = "CVI_ID_PROC",
    [CVI_ID_LOG] = "CVI_ID_LOG",
    [CVI_ID_H264D] = "CVI_ID_H264D",
    [CVI_ID_GDC] = "CVI_ID_GDC",
    [CVI_ID_PHOTO] = "CVI_ID_PHOTO",
    [CVI_ID_FB] = "CVI_ID_FB"
};

const char *video_format[VIDEO_FORMAT_MAX] = {
    [VIDEO_FORMAT_LINEAR] = "VIDEO_FORMAT_LINEAR"
};

const char *payload_type[PT_BUTT] = {
    [PT_PCMU] = "PT_PCMU",
    [PT_1016] = "PT_1016",
    [PT_G721] = "PT_G721",
    [PT_GSM] = "PT_GSM",
    [PT_G723] = "PT_G723",
    [PT_DVI4_8K] = "PT_DVI4_8K",
    [PT_DVI4_16K] = "PT_DVI4_16K",
    [PT_LPC] = "PT_LPC",
    [PT_PCMA] = "PT_PCMA",
    [PT_G722] = "PT_G722",
    [PT_S16BE_STEREO] = "PT_S16BE_STEREO",
    [PT_S16BE_MONO] = "PT_S16BE_MONO",
    [PT_QCELP] = "PT_QCELP",
    [PT_CN] = "PT_CN",
    [PT_MPEGAUDIO] = "PT_MPEGAUDIO",
    [PT_G728] = "PT_G728",
    [PT_DVI4_3] = "PT_DVI4_3",
    [PT_DVI4_4] = "PT_DVI4_4",
    [PT_G729] = "PT_G729",
    [PT_G711A] = "PT_G711A",
    [PT_G711U] = "PT_G711U",
    [PT_G726] = "PT_G726",
    [PT_G729A] = "PT_G729A",
    [PT_LPCM] = "PT_LPCM",
    [PT_CelB] = "PT_CelB",
    [PT_JPEG] = "PT_JPEG",
    [PT_CUSM] = "PT_CUSM",
    [PT_NV] = "PT_NV",
    [PT_PICW] = "PT_PICW",
    [PT_CPV] = "PT_CPV",
    [PT_H261] = "PT_H261",
    [PT_MPEGVIDEO] = "PT_MPEGVIDEO",
    [PT_MPEG2TS] = "PT_MPEG2TS",
    [PT_H263] = "PT_H263",
    [PT_SPEG] = "PT_SPEG",
    [PT_MPEG2VIDEO] = "PT_MPEG2VIDEO",
    [PT_AAC] = "PT_AAC",
    [PT_WMA9STD] = "PT_WMA9STD",
    [PT_HEAAC] = "PT_HEAAC",
    [PT_PCM_VOICE] = "PT_PCM_VOICE",
    [PT_PCM_AUDIO] = "PT_PCM_AUDIO",
    [PT_MP3] = "PT_MP3",
    [PT_ADPCMA] = "PT_ADPCMA",
    [PT_AEC] = "PT_AEC",
    [PT_X_LD] = "PT_X_LD",
    [PT_H264] = "PT_H264",
    [PT_D_GSM_HR] = "PT_D_GSM_HR",
    [PT_D_GSM_EFR] = "PT_D_GSM_EFR",
    [PT_D_L8] = "PT_D_L8",
    [PT_D_RED] = "PT_D_RED",
    [PT_D_VDVI] = "PT_D_VDVI",
    [PT_D_BT656] = "PT_D_BT656",
    [PT_D_H263_1998] = "PT_D_H263_1998",
    [PT_D_MP1S] = "PT_D_MP1S",
    [PT_D_MP2P] = "PT_D_MP2P",
    [PT_D_BMPEG] = "PT_D_BMPEG",
    [PT_MP4VIDEO] = "PT_MP4VIDEO",
    [PT_MP4AUDIO] = "PT_MP4AUDIO",
    [PT_VC1] = "PT_VC1",
    [PT_JVC_ASF] = "PT_JVC_ASF",
    [PT_D_AVI] = "PT_D_AVI",
    [PT_DIVX3] = "PT_DIVX3",
    [PT_AVS] = "PT_AVS",
    [PT_REAL8] = "PT_REAL8",
    [PT_REAL9] = "PT_REAL9",
    [PT_VP6] = "PT_VP6",
    [PT_VP6F] = "PT_VP6F",
    [PT_VP6A] = "PT_VP6A",
    [PT_SORENSON] = "PT_SORENSON64",
    [PT_H265] = "PT_H265",
    [PT_VP8] = "PT_VP8",
    [PT_MVC] = "PT_MVC",
    [PT_PNG] = "PT_PNG",
    [PT_AMR] = "PT_AMR",
    [PT_MJPEG] = "PT_MJPEG"
};

const char ** app_ipcam_Param_get_pixel_format()
{
    return pixel_format;
}

const char ** app_ipcam_Param_get_data_bitwidth()
{
    return data_bitwidth;
}

const char ** app_ipcam_Param_get_compress_mode()
{
    return compress_mode;
}

const char ** app_ipcam_Param_get_vi_vpss_mode()
{
    return vi_vpss_mode;
}

const char ** app_ipcam_Param_get_mode_id()
{
    return mode_id;
}

const char ** app_ipcam_Param_get_video_format()
{
    return video_format;
}

const char ** app_ipcam_Param_get_payload_type()
{
    return payload_type;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [-i]\n", prog);
    puts("  -i --input    input param config ini file (e.g. \"/mnt/data/param_config.ini\")\n");
    exit(1);
}

int app_ipcam_Opts_Parse(int argc, char *argv[])
{
    while (1) {
        static const struct option lopts[] = {
            { "input",   1, 0, 'i' },
            { NULL, 0, 0, 0 },
        };
        int c;

        c = getopt_long(argc, argv, "i:",
                lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'i':
            input_file = optarg;
            APP_CHK_RET(access(input_file, F_OK), "param config ini file access");
            strncpy(ParamCfgFile, input_file, 64);
            break;
        default:
            print_usage(argv[0]);
            break;
        }
    }

    return CVI_SUCCESS;
}


int app_ipcam_Param_Load(void)
{
    APP_CHK_RET(access(ParamCfgFile, F_OK), "param_config.ini access");
    APP_CHK_RET(Load_Param_Module(ParamCfgFile), "Load Module Param");
    APP_CHK_RET(Load_Param_Sys(ParamCfgFile), "Load SysVb Param");
    APP_CHK_RET(Load_Param_Vi(ParamCfgFile), "Load VI Param");
    APP_CHK_RET(Load_Param_Vpss(ParamCfgFile), "Load VPSS Param");
    APP_CHK_RET(Load_Param_Venc(ParamCfgFile), "Load VENC Param");
    APP_CHK_RET(Load_Param_Vdec(ParamCfgFile), "Load VDEC Param");
    APP_CHK_RET(Load_Param_Display(ParamCfgFile), "Load Display Param");
    APP_CHK_RET(Load_Param_Audio(ParamCfgFile), "Load Audio Param");
    APP_CHK_RET(Load_Param_Osdc(ParamCfgFile), "Load OSDC Param");
    APP_CHK_RET(Load_Param_Rtsp(ParamCfgFile), "Load RTSP Param");
    APP_CHK_RET(Load_Param_Gpio(ParamCfgFile), "Load GPIO Param");
    APP_CHK_RET(Load_Param_Pwm(ParamCfgFile), "Load PWM Param");
    APP_CHK_RET(Load_Param_Ai_MD(ParamCfgFile), "load MD Param");
    APP_CHK_RET(Load_Param_Ai_PD(ParamCfgFile), "Load AI PD Param");
    APP_CHK_RET(Load_Param_Ai_FD(ParamCfgFile), "Load AI FD Param");
    APP_CHK_RET(Load_Param_Ai_IRFD(ParamCfgFile), "Load AI IR FD Param");
    APP_CHK_RET(Load_Param_Ai_CRY(ParamCfgFile), "Load AI Cry Param");
    APP_CHK_RET(Load_Param_Ai_HumanKeypoint(ParamCfgFile), "Load AI Human Keypoint Param");
    APP_CHK_RET(Load_Param_Ai_OBJECT_TRACK(ParamCfgFile), "Load AI OBJECT TRACK Param");
    APP_CHK_RET(Load_Param_Ai_KeypointHandGesture(ParamCfgFile), "Load AI Keypoint Hand Gesture Param");
    APP_CHK_RET(Load_Param_Ai_OBJECT_TRACK(ParamCfgFile), "Load AI Object Track Param");
    APP_CHK_RET(Load_Param_Ai_Img_Txt_Clip(ParamCfgFile), "Load AI Image Txt Clip Param");
    APP_CHK_RET(Load_Param_Ai_LPR(ParamCfgFile), "Load AI LPR Param");
    APP_CHK_RET(Load_Param_Record(ParamCfgFile), "Load Record Param");
    APP_CHK_RET(Load_Param_Uvc(ParamCfgFile), "Load UVC Param");
    return CVI_SUCCESS;
}
