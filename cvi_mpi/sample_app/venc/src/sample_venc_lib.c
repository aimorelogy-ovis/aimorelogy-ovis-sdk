#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/select.h>
#include <assert.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <inttypes.h>
#include <limits.h>
#include <sys/ioctl.h>
#include "cvi_venc.h"
#include "sample_comm.h"
#include "sample_venc_lib.h"

#define MAX_VENC_OPTIONS	136
#define MAX_STRING_LEN		255
#define MAX_FILENAME_LEN	64

#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation=" /* Or  "-Wformat-overflow"  */
#endif

#define max(a, b)\
		({__typeof__(a) _a = (a);\
		__typeof__(b) _b = (b);\
		(_a) > (_b) ? (_a) : (_b); })

typedef enum _ARG_TYPE_ {
	ARG_INT = 0,
	ARG_UINT,
	ARG_STRING,
} ARG_TYPE;

typedef struct _optionExt_ {
	struct option opt;
	int type;
	int64_t min;
	int64_t max;
	const char *help;
} optionExt;

typedef union {
	CVI_S32 ival;
	CVI_U32 uval;
} SAMPLE_ARG;

static optionExt g_vencLongOptionExt[] = {
	{{"codec",     optional_argument, NULL, 'c'}, ARG_STRING, 0,   0,
		"265 = h.265, jpg = jpeg, mjp = motion jpeg" },
	{{"width",     optional_argument, NULL, 'w'}, ARG_UINT,    352, 3840,
		"width"},
	{{"height",    optional_argument, NULL, 'h'}, ARG_UINT,    352, 3840,
		"height"},
	{{"input",     optional_argument, NULL, 'i'}, ARG_STRING, 0,   0,
		"source yuv file"},
	{{"output",    optional_argument, NULL, 'o'}, ARG_STRING, 0,   0,
		"output bitstream"},
	{{"frame_num", optional_argument, NULL, 'n'}, ARG_UINT,    0,   1000000000,
		"number of frame to be encode"},
	{{"getBsMode", optional_argument, NULL, 0},   ARG_UINT,    0,   1,
		"get-bitstream mode, 0 = query status, 1 = select"},
	{{"profile",   optional_argument, NULL, 0},   ARG_INT,
		CVI_H264_PROFILE_MIN,   CVI_H264_PROFILE_MAX,
		"profile, 0 = h264 baseline, 1 = h264 main, 2 = h264 high, Default = 2"},
	{{"rcMode",    optional_argument, NULL, 0},   ARG_INT,    0,   6,
		"rate control mode, 0 = CBR, 1 = VBR, 2 = AVBR, 4 = FIXQP, 5 = QPMAP, 6 = UBR (User BR), default = 4"},
	{{"iqp",       optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"I frame QP"},
	{{"pqp",       optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"P frame QP"},
	{{"ipQpDelta",       optional_argument, NULL, 0},   ARG_INT,
		CVI_H26X_NORMALP_IP_QP_DELTA_MIN,   CVI_H26X_NORMALP_IP_QP_DELTA_MAX,
		"QP Delta between P frame and I frame"},
	{{"bgQpDelta", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SMARTP_BG_QP_DELTA_MIN, CVI_H26X_SMARTP_BG_QP_DELTA_MAX,
		"Smart-P QP delta between P frame and BG (background) frame. [-10, 30], default = 0"},
	{{"viQpDelta", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SMARTP_VI_QP_DELTA_MIN, CVI_H26X_SMARTP_VI_QP_DELTA_MAX,
		"Smart-P QP delta between P frame and VI (virtual I) frame. [-10, 30], default = 0"},
	{{"gop",       optional_argument, NULL, 0},   ARG_INT,
		CVI_H26X_GOP_MIN, CVI_H26X_GOP_MAX,
		"The period of one gop"},
	{{"gopMode",   optional_argument, NULL, 0},   ARG_UINT,
		CVI_H26X_GOP_MODE_MIN, CVI_H26X_GOP_MODE_MAX,
		"GOP mode. 0: Normal P, 2: Smart P, Default: 0"},
	{{"bitrate",   optional_argument, NULL, 0},   ARG_INT,    1,   1000000,
		"The average target bitrate (kbits)"},
	{{"initQp",    optional_argument, NULL, 0},   ARG_INT,    0,   100,
		"The Start Qp of 1st frame, 63 = default"},
	{{"minQp",     optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Minimum Qp for one frame"},
	{{"maxQp",     optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Maximum Qp for one frame"},
	{{"minIqp",    optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Minimum Qp for I frame"},
	{{"maxIqp",    optional_argument, NULL, 0},   ARG_INT,    0,   51,
		"Maximum Qp for I frame"},
	{{"srcFramerate", optional_argument, NULL, 0},   ARG_INT,	  0,   240,
		"source frame rate"},
	{{"framerate", optional_argument, NULL, 0},   ARG_INT,	  0,   INT_MAX,
		"destination frame rate"},
	{{"vfps", required_argument, NULL, 0},     ARG_INT,	  0,   1,
		"enable variable FPS"},
	{{"quality", required_argument, NULL, 0},     ARG_INT,	  0,   99,
		"jpeg encode quality"},
	{{"maxbitrate", optional_argument, NULL, 0},  ARG_INT,	  0,   1000000,
		"Maximum output bit rate (kbits)"},
	{{"changePos", optional_argument, NULL, 0},  ARG_INT,	  50,   100,
		"Ratio to change Qp"},
	{{"minStillPercent", optional_argument, NULL, 0},  ARG_INT,	  5,   100,
		"Percentage of target bitrate in low motion"},
	{{"maxStillQp", optional_argument, NULL, 0},  ARG_UINT,	  0,   51,
		"Maximum Qp in low motion"},
	{{"motionSense", optional_argument, NULL, 0},  ARG_UINT,	  0,   100,
		"Motion sensitivity"},
	{{"avbrFrmLostOpen", optional_argument, NULL, 0},  ARG_INT,	  0,   100,
		"avbrFrmLostOpen"},
	{{"avbrFrmGap", optional_argument, NULL, 0},  ARG_INT,	  0,   100,
		"avbrFrmGap"},
	{{"avbrPureStillThr", optional_argument, NULL, 0},  ARG_INT,	  0,   100,
		"avbrPureStillThr"},
	{{"bgEnhanceEn", optional_argument, NULL, 0},  ARG_INT,
		CVI_H26X_BG_ENHANCE_EN_MIN,   CVI_H26X_BG_ENHANCE_EN_MAX,
		"Enable background enhancement"},
	{{"bgDeltaQp", optional_argument, NULL, 0},  ARG_INT,
		CVI_H26X_BG_DELTA_QP_MIN,   CVI_H26X_BG_DELTA_QP_MAX,
		"background delta qp"},
	{{"statTime", optional_argument, NULL, 0},    ARG_INT,	  0,   240,
		"statistics time in seconds"},
	{{"getstream-timeout", optional_argument, NULL, 0},    ARG_INT,	  -1,   100000,
		"samele_venc getstream-timeout   -1:block mode, 0:try_once, >0 timeout in ms"},
	{{"sendframe-timeout", optional_argument, NULL, 0},    ARG_INT,	  -1,   100000,
		"samele_venc sendframe-timeout   -1:block mode, 0:try_once, >0 timeout in ms"},
	{{"ifInitVb", optional_argument, NULL, 0},    ARG_INT,	  0,   1,
		"if enable VB pool or not"},
	{{"vbMode", optional_argument, NULL, 0},    ARG_INT,	  0,   3,
		"if enable VB pool mode. 0 = common, 1 = module, 2 = private, 3 = user"},
	{{"yuvFolder", optional_argument, NULL, 0},   ARG_STRING, 0,   256,
		"yuv files folder"},
	{{"bindmode", optional_argument, NULL, 0},    ARG_UINT,	  0,   2,
		"bind mode"},
	{{"pixel_format", optional_argument, NULL, 0}, ARG_INT,	  0,   3,
		"0: 420 planar, 1: 422 planar, 2: NV12, 3: NV21"},
	{{"posX", optional_argument, NULL, 0},        ARG_INT,    0, 3840,
		"x axis of start position, need to be multiple of 16 (used for crop)"},
	{{"posY", optional_argument, NULL, 0},        ARG_INT,    0, 3840,
		"y axis of start position, need to be multiple of 16 (used for crop)"},
	{{"inWidth", optional_argument, NULL, 0},     ARG_INT,    352, 3840,
		"width of input frame (used for crop)"},
	{{"inHeight", optional_argument, NULL, 0},    ARG_INT,    288, 3840,
		"height of input frame (used for crop)"},
	{{"bufSize", optional_argument, NULL, 0}, ARG_UINT,    0,   1000000000,
		"bitstream Buffer size"},
	{{"single_LumaBuf", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use single luma buffer for H264"},
	{{"single_core", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use single core(h264 or h265 only)"},
	{{"forceIdr", optional_argument, NULL, 0}, ARG_INT,	  0,   1000000000,
		"0: disable, > 0: set force idr at number of frame"},
	{{"resetGop", optional_argument, NULL, 0}, ARG_UINT,	  0,   1,
		"0: not reset, 1: reset gop, reset when forceIdr > 0"},
	{{"chgNum", optional_argument, NULL, 0},	  ARG_INT,	  0, 1000000,
		"frame num to change attr"},
	{{"chgBitrate", optional_argument, NULL, 0},  ARG_INT,	  1, 1000000,
		"change bitrate  (kbits)"},
	{{"chgFramerate", optional_argument, NULL, 0},	ARG_INT,	  0, 240,
		"change dstframerate"},
	{{"tempLayer", optional_argument, NULL, 0}, ARG_INT,	  0,   3,
		"tempLayer"},
	{{"roiCfgFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"ROI configuration file"},
	{{"jpegQTableCfgFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"jpeg/mjpeg Quality Table config file"},
	{{"motionMapBinFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"motion Map bin file for smart enc test"},
	{{"roideltaqp", optional_argument, NULL, 0}, ARG_INT, 0, 10,
		"roi delta qp for AI test"},
	{{"qpMapCfgFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"Roi-based qpMap file"},
	{{"bgInterval", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SMARTP_BG_INTERVAL_MIN, CVI_H26X_SMARTP_BG_INTERVAL_MAX,
		"bgInterval"},
	{{"frame_lost", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use frame lost(h264 or h265 only)"},
	{{"frame_lost_gap", optional_argument, NULL, 0}, ARG_UINT,	  0,   65536,
		"The gap between 2 frame_lost frames(h264 or h265 only)"},
	{{"frame_lost_thr", optional_argument, NULL, 0}, ARG_UINT,	  0,   1200000000,
		"frame_lost bsp threshold(h264 or h265 only)"},
	{{"MCUPerECS", required_argument, NULL, 0},   ARG_INT,	  0,   1000000,
		"jpeg encode MCUPerECS"},
	{{"single_EsBuf", optional_argument, NULL, 0}, ARG_INT,	  0,   1,
		"0: disable, 1: use single stream buffer (jpege)"},
	{{"single_EsBuf_264", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"0: disable, 1: use single stream buffer (h264e)"},
	{{"single_EsBuf_265", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"0: disable, 1: use single stream buffer (h265e)"},
	{{"single_EsBufSize", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
		"single stream buffer size (jpege)"},
	{{"single_EsBufSize_264", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
		"single stream buffer size (h264e)"},
	{{"single_EsBufSize_265", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
		"single stream buffer size (h265e)"},
	{{"numChn",    optional_argument, NULL, 0},   ARG_INT,    1,   VENC_MAX_CHN_NUM,
		"number of channels to encode"},
	{{"chn",       optional_argument, NULL, 0},   ARG_UINT,    0,   VENC_MAX_CHN_NUM - 1,
		"set channel-id to configure the following parameters"},
	{{"viWidth",       optional_argument, NULL, 0},   ARG_UINT,    0,   4096,
		"for VI input width"},
	{{"viHeight",       optional_argument, NULL, 0},   ARG_UINT,    0,   2304,
		"for VI input height"},
	{{"vpssWidth",       optional_argument, NULL, 0},   ARG_UINT,    0,   4096,
		"for Vpss output width"},
	{{"vpssHeight",       optional_argument, NULL, 0},   ARG_UINT,    0,   2304,
		"for VPss output height"},
	{{"vpssSrcPath", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"source file path for vpss"},
	{{"user_data1", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 1"},
	{{"user_data2", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 2"},
	{{"user_data3", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 3"},
	{{"user_data4", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"user data binary file 4"},
	{{"h265RefreshType", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"0: IDR, 1: CRA, default = 0"},
	{{"initialDelay", optional_argument, NULL, 0}, ARG_INT,
		CVI_INITIAL_DELAY_MIN, CVI_INITIAL_DELAY_MAX,
		"rc initial delay in ms, default = 1000"},
	{{"jpegMarkerOrder", optional_argument, NULL, 0}, ARG_INT, 0, 2,
		"0: Cvitek, 1: SOI-JFIF-DQT_MERGE-SOF0-DHT_MERGE-DRI, 2: Cvitek w/ JFIF, default = 0"},
	{{"intraCost", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_INTRACOST_MIN, CVI_H26X_INTRACOST_MAX,
		"intraCost, the extra cost of intra mode"},
	{{"thrdLv", optional_argument, NULL, 0},
		ARG_UINT, CVI_H26X_THRDLV_MIN, CVI_H26X_THRDLV_MAX,
		"thrdLv, threhold to control block qp"},
	{{"h264EntropyMode", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H264_ENTROPY_MIN, CVI_H264_ENTROPY_MAX,
		"0: CAVLC, 1: CABAC, default = 1"},
	{{"h264ChromaQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
		"H264 Chroma QP offset [-12, 12], default = 0"},
	{{"h265CbQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
		"H265 Cb QP offset [-12, 12], default = 0"},
	{{"h265CrQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
		"H265 Cr QP offset [-12, 12], default = 0"},
	{{"maxIprop", optional_argument, NULL, 0}, ARG_INT, 1, 100,
		"max I frame bitrate ratio to P frame, default = 100"},
	{{"rowQpDelta", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_ROW_QP_DELTA_MIN, CVI_H26X_ROW_QP_DELTA_MAX,
		"rowQpDelta [0, 10], default = 1"},
	{{"superFrmMode", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SUPER_FRM_MODE_MIN, CVI_H26X_SUPER_FRM_MODE_MAX,
		"superFrmMode, 0 = disable, 3 = encode to IDR, default = 0"},
	{{"superIBitsThr", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SUPER_I_BITS_THR_MIN, CVI_H26X_SUPER_I_BITS_THR_MAX,
		"superIBitsThr [1000, 33554432], default = 4000000"},
	{{"superPBitsThr", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SUPER_P_BITS_THR_MIN, CVI_H26X_SUPER_P_BITS_THR_MAX,
		"superPBitsThr [1000, 33554432], default = 4000000"},
	{{"maxReEnc", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_MAX_RE_ENCODE_MIN, CVI_H26X_MAX_RE_ENCODE_MAX,
		"maxReEnc [0, 3], default = 0"},
	{{"aspectRatioInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_MIN, CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_MAX,
		"aspect ratio info present flag [0, 1], default = 0"},
	{{"aspectRatioIdc", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_ASPECT_RATIO_IDC_MIN, CVI_H26X_ASPECT_RATIO_IDC_MAX,
		"aspect ratio idc [0, 255], default = 1"},
	{{"overscanInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_MIN, CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_MAX,
		"overscan info present flag [0, 1], default = 0"},
	{{"overscanAppropriateFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_MIN, CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_MAX,
		"overscan appropriate flag [0, 1], default = 0"},
	{{"sarWidth", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SAR_WIDTH_MIN, CVI_H26X_SAR_WIDTH_MAX,
		"sar width [0, 65535], default = 1"},
	{{"sarHeight", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_SAR_HEIGHT_MIN, CVI_H26X_SAR_HEIGHT_MAX,
		"sar height [0, 65535], default = 1"},
	{{"timingInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_TIMING_INFO_PRESENT_FLAG_MIN, CVI_H26X_TIMING_INFO_PRESENT_FLAG_MAX,
		"timing info present flag [0, 1], default = 0"},
	{{"fixedFrameRateFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H264_FIXED_FRAME_RATE_FLAG_MIN, CVI_H264_FIXED_FRAME_RATE_FLAG_MAX,
		"fixed frame rate flag [0, 1], default = 0"},
	{{"numUnitsInTick", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_NUM_UNITS_IN_TICK_MIN, CVI_H26X_NUM_UNITS_IN_TICK_MAX,
		"num units in tick [0, 4294967295], default = 1"},
	{{"timeScale", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_TIME_SCALE_MIN, CVI_H26X_TIME_SCALE_MAX,
		"time scale [0, 4294967295], default = 60"},
	{{"videoSignalTypePresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_MIN, CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_MAX,
		"video signal type present flag [0, 1], default = 0"},
	{{"videoFormat", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_VIDEO_FORMAT_MIN, CVI_H264_VIDEO_FORMAT_MAX,
		"video format [0, 7], default = 5"},
	{{"videoFullRangeFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_VIDEO_FULL_RANGE_FLAG_MIN, CVI_H26X_VIDEO_FULL_RANGE_FLAG_MAX,
		"video full range flag [0, 1], default = 0"},
	{{"colourDescriptionPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_MIN, CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_MAX,
		"colour description present flag [0, 1], default = 0"},
	{{"colourPrimaries", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_COLOUR_PRIMARIES_MIN, CVI_H26X_COLOUR_PRIMARIES_MAX,
		"colour primaries [0, 255], default = 2"},
	{{"transferCharacteristics", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_TRANSFER_CHARACTERISTICS_MIN, CVI_H26X_TRANSFER_CHARACTERISTICS_MAX,
		"transfer characteristics [0, 255], default = 2"},
	{{"matrixCoefficients", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_MATRIX_COEFFICIENTS_MIN, CVI_H26X_MATRIX_COEFFICIENTS_MAX,
		"matrix coefficients [0, 255], default = 2"},
	{{"testUbrEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_TEST_UBR_EN_MIN, CVI_H26X_TEST_UBR_EN_MAX,
		"enable to test ubr [0, 1], default = 0"},
	{{"frameQp", optional_argument, NULL, 0}, ARG_UINT,
		CVI_H26X_FRAME_QP_MIN, CVI_H26X_FRAME_QP_MAX,
		"frameQp [0, 51], default = 38"},
	{{"esBufQueueEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_ES_BUFFER_QUEUE_MIN, CVI_H26X_ES_BUFFER_QUEUE_MAX,
		"esBufQueueEn [0, 1], default = 1"},
	{{"isoSendFrmEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_ISO_SEND_FRAME_MIN, CVI_H26X_ISO_SEND_FRAME_MAX,
		"isoSendFrmEn [0, 1], default = 1"},
	{{"sensorEn", optional_argument, NULL, 0}, ARG_INT,
		CVI_H26X_SENSOR_EN_MIN, CVI_H26X_SENSOR_EN_MAX,
		"sensorEn [0, 1], default = 0"},
	{{"sliceSplitCnt", optional_argument, NULL, 0}, ARG_INT, 0, 5,
		"sliceSplitCnt [1, 5], default = 1"},
	{{"disabledblk", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"disabledblk [0, 1], default = 0"},
	{{"betaOffset", optional_argument, NULL, 0}, ARG_INT, -6, 6,
		"betaOffset [-6, 6], default = 0"},
	{{"alphaoffset", optional_argument, NULL, 0}, ARG_INT, -6, 6,
		"alphaoffset [-6, 6], default = 0"},
	{{"intraPred", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"intraPred [0, 1], default = 0"},
	{{"svcEnable", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"svcEnable [0, 1], default = 0"},
	{{"fgProtectEn", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"fgProtectEn [0, 1], default = 0"},
	{{"fgDealtQp", optional_argument, NULL, 0}, ARG_INT, 0, 8,
		"fgDealtQp [0, 8], default = 0"},
	{{"cplxSceneDetectEn", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"cplxSceneDetectEn [0, 1], default = 0"},
	{{"cplxSceneLowTh", optional_argument, NULL, 0}, ARG_INT, 0, 512,
		"cplxSceneLowTh [0, 512], default = 256"},
	{{"cplxSceneHightTh", optional_argument, NULL, 0}, ARG_INT, 0, 512,
		"cplxSceneHightTh [0, 512], default = 318"},
	{{"smartAiEn", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"smartAiEn [0, 1], default = 0"},
	{{"aiMapBinFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
		"ai Map bin file for smart enc test"},
	{{"recRefShare", optional_argument, NULL, 0}, ARG_INT, 0, 1,
		"0: disable, 1: use address remap for save buf"},
	{{NULL, 0, NULL, 0}, ARG_INT, 0, 0, ""}
};
#if 0
static CVI_S32 _SAMPLE_VENC_FRM_testVpssVenc(sampleVenc *psv);

static VB_POOL vpssVB[VENC_MAX_CHN_NUM] = {[0 ...(VENC_MAX_CHN_NUM - 1)] = VB_INVALID_POOLID};
#endif

static CVI_S32 checkArg(CVI_S32 entryIdx, SAMPLE_ARG *pArg);
static CVI_S32 checkInputCfg(chnInputCfg *pIc);
static CVI_U32 SAMPLE_VENC_INIT_CHANNEL(sampleVenc *psv, CVI_U32 chnNum);
static CVI_S32 initSysAndVb(sampleVenc *psv);
static CVI_S32 SAMPLE_VENC_LoadCfgFile(vencChnCtx *pVencContext);
static CVI_S32 SAMPLE_VENC_StartGetStream(vencChnCtx *pVencContext, CVI_S32 s32ChnIdx);
static CVI_S32 getNonBindModeSrcFrame(vencChnCtx *pVencContext,
		VIDEO_FRAME_INFO_S *pstVideoFrame);
static CVI_VOID vencUnbindSource(chnInputCfg *pIc, VENC_CHN VencChn);
static CVI_S32 releaseNonBindModeSrcFrame(vencChnCtx *pVencContext);
static CVI_VOID *SAMPLE_VENC_GetVencStreamProc(CVI_VOID *pArgs);
static CVI_S32 SAMPLE_VENC_SendFrame(vencChnCtx *pVencContext, CVI_U32 i);
static CVI_S32 SAMPLE_VENC_GetStream(vencChnCtx *pVencContext);
static CVI_S32 cviReadSrcFrame(VIDEO_FRAME_S *pstVFrame, FILE *fp, PIXEL_FORMAT_E enPixelFormatIn);
static CVI_VOID exitSysAndVb(CVI_VOID);
static VIDEO_FRAME_INFO_S *allocate_frame(SIZE_S stSize, PIXEL_FORMAT_E pixel_format);
static CVI_S32 free_frame(VIDEO_FRAME_INFO_S *pstVideoFrame);
static CVI_S32 SAMPLE_VENC_SendOneFrame(vencChnCtx *pVencContext);
static CVI_S32 SAMPLE_COMM_VENC_SetUserFrameLevelRc(chnInputCfg *pIc, VENC_CHN VencChn);
static CVI_VOID SAMPLE_VENC_InsertUserData(VENC_CHN chn, chnInputCfg *pIc);

static CVI_S8 aiObjTab[64] = {
	-1, -1, -1, -1, -1, 5,   6,   6,   6, -1, -1, -1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0
};

static int smtEncAiTab[8][16] =
{
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0 },
	{  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,   2,   3,   3,   3 },
	{  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,   1,   1,   1,   1 },
	{  0,  0,  0,  0,  0,  0,  0, -1, -1, -1, -1, -1,  -1,  -1,  -1,  -1 },
	{  0,  0, -1, -1, -1, -1, -1, -2, -2, -2, -2, -2,  -2,  -3,  -3,  -3 },
	{  0, -1, -1, -1, -2, -2, -2, -3, -3, -3, -3, -4,  -4,  -4,  -5,  -5 },
	{  0, -1, -1, -2, -2, -3, -3, -4, -4, -4, -5, -5,  -6,  -6,  -7,  -7 },
	{ -1, -1, -2, -3, -3, -4, -4, -5, -6, -6, -7, -8,  -8,  -9,  -9, -10 }
};

static PIC_SIZE_E getEnSize(CVI_U32 u32Width, CVI_U32 u32Height)
{
	if (u32Width == 352 && u32Height == 288)
		return PIC_CIF;
	else if (u32Width == 720 && u32Height == 576)
		return PIC_D1_PAL;
	else if (u32Width == 720 && u32Height == 480)
		return PIC_D1_NTSC;
	else if (u32Width == 1280 && u32Height == 720)
		return PIC_720P;
	else if (u32Width == 1920 && u32Height == 1080)
		return PIC_1080P;
	else if (u32Width == 2592 && u32Height == 1520)
		return PIC_2592x1520;
	else if (u32Width == 2592 && u32Height == 1536)
		return PIC_2592x1536;
	else if (u32Width == 2592 && u32Height == 1944)
		return PIC_2592x1944;
	else if (u32Width == 2716 && u32Height == 1524)
		return PIC_2716x1524;
	else if (u32Width == 3840 && u32Height == 2160)
		return PIC_3840x2160;
	else if (u32Width == 4096 && u32Height == 2160)
		return PIC_4096x2160;
	else if (u32Width == 3000 && u32Height == 3000)
		return PIC_3000x3000;
	else if (u32Width == 4000 && u32Height == 3000)
		return PIC_4000x3000;
	else if (u32Width == 3840 && u32Height == 8640)
		return PIC_3840x8640;
	else if (u32Width == 640 && u32Height == 480)
		return PIC_640x480;
	else
		return PIC_CUSTOMIZE;
}

// Map command line input pixel format to PIXEL_FORMAT_E.
static PIXEL_FORMAT_E vencMapPixelFormat(CVI_S32 pixel_format)
{
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	switch (pixel_format) {
	case 0:
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		break;
	case 1:
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;
		break;
	case 2:
		enPixelFormat = PIXEL_FORMAT_NV12;
		break;
	case 3:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	default:
		//CVI_VENC_WARN("Unknown input pixel format. Assume YUV420P.\n");
		enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		break;
	}

	return enPixelFormat;
}

static CVI_U32 getSrcFrameSizeByPixelFormat(CVI_U32 width, CVI_U32 height, PIXEL_FORMAT_E enPixelFormat)
{
	CVI_U32 size = 0;

	switch (enPixelFormat) {
	case PIXEL_FORMAT_YUV_PLANAR_422:
		size = width * height * 2;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_420:
	case PIXEL_FORMAT_NV12:
	case PIXEL_FORMAT_NV21:
		size = width * height * 3 / 2;
		break;
	default:
		SAMPLE_PRT("Unknown pixel format. Assume YUV420P.\n");
		size = width * height * 3 / 2;
		break;
	}

	return size;
}

void print_help(char * const *argv)
{
	CVI_U32 idx;

	SAMPLE_PRT("// ------------------------------------------------\n");
	SAMPLE_PRT("// %s -c codec -w width -h height -i src.yuv -o enc\n", argv[0]);
	SAMPLE_PRT("EX.\n");
	SAMPLE_PRT("sample_venc -c 265 -w 1920 -h 1080 -i ReadySteadyGo_1920x1080_600.yuv -o enc\n");
	SAMPLE_PRT("// ------------------------------------------------\n");

	for (idx = 0; idx < sizeof(g_vencLongOptionExt) / sizeof(optionExt); idx++) {
		if (g_vencLongOptionExt[idx].opt.name == NULL)
			break;

		SAMPLE_PRT("--%s\n", g_vencLongOptionExt[idx].opt.name);
		SAMPLE_PRT("    %s\n", g_vencLongOptionExt[idx].help);
	}
}

CVI_S32 venc_main(int argc, char **argv)
{
	sampleVenc sv, *psv = &sv;
	commonInputCfg *pcic = &psv->commonIc;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_VENC_INIT_CFG(psv, argc, argv);
	if (s32Ret < 0) {
		SAMPLE_PRT("SAMPLE_VENC_INIT_CFG\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init fail\n");
		return s32Ret;
	}

#if 0
	if (pcic->bindmode == VENC_BIND_VPSS) {
		s32Ret = _SAMPLE_VENC_FRM_testVpssVenc(psv);
		if (s32Ret < 0) {
			SAMPLE_PRT("_SAMPLE_VENC_FRM_testVpssVenc failed\n");
			return s32Ret;
		}
	} else
#endif
	{
		s32Ret = SAMPLE_VENC_START(psv);
		if (s32Ret < 0) {
			SAMPLE_PRT("SAMPLE_VENC_START\n");
			return s32Ret;
		}

		s32Ret = SAMPLE_VENC_STOP(psv);
		if (s32Ret < 0) {
			SAMPLE_PRT("SAMPLE_VENC_STOP\n");
			return s32Ret;
		}
	}

	if (pcic->ifInitVb)
		exitSysAndVb();

	return s32Ret;
}

CVI_S32 SAMPLE_VENC_INIT_CFG(sampleVenc *psv, int argc, char **argv)
{
	chnInputCfg *pIc;
	commonInputCfg *pcic = &psv->commonIc;

	memset(psv, 0, sizeof(*psv));

	for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < VENC_MAX_CHN_NUM; s32ChnIdx++) {
		pIc = &psv->chnCtx[s32ChnIdx].chnIc;
		initInputCfg(pcic, pIc);
	}

	pIc = &psv->chnCtx[0].chnIc;
	if (parseEncArgv(psv, pIc, argc, argv) < 0) {
		SAMPLE_PRT("parseEncArgv\n");
		return -1;
	}

	if (pcic->numChn == 0)
		pcic->numChn = 1;

	if (pcic->numChn < 0) {
		SAMPLE_PRT("vcodec config failed\n");
		return -1;
	}
	for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
		pIc = &psv->chnCtx[s32ChnIdx].chnIc;
		if (checkInputCfg(pIc) < 0) {
			SAMPLE_PRT("checkInput (chn %d) failure\n", s32ChnIdx);
			return -1;
		}
	}
	return 0;
}

CVI_VOID initInputCfg(commonInputCfg *pcic, chnInputCfg *pIc)
{
	SAMPLE_COMM_VENC_InitCommonInputCfg(pcic);
	SAMPLE_COMM_VENC_InitChnInputCfg(pIc);
}

static CVI_S32 handleLongOption(chnInputCfg **ppIc, sampleVenc *psv, const char *optionName, SAMPLE_ARG *parsedArg)
{
	chnInputCfg *pIc = *ppIc;

	if (!strcmp(optionName, "getBsMode")) {
		pIc->bsMode = parsedArg->ival;
		SAMPLE_PRT("bsMode = %d\n", pIc->bsMode);
	} else if (!strcmp(optionName, "profile")) {
		pIc->u32Profile = parsedArg->uval;
	} else if (!strcmp(optionName, "rcMode")) {
		pIc->rcMode = parsedArg->ival;
	} else if (!strcmp(optionName, "iqp")) {
		pIc->iqp = parsedArg->ival;
	} else if (!strcmp(optionName, "pqp")) {
		pIc->pqp = parsedArg->ival;
	} else if (!strcmp(optionName, "ipQpDelta")) {
		pIc->s32IPQpDelta = parsedArg->ival;
	} else if (!strcmp(optionName, "bgQpDelta")) {
		pIc->s32BgQpDelta = parsedArg->ival;
	} else if (!strcmp(optionName, "viQpDelta")) {
		pIc->s32ViQpDelta = parsedArg->ival;
	} else if (!strcmp(optionName, "gop")) {
		pIc->gop = parsedArg->ival;
	} else if (!strcmp(optionName, "gopMode")) {
		pIc->gopMode = parsedArg->uval;
	} else if (!strcmp(optionName, "bitrate")) {
		pIc->bitrate = parsedArg->ival;
		SAMPLE_PRT("bitrate = %d\n", pIc->bitrate);
	} else if (!strcmp(optionName, "initQp")) {
		pIc->firstFrmstartQp = parsedArg->ival;
	} else if (!strcmp(optionName, "minIqp")) {
		pIc->minIqp = parsedArg->ival;
	} else if (!strcmp(optionName, "maxIqp")) {
		pIc->maxIqp = parsedArg->ival;
	} else if (!strcmp(optionName, "minQp")) {
		pIc->minQp = parsedArg->ival;
	} else if (!strcmp(optionName, "maxQp")) {
		pIc->maxQp = parsedArg->ival;
	} else if (!strcmp(optionName, "srcFramerate")) {
		pIc->srcFramerate = parsedArg->ival;
	} else if (!strcmp(optionName, "framerate")) {
		pIc->framerate = parsedArg->ival;
	} else if (!strcmp(optionName, "vfps")) {
		pIc->bVariFpsEn = parsedArg->ival;
	} else if (!strcmp(optionName, "quality")) {
		pIc->quality = parsedArg->ival;
	} else if (!strcmp(optionName, "maxbitrate")) {
		pIc->maxbitrate = parsedArg->ival;
	} else if (!strcmp(optionName, "changePos")) {
		pIc->s32ChangePos = parsedArg->ival;
	} else if (!strcmp(optionName, "minStillPercent")) {
		pIc->s32MinStillPercent = parsedArg->ival;
	} else if (!strcmp(optionName, "maxStillQp")) {
		pIc->u32MaxStillQP = parsedArg->uval;
	} else if (!strcmp(optionName, "motionSense")) {
		pIc->u32MotionSensitivity = parsedArg->uval;
	} else if (!strcmp(optionName, "avbrFrmLostOpen")) {
		pIc->s32AvbrFrmLostOpen = parsedArg->ival;
	} else if (!strcmp(optionName, "avbrFrmGap")) {
		pIc->s32AvbrFrmGap = parsedArg->ival;
	} else if (!strcmp(optionName, "avbrPureStillThr")) {
		pIc->s32AvbrPureStillThr = parsedArg->ival;
	} else if (!strcmp(optionName, "statTime")) {
		pIc->statTime = parsedArg->ival;
	} else if (!strcmp(optionName, "getstream-timeout")) {
		pIc->getstream_timeout = parsedArg->ival;
	} else if (!strcmp(optionName, "sendframe-timeout")) {
		pIc->sendframe_timeout = parsedArg->ival;
	} else if (!strcmp(optionName, "ifInitVb")) {
		psv->commonIc.ifInitVb = parsedArg->ival;
	} else if (!strcmp(optionName, "vbMode")) {
		psv->commonIc.vbMode = parsedArg->ival;
	} else if (!strcmp(optionName, "yuvFolder")) {
		strcpy(psv->commonIc.yuvFolder, optarg);
	} else if (!strcmp(optionName, "bindmode")) {
		psv->commonIc.bindmode = parsedArg->uval;
	} else if (!strcmp(optionName, "pixel_format")) {
		pIc->pixel_format = parsedArg->ival;
	} else if (!strcmp(optionName, "posX")) {
		pIc->posX = parsedArg->ival;
	} else if (!strcmp(optionName, "posY")) {
		pIc->posY = parsedArg->ival;
	} else if (!strcmp(optionName, "inWidth")) {
		pIc->inWidth = parsedArg->ival;
	} else if (!strcmp(optionName, "inHeight")) {
		pIc->inHeight = parsedArg->ival;
	} else if (!strcmp(optionName, "bufSize")) {
		pIc->bitstreamBufSize = parsedArg->uval;
	} else if (!strcmp(optionName, "single_core")) {
		pIc->single_core = parsedArg->ival;
	} else if (!strcmp(optionName, "single_LumaBuf")) {
		pIc->single_LumaBuf = parsedArg->ival;
	} else if (!strcmp(optionName, "forceIdr")) {
		pIc->forceIdr = parsedArg->ival;
	} else if (!strcmp(optionName, "resetGop")) {
		pIc->u32ResetGop = parsedArg->uval;
	} else if (!strcmp(optionName, "chgNum")) {
		pIc->chgNum = parsedArg->ival;
	} else if (!strcmp(optionName, "chgBitrate")) {
		pIc->chgBitrate = parsedArg->ival;
	} else if (!strcmp(optionName, "chgFramerate")) {
		pIc->chgFramerate = parsedArg->ival;
	} else if (!strcmp(optionName, "tempLayer")) {
		pIc->tempLayer = parsedArg->ival;
	} else if (!strcmp(optionName, "roiCfgFile")) {
		strcpy(pIc->roiCfgFile, optarg);
	} else if (!strcmp(optionName, "motionMapBinFile")) {
		strcpy(pIc->motionMapBinFile, optarg);
	} else if (!strcmp(optionName, "aiMapBinFile")) {
		strcpy(pIc->aiMapBinFile, optarg);
	} else if (!strcmp(optionName, "roideltaqp")) {
		pIc->roideltaqp = parsedArg->ival;
	} else if (!strcmp(optionName, "qpMapCfgFile")) {
		strcpy(pIc->qpMapCfgFile, optarg);
	} else if (!strcmp(optionName, "jpegQTableCfgFile")) {
		strcpy(pIc->jpegQTableCfgFile, optarg);
	} else if (!strcmp(optionName, "bgInterval")) {
		pIc->bgInterval = parsedArg->ival;
	} else if (!strcmp(optionName, "frame_lost")) {
		pIc->frameLost = parsedArg->ival;
	} else if (!strcmp(optionName, "frame_lost_gap")) {
		pIc->frameLostGap = parsedArg->uval;
	} else if (!strcmp(optionName, "frame_lost_thr")) {
		pIc->frameLostBspThr = parsedArg->uval;
	} else if (!strcmp(optionName, "MCUPerECS")) {
		pIc->MCUPerECS = parsedArg->ival;
	} else if (!strcmp(optionName, "numChn")) {
		psv->commonIc.numChn = parsedArg->ival;
	} else if (!strcmp(optionName, "chn")) {
		pIc = &psv->chnCtx[parsedArg->uval].chnIc;
		*ppIc = pIc;
	} else if (!strcmp(optionName, "viWidth")) {
		psv->commonIc.u32ViWidth = parsedArg->uval;
	} else if (!strcmp(optionName, "viHeight")) {
		psv->commonIc.u32ViHeight = parsedArg->uval;
	} else if (!strcmp(optionName, "vpssWidth")) {
		psv->commonIc.u32VpssWidth = parsedArg->uval;
	} else if (!strcmp(optionName, "vpssHeight")) {
		psv->commonIc.u32VpssHeight = parsedArg->uval;
	} else if (!strcmp(optionName, "vpssSrcPath")) {
		strcpy(pIc->vpssSrcPath, optarg);
	} else if (!strcmp(optionName, "user_data1")) {
		strcpy(pIc->user_data[0], optarg);
	} else if (!strcmp(optionName, "user_data2")) {
		strcpy(pIc->user_data[1], optarg);
	} else if (!strcmp(optionName, "user_data3")) {
		strcpy(pIc->user_data[2], optarg);
	} else if (!strcmp(optionName, "user_data4")) {
		strcpy(pIc->user_data[3], optarg);
	} else if (!strcmp(optionName, "h265RefreshType")) {
		psv->commonIc.h265RefreshType = parsedArg->ival;
	} else if (!strcmp(optionName, "initialDelay")) {
		pIc->initialDelay = parsedArg->ival;
	} else if (!strcmp(optionName, "jpegMarkerOrder")) {
		psv->commonIc.jpegMarkerOrder = parsedArg->ival;
	} else if (!strcmp(optionName, "intraCost")) {
		pIc->u32IntraCost = parsedArg->uval;
	} else if (!strcmp(optionName, "thrdLv")) {
		pIc->u32ThrdLv = parsedArg->uval;
	} else if (!strcmp(optionName, "bgEnhanceEn")) {
		pIc->bBgEnhanceEn = parsedArg->ival;
	} else if (!strcmp(optionName, "bgDeltaQp")) {
		pIc->s32BgDeltaQp = parsedArg->ival;
	} else if (!strcmp(optionName, "h264EntropyMode")) {
		pIc->h264EntropyMode = parsedArg->uval;
	} else if (!strcmp(optionName, "h264ChromaQpOffset")) {
		pIc->h264ChromaQpOffset = parsedArg->ival;
	} else if (!strcmp(optionName, "h265CbQpOffset")) {
		pIc->h265CbQpOffset = parsedArg->ival;
	} else if (!strcmp(optionName, "h265CrQpOffset")) {
		pIc->h265CrQpOffset = parsedArg->ival;
	} else if (!strcmp(optionName, "maxIprop")) {
		pIc->maxIprop = parsedArg->ival;
	} else if (!strcmp(optionName, "rowQpDelta")) {
		pIc->u32RowQpDelta = parsedArg->uval;
	} else if (!strcmp(optionName, "superFrmMode")) {
		pIc->enSuperFrmMode = parsedArg->uval;
	} else if (!strcmp(optionName, "superIBitsThr")) {
		pIc->u32SuperIFrmBitsThr = parsedArg->uval;
	} else if (!strcmp(optionName, "superPBitsThr")) {
		pIc->u32SuperPFrmBitsThr = parsedArg->uval;
	} else if (!strcmp(optionName, "maxReEnc")) {
		pIc->s32MaxReEncodeTimes = parsedArg->ival;
	} else if (!strcmp(optionName, "aspectRatioInfoPresentFlag")) {
		pIc->aspectRatioInfoPresentFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "aspectRatioIdc")) {
		pIc->aspectRatioIdc = parsedArg->uval;
	} else if (!strcmp(optionName, "overscanInfoPresentFlag")) {
		pIc->overscanInfoPresentFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "overscanAppropriateFlag")) {
		pIc->overscanAppropriateFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "sarWidth")) {
		pIc->sarWidth = parsedArg->uval;
	} else if (!strcmp(optionName, "sarHeight")) {
		pIc->sarHeight = parsedArg->uval;
	} else if (!strcmp(optionName, "timingInfoPresentFlag")) {
		pIc->timingInfoPresentFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "fixedFrameRateFlag")) {
		pIc->fixedFrameRateFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "numUnitsInTick")) {
		pIc->numUnitsInTick = parsedArg->uval;
	} else if (!strcmp(optionName, "timeScale")) {
		pIc->timeScale = parsedArg->uval;
	} else if (!strcmp(optionName, "videoSignalTypePresentFlag")) {
		pIc->videoSignalTypePresentFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "videoFormat")) {
		pIc->videoFormat = parsedArg->uval;
	} else if (!strcmp(optionName, "videoFullRangeFlag")) {
		pIc->videoFullRangeFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "colourDescriptionPresentFlag")) {
		pIc->colourDescriptionPresentFlag = parsedArg->uval;
	} else if (!strcmp(optionName, "colourPrimaries")) {
		pIc->colourPrimaries = parsedArg->uval;
	} else if (!strcmp(optionName, "transferCharacteristics")) {
		pIc->transferCharacteristics = parsedArg->uval;
	} else if (!strcmp(optionName, "matrixCoefficients")) {
		pIc->matrixCoefficients = parsedArg->uval;
	} else if (!strcmp(optionName, "testUbrEn")) {
		pIc->bTestUbrEn = parsedArg->ival;
	} else if (!strcmp(optionName, "frameQp")) {
		pIc->u32FrameQp = parsedArg->uval;
	} else if (!strcmp(optionName, "esBufQueueEn")) {
		pIc->bEsBufQueueEn = parsedArg->uval;
	} else if (!strcmp(optionName, "isoSendFrmEn")) {
		pIc->bIsoSendFrmEn = parsedArg->uval;
	} else if (!strcmp(optionName, "sensorEn")) {
		pIc->bSensorEn = parsedArg->uval;
	} else if (!strcmp(optionName, "sliceSplitCnt")) {
		pIc->u32SliceCnt = parsedArg->uval;
	} else if (!strcmp(optionName, "disabledblk")) {
		pIc->bDisableDeblk = parsedArg->uval;
	} else if (!strcmp(optionName, "betaOffset")) {
		pIc->betaOffset = parsedArg->ival;
	} else if (!strcmp(optionName, "alphaoffset")) {
		pIc->alphaOffset = parsedArg->ival;
	} else if (!strcmp(optionName, "intraPred")) {
		pIc->bIntraPred = parsedArg->uval;
	} else if (!strcmp(optionName, "smoothingEnable")) {
		pIc->bSmoothingEnable = parsedArg->uval;
	} else if (!strcmp(optionName, "svcEnable")) {
		pIc->svc_enable = parsedArg->uval;
	} else if (!strcmp(optionName, "fgProtectEn")) {
		pIc->fg_protect_en = parsedArg->uval;
	} else if (!strcmp(optionName, "fgDealtQp")) {
		pIc->fg_dealt_qp = parsedArg->uval;
	} else if (!strcmp(optionName, "cplxSceneDetectEn")) {
		pIc->complex_scene_detect_en = parsedArg->uval;
	} else if (!strcmp(optionName, "cplxSceneLowTh")) {
		pIc->complex_scene_low_th = parsedArg->uval;
	} else if (!strcmp(optionName, "cplxSceneHightTh")) {
		pIc->complex_scene_hight_th = parsedArg->uval;
	} else if (!strcmp(optionName, "middleMinPercent")) {
		pIc->middle_min_percent = parsedArg->uval;
	} else if (!strcmp(optionName, "cplxMinPercent")) {
		pIc->complex_min_percent = parsedArg->uval;
	} else if (!strcmp(optionName, "smartAiEn")) {
		pIc->smart_ai_en = parsedArg->uval;
		/*following param will change according ai segment change*/
		memcpy(pIc->dqp_tab, smtEncAiTab, sizeof(smtEncAiTab));
		memcpy(pIc->obj_tab, aiObjTab, sizeof(aiObjTab));
		pIc->obj_size = 12;
	}  else if (!strcmp(optionName, "recRefShare")) {
		pIc->bRcnRefShare = parsedArg->uval;
	} else {
		SAMPLE_PRT("Unhandled option: %s\n", optionName);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 parseEncArgv(sampleVenc *psv, chnInputCfg *pIc, CVI_S32 argc, char **argv)
{
	CVI_S32 optionChar, optionIndex, result;
	SAMPLE_ARG parsedArg = {0};
	struct option longOptions[MAX_VENC_OPTIONS + 1];

	// Initialize long options array
	memset((void *)longOptions, 0, sizeof(longOptions));
	for (optionIndex = 0; optionIndex < MAX_VENC_OPTIONS; optionIndex++) {
		if (g_vencLongOptionExt[optionIndex].opt.name == NULL)
			break;
		memcpy(&longOptions[optionIndex], &g_vencLongOptionExt[optionIndex].opt, sizeof(struct option));
	}

	// Parse command-line arguments
	while ((optionChar = getopt_long(argc, argv, "c:w:h:i:o:n:", longOptions, &optionIndex)) != -1) {
		switch (optionChar) {
		case 'c': // Codec
			strncpy(pIc->codec, optarg, sizeof(pIc->codec) - 1);
			break;
		case 'w': // Width
			pIc->width = atoi(optarg);
			break;
		case 'h': // Height
			pIc->height = atoi(optarg);
			break;
		case 'i': // Input file path
			strncpy(pIc->input_path, optarg, sizeof(pIc->input_path) - 1);
			break;
		case 'o': // Output file path
			strncpy(pIc->output_path, optarg, sizeof(pIc->output_path) - 1);
			break;
		case 'n': // Number of frames
			pIc->num_frames = atoi(optarg);
			break;
		case 0: // Handle long options
			result = checkArg(optionIndex, &parsedArg);
			if (result != CVI_SUCCESS) {
				SAMPLE_PRT("Invalid argument for option: %s\n", longOptions[optionIndex].name);
				print_help(argv);
				return result;
			}

			if (handleLongOption(&pIc, psv, longOptions[optionIndex].name, &parsedArg) != CVI_SUCCESS) {
				SAMPLE_PRT("Failed to handle option: %s\n", longOptions[optionIndex].name);
				print_help(argv);
				return CVI_FAILURE;
			}
			break;
		default: // Unknown option
			SAMPLE_PRT("Unknown option: %c\n", optionChar);
			print_help(argv);
			return CVI_FAILURE;
		}
	}

	// Check for unexpected arguments
	if (optind < argc) {
		SAMPLE_PRT("Unexpected arguments detected\n");
		print_help(argv);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}
#if 0
static CVI_S32 _PLAT_VPSS_INIT(VPSS_GRP VpssGrp,
	SIZE_S stSizeIn, SIZE_S stSizeOut, CVI_BOOL bWrapEn, CVI_U32 VpssChnNum)
{
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	CVI_BOOL           abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (VpssChnNum < 1 || VpssChnNum > 2)
		return CVI_FAILURE;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat                  = SAMPLE_PIXEL_FORMAT;
	stVpssGrpAttr.u32MaxW                        = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSizeIn.u32Height;
	stVpssGrpAttr.u8VpssDev                      = 0;

	astVpssChnAttr[VpssChn].u32Width                    = stSizeOut.u32Width;
	astVpssChnAttr[VpssChn].u32Height                   = stSizeOut.u32Height;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = SAMPLE_PIXEL_FORMAT;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 1;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;

	if (VpssChnNum == 2) {
		VpssChn = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width					= stSizeOut.u32Width;
		astVpssChnAttr[VpssChn].u32Height					= stSizeOut.u32Height;
		astVpssChnAttr[VpssChn].enVideoFormat				= VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat				= SAMPLE_PIXEL_FORMAT;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth					= 1;
		astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
		astVpssChnAttr[VpssChn].bFlip						= CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode		= ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor	= COLOR_RGB_BLACK;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		abChnEnable[1] = CVI_TRUE;
	}

	if (bWrapEn == CVI_TRUE) {
		// VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;

		// stVpssChnBufWrap.bEnable = CVI_TRUE;
		// stVpssChnBufWrap.u32BufLine = 64;
		// stVpssChnBufWrap.u32WrapBufferSize = 3;

		// s32Ret = SAMPLE_COMM_VPSS_WRAP_Start(VpssGrp,
		// 		abChnEnable, &stVpssGrpAttr, astVpssChnAttr, &stVpssChnBufWrap);
		// if (s32Ret != CVI_SUCCESS) {
		// 	SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		// 	goto error;
		// }
	} else {
		s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp,
				abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			goto error;
		}

		s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp,
				abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			goto error;
		}
	}

	return s32Ret;
error:
	//	_SAMPLE_PLAT_ERR_Exit();
	SAMPLE_PRT("_SAMPLE_PLAT_ERR_Exit: 0x%x !\n", s32Ret);
	return s32Ret;
}

static CVI_S32 _initVpss(SIZE_S *pstSizeIn, SIZE_S *pstSizeOut,
		VB_CAL_CONFIG_S *pstVbCalConfig,
	PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_BOOL bWrapEn,
	CVI_U32 VpssChnNum)
{
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	VB_BLK blk;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = _PLAT_VPSS_INIT(0, *pstSizeIn, *pstSizeOut, bWrapEn, VpssChnNum);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	COMMON_GetPicBufferConfig(pstSizeIn->u32Width, pstSizeIn->u32Height, enPixelFormat,
			DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN, pstVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVFrame->enCompressMode = COMPRESS_MODE_NONE;
	pstVFrame->enPixelFormat = enPixelFormat;
	pstVFrame->enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVFrame->enColorGamut = COLOR_GAMUT_BT709;
	pstVFrame->u32Width = pstSizeIn->u32Width;
	pstVFrame->u32Height = pstSizeIn->u32Height;
	pstVFrame->u32Stride[0] = pstVbCalConfig->u32MainStride;
	pstVFrame->u32Stride[1] = pstVbCalConfig->u32CStride;
	pstVFrame->u32Stride[2] = pstVbCalConfig->u32CStride;
	pstVFrame->u32TimeRef = 0;
	pstVFrame->u64PTS = 0;
	pstVFrame->enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, pstVbCalConfig->u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_PRT("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVFrame->u32Length[0] = pstVbCalConfig->u32MainYSize;
	pstVFrame->u32Length[1] = pstVbCalConfig->u32MainCSize;
	pstVFrame->u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVFrame->u64PhyAddr[1] = pstVFrame->u64PhyAddr[0]
		+ ALIGN(pstVbCalConfig->u32MainYSize, pstVbCalConfig->u16AddrAlign);
	if (pstVbCalConfig->plane_num == 3) {
		pstVFrame->u32Length[2] = pstVbCalConfig->u32MainCSize;
		pstVFrame->u64PhyAddr[2] = pstVFrame->u64PhyAddr[1]
			+ ALIGN(pstVbCalConfig->u32MainCSize, pstVbCalConfig->u16AddrAlign);
	}
	return CVI_SUCCESS;
}


static CVI_S32 _initVenc(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[0].chnIc;
	CVI_S32 s32Ret = CVI_SUCCESS;
	vencChnCtx *pVencContext;

	s32Ret = SAMPLE_VENC_START(psv);
	if (s32Ret < 0) {
		SAMPLE_PRT("SAMPLE_VENC_START\n");
		return s32Ret;
	}

	for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
		pVencContext = &psv->chnCtx[s32ChnIdx];
		pVencContext->chnStat = CHN_STAT_START;
		pVencContext->nextChnStat = CHN_STAT_START;
		pVencContext->s32VencFd = -1;


		if (SAMPLE_COMM_VPSS_Bind_VENC(0, 0, s32ChnIdx) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VPSS_Bind_VENC NG");
			return CVI_FAILURE;
		}

		if (pIc->bsMode == BS_MODE_SELECT) {
			pVencContext->s32VencFd = CVI_VENC_GetFd(pVencContext->VencChn);
			if (pVencContext->s32VencFd < 0) {
				SAMPLE_PRT("CVI_VENC_GetFd failed with%#x!\n", pVencContext->s32VencFd);
			}
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 _readToVpss(VPSS_GRP VpssGrp, VB_CAL_CONFIG_S *pstVbCalConfig,
		VIDEO_FRAME_INFO_S *pstVideoFrame, FILE *fp)
{
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	CVI_U32 u32len;

	for (int i = 0; i < pstVbCalConfig->plane_num; ++i) {
		if (pstVFrame->u32Length[i] == 0)
			continue;
		pstVFrame->pu8VirAddr[i]
			= CVI_SYS_MmapCache(pstVFrame->u64PhyAddr[i], pstVFrame->u32Length[i]);

		u32len = fread(pstVFrame->pu8VirAddr[i], pstVFrame->u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fread plane%d error\n", i);
			return CVI_FAILURE;
		}
		CVI_SYS_IonInvalidateCache(pstVFrame->u64PhyAddr[i],
				pstVFrame->pu8VirAddr[i],
				pstVFrame->u32Length[i]);
	}

	CVI_VPSS_SendFrame(VpssGrp, pstVideoFrame, -1);

	return CVI_SUCCESS;
}

static CVI_S32 _deInitVpss(VB_CAL_CONFIG_S *pstVbCalConfig,
		VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	VB_BLK blk;
	VPSS_GRP VpssGrp = 0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	for (int i = 0; i < pstVbCalConfig->plane_num; ++i) {
		if (pstVFrame->u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(pstVFrame->pu8VirAddr[i], pstVFrame->u32Length[i]);
	}

	blk = CVI_VB_PhysAddr2Handle(pstVFrame->u64PhyAddr[0]);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_PRT("blk VB_INVALID_HANDLE\n");
		return CVI_FAILURE;
	}

	CVI_VB_ReleaseBlock(blk);

	abChnEnable[0] = CVI_TRUE;
	SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);

	return CVI_SUCCESS;
}

static CVI_S32 _deInitVenc(sampleVenc *psv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	commonInputCfg *pcic = &psv->commonIc;
	vencChnCtx *pVencContext;
	VENC_CHN VencChn;

	for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
		pVencContext = &psv->chnCtx[s32ChnIdx];
		VencChn = pVencContext->VencChn;

		if (pVencContext->s32VencFd >= 0) {
			CVI_VENC_CloseFd(VencChn);
		}

		SAMPLE_COMM_VPSS_UnBind_VENC(0, 0, s32ChnIdx);
	}

	s32Ret = SAMPLE_VENC_STOP(psv);
	if (s32Ret < 0) {
		SAMPLE_PRT("SAMPLE_VENC_STOP\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _SAMPLE_VENC_FRM_testVpssVenc(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[0].chnIc;
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSizeIn, stSizeOut;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VIDEO_FRAME_INFO_S stVideoFrame, *pstVideoFrame = &stVideoFrame;
	VIDEO_FRAME_INFO_S stVideoFrameVpssOut, *pstVideoFrameVpssOut = &stVideoFrameVpssOut;
	VB_CAL_CONFIG_S stVbCalConfig, *pstVbCalConfig = &stVbCalConfig;
	FILE *fpVpssSrc = NULL;
	CVI_S32 idx;
	vencChnCtx *pVencContext;
	VENC_CHN VencChn;

	if (pcic->u32ViWidth < 320 || pcic->u32ViHeight < 256 ||
		pcic->u32VpssWidth < 320 || pcic->u32VpssHeight < 256) {

		SAMPLE_PRT("u32ViWidth = %d, u32ViHeight = %d\n",
				pcic->u32ViWidth, pcic->u32ViHeight);
		SAMPLE_PRT("u32VpssWidth = %d, u32VpssHeight = %d\n",
				pcic->u32VpssWidth, pcic->u32VpssHeight);
		return CVI_FAILURE;
	}

	stSizeIn.u32Width = pcic->u32ViWidth;
	stSizeIn.u32Height = pcic->u32ViHeight;
	stSizeOut.u32Width = pcic->u32VpssWidth;
	stSizeOut.u32Height = pcic->u32VpssHeight;

	s32Ret = SAMPLE_PLAT_SYS_INIT(stSizeIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_PLAT_SYS_INIT, s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	fpVpssSrc = fopen(pIc->vpssSrcPath, "rb");
	if (fpVpssSrc == NULL) {
		SAMPLE_PRT("Input file %s open failed !\n", pIc->vpssSrcPath);
		return CVI_FAILURE;
	}

	VpssGrp = 0;
	VpssChn = 0;

	s32Ret = _initVpss(&stSizeIn, &stSizeOut, pstVbCalConfig,
			SAMPLE_PIXEL_FORMAT, pstVideoFrame, CVI_FALSE, 1);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("_initVpss, 0x%X\n", s32Ret);
		return s32Ret;
	}

	pcic->numChn = 1;
	pcic->ifInitVb = 0;
	pcic->bThreadDisable = CVI_TRUE;
	pIc->vpssGrp = VpssGrp;
	pIc->vpssChn = VpssChn;
	pIc->width = pcic->u32VpssWidth;
	pIc->height = pcic->u32VpssHeight;
	VencChn = 0;
	pVencContext = &psv->chnCtx[VencChn];

	s32Ret = _initVenc(psv);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("_initVenc, 0x%X\n", s32Ret);
		return s32Ret;
	}

	for (idx = 0; idx < pIc->num_frames; idx++) {
		s32Ret = _readToVpss(VpssGrp, pstVbCalConfig,
				pstVideoFrame, fpVpssSrc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("_readToVpss , %d\n", s32Ret);
			goto exit;
		}
		s32Ret = _SAMPLE_VENC_GetStream(pVencContext);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("_SAMPLE_VENC_GetStream , %d\n", s32Ret);
			break;
		}
	}

exit:
	s32Ret = _deInitVpss(pstVbCalConfig, pstVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("_deInitVpss, 0x%X\n", s32Ret);
		return s32Ret;
	}

	s32Ret = _deInitVenc(psv);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("_deInitVenc, 0x%X\n", s32Ret);
		return s32Ret;
	}

	if (fpVpssSrc) {
		fclose(fpVpssSrc);
		fpVpssSrc = NULL;
	}
	return s32Ret;
}
#endif


CVI_S32 SAMPLE_VENC_START(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[0].chnIc;
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (!strcmp(pIc->codec, "265") ||
		!strcmp(pIc->codec, "264") ||
		!strcmp(pIc->codec, "mjp") ||
		!strcmp(pIc->codec, "jpg") ||
		!strcmp(pIc->codec, "multi")) {

		if (pcic->ifInitVb) {
			s32Ret = initSysAndVb(psv);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("fail to init sys and vb, 0x%x\n", s32Ret);
				return s32Ret;
			}
		}

		s32Ret = SAMPLE_COMM_VENC_SetModParam(pcic);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetModParam failure\n");
			return CVI_FAILURE;
		}

		for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
			s32Ret = SAMPLE_VENC_INIT_CHANNEL(psv, s32ChnIdx);
			if (s32Ret) {
				SAMPLE_PRT("[Chn %d]sample venc init failed\n", s32ChnIdx);
				return CVI_FAILURE;
			}
		}

		if (pcic->bThreadDisable == CVI_FALSE) {
			for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++)
				SAMPLE_VENC_StartGetStream(&psv->chnCtx[s32ChnIdx], s32ChnIdx);
		}
	} else {
		SAMPLE_PRT("codec = %s\n", pIc->codec);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_VENC_STOP(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[0].chnIc;
	vencChnCtx *pVencContext;
	CVI_S32 s32ChnIdx;

	if (!strcmp(pIc->codec, "265") ||
		!strcmp(pIc->codec, "264") ||
		!strcmp(pIc->codec, "mjp") ||
		!strcmp(pIc->codec, "jpg") ||
		!strcmp(pIc->codec, "multi")) {

		for (s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
			pVencContext = &psv->chnCtx[s32ChnIdx];
			pIc = &pVencContext->chnIc;
			vencUnbindSource(pIc, s32ChnIdx);
			SAMPLE_COMM_VENC_Stop(s32ChnIdx);
		}

		for (s32ChnIdx = 0; s32ChnIdx < pcic->numChn; s32ChnIdx++) {
			pVencContext = &psv->chnCtx[s32ChnIdx];
			pIc = &pVencContext->chnIc;

			if (pVencContext->pFile) {
				fclose(pVencContext->pFile);
				pVencContext->pFile = NULL;
			}

			if (pIc->bind_mode == VENC_BIND_DISABLE) {
				if (pVencContext->fpSrc) {
					fclose(pVencContext->fpSrc);
					pVencContext->fpSrc = NULL;
				}
			}

			if (pcic->ifInitVb) {
				SAMPLE_PRT("s32ChnIdx = %d, pstFrameInfo = 0x%p\n",
						s32ChnIdx, pVencContext->pstFrameInfo);
				free_frame(pVencContext->pstFrameInfo);
			}

			if (pVencContext->QpMapPool != VB_INVALID_POOLID) {
				if (pVencContext->QpMapBlk != VB_INVALID_HANDLE)
					CVI_VB_ReleaseBlock(pVencContext->QpMapBlk);
				if (pVencContext->AiMapBlk != VB_INVALID_HANDLE)
					CVI_VB_ReleaseBlock(pVencContext->AiMapBlk);
				CVI_VB_MunmapPool(pVencContext->QpMapPool);
				CVI_VB_DestroyPool(pVencContext->QpMapPool);
			} else if (pVencContext->pu8QpMap) {
				free(pVencContext->pu8QpMap);
				pVencContext->pu8QpMap = NULL;
			}
			/*
			if (vpssVB[s32ChnIdx] != VB_INVALID_POOLID) {
				CVI_VPSS_DetachVbPool(0, s32ChnIdx);
				CVI_VB_DestroyPool(vpssVB[s32ChnIdx]);
				vpssVB[s32ChnIdx] = VB_INVALID_POOLID;
			}*/
		}
	} else {
		SAMPLE_PRT("codec = %s\n", pIc->codec);
		return -1;
	}

	return 0;
}

CVI_S32 SAMPLE_VENC_MOVE_TO_STOP_STATE(sampleVenc *psv)
{
	int idx;
	commonInputCfg *pcic = &psv->commonIc;
	vencChnCtx *pVencContext;

	for (idx = 0; idx < pcic->numChn; idx++) {
		pVencContext = &psv->chnCtx[idx];
		pVencContext->chnStat = CHN_STAT_STOP;
	}
	return 0;
}

static CVI_S32 checkArg(CVI_S32 entryIdx, SAMPLE_ARG *pArg)
{
	SAMPLE_PRT("entryIdx = %d\n", entryIdx);

	if (g_vencLongOptionExt[entryIdx].type == ARG_INT) {
		pArg->ival = strtoimax(optarg, NULL, 10);
		if ((int64_t)(pArg->ival) < g_vencLongOptionExt[entryIdx].min ||
			(int64_t)(pArg->ival) > g_vencLongOptionExt[entryIdx].max) {
			SAMPLE_PRT("%s = %d, min = %"PRId64", max = %"PRId64"\n",
					g_vencLongOptionExt[entryIdx].opt.name,
					pArg->ival,
					g_vencLongOptionExt[entryIdx].min,
					g_vencLongOptionExt[entryIdx].max);
			return CVI_FAILURE;
		}
	} else if (g_vencLongOptionExt[entryIdx].type == ARG_UINT) {
		pArg->uval = strtoumax(optarg, NULL, 10);
		if ((int64_t)(pArg->uval) < g_vencLongOptionExt[entryIdx].min ||
			(int64_t)(pArg->uval) > g_vencLongOptionExt[entryIdx].max) {
			SAMPLE_PRT("%s = %u, min = %"PRId64", max = %"PRId64"\n",
					g_vencLongOptionExt[entryIdx].opt.name,
					pArg->uval,
					g_vencLongOptionExt[entryIdx].min,
					g_vencLongOptionExt[entryIdx].max);
			return CVI_FAILURE;
		}
	} else if (g_vencLongOptionExt[entryIdx].type == ARG_STRING) {
		if (optarg == NULL) {
			SAMPLE_PRT("%s = NULL\n", g_vencLongOptionExt[entryIdx].opt.name);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 checkInputCfg(chnInputCfg *pIc)
{
	if (!strcmp(pIc->codec, "264") || !strcmp(pIc->codec, "265")) {

		SAMPLE_PRT("framerate = %d\n", pIc->framerate);

		if (pIc->gop < 1) {
			if (!strcmp(pIc->codec, "264"))
				pIc->gop = DEF_264_GOP;
			else
				pIc->gop = DEF_GOP;
		}
		SAMPLE_PRT("gop = %d\n", pIc->gop);

		if (!strcmp(pIc->codec, "265")) {
			if (pIc->single_LumaBuf > 0) {
				SAMPLE_PRT("single_LumaBuf only supports H.264\n");
				pIc->single_LumaBuf = 0;
			}
		}
		pIc->iqp = (pIc->iqp >= 0) ? pIc->iqp : DEF_IQP;
		pIc->pqp = (pIc->pqp >= 0) ? pIc->pqp : DEF_PQP;

		if (pIc->complex_scene_detect_en) {
			if (pIc->complex_scene_hight_th <= pIc->complex_scene_low_th) {
				SAMPLE_PRT("scene complex threshold set issue\n");
				return -1;
			}
		}

		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
		}

		if (pIc->rcMode == SAMPLE_RC_CBR ||
			pIc->rcMode == SAMPLE_RC_VBR ||
			pIc->rcMode == SAMPLE_RC_AVBR ||
			pIc->rcMode == SAMPLE_RC_QPMAP ||
			pIc->rcMode == SAMPLE_RC_UBR) {

			if (pIc->rcMode == SAMPLE_RC_CBR ||
				pIc->rcMode == SAMPLE_RC_UBR) {
				if (pIc->bitrate <= 0) {
					SAMPLE_PRT("CBR / UBR bitrate must be not less than 0");
					return -1;
				}
				SAMPLE_PRT("bitrate = %d\n", pIc->bitrate);
			} else if (pIc->rcMode == SAMPLE_RC_VBR) {
				if (pIc->maxbitrate <= 0) {
					SAMPLE_PRT("VBR must be not less than 0");
					return -1;
				}
				SAMPLE_PRT("RC_VBR, maxbitrate = %d\n", pIc->maxbitrate);
			}

			pIc->firstFrmstartQp =
				(pIc->firstFrmstartQp < 0 ||
				 pIc->firstFrmstartQp > 51) ? 30 : pIc->firstFrmstartQp;
			SAMPLE_PRT("firstFrmstartQp = %d\n", pIc->firstFrmstartQp);

			pIc->maxIqp = (pIc->maxIqp >= 0) ? pIc->maxIqp : DEF_264_MAXIQP;
			pIc->minIqp = (pIc->minIqp >= 0) ? pIc->minIqp : DEF_264_MINIQP;
			pIc->maxQp = (pIc->maxQp >= 0) ? pIc->maxQp : DEF_264_MAXQP;
			pIc->minQp = (pIc->minQp >= 0) ? pIc->minQp : DEF_264_MINQP;
			SAMPLE_PRT("maxQp = %d, minQp = %d, maxIqp = %d, minIqp = %d\n",
					pIc->maxQp,
					pIc->minQp,
					pIc->maxIqp,
					pIc->minIqp);

			if (pIc->statTime == 0) {
				pIc->statTime = DEF_STAT_TIME;
			}
			SAMPLE_PRT("statTime = %d\n", pIc->statTime);
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			if (pIc->firstFrmstartQp != -1) {
				SAMPLE_PRT("firstFrmstartQp is invalid in FixQP mode\n");
				pIc->firstFrmstartQp = -1;
			}

			pIc->bitrate = 0;
			SAMPLE_PRT("RC_FIXQP, iqp = %d, pqp = %d\n",
				pIc->iqp,
				pIc->pqp);
		} else {
			SAMPLE_PRT("codec = %s, rcMode = %d, not supported RC mode\n", pIc->codec, pIc->rcMode);
			return -1;
		}
	} else if (!strcmp(pIc->codec, "mjp") || !strcmp(pIc->codec, "jpg")) {
		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
			pIc->quality = (pIc->quality != -1) ? pIc->quality : 0;
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			if (pIc->quality < 0)
				pIc->quality = 0;
			else if (pIc->quality >= 100)
				pIc->quality = 99;
			else if (pIc->quality == 0)
				pIc->quality = 1;
		}
	} else {
		SAMPLE_PRT("codec = %s\n", pIc->codec);
		return -1;
	}

	return 0;
}

static CVI_U32 SAMPLE_VENC_INIT_CHANNEL(sampleVenc *psv, CVI_U32 chnNum)
{
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[chnNum].chnIc;
	vencChnCtx *pVencContext = &psv->chnCtx[chnNum];
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S inFrmSize;
	char file_ext[16];

	pVencContext->enPixelFormatIn = vencMapPixelFormat(pIc->pixel_format);

	pVencContext->VencChn = chnNum;
	pVencContext->enGopMode = pIc->gopMode;
	pVencContext->s32FbCnt = 1;
	pVencContext->QpMapPool = VB_INVALID_POOLID;
	pVencContext->QpMapBlk = VB_INVALID_HANDLE;
	pVencContext->AiMapBlk = VB_INVALID_HANDLE;

	if (!strcmp(pIc->codec, "265"))
		pVencContext->enPayLoad = PT_H265;
	else if (!strcmp(pIc->codec, "264"))
		pVencContext->enPayLoad = PT_H264;
	else if (!strcmp(pIc->codec, "mjp"))
		pVencContext->enPayLoad = PT_MJPEG;
	else if (!strcmp(pIc->codec, "jpg"))
		pVencContext->enPayLoad = PT_JPEG;

	if (pVencContext->enPayLoad == PT_H264)
		pVencContext->u32Profile = pIc->u32Profile;
	else
		pVencContext->u32Profile = 0;

	if (pVencContext->enPayLoad == PT_H264 || pVencContext->enPayLoad == PT_H265)
		pVencContext->enPixelFormat = PIXEL_FORMAT_NV12;
	else
		pVencContext->enPixelFormat = pVencContext->enPixelFormatIn;

	pVencContext->stSize.u32Width = pIc->width;
	pVencContext->stSize.u32Height = pIc->height;
	if (pIc->inWidth || pIc->inHeight) {
		inFrmSize.u32Width = pIc->inWidth;
		inFrmSize.u32Height = pIc->inHeight;
	} else {
		inFrmSize = pVencContext->stSize;
	}

	pVencContext->u32FrameSize = getSrcFrameSizeByPixelFormat(
		pVencContext->stSize.u32Width, pVencContext->stSize.u32Height, pVencContext->enPixelFormat);

	pVencContext->enRcMode = pIc->rcMode;
	SAMPLE_PRT("enPayLoad = %d, enRcMode = %d, bsMode = %d\n",
			   pVencContext->enPayLoad, pVencContext->enRcMode, pIc->bsMode);

	pVencContext->enSize = getEnSize(pIc->width, pIc->height);
	pVencContext->enSize = PIC_CUSTOMIZE;
	if (pVencContext->enSize == PIC_BUTT) {
		SAMPLE_PRT("unsupport size %d x %d\n", pIc->width, pIc->height);
		return CVI_FAILURE;
	}

	if (pVencContext->enPayLoad != PT_JPEG) {
		s32Ret = SAMPLE_COMM_VENC_GetGopAttr(pVencContext->enGopMode, &pVencContext->stGopAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Venc Get GopAttr for %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		if ((pVencContext->enPayLoad != PT_JPEG) && (pVencContext->enPayLoad != PT_MJPEG)) {
			if (pcic->vbMode == VB_SOURCE_USER) {
				//create vb pool
				SAMPLE_PRT("vbMode VB_SOURCE_USER_MODE\n");
				s32Ret =  SAMPLE_COMM_VENC_InitVBPool(pVencContext, chnNum);
				if (s32Ret != CVI_SUCCESS)
					SAMPLE_PRT("init mod common vb fail\n");
				//should attach vb pool after chn create
			}
		}
	}

	s32Ret = SAMPLE_COMM_VENC_Start(
			pIc,
			pVencContext->VencChn,
			pVencContext->enPayLoad,
			pVencContext->enSize,
			pVencContext->enRcMode,
			pVencContext->u32Profile,
			pIc->bRcnRefShare,
			&pVencContext->stGopAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_VENC_LoadCfgFile(pVencContext);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VENC_LoadCfgFile %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	if (pcic->ifInitVb) {
		pVencContext->pstFrameInfo = allocate_frame(inFrmSize, pVencContext->enPixelFormat);
		SAMPLE_PRT("pstFrameInfo = 0x%p\n", pVencContext->pstFrameInfo);

		if (!pVencContext->pstFrameInfo) {
			SAMPLE_PRT("allocate_frame\n");
			return CVI_FAILURE;
		}

		pVencContext->pstVFrame = &pVencContext->pstFrameInfo->stVFrame;
	}

	if (pIc->bind_mode == VENC_BIND_DISABLE && strlen(pIc->input_path) != 0) {
		pVencContext->fpSrc = fopen(pIc->input_path, "rb");

		if (pVencContext->fpSrc == NULL) {
			SAMPLE_PRT("Input file %s open failed !\n", pIc->input_path);
			return CVI_FAILURE;
		}

		// Get the yuv file size
		fseek(pVencContext->fpSrc, 0, SEEK_END);
		pVencContext->file_size = ftell(pVencContext->fpSrc);
		pVencContext->num_frames = pVencContext->file_size / pVencContext->u32FrameSize;
		SAMPLE_PRT("file_size %ld\n", pVencContext->file_size);
		SAMPLE_PRT("u32FrameSize %d\n", pVencContext->u32FrameSize);

		if ((pIc->num_frames > 0) && (pVencContext->num_frames > (CVI_U32)pIc->num_frames))
			pVencContext->num_frames = pIc->num_frames;

		SAMPLE_PRT("chnNum = %d, num_frames = %d\n", chnNum, pVencContext->num_frames);

		rewind(pVencContext->fpSrc);

		SAMPLE_PRT(
				"input_path = %s, file_size = 0x%lX, num_frames = %d, u32FrameSize = 0x%X\n",
				pIc->input_path,
				pVencContext->file_size,
				pVencContext->num_frames,
				pVencContext->u32FrameSize);
	} else {
		pVencContext->num_frames = pIc->num_frames;
	}

	SAMPLE_COMM_VENC_GetFilePostfix(pVencContext->enPayLoad, file_ext);
	snprintf(pIc->outputFileName, MAX_STRING_LEN, "%.*s%s",
		(int)((strlen(pIc->output_path) + strlen(file_ext) >= MAX_STRING_LEN) ?
		MAX_STRING_LEN - 1 - strlen(file_ext) :
		strlen(pIc->output_path)), pIc->output_path, file_ext);


	SAMPLE_PRT("Begin to send frames ..., bsMode = %d\n", pIc->bsMode);

	pVencContext->pFile = fopen(pIc->outputFileName, "wb");
	if (pVencContext->pFile == NULL) {
		SAMPLE_PRT("open file err, %s\n", pIc->outputFileName);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 initSysAndVb(sampleVenc *psv)
{
	commonInputCfg *pcic = &psv->commonIc;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32BlkSize;
	VB_CONFIG_S stVbConf;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	stVbConf.u32MaxPoolCnt = 0;
	SAMPLE_PRT("chn:%d\n",pcic->numChn);
	for (int i = 0; i < pcic->numChn; i++) {
		SIZE_S stSize;
		PIXEL_FORMAT_E enPixelFormat;
		chnInputCfg *pIc;
		CVI_BOOL bRepeated = CVI_FALSE;

		pIc = &psv->chnCtx[i].chnIc;
		enPixelFormat = vencMapPixelFormat(pIc->pixel_format);

		if (pIc->inWidth || pIc->inHeight) {
			stSize.u32Width = pIc->inWidth;
			stSize.u32Height = pIc->inHeight;
		} else {
			stSize.u32Width = pIc->width;
			stSize.u32Height = pIc->height;
		}
		u32BlkSize = VENC_GetPicBufferSize(
						stSize.u32Width,
						stSize.u32Height,
						enPixelFormat,
						DATA_BITWIDTH_8,
						COMPRESS_MODE_NONE);

		u32BlkSize += 0x1000 * 3;
		SAMPLE_PRT("u32BlkSize[%d]align[%d]\n",
			u32BlkSize, VENC_ALIGN_W);

		for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
			if (u32BlkSize == stVbConf.astCommPool[j].u32BlkSize) {
				stVbConf.astCommPool[j].u32BlkCnt++;
				bRepeated = CVI_TRUE;
				break;
			}
		}

		if (bRepeated == CVI_FALSE) {
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[stVbConf.u32MaxPoolCnt].u32BlkCnt  = 1;
			stVbConf.u32MaxPoolCnt++;
		}

		if (stVbConf.u32MaxPoolCnt > VB_MAX_COMM_POOLS) {
			SAMPLE_PRT("u32MaxPoolCnt %d > , VB_MAX_COMM_POOLS %d\n",
				stVbConf.u32MaxPoolCnt, VB_MAX_COMM_POOLS);
			return CVI_FAILURE;
		}
	}

	for (CVI_U32 j = 0; j < stVbConf.u32MaxPoolCnt; j++) {
		SAMPLE_PRT("[Pool %d] u32BlkSize: %d, u32BlkCnt: %d\n",
			j, stVbConf.astCommPool[j].u32BlkSize, stVbConf.astCommPool[j].u32BlkCnt);
	}

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_Init, %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_VENC_SetUpQmapBuf(vencChnCtx *pVencContext, CVI_S32 width, CVI_S32 height)
{
	VB_POOL_CONFIG_S cfg = {0};
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pVencContext->QpMapPool != VB_INVALID_POOLID)
		return s32Ret;
	//use isp motion map for size calc
	cfg.u32BlkSize = (ALIGN(width, 256) >> 2) * (height >> 2) >> 2;
	cfg.u32BlkSize = ALIGN(cfg.u32BlkSize, 4096);
	cfg.u32BlkCnt = 2;
	cfg.enRemapMode = VB_REMAP_MODE_NOCACHE;
	memcpy(cfg.acName, "qp_map", 6);

	pVencContext->QpMapPool = CVI_VB_CreatePool(&cfg);
	s32Ret = CVI_VB_MmapPool(pVencContext->QpMapPool);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_MmapPool NG\n");
		return s32Ret;
	}
	pVencContext->QpMapBlk = CVI_VB_GetBlock(pVencContext->QpMapPool, cfg.u32BlkSize);
	pVencContext->pu8QpMap = CVI_SYS_Mmap(CVI_VB_Handle2PhysAddr(pVencContext->QpMapBlk), cfg.u32BlkSize);
	if (!pVencContext->pu8QpMap) {
		SAMPLE_PRT("CVI_SYS_Mmap NG\n");
		return s32Ret;
	}
	pVencContext->QpMapSize = cfg.u32BlkSize;
	return s32Ret;
}

static CVI_S32 SAMPLE_VENC_LoadCfgFile(vencChnCtx *pVencContext)
{
	chnInputCfg *pIc = &pVencContext->chnIc;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i;

	for (i = 0; i < MAX_NUM_ROI; i++) {
		pVencContext->vencRoi[i].u32FrameStart = (CVI_U32)(-1);
		pVencContext->vencRoi[i].u32FrameEnd = 0;
	}

	if ((pVencContext->enPayLoad == PT_H265 || pVencContext->enPayLoad == PT_H264) &&
		strlen(pIc->qpMapCfgFile) > 0) {

		s32Ret = SAMPLE_COMM_VENC_LoadRoiCfgFile(pVencContext->vencRoi, pIc->qpMapCfgFile);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Venc LoadRoiCfgFile failed for %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		if (SAMPLE_VENC_SetUpQmapBuf(pVencContext, pVencContext->stSize.u32Width, pVencContext->stSize.u32Height)) {
			return CVI_FAILURE;
		}
	} else if ((pVencContext->enPayLoad == PT_H264 || pVencContext->enPayLoad == PT_H265) && strlen(pIc->roiCfgFile) > 0) {
		s32Ret = SAMPLE_COMM_VENC_LoadRoiCfgFile(pVencContext->vencRoi, pIc->roiCfgFile);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Venc LoadRoiCfgFile failed for %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	return s32Ret;
}

static CVI_S32 _SAMPLE_VENC_SetChnParam(vencChnCtx *pVencChnCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_CHN VencChn = pVencChnCtx->VencChn;
	chnInputCfg *pIc = &pVencChnCtx->chnIc;

	VENC_CHN_ATTR_S stChnAttr;

	CVI_VENC_GetChnAttr(VencChn, &stChnAttr);

	if (stChnAttr.stVencAttr.enType == PT_H265) {
		stChnAttr.stRcAttr.stH265Cbr.u32BitRate = pIc->chgBitrate;
		stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = pIc->chgFramerate;
	} else if (stChnAttr.stVencAttr.enType == PT_H264) {
		stChnAttr.stRcAttr.stH264Cbr.u32BitRate = pIc->chgBitrate;
		stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = pIc->chgFramerate;
	} else if (stChnAttr.stVencAttr.enType == PT_MJPEG) {
		stChnAttr.stRcAttr.stMjpegCbr.u32BitRate = pIc->chgBitrate;
		stChnAttr.stRcAttr.stMjpegCbr.fr32DstFrameRate = pIc->chgFramerate;
	}

	CVI_VENC_SetChnAttr(VencChn, &stChnAttr);

	return s32Ret;
}

extern pthread_t gs_VencTask[VENC_MAX_CHN_NUM];
static CVI_S32 SAMPLE_VENC_StartGetStream(vencChnCtx *pVencContext, CVI_S32 s32ChnIdx)
{
	struct sched_param param;
	pthread_attr_t attr;

	SAMPLE_PRT("SAMPLE_VENC_StartGetStream\n");

	gs_VencTask[s32ChnIdx] = 0;

	param.sched_priority = 80;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(
			&gs_VencTask[s32ChnIdx],
			&attr,
			SAMPLE_VENC_GetVencStreamProc,
			(CVI_VOID *)pVencContext);

	return CVI_SUCCESS;
}

static CVI_S32 getNonBindModeSrcFrame(vencChnCtx *pVencContext,
		VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_CHN VencChn = pVencContext->VencChn;
	chnInputCfg *pIc = &pVencContext->chnIc;

	// non-bind mode
	//	+ input_path != 0, VENC-only test, read source file
	//	+ input_path == 0, test with VPSS
	//		* vpssSrcPath == 0, VPSS source comes from sensor.
	//	      Need to get / release frame
	//		* vpssSrcPath != 0, VPSS source comes from file.
	//	      User gets / releases frame
	if (strlen(pIc->input_path) != 0) {
		s32Ret = cviReadSrcFrame(pVencContext->pstVFrame, pVencContext->fpSrc, pVencContext->enPixelFormatIn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("(chn %d) cviReadSrcFrame fail\n", VencChn);
			return s32Ret;
		}
	} else if (strlen(pIc->vpssSrcPath) == 0) {
		pVencContext->pstFrameInfo = pstVideoFrame;
		s32Ret = CVI_VPSS_GetChnFrame(0, VencChn, pVencContext->pstFrameInfo, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			return s32Ret;
		}
	} else {
		pVencContext->pstFrameInfo = pstVideoFrame;
	}

	return s32Ret;
}

static CVI_VOID vencUnbindSource(chnInputCfg *pIc, VENC_CHN VencChn)
{
	if (pIc->bind_mode == VENC_BIND_VPSS) {
		SAMPLE_PRT("Venc unbind vpss\n");
		SAMPLE_COMM_VPSS_UnBind_VENC(pIc->vpssGrp, pIc->vpssChn, VencChn);
	}
}

static CVI_S32 releaseNonBindModeSrcFrame(vencChnCtx *pVencContext)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_CHN VencChn = pVencContext->VencChn;
	chnInputCfg *pIc = &pVencContext->chnIc;

	// non-bind mode
	//	+ input_path != 0, VENC-only test, read source file
	//	+ input_path == 0, test with VPSS
	//		* vpssSrcPath == 0, VPSS source comes from sensor.
	//	      Need to get / release frame
	//		* vpssSrcPath != 0, VPSS source comes from file.
	//	      User gets / releases frame
	if (strlen(pIc->input_path) == 0) {
		if (strlen(pIc->vpssSrcPath) == 0) {
			s32Ret = CVI_VPSS_ReleaseChnFrame(0, VencChn, pVencContext->pstFrameInfo);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame fail\n");
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

void *steam_save = NULL;
static CVI_VOID *SAMPLE_VENC_GetVencStreamProc(CVI_VOID *pArgs)
{
	vencChnCtx *pVencContext = (vencChnCtx *)pArgs;
	VENC_CHN VencChn = pVencContext->VencChn;
	chnInputCfg *pIc = &pVencContext->chnIc;
	VIDEO_FRAME_INFO_S stVideoFrame, *pstFrameInfo = &stVideoFrame;
	CVI_CHAR TaskName[64];
	CVI_S32 s32Ret;
	CVI_U32 i;

	sprintf(TaskName, "chn%dVencGetStream", VencChn);
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);

	SAMPLE_PRT("venc task%d start\n", VencChn);

	usleep(1000);

	pVencContext->chnStat = CHN_STAT_START;
	pVencContext->nextChnStat = CHN_STAT_START;
	pVencContext->s32VencFd = -1;

	if (pIc->bsMode == BS_MODE_SELECT) {
		pVencContext->s32VencFd = CVI_VENC_GetFd(VencChn);
		if (pVencContext->s32VencFd < 0) {
			SAMPLE_PRT("CVI_VENC_GetFd failed with%#x!\n", pVencContext->s32VencFd);
		}
	}

	if (strlen(pIc->motionMapBinFile) > 0) {
		pIc->motionMapFile = fopen(pIc->motionMapBinFile, "r");
		if (pIc->motionMapFile == NULL) {
			SAMPLE_PRT("open roi bin file fail\n");
			return (CVI_VOID *) CVI_FAILURE;
		}
	}

	if (strlen(pIc->aiMapBinFile) > 0) {
		pIc->aiMapFile = fopen(pIc->aiMapBinFile, "r");
		if (pIc->aiMapFile == NULL) {
			SAMPLE_PRT("open roi bin file fail\n");
			return (CVI_VOID *) CVI_FAILURE;
		}
	}

	i = 0;
	while (pVencContext->chnStat == CHN_STAT_START && pVencContext->pFile && i < pVencContext->num_frames) {
		if (pIc->bind_mode == VENC_BIND_DISABLE) {
			s32Ret = getNonBindModeSrcFrame(pVencContext, pstFrameInfo);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("(chn %d) getNonBindModeSrcFrame fail\n", VencChn);
				break;
			}
		}

		s32Ret = SAMPLE_VENC_SendFrame(pVencContext, i);
		if (s32Ret == CVI_ERR_VENC_FRC_NO_ENC) {
			continue;
		} else if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("_SAMPLE_VENC_SendFrame, %d\n", s32Ret);
			break;
		}

		s32Ret = SAMPLE_VENC_GetStream(pVencContext);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("_SAMPLE_VENC_GetStream, %d\n", s32Ret);
			break;
		}

		usleep(100);

		i++;
	}

	if (pVencContext->s32VencFd >= 0) {
		CVI_VENC_CloseFd(VencChn);
	}

	if (strlen(pIc->motionMapBinFile) > 0) {
		fclose(pIc->motionMapFile);
	}
	if (strlen(pIc->aiMapBinFile) > 0) {
		fclose(pIc->aiMapFile);
	}
	vencUnbindSource(pIc, VencChn);

	return (CVI_VOID *) CVI_SUCCESS;
}

static CVI_S32 SAMPLE_VENC_SendFrame(vencChnCtx *pVencContext, CVI_U32 i)
{
	size_t readBytes;
	chnInputCfg *pIc = &pVencContext->chnIc;
	VENC_CHN VencChn = pVencContext->VencChn;
	CVI_U32 enableMotionMap = (strlen(pIc->motionMapBinFile) > 0);
	CVI_U32 enableAiMap = (strlen(pIc->aiMapBinFile) > 0);
	CVI_U32 enableRoicfg = (strlen(pIc->roiCfgFile) > 0);
	CVI_U32 enableQpMap = strlen(pIc->qpMapCfgFile) > 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_PRT("[Chn%d] frame %d (%d)\n", VencChn, i, pVencContext->num_frames);

	if (pIc->forceIdr > 0 && pIc->forceIdr == (CVI_S32)i) {
		CVI_BOOL bInstant = pIc->u32ResetGop;

		CVI_VENC_RequestIDR(VencChn, bInstant);
		SAMPLE_PRT("CVI_VENC_RequestIDR, resetGop:%d\n", bInstant);
	}
	if (enableMotionMap) {
		size_t motion_size = (((pVencContext->stSize.u32Width + 255) & (~255)) >> 2) * (pVencContext->stSize.u32Height >> 2) >> 2;

		if (i == 0) {
			if (SAMPLE_VENC_SetUpQmapBuf(pVencContext, pVencContext->stSize.u32Width, pVencContext->stSize.u32Height)) {
				return -1;
			}
		}
		memset(pVencContext->pu8QpMap, 0, motion_size);
		if( i > 0) {
			readBytes = fread(pVencContext->pu8QpMap, 1, motion_size, pIc->motionMapFile);
			if (readBytes != motion_size) {
				SAMPLE_PRT("fread error.readBytes:%zu\n", readBytes);
				return -1;
			}
		}
		CVI_SYS_IonFlushCache(CVI_VB_Handle2PhysAddr(pVencContext->QpMapBlk), pVencContext->pu8QpMap, motion_size);
	}
	if (enableAiMap) {
		size_t ai_size = 0;
		if (!strcmp(pIc->codec, "264"))
			ai_size = (ALIGN(pVencContext->stSize.u32Width, 1024) / 1024) * 128 *
				ALIGN(pVencContext->stSize.u32Height, 16) / 16;
		else if (!strcmp(pIc->codec, "265"))
			ai_size = (ALIGN(pVencContext->stSize.u32Width, 256) / 256) * 128 *
				ALIGN(pVencContext->stSize.u32Height, 64) / 64;
		else {
			SAMPLE_PRT("codec no support ai map\n");
			return -1;
		}
		if (i == 0) {
			if (SAMPLE_VENC_SetUpQmapBuf(pVencContext, pVencContext->stSize.u32Width, pVencContext->stSize.u32Height)) {
				SAMPLE_PRT("_SAMPLE_VENC_SetUpQmapBuf fail\n");
				return -1;
			}
			pVencContext->AiMapBlk = CVI_VB_GetBlock(pVencContext->QpMapPool, ai_size);
			pVencContext->pu8AiMap = CVI_SYS_Mmap(CVI_VB_Handle2PhysAddr(pVencContext->AiMapBlk), ai_size);
			if (!pVencContext->pu8AiMap) {
				SAMPLE_PRT("CVI_SYS_Mmap NG\n");
				return s32Ret;
			}
			pVencContext->AiMapSize = ai_size;
		}
		readBytes = fread(pVencContext->pu8AiMap, 1, ai_size, pIc->aiMapFile);
		if (readBytes != ai_size) {
			SAMPLE_PRT("fread error.readBytes:%zu\n", readBytes);
			return -1;
		}
		CVI_SYS_IonFlushCache(CVI_VB_Handle2PhysAddr(pVencContext->AiMapBlk), pVencContext->pu8AiMap, ai_size);
	}

	if (enableQpMap) {
		s32Ret = SAMPLE_COMM_VENC_SetQpMapByCfgFile(VencChn,
				pVencContext->vencRoi, i, pVencContext->pu8QpMap, &pVencContext->QpMapMode,
				pVencContext->stSize.u32Width, pVencContext->stSize.u32Height,
				pVencContext->enPayLoad, &pVencContext->QpMapSize);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VENC_SetQpMapByCfgFile, %d\n", s32Ret);
			return s32Ret;
		}
		CVI_SYS_IonFlushCache(CVI_VB_Handle2PhysAddr(pVencContext->QpMapBlk),
			pVencContext->pu8QpMap, pVencContext->QpMapSize);
	}
	if (enableRoicfg) {
		if ((pVencContext->enPayLoad == PT_H264) ||
				(pVencContext->enPayLoad == PT_H265 && pVencContext->enRcMode != SAMPLE_RC_FIXQP)) {
			s32Ret = SAMPLE_COMM_VENC_SetRoiAttrByCfgFile(VencChn, pVencContext->vencRoi, i);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_VENC_SetRoiAttrByCfgFile, %d\n", s32Ret);
				return s32Ret;
			}
		}
	}

	if (pIc->bind_mode == VENC_BIND_DISABLE) {
		if ((pIc->chgNum != -1) &&  ((CVI_S32)i == pIc->chgNum)) {
			_SAMPLE_VENC_SetChnParam(pVencContext);
			pIc->chgNum = -1;
		}

		pVencContext->pstFrameInfo->stVFrame.u64PTS = (CVI_U64) i;

RETRY_SEND_FRAME:
		s32Ret = SAMPLE_VENC_SendOneFrame(pVencContext);
		if (s32Ret == CVI_ERR_VENC_FRC_NO_ENC) {
			SAMPLE_PRT("no encode\n");
			return s32Ret;
		} else if (s32Ret == CVI_ERR_VENC_BUSY) {
			SAMPLE_PRT("send frame timeout ..retry[%d]\n",
					pIc->sendframe_timeout);
			if (pIc->sendframe_timeout == 0) {
				//try once mode
				//user should retry
				SAMPLE_PRT("sendframe_timeout==0 failure ..retry\n");
			}
			goto RETRY_SEND_FRAME;
		} else if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VENC_SendFrame, VencChn = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_VENC_GetStream(vencChnCtx *pVencContext)
{
	chnInputCfg *pIc = &pVencContext->chnIc;
	VENC_CHN VencChn = pVencContext->VencChn;
	CVI_S32 s32Ret;

#if !defined(CONFIG_DUAL_OS)
	if (pIc->bsMode == BS_MODE_SELECT && pVencContext->s32VencFd >= 0) {
		struct timeval TimeoutVal;
		fd_set readFds;

		FD_ZERO(&readFds);
		FD_SET(pVencContext->s32VencFd, &readFds);
		TimeoutVal.tv_sec  = 10;
		TimeoutVal.tv_usec = 0;

		s32Ret = select(pVencContext->s32VencFd + 1, &readFds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0) {
			SAMPLE_PRT("select failed!\n");
			goto ERROR_HANDLER;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("select time out??\n");
			goto ERROR_HANDLER;
		} else if (!FD_ISSET(pVencContext->s32VencFd, &readFds)) {
			SAMPLE_PRT("select return not set??\n");
			goto ERROR_HANDLER;
		}
	}

	s32Ret = SAMPLE_COMM_VENC_SaveChannelStream(pVencContext);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VENC_SaveChannelStream, VencChn = %d, s32Ret = %d\n",
				VencChn, s32Ret);
		goto ERROR_HANDLER;
	}
#else
	int retry_cnt = 0;
	do {
		s32Ret = SAMPLE_COMM_VENC_SaveChannelStream(pVencContext);
		if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VENC_SaveChannelStream, VencChn = %d, s32Ret = %d\n",
				VencChn, s32Ret);
		}
		if (retry_cnt++ > 10) {
			SAMPLE_PRT("get stream fail retry_cnt%d\n", retry_cnt);
			goto ERROR_HANDLER;
		}
	} while ((s32Ret != CVI_SUCCESS) && s32Ret != CVI_ERR_VENC_GET_STREAM_END);
#endif
	if (pIc->bind_mode == VENC_BIND_DISABLE) {
		s32Ret = releaseNonBindModeSrcFrame(pVencContext);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("_releaseNonBindModeSrcFrame fail\n");
			return s32Ret;
		}
	}
	return CVI_SUCCESS;
ERROR_HANDLER:
	if (pIc->bind_mode == VENC_BIND_DISABLE) {
		releaseNonBindModeSrcFrame(pVencContext);
	}
	return s32Ret;
}

/**
 * Convert YUV420 planar format to NV12 format.
 *
 * @param yuv420Planes Input YUV420 planar data (Y, U, V planes).
 * @param nv12Planes Output NV12 data (Y, UV interleaved planes).
 * @param width Width of the frame.
 * @param height Height of the frame.
 */
void convertYuv420ToNv12(CVI_U8 *yuv420Planes[3], CVI_U8 *nv12Planes[3], int width, int height) {
	int frameSize = width * height; // Total number of pixels in the frame
	CVI_U8 *uPlane = yuv420Planes[1]; // U plane in YUV420
	CVI_U8 *vPlane = yuv420Planes[2]; // V plane in YUV420
	CVI_U8 *nv12UVPlane = nv12Planes[1]; // UV interleaved plane in NV12

	// Interleave U and V planes into NV12 UV plane
	for (int i = 0; i < frameSize / 4; i++) {
		nv12UVPlane[i * 2] = uPlane[i];     // Copy U value
		nv12UVPlane[i * 2 + 1] = vPlane[i]; // Copy V value
	}
}

/**
 * Convert NV21 format to NV12 format.
 *
 * @param nv21Planes Input NV21 data (Y, VU interleaved planes).
 * @param nv12Planes Output NV12 data (Y, UV interleaved planes).
 * @param width Width of the frame.
 * @param height Height of the frame.
 */
void convertNv21ToNv12(CVI_U8 *nv21Planes[3], CVI_U8 *nv12Planes[3], int width, int height) {
	int frameSize = width * height; // Total number of pixels in the frame
	CVI_U8 *nv21VUPlane = nv21Planes[1]; // VU interleaved plane in NV21
	CVI_U8 *nv12UVPlane = nv12Planes[1]; // UV interleaved plane in NV12

	// Convert NV21 VU plane to NV12 UV plane
	for (int i = 0; i < frameSize / 4; i++) {
		nv12UVPlane[i * 2] = nv21VUPlane[i * 2 + 1]; // Copy U value
		nv12UVPlane[i * 2 + 1] = nv21VUPlane[i * 2]; // Copy V value
	}
}
static CVI_S32 cviReadSrcFrame(VIDEO_FRAME_S *pstVFrame, FILE *fp, PIXEL_FORMAT_E enPixelFormatIn)
{
	size_t read_byte;
	CVI_U8 *pFramePtr;
	CVI_U32 rowIndex;
	CVI_U32 u32CbCrReadSrcHeight;
	CVI_U32 bCbWidthShift, bCrWidthShift;
	CVI_U32 bytesToRead; // bytes
	CVI_U8 *pTempBuffer = NULL;
	CVI_U8 *tmpVirAddr[3];

	switch (enPixelFormatIn) {
	case PIXEL_FORMAT_YUV_PLANAR_422:
		u32CbCrReadSrcHeight = pstVFrame->u32Height;
		bCbWidthShift = 1;
		bCrWidthShift = 1;
		break;
	case PIXEL_FORMAT_NV12:
	case PIXEL_FORMAT_NV21:
		u32CbCrReadSrcHeight = pstVFrame->u32Height >> 1;
		bCbWidthShift = 0;
		bCrWidthShift = 31;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_420:
	default:
		u32CbCrReadSrcHeight = pstVFrame->u32Height >> 1;
		bCbWidthShift = 1;
		bCrWidthShift = 1;
		break;
	}

	if (enPixelFormatIn != pstVFrame->enPixelFormat) {
		pTempBuffer = malloc(pstVFrame->u32Stride[0] * pstVFrame->u32Height / 2);
		if (pTempBuffer == NULL) {
			SAMPLE_PRT("malloc buf fail %d \n", pstVFrame->u32Stride[0] * pstVFrame->u32Height / 2);
		}
		tmpVirAddr[0] = pstVFrame->pu8VirAddr[0];
		tmpVirAddr[1] = pTempBuffer;
		tmpVirAddr[2] = tmpVirAddr[1] + pstVFrame->u32Stride[0] * pstVFrame->u32Height / 4;
	} else {
		tmpVirAddr[0] = pstVFrame->pu8VirAddr[0];
		tmpVirAddr[1] = pstVFrame->pu8VirAddr[1];
		tmpVirAddr[2] = pstVFrame->pu8VirAddr[2];
	}

	if (pstVFrame->u32Width == pstVFrame->u32Stride[0]) {
		// Luma
		pFramePtr = tmpVirAddr[0];
		bytesToRead = pstVFrame->u32Width * pstVFrame->u32Height;
		read_byte = fread((void *)pFramePtr, 1, bytesToRead, fp);
		if (read_byte != bytesToRead) {
			SAMPLE_PRT("Luma, fread %zu %d failed\n", read_byte, bytesToRead);
			return CVI_FAILURE;
		}
		// Cb
		pFramePtr = tmpVirAddr[1];
		bytesToRead = (pstVFrame->u32Width * u32CbCrReadSrcHeight) >> bCbWidthShift;
		read_byte = fread((void *)pFramePtr, 1, bytesToRead, fp);
		if (read_byte != bytesToRead) {
			SAMPLE_PRT("Cb, fread %zu %d failed\n", read_byte, bytesToRead);
			return CVI_FAILURE;
		}

		// Cr
		pFramePtr = tmpVirAddr[2];
		bytesToRead = (pstVFrame->u32Width * u32CbCrReadSrcHeight) >> bCrWidthShift;
		read_byte = fread((void *)pFramePtr, 1, bytesToRead, fp);
		if (read_byte != bytesToRead) {
			SAMPLE_PRT("Cr, fread %zu %d failed\n", read_byte, bytesToRead);
			return CVI_FAILURE;
		}
	} else {
		// Luma
		for (rowIndex = 0; rowIndex < pstVFrame->u32Height; rowIndex++) {
			pFramePtr = tmpVirAddr[0] + rowIndex * pstVFrame->u32Stride[0];
			bytesToRead = pstVFrame->u32Width;
			read_byte = fread((void *)pFramePtr, 1, bytesToRead, fp);
			if (read_byte != bytesToRead) {
				SAMPLE_PRT("Luma, (row %d) fread %zu %d failed\n",
					rowIndex, read_byte, bytesToRead);
				return CVI_FAILURE;
			}
		}

		// Cb
		for (rowIndex = 0; rowIndex < u32CbCrReadSrcHeight; rowIndex++) {
			pFramePtr = tmpVirAddr[1] + rowIndex * (pstVFrame->u32Stride[0] >> bCbWidthShift);
			bytesToRead = pstVFrame->u32Width >> bCbWidthShift;
			read_byte = fread((void *)pFramePtr, 1, bytesToRead, fp);
			if (read_byte != bytesToRead) {
				SAMPLE_PRT("Cb, (row %d) fread %zu %d failed\n",
					rowIndex, read_byte, bytesToRead);
				return CVI_FAILURE;
			}
		}

		// Cr
		for (rowIndex = 0; rowIndex < u32CbCrReadSrcHeight; rowIndex++) {
			pFramePtr = tmpVirAddr[2] + rowIndex * (pstVFrame->u32Stride[0] >> bCrWidthShift);
			bytesToRead = pstVFrame->u32Width >> bCrWidthShift;
			read_byte = fread((void *)pFramePtr, 1, bytesToRead, fp);
			if (read_byte != bytesToRead) {
				SAMPLE_PRT("Cr, (row %d) fread %zu %d failed\n",
					rowIndex, read_byte, bytesToRead);
				return CVI_FAILURE;
			}
		}
	}
	if (enPixelFormatIn != pstVFrame->enPixelFormat) {
		switch(enPixelFormatIn) {
		case PIXEL_FORMAT_YUV_PLANAR_420: {
			convertYuv420ToNv12(tmpVirAddr, pstVFrame->pu8VirAddr,
				pstVFrame->u32Stride[0], pstVFrame->u32Height);
		} break;
		case PIXEL_FORMAT_NV21: {
			convertNv21ToNv12(tmpVirAddr, pstVFrame->pu8VirAddr,
				pstVFrame->u32Stride[0], pstVFrame->u32Height);
		} break;
		case PIXEL_FORMAT_NV12: {
			return CVI_SUCCESS;
		} break;
		default:
			SAMPLE_PRT("no support fmt %d\n", enPixelFormatIn);
			return CVI_FAILURE;
		}

		if (pTempBuffer)
			free(pTempBuffer);
	}
	return CVI_SUCCESS;
}

static CVI_VOID exitSysAndVb(CVI_VOID)
{
	CVI_VB_Exit();
	CVI_SYS_Exit();
}

static VIDEO_FRAME_INFO_S *allocate_frame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat)
{
	VIDEO_FRAME_INFO_S *pstVideoFrame;
	VIDEO_FRAME_S *pstVFrame;
	VB_BLK vbBlock;
	VB_CAL_CONFIG_S stVbCfg;

	pstVideoFrame = (VIDEO_FRAME_INFO_S *)calloc(sizeof(*pstVideoFrame), 1);
	if (pstVideoFrame == NULL) {
		SAMPLE_PRT("Failed to allocate VIDEO_FRAME_INFO_S\n");
		return NULL;
	}

	memset(&stVbCfg, 0, sizeof(stVbCfg));
	VENC_GetPicBufferConfig(stSize.u32Width,
				stSize.u32Height,
				enPixelFormat,
				DATA_BITWIDTH_8,
				COMPRESS_MODE_NONE,
				&stVbCfg);

	pstVFrame = &pstVideoFrame->stVFrame;

	pstVFrame->enCompressMode = COMPRESS_MODE_NONE;
	pstVFrame->enPixelFormat = enPixelFormat;
	pstVFrame->enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVFrame->enColorGamut = COLOR_GAMUT_BT709;
	pstVFrame->u32Width = stSize.u32Width;
	pstVFrame->u32Height = stSize.u32Height;
	pstVFrame->u32TimeRef = 0;
	pstVFrame->u64PTS = 0;
	pstVFrame->enDynamicRange = DYNAMIC_RANGE_SDR8;

	if (pstVFrame->u32Width % VENC_ALIGN_W) {
		SAMPLE_PRT("u32Width is not algined to %d\n", VENC_ALIGN_W);
	}

	vbBlock = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCfg.u32VBSize);
	if (vbBlock == VB_INVALID_HANDLE) {
		SAMPLE_PRT("Can't acquire vb block\n");
		free(pstVideoFrame);
		return NULL;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(vbBlock);
	pstVFrame->u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(vbBlock);
	pstVFrame->u32Stride[0] = stVbCfg.u32MainStride;
	pstVFrame->u32Length[0] = stVbCfg.u32MainYSize;
	pstVFrame->pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_Mmap(pstVFrame->u64PhyAddr[0], pstVFrame->u32Length[0]);
	memset(pstVFrame->pu8VirAddr[0], 0, pstVFrame->u32Length[0]);

	if (stVbCfg.plane_num > 1) {
		pstVFrame->u64PhyAddr[1] = ALIGN(pstVFrame->u64PhyAddr[0] + stVbCfg.u32MainYSize, stVbCfg.u16AddrAlign);
		pstVFrame->u32Stride[1] = stVbCfg.u32CStride;
		pstVFrame->u32Length[1] = stVbCfg.u32MainCSize;
		pstVFrame->pu8VirAddr[1] = (CVI_U8 *)CVI_SYS_Mmap(pstVFrame->u64PhyAddr[1], pstVFrame->u32Length[1]);
		memset(pstVFrame->pu8VirAddr[1], 0, pstVFrame->u32Length[1]);
	}

	if (stVbCfg.plane_num > 2) {
		pstVFrame->u64PhyAddr[2] = ALIGN(pstVFrame->u64PhyAddr[1] + stVbCfg.u32MainCSize, stVbCfg.u16AddrAlign);
		pstVFrame->u32Stride[2] = stVbCfg.u32CStride;
		pstVFrame->u32Length[2] = stVbCfg.u32MainCSize;
		pstVFrame->pu8VirAddr[2] = (CVI_U8 *)CVI_SYS_Mmap(pstVFrame->u64PhyAddr[2], pstVFrame->u32Length[2]);
		memset(pstVFrame->pu8VirAddr[2], 0, pstVFrame->u32Length[2]);
	}

	SAMPLE_PRT("phy addr(%#llx, %#llx, %#llx), Size %x\n", (long long)pstVFrame->u64PhyAddr[0]
		, (long long)pstVFrame->u64PhyAddr[1], (long long)pstVFrame->u64PhyAddr[2], stVbCfg.u32VBSize);
	SAMPLE_PRT("vir addr(%p, %p, %p), Size %x\n", pstVFrame->pu8VirAddr[0]
		, pstVFrame->pu8VirAddr[1], pstVFrame->pu8VirAddr[2], stVbCfg.u32MainSize);

	return pstVideoFrame;
}

static CVI_S32 free_frame(VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	VB_BLK vbBlock;

	if (pstVFrame->pu8VirAddr[0])
		CVI_SYS_Munmap((CVI_VOID *)pstVFrame->pu8VirAddr[0], pstVFrame->u32Length[0]);
	if (pstVFrame->pu8VirAddr[1])
		CVI_SYS_Munmap((CVI_VOID *)pstVFrame->pu8VirAddr[1], pstVFrame->u32Length[1]);
	if (pstVFrame->pu8VirAddr[2])
		CVI_SYS_Munmap((CVI_VOID *)pstVFrame->pu8VirAddr[2], pstVFrame->u32Length[2]);

	vbBlock = CVI_VB_PhysAddr2Handle(pstVFrame->u64PhyAddr[0]);
	if (vbBlock != VB_INVALID_HANDLE) {
		CVI_VB_ReleaseBlock(vbBlock);
	}

	free(pstVideoFrame);

	return CVI_SUCCESS;
}

/**
 * Send one frame to the VENC channel.
 *
 * @param pVencContext The VENC channel context.
 * @return CVI_SUCCESS on success, or an error code on failure.
 */
static CVI_S32 SAMPLE_VENC_SendOneFrame(vencChnCtx *pVencContext) {
	VENC_CHN vencChannel = pVencContext->VencChn;
	chnInputCfg *pInputConfig = &pVencContext->chnIc;
	CVI_BOOL isMotionMapEnabled = strlen(pInputConfig->motionMapBinFile) > 0;
	CVI_BOOL isAiMapEnabled = strlen(pInputConfig->aiMapBinFile) > 0;
	CVI_BOOL isQpMapEnabled = strlen(pInputConfig->qpMapCfgFile) > 0;
	CVI_S32 ret = CVI_SUCCESS;

	// Handle user-defined rate control for UBR mode
	if (pInputConfig->rcMode == SAMPLE_RC_UBR) {
		ret = SAMPLE_COMM_VENC_SetUserFrameLevelRc(pInputConfig, vencChannel);
		if (ret != CVI_SUCCESS) {
			SAMPLE_PRT("(Channel %d) Failed to set user frame-level rate control\n", vencChannel);
			return ret;
		}
	}

	// Insert user data if applicable
	SAMPLE_VENC_InsertUserData(vencChannel, pInputConfig);

	// Handle QP map, motion map, and AI map
	if ((isQpMapEnabled && pVencContext->QpMapMode) ||
		isMotionMapEnabled || isAiMapEnabled) {
		USER_FRAME_INFO_S userFrameInfo;

		userFrameInfo.stUserFrame = *pVencContext->pstFrameInfo;
		// Configure QP map
		if (isQpMapEnabled) {
			userFrameInfo.stUserRcInfo.QpMapMode = MAP_MODE_QPMAP;
			userFrameInfo.stUserRcInfo.u64QpMapPhyAddr =
				CVI_VB_Handle2PhysAddr(pVencContext->QpMapBlk);
			SAMPLE_PRT("QP Map Physical Address: %" PRIx64 "\n",
				userFrameInfo.stUserRcInfo.u64QpMapPhyAddr);
		}

		// Configure motion map
		if (isMotionMapEnabled) {
			userFrameInfo.stUserRcInfo.QpMapMode |= MAP_MODE_MOTIONMAP;
			userFrameInfo.stUserRcInfo.u64QpMapPhyAddr =
				CVI_VB_Handle2PhysAddr(pVencContext->QpMapBlk);
		}

		// Configure AI map
		if (isAiMapEnabled) {
			userFrameInfo.stUserRcInfo.QpMapMode |= MAP_MODE_AIMAP;
			userFrameInfo.stUserRcInfo.u64AiMapPhyAddr =
				CVI_VB_Handle2PhysAddr(pVencContext->AiMapBlk);
			SAMPLE_PRT("AI Map Physical Address: %" PRIx64 "\n",
				userFrameInfo.stUserRcInfo.u64AiMapPhyAddr);
		}

		// Send frame with user-defined maps
		ret = CVI_VENC_SendFrameEx(vencChannel, &userFrameInfo, pInputConfig->sendframe_timeout);
	} else {
		// Send frame without user-defined maps
		ret = CVI_VENC_SendFrame(vencChannel, pVencContext->pstFrameInfo, pInputConfig->sendframe_timeout);
	}

	return ret;
}

static CVI_S32 SAMPLE_COMM_VENC_SetUserFrameLevelRc(chnInputCfg *pIc, VENC_CHN VencChn)
{
	VENC_FRAME_PARAM_S stFrameParam, *pstFrameParam = &stFrameParam;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_VENC_GetFrameParam(VencChn, pstFrameParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_GetFrameParam\n");
		return s32Ret;
	}


	// use User-defined RC
	pstFrameParam->u32FrameQp = pIc->u32FrameQp;
	pstFrameParam->u32FrameBits = pIc->bitrate * 1000 / pIc->framerate;

	SAMPLE_PRT("u32FrameQp = %d, u32FrameBits = %d\n",
			pstFrameParam->u32FrameQp, pstFrameParam->u32FrameBits);

	s32Ret = CVI_VENC_SetFrameParam(VencChn, pstFrameParam);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VENC_SetFrameParam fail\n");
		return s32Ret;
	}

	return s32Ret;
}

static CVI_VOID SAMPLE_VENC_InsertUserData(VENC_CHN chn, chnInputCfg *pIc)
{
	CVI_U8 buf[65536] = { 0 };
	FILE *fp;
	int i, len;

	for (i = 0; i < NUM_OF_USER_DATA_BUF; ++i) {
		if (strlen(pIc->user_data[i]) == 0)
			continue;

		fp = fopen(pIc->user_data[i], "rb");
		if (!fp)
			continue;

		do {
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			if (len <= 0 || len > 65536)
				break;

			fseek(fp, 0, SEEK_SET);
			fread(buf, 1, len, fp);
			CVI_VENC_InsertUserData(chn, buf, len);
		} while (0);

		fclose(fp);
	}
}
#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic pop
#endif
