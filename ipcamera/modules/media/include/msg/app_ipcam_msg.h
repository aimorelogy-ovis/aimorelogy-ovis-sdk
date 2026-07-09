#ifndef __APP_IPCAM_MSG_H__
#define __APP_IPCAM_MSG_H__
#include "cvi_type.h"

typedef enum _ANONMSG_TYPE_E{
    ANONMSG_PQPARM = 0,
    ANONMSG_MAX,
}ANONMSG_TYPE_E;

typedef int (*ANONMSG_FN)(ANONMSG_TYPE_E msgid, int buf_type,void * buf, unsigned int len);


int app_ipcam_Msg_Init();
int app_ipcam_Msg_Deinit();

#ifdef ANONMSG_ENABLE
int app_ipcam_MsgAnonSend(CVI_U8 port_id, CVI_U8 msg_id, void *buf, unsigned int len);
int app_ipcam_MsgAnonSendParam(CVI_U8 msg_id, int param);
int app_ipcam_MsgAnonInit();
int app_ipcam_MsgAnonDeInit();
int app_ipcam_MsgAnonCbRegister(ANONMSG_FN pfunction);
int app_ipcam_MsgAnonCbUnRegister();
#endif

#endif