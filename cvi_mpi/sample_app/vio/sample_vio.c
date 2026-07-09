#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */

#include "cvi_comm_vb.h"
#include "cvi_comm_vpss.h"
#include "cvi_sns_ctrl.h"
#include "cvi_comm_cif.h"
#include "cvi_comm_sns.h"
#include "cvi_mipi.h"
#include "sample_comm.h"
#include "sample_vio.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

static SNS_INI_CFG_S gstSnsIniCfg;

CVI_VOID SAMPLE_VIO_SYS_EXIT(CVI_VOID)
{
	char input[100] = {0};
	while (1) {
		SAMPLE_PRT("input 'exit' to exit current option:\n");
		if (fgets(input, sizeof(input), stdin)) {
			input[strcspn(input, "\n")] = '\0';
			if (strcmp(input, "exit") == 0)
				break;
		} else {
			clearerr(stdin);
		}
	}
	return;
}

CVI_S32 SAMPLE_VIO_VB_CFG(CVI_S32 vb_weigth, CVI_S32 vb_height, CVI_S32 vb_cnt, VB_CONFIG_S *pstVbConfig)
{
	CVI_BOOL createNewPool = CVI_TRUE;
	CVI_U32 u32BlkSize, u32BlkRotSize;

	u32BlkSize = COMMON_GetPicBufferSize(vb_weigth, vb_height,
			VI_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(vb_height, vb_weigth,
			VI_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = u32BlkSize > u32BlkRotSize ? u32BlkSize : u32BlkRotSize;

	for (CVI_U32 j = 0; j < pstVbConfig->u32MaxPoolCnt; j++) {
		if (pstVbConfig->astCommPool[j].u32BlkSize == u32BlkSize) {
			pstVbConfig->astCommPool[j].u32BlkCnt += vb_cnt;
			createNewPool = false;
			break;
		}
	}

	if (createNewPool) {
		pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkSize = u32BlkSize;
		pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkCnt = vb_cnt;
		pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].enRemapMode = VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("[INFO] set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
					pstVbConfig->u32MaxPoolCnt, vb_weigth, vb_height,
					pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkCnt,
					pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkSize);
		pstVbConfig->u32MaxPoolCnt++;
	} else {
		SAMPLE_PRT("[INFO] set VBpool [%d] %d:%d, BlkCnt= %d, Size = %d\n",
					pstVbConfig->u32MaxPoolCnt, vb_weigth, vb_height,
					pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkCnt,
					pstVbConfig->astCommPool[pstVbConfig->u32MaxPoolCnt].u32BlkSize);
	}

	if (pstVbConfig->u32MaxPoolCnt == 1) {
		pstVbConfig->astCommPool[0].u32BlkCnt += 2;
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_VIO_SYS_INIT(SAMPLE_VI_CONFIG_S *pstViConfig, SNS_INI_CFG_S *pstSnsIniCfg,
								VB_CONFIG_S *pstVbConfig, ROTATION_CFG_S *pstRotCfg)
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

	if (pstRotCfg->rotation_vi == ROTATION_90 || pstRotCfg->rotation_vi == ROTATION_270) {
		for (CVI_S32 i = 0; i < pstViConfig->s32ViNum; i++) {
			SAMPLE_VIO_VB_CFG(pstViConfig->astViInfo[i].stDevInfo.stSize.u32Width,
							pstViConfig->astViInfo[i].stDevInfo.stSize.u32Height, 3, pstVbConfig);
		}
	}

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
		if(!pstViConfig->astViInfo->stDevInfo.bPatgen){
			if (CVI_SNS_SetSnsProbe(i) != CVI_SUCCESS) {
				SAMPLE_PRT("[ERROR] sensor_%d probe failed!\n", i);
				return CVI_FAILURE;
			}
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
		if(!pstViConfig->astViInfo->stDevInfo.bPatgen){
			if (CVI_SNS_SetSnsInit(i) != CVI_SUCCESS) {
				SAMPLE_PRT("[ERROR] sensor_%d init failed!\n", i);
				return CVI_FAILURE;
			}
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

CVI_S32 SAMPLE_VIO_VPSS_INIT(SAMPLE_VI_CONFIG_S *pstViConfig,
							ASPECT_RATIO_E aspect_ratio, ROTATION_CFG_S *pstRotCfg)
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

	if (pstRotCfg->rotation_vo == ROTATION_90 || pstRotCfg->rotation_vo == ROTATION_270 ||
		pstRotCfg->rotation_vpss == ROTATION_90 || pstRotCfg->rotation_vpss == ROTATION_270) {
		astVpssChnAttr[VpssChn].u32Width		    = 1280;
		astVpssChnAttr[VpssChn].u32Height		    = 720;
	} else {
		astVpssChnAttr[VpssChn].u32Width		    = 720;
		astVpssChnAttr[VpssChn].u32Height		    = 1280;
	}
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_NV21;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = aspect_ratio;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = RGB_8BIT(0, 0, 0);
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	for (i = 0; i < pstViConfig->s32ViNum; i++) {
		// snr0
		pstViInfo = &pstViConfig->astViInfo[i];

		stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
		stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
		stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_NV12;
		if (pstRotCfg->rotation_vi == ROTATION_90 || pstRotCfg->rotation_vi == ROTATION_270) {
			stVpssGrpAttr.u32MaxW			     = pstViInfo->stDevInfo.stSize.u32Height;
			stVpssGrpAttr.u32MaxH			     = pstViInfo->stDevInfo.stSize.u32Width;
		} else {
			stVpssGrpAttr.u32MaxW			     = pstViInfo->stDevInfo.stSize.u32Width;
			stVpssGrpAttr.u32MaxH			     = pstViInfo->stDevInfo.stSize.u32Height;
		}
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