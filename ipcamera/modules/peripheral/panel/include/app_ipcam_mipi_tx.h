/*
 * Copyright (C) Cvitek Co., Ltd. 2023. All rights reserved.
 *
 * File Name: app_ipcam_panel.h
 * Description:
 */

#ifndef __APP_IPCAM_MIPI_TX_H__
#define __APP_IPCAM_MIPI_TX_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#include "cvi_type.h"
#include "cvi_comm_mipi_tx.h"
#include "cvi_mipi_tx.h"

CVI_S32 app_ipcam_MipiTx_Enable(
    const int* const ps32DevNo,
    const CVI_CHAR* const pchPanelName,
    const struct combo_dev_cfg_s* const pstDevCfg,
    const struct hs_settle_s* const pstHsTimingCfg,
    const struct dsc_instr* const pstDsiInitCmds,
    const CVI_S32* const ps32DsiInitCmdsSize,
    CVI_S32* const ps32MipiTxFd
);

CVI_S32 app_ipcam_MipiTx_Disable(const CVI_S32* const ps32MipiTxFd);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
