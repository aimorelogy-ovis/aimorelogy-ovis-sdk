/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_log_ctrl.h
 * Description:
 *
 */

#ifndef _ISP_LOG_CTRL_H_
#define _ISP_LOG_CTRL_H_

#include "cvi_comm_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define DEFAULT_LOG_PATH     "/var/log"
#define DEFAULT_LOG_FILENAME "isplog"
#define MAX_LOG_BUF_SIZE     (50 << 10)

typedef enum {
    ISP_LOG_SET_LOG_PATH       = (1 << 0),
    ISP_LOG_EXPORT_LOG_TO_FILE = (1 << 1),
    ISP_LOG_CLEAR_LOG_BUFFER   = (1 << 2),
    ISP_LOG_ADD_LOG_MESSAGE    = (1 << 3)
} ISP_LOG_CTRL_OPTION_E;

typedef struct {
    ISP_LOG_CTRL_OPTION_E eCommand;
    char                 *pszPathName;    // ISP_LOG_SET_LOG_PATH
    char                 *pszLogMessage;  // ISP_LOG_ADD_LOG_MESSAGE
} TISP_LOG_CTRL_OPTIONS;

CVI_S32 isp_log_ctrl_init(VI_PIPE ViPipe);
CVI_S32 isp_log_ctrl_uninit(VI_PIPE ViPipe);
CVI_S32 isp_log_ctrl_ctrl(VI_PIPE ViPipe, const TISP_LOG_CTRL_OPTIONS *ptCtrlOpts);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // _ISP_LOG_CTRL_H_
