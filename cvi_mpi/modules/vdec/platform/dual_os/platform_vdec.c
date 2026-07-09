
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <inttypes.h>
#include "cvi_vdec.h"
#include "cvi_type.h"
#include "cvi_debug.h"
#include "msg_vdec.h"
#include "cvi_msg_client.h"
#include "cvi_sys.h"
#include "cvi_datafifo.h"

#define UNUSED_VARIABLE(x) ((void)(x))

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

//#define FLOW_DEBUG 1

#ifdef FLOW_DEBUG

#define CVI_VDEC_PRINT(msg, ...)		\
do { \
	printf("%s %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
} while (0)

#define CVI_VDEC_API_IN  CVI_VDEC_PRINT("Chn:%d In\n", VdChn);
#define CVI_VDEC_API_OUT CVI_VDEC_PRINT("Chn:%d Out\n", VdChn);


#define DATA_DUMP(data, len)  data_dump((unsigned char *)data, len, __FUNCTION__, __LINE__)

__attribute__((unused)) static void data_dump(unsigned char *pu8, int len, const char *func, int line)
{
	int i = 0;
	int sum = 0;

	printf("=====%s %d====,len:%d\n", func, line, len);
	for (i = 0; i < len; i++) {
		// printf("%d:0x%x\n", i, pu8[i]);
		sum += pu8[i];
	}
	printf("sum:%d\n", sum);

}
#else
#define CVI_VDEC_API_IN
#define CVI_VDEC_API_OUT
#define DATA_DUMP(data, len)
#endif

#ifndef CVI_VDEC_ERR
#define  CVI_VDEC_ERR printf
#endif

#ifndef MOD_CHECK_NULL_PTR
#define MOD_CHECK_NULL_PTR(id, ptr) \
do { \
	if (!(ptr)) { \
		CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
		return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
	} \
} while (0)
#endif

typedef struct _stream_Ioninfo_ {
	uint64_t phyaddr;
	void *viraddr;
	CVI_U32 size;
} stream_Ioninfo;

stream_Ioninfo vdecIoninfo[VDEC_MAX_CHN_NUM];

//vdec_dbg vdecDbg;
static CVI_DATAFIFO_HANDLE hVdecDataFifoHandle[VDEC_MAX_CHN_NUM] = {0};

CVI_S32 CVI_VDEC_SetFirmware(PAYLOAD_TYPE_E enType, char *filename)
{
	UNUSED(enType);
	UNUSED(filename);
	return CVI_SUCCESS;
}

static CVI_S32 CVI_VDEC_IonAlloc(VDEC_CHN VdChn, uint64_t *paddr, CVI_S32 size)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);
	MSG_PRIV_DATA_S stPrivDate;

	CVI_VDEC_API_IN;

	stPrivDate.as32PrivData[0] = size;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_ION_ALLOC, (CVI_VOID *)paddr,
				sizeof(uint64_t), &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("GetIon fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

static CVI_S32 CVI_VDEC_IonFree(VDEC_CHN VdChn ,uint64_t paddr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_ION_FREE, (CVI_VOID *)&paddr,
		sizeof(uint64_t), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("GetIon fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_create_chn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstAttr);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_CREATE_CHN, (CVI_VOID *)pstAttr,
				sizeof(VDEC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("CreateChn fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	vdecIoninfo[VdChn].size = pstAttr->u32StreamBufSize;

	CVI_VDEC_IonAlloc(VdChn, &vdecIoninfo[VdChn].phyaddr, vdecIoninfo[VdChn].size);

	vdecIoninfo[VdChn].viraddr=
		CVI_SYS_Mmap(vdecIoninfo[VdChn].phyaddr,vdecIoninfo[VdChn].size);

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_destroy_chn(VDEC_CHN VdChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_DESTROY_CHN, NULL,
				0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("DestroyChn fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_SYS_Munmap(vdecIoninfo[VdChn].viraddr, vdecIoninfo[VdChn].size);
	CVI_VDEC_IonFree(VdChn ,vdecIoninfo[VdChn].phyaddr);

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_get_chn_attr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstAttr);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_GET_CHN_ATTR, (CVI_VOID *)pstAttr,
				sizeof(VDEC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("GetChnAttr fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_set_chn_attr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstAttr);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_SET_CHN_ATTR, (CVI_VOID *)pstAttr,
				sizeof(VDEC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("SetChnAttr fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_start_recv_stream(VDEC_CHN VdChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);
	MSG_PRIV_DATA_S stPrivDate = {0};
	CVI_U64 u64PhyAddr = 0;
	CVI_DATAFIFO_PARAMS_S stDataFifoParams = {
		.u32EntriesNum = 11,
		.u32CacheLineSize = sizeof(VIDEO_FRAME_INFO_S),
		.bDataReleaseByWriter = CVI_FALSE,
		.enOpenMode = DATAFIFO_READER,
	};

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync3(u32ModFd, MSG_CMD_VDEC_START_RECV_FRAME, NULL,
				0, (CVI_U32 *)&stPrivDate.as32PrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("StartRecvStream fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	memcpy(&u64PhyAddr, stPrivDate.as32PrivData, sizeof(u64PhyAddr));

	s32Ret = CVI_DATAFIFO_OpenByAddr(&hVdecDataFifoHandle[VdChn], &stDataFifoParams, u64PhyAddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("DATAFIFO_OpenByAddr, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_stop_recv_stream(VDEC_CHN VdChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_STOP_RECV_FRAME, NULL,
				0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("StopRecvStream fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	if (hVdecDataFifoHandle[VdChn] != 0) {
		s32Ret = CVI_DATAFIFO_Close(hVdecDataFifoHandle[VdChn]);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VDEC_ERR("CVI_DATAFIFO_Close fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
			return s32Ret;
		}
		hVdecDataFifoHandle[VdChn] = 0;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_query_status(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstStatus);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_QUERY_STATUS, (CVI_VOID *)pstStatus,
				sizeof(VDEC_CHN_STATUS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("QueryStatus fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_get_fd(VDEC_CHN VdChn)
{
	UNUSED_VARIABLE(VdChn);
	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_close_fd(VDEC_CHN VdChn)
{
	UNUSED_VARIABLE(VdChn);
	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_reset_chn(VDEC_CHN VdChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_RESET_CHN, NULL,
				0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("ResetChn fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_set_chn_param(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstParam);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_SET_CHN_PARAM, (CVI_VOID *)pstParam,
				sizeof(VDEC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("SetChnParam fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_get_chn_param(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstParam);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_GET_CHN_PARAM, (CVI_VOID *)pstParam,
				sizeof(VDEC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("GetChnParam fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

/* s32MilliSec: -1 is block,0 is no block,other positive number is timeout */
CVI_S32 platform_vdec_send_stream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VDEC, 0, VdChn, 1);
	MSG_PRIV_DATA_S stPrivDate;
	int len;

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstStream);

	CVI_VDEC_API_IN;

	len = pstStream->u32Len > vdecIoninfo[VdChn].size?vdecIoninfo[VdChn].size:pstStream->u32Len;

	memset(vdecIoninfo[VdChn].viraddr, 0, len);
	memcpy(vdecIoninfo[VdChn].viraddr, pstStream->pu8Addr, len);
	memcpy(&stPrivDate.as32PrivData[1], &vdecIoninfo[VdChn].phyaddr, sizeof(uint64_t));

	CVI_SYS_IonFlushCache(vdecIoninfo[VdChn].phyaddr, vdecIoninfo[VdChn].viraddr, len);

	stPrivDate.as32PrivData[0] = s32MilliSec;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_SEND_STREAM, (CVI_VOID *)pstStream,
				sizeof(VDEC_STREAM_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
//		CVI_VDEC_ERR("SendStream fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_get_frame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ReadLen;
	VIDEO_FRAME_S *pstVFrame;
	VIDEO_FRAME_INFO_S *pstFrameFifo;
	int i;
	CVI_U32 retrytimes;

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstFrameInfo);

	CVI_VDEC_API_IN;
#if 0
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);
	MSG_PRIV_DATA_S stPrivDate;

	stPrivDate.as32PrivData[0] = s32MilliSec;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_GET_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
//		CVI_VDEC_ERR("GetFrame fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		CVI_VDEC_API_OUT;
		return s32Ret;
	}
#endif

	if(s32MilliSec < 0) {
		retrytimes = 2000;
	} else {
		retrytimes = s32MilliSec/5;
	}

GET_FRAME_RETRY:
	s32Ret = CVI_DATAFIFO_CMD(hVdecDataFifoHandle[VdChn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &u32ReadLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("DATAFIFO_CMD GET_AVAIL_READ_LEN fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	if (u32ReadLen > 0) {
		s32Ret = CVI_DATAFIFO_Read(hVdecDataFifoHandle[VdChn], (CVI_VOID **)&pstFrameFifo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VDEC_ERR("CVI_DATAFIFO_Read fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
			return s32Ret;
		}
		DATA_DUMP(pstFrameFifo, sizeof(VIDEO_FRAME_INFO_S));
		memcpy(pstFrameInfo, pstFrameFifo, sizeof(VIDEO_FRAME_INFO_S));

		pstVFrame = &pstFrameInfo->stVFrame;
		for(i = 0; i < 3; i++){
			if(pstVFrame->u64PhyAddr[i] && pstVFrame->u32Length[i]){
				pstVFrame->pu8VirAddr[i] = CVI_SYS_MmapCache(pstVFrame->u64PhyAddr[i], pstVFrame->u32Length[i]);
				if(pstVFrame->pu8VirAddr[i] == NULL){
					CVI_VDEC_ERR("CVI_SYS_Mmap fail, chn:%d, PhyAddr:0x%"PRIu64" len:%d \n",
								VdChn, pstVFrame->u64PhyAddr[i] ,pstVFrame->u32Length[i]);
					return CVI_FAILURE;
				}
			} else {
				pstVFrame->pu8VirAddr[i] = NULL;
			}
		}

		CVI_VDEC_API_OUT;
		return CVI_SUCCESS;
	}

	if(retrytimes--) {
		usleep(5 * 1000);
		goto GET_FRAME_RETRY;
	}

	CVI_VDEC_API_OUT;

	return CVI_FAILURE;
}

CVI_S32 platform_vdec_release_frame(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);
	const VIDEO_FRAME_S *pstVFrame;
	int i;
	UNUSED(pstVFrame);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstFrameInfo);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_RELEASE_FRAME, (CVI_VOID *)pstFrameInfo,
				sizeof(VIDEO_FRAME_INFO_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("ReleaseFrame fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_DATAFIFO_CMD(hVdecDataFifoHandle[VdChn], DATAFIFO_CMD_READ_DONE, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("DATAFIFO_CMD READ_DONE fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	pstVFrame = &pstFrameInfo->stVFrame;
	for(i = 0; i < 3; i++){
		if(pstVFrame->pu8VirAddr[i])
			CVI_SYS_Munmap(pstVFrame->pu8VirAddr[i], pstVFrame->u32Length[i]);
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;

}

CVI_S32 platform_vdec_attach_vb_pool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstPool);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_ATTACH_VBPOOL, (CVI_VOID *)pstPool,
				sizeof(VDEC_CHN_POOL_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("AttachVbPool fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_detach_vb_pool(VDEC_CHN VdChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_DETACH_VBPOOL, NULL,
				0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("DetachVbPool fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_set_mod_param(const VDEC_MOD_PARAM_S *pstModParam)
{
	CVI_S32 s32Ret;
	VDEC_CHN VdChn = 0;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstModParam);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_SET_MOD_PARAM, (CVI_VOID *)pstModParam,
				sizeof(VDEC_MOD_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("SetModParam fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_get_mod_param(VDEC_MOD_PARAM_S *pstModParam)
{
	CVI_S32 s32Ret;
	VDEC_CHN VdChn = 0;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VDEC, 0, VdChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VDEC, pstModParam);

	CVI_VDEC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VDEC_GET_MOD_PARAM, (CVI_VOID *)pstModParam,
				sizeof(VDEC_MOD_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VDEC_ERR("GetModParam fail, chn:%d, ret:0x%x\n", VdChn, s32Ret);
		return s32Ret;
	}

	CVI_VDEC_API_OUT;

	return CVI_SUCCESS;
}

