//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2011  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "vpuapifunc.h"

#include "coda9.h"
#include "coda9_regdefine.h"
#include "coda9_vpuconfig.h"
#include "common_regdefine.h"
#include "errno.h"
#include "malloc.h"
#include "product.h"
#include "wave4_regdefine.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define MAX_LAVEL_IDX         16
#define UNREFERENCED_PARAM(x) ((void)(x))

static const int g_anLevel[MAX_LAVEL_IDX] = {10, 11, 11, 12, 13,
                                             // 10, 16, 11, 12, 13,
                                             20, 21, 22, 30, 31, 32, 40, 41, 42,
                                             50, 51};

static const int g_anLevelMaxMBPS[MAX_LAVEL_IDX] = {
    1485, 1485, 3000, 6000, 11880, 11880, 19800, 20250,
    40500, 108000, 216000, 245760, 245760, 522240, 589824, 983040};

static const int g_anLevelMaxFS[MAX_LAVEL_IDX] = {
    99, 99, 396, 396, 396, 396, 792, 1620,
    1620, 3600, 5120, 8192, 8192, 8704, 22080, 36864};

static const int g_anLevelMaxBR[MAX_LAVEL_IDX] = {
    64, 64, 192, 384, 768, 2000, 4000, 4000,
    10000, 14000, 20000, 20000, 50000, 50000, 135000, 240000};

static const int g_anLevelSliceRate[MAX_LAVEL_IDX] = {
    0, 0, 0, 0, 0, 0, 0, 0, 22, 60, 60, 60, 24, 24, 24, 24};

static const int g_anLevelMaxMbs[MAX_LAVEL_IDX] = {
    28, 28, 56, 56, 56, 56, 79, 113, 113, 169, 202, 256, 256, 263, 420, 543};

extern void vdi_vcodec_initlock(unsigned long core_idx);
extern void vdi_vcodec_deinitlock(unsigned long core_idx);

/******************************************************************************
    define value
******************************************************************************/

/******************************************************************************
    Codec Instance Slot Management
******************************************************************************/

RetCode InitCodecInstancePool(Uint32 coreIdx)
{
    int                  i;
    CodecInst           *pCodecInst;
    vpu_instance_pool_t *vip;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);
    if (!vip) return RETCODE_INSUFFICIENT_RESOURCE;

    if (vip->instance_pool_inited == 0)
    {
        for (i = 0; i < MAX_NUM_INSTANCE; i++)
        {
            pCodecInst            = (CodecInst *)vip->codecInstPool[i];
            pCodecInst->instIndex = i;
            pCodecInst->inUse     = 0;

            CVI_VC_INFO("%d, pCodecInst->instIndex %d, pCodecInst->inUse %d\n", i,
                        pCodecInst->instIndex, pCodecInst->inUse);
        }
        vip->instance_pool_inited = 1;
    }
    return RETCODE_SUCCESS;
}

/*
 * GetCodecInstance() obtains a instance.
 * It stores a pointer to the allocated instance in *ppInst
 * and returns RETCODE_SUCCESS on success.
 * Failure results in 0(null pointer) in *ppInst and RETCODE_FAILURE.
 */

RetCode GetCodecInstance(Uint32 coreIdx, CodecInst **ppInst)
{
    int                  i;
    CodecInst           *pCodecInst = 0;
    vpu_instance_pool_t *vip;
    Uint32               handleSize;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);
    if (!vip) return RETCODE_INSUFFICIENT_RESOURCE;

    for (i = 0; i < MAX_NUM_INSTANCE; i++)
    {
        pCodecInst = (CodecInst *)vip->codecInstPool[i];

        if (!pCodecInst)
        {
            return RETCODE_FAILURE;
        }

        if (!pCodecInst->inUse)
        {
            break;
        }
    }

    if (i == MAX_NUM_INSTANCE)
    {
        *ppInst = 0;
        return RETCODE_FAILURE;
    }

    pCodecInst->inUse        = 1;
    pCodecInst->coreIdx      = coreIdx;
    pCodecInst->codecMode    = -1;
    pCodecInst->codecModeAux = -1;
#ifdef ENABLE_CNM_DEBUG_MSG
    pCodecInst->loggingEnable = 0;
#endif
    pCodecInst->isDecoder = TRUE;
    pCodecInst->productId = ProductVpuGetId(coreIdx);
    memset((void *)&pCodecInst->CodecInfo, 0x00, sizeof(pCodecInst->CodecInfo));

    handleSize = sizeof(DecInfo);
    if (handleSize < sizeof(EncInfo))
    {
        handleSize = sizeof(EncInfo);
    }

    pCodecInst->CodecInfo = (void *)malloc(handleSize);
    if (pCodecInst->CodecInfo == NULL)
    {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }
    memset(pCodecInst->CodecInfo, 0x00, sizeof(handleSize));

    *ppInst = pCodecInst;

    if (vdi_open_instance(pCodecInst->coreIdx, pCodecInst->instIndex) < 0)
    {
        return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

void FreeCodecInstance(CodecInst *pCodecInst)
{
    pCodecInst->inUse        = 0;
    pCodecInst->codecMode    = -1;
    pCodecInst->codecModeAux = -1;

    vdi_close_instance(pCodecInst->coreIdx, pCodecInst->instIndex);

    free(pCodecInst->CodecInfo);
    pCodecInst->CodecInfo = NULL;
}

RetCode CheckInstanceValidity(CodecInst *pCodecInst)
{
    int                  i;
    vpu_instance_pool_t *vip;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(pCodecInst->coreIdx);
    if (!vip)
    {
        CVI_VC_ERR("RETCODE_INSUFFICIENT_RESOURCE\n");
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    for (i = 0; i < MAX_NUM_INSTANCE; i++)
    {
        if ((CodecInst *)vip->codecInstPool[i] == pCodecInst)
            return RETCODE_SUCCESS;
    }

    CVI_VC_ERR("RETCODE_INVALID_HANDLE\n");
    return RETCODE_INVALID_HANDLE;
}

/******************************************************************************
    API Subroutines
******************************************************************************/

RetCode CheckDecOpenParam(DecOpenParam *pop)
{
    if (pop == 0)
    {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->bitstreamBuffer % 8)
    {
        return RETCODE_INVALID_PARAM;
    }

    if (pop->bitstreamBufferSize % 1024 || pop->bitstreamBufferSize < 1024 || pop->bitstreamBufferSize > (256 * 1024 * 1024 - 1))
    {
        return RETCODE_INVALID_PARAM;
    }

    if (pop->bitstreamFormat != STD_AVC && pop->bitstreamFormat != STD_HEVC)
    {
        return RETCODE_INVALID_PARAM;
    }

    if (pop->wtlEnable && pop->tiled2LinearEnable)
    {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->wtlEnable)
    {
        if (pop->wtlMode != FF_FRAME && pop->wtlMode != FF_FIELD)
        {
            return RETCODE_INVALID_PARAM;
        }
    }

    if (pop->coreIdx > MAX_NUM_VPU_CORE)
    {
        return RETCODE_INVALID_PARAM;
    }

    return RETCODE_SUCCESS;
}

Uint64 GetTimestamp(EncHandle handle)
{
    CodecInst *pCodecInst = (CodecInst *)handle;
    EncInfo   *pEncInfo   = NULL;
    Uint64     pts;
    Uint32     fps;

    if (pCodecInst == NULL)
    {
        return 0;
    }

    pEncInfo = &pCodecInst->CodecInfo->encInfo;
    fps      = pEncInfo->openParam.frameRateInfo;
    if (fps == 0)
    {
        fps = 30; /* 30 fps */
    }

    pts               = pEncInfo->curPTS;
    pEncInfo->curPTS += 90000 / fps; /* 90KHz/fps */

    return pts;
}

RetCode CalcEncCropInfo(EncHevcParam *param, int rotMode, int srcWidth,
                        int srcHeight)
{
    int width32, height32, pad_right, pad_bot;
    int crop_right, crop_left, crop_top, crop_bot;
    int prp_mode = rotMode >> 1; // remove prp_enable bit

    width32  = (srcWidth + 31) & ~31;
    height32 = (srcHeight + 31) & ~31;

    pad_right = width32 - srcWidth;
    pad_bot   = height32 - srcHeight;

    if (param->confWinRight > 0)
        crop_right = param->confWinRight + pad_right;
    else
        crop_right = pad_right;

    if (param->confWinBot > 0)
        crop_bot = param->confWinBot + pad_bot;
    else
        crop_bot = pad_bot;

    crop_top  = param->confWinTop;
    crop_left = param->confWinLeft;

    param->confWinTop   = crop_top;
    param->confWinLeft  = crop_left;
    param->confWinBot   = crop_bot;
    param->confWinRight = crop_right;

    if (prp_mode == 1 || prp_mode == 15)
    {
        param->confWinTop   = crop_right;
        param->confWinLeft  = crop_top;
        param->confWinBot   = crop_left;
        param->confWinRight = crop_bot;
    }
    else if (prp_mode == 2 || prp_mode == 12)
    {
        param->confWinTop   = crop_bot;
        param->confWinLeft  = crop_right;
        param->confWinBot   = crop_top;
        param->confWinRight = crop_left;
    }
    else if (prp_mode == 3 || prp_mode == 13)
    {
        param->confWinTop   = crop_left;
        param->confWinLeft  = crop_bot;
        param->confWinBot   = crop_right;
        param->confWinRight = crop_top;
    }
    else if (prp_mode == 4 || prp_mode == 10)
    {
        param->confWinTop = crop_bot;
        param->confWinBot = crop_top;
    }
    else if (prp_mode == 8 || prp_mode == 6)
    {
        param->confWinLeft  = crop_right;
        param->confWinRight = crop_left;
    }
    else if (prp_mode == 5 || prp_mode == 11)
    {
        param->confWinTop   = crop_left;
        param->confWinLeft  = crop_top;
        param->confWinBot   = crop_right;
        param->confWinRight = crop_bot;
    }
    else if (prp_mode == 7 || prp_mode == 9)
    {
        param->confWinTop   = crop_right;
        param->confWinLeft  = crop_bot;
        param->confWinBot   = crop_left;
        param->confWinRight = crop_top;
    }

    CVI_VC_CFG("confWinTop = %d\n", param->confWinTop);
    CVI_VC_CFG("confWinBot = %d\n", param->confWinBot);
    CVI_VC_CFG("confWinBot = %d\n", param->confWinBot);
    CVI_VC_CFG("confWinRight = %d\n", param->confWinRight);

    return RETCODE_SUCCESS;
}

int DecBitstreamBufEmpty(DecInfo *pDecInfo)
{
    return (pDecInfo->streamRdPtr == pDecInfo->streamWrPtr);
}

RetCode CheckEncInstanceValidity(EncHandle handle)
{
    CodecInst *pCodecInst;
    RetCode    ret;

    if (handle == NULL)
    {
        CVI_VC_ERR("null handle %p\n", handle);
        return RETCODE_INVALID_HANDLE;
    }
    pCodecInst = handle;
    ret        = CheckInstanceValidity(pCodecInst);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("CheckInstanceValidity ret %d\n", ret);
        return RETCODE_INVALID_HANDLE;
    }
    if (!pCodecInst->inUse)
    {
        return RETCODE_INVALID_HANDLE;
    }

    if (pCodecInst->codecMode != MP4_ENC && pCodecInst->codecMode != HEVC_ENC && pCodecInst->codecMode != AVC_ENC && pCodecInst->codecMode != C7_MP4_ENC && pCodecInst->codecMode != C7_AVC_ENC)
    {
        return RETCODE_INVALID_HANDLE;
    }
    return RETCODE_SUCCESS;
}

RetCode CheckEncParam(EncHandle handle, EncParam *param)
{
    CodecInst *pCodecInst;
    EncInfo   *pEncInfo;

    pCodecInst = handle;
    pEncInfo   = &pCodecInst->CodecInfo->encInfo;

    if (param == 0)
    {
        CVI_VC_ERR("null param\n");
        return RETCODE_INVALID_PARAM;
    }

    if (param->setROI.mode && (pCodecInst->codecMode == AVC_ENC || pCodecInst->codecMode == C7_AVC_ENC))
    {
        if (param->setROI.number < 0 || param->setROI.number > MAX_CODA980_ROI_NUMBER)
        {
            CVI_VC_ERR("ROI number %d out of range\n", param->setROI.number);
            return RETCODE_INVALID_PARAM;
        }
    }

    if (param->skipPicture != 0 && param->skipPicture != 1)
    {
        CVI_VC_ERR("skipPicture %d out of range\n", param->skipPicture);
        return RETCODE_INVALID_PARAM;
    }

    if (param->skipPicture == 0)
    {
        if (param->sourceFrame == 0)
        {
            CVI_VC_ERR("null sourceFrame\n");
            return RETCODE_INVALID_FRAME_BUFFER;
        }
        if (param->forceIPicture != 0 && param->forceIPicture != 1)
        {
            CVI_VC_ERR("forceIPicture %d out of range\n", param->forceIPicture);
            return RETCODE_INVALID_PARAM;
        }
    }

    if (pEncInfo->openParam.bitRate == 0)
    { // no rate control
        if (pCodecInst->codecMode == HEVC_ENC)
        {
            if (param->forcePicQpEnable == 1)
            {
                if (param->forcePicQpI < 0 || param->forcePicQpI > 51)
                {
                    CVI_VC_ERR("forcePicQpI %d out of range\n", param->forcePicQpI);
                    return RETCODE_INVALID_PARAM;
                }

                if (param->forcePicQpP < 0 || param->forcePicQpP > 51)
                {
                    CVI_VC_ERR("forcePicQpP %d out of range\n", param->forcePicQpP);
                    return RETCODE_INVALID_PARAM;
                }

                if (param->forcePicQpB < 0 || param->forcePicQpB > 51)
                {
                    CVI_VC_ERR("forcePicQpB %d out of range\n", param->forcePicQpB);
                    return RETCODE_INVALID_PARAM;
                }
            }
            if (pEncInfo->ringBufferEnable == 0)
            {
                if (param->picStreamBufferAddr % 16 || param->picStreamBufferSize == 0)
                {
                    CVI_VC_ERR("picStreamBufferAddr / Size\n");
                    return RETCODE_INVALID_PARAM;
                }
            }
        }
        else
        { // AVC_ENC
            if (param->quantParam < 0 || param->quantParam > 51)
            {
                CVI_VC_ERR("quantParam %d out of range\n", param->quantParam);
                return RETCODE_INVALID_PARAM;
            }
        }
    }
    if (pEncInfo->ringBufferEnable == 0)
    {
        if (param->picStreamBufferAddr % 8 || param->picStreamBufferSize == 0)
        {
            CVI_VC_ERR("BufferAddr 0x" VCODEC_UINT64_FORMAT_HEX
                       " BufferSize %d fail\n",
                       param->picStreamBufferAddr, param->picStreamBufferSize);
            return RETCODE_INVALID_PARAM;
        }
    }

    if (param->ctuOptParam.roiEnable && param->ctuOptParam.ctuQpEnable)
    {
        CVI_VC_ERR("roiEnable + ctuQpEnable, not support\n");
        return RETCODE_INVALID_PARAM;
    }
    return RETCODE_SUCCESS;
}

#ifdef SUPPORT_980_ROI_RC_LIB
static int32_t fw_util_round_divide(int32_t dividend, int32_t divisor)
{
    return (dividend + (divisor >> 1)) / divisor;
}
#endif

/**
 * GetEncHeader()
 *  1. Generate encoder header bitstream
 * @param handle         : encoder handle
 * @param encHeaderParam : encoder header parameter (buffer, size, type)
 * @return none
 */
RetCode GetEncHeader(EncHandle handle, EncHeaderParam *encHeaderParam)
{
    CodecInst      *pCodecInst;
    EncInfo        *pEncInfo;
    EncOpenParam   *encOP;
    int             encHeaderCode;
    int             ppsCopySize = 0;
    unsigned char   pps_temp[256];
    int             spsID = 0;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;
    int             flag = 0;
    Uint32          val  = 0;

    pCodecInst = handle;
    pEncInfo   = &pCodecInst->CodecInfo->encInfo;
    encOP      = &(pEncInfo->openParam);

    if (pEncInfo->ringBufferEnable == 0)
    {
        if (pEncInfo->lineBufIntEn) val |= (0x1 << 6);
        val |= (0x1 << 5);
        val |= (0x1 << 4);
    }
    else
    {
        val |= (0x1 << 3);
    }
    val |= pEncInfo->openParam.streamEndian;
    VpuWriteReg(pCodecInst->coreIdx, BIT_BIT_STREAM_CTRL, val);

    if (pEncInfo->ringBufferEnable == 0)
    {
        SetEncBitStreamInfo(pCodecInst, encHeaderParam, NULL);
    }

    if ((encHeaderParam->headerType == SPS_RBSP || encHeaderParam->headerType == SPS_RBSP_MVC) && pEncInfo->openParam.bitstreamFormat == STD_AVC)
    {
        Uint32 CropV, CropH;
        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_PROFILE,
                    encOP->EncStdParam.avcParam.profile);
        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CHROMA_FORMAT,
                    encOP->EncStdParam.avcParam.chromaFormat400);
        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_FIELD_FLAG,
                    (encOP->EncStdParam.avcParam.fieldRefMode << 1) | encOP->EncStdParam.avcParam.fieldFlag);
        if (encOP->EncStdParam.avcParam.frameCroppingFlag == 1)
        {
            flag   = 1;
            CropH  = encOP->EncStdParam.avcParam.frameCropLeft << 16;
            CropH |= encOP->EncStdParam.avcParam.frameCropRight;
            CropV  = encOP->EncStdParam.avcParam.frameCropTop << 16;
            CropV |= encOP->EncStdParam.avcParam.frameCropBottom;
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_FRAME_CROP_H, CropH);
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_FRAME_CROP_V, CropV);
        }

#ifdef SUPPORT_980_ROI_RC_LIB

        // AVC don't care about that It is only used for svc_t
        VpuWriteReg(
            pCodecInst->coreIdx, CMD_ENC_HEADER_FRAME,
            pEncInfo->max_temporal_id << 16 | pEncInfo->num_ref_frames_decoder);
#endif
    }
    if (encHeaderParam->headerType == SPS_RBSP_MVC) spsID = 1;
    encHeaderCode = encHeaderParam->headerType | (flag << 3); // paraSetType>>
    // SPS: 0, PPS:
    // 1, VOS: 1,
    // VO: 2, VOL:
    // 0

    encHeaderCode |= (pEncInfo->openParam.EncStdParam.avcParam.level) << 8;
    encHeaderCode |= (spsID << 24);
    if ((encHeaderParam->headerType == PPS_RBSP || encHeaderParam->headerType == PPS_RBSP_MVC) && pEncInfo->openParam.bitstreamFormat == STD_AVC)
    {
        int          ActivePPSIdx = pEncInfo->ActivePPSIdx;
        AvcPpsParam *ActivePPS =
            &encOP->EncStdParam.avcParam.ppsParam[ActivePPSIdx];
        if (ActivePPS->entropyCodingMode != 2)
        {
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CABAC_MODE,
                        ActivePPS->entropyCodingMode);
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CABAC_INIT_IDC,
                        ActivePPS->cabacInitIdc);
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_TRANSFORM_8X8,
                        ActivePPS->transform8x8Mode);
            if (encHeaderParam->headerType == PPS_RBSP)
                encHeaderCode |= (ActivePPS->ppsId << 16);
            else if (encHeaderParam->headerType == PPS_RBSP_MVC)
                encHeaderCode |= (1 << 24) | ((ActivePPS->ppsId + 1) << 16);
        }
        else
        {
            int mvcPPSOffset = 0;
            if (encHeaderParam->headerType == PPS_RBSP_MVC)
            {
                encHeaderCode |= (1 << 24); /* sps_id: 1 */
                mvcPPSOffset   = 2;
            }
            // PPS for I frame (CAVLC)
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CABAC_MODE, 0);
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CABAC_INIT_IDC,
                        ActivePPS->cabacInitIdc);
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_TRANSFORM_8X8,
                        ActivePPS->transform8x8Mode);
            encHeaderCode |= ((ActivePPS->ppsId + mvcPPSOffset) << 16);
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CODE, encHeaderCode);

            Coda9BitIssueCommand(pCodecInst->coreIdx, pCodecInst, ENCODE_HEADER);
            if (vdi_wait_vpu_busy(pCodecInst->coreIdx, __VPU_BUSY_TIMEOUT,
                                  BIT_BUSY_FLAG)
                == -1)
            {
#ifdef ENABLE_CNM_DEBUG_MSG
                if (pCodecInst->loggingEnable)
                    vdi_log(pCodecInst->coreIdx, ENCODE_HEADER, 2);
#endif
                SetPendingInst(pCodecInst->coreIdx, NULL, __func__, __LINE__);
                CVI_VC_ERR("SetPendingInst clear\n");
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }
#ifdef ENABLE_CNM_DEBUG_MSG
            if (pCodecInst->loggingEnable)
                vdi_log(pCodecInst->coreIdx, ENCODE_HEADER, 0);
#endif

            if (pEncInfo->ringBufferEnable == 0)
            {
                ppsCopySize =
                    vdi_remap_memory_address(
                        pCodecInst->coreIdx,
                        VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr))
                    - encHeaderParam->buf;
                VpuReadMem(pCodecInst->coreIdx, encHeaderParam->buf, &pps_temp[0],
                           ppsCopySize, pEncInfo->openParam.streamEndian);
            }

            // PPS for P frame (CABAC)
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CABAC_MODE, 1);
            encHeaderCode |=
                ((ActivePPS->ppsId + mvcPPSOffset + 1) << 16); // ppsId=1;
        }
    }

    // SET_SEI param
    if (pEncInfo->openParam.bitstreamFormat == STD_AVC && encHeaderParam->headerType == SVC_RBSP_SEI)
    {
        int data = 0;
        if (pEncInfo->max_temporal_id)
        {
            int i;
            int num_layer;
            int frm_rate[MAX_GOP_SIZE] = {
                0,
            };

            if (pEncInfo->openParam.gopSize == 1) // ||
            // seq->intra_period
            // == 1) //all
            // intra
            {
                num_layer   = 1;
                frm_rate[0] = pEncInfo->openParam.frameRateInfo;
            }
            else
            {
                num_layer = 1;
                for (i = 0; i < pEncInfo->gop_size; i++)
                {
                    // assert(seq->gop_entry[i].temporal_id
                    // < MAX_GOP_SIZE);
                    num_layer = MAX(num_layer, pEncInfo->gop_entry[i].temporal_id + 1);
                    frm_rate[pEncInfo->gop_entry[i].temporal_id]++;
                }

                for (i = 0; i < num_layer; i++)
                {
                    frm_rate[i] = fw_util_round_divide(
                        frm_rate[i] * pEncInfo->openParam.frameRateInfo * 256,
                        pEncInfo->gop_size);
                    if (i > 0) frm_rate[i] += frm_rate[i - 1];
                }
            }

            data = num_layer << 16 | frm_rate[0];
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_SVC_SEI_INFO1, data);
            data = frm_rate[1] << 16 | frm_rate[2];
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_SVC_SEI_INFO2, data);
        }
        else
        {
            data = 0;
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_SVC_SEI_INFO1, data);
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_SVC_SEI_INFO2, data);
        }
    }

    encHeaderCode |= (encHeaderParam->zeroPaddingEnable & 1) << 31;
    VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_CODE, encHeaderCode);

    CVI_VC_TRACE("streamRdPtr = 0x" VCODEC_UINT64_FORMAT_HEX
                 ", streamWrPtr = 0x" VCODEC_UINT64_FORMAT_HEX "\n",
                 pEncInfo->streamRdPtr, pEncInfo->streamWrPtr);

    VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr,
                pEncInfo->streamRdPtr);
    VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr,
                pEncInfo->streamWrPtr);

    CVI_VC_FLOW("ENCODE_HEADER\n");

    Coda9BitIssueCommand(pCodecInst->coreIdx, pCodecInst, ENCODE_HEADER);
    if (vdi_wait_vpu_busy(pCodecInst->coreIdx, __VPU_BUSY_TIMEOUT,
                          BIT_BUSY_FLAG)
        == -1)
    {
#ifdef ENABLE_CNM_DEBUG_MSG
        if (pCodecInst->loggingEnable)
            vdi_log(pCodecInst->coreIdx, ENCODE_HEADER, 2);
#endif
        SetPendingInst(pCodecInst->coreIdx, NULL, __func__, __LINE__);
        CVI_VC_ERR("SetPendingInst clear\n");
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }
#ifdef ENABLE_CNM_DEBUG_MSG
    if (pCodecInst->loggingEnable) vdi_log(pCodecInst->coreIdx, ENCODE_HEADER, 0);
#endif

    if (pEncInfo->ringBufferEnable == 0 && pEncInfo->bEsBufQueueEn == 0)
    {
        rdPtr = encHeaderParam->buf;
        wrPtr = vdi_remap_memory_address(
            pCodecInst->coreIdx,
            VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr));

        if (ppsCopySize)
        {
            int size = wrPtr - rdPtr;
            VpuReadMem(pCodecInst->coreIdx, rdPtr, &pps_temp[ppsCopySize], size,
                       pEncInfo->openParam.streamEndian);
            ppsCopySize += size;
            VpuWriteMem(pCodecInst->coreIdx, rdPtr, &pps_temp[0], ppsCopySize,
                        pEncInfo->openParam.streamEndian);
            encHeaderParam->size = ppsCopySize;
            wrPtr                = rdPtr + ppsCopySize;
            VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr, wrPtr);
        }
        else
        {
            encHeaderParam->size = wrPtr - rdPtr;
        }
    }
    else
    {
        rdPtr = vdi_remap_memory_address(
            pCodecInst->coreIdx,
            VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr));
        wrPtr = vdi_remap_memory_address(
            pCodecInst->coreIdx,
            VpuReadReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr));
        encHeaderParam->buf  = rdPtr;
        encHeaderParam->size = wrPtr - rdPtr;
    }

    pEncInfo->streamWrPtr = wrPtr;
    pEncInfo->streamRdPtr = rdPtr;

    CVI_VC_BS("streamRdPtr = 0x" VCODEC_UINT64_FORMAT_HEX
              ", streamWrPtr = 0x" VCODEC_UINT64_FORMAT_HEX ", size = 0x%zX\n",
              pEncInfo->streamRdPtr, pEncInfo->streamWrPtr, encHeaderParam->size);

    return RETCODE_SUCCESS;
}

RetCode SetEncBitStreamInfo(CodecInst      *pCodecInst,
                            EncHeaderParam *encHeaderParam, EncParam *param)
{
    EncInfo        *pEncInfo         = &pCodecInst->CodecInfo->encInfo;
    PhysicalAddress paBitStreamWrPtr = VPU_ALIGN8(pEncInfo->streamWrPtr);
    PhysicalAddress paBitStreamStart;
    int             bitStreamBufSize;
    CVI_VC_BS("pEncInfo->bEsBufQueueEn:%d\n", pEncInfo->bEsBufQueueEn);
    if (encHeaderParam)
    {
        if (pEncInfo->bEsBufQueueEn == 0)
        {
            paBitStreamStart = encHeaderParam->buf;
            bitStreamBufSize = (int)encHeaderParam->size;
        }
        else
        {
            paBitStreamStart = paBitStreamWrPtr;
            bitStreamBufSize = (int)(pEncInfo->streamBufEndAddr - paBitStreamWrPtr);
        }

        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_BB_START, paBitStreamStart);
        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_HEADER_BB_SIZE,
                    bitStreamBufSize >> 10); // size in KB

        VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr,
                    paBitStreamStart);
        pEncInfo->streamRdPtr = paBitStreamStart;

        CVI_VC_BS("headerStreamBufferAddr = 0x" VCODEC_UINT64_FORMAT_HEX
                  ", headerStreamBufferSize = 0x%X\n",
                  paBitStreamStart, bitStreamBufSize);
    }
    else if (param)
    {
        // reset RdPtr/WrPtr to streamBufStartAddr:
        // 1. bEsBufQueueEn disable or
        // 2. empty size (pEncInfo->streamBufEndAddr - pEncInfo->streamWrPtr) lower
        // than threshold
        if (pEncInfo->bEsBufQueueEn == 0 || ((pEncInfo->streamBufEndAddr - paBitStreamWrPtr) < H264_MIN_BITSTREAM_SIZE))
        {
            paBitStreamStart = param->picStreamBufferAddr;
            bitStreamBufSize = param->picStreamBufferSize;
        }
        else
        {
            paBitStreamStart = paBitStreamWrPtr;
            bitStreamBufSize = (int)(pEncInfo->streamBufEndAddr - paBitStreamWrPtr);
        }

        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_PIC_BB_START, paBitStreamStart);
        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_PIC_BB_SIZE,
                    bitStreamBufSize >> 10); // size in KB
        VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamRdPtrRegAddr,
                    paBitStreamStart);
        pEncInfo->streamRdPtr = paBitStreamStart;

        CVI_VC_BS("picStreamBufferAddr = 0x" VCODEC_UINT64_FORMAT_HEX
                  ", picStreamBufferSize = 0x%X\n",
                  paBitStreamStart, bitStreamBufSize);
    }

    return RETCODE_SUCCESS;
}

RetCode SetBitrate(EncHandle handle, Uint32 *pBitrate)
{
    CodecInst *pCodecInst;
    int        data    = 0;
    Uint32     bitrate = *pBitrate;

    pCodecInst = handle;

    data = 1 << 2;

    VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_PARAM_CHANGE_ENABLE, data);
    VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_PARAM_CHANGE_BITRATE, bitrate);

    Coda9BitIssueCommand(pCodecInst->coreIdx, pCodecInst, RC_CHANGE_PARAMETER);
    if (vdi_wait_vpu_busy(pCodecInst->coreIdx, __VPU_BUSY_TIMEOUT,
                          BIT_BUSY_FLAG)
        == -1)
    {
#ifdef ENABLE_CNM_DEBUG_MSG
        if (pCodecInst->loggingEnable)
            vdi_log(pCodecInst->coreIdx, RC_CHANGE_PARAMETER, 2);
#endif
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }
#ifdef ENABLE_CNM_DEBUG_MSG
    if (pCodecInst->loggingEnable)
        vdi_log(pCodecInst->coreIdx, RC_CHANGE_PARAMETER, 0);
#endif

    return RETCODE_SUCCESS;
}

RetCode SetMinMaxQp(EncHandle handle, MinMaxQpChangeParam *param)
{
    CodecInst *pCodecInst;
    int        data = 0;
    pCodecInst      = handle;

    data = 1 << 6;

    VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_PARAM_CHANGE_ENABLE, data);

    data = (param->maxQpI << 0) | (param->maxQpIEnable << 6) | (param->minQpI << 8) | (param->maxQpIEnable << 14) | (param->maxQpP << 16) | (param->maxQpPEnable << 22) | (param->minQpP << 24) | (param->maxQpPEnable << 30);

    VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_PARAM_CHANGE_MIN_MAX_QP, data);

    Coda9BitIssueCommand(pCodecInst->coreIdx, pCodecInst, RC_CHANGE_PARAMETER);
    if (vdi_wait_vpu_busy(pCodecInst->coreIdx, __VPU_BUSY_TIMEOUT,
                          BIT_BUSY_FLAG)
        == -1)
    {
#ifdef ENABLE_CNM_DEBUG_MSG
        if (pCodecInst->loggingEnable)
            vdi_log(pCodecInst->coreIdx, RC_CHANGE_PARAMETER, 2);
#endif
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }
#ifdef ENABLE_CNM_DEBUG_MSG
    if (pCodecInst->loggingEnable)
        vdi_log(pCodecInst->coreIdx, RC_CHANGE_PARAMETER, 0);
#endif

    return RETCODE_SUCCESS;
}

RetCode VcodecTimeLock(Uint32 core_idx, int timeout_ms)
{
    int ret = 0;

    CVI_VC_LOCK("core_idx %d\n", core_idx);

    if (core_idx >= MAX_NUM_VPU_CORE)
    {
        CVI_VC_ERR("\n");
        return -1;
    }

    if (timeout_ms == 0)
    {
        ret = vdi_vcodec_trylock(core_idx);
        if (ret == 0)
        {
            // trylock success
            vdi_set_clock_gate(core_idx, CLK_ENABLE);
        }
        else
        {
            // trylock failure
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
    }
    else
    {
        ret = vdi_vcodec_timelock(core_idx, timeout_ms);

        if (ret != 0)
        {
            CVI_VC_TRACE("TimeLock Timeout, ret %p\n", __builtin_return_address(0));
            if (ret == ETIMEDOUT) return RETCODE_VPU_RESPONSE_TIMEOUT;

            CVI_VC_ERR("vdi_vcodec_timelock error[%d]\n", ret);
            return RETCODE_FAILURE;
        }
    }

    vdi_set_clock_gate(core_idx, CLK_ENABLE);

    CoreSleepWake(core_idx, 0);

    return RETCODE_SUCCESS;
}

void InitVcodecLock(Uint32 core_idx)
{
    CVI_VC_LOCK("init core_idx %d\n", core_idx);

    if (core_idx >= MAX_NUM_VPU_CORE)
    {
        CVI_VC_ERR("init core_idx %d\n", core_idx);
        return;
    }

    vdi_vcodec_initlock(core_idx);
}

void DeinitVcodecLock(Uint32 core_idx)
{
    CVI_VC_LOCK("init core_idx %d\n", core_idx);

    if (core_idx >= MAX_NUM_VPU_CORE)
    {
        CVI_VC_ERR("init core_idx %d\n", core_idx);
        return;
    }

    vdi_vcodec_deinitlock(core_idx);
}
/* EnterVcodecLock:
 *     Acquire mutex lock for 264/265 dec/enc thread in one process
 */
void EnterVcodecLock(Uint32 core_idx)
{
    CVI_VC_LOCK("core_idx %d\n", core_idx);

    if (core_idx >= MAX_NUM_VPU_CORE)
    {
        CVI_VC_ERR("core_idx %d\n", core_idx);
        return;
    }

    vdi_vcodec_lock(core_idx);
    vdi_set_clock_gate(core_idx, CLK_ENABLE);

    CoreSleepWake(core_idx, 0);
}

/* LeaveVcodecLock:
 *     Release mutex lock for 264/265 dec/enc thread in one process
 */
void LeaveVcodecLock(Uint32 core_idx)
{
    CVI_VC_LOCK("core_idx %d\n", core_idx);

    if (core_idx >= MAX_NUM_VPU_CORE)
    {
        CVI_VC_ERR("core_idx %d\n", core_idx);
        return;
    }

    CoreSleepWake(core_idx, 1);

    vdi_set_clock_gate(core_idx, CLK_DISABLE);
    vdi_vcodec_unlock(core_idx);
}

void CoreSleepWake(Uint32 coreIdx, int iSleepWake)
{
#ifdef ARCH_CV183X
    UNREFERENCED_PARAMETER(coreIdx);
    UNREFERENCED_PARAMETER(iSleepWake);
#else
    int coreStatus = CORE_STATUS_WAIT_INIT;

    if (coreIdx >= MAX_NUM_VPU_CORE)
    {
        CVI_VC_ERR("\n");
        return;
    }

    if (iSleepWake == 0)
    {
        VPU_GetCoreStatus(coreIdx, &coreStatus);

        if (coreStatus == CORE_STATUS_SLEEP)
        {
            VPU_SleepWake_EX(coreIdx, 0);
            VPU_SetCoreStatus(coreIdx, CORE_STATUS_WAKE);
        }
    }
    else if (iSleepWake == 1)
    {
        int TaskNum = VPU_GetCoreTaskNum(coreIdx);

        if (TaskNum == 0)
        {
            VPU_SetCoreStatus(coreIdx, CORE_STATUS_WAIT_INIT);
        }
        else
        {
            VPU_GetCoreStatus(coreIdx, &coreStatus);
            if (coreStatus != CORE_STATUS_SLEEP)
            {
                VPU_SleepWake_EX(coreIdx, 1);
                VPU_SetCoreStatus(coreIdx, CORE_STATUS_SLEEP);
            }
        }

        VPU_GetCoreStatus(coreIdx, &coreStatus);
    }
#endif
}

RetCode EnterDispFlagLock(Uint32 coreIdx)
{
    if (vdi_disp_lock(coreIdx) != 0) return RETCODE_FAILURE;
    return RETCODE_SUCCESS;
}

RetCode LeaveDispFlagLock(Uint32 coreIdx)
{
    vdi_disp_unlock(coreIdx);
    return RETCODE_SUCCESS;
}

RetCode SetClockGate(Uint32 coreIdx, Uint32 on)
{
    CodecInst           *inst;
    vpu_instance_pool_t *vip;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);
    if (!vip)
    {
        VLOG(ERR, "SetClockGate: RETCODE_INSUFFICIENT_RESOURCE\n");
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    inst = (CodecInst *)vip->pendingInst;

    if (!on && (inst || !vdi_lock_check(coreIdx))) return RETCODE_SUCCESS;

    vdi_set_clock_gate(coreIdx, on);

    return RETCODE_SUCCESS;
}

void SetPendingInst(Uint32 coreIdx, CodecInst *inst, const char *caller_fn,
                    int caller_line)
{
    vpu_instance_pool_t *vip;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);
    if (!vip) return;

    if (inst)
        CVI_VC_TRACE(
            "coreIdx %d, set pending inst = %p, inst->instIndex %d, s_n %s, s_line "
            "%d\n",
            coreIdx, inst, inst->instIndex, caller_fn, caller_line);
    else
        CVI_VC_TRACE("coreIdx %d, clear pending inst, s_n %s, s_line %d\n", coreIdx,
                     caller_fn, caller_line);

    vip->pendingInst = inst;
    if (inst)
        vip->pendingInstIdxPlus1 = (inst->instIndex + 1);
    else
        vip->pendingInstIdxPlus1 = 0;
}

void ClearPendingInst(Uint32 coreIdx)
{
    vpu_instance_pool_t *vip;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);
    if (!vip) return;

    if (vip->pendingInst)
    {
        vip->pendingInst         = 0;
        vip->pendingInstIdxPlus1 = 0;
    }
    else
    {
        CVI_VC_ERR("pendingInst is NULL\n");
    }
}

CodecInst *GetPendingInst(Uint32 coreIdx)
{
    vpu_instance_pool_t *vip;
    int                  pendingInstIdx;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(coreIdx);
    if (!vip)
    {
        CVI_VC_ERR("VIP null\n");
        return NULL;
    }

    if (!vip->pendingInst)
    {
        return NULL;
    }

    pendingInstIdx = vip->pendingInstIdxPlus1 - 1;
    if (pendingInstIdx < 0)
    {
        CVI_VC_ERR("pendingInstIdx < 0, vip->pendingInst %p,  lr %p\n",
                   vip->pendingInst, __builtin_return_address(0));
        CVI_VC_ERR("pendingInstIdx %d, vip->pendingInstIdxPlus1 %d\n",
                   pendingInstIdx, vip->pendingInstIdxPlus1);
        return NULL;
    }

    if (pendingInstIdx > MAX_NUM_INSTANCE)
    {
        CVI_VC_ERR("pendingInstIdx > MAX_NUM_INSTANCE\n");
        return NULL;
    }
    return (CodecInst *)vip->codecInstPool[pendingInstIdx];
}

Int32 MaverickCache2Config(MaverickCacheConfig *pCache, BOOL decoder,
                           BOOL interleave, Uint32 bypass, Uint32 burst,
                           Uint32 merge, TiledMapType mapType,
                           Uint32 wayshape)
{
    unsigned int cacheConfig = 0;

    if (decoder == TRUE)
    {
        if (mapType == 0)
        { // LINEAR_FRAME_MAP
            // VC1 opposite field padding is not allowable in UV
            // separated, burst 8 and linear map
            if (!interleave) burst = 0;

            wayshape = 15;

            if (merge == 1) merge = 3;

            // GDI constraint. Width should not be over 64
            if ((merge == 1) && (burst)) burst = 0;
        }
        else
        {
            // horizontal merge constraint in tiled map
            if (merge == 1) merge = 3;
        }
    }
    else
    { // encoder
        if (mapType == LINEAR_FRAME_MAP)
        {
            wayshape = 15;
            // GDI constraint. Width should not be over 64
            if ((merge == 1) && (burst)) burst = 0;
        }
        else
        {
            // horizontal merge constraint in tiled map
            if (merge == 1) merge = 3;
        }
    }

    cacheConfig = (merge & 0x3) << 9;
    cacheConfig = cacheConfig | ((wayshape & 0xf) << 5);
    cacheConfig = cacheConfig | ((burst & 0x1) << 3);
    cacheConfig = cacheConfig | (bypass & 0x3);

    if (mapType != 0) // LINEAR_FRAME_MAP
        cacheConfig = cacheConfig | 0x00000004;

    ///{16'b0, 5'b0, merge[1:0], wayshape[3:0], 1'b0, burst[0], map[0],
    /// bypass[1:0]};
    pCache->type2.CacheMode = cacheConfig;

    return 1;
}

RetCode AllocateLinearFrameBuffer(TiledMapType mapType, FrameBuffer *fbArr,
                                  Uint32 numOfFrameBuffers, Uint32 sizeLuma,
                                  Uint32 sizeChroma)
{
    Uint32 i;
    BOOL   yuv422Interleave = FALSE;
    BOOL   fieldFrame       = (BOOL)(mapType == LINEAR_FIELD_MAP);
    BOOL   cbcrInterleave   = (BOOL)(mapType == COMPRESSED_FRAME_MAP || fbArr[0].cbcrInterleave == TRUE);
    BOOL   reuseFb          = FALSE;

    if (mapType != COMPRESSED_FRAME_MAP)
    {
        switch (fbArr[0].format)
        {
        case FORMAT_YUYV:
        case FORMAT_YUYV_P10_16BIT_MSB:
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_YVYU:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
        case FORMAT_UYVY:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
            yuv422Interleave = TRUE;
            break;
        default:
            yuv422Interleave = FALSE;
            break;
        }
    }

    for (i = 0; i < numOfFrameBuffers; i++)
    {
        reuseFb = (fbArr[i].bufY != (PhysicalAddress)-1 && fbArr[i].bufCb != (PhysicalAddress)-1 && fbArr[i].bufCr != (PhysicalAddress)-1);
        if (reuseFb == FALSE)
        {
            if (yuv422Interleave == TRUE)
            {
                fbArr[i].bufCb = (PhysicalAddress)-1;
                fbArr[i].bufCr = (PhysicalAddress)-1;
            }
            else
            {
                if (fbArr[i].bufCb == (PhysicalAddress)-1)
                {
                    fbArr[i].bufCb = fbArr[i].bufY + (sizeLuma >> fieldFrame);
                    CVI_VC_TRACE("bufY = 0x" VCODEC_UINT64_FORMAT_HEX
                                 ", bufCb = 0x" VCODEC_UINT64_FORMAT_HEX "\n",
                                 fbArr[i].bufY, fbArr[i].bufCb);
                }
                if (fbArr[i].bufCr == (PhysicalAddress)-1)
                {
                    if (cbcrInterleave == TRUE)
                    {
                        fbArr[i].bufCr = (PhysicalAddress)-1;
                    }
                    else
                    {
                        fbArr[i].bufCr = fbArr[i].bufCb + (sizeChroma >> fieldFrame);
                    }
                }
            }
        }
    }

    return RETCODE_SUCCESS;
}

/* \brief   Allocate tiled framebuffer on GDI version 2.0 H/W
 */
RetCode AllocateTiledFrameBufferGdiV2(TiledMapType mapType, FrameBuffer *fbArr,
                                      Uint32 numOfFrameBuffers, Uint32 sizeLuma,
                                      Uint32 sizeChroma)
{
    Uint32 i;
    Uint32 fieldFrame;
    Uint32 sizeFb;
    BOOL   cbcrInterleave;

    sizeFb = sizeLuma + sizeChroma * 2;
    fieldFrame =
        (mapType == TILED_FIELD_V_MAP || mapType == TILED_FIELD_NO_BANK_MAP || mapType == LINEAR_FIELD_MAP);

    for (i = 0; i < numOfFrameBuffers; i++)
    {
        cbcrInterleave = fbArr[0].cbcrInterleave;

        if (fbArr[i].bufCb == (PhysicalAddress)-1)
            fbArr[i].bufCb = fbArr[i].bufY + (sizeLuma >> fieldFrame);

        if (fbArr[i].bufCr == (PhysicalAddress)-1)
            fbArr[i].bufCr = fbArr[i].bufCb + (sizeChroma >> fieldFrame);

        switch (mapType)
        {
        case TILED_FIELD_V_MAP:
        case TILED_FIELD_NO_BANK_MAP:
            fbArr[i].bufYBot  = fbArr[i].bufY + (sizeFb >> fieldFrame);
            fbArr[i].bufCbBot = fbArr[i].bufYBot + (sizeLuma >> fieldFrame);
            if (cbcrInterleave == FALSE)
            {
                fbArr[i].bufCrBot = fbArr[i].bufCbBot + (sizeChroma >> fieldFrame);
            }
            break;
        case TILED_FRAME_V_MAP:
        case TILED_FRAME_H_MAP:
        case TILED_MIXED_V_MAP:
        case TILED_FRAME_NO_BANK_MAP:
            fbArr[i].bufYBot  = fbArr[i].bufY;
            fbArr[i].bufCbBot = fbArr[i].bufCb;
            if (cbcrInterleave == FALSE)
            {
                fbArr[i].bufCrBot = fbArr[i].bufCr;
            }
            break;
        case TILED_FIELD_MB_RASTER_MAP:
            fbArr[i].bufYBot  = fbArr[i].bufY + (sizeLuma >> 1);
            fbArr[i].bufCbBot = fbArr[i].bufCb + sizeChroma;
            break;
        default:
            fbArr[i].bufYBot  = 0;
            fbArr[i].bufCbBot = 0;
            fbArr[i].bufCrBot = 0;
            break;
        }
    }

    return RETCODE_SUCCESS;
}

Int32 _ConfigSecAXICoda9_182x(Uint32 coreIdx, Int32 codecMode, SecAxiInfo *sa,
                              Uint32 width)
{
    vpu_buffer_t vb;
    int          offset;
    Uint32       MbNumX = ((width & 0xFFFF) + 15) / 16;
    int          free_size;

    if (vdi_get_sram_memory(coreIdx, &vb) < 0)
    {
        return 0;
    }

    if (!vb.size)
    {
        sa->bufSize               = 0;
        sa->u.coda9.useBitEnable  = 0;
        sa->u.coda9.useIpEnable   = 0;
        sa->u.coda9.useDbkYEnable = 0;
        sa->u.coda9.useDbkCEnable = 0;
        sa->u.coda9.useOvlEnable  = 0;
        sa->u.coda9.useBtpEnable  = 0;
        return 0;
    }

    CVI_VC_CFG("Get sram: addr = 0x" VCODEC_UINT64_FORMAT_HEX ", size = 0x%x\n",
               (Uint64)vb.phys_addr, vb.size);

    vb.phys_addr = (PhysicalAddress)vb.size;
    free_size    = vb.size;

    offset = 0;
    // BIT
    if (sa->u.coda9.useBitEnable)
    {
        sa->u.coda9.useBitEnable = 1;

        switch (codecMode)
        {
        case AVC_DEC:
            offset = offset + MbNumX * 144;
            break; // AVC
        case AVC_ENC: {
            offset = offset + MbNumX * 16;
        }
        break;
        default:
            offset = offset + MbNumX * 16;
            break; // MPEG-4, Divx3
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufBitUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufBitUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset= 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufBitUse, offset, free_size);
    }

    // Intra Prediction, ACDC
    if (sa->u.coda9.useIpEnable)
    {
        // sa->u.coda9.bufIpAcDcUse = vb.phys_addr + offset;
        sa->u.coda9.useIpEnable = 1;

        switch (codecMode)
        {
        case AVC_DEC:
            offset = offset + MbNumX * 32;
            break; // AVC
        case AVC_ENC:
            offset = offset + MbNumX * 64;
            break;
        default:
            offset = offset + MbNumX * 128;
            break; // MPEG-4, Divx3
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufIpAcDcUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufIpAcDcUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset = 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufIpAcDcUse, offset, free_size);
    }

    // Deblock Chroma
    if (sa->u.coda9.useDbkCEnable)
    {
        sa->u.coda9.useDbkCEnable = 1;
        switch (codecMode)
        {
        case AVC_DEC:
            offset = offset + MbNumX * 64;
            break; // AVC
        case AVC_ENC:
            offset = offset + MbNumX * 64;
            break;
        default:
            offset = offset + MbNumX * 64;
            break;
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufDbkCUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufDbkCUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset = 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufDbkCUse, offset, free_size);
    }

    // Deblock Luma
    if (sa->u.coda9.useDbkYEnable)
    {
        sa->u.coda9.useDbkYEnable = 1;

        switch (codecMode)
        {
        case AVC_DEC:
            offset = offset + MbNumX * 64;
            break; // AVC
        case AVC_ENC:
            offset = offset + MbNumX * 64;
            break;
        default:
            offset = offset + MbNumX * 128;
            break;
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufDbkYUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufDbkYUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset = 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufDbkYUse, offset, free_size);
    }

    // check the buffer address which is 256 byte is available.
    if (((offset + 255) & (~255)) > (int)vb.size)
    {
        VLOG(ERR, "%s:%d NOT ENOUGH SRAM: required(%d), sram(%d)\n", __FUNCTION__,
             __LINE__, offset, vb.size);
        sa->bufSize = 0;
        CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
        return 0;
    }

    // VC1 Bit-plane
    if (sa->u.coda9.useBtpEnable)
    {
        if (codecMode != VC1_DEC)
        {
            sa->u.coda9.useBtpEnable = 0;
        }
    }

    // VC1 Overlap
    if (sa->u.coda9.useOvlEnable)
    {
        if (codecMode != VC1_DEC)
        {
            sa->u.coda9.useOvlEnable = 0;
        }
    }

    sa->bufSize = offset;
    sa->bufBase = vb.phys_addr - offset;
    CVI_VC_CFG(
        "Calc sram used size=0x%x, sa->bufBase = 0x" VCODEC_UINT64_FORMAT_HEX
        "\n",
        offset, sa->bufBase);

    return 1;
}

Int32 ConfigSecAXICoda9(Uint32 coreIdx, Int32 codecMode, SecAxiInfo *sa,
                        Uint32 width, Uint32 profile, Uint32 interlace)
{
    vpu_buffer_t vb;
    int          offset;
    Uint32       MbNumX = ((width & 0xFFFF) + 15) >> 4;
    int          free_size;

    if ((codecMode == AVC_DEC) && (vdi_get_chip_version() == CHIP_ID_1821A || vdi_get_chip_version() == CHIP_ID_1822) && ((profile == 66) || (interlace == 0)))
    {
        return _ConfigSecAXICoda9_182x(coreIdx, codecMode, sa, width);
    }

    if (vdi_get_sram_memory(coreIdx, &vb) < 0)
    {
        return 0;
    }

    if (!vb.size)
    {
        sa->bufSize               = 0;
        sa->u.coda9.useBitEnable  = 0;
        sa->u.coda9.useIpEnable   = 0;
        sa->u.coda9.useDbkYEnable = 0;
        sa->u.coda9.useDbkCEnable = 0;
        sa->u.coda9.useOvlEnable  = 0;
        sa->u.coda9.useBtpEnable  = 0;
        return 0;
    }

    CVI_VC_CFG("Get sram: addr = 0x" VCODEC_UINT64_FORMAT_HEX ", size = 0x%x\n",
               (Uint64)vb.phys_addr, vb.size);

    vb.phys_addr = (PhysicalAddress)vb.size;
    free_size    = vb.size;

    offset = 0;
    // BIT
    if (sa->u.coda9.useBitEnable)
    {
        sa->u.coda9.useBitEnable = 1;

        switch (codecMode)
        {
        case AVC_DEC:
            offset = offset + MbNumX * 144;
            break; // AVC
        case AVC_ENC: {
            offset = offset + MbNumX * 16;
        }
        break;
        default:
            offset = offset + MbNumX * 16;
            break; // MPEG-4, Divx3
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufBitUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufBitUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset= 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufBitUse, offset, free_size);
    }

    // Intra Prediction, ACDC
    if (sa->u.coda9.useIpEnable)
    {
        // sa->u.coda9.bufIpAcDcUse = vb.phys_addr + offset;
        sa->u.coda9.useIpEnable = 1;

        switch (codecMode)
        {
        case AVC_DEC:
            offset = offset + MbNumX * 64;
            break; // AVC
        case RV_DEC:
            offset = offset + MbNumX * 64;
            break;
        case VC1_DEC:
            offset = offset + MbNumX * 128;
            break;
        case AVS_DEC:
            offset = offset + MbNumX * 64;
            break;
        case MP2_DEC:
            offset = offset + MbNumX * 0;
            break;
        case VPX_DEC:
            offset = offset + MbNumX * 64;
            break;
        case AVC_ENC:
            offset = offset + MbNumX * 64;
            break;
        case MP4_ENC:
            offset = offset + MbNumX * 128;
            break;
        default:
            offset = offset + MbNumX * 128;
            break; // MPEG-4, Divx3
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufIpAcDcUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufIpAcDcUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset = 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufIpAcDcUse, offset, free_size);
    }

    // Deblock Chroma
    if (sa->u.coda9.useDbkCEnable)
    {
        sa->u.coda9.useDbkCEnable = 1;
        switch (codecMode)
        {
        case AVC_DEC:
            offset = (profile == 66 /*AVC BP decoder*/) ? offset + (MbNumX * 64)
                                                        : offset + (MbNumX * 128);
            break; // AVC
        case RV_DEC:
            offset = offset + MbNumX * 128;
            break;
        case VC1_DEC:
            offset = profile == 2 ? offset + MbNumX * 256 : offset + MbNumX * 128;
            break;
        case AVS_DEC:
            offset = offset + MbNumX * 64;
            break;
        case MP2_DEC:
            offset = offset + MbNumX * 64;
            break;
        case VPX_DEC:
            offset = offset + MbNumX * 128;
            break;
        case MP4_DEC:
            offset = offset + MbNumX * 64;
            break;
        case AVC_ENC:
            offset = offset + MbNumX * 64;
            break;
        case MP4_ENC:
            offset = offset + MbNumX * 64;
            break;
        default:
            offset = offset + MbNumX * 64;
            break;
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufDbkCUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufDbkCUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset = 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufDbkCUse, offset, free_size);
    }

    // Deblock Luma
    if (sa->u.coda9.useDbkYEnable)
    {
        sa->u.coda9.useDbkYEnable = 1;

        switch (codecMode)
        {
        case AVC_DEC:
            offset = (profile == 66 /*AVC BP decoder*/) ? offset + (MbNumX * 64)
                                                        : offset + (MbNumX * 128);
            break; // AVC
        case RV_DEC:
            offset = offset + MbNumX * 128;
            break;
        case VC1_DEC:
            offset = profile == 2 ? offset + MbNumX * 256 : offset + MbNumX * 128;
            break;
        case AVS_DEC:
            offset = offset + MbNumX * 64;
            break;
        case MP2_DEC:
            offset = offset + MbNumX * 128;
            break;
        case VPX_DEC:
            offset = offset + MbNumX * 128;
            break;
        case MP4_DEC:
            offset = offset + MbNumX * 64;
            break;
        case AVC_ENC:
            offset = offset + MbNumX * 64;
            break;
        case MP4_ENC:
            offset = offset + MbNumX * 64;
            break;
        default:
            offset = offset + MbNumX * 128;
            break;
        }

        if (offset > free_size)
        {
            sa->bufSize = 0;
            CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
            return 0;
        }

        sa->u.coda9.bufDbkYUse = vb.phys_addr - offset;
        CVI_VC_CFG("bufDbkYUse = 0x" VCODEC_UINT64_FORMAT_HEX
                   ", offset = 0x%X, free_size = 0x%X\n",
                   (Uint64)sa->u.coda9.bufDbkYUse, offset, free_size);
    }

    // check the buffer address which is 256 byte is available.
    if (((offset + 255) & (~255)) > (int)vb.size)
    {
        VLOG(ERR, "%s:%d NOT ENOUGH SRAM: required(%d), sram(%d)\n", __FUNCTION__,
             __LINE__, offset, vb.size);
        sa->bufSize = 0;
        CVI_VC_ERR("offset (0x%X) > free_size (0x%X)\n", offset, free_size);
        return 0;
    }

    // VC1 Bit-plane
    if (sa->u.coda9.useBtpEnable)
    {
        if (codecMode != VC1_DEC)
        {
            sa->u.coda9.useBtpEnable = 0;
        } /* else {
            int oneBTP;

            offset = ((offset + 255) & ~255);
            sa->u.coda9.bufBtpUse = vb.phys_addr + offset;
            sa->u.coda9.useBtpEnable = 1;

            oneBTP = (((MbNumX + 15) / 16) * MbNumY + 1) * 2;
            oneBTP = (oneBTP % 256) ? ((oneBTP / 256) + 1) * 256 :
                                      oneBTP;

            offset = offset + oneBTP * 3;

            if (offset > (int)vb.size) {
                    sa->bufSize = 0;
                    return 0;
            }
    }*/
    }

    // VC1 Overlap
    if (sa->u.coda9.useOvlEnable)
    {
        if (codecMode != VC1_DEC)
        {
            sa->u.coda9.useOvlEnable = 0;
        } /* else {
            sa->u.coda9.bufOvlUse = vb.phys_addr + offset;
            sa->u.coda9.useOvlEnable = 1;

            offset = offset + MbNumX * 80;

            if (offset > (int)vb.size) {
                    sa->bufSize = 0;
                    return 0;
            }
    }*/
    }

    sa->bufSize = offset;
    sa->bufBase = vb.phys_addr - offset;
    CVI_VC_CFG(
        "Calc sram used size=0x%x, sa->bufBase = 0x" VCODEC_UINT64_FORMAT_HEX
        "\n",
        offset, sa->bufBase);

    return 1;
}

Int32 ConfigSecAXIWave(Uint32 coreIdx, Int32 codecMode, SecAxiInfo *sa,
                       Uint32 width, Uint32 height, Uint32 profile,
                       Uint32 levelIdc)
{
    vpu_buffer_t vb;
    int          offset;
    Uint32       size;
    Uint32       lumaSize;
    Uint32       chromaSize;
    Uint32       productId;

    UNREFERENCED_PARAMETER(codecMode);
    UNREFERENCED_PARAMETER(height);

    if (vdi_get_sram_memory(coreIdx, &vb) < 0) return 0;

    CVI_VC_CFG("Get sram: addr=0x" VCODEC_UINT64_FORMAT_HEX ", size=0x%x\n",
               (Uint64)vb.phys_addr, vb.size);

    productId = ProductVpuGetId(coreIdx);

    if (!vb.size)
    {
        sa->bufSize                 = 0;
        sa->u.wave4.useIpEnable     = 0;
        sa->u.wave4.useLfRowEnable  = 0;
        sa->u.wave4.useBitEnable    = 0;
        sa->u.wave4.useEncImdEnable = 0;
        sa->u.wave4.useEncLfEnable  = 0;
        sa->u.wave4.useEncRdoEnable = 0;
        return 0;
    }

    sa->bufBase = vb.phys_addr;
    offset      = 0;
    /* Intra Prediction */
    if (sa->u.wave4.useIpEnable == TRUE)
    {
        sa->u.wave4.bufIp = sa->bufBase + offset;

        CVI_VC_CFG("u.wave4.bufIp=0x" VCODEC_UINT64_FORMAT_HEX ", offset=%d\n",
                   (Uint64)sa->u.wave4.bufIp, offset);

        switch (productId)
        {
        case PRODUCT_ID_4102:
        case PRODUCT_ID_420:
        case PRODUCT_ID_420L:
        case PRODUCT_ID_510:
            lumaSize   = ((VPU_ALIGN16(width) * 10 + 127) >> 7) << 4;
            chromaSize = ((VPU_ALIGN16(width) * 10 + 127) >> 7) << 4;
            break;
        default:
            return 0;
        }

        offset = lumaSize + chromaSize;
        if (offset > (int)vb.size)
        {
            sa->bufSize = 0;
            return 0;
        }
    }

    /* Loopfilter row */
    if (sa->u.wave4.useLfRowEnable == TRUE)
    {
        sa->u.wave4.bufLfRow = sa->bufBase + offset;

        CVI_VC_CFG("u.wave4.bufLfRow=0x" VCODEC_UINT64_FORMAT_HEX ", offset=%d\n",
                   (Uint64)sa->u.wave4.bufLfRow, offset);

        if (codecMode == C7_VP9_DEC)
        {
            if (profile == VP9_PROFILE_2)
            {
                lumaSize   = VPU_ALIGN64(width) * 8 * 10 / 8; /* lumaLIne   : 8 */
                chromaSize = VPU_ALIGN64(width) * 8 * 10 / 8; /* chromaLine : 8 */
            }
            else
            {
                lumaSize   = VPU_ALIGN64(width) * 8; /* lumaLIne
                                                : 8 */
                chromaSize = VPU_ALIGN64(width) * 8; /* chromaLine
                                                : 8 */
            }
            offset += lumaSize + chromaSize;
        }
        else
        {
            Uint32 level = levelIdc / 30;
            if (level >= 5)
            {
                size = (VPU_ALIGN32(width) >> 1) * 13 + VPU_ALIGN64(width) * 4;
            }
            else
            {
                size = VPU_ALIGN64(width) * 13;
            }
            offset += size;
        }
        if (offset > (int)vb.size)
        {
            sa->bufSize = 0;
            return 0;
        }
    }

    if (sa->u.wave4.useBitEnable == TRUE)
    {
        sa->u.wave4.bufBit = sa->bufBase + offset;
        if (codecMode == C7_VP9_DEC)
        {
            size = (VPU_ALIGN64(width) >> 6) * (70 * 8);
        }
        else
        {
            size = 34 * 1024; /* Fixed size */
        }
        offset += size;
        if (offset > (int)vb.size)
        {
            sa->bufSize = 0;
            return 0;
        }
    }

    if (sa->u.wave4.useEncImdEnable == TRUE)
    {
        /* Main   profile(8bit) : Align32(picWidth)
     * Main10 profile(10bit): Align32(picWidth)
     */
        sa->u.wave4.bufImd = sa->bufBase + offset;
        CVI_VC_CFG("u.wave4.bufImd=0x" VCODEC_UINT64_FORMAT_HEX ", offset=%d\n",
                   (Uint64)sa->u.wave4.bufImd, offset);

        offset += VPU_ALIGN32(width);
        if (offset > (int)vb.size)
        {
            sa->bufSize = 0;
            return 0;
        }
    }

    if (sa->u.wave4.useEncLfEnable == TRUE)
    {
        /* Main   profile(8bit) :
     *              Luma   = Align64(picWidth) * 5
     *              Chroma = Align64(picWidth) * 3
     * Main10 profile(10bit) :
     *              Luma   = Align64(picWidth) * 7
     *              Chroma = Align64(picWidth) * 5
     */
        Uint32 luma   = (profile == HEVC_PROFILE_MAIN10 ? 7 : 5);
        Uint32 chroma = (profile == HEVC_PROFILE_MAIN10 ? 5 : 3);

        sa->u.wave4.bufLf = sa->bufBase + offset;
        CVI_VC_CFG("u.wave4.bufLf=0x" VCODEC_UINT64_FORMAT_HEX ", offset=%d\n",
                   (Uint64)sa->u.wave4.bufLf, offset);

        lumaSize    = VPU_ALIGN64(width) * luma;
        chromaSize  = VPU_ALIGN64(width) * chroma;
        offset     += lumaSize + chromaSize;

        if (offset > (int)vb.size)
        {
            sa->bufSize = 0;
            return 0;
        }
    }

    if (sa->u.wave4.useEncRdoEnable == TRUE)
    {
        switch (productId)
        {
        case PRODUCT_ID_520:
            sa->u.wave4.bufRdo  = sa->bufBase + offset;
            offset             += ((288 * VPU_ALIGN64(width)) >> 6);
            break;
        default:
            /* Main   profile(8bit) : (Align64(picWidth)/32) * 22*16
         * Main10 profile(10bit): (Align64(picWidth)/32) * 22*16
         */
            sa->u.wave4.bufRdo  = sa->bufBase + offset;
            offset             += (VPU_ALIGN64(width) >> 5) * 22 * 16;
        }

        CVI_VC_CFG("u.wave4.bufRdo=0x" VCODEC_UINT64_FORMAT_HEX ", offset=%d\n",
                   (Uint64)sa->u.wave4.bufRdo, offset);

        if (offset > (int)vb.size)
        {
            sa->bufSize = 0;
            return 0;
        }
    }

    sa->bufSize = offset;

    CVI_VC_CFG(
        "Calc sram used size=0x%x, sa->bufBase = 0x" VCODEC_UINT64_FORMAT_HEX
        "\n",
        offset, sa->bufBase);

    return 1;
}

static int SetTiledMapTypeV20(Uint32 coreIdx, TiledMapConfig *pMapCfg,
                              int mapType, int width, int interleave)
{
#define GEN_XY2AXI(INV, ZER, TBX, XY, BIT) \
    ((INV) << 7 | (ZER) << 6 | (TBX) << 5 | (XY) << 4 | (BIT))
#define GEN_CONFIG(A, B, C, D, E, F, G, H, I) \
    ((A) << 20 | (B) << 19 | (C) << 18 | (D) << 17 | (E) << 16 | (F) << 12 | (G) << 8 | (H) << 4 | (I))
#define X_SEL 0
#define Y_SEL 1

    const int luma_map = 0x40; // zero, inv = 1'b0, zero = 1'b1 , tbxor =
                               // 1'b0, xy = 1'b0, bit = 4'd0
    const int chro_map = 0x40; // zero, inv = 1'b0, zero = 1'b1 , tbxor =
                               // 1'b0, xy = 1'b0, bit = 4'd0
    int width_chr;
    int i;

    pMapCfg->mapType = mapType;

    for (i = 0; i < 32; i = i + 1)
    {
        pMapCfg->xy2axiLumMap[i] = luma_map;
        pMapCfg->xy2axiChrMap[i] = chro_map;
    }
    pMapCfg->xy2axiConfig = 0;

    width_chr = (interleave) ? width : (width >> 1);

    switch (mapType)
    {
    case LINEAR_FRAME_MAP:
    case LINEAR_FIELD_MAP:
        pMapCfg->xy2axiConfig = 0;
        //#ifndef PLATFORM_NON_OS
        return 1;
        //#endif
        break;
    case TILED_FRAME_V_MAP: {
        // luma
        pMapCfg->xy2axiLumMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiLumMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiLumMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiLumMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiLumMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiLumMap[8]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiLumMap[9]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiLumMap[10] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiLumMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 4);
        pMapCfg->xy2axiLumMap[12] = GEN_XY2AXI(0, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiLumMap[13] = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);

        if (width <= 512)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 1024)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 2048)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        { // 4K size
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        // chroma
        pMapCfg->xy2axiChrMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiChrMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiChrMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiChrMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiChrMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiChrMap[8]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiChrMap[9]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiChrMap[10] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiChrMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        pMapCfg->xy2axiChrMap[12] = GEN_XY2AXI(1, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiChrMap[13] = GEN_XY2AXI(1, 0, 0, Y_SEL, 4);

        if (width_chr <= 512)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 1024)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 2048)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        { // 4K size
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        // xy2axiConfig
        pMapCfg->xy2axiConfig = GEN_CONFIG(0, 0, 0, 1, 1, 15, 0, 15, 0);
        break;
    } // case TILED_FRAME_V_MAP
    case TILED_FRAME_H_MAP: {
        pMapCfg->xy2axiLumMap[3]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiLumMap[4]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiLumMap[5]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiLumMap[6]  = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiLumMap[7]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiLumMap[8]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiLumMap[9]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiLumMap[10] = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiLumMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 4);
        pMapCfg->xy2axiLumMap[12] = GEN_XY2AXI(0, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiLumMap[13] = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        if (width <= 512)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 1024)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 2048)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        { // 4K size
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        pMapCfg->xy2axiChrMap[3]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiChrMap[4]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiChrMap[5]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiChrMap[6]  = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiChrMap[7]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiChrMap[8]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiChrMap[9]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiChrMap[10] = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiChrMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        pMapCfg->xy2axiChrMap[12] = GEN_XY2AXI(1, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiChrMap[13] = GEN_XY2AXI(1, 0, 0, Y_SEL, 4);
        if (width_chr <= 512)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 1024)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 2048)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        { // 4K size
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        // xy2axiConfig
        pMapCfg->xy2axiConfig = GEN_CONFIG(0, 0, 0, 1, 0, 15, 15, 15, 15);
        break;
    } // case TILED_FRAME_H_MAP:
    case TILED_FIELD_V_MAP: {
        pMapCfg->xy2axiLumMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiLumMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiLumMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiLumMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiLumMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiLumMap[8]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiLumMap[9]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiLumMap[10] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiLumMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 4);
        pMapCfg->xy2axiLumMap[12] = GEN_XY2AXI(0, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiLumMap[13] = GEN_XY2AXI(0, 0, 1, Y_SEL, 5);
        if (width <= 512)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 1024)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 2048)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        { // 4K size
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        pMapCfg->xy2axiChrMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiChrMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiChrMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiChrMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiChrMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiChrMap[8]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiChrMap[9]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiChrMap[10] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiChrMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        pMapCfg->xy2axiChrMap[12] = GEN_XY2AXI(1, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiChrMap[13] = GEN_XY2AXI(1, 0, 1, Y_SEL, 4);
        if (width_chr <= 512)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 1024)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 2048)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        { // 4K size
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }

        // xy2axiConfig
        pMapCfg->xy2axiConfig = GEN_CONFIG(0, 1, 1, 1, 1, 15, 15, 15, 15);
        break;
    }
    case TILED_MIXED_V_MAP: {
        pMapCfg->xy2axiLumMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiLumMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiLumMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiLumMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiLumMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiLumMap[8]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiLumMap[9]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiLumMap[10] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiLumMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 4);
        pMapCfg->xy2axiLumMap[12] = GEN_XY2AXI(0, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiLumMap[13] = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        if (width <= 512)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 1024)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 2048)
        {
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        { // 4K size
            pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        pMapCfg->xy2axiChrMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiChrMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiChrMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiChrMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiChrMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiChrMap[8]  = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiChrMap[9]  = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiChrMap[10] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiChrMap[11] = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        pMapCfg->xy2axiChrMap[12] = GEN_XY2AXI(1, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiChrMap[13] = GEN_XY2AXI(1, 0, 0, Y_SEL, 4);
        if (width_chr <= 512)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 1024)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 2048)
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        {
            pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
            pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        // xy2axiConfig
        pMapCfg->xy2axiConfig = GEN_CONFIG(0, 0, 1, 1, 1, 7, 7, 7, 7);
        break;
    }
    case TILED_FRAME_MB_RASTER_MAP: {
        pMapCfg->xy2axiLumMap[3] = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiLumMap[4] = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiLumMap[5] = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiLumMap[6] = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiLumMap[7] = GEN_XY2AXI(0, 0, 0, X_SEL, 3);

        pMapCfg->xy2axiChrMap[3] = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiChrMap[4] = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiChrMap[5] = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiChrMap[6] = GEN_XY2AXI(0, 0, 0, X_SEL, 3);

        //-----------------------------------------------------------
        // mb_addr = mby*stride + mbx
        // mb_addr mapping:
        //   luma   : axi_addr[~:8] => axi_addr =
        //   {mb_addr[23:0],map_addr[7:0]} chroma : axi_addr[~:7] =>
        //   axi_addr = {mb_addr[23:0],map_addr[6:0]}
        //-----------------------------------------------------------

        // xy2axiConfig
        pMapCfg->xy2axiConfig = GEN_CONFIG(0, 0, 0, 1, 1, 15, 0, 7, 0);
        break;
    }
    case TILED_FIELD_MB_RASTER_MAP: {
        pMapCfg->xy2axiLumMap[3] = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiLumMap[4] = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiLumMap[5] = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiLumMap[6] = GEN_XY2AXI(0, 0, 0, X_SEL, 3);

        pMapCfg->xy2axiChrMap[3] = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiChrMap[4] = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiChrMap[5] = GEN_XY2AXI(0, 0, 0, X_SEL, 3);

        //-----------------------------------------------------------
        // mb_addr = mby*stride + mbx
        // mb_addr mapping:
        //   luma   : axi_addr[~:7] => axi_addr =
        //   {mb_addr[23:0],map_addr[6:0]} chroma : axi_addr[~:6] =>
        //   axi_addr = {mb_addr[23:0],map_addr[5:0]}
        //-----------------------------------------------------------

        // xy2axiConfig
        pMapCfg->xy2axiConfig = GEN_CONFIG(0, 1, 1, 1, 1, 7, 7, 3, 3);

        break;
    }
    case TILED_FRAME_NO_BANK_MAP:
    case TILED_FIELD_NO_BANK_MAP: {
        // luma
        pMapCfg->xy2axiLumMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiLumMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiLumMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiLumMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiLumMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiLumMap[8]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 4);
        pMapCfg->xy2axiLumMap[9]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        pMapCfg->xy2axiLumMap[10] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
        pMapCfg->xy2axiLumMap[11] = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiLumMap[12] = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiLumMap[13] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiLumMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiLumMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);
        if (width <= 512)
        {
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 1024)
        {
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width <= 2048)
        {
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        {
            pMapCfg->xy2axiLumMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiLumMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiLumMap[18] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiLumMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiLumMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiLumMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiLumMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiLumMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        // chroma
        pMapCfg->xy2axiChrMap[3]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 0);
        pMapCfg->xy2axiChrMap[4]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 1);
        pMapCfg->xy2axiChrMap[5]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 2);
        pMapCfg->xy2axiChrMap[6]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 3);
        pMapCfg->xy2axiChrMap[7]  = GEN_XY2AXI(0, 0, 0, X_SEL, 3);
        pMapCfg->xy2axiChrMap[8]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 4);
        pMapCfg->xy2axiChrMap[9]  = GEN_XY2AXI(0, 0, 0, Y_SEL, 5);
        pMapCfg->xy2axiChrMap[10] = GEN_XY2AXI(0, 0, 0, Y_SEL, 6);
        pMapCfg->xy2axiChrMap[11] = GEN_XY2AXI(0, 0, 0, X_SEL, 4);
        pMapCfg->xy2axiChrMap[12] = GEN_XY2AXI(0, 0, 0, X_SEL, 5);
        pMapCfg->xy2axiChrMap[13] = GEN_XY2AXI(0, 0, 0, X_SEL, 6);
        pMapCfg->xy2axiChrMap[14] = GEN_XY2AXI(0, 0, 0, X_SEL, 7);
        pMapCfg->xy2axiChrMap[15] = GEN_XY2AXI(0, 0, 0, X_SEL, 8);

        if (width_chr <= 512)
        {
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 1024)
        {
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else if (width_chr <= 2048)
        {
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }
        else
        {
            pMapCfg->xy2axiChrMap[16] = GEN_XY2AXI(0, 0, 0, X_SEL, 9);
            pMapCfg->xy2axiChrMap[17] = GEN_XY2AXI(0, 0, 0, X_SEL, 10);
            pMapCfg->xy2axiChrMap[18] = GEN_XY2AXI(0, 0, 0, X_SEL, 11);
            pMapCfg->xy2axiChrMap[19] = GEN_XY2AXI(0, 0, 0, Y_SEL, 7);
            pMapCfg->xy2axiChrMap[20] = GEN_XY2AXI(0, 0, 0, Y_SEL, 8);
            pMapCfg->xy2axiChrMap[21] = GEN_XY2AXI(0, 0, 0, Y_SEL, 9);
            pMapCfg->xy2axiChrMap[22] = GEN_XY2AXI(0, 0, 0, Y_SEL, 10);
            pMapCfg->xy2axiChrMap[23] = GEN_XY2AXI(0, 0, 0, Y_SEL, 11);
        }

        // xy2axiConfig
        if (mapType == TILED_FRAME_NO_BANK_MAP)
            pMapCfg->xy2axiConfig = GEN_CONFIG(0, 0, 0, 1, 1, 15, 0, 15, 0);
        else
            pMapCfg->xy2axiConfig = GEN_CONFIG(0, 1, 1, 1, 1, 15, 15, 15, 15);

        break;
    }
    default:
        return 0;
    }

    for (i = 0; i < 32; i++)
    { // xy2axiLumMap
        VpuWriteReg(coreIdx, GDI_XY2AXI_LUM_BIT00 + 4 * i,
                    pMapCfg->xy2axiLumMap[i]);
    }

    for (i = 0; i < 32; i++)
    { // xy2axiChrMap
        VpuWriteReg(coreIdx, GDI_XY2AXI_CHR_BIT00 + 4 * i,
                    pMapCfg->xy2axiChrMap[i]);
    }

    // xy2axiConfig
    VpuWriteReg(coreIdx, GDI_XY2AXI_CONFIG, pMapCfg->xy2axiConfig);

    // fast access for reading
    pMapCfg->tbSeparateMap = (pMapCfg->xy2axiConfig >> 19) & 0x1;
    pMapCfg->topBotSplit   = (pMapCfg->xy2axiConfig >> 18) & 0x1;
    pMapCfg->tiledMap      = (pMapCfg->xy2axiConfig >> 17) & 0x1;

    return 1;
}

int SetTiledMapType(Uint32 coreIdx, TiledMapConfig *pMapCfg, int mapType,
                    int stride, int interleave, DRAMConfig *pDramCfg)
{
    int ret;
    UNREFERENCED_PARAMETER(pDramCfg);

    switch (pMapCfg->productId)
    {
    case PRODUCT_ID_980:
        ret = SetTiledMapTypeV20(coreIdx, pMapCfg, mapType, stride, interleave);
        break;
    case PRODUCT_ID_420:
    case PRODUCT_ID_420L:
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}

Int32 CalcStride(Uint32 width, Uint32 height, FrameBufferFormat format,
                 BOOL cbcrInterleave, TiledMapType mapType, BOOL isVP9,
                 BOOL isDec)
{
    Uint32 lumaStride   = 0;
    Uint32 chromaStride = 0;

    lumaStride = VPU_ALIGN32(width);

    if (height > width)
    {
        if ((mapType >= TILED_FRAME_V_MAP && mapType <= TILED_MIXED_V_MAP) || mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP)
            width = VPU_ALIGN16(height); // TiledMap constraints
    }
    if (mapType == LINEAR_FRAME_MAP || mapType == LINEAR_FIELD_MAP)
    {
        Uint32 twice = 0;

        twice = cbcrInterleave == TRUE ? 2 : 1;
        switch (format)
        {
        case FORMAT_420:
            /* nothing to do */
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_420_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_LSB:
            lumaStride = VPU_ALIGN32(width) * 2;
            break;
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
            if (isVP9 == TRUE)
            {
                lumaStride   = VPU_ALIGN32(((width + 11) / 12) * 16);
                chromaStride = (((width >> 1) + 11) * twice / 12) * 16;
            }
            else
            {
                width        = VPU_ALIGN32(width);
                lumaStride   = ((VPU_ALIGN16(width) + 11) / 12) * 16;
                chromaStride = ((VPU_ALIGN16((width >> 1)) + 11) * twice / 12) * 16;
                // if ( isWAVE410 == TRUE ) {
                if ((chromaStride * 2) > lumaStride)
                {
                    lumaStride = chromaStride * 2;
                    VLOG(ERR, "double chromaStride size is bigger than lumaStride\n");
                }
                //}
            }
            if (cbcrInterleave == TRUE)
            {
                lumaStride = MAX(lumaStride, chromaStride);
            }
            break;
        case FORMAT_422:
            /* nothing to do */
            break;
        case FORMAT_YUYV: // 4:2:2 8bit packed
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
            lumaStride = VPU_ALIGN32(width) * 2;
            break;
        case FORMAT_YUYV_P10_16BIT_MSB: // 4:2:2 10bit packed
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
            lumaStride = VPU_ALIGN32(width) * 4;
            break;
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
            lumaStride = VPU_ALIGN32(width * 2) * 2;
            break;
        default:
            break;
        }
    }
    else if (mapType == COMPRESSED_FRAME_MAP)
    {
        switch (format)
        {
        case FORMAT_420:
        case FORMAT_422:
        case FORMAT_YUYV:
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_420_P10_16BIT_MSB:
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_LSB:
        case FORMAT_422_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
        case FORMAT_YUYV_P10_16BIT_MSB:
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
            lumaStride = VPU_ALIGN32(VPU_ALIGN16(width) * 5) >> 2;
            lumaStride = VPU_ALIGN32(lumaStride);
            break;
        default:
            return -1;
        }
    }
    else if (mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP)
    {
        lumaStride = (width > 4096)
                         ? 8192
                     : (width > 2048)
                         ? 4096
                     : (width > 1024) ? 2048
                     : (width > 512)  ? 1024
                                      : 512;
    }
    else if (mapType == TILED_FRAME_MB_RASTER_MAP || mapType == TILED_FIELD_MB_RASTER_MAP)
    {
        lumaStride = VPU_ALIGN64(width);
    }
    else
    {
        width = (width < height) ? height : width;

        lumaStride = (width > 4096)
                         ? 8192
                     : (width > 2048)
                         ? 4096
                     : (width > 1024) ? 2048
                     : (width > 512)  ? 1024
                                      : 512;
    }

#ifdef ARCH_CV183X
    UNREFERENCED_PARAMETER(isDec);
#else
    if ((format == FORMAT_420) && (isDec == TRUE))
    {
        if (cbcrInterleave == TRUE)
        {
            lumaStride = VPU_ALIGN64(lumaStride);
        }
        else
        {
            lumaStride = VPU_ALIGN128(lumaStride);
        }
    }
#endif

    return lumaStride;
}

int LevelCalculation(int MbNumX, int MbNumY, int frameRateInfo,
                     int interlaceFlag, int BitRate, int SliceNum)
{
    int mbps;
    int frameRateDiv, frameRateRes, frameRate;
    int mbPicNum = (MbNumX * MbNumY);
    int mbFrmNum;
    int MaxSliceNum;

    int LevelIdc = 0;
    int i, maxMbs;

    if (interlaceFlag)
    {
        mbFrmNum  = mbPicNum * 2;
        MbNumY   *= 2;
    }
    else
        mbFrmNum = mbPicNum;

    frameRateDiv = (frameRateInfo >> 16) + 1;
    frameRateRes = frameRateInfo & 0xFFFF;
    frameRate    = math_div(frameRateRes, frameRateDiv);
    mbps         = mbFrmNum * frameRate;

    for (i = 0; i < MAX_LAVEL_IDX; i++)
    {
        maxMbs = g_anLevelMaxMbs[i];
        if (mbps <= g_anLevelMaxMBPS[i] && mbFrmNum <= g_anLevelMaxFS[i] && MbNumX <= maxMbs && MbNumY <= maxMbs && BitRate <= g_anLevelMaxBR[i])
        {
            LevelIdc = g_anLevel[i];
            break;
        }
    }

    if (i == MAX_LAVEL_IDX) i = MAX_LAVEL_IDX - 1;

    if (SliceNum)
    {
        SliceNum = math_div(mbPicNum, SliceNum);

        if (g_anLevelSliceRate[i])
        {
            MaxSliceNum = math_div(
                MAX(mbPicNum, g_anLevelMaxMBPS[i] / (172 / (1 + interlaceFlag))),
                g_anLevelSliceRate[i]);

            if (SliceNum > MaxSliceNum) return -1;
        }
    }

    return LevelIdc;
}

Int32 CalcLumaSize(Int32 productId, Int32 stride, Int32 height, BOOL cbcrIntl,
                   TiledMapType mapType, DRAMConfig *pDramCfg)
{
    Int32 unit_size_hor_lum, unit_size_ver_lum, size_dpb_lum, field_map,
        size_dpb_lum_4k;
    UNREFERENCED_PARAMETER(cbcrIntl);
    UNREFERENCED_PARAM(productId);
    UNREFERENCED_PARAM(pDramCfg);

    if (mapType == TILED_FIELD_V_MAP || mapType == TILED_FIELD_NO_BANK_MAP || mapType == LINEAR_FIELD_MAP)
    {
        field_map = 1;
    }
    else
    {
        field_map = 0;
    }

    if (mapType == LINEAR_FRAME_MAP || mapType == LINEAR_FIELD_MAP)
    {
        size_dpb_lum = stride * height;
    }
    else if (mapType == COMPRESSED_FRAME_MAP)
    {
        size_dpb_lum = stride * height;
    }
    else if (mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP)
    {
        unit_size_hor_lum = stride;
        unit_size_ver_lum = ((((height >> field_map) + 127) >> 7) << 7);
        // unit vertical size is 128 pixel
        // (8MB)
        size_dpb_lum = unit_size_hor_lum * (unit_size_ver_lum << field_map);
    }
    else if (mapType == TILED_FRAME_MB_RASTER_MAP || mapType == TILED_FIELD_MB_RASTER_MAP)
    {
        {
            size_dpb_lum    = stride * (height >> field_map);
            size_dpb_lum_4k = ((size_dpb_lum + (16384 - 1)) >> 14) << 14;
            size_dpb_lum    = size_dpb_lum_4k << field_map;
        }
    }
    else
    {
        { // productId != 960
            unit_size_hor_lum = stride;
            unit_size_ver_lum = (((height >> field_map) + 63) >> 6) << 6;
            // unit vertical size is 64 pixel (4MB)
            size_dpb_lum = unit_size_hor_lum * (unit_size_ver_lum << field_map);
        }
    }

    if (vdi_get_chip_version() == CHIP_ID_1822) return size_dpb_lum;

    // 4k-aligned for frame buffer addr (VPSS usage)
    return VPU_ALIGN4096(size_dpb_lum);
}

Int32 CalcChromaSize(Int32 productId, Int32 stride, Int32 height,
                     FrameBufferFormat format, BOOL cbcrIntl,
                     TiledMapType mapType, DRAMConfig *pDramCfg)
{
    Int32 chr_size_y, chr_size_x;
    Int32 chr_vscale, chr_hscale;
    Int32 unit_size_hor_chr, unit_size_ver_chr;
    Int32 size_dpb_chr, size_dpb_chr_4k;
    Int32 field_map;

    UNREFERENCED_PARAM(productId);
    UNREFERENCED_PARAM(pDramCfg);

    unit_size_hor_chr = 0;
    unit_size_ver_chr = 0;

    chr_hscale = 1;
    chr_vscale = 1;

    switch (format)
    {
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_420:
        chr_hscale = 2;
        chr_vscale = 2;
        break;
    case FORMAT_224:
        chr_vscale = 2;
        break;
    case FORMAT_422:
    case FORMAT_422_P10_16BIT_LSB:
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_32BIT_LSB:
    case FORMAT_422_P10_32BIT_MSB:
        chr_hscale = 2;
        break;
    case FORMAT_444:
    case FORMAT_400:
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
    case FORMAT_YUYV_P10_16BIT_MSB: // 4:2:2 10bit packed
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        break;
    default:
        return 0;
    }

    if (mapType == TILED_FIELD_V_MAP || mapType == TILED_FIELD_NO_BANK_MAP || mapType == LINEAR_FIELD_MAP)
    {
        field_map = 1;
    }
    else
    {
        field_map = 0;
    }

    if (mapType == LINEAR_FRAME_MAP || mapType == LINEAR_FIELD_MAP)
    {
        switch (format)
        {
        case FORMAT_420:
            unit_size_hor_chr = stride >> 1;
            unit_size_ver_chr = height >> 1;
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_420_P10_16BIT_MSB:
            unit_size_hor_chr = stride >> 1;
            unit_size_ver_chr = height >> 1;
            break;
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
            unit_size_hor_chr = VPU_ALIGN16((stride >> 1));
            unit_size_ver_chr = height >> 1;
            break;
        case FORMAT_422:
        case FORMAT_422_P10_16BIT_LSB:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
        case FORMAT_422_P10_32BIT_MSB:
            unit_size_hor_chr = VPU_ALIGN32((stride >> 1));
            unit_size_ver_chr = height;
            break;
        case FORMAT_YUYV:
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
        case FORMAT_YUYV_P10_16BIT_MSB: // 4:2:2 10bit packed
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
            unit_size_hor_chr = 0;
            unit_size_ver_chr = 0;
            break;
        default:
            break;
        }
        size_dpb_chr =
            (format == FORMAT_400) ? 0 : unit_size_ver_chr * unit_size_hor_chr;
    }
    else if (mapType == COMPRESSED_FRAME_MAP)
    {
        switch (format)
        {
        case FORMAT_420:
        case FORMAT_YUYV: // 4:2:2 8bit packed
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
            size_dpb_chr = VPU_ALIGN16((stride >> 1)) * height;
            break;
        default:
            /* 10bit */
            stride       = VPU_ALIGN64((stride >> 1)) + 12; /* FIXME: need
                                                  width
                                                  information */
            size_dpb_chr = VPU_ALIGN32(stride) * VPU_ALIGN4(height);
            break;
        }
        size_dpb_chr = size_dpb_chr >> 1;
    }
    else if (mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP)
    {
        chr_size_y = (height >> field_map) / chr_hscale;
        chr_size_x = stride / chr_vscale;

        unit_size_hor_chr = (chr_size_x > 4096)
                                ? 8192
                            : (chr_size_x > 2048)
                                ? 4096
                            : (chr_size_x > 1024)
                                ? 2048
                            : (chr_size_x > 512) ? 1024
                                                 : 512;
        unit_size_ver_chr = ((chr_size_y + 127) >> 7) << 7;
        // unit vertical size is 128 pixel (8MB)

        size_dpb_chr = (format == FORMAT_400)
                           ? 0
                           : (unit_size_hor_chr * (unit_size_ver_chr << field_map));
    }
    else if (mapType == TILED_FRAME_MB_RASTER_MAP || mapType == TILED_FIELD_MB_RASTER_MAP)
    {
        {
            size_dpb_chr =
                (format == FORMAT_400)
                    ? 0
                    : ((stride * (height >> field_map)) / (chr_hscale * chr_vscale));
            size_dpb_chr_4k = ((size_dpb_chr + (16384 - 1)) >> 14) << 14;
            size_dpb_chr    = size_dpb_chr_4k << field_map;
        }
    }
    else
    {
        { // productId != 960
            chr_size_y = (height >> field_map) / chr_hscale;
            chr_size_x = cbcrIntl == TRUE ? stride : stride / chr_vscale;

            unit_size_hor_chr = (chr_size_x > 4096)
                                    ? 8192
                                : (chr_size_x > 2048)
                                    ? 4096
                                : (chr_size_x > 1024)
                                    ? 2048
                                : (chr_size_x > 512) ? 1024
                                                     : 512;
            unit_size_ver_chr = ((chr_size_y + 63) >> 6) << 6;
            // unit vertical size is 64 pixel (4MB)

            size_dpb_chr = (format == FORMAT_400)
                               ? 0
                               : unit_size_hor_chr * (unit_size_ver_chr << field_map);
            if (cbcrIntl == TRUE) size_dpb_chr >>= 1;
        }
    }

    if (vdi_get_chip_version() == CHIP_ID_1822) return size_dpb_chr;

    // 4k-aligned for frame buffer addr (VPSS usage)
    return VPU_ALIGN4096(size_dpb_chr);
}
