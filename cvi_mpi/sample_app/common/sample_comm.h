/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample_common.h
 * Description:
 */

#ifndef __SAMPLE_COMM_H__
#define __SAMPLE_COMM_H__

#include <pthread.h>
#include <stdarg.h>

#include "cvi_sys.h"
#include <cvi_common.h>
#include <cvi_comm_vb.h>
#include "cvi_comm_isp.h"
#include "cvi_comm_3a.h"
#include "cvi_buffer.h"
#include <cvi_comm_venc.h>
#include <cvi_defines.h>
#include "cvi_mipi.h"

#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_venc.h"
#include "cvi_bin.h"
#include "md5sum.h"

#include "cvi_audio.h"
#include "cvi_vdec.h"
#include "cvi_gdc.h"
#include "cvi_region.h"
#include "cvi_vo.h"
#include "cvi_isp.h"
#include "cvi_sensor.h"
#include "cvi_type.h"
#include <cvi_comm_sys.h>
#include <cvi_comm_vo.h>
#include <cvi_comm_vb.h>
#include <cvi_comm_adec.h>
#include <cvi_comm_aenc.h>
#include <cvi_comm_aio.h>
#include "sensor_cfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern SENSOR_CFG_S gstSensorCfg;
#define FILE_NAME_LEN 128
#define MAX_NUM_INSTANCE 4
#define NUM_OF_USER_DATA_BUF 4

#define CHECK_CHN_RET(express, Chn, name)                                                                              \
	do {                                                                                                           \
		CVI_S32 Ret;                                                                                           \
		Ret = express;                                                                                         \
		if (Ret != CVI_SUCCESS) {                                                                              \
			printf("\033[0;31m%s chn %d failed at %s: LINE: %d with %#x!\033[0;39m\n", name, Chn,          \
			       __func__, __LINE__, Ret);                                                               \
			fflush(stdout);                                                                                \
			return Ret;                                                                                    \
		}                                                                                                      \
	} while (0)

#define CHECK_RET(express, name)                                                                                       \
	do {                                                                                                           \
		CVI_S32 Ret;                                                                                           \
		Ret = express;                                                                                         \
		if (Ret != CVI_SUCCESS) {                                                                              \
			printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __func__,            \
			       __LINE__, Ret);                                                                         \
			return Ret;                                                                                    \
		}                                                                                                      \
	} while (0)

#define SAMPLE_PIXEL_FORMAT VI_PIXEL_FORMAT

#define COLOR_RGB_RED RGB_8BIT(0xFF, 0, 0)
#define COLOR_RGB_GREEN RGB_8BIT(0, 0xFF, 0)
#define COLOR_RGB_BLUE RGB_8BIT(0, 0, 0xFF)
#define COLOR_RGB_BLACK RGB_8BIT(0, 0, 0)
#define COLOR_RGB_YELLOW RGB_8BIT(0xFF, 0xFF, 0)
#define COLOR_RGB_CYN RGB_8BIT(0, 0xFF, 0xFF)
#define COLOR_RGB_WHITE RGB_8BIT(0xFF, 0xFF, 0xFF)

#define COLOR_10_RGB_RED RGB(0x3FF, 0, 0)
#define COLOR_10_RGB_GREEN RGB(0, 0x3FF, 0)
#define COLOR_10_RGB_BLUE RGB(0, 0, 0x3FF)
#define COLOR_10_RGB_BLACK RGB(0, 0, 0)
#define COLOR_10_RGB_YELLOW RGB(0x3FF, 0x3FF, 0)
#define COLOR_10_RGB_CYN RGB(0, 0x3FF, 0x3FF)
#define COLOR_10_RGB_WHITE RGB(0x3FF, 0x3FF, 0x3FF)

#define SAMPLE_VO_DEV_DHD0 0 /* VO's device HD0 */
#define SAMPLE_VO_DEV_DHD1 1 /* VO's device HD1 */
#define SAMPLE_VO_DEV_UHD SAMPLE_VO_DEV_DHD0 /* VO's ultra HD device:HD0 */
#define SAMPLE_VO_DEV_HD SAMPLE_VO_DEV_DHD1 /* VO's HD device:HD1 */
#define SAMPLE_VO_LAYER_VHD0 0
#define SAMPLE_VO_LAYER_VHD1 1
#define SAMPLE_VO_LAYER_VHD2 2
#define SAMPLE_VO_LAYER_PIP SAMPLE_VO_LAYER_VHD2

#define SAMPLE_AUDIO_EXTERN_AI_DEV 0
#define SAMPLE_AUDIO_EXTERN_AO_DEV 0
#define SAMPLE_AUDIO_INNER_AI_DEV 0
#define SAMPLE_AUDIO_INNER_AO_DEV 0
#define SAMPLE_AUDIO_INNER_HDMI_AO_DEV 1
#define SAMPLE_AUDIO_PTNUMPERFRM 480

#define WDR_MAX_PIPE_NUM 4 //need checking by jammy

#define __FILENAM__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define MAX_STRING_LEN 255

#define PAUSE()                                                                \
	do {                                                                       \
		printf("---------------press Enter key to exit!---------------\n");    \
		getchar();                                                             \
	} while (0)

static inline void sample_prt_with_newline(const char *func, int line, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("[%s]-%d: ", func, line);
	vprintf(fmt, args);
	va_end(args);

	size_t len = strlen(fmt);
	if (len > 0 && fmt[len - 1] != '\n') {
		printf("\n");
	}
}

#define SAMPLE_PRT(fmt, ...) sample_prt_with_newline(__func__, __LINE__, fmt, ##__VA_ARGS__)

#define CHECK_NULL_PTR(ptr)                                                    \
	do {                                                                       \
		if (ptr == NULL) {                                                     \
			printf("func:%s,line:%d, NULL pointer\n", __func__, __LINE__);     \
			return CVI_FAILURE;                                                \
		}                                                                      \
	} while (0)

#define ALIGN_BASE(val, base)	(((val) + ((base)-1)) & ~((base)-1))

/*******************************************************
 *  enum define
 *******************************************************/
/*gpio*/
typedef enum CVI_GPIO_NUM_E_ {
CVI_GPIO00 = 480,
CVI_GPIO01,    CVI_GPIO02,    CVI_GPIO03,    CVI_GPIO04,    CVI_GPIO05,
CVI_GPIO06,    CVI_GPIO07,    CVI_GPIO08,    CVI_GPIO09,    CVI_GPIO10,
CVI_GPIO11,    CVI_GPIO12,    CVI_GPIO13,    CVI_GPIO14,    CVI_GPIO15,
CVI_GPIO16,    CVI_GPIO17,    CVI_GPIO18,    CVI_GPIO19,    CVI_GPIO20,
CVI_GPIO21,    CVI_GPIO22,    CVI_GPIO23,    CVI_GPIO24,    CVI_GPIO25,
CVI_GPIO26,    CVI_GPIO27,    CVI_GPIO28,    CVI_GPIO29,    CVI_GPIO30,
CVI_GPIO31,
CVI_GPIO32 = 448,
CVI_GPIO33,    CVI_GPIO34,    CVI_GPIO35,    CVI_GPIO36,    CVI_GPIO37,
CVI_GPIO38,    CVI_GPIO39,    CVI_GPIO40,    CVI_GPIO41,    CVI_GPIO42,
CVI_GPIO43,    CVI_GPIO44,    CVI_GPIO45,    CVI_GPIO46,    CVI_GPIO47,
CVI_GPIO48,    CVI_GPIO49,    CVI_GPIO50,    CVI_GPIO51,    CVI_GPIO52,
CVI_GPIO53,    CVI_GPIO54,    CVI_GPIO55,    CVI_GPIO56,    CVI_GPIO57,
CVI_GPIO58,    CVI_GPIO59,    CVI_GPIO60,    CVI_GPIO61,    CVI_GPIO62,
CVI_GPIO63,
CVI_GPIO64 = 416,
CVI_GPIO65,    CVI_GPIO66,    CVI_GPIO67,    CVI_GPIO68,    CVI_GPIO69,
CVI_GPIO70,    CVI_GPIO71,    CVI_GPIO72,    CVI_GPIO73,    CVI_GPIO74,
CVI_GPIO75,    CVI_GPIO76,    CVI_GPIO77,    CVI_GPIO78,    CVI_GPIO79,
CVI_GPIO80,    CVI_GPIO81,    CVI_GPIO82,    CVI_GPIO83,    CVI_GPIO84,
CVI_GPIO85,    CVI_GPIO86,    CVI_GPIO87,    CVI_GPIO88,    CVI_GPIO89,
CVI_GPIO90,    CVI_GPIO91,    CVI_GPIO92,    CVI_GPIO93,    CVI_GPIO94,
CVI_GPIO95,
CVI_GPIO96 = 384,
CVI_GPIO97,    CVI_GPIO98,    CVI_GPIO99,    CVI_GPIO100,   CVI_GPIO101,
CVI_GPIO102,   CVI_GPIO103,   CVI_GPIO104,   CVI_GPIO105,   CVI_GPIO106,
CVI_GPIO107,   CVI_GPIO108,   CVI_GPIO109,   CVI_GPIO110,   CVI_GPIO111,
CVI_GPIO112,   CVI_GPIO113,   CVI_GPIO114,   CVI_GPIO115,   CVI_GPIO116,
CVI_GPIO117,   CVI_GPIO118,   CVI_GPIO119,   CVI_GPIO120,   CVI_GPIO121,
CVI_GPIO122,   CVI_GPIO123,   CVI_GPIO124,   CVI_GPIO125,   CVI_GPIO126,
CVI_GPIO127,
CVI_GPIO128 = 352,
CVI_GPIO129,   CVI_GPIO130,   CVI_GPIO131,   CVI_GPIO132,   CVI_GPIO133,
CVI_GPIO134,   CVI_GPIO135,   CVI_GPIO136,   CVI_GPIO137,   CVI_GPIO138,
CVI_GPIO139,   CVI_GPIO140,   CVI_GPIO141,   CVI_GPIO142,   CVI_GPIO143,
CVI_GPIO144,   CVI_GPIO145,   CVI_GPIO146,   CVI_GPIO147,   CVI_GPIO148,
CVI_GPIO149,   CVI_GPIO150,   CVI_GPIO151,   CVI_GPIO152,   CVI_GPIO153,
CVI_GPIO154,   CVI_GPIO155,   CVI_GPIO156,   CVI_GPIO157,   CVI_GPIO158,
CVI_GPIO159,
CVI_GPIO160 = 320,
CVI_GPIO161,   CVI_GPIO162,   CVI_GPIO163,   CVI_GPIO164,   CVI_GPIO165,
CVI_GPIO166,   CVI_GPIO167,   CVI_GPIO168,   CVI_GPIO169,   CVI_GPIO170,
CVI_GPIO171,   CVI_GPIO172,   CVI_GPIO173,   CVI_GPIO174,   CVI_GPIO175,
CVI_GPIO176,   CVI_GPIO177,   CVI_GPIO178,   CVI_GPIO179,   CVI_GPIO180,
CVI_GPIO181,   CVI_GPIO182,   CVI_GPIO183,   CVI_GPIO184,   CVI_GPIO185,
CVI_GPIO186,   CVI_GPIO187,   CVI_GPIO188,   CVI_GPIO189,   CVI_GPIO190,
CVI_GPIO191,
CVI_PWR_GPIO00 = 288,
CVI_PWR_GPIO01,  CVI_PWR_GPIO02,  CVI_PWR_GPIO03,  CVI_PWR_GPIO04,  CVI_PWR_GPIO05,
CVI_PWR_GPIO06,  CVI_PWR_GPIO07,  CVI_PWR_GPIO08,  CVI_PWR_GPIO09,  CVI_PWR_GPIO10,
CVI_PWR_GPIO11,  CVI_PWR_GPIO12,  CVI_PWR_GPIO13,  CVI_PWR_GPIO14,  CVI_PWR_GPIO15,
CVI_PWR_GPIO16,  CVI_PWR_GPIO17,  CVI_PWR_GPIO18,  CVI_PWR_GPIO19,  CVI_PWR_GPIO20,
CVI_PWR_GPIO21,  CVI_PWR_GPIO22,  CVI_PWR_GPIO23,  CVI_PWR_GPIO24,  CVI_PWR_GPIO25,
CVI_PWR_GPIO26,  CVI_PWR_GPIO27,  CVI_PWR_GPIO28,  CVI_PWR_GPIO29,  CVI_PWR_GPIO30,
CVI_PWR_GPIO31,
} CVI_GPIO_NUM_E;

#define CVI_GPIO_MIN CVI_PWR_GPIO00
#define CVI_GPIO_MAX CVI_GPIO31


#define GPIO_IN_RANGE(gpio) ((gpio >= CVI_GPIO_MIN) && (gpio <= CVI_GPIO_MAX))

typedef enum _PIC_SIZE_E {
	PIC_CIF,
	PIC_D1_PAL, /* 720 * 576 */
	PIC_D1_NTSC, /* 720 * 480 */
	PIC_720P, /* 1280 * 720  */
	PIC_1280x800, /* 1280 * 800 */
	PIC_1280x960, /* 1280 * 962 */
	PIC_1600x1200, /* 1600 * 1200 */
	PIC_1080P, /* 1920 * 1080 */
	PIC_1088, /* 1920 * 1088 */
	PIC_1440P, /* 2560 * 1440 */
	PIC_2304x1296,
	PIC_2048x1536,
	PIC_2048x2048,
	PIC_2560x1600,
	PIC_2560x1944,
	PIC_2592x1520,
	PIC_2592x1536,
	PIC_2592x1944,
	PIC_2560x2160,
	PIC_2688x1520,
	PIC_2688x1944,
	PIC_2716x1524,
	PIC_2880x1620,
	PIC_3200x1800,
	PIC_3844x1124,
	PIC_3840x2160,
	PIC_4096x2160,
	PIC_3000x3000,
	PIC_4000x3000,
	PIC_4032x3000,
	PIC_4032x2288,
	PIC_3840x8640,
	PIC_4608x4320,
	PIC_5120x3840,
	PIC_7688x1124,
	PIC_7680x4320,
	PIC_8192x4320,
	PIC_640x480,
	PIC_479P, /* 632 * 479 */
	PIC_400x400,
	PIC_288P, /* 384 * 288 */
	PIC_CUSTOMIZE,
	PIC_BUTT
} PIC_SIZE_E;
typedef struct _SAMPLE_SENSOR_MCLK_ATTR_S {
	CVI_U8 u8Mclk;
	CVI_BOOL bMclkEn;
} SAMPLE_SENSOR_MCLK_ATTR_S;

typedef struct _SAMPLE_SENSOR_INFO_S {
	CVI_SNS_TYPE_E enSnsType;
	CVI_S32 s32SnsId;
	CVI_S32 s32ModeId;
	CVI_S32 s32BusId;
	CVI_S32 s32SnsI2cAddr;
	// TODO: HGJ comb_dev_t
	unsigned int MipiDev;
	CVI_S16 as16LaneId[MIPI_LANE_NUM + 1];
	CVI_S16 as16FuncId[TTL_PIN_FUNC_NUM];
	CVI_S8  as8PNSwap[MIPI_LANE_NUM + 1];
	CVI_U8  u8HwSync;
	SAMPLE_SENSOR_MCLK_ATTR_S stMclkAttr;
	CVI_U8 u8Orien;	// 0: normal, 1: mirror, 2: flip, 3: mirror and flip.
	CVI_U8 u8Hsettle; // 0: 0-16
	CVI_BOOL bHsettlen;
	CVI_S32 s32RstPin;
	CVI_S32 s32RstActive;
} SAMPLE_SENSOR_INFO_S;

typedef struct _SAMPLE_DEV_INFO_S {
	VI_DEV ViDev;
	CVI_S32 mipiDev;
	CVI_S32	rstport;
	CVI_S32	rstpin;
	CVI_S32	rstpol;
	WDR_MODE_E enWDRMode;
	CVI_S32 fps;
	SIZE_S stSize;
	CVI_BOOL bPatgen;
	BAYER_FORMAT_E enBayerFormat;
	SNS_YUV_DATA_SEQ_E enYuvFormat;
	SNS_DATA_TYPE_E enFormatMode;
	SNS_INTF_MODE_E enInterFaceMode;
	SNS_CHN_MODE_E enChnMode;
} SAMPLE_DEV_INFO_S;

typedef struct _SAMPLE_PIPE_INFO_S {
	VI_PIPE aPipe[VI_MAX_PIPE_NUM];
	VI_VPSS_MODE_E enMastPipeMode;
	bool bMultiPipe;
	bool bVcNumCfged;
	bool bIspBypass;
	PIXEL_FORMAT_E enPixFmt;
	COMPRESS_MODE_E enCompressMode;
	CVI_U32 u32VCNum[VI_MAX_PHY_PIPE_NUM];
} SAMPLE_PIPE_INFO_S;

typedef struct _SAMPLE_CHN_INFO_S {
	VI_CHN ViChn;
	PIXEL_FORMAT_E enPixFormat;
	DYNAMIC_RANGE_E enDynamicRange;
	VIDEO_FORMAT_E enVideoFormat;
	COMPRESS_MODE_E enCompressMode;
} SAMPLE_CHN_INFO_S;

typedef struct _SAMPLE_VI_INFO_S {
	SAMPLE_SENSOR_INFO_S stSnsInfo;
	SAMPLE_DEV_INFO_S stDevInfo;
	SAMPLE_PIPE_INFO_S stPipeInfo;
	SAMPLE_CHN_INFO_S stChnInfo;
} SAMPLE_VI_INFO_S;

typedef struct _SAMPLE_VI_CONFIG_S {
	SNS_CFG_S stSnsCfg;
	SAMPLE_VI_INFO_S astViInfo[VI_MAX_DEV_NUM];
	CVI_S32 as32WorkingViId[VI_MAX_DEV_NUM];
	CVI_S32 s32ViNum;
} SAMPLE_VI_CONFIG_S;

typedef enum _SAMPLE_VO_MODE_E {
	VO_MODE_1MUX,
	VO_MODE_2MUX,
	VO_MODE_4MUX,
	VO_MODE_8MUX,
	VO_MODE_9MUX,
	VO_MODE_16MUX,
	VO_MODE_25MUX,
	VO_MODE_36MUX,
	VO_MODE_49MUX,
	VO_MODE_64MUX,
	VO_MODE_2X4,
	VO_MODE_BUTT
} SAMPLE_VO_MODE_E;

typedef struct _SAMPLE_VO_CONFIG_S {
	/* for device */
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr;

	/* for layer */
	PIXEL_FORMAT_E enPixFormat;
	RECT_S stDispRect;
	SIZE_S stImageSize;

	CVI_U32 u32DisBufLen;

	/* for channel */
	SAMPLE_VO_MODE_E enVoMode;
} SAMPLE_VO_CONFIG_S;

typedef enum _SAMPLE_RC_E {
	SAMPLE_RC_CBR = 0,
	SAMPLE_RC_VBR,
	SAMPLE_RC_AVBR,
	SAMPLE_RC_QVBR,
	SAMPLE_RC_FIXQP,
	SAMPLE_RC_QPMAP,
	SAMPLE_RC_UBR,
	SAMPLE_RC_MAX
} SAMPLE_RC_E;

typedef enum _QMAP_MODE_E {
	MAP_MODE_NONE = 0,
	MAP_MODE_QPMAP = 1,
	MAP_MODE_MOTIONMAP = 1 << 1,
	MAP_MODE_AIMAP = 1 << 2,
}QMAP_MODE_E;

typedef enum _THREAD_CONTRL_E {
	THREAD_CTRL_START,
	THREAD_CTRL_PAUSE,
	THREAD_CTRL_STOP,
} THREAD_CONTRL_E;

typedef struct _VDEC_THREAD_PARAM_S {
	CVI_S32 s32ChnId;
	PAYLOAD_TYPE_E enType;
	CVI_CHAR inFilePath[128];
	CVI_CHAR inFileName[128];
	CVI_CHAR outFilePath[128];
	CVI_CHAR outFileName[128];
	CVI_S32 s32StreamMode;
	CVI_S32 s32MilliSec_in;
	CVI_S32 s32MilliSec_out;
	CVI_S32 s32MinBufSize;
	CVI_S32 s32IntervalTime;
	THREAD_CONTRL_E eThreadCtrl;
	CVI_U64 u64PtsInit;
	CVI_U64 u64PtsIncrease;
	CVI_BOOL bCircleSend;
	CVI_BOOL bFileEnd;
	CVI_BOOL bDumpYUV;
	MD5_CTX tMD5Ctx;
	FILE *pDumpFile;
} VDEC_THREAD_PARAM_S;

typedef struct _SAMPLE_VDEC_BUF {
	CVI_U32  u32PicBufSize;
	CVI_U32  u32TmvBufSize;
	CVI_BOOL bPicBufAlloc;
	CVI_BOOL bTmvBufAlloc;
} SAMPLE_VDEC_BUF;


typedef struct _SAMPLE_VDEC_VIDEO_ATTR {
	VIDEO_DEC_MODE_E enDecMode;
	CVI_U32              u32RefFrameNum;
	DATA_BITWIDTH_E  enBitWidth;
} SAMPLE_VDEC_VIDEO_ATTR;

typedef struct _SAMPLE_VDEC_PICTURE_ATTR {
	CVI_U32         u32Alpha;
} SAMPLE_VDEC_PICTURE_ATTR;

typedef struct _SAMPLE_VDEC_ATTR {
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	VIDEO_MODE_E   enMode;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U32 u32FrameBufCnt;
	CVI_U32 u32DisplayFrameNum;
	union {
		SAMPLE_VDEC_VIDEO_ATTR stSampleVdecVideo;      /* structure with video ( h265/h264) */
		SAMPLE_VDEC_PICTURE_ATTR stSampleVdecPicture; /* structure with picture (jpeg/mjpeg )*/
	};
} SAMPLE_VDEC_ATTR;

typedef struct _vdecChnCtx_ {
	VDEC_THREAD_PARAM_S stVdecThreadParamSend;
	SAMPLE_VDEC_ATTR stSampleVdecAttr;
	pthread_t vdecThreadSend;
	pthread_t vdecThreadGet;
	VDEC_CHN VdecChn;
	CVI_S32 bCreateChn;
} vdecChnCtx;

typedef struct SAMPLE_VENC_GETSTREAM_PARA_S {
	CVI_BOOL bThreadStart;
	VENC_CHN VeChn[VENC_MAX_CHN_NUM];
	CVI_S32  s32Cnt;
} SAMPLE_VENC_GETSTREAM_PARA_S;

typedef struct _commonInputCfg_ {
	CVI_U32 testMode;
	CVI_S32 numChn;
	CVI_S32 perf;
	CVI_S32 ifInitVb;
	CVI_U32 bindmode;
	CVI_U32 u32ViWidth;		// frame width of VI input or VPSS input
	CVI_U32 u32ViHeight;	// frame height of VI input or VPSS input
	CVI_U32 u32VpssWidth;	// frame width of VPSS output
	CVI_U32 u32VpssHeight;	// frame height of VPSS output
	CVI_CHAR yuvFolder[MAX_STRING_LEN];
	CVI_S32 vbMode;
	CVI_S32 h265RefreshType;
	CVI_S32 jpegMarkerOrder;
	CVI_BOOL bThreadDisable;
} commonInputCfg;

typedef struct _chnInputCfg_ {
	char codec[64];
	unsigned int width;
	unsigned int height;
	char input_path[MAX_STRING_LEN];
	char vpssSrcPath[MAX_STRING_LEN];
	char output_path[MAX_STRING_LEN];
	char outputFileName[MAX_STRING_LEN];
	char roiCfgFile[MAX_STRING_LEN];
	char motionMapBinFile[MAX_STRING_LEN];
	char aiMapBinFile[MAX_STRING_LEN];
	int roideltaqp;
	char qpMapCfgFile[MAX_STRING_LEN];
	char jpegQTableCfgFile[MAX_STRING_LEN];
	char user_data[NUM_OF_USER_DATA_BUF][MAX_STRING_LEN];
	CVI_S32 num_frames;
	CVI_S32 bsMode;
	CVI_U32 u32Profile;
	CVI_S32 rcMode;
	CVI_S32 iqp;
	CVI_S32 pqp;
	CVI_S32 gop;
	CVI_U32 gopMode;
	CVI_S32 bitrate;
	CVI_S32 minIprop;
	CVI_S32 maxIprop;
	CVI_U32 u32RowQpDelta;
	CVI_S32 firstFrmstartQp;
	CVI_S32 minIqp;
	CVI_S32 maxIqp;
	CVI_S32 minQp;
	CVI_S32 maxQp;
	CVI_S32 framerate;
	CVI_S32 quality;
	CVI_S32 maxbitrate;
	CVI_S32 s32ChangePos;
	CVI_S32 s32MinStillPercent;
	CVI_U32 u32MaxStillQP;
	CVI_U32 u32MotionSensitivity;
	CVI_S32	s32AvbrFrmLostOpen;
	CVI_S32 s32AvbrFrmGap;
	CVI_S32 s32AvbrPureStillThr;
	CVI_S32 statTime;
	CVI_S32 bind_mode;
	CVI_S32 pixel_format;
	CVI_S32 posX;
	CVI_S32 posY;
	CVI_S32 inWidth;
	CVI_S32 inHeight;
	CVI_S32 srcFramerate;
	CVI_U32 bitstreamBufSize;
	CVI_S32 single_LumaBuf;
	CVI_S32 single_core;
	CVI_S32 vpssGrp;
	CVI_S32 vpssChn;
	CVI_S32 forceIdr;
	CVI_S32 chgNum;
	CVI_S32 chgBitrate;
	CVI_S32 chgFramerate;
	CVI_S32 tempLayer;
	CVI_S32 testRoi;
	CVI_S32 bgInterval;
	CVI_S32 frameLost;
	CVI_U32 frameLostGap;
	CVI_U32 frameLostBspThr;
	CVI_S32 MCUPerECS;
	CVI_S32 bCreateChn;
	CVI_S32 getstream_timeout;
	CVI_S32 sendframe_timeout;
	CVI_S32 s32IPQpDelta;
	CVI_S32 s32BgQpDelta;
	CVI_S32 s32ViQpDelta;
	CVI_S32 bVariFpsEn;
	CVI_S32 initialDelay;
	CVI_U32 u32IntraCost;
	CVI_U32 u32ThrdLv;
	CVI_BOOL bBgEnhanceEn;
	CVI_S32 s32BgDeltaQp;
	CVI_U32 h264EntropyMode;
	CVI_S32 h264ChromaQpOffset;
	CVI_S32 h265CbQpOffset;
	CVI_S32 h265CrQpOffset;
	CVI_U32 enSuperFrmMode;
	CVI_U32 u32SuperIFrmBitsThr;
	CVI_U32 u32SuperPFrmBitsThr;
	CVI_S32 s32MaxReEncodeTimes;

	CVI_U8 aspectRatioInfoPresentFlag;
	CVI_U8 aspectRatioIdc;
	CVI_U8 overscanInfoPresentFlag;
	CVI_U8 overscanAppropriateFlag;
	CVI_U16 sarWidth;
	CVI_U16 sarHeight;

	CVI_U8 timingInfoPresentFlag;
	CVI_U8 fixedFrameRateFlag;
	CVI_U32 numUnitsInTick;
	CVI_U32 timeScale;

	CVI_U8 videoSignalTypePresentFlag;
	CVI_U8 videoFormat;
	CVI_U8 videoFullRangeFlag;
	CVI_U8 colourDescriptionPresentFlag;
	CVI_U8 colourPrimaries;
	CVI_U8 transferCharacteristics;
	CVI_U8 matrixCoefficients;

	CVI_U32 u32FrameQp;
	CVI_BOOL bTestUbrEn;

	CVI_BOOL bEsBufQueueEn;
	CVI_BOOL bIsoSendFrmEn;
	CVI_BOOL bSensorEn;

	CVI_U32 u32SliceCnt;
	CVI_U8 bDisableDeblk;
	CVI_S32 betaOffset;
	CVI_S32 alphaOffset;
	CVI_U32 u32SaoEnable;
	CVI_BOOL bIntraPred;
	CVI_BOOL bSetPredUnit;
	CVI_BOOL bSmoothingEnable;
	FILE *motionMapFile;
	FILE *aiMapFile;

	CVI_BOOL svc_enable;
	CVI_BOOL fg_protect_en;
	CVI_S32  fg_dealt_qp;
	CVI_BOOL complex_scene_detect_en;
	CVI_U32  complex_scene_low_th;
	CVI_U32  complex_scene_hight_th;
	CVI_U32  middle_min_percent;
	CVI_U32  complex_min_percent;
	CVI_BOOL smart_ai_en;
	CVI_S8   obj_tab[64];
	CVI_U32  obj_size;
	CVI_U32  dqp_tab[128];
	CVI_U32  bRcnRefShare;
	CVI_BOOL bDebreathEn;
	CVI_U32  s32Strength0;
	CVI_U32  s32Strength1;

	CVI_U32 u32ResetGop;
} chnInputCfg;

typedef enum _CHN_STATE_ {
	CHN_STAT_NONE = 0,
	CHN_STAT_START,
	CHN_STAT_STOP,
} CHN_STATE;

typedef enum _BS_MODE_ {
	BS_MODE_QUERY_STAT = 0,
	BS_MODE_SELECT,
} BS_MODE;

typedef struct _SAMPLE_COMM_VENC_ROI_ATTR_ {
	VENC_ROI_ATTR_S stVencRoiAttr;
	CVI_U32 u32FrameStart;
	CVI_U32 u32FrameEnd;
} SAMPLE_COMM_VENC_ROI;

#define MAX_NUM_ROI 8

typedef struct _frame_buffer_param_
{
	CVI_U32 u32PoolId;
	CVI_U64 u64PhyAddr;
	int iUseFlag;
} frame_buffer_param;
#define MAX_SRC_FRAM_CNT    32

typedef struct _vencChnCtx_ {
	VENC_CHN VencChn;
	PIC_SIZE_E enSize;
	SIZE_S stSize;
	VIDEO_FRAME_INFO_S *pstFrameInfo;
	VIDEO_FRAME_S *pstVFrame;
	CVI_U32 u32LumaSize;
	CVI_U32 u32ChrmSize;
	CVI_U32 u32FrameSize;
	CVI_U32 num_frames;
	CVI_S32 s32ChnNum;
	CVI_U32 s32FbCnt;
	CVI_U32 u32Profile;
	PAYLOAD_TYPE_E enPayLoad;
	VENC_GOP_MODE_E enGopMode;
	VENC_GOP_ATTR_S stGopAttr;
	SAMPLE_RC_E enRcMode;
	FILE *fpSrc;
	long file_size;
	FILE *pFile;
	chnInputCfg chnIc;
	PIXEL_FORMAT_E enPixelFormat;
	PIXEL_FORMAT_E enPixelFormatIn;
	CHN_STATE chnStat;
	CHN_STATE nextChnStat;
	SAMPLE_COMM_VENC_ROI vencRoi[MAX_NUM_ROI];
	CVI_U8 *pu8QpMap;
	CVI_U32 QpMapSize;
	VB_POOL QpMapPool;
	VB_BLK QpMapBlk;
	CVI_U8 *pu8AiMap;
	VB_BLK AiMapBlk;
	CVI_U32 AiMapSize;
	CVI_U8 QpMapMode;
	CVI_S32 s32VencFd;
	CVI_BOOL bCircleSend;
	CVI_U32 u32BlkSize;
	frame_buffer_param frameUnusedQueue[MAX_SRC_FRAM_CNT];
	CVI_S32 perf;
	//VENC_PHYS_BUF_S stEncBistreamBuf;
} vencChnCtx;

/********************************************************
 *     function announce
 ********************************************************/
// isp part start
#define ENABLE_AF_LIB (0)
//The customer can deisgin the control of the motor by themself,
//and sophgo also provides the public practice for customer reference
//please contact sophgo get motor ko source code
//design it please flow cb func format
//cb sample start
CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusInCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusOutCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomSpeedCb(VI_PIPE ViPipe, CVI_U8 speed);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusSpeedCb(VI_PIPE ViPipe, CVI_U8 speed);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomInCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomOutCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomAndFocusInternalCb(VI_PIPE ViPipe, AF_DIRECTION eDir, CVI_U8 zoomStep, CVI_U8 focusStep);
CVI_S32 SAMPLE_COMM_ISP_Motor_GetLensInfoCb(VI_PIPE ViPipe, ISP_AF_LEN_INFO_S *info);
//cb sample end
CVI_S32 SAMPLE_COMM_VI_CreateIsp(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_DestroyIsp(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_ISP_Run(CVI_U8 IspDev);
CVI_VOID SAMPLE_COMM_ISP_Stop(CVI_U8 IspDev);
CVI_VOID SAMPLE_COMM_All_ISP_Stop(void);
CVI_S32 SAMPLE_COMM_ISP_Aelib_Callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Aelib_UnCallback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Awblib_Callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Sensor_Regiter_callback(ISP_DEV IspDev, CVI_U32 u32SnsId, CVI_S32 s32BusId,
						CVI_S32 s32I2cAddr);
CVI_S32 SAMPLE_COMM_ISP_Awblib_UnCallback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Aflib_Callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Aflib_UnCallback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_Sensor_UnRegiter_callback(ISP_DEV IspDev);
CVI_S32 SAMPLE_COMM_ISP_GetIspPubAttr(ISP_DEV IspDev, SAMPLE_VI_CONFIG_S *pstViConfig, ISP_PUB_ATTR_S *pstPubAttr);
CVI_S32 SAMPLE_COMM_ISP_SetSensorMode(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_ISP_SetSnsObj(CVI_U32 u32SnsId, CVI_SNS_TYPE_E enSnsType);
CVI_S32 SAMPLE_COMM_ISP_SetSnsInit(CVI_U32 u32SnsId, CVI_U8 u8HwSync);
CVI_S32 SAMPLE_COMM_ISP_PatchSnsObj(CVI_U32 u32SnsId, SAMPLE_SENSOR_INFO_S *pstSnsInfo);

CVI_S32 SAMPLE_COMM_BIN_ReadParaFrombin(void);
CVI_S32 SAMPLE_COMM_BIN_ReadBlockParaFrombin(enum CVI_BIN_SECTION_ID id);
// isp part end
CVI_S32 SAMPLE_COMM_VI_GetSizeBySensor(CVI_SNS_TYPE_E enMode, PIC_SIZE_E *penSize);
CVI_S32 SAMPLE_COMM_VI_GetDevAttrBySns(CVI_SNS_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr);

CVI_VOID *SAMPLE_SYS_IOMmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size);
CVI_S32 SAMPLE_SYS_Munmap(CVI_VOID *pVirAddr, CVI_U32 u32Size);
CVI_S32 SAMPLE_SYS_SetReg(CVI_U64 u64Addr, CVI_U32 u32Value);
CVI_S32 SAMPLE_SYS_GetReg(CVI_U64 u64Addr, CVI_U32 *pu32Value);

CVI_S32 SAMPLE_COMM_SYS_GetPicSize(PIC_SIZE_E enPicSize, SIZE_S *pstSize);
CVI_S32 SAMPLE_COMM_SYS_MemConfig(void);
CVI_VOID SAMPLE_COMM_SYS_Exit(void);
CVI_S32 SAMPLE_COMM_SYS_Init(VB_CONFIG_S *pstVbConfig);
CVI_S32 SAMPLE_COMM_SYS_InitWithVbSupplement(VB_CONFIG_S *pstVbConf, CVI_U32 u32SupplementConfig);

CVI_S32 SAMPLE_COMM_VI_Bind_VO(VI_PIPE ViPipe, VI_CHN ViChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VI_UnBind_VO(VI_PIPE ViPipe, VI_CHN ViChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VI_Bind_VPSS(VI_PIPE ViPipe, VI_CHN ViChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VI_UnBind_VPSS(VI_PIPE ViPipe, VI_CHN ViChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VI_Bind_VENC(VI_PIPE ViPipe, VI_CHN ViChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VI_UnBind_VENC(VI_PIPE ViPipe, VI_CHN ViChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VPSS_Bind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VPSS_UnBind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VPSS_Bind_VENC(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VPSS_UnBind_VENC(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VDEC_Bind_VPSS(VDEC_CHN VdecChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VDEC_UnBind_VPSS(VDEC_CHN VdecChn, VPSS_GRP VpssGrp);
CVI_S32 SAMPLE_COMM_VPSS_Bind_VPSS(VPSS_GRP VpssGrpSrc, VPSS_CHN VpssChnSrc, VPSS_GRP VpssGrpDst);
CVI_S32 SAMPLE_COMM_VPSS_UnBind_VPSS(VPSS_GRP VpssGrpSrc, VPSS_CHN VpssChnSrc, VPSS_GRP VpssGrpDst);
CVI_S32 SAMPLE_COMM_VDEC_Bind_VENC(VDEC_CHN VdecChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VDEC_UnBind_VENC(VDEC_CHN VdecChn, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VDEC_Bind_VO(VDEC_CHN VdecChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 SAMPLE_COMM_VDEC_UnBind_VO(VDEC_CHN VdecChn, VO_LAYER VoLayer, VO_CHN VoChn);

CVI_S32 SAMPLE_COMM_INI_SensorCfg(SENSOR_CFG_S *p_sns_cfg);
CVI_S32 SAMPLE_COMM_SnsIni2Vicfg(SNS_INI_CFG_S *pstIniCfg, SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_INI_INIT(SAMPLE_VI_CONFIG_S *pstViConfig, SNS_INI_CFG_S *pstSnsIniCfg, VB_CONFIG_S *pstVbConfig);
CVI_S32 SAMPLE_COMM_VI_ResetSensor(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_ResetMipi(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_UnresetSensor(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_UnresetMipi(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_SetMipiAttr(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_EnableSensorClock(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_StartMIPI(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_StartDev(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StopDev(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StartPipe(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StopPipe(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StartChn(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_StopChn(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_VI_DestroyVi(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_StartSensor(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_SensorProbe(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_GetYuvBypassSts(CVI_SNS_TYPE_E enSnsType);
CVI_S32 SAMPLE_COMM_VI_StartViChn(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_COMM_VI_StopViChn(SAMPLE_VI_INFO_S *pstViInfo);
CVI_S32 SAMPLE_COMM_GetDevnumBySnsmode(CVI_SNS_TYPE_E enSnsType);

CVI_S32 SAMPLE_COMM_VPSS_INIT(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr);
CVI_S32 SAMPLE_COMM_VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr);
CVI_S32 SAMPLE_COMM_VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable);

CVI_S32 SAMPLE_COMM_VO_GetWH(VO_INTF_SYNC_E enIntfSync, CVI_U32 *pu32W, CVI_U32 *pu32H, CVI_U32 *pu32Frm);
CVI_S32 SAMPLE_COMM_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);
CVI_S32 SAMPLE_COMM_VO_StopDev(VO_DEV VoDev);
CVI_S32 SAMPLE_COMM_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);
CVI_S32 SAMPLE_COMM_VO_StopLayer(VO_LAYER VoLayer);
CVI_S32 SAMPLE_COMM_VO_StartChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode);
CVI_S32 SAMPLE_COMM_VO_StopChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode);
CVI_S32 SAMPLE_COMM_VO_GetDefConfig(SAMPLE_VO_CONFIG_S *pstVoConfig);
CVI_S32 SAMPLE_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig);
CVI_S32 SAMPLE_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig);

CVI_VOID SAMPLE_COMM_VENC_InitCommonInputCfg(commonInputCfg *pCic);
CVI_VOID SAMPLE_COMM_VENC_InitChnInputCfg(chnInputCfg *pIc);
CVI_S32 SAMPLE_COMM_VENC_SaveStream(PAYLOAD_TYPE_E enType,
		FILE *pFd, VENC_STREAM_S *pstStream);
CVI_S32 SAMPLE_COMM_VENC_SaveChannelStream(vencChnCtx *pvecc);
CVI_S32 SAMPLE_COMM_VENC_Stop(VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_GetGopAttr(VENC_GOP_MODE_E enGopMode, VENC_GOP_ATTR_S *pstGopAttr);
CVI_S32 SAMPLE_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, char *szFilePostfix);
CVI_S32 SAMPLE_COMM_VENC_InitVBPool(vencChnCtx *pvecc, VENC_CHN VencChnIdx);
CVI_S32 SAMPLE_COMM_VENC_CloseReEncode(VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetJpegParam(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetMjpegParam(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetModParam(const commonInputCfg *pCic);
CVI_S32 SAMPLE_COMM_VENC_SetRoiAttr(VENC_CHN VencChn, PAYLOAD_TYPE_E enType);
CVI_S32 SAMPLE_COMM_VENC_SetQpMapByCfgFile(VENC_CHN VencChn,
		SAMPLE_COMM_VENC_ROI *vencRoi, CVI_U32 frameIdx,
		CVI_U8 *pu8QpMap, CVI_BOOL *pbQpMapValid,
		CVI_U32 u32Width, CVI_U32 u32Height,
		PAYLOAD_TYPE_E enPayLoad, CVI_U32 *u32QpMapSize);
CVI_S32 SAMPLE_COMM_VENC_SetRoiAttrByCfgFile(VENC_CHN VencChn, SAMPLE_COMM_VENC_ROI *vencRoi, CVI_U32 frameIdx);
CVI_S32 SAMPLE_COMM_VENC_LoadRoiCfgFile(SAMPLE_COMM_VENC_ROI *vencRoi, CVI_CHAR *cfgFileName);
CVI_S32 SAMPLE_COMM_VENC_SetH264Trans(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265Trans(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH264Vui(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265Vui(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH264Dblk(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265Dblk(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH264IntraPred(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetH265PredUnit(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetChnParam(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_EnableSvc(chnInputCfg *pIc, VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_SetSvcParam(chnInputCfg *pIc,VENC_CHN VencChn);
CVI_S32 SAMPLE_COMM_VENC_Start(
		chnInputCfg * pIc,
		VENC_CHN VencChn,
		PAYLOAD_TYPE_E enType,
		PIC_SIZE_E enSize,
		SAMPLE_RC_E enRcMode,
		CVI_U32 u32Profile,
		CVI_BOOL bRcnRefShareBuf,
		VENC_GOP_ATTR_S *pstGopAttr);

CVI_S32 SAMPLE_COMM_VDEC_InitVBPool(VDEC_CHN VdecChn, SAMPLE_VDEC_ATTR *pastSampleVdec);
CVI_VOID SAMPLE_COMM_VDEC_StartSendStream(VDEC_THREAD_PARAM_S *pstVdecSend,
		pthread_t *pVdecThread);
CVI_S32 SAMPLE_COMM_VDEC_SetVBPool(CVI_S32 Chn, CVI_U32 VBPoolID);
CVI_S32 SAMPLE_COMM_VDEC_GetVBPool(CVI_S32 Chn);
CVI_S32 SAMPLE_COMM_VDEC_SetVbMode(CVI_S32 VdecVbSrc);
CVI_S32 SAMPLE_COMM_VDEC_GetVbMode(void);
CVI_S32 SAMPLE_COMM_VDEC_Start(vdecChnCtx *pvdchnCtx);
CVI_VOID SAMPLE_COMM_VDEC_CmdCtrl(VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread);
CVI_VOID SAMPLE_COMM_VDEC_StopSendStream(VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread);
CVI_S32 SAMPLE_COMM_VDEC_Stop(CVI_S32 s32ChnNum);
CVI_VOID SAMPLE_COMM_VDEC_ExitVBPool(void);
CVI_VOID SAMPLE_COMM_VDEC_StartGetPic(VDEC_THREAD_PARAM_S *pstVdecGet,
		pthread_t *pVdecThread);

CVI_S32 SAMPLE_PLAT_SYS_INIT(SIZE_S stSize);
CVI_S32 SAMPLE_PLAT_VI_INIT(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 SAMPLE_PLAT_VPSS_INIT(VPSS_GRP VpssGrp, SIZE_S stSizeIn, SIZE_S stSizeOut);
CVI_S32 SAMPLE_PLAT_VO_INIT(VO_DEV VoDev);
CVI_S32 SAMPLE_PLAT_VO_INIT_BT656(VO_DEV VoDev);

/* SAMPLE_COMM_REGION_Create:
 *   Create region.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * [in] pixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_Create(CVI_S32 HandleNum, RGN_TYPE_E enType, PIXEL_FORMAT_E pixelFormat);

/* SAMPLE_COMM_REGION_Destroy:
 *   Destroy region.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_Destroy(CVI_S32 HandleNum, RGN_TYPE_E enType);

/* SAMPLE_COMM_REGION_AttachToChn:
 *   Region apply to chn.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_AttachToChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

/* SAMPLE_COMM_REGION_DetachFrmChn:
 *   Cancel region on chn.
 *
 * [in] HandleNum: Number of region handle.
 * [in] enType: Type of region.
 * [in] pstChn: Module chn.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_DetachFrmChn(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn);

/* SAMPLE_COMM_REGION_SetBitMap:
 *   Set bitmap.
 *
 * [in] Handle: RGN ID.
 * [in] filename: Bitmap filename.
 * [in] pixelFormat: Pixel format.
 * [in] bCompressed: Compression or not.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_SetBitMap(RGN_HANDLE Handle, const char *filename,
		PIXEL_FORMAT_E pixelFormat, CVI_BOOL bCompressed);

/* SAMPLE_COMM_REGION_GetUpCanvas:
 *   Get canvas info and update canvas.
 *
 * [in] Handle: RGN ID.
 * [in] filename: Bitmap filename.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_GetUpCanvas(RGN_HANDLE Handle, const char *filename);

/* SAMPLE_COMM_REGION_GetMinHandle:
 *   Get the minimum handle for a region of this type.
 *
 * [in] enType: Type of region.
 * return: MinHandle if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_GetMinHandle(RGN_TYPE_E enType);

/* SAMPLE_COMM_REGION_MST_LoadBmp:
 *   Load the bitmap file.
 *
 * [in] filename: Bitmap filename.
 * [in] pstBitmap: Bitmap attribute.
 * [in] bFil: Filter or not.
 * [in] u16FilColor: Filter color.
 * [in] pixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_MST_LoadBmp(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
			CVI_U32 u16FilColor, PIXEL_FORMAT_E enPixelFormat);

/* SAMPLE_COMM_REGION_GetOverlayPalette:
 *   Get overlay_palette.
 * return: Pointer to the overlay_palette.
 */
RGN_RGBQUARD_S *SAMPLE_COMM_REGION_GetOverlayPalette(void);

/* SAMPLE_COMM_REGION_MST_UpdateCanvas:
 *   Update canvas.
 *
 * [in] filename: Bitmap filename.
 * [in] pstBitmap: Bitmap attribute.
 * [in] bFil: Filter or not.
 * [in] u16FilColor: Filter color.
 * [in] pstSize: Canvas size.
 * [in] u32Stride: Stride length.
 * [in] pixelFormat: Pixel format.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_REGION_MST_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, CVI_BOOL bFil,
				CVI_U32 u16FilColor, SIZE_S *pstSize, CVI_U32 u32Stride, PIXEL_FORMAT_E enPixelFormat);

/* SAMPLE_COMM_FRAME_SaveToFile:
 *   Save videoframe to the file
 *
 * [in]filename: char string of the file to save data.
 * [IN]pstVideoFrame: the videoframe whose data will be saved to file.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_FRAME_SaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* SAMPLE_COMM_PrepareFrame:
 *   Prepare videoframe per size & format.
 *
 * [in]stSize: the size of videoframe
 * [in]enPixelFormat: pixel format of videoframes
 * [Out]pstVideoFrame: the videoframe generated.
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_PrepareFrame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* SAMPLE_COMM_FRAME_CompareWithFile:
 *   Compare data with frame.
 *
 * [in]filename: file to read.
 * [in]pstVideoFrame: the video-frame to store data from file.
 * return: True if match; False if mismatch.
 */
CVI_BOOL SAMPLE_COMM_FRAME_CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

/* SAMPLE_COMM_FRAME_LoadFromFile:
 *   Load data to frame, whose data loaded from given filename.
 *
 * [in]filename: file to read.
 * [in]pstVideoFrame: the video-frame to store data from file.
 * [in]stSize: size of image.
 * [in]enPixelFormat: format of image
 * return: CVI_SUCCESS if no problem.
 */
CVI_S32 SAMPLE_COMM_FRAME_LoadFromFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame,
	SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat);


int SAMPLE_COMM_GPIO_Export(unsigned int gpio);
int SAMPLE_COMM_GPIO_Unexport(unsigned int gpio);
int SAMPLE_COMM_GPIO_SetDirection(unsigned int gpio, unsigned int out_flag);
int SAMPLE_COMM_GPIO_SetValue(unsigned int gpio, unsigned int value);
int SAMPLE_COMM_GPIO_GetValue(unsigned int gpio, unsigned int *value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
