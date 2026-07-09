#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "devmem.h"
#include "platform_vdec.h"

#include "vc_uapi.h"

#define UNUSED(x)	((void)(x))

static CVI_S32 s32DevmemFd = -1;
static CVI_U32 u32ChannelCreatedCnt;

typedef struct _VIDEO_FRAME_INFO_EX_S {
	const VIDEO_FRAME_INFO_S *pstFrame;
	CVI_S32 s32MilliSec;
} VIDEO_FRAME_INFO_EX_S;

typedef struct _VDEC_STREAM_EX_S {
	const VDEC_STREAM_S *pstStream;
	CVI_S32 s32MilliSec;
} VDEC_STREAM_EX_S;

CVI_S32 s32VdecFd[VDEC_MAX_CHN_NUM] = {[0 ... (VDEC_MAX_CHN_NUM - 1)] = -1};
static pthread_mutex_t vdec_mutex[VDEC_MAX_CHN_NUM] = {[0 ... (VDEC_MAX_CHN_NUM - 1)] = PTHREAD_MUTEX_INITIALIZER};


static CVI_S32 open_device(VDEC_CHN VdChn)
{
    pthread_mutex_lock(&vdec_mutex[VdChn]);
	if (s32VdecFd[VdChn] < 0) {
		CVI_CHAR devName[255];

		sprintf(devName, "/dev/%s", CVI_VC_DRV_DECODER_DEV_NAME);
		s32VdecFd[VdChn] = open(devName, O_RDWR | O_DSYNC | O_CLOEXEC);
		printf("open vdec device (%s) fd: %d\n", devName, s32VdecFd[VdChn]);

		if (s32VdecFd[VdChn] < 0) {
			printf("open device (%s) fail:%s\n", devName, strerror(errno));
            pthread_mutex_unlock(&vdec_mutex[VdChn]);
			return CVI_FAILURE;
		}
		printf("open device (%s) fd: %d\n", devName, s32VdecFd[VdChn]);
	}

	if (s32VdecFd[VdChn] >= 0) {
		ioctl(s32VdecFd[VdChn], CVI_VC_VCODEC_SET_CHN, &VdChn);
	}

	if (s32DevmemFd == -1) {
		s32DevmemFd = devm_open();
		if (s32DevmemFd < 0) {
			printf("devm_open fail\n");
            pthread_mutex_unlock(&vdec_mutex[VdChn]);
			return CVI_FAILURE;
		}
	}
    pthread_mutex_unlock(&vdec_mutex[VdChn]);

	return CVI_SUCCESS;
}


CVI_S32 platform_vdec_create_chn(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_CHAR devName[255];

    if(!pstAttr) {
        return CVI_ERR_VDEC_NULL_PTR;
    }

    if((pstAttr->enType == PT_MJPEG)
        || (pstAttr->enType == PT_JPEG)) {
        if(VdChn < 0 || VdChn > VENC_MAX_CHN_NUM)
            return CVI_ERR_VDEC_INVALID_CHNID;
    }else if((pstAttr->enType == PT_H264)
        || (pstAttr->enType == PT_H265)) {
        if(VdChn < 0 || VdChn > 2*VC_MAX_CHN_NUM)
            return CVI_ERR_VDEC_INVALID_CHNID;
    } else {
        return CVI_ERR_VDEC_ILLEGAL_PARAM;
    }

	if (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		s32Ret = ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_CREATE_CHN, pstAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("ioctl CVI_VC_VDEC_CREATE_CHN fail with %d\n", s32Ret);
			return s32Ret;
		}
		u32ChannelCreatedCnt += 1;
	} else {
		printf("fail to open device %s\n", devName);
		s32Ret = CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 platform_vdec_destroy_chn(VDEC_CHN VdChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		s32Ret = ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_DESTROY_CHN, NULL);
		if (s32Ret != CVI_SUCCESS) {
			printf("ioctl CVI_VC_VDEC_DESTROY_CHN fail with %d\n", s32Ret);
			return s32Ret;
		}
		close(s32VdecFd[VdChn]);
		s32VdecFd[VdChn] = -1;
	}

	u32ChannelCreatedCnt -= 1;
	if (u32ChannelCreatedCnt == 0) {
		devm_close(s32DevmemFd);
		s32DevmemFd = -1;
	}

	return s32Ret;
}

CVI_S32 platform_vdec_get_chn_attr(VDEC_CHN VdChn, VDEC_CHN_ATTR_S *pstAttr)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
        printf("open_device fail\n");
        return CVI_ERR_VDEC_INVALID_CHNID;
    }

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_GET_CHN_ATTR, pstAttr);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_set_chn_attr(VDEC_CHN VdChn, const VDEC_CHN_ATTR_S *pstAttr)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_SET_CHN_ATTR, pstAttr);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_start_recv_stream(VDEC_CHN VdChn)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_START_RECV_STREAM, NULL);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_stop_recv_stream(VDEC_CHN VdChn)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_STOP_RECV_STREAM, NULL);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_query_status(VDEC_CHN VdChn, VDEC_CHN_STATUS_S *pstStatus)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_QUERY_STATUS, pstStatus);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_get_fd(VDEC_CHN VdChn)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	return s32VdecFd[VdChn];
}

CVI_S32 platform_vdec_close_fd(VDEC_CHN VdChn)
{
	// close fd in destroy channel
	UNUSED(VdChn);
	return CVI_SUCCESS;
}

CVI_S32 platform_vdec_reset_chn(VDEC_CHN VdChn)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_RESET_CHN);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_set_chn_param(VDEC_CHN VdChn, const VDEC_CHN_PARAM_S *pstParam)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_SET_CHN_PARAM, pstParam);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_get_chn_param(VDEC_CHN VdChn, VDEC_CHN_PARAM_S *pstParam)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_GET_CHN_PARAM, pstParam);
	}
	return CVI_FAILURE;
}

/* s32MilliSec: -1 is block,0 is no block,other positive number is timeout */
CVI_S32 platform_vdec_send_stream(VDEC_CHN VdChn, const VDEC_STREAM_S *pstStream, CVI_S32 s32MilliSec)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		VDEC_STREAM_EX_S stStreamEx, *pstStreamEx = &stStreamEx;

		pstStreamEx->pstStream = pstStream;
		pstStreamEx->s32MilliSec = s32MilliSec;

		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_SEND_STREAM, pstStreamEx);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_get_frame(VDEC_CHN VdChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		CVI_S32 s32Ret = CVI_SUCCESS;
		VIDEO_FRAME_INFO_EX_S stFrameInfoEx, *pstFrameInfoEx = &stFrameInfoEx;

		pstFrameInfoEx->pstFrame = pstFrameInfo;
		pstFrameInfoEx->s32MilliSec = s32MilliSec;

		s32Ret = ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_GET_FRAME, pstFrameInfoEx);
		if (s32Ret == CVI_SUCCESS) {
			int i = 0;

			for (i = 0; i < 3; i++) {
				if (pstFrameInfo->stVFrame.u64PhyAddr[i] &&
					pstFrameInfo->stVFrame.u32Length[i]) {
					pstFrameInfo->stVFrame.pu8VirAddr[i] =
						devm_map(s32DevmemFd,
								pstFrameInfo->stVFrame.u64PhyAddr[i],
								pstFrameInfo->stVFrame.u32Length[i]);
				}
			}
		}
		return s32Ret;
	}
	printf("fail\n");
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_release_frame(VDEC_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		int i = 0;

		for (i = 0; i < 3; i++) {
			if (pstFrameInfo->stVFrame.pu8VirAddr[i] && pstFrameInfo->stVFrame.u32Length[i]) {
				devm_unmap(pstFrameInfo->stVFrame.pu8VirAddr[i], pstFrameInfo->stVFrame.u32Length[i]);
			}
		}
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_RELEASE_FRAME, pstFrameInfo);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_attach_vb_pool(VDEC_CHN VdChn, const VDEC_CHN_POOL_S *pstPool)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_ATTACH_VBPOOL, pstPool);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_detach_vb_pool(VDEC_CHN VdChn)
{
    if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_DETACH_VBPOOL, NULL);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_set_mod_param(const VDEC_MOD_PARAM_S *pstModParam)
{
	VDEC_CHN VdChn = 0;	// default hard-code

	if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[0], CVI_VC_VDEC_SET_MOD_PARAM, pstModParam);
	}
	return CVI_FAILURE;
}

CVI_S32 platform_vdec_get_mod_param(VDEC_MOD_PARAM_S *pstModParam)
{
	VDEC_CHN VdChn = 0;	// default hard-code

	if (VdChn < 0 || (s32VdecFd[VdChn] < 0 && open_device(VdChn) != CVI_SUCCESS)) {
		printf("open_device fail\n");
		return CVI_ERR_VDEC_INVALID_CHNID;
	}

	if (s32VdecFd[VdChn] >= 0) {
		return ioctl(s32VdecFd[VdChn], CVI_VC_VDEC_GET_MOD_PARAM, pstModParam);
	}
	return CVI_FAILURE;
}