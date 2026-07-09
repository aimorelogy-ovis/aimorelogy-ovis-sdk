#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_type.h"
#include "cvi_ipcmsg.h"
#include "msg.h"
#include "cvi_msg_client.h"
#include "cvi_common.h"
#include <inttypes.h>
#include "cvi_debug.h"
#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/*
#define CVI_TRACE_MSG(CVI_DBG_ERR, fmt, ...)  \
		printf("%s:%d:%s(): " fmt, __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
*/

static MsgSnsCallback g_SnsCallback;
static CVI_S32 g_mMediaMsgId = -1;
static CVI_BOOL g_bMsgHeartbeatStartFlg = CVI_FALSE;
static pthread_t g_msgHeartbeatThread;

static CVI_S32 send_heartbeat(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;
	CVI_U32 u32ModFd = MODFD(CVI_ID_IPCMSG, 0, 0);

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Message not initialized.\n");
		return CVI_FAILURE;
	}

	pReq = CVI_IPCMSG_CreateMessage(u32ModFd, 0, NULL, 0);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return CVI_FAILURE;
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	if (pResp->s32RetVal != MSG_ACK) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ACK error, recv ack:%x\n", pResp->s32RetVal);
		s32Ret = CVI_FAILURE;
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

static void *Heartbeat_Thread(void *arg)
{
	arg = arg;
	prctl(PR_SET_NAME, "heartbeat", 0, 0, 0);

	CVI_TRACE_MSG(CVI_DBG_WARN, "Heartbeat thread running.\n");

	while (g_bMsgHeartbeatStartFlg) {
		send_heartbeat();
		sleep(1);
	}

	CVI_TRACE_MSG(CVI_DBG_WARN, "Heartbeat thread end.\n");

	return NULL;
}

CVI_VOID MSG_Run_Heartbeat_Thread(CVI_VOID)
{
	CVI_S32 s32Ret;

	g_bMsgHeartbeatStartFlg = CVI_TRUE;

	s32Ret = pthread_create(&g_msgHeartbeatThread, CVI_NULL, Heartbeat_Thread, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "pthread_create Heartbeat_Thread fail\n");
		return;
	}
}

CVI_VOID MSG_Exit_Heartbeat_Thread(CVI_VOID)
{
	g_bMsgHeartbeatStartFlg = CVI_FALSE;
	if (g_msgHeartbeatThread) {
		pthread_join(g_msgHeartbeatThread, CVI_NULL);
	}
}

CVI_VOID CVI_MSG_RegisterSNSCallback(MsgSnsCallback callback)
{
	g_SnsCallback = callback;
}

CVI_S32 handler_Sns_Callback(CVI_S32 siId, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	CVI_S32 mode = pstMsg->as32PrivData[0];
	CVI_S32 snsid = pstMsg->as32PrivData[1];

	if (g_SnsCallback) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Sensor %d callback ahd_mode: %d\n", snsid, mode);
		g_SnsCallback(snsid, mode);
	}

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "g_SnsCallback Failed : %#x!\n", s32Ret);
	}

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(siId, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(respMsg);
		return s32Ret;
	}

	CVI_IPCMSG_DestroyMessage(respMsg);
	return CVI_SUCCESS;
}

static void MEDIA_MSG_HandleMessage(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_U32 u32ModID;
	CVI_IPCMSG_MESSAGE_S *respMsg;

	u32ModID = GET_MOD_ID(pstMsg->u32Module);
	switch (u32ModID) {
		case CVI_ID_SENSOR:
			if (handler_Sns_Callback(s32Id, pstMsg) != CVI_SUCCESS) {
				CVI_TRACE_MSG(CVI_DBG_ERR, "handler_Sns_Callback fail\n");
			}
			break;
		default:

			respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, 0, NULL, 0);
			if (respMsg == CVI_NULL) {
				CVI_TRACE_MSG(CVI_DBG_ERR, "call CVI_IPCMSG_CreateRespMessage fail\n");
				break;
			}
			CVI_IPCMSG_SendOnly(s32Id, respMsg);
			CVI_IPCMSG_DestroyMessage(respMsg);
			break;
	}
	CVI_TRACE_MSG(CVI_DBG_INFO, "s32Id=%d msg_id=%llu, u32Module=%d cmd=%d len=%d\n",
		s32Id, (long long unsigned int)pstMsg->u64Id,
		pstMsg->u32Module, pstMsg->u32CMD, pstMsg->u32BodyLen);
}

CVI_S32 CVI_MSG_Init(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_CONNECT_S stConnectAttr = { 1, CVI_IPCMSG_MEDIA_PORT, 1 };

	if (g_mMediaMsgId >= 0)
		return CVI_SUCCESS;

	s32Ret = CVI_IPCMSG_AddService("CVI_MMF_MSG", &stConnectAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "client ipcmsg add service fail\n");
		return s32Ret;
	}

	/* connect with block to prevent tryconnect fail */
	s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Connect fail\n");
		return s32Ret;
	}

	CVI_IPCMSG_Run(g_mMediaMsgId);

	if (send_heartbeat()) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Communication failure.\n");
	} else {
		CVI_TRACE_MSG(CVI_DBG_WARN, "IPCMSG ready...\n");
	}

	MSG_Run_Heartbeat_Thread();

	return 0;
}

// TODO:
CVI_S32 CVI_MSG_IsInited(CVI_VOID)
{
	CVI_S32 s32Ret;

	s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
	if (s32Ret != CVI_SUCCESS) {
		return 0;
	}

	return 1;
}

CVI_S32 CVI_MSG_Deinit(CVI_VOID)
{
	CVI_S32 s32Ret;

	if (g_mMediaMsgId < 0)
		return CVI_SUCCESS;

	MSG_Exit_Heartbeat_Thread();

	s32Ret = CVI_IPCMSG_Disconnect(g_mMediaMsgId);
	CVI_TRACE_MSG(CVI_DBG_INFO, "CVI_IPCMSG_Disconnect\n");

	s32Ret |= CVI_IPCMSG_DelService("CVI_MMF_MSG");

	g_mMediaMsgId = -1;

	CVI_TRACE_MSG(CVI_DBG_INFO, "Media_MSG_DeInit\n");
	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					 MSG_PRIV_DATA_S *pstPrivData)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "There is no initialization message for this process, try connet...\n");
		/* Try connet, If failure is returned, the system has no initialization message */
		s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "The system has no initialization message\n");
			return s32Ret;
		}
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "Connected. (slave process)\n");
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS && (pResp->u32BodyLen > 0)) {
		memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync2(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					CVI_VOID *pRespBody, MSG_PRIV_DATA_S *pstPrivData)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "There is no initialization message for this process, try connet...\n");
		/* Try connet, If failure is returned, the system has no initialization message */
		s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "The system has no initialization message\n");
			return s32Ret;
		}
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "Connected. (slave process)\n");
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}

	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS && (pResp->u32BodyLen > 0)) {
		memcpy(pRespBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync3(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					CVI_U32 *pu32Data)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "There is no initialization message for this process, try connet...\n");
		/* Try connet, If failure is returned, the system has no initialization message */
		s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "The system has no initialization message\n");
			return s32Ret;
		}
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "Connected. (slave process)\n");
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}

	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if ((s32Ret == CVI_SUCCESS) && (pu32Data != NULL)) {
		*pu32Data = pResp->as32PrivData[0];
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync4(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
			MSG_PRIV_DATA_S *pstPrivData, CVI_S32 s32TimeoutMs)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "There is no initialization message for this process, try connet...\n");
		/* Try connet, If failure is returned, the system has no initialization message */
		s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "The system has no initialization message\n");
			return s32Ret;
		}
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "Connected. (slave process)\n");
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, s32TimeoutMs);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS && (pResp->u32BodyLen > 0)) {
		memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}

CVI_S32 CVI_MSG_SendSync5(CVI_U32 u32Module, CVI_U32 u32CMD, CVI_VOID *pBody, CVI_U32 u32BodyLen,
					 MSG_PRIV_DATA_S *pstPrivData)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *pReq = NULL;
	CVI_IPCMSG_MESSAGE_S *pResp = NULL;

	if (g_mMediaMsgId < 0) {
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "There is no initialization message for this process, try connet...\n");
		/* Try connet, If failure is returned, the system has no initialization message */
		s32Ret = CVI_IPCMSG_Connect(&g_mMediaMsgId, "CVI_MMF_MSG", MEDIA_MSG_HandleMessage);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_MSG(CVI_DBG_ERR, "The system has no initialization message\n");
			return s32Ret;
		}
		CVI_TRACE_MSG(CVI_DBG_NOTICE, "Connected. (slave process)\n");
	}

	pReq = CVI_IPCMSG_CreateMessage(u32Module, u32CMD, pBody, u32BodyLen);
	if (pReq == NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_CreateMessage return NULL.\n");
		return -1;
	}
	if (pstPrivData != NULL) {
		memcpy(pReq->as32PrivData, pstPrivData->as32PrivData, sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
	}
	s32Ret = CVI_IPCMSG_SendSync(g_mMediaMsgId, pReq, &pResp, CVI_IPCMSG_SEND_SYNC_TIMEOUT);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "CVI_IPCMSG_SendSync fail s32Ret:%x\n", s32Ret);
		CVI_IPCMSG_DestroyMessage(pReq);
		CVI_IPCMSG_DestroyMessage(pResp);
		return s32Ret;
	}
	s32Ret = pResp->s32RetVal;
	if (s32Ret == CVI_SUCCESS) {
		if (pResp->u32BodyLen > 0)
			memcpy(pBody, pResp->pBody, pResp->u32BodyLen);

		if (pstPrivData != NULL) {
			memcpy(pstPrivData->as32PrivData, pResp->as32PrivData,
				sizeof(CVI_S32) * CVI_IPCMSG_PRIVDATA_NUM);
		}
	}
	CVI_IPCMSG_DestroyMessage(pReq);
	CVI_IPCMSG_DestroyMessage(pResp);

	return s32Ret;
}
