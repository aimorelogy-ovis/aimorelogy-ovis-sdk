#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "app_ipcam_comm.h"
#include "app_ipcam_websocket.h"
#include "libwebsockets.h"
#include "cvi_mbuf.h"

#ifdef AI_SUPPORT
#include "app_ipcam_ai.h"
#endif

#define MAX_PAYLOAD_SIZE (1024 * 1024)
#define RX_BUFFER_SIZE      128
#define MAIN_STREAM_FLAG "main_stream"
#define SUB_STREAM_FLAG  "sub_stream"

typedef enum RX_STATUS_S{
    RX_STATUS_WAIT = 0,
    RX_STATUS_OPEN,
} WS_RX_STATUS_E;

typedef enum STREAM_NUM_S  {
    MAIN_STREAM = 0,
    SUB_STREAM = 1,
    STREAM_MAX
} WS_STREAM_TYPE_E;

typedef enum MSG_TYPE_S {
    MSG_TYPE_VIDEO = 0,
    MSG_TYPE_AIFPS = 1,
    MSG_TYPE_MAX = 2,
} WS_MSG_TYPE_E;

typedef struct {
    int stream_id;
    CVI_MBUF_HANDLE *reader;
    CVI_MEDIA_FRAME_INFO_T frame_info;
} stream_context;

static stream_context g_streams[STREAM_MAX];
struct lws *g_wsi = NULL;
static volatile int g_terminal = 0;
static int g_current_stream = MAIN_STREAM; // main default
static unsigned char *g_aifps_data = NULL;
static int g_aifps_size = 0;
pthread_mutex_t g_aiMutexLock = PTHREAD_MUTEX_INITIALIZER;
pthread_t g_ws_thread;

struct sessionData_s
{
    int msg_count;
    unsigned char buf[MAX_PAYLOAD_SIZE];
    int len;
    int bin;
    int fin;
};

int app_ipcam_WebSocketChn_Get()
{
    return g_current_stream;
}

int app_ipcam_WebSocketChn_Set(int venc_chn)
{
    g_current_stream = venc_chn;

    return 0;
}

static void CVI_IPC_WebsocketRequestIDR(int venc_chn)
{
    CVI_VENC_RequestIDR(venc_chn, 1);
}

static void init_frame_streams()
{
    for (int i = 0; i < STREAM_MAX; i++) {
        g_streams[i].stream_id = i;
        g_streams[i].reader = NULL;
        memset(&g_streams[i].frame_info.frameParam, 0, sizeof(g_streams[i].frame_info.frameParam));
        g_streams[i].frame_info.frameBuf = malloc(CVI_MBUF_STREAM_MAX_SIZE);
        if (NULL == g_streams[i].frame_info.frameBuf) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, " %d stream frameBuf malloc fail\n", i);
            return;
        }
    }
}

static void deinit_frame_streams()
{
    for (int i = 0; i < STREAM_MAX; i++) {

        if(g_streams[i].reader){
            app_ipcam_Mbuf_DestoryReader(g_streams[i].reader);
        }

        if(g_streams[i].frame_info.frameBuf){
            free(g_streams[i].frame_info.frameBuf);
        }

    }
}

static void destroy_buf_reader(stream_context *stream)
{
    if (stream->reader) {
        app_ipcam_Mbuf_DestoryReader(stream->reader);
        stream->reader = NULL;
    }
}

static void switch_to_stream(int stream_id)
{
    for (int i = 0; i < STREAM_MAX; i++) {

        stream_context *stream = &g_streams[i];

        if (stream->stream_id == stream_id) {
            for (int j = 0; j < STREAM_MAX; j++) {
                if (j != i) {
                    destroy_buf_reader(&g_streams[j]);
                }
            }

            if (NULL == stream->reader) {
                stream->reader = app_ipcam_Mbuf_CreateReader(stream_id, 1);
                CVI_IPC_WebsocketRequestIDR(stream_id);
            }
            break;
        }
    }
}

static void handle_stream_writeable(struct lws *wsi, int stream_id)
{
    for (int i = 0; i < STREAM_MAX; i++) {

        stream_context *stream = &g_streams[i];
        // note：send other type msg shoudle before video data
        if(MAIN_STREAM == stream_id) {
            pthread_mutex_lock(&g_aiMutexLock);
            if (g_aifps_size != 0 && g_aifps_data != NULL) {
                lws_write(wsi, g_aifps_data + LWS_PRE, g_aifps_size, LWS_WRITE_BINARY);
            }
            if (g_aifps_data) {
                free(g_aifps_data);
                g_aifps_data = NULL;
            }
            g_aifps_size = 0;
            pthread_mutex_unlock(&g_aiMutexLock);
        }

        if (stream->stream_id == stream_id && stream->reader) {

            lws_callback_on_writable(wsi);
            memset(stream->frame_info.frameBuf, 0, CVI_MBUF_STREAM_MAX_SIZE);
            stream->frame_info.frameBufLen = CVI_MBUF_STREAM_MAX_SIZE;
            if (0 < app_ipcam_Mbuf_ReadFrame(stream->reader, 0, &stream->frame_info, 100)) {
                lws_write(wsi, stream->frame_info.frameBuf, stream->frame_info.frameParam.frameLen + 1, LWS_WRITE_BINARY);
            }
            break;
        }
    }
}

static void cleanup_resources()
{
    for (int i = 0; i < STREAM_MAX; i++) {
        destroy_buf_reader(&g_streams[i]);
    }

    pthread_mutex_lock(&g_aiMutexLock);
    if (g_aifps_data) {
        free(g_aifps_data);
        g_aifps_data = NULL;
    }
    g_aifps_size = 0;
    pthread_mutex_unlock(&g_aiMutexLock);
}

static int stream_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    switch (reason)
    {
        case LWS_CALLBACK_ESTABLISHED:
            printf("Stream LWS_CALLBACK_ESTABLISHED!\n");

            g_wsi = wsi; // for other msg send
            switch_to_stream(g_current_stream); // main default
            break;

        case LWS_CALLBACK_RECEIVE:
            lws_rx_flow_control(wsi, RX_STATUS_WAIT);
            if (len > 0) {
                char *msg = (char *)in;
                if (strncmp(msg, MAIN_STREAM_FLAG, len) == 0) {
                    printf("======SWITCH TO MAIN STREAM======\n");
                    g_current_stream = MAIN_STREAM;
                    switch_to_stream(MAIN_STREAM);
                } else if (strncmp(msg, SUB_STREAM_FLAG, len) == 0) {
                    printf("======SWITCH TO SUB  STREAM======\n");
                    g_current_stream = SUB_STREAM;
                    switch_to_stream(SUB_STREAM);
                }
            }
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            handle_stream_writeable(wsi, g_current_stream);
            lws_rx_flow_control(wsi, RX_STATUS_OPEN);
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf("Stream LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
            break;

        case LWS_CALLBACK_WSI_DESTROY:
            printf("Stream LWS_CALLBACK_WSI_DESTROY\n");
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Stream LWS_CALLBACK_CLOSED\n");
            cleanup_resources();
            g_wsi = NULL;
            break;

        default:
            break;
    }

    return 0;
}

const struct lws_protocols protocols[] = {
    {"stream",
      stream_callback,
      sizeof(struct sessionData_s),
      RX_BUFFER_SIZE, 0, NULL, 0},
    { NULL, NULL, 0, 0 ,0, 0, 0 }       // Must end with an empty protocol
};

static void *ThreadWebsocket(void *arg)
{
    struct lws_context_creation_info ctx_info = {0};
    ctx_info.port = 8000;
    ctx_info.iface = NULL;  // Listen on all network interfaces
    ctx_info.protocols = protocols;
    ctx_info.gid = -1;
    ctx_info.uid = -1;
    ctx_info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;

    // ctx_info.ssl_ca_filepath = "../ca/ca-cert.pem";
    // ctx_info.ssl_cert_filepath = "./server-cert.pem";
    // ctx_info.ssl_private_key_filepath = "./server-key.pem";
    // ctx_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    // ctx_info.options |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;

    struct lws_context *context = lws_create_context(&ctx_info);
    while (!g_terminal) {
        lws_service(context, 1000);
    }
    lws_context_destroy(context);

    return NULL;
}

int app_ipcam_WebSocket_Init()
{
    int s32Ret = 0;

    init_frame_streams();

    s32Ret = pthread_create(&g_ws_thread, NULL, ThreadWebsocket, NULL);
    return s32Ret;
}

int app_ipcam_WebSocket_DeInit()
{
    g_terminal = 1;

    if (g_ws_thread) {
        pthread_join(g_ws_thread, NULL);
        g_ws_thread = 0;
    }

    deinit_frame_streams();

    return 0;
}

int app_ipcam_WebSocket_AiFps_Send(void)
{
    if (g_wsi == NULL) {
        return 0;
    }
    char AiFps[10] = {0};
    #if defined AI_SUPPORT && defined MD_SUPPORT && defined PD_SUPPORT
    snprintf(AiFps, sizeof(AiFps), "%d %d %d", app_ipcam_Ai_PD_ProcFps_Get(), app_ipcam_Ai_MD_ProcFps_Get(), app_ipcam_Ai_PD_ProcIntrusion_Num_Get());
    #else
    snprintf(AiFps, sizeof(AiFps), "N N N");
    #endif
    int len = strlen(AiFps);

    pthread_mutex_lock(&g_aiMutexLock);
    if (g_aifps_data != NULL) {
        free(g_aifps_data);
        g_aifps_data = NULL;
    }

    g_aifps_data = malloc(LWS_PRE + 1 + len); // one for type, one for reserve
    if (g_aifps_data == NULL) {
        printf("error, malloc img buff failed\n");
        pthread_mutex_unlock(&g_aiMutexLock);
        return -1;
    }
    memset(g_aifps_data, 0, LWS_PRE + 1 + len);

    g_aifps_data[LWS_PRE] = MSG_TYPE_AIFPS & 0xff;

    memcpy(g_aifps_data + LWS_PRE + 1, AiFps, len);

    // printf("g_aifps_data ====== %s  %s \n", g_aifps_data + LWS_PRE + 1 , AiFps);

    g_aifps_size = len + 1;

    if (g_wsi != NULL) {
        lws_callback_on_writable(g_wsi);
    } else {
        free(g_aifps_data);
        g_aifps_data = NULL;
    }
    pthread_mutex_unlock(&g_aiMutexLock);

    return CVI_SUCCESS;
}