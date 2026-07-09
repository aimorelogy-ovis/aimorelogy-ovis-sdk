#include "stdint.h"
#include "regdefine.h"
#include "jpuapi.h"
#include "jpuapifunc.h"
#include "jputable.h"
#include "jpulog.h"
#include "jpuhelper.h"
#include <tick.h>
#include "rtthread.h"

extern int jdi_flush_ion_cache(uint64_t u64PhyAddr, void *pVirAddr, uint32_t u32Len);

int FillSdramBurst(BufInfo *pBufInfo, PhysicalAddress targetAddr,
                   PhysicalAddress bsBufStartAddr, PhysicalAddress bsBufEndAddr,
                   Uint32 size, int checkeos, int *streameos, int endian);

JpgRet WriteJpgBsBufHelper(JpgDecHandle handle, BufInfo *pBufInfo,
                           PhysicalAddress paBsBufStart,
                           PhysicalAddress paBsBufEnd, int defaultsize,
                           int checkeos, int *pstreameos, int endian)
{
    JpgRet          ret      = JPG_RET_SUCCESS;
    int             size     = 0;
    int             fillSize = 0;
    PhysicalAddress paRdPtr, paWrPtr;

    ret = JPU_DecGetBitstreamBuffer(handle, &paRdPtr, &paWrPtr, &size);
    if (ret != JPG_RET_SUCCESS)
    {
        JLOG(ERR,
             "JPU_DecGetBitstreamBuffer failed Error code is 0x%x\n",
             ret);
        goto FILL_BS_ERROR;
    }

    if (size <= 0)
    {
#ifdef MJPEG_INTERFACE_API
        size += pBufInfo->size - pBufInfo->point;
#else
        /* !! error designed !!*/
        return JPG_RET_SUCCESS;
#endif
    }

    if (defaultsize)
    {
        if (size < defaultsize)
            fillSize = size;
        else
            fillSize = defaultsize;
    }
    else
    {
        fillSize = size;
    }

    fillSize = FillSdramBurst(pBufInfo, paWrPtr, paBsBufStart, paBsBufEnd,
                              fillSize, checkeos, pstreameos, endian);

    if (*pstreameos == 0)
    {
        ret = JPU_DecUpdateBitstreamBuffer(handle, fillSize);
        if (ret != JPG_RET_SUCCESS)
        {
            JLOG(ERR,
                 "JPU_DecUpdateBitstreamBuffer failed Error code is 0x%x\n",
                 ret);
            goto FILL_BS_ERROR;
        }

        if ((pBufInfo->size - pBufInfo->point) <= 0)
        {
            ret = JPU_DecUpdateBitstreamBuffer(handle,
                                               STREAM_END_SIZE);
            if (ret != JPG_RET_SUCCESS)
            {
                JLOG(ERR,
                     "JPU_DecUpdateBitstreamBuffer failed Error code is 0x%x\n",
                     ret);
                goto FILL_BS_ERROR;
            }

            pBufInfo->fillendbs = 1;
        }
    }
    else
    {
        if (!pBufInfo->fillendbs)
        {
            ret = JPU_DecUpdateBitstreamBuffer(handle,
                                               STREAM_END_SIZE);
            if (ret != JPG_RET_SUCCESS)
            {
                JLOG(ERR,
                     "JPU_DecUpdateBitstreamBuffer failed Error code is 0x%x\n",
                     ret);
                goto FILL_BS_ERROR;
            }
            pBufInfo->fillendbs = 1;
        }
    }

FILL_BS_ERROR:

    return ret;
}

//#define INCREASE_Q_MATRIX
/******************************************************************************
    JPEG specific Helper
******************************************************************************/
int jpgGetHuffTable(EncMjpgParam *param)
{
    memcpy(param->huffBits[DC_TABLE_INDEX0], lumaDcBits, 16);    // Luma DC BitLength
    memcpy(param->huffVal[DC_TABLE_INDEX0], lumaDcValue, 16);    // Luma DC HuffValue

    memcpy(param->huffBits[AC_TABLE_INDEX0], lumaAcBits, 16);    // Luma DC BitLength
    memcpy(param->huffVal[AC_TABLE_INDEX0], lumaAcValue, 162);   // Luma DC HuffValue

    memcpy(param->huffBits[DC_TABLE_INDEX1], chromaDcBits, 16);  // Chroma DC BitLength
    memcpy(param->huffVal[DC_TABLE_INDEX1], chromaDcValue, 16);  // Chroma DC HuffValue

    memcpy(param->huffBits[AC_TABLE_INDEX1], chromaAcBits, 16);  // Chroma AC BitLength
    memcpy(param->huffVal[AC_TABLE_INDEX1], chromaAcValue, 162); // Chorma AC HuffValue

    return 1;
}

int jpgGetQMatrix(EncMjpgParam *param)
{
    // Rearrange and insert pre-defined Q-matrix to deticated
    // variable.
#ifdef INCREASE_Q_MATRIX
    for (int idx = 0; idx < 64; idx++)
    {
        param->qMatTab[DC_TABLE_INDEX0][idx] = (lumaQ2[idx] * 3 > 255) ? 255 : lumaQ2[idx] * 3;
        param->qMatTab[AC_TABLE_INDEX0][idx] = (chromaBQ2[idx] * 3 > 255) ? 255 : chromaBQ2[idx] * 3;
    }
#else
    memcpy(param->qMatTab[DC_TABLE_INDEX0], lumaQ2, 64);
    memcpy(param->qMatTab[AC_TABLE_INDEX0], chromaBQ2, 64);
#endif
    memcpy(param->qMatTab[DC_TABLE_INDEX1], param->qMatTab[DC_TABLE_INDEX0], 64);
    memcpy(param->qMatTab[AC_TABLE_INDEX1], param->qMatTab[AC_TABLE_INDEX0], 64);

    return 1;
}

/******************************************************************************
    EncOpenParam Initialization
******************************************************************************/

int getJpgEncOpenParamDefault(JpgEncOpenParam *pEncOP, EncConfigParam *pEncConfig)
{
    EncMjpgParam mjpgParam;

    memset(&mjpgParam, 0x00, sizeof(EncMjpgParam));

    pEncOP->picWidth         = pEncConfig->picWidth;
    pEncOP->picHeight        = pEncConfig->picHeight;
    pEncOP->sourceFormat     = pEncConfig->sourceFormat;
    pEncOP->restartInterval  = 0;
    pEncOP->chromaInterleave = pEncConfig->chromaInterleave;
    pEncOP->packedFormat     = pEncConfig->packedFormat;
    mjpgParam.sourceFormat   = pEncConfig->sourceFormat;

    jpgGetHuffTable(&mjpgParam);
    jpgGetQMatrix(&mjpgParam);
    memcpy(pEncOP->huffVal, mjpgParam.huffVal, 4 * 162);
    memcpy(pEncOP->huffBits, mjpgParam.huffBits, 4 * 256);
    memcpy(pEncOP->qMatTab, mjpgParam.qMatTab, 4 * 64);

    return 1;
}

int FillSdramBurst(BufInfo *pBufInfo, PhysicalAddress targetAddr,
                   PhysicalAddress bsBufStartAddr, PhysicalAddress bsBufEndAddr,
                   Uint32 size, int checkeos, int *streameos, int endian)
{
    Uint8 *pBuf;
    int    room;

    pBufInfo->count = 0;

    if (checkeos == 1 && (pBufInfo->point >= pBufInfo->size))
    {
        *streameos = 1;
        return 0;
    }

    if ((pBufInfo->size - pBufInfo->point) < (int)size)
        pBufInfo->count = (pBufInfo->size - pBufInfo->point);
    else
        pBufInfo->count = size;

    pBuf = pBufInfo->buf + pBufInfo->point;
    if ((targetAddr + pBufInfo->count) > bsBufEndAddr)
    {
        room = bsBufEndAddr - targetAddr;
        JpuWriteMem(targetAddr, pBuf, room, endian);
#if (defined CVI_JPG_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
        jdi_flush_ion_cache(targetAddr, pBuf, room);
#endif
        JpuWriteMem(bsBufStartAddr, pBuf + room, (pBufInfo->count - room), endian);
#if (defined CVI_JPG_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
        jdi_flush_ion_cache(bsBufStartAddr, (pBuf + room), (pBufInfo->count - room));
#endif
    }
    else
    {
        JpuWriteMem(targetAddr, pBuf, pBufInfo->count, endian);
#if (defined CVI_JPG_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
        jdi_flush_ion_cache(targetAddr, pBuf, pBufInfo->count);
#endif
    }

    pBufInfo->point += pBufInfo->count;
    return pBufInfo->count;
}

void GetMcuUnitSize(int format, int *mcuWidth, int *mcuHeight)
{
    switch (format)
    {
    case FORMAT_420:
        *mcuWidth  = 16;
        *mcuHeight = 16;
        break;
    case FORMAT_422:
        *mcuWidth  = 16;
        *mcuHeight = 8;
        break;
    case FORMAT_224:
        *mcuWidth  = 8;
        *mcuHeight = 16;
        break;
    default: // FORMAT_444,400
        *mcuWidth  = 8;
        *mcuHeight = 8;
        break;
    }
}

unsigned int GetFrameBufSize(int framebufFormat, int picWidth, int picHeight)
{
    unsigned int framebufSize = 0;
    unsigned int framebufWidth, framebufHeight;

    if (framebufFormat == FORMAT_420 || framebufFormat == FORMAT_422)
        framebufWidth = ((picWidth + 15) >> 4) << 4;
    else
        framebufWidth = ((picWidth + 7) >> 3) << 3;

    if (framebufFormat == FORMAT_420 || framebufFormat == FORMAT_224)
        framebufHeight = ((picHeight + 15) >> 4) << 4;
    else
        framebufHeight = ((picHeight + 7) >> 3) << 3;

    switch (framebufFormat)
    {
    case FORMAT_420:
        framebufSize = (framebufWidth * (((framebufHeight + 1) >> 1) << 1)) + ((((framebufWidth + 1) >> 1) * ((framebufHeight + 1) >> 1)) << 1);
        break;
    case FORMAT_224:
        framebufSize =
            framebufWidth * (((framebufHeight + 1) >> 1) << 1) + ((framebufWidth * ((framebufHeight + 1) >> 1)) << 1);
        break;
    case FORMAT_422:
        framebufSize =
            framebufWidth * framebufHeight + ((((framebufWidth + 1) >> 1) * framebufHeight) << 1);
        break;
    case FORMAT_444:
        framebufSize = framebufWidth * framebufHeight * 3;
        break;
    case FORMAT_400:
        framebufSize = framebufWidth * framebufHeight;
        break;
    }

    framebufSize = ((framebufSize + 7) & ~7);

    return framebufSize;
}

Uint64 jpgGetCurrentTime(void)
{
    return rt_tick_get();
}
