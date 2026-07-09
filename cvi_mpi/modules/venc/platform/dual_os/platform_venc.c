
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <inttypes.h>
#include "cvi_venc.h"
#include "cvi_type.h"
#include "cvi_debug.h"
#include "msg_venc.h"
#include "msg_vdec.h"
#include "cvi_msg_client.h"
#include "cvi_sys.h"
#include "cvi_datafifo.h"
#include "platform_venc.h"
#include "devmem.h"

typedef struct _VENC_STREAM_PACK_S {
	VENC_PACK_S pstPack[8];
	VENC_STREAM_S stStream;
	VENC_CHN VeChn;
} VENC_STREAM_PACK_S;

#define UNUSED_VARIABLE(x) ((void)(x))

static CVI_DATAFIFO_HANDLE hVencDataFifoHandle[VENC_MAX_CHN_NUM] = {0};

//#define FLOW_DEBUG 1

#ifdef FLOW_DEBUG

#define CVI_VENC_PRINT(msg, ...)		\
do { \
	printf("%s %d, "msg, __func__, __LINE__, ## __VA_ARGS__); \
} while (0)

#define CVI_VENC_API_IN  CVI_VENC_PRINT("Chn:%d In\n", VeChn);
#define CVI_VENC_API_OUT CVI_VENC_PRINT("Chn:%d Out\n", VeChn);


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

#define CVI_VENC_API_IN
#define CVI_VENC_API_OUT
#define DATA_DUMP(data, len)
#endif
#ifndef CVI_VENC_ERR
#define  CVI_VENC_ERR printf
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

static CVI_S32 s32DevmemFd = -1;
static CVI_U32 u32ChannelCreatedCnt;
static pthread_mutex_t devmem_mutex = PTHREAD_MUTEX_INITIALIZER;

static CVI_S32 open_devmem(void)
{
	if (s32DevmemFd == -1) {
		s32DevmemFd = devm_open_cached();
		if (s32DevmemFd < 0) {
			return CVI_FAILURE;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModEnc = MODFD(CVI_ID_VENC, 0, 0);
	CVI_U32 u32ModDec = MODFD(CVI_ID_VDEC, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModDec, MSG_CMD_VDEC_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VDEC_Suspend fail\n");
		return s32Ret;
	}

	s32Ret = CVI_MSG_SendSync(u32ModEnc, MSG_CMD_VENC_SUSPEND, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_Suspend fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModEnc = MODFD(CVI_ID_VENC, 0, 0);
	CVI_U32 u32ModDec = MODFD(CVI_ID_VDEC, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModDec, MSG_CMD_VDEC_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VDEC_Resume fail\n");
		return s32Ret;
	}

	s32Ret = CVI_MSG_SendSync(u32ModEnc, MSG_CMD_VENC_RESUME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_Resume fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_VOID cviGetMask(void)
{
	return;
}

CVI_S32 CVI_VENC_SetFirmware(PAYLOAD_TYPE_E enType, char *filename)
{
	UNUSED_VARIABLE(enType);
	UNUSED_VARIABLE(filename);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_create_chn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstAttr);

	CVI_VENC_API_IN;
	DATA_DUMP(pstAttr, sizeof(VENC_CHN_ATTR_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_CREATE_CHN, (CVI_VOID *)pstAttr,
				sizeof(VENC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CreateChn fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	{
		pthread_mutex_lock(&devmem_mutex);
		open_devmem();
		u32ChannelCreatedCnt++;
		pthread_mutex_unlock(&devmem_mutex);
	}
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_destroy_chn(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_DESTROY_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DestroyChn fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	{
		pthread_mutex_lock(&devmem_mutex);
		u32ChannelCreatedCnt--;
		if (!u32ChannelCreatedCnt) {
			devm_close(s32DevmemFd);
			s32DevmemFd = -1;
		}
		pthread_mutex_unlock(&devmem_mutex);
	}
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_reset_chn(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_RESET_CHN, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("ResetChn fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}


CVI_S32 platform_venc_start_recv_frame(VENC_CHN VeChn, const VENC_RECV_PIC_PARAM_S *pstRecvParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate = {0};
	CVI_U64 u64PhyAddr = 0;
	CVI_U32 u32DataFifoLen = 0;
	CVI_DATAFIFO_PARAMS_S stDataFifoParams = {
		.u32EntriesNum = 0,
		.u32CacheLineSize = sizeof(VENC_STREAM_PACK_S),
		.bDataReleaseByWriter = CVI_FALSE,
		.enOpenMode = DATAFIFO_READER,
	};

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRecvParam);

	CVI_VENC_API_IN;
	DATA_DUMP(pstRecvParam, sizeof(VENC_RECV_PIC_PARAM_S));

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_DATA_FIFO_LEN, (CVI_VOID *)&u32DataFifoLen,
				sizeof(CVI_U32), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("StartRecvFrame GetDataFifo fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	if (!u32DataFifoLen) {
		CVI_VENC_ERR("chn:%d u32DataFifoLen is 0\n", VeChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_MSG_SendSync3(u32ModFd, MSG_CMD_VENC_START_RECV_FRAME, (CVI_VOID *)pstRecvParam,
				sizeof(VENC_RECV_PIC_PARAM_S), (CVI_U32 *)&stPrivDate.as32PrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("StartRecvFrame fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	DATA_DUMP(stPrivDate.as32PrivData, sizeof(MSG_PRIV_DATA_S));

	memcpy(&u64PhyAddr, stPrivDate.as32PrivData, sizeof(u64PhyAddr));

	stDataFifoParams.u32EntriesNum = u32DataFifoLen;
	s32Ret = CVI_DATAFIFO_OpenByAddr(&hVencDataFifoHandle[VeChn], &stDataFifoParams, u64PhyAddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DATAFIFO_OpenByAddr, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	pthread_mutex_lock(&devmem_mutex);
	if (s32DevmemFd == -1) {
		open_devmem();
	}
	pthread_mutex_unlock(&devmem_mutex);
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_stop_recv_frame(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_STOP_RECV_FRAME, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("StopRecvFrame fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	if (hVencDataFifoHandle[VeChn] != 0) {
		s32Ret = CVI_DATAFIFO_Close(hVencDataFifoHandle[VeChn]);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_DATAFIFO_Close fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
			return s32Ret;
		}
		hVencDataFifoHandle[VeChn] = 0;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_query_status(VENC_CHN VeChn, VENC_CHN_STATUS_S *pstStatus)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStatus);

	CVI_VENC_API_IN;
	DATA_DUMP(pstStatus, sizeof(VENC_CHN_STATUS_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_QUERY_STATUS, (CVI_VOID *)pstStatus,
				sizeof(VENC_CHN_STATUS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("QueryStatus fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_chn_attr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnAttr);

	CVI_VENC_API_IN;
	DATA_DUMP(pstChnAttr, sizeof(VENC_CHN_ATTR_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VENC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetChnAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_chn_attr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnAttr);

	CVI_VENC_API_IN;
	DATA_DUMP(pstChnAttr, sizeof(VENC_CHN_ATTR_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_CHN_ATTR, (CVI_VOID *)pstChnAttr,
				sizeof(VENC_CHN_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetChnAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	DATA_DUMP(pstChnAttr, sizeof(VENC_CHN_ATTR_S));
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_stream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, CVI_S32 S32MilliSec)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ReadLen;
	VENC_PACK_S *pPack = pstStream->pstPack;
	VENC_STREAM_PACK_S *pStreamPack;
	CVI_U32 i;
	CVI_U32 retrytimes;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);
	UNUSED_VARIABLE(S32MilliSec);

	if(S32MilliSec < 0) {
		retrytimes = 1000;
	} else {
		retrytimes = S32MilliSec/5;
	}

GET_STREAM_RETRY:
	s32Ret = CVI_DATAFIFO_CMD(hVencDataFifoHandle[VeChn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &u32ReadLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DATAFIFO_CMD GET_AVAIL_READ_LEN fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	if (u32ReadLen > 0) {
		s32Ret = CVI_DATAFIFO_Read(hVencDataFifoHandle[VeChn], (CVI_VOID **)&pStreamPack);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_DATAFIFO_Read fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
			return s32Ret;
		}
		DATA_DUMP(pStreamPack, sizeof(VENC_STREAM_PACK_S));

		memcpy(pstStream->pstPack, pStreamPack->pstPack, pStreamPack->stStream.u32PackCount * sizeof(VENC_PACK_S));

		*pstStream = pStreamPack->stStream;
		pstStream->pstPack = pPack;
		// in alios, no mmu, pyh addr is equal vir addr
		// stream form alios ion pyh addr, need map to vir addr
		for (i = 0; i < pstStream->u32PackCount; i++) {
			VENC_PACK_S *ppack = &pPack[i];
			if (ppack->u64RingBufBasePhyAddr && ppack->u32Len && ppack->u32RingBufLen &&
				(ppack->u64PhyAddr - ppack->u64RingBufBasePhyAddr + ppack->u32Len) >
				ppack->u32RingBufLen) {
					ppack->pu8Addr = devm_map_ring(s32DevmemFd,
						ppack->u64RingBufBasePhyAddr, ppack->u32RingBufLen);
					CVI_SYS_IonInvalidateCache(ppack->u64RingBufBasePhyAddr, ppack->pu8Addr, ppack->u32RingBufLen);
					CVI_SYS_IonInvalidateCache(ppack->u64RingBufBasePhyAddr, ppack->pu8Addr + ppack->u32RingBufLen, ppack->u32RingBufLen);
					ppack->pu8Addr += (ppack->u64PhyAddr - ppack->u64RingBufBasePhyAddr);
			} else if (pPack[i].u64PhyAddr && ppack->u32Len) {
				pPack[i].pu8Addr = CVI_SYS_MmapCache(pPack[i].u64PhyAddr, pPack[i].u32Len);
				if (pPack[i].pu8Addr == NULL) {
					CVI_VENC_ERR("CVI_SYS_Mmap fail, chn:%d, PhyAddr:0x%"PRIu64" len:%d \n",
								VeChn, pPack[i].u64PhyAddr ,pPack[i].u32Len);
					return CVI_FAILURE;
				}
			}
		}

		CVI_VENC_API_OUT;
		return CVI_SUCCESS;
	}

	if(retrytimes--) {
		usleep(5 * 1000);
		goto GET_STREAM_RETRY;
	}

	CVI_VENC_API_OUT;

	return CVI_ERR_VENC_NOBUF;
}

CVI_S32 platform_venc_release_stream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
	CVI_S32 s32Ret;
	CVI_U32 i;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstStream->pstPack);

	VENC_PACK_S *pPack = pstStream->pstPack;
	for(i = 0; i < pstStream->u32PackCount; i++){
		VENC_PACK_S *ppack = &pPack[i];

		if (ppack->u64RingBufBasePhyAddr && ppack->u32Len && ppack->u32RingBufLen &&
			(ppack->u64PhyAddr - ppack->u64RingBufBasePhyAddr + ppack->u32Len) >
				ppack->u32RingBufLen) {
			devm_unmap((ppack->pu8Addr - (ppack->u64PhyAddr - ppack->u64RingBufBasePhyAddr)),
				ppack->u32RingBufLen << 1);
		} else if(ppack->pu8Addr && ppack->u32Len) {
			CVI_SYS_Munmap(ppack->pu8Addr, ppack->u32Len);
		}
	}

	s32Ret = CVI_DATAFIFO_CMD(hVencDataFifoHandle[VeChn], DATAFIFO_CMD_READ_DONE, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DATAFIFO_CMD READ_DONE fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_insert_user_data(VENC_CHN VeChn, CVI_U8 *pu8Data, CVI_U32 u32Len)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pu8Data);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_USER_DATA, pu8Data, u32Len, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("Insert User Data fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return s32Ret;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_send_frame(VENC_CHN VeChn, const VIDEO_FRAME_INFO_S *pstFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);
	MSG_PRIV_DATA_S stPrivDate;
	CVI_S32 i;
	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstFrame);

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = s32MilliSec;

	for (i = 0; i < 3; i++) {
		if (pstFrame->stVFrame.u32Length[i] && pstFrame->stVFrame.pu8VirAddr[i] != NULL) {
			CVI_SYS_IonFlushCache(
				pstFrame->stVFrame.u64PhyAddr[i],
				pstFrame->stVFrame.pu8VirAddr[i],
				pstFrame->stVFrame.u32Length[i]
			);
		}
	}
	DATA_DUMP(pstFrame, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SEND_FRAME, (CVI_VOID *)pstFrame,
				sizeof(VIDEO_FRAME_INFO_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS && s32Ret != CVI_ERR_VENC_FRC_NO_ENC) {
		CVI_VENC_ERR("SendFrame fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return s32Ret;
}

CVI_S32 platform_venc_send_frame_ex(VENC_CHN VeChn, const USER_FRAME_INFO_S *pstFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD2(CVI_ID_VENC, 0, VeChn, 1);
	MSG_PRIV_DATA_S stPrivDate;
	CVI_S32 i;
	const VIDEO_FRAME_INFO_S *pstFrameInfo = &pstFrame->stUserFrame;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstFrame);

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = s32MilliSec;

	for (i = 0; i < 3; i++) {
		if (pstFrameInfo->stVFrame.u32Length[i] && pstFrameInfo->stVFrame.pu8VirAddr[i] != NULL) {
			CVI_SYS_IonFlushCache(
				pstFrameInfo->stVFrame.u64PhyAddr[i],
				pstFrameInfo->stVFrame.pu8VirAddr[i],
				pstFrameInfo->stVFrame.u32Length[i]
			);
		}
	}
	DATA_DUMP(pstFrame, sizeof(USER_FRAME_INFO_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SEND_FRAME_EX, (CVI_VOID *)pstFrame,
				sizeof(USER_FRAME_INFO_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS && s32Ret != CVI_ERR_VENC_FRC_NO_ENC) {
		CVI_VENC_ERR("SendFrame fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return s32Ret;
}

CVI_S32 platform_venc_request_idr(VENC_CHN VeChn, CVI_BOOL bInstant)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate;

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = bInstant;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_REQUEST_IDR, NULL,
				0, &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("RequestIDR fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_enable_idr(VENC_CHN VeChn, CVI_BOOL bInstant)
{
	UNUSED_VARIABLE(VeChn); // TODO
	UNUSED_VARIABLE(bInstant);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_fd(VENC_CHN VeChn)
{
	UNUSED_VARIABLE(VeChn);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_close_fd(VENC_CHN VeChn)
{
	UNUSED_VARIABLE(VeChn);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_roi_attr(VENC_CHN VeChn, const VENC_ROI_ATTR_S *pstRoiAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRoiAttr);

	CVI_VENC_API_IN;
	DATA_DUMP(pstRoiAttr, sizeof(VENC_ROI_ATTR_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_ROI_ATTR, (CVI_VOID *)pstRoiAttr,
				sizeof(VENC_ROI_ATTR_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetRoiAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_roi_attr(VENC_CHN VeChn, CVI_U32 u32Index, VENC_ROI_ATTR_S *pstRoiAttr)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate;

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRoiAttr);

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = u32Index;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_ROI_ATTR, (CVI_VOID *)pstRoiAttr,
				sizeof(VENC_ROI_ATTR_S), &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetRoiAttr fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	DATA_DUMP(pstRoiAttr, sizeof(VENC_ROI_ATTR_S));
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h264_trans(VENC_CHN VeChn, const VENC_H264_TRANS_S *pstH264Trans)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Trans);

	CVI_VENC_API_IN;
	DATA_DUMP(pstH264Trans, sizeof(VENC_H264_TRANS_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H264_TRANS, (CVI_VOID *)pstH264Trans,
				sizeof(VENC_H264_TRANS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetH264Trans fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h264_trans(VENC_CHN VeChn, VENC_H264_TRANS_S *pstH264Trans)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Trans);

	CVI_VENC_API_IN;
	DATA_DUMP(pstH264Trans, sizeof(VENC_H264_TRANS_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H264_TRANS, (CVI_VOID *)pstH264Trans,
				sizeof(VENC_H264_TRANS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetH264Trans fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
    DATA_DUMP(pstH264Trans, sizeof(VENC_H264_TRANS_S));
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h264_entropy(VENC_CHN VeChn, const VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264EntropyEnc);

	CVI_VENC_API_IN;
	DATA_DUMP(pstH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H264_ENTROPY, (CVI_VOID *)pstH264EntropyEnc,
				sizeof(VENC_H264_ENTROPY_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetH264Entropy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h264_entropy(VENC_CHN VeChn, VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264EntropyEnc);

	CVI_VENC_API_IN;
	DATA_DUMP(pstH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H264_ENTROPY, (CVI_VOID *)pstH264EntropyEnc,
				sizeof(VENC_H264_ENTROPY_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetH264Entropy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	DATA_DUMP(pstH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h264_vui(VENC_CHN VeChn, const VENC_H264_VUI_S *pstH264Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Vui);

	CVI_VENC_API_IN;
	DATA_DUMP(pstH264Vui, sizeof(VENC_H264_VUI_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H264VUI, (CVI_VOID *)pstH264Vui,
				sizeof(VENC_H264_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_SetH264Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h264_vui(VENC_CHN VeChn, VENC_H264_VUI_S *pstH264Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Vui);

	CVI_VENC_API_IN;
	DATA_DUMP(pstH264Vui, sizeof(VENC_H264_VUI_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H264VUI, (CVI_VOID *)pstH264Vui,
				sizeof(VENC_H264_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_GetH264Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	DATA_DUMP(pstH264Vui, sizeof(VENC_H264_VUI_S));
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h265_vui(VENC_CHN VeChn, const VENC_H265_VUI_S *pstH265Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Vui);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H265VUI, (CVI_VOID *)pstH265Vui,
				sizeof(VENC_H265_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_SetH265Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h265_vui(VENC_CHN VeChn, VENC_H265_VUI_S *pstH265Vui)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Vui);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H265VUI, (CVI_VOID *)pstH265Vui,
				sizeof(VENC_H265_VUI_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_GetH265Vui fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_jpeg_param(VENC_CHN VeChn, const VENC_JPEG_PARAM_S *pstJpegParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstJpegParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_JPEG_PARAM, (CVI_VOID *)pstJpegParam,
				sizeof(VENC_JPEG_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetJpegParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_jpeg_param(VENC_CHN VeChn, VENC_JPEG_PARAM_S *pstJpegParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstJpegParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_JPEG_PARAM, (CVI_VOID *)pstJpegParam,
				sizeof(VENC_JPEG_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetJpegParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_rc_param(VENC_CHN VeChn, const VENC_RC_PARAM_S *pstRcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRcParam);

	CVI_VENC_API_IN;
	DATA_DUMP(pstRcParam, sizeof(VENC_RC_PARAM_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_RC_PARAM, (CVI_VOID *)pstRcParam,
				sizeof(VENC_RC_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetRcParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_rc_param(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRcParam);

	CVI_VENC_API_IN;
	DATA_DUMP(pstRcParam, sizeof(VENC_RC_PARAM_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_RC_PARAM, (CVI_VOID *)pstRcParam,
				sizeof(VENC_RC_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetRcParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	DATA_DUMP(pstRcParam, sizeof(VENC_RC_PARAM_S));
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}
CVI_S32 platform_venc_set_mjpeg_param(VENC_CHN VeChn, const VENC_MJPEG_PARAM_S *pstMJpegParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstMJpegParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_MJPEG_PARAM, (CVI_VOID *)pstMJpegParam,
				sizeof(VENC_MJPEG_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetJpegParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_mjpeg_param(VENC_CHN VeChn, VENC_MJPEG_PARAM_S *pstMJpegParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstMJpegParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_MJPEG_PARAM, (CVI_VOID *)pstMJpegParam,
				sizeof(VENC_MJPEG_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetJpegParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}
CVI_S32 platform_venc_set_ref_param(VENC_CHN VeChn, const VENC_REF_PARAM_S *pstRefParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRefParam);

	CVI_VENC_API_IN;
	DATA_DUMP(pstRefParam, sizeof(VENC_REF_PARAM_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_REF_PARAM, (CVI_VOID *)pstRefParam,
				sizeof(VENC_REF_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetRefParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_ref_param(VENC_CHN VeChn, VENC_REF_PARAM_S *pstRefParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstRefParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_REF_PARAM, (CVI_VOID *)pstRefParam,
				sizeof(VENC_REF_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetRefParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h265_trans(VENC_CHN VeChn, const VENC_H265_TRANS_S *pstH265Trans)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Trans);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H265_TRANS, (CVI_VOID *)pstH265Trans,
				sizeof(VENC_H265_TRANS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("set h265 trans fail fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h265_trans(VENC_CHN VeChn, VENC_H265_TRANS_S *pstH265Trans)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Trans);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H265_TRANS, (CVI_VOID *)pstH265Trans,
				sizeof(VENC_H265_TRANS_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("set h265 trans fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_frame_lost_strategy(VENC_CHN VeChn, const VENC_FRAMELOST_S *pstFrmLostParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstFrmLostParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_FRAME_LOST, (CVI_VOID *)pstFrmLostParam,
				sizeof(VENC_FRAMELOST_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetFrameLostStrategy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_frame_lost_strategy(VENC_CHN VeChn, VENC_FRAMELOST_S *pstFrmLostParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstFrmLostParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_FRAME_LOST, (CVI_VOID *)pstFrmLostParam,
				sizeof(VENC_FRAMELOST_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetFrameLostStrategy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_super_frame_strategy(VENC_CHN VeChn, const VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstSuperFrmParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd,
		MSG_CMD_VENC_SET_SUPERFRAME_STRATEGY, (CVI_VOID *)pstSuperFrmParam,
				sizeof(VENC_SUPERFRAME_CFG_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetSuperFrameStrategy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_super_frame_strategy(VENC_CHN VeChn, VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstSuperFrmParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd,
		MSG_CMD_VENC_GET_SUPERFRAME_STRATEGY, (CVI_VOID *)pstSuperFrmParam,
				sizeof(VENC_SUPERFRAME_CFG_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetSuperFrameStrategy fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_chn_param(VENC_CHN VeChn, const VENC_CHN_PARAM_S *pstChnParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnParam);

	CVI_VENC_API_IN;
	DATA_DUMP(pstChnParam, sizeof(VENC_CHN_PARAM_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_CHN_PARAM, (CVI_VOID *)pstChnParam,
				sizeof(VENC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetChnParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_chn_param(VENC_CHN VeChn, VENC_CHN_PARAM_S *pstChnParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstChnParam);

	CVI_VENC_API_IN;
	DATA_DUMP(pstChnParam, sizeof(VENC_CHN_PARAM_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_CHN_PARAM, (CVI_VOID *)pstChnParam,
				sizeof(VENC_CHN_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetChnParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}
	DATA_DUMP(pstChnParam, sizeof(VENC_CHN_PARAM_S));
	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_mod_param(const VENC_PARAM_MOD_S *pstModParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, 0);
	VENC_CHN VeChn = 0;
	UNUSED_VARIABLE(VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstModParam);

	CVI_VENC_API_IN;
	DATA_DUMP(pstModParam, sizeof(VENC_PARAM_MOD_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_MOD_PARAM, (CVI_VOID *)pstModParam,
				sizeof(VENC_PARAM_MOD_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetModParam fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_mod_param(VENC_PARAM_MOD_S *pstModParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, 0);
	VENC_CHN VeChn = 0;
	UNUSED_VARIABLE(VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstModParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_MOD_PARAM, (CVI_VOID *)pstModParam,
				sizeof(VENC_PARAM_MOD_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetModParam fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_attach_vb_pool(VENC_CHN VeChn, const VENC_CHN_POOL_S *pstPool)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstPool);

	CVI_VENC_API_IN;
	DATA_DUMP(pstPool, sizeof(VENC_CHN_POOL_S));
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_ATTACH_VBPOOL, (CVI_VOID *)pstPool,
				sizeof(VENC_CHN_POOL_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("AttachVbPool fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_detach_vb_pool(VENC_CHN VeChn)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_DETACH_VBPOOL, NULL,
				0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("DetachVbPool fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetDataFifoLen(VENC_CHN VeChn, CVI_U32 u32Len)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_DATA_FIFO_LEN, (CVI_VOID *)&u32Len,
				sizeof(u32Len), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetDataFifoLen fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetDataFifoLen(VENC_CHN VeChn, CVI_U32 *pU32Len)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_DATA_FIFO_LEN, (CVI_VOID *)pU32Len,
				sizeof(CVI_U32), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetDataFifoLen fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_cu_prediction(VENC_CHN VeChn,
		const VENC_CU_PREDICTION_S *pstCuPrediction)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstCuPrediction);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_CUPREDICTION, (CVI_VOID *)pstCuPrediction,
				sizeof(VENC_CU_PREDICTION_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetCuPrediction fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_cu_prediction(VENC_CHN VeChn, VENC_CU_PREDICTION_S *pstCuPrediction)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstCuPrediction);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_CUPREDICTION, (CVI_VOID *)pstCuPrediction,
				sizeof(VENC_CU_PREDICTION_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetCuPrediction fail, ret:0x%x\n", s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_calc_frame_param(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstFrameParam);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_frame_param(VENC_CHN VeChn, const VENC_FRAME_PARAM_S *pstFrameParam)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstFrameParam);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_frame_param(VENC_CHN VeChn, VENC_FRAME_PARAM_S *pstFrameParam)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstFrameParam);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h264_slice_split(VENC_CHN VeChn, const VENC_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h264_slice_split(VENC_CHN VeChn, VENC_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h265_slice_split(VENC_CHN VeChn, const VENC_H265_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h265_slice_split(VENC_CHN VeChn, VENC_H265_SLICE_SPLIT_S *pstSliceSplit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstSliceSplit);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h264_dblk(VENC_CHN VeChn, const VENC_H264_DBLK_S *pstH264Dblk)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Dblk);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H264_DBLK, (CVI_VOID *)pstH264Dblk,
				sizeof(VENC_H264_DBLK_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("Set h264 dblk fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h264_dblk(VENC_CHN VeChn, VENC_H264_DBLK_S *pstH264Dblk)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH264Dblk);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H264_DBLK, (CVI_VOID *)pstH264Dblk,
				sizeof(VENC_H264_DBLK_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("Set h264 dblk fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h265_dblk(VENC_CHN VeChn, const VENC_H265_DBLK_S *pstH265Dblk)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Dblk);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_H265_DBLK, (CVI_VOID *)pstH265Dblk,
				sizeof(VENC_H265_DBLK_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("Set h265 dblk fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h265_dblk(VENC_CHN VeChn, VENC_H265_DBLK_S *pstH265Dblk)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstH265Dblk);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_H265_DBLK, (CVI_VOID *)pstH265Dblk,
				sizeof(VENC_H265_DBLK_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("Set h265 dblk fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h264_intra_pred(VENC_CHN VeChn, const VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH264IntraPred);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_h264_intra_pred(VENC_CHN VeChn, VENC_H264_INTRA_PRED_S *pstH264IntraPred)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH264IntraPred);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h265_sao(VENC_CHN VeChn, const VENC_H265_SAO_S *pstH265Sao) {
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH265Sao);
	return CVI_SUCCESS;
}
CVI_S32 platform_venc_get_h265_sao(VENC_CHN VeChn, VENC_H265_SAO_S *pstH265Sao) {
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstH265Sao);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_h265_pred_unit(VENC_CHN VeChn, const VENC_H265_PU_S *pstPredUnit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstPredUnit);
	return CVI_SUCCESS;

}

CVI_S32 platform_venc_get_h265_pred_unit(VENC_CHN VeChn, VENC_H265_PU_S *pstPredUnit)
{
	UNUSED_VARIABLE(VeChn);
	UNUSED_VARIABLE(pstPredUnit);
	return CVI_SUCCESS;
}

CVI_S32 platform_venc_enable_svc(VENC_CHN VeChn, CVI_BOOL bEnable)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);
	MSG_PRIV_DATA_S stPrivDate;

	CVI_VENC_API_IN;

	stPrivDate.as32PrivData[0] = bEnable;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_ENABLE_SVC, NULL,
				0, &stPrivDate);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("EnableSVC fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_svc_param(VENC_CHN VeChn, const VENC_SVC_PARAM_S *pstSvcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstSvcParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_SVC_PARAM, (CVI_VOID *)pstSvcParam,
				sizeof(VENC_SVC_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetSvcParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_svc_param(VENC_CHN VeChn, VENC_SVC_PARAM_S *pstSvcParam)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstSvcParam);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_SVC_PARAM, (CVI_VOID *)pstSvcParam,
				sizeof(VENC_SVC_PARAM_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetSvcParam fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_set_debreath_effect(VENC_CHN VeChn, const VENC_DEBREATHEFFECT_S *pstDebreathEffect)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstDebreathEffect);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_SET_DEBREATH_EFFECT, (CVI_VOID *)pstDebreathEffect,
				sizeof(VENC_DEBREATHEFFECT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("SetDebreathEffect fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}

CVI_S32 platform_venc_get_debreath_effect(VENC_CHN VeChn, VENC_DEBREATHEFFECT_S *pstDebreathEffect)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ModFd = MODFD(CVI_ID_VENC, 0, VeChn);

	MOD_CHECK_NULL_PTR(CVI_ID_VENC, pstDebreathEffect);

	CVI_VENC_API_IN;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_VENC_GET_DEBREATH_EFFECT, (CVI_VOID *)pstDebreathEffect,
				sizeof(VENC_DEBREATHEFFECT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("GetDebreathEffect fail, chn:%d, ret:0x%x\n", VeChn, s32Ret);
		return s32Ret;
	}

	CVI_VENC_API_OUT;

	return CVI_SUCCESS;
}
