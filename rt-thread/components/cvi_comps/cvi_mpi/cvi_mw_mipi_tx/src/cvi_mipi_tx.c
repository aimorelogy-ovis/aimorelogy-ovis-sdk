#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

#include "rtos_types.h"
#include "errno.h"

#include "cvi_base.h"
#include "cvi_vip.h"
#include "cvi_vo.h"
#include "cvi_comm_vo.h"
#include "cvi_comm_mipi_tx.h"
#include "mipi_tx_uapi.h"
#include "cvi_mipi_tx.h"

#define MIPI_TX_CHECK_RET(ret, fmt, arg...)         \
	do {                                            \
		if ((ret) != 0) {                           \
			printf("[%d]:%s():ret=%d " fmt,     \
			__LINE__, __func__, ret, ## arg);       \
			return -1;                              \
		}                                           \
	} while (0)

#define MIPI_TX_CHECK_RET_NULL(ret, fmt, arg...)    \
	do {                                            \
		if ((ret) != 0) {                           \
			printf("[%d]:%s():ret=%d " fmt,     \
			__LINE__, __func__, ret, ## arg);       \
			return NULL;                              \
		}                                           \
	} while (0)

extern void udelay(uint32_t us);

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

int mipi_tx_cfg(int fd, const struct combo_dev_cfg_s *dev_cfg)
{
	struct mipi_tx_ctrls ctrls;
	struct combo_dev_cfg_s dev_cfg_t = *dev_cfg;

	UNUSED(fd);
	_cal_htt_extra(&dev_cfg_t);

	ctrls.cmd = CVI_VIP_MIPI_TX_SET_DEV_CFG;
	ctrls.ptr = (void *)dev_cfg;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_SET_DEV_CFG failed!\n");

	return 0;
}

int mipi_tx_send_cmd(int fd, struct cmd_info_s *cmd_info)
{
	if (cmd_info->cmd_size == 0)
		return -1;

	struct mipi_tx_ctrls ctrls;

	UNUSED(fd);
	ctrls.cmd = CVI_VIP_MIPI_TX_SET_CMD;
	ctrls.ptr = (void *)cmd_info;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_SET_CMD failed!\n");

	return 0;
}


int mipi_tx_recv_cmd(int fd, struct get_cmd_info_s *cmd_info)
{
	if (cmd_info->get_data_size == 0)
		return -1;

	struct mipi_tx_ctrls ctrls;

	UNUSED(fd);
	ctrls.cmd = CVI_VIP_MIPI_TX_GET_CMD;
	ctrls.ptr = (void *)cmd_info;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_GET_CMD failed!\n");

	return 0;
}

int mipi_tx_enable(int fd)
{
	struct mipi_tx_ctrls ctrls;

	UNUSED(fd);
	ctrls.cmd = CVI_VIP_MIPI_TX_ENABLE;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_ENABLE failed!\n");

	return 0;
}

int mipi_tx_disable(int fd)
{
	struct mipi_tx_ctrls ctrls;

	UNUSED(fd);
	ctrls.cmd = CVI_VIP_MIPI_TX_DISABLE;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_DISABLE failed!\n");

	return 0;
}

int mipi_tx_set_hs_settle(int fd, const struct hs_settle_s *hs_cfg)
{
	struct mipi_tx_ctrls ctrls;

	UNUSED(fd);
	ctrls.cmd = CVI_VIP_MIPI_TX_SET_HS_SETTLE;
	ctrls.ptr = (void *)hs_cfg;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_SET_HS_SETTLE failed!\n");

	return 0;
}

int mipi_tx_get_hs_settle(int fd, struct hs_settle_s *hs_cfg)
{
	struct mipi_tx_ctrls ctrls;

	UNUSED(fd);
	ctrls.cmd = CVI_VIP_MIPI_TX_GET_HS_SETTLE;
	ctrls.ptr = (void *)hs_cfg;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_GET_HS_SETTLE failed!\n");

	return 0;
}

int mipi_tx_poweroff(int fd)
{
	struct mipi_tx_ctrls ctrls;

	UNUSED(fd);
	ctrls.cmd = CVI_VIP_MIPI_TX_POWEROFF;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(0, &ctrls), "CVI_VIP_MIPI_TX_POWEROFF failed!\n");

	return 0;
}

int dsi_init(int devno, const struct dsc_instr *cmds, int size)
{
	for (int i = 0; i < size; i++) {
		const struct dsc_instr *instr = &cmds[i];
		struct cmd_info_s cmd_info = {
			.devno = devno,
			.cmd_size = instr->size,
			.data_type = instr->data_type,
			.cmd = (void *)instr->data
		};

		MIPI_TX_CHECK_RET(mipi_tx_send_cmd(0, &cmd_info), "dsi init failed at %d instr.\n", i);
		if (instr->delay) {
			udelay(instr->delay * 1000);
		}
	}

	return 0;
}

int mipi_tx_init(const struct combo_dev_cfg_s *dev_cfg)
{
	MIPI_TX_CHECK_RET(mipi_tx_probe(dev_cfg), "mipi_tx_probe failed!.\n");
	return 0;
}

int mipi_tx_suspend(void)
{
	MIPI_TX_CHECK_RET(mipi_tx_poweroff(0), "%s failed!\n", __func__);

	return 0;
}

int mipi_tx_resume(const struct combo_dev_cfg_s *dev_cfg)
{
	MIPI_TX_CHECK_RET(mipi_tx_init(dev_cfg), "%s failed!\n", __func__);

	return 0;
}

int mipi_tx_rstpin(int devno)
{
	struct mipi_tx_ctrls ctrls;

	ctrls.cmd = CVI_VIP_MIPI_TX_PINRST;
	MIPI_TX_CHECK_RET(mipi_tx_ctrls(devno, &ctrls), "CVI_VIP_MIPI_TX_PINRST failed!\n");

	return 0;
}
