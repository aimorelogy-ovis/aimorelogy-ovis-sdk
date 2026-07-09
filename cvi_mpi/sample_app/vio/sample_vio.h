#ifndef _SAMPLE_VIO_H_
#define _SAMPLE_VIO_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "sample_comm.h"

typedef struct SAMPLE_VIO_ROTATION_CFG_S {
	ROTATION_E rotation_vi;
	ROTATION_E rotation_vpss;
	ROTATION_E rotation_vo;
} ROTATION_CFG_S;

CVI_S32 SAMPLE_VIO_SYS_INIT(SAMPLE_VI_CONFIG_S *pstViConfig, SNS_INI_CFG_S *pstSnsIniCfg,
	VB_CONFIG_S *pstVbConfig, ROTATION_CFG_S *pstRotCfg);
CVI_S32 SAMPLE_VIO_SET_VI_VPSS_MODE(VI_VPSS_MODE_E vivpssMode);
CVI_S32 SAMPLE_VIO_VI_INIT(SAMPLE_VI_CONFIG_S *stViConfig);
CVI_S32 SAMPLE_VIO_VI_DEINIT(SAMPLE_VI_CONFIG_S *stViConfig);
CVI_S32 SAMPLE_VIO_VPSS_INIT(SAMPLE_VI_CONFIG_S *pstViConfig,
	ASPECT_RATIO_E aspect_ratio, ROTATION_CFG_S *pstRotCfg);
CVI_S32 SAMPLE_VIO_VO_INIT(SAMPLE_VO_CONFIG_S *stVoConfig, VO_DEV VoDev);
CVI_VOID SAMPLE_VIO_SYS_EXIT(CVI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __SAMPLE_VIO_H__*/
