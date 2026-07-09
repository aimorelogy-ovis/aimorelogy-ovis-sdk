#include "platform_mipi_tx.h"

int mipi_tx_cfg(int fd, struct combo_dev_cfg_s *dev_cfg)
{
        return platform_mipi_tx_cfg(fd, dev_cfg);
}

int mipi_tx_send_cmd(int fd, struct cmd_info_s *cmd_info)
{
        return platform_mipi_tx_send_cmd(fd, cmd_info);
}

int mipi_tx_recv_cmd(int fd, struct get_cmd_info_s *cmd_info)
{
        return platform_mipi_tx_recv_cmd(fd, cmd_info);
}

int mipi_tx_enable(int fd)
{
        return platform_mipi_tx_enable(fd);
}

int mipi_tx_disable(int fd)
{
        return platform_mipi_tx_disable(fd);
}

int mipi_tx_suspend(int fd)
{
        return platform_mipi_tx_suspend(fd);
}

int mipi_tx_resume(int fd)
{
        return platform_mipi_tx_resume(fd);
}

int mipi_tx_set_hs_settle(int fd, const struct hs_settle_s *hs_cfg)
{
        return platform_mipi_tx_set_hs_settle(fd, hs_cfg);
}

int mipi_tx_get_hs_settle(int fd, struct hs_settle_s *hs_cfg)
{
        return platform_mipi_tx_get_hs_settle(fd, hs_cfg);
}