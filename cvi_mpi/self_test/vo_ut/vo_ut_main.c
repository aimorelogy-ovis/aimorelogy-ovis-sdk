#include "cvi_buffer.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_vpss.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"

#include "vo_ut_comm.h"

#define VO_DEVNODE	"/dev/soph-vo"

#define COMMON_POOL0_BLK_SIZE (0x600000) // 6M
#define COMMON_POOL1_BLK_SIZE (0x300000) // 3M
#define COMMON_POOL0_BLK_CNT (8)
#define COMMON_POOL1_BLK_CNT (8)

typedef struct vo_ut_FILE {
	SIZE_S stSize;
	PIXEL_FORMAT_E enPixelFormat;
	CVI_CHAR filename[30];
} VO_UT_FILE;

CVI_S32 vo_fd = -1;
CVI_S32 intf;
VO_CONFIG_S stVoConfig;

VO_UT_FILE vo_ut_file1[] = {
	{.stSize.u32Width = 720, .stSize.u32Height = 1280,
	.filename = "res/vo/720x1280_0.nv21", .enPixelFormat = PIXEL_FORMAT_NV21},
	{.stSize.u32Width = 720, .stSize.u32Height = 1280,
	.filename = "res/vo/720x1280_1.nv21", .enPixelFormat = PIXEL_FORMAT_NV21},
	{.stSize.u32Width = 1280, .stSize.u32Height = 720,
	.filename = "res/vo/1280x720.nv21",  .enPixelFormat = PIXEL_FORMAT_NV21},
	{.stSize.u32Width = 1080, .stSize.u32Height = 1920,
	.filename = "res/vo/golden_1080x1920.nv21", .enPixelFormat = PIXEL_FORMAT_NV21},
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv21", .enPixelFormat = PIXEL_FORMAT_NV21}
};

VO_UT_FILE vo_ut_file2[] = {
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.rgb", .enPixelFormat = PIXEL_FORMAT_RGB_888},//0:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.bgr", .enPixelFormat = PIXEL_FORMAT_BGR_888},//1:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.rgbm", .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR},//2:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.bgrm", .enPixelFormat = PIXEL_FORMAT_BGR_888_PLANAR},//3:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yuv420", .enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420},//4:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yuv422", .enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422},//5:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.y", .enPixelFormat = PIXEL_FORMAT_YUV_400},//6:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv12", .enPixelFormat = PIXEL_FORMAT_NV12},//7:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv21", .enPixelFormat = PIXEL_FORMAT_NV21},//8:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv16", .enPixelFormat = PIXEL_FORMAT_NV16},//9:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv61", .enPixelFormat = PIXEL_FORMAT_NV61},//10:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yuyv", .enPixelFormat = PIXEL_FORMAT_YUYV},//11:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.uyvy", .enPixelFormat = PIXEL_FORMAT_UYVY},//12:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yvyu", .enPixelFormat = PIXEL_FORMAT_YVYU},//13:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.vyuy", .enPixelFormat = PIXEL_FORMAT_VYUY},//14:
};

typedef enum _VO_TEST_OP
{
	VO_TEST_SEND_SINGLE_FRAME = 1,
	VO_TEST_ROTATION = 2,
	VO_TEST_GET_INFO = 3,
	VO_TEST_ENABLE_DISABLE = 4,
	VO_TEST_SHOW_HIDE = 5,
	VO_TEST_PAUSE_RESUME = 6,
	VO_TEST_PROC_AMP = 7,
	VO_TEST_GAMMA = 8,
	VO_TEST_MULTI_FRAME = 9,
	VO_TEST_CLEAR_BUF = 10,
	VO_TEST_FMT_BIND = 30,
	VO_TEST_SUSPEND = 31,
	VO_TEST_AUTO = 99,
} VO_TEST_OP;

static CVI_S32 vo_prepare_frame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		UT_PRT("Null pointer!\n");
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
		UT_PRT("Can't acquire vb block\n");
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

static CVI_S32 vo_ut_send_frame(VO_UT_FILE *fileptr, VO_DEV VoDev)
{
	CVI_S32 ret = 0;
	FILE *fp;
	SIZE_S stSize;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	stSize.u32Width = fileptr->stSize.u32Width;
	stSize.u32Height = fileptr->stSize.u32Height;

	UT_PRT("File[name, w, h, format]=[%s, %d,%d, %d]\n",
		  fileptr->filename, fileptr->stSize.u32Width,
		  fileptr->stSize.u32Height, fileptr->enPixelFormat);

	if (vo_prepare_frame(stSize, fileptr->enPixelFormat, &stVideoFrame) != CVI_SUCCESS) {
		UT_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return CVI_FAILURE;
	}

	stVideoFrame.stVFrame.pu8VirAddr[0] = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0],
							   stVideoFrame.stVFrame.u32Length[0]);
	if (stVideoFrame.stVFrame.pu8VirAddr[0] == NULL) {
		UT_PRT("CVI_SYS_Mmap failed\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.pu8VirAddr[1] = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[1],
							   stVideoFrame.stVFrame.u32Length[1]);
	if (stVideoFrame.stVFrame.pu8VirAddr[1] == NULL) {
		UT_PRT("CVI_SYS_Mmap failed\n");
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
		return CVI_FAILURE;
	}

	UT_PRT("phy addr(%#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0],
		  stVideoFrame.stVFrame.u64PhyAddr[1]);
	UT_PRT("vir addr(%p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0],
		  stVideoFrame.stVFrame.pu8VirAddr[1]);

	fp = fopen(fileptr->filename, "r");
	if (fp == NULL) {
		UT_PRT("open file %s fail\n", fileptr->filename);
		return CVI_FAILURE;
	}

	UT_PRT("open file %s success\n", fileptr->filename);

	for (CVI_S32 i = 0; i < 2; i++) {
		UT_PRT("vir addr(%p, %d)\n", stVideoFrame.stVFrame.pu8VirAddr[i],
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

static CVI_VOID vb_ut_handle_sig(CVI_S32 nSignal, siginfo_t *si, CVI_VOID *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	s32Ret = CVI_VB_Exit();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Exit failed!\n");
		exit(1);
	}

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Exit failed!\n");
		exit(1);
	}

	exit(1);
}

// TODO:
#ifdef VO_SUSPEND_RESUME_IMPLEMENT
static CVI_S32 vo_ut_suspend_function(CVI_VOID *pvData)
{
	UNUSED(pvData);

	return 0;
}
#endif

static CVI_S32 vo_ut_vpss_init_by_fmt(CVI_S32 fmt, SIZE_S in_size, SIZE_S out_size)
{
	PIXEL_FORMAT_E enPixelFormat = fmt;
	PIXEL_FORMAT_E enPixelFormatOut = fmt;

	SIZE_S stSize = in_size;
	SIZE_S stOutSize = out_size;
	CVI_S32 s32Ret = CVI_SUCCESS;

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP VpssGrp = 0;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN VpssChn = VPSS_CHN0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

	// grp0 for left half
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = enPixelFormat;
	stVpssGrpAttr.u32MaxW = stSize.u32Width;
	stVpssGrpAttr.u32MaxH = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width = stOutSize.u32Width;
	astVpssChnAttr[VpssChn].u32Height = stOutSize.u32Height;
	astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat = enPixelFormatOut;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth = 1;
	astVpssChnAttr[VpssChn].bMirror = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width = stOutSize.u32Width;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = stOutSize.u32Height;
	astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
	}

	return s32Ret;
}

static CVI_S32 vo_ut_vpss_deinit(CVI_VOID)
{
	VPSS_GRP VpssGrp = 0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_S32 s32Ret = CVI_SUCCESS;

	abChnEnable[0] = CVI_TRUE;
	s32Ret = VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
	}

	return s32Ret;
}

#ifdef VO_SUSPEND_RESUME_IMPLEMENT
static CVI_S32 vo_ut_resume_function(CVI_VOID *pvData)
{
	UNUSED(pvData);
	return 0;
}
#endif

static CVI_S32 vo_ut_sys_init(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	struct sigaction sa = {};

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = vb_ut_handle_sig;
	sa.sa_flags = SA_SIGINFO | SA_RESETHAND; // Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	CVI_VB_Exit();
	CVI_SYS_Exit();

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 2;
	stVbConf.astCommPool[0].u32BlkSize = COMMON_POOL0_BLK_SIZE;
	stVbConf.astCommPool[0].u32BlkCnt = COMMON_POOL0_BLK_CNT;
	UT_PRT("common pool[0] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL0_BLK_SIZE, COMMON_POOL0_BLK_CNT);
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize = COMMON_POOL1_BLK_SIZE;
	stVbConf.astCommPool[1].u32BlkCnt = COMMON_POOL1_BLK_CNT;
	UT_PRT("common pool[1] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL1_BLK_SIZE, COMMON_POOL1_BLK_CNT);
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_SetConf failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Init failed!\n");
		CVI_SYS_Exit();
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 vo_ut_sys_deinit(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VB_Exit();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VB_Exit failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Exit failed!\n");
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 vo_ut_vo_init_by_fmt(CVI_S32 fmt, VO_DEV VoDev)
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
	s32Ret = VO_GetDefConfig(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("VO_GetDefConfig failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVoConfig.VoDev = VoDev;
	stVoConfig.stVoPubAttr.enIntfType = VO_INTF_MIPI;
	stVoConfig.stVoPubAttr.enIntfSync = VO_OUTPUT_720x1280_60;
	stVoConfig.stDispRect = stDefDispRect;
	stVoConfig.stImageSize = stDefImageSize;
	stVoConfig.enPixFormat = fmt;
	UT_PRT("VO_StartVO fmt %x\n", fmt);
	stVoConfig.enVoMode = VO_MODE_1MUX;
	s32Ret = VO_StartVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("VO_StartVO failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, ROTATION_0);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_SetChnRotation is fail\n");
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 vo_ut_vo_deinit(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = VO_StopVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("VO_StopVO failed with %#x\n", s32Ret);
	}

	return s32Ret;
}

static CVI_S32 vo_ut_test_single_frame(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(3);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_rotation(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, ROTATION_90);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_SetChnRotation is fail\n");
		return s32Ret;
	}

	s32Ret = vo_ut_send_frame(&vo_ut_file1[2], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(3);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, ROTATION_270);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_SetChnRotation is fail\n");
		return s32Ret;
	}

	s32Ret = vo_ut_send_frame(&vo_ut_file1[2], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(3);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_get_info(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;
	VO_PUB_ATTR_S stPubAttr;
	VO_VIDEO_LAYER_ATTR_S stVideoAttr;
	CVI_U32 u32BufLen;
	VO_CHN_ATTR_S stChnAttr;
	ROTATION_E enRotation;
	CVI_S32 s32ChnFrmRate;
	CVI_U64 u64ChnPTS;
	VO_QUERY_STATUS_S stStatus;

	s32Ret = vo_ut_vo_deinit();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_vo_deinit failed\n");
		return s32Ret;
	}

	s32Ret = vo_ut_vo_init_by_fmt(vo_ut_file1[0].enPixelFormat, VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_vo_init_by_fmt failed\n");
		return s32Ret;
	}

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame is fail\n");
		return s32Ret;
	}

	sleep(3);

	s32Ret = CVI_VO_GetPubAttr(VoDev, &stPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetPubAttr is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stVideoAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetVideoLayerAttr is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_GetDisplayBufLen(VoLayer, &u32BufLen);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetDisplayBufLen is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_GetChnRotation(VoLayer, VoChn, &enRotation);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetChnRotation is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_GetChnAttr(VoLayer, VoChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetChnAttr is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_GetChnFrameRate(VoLayer, VoChn, &s32ChnFrmRate);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetChnRecvThreshold is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_GetChnPTS(VoLayer, VoChn, &u64ChnPTS);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetChnPTS is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_QueryChnStatus(VoLayer, VoChn, &stStatus);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetChnPTS is fail\n");
		return s32Ret;
	}

	UT_PRT("-- VO Dev info --\n");
	UT_PRT("Intf Type(%d)  Sync Type(%d)\n", stPubAttr.enIntfType, stPubAttr.enIntfSync);
	if (stPubAttr.enIntfSync == VO_OUTPUT_USER) {
		UT_PRT("Hor front-porch(%d) back-porch(%d) active(%d) sync(%d)\n"
			, stPubAttr.stSyncInfo.u16Hfb, stPubAttr.stSyncInfo.u16Hbb
			, stPubAttr.stSyncInfo.u16Hact, stPubAttr.stSyncInfo.u16Hpw);
		UT_PRT("Ver front-porch(%d) back-porch(%d) active(%d) sync(%d)\n"
			, stPubAttr.stSyncInfo.u16Vfb, stPubAttr.stSyncInfo.u16Vbb
			, stPubAttr.stSyncInfo.u16Vact, stPubAttr.stSyncInfo.u16Vpw);
	}

	UT_PRT("-- VO VideoLayer info --\n");
	UT_PRT("width(%d)  height(%d)\n"
		, stVideoAttr.stImageSize.u32Width, stVideoAttr.stImageSize.u32Height);
	UT_PRT("PixFormat(%d)  DispFrmRate(%d)  u32BufLen(%d)\n"
		, stVideoAttr.enPixFormat, stVideoAttr.u32DispFrmRt, u32BufLen);

	UT_PRT("-- VO Chn info --\n");
	UT_PRT("Chn Rotation(%d)\n", enRotation);
	UT_PRT("Chn Attr(x,y,w,h,priority)=(%d, %d, %d, %d, %d)\n",
		stChnAttr.stRect.s32X, stChnAttr.stRect.s32Y,
		stChnAttr.stRect.u32Width, stChnAttr.stRect.u32Height,
		stChnAttr.u32Priority);
	UT_PRT("Chn ChnFrmRate(%d) PlayPts(%" PRIu64 ") BufferUsed(%d)\n",
		s32ChnFrmRate, u64ChnPTS, stStatus.u32ChnBufUsed);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_enable_disable(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;
	CVI_S32 enable = 0;

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame is fail\n");
		return s32Ret;
	}

	sleep(3);

	s32Ret = CVI_VO_DisableChn(VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_DisableChn is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_DisableVideoLayer is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_Disable(VoDev);
	enable = CVI_VO_IsEnabled(VoDev);
	if (enable) {
		UT_PRT("VO disable failed!\n");
		return s32Ret;
	}

	sleep(3);

	s32Ret = CVI_VO_Enable(VoDev);
	enable = CVI_VO_IsEnabled(VoDev);
	if (!enable) {
		UT_PRT("VO enable failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_DisableVideoLayer is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VO_EnableChn(VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_EnableChn is fail\n");
		return s32Ret;
	}

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame is fail\n");
		return s32Ret;
	}

	sleep(3);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_show_hide(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(3);

	for (CVI_S32 i = 0; i < 5; i++) {
		s32Ret = CVI_VO_HideChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo hide chn failed\n");
			return s32Ret;
		}

		sleep(1);

		s32Ret = CVI_VO_ShowChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo show chn failed\n");
			return s32Ret;
		}

		sleep(1);
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_pause_resume(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(3);

	s32Ret = CVI_VO_PauseChn(VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo hide chn failed\n");
		return s32Ret;
	}

	s32Ret = vo_ut_send_frame(&vo_ut_file1[1], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(1);

	s32Ret = CVI_VO_ResumeChn(VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo show chn failed\n");
		return s32Ret;
	}

	s32Ret = vo_ut_send_frame(&vo_ut_file1[1], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(1);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_proc_amp(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	PROC_AMP_E type;
	PROC_AMP_CTRL_S ctrl;
	CVI_S32 cur, value;

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(3);

	value = 0;
	for (type = PROC_AMP_BRIGHTNESS; type < PROC_AMP_MAX; ++type) {
		s32Ret = CVI_VO_SetLayerProcAmp(VoLayer, type, value);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VO_SetLayerProcAmp is fail\n");
			return s32Ret;
		}

		s32Ret = CVI_VO_GetLayerProcAmpCtrl(VoLayer, type, &ctrl);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VO_GetLayerProcAmpCtrl is fail\n");
			return s32Ret;
		}
		s32Ret = CVI_VO_GetLayerProcAmp(VoLayer, type, &cur);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_VO_GetLayerProcAmp is fail\n");
			return s32Ret;
		}
		UT_PRT("min(%d) max(%d) step(%d) default(%d) current(%d) value(%d)\n"
			, ctrl.minimum, ctrl.maximum, ctrl.step, ctrl.default_value, cur, value);
		value++;
	}

	sleep(3);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_gamma(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_BIN_INFO_S Get_vo_bin_info;
	CVI_S32 i = 0;

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(3);

	VO_BIN_INFO_S vo_bin_info = {
		.gamma_info = {
			.enable = CVI_TRUE,
			.osd_apply = CVI_TRUE,
			.value = {
				0, 0, 0, 1, 2, 3, 5, 6, 8, 10, 11, 13,
				16, 18, 20, 23, 25, 28, 31, 34, 37, 40,
				43, 47, 50, 54, 57, 61, 65, 69, 73, 77,
				81, 85, 89, 94, 98, 103, 107, 112, 117,
				122, 127, 132, 137, 142, 147, 153, 158,
				164, 169, 175, 181, 186, 192, 198, 204,
				210, 216, 222, 229, 235, 241, 248, 255
			},

			// .value = {
			//	0,	 3,   7,   11,	15,  19,  23,  27,
			//	31,  35,  39,  43,	47,  51,  55,  59,
			//	63,  67,  71,  75,	79,  83,  87,  91,
			//	95,  99,  103, 107, 111, 115, 119, 123,
			//	127, 131, 135, 139, 143, 147, 151, 155,
			//	159, 163, 167, 171, 175, 179, 183, 187,
			//	191, 195, 199, 203, 207, 211, 215, 219,
			//	223, 227, 231, 235, 239, 243, 247, 251,
			//	255
			// }
		},
		.guard_magic = 0x12345678
	};

	vo_bin_info.gamma_info.s32VoDev = VoDev;
	Get_vo_bin_info.gamma_info.s32VoDev = VoDev;

	s32Ret = CVI_VO_SetGammaInfo(&vo_bin_info.gamma_info);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_SetGammaInfo failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VO_GetGammaInfo(&Get_vo_bin_info.gamma_info);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_GetGammaInfo failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	for (i = 0; i < VO_GAMMA_NODENUM; ++i) {
		if (Get_vo_bin_info.gamma_info.value[i] != vo_bin_info.gamma_info.value[i]) {
			s32Ret = CVI_FAILURE;
			UT_PRT("Gamma info compare fail!\n");
			return s32Ret;
		}
	}

	if ((Get_vo_bin_info.gamma_info.enable != vo_bin_info.gamma_info.enable) &&
		(Get_vo_bin_info.gamma_info.osd_apply != vo_bin_info.gamma_info.osd_apply) &&
		(Get_vo_bin_info.gamma_info.s32VoDev != vo_bin_info.gamma_info.s32VoDev)) {
		s32Ret = CVI_FAILURE;
		UT_PRT("Gamma info compare fail!\n");
	}

	sleep(3);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_multi_frame(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vo_ut_vo_deinit();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_vo_deinit failed\n");
		return s32Ret;
	}

	s32Ret = vo_ut_vo_init_by_fmt(vo_ut_file1[0].enPixelFormat, VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_vo_init_by_fmt failed\n");
		return s32Ret;
	}

	for (CVI_S32 i = 0; i < 50; i++) {
		s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo_ut_send_frame failed\n");
			return s32Ret;
		}

		usleep(100 * 1000);
		s32Ret = vo_ut_send_frame(&vo_ut_file1[1], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo_ut_send_frame failed\n");
			return s32Ret;
		}

		usleep(100 * 1000);
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_clear_buf(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vo_ut_send_frame(&vo_ut_file1[0], VoDev);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_send_frame failed\n");
		return s32Ret;
	}

	sleep(1);

	s32Ret = CVI_VO_ClearChnBuf(0, 0, 1);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_ClearChnBuf failed\n");
		return s32Ret;
	}

	sleep(1);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_fmt_bind(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;
	CVI_U32 index;
	SIZE_S stSize;
	SIZE_S stOutSize_720x1280 = {.u32Width = 720, .u32Height = 1280};

	for (index = 0; index < ARRAY_SIZE(vo_ut_file2); ++index) {
		s32Ret = vo_ut_vo_deinit();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo_ut_vo_deinit failed\n");
			return s32Ret;
		}

		s32Ret = vo_ut_vo_init_by_fmt(vo_ut_file2[index].enPixelFormat, VoDev);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo_ut_vo_init_by_fmt failed\n");
			return s32Ret;
		}

		s32Ret = vo_ut_vpss_init_by_fmt(vo_ut_file2[index].enPixelFormat, vo_ut_file2[index].stSize,
										stOutSize_720x1280);
		if (s32Ret != CVI_SUCCESS) {
			vo_ut_vpss_deinit();
			UT_PRT("vo_ut_vpss_init_by_fmt failed\n");
			return s32Ret;
		}

		s32Ret = VPSS_Bind_VO(0, 0, VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			vo_ut_vpss_deinit();
			UT_PRT("SAMPLE_COMM_VPSS_Bind_VO failed\n");
			return s32Ret;
		}

		stSize.u32Height = vo_ut_file2[index].stSize.u32Height;
		stSize.u32Width = vo_ut_file2[index].stSize.u32Width;

		s32Ret = VPSS_SendFrame(0, &stSize, vo_ut_file2[index].enPixelFormat,
								vo_ut_file2[index].filename);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UnBind_VO(0, 0, VoLayer, VoChn);
			vo_ut_vpss_deinit();
			UT_PRT("s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		sleep(1);
		VPSS_UnBind_VO(0, 0, VoLayer, VoChn);
		vo_ut_vpss_deinit();
	}

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_test_suspend(VO_DEV VoDev)
{
	UNUSED(VoDev);
	CVI_S32 s32Ret = CVI_SUCCESS;
#ifdef VO_SUSPEND_RESUME_IMPLEMENT
	VO_PM_OPS_S vo_ops = {
		.pfnPanelSuspend = vo_ut_suspend_function,
		.pfnPanelResume = vo_ut_resume_function,
	};

	s32Ret = CVI_VO_RegPmCallBack(VoDev, &vo_ops, NULL);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VO_RegPmCallBack failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VO_Suspend();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo suspend failed\n");
		return s32Ret;
	}
	usleep(30 * 1000);

	s32Ret = CVI_VO_Resume();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo resume failed\n");
		return s32Ret;
	}
#endif
	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_test_auto(VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret |= vo_ut_test_single_frame(VoDev);
	s32Ret |= vo_ut_test_rotation(VoDev);
	s32Ret |= vo_ut_test_get_info(VoDev);
	s32Ret |= vo_ut_test_enable_disable(VoDev);
	s32Ret |= vo_ut_test_show_hide(VoDev);
	s32Ret |= vo_ut_test_pause_resume(VoDev);
	s32Ret |= vo_ut_test_proc_amp(VoDev);
	s32Ret |= vo_ut_test_gamma(VoDev);
	s32Ret |= vo_ut_test_multi_frame(VoDev);
	s32Ret |= vo_ut_test_clear_buf(VoDev);
	s32Ret |= vo_ut_test_fmt_bind(VoDev);
	s32Ret |= vo_ut_test_suspend(VoDev);

	UT_CHECK_CASE_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 vo_ut_handle_op(CVI_S32 op, VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (op != 255) {
		s32Ret = vo_ut_vo_init_by_fmt(vo_ut_file1[0].enPixelFormat, VoDev);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo_ut_vo_init_by_fmt failed\n");
			return -1;
		}
	}

	switch (op)
	{
	case VO_TEST_SEND_SINGLE_FRAME:
		s32Ret = vo_ut_test_single_frame(VoDev);
		break;
	case VO_TEST_ROTATION:
		s32Ret = vo_ut_test_rotation(VoDev);
		break;
	case VO_TEST_GET_INFO:
		s32Ret = vo_ut_test_get_info(VoDev);
		break;
	case VO_TEST_ENABLE_DISABLE:
		s32Ret = vo_ut_test_enable_disable(VoDev);
		break;
	case VO_TEST_SHOW_HIDE:
		s32Ret = vo_ut_test_show_hide(VoDev);
		break;
	case VO_TEST_PAUSE_RESUME:
		s32Ret = vo_ut_test_pause_resume(VoDev);
		break;
	case VO_TEST_PROC_AMP:
		s32Ret = vo_ut_test_proc_amp(VoDev);
		break;
	case VO_TEST_GAMMA:
		s32Ret = vo_ut_test_gamma(VoDev);
		break;
	case VO_TEST_MULTI_FRAME:
		s32Ret = vo_ut_test_multi_frame(VoDev);
		break;
	case VO_TEST_CLEAR_BUF:
		s32Ret = vo_ut_test_clear_buf(VoDev);
		break;
	case VO_TEST_FMT_BIND:
		s32Ret = vo_ut_test_fmt_bind(VoDev);
		break;
	case VO_TEST_SUSPEND:
		s32Ret = vo_ut_test_suspend(VoDev);
		break;
	case VO_TEST_AUTO:
		s32Ret = vo_test_auto(VoDev);
		break;
	case 255:
		break;
	default:
		UT_PRT("the index %d is invaild!\n", op);
		break;
	}

	if (op != 255) {
		s32Ret |= vo_ut_vo_deinit();
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vo_ut_vo_deinit failed\n");
		}
	}

	return s32Ret;
}

static CVI_VOID vo_show_help(CVI_VOID)
{
	UT_PRT("%4d: vo test Send one frame 720x1280\n", VO_TEST_SEND_SINGLE_FRAME);
	UT_PRT("%4d: vo test Send one frame 1280x720 with rotation 90/270\n", VO_TEST_ROTATION);
	UT_PRT("%4d: vo test Get VO info\n", VO_TEST_GET_INFO);
	UT_PRT("%4d: vo test dev/videolayer/chn disable/enable\n", VO_TEST_ENABLE_DISABLE);
	UT_PRT("%4d: vo test Vo Chn Show/Hide\n", VO_TEST_SHOW_HIDE);
	UT_PRT("%4d: vo test Vo Chn Pause/Resume\n", VO_TEST_PAUSE_RESUME);
	UT_PRT("%4d: vo test ProcAmp ctrl\n", VO_TEST_PROC_AMP);
	UT_PRT("%4d: vo test Set/Get gamma info\n", VO_TEST_GAMMA);
	UT_PRT("%4d: vo test Send 2 frames repeatly\n", VO_TEST_MULTI_FRAME);
	UT_PRT("%4d: vo test vo clear chn buffer\n", VO_TEST_CLEAR_BUF);
	UT_PRT("%4d: vo test vo init by fmt & vpss bind vo\n", VO_TEST_FMT_BIND);
	UT_PRT("%4d: vo test Susepnd/Resume\n", VO_TEST_SUSPEND);
	UT_PRT("%4d: vo test auto test\n", VO_TEST_AUTO);
	UT_PRT(" 255: exit\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 op;
	VO_DEV VoDev = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vo_ut_sys_init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_sys_init failed\n");
		return CVI_FAILURE;
	}

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = vo_ut_handle_op(op, VoDev);
		UT_PRT("vo ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			vo_show_help();
			scanf("%d", &op);
			s32Ret = vo_ut_handle_op(op, VoDev);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
		} while (op != 255);
	}

	s32Ret = vo_ut_sys_deinit();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vo_ut_sys_deinit failed\n");
		return CVI_FAILURE;
	}

	return 0;
}
