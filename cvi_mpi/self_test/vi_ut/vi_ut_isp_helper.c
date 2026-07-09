#ifndef CV184X_FPGA_RAW_REPLAY
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_awb.h"
#include "cvi_ae.h"
#include "cvi_af.h"

#include "cvi_sns_ctrl.h"
#include "cvi_comm_cif.h"
#include "cvi_comm_sns.h"
#include "cvi_mipi.h"

#include "cvi_ispd2.h"

#include "ut_comm.h"
#include "cvi_isp.h"
#include "cvi_bin.h"
#include "vi_ut_isp_helper.h"
#include "sensor_cfg.h"
#include "cvi_sensor.h"
#include "cvi_buffer.h"

#define CVI_AE_LIB_NAME "cvi_ae_lib"
#define CVI_AWB_LIB_NAME "cvi_awb_lib"
#define CVI_AF_LIB_NAME "cvi_af_lib"

#define JSONRPC_PORT (5566)

static int _getFileSize(FILE *fp, CVI_U32 *size);
static int isp_readpramfrombin(void);

static pthread_t g_IspPid[VI_MAX_DEV_NUM];
static int g_ISPDaemon;
static int g_ISPCount;
static int isp_get_isp_attr_bysensor(int pipe, ISP_PUB_ATTR_S *pub_attr);
static int isp_get_isp_attr_by_rawreplay(int pipe, SAMPLE_ISP_RAW_REPLAY_CONFIG_S *raw_replay_config,
		ISP_PUB_ATTR_S *pub_attr);
static void *isp_run_thread(void *arg);
static int isp_awblib_callback(int pipe);
static int isp_aelib_callback(int pipe);
static int isp_aflib_callback(int pipe);
static int isp_awblib_uncallback(int pipe);
static int isp_aelib_uncallback(int pipe);
static int isp_aflib_uncallback(int pipe);
static int isp_start(int pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config);
static int isp_run(int pipe);
static void isp_stop(int pipe);
static int isp_init(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config);
static int isp_exit(VI_PIPE pipe);

int SAMPLE_ISP_Init(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config)
{
	return isp_init(pipe, sample_isp_config);
}

int SAMPLE_ISP_Exit(VI_PIPE pipe)
{
	return isp_exit(pipe);
}

static int isp_run(int pipe)
{
	UT_PRT("isp run\n");
	int ret = 0;
	int *arg = malloc(sizeof(*arg));
	struct sched_param param;
	pthread_attr_t attr;

	if (arg == NULL) {
		UT_PRT("Error: malloc failed\n");
		goto out;
	}

	*arg = pipe;
	param.sched_priority = 80;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	ret = pthread_create(&g_IspPid[pipe], &attr, isp_run_thread, arg);
	if (ret != 0) {
		UT_PRT("Error: create isp running thread failed!, error: %d, %s\r\n",
					ret, strerror(ret));
		goto out;
	}

	if (!g_ISPDaemon) {
		UT_PRT("--------- isp daemon2 init\n");
		isp_daemon2_init(JSONRPC_PORT);
		g_ISPDaemon = 1;
	}
out:
	return ret;
}

static void isp_stop(int pipe)
{
	int ret;

	if (g_IspPid[pipe]) {
		ret = CVI_ISP_Exit(pipe);
		if (ret != 0) {
			UT_PRT("Error: CVI_ISP_Exit fail with pipe: %d\n", pipe);
			return;
		}

		pthread_join(g_IspPid[pipe], NULL);
		g_IspPid[pipe] = 0;

		isp_awblib_uncallback(pipe);
		isp_aelib_uncallback(pipe);
		isp_aflib_uncallback(pipe);
	}
}

static int isp_init(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config)
{
	int ret = 0;

	UT_PRT("isp init %d\n", pipe);

	ret = isp_start(pipe, sample_isp_config);
	if (ret != 0) {
		UT_PRT("Error: isp start pipe: %d fail!\n", pipe);
		return -1;
	}

	// same to ISPDaemon
	if (!g_ISPDaemon) {
		isp_readpramfrombin();
		UT_PRT("Load the pqbin!");
	}

	ret = isp_run(pipe);
	if (ret != 0) {
		UT_PRT("Error: isp run pipe: %d fail!\n", pipe);
		return -1;
	}

	g_ISPCount++;

	return 0;
}

static int isp_exit(VI_PIPE pipe)
{
	g_ISPCount--;

	isp_stop(pipe);

	if (!g_ISPCount)
		isp_daemon2_uninit();

	return 0;
}

static void *isp_run_thread(void *arg)
{
	int ret = 0;
	int pipe = *(int *)arg;
	char szThreadName[20];

	free(arg);
	snprintf(szThreadName, sizeof(szThreadName), "ISP%d_RUN", pipe);
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	if (pipe > 0) {
		UT_PRT("ISP Dev %d return\n", pipe);
		return NULL;
	}

	UT_PRT("ISP Dev %d running!\n", pipe);
	ret = CVI_ISP_Run(pipe);
	ret = 0;
	if (ret != 0)
		UT_PRT("Error: CVI_ISP_Run failed with %#x!\n", ret);

	return NULL;
}

static int isp_get_isp_attr_by_rawreplay(int pipe, SAMPLE_ISP_RAW_REPLAY_CONFIG_S *raw_replay_config,
		ISP_PUB_ATTR_S *pub_attr)
{
	UNUSED(pipe);
	int ret = 0;

	if (raw_replay_config == NULL) {
		UT_PRT("Error: no raw replay information!\n");
		return -1;
	}

	pub_attr->stSnsSize.u32Width = raw_replay_config->width;
	pub_attr->stSnsSize.u32Height = raw_replay_config->height;
	pub_attr->stWndRect.u32Width = raw_replay_config->width;
	pub_attr->stWndRect.u32Height = raw_replay_config->height;

	pub_attr->enWDRMode = raw_replay_config->wdr_mode;

	if (raw_replay_config->frame_rate > 0) {
		pub_attr->f32FrameRate = raw_replay_config->frame_rate;
	} else {
		pub_attr->f32FrameRate = 20;
	}

	pub_attr->enBayer = raw_replay_config->bayer_id;

	return ret;
}


static int isp_awblib_callback(int pipe)
{
	ALG_LIB_S stAwbLib;
	int ret = 0;

	stAwbLib.s32Id = pipe;
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	ret = CVI_AWB_Register(pipe, &stAwbLib);
	if (ret != 0) {
		UT_PRT("Error: Pipe(%d) AWB Algo register failed!, error: %d\n", pipe, ret);
		return ret;
	}
	return 0;
}

static int isp_aelib_callback(int pipe)
{
	int ret = 0;
	ALG_LIB_S stAeLib;

	stAeLib.s32Id = pipe;
	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	ret = CVI_AE_Register(pipe, &stAeLib);
	if (ret != 0) {
		UT_PRT("Error: Pipe(%d) AE Algo register failed!, error: %d\n",	pipe, ret);
		return ret;
	}
	return 0;
}

static int isp_aflib_callback(int pipe)
{
	ALG_LIB_S stAfLib;
	int ret = 0;

	stAfLib.s32Id = pipe;
	strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
	ret = CVI_AF_Register(pipe, &stAfLib);

	if (ret != 0) {
		UT_PRT("Error: Pipe(%d) AF Algo register failed!, error: %d\n", pipe, ret);
		return ret;
	}
	return 0;
}

static int isp_awblib_uncallback(int pipe)
{
	int ret = 0;
	ALG_LIB_S stAwbLib;

	stAwbLib.s32Id = pipe;
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	ret = CVI_AWB_UnRegister(pipe, &stAwbLib);
	if (ret) {
		UT_PRT("Error: AWB Algo unRegister failed!, error: %d\n",	ret);
		return ret;
	}
	return 0;
}

static int isp_aelib_uncallback(int pipe)
{
	int ret = 0;
	ALG_LIB_S stAeLib;

	stAeLib.s32Id = pipe;
	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	ret = CVI_AE_UnRegister(pipe, &stAeLib);
	if (ret) {
		UT_PRT("Error: AE Algo unRegister failed!, error: %d\n",	ret);
		return ret;
	}
	return 0;
}

static int isp_aflib_uncallback(int pipe)
{
	int ret = 0;
	ALG_LIB_S stAfLib;

	stAfLib.s32Id = pipe;
	strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
	ret = CVI_AF_UnRegister(pipe, &stAfLib);
	if (ret) {
		UT_PRT("Error: AF Algo unRegister failed!, error: %d\n",	ret);
		return ret;
	}
	return 0;
}

static int isp_get_isp_attr_bysensor(int pipe, ISP_PUB_ATTR_S *pub_attr)
{
	int ret = 0;

	SENSOR_CFG_S sensor_cfg = {0};

	ret = CVI_SNS_ParseIni(&sensor_cfg);
	if (ret == CVI_FAILURE) {
		UT_PRT("[ERROR] parse ini failed\n");
	}

	CVI_SNS_GetConfigInfo(&sensor_cfg);

	pub_attr->stSnsSize.u32Width = sensor_cfg.sns_cfg.u32ImageWigth[pipe];
	pub_attr->stSnsSize.u32Height = sensor_cfg.sns_cfg.u32ImageHeight[pipe];
	pub_attr->stWndRect.u32Width = sensor_cfg.sns_cfg.u32ImageWigth[pipe];
	pub_attr->stWndRect.u32Height = sensor_cfg.sns_cfg.u32ImageHeight[pipe];

	pub_attr->enWDRMode = sensor_cfg.sns_cfg.enWDRMode[pipe];

	pub_attr->f32FrameRate = sensor_cfg.sns_cfg.f32FrameRate[pipe];
	pub_attr->enBayer = (ISP_BAYER_FORMAT_E)sensor_cfg.sns_cfg.enBayerFormat[pipe];
	pub_attr->u8LaneNum = sensor_cfg.sns_cfg.u8LaneNumber[pipe];
	pub_attr->u8EnableMaster = sensor_cfg.sns_cfg.u8EnMasterMode[pipe];

	return ret;
}


static int isp_start(int pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config)
{
	UT_PRT("isp start\n");
	int ret = 0;
	ISP_PUB_ATTR_S stPubAttr = {0};
	ISP_BIND_ATTR_S stBindAttr = {0};
	int is_raw_replay_mode = sample_isp_config->is_raw_replay_mode;

	isp_awblib_callback(pipe);
	isp_aelib_callback(pipe);
	isp_aflib_callback(pipe);

	snprintf(stBindAttr.stAeLib.acLibName, sizeof(CVI_AE_LIB_NAME), "%s", CVI_AE_LIB_NAME);
	stBindAttr.stAeLib.s32Id = pipe;
	stBindAttr.sensorId = pipe;
	snprintf(stBindAttr.stAwbLib.acLibName, sizeof(CVI_AWB_LIB_NAME), "%s", CVI_AWB_LIB_NAME);
	stBindAttr.stAwbLib.s32Id = pipe;
	snprintf(stBindAttr.stAfLib.acLibName, sizeof(CVI_AF_LIB_NAME), "%s", CVI_AF_LIB_NAME);
	stBindAttr.stAfLib.s32Id = pipe;

	ret = CVI_ISP_SetBindAttr(pipe, &stBindAttr);
	if (ret != 0) {
		UT_PRT("Error: Bind Algo failed with %#x!\n", ret);
	}

	ret = CVI_ISP_MemInit(pipe);
	if (ret != CVI_SUCCESS) {
		UT_PRT("Error: Init Ext memory failed with %#x!\n", ret);
		return ret;
	}

	if (is_raw_replay_mode) {
		UT_PRT("isp mw start, is raw replay mode\n");
		isp_get_isp_attr_by_rawreplay(pipe, sample_isp_config->isp_raw_replay_config, &stPubAttr);
	} else {
		isp_get_isp_attr_bysensor(pipe, &stPubAttr);
	}

	UT_PRT("isp mw pub attr:\n");
	UT_PRT("width: %u\n", stPubAttr.stSnsSize.u32Width);
	UT_PRT("height: %u\n", stPubAttr.stSnsSize.u32Height);
	UT_PRT("enwdrmode: %d\n", stPubAttr.enWDRMode);
	UT_PRT("framerate: %f\n", stPubAttr.f32FrameRate);
	UT_PRT("bayerid: %d\n", stPubAttr.enBayer);

	ret = CVI_ISP_SetPubAttr(pipe, &stPubAttr);
	if (ret != CVI_SUCCESS) {
		UT_PRT("Error: SetPubAttr failed with %#x!\n", ret);
		return ret;
	}

	ret = CVI_ISP_Init(pipe);
	if (ret != CVI_SUCCESS) {
		UT_PRT("Error: ISP Init failed with %#x!\n", ret);
		return ret;
	}

	return ret;
}

static int _getFileSize(FILE *fp, CVI_U32 *size)
{
	CVI_S32 ret = CVI_SUCCESS;

	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	rewind(fp);

	return ret;
}

static int isp_readpramfrombin(void)
{
#define SDR_BIN_PATH "/mnt/sd/cvi_sdr.bin"
#define WDR_BIN_PATH "/mnt/sd/cvi_wdr.bin"
#define BIN_PATH_LEN 64
	CVI_S32 ret = CVI_SUCCESS;
	FILE *fp = NULL;
	CVI_U8 *buf = NULL;
	CVI_CHAR binName[BIN_PATH_LEN] = {0};
	CVI_U32 u32TempLen = 0, u32FileSize = 0;
	ISP_PUB_ATTR_S stPubAttr = {0};

	ret = CVI_ISP_GetPubAttr(0, &stPubAttr);
	if (stPubAttr.enWDRMode) {
		snprintf(binName, BIN_PATH_LEN, "%s", WDR_BIN_PATH);
	} else {
		snprintf(binName, BIN_PATH_LEN, "%s", SDR_BIN_PATH);
	}

	fp = fopen((const CVI_CHAR *)binName, "rb");
	if (fp == NULL) {
		UT_PRT("Can't find bin(%s)\n", binName);
		ret = CVI_FAILURE;
		goto ERROR_HANDLER;
	} else {
		UT_PRT("Bin exist (%s)\n", binName);
	}
	_getFileSize(fp, &u32FileSize);

	buf = (CVI_U8 *)malloc(u32FileSize);
	if (buf == NULL) {
		ret = CVI_FAILURE;
		UT_PRT("Allocate memory fail\n");
		goto ERROR_HANDLER;
	}
	u32TempLen = fread(buf, u32FileSize, 1, fp);
	if (u32TempLen <= 0) {
		UT_PRT("read data to buff fail!\n");
		ret = CVI_FAILURE;
		goto ERROR_HANDLER;
	}

	ret = CVI_BIN_ImportBinData(buf, (CVI_U32)u32FileSize);
	if (ret != CVI_SUCCESS) {
		UT_PRT("CVI_BIN_ImportBinData error! value:(0x%x)\n", ret);
		goto ERROR_HANDLER;
	}

ERROR_HANDLER:
	if (fp != NULL) {
		fclose(fp);
	}
	if (buf != NULL) {
		free(buf);
	}

	return ret;
}

#endif
