#include "cvi_debug.h"
#include "cvi_comm_mipi_tx.h"
#include "msg_mipi_tx.h"
#include "cvi_msg_client.h"
#include "cvi_errno.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define CHECK_MIPI_TX_NULL_PTR(ptr)							\
	do {									\
		if (ptr == NULL) {						\
			CVI_TRACE_VO(CVI_DBG_ERR, " Invalid null pointer\n");	\
			return CVI_ERR_VO_NULL_PTR;			\
		}								\
	} while (0)

static void _cal_htt_extra(struct combo_dev_cfg_s *dev_cfg)
{
	unsigned char lane_num = 0, bits = 0;
	unsigned short htt_old, htt_new, htt_new_extra;
	unsigned short vtt;
	float fps;
	float bit_rate_MHz;
	float clk_hs_MHz;
	float clk_hs_ns;
	float line_rate_KHz, line_time_us;
	float over_head;
	float t_period_max, t_period_real;
	struct sync_info_s *sync_info = &dev_cfg->sync_info;

	for (int i = 0; i < LANE_MAX_NUM; i++) {
		if ((dev_cfg->lane_id[i] < 0) || (dev_cfg->lane_id[i] >= MIPI_TX_LANE_MAX))
			continue;
		if (dev_cfg->lane_id[i] != MIPI_TX_LANE_CLK)
			++lane_num;
	}

	switch (dev_cfg->output_format) {
	case OUT_FORMAT_RGB_16_BIT:
		bits = 16;
		break;

	case OUT_FORMAT_RGB_18_BIT:
		bits = 18;
		break;

	case OUT_FORMAT_RGB_24_BIT:
		bits = 24;
		break;

	case OUT_FORMAT_RGB_30_BIT:
		bits = 30;
		break;

	default:
		break;
	}

	htt_old = sync_info->vid_hsa_pixels + sync_info->vid_hbp_pixels
			+ sync_info->vid_hline_pixels + sync_info->vid_hfp_pixels;
	vtt = sync_info->vid_vsa_lines + sync_info->vid_vbp_lines
			+ sync_info->vid_active_lines + sync_info->vid_vfp_lines;
	fps = dev_cfg->pixel_clk * 1000.0 / (htt_old * vtt);
	bit_rate_MHz = dev_cfg->pixel_clk / 1000.0 * bits / lane_num;
	clk_hs_MHz = bit_rate_MHz / 2;
	clk_hs_ns = 1000 / clk_hs_MHz;
	line_rate_KHz = vtt * fps / 1000;
	line_time_us = 1000 / line_rate_KHz;
	over_head = (3 * 50 * 2 * 3) + clk_hs_ns * 360;
	t_period_max = line_time_us * 1000 - over_head;
	t_period_real = clk_hs_ns * sync_info->vid_hline_pixels * bits / 4 / 2;
	htt_new = (unsigned short)(htt_old * t_period_real / t_period_max);
	if (htt_new > htt_old) {
		if (htt_new & 0x0003)
			htt_new += (4 - (htt_new & 0x0003));
		htt_new_extra = htt_new - htt_old;
		sync_info->vid_hfp_pixels += htt_new_extra;
		dev_cfg->pixel_clk = htt_new * vtt * fps / 1000;
	}
}

int platform_mipi_tx_cfg(int fd, struct combo_dev_cfg_s *dev_cfg)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_MIPI_TX, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;
	struct combo_dev_cfg_s dev_cfg_t;

	CHECK_MIPI_TX_NULL_PTR(dev_cfg);

	dev_cfg_t = *dev_cfg;
	_cal_htt_extra(&dev_cfg_t);

	stPrivData.as32PrivData[0] = fd;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_MIPI_TX_SET_DEV_CFG,
				(CVI_VOID *)&dev_cfg_t, sizeof(struct combo_dev_cfg_s), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		perror("MSG_CMD_MIPI_TX_SET_DEV_CFG");
		return s32Ret;
	}

	return s32Ret;
}

int platform_mipi_tx_send_cmd(int fd, struct cmd_info_s *cmd_info)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_MIPI_TX, 0, 0);
	struct cmd_info_msg_s cmd_info_msg;
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	CHECK_MIPI_TX_NULL_PTR(cmd_info);
	CHECK_MIPI_TX_NULL_PTR(cmd_info->cmd);
	if (cmd_info->cmd_size == 0)
		return -1;

	cmd_info_msg.devno = cmd_info->devno;
	cmd_info_msg.data_type = cmd_info->data_type;
	cmd_info_msg.cmd_size = cmd_info->cmd_size;
	memcpy(cmd_info_msg.cmd, cmd_info->cmd, cmd_info_msg.cmd_size);

	stPrivData.as32PrivData[0] = fd;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_MIPI_TX_SET_CMD,
				(CVI_VOID *)&cmd_info_msg, sizeof(struct cmd_info_msg_s), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		perror("MSG_CMD_MIPI_TX_SET_CMD");
		return s32Ret;
	}

	return s32Ret;
}


int platform_mipi_tx_recv_cmd(int fd, struct get_cmd_info_s *cmd_info)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_MIPI_TX, 0, 0);
	struct get_cmd_info_msg_s get_cmd_info_msg;
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	CHECK_MIPI_TX_NULL_PTR(cmd_info);
	CHECK_MIPI_TX_NULL_PTR(cmd_info->get_data);
	if (cmd_info->get_data_size == 0)
		return -1;

	get_cmd_info_msg.devno = cmd_info->devno;
	get_cmd_info_msg.data_type = cmd_info->data_type;
	get_cmd_info_msg.data_param = cmd_info->data_param;
	get_cmd_info_msg.get_data_size = cmd_info->get_data_size;

	stPrivData.as32PrivData[0] = fd;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_MIPI_TX_GET_CMD,
				(CVI_VOID *)&get_cmd_info_msg, sizeof(struct cmd_info_msg_s), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		perror("MSG_CMD_MIPI_TX_GET_CMD");
		return s32Ret;
	}

	memcpy(cmd_info->get_data, get_cmd_info_msg.get_data, get_cmd_info_msg.get_data_size);

	return s32Ret;
}

int platform_mipi_tx_enable(int fd)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_MIPI_TX, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	stPrivData.as32PrivData[0] = fd;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_MIPI_TX_ENABLE,
				NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		perror("MSG_CMD_MIPI_TX_ENABLE");
		return s32Ret;
	}

	return s32Ret;
}

int platform_mipi_tx_disable(int fd)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_MIPI_TX, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	stPrivData.as32PrivData[0] = fd;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_MIPI_TX_DISABLE,
				NULL, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		perror("MSG_CMD_MIPI_TX_DISABLE");
		return s32Ret;
	}

	return s32Ret;
}

int platform_mipi_tx_suspend(int fd)
{
	UNUSED(fd);
	return 0;
}

int platform_mipi_tx_resume(int fd)
{
	UNUSED(fd);
	return 0;
}

int platform_mipi_tx_set_hs_settle(int fd, const struct hs_settle_s *hs_cfg)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_MIPI_TX, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	CHECK_MIPI_TX_NULL_PTR(hs_cfg);
	stPrivData.as32PrivData[0] = fd;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_MIPI_TX_SET_HS_SETTLE,
				(CVI_VOID *)hs_cfg, sizeof(struct hs_settle_s), &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		perror("MSG_CMD_MIPI_TX_SET_HS_SETTLE");
		return s32Ret;
	}

	return s32Ret;
}

int platform_mipi_tx_get_hs_settle(int fd, struct hs_settle_s *hs_cfg)
{
	CVI_U32 u32ModFd = MODFD(CVI_ID_MIPI_TX, 0, 0);
	MSG_PRIV_DATA_S stPrivData;
	CVI_S32 s32Ret;

	CHECK_MIPI_TX_NULL_PTR(hs_cfg);
	stPrivData.as32PrivData[0] = fd;
	s32Ret = CVI_MSG_SendSync(u32ModFd, MSG_CMD_MIPI_TX_GET_HS_SETTLE,
				(CVI_VOID *)hs_cfg, 0, &stPrivData);
	if (s32Ret != CVI_SUCCESS) {
		perror("MSG_CMD_MIPI_TX_GET_HS_SETTLE");
		return s32Ret;
	}

	return s32Ret;
}
