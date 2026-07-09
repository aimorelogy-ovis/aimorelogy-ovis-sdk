#include "jputypes.h"
#include "jpuapi.h"
#include "regdefine.h"
#include "jpulog.h"
#include "jpuhelper.h"
#include "jpuapifunc.h"

#include "cvi_jpg_interface.h"
#include "cvi_jpg_internal.h"
#include "version.h"
#include "malloc.h"

#ifndef UNREFERENCED_PARAM
#define UNREFERENCED_PARAM(x) ((void)(x))
#endif

#define RET_JPG_TIMEOUT (-2)
#define REENCODE_DEFAULT_QUALITY 55

static void cviGetJpegMask(void);

void cviJpgGetVersion(void)
{
	CVI_JPG_DBG_INFO("JPEG_VERSION = %s\n", JPEG_VERSION);
}

/* initial jpu core */
int CVIJpgInit(void)
{
	JpgRet ret = JPG_RET_SUCCESS;

	CVI_JPG_DBG_IF("\n");

	cviGetJpegMask();

	cviJpgGetVersion();
	JpgInitLock();

	JpgEnterLock();
	ret = JPU_Init();
	if (ret != JPG_RET_SUCCESS && ret != JPG_RET_CALLED_BEFORE) {
		JLOG(ERR, "JPU_Init failed Error code is 0x%x\n", ret);
		JpgLeaveLock();
		return ret;
	}
	JpgLeaveLock();
	return JPG_RET_SUCCESS;
}

static void cviGetJpegMask(void)
{
	jpeg_mask |= CVI_MASK_ERR;
}

/* uninitial jpu core */
void CVIJpgUninit(void)
{
	// JLOG(INFO, "CVIJpgUninit ...\n");
	JpgEnterLock();
	JPU_DeInit();
	JpgLeaveLock();
	JpgDeinitLock();
}

/* alloc a jpu handle for dcoder or encoder */
CVIJpgHandle CVIJpgOpen(CVIJpgConfig config)
{
	JpgRet ret = JPG_RET_INVALID_PARAM;
	CVIJpgHandle handle = NULL;
	JpgInst *pJpgInst = NULL;

	JpgEnterLock();
	CVI_JPG_DBG_IF("\n");
	/* check param */
	// if (CVIJPGCOD_DEC == config.type)
	//     ret = CheckJpgDecOpenParam( pop );
	// else if (CVIJPGCOD_ENC == config.type)
	//     ret = CheckJpgDecOpenParam( pop );
	// else {
	//     ret = JPG_RET_INVALID_PARAM;
	//     goto OPEN_ERROR;
	// }

	/* open flock file handle */
	open_flock();

	/* get new instance handle */
	if (CVIJPGCOD_DEC == config.type) {
		// printf("Open decoder devices!\n");
		ret = cviJpgDecOpen(&handle, &config.u.dec);
		if (JPG_RET_SUCCESS != ret) {
			CVI_JPG_DBG_ERR("Open Decode Device fail, ret %d\n", ret);
		}
	} else if (CVIJPGCOD_ENC == config.type) {
		// printf("Open encoder devices!\n");
		ret = cviJpgEncOpen(&handle, &config.u.enc);
		if (JPG_RET_SUCCESS != ret) {
			CVI_JPG_DBG_ERR("Open Encode Device fail, ret %d\n", ret);
		}
	}

	pJpgInst = (JpgInst *)handle;
	pJpgInst->s32ChnNum = config.s32ChnNum;

#ifdef REENCODE_JPEG_SUPERFRAME
	pJpgInst->datainfo = calloc(1, sizeof(CVIFRAMEBUF));
	if (pJpgInst->datainfo == NULL) {
		CVI_JPG_DBG_ERR("alloc datainfo failed\n");
	}
#endif

	CVI_JPG_DBG_IF("handle = %p\n", handle);
	JpgLeaveLock();
	return handle;
}

/* close and free alloced jpu handle */
int CVIJpgClose(CVIJpgHandle jpgHandle)
{
	int ret = JPG_RET_SUCCESS;
	JpgInst *pJpgInst = jpgHandle;
	JpgEnterLock();
	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	/* close instance handle */
	if (NULL == jpgHandle) {
		JpgLeaveLock();
		CVI_JPG_DBG_ERR("jpgHandle = NULL\n");
		return -1;
	}

#ifdef REENCODE_JPEG_SUPERFRAME
	if (pJpgInst->datainfo) {
		free(pJpgInst->datainfo);
		pJpgInst->datainfo = NULL;
	}
#endif

	if (CVIJPGCOD_DEC == pJpgInst->type) {
		ret = cviJpgDecClose(jpgHandle);
	} else if (CVIJPGCOD_ENC == pJpgInst->type) {
		ret = cviJpgEncClose(jpgHandle);
	}

	/* close flock file handle */
	close_flock();
	JpgLeaveLock();
	return ret;
}

/* */
int CVIJpgGetCaps(CVIJpgHandle jpgHandle)
{
	UNREFERENCED_PARAM(jpgHandle);
	return JPG_RET_SUCCESS;
}

/* reset jpu core */
int CVIJpgReset(CVIJpgHandle jpgHandle)
{
	UNREFERENCED_PARAM(jpgHandle);
	return JPG_RET_SUCCESS;
}

/* flush data */
int CVIJpgFlush(CVIJpgHandle jpgHandle)
{
	JpgInst *pJpgInst = jpgHandle;
	int ret = JPG_RET_SUCCESS;
	if (NULL == jpgHandle)
		return -1;
	JpgEnterLock();

	if (CVIJPGCOD_DEC == pJpgInst->type) {
		ret = cviJpgDecFlush(jpgHandle);
	} else if (CVIJPGCOD_ENC == pJpgInst->type) {
		ret = cviJpgEncFlush(jpgHandle);
	}
	JpgLeaveLock();
	return ret;
}

/* send jpu data to decode or encode */
int CVIJpgSendFrameData(CVIJpgHandle jpgHandle, void *data, int length, int s32TimeOut)
{
	JpgInst *pJpgInst = jpgHandle;
	int ret = JPG_RET_SUCCESS;
	int count = 0;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	if (NULL == jpgHandle) {
		CVI_JPG_DBG_ERR("jpgHandle = NULL\n");
		return -1;
	}

	if (s32TimeOut <= -1) {
		//block mode
		JpgEnterLock();
	} else if (s32TimeOut == 0) {
		//try once mode
		ret = JpgEnterTryLock();
		if (ret != JPG_RET_SUCCESS) {
			//timeout
			CVI_JPG_DBG_IF("try lock failure\n");
			return RET_JPG_TIMEOUT;

		} else {
			//lock success
			CVI_JPG_DBG_IF("try lock success\n");
		}
	} else {
		//time lock
		ret = JpgEnterTimeLock(s32TimeOut);
		if (ret != JPG_RET_SUCCESS) {
			//timeout
			CVI_JPG_DBG_IF("JPEG time lock timeout\n");
			return RET_JPG_TIMEOUT;

		} else {
			//lock success
			CVI_JPG_DBG_IF("time lock success\n");
		}
	}

	if (CVIJPGCOD_DEC == pJpgInst->type) {
		ret = cviJpgDecFlush(jpgHandle);
	} else if (CVIJPGCOD_ENC == pJpgInst->type) {
		ret = cviJpgEncFlush(jpgHandle);
#ifdef REENCODE_JPEG_SUPERFRAME
		memcpy(pJpgInst->datainfo, data, sizeof(CVIFRAMEBUF));
		pJpgInst->length = length;
#endif
	}

	do {
		if (CVIJPGCOD_DEC == pJpgInst->type) {
			ret = cviJpgDecSendFrameData(jpgHandle, data, length);
		} else if (CVIJPGCOD_ENC == pJpgInst->type) {
			ret = cviJpgEncSendFrameData(jpgHandle, data, length);
		}
	} while ((ret == JPG_RET_HWRESET_SUCCESS) && (count++ < 3));

	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("SendFrameData fail, ret %d\n", ret);
		JpgLeaveLock();
	}

	return ret;
}

/* after decoded or encoded, get data from jpu */
int CVIJpgGetFrameData(CVIJpgHandle jpgHandle, void *data, int length, unsigned long int *pu64HwTime)
{
	JpgInst *pJpgInst = jpgHandle;
	int ret = JPG_RET_SUCCESS;

	UNREFERENCED_PARAM(length);

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	if (NULL == jpgHandle) {
		CVI_JPG_DBG_ERR("jpgHandle = NULL\n");
		JpgLeaveLock();
		return -1;
	}

	if (CVIJPGCOD_DEC == pJpgInst->type) {
		ret = cviJpgDecGetFrameData(jpgHandle, data);
	} else if (CVIJPGCOD_ENC == pJpgInst->type) {
		ret = cviJpgEncGetFrameData(jpgHandle, data);
	}
	if (pu64HwTime) {
		*pu64HwTime = pJpgInst->u64EndTime - pJpgInst->u64StartTime;
	}

	return ret;
}

/* release stream buffer */
int CVIJpgReleaseFrameData(CVIJpgHandle jpgHandle)
{
	int ret = JPG_RET_SUCCESS;
	JpgInst *pJpgInst = jpgHandle;
	JpgEncInfo *pEncInfo;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	if (NULL == jpgHandle) {
		CVI_JPG_DBG_ERR("jpgHandle = NULL\n");
		JpgLeaveLock();
		return -1;
	}

	pEncInfo = &pJpgInst->JpgInfo.encInfo;
	if (pEncInfo->pFinalStream) {
		free(pEncInfo->pFinalStream);
		pEncInfo->pFinalStream = NULL;
	}

	// if sharing es buffer, to unlock when releasing frame
	if (jdi_use_single_es_buffer())
		JpgLeaveLock();
	return ret;
}

#ifdef REENCODE_JPEG_SUPERFRAME
int CVIJpegCheckSuperFrame(CVIJpgHandle jpgHandle, int outsize)
{
	JpgInst *pJpgInst = jpgHandle;
	int issuperframe = 0;
	JpgEncInfo *pEncInfo;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	pEncInfo = &pJpgInst->JpgInfo.encInfo;

	if (outsize > pEncInfo->streamBufSize) {
		CVI_JPG_DBG_WARN("encode size(%d) > streamBufSize(%d)\n",
			outsize, pEncInfo->streamBufSize);
		issuperframe = 1;
	}

	return issuperframe;
}

int CVIJpegProcessSuperFrame(CVIJpgHandle jpgHandle, void *data)
{
	JpgInst *pJpgInst = jpgHandle;
	int ret = JPG_RET_SUCCESS;
	JpgEncInfo *pEncInfo;
	int quality = 0;
	int bitrate = 0;
	int retry_cnt = 0;
	CVIBUF *cviBuf = (CVIBUF *)data;

	if (jpgHandle == NULL) {
		CVI_JPG_DBG_ERR("jpgHandle = NULL\n");
		return -1;
	}

	pEncInfo = &pJpgInst->JpgInfo.encInfo;
	quality = pEncInfo->openParam.quality;
	bitrate = pEncInfo->openParam.bitrate;
RETRY:
	if (pEncInfo->openParam.bitrate != 0 || pEncInfo->openParam.quality != 0) {
		if (pEncInfo->openParam.quality == 0) {
			//cbr/vbr mode
			int qualityInfo = 0;

			qualityInfo = cvi_jpeg_scale_quality(pEncInfo->curScalar);
			pEncInfo->openParam.quality = (qualityInfo * 7) / 10;
			pEncInfo->openParam.bitrate = 0;
		} else {
			//fixqp mode
			pEncInfo->openParam.quality = (pEncInfo->openParam.quality * 7) / 10;
		}
	} else {
		pEncInfo->openParam.quality = REENCODE_DEFAULT_QUALITY;
	}

	if (!jdi_use_single_es_buffer())
		JpgEnterLock();

	ret = cviJpgEncFlush(jpgHandle);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("cviJpgEncFlush fail, ret %d\n", ret);
		if (!jdi_use_single_es_buffer())
			JpgLeaveLock();
		goto OUT;
	}

	pEncInfo->reEncode++;
	ret = cviJpgEncSendFrameData(jpgHandle, pJpgInst->datainfo, pJpgInst->length);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("cviJpgEncSendFrameData fail, ret %d\n", ret);
		if (!jdi_use_single_es_buffer())
			JpgLeaveLock();
		goto OUT;
	}

	ret = cviJpgEncGetFrameData(jpgHandle, data);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("cviJpgEncSendFrameData fail, ret %d\n", ret);
		goto OUT;
	}

	if (CVIJpegCheckSuperFrame(jpgHandle, cviBuf->size)) {
		retry_cnt++;
		if (retry_cnt >= 5) {
			CVI_JPG_DBG_ERR("reencode 5 time fail, ret %d\n", ret);
			goto OUT;
		}
		goto RETRY;
	}
OUT:

	pEncInfo->openParam.bitrate = bitrate;
	pEncInfo->openParam.quality = quality;
	return ret;
}
#endif

/* get jpu encoder input data buffer */
int CVIJpgGetInputDataBuf(CVIJpgHandle jpgHandle, void *data, int length)
{
	JpgInst *pJpgInst = jpgHandle;
	int ret = JPG_RET_SUCCESS;

	UNREFERENCED_PARAM(length);

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	if (NULL == jpgHandle) {
		CVI_JPG_DBG_ERR("jpgHandle = NULL\n");
		return -1;
	}

	if (CVIJPGCOD_DEC == pJpgInst->type) {
		CVI_JPG_DBG_ERR("DO NOT SUPPORT DECODER!!\n");

	} else if (CVIJPGCOD_ENC == pJpgInst->type) {
		ret = cviJpgEncGetInputDataBuf(jpgHandle, data);
	}

	return ret;
}

int CVIVidJpuReset(void)
{
	JPU_HWReset();

	return JPG_RET_SUCCESS;
}

int cviJpegSetQuality(CVIJpgHandle jpgHandle, void *data)
{
	int ret = 0;
	int *quality = data;
	JpgInst *pJpgInst;
	JpgEncInfo *pEncInfo;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	ret = CheckJpgInstValidity(jpgHandle);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("CheckJpgInstValidity, %d\n", ret);
		return ret;
	}

	pJpgInst = jpgHandle;
	pEncInfo = &pJpgInst->JpgInfo.encInfo;

	// update correct quality to driver
	pEncInfo->openParam.quality = *quality;
	CVI_JPG_DBG_RC("quality = %d\n", pEncInfo->openParam.quality);

	return ret;
}

static int cviJpegSetChnAttr(CVIJpgHandle jpgHandle, void *arg)
{
	JpgInst *pJpgInst;
	JpgEncInfo *pEncInfo;
	cviJpegChnAttr *pChnAttr = (cviJpegChnAttr *)arg;
	int ret = 0;
	unsigned int u32Sec = 0;
	unsigned int u32Frm = 0;

	pJpgInst = jpgHandle;
	pEncInfo = &pJpgInst->JpgInfo.encInfo;

	pEncInfo->openParam.bitrate = pChnAttr->u32BitRate;

	u32Sec = pChnAttr->fr32DstFrameRate >> 16;
	u32Frm = pChnAttr->fr32DstFrameRate & 0xFFFF;

	if (u32Sec == 0) {
		pEncInfo->openParam.framerate = u32Frm;
	} else {
		pEncInfo->openParam.framerate = u32Frm / u32Sec;
	}

	pEncInfo->picWidth = pChnAttr->picWidth;
	pEncInfo->picHeight = pChnAttr->picHeight;

	return ret;
}

static int cviJpegSetMCUPerECS(CVIJpgHandle jpgHandle, void *data)
{
	int ret = 0;
	int *MCUPerECS = data;
	JpgInst *pJpgInst;
	JpgEncInfo *pEncInfo;

	pJpgInst = jpgHandle;
	pEncInfo = &pJpgInst->JpgInfo.encInfo;

	pEncInfo->openParam.restartInterval = *MCUPerECS;
	pEncInfo->rstIntval = pEncInfo->openParam.restartInterval;
	CVI_JPG_DBG_RC("MCUPerECS = %d\n", pEncInfo->openParam.restartInterval);

	return ret;
}

int cviJpegResetChn(CVIJpgHandle jpgHandle, void *data)
{
	int ret = 0;
	JpgInst *pJpgInst;
	JpgEncInfo *pEncInfo;

	UNREFERENCED_PARAM(data);

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	ret = CheckJpgInstValidity(jpgHandle);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("CheckJpgInstValidity, %d\n", ret);
		return ret;
	}

	pJpgInst = jpgHandle;
	pEncInfo = &pJpgInst->JpgInfo.encInfo;

	// reset frameIdx since JpgEncEncodeHeader() use frameIdx as header
	pEncInfo->frameIdx = 0;

	// reset quality-related table
	if (CVIJPGCOD_DEC == pJpgInst->type) {
		// do nothing now
	} else if (CVIJPGCOD_ENC == pJpgInst->type) {
		ret = cviJpgEncResetQualityTable(jpgHandle);
	}

	// reset pixel_format setting for alignedWidth/alignedHeight calculation
	pEncInfo->sourceFormat = FORMAT_420;

	return ret;
}

int cviJpegSetUserData(CVIJpgHandle jpgHandle, void *data)
{
	int ret = 0;
	JpgInst *pJpgInst;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	ret = CheckJpgInstValidity(jpgHandle);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("CheckJpgInstValidity, %d\n", ret);
		return ret;
	}

	pJpgInst = jpgHandle;

	if (CVIJPGCOD_DEC == pJpgInst->type) {
		ret = JPG_RET_WRONG_CALL_SEQUENCE;
		CVI_JPG_DBG_ERR("decoder does not support set user data\n");
	} else if (CVIJPGCOD_ENC == pJpgInst->type) {
		ret = cviJpgEncEncodeUserData(jpgHandle, data);
		if (ret != JPG_RET_SUCCESS)
			CVI_JPG_DBG_ERR("cviJpegSetUserData %d\n", ret);
	}

	return ret;
}

int cviJpegStart(CVIJpgHandle jpgHandle, void *data)
{
	int ret = JPG_RET_SUCCESS;
	JpgInst *pJpgInst;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	ret = CheckJpgInstValidity(jpgHandle);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("CheckJpgInstValidity, %d\n", ret);
		return ret;
	}

	pJpgInst = jpgHandle;

	if (pJpgInst->type == CVIJPGCOD_DEC) {
		// do nothing now
	} else if (pJpgInst->type == CVIJPGCOD_ENC) {
		ret = cviJpgEncStart(jpgHandle, data);
	}

	return ret;
}

int cviJpegSetSbmEnable(CVIJpgHandle jpgHandle, void *data)
{
	int ret = JPG_RET_SUCCESS;
	JpgInst *pJpgInst;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	ret = CheckJpgInstValidity(jpgHandle);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("CheckJpgInstValidity, %d\n", ret);
		return ret;
	}

	pJpgInst = jpgHandle;

	if (pJpgInst->type == CVIJPGCOD_DEC) {
		ret = JPG_RET_FAILURE;
	} else if (pJpgInst->type == CVIJPGCOD_ENC) {
		JpgEncInfo *pEncInfo = &pJpgInst->JpgInfo.encInfo;

		pEncInfo->bSbmEn = *(BOOL *)data;
	}
	return ret;
}

int cviJpegSetRcParam(CVIJpgHandle jpgHandle, void *data)
{
	int ret = JPG_RET_SUCCESS;
	struct CVIJpegEncRcParam *rc = (struct CVIJpegEncRcParam *)data;

	cviJpeEncSetRcParam(jpgHandle, rc);
	return ret;
}

int cviJpegShowChnInfo(CVIJpgHandle jpgHandle, void *data)
{
	int ret = JPG_RET_SUCCESS;
	JpgInst *pJpgInst;

	CVI_JPG_DBG_IF("handle = %p\n", jpgHandle);

	ret = CheckJpgInstValidity(jpgHandle);
	if (ret != JPG_RET_SUCCESS) {
		CVI_JPG_DBG_ERR("CheckJpgInstValidity, %d\n", ret);
		return ret;
	}

	pJpgInst = jpgHandle;

	printf("chn num:%d type:%d\n", pJpgInst->s32ChnNum, pJpgInst->type);

	//ENC
	if(pJpgInst->type == 2) {
		JpgEncInfo *pencInfo = &pJpgInst->JpgInfo.encInfo;

		printf("sbm:%d bIsoSendFrmEn:%d reEncode:%d\n",
			pencInfo->bSbmEn, pencInfo->bIsoSendFrmEn, pencInfo->reEncode);
	}

	return ret;
}

typedef struct _CVI_JPEG_IOCTL_OP_ {
	int opNum;
	int (*ioctlFunc)(CVIJpgHandle jpgHandle, void *arg);
} CVI_JPEG_IOCTL_OP;

CVI_JPEG_IOCTL_OP cviJpegIoctlOp[] = {
	{ CVI_JPEG_OP_NONE, NULL },
	{ CVI_JPEG_OP_SET_QUALITY, cviJpegSetQuality },
	{ CVI_JPEG_OP_SET_CHN_ATTR, cviJpegSetChnAttr },
	{ CVI_JPEG_OP_SET_MCUPerECS, cviJpegSetMCUPerECS },
	{ CVI_JPEG_OP_RESET_CHN, cviJpegResetChn },
	{ CVI_JPEG_OP_SET_USER_DATA, cviJpegSetUserData },
	{ CVI_JPEG_OP_START, cviJpegStart },
	{ CVI_JPEG_OP_SET_SBM_ENABLE, cviJpegSetSbmEnable },
	{ CVI_JPEG_OP_SET_RC_PARAM, cviJpegSetRcParam },
	{ CVI_JPEG_OP_SHOW_CHN_INFO, cviJpegShowChnInfo },
};

int cviJpegIoctl(void *handle, int op, void *arg)
{
	CVIJpgHandle jpgHandle = (CVIJpgHandle)handle;
	int ret = 0;
	int currOp;

	CVI_JPG_DBG_IF("\n");

	if (op <= 0 || op >= CVI_JPEG_OP_MAX) {
		CVI_JPG_DBG_ERR("op = %d\n", op);
		return -1;
	}

	currOp = (cviJpegIoctlOp[op].opNum & CVI_JPEG_OP_MASK) >> CVI_JPEG_OP_SHIFT;
	if (op != currOp) {
		CVI_JPG_DBG_ERR("op = %d\n", op);
		return -1;
	}

	ret = cviJpegIoctlOp[op].ioctlFunc(jpgHandle, arg);

	return ret;
}
