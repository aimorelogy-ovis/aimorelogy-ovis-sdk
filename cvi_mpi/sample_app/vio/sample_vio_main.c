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
#include "cvi_vo.h"
#include "sample_vio.h"
#include "sample_comm.h"
#include <signal.h>

CVI_S32 SAMPLE_VIO_TWO_DEV_VO(void)
{
	CVI_S32		s32Ret;
	VI_VPSS_MODE_E stViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	SAMPLE_VI_CONFIG_S stViConfig;
	SNS_INI_CFG_S stSnsIniCfg;
	VB_CONFIG_S stVbConfig;
	int i = 0;
	ROTATION_CFG_S stRotationCfg = {ROTATION_0,
									ROTATION_0,
									ROTATION_90};

	/************************************************
	 * step1:  Init SYS
	 ************************************************/

	s32Ret = SAMPLE_VIO_SYS_INIT(&stViConfig, &stSnsIniCfg, &stVbConfig, &stRotationCfg);
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

	s32Ret = CVI_VI_SetChnRotation(0, 0, stRotationCfg.rotation_vi);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	ASPECT_RATIO_E aspect_ratio = ASPECT_RATIO_NONE;

	s32Ret = SAMPLE_VIO_VPSS_INIT(&stViConfig, aspect_ratio, &stRotationCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VPSS_SetChnRotation(0, 0, stRotationCfg.rotation_vpss);
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
	SAMPLE_VO_CONFIG_S stVoConfig;
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	SAMPLE_VIO_VO_INIT(&stVoConfig, VoDev);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, stRotationCfg.rotation_vo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %d\n", s32Ret);
		return s32Ret;
	}

	CVI_U32 chnID = 0;
	CVI_S32 vo_bind_vpssgrp = 0;

	do {
		SAMPLE_PRT("Please input show sensorID or input 255 to exit\n");
		scanf("%d", &chnID);
		if (chnID == 0) {
			if (vo_bind_vpssgrp == 0)
				continue;
			else if (vo_bind_vpssgrp == 1) {
				SAMPLE_COMM_VPSS_UnBind_VO(1, 0, 0, 0);
				SAMPLE_COMM_VPSS_Bind_VO(0, 0, 0, 0);
				vo_bind_vpssgrp = 0;
			}
		} else if (chnID == 1) {
			if (vo_bind_vpssgrp == 1)
				continue;
			else if (vo_bind_vpssgrp == 0) {
				SAMPLE_COMM_VPSS_UnBind_VO(0, 0, 0, 0);
				SAMPLE_COMM_VPSS_Bind_VO(1, 0, 0, 0);
				vo_bind_vpssgrp = 1;
			}
		}
	} while (chnID != 255);

	SAMPLE_PRT("vo_bind_vpssgrp = %d\n", vo_bind_vpssgrp);

	if (vo_bind_vpssgrp == 0)
		SAMPLE_COMM_VPSS_UnBind_VO(0, 0, 0, 0);
	else if (vo_bind_vpssgrp == 1) {
		SAMPLE_COMM_VPSS_UnBind_VO(1, 0, 0, 0);
		vo_bind_vpssgrp = 0;
	}

	SAMPLE_COMM_VO_StopVO(&stVoConfig);

	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_UnBind_VPSS(i, 0, i);
		}
	}

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, 0);
		CVI_VPSS_DestroyGrp(i);
	}

	SAMPLE_VIO_VI_DEINIT(&stViConfig);

	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

CVI_S32 SAMPLE_VIO_VoRotation(void)
{
	CVI_S32		s32Ret;
	VI_VPSS_MODE_E stViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	SAMPLE_VI_CONFIG_S stViConfig;
	SNS_INI_CFG_S stSnsIniCfg;
	VB_CONFIG_S stVbConfig;
	int i = 0;
	ROTATION_CFG_S stRotationCfg = {ROTATION_0,
									ROTATION_0,
									ROTATION_90};

	/************************************************
	 * step1:  Init SYS
	 ************************************************/

	s32Ret = SAMPLE_VIO_SYS_INIT(&stViConfig, &stSnsIniCfg, &stVbConfig, &stRotationCfg);
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

	s32Ret = CVI_VI_SetChnRotation(0, 0, stRotationCfg.rotation_vi);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	ASPECT_RATIO_E aspect_ratio = ASPECT_RATIO_NONE;

	s32Ret = SAMPLE_VIO_VPSS_INIT(&stViConfig, aspect_ratio, &stRotationCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VPSS_SetChnRotation(0, 0, stRotationCfg.rotation_vpss);
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
	SAMPLE_VO_CONFIG_S stVoConfig;
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	SAMPLE_VIO_VO_INIT(&stVoConfig, VoDev);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, stRotationCfg.rotation_vo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %d\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_VIO_SYS_EXIT();

	SAMPLE_COMM_VPSS_UnBind_VO(0, 0, VoLayer, VoChn);

	SAMPLE_COMM_VO_StopVO(&stVoConfig);


	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_UnBind_VPSS(i, 0, i);
		}
	}

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, 0);
		CVI_VPSS_DestroyGrp(i);
	}

	SAMPLE_VIO_VI_DEINIT(&stViConfig);

	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

CVI_S32 SAMPLE_VIO_ViVpssAspectRatio(void)
{
	CVI_S32		s32Ret;
	VI_VPSS_MODE_E stViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	SAMPLE_VI_CONFIG_S stViConfig;
	SNS_INI_CFG_S stSnsIniCfg;
	VB_CONFIG_S stVbConfig;
	int i = 0;
	ROTATION_CFG_S stRotationCfg = {ROTATION_0,
									ROTATION_0,
									ROTATION_0};

	/************************************************
	 * step1:  Init SYS
	 ************************************************/

	s32Ret = SAMPLE_VIO_SYS_INIT(&stViConfig, &stSnsIniCfg, &stVbConfig, &stRotationCfg);
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

	s32Ret = CVI_VI_SetChnRotation(0, 0, stRotationCfg.rotation_vi);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	ASPECT_RATIO_E aspect_ratio = ASPECT_RATIO_AUTO;

	s32Ret = SAMPLE_VIO_VPSS_INIT(&stViConfig, aspect_ratio, &stRotationCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VPSS_SetChnRotation(0, 0, stRotationCfg.rotation_vpss);
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
	SAMPLE_VO_CONFIG_S stVoConfig;
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	SAMPLE_VIO_VO_INIT(&stVoConfig, VoDev);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, stRotationCfg.rotation_vo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %d\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_VIO_SYS_EXIT();

	SAMPLE_COMM_VPSS_UnBind_VO(0, 0, VoLayer, VoChn);

	SAMPLE_COMM_VO_StopVO(&stVoConfig);

	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_UnBind_VPSS(i, 0, i);
		}
	}

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, 0);
		CVI_VPSS_DestroyGrp(i);
	}

	SAMPLE_VIO_VI_DEINIT(&stViConfig);

	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

CVI_S32 SAMPLE_VIO_ViRotation(void)
{
	CVI_S32		s32Ret;
	VI_VPSS_MODE_E stViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	SAMPLE_VI_CONFIG_S stViConfig;
	SNS_INI_CFG_S stSnsIniCfg;
	VB_CONFIG_S stVbConfig;
	int i = 0;
	ROTATION_CFG_S stRotationCfg = {ROTATION_90,
									ROTATION_0,
									ROTATION_0};

	/************************************************
	 * step1:  Init SYS
	 ************************************************/

	s32Ret = SAMPLE_VIO_SYS_INIT(&stViConfig, &stSnsIniCfg, &stVbConfig, &stRotationCfg);
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

	s32Ret = CVI_VI_SetChnRotation(0, 0, stRotationCfg.rotation_vi);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	ASPECT_RATIO_E aspect_ratio = ASPECT_RATIO_NONE;

	s32Ret = SAMPLE_VIO_VPSS_INIT(&stViConfig, aspect_ratio, &stRotationCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VPSS_SetChnRotation(0, 0, stRotationCfg.rotation_vpss);
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
	SAMPLE_VO_CONFIG_S stVoConfig;
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	SAMPLE_VIO_VO_INIT(&stVoConfig, VoDev);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, stRotationCfg.rotation_vo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %d\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_VIO_SYS_EXIT();

	SAMPLE_COMM_VPSS_UnBind_VO(0, 0, VoLayer, VoChn);

	SAMPLE_COMM_VO_StopVO(&stVoConfig);

	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_UnBind_VPSS(i, 0, i);
		}
	}

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, 0);
		CVI_VPSS_DestroyGrp(i);
	}

	SAMPLE_VIO_VI_DEINIT(&stViConfig);

	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

CVI_S32 SAMPLE_VIO_VpssRotation(void)
{
	CVI_S32		s32Ret;
	VI_VPSS_MODE_E stViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	SAMPLE_VI_CONFIG_S stViConfig;
	SNS_INI_CFG_S stSnsIniCfg;
	VB_CONFIG_S stVbConfig;
	int i = 0;
	ROTATION_CFG_S stRotationCfg = {ROTATION_0,
									ROTATION_90,
									ROTATION_0};
	/************************************************
	 * step1:  Init SYS
	 ************************************************/

	s32Ret = SAMPLE_VIO_SYS_INIT(&stViConfig, &stSnsIniCfg, &stVbConfig, &stRotationCfg);
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

	s32Ret = CVI_VI_SetChnRotation(0, 0, stRotationCfg.rotation_vi);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	ASPECT_RATIO_E aspect_ratio = ASPECT_RATIO_NONE;

	s32Ret = SAMPLE_VIO_VPSS_INIT(&stViConfig, aspect_ratio, &stRotationCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_Bind_VPSS(i, 0, i);
		}
	}

	s32Ret = CVI_VPSS_SetChnRotation(0, 0, stRotationCfg.rotation_vpss);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step5:  Init VO
	 ************************************************/
	SAMPLE_VO_CONFIG_S stVoConfig;
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	SAMPLE_VIO_VO_INIT(&stVoConfig, VoDev);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, stRotationCfg.rotation_vo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation is fail\n");
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %d\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_VIO_SYS_EXIT();

	SAMPLE_COMM_VPSS_UnBind_VO(0, 0, VoLayer, VoChn);

	SAMPLE_COMM_VO_StopVO(&stVoConfig);

	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_UnBind_VPSS(i, 0, i);
		}
	}

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, 0);
		CVI_VPSS_DestroyGrp(i);
	}

	SAMPLE_VIO_VI_DEINIT(&stViConfig);

	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

CVI_S32 SAMPLE_VIO_ViVpss_RotationLdc(void)
{
	CVI_S32		s32Ret;
	VI_VPSS_MODE_E stViVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	SAMPLE_VI_CONFIG_S stViConfig;
	SNS_INI_CFG_S stSnsIniCfg;
	VB_CONFIG_S stVbConfig;
	int i = 0;
	ROTATION_CFG_S stRotationCfg = {ROTATION_90,
									ROTATION_0,
									ROTATION_0};
	VI_LDC_ATTR_S stViLdcAttr = {0};
	VPSS_LDC_ATTR_S stVpssLDCAttr = {0};

	/************************************************
	 * step1:  Init SYS
	 ************************************************/

	s32Ret = SAMPLE_VIO_SYS_INIT(&stViConfig, &stSnsIniCfg, &stVbConfig, &stRotationCfg);
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

	stViLdcAttr.bEnable = CVI_TRUE;
	stViLdcAttr.stAttr.bAspect = true;
	stViLdcAttr.stAttr.s32XYRatio = 50;
	stViLdcAttr.stAttr.s32CenterXOffset = 50;
	stViLdcAttr.stAttr.s32CenterYOffset = 50;
	stViLdcAttr.stAttr.s32DistortionRatio = 50;
	stViLdcAttr.stAttr.enRotation = stRotationCfg.rotation_vi;

	s32Ret = CVI_VI_SetChnLDCAttr(0, 0, &stViLdcAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_SetChnLDCAttr failed with %d\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init VPSS
	 ************************************************/
	ASPECT_RATIO_E aspect_ratio = ASPECT_RATIO_NONE;

	s32Ret = SAMPLE_VIO_VPSS_INIT(&stViConfig, aspect_ratio, &stRotationCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	stVpssLDCAttr.bEnable = CVI_TRUE;
	stVpssLDCAttr.stAttr.bAspect = true;
	stVpssLDCAttr.stAttr.s32XYRatio = 50;
	stVpssLDCAttr.stAttr.s32CenterXOffset = 50;
	stVpssLDCAttr.stAttr.s32CenterYOffset = 50;
	stVpssLDCAttr.stAttr.s32DistortionRatio = 50;
	stVpssLDCAttr.stAttr.enRotation = stRotationCfg.rotation_vpss;
	s32Ret = CVI_VPSS_SetChnLDCAttr(0, 0, &stVpssLDCAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnLDCAttr failed with %d\n", s32Ret);
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
	SAMPLE_VO_CONFIG_S stVoConfig;
	VO_DEV VoDev = 0;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	SAMPLE_VIO_VO_INIT(&stVoConfig, VoDev);

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, stRotationCfg.rotation_vo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %d\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_VIO_SYS_EXIT();

	SAMPLE_COMM_VPSS_UnBind_VO(0, 0, VoLayer, VoChn);

	SAMPLE_COMM_VO_StopVO(&stVoConfig);

	if (stViVpssMode == VI_OFFLINE_VPSS_OFFLINE || stViVpssMode == VI_ONLINE_VPSS_OFFLINE) {
		for (i = 0; i < stViConfig.s32ViNum; i++) {
			SAMPLE_COMM_VI_UnBind_VPSS(i, 0, i);
		}
	}

	for (i = 0; i < stViConfig.s32ViNum; i++) {
		CVI_VPSS_StopGrp(i);
		CVI_VPSS_DisableChn(i, 0);
		CVI_VPSS_DestroyGrp(i);
	}

	SAMPLE_VIO_VI_DEINIT(&stViConfig);

	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

void SAMPLE_VIO_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		//todo for release
		SAMPLE_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

void SAMPLE_VIO_Usage(char *sPrgNm)
{
	SAMPLE_PRT("Usage : %s <index>\n", sPrgNm);
	SAMPLE_PRT("index:\n");
	SAMPLE_PRT("\t 0)VI (Offline) - VPSS(Offline) - VO(Rotation).\n");
	SAMPLE_PRT("\t 1)VI (Offline) - VPSS(Offline, Keep Aspect Ratio) - VO.\n");
	SAMPLE_PRT("\t 2)VI (Offline, Rotation) - VPSS(Offline, Keep Aspect Ratio) - VO.\n");
	SAMPLE_PRT("\t 3)VI (Offline) - VPSS(Offline, Rotation) - VO.\n");
	SAMPLE_PRT("\t 4)VI (Offline, Two devs) - VPSS(Offline) - VO(Rotation).\n");
	SAMPLE_PRT("\t 5)VI (Offline, Rotation, Ldc) - VPSS(Offline, Rotation, Ldc) - VO.\n");
}

int main(int argc, char *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32Index;

	if (argc < 2) {
		SAMPLE_VIO_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (!strncmp(argv[1], "-h", 2)) {
		SAMPLE_VIO_Usage(argv[0]);
		return CVI_SUCCESS;
	}

	signal(SIGINT, SAMPLE_VIO_HandleSig);
	signal(SIGTERM, SAMPLE_VIO_HandleSig);

	s32Index = atoi(argv[1]);
	switch (s32Index) {
	case 0:
		s32Ret = SAMPLE_VIO_VoRotation();
		break;

	case 1:
		s32Ret = SAMPLE_VIO_ViVpssAspectRatio();
		break;

	case 2:
		s32Ret = SAMPLE_VIO_ViRotation();
		break;

	case 3:
		s32Ret = SAMPLE_VIO_VpssRotation();
		break;

	case 4:
		s32Ret = SAMPLE_VIO_TWO_DEV_VO();
		break;

	case 5:
		s32Ret = SAMPLE_VIO_ViVpss_RotationLdc();
		break;
	default:
		SAMPLE_PRT("the index %d is invaild!\n", s32Index);
		SAMPLE_VIO_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (s32Ret == CVI_SUCCESS)
		SAMPLE_PRT("sample_vio exit success!\n");
	else
		SAMPLE_PRT("sample_vio exit abnormally!\n");

	return s32Ret;
}
