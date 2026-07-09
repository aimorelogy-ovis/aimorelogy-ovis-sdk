#include <cvi_comm_vdec.h>
#include <cvi_comm_venc.h>

#include "cvi_venc.h"
#include "venc.h"
#include "vpuconfig.h"

extern venc_context *handle;

uint32_t MaxVencChnNum = VENC_MAX_CHN_NUM;

static void getCodecTypeStr(PAYLOAD_TYPE_E enType, char *pcCodecType)
{
    switch (enType)
    {
    case PT_JPEG:
        strcpy(pcCodecType, "JPEG");
        break;
    case PT_MJPEG:
        strcpy(pcCodecType, "MJPEG");
        break;
    case PT_H264:
        strcpy(pcCodecType, "H264");
        break;
    case PT_H265:
        strcpy(pcCodecType, "H265");
        break;
    default:
        strcpy(pcCodecType, "N/A");
        break;
    }
}

static void getRcModeStr(VENC_RC_MODE_E enRcMode, char *pRcMode)
{
    switch (enRcMode)
    {
    case VENC_RC_MODE_H264CBR:
    case VENC_RC_MODE_H265CBR:
    case VENC_RC_MODE_MJPEGCBR:
        strcpy(pRcMode, "CBR");
        break;
    case VENC_RC_MODE_H264VBR:
    case VENC_RC_MODE_H265VBR:
    case VENC_RC_MODE_MJPEGVBR:
        strcpy(pRcMode, "VBR");
        break;
    case VENC_RC_MODE_H264AVBR:
    case VENC_RC_MODE_H265AVBR:
        strcpy(pRcMode, "AVBR");
        break;
    case VENC_RC_MODE_H264QVBR:
    case VENC_RC_MODE_H265QVBR:
        strcpy(pRcMode, "QVBR");
        break;
    case VENC_RC_MODE_H264FIXQP:
    case VENC_RC_MODE_H265FIXQP:
    case VENC_RC_MODE_MJPEGFIXQP:
        strcpy(pRcMode, "FIXQP");
        break;
    case VENC_RC_MODE_H264QPMAP:
    case VENC_RC_MODE_H265QPMAP:
        strcpy(pRcMode, "QPMAP");
        break;
    default:
        strcpy(pRcMode, "N/A");
        break;
    }
}

static void getGopModeStr(VENC_GOP_MODE_E enGopMode, char *pcGopMode)
{
    switch (enGopMode)
    {
    case VENC_GOPMODE_NORMALP:
        strcpy(pcGopMode, "NORMALP");
        break;
    case VENC_GOPMODE_DUALP:
        strcpy(pcGopMode, "DUALP");
        break;
    case VENC_GOPMODE_SMARTP:
        strcpy(pcGopMode, "SMARTP");
        break;
    case VENC_GOPMODE_ADVSMARTP:
        strcpy(pcGopMode, "ADVSMARTP");
        break;
    case VENC_GOPMODE_BIPREDB:
        strcpy(pcGopMode, "BIPREDB");
        break;
    case VENC_GOPMODE_LOWDELAYB:
        strcpy(pcGopMode, "LOWDELAYB");
        break;
    case VENC_GOPMODE_BUTT:
        strcpy(pcGopMode, "BUTT");
        break;
    default:
        strcpy(pcGopMode, "N/A");
        break;
    }
}

static void getPixelFormatStr(PIXEL_FORMAT_E enPixelFormat,
                              char          *pcPixelFormat)
{
    switch (enPixelFormat)
    {
    case PIXEL_FORMAT_YUV_PLANAR_422:
        strcpy(pcPixelFormat, "YUV422");
        break;
    case PIXEL_FORMAT_YUV_PLANAR_420:
        strcpy(pcPixelFormat, "YUV420");
        break;
    case PIXEL_FORMAT_YUV_400:
        strcpy(pcPixelFormat, "YUV400");
        break;
    case PIXEL_FORMAT_YUV_PLANAR_444:
        strcpy(pcPixelFormat, "YUV444");
        break;
    case PIXEL_FORMAT_NV12:
        strcpy(pcPixelFormat, "NV12");
        break;
    case PIXEL_FORMAT_NV21:
        strcpy(pcPixelFormat, "NV21");
        break;
    default:
        strcpy(pcPixelFormat, "N/A");
        break;
    }
}

static void getFrameRate(VENC_CHN_ATTR_S *pstChnAttr, CVI_U32 *pu32SrcFrameRate,
                         CVI_FR32 *pfr32DstFrameRate)
{
    switch (pstChnAttr->stRcAttr.enRcMode)
    {
    case VENC_RC_MODE_H264CBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH264Cbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH264Cbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H265CBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH265Cbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH265Cbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_MJPEGCBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stMjpegCbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stMjpegCbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H264VBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH264Vbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH264Vbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H265VBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH265Vbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH265Vbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_MJPEGVBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stMjpegVbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stMjpegVbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H264FIXQP:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH264FixQp.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH264FixQp.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H265FIXQP:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH265FixQp.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH265FixQp.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_MJPEGFIXQP:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stMjpegFixQp.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stMjpegFixQp.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H264AVBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH264AVbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH264AVbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H265AVBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH265AVbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH265AVbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H264QVBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH264QVbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH264QVbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H265QVBR:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH265QVbr.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH265QVbr.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H264QPMAP:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH264QpMap.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH264QpMap.fr32DstFrameRate;
        break;
    case VENC_RC_MODE_H265QPMAP:
        *pu32SrcFrameRate  = pstChnAttr->stRcAttr.stH265QpMap.u32SrcFrameRate;
        *pfr32DstFrameRate = pstChnAttr->stRcAttr.stH265QpMap.fr32DstFrameRate;
        break;
    default:
        break;
    }
}

static int venc_proc_show()
{
    printf("Module: [VENC]\n");

    if (handle != NULL)
    {
        int                   idx           = 0;
        CVI_VENC_PARAM_MOD_S *pVencModParam = &handle->ModParam;

        printf("-----MODULE PARAM---------------------------------------------\n");
        printf("VencBufferCache: %u\t FrameBufRecycle: %d\t VencMaxChnNum: %u\n",
               pVencModParam->stVencModParam.u32VencBufferCache,
               pVencModParam->stVencModParam.u32FrameBufRecycle, MaxVencChnNum);
#ifdef CONFIG_ARCH_CV182X
        printf("H264/H265 share singleESBuf %d\n",
               pVencModParam->stH264eModParam.bSingleEsBuf && pVencModParam->stH265eModParam.bSingleEsBuf && pVencModParam->stH264eModParam.u32SingleEsBufSize == pVencModParam->stH265eModParam.u32SingleEsBufSize);
#endif

        for (idx = 0; idx < MaxVencChnNum; idx++)
        {
            if (handle && handle->chn_handle[idx] != NULL)
            {
                char     cCodecType[6]    = {'\0'};
                char     cRcMode[6]       = {'\0'};
                char     cGopMode[10]     = {'\0'};
                char     cPixelFormat[8]  = {'\0'};
                CVI_U32  u32SrcFrameRate  = 0;
                CVI_FR32 fr32DstFrameRate = 0;
                int      roiIdx           = 0;

                VENC_CHN_ATTR_S  *pstChnAttr = handle->chn_handle[idx]->pChnAttr;
                VENC_CHN_PARAM_S *pstChnParam =
                    &handle->chn_handle[idx]->pChnVars->stChnParam;
                VENC_CHN_STATUS_S *pchnStatus =
                    &handle->chn_handle[idx]->pChnVars->chnStatus;
                VENC_STREAM_S     *pstStream = &handle->chn_handle[idx]->pChnVars->stStream;
                VCODEC_PERF_FPS_S *pstFPS    = &handle->chn_handle[idx]->pChnVars->stFPS;
                VIDEO_FRAME_S     *pstVFrame =
                    &handle->chn_handle[idx]->pChnVars->stFrameInfo.stVFrame;

                getCodecTypeStr(pstChnAttr->stVencAttr.enType, cCodecType);
                getRcModeStr(pstChnAttr->stRcAttr.enRcMode, cRcMode);
                getGopModeStr(pstChnAttr->stGopAttr.enGopMode, cGopMode);
                getPixelFormatStr(pstVFrame->enPixelFormat, cPixelFormat);
                getFrameRate(pstChnAttr, &u32SrcFrameRate, &fr32DstFrameRate);

                printf(
                    "-----VENC CHN ATTR "
                    "1---------------------------------------------\n");
                printf("ID: %d\t Width: %u\t Height: %u\t Type: %s\t RcMode: %s", idx,
                       pstChnAttr->stVencAttr.u32PicWidth,
                       pstChnAttr->stVencAttr.u32PicHeight, cCodecType, cRcMode);
                printf("\t EsBufQueueEn: %d\t bIsoSendFrmEn: %d",
                       pstChnAttr->stVencAttr.bEsBufQueueEn,
                       pstChnAttr->stVencAttr.bIsoSendFrmEn);
                printf("\t ByFrame: %s\t Sequence: %u\t LeftBytes: %u\t LeftFrm: %u",
                       pstChnAttr->stVencAttr.bByFrame ? "Y" : "N", pstStream->u32Seq,
                       pchnStatus->u32LeftStreamBytes, pchnStatus->u32LeftStreamFrames);
                printf("\t CurPacks: %u\t GopMode: %s\t Prio: %d\n",
                       pchnStatus->u32CurPacks, cGopMode, pstChnParam->u32Priority);

                printf(
                    "-----VENC CHN ATTR "
                    "2-----------------------------------------------\n");
                printf("VeStr: Y\t SrcFr: %u\t TarFr: %u\t Timeref: %u\t PixFmt: %s",
                       u32SrcFrameRate, fr32DstFrameRate, pstVFrame->u32TimeRef,
                       cPixelFormat);
                printf("\t PicAddr: 0x%llx\t WakeUpFrmCnt: %u\n",
                       pstVFrame->u64PhyAddr[0], pstChnParam->u32PollWakeUpFrmCnt);

// TODO: following info should be amended later
#if 0
				seq_puts(m, "-----VENC JPEGE ATTR ----------------------------------------------\n");
					printf("ID: %d\t RcvMode: %d\t MpfCnt: %d\t Mpf0Width: %d\t Mpf0Height: %d",
					idx, 0, 0, 0, 0);
					printf("\t Mpf1Width: %d\t Mpf1Height: %d\n", 0, 0);

				seq_puts(m, "-----VENC CHN RECEIVE STAT-----------------------------------------\n");
					printf("ID: %d\t Start: %d\t StartEx: %d\t RecvLeft: %d\t EncLeft: %d",
					idx, 0, 0, 0, 0);
				seq_puts(m, "\t JpegEncodeMode: NA\n");

				seq_puts(m, "-----VENC VPSS QUERY----------------------------------------------\n");
					printf("ID: %d\t Query: %d\t QueryOk: %d\t QueryFR: %d\t Invld: %d",
					idx, 0, 0, 0, 0);
					printf("\t Full: %d\t VbFail: %d\t QueryFail: %d\t InfoErr: %d\t Stop: %d\n",
					0, 0, 0, 0, 0);

				seq_puts(m, "-----VENC SEND1---------------------------------------------------\n");
					printf("ID: %d\t VpssSnd: %d\t VInfErr: %d\t OthrSnd: %d\t OInfErr: %d\t Send: %d",
					idx, 0, 0, 0, 0, 0);
					printf("\t Stop: %d\t Full: %d\t CropErr: %d\t DrectSnd: %d\t SizeErr: %d\n",
					0, 0, 0, 0, 0);

				seq_puts(m, "-----VENC SEND2--------------------------------------------------\n");
					printf("ID: %d\t SendVgs: %d\t StartOk: %d\t StartFail: %d\t IntOk: %d",
					idx, 0, 0, 0, 0);
					printf("\t IntFail: %d\t SrcAdd: %d\t SrcSub: %d\t DestAdd: %d\t DestSub: %d\n",
					0, 0, 0, 0, 0);

				seq_puts(m, "-----VENC PIC QUEUE STATE-----------------------------------------\n");
					printf("ID: %d\t Free: %d\t Busy: %d\t Vgs: %d\t BFrame: %d\n",
					idx, 0, 0, 0, 0);

				seq_puts(m, "-----VENC DCF/MPF QUEUE STATE-----------------------------------------\n");
					printf("ID: %d\t ThumbFree: %d\t ThumbBusy: %d",
					idx, 0, 0);
					printf("\t Mpf0Free: %d\t Mpf0Busy: %d\t Mpf1Free: %d\t Mpf1Busy: %d\n",
					0, 0, 0, 0);

				seq_puts(m, "-----VENC CHNL INFO------------------------------------------------\n");
					printf("ID: %d\t Inq: %d\t InqOk: %d\t Start: %d\t StartOk: %d\t Config: %d",
					idx, 0, 0, 0, 0, 0);
					printf("\t VencInt: %d\t ChaResLost: %d\t OverLoad: %d\t RingSkip: %d\t RcSkip: %d\n",
					0, 0, 0, 0, 0);
#endif

                printf(
                    "-----VENC CROP "
                    "INFO------------------------------------------------\n");
                printf(
                    "ID: %d\t CropEn: %s\t StartX: %d\t StartY: %d\t Width: %u\t "
                    "Height: %u\n",
                    idx, pstChnParam->stCropCfg.bEnable ? "Y" : "N",
                    pstChnParam->stCropCfg.stRect.s32X,
                    pstChnParam->stCropCfg.stRect.s32Y,
                    pstChnParam->stCropCfg.stRect.u32Width,
                    pstChnParam->stCropCfg.stRect.u32Height);

                printf(
                    "-----ROI "
                    "INFO-----------------------------------------------------\n");
                for (roiIdx = 0; roiIdx < 8; roiIdx++)
                {
                    if (handle->chn_handle[idx]->pChnVars->stRoiAttr[roiIdx].bEnable)
                    {
                        printf(
                            "ID: %d\t Index: %u\t bRoiEn: %s\t bAbsQp: %s\t Qp: %d", idx,
                            handle->chn_handle[idx]->pChnVars->stRoiAttr[roiIdx].u32Index,
                            handle->chn_handle[idx]->pChnVars->stRoiAttr[roiIdx].bEnable
                                ? "Y"
                                : "N",
                            handle->chn_handle[idx]->pChnVars->stRoiAttr[roiIdx].bAbsQp
                                ? "Y"
                                : "N",
                            handle->chn_handle[idx]->pChnVars->stRoiAttr[roiIdx].s32Qp);
                        printf("\t Width: %u\t Height: %u\t StartX: %d\t StartY: %d\n",
                               handle->chn_handle[idx]
                                   ->pChnVars->stRoiAttr[roiIdx]
                                   .stRect.u32Width,
                               handle->chn_handle[idx]
                                   ->pChnVars->stRoiAttr[roiIdx]
                                   .stRect.u32Height,
                               handle->chn_handle[idx]
                                   ->pChnVars->stRoiAttr[roiIdx]
                                   .stRect.s32X,
                               handle->chn_handle[idx]
                                   ->pChnVars->stRoiAttr[roiIdx]
                                   .stRect.s32Y);
                    }
                }

#if 0
				seq_puts(m, "-----VENC STREAM STATE---------------------------------------------\n");
					printf("ID: %d\t FreeCnt: %d\t BusyCnt: %d\t UserCnt: %d\t UserGet: %d",
					idx, 0, 0, 0, 0);
					printf("\t UserRls: %d\t GetTimes: %d\t Interval: %d\t FrameRate: %d\n",
					0, 0, 0, 0);
#endif

                printf(
                    "-----VENC PTS "
                    "STATE------------------------------------------------\n");
                printf("ID: %d\t RcvFirstFrmPts: %llu\t RcvFrmPts: %llu\n", idx, 0LL,
                       pstVFrame->u64PTS);

                printf(
                    "-----VENC CHN "
                    "PERFORMANCE------------------------------------------------\n");
                printf(
                    "ID: %d\t No.SendFramePerSec: %u\t No.EncFramePerSec: %u\t "
                    "HwEncTime: %llu us\n\n",
                    idx, pstFPS->u32InFPS, pstFPS->u32OutFPS, pstFPS->u64HwTime);
            }
        }
    }

    // printf(
    // 	"\n----- CVITEK Debug Level STATE
    // ----------------------------------------\n"); printf( 	"VencDebugMask:
    // 0x%X\t VencStartFrmIdx: %u\t VencEndFrmIdx: %u\t VencDumpPath: %s\t ",
    // 	tVencDebugConfig.u32DbgMask, tVencDebugConfig.u32StartFrmIdx,
    // 	tVencDebugConfig.u32EndFrmIdx, tVencDebugConfig.cDumpPath);
    // printf( "VencNoDataTimeout: %u\n",
    // 	   tVencDebugConfig.u32NoDataTimeout);

    return 0;
}

static int h265e_proc_show()
{
    int                   idx = 0;
    CVI_VENC_PARAM_MOD_S *pVencModParam;

    printf("Module: [H265E]\n");

    if (handle == NULL) return 0;

    pVencModParam = &handle->ModParam;

    printf(
        "-----MODULE PARAM-------------------------------------------------\n");
    printf("OnePack: %u\t H265eVBSource: %d\t PowerSaveEn: %u",
           pVencModParam->stH265eModParam.u32OneStreamBuffer,
           pVencModParam->stH265eModParam.enH265eVBSource,
           pVencModParam->stH265eModParam.u32H265ePowerSaveEn);
    printf("\t MiniBufMode: %u\t bQpHstgrmEn: %u\t UserDataMaxLen: %u\n",
           pVencModParam->stH265eModParam.u32H265eMiniBufMode,
           pVencModParam->stH265eModParam.bQpHstgrmEn,
           pVencModParam->stH265eModParam.u32UserDataMaxLen);
    printf("SingleEsBuf: %u\t SingleEsBufSize: %u\t RefreshType: %u\n",
           pVencModParam->stH265eModParam.bSingleEsBuf,
           pVencModParam->stH265eModParam.u32SingleEsBufSize,
           pVencModParam->stH265eModParam.enRefreshType);

    for (idx = 0; idx < MaxVencChnNum; idx++)
    {
        if (handle->chn_handle[idx] != NULL && handle->chn_handle[idx]->pChnAttr->stVencAttr.enType == PT_H265)
        {
            char              cGopMode[10] = {'\0'};
            VENC_CHN_ATTR_S  *pstChnAttr   = handle->chn_handle[idx]->pChnAttr;
            VENC_CHN_PARAM_S *pstChnParam =
                &handle->chn_handle[idx]->pChnVars->stChnParam;
            VENC_REF_PARAM_S *pstRefParam = &handle->chn_handle[idx]->refParam;

            getGopModeStr(pstChnAttr->stGopAttr.enGopMode, cGopMode);

            printf(
                "-----CHN "
                "ATTR-----------------------------------------------------\n");
            printf("ID: %d\t MaxWidth: %u\t MaxHeight: %u\t Width: %u\t Height: %u",
                   idx, pstChnAttr->stVencAttr.u32MaxPicWidth,
                   pstChnAttr->stVencAttr.u32MaxPicHeight,
                   pstChnAttr->stVencAttr.u32PicWidth,
                   pstChnAttr->stVencAttr.u32PicHeight);
            printf(
                "\t C2GEn: %d\t BufSize: %u\t ByFrame: %d\t GopMode: %s\t MaxStrCnt: "
                "%u\n",
                pstChnParam->bColor2Grey, pstChnAttr->stVencAttr.u32BufSize,
                pstChnAttr->stVencAttr.bByFrame, cGopMode,
                pstChnParam->u32MaxStrmCnt);

            printf(
                "-----RefParam INFO---------------------------------------------\n");
            printf(
                "ID: %d\t EnPred: %s\t Base: %u\t Enhance: %u\t RcnRefShareBuf: %u\n",
                idx, pstRefParam->bEnablePred ? "Y" : "N", pstRefParam->u32Base,
                pstRefParam->u32Enhance,
                pstChnAttr->stVencAttr.stAttrH265e.bRcnRefShareBuf);

            printf("-----Syntax INFO---------------------------------------------\n");
            printf("ID: %d\t Profile: Main\n", idx);
        }
    }
    return 0;
}

static int h264e_proc_show()
{
    static const char * const prifle[] = {"Base", "Main", "High", "Svc-t", "Err"};
    int                       idx      = 0;

    CVI_VENC_PARAM_MOD_S *pVencModParam;
    CVI_U32               u32Profile;

    printf("Module: [H264E]\n");

    if (handle == NULL) return 0;

    pVencModParam = &handle->ModParam;

    printf(
        "-----MODULE PARAM-------------------------------------------------\n");
    printf("OnePack: %u\t H264eVBSource: %d\t PowerSaveEn: %u",
           pVencModParam->stH264eModParam.u32OneStreamBuffer,
           pVencModParam->stH264eModParam.enH264eVBSource,
           pVencModParam->stH264eModParam.u32H264ePowerSaveEn);
    printf("\t MiniBufMode: %u\t QpHstgrmEn: %u\t UserDataMaxLen: %u\n",
           pVencModParam->stH264eModParam.u32H264eMiniBufMode,
           pVencModParam->stH264eModParam.bQpHstgrmEn,
           pVencModParam->stH264eModParam.u32UserDataMaxLen);
    printf("SingleEsBuf: %u\t SingleEsBufSize: %u\n",
           pVencModParam->stH264eModParam.bSingleEsBuf,
           pVencModParam->stH264eModParam.u32SingleEsBufSize);

    for (idx = 0; idx < MaxVencChnNum; idx++)
    {
        if (handle->chn_handle[idx] != NULL && handle->chn_handle[idx]->pChnAttr->stVencAttr.enType == PT_H264)
        {
            char              cGopMode[10] = {'\0'};
            VENC_CHN_ATTR_S  *pstChnAttr   = handle->chn_handle[idx]->pChnAttr;
            VENC_CHN_PARAM_S *pstChnParam =
                &handle->chn_handle[idx]->pChnVars->stChnParam;
            VENC_REF_PARAM_S *pstRefParam = &handle->chn_handle[idx]->refParam;

            getGopModeStr(pstChnAttr->stGopAttr.enGopMode, cGopMode);

            printf(
                "-----CHN "
                "ATTR-----------------------------------------------------\n");
            printf("ID: %d\t MaxWidth: %u\t MaxHeight: %u\t Width: %u\t Height: %u",
                   idx, pstChnAttr->stVencAttr.u32MaxPicWidth,
                   pstChnAttr->stVencAttr.u32MaxPicHeight,
                   pstChnAttr->stVencAttr.u32PicWidth,
                   pstChnAttr->stVencAttr.u32PicHeight);
            printf(
                "\t C2GEn: %d\t BufSize: %u\t ByFrame: %d\t GopMode: %s\t MaxStrCnt: "
                "%u\n",
                pstChnParam->bColor2Grey, pstChnAttr->stVencAttr.u32BufSize,
                pstChnAttr->stVencAttr.bByFrame, cGopMode,
                pstChnParam->u32MaxStrmCnt);

            printf(
                "-----RefParam INFO---------------------------------------------\n");
            printf(
                "ID: %d\t EnPred: %s\t Base: %u\t Enhance: %u\t RcnRefShareBuf: %u\n",
                idx, pstRefParam->bEnablePred ? "Y" : "N", pstRefParam->u32Base,
                pstRefParam->u32Enhance,
                pstChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf);

            printf("-----Syntax INFO---------------------------------------------\n");
            u32Profile = pstChnAttr->stVencAttr.u32Profile;
            if (u32Profile >= 4) u32Profile = 4;

            printf("ID: %d\t Profile: %s\n", idx, prifle[u32Profile]);
        }
    }
    return 0;
}

static int jpege_proc_show()
{
    int                   idx = 0;
    CVI_VENC_PARAM_MOD_S *pVencModParam;

    printf("Module: [JPEGE]\n");

    if (handle == NULL) return 0;

    pVencModParam = &handle->ModParam;

    printf(
        "-----MODULE PARAM-------------------------------------------------\n");
    printf("OnePack: %u\t JpegeMiniBufMode: %d",
           pVencModParam->stJpegeModParam.u32OneStreamBuffer,
           pVencModParam->stJpegeModParam.u32JpegeMiniBufMode);
    printf("\t JpegClearStreamBuf: %u\t JpegeDeringMode: %u\n",
           pVencModParam->stJpegeModParam.u32JpegClearStreamBuf, 0);
    printf("SingleEsBuf: %u\t SingleEsBufSize: %u\t JpegeFormat: %u\n",
           pVencModParam->stJpegeModParam.bSingleEsBuf,
           pVencModParam->stJpegeModParam.u32SingleEsBufSize,
           pVencModParam->stJpegeModParam.enJpegeFormat);
    printf("JpegMarkerOrder: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
           pVencModParam->stJpegeModParam.JpegMarkerOrder[0],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[1],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[2],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[3],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[4],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[5],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[6],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[7],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[8],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[9],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[10],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[11],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[12],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[13],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[14],
           pVencModParam->stJpegeModParam.JpegMarkerOrder[15]);

    for (idx = 0; idx < MaxVencChnNum; idx++)
    {
        if (handle->chn_handle[idx] != NULL && (handle->chn_handle[idx]->pChnAttr->stVencAttr.enType == PT_JPEG || handle->chn_handle[idx]->pChnAttr->stVencAttr.enType == PT_MJPEG))
        {
            char    cGopMode[10] = {'\0'};
            char    cPicType[8]  = {'\0'};
            CVI_U32 u32Qfactor   = 0;

            VENC_CHN_ATTR_S  *pstChnAttr = handle->chn_handle[idx]->pChnAttr;
            VENC_CHN_PARAM_S *pstChnParam =
                &handle->chn_handle[idx]->pChnVars->stChnParam;
            VIDEO_FRAME_S *pstVFrame =
                &handle->chn_handle[idx]->pChnVars->stFrameInfo.stVFrame;

            getGopModeStr(pstChnAttr->stGopAttr.enGopMode, cGopMode);
            getPixelFormatStr(pstVFrame->enPixelFormat, cPicType);

            if (pstChnAttr->stVencAttr.enType == PT_JPEG)
            {
                u32Qfactor = handle->chn_handle[idx]->pChnVars->stJpegParam.u32Qfactor;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGFIXQP)
            {
                u32Qfactor = pstChnAttr->stRcAttr.stMjpegFixQp.u32Qfactor;
            }

            printf(
                "-----CHN "
                "ATTR-----------------------------------------------------\n");
            printf("ID: %d\t bMjpeg: %s\t PicType: %s\t MaxWidth: %u\t MaxHeight: %u",
                   idx, pstChnAttr->stVencAttr.enType == PT_MJPEG ? "Y" : "N",
                   cPicType, pstChnAttr->stVencAttr.u32MaxPicWidth,
                   pstChnAttr->stVencAttr.u32MaxPicHeight);
            printf("\t Width: %u\t Height: %u\t BufSize: %u\t ByFrm: %d",
                   pstChnAttr->stVencAttr.u32PicWidth,
                   pstChnAttr->stVencAttr.u32PicHeight,
                   pstChnAttr->stVencAttr.u32BufSize,
                   pstChnAttr->stVencAttr.bByFrame);
            printf("\t MCU: %d\t Qfactor: %u\t C2GEn: %d\t DcfEn: %d\n", 1,
                   u32Qfactor, pstChnParam->bColor2Grey, 0);
        }
    }
    return 0;
}

static int rc_proc_show()
{
    int idx = 0;

    printf("Module: [RC] \n");

    if (handle == NULL) return 0;

    for (idx = 0; idx < MaxVencChnNum; idx++)
    {
        if (handle->chn_handle[idx] != NULL)
        {
            char     cCodecType[16]   = {'\0'};
            char     cRcMode[8]       = {'\0'};
            char     cQpMapMode[8]    = {'\0'};
            char     cGopMode[10]     = {'\0'};
            CVI_U32  u32Gop           = 0;
            CVI_U32  u32StatTime      = 0;
            CVI_U32  u32SrcFrameRate  = 0;
            CVI_FR32 fr32DstFrameRate = 0;
            CVI_U32  u32BitRate       = 0;
            CVI_U32  u32IQp           = 0;
            CVI_U32  u32PQp           = 0;
            CVI_U32  u32BQp           = 0;
            // CVI_U32 u32Qfactor = 0;
            CVI_U32  u32MinIprop   = 0;
            CVI_U32  u32MaxIprop   = 0;
            CVI_U32  u32MaxQp      = 0;
            CVI_U32  u32MinQp      = 0;
            CVI_U32  u32MaxIQp     = 0;
            CVI_U32  u32MinIQp     = 0;
            CVI_BOOL bQpMapEn      = CVI_FALSE;
            CVI_BOOL bVariFpsEn    = CVI_FALSE;
            CVI_S32  s32IPQpDelta  = 0;
            CVI_U32  u32SPInterval = 0;
            CVI_S32  s32SPQpDelta  = 0;
            CVI_U32  u32BgInterval = 0;
            // CVI_S32 s32BgQpDelta = 0;
            CVI_S32              s32ViQpDelta         = 0;
            CVI_U32              u32BFrmNum           = 0;
            CVI_S32              s32BQpDelta          = 0;
            CVI_S32              s32MaxReEncodeTimes  = 0;
            CVI_S32              s32ChangePos         = 0;
            CVI_U32              u32MaxQfactor        = 0;
            CVI_U32              u32MinQfactor        = 0;
            VENC_RC_QPMAP_MODE_E enQpMapMode          = VENC_RC_QPMAP_MODE_BUTT + 1;
            CVI_S32              s32MinStillPercent   = 0;
            CVI_U32              u32MaxStillQP        = 0;
            CVI_U32              u32MinStillPSNR      = 0;
            CVI_U32              u32MinQpDelta        = 0;
            CVI_U32              u32MotionSensitivity = 0;
            CVI_S32              s32AvbrFrmLostOpen   = 0;
            CVI_S32              s32AvbrFrmGap        = 0;

            VENC_CHN_ATTR_S       *pstChnAttr = handle->chn_handle[idx]->pChnAttr;
            VENC_RC_PARAM_S       *pRcParam   = &handle->chn_handle[idx]->rcParam;
            VENC_SUPERFRAME_CFG_S *pstSuperFrmParam =
                &handle->chn_handle[idx]->pChnVars->stSuperFrmParam;

            getCodecTypeStr(pstChnAttr->stVencAttr.enType, cCodecType);
            getGopModeStr(pstChnAttr->stGopAttr.enGopMode, cGopMode);
            getFrameRate(pstChnAttr, &u32SrcFrameRate, &fr32DstFrameRate);

            switch (pstChnAttr->stGopAttr.enGopMode)
            {
            case VENC_GOPMODE_NORMALP:
                s32IPQpDelta = pstChnAttr->stGopAttr.stNormalP.s32IPQpDelta;
                break;
            case VENC_GOPMODE_DUALP:
                u32SPInterval = pstChnAttr->stGopAttr.stDualP.u32SPInterval;
                s32SPQpDelta  = pstChnAttr->stGopAttr.stDualP.s32SPQpDelta;
                s32IPQpDelta  = pstChnAttr->stGopAttr.stDualP.s32IPQpDelta;
                break;
            case VENC_GOPMODE_SMARTP:
                u32BgInterval = pstChnAttr->stGopAttr.stSmartP.u32BgInterval;
                // s32BgQpDelta = pstChnAttr->stGopAttr.stSmartP
                // 		       .s32BgQpDelta;
                s32ViQpDelta = pstChnAttr->stGopAttr.stSmartP.s32ViQpDelta;
                break;
            case VENC_GOPMODE_ADVSMARTP:
                u32BgInterval = pstChnAttr->stGopAttr.stAdvSmartP.u32BgInterval;
                // s32BgQpDelta = pstChnAttr->stGopAttr.stAdvSmartP
                // 		       .s32BgQpDelta;
                s32ViQpDelta = pstChnAttr->stGopAttr.stAdvSmartP.s32ViQpDelta;
                break;
            case VENC_GOPMODE_BIPREDB:
                u32BFrmNum   = pstChnAttr->stGopAttr.stBipredB.u32BFrmNum;
                s32BQpDelta  = pstChnAttr->stGopAttr.stBipredB.s32BQpDelta;
                s32IPQpDelta = pstChnAttr->stGopAttr.stBipredB.s32IPQpDelta;
                break;
            default:
                break;
            }

            if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264CBR)
            {
                strcpy(cRcMode, "CBR");
                u32Gop      = pstChnAttr->stRcAttr.stH264Cbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH264Cbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH264Cbr.u32BitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH264Cbr.bVariFpsEn;

                u32MinIprop         = pRcParam->stParamH264Cbr.u32MinIprop;
                u32MaxIprop         = pRcParam->stParamH264Cbr.u32MaxIprop;
                u32MaxQp            = pRcParam->stParamH264Cbr.u32MaxQp;
                u32MinQp            = pRcParam->stParamH264Cbr.u32MinQp;
                u32MaxIQp           = pRcParam->stParamH264Cbr.u32MaxIQp;
                u32MinIQp           = pRcParam->stParamH264Cbr.u32MinIQp;
                s32MaxReEncodeTimes = pRcParam->stParamH264Cbr.s32MaxReEncodeTimes;
                bQpMapEn            = pRcParam->stParamH264Cbr.bQpMapEn;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265CBR)
            {
                strcpy(cRcMode, "CBR");
                u32Gop      = pstChnAttr->stRcAttr.stH265Cbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH265Cbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH265Cbr.u32BitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH265Cbr.bVariFpsEn;

                u32MinIprop         = pRcParam->stParamH265Cbr.u32MinIprop;
                u32MaxIprop         = pRcParam->stParamH265Cbr.u32MaxIprop;
                u32MaxQp            = pRcParam->stParamH265Cbr.u32MaxQp;
                u32MinQp            = pRcParam->stParamH265Cbr.u32MinQp;
                u32MaxIQp           = pRcParam->stParamH265Cbr.u32MaxIQp;
                u32MinIQp           = pRcParam->stParamH265Cbr.u32MinIQp;
                s32MaxReEncodeTimes = pRcParam->stParamH265Cbr.s32MaxReEncodeTimes;
                bQpMapEn            = pRcParam->stParamH265Cbr.bQpMapEn;
                enQpMapMode         = pRcParam->stParamH265Cbr.enQpMapMode;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGCBR)
            {
                strcpy(cRcMode, "CBR");
                u32StatTime = pstChnAttr->stRcAttr.stMjpegCbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stMjpegCbr.u32BitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stMjpegCbr.bVariFpsEn;

                u32MaxQfactor = pRcParam->stParamMjpegCbr.u32MaxQfactor;
                u32MinQfactor = pRcParam->stParamMjpegCbr.u32MinQfactor;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264VBR)
            {
                strcpy(cRcMode, "VBR");
                u32Gop      = pstChnAttr->stRcAttr.stH264Vbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH264Vbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH264Vbr.u32MaxBitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH264Vbr.bVariFpsEn;

                s32ChangePos        = pRcParam->stParamH264Vbr.s32ChangePos;
                u32MinIprop         = pRcParam->stParamH264Vbr.u32MinIprop;
                u32MaxIprop         = pRcParam->stParamH264Vbr.u32MaxIprop;
                u32MaxQp            = pRcParam->stParamH264Vbr.u32MaxQp;
                u32MinQp            = pRcParam->stParamH264Vbr.u32MinQp;
                u32MaxIQp           = pRcParam->stParamH264Vbr.u32MaxIQp;
                u32MinIQp           = pRcParam->stParamH264Vbr.u32MinIQp;
                bQpMapEn            = pRcParam->stParamH264Vbr.bQpMapEn;
                s32MaxReEncodeTimes = pRcParam->stParamH264Vbr.s32MaxReEncodeTimes;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265VBR)
            {
                strcpy(cRcMode, "VBR");
                u32Gop      = pstChnAttr->stRcAttr.stH265Vbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH265Vbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH265Vbr.u32MaxBitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH265Vbr.bVariFpsEn;

                s32ChangePos        = pRcParam->stParamH265Vbr.s32ChangePos;
                u32MinIprop         = pRcParam->stParamH265Vbr.u32MinIprop;
                u32MaxIprop         = pRcParam->stParamH265Vbr.u32MaxIprop;
                u32MaxQp            = pRcParam->stParamH265Vbr.u32MaxQp;
                u32MinQp            = pRcParam->stParamH265Vbr.u32MinQp;
                u32MaxIQp           = pRcParam->stParamH265Vbr.u32MaxIQp;
                u32MinIQp           = pRcParam->stParamH265Vbr.u32MinIQp;
                bQpMapEn            = pRcParam->stParamH265Vbr.bQpMapEn;
                s32MaxReEncodeTimes = pRcParam->stParamH265Vbr.s32MaxReEncodeTimes;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGVBR)
            {
                strcpy(cRcMode, "VBR");
                u32StatTime = pstChnAttr->stRcAttr.stMjpegVbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stMjpegVbr.u32MaxBitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stMjpegVbr.bVariFpsEn;

                s32ChangePos  = pRcParam->stParamMjpegVbr.s32ChangePos;
                u32MaxQfactor = pRcParam->stParamMjpegVbr.u32MaxQfactor;
                u32MinQfactor = pRcParam->stParamMjpegVbr.u32MinQfactor;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264FIXQP)
            {
                strcpy(cRcMode, "FIXQP");
                u32Gop     = pstChnAttr->stRcAttr.stH264FixQp.u32Gop;
                u32IQp     = pstChnAttr->stRcAttr.stH264FixQp.u32IQp;
                u32PQp     = pstChnAttr->stRcAttr.stH264FixQp.u32PQp;
                u32BQp     = pstChnAttr->stRcAttr.stH264FixQp.u32BQp;
                bVariFpsEn = pstChnAttr->stRcAttr.stH264FixQp.bVariFpsEn;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265FIXQP)
            {
                strcpy(cRcMode, "FIXQP");
                u32Gop     = pstChnAttr->stRcAttr.stH265FixQp.u32Gop;
                u32IQp     = pstChnAttr->stRcAttr.stH265FixQp.u32IQp;
                u32PQp     = pstChnAttr->stRcAttr.stH265FixQp.u32PQp;
                u32BQp     = pstChnAttr->stRcAttr.stH265FixQp.u32BQp;
                bVariFpsEn = pstChnAttr->stRcAttr.stH265FixQp.bVariFpsEn;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGFIXQP)
            {
                strcpy(cRcMode, "FIXQP");
                // u32Qfactor = pstChnAttr->stRcAttr.stMjpegFixQp
                // 		     .u32Qfactor;
                bVariFpsEn = pstChnAttr->stRcAttr.stMjpegFixQp.bVariFpsEn;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264AVBR)
            {
                strcpy(cRcMode, "AVBR");
                u32Gop      = pstChnAttr->stRcAttr.stH264AVbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH264AVbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH264AVbr.u32MaxBitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH264AVbr.bVariFpsEn;

                s32ChangePos         = pRcParam->stParamH264AVbr.s32ChangePos;
                u32MinIprop          = pRcParam->stParamH264AVbr.u32MinIprop;
                u32MaxIprop          = pRcParam->stParamH264AVbr.u32MaxIprop;
                s32MinStillPercent   = pRcParam->stParamH264AVbr.s32MinStillPercent;
                u32MaxStillQP        = pRcParam->stParamH264AVbr.u32MaxStillQP;
                u32MinStillPSNR      = pRcParam->stParamH264AVbr.u32MinStillPSNR;
                u32MaxQp             = pRcParam->stParamH264AVbr.u32MaxQp;
                u32MinQp             = pRcParam->stParamH264AVbr.u32MinQp;
                u32MaxIQp            = pRcParam->stParamH264AVbr.u32MaxIQp;
                u32MinIQp            = pRcParam->stParamH264AVbr.u32MinIQp;
                u32MinQpDelta        = pRcParam->stParamH264AVbr.u32MinQpDelta;
                u32MotionSensitivity = pRcParam->stParamH264AVbr.u32MotionSensitivity;
                s32AvbrFrmLostOpen   = pRcParam->stParamH264AVbr.s32AvbrFrmLostOpen;
                s32AvbrFrmGap        = pRcParam->stParamH264AVbr.s32AvbrFrmGap;
                u32MinStillPSNR      = pRcParam->stParamH264AVbr.s32AvbrPureStillThr;
                bQpMapEn             = pRcParam->stParamH264AVbr.bQpMapEn;
                s32MaxReEncodeTimes  = pRcParam->stParamH264AVbr.s32MaxReEncodeTimes;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265AVBR)
            {
                strcpy(cRcMode, "AVBR");
                u32Gop      = pstChnAttr->stRcAttr.stH265AVbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH265AVbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH265AVbr.u32MaxBitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH265AVbr.bVariFpsEn;

                s32ChangePos         = pRcParam->stParamH265AVbr.s32ChangePos;
                u32MinIprop          = pRcParam->stParamH265AVbr.u32MinIprop;
                u32MaxIprop          = pRcParam->stParamH265AVbr.u32MaxIprop;
                s32MinStillPercent   = pRcParam->stParamH265AVbr.s32MinStillPercent;
                u32MaxStillQP        = pRcParam->stParamH265AVbr.u32MaxStillQP;
                u32MinStillPSNR      = pRcParam->stParamH265AVbr.u32MinStillPSNR;
                u32MaxQp             = pRcParam->stParamH265AVbr.u32MaxQp;
                u32MinQp             = pRcParam->stParamH265AVbr.u32MinQp;
                u32MaxIQp            = pRcParam->stParamH265AVbr.u32MaxIQp;
                u32MinIQp            = pRcParam->stParamH265AVbr.u32MinIQp;
                u32MinQpDelta        = pRcParam->stParamH265AVbr.u32MinQpDelta;
                u32MotionSensitivity = pRcParam->stParamH265AVbr.u32MotionSensitivity;
                s32AvbrFrmLostOpen   = pRcParam->stParamH265AVbr.s32AvbrFrmLostOpen;
                s32AvbrFrmGap        = pRcParam->stParamH265AVbr.s32AvbrFrmGap;
                u32MinStillPSNR      = pRcParam->stParamH265AVbr.s32AvbrPureStillThr;
                bQpMapEn             = pRcParam->stParamH265AVbr.bQpMapEn;
                s32MaxReEncodeTimes  = pRcParam->stParamH265AVbr.s32MaxReEncodeTimes;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264QVBR)
            {
                strcpy(cRcMode, "QVBR");
                u32Gop      = pstChnAttr->stRcAttr.stH264QVbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH264QVbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH264QVbr.u32TargetBitRate;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QVBR)
            {
                strcpy(cRcMode, "QVBR");
                u32Gop      = pstChnAttr->stRcAttr.stH265QVbr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH265QVbr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH265QVbr.u32TargetBitRate;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264QPMAP)
            {
                strcpy(cRcMode, "QPMAP");
                u32Gop      = pstChnAttr->stRcAttr.stH264QpMap.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH264QpMap.u32StatTime;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QPMAP)
            {
                strcpy(cRcMode, "QPMAP");
                u32Gop      = pstChnAttr->stRcAttr.stH265QpMap.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH265QpMap.u32StatTime;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264UBR)
            {
                strcpy(cRcMode, "UBR");
                u32Gop      = pstChnAttr->stRcAttr.stH264Ubr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH264Ubr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH264Ubr.u32BitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH264Ubr.bVariFpsEn;

                u32MinIprop         = pRcParam->stParamH264Ubr.u32MinIprop;
                u32MaxIprop         = pRcParam->stParamH264Ubr.u32MaxIprop;
                u32MaxQp            = pRcParam->stParamH264Ubr.u32MaxQp;
                u32MinQp            = pRcParam->stParamH264Ubr.u32MinQp;
                u32MaxIQp           = pRcParam->stParamH264Ubr.u32MaxIQp;
                u32MinIQp           = pRcParam->stParamH264Ubr.u32MinIQp;
                s32MaxReEncodeTimes = pRcParam->stParamH264Ubr.s32MaxReEncodeTimes;
                bQpMapEn            = pRcParam->stParamH264Ubr.bQpMapEn;
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265UBR)
            {
                strcpy(cRcMode, "UBR");
                u32Gop      = pstChnAttr->stRcAttr.stH265Ubr.u32Gop;
                u32StatTime = pstChnAttr->stRcAttr.stH265Ubr.u32StatTime;
                u32BitRate  = pstChnAttr->stRcAttr.stH265Ubr.u32BitRate;
                bVariFpsEn  = pstChnAttr->stRcAttr.stH265Ubr.bVariFpsEn;

                u32MinIprop         = pRcParam->stParamH265Ubr.u32MinIprop;
                u32MaxIprop         = pRcParam->stParamH265Ubr.u32MaxIprop;
                u32MaxQp            = pRcParam->stParamH265Ubr.u32MaxQp;
                u32MinQp            = pRcParam->stParamH265Ubr.u32MinQp;
                u32MaxIQp           = pRcParam->stParamH265Ubr.u32MaxIQp;
                u32MinIQp           = pRcParam->stParamH265Ubr.u32MinIQp;
                s32MaxReEncodeTimes = pRcParam->stParamH265Ubr.s32MaxReEncodeTimes;
                bQpMapEn            = pRcParam->stParamH265Ubr.bQpMapEn;
                enQpMapMode         = pRcParam->stParamH265Ubr.enQpMapMode;
            }
            else
            {
                strcpy(cRcMode, "N/A");
            }

            switch (enQpMapMode)
            {
            case VENC_RC_QPMAP_MODE_MEANQP:
                strcpy(cQpMapMode, "MEANQP");
                break;
            case VENC_RC_QPMAP_MODE_MINQP:
                strcpy(cQpMapMode, "MINQP");
                break;
            case VENC_RC_QPMAP_MODE_MAXQP:
                strcpy(cQpMapMode, "MAXQP");
                break;
            case VENC_RC_QPMAP_MODE_BUTT:
                strcpy(cQpMapMode, "BUTT");
                break;
            default:
                strcpy(cQpMapMode, "N/A");
                break;
            }

            printf(
                "------BASE PARAMS "
                "1------------------------------------------------------\n");
            printf(
                "ChnId: %d\t Gop: %u\t StatTm: %u\t ViFr: %u\t TrgFr: %u\t ProType: "
                "%s",
                idx, u32Gop, u32StatTime, u32SrcFrameRate, fr32DstFrameRate,
                cCodecType);
            printf(
                "\t RcMode: %s\t Br(kbps): %u\t FluLev: %d\t IQp: %u\t PQp: %u\t "
                "BQp: %u\n",
                cRcMode, u32BitRate, 0, u32IQp, u32PQp, u32BQp);

            printf(
                "------BASE PARAMS "
                "2------------------------------------------------------\n");
            printf("ChnId: %d\t MinQp: %u\t MaxQp: %u\t MinIQp: %u\t MaxIQp: %u", idx,
                   u32MinQp, u32MaxQp, u32MinIQp, u32MaxIQp);
            printf("\t EnableIdr: %d\t bQpMapEn: %d\t QpMapMode: %s\n", 0, bQpMapEn,
                   cQpMapMode);
            printf("u32RowQpDelta: %d\t", pRcParam->u32RowQpDelta);
            printf(
                "InitialDelay: %d\t VariFpsEn: %d\t ThrdLv: %d\t BgEnhanceEn: %d\t "
                "BgDeltaQp: %d\n",
                pRcParam->s32InitialDelay, bVariFpsEn, pRcParam->u32ThrdLv,
                pRcParam->bBgEnhanceEn, pRcParam->s32BgDeltaQp);

            printf(
                "-----GOP MODE "
                "ATTR-------------------------------------------------------\n");
            printf(
                "ChnId: %d\t GopMode: %s\t IpQpDelta: %d\t SPInterval: %u\t "
                "SPQpDelta: %d",
                idx, cGopMode, s32IPQpDelta, u32SPInterval, s32SPQpDelta);
            printf("\t BFrmNum: %u\t BQpDelta: %d\t BgInterval: %u\t ViQpDelta: %d\n",
                   u32BFrmNum, s32BQpDelta, u32BgInterval, s32ViQpDelta);

            if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264CBR || pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265CBR)
            {
                printf(
                    "-----SUPER FRAME PARAM "
                    "-------------------------------------------\n");
                printf("ChnId: %d\t FrmMode: %d\t IFrmBitsThr: %d\t PFrmBitsThr: %d\n",
                       idx, pstSuperFrmParam->enSuperFrmMode,
                       pstSuperFrmParam->u32SuperIFrmBitsThr,
                       pstSuperFrmParam->u32SuperPFrmBitsThr);

                printf(
                    "-----RUN CBR PARAM -------------------------------------------\n");
                printf(
                    "ChnId: %d\t MinIprop: %u\t MaxIprop: %u\t MaxQp: %u\t MinQp: %u",
                    idx, u32MinIprop, u32MaxIprop, u32MaxQp, u32MinQp);
                printf("\t MaxIQp: %u\t MinIQp: %u\t MaxReEncTimes: %d\n", u32MaxIQp,
                       u32MinIQp, s32MaxReEncodeTimes);
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGCBR)
            {
                printf(
                    "-----RUN CBR PARAM -------------------------------------------\n");
                printf("ChnId: %d\t MaxQfactor: %u\t MinQfactor: %u\n", idx,
                       u32MaxQfactor, u32MinQfactor);
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264VBR || pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265VBR)
            {
                printf(
                    "-----RUN VBR PARAM -------------------------------------------\n");
                printf(
                    "ChnId: %d\t ChgPs: %d\t MinIprop: %u\t MaxIprop: %u\t MaxQp: %u",
                    idx, s32ChangePos, u32MinIprop, u32MaxIprop, u32MaxQp);
                printf("\t MinQp: %u\t MaxIQp: %u\t MinIQp: %u\t MaxReEncTimes: %d\n",
                       u32MinQp, u32MaxIQp, u32MinIQp, s32MaxReEncodeTimes);
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGVBR)
            {
                printf(
                    "-----RUN VBR PARAM -------------------------------------------\n");
                printf("ChnId: %d\t ChgPs: %d\t MaxQfactor: %u\t MinQfactor: %u\n", idx,
                       s32ChangePos, u32MaxQfactor, u32MinQfactor);
            }
            else if (pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264AVBR || pstChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265AVBR)
            {
                printf(
                    "-----RUN AVBR PARAM "
                    "-------------------------------------------\n");
                printf(
                    "ChnId: %d\t ChgPs: %d\t MinIprop: %u\t MaxIprop: %u\t MaxQp: %u",
                    idx, s32ChangePos, u32MinIprop, u32MaxIprop, u32MaxQp);
                printf("\t MinQp: %u\t MaxIQp: %u\t MinIQp: %u\t MaxReEncTimes: %d\n",
                       u32MinQp, u32MaxIQp, u32MinIQp, s32MaxReEncodeTimes);
                printf(
                    "MinStillPercent: %d\t MaxStillQP: %u\t MinStillPSNR: %u\t "
                    "MinQpDelta: %u\n",
                    s32MinStillPercent, u32MaxStillQP, u32MinStillPSNR, u32MinQpDelta);
                printf(
                    "MotionSensitivity: %u\t AvbrFrmLostOpen: %d\t AvbrFrmGap: %d\t "
                    "bQpMapEn: %d\n",
                    u32MotionSensitivity, s32AvbrFrmLostOpen, s32AvbrFrmGap, bQpMapEn);
            }
        }
    }
    return 0;
}

static int codec_proc_show()
{
    int               idx = 0;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;

    printf("Module: [CodecInst] System Build Time [%s-%s]\n", __DATE__, __TIME__);

    if (handle == NULL) return 0;

    for (idx = 0; idx < MaxVencChnNum; idx++)
    {
        pChnHandle = handle->chn_handle[idx];
        if (pChnHandle != NULL)
        {
            pEncCtx = &pChnHandle->encCtx;
            enType  = pChnHandle->pChnAttr->stVencAttr.enType;

            if (pEncCtx->base.ioctl && (enType == PT_H264 || enType == PT_H265))
            {
                // pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_GET_CHN_INFO, pChnHandle);
            }
            else if (pEncCtx->base.ioctl && (enType == PT_JPEG || enType == PT_MJPEG))
            {
                pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_SHOW_CHN_INFO, pChnHandle);
            }
        }
    }

    return 0;
}

static void vcodec_venc_proc_show(int32_t argc, char **argv)
{
    venc_proc_show();
}

static void vcodec_h265e_proc_show(int32_t argc, char **argv)
{
    h265e_proc_show();
}

static void vcodec_h264e_proc_show(int32_t argc, char **argv)
{
    h264e_proc_show();
}

static void vcodec_jpege_proc_show(int32_t argc, char **argv)
{
    jpege_proc_show();
}

static void vcodec_rc_proc_show(int32_t argc, char **argv)
{
    rc_proc_show();
}

static void vcodec_codec_proc_show(int32_t argc, char **argv)
{
    codec_proc_show();
}

MSH_CMD_EXPORT_ALIAS(vcodec_venc_proc_show, proc_venc, show venc info);