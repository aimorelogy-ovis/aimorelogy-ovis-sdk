/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_log_ctrl.c
 * Description:
 *
 */

#include "isp_log_ctrl.h"

#include "isp_defines.h"

#define MAX_LOG_FILEPATH_LENGTH (64)

struct isp_log_ctrl_runtime {
    char    szLogFilePath[MAX_LOG_FILEPATH_LENGTH];
    char   *pcBuf;
    CVI_U32 u32MaxBufLen;
    CVI_U32 u32BufHead;
    CVI_U32 u32BufTail;
    CVI_U16 u16FileIndex;
};

struct isp_log_ctrl_runtime *log_ctrl_runtime[VI_MAX_PIPE_NUM] = {NULL};

// -----------------------------------------------------------------------------
static CVI_S32 isp_log_add_log_message(VI_PIPE ViPipe, const char *pszLogMessage);
static CVI_S32 isp_log_export_log(VI_PIPE ViPipe);
static struct isp_log_ctrl_runtime **_get_log_ctrl_runtime(VI_PIPE ViPipe);

// -----------------------------------------------------------------------------
CVI_S32 isp_log_ctrl_init(VI_PIPE ViPipe)
{
    ISP_LOG_DEBUG("+\n");
    CVI_S32 ret = CVI_SUCCESS;

    struct isp_log_ctrl_runtime **runtime_ptr = _get_log_ctrl_runtime(ViPipe);

    ISP_CREATE_RUNTIME(*runtime_ptr, struct isp_log_ctrl_runtime);
    struct isp_log_ctrl_runtime *runtime = *runtime_ptr;

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    runtime->pcBuf        = NULL;
    runtime->u32MaxBufLen = MAX_LOG_BUF_SIZE;
    runtime->u32BufHead   = 0;
    runtime->u32BufTail   = 0;
    runtime->u16FileIndex = 0;
    snprintf(runtime->szLogFilePath, MAX_LOG_FILEPATH_LENGTH, DEFAULT_LOG_PATH);

    return ret;
}

// -----------------------------------------------------------------------------
CVI_S32 isp_log_ctrl_uninit(VI_PIPE ViPipe)
{
    ISP_LOG_DEBUG("+\n");

    struct isp_log_ctrl_runtime **runtime_ptr = _get_log_ctrl_runtime(ViPipe);
    struct isp_log_ctrl_runtime  *runtime     = *runtime_ptr;

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    if (runtime->pcBuf) {
        free(runtime->pcBuf);
        runtime->pcBuf = NULL;
    }

    ISP_RELEASE_MEMORY(*runtime_ptr);

    return CVI_SUCCESS;
}

// -----------------------------------------------------------------------------
CVI_S32 isp_log_ctrl_ctrl(VI_PIPE ViPipe, const TISP_LOG_CTRL_OPTIONS *ptCtrlOpts)
{
    ISP_LOG_DEBUG("+\n");
    CVI_S32 ret = CVI_SUCCESS;
    CVI_S32 temp_ret;

    if (ptCtrlOpts == CVI_NULL) {
        return CVI_FAILURE;
    }

    struct isp_log_ctrl_runtime **runtime_ptr = _get_log_ctrl_runtime(ViPipe);
    struct isp_log_ctrl_runtime  *runtime     = *runtime_ptr;

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    if (ptCtrlOpts->eCommand & ISP_LOG_SET_LOG_PATH) {
        snprintf(runtime->szLogFilePath, MAX_LOG_FILEPATH_LENGTH, "%s", ptCtrlOpts->pszPathName);
    }
    if (ptCtrlOpts->eCommand & ISP_LOG_EXPORT_LOG_TO_FILE) {
        temp_ret = isp_log_export_log(ViPipe);
        if (temp_ret != CVI_SUCCESS) {
            ret = temp_ret;
        }
    }
    if (ptCtrlOpts->eCommand & ISP_LOG_CLEAR_LOG_BUFFER) {
        runtime->u32BufHead = 0;
        runtime->u32BufTail = 0;
    }
    if (ptCtrlOpts->eCommand & ISP_LOG_ADD_LOG_MESSAGE) {
        temp_ret = isp_log_add_log_message(ViPipe, ptCtrlOpts->pszLogMessage);
        if (temp_ret != CVI_SUCCESS) {
            ret = temp_ret;
        }
    }

    return ret;
}

// -----------------------------------------------------------------------------
//  private functions
// -----------------------------------------------------------------------------
static CVI_S32 isp_log_export_log(VI_PIPE ViPipe)
{
    struct isp_log_ctrl_runtime **runtime_ptr = _get_log_ctrl_runtime(ViPipe);
    struct isp_log_ctrl_runtime  *runtime     = *runtime_ptr;

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    if (runtime->pcBuf == NULL) {
        runtime->pcBuf = (char *)calloc(MAX_LOG_BUF_SIZE, sizeof(char));
        if (runtime->pcBuf == NULL) {
            ISP_LOG_ERR("Allocate isp log buffer fail\n");
            return CVI_FAILURE;
        }
        runtime->u32MaxBufLen = MAX_LOG_BUF_SIZE;
        runtime->u32BufHead   = 0;
        runtime->u32BufTail   = 0;
        runtime->u16FileIndex = 0;

        return CVI_SUCCESS;
    }

    if (runtime->u32BufTail == runtime->u32BufHead) {
        ISP_LOG_DEBUG("No data in buffer\n");
        return CVI_SUCCESS;
    }

    char szFullFilePath[MAX_LOG_FILEPATH_LENGTH + 20];

    snprintf(szFullFilePath, MAX_LOG_FILEPATH_LENGTH + 20, "%s/%s%d_%03d", runtime->szLogFilePath,
             DEFAULT_LOG_FILENAME, ViPipe, runtime->u16FileIndex);

    FILE *pfLogFile = NULL;

    pfLogFile = fopen(szFullFilePath, "w");
    if (pfLogFile == NULL) {
        ISP_LOG_ERR("Open log file (%s) error\n", szFullFilePath);
        return CVI_FAILURE;
    }

    char   *pcStart = NULL;
    CVI_U32 u32Length;

    if (runtime->u32BufTail > runtime->u32BufHead) {
        pcStart   = runtime->pcBuf + runtime->u32BufHead;
        u32Length = runtime->u32BufTail - runtime->u32BufHead;
        fwrite(pcStart, sizeof(char), u32Length, pfLogFile);
    } else {
        pcStart   = runtime->pcBuf + runtime->u32BufHead;
        u32Length = runtime->u32MaxBufLen - runtime->u32BufHead - 1;
        fwrite(pcStart, sizeof(char), u32Length, pfLogFile);

        pcStart   = runtime->pcBuf;
        u32Length = runtime->u32BufTail;
        fwrite(pcStart, sizeof(char), u32Length, pfLogFile);
    }

    runtime->u32BufHead = runtime->u32BufTail;
    fclose(pfLogFile);
    pfLogFile = NULL;

    runtime->u16FileIndex++;

    return CVI_SUCCESS;
}

// -----------------------------------------------------------------------------
static CVI_S32 isp_log_add_log_message(VI_PIPE ViPipe, const char *pszLogMessage)
{
    struct isp_log_ctrl_runtime **runtime_ptr = _get_log_ctrl_runtime(ViPipe);
    struct isp_log_ctrl_runtime  *runtime     = *runtime_ptr;

    if (runtime == CVI_NULL) {
        return CVI_FAILURE;
    }

    if (runtime->pcBuf == NULL) {
        return CVI_SUCCESS;
    }

    CVI_U32 u32MessageLen = strlen(pszLogMessage);
    CVI_U32 u32MaxBufLen  = runtime->u32MaxBufLen;

    char   *pcBufAddr;
    CVI_U32 u32RemainLen;
    int     iPrintLen;

    CVI_BOOL bUpdateHead = CVI_FALSE;

    if ((runtime->u32BufTail < runtime->u32BufHead) &&
        ((runtime->u32BufTail + u32MessageLen) >= runtime->u32BufHead)) {
        bUpdateHead = CVI_TRUE;
    }

    if ((runtime->u32BufTail + u32MessageLen) <= u32MaxBufLen) {
        pcBufAddr    = runtime->pcBuf + runtime->u32BufTail;
        u32RemainLen = u32MaxBufLen - runtime->u32BufTail;
        iPrintLen    = snprintf(pcBufAddr, u32RemainLen, "%s", pszLogMessage);

        if (iPrintLen != (int)u32MessageLen) {
            ISP_LOG_ERR("Add message fail (length mismatch (%d vs %d)\n", iPrintLen, u32MessageLen);
            return CVI_FAILURE;
        }

        runtime->u32BufTail += u32MessageLen;
        if (runtime->u32BufTail >= u32MaxBufLen) {
            runtime->u32BufTail = runtime->u32BufTail - u32MaxBufLen;

            if ((runtime->u32BufTail > runtime->u32BufHead) || (bUpdateHead)) {
                runtime->u32BufHead = runtime->u32BufTail + 1;
            }
        } else if (bUpdateHead) {
            runtime->u32BufHead = runtime->u32BufTail + 1;
        }
    } else {
        // Part 1
        pcBufAddr    = runtime->pcBuf + runtime->u32BufTail;
        u32RemainLen = u32MaxBufLen - runtime->u32BufTail;
        iPrintLen    = snprintf(pcBufAddr, u32RemainLen, "%s", pszLogMessage);

        iPrintLen = u32RemainLen - 1;

        // Part 2
        pcBufAddr    = runtime->pcBuf;
        u32RemainLen = u32MaxBufLen;
        iPrintLen    = snprintf(pcBufAddr, u32RemainLen, "%s", pszLogMessage + iPrintLen);

        runtime->u32BufTail = iPrintLen;
        if ((runtime->u32BufTail > runtime->u32BufHead) || (bUpdateHead)) {
            runtime->u32BufHead = runtime->u32BufTail + 1;
        }
    }

    return CVI_SUCCESS;
}

// -----------------------------------------------------------------------------
static struct isp_log_ctrl_runtime **_get_log_ctrl_runtime(VI_PIPE ViPipe)
{
    CVI_BOOL bIsViPipeValid = ((ViPipe >= 0) && (ViPipe < VI_MAX_PIPE_NUM));

    if (!bIsViPipeValid) {
        ISP_LOG_WARNING("Wrong ViPipe(%d)\n", ViPipe);
        return NULL;
    }

    return &log_ctrl_runtime[ViPipe];
}

// -----------------------------------------------------------------------------
