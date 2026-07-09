#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "sample_comm.h"

CVI_S32 SAMPLE_COMM_VPSS_INIT(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	VPSS_CHN VpssChn;
	CVI_S32 s32Ret;
	CVI_S32 j;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VPSS_ResetGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_ResetGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return CVI_FAILURE;
	}

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);

			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);

			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VPSS_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	CVI_S32 s32Ret;
	UNUSED(pabChnEnable);
	UNUSED(pstVpssGrpAttr);
	UNUSED(pastVpssChnAttr);

	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_VPSS_Stop(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable)
{
	CVI_S32 j;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_CHN VpssChn;

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("Vpss stop Grp %d channel %d failed! Please check param\n",
				VpssGrp, VpssChn);
				return CVI_FAILURE;
			}
		}
	}

	s32Ret = CVI_VPSS_StopGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Vpss Stop Grp %d failed! Please check param\n", VpssGrp);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Vpss Destroy Grp %d failed! Please check\n", VpssGrp);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}
