#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "app_ipcam_paramparse.h"
#include "cvi_mbuf.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static pthread_t g_pthMisc;
static CVI_BOOL g_bMisc;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int app_ipcam_Exit(void);

static CVI_VOID app_ipcam_ExitSig_handle(CVI_S32 signo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    if ((SIGINT == signo) || (SIGTERM == signo)) {
        app_ipcam_Exit();
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate\n", signo);
    }

    exit(s32Ret);
}

static CVI_VOID app_ipcam_Usr1Sig_handle(CVI_S32 signo)
{
    if (SIGUSR1 == signo) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate and start trigger a picture\n", signo);
        app_ipcam_JpgCapFlag_Set(CVI_TRUE);
    }
}

static int app_ipcam_Peripheral_Init(void)
{
    /* do peripheral Init here */
    app_ipcam_IRCut_Init();

    return CVI_SUCCESS;
}

static int app_ipcam_Peripheral_UnInit(void)
{
    /* do peripheral Init here */
    app_ipcam_IrCut_DeInit();

    return CVI_SUCCESS;
}

/*
* this thread handle a series of small tasks include
* a. send AI framerate to Web-client
* b.
*/
static void *ThreadMisc(void *arg)
{
    while (g_bMisc) {
        #ifdef WEB_SOCKET
        app_ipcam_WebSocket_AiFps_Send();
        #endif
        sleep(1);
    }

    return NULL;
}

static int app_ipcam_MiscThread_Init(void)
{
    int s32Ret = CVI_SUCCESS;

    g_bMisc = CVI_TRUE;
    s32Ret = pthread_create(&g_pthMisc, NULL, ThreadMisc, NULL);
    if (s32Ret != 0) {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG,"pthread_create failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static int app_ipcam_MiscThread_DeInit(void)
{
    g_bMisc = CVI_FALSE;

    if (g_pthMisc) {
        pthread_cancel(g_pthMisc);
        pthread_join(g_pthMisc, NULL);
        g_pthMisc = 0;
    }

    return CVI_SUCCESS;
}

static int app_ipcam_Exit(void)
{
    APP_CHK_RET(app_ipcam_MiscThread_DeInit(), "DeInit Misc Process");

    #ifdef RTSP_SUPPORT
    APP_CHK_RET(app_ipcam_rtsp_Server_Destroy(), "RTSP Server Destroy");
    #endif

    #ifdef CVI_UVC_SUPPORT
    app_uvc_exit();
    #endif

    #ifdef CVI_UVC_HOST_SUPPORT
    APP_CHK_RET(app_ipcam_UvcHost_DeInit(), "deinit uvc host module");
    #endif

    #ifdef CVI_UAC_SUPPORT
    app_uac_exit();
    #endif

    #ifdef RECORD_SUPPORT
    APP_CHK_RET(app_ipcam_Record_UnInit(), "running SD Record");
    #endif

    #ifdef AI_SUPPORT
    #ifdef PD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_PD_Stop(), "PD Stop");
    #endif
    #ifdef MD_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_MD_Stop(), "MD Stop");
    #endif
    #ifdef FACE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_FD_Stop(), "FD Stop");
    #endif
    #ifdef HUMAN_KEYPOINT_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Human_Keypoint_Stop(), "Human Keypoint Detection Stop");
    #endif
    #ifdef OBJECT_TRACK_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Object_Track_Stop(), "ObjectTrack Stop");
    #endif
    #ifdef KEYPOINT_HAND_GESTURE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Keypoint_Hand_Gesture_Stop(), "Keypoint Hand Gesture Detection Stop");
    #endif
    #ifdef IMG_TXT_CLIP_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Img_Txt_Clip_Stop(), "Img Txt Clip Stop");
    #endif
    #ifdef LPR_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_LPR_Stop(), "LPR Stop");
    #endif
    #endif

    #ifdef OSDC_SUPPORT
    APP_CHK_RET(app_ipcam_Osdc_DeInit(), "OsdC DeInit");
    #endif

    #ifdef AUDIO_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_UnInit(), "Audio Stop");
    #endif

    #ifdef VDEC_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Stop(), "VDEC Stop");
    #endif

    #ifdef DISPLAY
    APP_CHK_RET(app_ipcam_Display_Exit(), "Display Exit");
    #endif

    APP_CHK_RET(app_ipcam_Vpss_DeInit(), "Vpss DeInit");

    APP_CHK_RET(app_ipcam_Venc_Stop(APP_VENC_ALL), "Venc Stop");

    APP_CHK_RET(app_ipcam_Vi_DeInit(), "Vi DeInit");

    APP_CHK_RET(app_ipcam_Sys_DeInit(), "System DeInit");

    #ifdef WEB_SOCKET
    APP_CHK_RET(app_ipcam_WebSocket_DeInit(), "WebSocket DeInit");
    APP_CHK_RET(app_ipcam_NetCtrl_DeInit(), "NetCtrl DeInit");
    #endif

    APP_CHK_RET(app_ipcam_Peripheral_UnInit(), "UnInit Peripheral");

    #ifdef ANONMSG_ENABLE
    APP_CHK_RET(app_ipcam_MsgAnonDeInit(), "MsgAnon DeInit");
    #endif
    APP_CHK_RET(app_ipcam_Mbuf_UnInit(), "UnInit Mbuf");

    return CVI_SUCCESS;

}

static int app_ipcam_Init(void)
{
    APP_CHK_RET(app_ipcam_Peripheral_Init(), "Init Peripheral");

    APP_CHK_RET(app_ipcam_Sys_Init(), "init systerm");

    APP_CHK_RET(app_ipcam_Vi_Init(), "init vi module");

    APP_CHK_RET(app_ipcam_Vpss_Init(), "init vpss module");

    #ifdef OSDC_SUPPORT
    APP_CHK_RET(app_ipcam_Osdc_Init(), "init Draw Osdc");
    #endif

    #ifdef WEB_SOCKET
    APP_CHK_RET(app_ipcam_NetCtrl_Init(), "Net Ctrl init");

    APP_CHK_RET(app_ipcam_WebSocket_Init(), "websocket init");
    #endif

    APP_CHK_RET(app_ipcam_Venc_Init(APP_VENC_ALL), "init video encode");

    #ifdef VDEC_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Init(), "Init VDEC");
    #endif

    #ifdef AUDIO_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_Init(), "start audio processing");
    #endif

    #ifdef CVI_UVC_SUPPORT
    app_uvc_init();
    #endif

    #ifdef CVI_UVC_HOST_SUPPORT
    APP_CHK_RET(app_ipcam_UvcHost_Init(), "init uvc host module");
    #endif

    #ifdef CVI_UAC_SUPPORT
    app_uac_init();
    #endif

    #ifdef DISPLAY
    APP_CHK_RET(app_ipcam_Display_Init(), "Init Display");
    #endif

    APP_CHK_RET(app_ipcam_MiscThread_Init(), "Init Misc Process");

    return CVI_SUCCESS;
}

int main(int argc, char *argv[])
{
    system("echo /mnt/nfs/core-%e-%p-%t > /proc/sys/kernel/core_pattern");
    APP_CHK_RET(app_ipcam_Opts_Parse(argc, argv), "parse optinos");

    signal(SIGINT, app_ipcam_ExitSig_handle);
    signal(SIGTERM, app_ipcam_ExitSig_handle);
    signal(SIGUSR1, app_ipcam_Usr1Sig_handle);

    /* load each moudles parameter from param_config.ini */
    APP_CHK_RET(app_ipcam_Param_Load(), "load global parameter");

    APP_CHK_RET(app_ipcam_Mbuf_Init(), "Init Mbuf");

    /* init modules include <Peripheral; Sys; VI; VB; OSD; Venc; AI; Audio; etc.> */
    APP_CHK_RET(app_ipcam_Init(), "app_ipcam_Init");

    #ifdef RTSP_SUPPORT
    /* create rtsp server */
    APP_CHK_RET(app_ipcam_Rtsp_Server_Create(), "create rtsp server");
    #endif

    /* start video encode */
    APP_CHK_RET(app_ipcam_Venc_Start(APP_VENC_ALL), "start video processing");

    #ifdef VDEC_SUPPORT
    APP_CHK_RET(app_ipcam_Vdec_Start(), "Start VDEC");
    #endif

    #ifdef AI_SUPPORT
    #ifdef PD_SUPPORT
    /* start AI PD (Pedestrian Detection) */
    APP_CHK_RET(app_ipcam_Ai_PD_Start(), "running AI PD");
    #endif

    #ifdef MD_SUPPORT
    /* start AI MD (Motion Detection)*/
    APP_CHK_RET(app_ipcam_Ai_MD_Start(), "running AI MD");
    #endif

    /* start AI FD (Face Detection)*/
    #ifdef FACE_SUPPORT
    #ifdef IR_FACE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_IR_FD_Start(), "running AI IR FD");
    #else
    APP_CHK_RET(app_ipcam_Ai_FD_Start(), "running AI FD");
    #endif
    #endif

    #ifdef AUDIO_SUPPORT
    #ifdef AI_BABYCRY_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Cry_Start(), "running AI CRY");
    #endif
    #endif

    #ifdef HUMAN_KEYPOINT_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Human_Keypoint_Start(), "Human Keypoint Detection Start");
    #endif

    #ifdef OBJECT_TRACK_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Object_Track_Start(), "ObjectTrack Start");
    #endif

    #ifdef KEYPOINT_HAND_GESTURE_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Keypoint_Hand_Gesture_Start(), "Keypoint Hand Gesture Detection Start");
    #endif

    #ifdef IMG_TXT_CLIP_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_Img_Txt_Clip_Start(), "Img Txt Clip Stop");
    #endif

    #ifdef LPR_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_LPR_Start(), "running AI LPR");
    #endif
    #endif

    #ifdef RECORD_SUPPORT
    /* start SD Record */
    APP_CHK_RET(app_ipcam_Record_Recover_Init(), "Init SD Record Recover");
    APP_CHK_RET(app_ipcam_Record_Init(), "running SD Record");
    #endif

    /* enable receive a command form another progress for test ipcam */
    //APP_CHK_RET(app_ipcam_CmdTask_Create(), "running cmd test");

    while (1) {
        sleep(1);
    };

    return CVI_SUCCESS;
}
