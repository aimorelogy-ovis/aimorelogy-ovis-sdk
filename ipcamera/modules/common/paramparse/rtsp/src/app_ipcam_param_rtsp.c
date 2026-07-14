#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"

int Load_Param_Rtsp(const char * file)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading RTSP config ------------------> start \n");

    char tmp_section[16] = {0};
    APP_PARAM_RTSP_T *Rtsp = app_ipcam_Rtsp_Param_Get();

    Rtsp->session_cnt = ini_getl("rtsp_config", "rtsp_cnt", 0, file);
    if (Rtsp->session_cnt > RTSP_INSTANCE_NUM) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR
            , "session_cnt:%d exceeds RTSP_INSTANCE_NUM.\n", Rtsp->session_cnt);
        return CVI_FAILURE;
    }

    Rtsp->port = ini_getl("rtsp_config", "port", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp session cnt=%d port:%d\n", Rtsp->session_cnt, Rtsp->port);
    for (int i = 0; i < Rtsp->session_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "session%d", i);

        Rtsp->VencChn[i] = ini_getl(tmp_section, "venc_chn", 0, file);
        Rtsp->bitrate_kbps[i] = ini_getl(tmp_section, "bitrate", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "Vecn Chn=%d Vbitrate=%d\n",
            Rtsp->VencChn[i], Rtsp->bitrate_kbps[i]);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading RTSP config ------------------> done \n\n");

    return CVI_SUCCESS;
}
