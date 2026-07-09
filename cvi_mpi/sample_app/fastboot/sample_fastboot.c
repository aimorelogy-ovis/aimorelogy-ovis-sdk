#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "cvi_defines.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_venc.h"
#include "cvi_vi.h"
#include "sample_comm.h"

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"
#define dog_argb8888_bin		  "res/dog_s_72x60_pngto8888.bin"

//#define SAVE_FILE_PATH "/tmp/"
#define SAVE_FILE_PATH "/mnt/sd/"
#define MAX_CHN (8)

extern unsigned long long timer_get_boot_us(void);
int g_enc_exit = 0;
struct VencCtx {
	VENC_CHN Vechn;
};

#define TEST_CHECK_RET(s32Ret, fmt, ...) \
	do { \
		if (s32Ret == CVI_SUCCESS) \
			SAMPLE_PRT(GREEN fmt" pass\n"NONE, ##__VA_ARGS__); \
		else \
			SAMPLE_PRT(RED fmt" fail, [%s][%d]\n"NONE, __func__, __LINE__,  ##__VA_ARGS__); \
	} while (0)

#define TEST_CHECK_ERR_RET(s32Ret, fmt, ...) \
	do { \
		if (s32Ret != CVI_SUCCESS) \
			SAMPLE_PRT(RED fmt" fail, [%s][%d]\n"NONE, __func__, __LINE__,	##__VA_ARGS__); \
	} while (0)

CVI_S32 TEST_COMM_SaveFrame(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		return CVI_FAILURE;
	}
	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		SAMPLE_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		SAMPLE_PRT(" data_len(%d) plane_len(%d)\n",
				  u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fwrite data(%d) error\n", i);
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return CVI_SUCCESS;
}

void *venc_get_stream(void *arg) {
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32OutFd = -1;
	CVI_U32 i = 0;
	size_t siRet = 0;

	struct VencCtx VencAttr = *(struct VencCtx *)arg;
	VENC_CHN VeChn = VencAttr.Vechn;

	CVI_U8 u8OutPath[128] = {0};

	VENC_RECV_PIC_PARAM_S stRecvParam = {0};
	VENC_STREAM_S stStream = {0};
	VENC_PACK_S *pstPack = NULL;
	struct timespec ts;


	stRecvParam.s32RecvPicNum = -1;
	TEST_CHECK_RET(CVI_VENC_StartRecvFrame(VeChn, &stRecvParam), "CVI_VENC_StartRecvFrame");

	SAMPLE_PRT("=====start get stream======\n");
	while(1) {
		stStream.pstPack = malloc(sizeof(VENC_PACK_S) * 8);
		if (!stStream.pstPack) {
			SAMPLE_PRT("malloc pack fail \n");
			return NULL;
		}
RETRY:
		s32Ret = CVI_VENC_GetStream(VeChn, &stStream, 2000);
		if (s32Ret == CVI_ERR_VENC_BUSY || s32Ret == CVI_ERR_VENC_NOBUF)  {
			usleep(1000 * 10);
			if (g_enc_exit)
			{
				SAMPLE_PRT("exit get stream thread\n");
				break;
			}
			goto RETRY;
		}
		else if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_GetStream fail, ret:%x \n", s32Ret);
			return NULL;
		}

		if (s32OutFd <= 0) {
			if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
				return NULL;
			}
			SAMPLE_PRT("venc first stream PTS: %lld ms, [%lld-%lld]\n",
				ts.tv_sec * 1000 + ts.tv_nsec / 1000000,
				ts.tv_sec, ts.tv_nsec);

			snprintf((char *)u8OutPath, sizeof(u8OutPath), "%sfast_chn%d.h264", SAVE_FILE_PATH, VeChn);
			s32OutFd = open((const char *)u8OutPath, O_CREAT | O_TRUNC | O_RDWR);
			if (s32OutFd < 0) {
				SAMPLE_PRT("open %s fail \n", u8OutPath);
				return NULL;
			}
		}

		for (i = 0; i < stStream.u32PackCount; i++) {
			pstPack = &stStream.pstPack[i];
			siRet = write(s32OutFd, pstPack->pu8Addr + pstPack->u32Offset, pstPack->u32Len - pstPack->u32Offset);
			if (siRet != pstPack->u32Len - pstPack->u32Offset) {
				SAMPLE_PRT("write fail, write size:%d, ret:%zu \n", pstPack->u32Len - pstPack->u32Offset, siRet);
			}
		}

		TEST_CHECK_ERR_RET(CVI_VENC_ReleaseStream(VeChn, &stStream), "CVI_VENC_ReleaseStream");
		if (stStream.pstPack != NULL) {
			free(stStream.pstPack);
			stStream.pstPack = NULL;
		}
		if (g_enc_exit)
		{
			SAMPLE_PRT("exit get stream thread\n");
			break;
		}
	}
	TEST_CHECK_RET(CVI_VENC_StopRecvFrame(VeChn), "CVI_VENC_StopRecvFrame");
	SAMPLE_PRT("=====end get stream======\n");
	sync();
	close(s32OutFd);
	return NULL;
}

static CVI_S32 venc_func_test(int chnNum)
{
	pthread_t enc_pthread_id[MAX_CHN] = {0};
	struct VencCtx VencAttr[MAX_CHN];

	int i = 0;
	int ret = 0;

	for (i = 0; i < chnNum; i++) {
		VencAttr[i].Vechn = i;
		ret = pthread_create(&enc_pthread_id[i], NULL, venc_get_stream, &VencAttr[i]);
		if (ret != 0) {
			SAMPLE_PRT("pthread :%d fail\n", i);
		}
	}

	for (i = 0; i < MAX_CHN; i++) {
		if (enc_pthread_id[i])
			pthread_join(enc_pthread_id[i], NULL);
	}

	return CVI_SUCCESS;
}

static CVI_S32 _vi_get_chn_frm()
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR outFile[64];
	VI_PIPE pipe = 0;
	VI_CHN chn = 0;

	snprintf(outFile, sizeof(outFile), "%s", "vi_out.yuv");

	s32Ret = CVI_VI_GetChnFrame(pipe, chn, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		goto err_handle;
	}
	TEST_COMM_SaveFrame(outFile, &stVideoFrame);
	TEST_CHECK_RET(CVI_VI_ReleaseChnFrame(pipe, chn, &stVideoFrame),
				"CVI_VI_ReleaseChnFrame");
err_handle:

	return s32Ret;
}

static CVI_S32 _vpss_get_chn_frm()
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR outFile[64];
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;

	snprintf(outFile, sizeof(outFile), "%s", "vpss_out.yuv");

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		goto err_handle;
	}

	TEST_COMM_SaveFrame(outFile, &stVideoFrame);
	TEST_CHECK_RET(CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame),
				"CVI_VPSS_ReleaseChnFrame");
err_handle:

	return s32Ret;
}

static CVI_S32 _test_handle_op(CVI_S32 op, CVI_S32 chnNum)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case 1: {
		s32Ret = _vi_get_chn_frm();
		if (s32Ret == CVI_SUCCESS)
			SAMPLE_PRT("\nsys test success\n");
		break;
	}

	case 2: {
		s32Ret = venc_func_test(chnNum);
		if (s32Ret == CVI_SUCCESS)
			SAMPLE_PRT("\nvenc test success\n");
		break;
	}

	case 3: {
		s32Ret = _vpss_get_chn_frm();
		if (s32Ret == CVI_SUCCESS)
			SAMPLE_PRT("\nvpss get chn frm success\n");
		break;
	}

	default:
		break;
	}

	return s32Ret;
}

CVI_VOID SAMPLE_FASTBOOT_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		g_enc_exit = 1;
		SAMPLE_PRT("Program termination abnormally\n");
	}
}

int main(int argc, char *argv[])
{
	int op;
	int arg = -1;
	CVI_S32 s32Ret;
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		return 0;
	}
	SAMPLE_PRT("main PTS: %lld ms, [%lld-%lld]\n", ts.tv_sec * 1000 + ts.tv_nsec / 1000000, ts.tv_sec, ts.tv_nsec);

	if ((argc < 2)) {
		SAMPLE_PRT("./sample_fastboot <op> <chn num>\n");
		SAMPLE_PRT("1: vi test\n");
		SAMPLE_PRT("2: venc test\n");
		SAMPLE_PRT("3: vpss test\n");
		SAMPLE_PRT("ex:./sample_fastboot 2 1)\n");
		return 0;
	}

	signal(SIGINT, SAMPLE_FASTBOOT_HandleSig);
	signal(SIGTERM, SAMPLE_FASTBOOT_HandleSig);

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return CVI_FAILURE;
	}

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		if (argc == 3) {
			arg = (CVI_S32)atoi(argv[2]);
		}

		if (arg == -1)
			arg = 100;

		s32Ret = _test_handle_op(op, arg);
		SAMPLE_PRT("\ntest op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	}

	CVI_SYS_Exit();

	return s32Ret;
}

