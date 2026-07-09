#ifndef __VO_UT_COMM_H__
#define __VO_UT_COMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#include "cvi_common.h"
#include "cvi_comm_vo.h"
#include "cvi_comm_vpss.h"

#include "ut_comm.h"

typedef enum _VO_MODE_E {
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
} VO_MODE_E;

typedef struct _VO_CONFIG_S {
	/* for device */
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr;

	/* for layer */
	PIXEL_FORMAT_E enPixFormat;
	RECT_S stDispRect;
	SIZE_S stImageSize;

	CVI_U32 u32DisBufLen;

	/* for channel */
	VO_MODE_E enVoMode;
} VO_CONFIG_S;

CVI_S32 VO_GetDefConfig(VO_CONFIG_S *pstVoConfig);
CVI_S32 VO_StartVO(VO_CONFIG_S *pstVoConfig);
CVI_S32 VO_StopVO(VO_CONFIG_S *pstVoConfig);
CVI_S32 VPSS_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
		  VPSS_CHN_ATTR_S *pastVpssChnAttr);
CVI_S32 VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
		   VPSS_CHN_ATTR_S *pastVpssChnAttr);
CVI_S32 VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable);
CVI_S32 VPSS_Bind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 VPSS_UnBind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
CVI_S32 VPSS_SendFrame(VPSS_GRP VpssGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __VO_UT_COMM_H__ */
