#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "cvi_type.h"

#include "app_ipcam_comm.h"
#include "app_ipcam_mipi_tx.h"

CVI_S32 app_ipcam_MipiTx_Enable(
    const int* const ps32DevNo,
    const CVI_CHAR* const pchPanelName,
    const struct combo_dev_cfg_s* const pstDevCfg,
    const struct hs_settle_s* const pstHsTimingCfg,
    const struct dsc_instr* const pstDsiInitCmds,
    const CVI_S32* const ps32DsiInitCmdsSize,
    CVI_S32* const ps32MipiTxFd)
{
    _NULL_POINTER_CHECK_(ps32DevNo, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pchPanelName, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pstDevCfg, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pstHsTimingCfg, CVI_FAILURE);
    _NULL_POINTER_CHECK_(pstDsiInitCmds, CVI_FAILURE);
    _NULL_POINTER_CHECK_(ps32DsiInitCmdsSize, CVI_FAILURE);
    _NULL_POINTER_CHECK_(ps32MipiTxFd, CVI_FAILURE);

    *ps32MipiTxFd = 0;
    ps32MipiTxFd[0] = open(MIPI_TX_NAME, O_RDWR | O_NONBLOCK, 0);
    if (*ps32MipiTxFd == -1) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Cannot open '%s': %d, %s\n", MIPI_TX_NAME, errno, strerror(errno));
        return CVI_FAILURE;
    }

    APP_CHK_RET(mipi_tx_cfg(*ps32MipiTxFd, (struct combo_dev_cfg_s*)pstDevCfg), "mipi_tx_cfg");

    for (CVI_S32 i = 0; i < *ps32DsiInitCmdsSize; i++) {
        struct cmd_info_s stCmdInfo = {
            .devno = *ps32DevNo,
            .cmd_size = pstDsiInitCmds[i].size,
            .data_type = pstDsiInitCmds[i].data_type,
            .cmd = (void *)pstDsiInitCmds[i].data
        };
        APP_FUNC_RET_CALLBACK(mipi_tx_send_cmd(*ps32MipiTxFd, &stCmdInfo), {
            if (pstDsiInitCmds[i].delay) {
                usleep(pstDsiInitCmds[i].delay * 1000);
            }
        }, {
            if (pstDsiInitCmds[i].delay) {
                usleep(pstDsiInitCmds[i].delay * 1000);
            }

            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[mipi_tx_send_cmd] didn't return success!\n");

            return CVI_FAILURE;
        });
    }

    APP_CHK_RET(mipi_tx_set_hs_settle(*ps32MipiTxFd, pstHsTimingCfg), "mipi_tx_set_hs_settle");
    APP_CHK_RET(mipi_tx_enable(*ps32MipiTxFd), "mipi_tx_enable");
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Enable MIPI-TX driver for panel [%s].\n", pchPanelName);

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_MipiTx_Disable(const CVI_S32* const ps32MipiTxFd)
{
    _NULL_POINTER_CHECK_(ps32MipiTxFd, CVI_FAILURE);

    if (*ps32MipiTxFd == -1) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Cannot close '%s': %d, %s\n", MIPI_TX_NAME, errno, strerror(errno));
        return CVI_FAILURE;
    }

    APP_CHK_RET(mipi_tx_disable(*ps32MipiTxFd), "mipi_tx_disable");

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Disable MIPI-TX driver.\n");

    return CVI_SUCCESS;
}
