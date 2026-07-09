/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: raw_dump.c
 * Description:
 *
 */

#include <errno.h>
#include <inttypes.h>
#include <k_api.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "3A_internal.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_buffer.h"
#include "cvi_isp.h"
#include "cvi_mw_base.h"
#include "cvi_sys.h"
#include "cvi_tpu_interface.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "raw_dump_internal.h"

#define LOGOUT(fmt, arg...) printf("[DUMP_RAW] %s,%d: " fmt, __func__, __LINE__, ##arg)

#define ABORT_IF(EXP)                      \
    do {                                   \
        if (EXP) {                         \
            LOGOUT("abort: " #EXP "\r\n"); \
            abort();                       \
        }                                  \
    } while (0)

#define RETURN_FAILURE_IF(EXP)            \
    do {                                  \
        if (EXP) {                        \
            LOGOUT("fail: " #EXP "\r\n"); \
            return CVI_FAILURE;           \
        }                                 \
    } while (0)

#define GOTO_FAIL_IF(EXP, LABEL)          \
    do {                                  \
        if (EXP) {                        \
            LOGOUT("fail: " #EXP "\r\n"); \
            goto LABEL;                   \
        }                                 \
    } while (0)

#define WARN_IF(EXP)                      \
    do {                                  \
        if (EXP) {                        \
            LOGOUT("warn: " #EXP "\r\n"); \
        }                                 \
    } while (0)

#define ERROR_IF(EXP)                      \
    do {                                   \
        if (EXP) {                         \
            LOGOUT("error: " #EXP "\r\n"); \
        }                                  \
    } while (0)

#define SNPRINTF_LOCAL(offset, buf, buf_size, fmt, ...)                                \
    do {                                                                               \
        int ret;                                                                       \
        ret = snprintf((char *)(buf + offset), buf_size - offset, fmt, ##__VA_ARGS__); \
        if (ret > 0 && ret < buf_size - offset) {                                      \
            offset += ret;                                                             \
        } else if (ret <= 0) {                                                         \
            LOGOUT("snprintf fail\r\n");                                               \
            return CVI_FAILURE;                                                        \
        } else {                                                                       \
            LOGOUT("snprintf overflow\r\n");                                           \
            return CVI_FAILURE;                                                        \
        }                                                                              \
    } while (0)

#define VI_GET_TIME_OUT_MS 1000
#define SD_PATH            "/mnt/sd"

static void get_file_name_prefix(char *prefix, CVI_U32 len)
{
    struct tm *t;
    time_t     tt;

    ISP_PUB_ATTR_S  stPubAttr;
    ISP_EXP_INFO_S *pstExpInfo;

    const char *pBayerStr = NULL;
    const char *pMode     = NULL;

    memset(&stPubAttr, 0, sizeof(ISP_PUB_ATTR_S));
    CVI_ISP_GetPubAttr(0, &stPubAttr);

    pstExpInfo = (ISP_EXP_INFO_S *)calloc(1, sizeof(ISP_EXP_INFO_S));
    CVI_ISP_QueryExposureInfo(0, pstExpInfo);

    switch (stPubAttr.enBayer) {
    case BAYER_BGGR:
        pBayerStr = "BGGR";
        break;
    case BAYER_GBRG:
        pBayerStr = "GBRG";
        break;
    case BAYER_GRBG:
        pBayerStr = "GRBG";
        break;
    case BAYER_RGGB:
        pBayerStr = "RGGB";
        break;
    default:
        pBayerStr = "XXOO";
        break;
    }

    if (stPubAttr.enWDRMode == WDR_MODE_NONE) {
        pMode = "Linear";
    } else if (stPubAttr.enWDRMode >= WDR_MODE_2To1_LINE) {
        pMode = "WDR";
    } else {
        pMode = "XXOO";
    }

    time(&tt);
    t = localtime(&tt);

    snprintf(prefix, len,
             "%dX%d_%s_%s_-color=%d_-bits=12_-frame=1_-hdr=%d_ISO=%d_%04d%02d%02d%02d%02d%02d",
             stPubAttr.enWDRMode == WDR_MODE_NONE ? stPubAttr.stWndRect.u32Width
                                                  : stPubAttr.stWndRect.u32Width * 2,
             stPubAttr.stWndRect.u32Height, pBayerStr, pMode, stPubAttr.enBayer,
             stPubAttr.enWDRMode == WDR_MODE_NONE ? 0 : 1, pstExpInfo->u32ISO, t->tm_year + 1900,
             t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    free(pstExpInfo);
}

static void dump_log_init(VI_PIPE ViPipe)
{
    ISP_EXPOSURE_ATTR_S stExpAttr;

    memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));

    CVI_ISP_GetExposureAttr(ViPipe, &stExpAttr);

    if (stExpAttr.u8DebugMode != 255) {
        stExpAttr.u8DebugMode = 255;

        CVI_ISP_SetExposureAttr(ViPipe, &stExpAttr);
    }
}

static CVI_S32 dump_log_buffer_init(RAW_DUMP_INFO_S *pstRawDumpInfo)
{
    CVI_U32 u32AWBLogBufSize;
    CVI_U32 u32Temp;

    pstRawDumpInfo->pu8RawInfo = (CVI_U8 *)calloc(RAW_INFO_BUFFER_SIZE, sizeof(CVI_U8));
    if (pstRawDumpInfo->pu8RawInfo == NULL) {
        return CVI_FAILURE;
    }

    pstRawDumpInfo->pu8RegDump = (CVI_U8 *)calloc(REG_DUMP_BUFFER_SIZE, sizeof(CVI_U8));
    if (pstRawDumpInfo->pu8RegDump == NULL) {
        return CVI_FAILURE;
    }

    CVI_ISP_GetAELogBufSize(0, &u32Temp);
    pstRawDumpInfo->u32AELogBufferSize = u32Temp;

    // ※※※linux:calloc return !Null when calloc size is 0,alios:calloc return NUll when calloc size
    // is 0
    if (u32Temp) {
        pstRawDumpInfo->pu8AELogBuffer = (CVI_U8 *)calloc(u32Temp, sizeof(CVI_U8));
    } else {
        pstRawDumpInfo->pu8AELogBuffer = (CVI_U8 *)calloc(1, sizeof(CVI_U8));
    }
    if (pstRawDumpInfo->pu8AELogBuffer == NULL) {
        return CVI_FAILURE;
    }

    pstRawDumpInfo->u32ExpInfoSize   = 0;
    pstRawDumpInfo->pu8ExpInfoBuffer = (CVI_U8 *)calloc(AE_SNAP_LOG_BUFF_SIZE, sizeof(CVI_U8));
    if (pstRawDumpInfo->pu8ExpInfoBuffer == NULL) {
        return CVI_FAILURE;
    }

    u32AWBLogBufSize                = AWB_SNAP_LOG_BUFF_SIZE;
    pstRawDumpInfo->pu8AWBLogBuffer = (CVI_U8 *)calloc(u32AWBLogBufSize, sizeof(CVI_U8));
    if (pstRawDumpInfo->pu8AWBLogBuffer == NULL) {
        return CVI_FAILURE;
    }

    u32Temp                       = CVI_ISP_GetAWBDbgBinSize();
    pstRawDumpInfo->u32AWBbinSize = u32Temp;
    pstRawDumpInfo->pu8AWBbin     = (CVI_U8 *)calloc(pstRawDumpInfo->u32AWBbinSize, sizeof(CVI_U8));
    if (pstRawDumpInfo->pu8AWBbin == NULL) {
        return CVI_FAILURE;
    }
    return CVI_SUCCESS;
}

static CVI_S32 dump_log_buffer_uninit(RAW_DUMP_INFO_S *pstRawDumpInfo)
{
    if (pstRawDumpInfo->pu8RawInfo != NULL) {
        free(pstRawDumpInfo->pu8RawInfo);
        pstRawDumpInfo->pu8RawInfo = NULL;
    }

    if (pstRawDumpInfo->pu8RegDump != NULL) {
        free(pstRawDumpInfo->pu8RegDump);
        pstRawDumpInfo->pu8RegDump = NULL;
    }

    if (pstRawDumpInfo->pu8AELogBuffer != NULL) {
        free(pstRawDumpInfo->pu8AELogBuffer);
        pstRawDumpInfo->pu8AELogBuffer = NULL;
    }

    if (pstRawDumpInfo->pu8ExpInfoBuffer != NULL) {
        free(pstRawDumpInfo->pu8ExpInfoBuffer);
        pstRawDumpInfo->pu8ExpInfoBuffer = NULL;
    }

    if (pstRawDumpInfo->pu8AWBLogBuffer != NULL) {
        free(pstRawDumpInfo->pu8AWBLogBuffer);
        pstRawDumpInfo->pu8AWBLogBuffer = NULL;
    }

    if (pstRawDumpInfo->pu8AWBbin != NULL) {
        free(pstRawDumpInfo->pu8AWBbin);
        pstRawDumpInfo->pu8AWBbin = NULL;
    }

    return CVI_SUCCESS;
}

static void dump_log(VI_PIPE ViPipe, RAW_DUMP_INFO_S *pstRawDumpInfo)
{
    VI_DUMP_REGISTER_TABLE_S reg_tbl;
    ISP_INNER_STATE_INFO_S   pstInnerStateInfo;

    CVI_ISP_QueryInnerStateInfo(ViPipe, &pstInnerStateInfo);
    reg_tbl.MlscGainLut.RGain = pstInnerStateInfo.mlscGainTable.RGain;
    reg_tbl.MlscGainLut.GGain = pstInnerStateInfo.mlscGainTable.GGain;
    reg_tbl.MlscGainLut.BGain = pstInnerStateInfo.mlscGainTable.BGain;

    CVI_VI_DumpHwRegisterToFile(ViPipe, pstRawDumpInfo->pu8RegDump, &reg_tbl);

    CVI_ISP_DumpFrameRawInfoToBuffer(ViPipe, pstRawDumpInfo->pu8RawInfo, RAW_INFO_BUFFER_SIZE);

    CVI_ISP_GetAWBDbgBinBuf(ViPipe, pstRawDumpInfo->pu8AWBbin, CVI_ISP_GetAWBDbgBinSize());

    CVI_ISP_GetAWBSnapLogBuf(ViPipe, pstRawDumpInfo->pu8AWBLogBuffer, AWB_SNAP_LOG_BUFF_SIZE);

    CVI_ISP_GetAELogBuf(ViPipe, pstRawDumpInfo->pu8AELogBuffer, pstRawDumpInfo->u32AELogBufferSize);

    CVI_ISP_AEGetRawReplayExpBuf(ViPipe, pstRawDumpInfo->pu8ExpInfoBuffer,
                                 &pstRawDumpInfo->u32ExpInfoSize);
}

static CVI_S32 dump_one(VI_PIPE ViPipe, RAW_DUMP_INFO_S *pstRawDumpInfo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_U32 i      = 0;
    CVI_U8  frmNum = 1;

    CVI_CHAR acname[FILE_NAME_MAX_LENGTH + 20] = {0};
    FILE    *output                            = NULL;

    VIDEO_FRAME_INFO_S stVideoFrame[2] = {0};
    VI_DUMP_ATTR_S     attr            = {};

    get_file_name_prefix(pstRawDumpInfo->acfileName, FILE_NAME_MAX_LENGTH);

    dump_log(ViPipe, pstRawDumpInfo);

    memset(stVideoFrame, 0, sizeof(stVideoFrame));

    stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
    stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

    attr.bEnable    = 1;
    attr.u32Depth   = 0;
    attr.enDumpType = VI_DUMP_TYPE_RAW;

    s32Ret = CVI_VI_SetPipeDumpAttr(ViPipe, &attr);
    RETURN_FAILURE_IF(s32Ret != CVI_SUCCESS);

    attr.bEnable    = 0;
    attr.enDumpType = VI_DUMP_TYPE_IR;
    CVI_VI_GetPipeDumpAttr(0, &attr);

    if ((attr.bEnable != 1) || (attr.enDumpType != VI_DUMP_TYPE_RAW)) {
        LOGOUT("ERROR: Enable(%d), DumpType(%d)\r\n", attr.bEnable, attr.enDumpType);
        return CVI_FAILURE;
    }

    s32Ret = CVI_VI_GetPipeFrame(ViPipe, stVideoFrame, VI_GET_TIME_OUT_MS);
    RETURN_FAILURE_IF(s32Ret != CVI_SUCCESS);

    if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
        frmNum = 2;
    }

    snprintf(acname, sizeof(acname), "%s/%s.raw", SD_PATH, pstRawDumpInfo->acfileName);

    output = fopen(acname, "wb");
    GOTO_FAIL_IF(output == NULL, fail);

    for (i = 0; i < frmNum; i++) {
        stVideoFrame[i].stVFrame.pu8VirAddr[0] = (CVI_U8 *)stVideoFrame[i].stVFrame.u64PhyAddr[0];
        fwrite(stVideoFrame[i].stVFrame.pu8VirAddr[0], stVideoFrame[i].stVFrame.u32Length[0], 1,
               output);
    }

    fclose(output);

    snprintf(acname, sizeof(acname), "%s/%s.txt", SD_PATH, pstRawDumpInfo->acfileName);

    output = fopen(acname, "a");
    GOTO_FAIL_IF(output == NULL, fail);

    fprintf(output, "LE crop = %d,%d,%d,%d\r\n", stVideoFrame[0].stVFrame.s16OffsetLeft,
            stVideoFrame[0].stVFrame.s16OffsetTop, stVideoFrame[0].stVFrame.u32Width,
            stVideoFrame[0].stVFrame.u32Height);

    if (frmNum > 1) {
        fprintf(output, "SE crop = %d,%d,%d,%d\r\n", stVideoFrame[1].stVFrame.s16OffsetLeft,
                stVideoFrame[1].stVFrame.s16OffsetTop, stVideoFrame[1].stVFrame.u32Width,
                stVideoFrame[1].stVFrame.u32Height);
    }

    fclose(output);

    s32Ret = CVI_VI_ReleasePipeFrame(ViPipe, stVideoFrame);
    RETURN_FAILURE_IF(s32Ret != CVI_SUCCESS);

    return s32Ret;

fail:
    if (output != NULL) {
        fclose(output);
    }

    s32Ret = CVI_VI_ReleasePipeFrame(ViPipe, stVideoFrame);
    RETURN_FAILURE_IF(s32Ret != CVI_SUCCESS);

    return CVI_FAILURE;
}

static CVI_S32 calculate_blk_cnt(CVI_U32 u32BlkSize)
{
    CVI_S32 cnt = 0;

    cnt = g_kmm_head->free_size / u32BlkSize;
    cnt = cnt & (~0x01);

    if (cnt > VB_POOL_MAX_BLK) {
        cnt = VB_POOL_MAX_BLK;
    }

    cnt = cnt - 2;
    LOGOUT("mm_free_size = %lu,max_vb_cnt = %d,blkSize = %d\r\n", g_kmm_head->free_size, cnt,
           u32BlkSize);

    return cnt;
}

struct _vb_fifo_elm {
    VB_BLK vb;
};

FIFO_HEAD(_vb_fifo, _vb_fifo_elm);

static struct _vb_fifo vb_fifo;

static CVI_U32 vbFifoBlkSize;

static void *dump_smooth_raw_thread(void *param)
{
    RAW_DUMP_INFO_S *pstRawDumpInfo = (RAW_DUMP_INFO_S *)param;

    FILE    *output                                     = NULL;
    CVI_CHAR acLocalFileName[FILE_NAME_MAX_LENGTH + 40] = {0};
    CVI_U8   TotalFrameCnt                              = pstRawDumpInfo->u32TotalFrameCnt;
    RECT_S   RoiRect                                    = pstRawDumpInfo->stRoiRect;

    LOGOUT("dump raw thread start...\r\n");

    snprintf(acLocalFileName, sizeof(acLocalFileName), "%s/%s_roi=%d,%d,%d,%d,%d.raw", SD_PATH,
             pstRawDumpInfo->acfileName, TotalFrameCnt, RoiRect.s32X, RoiRect.s32Y,
             RoiRect.u32Width, RoiRect.u32Height);

    if (!pstRawDumpInfo->bDump2Remote) {
        output = fopen(acLocalFileName, "wb");

        if (output == NULL) {
            LOGOUT("\r\nfopen %s, fail...\r\n", acLocalFileName);
            return NULL;
        }
    } else {
        send_raw(strrchr(acLocalFileName, '/') + 1, TotalFrameCnt * vbFifoBlkSize);
    }

    struct _vb_fifo_elm vbElm;
    CVI_U64             phyAddr;

    void *virAddr = NULL;

    struct timespec tv1, tv2;

    clock_gettime(CLOCK_MONOTONIC, &tv1);

    for (int i = 0; i < TotalFrameCnt; i++) {
        while (FIFO_EMPTY(&vb_fifo)) {
            usleep(10 * 1000);
        }

        FIFO_POP(&vb_fifo, &vbElm);

        phyAddr = CVI_VB_Handle2PhysAddr(vbElm.vb);

        virAddr = (void *)phyAddr;

        if (!pstRawDumpInfo->bDump2Remote) {
            fwrite(virAddr, vbFifoBlkSize, 1, output);
            send_status(i + 1);
        } else {
            send_raw_data(virAddr, vbFifoBlkSize);
        }

        CVI_VB_ReleaseBlock(vbElm.vb);
    }

    clock_gettime(CLOCK_MONOTONIC, &tv2);

    CVI_U64 nsec = 0;

    if (tv2.tv_sec > tv1.tv_sec) {
        nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1.tv_nsec + tv2.tv_nsec;
    } else {
        nsec = tv2.tv_nsec - tv1.tv_nsec;
    }

    if (!pstRawDumpInfo->bDump2Remote) {
        LOGOUT("fwrite roi frame time: %lld\r\n", (long long int)nsec / 1000);

        fflush(output);
        fclose(output);

        system("sync");

        sleep(2);

        struct stat statbuf;

        stat(acLocalFileName, &statbuf);

        LOGOUT("check roi data size: %d, %d\r\n", (CVI_U32)statbuf.st_size,
               TotalFrameCnt * vbFifoBlkSize);

        if ((CVI_U32)statbuf.st_size != (TotalFrameCnt * vbFifoBlkSize)) {
            send_status(-1);
        }
    }

    LOGOUT("dump raw thread end...\r\n");

    return NULL;
}

static int send_log_to_remote(RAW_DUMP_INFO_S *pstRawDumpInfo)
{
    int      ret                             = 0;
    CVI_CHAR name[FILE_NAME_MAX_LENGTH + 20] = {0};

    snprintf(name, sizeof(name), "%s.json", pstRawDumpInfo->acfileName);
    ret =
        send_file(pstRawDumpInfo->pu8RegDump, strlen((CVI_CHAR *)pstRawDumpInfo->pu8RegDump), name);

    snprintf(name, sizeof(name), "%s.txt", pstRawDumpInfo->acfileName);
    ret =
        send_file(pstRawDumpInfo->pu8RawInfo, strlen((CVI_CHAR *)pstRawDumpInfo->pu8RawInfo), name);

    snprintf(name, sizeof(name), "%s-awb.bin", pstRawDumpInfo->acfileName);
    ret = send_file(pstRawDumpInfo->pu8AWBbin, pstRawDumpInfo->u32AWBbinSize, name);

    snprintf(name, sizeof(name), "%s-awblog.txt", pstRawDumpInfo->acfileName);
    ret = send_file(pstRawDumpInfo->pu8AWBLogBuffer,
                    strlen((CVI_CHAR *)pstRawDumpInfo->pu8AWBLogBuffer), name);

    snprintf(name, sizeof(name), "%s-aelog.txt", pstRawDumpInfo->acfileName);
    ret = send_file(pstRawDumpInfo->pu8AELogBuffer,
                    strlen((CVI_CHAR *)pstRawDumpInfo->pu8AELogBuffer), name);

    snprintf(name, sizeof(name), "%s-expInfo.txt", pstRawDumpInfo->acfileName);
    ret = send_file(pstRawDumpInfo->pu8ExpInfoBuffer,
                    strlen((CVI_CHAR *)pstRawDumpInfo->pu8ExpInfoBuffer), name);

    return ret;
}

static CVI_S32 dump_smooth_raw(VI_PIPE ViPipe, RAW_DUMP_INFO_S *pstRawDumpInfo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    char  acname[FILE_NAME_MAX_LENGTH + 20] = {0};
    FILE *output                            = NULL;

    VB_POOL          fullVbPoolID = VB_INVALID_POOLID;
    VB_POOL_CONFIG_S cfg;
    CVI_U32          fullVbBlkSize;
    CVI_U32          tileWidth, tileIdx;

    VB_POOL vbFifoPoolID = VB_INVALID_POOLID;

    VI_SMOOTH_RAW_DUMP_INFO_S stDumpInfo;

    VIDEO_FRAME_INFO_S stVideoFrameFullStart[2];
    VIDEO_FRAME_INFO_S stVideoFrameFull[2];

    CVI_U64  u64FullVbPhyAddr   = 0;
    CVI_U64 *pFullVbPhyAddrList = NULL;
    VB_BLK   fullVbBlk;

    VI_DEV_ATTR_S  stDevAttr;
    VI_PIPE_ATTR_S stPipeAttr;
    VI_CHN_ATTR_S  stChnAttr;

    CVI_U8 TotalFrameCnt  = pstRawDumpInfo->u32TotalFrameCnt;
    CVI_U8 fullSizeBlkCnt = 3;

    RECT_S  RoiRect = pstRawDumpInfo->stRoiRect;
    CVI_U32 rawFid;
    int     frm_num = 0;

    LOGOUT("roi rect: %d,%d,%d,%d\r\n", RoiRect.s32X, RoiRect.s32Y, RoiRect.u32Width,
           RoiRect.u32Height);
    cvi_tpu_init();

    CVI_VI_GetDevAttr(ViPipe, &stDevAttr);
    CVI_VI_GetChnAttr(ViPipe, 0, &stChnAttr);
    CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);

    RETURN_FAILURE_IF(RoiRect.s32X != 0);
    RETURN_FAILURE_IF(stPipeAttr.enCompressMode == COMPRESS_MODE_NONE);

    tileWidth = 0xFFFFFFFF;
    tileIdx   = 0xFFFFFFFF;

#if defined(__CV181X__) || defined(__CV180X__)
#else
#error "should check here when porting new chip"
#endif

    frm_num = (stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ? 2 : 1;

    cfg.u32BlkCnt = frm_num * fullSizeBlkCnt;
    cfg.u32BlkSize =
        VI_GetRawBufferSize(stChnAttr.stSize.u32Width, stChnAttr.stSize.u32Height,
                            PIXEL_FORMAT_RGB_BAYER_12BPP, stPipeAttr.enCompressMode, DEFAULT_ALIGN,
                            (stChnAttr.stSize.u32Width > tileWidth) ? CVI_TRUE : CVI_FALSE);

    fullVbBlkSize = cfg.u32BlkSize;

    fullVbPoolID = CVI_VB_CreatePool(&cfg);
    RETURN_FAILURE_IF(fullVbPoolID == VB_INVALID_POOLID);

    LOGOUT("create full vb pool: %d, blkCnt: %d, blkSize: %d\r\n", fullVbPoolID, cfg.u32BlkCnt,
           cfg.u32BlkSize);

    pFullVbPhyAddrList = malloc(sizeof(*pFullVbPhyAddrList) * cfg.u32BlkCnt);
    GOTO_FAIL_IF(pFullVbPhyAddrList == NULL, fail);

    for (CVI_U8 i = 0; i < fullSizeBlkCnt * frm_num; i++) {
        fullVbBlk = CVI_VB_GetBlock(fullVbPoolID, fullVbBlkSize);
        GOTO_FAIL_IF(fullVbBlk == VB_INVALID_HANDLE, fail);
        u64FullVbPhyAddr          = CVI_VB_Handle2PhysAddr(fullVbBlk);
        *(pFullVbPhyAddrList + i) = u64FullVbPhyAddr;
    }

    memset(&stDumpInfo, 0, sizeof(stDumpInfo));

    stDumpInfo.ViPipe        = ViPipe;
    stDumpInfo.u8BlkCnt      = fullSizeBlkCnt;
    stDumpInfo.phy_addr_list = pFullVbPhyAddrList;

    cfg.u32BlkSize = VI_GetRawBufferSize(
        RoiRect.u32Width, RoiRect.u32Height, PIXEL_FORMAT_RGB_BAYER_12BPP,
        stPipeAttr.enCompressMode, DEFAULT_ALIGN,
        ((stChnAttr.stSize.u32Width > tileWidth) && (RoiRect.u32Width > tileIdx)) ? CVI_TRUE
                                                                                  : CVI_FALSE);

    if (frm_num == 2) {
        cfg.u32BlkSize = cfg.u32BlkSize * 2;
    }

    CVI_S32 tempVbBlkCnt = calculate_blk_cnt(cfg.u32BlkSize);

    LOGOUT("calculate_vb_blk_cnt: %d\r\n", tempVbBlkCnt);

    if (tempVbBlkCnt > (CVI_S32)cfg.u32BlkCnt) {
        cfg.u32BlkCnt = tempVbBlkCnt;
    }

    if (cfg.u32BlkCnt > TotalFrameCnt) {
        cfg.u32BlkCnt = TotalFrameCnt;
    }

    vbFifoBlkSize = cfg.u32BlkSize;

    FIFO_INIT(&vb_fifo, cfg.u32BlkCnt);

    vbFifoPoolID = CVI_VB_CreatePool(&cfg);
    RETURN_FAILURE_IF(vbFifoPoolID == VB_INVALID_POOLID);

    LOGOUT("create vb fifo pool: %d, blkCnt: %d, blkSize: %d\r\n", vbFifoPoolID, cfg.u32BlkCnt,
           cfg.u32BlkSize);

    get_file_name_prefix(pstRawDumpInfo->acfileName, FILE_NAME_MAX_LENGTH);

    pthread_t dumpThread;

    pthread_create(&dumpThread, NULL, dump_smooth_raw_thread, (void *)pstRawDumpInfo);

    CVI_U32 fullFrameCnt = 0;
    CVI_U32 dropFlag     = 0;

    struct timespec tv1, tv2;

    clock_gettime(CLOCK_MONOTONIC, &tv1);

    s32Ret = CVI_VI_StartSmoothRawDump(&stDumpInfo);
    GOTO_FAIL_IF(s32Ret != CVI_SUCCESS, fail);

    struct _vb_fifo_elm vbElm;
    CVI_TDMA_2D_S       tdmaParam;

    tdmaParam.h = RoiRect.u32Height;

    /* compress mode */
    // tdmaParam.stride_bytes = ALIGN(ALIGN(stChnAttr.stSize.u32Width * 6 / 8, 8), 32);
    // tdmaParam.w_bytes = ALIGN(ALIGN(RoiRect.u32Width * 6 / 8, 8), 32);

    tdmaParam.stride_bytes_src = fullVbBlkSize / stChnAttr.stSize.u32Height;
    tdmaParam.w_bytes          = vbFifoBlkSize / RoiRect.u32Height / frm_num;

    tdmaParam.stride_bytes_dst = tdmaParam.w_bytes;

    CVI_U64 paddr_dst       = 0x00;
    CVI_U64 src_addr_offset = tdmaParam.stride_bytes_src * RoiRect.s32Y;

    LOGOUT(
        "full size w: %d, h: %d, roi size w: %d, h: %d, stride_bytes_src: %d, w_bytes: %d, "
        "src_addr_offset: %d\r\n",
        stChnAttr.stSize.u32Width, stChnAttr.stSize.u32Height, RoiRect.u32Width, RoiRect.u32Height,
        tdmaParam.stride_bytes_src, tdmaParam.w_bytes, (CVI_U32)src_addr_offset);

    bool bTDMACopyFailFlag = false;

    memset(stVideoFrameFullStart, 0, sizeof(VIDEO_FRAME_INFO_S));

    s32Ret = CVI_VI_GetSmoothRawDump(ViPipe, stVideoFrameFullStart, VI_GET_TIME_OUT_MS);
    GOTO_FAIL_IF(s32Ret != CVI_SUCCESS, fail);

    // rawFid = stVideoFrameFullStart[0].stVFrame.u32TimeRef;
    CVI_ISP_GetFrameID(ViPipe, &rawFid);
    CVI_ISP_AESetRawDumpFrameID(ViPipe, rawFid + 1, TotalFrameCnt + 1);
    LOGOUT("fid:%u %u\r\n", stVideoFrameFullStart[0].stVFrame.u32TimeRef, rawFid + 1);

    for (int i = 0; i < TotalFrameCnt; i++) {
        memset(stVideoFrameFull, 0, sizeof(VIDEO_FRAME_INFO_S));

        s32Ret = CVI_VI_GetSmoothRawDump(ViPipe, stVideoFrameFull, VI_GET_TIME_OUT_MS);
        GOTO_FAIL_IF(s32Ret != CVI_SUCCESS, fail);

        vbElm.vb = CVI_VB_GetBlock(vbFifoPoolID, vbFifoBlkSize);

        if (vbElm.vb != VB_INVALID_HANDLE) {
            paddr_dst = CVI_VB_Handle2PhysAddr(vbElm.vb);

            fullFrameCnt++;

            for (int j = 0; j < frm_num; j++) {
                // LOGOUT("full size raw, size: %d, cnt: %d\r\n",
                // stVideoFrameFull[j].stVFrame.u32Length[0],
                //	stVideoFrameFull[j].stVFrame.u32TimeRef);

                if (fullFrameCnt != stVideoFrameFull[j].stVFrame.u32TimeRef) {
                    dropFlag++;
                    fullFrameCnt = stVideoFrameFull[j].stVFrame.u32TimeRef;
                }

                paddr_dst = paddr_dst + j * vbFifoBlkSize / 2;

                tdmaParam.paddr_dst = paddr_dst;
                tdmaParam.paddr_src = stVideoFrameFull[j].stVFrame.u64PhyAddr[0] + src_addr_offset;

#define MEASURE_TDMA_COPY_TIME 0

#if MEASURE_TDMA_COPY_TIME
                struct timespec tv1, tv2;

                clock_gettime(CLOCK_MONOTONIC, &tv1);
#endif

                s32Ret = CVI_SYS_TDMACopy2D(&tdmaParam);

#if MEASURE_TDMA_COPY_TIME
                clock_gettime(CLOCK_MONOTONIC, &tv2);

                CVI_U64 nsec = 0;

                if (tv2.tv_sec > tv1.tv_sec) {
                    nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1.tv_nsec +
                           tv2.tv_nsec;
                } else {
                    nsec = tv2.tv_nsec - tv1.tv_nsec;
                }

                LOGOUT("TDMA copy time: %lld\r\n", nsec / 1000);
#endif
                if (s32Ret != CVI_SUCCESS) {
                    LOGOUT("\r\nTDMA Copy 2D fail...\r\n");
                    bTDMACopyFailFlag = true;
                }
            }

            if (!bTDMACopyFailFlag) {
                if (!FIFO_FULL(&vb_fifo)) {
                    FIFO_PUSH(&vb_fifo, vbElm);
                } else {
                    i--;
                    dropFlag++;
                    LOGOUT("vb fifo full...\r\n");
                    CVI_VB_ReleaseBlock(vbElm.vb);
                }
            } else {
                i--;
                dropFlag++;
                CVI_VB_ReleaseBlock(vbElm.vb);
            }
        } else {
            i--;
            dropFlag++;
        }

        s32Ret = CVI_VI_PutSmoothRawDump(ViPipe, stVideoFrameFull);
        GOTO_FAIL_IF(s32Ret != CVI_SUCCESS, fail);
    }

    s32Ret = CVI_VI_StopSmoothRawDump(&stDumpInfo);
    ERROR_IF(s32Ret != CVI_SUCCESS);

    dump_log(ViPipe, pstRawDumpInfo);

    if (dropFlag > 1) {
        LOGOUT("\r\nWARN, roi frame drop, dropFlag: %d\r\n", dropFlag);
    }

    clock_gettime(CLOCK_MONOTONIC, &tv2);

    CVI_U64 nsec = 0;

    if (tv2.tv_sec > tv1.tv_sec) {
        nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1.tv_nsec + tv2.tv_nsec;
    } else {
        nsec = tv2.tv_nsec - tv1.tv_nsec;
    }

    LOGOUT("get frame time: %lld\r\n", (long long int)nsec / 1000);

    if (!pstRawDumpInfo->bDump2Remote) {
        snprintf(acname, sizeof(acname), "%s/%s.raw", SD_PATH, pstRawDumpInfo->acfileName);

        output = fopen(acname, "wb");
        GOTO_FAIL_IF(output == NULL, fail);

        for (int i = 0; i < frm_num; i++) {
            stVideoFrameFullStart[i].stVFrame.pu8VirAddr[0] =
                (CVI_U8 *)stVideoFrameFullStart[i].stVFrame.u64PhyAddr[0];
            //	= CVI_SYS_Mmap(stVideoFrameFullStart[i].stVFrame.u64PhyAddr[0],
            //		stVideoFrameFullStart[i].stVFrame.u32Length[0]);

            LOGOUT("full frame, size: %d, cnt: %d\r\n",
                   stVideoFrameFullStart[i].stVFrame.u32Length[0],
                   stVideoFrameFullStart[i].stVFrame.u32TimeRef);

            fwrite(stVideoFrameFullStart[i].stVFrame.pu8VirAddr[0],
                   stVideoFrameFullStart[i].stVFrame.u32Length[0], 1, output);

            // CVI_SYS_Munmap((void *)stVideoFrameFullStart[i].stVFrame.pu8VirAddr[0],
            //	stVideoFrameFullStart[i].stVFrame.u32Length[0]);
        }

        fclose(output);
    }

    CVI_U32 Offset = 0;
    Offset         = strlen((char *)pstRawDumpInfo->pu8RawInfo);
    SNPRINTF_LOCAL(Offset, pstRawDumpInfo->pu8RawInfo, RAW_INFO_BUFFER_SIZE,
                   "LE crop = %d,%d,%d,%d\r\n", stVideoFrameFullStart[0].stVFrame.s16OffsetLeft,
                   stVideoFrameFullStart[0].stVFrame.s16OffsetTop,
                   stVideoFrameFullStart[0].stVFrame.u32Width,
                   stVideoFrameFullStart[0].stVFrame.u32Height);

    if (frm_num > 1) {
        SNPRINTF_LOCAL(Offset, pstRawDumpInfo->pu8RawInfo, RAW_INFO_BUFFER_SIZE,
                       "SE crop = %d,%d,%d,%d\r\n", stVideoFrameFullStart[1].stVFrame.s16OffsetLeft,
                       stVideoFrameFullStart[1].stVFrame.s16OffsetTop,
                       stVideoFrameFullStart[1].stVFrame.u32Width,
                       stVideoFrameFullStart[1].stVFrame.u32Height);
    }
    SNPRINTF_LOCAL(Offset, pstRawDumpInfo->pu8RawInfo, RAW_INFO_BUFFER_SIZE,
                   "roi = %d,%d,%d,%d,%d\r\n", TotalFrameCnt, RoiRect.s32X, RoiRect.s32Y,
                   RoiRect.u32Width, RoiRect.u32Height);

    LOGOUT("join dumpThread....\r\n");

    pthread_join(dumpThread, NULL);

    if (pstRawDumpInfo->bDump2Remote) {
        send_log_to_remote(pstRawDumpInfo);
        snprintf(acname, sizeof(acname), "%s.raw", pstRawDumpInfo->acfileName);
        send_raw(acname, stVideoFrameFullStart[0].stVFrame.u32Length[0] * frm_num);

        for (int i = 0; i < frm_num; i++) {
            stVideoFrameFullStart[i].stVFrame.pu8VirAddr[0] =
                (CVI_U8 *)stVideoFrameFullStart[i].stVFrame.u64PhyAddr[0];
            //	= CVI_SYS_Mmap(stVideoFrameFullStart[i].stVFrame.u64PhyAddr[0]
            //		, stVideoFrameFullStart[i].stVFrame.u32Length[0]);

            LOGOUT("full frame, size: %d, cnt: %d\r\n",
                   stVideoFrameFullStart[i].stVFrame.u32Length[0],
                   stVideoFrameFullStart[i].stVFrame.u32TimeRef);

            send_raw_data(stVideoFrameFullStart[i].stVFrame.pu8VirAddr[0],
                          stVideoFrameFullStart[i].stVFrame.u32Length[0]);

            // CVI_SYS_Munmap((void *)stVideoFrameFullStart[i].stVFrame.pu8VirAddr[0],
            //	stVideoFrameFullStart[i].stVFrame.u32Length[0]);
        }
    }

    if (pFullVbPhyAddrList != NULL) {
        for (CVI_U8 i = 0; i < fullSizeBlkCnt * frm_num; i++) {
            u64FullVbPhyAddr = *(pFullVbPhyAddrList + i);
            fullVbBlk        = CVI_VB_PhysAddr2Handle(u64FullVbPhyAddr);
            if (fullVbBlk != VB_INVALID_HANDLE) {
                CVI_VB_ReleaseBlock(fullVbBlk);
            }
        }
        free(pFullVbPhyAddrList);
        pFullVbPhyAddrList = NULL;
    }

    s32Ret = CVI_VB_DestroyPool(fullVbPoolID);
    ERROR_IF(s32Ret != CVI_SUCCESS);

    s32Ret = CVI_VB_DestroyPool(vbFifoPoolID);
    ERROR_IF(s32Ret != CVI_SUCCESS);

    if (dropFlag > 1) {
        LOGOUT("\r\nERROR: frame drop, return error...\r\n");
        send_status(-1);
    } else {
        send_status(0);
    }

    cvi_tpu_deinit();

    return s32Ret;

fail:

    CVI_VI_StopSmoothRawDump(&stDumpInfo);

    if (pFullVbPhyAddrList != NULL) {
        for (CVI_U8 i = 0; i < fullSizeBlkCnt * frm_num; i++) {
            u64FullVbPhyAddr = *(pFullVbPhyAddrList + i);
            fullVbBlk        = CVI_VB_PhysAddr2Handle(u64FullVbPhyAddr);
            if (fullVbBlk != VB_INVALID_HANDLE) {
                CVI_VB_ReleaseBlock(fullVbBlk);
            }
        }
        free(pFullVbPhyAddrList);
        pFullVbPhyAddrList = NULL;
    }

    if (fullVbPoolID != VB_INVALID_POOLID) {
        CVI_VB_DestroyPool(fullVbPoolID);
    }

    if (vbFifoPoolID != VB_INVALID_POOLID) {
        s32Ret = CVI_VB_DestroyPool(vbFifoPoolID);
    }

    send_status(-1);

    cvi_tpu_deinit();

    return CVI_FAILURE;
}

CVI_S32 cvi_raw_dump(VI_PIPE ViPipe, RAW_DUMP_INFO_S *pstRawDumpInfo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    LOGOUT("raw dump: ViPipe: %d, Cnt: %d\r\n", ViPipe, pstRawDumpInfo->u32TotalFrameCnt);
    dump_log_init(ViPipe);
    s32Ret = dump_log_buffer_init(pstRawDumpInfo);
    RETURN_FAILURE_IF(s32Ret != CVI_SUCCESS);

    if (pstRawDumpInfo->u32TotalFrameCnt > 1) {
        s32Ret = dump_smooth_raw(ViPipe, pstRawDumpInfo);

    } else if (pstRawDumpInfo->u32TotalFrameCnt == 1) {
        s32Ret = dump_one(ViPipe, pstRawDumpInfo);

    } else {
        s32Ret = CVI_FAILURE;
    }

    dump_log_buffer_uninit(pstRawDumpInfo);
    return s32Ret;
}
