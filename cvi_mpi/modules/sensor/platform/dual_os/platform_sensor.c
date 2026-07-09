#include "sensor_cfg.h"
#include "msg_sensor.h"
#include "cvi_msg_client.h"
#include "cvi_comm_ipcmsg.h"
#include "cvi_ipcmsg.h"

CVI_S32 platform_sns_SetSnsRxAttr(VI_PIPE ViPipe, RX_INIT_ATTR_S *pstRxAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_RX_ATTR, (CVI_VOID *)pstRxAttr,
			sizeof(RX_INIT_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set combo attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsI2c(VI_PIPE ViPipe, CVI_S32 astI2cDev, CVI_S32 s32I2cAddr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = astI2cDev;
	stPrivData.as32PrivData[1] = s32I2cAddr;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_I2C, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set I2C fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsIspAttr(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_ISP_INIT, (CVI_VOID *)pstInitAttr,
			sizeof(ISP_INIT_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set ISP init fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_RegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = IspDev;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_ISP_REG_CB, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Isp RegisterCallback fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_UnRegCallback(VI_PIPE ViPipe, ISP_DEV IspDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = IspDev;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_ISP_UNREG_CB, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Isp RegisterCallback fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsImgMode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *stSnsrMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_IMG_MODE, (CVI_VOID *)stSnsrMode,
			sizeof(ISP_CMOS_SENSOR_IMAGE_MODE_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set image mode fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsWdrMode(VI_PIPE ViPipe, WDR_MODE_E wdrMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = wdrMode;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_WDR_MODE, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set wdr mode fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetSnsRxAttr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *stDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_RX_ATTR, (CVI_VOID *)stDevAttr,
			sizeof(SNS_COMBO_DEV_ATTR_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get rx attr fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsProbe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_PROBE, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sensor probe fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsStandby(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_STANDBY, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set Sensor standby fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsRestart(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_RESTART, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set Sensor restart fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsInit(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_INIT, CVI_NULL, 0, CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set Sensor Init fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetVIFlipMirrorCB(VI_PIPE ViPipe, VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = ViDev;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_FLIPMIRROR_CB, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set mirror & filp callback to vi fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetAeDefault(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *stAeDefault)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_AE_DEFAULT, (CVI_VOID *)stAeDefault,
			sizeof(AE_SENSOR_DEFAULT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get AE default param fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetIspBlkLev(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *stBlc)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_BLK_LEVEL, (CVI_VOID *)stBlc,
			sizeof(ISP_CMOS_BLACK_LEVEL_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get AE BLACK level fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetSnsFps(VI_PIPE ViPipe, CVI_U8 fps, AE_SENSOR_DEFAULT_S *stSnsDft)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, fps);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_SNS_FPS, (CVI_VOID *)stSnsDft,
			sizeof(AE_SENSOR_DEFAULT_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sensor fps fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetExpRatio(VI_PIPE ViPipe, SNS_EXP_MAX_S *stExpMax)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_EXP_RAT, (CVI_VOID *)stExpMax,
			sizeof(SNS_EXP_MAX_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get exp max and min fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetDgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stDgain)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_DGAIN_CALC, (CVI_VOID *)stDgain,
			sizeof(SNS_GAIN_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sns dgian cal fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetAgainCalc(VI_PIPE ViPipe, SNS_GAIN_S *stAgain)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", ViPipe);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_AGAIN_CALC, (CVI_VOID *)stAgain,
			sizeof(SNS_GAIN_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sns agian cal fail, ViPipe:%d,s32Ret:%x\n", ViPipe, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_getconfiginfo(SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_CONFIG_INFO, (CVI_VOID *)sensor_cfg,
								sizeof(SENSOR_CFG_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "platform_sns_getconfiginfo sync failed, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sns_setsnsdrvcfg(SENSOR_CFG_S *sensor_cfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, 0, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_DRV_CFG, (CVI_VOID *)sensor_cfg,
								sizeof(SENSOR_CFG_S), NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "platform_sns_setsnsdrvcfg sync failed, s32Ret:%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static AHD_Callback User_Callback[VI_MAX_PIPE_NUM];

CVI_S32 MSG_SENSOR_SetAHDEnable(CVI_S32 snsid, CVI_S32 mode)
{
	if (User_Callback[snsid]) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Sensor %d callback ahd_mode: %d\n", snsid, mode);
		User_Callback[snsid](snsid, mode);
	} else {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Sensor %d callback is NULL\n", snsid);
	}

	return CVI_SUCCESS;
}

CVI_VOID platform_AHDRegisterDetect(VI_PIPE ViPipe, AHD_Callback CB)
{
	User_Callback[ViPipe] = CB;
	CVI_MSG_RegisterSNSCallback(MSG_SENSOR_SetAHDEnable);
}

CVI_S32 platform_sns_EnableDetect(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, 0, 0);

	stPrivData.as32PrivData[0] = ViPipe;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_EN_AHD_THREAD, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Enable AHD Thread fail!\n");
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetAHDMode(VI_PIPE ViPipe, CVI_U32 AhdMode)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = AhdMode;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_AHD_MODE, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Set %d AhdMode %d fail!\n", ViPipe, AhdMode);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetAhdStatus(SNS_STATUS_MSG_S *pstStatus)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if ((pstStatus->s32SnsId < 0) || (pstStatus->s32SnsId >= VI_MAX_PIPE_NUM)) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "ViPipe %d value error\n", pstStatus->s32SnsId);
		return CVI_FAILURE;
	}

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, pstStatus->s32SnsId, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_STATUS, (CVI_VOID *)pstStatus,
			sizeof(SNS_STATUS_MSG_S), CVI_NULL);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get sensor status fail, ViPipe:%d, s32Ret:%x\n", pstStatus->s32SnsId, s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetAHDInit(VI_PIPE ViPipe, bool isFirstInit)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	stPrivData.as32PrivData[0] = isFirstInit;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_AHD_INIT, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Init sensor_%d ahd mode fail!\n", ViPipe);
	}

	return s32Ret;
}

CVI_S32 platform_sns_SetAHDDeInit(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_AHD_DEINIT, CVI_NULL, 0, NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "DeInit sensor_%d ahd mode fail!\n", ViPipe);
	}

	return s32Ret;
}

CVI_S32 platform_sns_GetAHDMode(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SNS_AHD_MODE_S ahd_mode = AHD_MODE_NONE;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_AHD_MODE, (CVI_VOID *)(&ahd_mode),
			sizeof(SNS_AHD_MODE_S), CVI_NULL);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "get sensor ahd mode fail, ViPipe:%d\n", ViPipe);
		return s32Ret;
	}

	return ahd_mode;
}

CVI_S32 platform_sns_SetAHDBusInfo(VI_PIPE ViPipe, CVI_S32 astI2cDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	stPrivData.as32PrivData[0] = astI2cDev;

	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_AHD_BUSINFO, CVI_NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "set sensor_%d ahd bus info fail!\n", ViPipe);
	}

	return s32Ret;
}

CVI_S32 platform_sns_DetectAhdStatus(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	MSG_PRIV_DATA_S stPrivData;
	stPrivData.as32PrivData[0] = ahdOldType;

	CVI_U32 u32ModFd = MODFD(CVI_ID_SENSOR, ViPipe, 0);
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_SENSOR_DETECT_AHD_STATUS, (CVI_VOID *)ahdType,
			sizeof(CVI_S32), &stPrivData);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "Detect sensor status fail, ViPipe:%d\n", ViPipe);
		return s32Ret;
	}

	return s32Ret;
}