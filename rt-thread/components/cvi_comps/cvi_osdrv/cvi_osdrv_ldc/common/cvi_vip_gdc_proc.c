/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_vip_gdc_proc.c
 * Description: video pipeline graphic distortion correction engine info
 */
#include <stdio.h>
#include "cvi_vip_gdc_proc.h"
#include "cvi_base.h"
#include "cvi_vip.h"
#include "base_ctx.h"
#include "ldc_debug.h"


#define GENERATE_STRING(STRING) (#STRING),
#define GDC_PROC_NAME           "cvitek/gdc"

static void              *gdc_shared_mem;
static const char * const MOD_STRING[] = FOREACH_MOD(GENERATE_STRING);
/*************************************************************************
 *	GDC proc functions
 *************************************************************************/
int get_gdc_proc(char *outbuf)
{
    struct cvi_gdc_proc_ctx *pgdcCtx = NULL;
    int                      i, j, idx, total_hwTime, total_busyTime;
    char                     c[32];
    char                    *m      = outbuf;
    int                      offset = 0;

    pgdcCtx = (struct cvi_gdc_proc_ctx *)(gdc_shared_mem);
    if (!pgdcCtx)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "gdc shm = NULL\n");
        return -1;
    }

    offset += sprintf(m + offset, "\nModule: [GDC]\n");
    // recent job info
    offset += sprintf(m + offset, "\n-------------------------------RECENT JOB INFO----------------------------\n");
    offset += sprintf(m + offset, "%10s%10s%10s%10s%20s%20s%20s%20s\n",
                      "SeqNo", "ModName", "TaskNum", "State",
                      "InSize(pixel)", "OutSize(pixel)", "CostTime(us)", "HwTime(us)");

    idx = pgdcCtx->u16JobInfoIdx;
    for (i = 0; i < 8; ++i)
    {
        if (!pgdcCtx->stJobInfo[idx].hHandle)
            break;

        memset(c, 0, sizeof(c));
        if (pgdcCtx->stJobInfo[idx].eState == GDC_JOB_SUCCESS)
            strncpy(c, "SUCCESS", sizeof(c));
        else if (pgdcCtx->stJobInfo[idx].eState == GDC_JOB_FAIL)
            strncpy(c, "FAIL", sizeof(c));
        else if (pgdcCtx->stJobInfo[idx].eState == GDC_JOB_WORKING)
            strncpy(c, "WORKING", sizeof(c));
        else
            strncpy(c, "UNKNOWN", sizeof(c));

        offset += sprintf(m + offset, "%8s%2d%10s%10d%10s%20d%20d%20d%20d\n",
                          "#",
                          i,
                          (pgdcCtx->stJobInfo[idx].enModId == CVI_ID_BUTT) ? "USER" : MOD_STRING[pgdcCtx->stJobInfo[idx].enModId],
                          pgdcCtx->stJobInfo[idx].u32TaskNum,
                          c,
                          pgdcCtx->stJobInfo[idx].u32InSize,
                          pgdcCtx->stJobInfo[idx].u32OutSize,
                          pgdcCtx->stJobInfo[idx].u32CostTime,
                          pgdcCtx->stJobInfo[idx].u32HwTime);

        idx = (--idx < 0) ? (GDC_PROC_JOB_INFO_NUM - 1) : idx;
    }

    // Max waste time job info
    offset += sprintf(m + offset, "\n-------------------------------MAX WASTE TIME JOB INFO--------------------\n");
    offset += sprintf(m + offset, "%10s%10s%10s%20s%20s%20s%20s\n",
                      "ModName", "TaskNum", "State",
                      "InSize(pixel)", "OutSize(pixel)", "CostTime(us)", "HwTime(us)");

    idx = i = pgdcCtx->u16JobInfoIdx;
    for (j = 1; j < GDC_PROC_JOB_INFO_NUM; ++j)
    {
        i = (--i < 0) ? (GDC_PROC_JOB_INFO_NUM - 1) : i;
        if (!pgdcCtx->stJobInfo[i].hHandle)
            break;

        if (pgdcCtx->stJobInfo[i].u32CostTime > pgdcCtx->stJobInfo[idx].u32CostTime)
            idx = i;
    }

    if (pgdcCtx->stJobInfo[idx].hHandle)
    {
        memset(c, 0, sizeof(c));
        if (pgdcCtx->stJobInfo[idx].eState == GDC_JOB_SUCCESS)
            strncpy(c, "SUCCESS", sizeof(c));
        else if (pgdcCtx->stJobInfo[idx].eState == GDC_JOB_FAIL)
            strncpy(c, "FAIL", sizeof(c));
        else if (pgdcCtx->stJobInfo[idx].eState == GDC_JOB_WORKING)
            strncpy(c, "WORKING", sizeof(c));
        else
            strncpy(c, "UNKNOWN", sizeof(c));

        offset += sprintf(m + offset, "%10s%10d%10s%20d%20d%20d%20d\n",
                          MOD_STRING[pgdcCtx->stJobInfo[idx].enModId],
                          pgdcCtx->stJobInfo[idx].u32TaskNum,
                          c,
                          pgdcCtx->stJobInfo[idx].u32InSize,
                          pgdcCtx->stJobInfo[idx].u32OutSize,
                          pgdcCtx->stJobInfo[idx].u32CostTime,
                          pgdcCtx->stJobInfo[idx].u32HwTime);
    }

    // GDC job status
    offset += sprintf(m + offset, "\n-------------------------------GDC JOB STATUS-----------------------------\n");
    offset += sprintf(m + offset, "%10s%10s%10s%10s%10s%20s\n",
                      "Success", "Fail", "Cancel", "BeginNum", "BusyNum", "ProcingNum");

    offset += sprintf(m + offset, "%10d%10d%10d%10d%10d%20d\n",
                      pgdcCtx->stJobStatus.u32Success,
                      pgdcCtx->stJobStatus.u32Fail,
                      pgdcCtx->stJobStatus.u32Cancel,
                      pgdcCtx->stJobStatus.u32BeginNum,
                      pgdcCtx->stJobStatus.u32BusyNum,
                      pgdcCtx->stJobStatus.u32ProcingNum);

    // GDC task status
    offset += sprintf(m + offset, "\n-------------------------------GDC TASK STATUS----------------------------\n");
    offset += sprintf(m + offset, "%10s%10s%10s%10s\n", "Success", "Fail", "Cancel", "BusyNum");
    offset += sprintf(m + offset, "%10d%10d%10d%10d\n",
                      pgdcCtx->stTaskStatus.u32Success,
                      pgdcCtx->stTaskStatus.u32Fail,
                      pgdcCtx->stTaskStatus.u32Cancel,
                      pgdcCtx->stTaskStatus.u32BusyNum);

    // GDC interrupt status
    offset += sprintf(m + offset, "\n-------------------------------GDC INT STATUS-----------------------------\n");
    offset += sprintf(m + offset, "%10s%20s%20s\n", "IntNum", "IntTm(us)", "HalProcTm(us)");

    total_hwTime = total_busyTime = 0;
    for (i = 0; i < GDC_PROC_JOB_INFO_NUM; ++i)
    {
        total_hwTime   += pgdcCtx->stJobInfo[i].u32HwTime;
        total_busyTime += pgdcCtx->stJobInfo[i].u32BusyTime;
    }

    offset += sprintf(m + offset, "%10d%20d%20d\n",
                      pgdcCtx->stJobStatus.u32Success,
                      total_hwTime / GDC_PROC_JOB_INFO_NUM,
                      total_busyTime / GDC_PROC_JOB_INFO_NUM);

    // GDC call correction status
    offset += sprintf(m + offset, "\n-------------------------------GDC CALL CORRECTION STATUS-----------------\n");
    offset += sprintf(m + offset, "%10s%10s%10s%10s%10s\n", "TaskSuc", "TaskFail", "EndSuc", "EndFail", "CbCnt");
    offset += sprintf(m + offset, "%10d%10d%10d%10d%10d\n",
                      pgdcCtx->stFishEyeStatus.u32AddTaskSuc,
                      pgdcCtx->stFishEyeStatus.u32AddTaskFail,
                      pgdcCtx->stFishEyeStatus.u32EndSuc,
                      pgdcCtx->stFishEyeStatus.u32EndFail,
                      pgdcCtx->stFishEyeStatus.u32CbCnt);

    return 0;
}

int gdc_proc_init(void *shm)
{
    int rc = 0;

    gdc_shared_mem = shm;
    return rc;
}

int gdc_proc_remove(void)
{
    gdc_shared_mem = NULL;
    return 0;
}

static void proc_gdc(int32_t argc, char **argv)
{
    int   rc  = 0;
    char *str = (char *)rt_malloc(4096);

    if (!str)
    {
        printf("malloc failed\n");
        return;
    }
    rc = get_gdc_proc(str);
    if (!rc)
        printf("%s\n", str);
    rt_free(str);
}

MSH_CMD_EXPORT(proc_gdc, gdc info);

