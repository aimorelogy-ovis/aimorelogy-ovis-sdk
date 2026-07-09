#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <time.h>

#include "fontmod.h"
#include "sample_comm.h"

#define  tiger_bmp  "res/tiger.bmp"
#define  tiger_8bitmode "res/tiger_8bitmode.bmp"
#define  test_bmp tiger_bmp
#define IsASCII(a)				(((a) <= 0x7F) ? 1 : 0)
#define BYTE_BITS				8
#define NOASCII_CHARACTER_BYTES	2
#define OSD_LIB_FONT_W			24
#define OSD_LIB_FONT_H			24

static SAMPLE_VI_CONFIG_S stViConfig;
static SAMPLE_VO_CONFIG_S stVoConfig;
static SNS_INI_CFG_S gstSnsIniCfg;
CVI_CHAR *Path_BMP;
#define MAX_STR_LEN  (64)


void SAMPLE_REGION_Usage(char *sPrgNm)
{
	SAMPLE_PRT("Usage : %s <index>\n", sPrgNm);
	SAMPLE_PRT("index:\n");
	SAMPLE_PRT("\t 0)VPSS OSD.\n");
	SAMPLE_PRT("\t 1)VPSS COVER.\n");
	SAMPLE_PRT("\t 2)VPSS OSD TIME.\n");
	SAMPLE_PRT("\t 3)VPSS OSD 8bit mode OVERLAY.\n");
	SAMPLE_PRT("\t 4)VPSS OSD objects OVERLAY.\n");
	SAMPLE_PRT("\t 5)VPSS COVEREX.\n");
	SAMPLE_PRT("\t 6)VPSS MOSAIC.\n");
}

void SAMPLE_REGION_HandleSig(CVI_S32 signo)
{
	CVI_BOOL abChnEnable[VPSS_MAX_CHN_NUM] = {CVI_TRUE, };

	if (SIGINT == signo || SIGTERM == signo) {
		//SAMPLE_COMM_All_ISP_Stop();
		SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
		SAMPLE_COMM_VO_StopVO(&stVoConfig);
		SAMPLE_COMM_SYS_Exit();
		SAMPLE_PRT("\033[0;35mprogram termination abnormally!\033[0;39m\n");
	}
	exit(-1);
}

static int SAMPLE_REGION_GetFontMod(char *Character, CVI_U8 **FontMod, int *FontModLen)
{
	CVI_U32 offset = 0;
	CVI_U32 areacode = 0;
	CVI_U32 bitcode = 0;

	if (IsASCII(Character[0])) {
		areacode = 3;
		bitcode = (CVI_U32)((CVI_U8)Character[0] - 0x20);
	} else {
		areacode = (CVI_U32)((CVI_U8)Character[0] - 0xA0);
		bitcode = (CVI_U32)((CVI_U8)Character[1] - 0xA0);
	}
	offset = (94 * (areacode - 1) + (bitcode - 1)) * (OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8);
	*FontMod = (CVI_U8 *)g_fontLib + offset;
	*FontModLen = OSD_LIB_FONT_W*OSD_LIB_FONT_H / 8;
	return CVI_SUCCESS;
}

static int SAMPLE_REGION_GetNonASCNum(char *string, int len)
{
	int i;
	int n = 0;

	for (i = 0; i < len; i++) {
		if (string[i] == '\0')
			break;
		if (!IsASCII(string[i])) {
			i++;
			n++;
		}
	}

	return n;
}

static void SAMPLE_REGION_GetTimeStr(const struct tm *pstTime, char *pazStr, int s32Len)
{
	time_t nowTime;
	struct tm stTime = {
		0,
	};

	if (!pstTime) {
		time(&nowTime);
		localtime_r(&nowTime, &stTime);
		pstTime = &stTime;
	}

	snprintf(pazStr, s32Len, "%04d-%02d-%02d %02d:%02d:%02d",
		pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday,
		pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
}

static int SAMPLE_REGION_UpdateBitmap(RGN_HANDLE RgnHdl, char *szStr, BITMAP_S *pstBitmap, CVI_U32 *pu32Color)
{
	int s32Ret;
	CVI_U32 u32CanvasWidth, u32CanvasHeight, u32BgColor, u32Color;
	SIZE_S stFontSize;
	int s32StrLen = strnlen(szStr, MAX_STR_LEN);
	int NonASCNum = SAMPLE_REGION_GetNonASCNum(szStr, s32StrLen);

	u32CanvasWidth = OSD_LIB_FONT_W * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
	u32CanvasHeight = OSD_LIB_FONT_H;
	stFontSize.u32Width = OSD_LIB_FONT_W;
	stFontSize.u32Height = OSD_LIB_FONT_H;
	u32BgColor = 0x7fff;

	pstBitmap->u32Width = u32CanvasWidth;
	pstBitmap->u32Height = u32CanvasHeight;
	pstBitmap->pData = malloc(2 * (pstBitmap->u32Width) * (pstBitmap->u32Height));
	if (pstBitmap->pData == NULL)
		SAMPLE_PRT("malloc osd memroy err!\n");

	CVI_U16 *puBmData = (CVI_U16 *)pstBitmap->pData;
	CVI_U32 u32BmRow, u32BmCol;

	for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
		int NonASCShow = 0;

		for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
			int s32BmDataIdx = u32BmRow * pstBitmap->u32Width + u32BmCol;
			int s32CharIdx = u32BmCol / stFontSize.u32Width;
			int s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);

			if (NonASCNum > 0 && s32CharIdx > 0) {
				NonASCShow = SAMPLE_REGION_GetNonASCNum(szStr, s32StringIdx);
				s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
			}
			int s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
							stFontSize.u32Width;
			int s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
			int s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
			int s32BitOffset = s32CharCol % BYTE_BITS;
			CVI_U8 *FontMod = NULL;
			int FontModLen = 0;

			if (SAMPLE_REGION_GetFontMod(&szStr[s32StringIdx], &FontMod, &FontModLen) == CVI_SUCCESS) {
				if (FontMod != NULL && s32HexOffset < FontModLen) {
					CVI_U8 temp = FontMod[s32HexOffset];

					u32Color = *(pu32Color + s32CharIdx);
					if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1)
						puBmData[s32BmDataIdx] = (CVI_U16)u32Color;
					else
						puBmData[s32BmDataIdx] = (CVI_U16)u32BgColor;
					continue;
				}
			}
			SAMPLE_PRT("GetFontMod Fail\n");
			return CVI_FAILURE;
		}
	}

	s32Ret = CVI_RGN_SetBitMap(RgnHdl, pstBitmap);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	free(pstBitmap->pData);

	return s32Ret;
}

CVI_S32 SAMPLE_VIO_SYS_INIT(SAMPLE_VI_CONFIG_S *pstViConfig, SNS_INI_CFG_S *pstSnsIniCfg, VB_CONFIG_S *pstVbConfig)
{
	CVI_S32		s32Ret = CVI_SUCCESS;

	/************************************************
	 * ipcm init for dual os
	 ************************************************/
	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("[ERROR] CVI_SYS_Init failed!\n");
		return s32Ret;
	}
	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_INI_INIT(pstViConfig, pstSnsIniCfg, pstVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_INI_INIT failed with %#x\n", s32Ret);
		return s32Ret;
	}
	memcpy(&gstSnsIniCfg, pstSnsIniCfg, sizeof(SNS_INI_CFG_S));

	/************************************************
	 * step2:  Init modules
	 ************************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(pstVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_Init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_VIO_SET_VI_VPSS_MODE(VI_VPSS_MODE_E vivpssMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_VPSS_MODE_S	stVIVPSSMode;
	VPSS_MODE_S stVPSSMode;

	/************************************************
	 * Config vpss online or offline mode
	 ************************************************/
	stVIVPSSMode.aenMode[0] = stVIVPSSMode.aenMode[1] = vivpssMode;

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	if (vivpssMode == VI_OFFLINE_VPSS_ONLINE ||
	    vivpssMode == VI_ONLINE_VPSS_ONLINE ||
	    vivpssMode == VI_SLICE_VPSS_ONLINE) {
		stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;
	} else {
		stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;
	}

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_VIO_VI_INIT(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i = 0;

	/************************************************
	 * Set sns reset, probe; Set MIPI attr
	 ************************************************/
	SAMPLE_COMM_VI_StartMIPI(pstViConfig);

	for (i = 0; i < gstSnsIniCfg.devNum; i++) {
		if (CVI_SNS_SetSnsProbe(i) != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] sensor_%d probe failed!\n", i);
			return CVI_FAILURE;
		}
	}
	/************************************************
	 * Set VI dev config
	 ************************************************/
	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StartDev(&pstViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_StartDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}
	/************************************************
	 * Set VI pipe config
	 ************************************************/
	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StartPipe(&pstViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_StartPipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	/************************************************
	 * Create ISP
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_CreateIsp(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_CreateIsp failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	/************************************************
	 * Set sensor init
	 ************************************************/
	for (i = 0; i < gstSnsIniCfg.devNum; i++) {
		if (CVI_SNS_SetSnsInit(i) != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] sensor_%d init failed!\n", i);
			return CVI_FAILURE;
		}
	}
	/************************************************
	 * Set VI chn config
	 ************************************************/
	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StartChn(&pstViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_StartChn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_VIO_VI_DEINIT(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	int i = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_COMM_VI_DestroyIsp(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_DestroyIsp failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StopChn(&pstViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_StopChn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StopPipe(&pstViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_StopPipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StopDev(&pstViConfig->astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("[ERROR] SAMPLE_COMM_VI_StopDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

CVI_S32 SAMPLE_VIO_VPSS_INIT(SAMPLE_VI_CONFIG_S *pstViConfig, ASPECT_RATIO_E aspect_ratio)
{
	/************************************************
	 * Config and init VPSS
	 ************************************************/
	int i = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};
	SAMPLE_VI_INFO_S *pstViInfo = NULL;

	astVpssChnAttr[VpssChn].u32Width		    = 1280;
	astVpssChnAttr[VpssChn].u32Height		    = 720;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_NV21;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = aspect_ratio;
	if (!astVpssChnAttr[VpssChn].stAspectRatio.enMode) {
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = RGB_8BIT(0, 0, 0);
	}
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		// snr0
		pstViInfo = &pstViConfig->astViInfo[i];

		stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
		stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
		stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_NV12;
		stVpssGrpAttr.u32MaxW			     = pstViInfo->stDevInfo.stSize.u32Width;
		stVpssGrpAttr.u32MaxH			     = pstViInfo->stDevInfo.stSize.u32Height;
		stVpssGrpAttr.u8VpssDev			     = 1;
		/*start vpss*/
		s32Ret = CVI_VPSS_CreateGrp(i, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", i, s32Ret);
			goto exit1;
		}

		s32Ret = CVI_VPSS_SetChnAttr(i, VpssChn, &astVpssChnAttr[VpssChn]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit1;
		}

		s32Ret = CVI_VPSS_EnableChn(i, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit1;
		}

		/*start vpss*/
		s32Ret = CVI_VPSS_StartGrp(i);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
			goto exit1;
		}
	}

	return CVI_SUCCESS;

exit1:
	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		CVI_VPSS_StopGrp(VpssGrp);
		CVI_VPSS_DisableChn(VpssGrp, VpssChn);
		CVI_VPSS_DestroyGrp(VpssGrp);
	}

	return CVI_FAILURE;
}

CVI_S32 SAMPLE_VIO_VO_INIT(SAMPLE_VO_CONFIG_S *stVoConfig, VO_DEV VoDev)
{
	RECT_S stDefDispRect;
	SIZE_S stDefImageSize;
	CVI_S32 s32Ret = CVI_SUCCESS;

	stDefDispRect.s32X = 0;
	stDefDispRect.s32Y = 0;
	stDefDispRect.u32Width = 720;
	stDefDispRect.u32Height = 1280;
	stDefImageSize.u32Width = stDefDispRect.u32Width;
	stDefImageSize.u32Height = stDefDispRect.u32Height;
	s32Ret = SAMPLE_COMM_VO_GetDefConfig(stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVoConfig->VoDev	 = VoDev;
	stVoConfig->stVoPubAttr.enIntfType = VO_INTF_MIPI;
	stVoConfig->stVoPubAttr.enIntfSync = VO_OUTPUT_720x1280_60;
	stVoConfig->stDispRect	 = stDefDispRect;
	stVoConfig->stImageSize	 = stDefImageSize;
	stVoConfig->enPixFormat	 = PIXEL_FORMAT_NV21;
	stVoConfig->enVoMode	 = VO_MODE_1MUX;
	s32Ret = SAMPLE_COMM_VO_StartVO(stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %#x\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VI_VPSS_VO_START(CVI_VOID)
{
	CVI_S32		s32Ret;
	VI_VPSS_MODE_E stViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	SNS_INI_CFG_S stSnsIniCfg;
	VB_CONFIG_S stVbConfig;
	int i = 0;
	ROTATION_E rotation_vi = ROTATION_0;
	ROTATION_E rotation_vpss = ROTATION_0;
	ROTATION_E rotation_vo = ROTATION_90;

	/************************************************
	 * step1:  Init SYS
	 ************************************************/

	s32Ret = SAMPLE_VIO_SYS_INIT(&stViConfig, &stSnsIniCfg, &stVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("[ERROR]SAMPLE_VIO_SYS_INIT failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_VIO_SET_VI_VPSS_MODE(stViVpssMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VIO_SET_VI_VPSS_MODE failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step2:  Init VI
	 ************************************************/

	s32Ret = SAMPLE_VIO_VI_INIT(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_SetChnRotation(0, 0, rotation_vi);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	ASPECT_RATIO_E aspect_ratio = ASPECT_RATIO_NONE;

	s32Ret = SAMPLE_VIO_VPSS_INIT(&stViConfig, aspect_ratio);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VPSS_SetChnRotation(0, 0, rotation_vpss);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_Bind_VPSS(i, 0, i);
		}
	}

	/************************************************
	 * step5:  Init VO
	 ************************************************/
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	SAMPLE_VIO_VO_INIT(&stVoConfig, VoDev);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, rotation_vo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %d\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

CVI_VOID SAMPLE_REGION_VI_VPSS_VO_END(CVI_VOID)
{
	int i = 0;

	SAMPLE_COMM_VPSS_UnBind_VO(0, 0, 0, 0);

	SAMPLE_COMM_VO_StopVO(&stVoConfig);

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		SAMPLE_COMM_VI_UnBind_VPSS(i, 0, i);
	}

	SAMPLE_VIO_VI_DEINIT(&stViConfig);

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, 0);
		CVI_VPSS_DestroyGrp(i);
	}

	SAMPLE_COMM_SYS_Exit();
}

CVI_S32 SAMPLE_REGION_VI_VPSS_VO(CVI_S32 HandleNum, RGN_TYPE_E enType, MMF_CHN_S *pstChn)
{
	CVI_S32 i;
	CVI_S32 s32Ret;
	CVI_S32 s32ExtRet;
	CVI_S32 MinHandle;
	PIXEL_FORMAT_E pixelFormat;

	s32Ret = SAMPLE_REGION_VI_VPSS_VO_START();
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	pixelFormat = PIXEL_FORMAT_ARGB_1555;
	s32Ret = SAMPLE_COMM_REGION_Create(HandleNum, enType, pixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}
	s32Ret = SAMPLE_COMM_REGION_AttachToChn(HandleNum, enType, pstChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	if (enType == OVERLAY_RGN || enType == OVERLAYEX_RGN) {
		MinHandle = SAMPLE_COMM_REGION_GetMinHandle(enType);

		for (i = MinHandle; i < MinHandle + HandleNum; i++) {
			//s32Ret = SAMPLE_COMM_REGION_SetBitMap(i, Path_BMP);
			s32Ret = SAMPLE_COMM_REGION_GetUpCanvas(i, Path_BMP);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_REGION_GetUpCanvas failed!\n");
				goto EXIT2;
			}
		}
	}

	usleep(1000 * 1000);
EXIT2:
	s32ExtRet = SAMPLE_COMM_REGION_DetachFrmChn(HandleNum, enType, pstChn);
	if (s32ExtRet != CVI_SUCCESS)
		SAMPLE_PRT("SAMPLE_COMM_REGION_DetachFrmChn failed!\n");
EXIT1:
	s32ExtRet = SAMPLE_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS)
		SAMPLE_PRT("SAMPLE_COMM_REGION_Destroy failed!\n");

	SAMPLE_REGION_VI_VPSS_VO_END();
	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VI_VPSS_VO_8BIT_MODE(CVI_S32 HandleNum, RGN_TYPE_E enType,
		MMF_CHN_S *pstChn, PIXEL_FORMAT_E pixelFormat)
{
	CVI_S32 i;
	CVI_S32 s32Ret;
	CVI_S32 MinHandle;
	RGN_PALETTE_S stPalette;
	RGN_RGBQUARD_S *overlay_palette;

	s32Ret = SAMPLE_REGION_VI_VPSS_VO_START();
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = SAMPLE_COMM_REGION_Create(HandleNum, enType, pixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_REGION_Create failed!\n");
		goto EXIT1;
	}

	s32Ret = SAMPLE_COMM_REGION_AttachToChn(HandleNum, enType, pstChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_REGION_AttachToChn failed!\n");
		goto EXIT2;
	}

	MinHandle = SAMPLE_COMM_REGION_GetMinHandle(enType);

	/* Use indexed palettes format of bmp file in OVERLAY example. */
	if (enType == OVERLAY_RGN) {
		for (i = MinHandle; i < MinHandle + HandleNum; i++) {
			s32Ret = SAMPLE_COMM_REGION_SetBitMap(i, Path_BMP, pixelFormat, false);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_REGION_SetBitMap failed!\n");
				goto EXIT2;
			}
		}

	overlay_palette = SAMPLE_COMM_REGION_GetOverlayPalette();
#ifdef _SAMPLE_REGION_DEBUG_
		for (i = 0; i < 256 ; i++) {
			CVI_U32 u32Pixel =
					((overlay_palette[i].argbBlue | overlay_palette[i].argbGreen << 8) |
					 (overlay_palette[i].argbRed << 16 | overlay_palette[i].argbAlpha << 24));
			SAMPLE_PRT("overlay_palette index(%d) (0x%x).\n", i, u32Pixel);
		}
#endif
		stPalette.pstPaletteTable = (void *)overlay_palette;
		stPalette.lut_length = 256;
		stPalette.pixelFormat = RGN_COLOR_FMT_RGB888;
		CVI_RGN_SetChnPalette(MinHandle, pstChn, &stPalette);
	}

	usleep(1000 * 1000);
EXIT2:
	s32Ret = SAMPLE_COMM_REGION_DetachFrmChn(HandleNum, enType, pstChn);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_PRT("SAMPLE_COMM_REGION_AttachToChn failed!\n");
EXIT1:
	s32Ret = SAMPLE_COMM_REGION_Destroy(HandleNum, enType);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_PRT("SAMPLE_COMM_REGION_AttachToChn failed!\n");

	SAMPLE_REGION_VI_VPSS_VO_END();
	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VPSS_OSD(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	Path_BMP = test_bmp;
	s32Ret = SAMPLE_REGION_VI_VPSS_VO(HandleNum, enType, &stChn);
	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VPSS_COVER(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;

	HandleNum = 3;
	enType = COVER_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	s32Ret = SAMPLE_REGION_VI_VPSS_VO(HandleNum, enType, &stChn);
	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VPSS_OSD_8BIT_MODE(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	PIXEL_FORMAT_E pixelFormat;

	HandleNum = 3;
	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	pixelFormat = PIXEL_FORMAT_8BIT_MODE;
	Path_BMP = tiger_8bitmode;
	s32Ret = SAMPLE_REGION_VI_VPSS_VO_8BIT_MODE(HandleNum, enType, &stChn, pixelFormat);
	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VPSS_OSD_TIME(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 s32ExtRet;
	CVI_S32 HandleNum;
	CVI_S32 MinHandle;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn0;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	CVI_S32 i, j;
	char szStr[MAX_STR_LEN];
	int s32StrLen;

	s32Ret = SAMPLE_REGION_VI_VPSS_VO_START();
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	VPSS_CHN_ATTR_S stVpssChnAttr;

	CVI_VPSS_GetChnAttr(0, 0, &stVpssChnAttr);
	stVpssChnAttr.bFlip = CVI_FALSE;
	stVpssChnAttr.bMirror = CVI_FALSE;
	CVI_VPSS_SetChnAttr(0, 0, &stVpssChnAttr);

	SAMPLE_REGION_GetTimeStr(NULL, szStr, MAX_STR_LEN);
	s32StrLen = strnlen(szStr, MAX_STR_LEN);

	HandleNum = 1;
	enType = OVERLAY_RGN;
	stChn0.enModId = CVI_ID_VPSS;
	stChn0.s32DevId = 0;
	stChn0.s32ChnId = 0;
	MinHandle = SAMPLE_COMM_REGION_GetMinHandle(enType);

	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
	stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
	stRegion.unAttr.stOverlay.u32BgColor = 0x7fff;
	stRegion.unAttr.stOverlay.u32CanvasNum = 1;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
	for (i = MinHandle; i < MinHandle + HandleNum; i++) {
		s32Ret = CVI_RGN_Create(i, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
			goto EXIT1;
		}
	}

	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	for (i = MinHandle; i < MinHandle + HandleNum; i++) {
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20 + 300 * (i - MinHandle);
		stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20 + 300 * (i - MinHandle);
		s32Ret = CVI_RGN_AttachToChn(i, &stChn0, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
			goto EXIT2;
		}
	}

	BITMAP_S stBitmap;
	CVI_U32 *pu32Color = (CVI_U32 *)malloc(s32StrLen * sizeof(CVI_U32));

	for (j = 0; j < 3; ++j) {
		for (i = MinHandle; i < MinHandle + HandleNum; i++) {
			memset(pu32Color, 0, sizeof(CVI_U32) * s32StrLen);
			CVI_RGN_GetDisplayAttr(i, &stChn0, &stChnAttr);
			memset(pu32Color, 0xffff, sizeof(CVI_U32) * s32StrLen);
			stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
			stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
			stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
			SAMPLE_REGION_GetTimeStr(NULL, szStr, MAX_STR_LEN);
			s32Ret = SAMPLE_REGION_UpdateBitmap(i, szStr, &stBitmap, pu32Color);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_REGION_UpdateBitmap failed!\n");
				goto EXIT2;
			}
		}
		usleep(1000 * 1000);
	}
	free(pu32Color);

EXIT2:
	s32ExtRet = SAMPLE_COMM_REGION_DetachFrmChn(HandleNum, enType, &stChn0);
	if (s32ExtRet != CVI_SUCCESS)
		SAMPLE_PRT("SAMPLE_COMM_REGION_DetachFrmChn failed!\n");
EXIT1:
	s32ExtRet = SAMPLE_COMM_REGION_Destroy(HandleNum, enType);
	if (s32ExtRet != CVI_SUCCESS)
		SAMPLE_PRT("SAMPLE_COMM_REGION_Destroy failed!\n");

	SAMPLE_REGION_VI_VPSS_VO_END();

	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VPSS_OSD_OBJECTS(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 MinHandle;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	BITMAP_S stBitmap;
	CVI_U64 u64BitmapPhyAddr;
	CVI_VOID *pBitmapVirAddr;

	s32Ret = SAMPLE_REGION_VI_VPSS_VO_START();
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	VPSS_CHN_ATTR_S stVpssChnAttr;

	CVI_VPSS_GetChnAttr(0, 0, &stVpssChnAttr);
	stVpssChnAttr.bFlip = CVI_FALSE;
	stVpssChnAttr.bMirror = CVI_FALSE;
	CVI_VPSS_SetChnAttr(0, 0, &stVpssChnAttr);

	enType = OVERLAY_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;

	MinHandle = SAMPLE_COMM_REGION_GetMinHandle(enType);

	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 1280;
	stRegion.unAttr.stOverlay.stSize.u32Height = 720;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize = RGN_CMPR_MIN_SIZE;
	s32Ret = CVI_RGN_Create(MinHandle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		goto EXIT0;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(MinHandle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT1;
	}

	RGN_CANVAS_CMPR_ATTR_S * pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;

	s32Ret = CVI_RGN_GetCanvasInfo(MinHandle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		goto EXIT2;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 13;

	pstObjAttr[0].stRgnRect.stRect.s32X = 0;
	pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
	pstObjAttr[0].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[0].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[0].stRgnRect.u32Thick = 1;
	pstObjAttr[0].stRgnRect.u32Color = 0xffff;
	pstObjAttr[0].stRgnRect.u32IsFill = false;
	pstObjAttr[0].enObjType = RGN_CMPR_RECT;
	pstObjAttr[1].stRgnRect.stRect.s32X = 100;
	pstObjAttr[1].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[1].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[1].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[1].stRgnRect.u32Thick = 2;
	pstObjAttr[1].stRgnRect.u32Color = 0x8000;
	pstObjAttr[1].stRgnRect.u32IsFill = false;
	pstObjAttr[1].enObjType = RGN_CMPR_RECT;
	pstObjAttr[2].stRgnRect.stRect.s32X = 200;
	pstObjAttr[2].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[2].stRgnRect.u32Thick = 3;
	pstObjAttr[2].stRgnRect.u32Color = 0x801f;
	pstObjAttr[2].stRgnRect.u32IsFill = false;
	pstObjAttr[2].enObjType = RGN_CMPR_RECT;
	pstObjAttr[3].stRgnRect.stRect.s32X = 300;
	pstObjAttr[3].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[3].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[3].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[3].stRgnRect.u32Thick = 4;
	pstObjAttr[3].stRgnRect.u32Color = 0x83e0;
	pstObjAttr[3].stRgnRect.u32IsFill = false;
	pstObjAttr[3].enObjType = RGN_CMPR_RECT;
	pstObjAttr[4].stRgnRect.stRect.s32X = 400;
	pstObjAttr[4].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[4].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[4].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[4].stRgnRect.u32Thick = 5;
	pstObjAttr[4].stRgnRect.u32Color = 0xfc00;
	pstObjAttr[4].stRgnRect.u32IsFill = false;
	pstObjAttr[4].enObjType = RGN_CMPR_RECT;
	pstObjAttr[5].stRgnRect.stRect.s32X = 500;
	pstObjAttr[5].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[5].stRgnRect.u32Thick = 6;
	pstObjAttr[5].stRgnRect.u32Color = 0xffe0;
	pstObjAttr[5].stRgnRect.u32IsFill = false;
	pstObjAttr[5].enObjType = RGN_CMPR_RECT;
	pstObjAttr[6].stRgnRect.stRect.s32X = 600;
	pstObjAttr[6].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[6].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[6].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[6].stRgnRect.u32Thick = 7;
	pstObjAttr[6].stRgnRect.u32Color = 0xfc1f;
	pstObjAttr[6].stRgnRect.u32IsFill = false;
	pstObjAttr[6].enObjType = RGN_CMPR_RECT;
	pstObjAttr[7].stRgnRect.stRect.s32X = 700;
	pstObjAttr[7].stRgnRect.stRect.s32Y = 000;
	pstObjAttr[7].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[7].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[7].stRgnRect.u32Thick = 8;
	pstObjAttr[7].stRgnRect.u32Color = 0x83ff;
	pstObjAttr[7].stRgnRect.u32IsFill = false;
	pstObjAttr[7].enObjType = RGN_CMPR_RECT;

	pstObjAttr[8].stLine.stPointStart.s32X = 600;
	pstObjAttr[8].stLine.stPointStart.s32Y = 200;
	pstObjAttr[8].stLine.stPointEnd.s32X = 300;
	pstObjAttr[8].stLine.stPointEnd.s32Y = 400;
	pstObjAttr[8].stLine.u32Thick = 8;
	pstObjAttr[8].stLine.u32Color = 0xfc10;
	pstObjAttr[8].enObjType = RGN_CMPR_LINE;
	pstObjAttr[9].stLine.stPointStart.s32X = 300;
	pstObjAttr[9].stLine.stPointStart.s32Y = 400;
	pstObjAttr[9].stLine.stPointEnd.s32X = 800;
	pstObjAttr[9].stLine.stPointEnd.s32Y = 700;
	pstObjAttr[9].stLine.u32Thick = 8;
	pstObjAttr[9].stLine.u32Color = 0xfff0;
	pstObjAttr[9].enObjType = RGN_CMPR_LINE;
	pstObjAttr[10].stLine.stPointStart.s32X = 800;
	pstObjAttr[10].stLine.stPointStart.s32Y = 700;
	pstObjAttr[10].stLine.stPointEnd.s32X = 1100;
	pstObjAttr[10].stLine.stPointEnd.s32Y = 600;
	pstObjAttr[10].stLine.u32Thick = 8;
	pstObjAttr[10].stLine.u32Color = 0xfc7f;
	pstObjAttr[10].enObjType = RGN_CMPR_LINE;
	pstObjAttr[11].stLine.stPointStart.s32X = 1100;
	pstObjAttr[11].stLine.stPointStart.s32Y = 600;
	pstObjAttr[11].stLine.stPointEnd.s32X = 600;
	pstObjAttr[11].stLine.stPointEnd.s32Y = 200;
	pstObjAttr[11].stLine.u32Thick = 8;
	pstObjAttr[11].stLine.u32Color = 0x8fff;
	pstObjAttr[11].enObjType = RGN_CMPR_LINE;

	s32Ret = SAMPLE_COMM_REGION_MST_LoadBmp(tiger_bmp, &stBitmap, CVI_FALSE, 0x00,
		pstCanvasCmprAttr->enPixelFormat);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_REGION_MST_LoadBmp failed with %#x!\n", s32Ret);
		goto EXIT2;
	}
	s32Ret = CVI_SYS_IonAlloc(&u64BitmapPhyAddr, (CVI_VOID **)&pBitmapVirAddr, "rgn_cmpr_bitmap1",
			stBitmap.u32Width * stBitmap.u32Height * 2);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
		goto EXIT3;
	}
	memcpy(pBitmapVirAddr, stBitmap.pData, stBitmap.u32Width * stBitmap.u32Height * 2);
	pstObjAttr[12].stBitmap.stRect.s32X = 20;
	pstObjAttr[12].stBitmap.stRect.s32Y = 100;
	pstObjAttr[12].stBitmap.stRect.u32Width = stBitmap.u32Width;
	pstObjAttr[12].stBitmap.stRect.u32Height = stBitmap.u32Height;
	pstObjAttr[12].stBitmap.u64BitmapPAddr = u64BitmapPhyAddr;
	pstObjAttr[12].enObjType = RGN_CMPR_BIT_MAP;

	s32Ret = CVI_RGN_UpdateCanvas(MinHandle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
		goto EXIT4;
	}

	usleep(1000 * 1000);

EXIT4:
	CVI_SYS_IonFree(u64BitmapPhyAddr, pBitmapVirAddr);
EXIT3:
	free(stBitmap.pData);
EXIT2:
	CVI_RGN_DetachFromChn(MinHandle, &stChn);
EXIT1:
	CVI_RGN_Destroy(MinHandle);
EXIT0:
	SAMPLE_REGION_VI_VPSS_VO_END();

	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VPSS_COVEREX(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;

	HandleNum = 3;
	enType = COVEREX_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	s32Ret = SAMPLE_REGION_VI_VPSS_VO(HandleNum, enType, &stChn);
	return s32Ret;
}

CVI_S32 SAMPLE_REGION_VPSS_MOSAIC(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 HandleNum;
	RGN_TYPE_E enType;
	MMF_CHN_S stChn;

	HandleNum = 3;
	enType = MOSAIC_RGN;
	stChn.enModId = CVI_ID_VPSS;
	stChn.s32DevId = 0;
	stChn.s32ChnId = 0;
	s32Ret = SAMPLE_REGION_VI_VPSS_VO(HandleNum, enType, &stChn);
	return s32Ret;
}

int main(int argc, char *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32Index;

	if (argc < 2 || argc > 2) {
		SAMPLE_REGION_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (!strncmp(argv[1], "-h", 2)) {
		SAMPLE_REGION_Usage(argv[0]);
		return CVI_SUCCESS;
	}

	signal(SIGINT, SAMPLE_REGION_HandleSig);
	signal(SIGTERM, SAMPLE_REGION_HandleSig);

	s32Index = atoi(argv[1]);
	switch (s32Index) {
	case 0:
		s32Ret = SAMPLE_REGION_VPSS_OSD();
		break;
	case 1:
		s32Ret = SAMPLE_REGION_VPSS_COVER();
		break;
	case 2:
		s32Ret = SAMPLE_REGION_VPSS_OSD_TIME();
		break;
	case 3:
		s32Ret = SAMPLE_REGION_VPSS_OSD_8BIT_MODE();
		break;
	case 4:
		s32Ret = SAMPLE_REGION_VPSS_OSD_OBJECTS();
		break;
	case 5:
		s32Ret = SAMPLE_REGION_VPSS_COVEREX();
		break;
	case 6:
		s32Ret = SAMPLE_REGION_VPSS_MOSAIC();
		break;
	default:
		SAMPLE_PRT("option, %d, is invaild!\n", s32Index);
		SAMPLE_REGION_Usage(argv[0]);
		s32Ret = CVI_FAILURE;
		break;
	}

	if (s32Ret == CVI_SUCCESS)
		SAMPLE_PRT("program exit normally!\n");
	else
		SAMPLE_PRT("program exit abnormally!\n");

	return s32Ret;
}
