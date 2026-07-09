#include "vo_comm.h"

#define VO_DEVNODE	"/dev/soph-vo"

#define COMMON_POOL0_BLK_SIZE (0x600000) // 6M
#define COMMON_POOL1_BLK_SIZE (0x300000) // 3M
#define COMMON_POOL0_BLK_CNT (8)
#define COMMON_POOL1_BLK_CNT (8)

SAMPLE_VO_CONFIG_S stVoConfig;

static CVI_VOID vb_ut_handle_sig(CVI_S32 nSignal, siginfo_t *si, CVI_VOID *arg)
{
	CVI_S32 s32Ret;

	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	s32Ret = CVI_VB_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Exit failed!\n");
		exit(1);
	}

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Exit failed!\n");
		exit(1);
	}

	exit(1);
}

CVI_S32 vo_sys_init(CVI_VOID)
{
	CVI_S32 s32Ret;
	VB_CONFIG_S	stVbConf;
	struct sigaction sa = {};

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = vb_ut_handle_sig;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND;	// Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	CVI_VB_Exit();
	CVI_SYS_Exit();

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 2;
	stVbConf.astCommPool[0].u32BlkSize = COMMON_POOL0_BLK_SIZE;
	stVbConf.astCommPool[0].u32BlkCnt = COMMON_POOL0_BLK_CNT;
	SAMPLE_PRT("common pool[0] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL0_BLK_SIZE, COMMON_POOL0_BLK_CNT);
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize = COMMON_POOL1_BLK_SIZE;
	stVbConf.astCommPool[1].u32BlkCnt = COMMON_POOL1_BLK_CNT;
	SAMPLE_PRT("common pool[1] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL1_BLK_SIZE, COMMON_POOL1_BLK_CNT);
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 vo_sys_deinit(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VB_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Exit failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Exit failed!\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vo_init_by_fmt(CVI_S32 fmt, VO_DEV VoDev)
{
	RECT_S stDefDispRect;
	SIZE_S stDefImageSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	stDefDispRect.s32X = 0;
	stDefDispRect.s32Y = 0;
	stDefDispRect.u32Width = 720;
	stDefDispRect.u32Height = 1280;
	stDefImageSize.u32Width = stDefDispRect.u32Width;
	stDefImageSize.u32Height = stDefDispRect.u32Height;
	s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVoConfig.VoDev	 = VoDev;
	stVoConfig.stVoPubAttr.enIntfType = VO_INTF_MIPI;
	stVoConfig.stVoPubAttr.enIntfSync = VO_OUTPUT_720x1280_60;
	stVoConfig.stDispRect	 = stDefDispRect;
	stVoConfig.stImageSize	 = stDefImageSize;
	stVoConfig.enPixFormat	 = fmt;
	SAMPLE_PRT("SAMPLE_COMM_VO_StartVO fmt %x\n", fmt);
	stVoConfig.enVoMode	 = VO_MODE_1MUX;
	s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, ROTATION_0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation is fail\n");
		SAMPLE_COMM_VO_StopVO(&stVoConfig);
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 vo_prepare_frame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		SAMPLE_PRT("Null pointer!\n");
		return CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(stSize.u32Width, stSize.u32Height, enPixelFormat, DATA_BITWIDTH_8,
				  COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize.u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize.u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0x12345678;
	pstVideoFrame->stVFrame.u64PTS = 0x12345678;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_PRT("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0] +
			ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1] +
			ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	return CVI_SUCCESS;
}

CVI_S32 vo_send_frame(VO_UT_FILE *fileptr, VO_DEV VoDev)
{
	CVI_S32 ret = 0;
	FILE *fp;
	SIZE_S stSize;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	stSize.u32Width = fileptr->stSize.u32Width;
	stSize.u32Height = fileptr->stSize.u32Height;

	SAMPLE_PRT("File[name, w, h, format]=[%s, %d,%d, %d]\n",
		  fileptr->filename, fileptr->stSize.u32Width,
		  fileptr->stSize.u32Height, fileptr->enPixelFormat);

	if (vo_prepare_frame(stSize, fileptr->enPixelFormat, &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return CVI_FAILURE;
	}

	stVideoFrame.stVFrame.pu8VirAddr[0] = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0],
							   stVideoFrame.stVFrame.u32Length[0]);
	if (stVideoFrame.stVFrame.pu8VirAddr[0] == NULL) {
		SAMPLE_PRT("CVI_SYS_Mmap failed\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.pu8VirAddr[1] = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[1],
							   stVideoFrame.stVFrame.u32Length[1]);
	if (stVideoFrame.stVFrame.pu8VirAddr[1] == NULL) {
		SAMPLE_PRT("CVI_SYS_Mmap failed\n");
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
		return CVI_FAILURE;
	}

	SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0],
		  stVideoFrame.stVFrame.u64PhyAddr[1]);
	SAMPLE_PRT("vir addr(%p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0],
		  stVideoFrame.stVFrame.pu8VirAddr[1]);

	fp = fopen(fileptr->filename, "r");
	if (fp == NULL) {
		SAMPLE_PRT("open file %s fail\n", fileptr->filename);
		return CVI_FAILURE;
	}

	SAMPLE_PRT("open file %s success\n", fileptr->filename);

	for (CVI_S32 i = 0; i < 2; i++) {
		SAMPLE_PRT("vir addr(%p, %d)\n", stVideoFrame.stVFrame.pu8VirAddr[i],
						stVideoFrame.stVFrame.u32Length[i]);
		fread((CVI_VOID *)stVideoFrame.stVFrame.pu8VirAddr[i]
			, stVideoFrame.stVFrame.u32Length[i], 1, fp);
	}

	fclose(fp);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.u32Length[1]);

	CVI_VO_SendFrame(VoLayer, VoChn, &stVideoFrame, -1);
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]));

	return ret;
}

CVI_S32 vo_deinit(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_COMM_VO_StopVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StopVO failed with %#x\n", s32Ret);
	}

	return s32Ret;
}