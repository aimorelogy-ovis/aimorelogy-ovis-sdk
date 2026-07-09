//------------------------------------------------------------------------------
// File: $Id$
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#include "main_helper.h"
#include "unistd.h"
#include "vdi_osal.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
/*******************************************************************************
 * REPORT                                                                      *
 *******************************************************************************/
#define USER_DATA_INFO_OFFSET (8 * 17)
#define FN_PIC_INFO           "dec_pic_disp_info.log"
#define FN_SEQ_INFO           "dec_seq_disp_info.log"
#define FN_PIC_TYPE           "dec_pic_type.log"
#define FN_USER_DATA          "dec_user_data.log"
#define FN_SEQ_USER_DATA      "dec_seq_user_data.log"

#ifndef SET_DEFAULT_CFG
#define SET_DEFAULT_CFG 0 // avoid warning
#endif
#define DEFAULT_QP 34

// VC1 specific
enum
{
    BDU_SEQUENCE_END               = 0x0A,
    BDU_SLICE                      = 0x0B,
    BDU_FIELD                      = 0x0C,
    BDU_FRAME                      = 0x0D,
    BDU_ENTRYPOINT_HEADER          = 0x0E,
    BDU_SEQUENCE_HEADER            = 0x0F,
    BDU_SLICE_LEVEL_USER_DATA      = 0x1B,
    BDU_FIELD_LEVEL_USER_DATA      = 0x1C,
    BDU_FRAME_LEVEL_USER_DATA      = 0x1D,
    BDU_ENTRYPOINT_LEVEL_USER_DATA = 0x1E,
    BDU_SEQUENCE_LEVEL_USER_DATA   = 0x1F
};

// AVC specific - SEI
enum
{
    SEI_REGISTERED_ITUTT35_USERDATA = 0x04,
    SEI_UNREGISTERED_USERDATA       = 0x05,
    SEI_MVC_SCALABLE_NESTING        = 0x25
};

extern void set_gop_info(ENC_CFG *pEncCfg);

static void setAvcEncDefaultParam(EncOpenParam  *pEncOP,
                                  TestEncConfig *pEncConfig);
static void rcLibSetParam(EncOpenParam *pEncOP, TestEncConfig *pEncConfig);
static void setHevcEncDefaultParam(EncOpenParam  *pEncOP,
                                   TestEncConfig *pEncConfig);
#if SET_DEFAULT_CFG
static void setDefaultCfg(TestEncConfig *pEncConfig, ENC_CFG *pCfg);
#endif

#define DEFAULT_ENC_OUTPUT_NUM 30

/*******************************************************************************
 * FUNCTIONS RELATED TO CPB                                                    *
 *******************************************************************************/
/*lint -restore */

int setWaveEncOpenParam(EncOpenParam *pEncOP, TestEncConfig *pEncConfig,
                        ENC_CFG *pCfg)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;
    Int32      i      = 0;
    Int32      srcWidth;
    Int32      srcHeight;
    Int32      outputNum;

    EncHevcParam *param = &pEncOP->EncStdParam.hevcParam;

    srcWidth =
        (pEncConfig->picWidth > 0) ? pEncConfig->picWidth : pCfg->hevcCfg.picX;
    srcHeight =
        (pEncConfig->picHeight > 0) ? pEncConfig->picHeight : pCfg->hevcCfg.picY;
    outputNum       = (pEncConfig->outNum > 0) ? pEncConfig->outNum : pCfg->NumFrame;
    pEncOP->bitRate = (pEncConfig->kbps > 0) ? pEncConfig->kbps : pCfg->RcBitRate;

    pEncOP->rcMode = pEncConfig->rcMode;

    pEncOP->statTime = 2;
    pEncOP->changePos =
        (pEncConfig->changePos > 50 && pEncConfig->changePos < 100)
            ? pEncConfig->changePos
            : 90;
    pEncOP->frmLostOpen   = pEncConfig->frmLostOpen;
    pEncOP->frmLostBpsThr = pEncOP->bitRate * 1100;
    pEncOP->encFrmGaps    = 1;
    pEncConfig->outNum    = outputNum;
    pEncOP->picWidth      = srcWidth;
    pEncOP->picHeight     = srcHeight;
    pEncOP->frameRateInfo = pCfg->hevcCfg.frameRate;
    param->level          = 0;
    param->tier           = 0;
    pEncOP->srcBitDepth   = pCfg->SrcBitDepth;

    if (pCfg->hevcCfg.internalBitDepth == 0)
        param->internalBitDepth = pCfg->SrcBitDepth;
    else
        param->internalBitDepth = pCfg->hevcCfg.internalBitDepth;

    if (param->internalBitDepth > 8)
        param->profile = HEVC_PROFILE_MAIN10;
    else
        param->profile = HEVC_PROFILE_MAIN;

    param->chromaFormatIdc    = 0;
    param->losslessEnable     = pCfg->hevcCfg.losslessEnable;
    param->constIntraPredFlag = pCfg->hevcCfg.constIntraPredFlag;

    if (pCfg->hevcCfg.useAsLongtermPeriod > 0 || pCfg->hevcCfg.refLongtermPeriod > 0)
        param->useLongTerm = 1;
    else
        param->useLongTerm = 0;

    /* for CMD_ENC_SEQ_GOP_PARAM */
    param->gopPresetIdx =
        (pEncConfig->tempLayer == 2)
            ? 17
        : (pEncConfig->tempLayer == 3) ? 19
                                       : pCfg->hevcCfg.gopPresetIdx;

    /* for CMD_ENC_SEQ_INTRA_PARAM */
    param->decodingRefreshType = (pEncConfig->decodingRefreshType)
                                     ? pEncConfig->decodingRefreshType
                                     : pCfg->hevcCfg.decodingRefreshType;
    param->intraPeriod =
        (pEncConfig->gopSize) ? pEncConfig->gopSize : pCfg->hevcCfg.intraPeriod;

    pEncOP->gopSize              = param->intraPeriod;
    param->intraQP               = pCfg->hevcCfg.intraQP;
    param->forcedIdrHeaderEnable = pCfg->hevcCfg.forcedIdrHeaderEnable;
    CVI_VC_TRACE("gopSize = %d, intraPeriod = %d\n", pEncOP->gopSize,
                 pCfg->hevcCfg.intraPeriod);

    /* for CMD_ENC_SEQ_CONF_WIN_TOP_BOT/LEFT_RIGHT */
    param->confWinTop   = pCfg->hevcCfg.confWinTop;
    param->confWinBot   = pCfg->hevcCfg.confWinBot;
    param->confWinLeft  = pCfg->hevcCfg.confWinLeft;
    param->confWinRight = pCfg->hevcCfg.confWinRight;
    CVI_VC_CFG("confWinBot = %d, confWinRight = %d\n", param->confWinBot,
               param->confWinRight);

    /* for CMD_ENC_SEQ_INDEPENDENT_SLICE */
    param->independSliceMode    = (pEncConfig->independSliceMode)
                                      ? pEncConfig->independSliceMode
                                      : pCfg->hevcCfg.independSliceMode;
    param->independSliceModeArg = (pEncConfig->independSliceModeArg)
                                      ? (pEncConfig->independSliceModeArg)
                                      : pCfg->hevcCfg.independSliceModeArg;

    CVI_VC_CFG("independSliceMode = %d, independSliceModeArg = %d\n",
               param->independSliceMode, param->independSliceModeArg);

    /* for CMD_ENC_SEQ_DEPENDENT_SLICE */
    param->dependSliceMode    = pCfg->hevcCfg.dependSliceMode;
    param->dependSliceModeArg = pCfg->hevcCfg.dependSliceModeArg;

    /* for CMD_ENC_SEQ_INTRA_REFRESH_PARAM */
    param->intraRefreshMode     = pCfg->hevcCfg.intraRefreshMode;
    param->intraRefreshArg      = pCfg->hevcCfg.intraRefreshArg;
    param->useRecommendEncParam = pCfg->hevcCfg.useRecommendEncParam;

    /* for CMD_ENC_PARAM */
    param->scalingListEnable = pCfg->hevcCfg.scalingListEnable;
    param->cuSizeMode        = pCfg->hevcCfg.cuSizeMode;
    param->tmvpEnable        = pCfg->hevcCfg.tmvpEnable;
    param->wppEnable         = pCfg->hevcCfg.wppenable;
    param->maxNumMerge       = pCfg->hevcCfg.maxNumMerge;

    if (pEncConfig->dynamicMergeEnable)
    {
        param->dynamicMerge8x8Enable   = pEncConfig->dynamicMergeEnable;
        param->dynamicMerge16x16Enable = pEncConfig->dynamicMergeEnable;
        param->dynamicMerge32x32Enable = pEncConfig->dynamicMergeEnable;
    }
    else
    {
        param->dynamicMerge8x8Enable   = pCfg->hevcCfg.dynamicMerge8x8Enable;
        param->dynamicMerge16x16Enable = pCfg->hevcCfg.dynamicMerge16x16Enable;
        param->dynamicMerge32x32Enable = pCfg->hevcCfg.dynamicMerge32x32Enable;
    }

    CVI_VC_CFG(
        "dynamicMerge8x8Enable = %d, dynamicMerge16x16Enable = %d, "
        "dynamicMerge32x32Enable = %d\n",
        param->dynamicMerge8x8Enable, param->dynamicMerge16x16Enable,
        param->dynamicMerge32x32Enable);

    param->disableDeblk               = pCfg->hevcCfg.disableDeblk;
    param->lfCrossSliceBoundaryEnable = pCfg->hevcCfg.lfCrossSliceBoundaryEnable;
    param->betaOffsetDiv2             = pCfg->hevcCfg.betaOffsetDiv2;
    param->tcOffsetDiv2               = pCfg->hevcCfg.tcOffsetDiv2;
    param->skipIntraTrans             = pCfg->hevcCfg.skipIntraTrans;
    param->saoEnable                  = pCfg->hevcCfg.saoEnable;
    param->intraInInterSliceEnable    = pCfg->hevcCfg.intraInInterSliceEnable;
    param->intraNxNEnable             = pCfg->hevcCfg.intraNxNEnable;

    /* for CMD_ENC_RC_PARAM */
    pEncOP->rcEnable =
        (pEncConfig->RcEnable >= 0) ? pEncConfig->RcEnable : pCfg->RcEnable;
    CVI_VC_CFG("rcEnable = %d, bitRate = %d\n", pEncOP->rcEnable,
               pEncOP->bitRate);

    pEncOP->initialDelay    = pCfg->RcInitDelay;
    param->cuLevelRCEnable  = pCfg->hevcCfg.cuLevelRCEnable;
    param->hvsQPEnable      = pCfg->hevcCfg.hvsQPEnable;
    param->hvsQpScaleEnable = pCfg->hevcCfg.hvsQpScaleEnable;
    param->hvsQpScale       = pCfg->hevcCfg.hvsQpScale;

    param->ctuOptParam.roiDeltaQp = pCfg->hevcCfg.ctuOptParam.roiDeltaQp;
    param->intraQpOffset          = pCfg->hevcCfg.intraQpOffset;
    CVI_VC_CFG("intraQpOffset = %d\n", param->intraQpOffset);

    param->initBufLevelx8 = pCfg->hevcCfg.initBufLevelx8;
    param->bitAllocMode   = pCfg->hevcCfg.bitAllocMode;
    for (i = 0; i < MAX_GOP_NUM; i++)
    {
        param->fixedBitRatio[i] = pCfg->hevcCfg.fixedBitRatio[i];
    }

    /* for CMD_ENC_RC_MIN_MAX_QP */
    pEncOP->userQpMinP                = pCfg->hevcCfg.minQp;
    pEncOP->userQpMaxP                = pCfg->hevcCfg.maxQp;
    param->maxDeltaQp                 = pCfg->hevcCfg.maxDeltaQp;
    param->transRate                  = pCfg->hevcCfg.transRate;
    param->gopParam.enTemporalLayerQp = pCfg->hevcCfg.gopParam.enTemporalLayerQp;
    param->gopParam.tidQp0            = pCfg->hevcCfg.gopParam.tidQp0;
    param->gopParam.tidQp1            = pCfg->hevcCfg.gopParam.tidQp1;
    param->gopParam.tidQp2            = pCfg->hevcCfg.gopParam.tidQp2;
    param->gopParam.tidPeriod0 =
        (pEncConfig->tempLayer == 2) ? 2 : (pEncConfig->tempLayer == 3) ? 4
                                                                        : 1;
    /* for CMD_ENC_CUSTOM_GOP_PARAM */
    param->gopParam.customGopSize = pCfg->hevcCfg.gopParam.customGopSize;
    param->gopParam.useDeriveLambdaWeight =
        (pEncConfig->useDeriveLambdaWeight)
            ? pEncConfig->useDeriveLambdaWeight
            : pCfg->hevcCfg.gopParam.useDeriveLambdaWeight;

    for (i = 0; i < param->gopParam.customGopSize; i++)
    {
        param->gopParam.picParam[i].picType =
            pCfg->hevcCfg.gopParam.picParam[i].picType;
        param->gopParam.picParam[i].pocOffset =
            pCfg->hevcCfg.gopParam.picParam[i].pocOffset;
        param->gopParam.picParam[i].picQp =
            pCfg->hevcCfg.gopParam.picParam[i].picQp;
        param->gopParam.picParam[i].refPocL0 =
            pCfg->hevcCfg.gopParam.picParam[i].refPocL0;
        param->gopParam.picParam[i].refPocL1 =
            pCfg->hevcCfg.gopParam.picParam[i].refPocL1;
        param->gopParam.picParam[i].temporalId =
            pCfg->hevcCfg.gopParam.picParam[i].temporalId;
        param->gopParam.gopPicLambda[i] = pCfg->hevcCfg.gopParam.gopPicLambda[i];
    }

    param->ctuOptParam.roiEnable     = pCfg->hevcCfg.ctuOptParam.roiEnable;
    param->ctuOptParam.ctuQpEnable   = pCfg->hevcCfg.ctuOptParam.ctuQpEnable;
    param->ctuOptParam.ctuModeEnable = pCfg->hevcCfg.ctuOptParam.ctuModeEnable;

    // VPS & VUI
    param->numUnitsInTick     = pCfg->hevcCfg.numUnitsInTick;
    param->timeScale          = pCfg->hevcCfg.timeScale;
    param->numTicksPocDiffOne = pCfg->hevcCfg.numTicksPocDiffOne;

    param->vuiParam.vuiParamFlags     = pCfg->hevcCfg.vuiParam.vuiParamFlags;
    param->vuiParam.vuiAspectRatioIdc = pCfg->hevcCfg.vuiParam.vuiAspectRatioIdc;
    param->vuiParam.vuiSarSize        = pCfg->hevcCfg.vuiParam.vuiSarSize;
    param->vuiParam.vuiOverScanAppropriate =
        pCfg->hevcCfg.vuiParam.vuiOverScanAppropriate;
    param->vuiParam.videoSignal = pCfg->hevcCfg.vuiParam.videoSignal;
    param->vuiParam.vuiChromaSampleLoc =
        pCfg->hevcCfg.vuiParam.vuiChromaSampleLoc;
    param->vuiParam.vuiDispWinLeftRight =
        pCfg->hevcCfg.vuiParam.vuiDispWinLeftRight;
    param->vuiParam.vuiDispWinTopBottom =
        pCfg->hevcCfg.vuiParam.vuiDispWinTopBottom;

    pEncOP->encodeVuiRbsp      = pCfg->hevcCfg.vuiDataEnable;
    pEncOP->vuiRbspDataSize    = pCfg->hevcCfg.vuiDataSize;
    pEncOP->encodeHrdRbspInVPS = pCfg->hevcCfg.hrdInVPS;
    pEncOP->hrdRbspDataSize    = pCfg->hevcCfg.hrdDataSize;
    pEncOP->encodeHrdRbspInVUI = pCfg->hevcCfg.hrdInVUI;

    param->chromaCbQpOffset = pCfg->hevcCfg.chromaCbQpOffset;
    param->chromaCrQpOffset = pCfg->hevcCfg.chromaCrQpOffset;
    param->initialRcQp      = (pCviEc->firstFrmstartQp > -1)
                                  ? pCviEc->firstFrmstartQp
                                  : pCfg->hevcCfg.initialRcQp;
    CVI_VC_CFG("firstFrmstartQp = %d, initialRcQp = %d\n",
               pCviEc->firstFrmstartQp, pCfg->hevcCfg.initialRcQp);

    param->nrYEnable        = pCfg->hevcCfg.nrYEnable;
    param->nrCbEnable       = pCfg->hevcCfg.nrCbEnable;
    param->nrCrEnable       = pCfg->hevcCfg.nrCrEnable;
    param->nrNoiseEstEnable = pCfg->hevcCfg.nrNoiseEstEnable;
    param->nrNoiseSigmaY    = pCfg->hevcCfg.nrNoiseSigmaY;
    param->nrNoiseSigmaCb   = pCfg->hevcCfg.nrNoiseSigmaCb;
    param->nrNoiseSigmaCr   = pCfg->hevcCfg.nrNoiseSigmaCr;
    param->nrIntraWeightY   = pCfg->hevcCfg.nrIntraWeightY;
    param->nrIntraWeightCb  = pCfg->hevcCfg.nrIntraWeightCb;
    param->nrIntraWeightCr  = pCfg->hevcCfg.nrIntraWeightCr;
    param->nrInterWeightY   = pCfg->hevcCfg.nrInterWeightY;
    param->nrInterWeightCb  = pCfg->hevcCfg.nrInterWeightCb;
    param->nrInterWeightCr  = pCfg->hevcCfg.nrInterWeightCr;

    pEncOP->userQpMinI = pCfg->hevcCfg.intraMinQp;
    pEncOP->userQpMaxI = pCfg->hevcCfg.intraMaxQp;

    // avbr
    pEncOP->picMotionLevel  = 32;
    pEncOP->minStillPercent = 10;
    pEncOP->motionSensitivy = 30;

    pEncOP->avbrFrmLostOpen = 0;
    pEncOP->pureStillThr    = 4;
    pEncOP->avbrFrmGaps     = 1;

    pEncOP->maxIprop = -1; // not support
    return 1;
}

int setCoda9EncOpenParam(EncOpenParam *pEncOP, TestEncConfig *pEncConfig,
                         ENC_CFG *pCfg)
{
    Int32      bitFormat;
    Int32      srcWidth;
    Int32      srcHeight;
    Int32      outputNum;
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

    bitFormat = pEncOP->bitstreamFormat;

    srcWidth  = (pEncConfig->picWidth > 0) ? pEncConfig->picWidth : pCfg->PicX;
    srcHeight = (pEncConfig->picHeight > 0) ? pEncConfig->picHeight : pCfg->PicY;
    outputNum = (pEncConfig->outNum > 0) ? pEncConfig->outNum : pCfg->NumFrame;

    pEncConfig->outNum = outputNum;
    memcpy(pEncConfig->skipPicNums, pCfg->skipPicNums, sizeof(pCfg->skipPicNums));
    pEncOP->picWidth      = srcWidth;
    pEncOP->picHeight     = srcHeight;
    pEncOP->frameRateInfo = pCfg->FrameRate;
    pEncOP->bitRate       = (pEncConfig->kbps > 0) ? pEncConfig->kbps : pCfg->RcBitRate;
    pEncOP->rcMode        = pEncConfig->rcMode;

    pEncOP->statTime = 2;
    pEncOP->changePos =
        (pEncConfig->changePos > 50 && pEncConfig->changePos < 100)
            ? pEncConfig->changePos
            : 90;
    pEncOP->frmLostOpen   = pEncConfig->frmLostOpen;
    pEncOP->frmLostBpsThr = pEncOP->bitRate * 1200;
    pEncOP->encFrmGaps    = 1;
    pEncConfig->picQpY =
        (pEncConfig->picQpY >= 0) ? pEncConfig->picQpY : pCfg->PicQpY;
    CVI_VC_TRACE("picQpY = %d, PicQpY = %d\n", pEncConfig->picQpY, pCfg->PicQpY);

    pEncOP->rcInitDelay   = pCfg->RcInitDelay;
    pEncOP->vbvBufferSize = pCfg->RcBufSize;
    // for compare with C-model ( C-model = only 1 )
    pEncOP->frameSkipDisable = pCfg->frameSkipDisable;
    // for compare with C-model ( C-model = only 0 )
    pEncOP->meBlkMode = pCfg->MeBlkModeEnable;

    CVI_VC_TRACE("gopSize = %d, GopPicNum = %d\n", pEncConfig->gopSize,
                 pCfg->GopPicNum);
    pEncOP->gopSize =
        (pEncConfig->gopSize) ? pEncConfig->gopSize : pCfg->GopPicNum;
    pEncOP->idrInterval                = pCfg->IDRInterval;
    pEncOP->sliceMode.sliceMode        = pCfg->SliceMode;
    pEncOP->sliceMode.sliceSizeMode    = pCfg->SliceSizeMode;
    pEncOP->sliceMode.sliceSize        = pCfg->SliceSizeNum;
    pEncOP->intraRefreshNum            = pCfg->IntraRefreshNum;
    pEncOP->ConscIntraRefreshEnable    = pCfg->ConscIntraRefreshEnable;
    pEncOP->CountIntraMbEnable         = pCfg->CountIntraMbEnable;
    pEncOP->FieldSeqIntraRefreshEnable = pCfg->FieldSeqIntraRefreshEnable;
    pEncOP->rcIntraQp                  = pCfg->RCIntraQP;
    pEncOP->intraCostWeight            = pCfg->intraCostWeight;
    pEncOP->rcGopIQpOffsetEn           = pCfg->RcGopIQpOffsetEn;
    pEncOP->rcGopIQpOffset             = pCfg->RcGopIQpOffset;
    strcpy(pEncOP->paramChange.pchChangeCfgFileName, pCfg->pchChangeCfgFileName);
    pEncOP->paramChange.ChangeFrameNum = pCfg->ChangeFrameNum;

    if (bitFormat == STD_AVC && pEncOP->EncStdParam.avcParam.mvcExtension)
    {
        pEncOP->EncStdParam.avcParam.interviewEn      = pCfg->interviewEn;
        pEncOP->EncStdParam.avcParam.parasetRefreshEn = pCfg->parasetRefreshEn;
        pEncOP->EncStdParam.avcParam.prefixNalEn      = pCfg->prefixNalEn;
    }

#ifdef CODA980
    pEncOP->MESearchRangeX = 3; // pCfg->SearchRangeX;
    pEncOP->MESearchRangeY = 2; // pCfg->SearchRangeY;
#else
    pEncOP->MESearchRangeY = pCfg->SearchRangeY;
    pEncOP->MESearchRangeX = pCfg->SearchRangeX;
#endif
    CVI_VC_TRACE("MESearchRangeX = %d, MESearchRangeY = %d\n",
                 pEncOP->MESearchRangeX, pEncOP->MESearchRangeY);

    pEncOP->maxIntraSize = pCfg->RcMaxIntraSize;
    pEncOP->rcEnable =
        (pEncConfig->RcEnable >= 0) ? pEncConfig->RcEnable : pCfg->RcEnable;
    if (!pEncOP->rcEnable) pEncOP->bitRate = 0;

    CVI_VC_TRACE("RcEnable = %d, bitRate = %d bps\n", pCfg->RcEnable,
                 pEncOP->bitRate);

    pEncOP->maxIprop = pCviEc->u32MaxIprop;
    CVI_VC_TRACE("maxIprop = %d\n", pEncOP->maxIprop);

    if (!pCfg->GammaSetEnable)
        pEncOP->userGamma = -1;
    else
        pEncOP->userGamma = pCfg->Gamma;
    pEncOP->MEUseZeroPmv = pCfg->MeUseZeroPmv;
    /* It was agreed that the statements below would be used. but Cmodel at
r25518 is not changed yet according to the statements below if (bitFormat ==
STD_MPEG4) pEncOP->MEUseZeroPmv = 1; else pEncOP->MEUseZeroPmv = 0;
*/
    // MP4 263 Only
    if (!pCfg->ConstantIntraQPEnable) pEncOP->rcIntraQp = -1;

    if (pCfg->MaxQpSetEnable)
        pEncOP->userQpMax = pCfg->MaxQp;
    else
        pEncOP->userQpMax = -1;
    // H.264 Only
    if (bitFormat == STD_AVC)
    {
        if (pCfg->MaxQpSetEnable)
            pEncOP->userQpMaxI = pEncOP->userQpMaxP = pCfg->MaxQp;
        else
            pEncOP->userQpMaxI = pEncOP->userQpMaxP = 51;

        if (pCfg->MinQpSetEnable)
            pEncOP->userQpMinI = pEncOP->userQpMinP = pCfg->MinQp;
        else
            pEncOP->userQpMinI = pEncOP->userQpMinP = 12;

        if (pCfg->MaxDeltaQpSetEnable)
            pEncOP->userMaxDeltaQp = pCfg->MaxDeltaQp;
        else
            pEncOP->userMaxDeltaQp = -1;

        if (pCfg->MinDeltaQpSetEnable)
            pEncOP->userMinDeltaQp = pCfg->MinDeltaQp;
        else
            pEncOP->userMinDeltaQp = -1;
    }
    pEncOP->rcIntervalMode = pCfg->rcIntervalMode; // 0:normal,
                                                   // 1:frame_level,
                                                   // 2:slice_level, 3: user
                                                   // defined Mb_level
    pEncOP->mbInterval = pCfg->RcMBInterval;       // FIXME

    // Standard specific
    if (bitFormat == STD_AVC)
    {
        pEncOP->EncStdParam.avcParam.constrainedIntraPredFlag =
            pCfg->ConstIntraPredFlag;
        pEncOP->EncStdParam.avcParam.disableDeblk           = pCfg->DisableDeblk;
        pEncOP->EncStdParam.avcParam.deblkFilterOffsetAlpha = pCfg->DeblkOffsetA;
        pEncOP->EncStdParam.avcParam.deblkFilterOffsetBeta  = pCfg->DeblkOffsetB;
        pEncOP->EncStdParam.avcParam.chromaQpOffset         = pCfg->ChromaQpOffset;
        pEncOP->EncStdParam.avcParam.audEnable              = pCfg->aud_en;
        pEncOP->EncStdParam.avcParam.frameCroppingFlag      = pCfg->frameCroppingFlag;
        pEncOP->EncStdParam.avcParam.frameCropLeft          = pCfg->frameCropLeft;
        pEncOP->EncStdParam.avcParam.frameCropRight         = pCfg->frameCropRight;
        pEncOP->EncStdParam.avcParam.frameCropTop           = pCfg->frameCropTop;
        pEncOP->EncStdParam.avcParam.frameCropBottom        = pCfg->frameCropBottom;
        pEncOP->EncStdParam.avcParam.level                  = pCfg->level;

        // Update cropping information : Usage example for H.264
        // frame_cropping_flag
        if ((pEncOP->picHeight % 16) != 0)
        {
            // In case of AVC encoder, when we want to use unaligned
            // display width(For example, 1080), frameCroppingFlag
            // parameters should be adjusted to displayable
            // rectangle
            if (pEncConfig->rotAngle != 90 && pEncConfig->rotAngle != 270)
            {
                // except rotation
                if (pEncOP->EncStdParam.avcParam.frameCroppingFlag == 0)
                {
                    pEncOP->EncStdParam.avcParam.frameCroppingFlag = 1;
                    // frameCropBottomOffset =
                    // picHeight(MB-aligned) - displayable
                    // rectangle height
                    pEncOP->EncStdParam.avcParam.frameCropBottom =
                        (((pEncOP->picHeight + 15) >> 4) << 4) - pEncOP->picHeight;
                }
            }
        }

        // ENCODE SEQUENCE HEADER
        pEncOP->EncStdParam.avcParam.ppsParam[0].ppsId = 0;
        pEncOP->EncStdParam.avcParam.ppsParam[0].entropyCodingMode =
            pCfg->entropyCodingMode; // 0 : CAVLC, 1 : CABAC
        pEncOP->EncStdParam.avcParam.ppsParam[0].cabacInitIdc = pCfg->cabacInitIdc;
        pEncOP->EncStdParam.avcParam.ppsParam[0].transform8x8Mode =
            pCfg->transform8x8Mode;
        pEncOP->EncStdParam.avcParam.ppsNum          = 1;
        pEncOP->EncStdParam.avcParam.chromaFormat400 = pCfg->chroma_format_400;
        pEncOP->EncStdParam.avcParam.fieldFlag       = pCfg->field_flag;
        pEncOP->EncStdParam.avcParam.fieldRefMode    = pCfg->field_ref_mode;

        if (pCfg->transform8x8Mode == 1 || pCfg->chroma_format_400 == 1)
            pEncOP->EncStdParam.avcParam.profile = CVI_H264E_PROFILE_HIGH;
        else if (pCfg->entropyCodingMode >= 1 || pCfg->field_flag == 1)
            pEncOP->EncStdParam.avcParam.profile = CVI_H264E_PROFILE_MAIN;
        else
            pEncOP->EncStdParam.avcParam.profile = CVI_H264E_PROFILE_BASELINE;

        pEncOP->gopPreset =
            (pEncConfig->tempLayer > 0) ? pEncConfig->tempLayer : pCfg->GopPreset;

        if (pEncOP->gopPreset > 0)
        {
            pCfg->GopPreset = pEncOP->gopPreset;
            set_gop_info(pCfg);
        }
        memcpy(pEncOP->gopEntry, pCfg->gop_entry,
               sizeof(gop_entry_t) * MAX_GOP_SIZE);
        pEncOP->set_dqp_pic_num = pCfg->set_dqp_pic_num;

        pEncOP->LongTermPeriod  = pCfg->LongTermPeriod;
        pEncOP->LongTermDeltaQp = pCfg->LongTermDeltaQp;
        pEncOP->VirtualIPeriod  = pCfg->VirtualIPeriod;
        pEncOP->HvsQpScaleDiv2  = pCfg->HvsQpScaleDiv2;
        pEncOP->EnHvsQp         = pCfg->EnHvsQp;
        pEncOP->EnRowLevelRc    = pCfg->EnRowLevelRc;
        pEncOP->RcInitialQp     = (pCviEc->firstFrmstartQp > -1)
                                      ? pCviEc->firstFrmstartQp
                                      : pCfg->RcInitialQp;
        pEncOP->RcHvsMaxDeltaQp = pCfg->RcHvsMaxDeltaQp;
        // avbr
        pEncOP->picMotionLevel  = 32;
        pEncOP->minStillPercent = 10;
        pEncOP->motionSensitivy = 30;
        pEncOP->avbrFrmLostOpen = 0;
        pEncOP->pureStillThr    = 4;
        pEncOP->avbrFrmGaps     = 1;
#ifdef ROI_MB_RC
        pEncOP->roi_max_delta_qp_minus = pCfg->roi_max_delta_qp_minus;
        pEncOP->roi_max_delta_qp_plus  = pCfg->roi_max_delta_qp_plus;
#endif
#ifdef AUTO_FRM_SKIP_DROP
        pEncOP->enAutoFrmSkip            = pCfg->enAutoFrmSkip;
        pEncOP->enAutoFrmDrop            = pCfg->enAutoFrmDrop;
        pEncOP->vbvThreshold             = pCfg->vbvThreshold;
        pEncOP->qpThreshold              = pCfg->qpThreshold;
        pEncOP->maxContinuosFrameDropNum = pCfg->maxContinuosFrameDropNum;
        pEncOP->maxContinuosFrameSkipNum = pCfg->maxContinuosFrameSkipNum;
#endif
        pEncOP->rcWeightFactor = pCfg->rcWeightFactor;

        // ROI
        pEncOP->coda9RoiEnable = pCfg->coda9RoiEnable;
        pEncOP->RoiPicAvgQp    = pCfg->RoiPicAvgQp;
    }
    else
    {
        VLOG(ERR, "Invalid codec standard mode\n");
        return 0;
    }
    return 1;
}

static int get_entropy_coding_mode(const cviH264Entropy *pEntropy)
{
    if (pEntropy->entropyEncModeI == 0 && pEntropy->entropyEncModeP == 0)
        return CVI_CAVLC;
    else if (pEntropy->entropyEncModeI == 1 && pEntropy->entropyEncModeP == 1)
        return CVI_CABAC;
    else
        return CVI_CABAC;
}

/******************************************************************************
EncOpenParam Initialization
******************************************************************************/
/**
 * To init EncOpenParam by runtime evaluation
 * IN
 *   EncConfigParam *pEncConfig
 * OUT
 *   EncOpenParam *pEncOP
 */
Int32 GetEncOpenParamDefault(EncOpenParam *pEncOP, TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;
    int        bitFormat;
    int        frameRateDiv, frameRateRes;

    pEncConfig->outNum =
        pEncConfig->outNum == 0 ? DEFAULT_ENC_OUTPUT_NUM : pEncConfig->outNum;

    bitFormat = pEncOP->bitstreamFormat;

    pEncOP->picWidth  = pEncConfig->picWidth;
    pEncOP->picHeight = pEncConfig->picHeight;
    pEncOP->gopSize   = pCviEc->gop;

    CVI_VC_CFG("picWidth = %d, picHeight = %d\n", pEncConfig->picWidth,
               pEncConfig->picHeight);
    CVI_VC_CFG("secondary_axi = 0x%X, lineBufIntEnbitRate = %d\n",
               pEncConfig->secondary_axi, pEncConfig->lineBufIntEn);

    frameRateDiv = (pCviEc->framerate >> 16);
    frameRateRes = pCviEc->framerate & 0xFFFF;

    if (frameRateDiv == 0)
    {
        pEncOP->frameRateInfo = pCviEc->framerate;
    }
    else
    {
        pEncOP->frameRateInfo = ((frameRateDiv - 1) << 16) + frameRateRes;
    }

    pEncOP->maxIntraSize = 0;

    pEncOP->bitRate =
        (pCviEc->rcMode == RC_MODE_CBR || pCviEc->rcMode == RC_MODE_UBR)
            ? pCviEc->bitrate
            : pCviEc->maxbitrate;

    CVI_VC_CFG("rcMode = %d,\n", pCviEc->rcMode);
    CVI_VC_CFG("bitRate = %d, framerate = %d\n", pCviEc->bitrate,
               pEncOP->frameRateInfo);
    CVI_VC_CFG("statTime = %d, maxbitrate = %d\n", pCviEc->statTime,
               pCviEc->maxbitrate);

    pEncOP->vbvBufferSize = 0; // 0 = ignore
    pEncOP->meBlkMode     = 0; // for compare with C-model ( C-model = only 0 )
    pEncOP->frameSkipDisable =
        0;                     // for compare with C-model ( C-model = only 1 )
    pEncOP->sliceMode.sliceSizeMode = 1;
    pEncOP->sliceMode.sliceSize     = 115;
    pEncOP->intraRefreshNum         = 0;
    pEncOP->rcIntraQp               = -1; // disable == -1
    pEncOP->userQpMax               = -1; // disable == -1

#if 1                                     // def USE_KERNEL_MODE
    pCviEc->cviRcEn = 1;
#else
    pCviEc->cviRcEn = (cviVcodecGetEnv("cviRcEn") == 1) ? 1 : 0;
#endif

    pEncOP->cviRcEn = pCviEc->cviRcEn;
    CVI_VC_CFG("cviRcEn = %d\n", pCviEc->cviRcEn);

    pEncOP->rcMode   = pCviEc->rcMode;
    pEncOP->statTime = pCviEc->statTime;

    // for VBR, AVBR
    pEncOP->changePos = pEncConfig->changePos;
    CVI_VC_CFG("statTime = %d, changePos = %d\n", pEncOP->statTime,
               pEncOP->changePos);

    // for AVBR
    pEncOP->picMotionLevel  = 32;
    pEncOP->minStillPercent = pCviEc->s32MinStillPercent;
    pEncOP->motionSensitivy = pCviEc->u32MotionSensitivity;
    pEncOP->maxStillQp      = pCviEc->u32MaxStillQP;
    pEncOP->avbrFrmLostOpen = pCviEc->s32AvbrFrmLostOpen;
    pEncOP->pureStillThr    = pCviEc->s32AvbrPureStillThr;
    pEncOP->avbrFrmGaps     = pCviEc->s32AvbrFrmGap;
    //	pEncOP->bBgEnhanceEn = pCviEc->bBgEnhanceEn;
    //	pEncOP->s32BgDeltaQp = pCviEc->s32BgDeltaQp;

    CVI_VC_CFG("picMotionLevel = %d\n", pEncOP->picMotionLevel);
    CVI_VC_CFG("StillPercent = %d, StillQP = %d, MotionSensitivity = %d\n",
               pCviEc->s32MinStillPercent, pCviEc->u32MaxStillQP,
               pCviEc->u32MotionSensitivity);
    CVI_VC_CFG("FrmLostOpen = %d, FrmGap = %d, PureStillThr = %d\n",
               pCviEc->s32AvbrFrmLostOpen, pCviEc->s32AvbrFrmGap,
               pCviEc->s32AvbrPureStillThr);

    pEncOP->frmLostBpsThr = 0;
    pEncOP->frmLostOpen   = pEncConfig->frmLostOpen;
    pEncOP->frmLostMode   = 1; // only support P_SKIP
    pEncOP->encFrmGaps    = pCviEc->encFrmGaps;
    pEncOP->frmLostBpsThr = pCviEc->frmLostBpsThr;
    pEncOP->minQpDelta    = -1;

    pEncConfig->picQpY = pCviEc->iqp;

    if (bitFormat == STD_AVC)
    {
        setAvcEncDefaultParam(pEncOP, pEncConfig);
    }
    else if (bitFormat == STD_HEVC)
    {
        setHevcEncDefaultParam(pEncOP, pEncConfig);
    }
    else
    {
        VLOG(ERR, "Invalid codec standard mode: codec index(%d) \n", bitFormat);
        return 0;
    }

    pEncOP->EncStdParam.avcParam.interviewEn      = 0;
    pEncOP->EncStdParam.avcParam.parasetRefreshEn = 0;
    pEncOP->EncStdParam.avcParam.prefixNalEn      = 0;

    return 1;
}

static void setAvcEncDefaultParam(EncOpenParam  *pEncOP,
                                  TestEncConfig *pEncConfig)
{
    EncAvcParam *pAvcParam = &pEncOP->EncStdParam.avcParam;
    AvcPpsParam *pPpsParam = &pEncOP->EncStdParam.avcParam.ppsParam[0];
    cviEncCfg   *pCviEc    = &pEncConfig->cviEc;

    pAvcParam->constrainedIntraPredFlag = 0;
    pAvcParam->disableDeblk             = 0;
    pAvcParam->deblkFilterOffsetAlpha   = 0;
    pAvcParam->deblkFilterOffsetBeta    = 0;
    pAvcParam->chromaQpOffset           = pCviEc->h264Trans.chroma_qp_index_offset;
    pAvcParam->audEnable                = 0;
    pAvcParam->frameCroppingFlag        = 0;
    pAvcParam->frameCropLeft            = 0;
    pAvcParam->frameCropRight           = 0;
    pAvcParam->frameCropTop             = 0;
    pAvcParam->frameCropBottom          = 0;
    pAvcParam->level                    = 0;

    // Update cropping information : Usage example for H.264
    // frame_cropping_flag
    if ((pEncOP->picWidth % 16) != 0 || (pEncOP->picHeight % 16) != 0)
    {
        // In case of AVC encoder, when we want to use unaligned
        // display width(For example, 1080), frameCroppingFlag
        // parameters should be adjusted to displayable
        // rectangle
        if (pEncConfig->rotAngle != 90 && pEncConfig->rotAngle != 270)
        {
            // except rotation
            if (pAvcParam->frameCroppingFlag == 0)
            {
                pAvcParam->frameCroppingFlag = 1;

                if ((pEncOP->picWidth % 16) != 0)
                {
                    pAvcParam->frameCropRight =
                        (((pEncOP->picWidth + 15) >> 4) << 4) - pEncOP->picWidth;
                }

                if ((pEncOP->picHeight % 16) != 0)
                {
                    pAvcParam->frameCropBottom =
                        (((pEncOP->picHeight + 15) >> 4) << 4) - pEncOP->picHeight;
                }
            }
        }
    }

    pPpsParam->ppsId             = 0;
    pPpsParam->entropyCodingMode = get_entropy_coding_mode(&pCviEc->h264Entropy);
    pPpsParam->cabacInitIdc      = pCviEc->h264Entropy.cabac_init_idc;
    pPpsParam->transform8x8Mode  = (pCviEc->u32Profile == CVI_H264E_PROFILE_HIGH);
    CVI_VC_CFG("u32Profile = %d, entropy = %d, transform8x8Mode = %d\n",
               pCviEc->u32Profile, pPpsParam->entropyCodingMode,
               pPpsParam->transform8x8Mode);
    pAvcParam->profile         = pCviEc->u32Profile;
    pAvcParam->ppsNum          = 1;
    pAvcParam->chromaFormat400 = 0;
    pAvcParam->fieldFlag       = 0;
    pAvcParam->fieldRefMode    = 1;

    rcLibSetParam(pEncOP, pEncConfig);
}

static void rcLibSetParam(EncOpenParam *pEncOP, TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc       = &pEncConfig->cviEc;
    pEncOP->rcEnable        = (pEncOP->bitRate) ? 4 : 0;
    pEncOP->EnRowLevelRc    = (pCviEc->u32RowQpDelta > 0);
    pEncOP->RcHvsMaxDeltaQp = 8;
    pEncOP->HvsQpScaleDiv2 =
        (pCviEc->u32ThrdLv <= 4) ? (int)pCviEc->u32ThrdLv : 2;
    CVI_VC_CFG("u32ThrdLv = %d, HvsQpScaleDiv2 = %d\n", pCviEc->u32ThrdLv,
               pEncOP->HvsQpScaleDiv2);
    pEncOP->EnHvsQp     = 1;
    pEncOP->rcInitDelay = pCviEc->initialDelay;

    if (pEncOP->bitstreamFormat == STD_HEVC && pCviEc->cviRcEn && pCviEc->firstFrmstartQp == 63)
    {
        pEncOP->RcInitialQp = DEFAULT_QP;
    }
    else
    {
        pEncOP->RcInitialQp =
            (pCviEc->firstFrmstartQp > -1) ? pCviEc->firstFrmstartQp : DEFAULT_QP;
    }

    pEncOP->userMinDeltaQp = -1;
    pEncOP->userMaxDeltaQp = -1;

    {
        ENC_CFG encCfg, *pCfg = &encCfg;
        pEncOP->gopPreset = (pEncConfig->tempLayer > 0) ? pEncConfig->tempLayer : 1;

        if (pEncOP->gopPreset > 0)
        {
            pCfg->GopPreset = pEncOP->gopPreset;
            set_gop_info(pCfg);
        }
        pEncOP->set_dqp_pic_num = pCfg->set_dqp_pic_num;
        memcpy(pEncOP->gopEntry, pCfg->gop_entry,
               sizeof(gop_entry_t) * MAX_GOP_SIZE);
    }

    pEncOP->LongTermPeriod = -1;
    CVI_VC_CFG("LongTermPeriod = %d\n", pEncOP->LongTermPeriod);

    pEncOP->LongTermDeltaQp = -pCviEc->s32IPQpDelta;
    pEncOP->VirtualIPeriod  = -1;
    if (pCviEc->virtualIPeriod)
    {
        pEncOP->LongTermDeltaQp = pCviEc->s32ViQpDelta;
        pEncOP->LongTermPeriod  = pCviEc->gop;
        pEncOP->VirtualIPeriod  = pCviEc->virtualIPeriod;
    }
    pEncOP->userQpMinI = CLIP3(0, 51, (int)pCviEc->u32MinIQp);
    pEncOP->userQpMaxI = CLIP3(0, 51, (int)pCviEc->u32MaxIQp);
    pEncOP->userQpMinP = CLIP3(0, 51, (int)pCviEc->u32MinQp);
    pEncOP->userQpMaxP = CLIP3(0, 51, (int)pCviEc->u32MaxQp);

    CVI_VC_CFG("QP %d, I max %d, min %d, P max %d, min %d\n", pCviEc->iqp,
               pEncOP->userQpMaxI, pEncOP->userQpMinI, pEncOP->userQpMaxP,
               pEncOP->userQpMinP);
    pEncOP->MEUseZeroPmv        = 0;
    pEncOP->sliceMode.sliceMode = 0;
    pEncOP->idrInterval         = 1;
    pEncOP->userGamma           = 16;
    pEncOP->rcIntervalMode      = 0;
    pEncOP->mbInterval          = 1;
    pEncOP->MESearchRangeX      = 0;
    pEncOP->MESearchRangeY      = 0;
    pEncOP->rcGopIQpOffsetEn    = 1;

    if (pCviEc->virtualIPeriod)
        pEncOP->rcGopIQpOffset = pCviEc->s32BgQpDelta;
    else
        pEncOP->rcGopIQpOffset = pCviEc->s32IPQpDelta;
    CVI_VC_CFG("intraQpOffset = %d\n", pEncOP->rcGopIQpOffset);

    pEncOP->frameSkipDisable =
        1; // for compare with C-model ( C-model = only 1 )
    pEncOP->enAutoFrmSkip   = 0;
    pEncOP->rcWeightFactor  = 4;
    pEncOP->intraCostWeight = (int)pCviEc->u32IntraCost;
    CVI_VC_CFG("intraCostWeight = %d\n", pEncOP->intraCostWeight);
    pEncOP->vbvThreshold = 3000;

    pEncOP->maxIprop = pCviEc->u32MaxIprop;
    CVI_VC_CFG("maxIprop = %d\n", pEncOP->maxIprop);
}

static void setHevcEncDefaultParam(EncOpenParam  *pEncOP,
                                   TestEncConfig *pEncConfig)
{
    EncHevcParam *pHevcParam = &pEncOP->EncStdParam.hevcParam;
    cviEncCfg    *pCviEc     = &pEncConfig->cviEc;
    Int32         i          = 0;

    CVI_VC_TRACE("cviRcEn = %d\n", pEncOP->cviRcEn);
    if (pEncOP->cviRcEn || pEncOP->rcMode == RC_MODE_UBR)
    {
        rcLibSetParam(pEncOP, pEncConfig);
    }

    pEncOP->idrInterval            = 1;
    pHevcParam->profile            = HEVC_PROFILE_MAIN;
    pHevcParam->level              = 0;
    pHevcParam->tier               = 0;
    pHevcParam->internalBitDepth   = 8;
    pEncOP->srcBitDepth            = 8;
    pHevcParam->chromaFormatIdc    = 0;
    pHevcParam->losslessEnable     = 0;
    pHevcParam->constIntraPredFlag = 0;
    pHevcParam->enableAFBCD        = 0;
    pHevcParam->useLongTerm        = 0;

    /* HEVC SmartP mode is implemented with GOP preset 17 */
    if (pCviEc->virtualIPeriod)
        pEncConfig->tempLayer = 2; // implement SmartP with GOP preset 17

    /* for CMD_ENC_SEQ_GOP_PARAM */
    if (pEncConfig->tempLayer == 2)
        pHevcParam->gopPresetIdx = 17;
    else if (pEncConfig->tempLayer == 3)
        pHevcParam->gopPresetIdx = 19;
    else
        pHevcParam->gopPresetIdx = PRESET_IDX_CUSTOM_GOP;

    /* for CMD_ENC_SEQ_INTRA_PARAM */
    pHevcParam->decodingRefreshType = pEncConfig->decodingRefreshType;
    pHevcParam->intraPeriod         = pCviEc->gop;
    pHevcParam->intraQP             = pCviEc->iqp;
    CVI_VC_CFG("intraQP = %d\n", pHevcParam->intraQP);

    /* for CMD_ENC_SEQ_CONF_WIN_TOP_BOT/LEFT_RIGHT */
    pHevcParam->confWinTop   = 0;
    pHevcParam->confWinBot   = 0;
    pHevcParam->confWinLeft  = 0;
    pHevcParam->confWinRight = 0;

    /* for CMD_ENC_SEQ_INDEPENDENT_SLICE */
    pHevcParam->independSliceMode    = 0;
    pHevcParam->independSliceModeArg = 0;

    /* for CMD_ENC_SEQ_DEPENDENT_SLICE */
    pHevcParam->dependSliceMode    = 0;
    pHevcParam->dependSliceModeArg = 0;

    /* for CMD_ENC_SEQ_INTRA_REFRESH_PARAM */
    pHevcParam->intraRefreshMode              = 0;
    pHevcParam->intraRefreshArg               = 0;
    pHevcParam->useRecommendEncParam          = 0;
    pEncConfig->seiDataEnc.prefixSeiNalEnable = 0;
    pEncConfig->seiDataEnc.suffixSeiNalEnable = 0;
    pEncOP->encodeHrdRbspInVPS                = 0;
    pEncOP->encodeHrdRbspInVUI                = 0;
    pEncOP->encodeVuiRbsp                     = 0;
    pEncConfig->roi_enable                    = 0;
    pEncConfig->ctu_mode_enable               = 0;
    pEncConfig->ctu_qp_enable                 = 0;

    /* for CMD_ENC_PARAM */
    if (pHevcParam->useRecommendEncParam != 1)
    {
        // 0 : Custom
        // 2 : Boost mode (normal encoding speed, normal picture quality)
        // 3 : Fast mode (high encoding speed, low picture quality)
        pHevcParam->scalingListEnable          = 0;
        pHevcParam->cuSizeMode                 = 0x7;
        pHevcParam->tmvpEnable                 = 1;
        pHevcParam->wppEnable                  = 0;
        pHevcParam->maxNumMerge                = 2;
        pHevcParam->dynamicMerge8x8Enable      = 1;
        pHevcParam->dynamicMerge16x16Enable    = 1;
        pHevcParam->dynamicMerge32x32Enable    = 1;
        pHevcParam->disableDeblk               = 0;
        pHevcParam->lfCrossSliceBoundaryEnable = 1;
        pHevcParam->betaOffsetDiv2             = 0;
        pHevcParam->tcOffsetDiv2               = 0;
        pHevcParam->skipIntraTrans             = 1;
        pHevcParam->saoEnable                  = 0;
        pHevcParam->intraInInterSliceEnable    = 1;
        pHevcParam->intraNxNEnable             = 0;
    }

    /* for CMD_ENC_RC_PARAM */
    pEncOP->rcEnable     = (pEncOP->bitRate) ? 1 : 0;
    pEncOP->initialDelay = pCviEc->initialDelay;

    pHevcParam->ctuOptParam.roiEnable  = 0;
    pHevcParam->ctuOptParam.roiDeltaQp = 3;

    if (pEncOP->rcMode == RC_MODE_QPMAP)
    {
        pHevcParam->ctuOptParam.ctuQpEnable = 1;
        pEncConfig->ctu_qp_enable           = 1;
    }
    else
    {
        pHevcParam->ctuOptParam.ctuQpEnable = 0;
        pEncConfig->ctu_qp_enable           = 0;
    }

    if (pCviEc->virtualIPeriod > 0)
        pHevcParam->intraQpOffset = -pCviEc->s32BgQpDelta;
    else
        pHevcParam->intraQpOffset = -pCviEc->s32IPQpDelta;
    CVI_VC_CFG("ctuQpEnable = %d, ctu_qp_enable = %d, intraQpOffset = %d\n",
               pHevcParam->ctuOptParam.ctuQpEnable, pEncConfig->ctu_qp_enable,
               pHevcParam->intraQpOffset);

    pHevcParam->initBufLevelx8 = 1;
    pHevcParam->bitAllocMode   = 1;
    for (i = 0; i < MAX_GOP_NUM; i++)
    {
        pHevcParam->fixedBitRatio[i] = 1;
    }
    pHevcParam->cuLevelRCEnable = (pCviEc->u32RowQpDelta > 0);
    pHevcParam->hvsQPEnable     = 1;
    pHevcParam->hvsQpScale =
        (pCviEc->u32ThrdLv <= 4) ? (int)pCviEc->u32ThrdLv : 2;
    CVI_VC_CFG("u32ThrdLv = %d, hvsQpScale = %d\n", pCviEc->u32ThrdLv,
               pHevcParam->hvsQpScale);
    pHevcParam->hvsQpScaleEnable = (pHevcParam->hvsQpScale > 0) ? 1 : 0;

    /* for CMD_ENC_RC_MIN_MAX_QP */
    {
        unsigned int MinQP = 8, MaxQP = 51;
        if (pCviEc->u32MinQp > 0 && pCviEc->u32MinQp <= 51)
            MinQP = pCviEc->u32MinQp;
        if (pCviEc->u32MinIQp > 0 && pCviEc->u32MinIQp <= 51 && pCviEc->u32MinIQp > pCviEc->u32MinQp)
            MinQP = pCviEc->u32MinIQp;
        pEncOP->userQpMinP = MinQP;
        pEncOP->userQpMinI = MinQP;

        if (pCviEc->u32MaxQp > 0 && pCviEc->u32MaxQp <= 51)
            MaxQP = pCviEc->u32MaxQp;
        if (pCviEc->u32MaxIQp > 0 && pCviEc->u32MaxIQp <= 51 && pCviEc->u32MaxIQp < pCviEc->u32MaxQp)
            MaxQP = pCviEc->u32MaxIQp;
        pEncOP->userQpMaxP = MaxQP;
        pEncOP->userQpMaxI = MaxQP;
    }

    CVI_VC_TRACE("minQP:%d maxQP:%d,minIQp:%d,maxIQp:%d\n", pEncOP->userQpMinP,
                 pEncOP->userQpMaxP, pEncOP->userQpMinI, pEncOP->userQpMaxI);

    pHevcParam->maxDeltaQp    = 10;
    pHevcParam->hvsMaxDeltaQp = 10;

    if (pCviEc->virtualIPeriod > 0 && pEncOP->rcMode != RC_MODE_FIXQP && pEncOP->rcMode != RC_MODE_QPMAP)
    {
        pHevcParam->gopParam.enTemporalLayerQp = 1;
        pHevcParam->gopParam.tidQp0            = 20;
        pHevcParam->gopParam.tidQp1 =
            pHevcParam->gopParam.tidQp0 + pCviEc->s32ViQpDelta;
        pHevcParam->gopParam.tidQp2 = 26;
    }
    else
    {
        pHevcParam->gopParam.enTemporalLayerQp = 0;
        pHevcParam->gopParam.tidQp0            = 30;
        pHevcParam->gopParam.tidQp1            = 33;
        pHevcParam->gopParam.tidQp2            = 36;
    }
    CVI_VC_CFG("enTemporalLayerQp %d, tidQp0 %d, tidQp1 %d, tidQp2 %d\n",
               pHevcParam->gopParam.enTemporalLayerQp,
               pHevcParam->gopParam.tidQp0, pHevcParam->gopParam.tidQp1,
               pHevcParam->gopParam.tidQp2);

    if (pEncConfig->tempLayer == 2)
    {
        if (pCviEc->virtualIPeriod)
            pHevcParam->gopParam.tidPeriod0 = pCviEc->virtualIPeriod; // HEVC SmartP
        else
            pHevcParam->gopParam.tidPeriod0 = 2;
    }
    else if (pEncConfig->tempLayer == 3)
        pHevcParam->gopParam.tidPeriod0 = 4;
    else
        pHevcParam->gopParam.tidPeriod0 = 1;

    /* for CMD_ENC_CUSTOM_GOP_PARAM */
    pHevcParam->gopParam.customGopSize         = 1;
    pHevcParam->gopParam.useDeriveLambdaWeight = 1;

    for (i = 0; i < pHevcParam->gopParam.customGopSize; i++)
    {
        pHevcParam->gopParam.picParam[i].picType    = PIC_TYPE_P;
        pHevcParam->gopParam.picParam[i].pocOffset  = 1;
        pHevcParam->gopParam.picParam[i].picQp      = pCviEc->pqp;
        pHevcParam->gopParam.picParam[i].refPocL0   = 0;
        pHevcParam->gopParam.picParam[i].refPocL1   = 0;
        pHevcParam->gopParam.picParam[i].temporalId = 0;
        pHevcParam->gopParam.gopPicLambda[i]        = 0;
    }

    pHevcParam->transRate =
        ((pEncOP->rcMode != RC_MODE_CBR && pEncOP->rcMode != RC_MODE_UBR) || (pEncOP->statTime <= 0))
            ? MAX_TRANSRATE
            : pEncOP->bitRate * pEncOP->statTime * 1000;
    CVI_VC_CFG("transRate = %d\n", pHevcParam->transRate);

    // for VUI / time information.
    pHevcParam->numTicksPocDiffOne = 0;
    pHevcParam->timeScale          = 0;
    pHevcParam->numUnitsInTick     = 0;

    // when vuiParamFlags == 0, VPU doesn't encode VUI
    pHevcParam->vuiParam.vuiParamFlags = 0;
    pHevcParam->chromaCbQpOffset       = pCviEc->h265Trans.cb_qp_offset;
    pHevcParam->chromaCrQpOffset       = pCviEc->h265Trans.cr_qp_offset;
    pHevcParam->initialRcQp            = pCviEc->firstFrmstartQp; // 63 is meaningless.
    CVI_VC_TRACE("initialRcQp = %d\n", pHevcParam->initialRcQp);

    pHevcParam->nrYEnable        = 0;
    pHevcParam->nrCbEnable       = 0;
    pHevcParam->nrCrEnable       = 0;
    pHevcParam->nrNoiseEstEnable = 1;
    pHevcParam->nrIntraWeightY   = 7;
    pHevcParam->nrIntraWeightCb  = 7;
    pHevcParam->nrIntraWeightCr  = 7;
    pHevcParam->nrInterWeightY   = 4;
    pHevcParam->nrInterWeightCb  = 4;
    pHevcParam->nrInterWeightCr  = 4;

    pHevcParam->rcWeightParam = 2;
    pHevcParam->rcWeightBuf   = 128;
}

#if SET_DEFAULT_CFG
static void setDefaultCfg(TestEncConfig *pEncConfig, ENC_CFG *pCfg)
{
    pEncConfig->picWidth    = WIDTH;
    pEncConfig->picHeight   = HEIGHT;
    pEncConfig->outNum      = DEFAULT_ENC_OUTPUT_NUM;
    pCfg->FrameRate         = 30;
    pCfg->RcBitRate         = 0;
    pCfg->frameSkipDisable  = 1;
    pCfg->GopPicNum         = 60;
    pCfg->IDRInterval       = 60;
    pCfg->RCIntraQP         = 33;
    pCfg->level             = 51;
    pCfg->entropyCodingMode = 1;
    pCfg->LongTermPeriod    = 60;
    pCfg->LongTermDeltaQp   = 33;
    pCfg->RcInitialQp       = 33;
    pCfg->rcWeightFactor    = 2;
    pCfg->PicQpY            = 33;
}
#endif

#define AVC_ENC_NUT_SLICE 1
#define AVC_ENC_NUT_IDR   5
static frm_t *get_current_frame(EncInfo *seq)
{
    int    i;
    frm_t *curr_frm = NULL;

    if (seq->openParam.gopSize == 1)
    {
        // all intra
        curr_frm      = &seq->frm[0];
        seq->prev_idx = 0;
    }
    else
    {
        for (i = 0; i < seq->num_total_frames; i++)
        {
            if (seq->frm[i].used_for_ref == 0)
            {
                curr_frm      = &seq->frm[i];
                seq->prev_idx = i;
                break;
            }
        }
    }

    return curr_frm;
}

static void set_mmco(EncInfo *seq)
{
    int frmidx;
    int next_idx =
        (seq->enc_idx_modulo == seq->gop_size - 1) ? 0 : seq->enc_idx_modulo + 1;
    rps_t *next_rps = &seq->rps[next_idx];
    int    curr_gop_poc =
        seq->curr_frm->poc - seq->gop_entry[seq->enc_idx_modulo].curr_poc;
    int next_gop_poc = curr_gop_poc + ((next_idx == 0) ? seq->gop_size : 0);

    // mmco1 : set short-term as non-ref
    seq->num_mmco1 = 0;

    for (frmidx = 0; frmidx < seq->num_total_frames; frmidx++)
    {
        frm_t *frm = &seq->frm[frmidx];

        if (frm->used_for_ref && (frm != seq->curr_frm) && (frmidx != seq->longterm_frmidx))
        {
            int i;
            int found = 0;

            // check if the ref-frame is in next rps
            for (i = 0; i < next_rps->num_poc; i++)
                if (frm->poc == next_gop_poc + next_rps->poc[i]) found = 1;

            if (!found)
            {
                seq->mmco1_frmidx[seq->num_mmco1++] = frmidx;
            }
        }
    }
}

static int search_ref_frame(EncInfo *seq, int poc)
{
    int i;
    for (i = 0; i < 31; i++)
        if (seq->frm[i].used_for_ref && seq->frm[i].poc == poc) return i;
    return -1;
}

static void set_refpic_list(EncInfo *seq, int ref_long_term)
{
    int ref, list;
    int num_ref, num_list;
    int enc_idx;
    int gop_poc;

    enc_idx = seq->enc_idx_modulo;

    gop_poc = seq->curr_frm->poc - seq->gop_entry[enc_idx].curr_poc;

    seq->num_ref_idx  = 0;
    seq->ref_pic_list = 0;

    // fill ref_list
    num_ref = 1; //(seq->curr_frm->slice_type == AVC_ENC_SLICE_TYPE_B ||
                 // seq->gop_entry[enc_idx].use_multi_ref_p) ? 2 : 1;
    for (ref = 0; ref < num_ref; ref++)
    {
        int frmidx;
        int poc = gop_poc + seq->gop_entry[enc_idx].ref_poc;

        if (ref_long_term && (ref == num_ref - 1) && (seq->longterm_frmidx >= 0)) // long-term
            frmidx = seq->longterm_frmidx;
        else
            frmidx = search_ref_frame(seq, poc);

        if (frmidx != -1)
        {
            seq->ref_pic_list = frmidx;
            seq->num_ref_idx++;
        }
    }

    // when num_ref_idx is 0, fill ref_list with rps
    num_list = 1;
    for (list = 0; list < num_list; list++)
    {
        if (seq->num_ref_idx == 0)
        {
            int    i;
            rps_t *rps = &seq->rps[enc_idx];

            for (i = 0; i < rps->num_poc; i++)
            {
                int poc    = gop_poc + rps->poc[i];
                int frmidx = search_ref_frame(seq, poc);
                if (frmidx != -1)
                {
                    seq->ref_pic_list = frmidx;
                    seq->num_ref_idx++;
                    break;
                }
            }
        }
    }
}

static int get_nal_ref_idc(EncInfo *seq, int enc_idx)
{
    if (seq->idr_picture)
        return 3;
    else if (seq->gop_entry[enc_idx].curr_poc == seq->gop_size) // anchor
        return 2;
    else if (seq->curr_frm->used_for_ref)
        return 1;
    else
        return 0;
}

#define AVC_ENC_NUT_SLICE 1
#define AVC_ENC_NUT_IDR   5

void dpb_pic_init(EncInfo *pAvcInfo)
{
    int encode_frame;
    int enc_idx; // = pAvcInfo->enc_idx_modulo;
    int i;
    int gop_num = pAvcInfo->openParam.gopSize;

    encode_frame = pAvcInfo->encoded_frames_in_gop;
    enc_idx      = pAvcInfo->enc_idx_modulo;

    if (gop_num == 1) // All I
        encode_frame = 0;
    else
    {
        if (encode_frame >= gop_num && gop_num != 0) // I frame
            encode_frame = 0;
    }

    pAvcInfo->idr_picture = 0;
    if (pAvcInfo->frm_cnt == 0)
    {
        pAvcInfo->Idr_cnt     = 0;
        pAvcInfo->idr_picture = 1;
    }
    else
    {
        if ((pAvcInfo->openParam.idrInterval) > 0 && (pAvcInfo->openParam.gopSize > 0))
        {
            if (encode_frame == 0)
            {
                pAvcInfo->Idr_cnt++;
            }

            if (pAvcInfo->openParam.idrInterval == pAvcInfo->Idr_cnt)
            {
                pAvcInfo->frm_cnt     = 0;
                pAvcInfo->Idr_cnt     = 0;
                pAvcInfo->idr_picture = 1;

                if (pAvcInfo->singleLumaBuf)
                {
                    for (i = 0; i < 31; i++) pAvcInfo->frm[i].used_for_ref = 0;
                }
            }
        }
    }

    if (pAvcInfo->idr_picture)
        pAvcInfo->Idr_picId =
            (pAvcInfo->Idr_picId + 1) & 0xFFFF; // Idr_pic_id: 0 ~ 65535

    pAvcInfo->slice_type = (encode_frame == 0) ? 0 : 1;

    if (pAvcInfo->enc_long_term_cnt > 0)
    {
        if (pAvcInfo->enc_long_term_cnt == pAvcInfo->long_term_period)
            pAvcInfo->enc_long_term_cnt = 0;
    }

    if ((pAvcInfo->enc_long_term_cnt == 0 || pAvcInfo->force_as_long_term_ref) && pAvcInfo->long_term_period != -1)
        pAvcInfo->curr_long_term = 1; // enc_long_term_cnt %
                                      // pAvcInfo->long_term_d;
    else
        pAvcInfo->curr_long_term = 0;

    pAvcInfo->ref_long_term = // pAvcInfo->curr_long_term ||
        ((pAvcInfo->virtual_i_period > 0)
             ? (pAvcInfo->frm_cnt % pAvcInfo->virtual_i_period == 0)
             : 0)
        || pAvcInfo->gop_entry[pAvcInfo->enc_idx_modulo].ref_long_term;

    if (pAvcInfo->enc_idx_modulo >= 0)
        pAvcInfo->src_idx = pAvcInfo->enc_idx_gop + pAvcInfo->gop_entry[pAvcInfo->enc_idx_modulo].curr_poc;
    else
        pAvcInfo->src_idx = pAvcInfo->enc_idx_gop;

    pAvcInfo->num_mmco1 = 0;
    pAvcInfo->curr_frm  = get_current_frame(pAvcInfo);

    if (pAvcInfo->idr_picture) pAvcInfo->src_idx_last_idr = pAvcInfo->src_idx;

    if (pAvcInfo->idr_picture)
    {
        // pAvcInfo->Idr_picId ^= 1;

        // clear all refs
        for (i = 0; i < 31; i++) pAvcInfo->frm[i].used_for_ref = 0;

        pAvcInfo->curr_frm->used_for_ref = 1;
        pAvcInfo->curr_frm->slice_type   = 0; // I_SLICE;
        pAvcInfo->curr_frm->poc          = 0;
        pAvcInfo->curr_frm->frame_num    = 0;
    }
    else
    {
        int enc_idx = pAvcInfo->enc_idx_modulo;

        pAvcInfo->curr_frm->used_for_ref =
            pAvcInfo->rps[enc_idx].used_for_ref || pAvcInfo->curr_long_term;
        pAvcInfo->curr_frm->slice_type = pAvcInfo->slice_type;
        pAvcInfo->curr_frm->poc        = pAvcInfo->src_idx - pAvcInfo->src_idx_last_idr;
        pAvcInfo->curr_frm->frame_num  = (pAvcInfo->prev_frame_num + 1) % (1 << 4);

        if (pAvcInfo->curr_frm->used_for_ref) set_mmco(pAvcInfo);

        if (pAvcInfo->curr_frm->slice_type != 0) // I_SLICE)
            set_refpic_list(pAvcInfo, pAvcInfo->ref_long_term);
    }

    pAvcInfo->nal_ref_idc = get_nal_ref_idc(pAvcInfo, enc_idx);
    pAvcInfo->nal_unit_type =
        pAvcInfo->idr_picture ? AVC_ENC_NUT_IDR : AVC_ENC_NUT_SLICE;

    // update prev_frame_num
    if (pAvcInfo->curr_frm->used_for_ref)
        pAvcInfo->prev_frame_num = pAvcInfo->curr_frm->frame_num;

    //    pAvcInfo->RefPicList[0] = pAvcInfo->ref_pic_list;

    pAvcInfo->temporal_id =
        (pAvcInfo->enc_idx_modulo == -1)
            ? 0
            : pAvcInfo->gop_entry[pAvcInfo->enc_idx_modulo].temporal_id;

    if (pAvcInfo->curr_long_term) pAvcInfo->temporal_id = 0;

    // Update cnt
    pAvcInfo->enc_long_term_cnt++;
    pAvcInfo->frm_cnt++;
}
