/*
 * Copyright Cvitek Technologies Inc.
 *
 * Created Time: July, 2020
 */
#include <tick.h>
#include "stdint.h"
#include "cvi_vcodec_lib.h"
#include "vdi_osal.h"
#include "time.h"
#include "rtthread.h"

int dbg_mask = CVI_MASK_ERR;

BOOL addrRemapEn = 1;
BOOL ARMode      = AR_MODE_OFFSET;
int  ARExtraLine = AR_DEFAULT_EXTRA_LINE;
int  cviVcodecInit(void)
{
    cvi_vdi_init();

    cviVcodecMask();

    return 0;
}

void cviVcodecMask(void)
{
    dbg_mask |= CVI_MASK_ERR;
}

int cviVcodecGetEnv(char *envVar)
{
    if (strcmp(envVar, "addrRemapEn") == 0)
        return addrRemapEn;

    if (strcmp(envVar, "ARMode") == 0)
        return ARMode;

    if (strcmp(envVar, "ARExtraLine") == 0)
        return ARExtraLine;
    return -1;
}

int cviSetCoreIdx(int *pCoreIdx, CodStd stdMode)
{
    if (stdMode == STD_HEVC)
        *pCoreIdx = CORE_H265;
    else if (stdMode == STD_AVC)
        *pCoreIdx = CORE_H264;
    else
    {
        CVI_VC_ERR("stdMode = %d\n", stdMode);
        return -1;
    }

    CVI_VC_TRACE("stdMode = %d, coreIdx = %d\n", stdMode, *pCoreIdx);

    return 0;
}

Uint64 cviGetCurrentTime(void)
{
    // struct timespec ts;
    // clock_gettime(CLOCK_MONOTONIC, &ts);
    Uint64 current = rt_tick_get();
    return current; // in us
}
