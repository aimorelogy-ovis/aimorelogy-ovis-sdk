#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <cvi_type.h>
#include <cvi_comm_video.h>
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"

#include "sc202l_cmos_ex.h"
#include "sc202l_cmos_param.h"

#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define SC202L_ID 202
#define SC202L_I2C_ADDR 0x30
#define SC202L_I2C_ADDR_IS_VALID(addr)	((addr) == SC202L_I2C_ADDR)
/****************************************************************************
 * global variables                                                            *
 ****************************************************************************/

ISP_SNS_STATE_S *g_pastSC202L[VI_MAX_PIPE_NUM] = {CVI_NULL};
SNS_COMBO_DEV_ATTR_S *g_pastSC202LComboDevArray[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SC202L_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastSC202L[dev])
#define SC202L_SENSOR_SET_CTX(dev, pstCtx)   (g_pastSC202L[dev] = pstCtx)
#define SC202L_SENSOR_RESET_CTX(dev)         (g_pastSC202L[dev] = CVI_NULL)
#define SC202L_SENSOR_GET_COMBO(dev, pstCtx)   (pstCtx = g_pastSC202LComboDevArray[dev])
#define SC202L_SENSOR_SET_COMBO(dev, pstCtx)   (g_pastSC202LComboDevArray[dev] = pstCtx)

ISP_SNS_COMMBUS_U g_aunSC202L_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 2},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_COMMADDR_U g_aunSC202L_AddrInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cAddr = 0},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cAddr = -1}
};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc020hgs_MirrorFip[VI_MAX_PIPE_NUM] = {0};

CVI_U16 g_au16SC202L_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16SC202L_L2SMode[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                            *
 ****************************************************************************/
static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****SC202L Lines Range*****/
#define SC202L_FULL_LINES_MAX  (0xFFF0)

/*****SC202L Register Address*****/
#define SC202L_EXP_H_ADDR                     (0x3e00)
#define SC202L_EXP_M_ADDR                     (0x3e01)
#define SC202L_EXP_L_ADDR                     (0x3e02)

#define SC202L_AGAIN_ADDR                     (0x3e09)
#define SC202L_DGAIN_ADDR                     (0x3e06)
#define SC202L_D_FINEGAIN_ADDR                (0x3e07)


#define SC202L_VMAX_H_ADDR                    (0x320e)
#define SC202L_VMAX_L_ADDR                    (0x320f)
#define SC202L_GROUP_HOLD_ADDR              (0x3812)

#define SC202L_RES_IS_1080P(w, h)      ((w) == 1920 && (h) == 1080)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	const SC202L_MODE_S *pstMode;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstMode = &g_astSC202L_mode[pstSnsState->u8ImgMode];

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SC202L_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd *
		g_astSC202L_mode[pstSnsState->u8ImgMode].f32MaxFps);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;

	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;

	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd *
			g_astSC202L_mode[pstSnsState->u8ImgMode].f32MaxFps / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	pstAeSnsDft->u32SnsStableFrame = 0;

	switch (pstSnsState->enWDRMode) {
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = pstMode->f32MaxFps;
		pstAeSnsDft->f32MinFps = pstMode->f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = pstMode->stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = pstMode->stAgain[0].u32Min;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = pstMode->stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = pstMode->stDgain[0].u32Min;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ? g_au32InitExposure[ViPipe] : 76151;

		pstAeSnsDft->u32MaxIntTime = pstMode->stExp[0].u16Max;
		pstAeSnsDft->u32MinIntTime = pstMode->stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = pstAeSnsDft->u32MaxIntTime;
		pstAeSnsDft->u32MinIntTimeTarget = pstAeSnsDft->u32MinIntTime;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

/* the function of sensor set fps */
static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32VMAX;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Vts = g_astSC202L_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSC202L_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSC202L_mode[pstSnsState->u8ImgMode].f32MinFps;

	switch (pstSnsState->u8ImgMode) {
	case SC202L_MODE_1920X1080P30_MASTER:
	case SC202L_MODE_1920X1080P30_SLAVE:
		if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
			u32VMAX = u32Vts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support Fps: %f\n", f32Fps);
			return CVI_FAILURE;
		}
		u32VMAX = (u32VMAX > SC202L_FULL_LINES_MAX) ? SC202L_FULL_LINES_MAX : u32VMAX;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support sensor mode: %d\n", pstSnsState->u8ImgMode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = u32VMAX;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_H_ADDR].u32Data = ((u32VMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_L_ADDR].u32Data = (u32VMAX & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	pstAeSnsDft->f32Fps = f32Fps;
	pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * f32Fps / 2;
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 4;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32TmpIntTime, u32MinTime, u32MaxTime, u32TmpIntTimeReg;

	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear exposure reg range:
		 * min : 1
		 * max : vts - 4
		 * step : 1
		 */
		u32MinTime = 1;
		u32MaxTime = (pstSnsState->au32FL[0]) -4;

		u32TmpIntTime = (u32IntTime[0] > u32MaxTime) ? u32MaxTime : u32IntTime[0];
		u32TmpIntTime = (u32TmpIntTime < u32MinTime) ? u32MinTime : u32TmpIntTime;

		u32TmpIntTimeReg = u32TmpIntTime << 1;
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_H_ADDR].u32Data = (u32TmpIntTimeReg >> 12) & 0x0F;
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_M_ADDR].u32Data = (u32TmpIntTimeReg >> 4) & 0xFF;
		pstSnsRegsInfo->astI2cData[LINEAR_EXP_L_ADDR].u32Data = ((u32TmpIntTimeReg & 0x0F) << 4);

	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;

}

#define MAX_AGAIN_STEP		4

struct gain_tbl_info_s {
	CVI_U32	gainMax;
	CVI_U32	gainMin;
	CVI_U32	idxBase;
	CVI_U8	regGain;
	CVI_U8	regGainFineBase;
	CVI_U8	regGainFineStep;
};

static struct gain_tbl_info_s AgainInfo[] = {
	{
		/* 1x -> 1.73x */
		.gainMin = 1024,
		.gainMax = 1772,
		.idxBase = 0,
		.regGain = 0x00,
	},
	{
		/* 1.73x -> 3.62x */
		.gainMin = 1772,
		.gainMax = 3707,
		.idxBase = 64,
		.regGain = 0x40,
	},
	{
		/* 3.62x -> 7.24x */
		.gainMin = 3707,
		.gainMax = 7414,
		.idxBase = 128,
		.regGain = 0x48,
	},
	{
		/* 7.24x -> 14.48x */
		.gainMin = 7414,
		.gainMax = 14828,
		.idxBase = 192,
		.regGain = 0x49,
	},
	{
		/* 14.48x -> 28.96x */
		.gainMin = 14828,
		.gainMax = 29655,
		.idxBase = 256,
		.regGain = 0x4B,
	},
	{
		/* 28.96x -> 57.92x */
		.gainMin = 29655,
		.gainMax = 58839,
		.idxBase = 320,
		.regGain = 0x4F,
	},
	{
		/* 57.92x (max) */
		.gainMin = 59310,
		.gainMax = 59310,
		.idxBase = 383,
		.regGain = 0x5F,
	},
};

/*
 * Again_table，
 *   1x -> 1.81x    步长 1/64
 *   1.81x -> 3.62x 步长 1/64
 *   3.62x -> 7.24x 步长 1/64
 *   7.24x -> 14.48x 步长 1/64
 *   14.48x -> 28.96x 步长 1/64
 *   28.96x -> 57.92x 步长 1/64
 * 底数单位为 1x=1024，表中数值均为 round(1024 * 增益)。
 */
static const uint32_t Again_table[] = {
	/* 1x -> 1.81x (linear, 64 points) */
	1024, 1037, 1050, 1063, 1077, 1090, 1103, 1116,
	1129, 1142, 1156, 1169, 1182, 1195, 1208, 1221,
	1235, 1248, 1261, 1274, 1287, 1300, 1314, 1327,
	1340, 1353, 1366, 1379, 1393, 1406, 1419, 1432,
	1445, 1458, 1472, 1485, 1498, 1511, 1524, 1537,
	1551, 1564, 1577, 1590, 1603, 1616, 1630, 1643,
	1656, 1669, 1682, 1695, 1709, 1722, 1735, 1748,
	1761, 1774, 1788, 1801, 1814, 1827, 1840, 1853,

	/* 1.81x -> 3.62x (linear, 64 points) */
	1853, 1883, 1912, 1942, 1971, 2001, 2030, 2059,
	2089, 2118, 2148, 2177, 2206, 2236, 2265, 2295,
	2324, 2354, 2383, 2412, 2442, 2471, 2501, 2530,
	2560, 2589, 2618, 2648, 2677, 2707, 2736, 2765,
	2795, 2824, 2854, 2883, 2913, 2942, 2971, 3001,
	3030, 3060, 3089, 3118, 3148, 3177, 3207, 3236,
	3266, 3295, 3324, 3354, 3383, 3413, 3442, 3472,
	3501, 3530, 3560, 3589, 3619, 3648, 3677, 3707,

	/* 3.62x -> 7.24x (linear, 64 points) */
	3707, 3766, 3825, 3883, 3942, 4001, 4060, 4119,
	4178, 4236, 4295, 4354, 4413, 4472, 4531, 4589,
	4648, 4707, 4766, 4825, 4884, 4943, 5001, 5060,
	5119, 5178, 5237, 5296, 5354, 5413, 5472, 5531,
	5590, 5649, 5707, 5766, 5825, 5884, 5943, 6002,
	6060, 6119, 6178, 6237, 6296, 6355, 6413, 6472,
	6531, 6590, 6649, 6708, 6767, 6825, 6884, 6943,
	7002, 7061, 7120, 7178, 7237, 7296, 7355, 7414,

	/* 7.24x -> 14.48x (linear, 64 points) */
	7414, 7531, 7649, 7767, 7884, 8002, 8120, 8238,
	8355, 8473, 8591, 8708, 8826, 8944, 9061, 9179,
	9297, 9414, 9532, 9650, 9767, 9885, 10003, 10120,
	10238, 10356, 10473, 10591, 10709, 10826, 10944, 11062,
	11179, 11297, 11415, 11533, 11650, 11768, 11886, 12003,
	12121, 12239, 12356, 12474, 12592, 12709, 12827, 12945,
	13062, 13180, 13298, 13415, 13533, 13651, 13768, 13886,
	14004, 14121, 14239, 14357, 14474, 14592, 14710, 14828,

	/* 14.48x -> 28.96x (linear, 64 points) */
	14828, 15063, 15298, 15534, 15769, 16004, 16240, 16475,
	16710, 16946, 17181, 17416, 17652, 17887, 18123, 18358,
	18593, 18829, 19064, 19299, 19535, 19770, 20005, 20241,
	20476, 20711, 20947, 21182, 21418, 21653, 21888, 22124,
	22359, 22594, 22830, 23065, 23300, 23536, 23771, 24006,
	24242, 24477, 24713, 24948, 25183, 25419, 25654, 25889,
	26125, 26360, 26595, 26831, 27066, 27301, 27537, 27772,
	28008, 28243, 28478, 28714, 28949, 29184, 29420, 29655,

	/* 28.96x -> 57.92x (linear, 64 points) */
	29655, 30126, 30596, 31067, 31538, 32009, 32479, 32950,
	33421, 33891, 34362, 34833, 35304, 35774, 36245, 36716,
	37186, 37657, 38128, 38599, 39069, 39540, 40011, 40481,
	40952, 41423, 41894, 42364, 42835, 43306, 43776, 44247,
	44718, 45189, 45659, 46130, 46601, 47071, 47542, 48013,
	48484, 48954, 49425, 49896, 50366, 50837, 51308, 51779,
	52249, 52720, 53191, 53662, 54132, 54603, 55074, 55544,
	56015, 56486, 56957, 57427, 57898, 58369, 58839, 59310
};

static struct gain_tbl_info_s DgainInfo[3] = {
	{
		.gainMin = 1024,
		.gainMax = 2016,
		.idxBase = 0,
		.regGain = 0x00,
		.regGainFineBase = 0x80,
		.regGainFineStep = 0x04,
	},
	{
		.gainMin = 2048,
		.gainMax = 4032,
		.idxBase = 32,
		.regGain = 0x01,
		.regGainFineBase = 0x80,
		.regGainFineStep = 0x04,
	},
	{
		.gainMin = 4096,
		.gainMax = 4096,
		.idxBase = 64,
		.regGain = 0x03,
		.regGainFineBase = 0x80,
		.regGainFineStep = 0x04,
	},
};

static const uint32_t Dgain_table[] = {
	/* 1x -> 4x (step = 1/32) */
	1024, 1056, 1088, 1120, 1152, 1184, 1216, 1248,
	1280, 1312, 1344, 1376, 1408, 1440, 1472, 1504,
	1536, 1568, 1600, 1632, 1664, 1696, 1728, 1760,
	1792, 1824, 1856, 1888, 1920, 1952, 1984, 2016,
	2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496,
	2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008,
	3072, 3136, 3200, 3264, 3328, 3392, 3456, 3520,
	3584, 3648, 3712, 3776, 3840, 3904, 3968, 4032,
	4096
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	int i, total;

	(void) ViPipe;
	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);
	total = sizeof(Again_table) / sizeof(CVI_U32);

	if (*pu32AgainLin >= Again_table[total - 1]) {
		*pu32AgainLin = Again_table[total - 1];
		*pu32AgainDb = total - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < total; i++) {
		if (*pu32AgainLin < Again_table[i]) {
			*pu32AgainLin = Again_table[i - 1];
			*pu32AgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	int i, total;

	(void) ViPipe;
	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);
	total = sizeof(Dgain_table) / sizeof(CVI_U32);

	if (*pu32DgainLin >= Dgain_table[total - 1]) {
		*pu32DgainLin = Dgain_table[total - 1];
		*pu32DgainDb = total - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < total; i++) {
		if (*pu32DgainLin < Dgain_table[i]) {
			*pu32DgainLin = Dgain_table[i - 1];
			*pu32DgainDb = i - 1;
			break;
		}
	}
	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32AgainIdx;
	CVI_U32 u32DgainIdx;
	CVI_U32 u32Again;
	CVI_U32 Aagain_val;
	CVI_U32 Adgain_val;
	CVI_U32 Dgain_val;
	CVI_U32 again_table_size;

	struct gain_tbl_info_s *info, *info_dgain;
	int i, tbl_num;

	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32AgainIdx = pu32Again[0];
	u32DgainIdx = pu32Dgain[0];

	again_table_size = sizeof(Again_table) / sizeof(CVI_U32);
	if (u32AgainIdx < (again_table_size - 1) && u32DgainIdx > 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Invalid gain setting: u32AgainIdx=%u (max=%u), u32DgainIdx=%u. "
			"u32DgainIdx should be 0 when u32AgainIdx hasn't reached maximum.\n",
			u32AgainIdx, again_table_size - 1, u32DgainIdx);
		return CVI_FAILURE;
	}

	/* linear mode */

	/* find Again register setting. */
	tbl_num = sizeof(AgainInfo)/sizeof(struct gain_tbl_info_s);
	for (i = tbl_num - 1; i >= 0; i--) {
		info = &AgainInfo[i];

		if (u32AgainIdx >= info->idxBase)
			break;
	}

	pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_ADDR].u32Data = (info->regGain & 0xFF);
	u32Again = Again_table[u32AgainIdx];
	Aagain_val = info->gainMin;
	{
		double ratio = (double)u32Again / (double)Aagain_val;
		double ratio_scaled;
		int fine_steps;

		if (ratio >= 2.0) {
			info_dgain = &DgainInfo[1]; /* 2x ~ 4x */
			ratio_scaled = ratio / 2.0;
		} else {
			info_dgain = &DgainInfo[0]; /* 1x ~ 2x */
			ratio_scaled = ratio;
		}

		fine_steps = (int)((ratio_scaled - 1.0) * 32.0);
		if (fine_steps < 0)
			fine_steps = 0;
		if (fine_steps > 31)
			fine_steps = 31;

		Adgain_val = info_dgain->regGainFineBase +
			fine_steps * info_dgain->regGainFineStep;
	}
	pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR].u32Data = (info_dgain->regGain & 0xFF);
	pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_FINE_ADDR].u32Data = (Adgain_val & 0xFF);

	if (u32DgainIdx > 0) {
		/* find Dgain register setting. */
		tbl_num = sizeof(DgainInfo)/sizeof(struct gain_tbl_info_s);
		for (i = tbl_num - 1; i >= 0; i--) {
			info = &DgainInfo[i];

			if (u32DgainIdx >= info->idxBase)
				break;
		}

		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_ADDR].u32Data = (info->regGain & 0xFF);
		Dgain_val = info->regGainFineBase + (u32DgainIdx - info->idxBase) * info->regGainFineStep;
		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_FINE_ADDR].u32Data = (Dgain_val & 0xFF);
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
	//pstExpFuncs->pfn_cmos_slow_framerate_set = cmos_slow_framerate_set;
	pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;
	//pstExpFuncs->pfn_cmos_get_inttime_max   = cmos_get_inttime_max;
	//pstExpFuncs->pfn_cmos_ae_fswdr_attr_set = cmos_ae_fswdr_attr_set;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
	(void) ViPipe;

	CMOS_CHECK_POINTER(pstAwbSnsDft);

	memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

	pstAwbSnsDft->u16InitGgain = 1024;
	pstAwbSnsDft->u8AWBRunInterval = 1;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_isp_default(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef)
{
	(void) ViPipe;

	memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_blc_default(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlc)
{
	(void) ViPipe;

	CMOS_CHECK_POINTER(pstBlc);

	memset(pstBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));

	memcpy(pstBlc, &g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const SC202L_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSC202L_mode[pstSnsState->u8ImgMode];

	pstIspCfg->frm_num = 1;
	memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	UNUSED(ViPipe);
	UNUSED(u8Mode);
	CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport sensor mode!\n");
	return CVI_SUCCESS;

}

static CVI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsSyncInfo)
{
	CVI_U32 i;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg0 = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg1 = CVI_NULL;
	ISP_I2C_DATA_S *pstI2c_data = CVI_NULL;

	CMOS_CHECK_POINTER(pstSnsSyncInfo);
	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSC202L_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 1;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = g_aunSC202L_AddrInfo[ViPipe].s8I2cAddr;
			pstI2c_data[i].u32AddrByteNum = sc202l_addr_byte;
			pstI2c_data[i].u32DataByteNum = sc202l_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			//Linear Mode Regs
			pstI2c_data[LINEAR_HOLD_START].u32RegAddr     = SC202L_GROUP_HOLD_ADDR;
			pstI2c_data[LINEAR_HOLD_START].u32Data = 0x00;
			pstI2c_data[LINEAR_EXP_H_ADDR].u32RegAddr     = SC202L_EXP_H_ADDR;
			pstI2c_data[LINEAR_EXP_M_ADDR].u32RegAddr     = SC202L_EXP_M_ADDR;
			pstI2c_data[LINEAR_EXP_L_ADDR].u32RegAddr     = SC202L_EXP_L_ADDR;

			pstI2c_data[LINEAR_AGAIN_ADDR].u32RegAddr      = SC202L_AGAIN_ADDR;
			pstI2c_data[LINEAR_DGAIN_ADDR].u32RegAddr      = SC202L_DGAIN_ADDR;
			pstI2c_data[LINEAR_DGAIN_FINE_ADDR].u32RegAddr = SC202L_D_FINEGAIN_ADDR;

			pstI2c_data[LINEAR_VMAX_H_ADDR].u32RegAddr     = SC202L_VMAX_H_ADDR;
			pstI2c_data[LINEAR_VMAX_L_ADDR].u32RegAddr     = SC202L_VMAX_L_ADDR;
			pstI2c_data[LINEAR_HOLD_END].u32RegAddr     = SC202L_GROUP_HOLD_ADDR;
			pstI2c_data[LINEAR_HOLD_END].u32Data = 0x30;
			break;
		default:
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
		pstSnsState->bSyncInit = CVI_TRUE;
		pstCfg0->snsCfg.need_update = CVI_TRUE;
		/* recalcualte WDR size */
		cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
		pstCfg0->ispCfg.need_update = CVI_TRUE;
	} else {
		pstCfg0->snsCfg.need_update = CVI_FALSE;
		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			if (pstCfg0->snsCfg.astI2cData[i].u32Data == pstCfg1->snsCfg.astI2cData[i].u32Data) {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_FALSE;
			} else {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.need_update = CVI_TRUE;
			}
		}
	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC202L_RES_IS_1080P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height)) {
				if (pstSensorImageMode->u8EnableMaster == ISP_SNS_MASTER_BY_FSYNC) {
				u8SensorImageMode = SC202L_MODE_1920X1080P30_MASTER;
				} else if (pstSensorImageMode->u8EnableMaster == ISP_SNS_SLAVE_BY_FSYNC) {
					u8SensorImageMode = SC202L_MODE_1920X1080P30_SLAVE;
				}
			} else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
				       pstSensorImageMode->u16Width,
				       pstSensorImageMode->u16Height,
				       pstSensorImageMode->f32Fps,
				       pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
			       pstSensorImageMode->u16Width,
			       pstSensorImageMode->u16Height,
			       pstSensorImageMode->f32Fps,
			       pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);
	/* Apply the setting on the fly  */
	if (pstSnsState->bInit == CVI_TRUE && g_aeSc020hgs_MirrorFip[ViPipe] != eSnsMirrorFlip) {
		sc202l_mirror_flip(ViPipe, eSnsMirrorFlip);
		g_aeSc020hgs_MirrorFip[ViPipe] = eSnsMirrorFlip;
	}
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	const SC202L_MODE_S *pstMode = CVI_NULL;

	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SC202L_MODE_1920X1080P30_MASTER;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstMode = &g_astSC202L_mode[pstSnsState->u8ImgMode];
	pstSnsState->u32FLStd  = pstMode->u32VtsDef;
	pstSnsState->au32FL[0] = pstMode->u32VtsDef;
	pstSnsState->au32FL[1] = pstMode->u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	SNS_COMBO_DEV_ATTR_S *pstRxAttrSrc = CVI_NULL;

	SC202L_SENSOR_GET_CTX(ViPipe, pstSnsState);
	SC202L_SENSOR_GET_COMBO(ViPipe, pstRxAttrSrc);

	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);
	CMOS_CHECK_POINTER(pstRxAttrSrc);

	memcpy(pstRxAttr, pstRxAttrSrc, sizeof(*pstRxAttr));

	pstRxAttr->img_size.start_x = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32X;
	pstRxAttr->img_size.start_y = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.s32Y;
	pstRxAttr->img_size.active_w = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Width;
	pstRxAttr->img_size.active_h = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stWndRect.u32Height;
	pstRxAttr->img_size.width = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	pstRxAttr->img_size.max_width = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Width;
	pstRxAttr->img_size.max_height = g_astSC202L_mode[pstSnsState->u8ImgMode].astImg[0].stMaxSize.u32Height;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;
	}
	pstRxAttrSrc = CVI_NULL;
	return CVI_SUCCESS;
}

static CVI_S32 sensor_patch_rx_attr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxInitAttr)
{
	int i;
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));

	if (!g_pastSC202LComboDevArray[ViPipe]) {
		pstRxAttr = malloc(sizeof(SNS_COMBO_DEV_ATTR_S));
	} else {
		SC202L_SENSOR_GET_COMBO(ViPipe, pstRxAttr);
	}
	memcpy(pstRxAttr, &sc202l_rx_attr, sizeof(SNS_COMBO_DEV_ATTR_S));
	SC202L_SENSOR_SET_COMBO(ViPipe, pstRxAttr);

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->MipiDev >= VI_MAX_DEV_NUM)
		return CVI_SUCCESS;

	pstRxAttr->devno = pstRxInitAttr->MipiDev;
	pstRxAttr->cif_mode = pstRxInitAttr->MipiMode;

	if (pstRxAttr->input_mode == INPUT_MODE_MIPI) {
		struct mipi_dev_attr_s *attr = &pstRxAttr->mipi_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	} else {
		struct lvds_dev_attr_s *attr = &pstRxAttr->lvds_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	}
	pstRxAttr = CVI_NULL;
	return CVI_SUCCESS;
}

void sc202l_exit(VI_PIPE ViPipe)
{
	if (g_pastSC202LComboDevArray[ViPipe]) {
		free(g_pastSC202LComboDevArray[ViPipe]);
		g_pastSC202LComboDevArray[ViPipe] = CVI_NULL;
	}
	sc202l_i2c_exit(ViPipe);
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = sc202l_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = sc202l_exit;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = sensor_global_init;
	pstSensorExpFunc->pfn_cmos_set_image_mode = cmos_set_image_mode;
	pstSensorExpFunc->pfn_cmos_set_wdr_mode = cmos_set_wdr_mode;

	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_get_isp_black_level = cmos_get_blc_default;
	pstSensorExpFunc->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;

	return CVI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
static CVI_VOID sensor_patch_i2c_addr(VI_PIPE ViPipe, CVI_S32 s32I2cAddr)
{
	if (SC202L_I2C_ADDR_IS_VALID(s32I2cAddr))
		g_aunSC202L_AddrInfo[ViPipe].s8I2cAddr = s32I2cAddr;
	else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C addr input error ,please check [0x%x]\n", s32I2cAddr);
		g_aunSC202L_AddrInfo[ViPipe].s8I2cAddr = SC202L_I2C_ADDR;
	}
}

static CVI_S32 sc202l_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunSC202L_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC202L_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SC202L_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC202L_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SC202L_SENSOR_RESET_CTX(ViPipe);
	g_aeSc020hgs_MirrorFip[ViPipe] = ISP_SNS_NORMAL;
}

static CVI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret;
	ISP_SENSOR_REGISTER_S stIspRegister;
	AE_SENSOR_REGISTER_S  stAeRegister;
	AWB_SENSOR_REGISTER_S stAwbRegister;
	ISP_SNS_ATTR_INFO_S   stSnsAttrInfo;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = sensor_ctx_init(ViPipe);

	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	stSnsAttrInfo.eSensorId = SC202L_ID;

	s32Ret  = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
	s32Ret |= CVI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_ae_exp_function(&stAeRegister.stAeExp);
	s32Ret |= CVI_AE_SensorRegCallBack(ViPipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_awb_exp_function(&stAwbRegister.stAwbExp);
	s32Ret |= CVI_AWB_SensorRegCallBack(ViPipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to awb lib failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SC202L_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SC202L_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SC202L_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
		return s32Ret;
	}

	sensor_ctx_exit(ViPipe);

	return CVI_SUCCESS;
}

static CVI_S32 sensor_set_init(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	CMOS_CHECK_POINTER(pstInitAttr);

	g_au32InitExposure[ViPipe] = pstInitAttr->u32Exposure;
	g_au32LinesPer500ms[ViPipe] = pstInitAttr->u32LinesPer500ms;
	g_au16InitWBGain[ViPipe][0] = pstInitAttr->u16WBRgain;
	g_au16InitWBGain[ViPipe][1] = pstInitAttr->u16WBGgain;
	g_au16InitWBGain[ViPipe][2] = pstInitAttr->u16WBBgain;
	g_au16SampleRgain[ViPipe] = pstInitAttr->u16SampleRgain;
	g_au16SampleBgain[ViPipe] = pstInitAttr->u16SampleBgain;
	g_au16SC202L_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16SC202L_L2SMode[ViPipe] = pstInitAttr->enL2SMode;

	return CVI_SUCCESS;
}

ISP_SNS_OBJ_S stSnsSC202L_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = sc202l_standby,
	.pfnRestart             = sc202l_restart,
	.pfnMirrorFlip          = sensor_mirror_flip,
	.pfnWriteReg            = sc202l_write_register,
	.pfnReadReg             = sc202l_read_register,
	.pfnSetBusInfo          = sc202l_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnPatchRxAttr		= sensor_patch_rx_attr,
	.pfnPatchI2cAddr	= sensor_patch_i2c_addr,
	.pfnGetRxAttr		= sensor_rx_attr,
	.pfnExpSensorCb		= cmos_init_sensor_exp_function,
	.pfnExpAeCb		= cmos_init_ae_exp_function,
	.pfnSnsProbe		= sc202l_probe,
};

