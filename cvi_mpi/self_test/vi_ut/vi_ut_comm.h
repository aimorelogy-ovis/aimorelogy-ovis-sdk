#ifndef _VI_UT_COMM_H_
#define _VI_UT_COMM_H_

#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "cvi_type.h"
#include "cvi_vi.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "cvi_buffer.h"
#include "cvi_type.h"
#include "cvi_comm_sys.h"
#include "sensor_cfg.h"

#include "ut_comm.h"

#define CHIP_TYPE "cv184x_vi"

typedef struct _SNSR_RST_S {
	CVI_S32 s32RstPort;
	CVI_S32 s32RstPin;
	CVI_S32 s32RstPol;
} SNSR_RST_S;

typedef struct _SNSR_HSETTLE_S {
	CVI_BOOL bHsettlen;
	CVI_U8 u8Hsettle;
} SNSR_HSETTLE_S;

typedef struct _DEV_INFO_S {
	VI_DEV ViDev;
	CVI_S32 mipiDev;

	CVI_BOOL bPatgen;
	//switch config
	CVI_BOOL bMuxDev;
	CVI_S32 s32AttchDev;
	CVI_S32 s32SwitchSnsrNum;
	CVI_S32 s32SwitchPort[SWITCH_GPIO_NUM];
	CVI_S32 s32SwitchPin[SWITCH_GPIO_NUM];
	CVI_S32 s32SwitchPol[SWITCH_GPIO_NUM];

	CVI_S32 enSnsMode;
	SNSR_RST_S stRstInfo;
	SNSR_HSETTLE_S stHsettle;

	WDR_MODE_E enWDRMode;
	SIZE_S stSize;
	BAYER_FORMAT_E enBayerFormat;
	VI_YUV_DATA_SEQ_E enYuvFormat;
	VI_DATA_TYPE_E enFormatMode;
	VI_INTF_MODE_E enInterFaceMode;
	VI_WORK_MODE_E enChnMode;
	VI_ISP_YUV_SCENE_E enYuvScene;
} DEV_INFO_S;

typedef struct _PIPE_INFO_S {
	VI_PIPE aPipe[VI_MAX_PIPE_NUM];
	VI_VPSS_MODE_E enMastPipeMode;
	CVI_BOOL bMultiPipe;
	CVI_BOOL bVcNumCfged;
	CVI_BOOL bIspBypass;
	PIXEL_FORMAT_E enPixFmt;
	COMPRESS_MODE_E enCompressMode;
	CVI_U32 u32VCNum[VI_MAX_PIPE_NUM];
} PIPE_INFO_S;

typedef struct _CHN_INFO_S {
	VI_CHN ViChn;
	PIXEL_FORMAT_E enPixFormat;
	DYNAMIC_RANGE_E enDynamicRange;
	VIDEO_FORMAT_E enVideoFormat;
	COMPRESS_MODE_E enCompressMode;
	CVI_U32 u32Depth;
} CHN_INFO_S;

typedef struct _VI_INFO_S {
	DEV_INFO_S stDevInfo;
	PIPE_INFO_S stPipeInfo;
	CHN_INFO_S stChnInfo;
} VI_INFO_S;

typedef struct _VI_CONFIG_S {
	SNS_CFG_S stSnsCfg;
	VI_INFO_S astViInfo[VI_MAX_DEV_NUM];
	CVI_S32 as32WorkingViId[VI_MAX_DEV_NUM];
	CVI_S32 s32ViNum;
} VI_CONFIG_S;

typedef struct _VI_USR_PIC_INFO_S {
	CVI_BOOL isHdrOn;
	CVI_S32 s32FrmRate;
	CVI_CHAR file[2][64];
	VB_POOL poolId;
	VB_BLK usrBlk[2];
	CVI_U64 usrPhyAddr[2];
	CVI_VOID * usrVirAddr[2];

	CVI_U32 u32ImgWidth;
	CVI_U32 u32ImgHeight;
	CVI_U32 bayFormat;
	RECT_S crop;
} VI_USR_PIC_INFO_S;

typedef struct _VI_SMOOTH_INFO_S {
	CVI_U32 u32Dev;
	CVI_U32 u32BlkCnt;
	CVI_U32 u32TotalFrameCnt;
} VI_SMOOTH_INFO_S;

typedef struct _VI_ROTATION_INFO_S {
	VI_PIPE pipe;
	VI_CHN chn;
	CVI_U32 rotation;
} VI_ROTATION_INFO_S;

typedef struct _VI_FLIP_MIRROR_INFO_S {
	VI_PIPE pipe;
	VI_CHN chn;
	CVI_U32 flip;
	CVI_U32 mirror;
} VI_FLIP_MIRROR_INFO_S;

typedef struct _VI_LDC_INFO_S {
	VI_PIPE pipe;
	VI_CHN chn;
	VI_LDC_ATTR_S ldcAttr;
} VI_LDC_INFO_S;

typedef struct _VI_UT_CROP_INFO_S {
	VI_PIPE pipe;
	VI_CHN chn;
	VI_CROP_INFO_S crop;
} VI_UT_CROP_INFO_S;

typedef struct _VI_MULTI_INIT_S {
	pthread_t viThread;
	atomic_bool exitFlag;
	atomic_int snsrId;
	atomic_bool isInit;
} VI_MULTI_INIT_S;

typedef struct _VI_UT_CTX {
	VI_CONFIG_S viConfig;
	VI_VPSS_MODE_E viVpssMode;
	VI_USR_PIC_INFO_S rawReplayInfo;
	VI_SMOOTH_INFO_S smoothInfo;
	VI_UT_CROP_INFO_S stCropInfo;
	VI_ROTATION_INFO_S rotationInfo;
	VI_FLIP_MIRROR_INFO_S flipMirrorInfo;
	VI_LDC_INFO_S ldcInfo;
	VI_MULTI_INIT_S multiInit;

	CVI_BOOL isDpcmOn;
	CVI_BOOL isSkipSensor;
	CVI_BOOL isAutoTest;
	CVI_BOOL isPatgen;
	CVI_BOOL isBindVb;
	CVI_BOOL isMultiInit;
	CVI_BOOL isRawReplay;
	CVI_BOOL isWithIsp;
	CVI_BOOL isOnlineSc;
	CVI_BOOL is_use_isp_raw_replay;
} VI_UT_CTX;

CVI_S32 vi_test(VI_UT_CTX *pUtCtx);
CVI_S32 vi_ut_smooth_rawdump(VI_UT_CTX *pUtCtx, CVI_BOOL isSave2File);
CVI_S32 vi_ut_get_pipe_frame(VI_UT_CTX *pUtCtx, CVI_U8 u8Dev, CVI_BOOL isSave2File);
CVI_S32 vi_ut_get_chn_frame(CVI_U8 u8Pipe, CVI_U8 u8Chn, CVI_BOOL isSave2File);
CVI_S32 vi_ut_auto_test_dump(VI_UT_CTX *pUtCtx, CVI_BOOL isDumpRaw, CVI_BOOL isDumpYuv, CVI_BOOL isSave2File);
CVI_S32 vi_ut_get_vpss_chn_frame(CVI_U8 u8Chn);
CVI_S32 rawreplay_send_usr_pic(VI_UT_CTX *pUtCtx);
CVI_S32 vi_ut_vpss_deinit(VI_UT_CTX *pUtCtx);
CVI_S32 vi_ut_vi_deinit(VI_UT_CTX *pUtCtx);
CVI_VOID vi_ut_sys_exit();
CVI_S32 vi_ut_set_vi_vpss_mode(VI_UT_CTX *pUtCtx);
CVI_S32 vi_ut_ai_isp_test(VI_UT_CTX *pUtCtx);

#endif
