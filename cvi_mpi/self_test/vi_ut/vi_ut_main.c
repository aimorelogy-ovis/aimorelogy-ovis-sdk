#include "vi_ut_comm.h"
#include <time.h>
#include "vi_ut_isp_rawreplayoffline.h"
#include "cvi_isp.h"
#include "cvi_gdc.h"
#include "vi_ut_isp_utils.h"

#define VI_LDC_MESH_FILE      "res/vi_mesh_MW.bin"
#define VI_LDC_DUMP_MESH_FILE   "res/dump_vi_mesh_MW.bin"
#define VI_CASE_EXIT          (255)

#define UT_INFO(_case, _func, _flags)			\
	[(_case)] = {					\
		.case_no = _case,			\
		.flags = _flags,			\
		.name = #_case,				\
		.func = _func,				\
	}

enum CASE_UT {
	CASE_BEGIN,
	// CASE_VI_IOCTRL,
	// CASE_VI_MMAP,
	// CASE_VI_POLL,
	CASE_RAW_REPLAY,
	CASE_SENSOR_ON_THE_FLY,
	CASE_SENSOR_FE_DRAM_POST_DRAM,
	CASE_SENSOR_FE_SLICE_POST_DRAM,
	CASE_PATGEN_ON_THE_FLY,
	CASE_PATGEN_FE_DRAM_POST_DRAM,
	CASE_PATGEN_FE_SLICE_POST_DRAM,
	CASE_PATGEN_ON_THE_FLY_SC,
	CASE_PATGEN_FE_DRAM_POST_SC,
	CASE_DUMP_VI_YUV_FRAME,
	CASE_DUMP_VI_RAW_FRAME,
	CASE_DUMP_VI_SMOOTH_RAW_FRAME,
	CASE_DUMP_VI_REGISTER,
	CASE_SHOW_PROC_VI,
	CASE_SHOW_PROC_VI_DBG,
	CASE_SET_CHN_CROP,
	CASE_SET_CHN_ROTATION,
	CASE_SET_CHN_FLIP_MIRROR,
	CASE_SET_CHN_LDC,
	CASE_LOAD_MESH_LDC,
	// CASE_VI_MULTI_PROCESSES_TEST,
	CASE_VI_SDK_TEST,
	CASE_VI_MULTI_INIT_TEST,
	// CASE_VI_PLD_TEST,
	// CASE_VI_SLT_TEST,
	CASE_AI_ISP_TEST,
	CASE_MAX,
};

struct vi_ut_info {
	unsigned int case_no;
	unsigned int flags;
	const char * const name;
	CVI_S32 (*func)(CVI_VOID *p);
};

static VI_UT_CTX ut_ctx;

static CVI_VOID scanf_raw_replay_info(VI_UT_CTX *pUtCtx)
{
	VI_USR_PIC_INFO_S *picInfo = &pUtCtx->rawReplayInfo;
	CVI_U32 value = 0;

	if (!pUtCtx->isAutoTest) {
		if (!picInfo->usrBlk[0] && !picInfo->usrBlk[1]) {
			UT_PRT("is_hdr_input? (0:no, 1:yes):\n");
			scanf("%d", &value);
			picInfo->isHdrOn = value ? true : false;
			UT_PRT("is_dpcm? (0:no, 1: yes):\n");
			scanf("%d", &value);
			pUtCtx->isDpcmOn = value ? true : false;
		}

		UT_PRT("LE filename:\n");
		scanf("%s", picInfo->file[0]);
		if (picInfo->isHdrOn) {
			UT_PRT("SE filename:\n");
			scanf("%s", picInfo->file[1]);
		}

		if (!picInfo->usrBlk[0] && !picInfo->usrBlk[1]) {
			UT_PRT("img width:\n");
			scanf("%d", &picInfo->u32ImgWidth);
			UT_PRT("img height:\n");
			scanf("%d", &picInfo->u32ImgHeight);
			UT_PRT("bayer format BG(0)/GB(1)/GR(2)/RG(3):\n");
			scanf("%d", &picInfo->bayFormat);
			UT_PRT("FrameRate: (user trig set 0):\n");
			scanf("%d", &picInfo->s32FrmRate);
		}
	} else {
		pUtCtx->isSkipSensor = true;
		pUtCtx->isDpcmOn = false;
		picInfo->isHdrOn = false;
		picInfo->u32ImgWidth = 1920;
		picInfo->u32ImgHeight = 1080;
		picInfo->bayFormat = 2;
		picInfo->s32FrmRate = 5;
		strcpy(picInfo->file[0], "res/2k_ballon_bayer_12_GR.bin");
	}

	UT_PRT("[RawReplay] FPS(%d) isHdr(%d), ImageSize(%dx%d), file_path(%s, %s)\n",
		picInfo->s32FrmRate,
		picInfo->isHdrOn, picInfo->u32ImgWidth, picInfo->u32ImgHeight,
		picInfo->file[0], picInfo->file[1]);
}

static CVI_S32 case_raw_replay(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	CVI_S32 exit = 0;

#ifdef CV184X_FPGA_RAW_REPLAY
	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	pUtCtx->isRawReplay = true;

	if (pUtCtx->is_use_isp_raw_replay) {
		// read config from json
		const char *rawReplayJsonPath = "replay.json";
		REPLAY_PARAM replayParam = {0};

		set_replay(&replayParam, rawReplayJsonPath);

		pUtCtx->rawReplayInfo.u32ImgWidth = replayParam.width;
		pUtCtx->rawReplayInfo.u32ImgHeight = replayParam.height;
		pUtCtx->isDpcmOn = replayParam.compressMode;
		pUtCtx->rawReplayInfo.bayFormat = replayParam.bayerFormat;
		pUtCtx->rawReplayInfo.s32FrmRate = replayParam.frameRate;
		pUtCtx->rawReplayInfo.isHdrOn = replayParam.WDRMode;
	} else {
		scanf_raw_replay_info(pUtCtx);
	}
#else
	pUtCtx->isRawReplay = true;
	pUtCtx->isSkipSensor = true;
	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;

	scanf_raw_replay_info(pUtCtx);
#endif

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

#ifdef CV184X_FPGA_RAW_REPLAY
	if (pUtCtx->is_use_isp_raw_replay) {
		return s32Ret;
	}
#endif

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	} else if (pUtCtx->rawReplayInfo.s32FrmRate == 0) {
		while (exit != 255) {
			UT_PRT("User trig(input 255 exit):");
			scanf("%d", &exit);

			if (exit == 255)
				break;

			scanf_raw_replay_info(pUtCtx);
			s32Ret = rawreplay_send_usr_pic(pUtCtx);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("send usr pic fail\n");
				return s32Ret;
			}
		}
	}

	return s32Ret;
}

static CVI_S32 case_sensor_onthefly(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_ONLINE_VPSS_OFFLINE;

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_VOID scanf_fe_dram_post_dram_info(VI_UT_CTX *pUtCtx)
{
	CVI_U32 value = 0;

	if (!pUtCtx->isAutoTest) {
		UT_PRT("is_with_isp(Rgb always with isp, only ctrl yuv)? (0:no, 1: yes):\n");
		scanf("%d", &value);
		pUtCtx->isWithIsp = value ? true : false;
	} else {
		pUtCtx->isSkipSensor = true;
		pUtCtx->isWithIsp = false;
	}

	UT_PRT("[FE_DRAM_POST_DRAM] isWithIsp(%d)\n", pUtCtx->isWithIsp);
}

static CVI_S32 case_sensor_fe_dram_post_dram(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	pUtCtx->isDpcmOn = true;

	scanf_fe_dram_post_dram_info(pUtCtx);

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_S32 case_sensor_fe_slice_post_dram(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_SLICE_VPSS_OFFLINE;

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_S32 case_patgen_onthefly(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_ONLINE_VPSS_OFFLINE;
	pUtCtx->isPatgen = true;
	pUtCtx->isSkipSensor = true;

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_S32 case_patgen_fe_dram_post_dram(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	pUtCtx->isPatgen = true;
	pUtCtx->isSkipSensor = true;
	pUtCtx->isDpcmOn = true;

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_S32 case_patgen_fe_slice_post_dram(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_SLICE_VPSS_OFFLINE;
	pUtCtx->isPatgen = true;
	pUtCtx->isSkipSensor = true;

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_S32 case_patgen_onthefly_sc(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_ONLINE_VPSS_ONLINE;
	pUtCtx->isPatgen = true;
	pUtCtx->isSkipSensor = true;
	pUtCtx->isOnlineSc = true;

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_vpss_chn_frame(0);
	}

	return s32Ret;
}

static CVI_S32 case_patgen_fe_dram_post_sc(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_ONLINE;
	pUtCtx->isPatgen = true;
	pUtCtx->isSkipSensor = true;
	pUtCtx->isOnlineSc = true;

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_vpss_chn_frame(0);
	}

	return s32Ret;
}

static CVI_S32 case_dump_vi_yuv_frame(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	CVI_S32 loop = 1, tmp;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8 pipe = 0;
	struct timespec start, end;

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	} else {
		UT_PRT("Get frm from which pipe(0~1): ");
		scanf("%d", &tmp);
		pipe = tmp;
		UT_PRT("how many loops to do(11111) is infinite: ");
		scanf("%d", &loop);
	}

	while (loop-- > 0) {
		if (pUtCtx->isAutoTest) {
			s32Ret = vi_ut_auto_test_dump(pUtCtx, false, true, false);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi_ut_auto_test_dump failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		} else {
			clock_gettime(CLOCK_MONOTONIC, &start);
			s32Ret = vi_ut_get_chn_frame(pipe, 0, true);
			if (s32Ret == CVI_SUCCESS) {
				ok++;
				clock_gettime(CLOCK_MONOTONIC, &end);
				UT_PRT("ms consumed: %f\n", (CVI_FLOAT)GetTimeDiffInUs(start, end)/1000);
			} else {
				ng--;
				break;
			}
		}
	}

	UT_PRT("Dump VI yuv TEST-%s\n", ng ? "FAIL" : "PASS");

	UT_PRT("VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

	return s32Ret;
}

static CVI_S32 case_dump_vi_raw_frame(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	CVI_U32 ok = 0, ng = 0;
	CVI_U32 dev = 0, loop = 1;
	struct timespec start, end;

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	} else {
		UT_PRT("To get raw dump from pipe(0~1): ");
		scanf("%d", &dev);
		UT_PRT("How many loops to do (1~60): ");
		scanf("%d", &loop);
	}

	while (loop-- > 0) {
		if (pUtCtx->isAutoTest) {
			s32Ret = vi_ut_auto_test_dump(pUtCtx, true, false, false);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi_ut_auto_test_dump failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		} else {
			clock_gettime(CLOCK_MONOTONIC, &start);
			s32Ret = vi_ut_get_pipe_frame(pUtCtx, dev, true);
			if (s32Ret == CVI_SUCCESS) {
				ok++;
				clock_gettime(CLOCK_MONOTONIC, &end);
				UT_PRT("ms consumed: %f\n", (CVI_FLOAT)GetTimeDiffInUs(start, end)/1000);
			} else {
				ng--;
				break;
			}
		}
	}

	UT_PRT("Dump VI raw TEST-%s\n", ng ? "FAIL" : "PASS");

	UT_PRT("VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

	return s32Ret;
}

static CVI_VOID scanf_smooth_dump_info(VI_UT_CTX *pUtCtx)
{
	VI_SMOOTH_INFO_S *smoothInfo = &pUtCtx->smoothInfo;

	if (!pUtCtx->isAutoTest) {
		UT_PRT("The vi dev to dump =\n");
		scanf("%d", &smoothInfo->u32Dev);
		UT_PRT("The ring buf number to create =\n");
		scanf("%d", &smoothInfo->u32BlkCnt);
		UT_PRT("The total frame number to get =\n");
		scanf("%d", &smoothInfo->u32TotalFrameCnt);
	} else {
		smoothInfo->u32Dev = 0;
		smoothInfo->u32BlkCnt = 2;
		smoothInfo->u32TotalFrameCnt = 2;
	}

	UT_PRT("[SmoothDump] dev(%d) blkCnt(%d), totalFrameCnt(%d)\n",
		smoothInfo->u32Dev,
		smoothInfo->u32BlkCnt,
		smoothInfo->u32TotalFrameCnt);
}

static CVI_S32 case_dump_vi_smooth_raw_frame(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;

	scanf_smooth_dump_info(pUtCtx);

	if (pUtCtx->isAutoTest) {
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = vi_ut_smooth_rawdump(pUtCtx, !pUtCtx->isAutoTest);

	return s32Ret;
}

static CVI_S32 case_dump_vi_register(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	CVI_CHAR name[64] = {0};
	FILE *fp = NULL;
	VI_DUMP_REGISTER_TABLE_S	regTbl;
	ISP_INNER_STATE_INFO_S		stInnerStateInfo;

	if (pUtCtx->isAutoTest) {
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}
#ifdef TODO_ISP
	if (CVI_ISP_QueryInnerStateInfo(0, &stInnerStateInfo) != CVI_SUCCESS) {
		UT_PRT("CVI_ISP_QueryInnerStateInfo fail");
		return CVI_FAILURE;
	}
#endif

	regTbl.MlscGainLut.RGain = stInnerStateInfo.mlscGainTable.RGain;
	regTbl.MlscGainLut.GGain = stInnerStateInfo.mlscGainTable.GGain;
	regTbl.MlscGainLut.BGain = stInnerStateInfo.mlscGainTable.BGain;

	snprintf(name, 64, "vi_dump_register.json");
	fp = fopen(name, "w");
	if (fp == NULL) {
		UT_PRT("open %s fail!!!\n", name);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_DumpHwRegisterToFile(0, fp, &regTbl);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("dump_register fail\n");
		return s32Ret;
	}

	fclose(fp);
	UT_PRT("Dump register pass\n");

	return s32Ret;
}

static CVI_S32 case_show_proc_vi(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

#ifndef CONFIG_DUAL_OS
	system("cat /proc/soph/vi");
#endif
	return s32Ret;
}

static CVI_S32 case_show_proc_vi_dbg(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

#ifndef CONFIG_DUAL_OS
	system("cat /proc/soph/vi_dbg");
#endif
	return s32Ret;
}

static CVI_VOID scanf_chn_crop_info(VI_UT_CTX *pUtCtx)
{
	VI_UT_CROP_INFO_S *pstCropInfo = &pUtCtx->stCropInfo;

	pstCropInfo->crop.bEnable = true;

	if (!pUtCtx->isAutoTest) {
		UT_PRT("Set Chn Crop. plz set:\n");
		UT_PRT("input Pipe:\n");
		scanf("%d", &pstCropInfo->pipe);
		UT_PRT("input Chn:\n");
		scanf("%d", &pstCropInfo->chn);
		UT_PRT("input x:\n");
		scanf("%d", &pstCropInfo->crop.stCropRect.s32X);
		UT_PRT("input y:\n");
		scanf("%d", &pstCropInfo->crop.stCropRect.s32Y);
		UT_PRT("input width:\n");
		scanf("%d", &pstCropInfo->crop.stCropRect.u32Width);
		UT_PRT("input height:\n");
		scanf("%d", &pstCropInfo->crop.stCropRect.u32Height);
	} else {
		pstCropInfo->pipe = 0;
		pstCropInfo->chn = 0;
		pstCropInfo->crop.stCropRect.s32X = 0;
		pstCropInfo->crop.stCropRect.s32Y = 0;
		pstCropInfo->crop.stCropRect.u32Width = 960;
		pstCropInfo->crop.stCropRect.u32Height = 540;
	}

	UT_PRT("[ChnCropInfo] pipe:%d chn:%d x:y:w:h(%d:%d:%d:%d)\n",
			pstCropInfo->pipe,
			pstCropInfo->chn,
			pstCropInfo->crop.stCropRect.s32X,
			pstCropInfo->crop.stCropRect.s32Y,
			pstCropInfo->crop.stCropRect.u32Width,
			pstCropInfo->crop.stCropRect.u32Height);
}

static CVI_S32 case_set_chn_crop(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	VI_CROP_INFO_S stCropInfo;

	scanf_chn_crop_info(pUtCtx);

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_SetChnCrop(pUtCtx->stCropInfo.pipe, pUtCtx->stCropInfo.chn, &pUtCtx->stCropInfo.crop);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnCrop(pUtCtx->stCropInfo.pipe, pUtCtx->stCropInfo.chn, &stCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_GetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if ((stCropInfo.stCropRect.s32X != pUtCtx->stCropInfo.crop.stCropRect.s32X) ||
		(stCropInfo.stCropRect.s32Y != pUtCtx->stCropInfo.crop.stCropRect.s32Y) ||
		(stCropInfo.stCropRect.u32Width != pUtCtx->stCropInfo.crop.stCropRect.u32Width) ||
		(stCropInfo.stCropRect.u32Height != pUtCtx->stCropInfo.crop.stCropRect.u32Height)) {

		UT_PRT("expect(%d,%d,%d,%d) but(%d:%d:%d:%d)\n",
			pUtCtx->stCropInfo.crop.stCropRect.s32X, pUtCtx->stCropInfo.crop.stCropRect.s32Y,
			pUtCtx->stCropInfo.crop.stCropRect.u32Width, pUtCtx->stCropInfo.crop.stCropRect.u32Height,
			stCropInfo.stCropRect.s32X, stCropInfo.stCropRect.s32Y,
			stCropInfo.stCropRect.u32Width, stCropInfo.stCropRect.u32Height);

		return CVI_FAILURE;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_VOID scanf_chn_rotation_info(VI_UT_CTX *pUtCtx)
{
	VI_ROTATION_INFO_S *pRotaInfo = &pUtCtx->rotationInfo;

	if (!pUtCtx->isAutoTest) {
		UT_PRT("Set Chn Crop. plz set:\n");
		UT_PRT("input Pipe:\n");
		scanf("%d", &pRotaInfo->pipe);
		UT_PRT("input Chn:\n");
		scanf("%d", &pRotaInfo->chn);
		UT_PRT("Rotation 0(0)/1(90)/2(180)/3(270):\n");
		scanf("%d", &pRotaInfo->rotation);
	} else {
		pRotaInfo->pipe = 0;
		pRotaInfo->chn = 0;
		pRotaInfo->rotation = 1;
	}

	UT_PRT("[RotationInfo] pipe:%d chn:%d rotation:%d\n",
			pRotaInfo->pipe,
			pRotaInfo->chn,
			pRotaInfo->rotation);
}

static CVI_S32 case_set_chn_rotation(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	VI_ROTATION_INFO_S *pRotaInfo = &pUtCtx->rotationInfo;
	ROTATION_E rot;

	scanf_chn_rotation_info(pUtCtx);

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_SetChnRotation(pRotaInfo->pipe, pRotaInfo->chn, pRotaInfo->rotation);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnRotation(pRotaInfo->pipe, pRotaInfo->chn, &rot);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_GetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if (rot != pRotaInfo->rotation) {
		UT_PRT("expect %d but %d\n", pRotaInfo->rotation, rot);
		return CVI_FAILURE;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_VOID scanf_flip_mirror_info(VI_UT_CTX *pUtCtx)
{
	VI_FLIP_MIRROR_INFO_S *pflipMirrorInfo = &pUtCtx->flipMirrorInfo;

	if (!pUtCtx->isAutoTest) {
		UT_PRT("Set Chn Crop. plz set:\n");
		UT_PRT("input Pipe:\n");
		scanf("%d", &pflipMirrorInfo->pipe);
		UT_PRT("input Chn:\n");
		scanf("%d", &pflipMirrorInfo->chn);
		UT_PRT("flip enable/disable(1/0):\n");
		scanf("%d", &pflipMirrorInfo->flip);
		UT_PRT("mirror enable/disable(1/0):\n");
		scanf("%d", &pflipMirrorInfo->mirror);
	} else {
		pflipMirrorInfo->pipe = 0;
		pflipMirrorInfo->chn = 0;
		pflipMirrorInfo->flip = 1;
		pflipMirrorInfo->mirror = 1;
	}

	UT_PRT("[FlipMirrorInfo] pipe:%d chn:%d flip:%d mirror:%d\n",
			pflipMirrorInfo->pipe,
			pflipMirrorInfo->chn,
			pflipMirrorInfo->flip,
			pflipMirrorInfo->mirror);
}

static CVI_S32 case_set_chn_flip_mirror(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	VI_FLIP_MIRROR_INFO_S *pflipMirrorInfo = &pUtCtx->flipMirrorInfo;
	CVI_BOOL flip, mirror;

	scanf_flip_mirror_info(pUtCtx);

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_SetChnFlipMirror(pflipMirrorInfo->pipe, pflipMirrorInfo->chn,
					 pflipMirrorInfo->flip, pflipMirrorInfo->mirror);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetChnFlipMirror TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnFlipMirror(pflipMirrorInfo->pipe, pflipMirrorInfo->chn, &flip, &mirror);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_GetChnFlipMirror TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if ((flip != pflipMirrorInfo->flip) || (mirror != pflipMirrorInfo->mirror)) {
		UT_PRT("flip_mirror expect (%d:%d) but (%d:%d)\n",
						pflipMirrorInfo->flip, pflipMirrorInfo->mirror, flip, mirror);
		return CVI_FAILURE;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static CVI_VOID scanf_chn_ldc_info(VI_UT_CTX *pUtCtx)
{
	VI_LDC_INFO_S *pLdcInfo = &pUtCtx->ldcInfo;
	CVI_S32 tmp;

	pLdcInfo->ldcAttr.bEnable = true;

	if (!pUtCtx->isAutoTest) {
		UT_PRT("Set Chn Crop. plz set:\n");
		UT_PRT("input Pipe:\n");
		scanf("%d", &pLdcInfo->pipe);
		UT_PRT("input Chn:\n");
		scanf("%d", &pLdcInfo->chn);

		UT_PRT("Keep AspectRatio 1(Y)/0(N):\n");
		scanf("%d", &tmp);
		pLdcInfo->ldcAttr.stAttr.bAspect = tmp;
		if (pLdcInfo->ldcAttr.stAttr.bAspect) {
			UT_PRT("Ratio (0 ~ 100):\n");
			scanf("%d", &pLdcInfo->ldcAttr.stAttr.s32XYRatio);
		} else {
			UT_PRT("XRatio (0 ~ 100):\n");
			scanf("%d", &pLdcInfo->ldcAttr.stAttr.s32XRatio);
			UT_PRT("YRatio (0 ~ 100):\n");
			scanf("%d", &pLdcInfo->ldcAttr.stAttr.s32YRatio);
		}
		UT_PRT("XOffset (-511 ~ 511):\n");
		scanf("%d", &pLdcInfo->ldcAttr.stAttr.s32CenterXOffset);
		UT_PRT("YOffset (-511 ~ 511):\n");
		scanf("%d", &pLdcInfo->ldcAttr.stAttr.s32CenterYOffset);
		UT_PRT("DistortionRatio (-300 ~ 500):\n");
		scanf("%d", &pLdcInfo->ldcAttr.stAttr.s32DistortionRatio);
	} else {
		pLdcInfo->pipe = 0;
		pLdcInfo->chn = 0;
		pLdcInfo->ldcAttr.stAttr.bAspect = true;
		pLdcInfo->ldcAttr.stAttr.s32XYRatio = 50;
		pLdcInfo->ldcAttr.stAttr.s32CenterXOffset = 50;
		pLdcInfo->ldcAttr.stAttr.s32CenterYOffset = 50;
		pLdcInfo->ldcAttr.stAttr.s32DistortionRatio = 50;
	}

	UT_PRT("[LdcInfo] pipe:%d chn:%d bAspect(%d:%d) RatioXY(%d:%d) offsetXY(%d:%d) distortionR(%d)\n",
			pLdcInfo->pipe,
			pLdcInfo->chn,
			pLdcInfo->ldcAttr.stAttr.bAspect,
			pLdcInfo->ldcAttr.stAttr.s32XYRatio,
			pLdcInfo->ldcAttr.stAttr.s32XRatio,
			pLdcInfo->ldcAttr.stAttr.s32YRatio,
			pLdcInfo->ldcAttr.stAttr.s32CenterXOffset,
			pLdcInfo->ldcAttr.stAttr.s32CenterYOffset,
			pLdcInfo->ldcAttr.stAttr.s32DistortionRatio);
}

static CVI_S32 case_set_chn_ldc(CVI_VOID *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	VI_LDC_INFO_S *pLdcInfo = &pUtCtx->ldcInfo;
	VI_LDC_ATTR_S stLDCAttr;
	MESH_DUMP_ATTR_S MeshDumpAttr = {0};

	scanf_chn_ldc_info(pUtCtx);

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_SetChnLDCAttr(pLdcInfo->pipe, pLdcInfo->chn, &pLdcInfo->ldcAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	strcpy(MeshDumpAttr.binFileName, VI_LDC_DUMP_MESH_FILE);
	MeshDumpAttr.enModId = CVI_ID_VI;
	MeshDumpAttr.viMeshAttr.chn = pLdcInfo->chn;
	MeshDumpAttr.viMeshAttr.pipe = pLdcInfo->pipe;

	s32Ret = CVI_GDC_DumpMesh(&MeshDumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_GDC_DumpMesh TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnLDCAttr(pLdcInfo->pipe, pLdcInfo->chn, &stLDCAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_GetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if (pLdcInfo->ldcAttr.stAttr.s32CenterXOffset != stLDCAttr.stAttr.s32CenterXOffset) {
		UT_PRT("expect %d but %d\n", pLdcInfo->ldcAttr.stAttr.s32CenterXOffset,
						stLDCAttr.stAttr.s32CenterXOffset);
		return CVI_FAILURE;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static int case_load_mesh_ldc(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;
	VI_LDC_INFO_S *pLdcInfo = &pUtCtx->ldcInfo;
	VI_LDC_ATTR_S stLDCAttr;
	MESH_DUMP_ATTR_S MeshDumpAttr = {0};

	pUtCtx->isAutoTest = true;
	scanf_chn_ldc_info(pUtCtx);

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	strcpy(MeshDumpAttr.binFileName, VI_LDC_MESH_FILE);
	MeshDumpAttr.enModId = CVI_ID_VI;
	MeshDumpAttr.viMeshAttr.chn = pLdcInfo->chn;
	MeshDumpAttr.viMeshAttr.pipe = pLdcInfo->pipe;

	s32Ret = CVI_GDC_LoadMesh(&MeshDumpAttr, &pLdcInfo->ldcAttr.stAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_SetChnLDCAttr(pLdcInfo->pipe, pLdcInfo->chn, &pLdcInfo->ldcAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VI_GetChnLDCAttr(pLdcInfo->pipe, pLdcInfo->chn, &stLDCAttr);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("CVI_VI_GetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	if (pLdcInfo->ldcAttr.stAttr.s32CenterXOffset != stLDCAttr.stAttr.s32CenterXOffset) {
		UT_PRT("expect %d but %d\n", pLdcInfo->ldcAttr.stAttr.s32CenterXOffset,
						stLDCAttr.stAttr.s32CenterXOffset);
		return CVI_FAILURE;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_get_chn_frame(0, 0, false);
	}

	return s32Ret;
}

static int case_vi_sdk_test(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE pipe = 0;
	VI_CHN chn = 0;
	VI_PIPE_STATUS_S stStatus;
	VI_CHN_STATUS_S stChnStatus;
	CVI_U32 devNum;

	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	if (pUtCtx->isAutoTest) {
		pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;
		pUtCtx->isBindVb = true;

		s32Ret = vi_test(pUtCtx);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	s32Ret = CVI_VI_QueryDevStatus(pipe);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("Dev(%d) QueryDevStatus failed. s32Ret: 0x%x !\n", pipe, s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_QueryPipeStatus(pipe, &stStatus);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("pipe(%d) QueryPipeStatus failed. s32Ret: 0x%x !\n", pipe, s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_QueryChnStatus(pipe, chn, &stChnStatus);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("pipe(%d) chn(%d) QueryPipeStatus failed. s32Ret: 0x%x !\n", pipe, chn, s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_GetDevNum(&devNum);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("GetdevNum Fail\n");
		return s32Ret;
	}

	return s32Ret;
}

static int case_vi_multi_init_test(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	pUtCtx->isDpcmOn = true;
	pUtCtx->multiInit.isInit = true;

	if (pUtCtx->isAutoTest) {
		pUtCtx->isPatgen = true;
		pUtCtx->isSkipSensor = true;
	}

	s32Ret = vi_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (pUtCtx->isAutoTest) {
		s32Ret = vi_ut_auto_test_dump(pUtCtx, true, true, false);
		if (s32Ret != CVI_SUCCESS) {
			UT_PRT("vi_ut_auto_test_dump failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}

static int case_ai_isp_test(void *p)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_UT_CTX *pUtCtx = (VI_UT_CTX *)p;

	pUtCtx->viVpssMode = VI_OFFLINE_VPSS_OFFLINE;
	pUtCtx->isDpcmOn = true;

	scanf_fe_dram_post_dram_info(pUtCtx);

	s32Ret = vi_ut_ai_isp_test(pUtCtx);
	if (s32Ret != CVI_SUCCESS) {
		UT_PRT("vi_ut_ai_isp_test failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static const struct vi_ut_info vi_uts[] = {
	UT_INFO(CASE_RAW_REPLAY,			case_raw_replay,		0),
	UT_INFO(CASE_SENSOR_ON_THE_FLY,			case_sensor_onthefly,		0),
	UT_INFO(CASE_SENSOR_FE_DRAM_POST_DRAM,		case_sensor_fe_dram_post_dram,	0),
	UT_INFO(CASE_SENSOR_FE_SLICE_POST_DRAM,		case_sensor_fe_slice_post_dram,	0),
	UT_INFO(CASE_PATGEN_ON_THE_FLY,			case_patgen_onthefly,		0),
	UT_INFO(CASE_PATGEN_FE_DRAM_POST_DRAM,		case_patgen_fe_dram_post_dram,	0),
	UT_INFO(CASE_PATGEN_FE_SLICE_POST_DRAM,		case_patgen_fe_slice_post_dram,	0),
	UT_INFO(CASE_PATGEN_ON_THE_FLY_SC,		case_patgen_onthefly_sc,	0),
	UT_INFO(CASE_PATGEN_FE_DRAM_POST_SC,		case_patgen_fe_dram_post_sc,	0),
	UT_INFO(CASE_DUMP_VI_YUV_FRAME,			case_dump_vi_yuv_frame,		0),
	UT_INFO(CASE_DUMP_VI_RAW_FRAME,			case_dump_vi_raw_frame,		0),
	UT_INFO(CASE_DUMP_VI_SMOOTH_RAW_FRAME,		case_dump_vi_smooth_raw_frame,	0),
	UT_INFO(CASE_DUMP_VI_REGISTER,			case_dump_vi_register,		0),
	UT_INFO(CASE_SHOW_PROC_VI,			case_show_proc_vi,		0),
	UT_INFO(CASE_SHOW_PROC_VI_DBG,			case_show_proc_vi_dbg,		0),
	UT_INFO(CASE_SET_CHN_CROP,			case_set_chn_crop,		0),
	UT_INFO(CASE_SET_CHN_ROTATION,			case_set_chn_rotation,		0),
	UT_INFO(CASE_SET_CHN_FLIP_MIRROR,		case_set_chn_flip_mirror,	0),
	UT_INFO(CASE_SET_CHN_LDC,			case_set_chn_ldc,		0),
	UT_INFO(CASE_LOAD_MESH_LDC,			case_load_mesh_ldc,		0),
	UT_INFO(CASE_VI_SDK_TEST,			case_vi_sdk_test,		0),
	UT_INFO(CASE_VI_MULTI_INIT_TEST,		case_vi_multi_init_test,	0),
	UT_INFO(CASE_AI_ISP_TEST,			case_ai_isp_test,		0),
};

static const CVI_U8 *strlwc(const CVI_U8 *in, CVI_U8 *out, CVI_U32 len)
{
	CVI_U32 i = 0;

	if (in == NULL || out == NULL || len == 0)
		return NULL;

	while (in[i] != '\0' && i < len - 1) {
		out[i] = (CVI_U8)tolower((CVI_S32)in[i]);
		i++;
	}
	out[i] = '\0';
	return out;
}

static CVI_VOID _vi_ut_show_help(CVI_VOID)
{
	CVI_U32 i = CASE_BEGIN + 1;
	CVI_U8 lowName[64] = {0};

	for (; i < CASE_MAX; i++) {
		if (vi_uts[i].case_no != i)
			continue;
		memset(lowName, 0, sizeof(lowName));
		strlwc((CVI_U8 *)vi_uts[i].name, lowName, strlen(vi_uts[i].name) + 1);
		UT_PRT("%4d: %s\n", vi_uts[i].case_no, lowName);
	}
	UT_PRT(" 255: exit\n");
}

static CVI_S32 _vi_set_tuning_dis(CVI_U8 pipe, CVI_U8 feCtrl, CVI_U8 postCtrl)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
#ifndef CONFIG_DUAL_OS
	pid_t status;
	CVI_CHAR cmd[128] = {0};

	sprintf(cmd, "echo %d,%d,%d > /sys/module/%s/parameters/tuning_dis", pipe, feCtrl, postCtrl, CHIP_TYPE);
	UT_PRT("%s\n", cmd);
	status = system(cmd);

	if (status == -1) {
		UT_PRT("system call error\n");
		s32Ret =  CVI_FAILURE;
	} else {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				UT_PRT("run shell script successfully.\n");
			} else {
				UT_PRT("run shell script fail, script exit code: %d\n",
						WEXITSTATUS(status));
				s32Ret =  CVI_FAILURE;
			}
		} else {
			UT_PRT("exit status = [%d]\n", WEXITSTATUS(status));
		}
	}
#else
	UNUSED(pipe);
	UNUSED(feCtrl);
	UNUSED(postCtrl);
#endif
	return s32Ret;
}

static CVI_VOID _vi_ut_handle_signal(CVI_S32 nSignal, siginfo_t *si, CVI_VOID *arg)
{
	VI_UT_CTX *pUtCtx = &ut_ctx;

	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	vi_ut_vi_deinit(pUtCtx);

	if (pUtCtx->isOnlineSc) {
		vi_ut_vpss_deinit(pUtCtx);
	}

	vi_ut_sys_exit();

	exit(1);
}

static CVI_S32 _vi_ut_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	const struct vi_ut_info *info = NULL;
	struct sigaction sa = {};

	if (op < CASE_BEGIN || op >= CASE_MAX) {
		UT_PRT("Invalid operation [%d]\n", op);
		return CVI_FAILURE;
	}

	info = &vi_uts[op];

	if (ut_ctx.isAutoTest && info->flags)
		return s32Ret;

	if (ut_ctx.isAutoTest) {
		_vi_set_tuning_dis(0, 1, 1);
		UT_PRT("disable tuning_dis\n");
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = _vi_ut_handle_signal;
	sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if (info->func)
		s32Ret = info->func(&ut_ctx);

	if (ut_ctx.isAutoTest) {
		_vi_set_tuning_dis(0, 0, 0);
	}

	return s32Ret;
}

static CVI_VOID vi_ut_exit(CVI_VOID)
{
	vi_ut_vi_deinit(&ut_ctx);

	if (ut_ctx.isOnlineSc) {
		vi_ut_vpss_deinit(&ut_ctx);
	}

	vi_ut_sys_exit();

	UT_PRT("exit vi ut test\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 op;
	CVI_S32 s32Ret;

	system("stty erase ^H");

#if defined(CONFIG_DUAL_OS)
#ifdef CV184X_FPGA_RAW_REPLAY
	vi_ut_sys_exit();
#endif
#endif

	memset(&ut_ctx, 0, sizeof(ut_ctx));

#ifdef CV184X_FPGA_RAW_REPLAY
	ut_ctx.isSkipSensor = 0;
	ut_ctx.is_use_isp_raw_replay = 1;
#endif

	if (argc >= 2) {
		ut_ctx.isAutoTest = true;
		if (argc == 3 && strcmp(argv[2], "CI") == 0) {
			ut_ctx.isSkipSensor = true;
		}
		op = atoi(argv[1]);
		s32Ret = _vi_ut_handle_op(op);
		UT_PRT("vi ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			_vi_ut_show_help();

			if (scanf("%u", &op) != 1) {
				while (getchar() != '\n' && getchar() != EOF) {
					;
				}
			}

			s32Ret = _vi_ut_handle_op(op);
			if (s32Ret != CVI_SUCCESS) {
				UT_PRT("vi ut op[%d] %s\n", op, "fail");
			} else {
				UT_PRT("vi ut op[%d] %s\n", op, "pass");
			}
		} while (op != VI_CASE_EXIT);
	}

	vi_ut_exit();

	return 0;
}

