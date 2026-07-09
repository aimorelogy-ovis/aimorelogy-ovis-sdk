#ifndef __APP_IPCAM_PARAM_PARSE_H__
#define __APP_IPCAM_PARAM_PARSE_H__

#include "cvi_common.h"
#include "cvi_comm_video.h"
#include "app_ipcam_comm.h"
#include "app_ipcam_sys.h"
#include "app_ipcam_vi.h"
#include "app_ipcam_vpss.h"
#include "app_ipcam_venc.h"
#include "app_ipcam_mq.h"
#include "app_ipcam_ircut.h"
#include "app_ipcam_module.h"

#ifdef OSDC_SUPPORT
#include "app_ipcam_osd.h"
#endif

#ifdef RTSP_SUPPORT
#include "app_ipcam_rtsp.h"
#endif

#ifdef AUDIO_SUPPORT
#include "app_ipcam_audio.h"
#endif

#ifdef VDEC_SUPPORT
#include "app_ipcam_vdec.h"
#endif

#ifdef DISPLAY
#include "app_ipcam_display.h"
#endif

#ifdef AI_SUPPORT
#include "app_ipcam_ai.h"
#endif

#ifdef RECORD_SUPPORT
#include "app_ipcam_record.h"
#endif

#ifdef WEB_SOCKET
#include "app_ipcam_websocket.h"
#include "app_ipcam_netctrl.h"
#endif

#ifdef CVI_UVC_SUPPORT
#include "cvi_uvc.h"
#endif

#ifdef CVI_UVC_HOST_SUPPORT
#include "app_ipcam_uvc_host.h"
#endif

#ifdef CVI_UAC_SUPPORT
#include "cvi_audio_uac.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define PARAM_STRING_LEN (64)
#define PARAM_STRING_NAME_LEN (128)

#define VI_FRAMERATE			15

int app_ipcam_Param_Convert_StrName_to_EnumNum(const char *str_name, const char *str_enum[], const int enum_upper_bound, int * const enum_num);
//common attribute
const char ** app_ipcam_Param_get_pixel_format();
const char ** app_ipcam_Param_get_data_bitwidth();
const char ** app_ipcam_Param_get_compress_mode();
const char ** app_ipcam_Param_get_vi_vpss_mode();
const char ** app_ipcam_Param_get_mode_id();
const char ** app_ipcam_Param_get_video_format();
const char ** app_ipcam_Param_get_payload_type();
#ifdef AI_SUPPORT
const char ** app_ipcam_Param_get_ai_supported_model();
#endif

int app_ipcam_Param_Load(void);

int app_ipcam_Opts_Parse(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif
