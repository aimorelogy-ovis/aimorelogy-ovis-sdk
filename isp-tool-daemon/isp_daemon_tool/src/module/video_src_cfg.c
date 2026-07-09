
#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "video_src_cfg"

#include "daemon_base.h"
#include "daemon_cfg.h"
#include "daemon_module.h"
#include "daemon_utils.h"
#include "video_src_cfg.h"

#include "cvi_buffer.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vi.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_sensor.h"
#include "cvi_mipi.h"

#include "isp_cfg.h"

#include <dlfcn.h>
#define TEAISP_SO_LIB "libteaisp.so"
static int teaisp_so_ref_cnt;
static void *teaisp_so_dl;
typedef CVI_S32 (*TEAISP_FUN)(VI_PIPE ViPipe, CVI_S32 param);
static TEAISP_FUN teaisp_init;

static int load_bnr_model(VI_PIPE ViPipe, char *model_path)
{
	FILE *fp = fopen(model_path, "r");

	if (fp == NULL) {
		clog_e("open model path failed, %s\n", model_path);
		return -1;
	}

	char line[1024];
	char *token;
	char *rest;

	TEAISP_BNR_MODEL_INFO_S stModelInfo;

	while (fgets(line, sizeof(line), fp) != NULL) {
		clog_i("load model, list, %s", line);

		rest = line;
		token = strtok_r(rest, " ", &rest);

		memset(&stModelInfo, 0, sizeof(TEAISP_BNR_MODEL_INFO_S));
		snprintf(stModelInfo.path, TEAISP_MODEL_PATH_LEN, "%s", token);

		clog_i("load model, path, %s\n", token);

		token = strtok_r(rest, " ", &rest);

		int iso = atoi(token);

		stModelInfo.enterISO = iso;
		stModelInfo.tolerance = iso * 10 / 100;

		clog_i("load model, iso, %d, %d\n", stModelInfo.enterISO,
		       stModelInfo.tolerance);
		CVI_TEAISP_BNR_SetModel(ViPipe, &stModelInfo);
	}

	fclose(fp);

	return 0;
}

static int init_teaisp_bnr(int pipe, TEAISP_DRIVER_CFG_S *cfg)
{
	int ret = CVI_SUCCESS;

	if (teaisp_so_dl == NULL) {

		clog_d("Loading TEAISP SO lib: %s\n", TEAISP_SO_LIB);

		teaisp_so_dl = dlopen(TEAISP_SO_LIB, RTLD_LAZY);
		if (!teaisp_so_dl) {
			clog_e("dlopen %s fail: %s\n", TEAISP_SO_LIB,
			       dlerror());
			return -1;
		}

		dlerror();

		teaisp_init = (TEAISP_FUN)dlsym(teaisp_so_dl, "CVI_TEAISP_Init");
		if (!teaisp_init) {
			clog_e("dlsym CVI_TEAISP_Init fail: %s\n", dlerror());
			return -1;
		}
	}

	teaisp_so_ref_cnt++;

	teaisp_init(pipe, 1);

	ret = CVI_TEAISP_BNR_Set_Driver_Init(pipe, cfg);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_TEAISP_BNR_Set_Driver_Init failed\n");
		return -1;
	}

	return 0;
}

static int deinit_teaisp_bnr(int pipe)
{
	if (CVI_TEAISP_BNR_Set_Driver_Deinit(pipe) != 0) {
		clog_e("CVI_TEAISP_BNR_Set_Driver_Deinit failed\n");
		return -1;
	}
	teaisp_so_ref_cnt--;
	if (teaisp_so_ref_cnt == 0) {
		if (teaisp_so_dl) {
			dlclose(teaisp_so_dl);
			teaisp_so_dl = NULL;
			clog_d("Unloading TEAISP SO lib: %s\n", TEAISP_SO_LIB);
		}
	}
	return 0;
}

static int init_vi_vpss_mode(uint8_t pipe_id, module_video_src_cfg_t *cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_VPSS_MODE_S stVIVPSSMode;
	VPSS_MODE_S stVPSSMode;

	memset(&stVIVPSSMode, 0, sizeof(VI_VPSS_MODE_S));
	memset(&stVPSSMode, 0, sizeof(VPSS_MODE_S));

	s32Ret = CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_SYS_GetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVIVPSSMode.aenMode[pipe_id] = cfg->vi_vpss_mode;

	s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVPSSMode.enMode = VPSS_MODE_DUAL;
	stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
	stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;
	if (cfg->vi_vpss_mode == VI_ONLINE_VPSS_ONLINE ||
		cfg->vi_vpss_mode == VI_OFFLINE_VPSS_ONLINE) {
		stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;
	} else {
		stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;
	}

	s32Ret = CVI_VPSS_SetMode(&stVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VPSS_SetMode failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static int init_mipi_rx_attr(uint8_t pipe_id, module_video_src_cfg_t *cfg, SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SNS_COMBO_DEV_ATTR_S pstRxAttr;
	SNS_INI_CFG_S *stSnsIniCfg = &sensor_cfg->sns_ini_cfg;

	UNUSED(cfg);

	memset(&pstRxAttr, 0, sizeof(SNS_COMBO_DEV_ATTR_S));

	// rest sensor
	if (stSnsIniCfg->s32RstPort[pipe_id] == -1 ||
		stSnsIniCfg->s32RstPin[pipe_id] == -1) {
		clog_d("sensor_%d reset pin shared with previous sensor, skip reset\n", pipe_id);
	} else {
		s32Ret = CVI_MIPI_SetSensorReset(
			stSnsIniCfg->MipiDev[pipe_id],
			stSnsIniCfg->s32RstPort[pipe_id],
			stSnsIniCfg->s32RstPin[pipe_id],
			stSnsIniCfg->s32RstPol[pipe_id], 1);
		if (s32Ret != CVI_SUCCESS) {
			clog_e("[ERROR] sensor_%d reset failed!\n", pipe_id);
			return s32Ret;
		}
	}

	s32Ret = CVI_MIPI_SetMipiReset(stSnsIniCfg->MipiDev[pipe_id], 1);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] mipi dev_%d reset failed!\n", pipe_id);
		return s32Ret;
	}

	if (CVI_SNS_GetSnsRxAttr(pipe_id, &pstRxAttr) != CVI_SUCCESS) {
		clog_e("[ERROR] get mipi dev_%d attr failed!\n", pipe_id);
		return CVI_FAILURE;
	}
	if (pstRxAttr.input_mode == INPUT_MODE_MIPI) {
		if (stSnsIniCfg->bHsettlen[pipe_id]) {
			pstRxAttr.mipi_attr.dphy.enable = 1;
			pstRxAttr.mipi_attr.dphy.hs_settle = stSnsIniCfg->u8Hsettle[pipe_id];
		}
	}
	if (pstRxAttr.input_mode == INPUT_MODE_MIPI ||
		pstRxAttr.input_mode == INPUT_MODE_SUBLVDS ||
		pstRxAttr.input_mode == INPUT_MODE_HISPI) {
		pstRxAttr.cif_mode = stSnsIniCfg->enSnsMode;
	}
	s32Ret = CVI_MIPI_SetMipiAttr(pipe_id, (CVI_VOID *)&pstRxAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] set mipi dev_%d attr failed!\n", pipe_id);
		return s32Ret;
	}

	s32Ret = CVI_MIPI_SetSensorClock(stSnsIniCfg->MipiDev[pipe_id], 1);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] sensor %d clock enable failed!\n", pipe_id);
		return s32Ret;
	}

	// unrest sensor
	if (stSnsIniCfg->s32RstPort[pipe_id] == -1 ||
		stSnsIniCfg->s32RstPin[pipe_id] == -1) {
		clog_d("sensor_%d unreset pin shared with previous sensor, skip unreset\n", pipe_id);
	} else {
		s32Ret = CVI_MIPI_SetSensorReset(
			stSnsIniCfg->MipiDev[pipe_id],
			stSnsIniCfg->s32RstPort[pipe_id],
			stSnsIniCfg->s32RstPin[pipe_id],
			stSnsIniCfg->s32RstPol[pipe_id], 0);
		if (s32Ret != CVI_SUCCESS) {
			clog_e("[ERROR] sensor_%d unreset failed!\n", pipe_id);
			return s32Ret;
		}
	}

	if (CVI_SNS_SetSnsProbe(pipe_id) != CVI_SUCCESS) {
		clog_e("[ERROR] sensor_%d probe failed!\n", pipe_id);
		return CVI_FAILURE;
	}

	return s32Ret;
}

static int init_vi_dev(uint8_t pipe_id, module_video_src_cfg_t *cfg, SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SNS_INI_CFG_S *stSnsIniCfg = &sensor_cfg->sns_ini_cfg;
	SNS_CFG_S *stSnsCfg = &sensor_cfg->sns_cfg;

	UNUSED(cfg);

	VI_DEV_ATTR_S       stViDevAttr;
	VI_DEV_BIND_PIPE_S  stViDevBindAttr;

	memset(&stViDevAttr, 0, sizeof(VI_DEV_ATTR_S));
	memset(&stViDevBindAttr, 0, sizeof(VI_DEV_BIND_PIPE_S));

	stViDevAttr.snrFps				= stSnsCfg->f32FrameRate[pipe_id];
	stViDevAttr.stSize.u32Width		= stSnsCfg->u32ImageWigth[pipe_id];
	stViDevAttr.stSize.u32Height	= stSnsCfg->u32ImageHeight[pipe_id];
	stViDevAttr.enIntfMode			= (VI_INTF_MODE_E)stSnsCfg->enInterFaceMode[pipe_id];
	stViDevAttr.enInputDataType		= (VI_DATA_TYPE_E)stSnsCfg->enFormatMode[pipe_id];
	stViDevAttr.enDataSeq			= (VI_YUV_DATA_SEQ_E)stSnsCfg->enYuvFormat[pipe_id];
	stViDevAttr.stWDRAttr.enWDRMode	= stSnsCfg->enWDRMode[pipe_id];
	stViDevAttr.enWorkMode			= (VI_WORK_MODE_E)stSnsCfg->enChnMode[pipe_id];
	stViDevAttr.enBayerFormat       = (BAYER_FORMAT_E)stSnsCfg->enBayerFormat[pipe_id];
	stViDevBindAttr.PipeId[0]		= stSnsIniCfg->MipiDev[pipe_id];
	stViDevBindAttr.u32Num			= 1;
	stViDevBindAttr.MipiDev			= stSnsIniCfg->MipiDev[pipe_id];

	s32Ret = CVI_VI_SetDevAttr(pipe_id, &stViDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	s32Ret = CVI_VI_SetDevBindAttr(pipe_id, &stViDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_VI_SetDevBindAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_EnableDev(pipe_id);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_VI_EnableDev failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static int init_vi_pipe(uint8_t pipe_id, module_video_src_cfg_t *cfg, SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE_ATTR_S stPipeAttr;
	SNS_CFG_S *stSnsCfg = &sensor_cfg->sns_cfg;

	memset(&stPipeAttr, 0, sizeof(VI_PIPE_ATTR_S));

	stPipeAttr.u32MaxW = stSnsCfg->u32ImageWigth[pipe_id];
	stPipeAttr.u32MaxH = stSnsCfg->u32ImageHeight[pipe_id];
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.bYuvBypassPath = stSnsCfg->bBypassIsp[pipe_id];
	stPipeAttr.enCompressMode = cfg->enable_teaisp_bnr ?
		COMPRESS_MODE_NONE : cfg->compress_mode;

	s32Ret = CVI_VI_CreatePipe(pipe_id, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_VI_CreatePipe(%d) failed with %#x!\n", pipe_id, s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_StartPipe(pipe_id);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_VI_StartPipe(%d) failed with %#x!\n", pipe_id, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

static int init_vi_chn(uint8_t pipe_id, module_video_src_cfg_t *cfg, SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CHN_ATTR_S stChnAttr;
	SNS_CFG_S *stSnsCfg = &sensor_cfg->sns_cfg;

	memset(&stChnAttr, 0, sizeof(VI_CHN_ATTR_S));

	stChnAttr.stSize.u32Width = stSnsCfg->u32ImageWigth[pipe_id];
	stChnAttr.stSize.u32Height = stSnsCfg->u32ImageHeight[pipe_id];
	stChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
	stChnAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
	stChnAttr.enCompressMode = cfg->enable_teaisp_bnr ?
		COMPRESS_MODE_NONE : cfg->compress_mode;
	stChnAttr.enPixelFormat = stSnsCfg->bBypassIsp[pipe_id] ?
		PIXEL_FORMAT_YUYV : VI_PIXEL_FORMAT;
	stChnAttr.u32Depth = 0;
	stChnAttr.u32BindVbPool = -1;
	/* fill the sensor orientation */
	stChnAttr.bMirror = cfg->enable_mirror;
	stChnAttr.bFlip = cfg->enable_flip;

	s32Ret = CVI_VI_SetChnAttr(pipe_id, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	if (CVI_SNS_SetVIFlipMirrorCB(pipe_id, 0) != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_SNS_SetVIFlipMirrorCB failed!\n");
	}

	s32Ret = CVI_VI_EnableChn(pipe_id, 0);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("[ERROR] CVI_VI_EnableChn failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

int module_video_src_init(uint8_t pipe_id, void *video_src_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SENSOR_CFG_S *sensor_cfg = get_sensor_cfg(NULL);
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)video_src_cfg;

	if (cfg->enable_teaisp_bnr) {
		TEAISP_DRIVER_CFG_S teaisp_cfg;

		memset(&teaisp_cfg, 0, sizeof(TEAISP_DRIVER_CFG_S));
		if (cfg->enable_raw_replay) {
			teaisp_cfg.enFeProcessType = TEAISP_FE_PROCESS_OFF;
		} else {
			teaisp_cfg.enFeProcessType = TEAISP_FE_PROCESS_BF16;
		}
		if (init_teaisp_bnr(pipe_id, &teaisp_cfg) != 0) {
			clog_a("init_teaisp_bnr failed\n");
		}
	}

	s32Ret = init_vi_vpss_mode(pipe_id, cfg);
	if (s32Ret != CVI_SUCCESS) {
		return s32Ret;
	}

	if (cfg->enable_raw_replay) {
		s32Ret = module_video_src_replay_init(pipe_id, video_src_cfg);
		if (s32Ret != CVI_SUCCESS) {
			clog_e("module_video_src_replay_init failed with %#x!\n", s32Ret);
		} else {
			clog_i("start replay mode success!\n");
		}
		if (cfg->enable_teaisp_bnr) {
			if (load_bnr_model(pipe_id, cfg->bnr_model_list) != 0) {
				clog_a("load_bnr_model failed\n");
			}
		}
		return s32Ret;
	}

	s32Ret = init_mipi_rx_attr(pipe_id, cfg, sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		return s32Ret;
	}

	s32Ret = init_vi_dev(pipe_id, cfg, sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		return s32Ret;
	}

	s32Ret = init_vi_pipe(pipe_id, cfg, sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		return s32Ret;
	}

	s32Ret = init_isp(pipe_id, video_src_cfg);
	if (s32Ret != CVI_SUCCESS) {
		return s32Ret;
	}

	if (!cfg->enable_patgen) {
		s32Ret = CVI_SNS_SetSnsInit(pipe_id);
		if (s32Ret != CVI_SUCCESS) {
			clog_e("[ERROR] sensor_%d init failed!\n", pipe_id);
			return s32Ret;
		}
	}

	s32Ret = init_vi_chn(pipe_id, cfg, sensor_cfg);
	if (s32Ret != CVI_SUCCESS) {
		return s32Ret;
	}

	if (cfg->enable_teaisp_bnr) {
		if (load_bnr_model(pipe_id, cfg->bnr_model_list) != 0) {
			clog_a("load_bnr_model failed\n");
		}
	}

	return s32Ret;
}

int module_video_src_deinit(uint8_t pipe_id, void *video_src_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)video_src_cfg;

	s32Ret = deinit_isp(pipe_id, cfg);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("deinit_isp failed with %#x!\n", s32Ret);
	}

	s32Ret = CVI_VI_DisableChn(pipe_id, 0);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_DisableChn failed with %#x!\n", s32Ret);
	}

	s32Ret = CVI_VI_StopPipe(pipe_id);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_StopPipe(%d) failed with %#x!\n", pipe_id, s32Ret);
	}
	s32Ret = CVI_VI_DestroyPipe(pipe_id);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_DestroyPipe(%d) failed with %#x!\n", pipe_id, s32Ret);
	}

	s32Ret = CVI_VI_DisableDev(pipe_id);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_DisableDev failed with %#x!\n", s32Ret);
	}

	s32Ret = CVI_VI_UnRegChnFlipMirrorCallBack(0, pipe_id);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("CVI_VI_UnRegChnFlipMirrorCallBack failed with %#x!\n", s32Ret);
	}

	//s32Ret = CVI_VI_UnRegPmCallBack(pipe_id);
	//if (s32Ret != CVI_SUCCESS) {
	//	clog_e("CVI_VI_UnRegPmCallBack failed with %#x!\n", s32Ret);
	//}

	if (cfg->enable_teaisp_bnr) {
		if (deinit_teaisp_bnr(pipe_id) != 0) {
			clog_a("deinit_teaisp_bnr failed\n");
		}
	}

	return s32Ret;
}
