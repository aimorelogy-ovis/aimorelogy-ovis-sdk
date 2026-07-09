#ifndef __PLATFORM_MIPI_TX_H__
#define __PLATFORM_MIPI_TX_H__

#include "cvi_comm_mipi_tx.h"

int platform_mipi_tx_cfg(int fd, struct combo_dev_cfg_s *dev_cfg);
int platform_mipi_tx_send_cmd(int fd, struct cmd_info_s *cmd_info);
int platform_mipi_tx_recv_cmd(int fd, struct get_cmd_info_s *cmd_info);
int platform_mipi_tx_enable(int fd);
int platform_mipi_tx_disable(int fd);
int platform_mipi_tx_suspend(int fd);
int platform_mipi_tx_resume(int fd);
int platform_mipi_tx_set_hs_settle(int fd, const struct hs_settle_s *hs_cfg);
int platform_mipi_tx_get_hs_settle(int fd, struct hs_settle_s *hs_cfg);

#endif
