
#define CLOG_OUTPUT_LVL CLOG_LVL_DEBUG
#define CLOG_TAG "isp_cfg"

#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include "daemon_module.h"
#include "daemon_utils.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_af_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_af.h"
#include "cvi_isp.h"

//#define ENABLE_AF_LIB  (1)

#ifdef ENABLE_AF_LIB
#include "motor_ioctl.h"
#define DEVICE_NAME "/dev/cvi-motor"
static int motor_fd;

static CVI_S32 AF_Motor_SetFocusInCb(VI_PIPE ViPipe, CVI_U8 step)
{
	struct cvi_motor_regval reg;

	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}
		reg.val = 0;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_ZOOM_IN, &reg) < 0) {
			clog_e("Focus In err\n");
			return CVI_FAILURE;
		}

		reg.val = step;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_FOCUS_IN, &reg) < 0) {
			clog_e("Focus In err\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_APPLY, &reg) < 0) {
			clog_e("Apply err\n");
			return CVI_FAILURE;
		}
	} else {
		clog_e("Not implement cb func\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 AF_Motor_SetFocusOutCb(VI_PIPE ViPipe, CVI_U8 step)
{
	struct cvi_motor_regval reg;

	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}
		reg.val = 0;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_ZOOM_OUT, &reg) < 0) {
			clog_e("Focus Out err\n");
			return CVI_FAILURE;
		}

		reg.val = step;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_FOCUS_OUT, &reg) < 0) {
			clog_e("Focus Out err\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_APPLY, &reg) < 0) {
			clog_e("Apply err\n");
			return CVI_FAILURE;
		}
	} else {
		clog_e("Not implement %d cb func\n", ViPipe);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 AF_Motor_SetZoomSpeedCb(VI_PIPE ViPipe, CVI_U8 speed)
{
	struct cvi_motor_regval reg;

	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}

		reg.val = speed;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_SET_ZOOM_SPEED, &reg) < 0) {
			clog_e("Zoom Speed err\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_APPLY, &reg) < 0) {
			clog_e("Apply err\n");
			return CVI_FAILURE;
		}
	} else {
		clog_e("Not implement %d cb func\n", ViPipe);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 AF_Motor_SetFocusSpeedCb(VI_PIPE ViPipe, CVI_U8 speed)
{
	struct cvi_motor_regval reg;

	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}

		reg.val = speed;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_SET_FOCUS_SPEED, &reg) < 0) {
			clog_e("Focus Speed err\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_APPLY, &reg) < 0) {
			clog_e("Apply err\n");
			return CVI_FAILURE;
		}
	} else {
		clog_e("Not implement %d cb func\n", ViPipe);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 AF_Motor_SetZoomInCb(VI_PIPE ViPipe, CVI_U8 step)
{
	struct cvi_motor_regval reg;

	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}

		reg.val = 0;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_FOCUS_IN, &reg) < 0) {
			clog_e("Zoom In err\n");
			return CVI_FAILURE;
		}

		reg.val = step;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_ZOOM_IN, &reg) < 0) {
			clog_e("Zoom In err\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_APPLY, &reg) < 0) {
			clog_e("Apply err\n");
			return CVI_FAILURE;
		}
	} else {
		clog_e("Not implement %d cb func\n", ViPipe);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 AF_Motor_SetZoomOutCb(VI_PIPE ViPipe, CVI_U8 step)
{
	struct cvi_motor_regval reg;

	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}
		reg.val = 0;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_FOCUS_OUT, &reg) < 0) {
			clog_e("Focus Out err\n");
			return CVI_FAILURE;
		}

		reg.val = step;

		if (ioctl(motor_fd, CVI_MOTOR_IOC_ZOOM_OUT, &reg) < 0) {
			clog_e("Zoom Out err\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_APPLY, &reg) < 0) {
			clog_e("Apply err\n");
			return CVI_FAILURE;
		}
	} else {
		clog_e("Not implement %d cb func\n", ViPipe);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 AF_Motor_SetZoomAndFocusCb(VI_PIPE ViPipe,
	AF_DIRECTION eDirz, AF_DIRECTION eDirf, CVI_U8 zoomStep, CVI_U8 focusStep)
{
	struct cvi_motor_regval reg;
	CVI_U32 zoom_dir_cmd;
	CVI_U32 focus_dir_cmd;

	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}

		reg.val = zoomStep;

		if (eDirz == AF_DIR_FAR)
			zoom_dir_cmd = CVI_MOTOR_IOC_ZOOM_IN;
		else
			zoom_dir_cmd = CVI_MOTOR_IOC_ZOOM_OUT;

		if (ioctl(motor_fd, zoom_dir_cmd, &reg) < 0) {
			clog_e("Zoom In/Out err\n");
			return CVI_FAILURE;
		}

		reg.val = focusStep;

		if (eDirf == AF_DIR_FAR)
			focus_dir_cmd = CVI_MOTOR_IOC_FOCUS_IN;
		else
			focus_dir_cmd = CVI_MOTOR_IOC_FOCUS_OUT;

		if (ioctl(motor_fd, focus_dir_cmd, &reg) < 0) {
			clog_e("Focus Out err\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_APPLY, &reg) < 0) {
			clog_e("Apply err\n");
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 AF_Motor_GetLensInfoCb(VI_PIPE ViPipe, ISP_AF_LEN_INFO_S *info)
{
	if (ViPipe == 0) {
		if (motor_fd <= 0) {
			clog_e("No motor drive\n");
			return CVI_FAILURE;
		}

		if (ioctl(motor_fd, CVI_MOTOR_IOC_GET_INFO, info) < 0) {
			clog_e("Get Info err\n");
			return CVI_FAILURE;
		}
	} else {
		clog_e("Not implement %d cb func\n", ViPipe);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static int reg_af_lib(uint8_t pipe_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ALG_LIB_S stAfLib;
	ISP_AF_MOTOR_FUNC_S motorCb;

	memset(&stAfLib, 0, sizeof(ALG_LIB_S));
	stAfLib.s32Id = pipe_id;
	strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
	s32Ret = CVI_AF_Register(pipe_id, &stAfLib);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AF Algo lib register failed!, error: %d\n", s32Ret);
		return s32Ret;
	}

	motor_fd = open(DEVICE_NAME, O_RDWR);
	if (motor_fd < 0) {
		clog_w("open motor device:%s err\n", DEVICE_NAME);
	}

	memset(&motorCb, 0, sizeof(ISP_AF_MOTOR_FUNC_S));
	motorCb.pfn_af_motor_init = NULL;
	motorCb.pfn_af_motor_deinit = NULL;
	motorCb.pfn_af_set_zoom_in = AF_Motor_SetZoomInCb;
	motorCb.pfn_af_set_zoom_out = AF_Motor_SetZoomOutCb;
	motorCb.pfn_af_set_zoom_speed = AF_Motor_SetZoomSpeedCb;
	motorCb.pfn_af_set_focus_in = AF_Motor_SetFocusInCb;
	motorCb.pfn_af_set_focus_out = AF_Motor_SetFocusOutCb;
	motorCb.pfn_af_set_focus_speed = AF_Motor_SetFocusSpeedCb;
	motorCb.pfn_af_set_zoom_focus = AF_Motor_SetZoomAndFocusCb;
	motorCb.pfn_af_get_len_info = AF_Motor_GetLensInfoCb;
	s32Ret = CVI_AF_MOTOR_Register(pipe_id, &motorCb);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AF Algo lib motor register failed!, error: %d\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int unreg_af_lib(uint8_t pipe_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ALG_LIB_S stAfLib;
	ISP_AF_MOTOR_FUNC_S motorCb;

	memset(&motorCb, 0, sizeof(ISP_AF_MOTOR_FUNC_S));
	s32Ret = CVI_AF_MOTOR_UnRegister(pipe_id, &motorCb);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AF Algo lib motor unRegister failed!, error: %d\n", s32Ret);
		return s32Ret;
	}

	close(motor_fd);
	motor_fd = -1;

	memset(&stAfLib, 0, sizeof(ALG_LIB_S));
	stAfLib.s32Id = pipe_id;
	strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
	s32Ret = CVI_AF_UnRegister(pipe_id, &stAfLib);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AF Algo lib unRegister failed!, error: %d\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}
#endif

static int reg_ae_lib(uint8_t pipe_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ALG_LIB_S stAeLib;

	memset(&stAeLib, 0, sizeof(ALG_LIB_S));
	stAeLib.s32Id = pipe_id;
	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	s32Ret = CVI_AE_Register(pipe_id, &stAeLib);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AE Algo lib register failed!, error: %d\n",	s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int unreg_ae_lib(uint8_t pipe_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ALG_LIB_S stAeLib;

	memset(&stAeLib, 0, sizeof(ALG_LIB_S));
	stAeLib.s32Id = pipe_id;
	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	s32Ret = CVI_AE_UnRegister(pipe_id, &stAeLib);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AE Algo lib unRegister failed!, error: %d\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int reg_awb_lib(uint8_t pipe_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ALG_LIB_S stAwbLib;

	memset(&stAwbLib, 0, sizeof(ALG_LIB_S));
	stAwbLib.s32Id = pipe_id;
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	s32Ret = CVI_AWB_Register(pipe_id, &stAwbLib);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AWB Algo lib register failed!, error: %d\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int unreg_awb_lib(uint8_t pipe_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ALG_LIB_S stAwbLib;

	memset(&stAwbLib, 0, sizeof(ALG_LIB_S));
	stAwbLib.s32Id = pipe_id;
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	s32Ret = CVI_AWB_UnRegister(pipe_id, &stAwbLib);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("AWB Algo lib unRegister failed!, error: %d\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int bind_3a_lib(uint8_t pipe_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ISP_BIND_ATTR_S stBindAttr;

	memset(&stBindAttr, 0, sizeof(ISP_BIND_ATTR_S));

	stBindAttr.sensorId = 0;
	snprintf(stBindAttr.stAeLib.acLibName,
		sizeof(CVI_AE_LIB_NAME), "%s", CVI_AE_LIB_NAME);
	stBindAttr.stAeLib.s32Id = pipe_id;
	snprintf(stBindAttr.stAwbLib.acLibName,
		sizeof(CVI_AWB_LIB_NAME), "%s", CVI_AWB_LIB_NAME);
	stBindAttr.stAwbLib.s32Id = pipe_id;
#ifdef ENABLE_AF_LIB
	snprintf(stBindAttr.stAfLib.acLibName,
		sizeof(CVI_AF_LIB_NAME), "%s", CVI_AF_LIB_NAME);
	stBindAttr.stAfLib.s32Id = pipe_id;
#endif

	s32Ret = CVI_ISP_SetBindAttr(pipe_id, &stBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("Bind Algo failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int init_isp_pub_attr(uint8_t pipe_id, void *video_src_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ISP_PUB_ATTR_S stPubAttr;
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)video_src_cfg;
	raw_replay_cfg_t *replay_cfg = &cfg->raw_replay_cfg;
	SENSOR_CFG_S *sensor_cfg = get_sensor_cfg(NULL);

	memset(&stPubAttr, 0, sizeof(ISP_PUB_ATTR_S));

	if (cfg->enable_raw_replay) {
		stPubAttr.stSnsSize.u32Width = replay_cfg->width;
		stPubAttr.stSnsSize.u32Height = replay_cfg->height;
		stPubAttr.stWndRect.u32Width = replay_cfg->width;
		stPubAttr.stWndRect.u32Height = replay_cfg->height;
		stPubAttr.stWndRect.s32X = 0;
		stPubAttr.stWndRect.s32Y = 0;
		stPubAttr.enWDRMode =
			replay_cfg->wdr_mode ? WDR_MODE_2To1_LINE : WDR_MODE_NONE;
		stPubAttr.f32FrameRate = replay_cfg->frame_rate;
		stPubAttr.enBayer = (ISP_BAYER_FORMAT_E)replay_cfg->bayer_format;
	} else {
		stPubAttr.stWndRect.s32X = 0;
		stPubAttr.stWndRect.s32Y = 0;
		stPubAttr.stWndRect.u32Width = sensor_cfg->sns_cfg.u32ImageWigth[pipe_id];
		stPubAttr.stWndRect.u32Height = sensor_cfg->sns_cfg.u32ImageHeight[pipe_id];
		stPubAttr.stSnsSize.u32Width = sensor_cfg->sns_cfg.u32ImageWigth[pipe_id];
		stPubAttr.stSnsSize.u32Height = sensor_cfg->sns_cfg.u32ImageHeight[pipe_id];

		stPubAttr.f32FrameRate = sensor_cfg->sns_cfg.f32FrameRate[pipe_id];
		stPubAttr.enBayer = (ISP_BAYER_FORMAT_E)sensor_cfg->sns_cfg.enBayerFormat[pipe_id];
		stPubAttr.enWDRMode = sensor_cfg->sns_cfg.enWDRMode[pipe_id];
		stPubAttr.u8SnsMode = 0;
		stPubAttr.u8LaneNum = sensor_cfg->sns_cfg.u8LaneNumber[pipe_id];
		stPubAttr.u8EnableMaster = sensor_cfg->sns_cfg.u8EnMasterMode[pipe_id];
	}

	s32Ret = CVI_ISP_SetPubAttr(pipe_id, &stPubAttr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("Set ISP Pub Attr failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int init_isp_stitch_attr(uint8_t pipe_id, void *video_src_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)video_src_cfg;

	s32Ret = CVI_ISP_SetStitchAttr(pipe_id, &cfg->stitch_attr);
	if (s32Ret != CVI_SUCCESS) {
		clog_e("Set ISP Stitch Attr failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

static int load_pqbin(uint8_t pipe_id, void *video_src_cfg)
{
	CVI_S32 ret = CVI_SUCCESS;
	FILE *fp = NULL;
	uint8_t *pqbin_buf = NULL;
	size_t pqbin_size = 0;
	char pqbin_path[256] = {0};
	SENSOR_CFG_S *sensor_cfg = get_sensor_cfg(NULL);

	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)video_src_cfg;

	if (sensor_cfg->sns_cfg.enWDRMode[pipe_id] == WDR_MODE_NONE) {
		snprintf(pqbin_path, sizeof(pqbin_path), "%s/cvi_sdr_bin", cfg->cvi_bin_path);
	} else {
		snprintf(pqbin_path, sizeof(pqbin_path), "%s/cvi_wdr_bin", cfg->cvi_bin_path);
	}

	fp = fopen(pqbin_path, "rb");
	if (!fp) {
		clog_e("Open pqbin file %s failed!\n", pqbin_path);
		return CVI_FAILURE;
	}

	fseek(fp, 0, SEEK_END);
	pqbin_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pqbin_buf = malloc(pqbin_size);
	if (!pqbin_buf) {
		clog_e("Malloc pqbin buffer failed!\n");
		fclose(fp);
		return CVI_FAILURE;
	}

	if (fread(pqbin_buf, 1, pqbin_size, fp) != pqbin_size) {
		clog_e("Read pqbin file failed!\n");
		free(pqbin_buf);
		fclose(fp);
		return CVI_FAILURE;
	}
	fclose(fp);

	ret = CVI_BIN_LoadParamFromBinEx(CVI_BIN_ID_HEADER, pqbin_buf, pqbin_size);
	if (ret != CVI_SUCCESS) {
		clog_e("Load PQ bin header failed with %#x!\n", ret);
		free(pqbin_buf);
		return ret;
	}

	ret = CVI_BIN_LoadParamFromBinEx(CVI_BIN_ID_ISP0 + pipe_id, pqbin_buf, pqbin_size);
	if (ret != CVI_SUCCESS) {
		clog_e("Load PQ bin ISP_%d failed with %#x!\n", pipe_id, ret);
		free(pqbin_buf);
		return ret;
	}
	free(pqbin_buf);
	return CVI_SUCCESS;
}

static void *isp_run_thread(void *arg)
{
	int ret = CVI_SUCCESS;
	uint8_t pipe_id = (uint8_t)(uintptr_t)arg;

	prctl(PR_SET_NAME, (unsigned long)"isp_run", 0, 0, 0);

	ret = CVI_ISP_Run(pipe_id);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_ISP_Run failed with %#x!\n", ret);
	}

	return NULL;
}

static int start_isp_run(uint8_t pipe_id)
{
	int ret = CVI_SUCCESS;

	pthread_t isp_run_thread_id;
	struct sched_param sch_params;
	pthread_attr_t attr;

	memset(&sch_params, 0, sizeof(sch_params));
	sch_params.sched_priority = 80;

	memset(&attr, 0, sizeof(pthread_attr_t));
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &sch_params);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	ret = pthread_create(&isp_run_thread_id, &attr, isp_run_thread, (void *)(uintptr_t)pipe_id);
	if (ret != 0) {
		clog_e("Create ISP run thread failed with %#x!\n", ret);
		return CVI_FAILURE;
	}
	pthread_attr_destroy(&attr);
	pthread_detach(isp_run_thread_id);
	return ret;
}

int init_isp(uint8_t pipe_id, void *video_src_cfg)
{
	CVI_S32 ret = CVI_SUCCESS;

	module_video_src_cfg_t *cfg = (module_video_src_cfg_t *)video_src_cfg;

	ret = reg_ae_lib(pipe_id);
	if (ret != CVI_SUCCESS) {
		return ret;
	}

	ret = reg_awb_lib(pipe_id);
	if (ret != CVI_SUCCESS) {
		return ret;
	}
#ifdef ENABLE_AF_LIB
	ret = reg_af_lib(pipe_id);
	if (ret != CVI_SUCCESS) {
		return ret;
	}
#endif
	ret = bind_3a_lib(pipe_id);
	if (ret != CVI_SUCCESS) {
		return ret;
	}

	ret = CVI_ISP_MemInit(pipe_id);
	if (ret != CVI_SUCCESS) {
		clog_e("Init ISP memory failed with %#x!\n", ret);
		return ret;
	}

	ret = init_isp_pub_attr(pipe_id, video_src_cfg);
	if (ret != CVI_SUCCESS) {
		return ret;
	}

	if (cfg->enable_isp_stitch) {
		ret = init_isp_stitch_attr(pipe_id, video_src_cfg);
		if (ret != CVI_SUCCESS) {
			return ret;
		}
	}

	if (cfg->bypass_src_frame_num >= 0) {
		CVI_ISP_SetBypassFrm(pipe_id, cfg->bypass_src_frame_num);
	}

	ret = CVI_ISP_Init(pipe_id);
	if (ret != CVI_SUCCESS) {
		clog_e("Init ISP failed with %#x!\n", ret);
		return ret;
	}

	ret = load_pqbin(pipe_id, video_src_cfg);
	if (ret != CVI_SUCCESS) {
		clog_e("Load PQ bin failed with %#x!\n", ret);
	}

	ret = start_isp_run(pipe_id);
	if (ret != CVI_SUCCESS) {
		return ret;
	}

	return ret;
}

int deinit_isp(uint8_t pipe_id, void *video_src_cfg)
{
	int ret = CVI_SUCCESS;

	UNUSED(video_src_cfg);

	ret = CVI_ISP_Exit(pipe_id);
	if (ret != CVI_SUCCESS) {
		clog_e("CVI_ISP_Exit failed with %#x!\n", ret);
	}

#ifdef ENABLE_AF_LIB
	ret = unreg_af_lib(pipe_id);
#endif
	ret = unreg_ae_lib(pipe_id);
	ret = unreg_awb_lib(pipe_id);

	return ret;
}
