#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>

#include "cvi_sys.h"
#include "cvi_vb.h"

#include "ut_comm.h"

#define SYS_UT_ION_LEN	0x1000

typedef enum _SYS_TEST_OP {
	SYS_TEST_SET_VIVPSS_MODE = 0,
	SYS_TEST_ION,
	SYS_TEST_BIND,
	SYS_TEST_INFO,
	SYS_TEST_LOG,
	SYS_TEST_AUTO = 99,
} SYS_TEST_OP;

static CVI_VOID sys_memcpy_test(CVI_VOID *dst, CVI_VOID *src, size_t size, CVI_U32 *duration)
{
	struct timespec time[2];

	clock_gettime(CLOCK_MONOTONIC, &time[0]);
	memcpy(dst, src, size);
	clock_gettime(CLOCK_MONOTONIC, &time[1]);
	*duration = GetTimeDiffInUs(time[0], time[1]);
}

/*
static static CVI_S32 sys_flush_test(CVI_VOID)
{
	CVI_U64 u64PhySrc = 0, u64PhyDst = 0;
	CVI_VOID *pVirAddrSrc = NULL, *pVirAddrDst = NULL;
	CVI_U32 u32BufLen = 0x10000;
	CVI_S32 pattern = 0x37;
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (CVI_SYS_IonAlloc(&u64PhySrc, &pVirAddrSrc, "sys_test_src", u32BufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc NG.\n");
		return CVI_FAILURE;
	}
	if (CVI_SYS_IonAlloc(&u64PhyDst, &pVirAddrDst, "sys_test_dst", u32BufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc NG.\n");
		CVI_SYS_IonFree(u64PhySrc, pVirAddrSrc);
		return CVI_FAILURE;
	}
	memset(pVirAddrSrc, pattern, u32BufLen);

	if (CVI_SYS_IonFlushCache(u64PhySrc, pVirAddrSrc, u32BufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonFlushCache NG.\n");
		s32Ret = CVI_FAILURE;
		goto exit;
	}
	//cdma copy src to dst
	//s32Ret = CVI_SYS_CDMACopy(u64PhyDst, u64PhySrc, u32BufLen);
	if (s32Ret) {
		UT_PRT("CVI_SYS_CDMACopy NG.\n");
		goto exit;
	}
	s32Ret = memcmp(pVirAddrDst, pVirAddrSrc, u32BufLen);
	if (s32Ret)
		UT_PRT("memncmp fail.\n");

exit:
	CVI_SYS_IonFree(u64PhySrc, pVirAddrSrc);
	CVI_SYS_IonFree(u64PhyDst, pVirAddrDst);

	return s32Ret;
}

static static CVI_S32 sys_invalidate_test(CVI_VOID)
{
	CVI_U64 u64PhySrc = 0, u64PhyDst = 0;
	CVI_VOID *pVirAddrSrc = NULL, *pVirAddrDst = NULL;
	CVI_U32 u32BufLen = 0x10000;
	CVI_S32 pattern = 0x37;
	CVI_CHAR *p;
	CVI_U32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (CVI_SYS_IonAlloc(&u64PhySrc, &pVirAddrSrc, "sys_test_src", u32BufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc NG.\n");
		return CVI_FAILURE;
	}
	if (CVI_SYS_IonAlloc(&u64PhyDst, &pVirAddrDst, "sys_test_dst", u32BufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc NG.\n");
		CVI_SYS_IonFree(u64PhySrc, pVirAddrSrc);
		return CVI_FAILURE;
	}
	memset(pVirAddrSrc, pattern, u32BufLen);
	memset(pVirAddrDst, 0, u32BufLen);

	p = pVirAddrDst;
	for (i = 0; i < u32BufLen; i++)
		if (p[i] != 0) {
			UT_PRT("pVirAddrDst err.\n");
			s32Ret = CVI_FAILURE;
			goto exit;
		}

	if (CVI_SYS_IonFlushCache(u64PhySrc, pVirAddrSrc, u32BufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonFlushCache NG.\n");
		s32Ret = CVI_FAILURE;
		goto exit;
	}

	//s32Ret = CVI_SYS_CDMACopy(u64PhyDst, u64PhySrc, u32BufLen);
	if (s32Ret) {
		UT_PRT("CVI_SYS_CDMACopy NG.\n");
		goto exit;
	}

	if (CVI_SYS_IonInvalidateCache(u64PhyDst, pVirAddrDst, u32BufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonInvalidateCache NG.\n");
		s32Ret = CVI_FAILURE;
		goto exit;
	}

	for (i = 0; i < u32BufLen; i++)
		if (p[i] != pattern) {
			UT_PRT("Ion Invalid failed!!!.\n");
			s32Ret = CVI_FAILURE;
			goto exit;
		}

exit:
	CVI_SYS_IonFree(u64PhySrc, pVirAddrSrc);
	CVI_SYS_IonFree(u64PhyDst, pVirAddrDst);

	return s32Ret;
} */

static CVI_CHAR *sys_ut_pwreason_to_str(CVI_U32 u32PowerOnReason)
{
	char *str = "";

	switch (u32PowerOnReason) {
	case CVI_COLDBOOT:
		str = "CVI_COLDBOOT";
	break;
	case CVI_WDTBOOT:
		str = "CVI_WDTBOOT";
	break;
	case CVI_SUSPENDBOOT:
		str = "CVI_SUSPENDBOOT";
	break;
	case CVI_WARMBOOT:
		str = "CVI_WARMBOOT";
	break;
	default:
		str = "CVI_COLDBOOT";
	break;
	}
	return str;
}

static CVI_S32 sys_ut_setmode(CVI_VOID)
{
	VI_VPSS_MODE_S vivpss_mode;
	CVI_U32 i;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		vivpss_mode.aenMode[i] = VI_OFFLINE_VPSS_OFFLINE + i;
	}
	if (CVI_SYS_SetVIVPSSMode(&vivpss_mode) != CVI_SUCCESS)
		return CVI_FAILURE;

	memset(&vivpss_mode, 0, sizeof(VI_VPSS_MODE_S));

	if (CVI_SYS_GetVIVPSSMode(&vivpss_mode) != CVI_SUCCESS)
		return CVI_FAILURE;

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		if (vivpss_mode.aenMode[i] != VI_OFFLINE_VPSS_OFFLINE + i) {
			UT_PRT("ut set/get vivpssmode failed\n");
			return CVI_FAILURE;
		}
		UT_PRT("ut set/get vivpssmode[%d]=0x%x\n", i, vivpss_mode.aenMode[i]);
	}

	for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
		vivpss_mode.aenMode[i] = VI_OFFLINE_VPSS_OFFLINE;
	}
	if (CVI_SYS_SetVIVPSSMode(&vivpss_mode) != CVI_SUCCESS)
		return CVI_FAILURE;

	return 0;
}

static CVI_S32 sys_ut_ion(CVI_VOID)
{
	CVI_U64 u64PhyAddr = 0;
	CVI_VOID *pVirAddr = NULL;
	CVI_U8 *pu8TestBuf;
	CVI_U32 duration1, duration_cached, duration_non_cached;
	const CVI_U32 bufLen = 0x2F0000;
	CVI_U64 add_p = 0, i = 0;
	CVI_VOID *add_v = NULL;

	if (CVI_SYS_IonAlloc((CVI_U64 *)NULL, &pVirAddr, "sys_test", bufLen) ==
	    CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc NG. Null pointer\n");
		return CVI_FAILURE;
	}

	//test ion alloc/free
	for (i = 0; i < 0x10; i++) {
		CVI_SYS_IonAlloc(&add_p, &add_v, "sys_ut_ion", SYS_UT_ION_LEN);
		UT_PRT("add_p=0x%#"PRIx64", add_v=0x%p\n", add_p, add_v);
		CVI_SYS_IonFree(add_p, add_v);
	}

	// test ion non-cached
	// --------------------------------------
	if (CVI_SYS_IonAlloc(&u64PhyAddr, &pVirAddr, "sys_test", bufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc NG.\n");
		return CVI_FAILURE;
	}

	if (u64PhyAddr == 0 || pVirAddr == 0) {
		UT_PRT("CVI_SYS_IonAlloc NG. zero phy/vir address\n");
		return CVI_FAILURE;
	}

	pu8TestBuf = malloc(bufLen);
	sys_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
	sys_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration_non_cached);
	free(pu8TestBuf);
	UT_PRT("*non-cached memcpy(%d bytes) %d - %d\n", bufLen, duration1, duration_non_cached);
	if (CVI_SYS_IonFree(u64PhyAddr, pVirAddr) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonFree NG.\n");
		return CVI_FAILURE;
	}

	// test ion cached
	// --------------------------------------
	if (CVI_SYS_IonAlloc_Cached(&u64PhyAddr, &pVirAddr, "sys_test", bufLen) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonAlloc_Cached NG.\n");
		return CVI_FAILURE;
	}

	if (u64PhyAddr == 0 || pVirAddr == 0) {
		UT_PRT("CVI_SYS_IonAlloc NG. zero phy/vir address\n");
		return CVI_FAILURE;
	}

	pu8TestBuf = malloc(bufLen);
	sys_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
	sys_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration_cached);
	free(pu8TestBuf);
	UT_PRT("*cached memcpy(%d bytes)     %d - %d\n", bufLen, duration1, duration_cached);

	if (CVI_SYS_IonFree(u64PhyAddr, pVirAddr) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_IonFree NG.\n");
		return CVI_FAILURE;
	}

	if ((duration_cached * 2) > duration_non_cached) {
		UT_PRT("cached not work. cached(%dus) non-cached(%dus)\n",
			      duration_cached, duration_non_cached);
		return CVI_FAILURE;
	}

	// test flush/invalidate
	/*
	if (sys_flush_test() != CVI_SUCCESS) {
		UT_PRT("flush NG.\n");
		return CVI_FAILURE;
	}

	if (sys_invalidate_test() != CVI_SUCCESS) {
		UT_PRT("invalidate NG.\n");
		return CVI_FAILURE;
	}*/

	return CVI_SUCCESS;
}

static CVI_S32 sys_ut_bind(CVI_VOID)
{
	MMF_CHN_S stSrcChn0, stDestChn0;
	MMF_CHN_S stSrcChn1;
	MMF_BIND_DEST_S stDests;

	stSrcChn0.enModId = CVI_ID_VI;
	stSrcChn0.s32DevId = 0;
	stSrcChn0.s32ChnId = 1;

	stDestChn0.enModId = CVI_ID_VPSS;
	stDestChn0.s32DevId = 4;
	stDestChn0.s32ChnId = 0;

	if (CVI_SYS_Bind(&stSrcChn0, &stDestChn0) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Bind(VI-VPSS) failed.\n");
		return CVI_FAILURE;
	}

	if (CVI_SYS_GetBindbyDest(&stDestChn0, &stSrcChn1) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_GetBindbyDest failed.\n");
		CVI_SYS_UnBind(&stSrcChn0, &stDestChn0);
		return CVI_FAILURE;
	}
	if (stSrcChn1.enModId != stSrcChn0.enModId ||
		stSrcChn1.s32DevId != stSrcChn0.s32DevId ||
		stSrcChn1.s32ChnId != stSrcChn0.s32ChnId) {
		UT_PRT("src chn info incorrect. !\n");
		CVI_SYS_UnBind(&stSrcChn0, &stDestChn0);
		return CVI_FAILURE;
	}

	if (CVI_SYS_GetBindbySrc(&stSrcChn0, &stDests) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_GetBindbySrc failed.\n");
		CVI_SYS_UnBind(&stSrcChn0, &stDestChn0);
		return CVI_FAILURE;
	}
	if (stDests.u32Num != 1 || stDests.astMmfChn[0].enModId != stDestChn0.enModId
		|| stDests.astMmfChn[0].s32DevId != stDestChn0.s32DevId
		|| stDests.astMmfChn[0].s32ChnId != stDestChn0.s32ChnId) {
		UT_PRT("dst chn info incorrect.\n");
		CVI_SYS_UnBind(&stSrcChn0, &stDestChn0);
		return CVI_FAILURE;
	}

	if (CVI_SYS_UnBind(&stSrcChn0, &stDestChn0) != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_UnBind(VI-VPSS) failed.\n");
	}

	return CVI_SUCCESS;
}

static CVI_S32 sys_ut_info(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MMF_VERSION_S stVersion;
	CVI_U32 u32ChipId;
	CVI_U32 u32ChipVersion;
	CVI_U32 u32PowerOnReason;
	CVI_U64 u64CurPTS;
	MOD_ID_E id = 0;

	do {
		s32Ret = CVI_SYS_GetVersion(&stVersion);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_GetVersion failed ret:%d\n", s32Ret);
			break;
		}
		UT_PRT("sys_ut_getversion version:%s success\n", stVersion.version);

		s32Ret = CVI_SYS_GetChipId(&u32ChipId);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_GetChipId failed ret:%d\n", s32Ret);
			break;
		}
		UT_PRT("sys_ut_getchipid id:%u success\n", u32ChipId);

		s32Ret = CVI_SYS_GetChipVersion(&u32ChipVersion);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_GetChipVersion failed ret:%d\n", s32Ret);
			break;
		}
		UT_PRT("sys_ut_getchipversion version:%u success\n", u32ChipVersion);

		s32Ret = CVI_SYS_GetPowerOnReason(&u32PowerOnReason);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_GetPowerOnReason failed ret:%d\n", s32Ret);
			break;
		}
		UT_PRT("sys_ut_getpwreason %s success\n", sys_ut_pwreason_to_str(u32PowerOnReason));

		s32Ret = CVI_SYS_GetCurPTS(&u64CurPTS);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_SYS_GetCurPTS failed ret:%d\n", s32Ret);
			break;
		}
		UT_PRT("sys_ut_getpwreason curpts:%"PRIu64" success\n", u64CurPTS);

		for (id = 0; id < CVI_ID_BUTT; id++) {
			UT_PRT("CVI_SYS_GetModName id:%d name:%s\n", id, CVI_SYS_GetModName(id));
		}

	} while (0);

	return s32Ret;
}

static CVI_S32 sys_ut_log(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MOD_ID_E id = 0;
	LOG_LEVEL_CONF_S stConf = {};
	CVI_S32 oldlevel[CVI_ID_BUTT] = {};

	UT_PRT("before set level:\n");
	for (id = 0; id < CVI_ID_BUTT; id++) {
		stConf.enModId = id;
		s32Ret = CVI_LOG_GetLevelConf(&stConf);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_LOG_GetLevelConf id:%d failed\n", id);
			break;
		}
		UT_PRT("id:%d, level:%d\n", id, stConf.s32Level);
		oldlevel[id] = stConf.s32Level;
	}

	// UT_PRT("set all level to CVI_DBG_DEBUG\n");
	for (id = 0; id < CVI_ID_BUTT; id++) {
		stConf.enModId = id;
		stConf.s32Level = CVI_DBG_DEBUG;
		s32Ret = CVI_LOG_SetLevelConf(&stConf);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_LOG_SetLevelConf id:%d failed\n", id);
			break;
		}
	}

	UT_PRT("after set all level to CVI_DBG_DEBUG:\n");
	for (id = 0; id < CVI_ID_BUTT; id++) {
		stConf.enModId = id;
		s32Ret = CVI_LOG_GetLevelConf(&stConf);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_LOG_GetLevelConf id:%d failed\n", id);
			break;
		}
		UT_PRT("id:%d level:%d\n", id, stConf.s32Level);
	}

	// set back log level
	for (id = 0; id < CVI_ID_BUTT; id++) {
		stConf.enModId = id;
		stConf.s32Level = oldlevel[id];
		s32Ret = CVI_LOG_SetLevelConf(&stConf);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("CVI_LOG_SetLevelConf id:%d failed\n", id);
			break;
		}
	}

	return s32Ret;
}

static CVI_S32 sys_ut_auto()
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	s32Ret |= sys_ut_setmode();
	s32Ret |= sys_ut_ion();
	s32Ret |= sys_ut_bind();
	s32Ret |= sys_ut_info();
	s32Ret |= sys_ut_log();
	return s32Ret;
}

static CVI_VOID sys_ut_handleSig(CVI_S32 nSignal, siginfo_t *si, CVI_VOID *arg)
{
	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	CVI_SYS_Exit();

	exit(1);
}

static CVI_S32 sys_ut_init(CVI_VOID)
{
	CVI_S32 s32Ret;
	struct sigaction sa = {};

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = sys_ut_handleSig;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND;	// Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}
	return CVI_SUCCESS;
}

static CVI_S32 sys_ut_deinit(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_SYS_Exit();

	return s32Ret;
}

static CVI_S32 _sys_ut_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case SYS_TEST_SET_VIVPSS_MODE: {
		s32Ret = sys_ut_setmode();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("\nsys_ut_setmode success\n");
		break;
	}
	case SYS_TEST_ION: {
		s32Ret = sys_ut_ion();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("\nsys_ion_test success\n");
		break;
	}
	case SYS_TEST_BIND: {
		s32Ret = sys_ut_bind();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("\nsys_ut_bind success\n");
		break;
	}
	case SYS_TEST_INFO: {
		s32Ret = sys_ut_info();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("\nsys_ut_info success\n");
		break;
	}
	case SYS_TEST_LOG: {
		s32Ret = sys_ut_log();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("\nsys_ut_log success\n");
		break;
	}
	case SYS_TEST_AUTO: {
		s32Ret = sys_ut_auto();
		if (s32Ret == CVI_SUCCESS)
			UT_PRT("\nsys_ut_auto success\n");
		break;
	}
	default:
		break;
	}

	return s32Ret;
}

static CVI_VOID sys_show_help(CVI_VOID)
{
	UT_PRT("%4d: Set mode test\n", SYS_TEST_SET_VIVPSS_MODE);
	UT_PRT("%4d: ion test\n", SYS_TEST_ION);
	UT_PRT("%4d: bind test\n", SYS_TEST_BIND);
	UT_PRT("%4d: info test\n", SYS_TEST_INFO);
	UT_PRT("%4d: log level test\n", SYS_TEST_LOG);
	UT_PRT("%4d: auto test\n", SYS_TEST_AUTO);
	UT_PRT("255: exit sys ut\n");
}

CVI_S32 main(CVI_S32 argc, char *argv[])
{
	CVI_S32 op;
	CVI_S32 s32Ret;

	s32Ret = sys_ut_init();

	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("sys_ut_init fail, ret=%d\n", s32Ret);
		return -1;
	}

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = _sys_ut_handle_op(op);
		UT_PRT("\nsys ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		system("stty erase ^H");
		do {
			sys_show_help();
			scanf("%d", &op);
			s32Ret = _sys_ut_handle_op(op);
			if (op != 255)
				UT_PRT("\nsys ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	sys_ut_deinit();
	return s32Ret;
}

