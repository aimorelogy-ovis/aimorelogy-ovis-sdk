#include <stdio.h>
#include <unistd.h>
#include "app_ipcam_msg.h"
#include "cvi_comm_vb.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_ipcm.h"
#include "cvi_msg_client.h"


#ifdef ANONMSG_ENABLE
ANONMSG_FN g_annonmsg_cb = NULL;
#endif


int app_ipcam_Msg_Init()
{
    return CVI_MSG_Init();
}

int app_ipcam_Msg_Deinit()
{
    return CVI_MSG_Deinit();
}

#ifdef ANONMSG_ENABLE
static int _anon_msg_process(void *priv, IPCM_ANON_MSG_S *data)
{
    unsigned char msg_id, data_type;
    unsigned int data_len;
    int ret = 0;
    if (data == NULL) {
        printf( " _anon_msg_process error handle data null \r\n");
        return -1;
    }
    if (data->u8PortID != CVI_IPCM_PORT_ANON_MSG) {
        printf( " _anon_msg_process error port_id error \r\n");
        return -1;
    }
    msg_id = data->u8MsgID;
    data_type = data->u8DataType;
    data_len = 0;

    //printf( "anon recv port_id(%u) msg_id(%u) data_type(%u) data(%lx) len(%u)\r\n",
    //    port_id, msg_id, data_type, (unsigned long)data->data, data_len);

    if (g_annonmsg_cb) {
        if (data_type == IPCM_MSG_TYPE_RAW_PARAM) {
            g_annonmsg_cb(msg_id, data_type, (void *)&data->u32Param, data_len);
        } else {
            g_annonmsg_cb(msg_id, data_type, data->stData.pData, data_len);
        }
    }
    return ret;
}

int app_ipcam_MsgAnonCbRegister(ANONMSG_FN pfunction)
{
    if (g_annonmsg_cb == NULL) {
        g_annonmsg_cb = pfunction;
    } else {
        return -1;
    }
    return 0;
}

int app_ipcam_MsgAnonCbUnRegister()
{
    if (g_annonmsg_cb != NULL) {
        g_annonmsg_cb = NULL;
    } else {
        return -1;
    }
    return 0;
}

int app_ipcam_MsgAnonSendParam(CVI_U8 msg_id, int param)
{
    int ret = CVI_IPCM_AnonSendParam(CVI_IPCM_PORT_ANON_MSG, msg_id, param);
    if (ret != 0) {
        printf("app_ipcam_MsgAnonSendParam err \r\n");
    }
    usleep(20*1000);
    return ret;
}

int app_ipcam_MsgAnonSend(CVI_U8 port_id, CVI_U8 msg_id, void *buf, unsigned int len)
{
    int ret = 0;
    void *data = NULL;

    data = CVI_IPCM_GetBuff(len);
    if (data) {
        memcpy(data, buf, len);
        ret = CVI_IPCM_AnonSendMsg(port_id, msg_id, data, len);
        if (ret) {
            printf("ipcm_anon_send_msg fail ret:%d\n", ret);
        }
    } else {
        printf("ipcm_get_buff get len %d error \r\n", len);
        return CVI_FAILURE;
    }
    // waiting for msg recvied and buffer release by alios
    usleep(20 * 1000);//usleep 20MS
    return ret;
}

int app_ipcam_MsgAnonInit()
{
    //anonymous init
    int ret = 0;
    ret = CVI_IPCM_AnonInit();
    if (ret) {
        printf( "ipcm_anon_init fail:%x\r\n", ret);
        return ret;
    }
    ret = CVI_IPCM_RegisterAnonHandle(CVI_IPCM_PORT_ANON_MSG, _anon_msg_process, NULL);
    if (ret) {
        printf( "ipcm_anon_register_handle fail:%x\r\n", ret);
        CVI_IPCM_AnonUninit();
        return ret;
    }
    return ret;
}

int app_ipcam_MsgAnonDeInit()
{
    int ret = 0;

    ret = CVI_IPCM_AnonUninit();
    if (ret) {
        printf("ipcm_anon_uninit fail:%x\r\n", ret);
    }
    return ret;
}

#endif