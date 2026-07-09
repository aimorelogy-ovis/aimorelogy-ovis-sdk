
#include <unistd.h>
#include <dlfcn.h>

#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "video_src_cfg_replay"

#include "sample_comm.h"
#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "video_src_cfg.h"
#include "isp_cfg.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_af_comm.h"

//#include "raw_replay_offline.h"

#define OFFLINE_REPLAY_SO "libraw_replay_offline.so"

#define GET_SO_FUNC(p_func, func_name) \
{\
	if (!p_func) {\
		char *error = NULL;\
		void *so_handle;\
		dlerror();\
		so_handle = dlopen(OFFLINE_REPLAY_SO, RTLD_LAZY);\
		error = dlerror();\
		if (error != NULL) {\
			clog_e("dlopen %s error : %s", OFFLINE_REPLAY_SO, error);\
		}\
		if (so_handle == NULL) {\
			return CVI_FAILURE;\
		}\
		dlerror();\
		p_func = dlsym(so_handle, func_name);\
		error = dlerror();\
		if (error != NULL) {\
			p_func = NULL;\
			dlclose(so_handle);\
			return CVI_FAILURE;\
		}\
	}\
}

static CVI_S32 (*raw_replay_offline_init)(char *);
static CVI_S32 (*start_raw_replay_offline)(char *);

static VI_DEV_ATTR_S DEV_ATTR_SENSOR_DEFAULT = {
	VI_MODE_MIPI,
	VI_WORK_MODE_1Multiplex,
	VI_SCAN_PROGRESSIVE,
	{ -1, -1, -1, -1 },
	VI_DATA_SEQ_YUYV,
	{
		VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,
		VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,
		{ 0, 1920, 0, 0, 1080, 0, 0, 0, 0 }
	},
	VI_DATA_TYPE_RGB,
	{ 1920, 1080 },
	{ WDR_MODE_NONE, 1080 },
	.enBayerFormat = BAYER_FORMAT_BG,
};

static VI_CHN_ATTR_S CHN_ATTR_DEFAULT = {
	{ 1920, 1080 },
	PIXEL_FORMAT_YUV_PLANAR_420,
	DYNAMIC_RANGE_SDR8,
	VIDEO_FORMAT_LINEAR,
	COMPRESS_MODE_NONE,
	CVI_FALSE, CVI_FALSE,
	0,
	{ -1, -1 },
	0,
	0
};

static CVI_S32 replay_vi_start_dev(raw_replay_cfg_t *cfg)
{
	CVI_S32 s32Ret;
	VI_DEV_ATTR_S stViDevAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;

	memcpy(&stViDevAttr, &DEV_ATTR_SENSOR_DEFAULT, sizeof(VI_DEV_ATTR_S));

	if (!cfg->pixel_format) {
		stViDevAttr.stSize.u32Width = cfg->width;
		stViDevAttr.stSize.u32Height = cfg->height;
		stViDevAttr.enInputDataType = VI_DATA_TYPE_RGB;
		stViDevAttr.enBayerFormat = (BAYER_FORMAT_E)cfg->bayer_format;
		if (cfg->wdr_mode) {
			stViDevAttr.stWDRAttr.enWDRMode = WDR_MODE_2To1_LINE;
			stViDevAttr.stWDRAttr.u32CacheLine = cfg->width;
		}
	} else {
		stViDevAttr.stSize.u32Width = cfg->width;
		stViDevAttr.stSize.u32Height = cfg->height;
		stViDevAttr.stWDRAttr.u32CacheLine = cfg->width;
		stViDevAttr.enDataSeq = VI_DATA_SEQ_YUYV;
		stViDevAttr.enInputDataType = VI_DATA_TYPE_YUV;
		stViDevAttr.enIntfMode = VI_MODE_MIPI_YUV422;
	}
	stViDevAttr.snrFps = cfg->frame_rate;

	// TODO: check
	stViDevBindAttr.PipeId[0]		= 0;
	stViDevBindAttr.u32Num			= 1;
	stViDevBindAttr.MipiDev			= 0;

	s32Ret = CVI_VI_SetDevAttr(0, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_SetDevBindAttr(0, &stViDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_EnableDev(0);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 replay_vi_start_pipe(raw_replay_cfg_t *cfg)
{
	CVI_S32 s32Ret = 0;
	VI_PIPE_ATTR_S stPipeAttr = {0};

	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = cfg->width;
	stPipeAttr.u32MaxH = cfg->height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.enCompressMode = (COMPRESS_MODE_E)cfg->compress_mode;
	stPipeAttr.bYuvBypassPath = cfg->pixel_format ? 1 : 0;

	s32Ret = CVI_VI_CreatePipe(0, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_StartPipe(0);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_GetPipeAttr(0, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_GetPipeAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 replay_vi_start_chn(raw_replay_cfg_t *cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	VI_CHN_ATTR_S stChnAttr = {0};

	memcpy(&stChnAttr, &CHN_ATTR_DEFAULT, sizeof(VI_CHN_ATTR_S));
	stChnAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	stChnAttr.stSize.u32Width = cfg->width;
	stChnAttr.stSize.u32Height = cfg->height;
	stChnAttr.enDynamicRange =
		cfg->wdr_mode ? DYNAMIC_RANGE_HDR10 : DYNAMIC_RANGE_SDR10;
	stChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stChnAttr.enCompressMode = (COMPRESS_MODE_E)cfg->compress_mode;
	stChnAttr.u32Depth = 0;
	stChnAttr.u32BindVbPool = -1;
	/* fill the sensor orientation */
	stChnAttr.bMirror = 0;
	stChnAttr.bFlip = 0;

	s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_EnableChn failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 replay_offline_init(raw_replay_cfg_t *cfg)
{
	CVI_S32 s32Ret;

	printf("replay offline init! offline raw path:%s\n",
			cfg->offline_raw_dir);

	if (access(cfg->offline_raw_dir, F_OK) != 0) {
		clog_e("dump raw path: %s not exists!\n", cfg->offline_raw_dir);
		return -1;
	}

	GET_SO_FUNC(raw_replay_offline_init, "raw_replay_offline_init");
	s32Ret = raw_replay_offline_init(cfg->offline_raw_dir);
	if (s32Ret != 0) {
		clog_e("raw replay offline init fail!\n");
		return s32Ret;
	}

	GET_SO_FUNC(start_raw_replay_offline, "start_raw_replay_offline");
	s32Ret = start_raw_replay_offline(0);

	if (s32Ret != 0) {
		clog_e("raw start fail!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 replay_send_usr_pic(raw_replay_cfg_t *cfg)
{
	CVI_S32 s32Ret;
	VI_DEV_TIMING_ATTR_S stTimingAttr;
	VI_PIPE_FRAME_SOURCE_E frameSource = VI_PIPE_FRAME_SOURCE_DEV;

	stTimingAttr.bEnable = cfg->timing_enable;
	stTimingAttr.s32FrmRate = cfg->frame_rate;

	s32Ret = CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_USER_BE);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_SetPipeFrameSource failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_GetPipeFrameSource(0, &frameSource);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_SetPipeFrameSource failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (frameSource != VI_PIPE_FRAME_SOURCE_USER_BE) {
		clog_e("CVI_VI_GetPipeFrameSource failed with %d\n", frameSource);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_SetDevTimingAttr(0, &stTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_SetDevTimingAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	VI_PIPE PipeId[] = { 0 };
	const VIDEO_FRAME_INFO_S *pstVideoFrame[1];
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk_le = VB_INVALID_HANDLE;
	VB_BLK blk_se = VB_INVALID_HANDLE;
	CVI_U32 u32BlkSize = 0;
	CVI_U64 u64PhyAddr_le = 0, u64PhyAddr_se = 0;
	CVI_VOID *puVirAddr_le = NULL, *puVirAddr_se = NULL;

	memset(&stVideoFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

	stVideoFrame.stVFrame.enBayerFormat = (BAYER_FORMAT_E)cfg->bayer_format;
	stVideoFrame.stVFrame.enPixelFormat = (PIXEL_FORMAT_E)cfg->pixel_format;
	stVideoFrame.stVFrame.enCompressMode = (COMPRESS_MODE_E)cfg->compress_mode;
	stVideoFrame.stVFrame.enDynamicRange =
		cfg->wdr_mode ? DYNAMIC_RANGE_HDR10 : DYNAMIC_RANGE_SDR10;
	stVideoFrame.stVFrame.u32Width = cfg->width;
	stVideoFrame.stVFrame.u32Height = cfg->height;
	stVideoFrame.stVFrame.s16OffsetLeft = 0;
	stVideoFrame.stVFrame.s16OffsetTop = 0;
	stVideoFrame.stVFrame.s16OffsetRight = 0;
	stVideoFrame.stVFrame.s16OffsetBottom = 0;

	// 0: bayer, 1: yuv
	if (!stVideoFrame.stVFrame.enPixelFormat) {
		u32BlkSize = VI_GetRawBufferSize(
			stVideoFrame.stVFrame.u32Width,
			stVideoFrame.stVFrame.u32Height,
			PIXEL_FORMAT_RGB_BAYER_12BPP,
			stVideoFrame.stVFrame.enCompressMode, DEFAULT_ALIGN, 0);
	} else {
		stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR10;
		u32BlkSize = COMMON_GetPicBufferSize(
			stVideoFrame.stVFrame.u32Width,
			stVideoFrame.stVFrame.u32Height, PIXEL_FORMAT_YUYV,
			DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	}

	blk_le = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
	if (blk_le == VB_INVALID_HANDLE) {
		clog_e("get blk_le failed\n");
		return CVI_FAILURE;
	}
	u64PhyAddr_le = CVI_VB_Handle2PhysAddr(blk_le);
	puVirAddr_le = CVI_SYS_MmapCache(u64PhyAddr_le, u32BlkSize);
	memset(puVirAddr_le, 0, u32BlkSize);
	CVI_SYS_Munmap(puVirAddr_le, u32BlkSize);

	if (cfg->wdr_mode) {
		blk_se = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		if (blk_se == VB_INVALID_HANDLE) {
			clog_e("get blk_se failed\n");
			return CVI_FAILURE;
		}
		u64PhyAddr_se = CVI_VB_Handle2PhysAddr(blk_se);
		puVirAddr_se =
			CVI_SYS_MmapCache(u64PhyAddr_se, u32BlkSize);
		memset(puVirAddr_se, 0, u32BlkSize);
		CVI_SYS_Munmap(puVirAddr_se, u32BlkSize);
	}

	stVideoFrame.stVFrame.u64PhyAddr[0] = u64PhyAddr_le;
	stVideoFrame.stVFrame.u64PhyAddr[1] = u64PhyAddr_se;
	pstVideoFrame[0] = &stVideoFrame;

	if (CVI_VI_SendPipeRaw(1, PipeId, pstVideoFrame, 0) != CVI_SUCCESS) {
		clog_e("Trig vsync failed\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

int module_video_src_replay_init(uint8_t pipe_id, void *video_src_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)video_src_cfg;
	raw_replay_cfg_t *replay_cfg = &cfg->raw_replay_cfg;

	s32Ret = replay_send_usr_pic(replay_cfg);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("replay_send_usr_pic failed with %#x!\n", s32Ret);
		return -1;
	}

	s32Ret = replay_vi_start_dev(replay_cfg);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("replay_vi_start_dev failed with %#x!\n", s32Ret);
		return -1;
	}

	s32Ret = replay_vi_start_pipe(replay_cfg);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("replay_vi_start_pipe failed with %#x!\n", s32Ret);
		return -1;
	}

	s32Ret = init_isp(pipe_id, video_src_cfg);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("replay init_isp failed with %#x!\n", s32Ret);
		return -1;
	}

	s32Ret = replay_vi_start_chn(replay_cfg);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("replay_vi_start_chn failed with %#x!\n", s32Ret);
		return -1;
	}

	// ofline replay init
	if (replay_cfg->replay_scene == 0) {
		s32Ret = replay_offline_init(replay_cfg);
		if (s32Ret != CVI_SUCCESS) {
			clog_e("replay offline init failed with %#x!\n", s32Ret);
			return -1;
		}
	}

	return s32Ret;
}
