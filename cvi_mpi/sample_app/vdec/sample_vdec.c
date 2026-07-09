#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "sample_vdec_lib.h"

#define MAX_FILENAME_LEN	64

#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation=" /* Or  "-Wformat-overflow"  */
#endif

static CVI_S32 SAMPLE_VDEC(sampleVdec *psvdec);

int main(int argc, char **argv)
{
	sampleVdec sv, *psvdec = &sv;
	vdecInputCfg *pic = &psvdec->inputCfg;
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_PRT("%s\n", argv[0]);

	s32Ret = parseDecArgv(pic, argc, argv);
	if (s32Ret < 0) {
		if (s32Ret == STATUS_HELP)
			return CVI_SUCCESS;

		SAMPLE_PRT("parseDecArgv\n");
		return CVI_FAILURE;
	}

	signal(SIGINT, SAMPLE_VDEC_HandleSig);
	signal(SIGTERM, SAMPLE_VDEC_HandleSig);

	s32Ret = SAMPLE_VDEC(psvdec);
	if (s32Ret == CVI_SUCCESS) {
		SAMPLE_PRT("program exit normally!\n");
	} else {
		SAMPLE_PRT("program exit abnormally!\n");
	}

	return s32Ret;
}

static CVI_S32 SAMPLE_VDEC(sampleVdec *psvdec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_VDEC_INIT_VB(psvdec);
	if (s32Ret == STAT_ERR_VDEC_INIT_ATTR) {
		SAMPLE_PRT("STAT_ERR_VDEC_INIT_ATTR\n");
		return s32Ret;
	} else if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("INIT_VB FAIL\n");
		goto END2;
	}
	s32Ret = SAMPLE_VDEC_START(psvdec);

	if (s32Ret == STAT_ERR_VDEC_COMMON_START) {
		SAMPLE_PRT("STAT_ERR_VDEC_COMMON_START\n");
		goto END3;
	} else if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_VDEC_START\n");
		return s32Ret;
	}
	SAMPLE_VDEC_STOP(psvdec);

END3:
	SAMPLE_COMM_VDEC_Stop(psvdec->u32VdecNumAllChns);

END2:
	SAMPLE_COMM_VDEC_ExitVBPool();
#ifdef VDEC_BIND_MODE
END1:
#endif
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}


#if defined(__GNUC__) && defined(__riscv)
#pragma GCC diagnostic pop
#endif