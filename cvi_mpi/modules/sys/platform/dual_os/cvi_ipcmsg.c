#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <errno.h>
#include <signal.h>
#include <inttypes.h>
#include "cvi_type.h"
#include "ipcmsg_uapi.h"
#include "list.h"
#include "cvi_ipcmsg.h"
#include "cvi_debug.h"

#define IPCMSG_DEV "/dev/ipcmsg"

#define MSG_MAX_LEN (2048)


#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/****
#define CVI_TRACE_IPCMSG(fmt, ...)  \
		printf("%s:%d:%s(): " fmt"\r", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__)
****/

#define IPCMSG_CHECK_NULL_PTR(ptr)  \
	do {  \
		if (!(ptr)) {  \
			CVI_TRACE_IPCMSG(CVI_DBG_ERR, "NULL pointer.\n");  \
			return CVI_IPCMSG_ENULL_PTR;  \
		}  \
	} while (0)

struct recv_msg_item {
	struct list_head node;
	CVI_S32 s32Id;
	CVI_IPCMSG_MESSAGE_S stMsg;
};


static CVI_S32 ipcmsg_fd = -1;
static CVI_BOOL s_bMsgStartFlg = CVI_FALSE;
static pthread_t s_msgReceiveThread;

static CVI_IPCMSG_HANDLE_FN_PTR s_pfnMessageHandle;
static pthread_mutex_t s_msg_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_msg_cond = PTHREAD_COND_INITIALIZER;

LIST_HEAD(s_RecvMsgList);



static CVI_S32 ipcmsg_dev_open(CVI_VOID)
{
	struct stat st;
	char *dev_name = IPCMSG_DEV;

	if (ipcmsg_fd != -1) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "sys dev has already opened\n");
		return CVI_SUCCESS;
	}

	ipcmsg_fd = open(dev_name, O_RDWR /* required */| O_NONBLOCK | O_CLOEXEC, 0);
	if (-1 == ipcmsg_fd) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Cannot open '%s': %d, %s\n", dev_name, errno,
			strerror(errno));
		return CVI_FAILURE;
	}

	if (-1 == fstat(ipcmsg_fd, &st)) {
		close(ipcmsg_fd);
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "Cannot identify '%s': %d, %s\n", dev_name,
			errno, strerror(errno));
		return CVI_FAILURE;
	}

	if (!S_ISCHR(st.st_mode)) {
		close(ipcmsg_fd);
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "%s is no device\n", dev_name);
		return -ENODEV;
	}

	return CVI_SUCCESS;
}

static CVI_S32 ipcmsg_dev_close(CVI_VOID)
{
	if (ipcmsg_fd == -1) {
		CVI_TRACE_IPCMSG(CVI_DBG_INFO, "sys dev is not opened\n");
		return CVI_SUCCESS;
	}

	if (-1 == close(ipcmsg_fd)) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "%s: fd(%d) failure\n", __func__, ipcmsg_fd);
		return CVI_FAILURE;
	}
	ipcmsg_fd = -1;
	return CVI_SUCCESS;
}

CVI_S32 get_ipcmsg_fd(CVI_VOID)
{
	if (ipcmsg_fd == -1)
		ipcmsg_dev_open();
	return ipcmsg_fd;
}

CVI_VOID recv_signal(CVI_S32 signum)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	struct ipcmsg_signal_cfg cfg;
	CVI_VOID *buf;
	struct recv_msg_item *item;
	CVI_VOID *pBody;
	CVI_U32 u32Len = sizeof(struct recv_msg_item) + MSG_MAX_LEN;

	if (signum != SIGIO) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "signum err\n");
		return;
	}

	buf = calloc(u32Len, 1);
	if (!buf) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "malloc failed\n");
		return;
	}
	item = (struct recv_msg_item *)buf;
	pBody = buf + sizeof(struct recv_msg_item);

	cfg.pstMsg = &item->stMsg;
	cfg.pBody = pBody;

	s32Ret = ioctl(fd, IPCMSG_IOC_SIG_DATA, &cfg);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_SIG_DATA failed\n");
		free(buf);
		return;
	}
	item->s32Id = cfg.s32Id;
	item->stMsg.pBody = pBody;

	pthread_mutex_lock(&s_msg_lock);
	list_add_tail(&item->node, &s_RecvMsgList);
	pthread_cond_signal(&s_msg_cond);
	pthread_mutex_unlock(&s_msg_lock);
	//CVI_TRACE_IPCMSG(CVI_DBG_ERR, "signal msg id=%ld\n", item->stMsg.u64Id);
}

static CVI_VOID *recv_thread(CVI_VOID *arg)
{
	//CVI_S32 s32Ret;
	struct timespec abstime;
	struct recv_msg_item *item;

	arg = arg;
	prctl(PR_SET_NAME, "ipcmsg_recv", 0, 0, 0);

	while (s_bMsgStartFlg) {
		if (list_empty(&s_RecvMsgList)) {
			if (clock_gettime(CLOCK_MONOTONIC, &abstime) != 0)
				continue;
			abstime.tv_sec++;
			pthread_mutex_lock(&s_msg_lock);
			pthread_cond_timedwait(&s_msg_cond, &s_msg_lock, &abstime);
			pthread_mutex_unlock(&s_msg_lock);
		} else {

			pthread_mutex_lock(&s_msg_lock);
			item = list_first_entry(&s_RecvMsgList,
				struct recv_msg_item, node);
			list_del(&item->node);
			pthread_mutex_unlock(&s_msg_lock);
			CVI_TRACE_IPCMSG(CVI_DBG_INFO, "Enter handle, item id=%llu\n", (unsigned long long)item->stMsg.u64Id);
			s_pfnMessageHandle(item->s32Id, &item->stMsg);
			CVI_TRACE_IPCMSG(CVI_DBG_INFO, "End handle, item id=%llu\n", (unsigned long long)item->stMsg.u64Id);
			free(item);
		}
	}
	CVI_TRACE_IPCMSG(CVI_DBG_INFO, "recv_thread end\n");

	return NULL;
}

static CVI_S32 recv_start(CVI_S32 fd)
{
	CVI_S32 flags;
	CVI_S32 s32Ret;
	pthread_attr_t attr;
	struct sched_param tsk;
	pthread_condattr_t condAttr;

	signal(SIGIO, recv_signal);
	fcntl(fd, F_SETOWN, getpid());
	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_ASYNC);

	pthread_condattr_init(&condAttr);
	pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
	if (pthread_cond_init(&s_msg_cond, &condAttr) != 0) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "get_chn_buffer cond init failed.\n");
		return CVI_FAILURE;
	}

	s_bMsgStartFlg = CVI_TRUE;
	tsk.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &tsk);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	s32Ret = pthread_create(&s_msgReceiveThread, &attr, recv_thread, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "pthread_create recv_thread fail\n");
		return CVI_FAILURE;
	}
	//pthread_attr_destroy(&attr);

	CVI_TRACE_IPCMSG(CVI_DBG_INFO, "recv_thread run\n");
	return CVI_SUCCESS;
}

static CVI_S32 recv_stop(CVI_S32 fd)
{
	CVI_S32 flags;

	signal(SIGIO, SIG_DFL);
	flags = fcntl(fd, F_GETFL);
	flags &= ~(O_ASYNC);
	fcntl(fd, F_SETFL, flags);
	fd = fd;

	s_bMsgStartFlg = CVI_FALSE;
	pthread_cond_signal(&s_msg_cond);
	pthread_join(s_msgReceiveThread, CVI_NULL);
	CVI_TRACE_IPCMSG(CVI_DBG_INFO, "recv_stop\n");

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_AddService(const CVI_CHAR *pszServiceName, const CVI_IPCMSG_CONNECT_S *pstConnectAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	struct ipcmsg_add_service_cfg cfg;

	memcpy(cfg.aszServiceName, pszServiceName, sizeof(cfg.aszServiceName));
	cfg.stConnectAttr = *pstConnectAttr;

	s32Ret = ioctl(fd, IPCMSG_IOC_ADD_SERVICE, &cfg);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_ADD_SERVICE failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_DelService(const CVI_CHAR *pszServiceName)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();

	s32Ret = ioctl(fd, IPCMSG_IOC_DEL_SERVICE, pszServiceName);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_DEL_SERVICE failed\n");
		return s32Ret;
	}
	ipcmsg_dev_close();

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_TryConnect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
		CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	struct ipcmsg_connect_cfg cfg;

	memcpy(cfg.aszServiceName, pszServiceName, sizeof(cfg.aszServiceName));
	cfg.isTry = 1;

	s32Ret = ioctl(fd, IPCMSG_IOC_CONNECT, &cfg);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_CONNECT failed\n");
		return s32Ret;
	}
	*ps32Id = cfg.s32Id;
	s_pfnMessageHandle = pfnMessageHandle;

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_Connect(CVI_S32 *ps32Id, const CVI_CHAR *pszServiceName,
		CVI_IPCMSG_HANDLE_FN_PTR pfnMessageHandle)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	struct ipcmsg_connect_cfg cfg;

	memcpy(cfg.aszServiceName, pszServiceName, sizeof(cfg.aszServiceName));
	cfg.isTry = 0;

	s32Ret = ioctl(fd, IPCMSG_IOC_CONNECT, &cfg);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_CONNECT failed\n");
		return s32Ret;
	}
	*ps32Id = cfg.s32Id;
	s_pfnMessageHandle = pfnMessageHandle;

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_Disconnect(CVI_S32 s32Id)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();

	s32Ret = ioctl(fd, IPCMSG_IOC_DISCONNECT, &s32Id);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_DISCONNECT failed\n");
		return s32Ret;
	}

	if (s_bMsgStartFlg)
		recv_stop(fd);

	return CVI_SUCCESS;
}

CVI_BOOL CVI_IPCMSG_IsConnected(CVI_S32 s32Id)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();

	s32Ret = ioctl(fd, IPCMSG_IOC_IS_CONNECT, &s32Id);

	return s32Ret ? CVI_TRUE : CVI_FALSE;
}

CVI_IPCMSG_MESSAGE_S *CVI_IPCMSG_CreateMessage(CVI_U32 u32Module, CVI_U32 u32CMD,
		CVI_VOID *pBody, CVI_U32 u32BodyLen)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	CVI_IPCMSG_MESSAGE_S *pstMsg = NULL;
	CVI_U32 u32Len = u32BodyLen + sizeof(CVI_IPCMSG_MESSAGE_S);
	CVI_S32 s32MsgId;

	pstMsg = (CVI_IPCMSG_MESSAGE_S *)calloc(u32Len, 1);
	if (!pstMsg) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return NULL;
	}

	s32Ret = ioctl(fd, IPCMSG_IOC_GET_ID, &s32MsgId);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_GET_ID failed\n");
		free(pstMsg);
		return NULL;
	}

	pstMsg->bIsResp = CVI_FALSE;
	pstMsg->u32Module = u32Module;
	pstMsg->u64Id = s32MsgId;
	pstMsg->u32CMD = u32CMD;
	pstMsg->s32RetVal = 0;
	pstMsg->u32BodyLen = u32BodyLen;
	if ((u32BodyLen > 0) && pBody) {
		pstMsg->pBody = (CVI_VOID *)pstMsg + sizeof(CVI_IPCMSG_MESSAGE_S);
		memcpy(pstMsg->pBody, pBody, u32BodyLen);
	}
	return pstMsg;
}

CVI_IPCMSG_MESSAGE_S *CVI_IPCMSG_CreateRespMessage(CVI_IPCMSG_MESSAGE_S *pstRequest,
		CVI_S32 s32RetVal, CVI_VOID *pBody, CVI_U32 u32BodyLen)
{
	CVI_IPCMSG_MESSAGE_S *pRespMsg = NULL;
	CVI_U32 u32Len = u32BodyLen + sizeof(CVI_IPCMSG_MESSAGE_S);

	if(!pstRequest) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "NULL pointer.\n");
		return NULL;
	}

	pRespMsg = (CVI_IPCMSG_MESSAGE_S *)calloc(u32Len, 1);
	if (!pRespMsg) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "calloc failed, size(%d)\n", u32Len);
		return NULL;
	}

	pRespMsg->bIsResp = CVI_TRUE;
	pRespMsg->u64Id = pstRequest->u64Id;
	pRespMsg->u32Module = pstRequest->u32Module;
	pRespMsg->u32CMD = pstRequest->u32CMD;
	pRespMsg->s32RetVal = s32RetVal;
	pRespMsg->u32BodyLen = u32BodyLen;
	if ((u32BodyLen > 0) && pBody) {
		pRespMsg->pBody = (CVI_VOID *)pRespMsg + sizeof(CVI_IPCMSG_MESSAGE_S);
		memcpy(pRespMsg->pBody, pBody, u32BodyLen);
	}
	return pRespMsg;
}

CVI_VOID CVI_IPCMSG_DestroyMessage(CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	if (pstMsg)
		free(pstMsg);
}

CVI_S32 CVI_IPCMSG_SendOnly(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstRequest)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	struct ipcmsg_send_only_cfg cfg;

	cfg.s32Id = s32Id;
	cfg.pstRequest = pstRequest;

	s32Ret = ioctl(fd, IPCMSG_IOC_SEND_ONLY, &cfg);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_SEND_ONLY failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_IPCMSG_SendAsync(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg,
		CVI_IPCMSG_RESPHANDLE_FN_PTR pfnRespHandle)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	struct ipcmsg_send_cfg cfg;

	pfnRespHandle = pfnRespHandle;

	cfg.s32Id = s32Id;
	cfg.pstRequest = pstMsg;

	s32Ret = ioctl(fd, IPCMSG_IOC_SEND_ASYNC, &cfg);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_SEND_ASYNC failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;

}

//extern long get_diff_in_us(struct timespec t1, struct timespec t2);
CVI_S32 CVI_IPCMSG_SendSync(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg,
		CVI_IPCMSG_MESSAGE_S **ppstMsg, CVI_S32 s32TimeoutMs)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ipcmsg_fd();
	struct ipcmsg_send_cfg cfg;
	CVI_U32 u32Len = sizeof(CVI_IPCMSG_MESSAGE_S) + MSG_MAX_LEN;
	CVI_VOID *buf;
	CVI_IPCMSG_MESSAGE_S *pstRespMsg;
	//struct timespec t1, t2;
	//CVI_U64 time;
	CVI_VOID *pRespBody;

	//clock_gettime(CLOCK_MONOTONIC, &t1);

	buf = malloc(u32Len);
	if (!buf) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "malloc failed\n");
		return CVI_FAILURE;
	}
	pstRespMsg = (CVI_IPCMSG_MESSAGE_S *)buf;
	pRespBody = buf + sizeof(CVI_IPCMSG_MESSAGE_S);

	cfg.s32Id = s32Id;
	cfg.pstRequest = pstMsg;
	cfg.pstResq = pstRespMsg;
	cfg.pRespBody = pRespBody;
	cfg.s32TimeoutMs = s32TimeoutMs;

	s32Ret = ioctl(fd, IPCMSG_IOC_SEND_SYNC, &cfg);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_SEND_SYNC failed\n");
		free(buf);
		return s32Ret;
	}
	pstRespMsg->pBody = pRespBody;
	*ppstMsg = pstRespMsg;
	//clock_gettime(CLOCK_MONOTONIC, &t2);
	//time = get_diff_in_us(t1, t2);
	//CVI_TRACE_IPCMSG(CVI_DBG_ERR, "time=%ld\n", time);

	return CVI_SUCCESS;
}

CVI_VOID CVI_IPCMSG_Run(CVI_S32 s32Id)
{
	CVI_S32 s32Ret, s32UserCnt;
	CVI_S32 fd = get_ipcmsg_fd();

	s32Ret = ioctl(fd, IPCMSG_IOC_RUN, &s32Id);
	if (s32Ret) {
		CVI_TRACE_IPCMSG(CVI_DBG_ERR, "ioctl IPCMSG_IOC_RUN failed\n");
		return;
	}

	s32UserCnt = ioctl(fd, IPCMSG_IOC_GET_USER_CNT, &s32Id);
	if (s32UserCnt == 1)
		recv_start(fd);
}

