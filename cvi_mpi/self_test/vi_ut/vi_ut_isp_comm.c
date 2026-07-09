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
#include "sensor_cfg.h"
#include "cvi_comm_cif.h"
#include "cvi_comm_sns.h"
#include "cvi_mipi.h"
#include "cvi_ispd2.h"
#include "cvi_isp.h"
#include "cvi_bin.h"

#include "vi_ut_isp_comm.h"

#define SUPPORT_ISP_PQTOOL

#define CVI_AE_LIB_NAME "cvi_ae_lib"
#define CVI_AWB_LIB_NAME "cvi_awb_lib"
#define CVI_AF_LIB_NAME "cvi_af_lib"

#define JSONRPC_PORT (5566)

static CVI_S32 isp_get_isp_attr_bysensor(CVI_S32 pipe, ISP_PUB_ATTR_S *pub_attr);
static CVI_S32 isp_get_isp_attr_by_rawreplay(CVI_S32 pipe, SAMPLE_ISP_RAW_REPLAY_CONFIG_S *raw_replay_config,
		ISP_PUB_ATTR_S *pub_attr);
static CVI_S32 isp_awblib_callback(CVI_S32 pipe);
static CVI_S32 isp_awblib_uncallback(CVI_S32 pipe);
static CVI_S32 isp_aelib_callback(CVI_S32 pipe);
static CVI_S32 isp_aelib_uncallback(CVI_S32 pipe);
static CVI_S32 isp_aflib_callback(CVI_S32 pipe);
static CVI_S32 isp_aflib_uncallback(CVI_S32 pipe);
static CVI_S32 isp_start(CVI_S32 pipe, SAMPLE_ISP_CONFIG_S *sampleIspConfig);
static CVI_S32 isp_run(CVI_S32 pipe);
static CVI_VOID *isp_run_thread(CVI_VOID *arg);
static CVI_VOID isp_stop(CVI_S32 pipe);
static CVI_S32 _getFileSize(FILE *fp, CVI_U32 *size);
static CVI_S32 isp_readpramfrombin(CVI_VOID);

static CVI_S32 g_ISPDaemon;
static pthread_t g_IspPid[VI_MAX_PIPE_NUM];

CVI_S32 isp_init(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config)
{
	UT_PRT("isp init\n");
	CVI_S32 ret = 0;

	ret = isp_start(pipe, sample_isp_config);
	if (ret != 0) {
		UT_PRT("Error: isp start pipe: %d fail!\n", pipe);
		return -1;
	}

	if (pipe == sample_isp_config->vi_num - 1) {
		isp_readpramfrombin();
		UT_PRT("Load the pqbin!");
	}

	ret = isp_run(pipe);
	if (ret != 0) {
		UT_PRT("Error: isp run pipe: %d fail!\n", pipe);
		return -1;
	}

	return 0;
}

CVI_S32 isp_exit(VI_PIPE pipe, SAMPLE_ISP_CONFIG_S *sample_isp_config)
{
	if (pipe == sample_isp_config->vi_num - 1)
		isp_daemon2_uninit();

	isp_stop(pipe);

	return 0;
}

static CVI_S32 isp_start(CVI_S32 pipe, SAMPLE_ISP_CONFIG_S *sampleIspConfig)
{
	UT_PRT("isp start\n");
	CVI_S32 retVal = 0;
	ISP_PUB_ATTR_S stPubAttr = {0};
	ISP_BIND_ATTR_S stBindAttr = {0};
	CVI_S32 isRawReplayMode = sampleIspConfig->is_raw_replay_mode;

	// 3alib callback
	isp_awblib_callback(pipe);
	isp_aelib_callback(pipe);
//#if ENABLE_AF_LIB
	isp_aflib_callback(pipe);
//#endif

	// bind the lib
	snprintf(stBindAttr.stAeLib.acLibName, sizeof(CVI_AE_LIB_NAME), "%s", CVI_AE_LIB_NAME);
	stBindAttr.stAeLib.s32Id = pipe;
	stBindAttr.sensorId = 0;
	snprintf(stBindAttr.stAwbLib.acLibName, sizeof(CVI_AWB_LIB_NAME), "%s", CVI_AWB_LIB_NAME);
	stBindAttr.stAwbLib.s32Id = pipe;
//#if ENABLE_AF_LIB
	snprintf(stBindAttr.stAfLib.acLibName, sizeof(CVI_AF_LIB_NAME), "%s", CVI_AF_LIB_NAME);
	stBindAttr.stAfLib.s32Id = pipe;
//#endif

	retVal = CVI_ISP_SetBindAttr(pipe, &stBindAttr);
	if (retVal != 0) {
		UT_PRT("Error: Bind Algo failed with %#x!\n", retVal);
	}

	retVal = CVI_ISP_MemInit(pipe);
	if (retVal != CVI_SUCCESS) {
		UT_PRT("Error: Init Ext memory failed with %#x!\n", retVal);
		return retVal;
	}

	if (isRawReplayMode) {
		UT_PRT("isp mw start, is raw replay mode\n");
		isp_get_isp_attr_by_rawreplay(pipe, sampleIspConfig->isp_raw_replay_config, &stPubAttr);
	} else {
		isp_get_isp_attr_bysensor(pipe, &stPubAttr);
	}

	UT_PRT("isp mw pub attr:\n");
	UT_PRT("width: %u\n", stPubAttr.stSnsSize.u32Width);
	UT_PRT("height: %u\n", stPubAttr.stSnsSize.u32Height);
	UT_PRT("enwdrmode: %d\n", stPubAttr.enWDRMode);
	UT_PRT("framerate: %f\n", stPubAttr.f32FrameRate);
	UT_PRT("bayerid: %d\n", stPubAttr.enBayer);

	retVal = CVI_ISP_SetPubAttr(pipe, &stPubAttr);
	if (retVal != CVI_SUCCESS) {
		UT_PRT("Error: SetPubAttr failed with %#x!\n", retVal);
		return retVal;
	}

	retVal = CVI_ISP_Init(pipe);
	if (retVal != CVI_SUCCESS) {
		UT_PRT("Error: ISP Init failed with %#x!\n", retVal);
		return retVal;
	}

	return retVal;
}

static CVI_S32 isp_run(CVI_S32 pipe)
{
	UT_PRT("isp run\n");
	CVI_S32 ret = 0;
	CVI_S32 *arg = malloc(sizeof(*arg));
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

#ifdef SUPPORT_ISP_PQTOOL
	if (!g_ISPDaemon) {
		UT_PRT("--------- isp daemon2 init\n");
		isp_daemon2_init(JSONRPC_PORT);
		g_ISPDaemon = 1;
	}
#endif
out:
	return ret;
}

static CVI_VOID isp_stop(CVI_S32 pipe)
{
	CVI_S32 ret;

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

static CVI_S32 isp_get_isp_attr_bysensor(CVI_S32 pipe, ISP_PUB_ATTR_S *pub_attr)
{
	CVI_S32 ret = 0;

	SENSOR_CFG_S sensor_cfg = {0};

	cvi_sns_getsize(pipe, &sensor_cfg);
	cvi_sns_getdevattr(pipe, &sensor_cfg);
	cvi_sns_getispattr(pipe, &sensor_cfg);

	pub_attr->stSnsSize.u32Width = sensor_cfg.sns_cfg.u32ImageWigth[pipe];
	pub_attr->stSnsSize.u32Height = sensor_cfg.sns_cfg.u32ImageHeight[pipe];
	pub_attr->stWndRect.u32Width = sensor_cfg.sns_cfg.u32ImageWigth[pipe];
	pub_attr->stWndRect.u32Height = sensor_cfg.sns_cfg.u32ImageHeight[pipe];

	pub_attr->enWDRMode = sensor_cfg.sns_cfg.enWDRMode[pipe];

	pub_attr->f32FrameRate = sensor_cfg.sns_cfg.f32FrameRate[pipe];
	pub_attr->enBayer = sensor_cfg.sns_cfg.enBayerFormat[pipe];
	pub_attr->u8LaneNum = sensor_cfg.sns_cfg.u8LaneNumber[pipe];
	pub_attr->u8EnableMaster = sensor_cfg.sns_cfg.u8EnMasterMode[pipe];

	return ret;
}

static CVI_S32 isp_get_isp_attr_by_rawreplay(CVI_S32 pipe, SAMPLE_ISP_RAW_REPLAY_CONFIG_S *raw_replay_config,
		ISP_PUB_ATTR_S *pub_attr)
{
	UNUSED(pipe);
	CVI_S32 ret = 0;

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

static CVI_VOID *isp_run_thread(CVI_VOID *arg)
{
	CVI_S32 retVal = 0;
	CVI_S32 pipe = *(CVI_S32 *)arg;
	CVI_U8 threadName[20];

	free(arg);
	snprintf(threadName, sizeof(threadName), "ISP%d_RUN", pipe);
	prctl(PR_SET_NAME, threadName, 0, 0, 0);

	if (pipe > 0) {
		UT_PRT("ISP Dev %d return\n", pipe);
		return NULL;
	}

	// TODO:  set the fps?
	//CVI_SYS_RegisterThermalCallback(callback_FPS);

	UT_PRT("ISP Dev %d running!\n", pipe);
	retVal = CVI_ISP_Run(pipe);
	retVal = 0;
	if (retVal != 0)
		UT_PRT("Error: CVI_ISP_Run failed with %#x!\n", retVal);

	return NULL;
}

static CVI_S32 isp_awblib_callback(CVI_S32 pipe)
{
	ALG_LIB_S stAwbLib;
	CVI_S32 ret = 0;

	stAwbLib.s32Id = pipe;
	strncpy(stAwbLib.acLibName, CVI_AWB_LIB_NAME, sizeof(stAwbLib.acLibName));
	ret = CVI_AWB_Register(pipe, &stAwbLib);
	if (ret != 0) {
		UT_PRT("Error: AWB Algo register failed!, error: %d\n",	ret);
		return ret;
	}
	return 0;
}

static CVI_S32 isp_awblib_uncallback(CVI_S32 pipe)
{
	CVI_S32 ret = 0;
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

static CVI_S32 isp_aelib_callback(CVI_S32 pipe)
{
	CVI_S32 ret = 0;
	ALG_LIB_S stAeLib;

	stAeLib.s32Id = pipe;
	strncpy(stAeLib.acLibName, CVI_AE_LIB_NAME, sizeof(stAeLib.acLibName));
	ret = CVI_AE_Register(pipe, &stAeLib);
	if (ret != 0) {
		UT_PRT("Error: AE Algo register failed!, error: %d\n",	ret);
		return ret;
	}
	return 0;
}

static CVI_S32 isp_aelib_uncallback(CVI_S32 pipe)
{
	CVI_S32 ret = 0;
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

static CVI_S32 isp_aflib_callback(CVI_S32 pipe)
{
	ALG_LIB_S stAfLib;
	CVI_S32 ret = 0;

	stAfLib.s32Id = pipe;
	strncpy(stAfLib.acLibName, CVI_AF_LIB_NAME, sizeof(stAfLib.acLibName));
	ret = CVI_AF_Register(pipe, &stAfLib);

	if (ret != 0) {
		UT_PRT("AF Algo register failed!, error: %d\n", ret);
		return ret;
	}
	return 0;
}

static CVI_S32 isp_aflib_uncallback(CVI_S32 pipe)
{
	CVI_S32 ret = 0;
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

static CVI_S32 _getFileSize(FILE *fp, CVI_U32 *size)
{
	CVI_S32 retVal = CVI_SUCCESS;

	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	rewind(fp);

	return retVal;
}

static CVI_S32 isp_readpramfrombin(CVI_VOID)
{
#define SDR_BIN_PATH "/mnt/sd/cvi_sdr.bin"
#define WDR_BIN_PATH "/mnt/sd/cvi_wdr.bin"
#define BIN_PATH_LEN 64
	CVI_S32 retVal = CVI_SUCCESS;
	FILE *filePtr = NULL;
	CVI_U8 *bufPtr = NULL;
	CVI_CHAR binName[BIN_PATH_LEN] = {0};
	CVI_U32 tempLen = 0, fileSize = 0;
	ISP_PUB_ATTR_S stPubAttr = {0};

	retVal = CVI_ISP_GetPubAttr(0, &stPubAttr);
	if (stPubAttr.enWDRMode) {
		snprintf(binName, BIN_PATH_LEN, "%s", WDR_BIN_PATH);
	} else {
		snprintf(binName, BIN_PATH_LEN, "%s", SDR_BIN_PATH);
	}

	filePtr = fopen((const CVI_CHAR *)binName, "rb");
	if (filePtr == NULL) {
		UT_PRT("Can't find bin(%s)\n", binName);
		retVal = CVI_FAILURE;
		goto ERROR_HANDLER;
	} else {
		UT_PRT("Bin exist (%s)\n", binName);
	}
	_getFileSize(filePtr, &fileSize);

	bufPtr = (CVI_U8 *)malloc(fileSize);
	if (bufPtr == NULL) {
		retVal = CVI_FAILURE;
		UT_PRT("Allocate memory fail\n");
		goto ERROR_HANDLER;
	}
	tempLen = fread(bufPtr, fileSize, 1, filePtr);
	if (tempLen <= 0) {
		UT_PRT("read data to buff fail!\n");
		retVal = CVI_FAILURE;
		goto ERROR_HANDLER;
	}

	retVal = CVI_BIN_ImportBinData(bufPtr, (CVI_U32)fileSize);
	if (retVal != CVI_SUCCESS) {
		UT_PRT("CVI_BIN_ImportBinData error! value:(0x%x)\n", retVal);
		goto ERROR_HANDLER;
	}

ERROR_HANDLER:
	if (filePtr != NULL) {
		fclose(filePtr);
	}
	if (bufPtr != NULL) {
		free(bufPtr);
	}

	return retVal;
}

