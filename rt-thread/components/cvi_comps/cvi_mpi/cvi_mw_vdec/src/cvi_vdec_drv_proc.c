#include <base_ctx.h>
#include <cvi_comm_vdec.h>

#include "cvi_vdec.h"
#include "vdec.h"
#include "venc.h"
#include "vpuconfig.h"

extern vdec_context *vdec_handle;

uint32_t MaxVdecChnNum = VDEC_MAX_CHN_NUM;

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

static int vdec_proc_show()
{
    printf("Module: [VDEC] \n");

    if (vdec_handle != NULL)
    {
        int               idx;
        VDEC_MOD_PARAM_S *pVdecModParam = &vdec_handle->g_stModParam;

        printf(
            "VdecMaxChnNum: %u\t MiniBufMode: %u\t enVdecVBSource: %d\t "
            "ParallelMode: %u\n",
            MaxVdecChnNum, pVdecModParam->u32MiniBufMode,
            pVdecModParam->enVdecVBSource, pVdecModParam->u32ParallelMode);

        printf("MaxPicWidth: %u\t MaxPicHeight: %u\t MaxSliceNum: %u",
               pVdecModParam->stVideoModParam.u32MaxPicWidth,
               pVdecModParam->stVideoModParam.u32MaxPicHeight,
               pVdecModParam->stVideoModParam.u32MaxSliceNum);
        printf("\t VdhMsgNum: %u\t VdhBinSize: %u\t VdhExtMemLevel: %u",
               pVdecModParam->stVideoModParam.u32VdhMsgNum,
               pVdecModParam->stVideoModParam.u32VdhBinSize,
               pVdecModParam->stVideoModParam.u32VdhExtMemLevel);
        printf("\t MaxJpegeWidth: %u\t MaxJpegeHeight: %u",
               pVdecModParam->stPictureModParam.u32MaxPicWidth,
               pVdecModParam->stPictureModParam.u32MaxPicHeight);
        printf(
            "\t SupportProgressive: %d\t DynamicAllocate: %d\t CapStrategy: %d\n",
            pVdecModParam->stPictureModParam.bSupportProgressive,
            pVdecModParam->stPictureModParam.bDynamicAllocate,
            pVdecModParam->stPictureModParam.enCapStrategy);

        for (idx = 0; idx < MaxVdecChnNum; idx++)
        {
            if (vdec_handle && vdec_handle->chn_handle[idx] != NULL)
            {
                char cCodecType[16]   = {'\0'};
                char cDecMode[8]      = {'\0'};
                char cOutputOrder[8]  = {'\0'};
                char cCompressMode[8] = {'\0'};
                char cPixelFormat[8]  = {'\0'};

                vdec_chn_context *pChnHandle = vdec_handle->chn_handle[idx];

                VDEC_CHN_ATTR_S  *pstChnAttr  = &pChnHandle->ChnAttr;
                VDEC_CHN_PARAM_S *pstChnParam = &pChnHandle->ChnParam;

                VIDEO_FRAME_S *pstFrame = &pChnHandle->stVideoFrameInfo.stVFrame;

                getCodecTypeStr(pstChnAttr->enType, cCodecType);
                getPixelFormatStr(pstFrame->enPixelFormat, cPixelFormat);

                switch (pstChnParam->stVdecVideoParam.enDecMode)
                {
                case VIDEO_DEC_MODE_IP:
                    strcpy(cDecMode, "IP");
                    break;
                case VIDEO_DEC_MODE_I:
                    strcpy(cDecMode, "I");
                    break;
                case VIDEO_DEC_MODE_BUTT:
                    strcpy(cDecMode, "BUTT");
                    break;
                case VIDEO_DEC_MODE_IPB:
                default:
                    strcpy(cDecMode, "IPB");
                    break;
                }

                switch (pstChnParam->stVdecVideoParam.enOutputOrder)
                {
                case VIDEO_OUTPUT_ORDER_DEC:
                    strcpy(cOutputOrder, "DEC");
                    break;
                case VIDEO_OUTPUT_ORDER_BUTT:
                    strcpy(cOutputOrder, "BUTT");
                    break;
                case VIDEO_OUTPUT_ORDER_DISP:
                default:
                    strcpy(cOutputOrder, "DISP");
                    break;
                }

                switch (pstChnParam->stVdecVideoParam.enCompressMode)
                {
                case COMPRESS_MODE_TILE:
                    strcpy(cCompressMode, "TILE");
                    break;
                case COMPRESS_MODE_LINE:
                    strcpy(cCompressMode, "LINE");
                    break;
                case COMPRESS_MODE_FRAME:
                    strcpy(cCompressMode, "FRAME");
                    break;
                case COMPRESS_MODE_BUTT:
                    strcpy(cCompressMode, "BUTT");
                    break;
                case COMPRESS_MODE_NONE:
                default:
                    strcpy(cCompressMode, "NONE");
                    break;
                }

                printf(
                    "----- CHN COMM ATTR & PARAMS "
                    "--------------------------------------\n");
                printf(
                    "ID: %d\t TYPE: %s\t MaxW: %u\t MaxH: %u\t Width: %u\t Height: %u",
                    idx, cCodecType, MAX_DEC_PIC_WIDTH, MAX_DEC_PIC_HEIGHT,
                    pstFrame->u32Width, pstFrame->u32Height);
                printf("\t Stride: %u\t PixelFormat: %s\t PTS: %llu\t PA: 0x%llx\n",
                       pstFrame->u32Stride[0], cPixelFormat, pstFrame->u64PTS,
                       pstFrame->u64PhyAddr[0]);

                getPixelFormatStr(pstChnParam->enPixelFormat, cPixelFormat);
                printf(
                    "StrInputMode: %s\t StrBufSize: %u\t FrmBufSize: %u\t "
                    "ParamPixelFormat %s",
                    "FRAME/NOBLOCK", pstChnAttr->u32StreamBufSize,
                    pstChnAttr->u32FrameBufSize, cPixelFormat);
                printf("\t FrmBufCnt: %u\t TmvBufSize: %u\n",
                       pstChnAttr->u32FrameBufCnt,
                       pstChnAttr->stVdecVideoAttr.u32TmvBufSize);

                printf(
                    "ID: %d\t DispNum: %d\t DispMode: %s\t SetUserPic: %s\t EnUserPic: "
                    "%s",
                    idx, 2, "PLAYBACK", "N", "N");
                printf("\t Rotation: %u\t PicPoolId: %d\t TmvPoolId: %d\t STATE: %s\n",
                       0, -1, -1, "START");

                printf(
                    "----- CHN VIDEO ATTR & PARAMS "
                    "-------------------------------------\n");
                printf(
                    "ID: %d\t VfmwID: %d\t RefNum: %u\t TemporalMvp: %s\t ErrThr: %d",
                    idx, pstChnAttr->enType == PT_H265 ? 0 : 1,
                    pstChnAttr->stVdecVideoAttr.u32RefFrameNum,
                    pstChnAttr->stVdecVideoAttr.bTemporalMvpEnable ? "Y" : "N",
                    pstChnParam->stVdecVideoParam.s32ErrThreshold);
                printf(
                    "\t DecMode: %s\t OutPutOrder: %s\t Compress: %s\t VideoFormat: %d",
                    cDecMode, cOutputOrder, cCompressMode,
                    pstChnParam->stVdecVideoParam.enVideoFormat);
                printf("\t MaxVPS: %u\t MaxSPS: %u\t MaxPPS: %u\t MaxSlice: %u\n", 0, 0,
                       0, pVdecModParam->stVideoModParam.u32MaxSliceNum);

                printf(
                    "----- CHN PICTURE ATTR & "
                    "PARAMS---------------------------------\n");
                printf("ID: %d\t Alpha: %u\n", idx,
                       pstChnParam->stVdecPictureParam.u32Alpha);
                printf(
                    "-----VDEC CHN "
                    "PERFORMANCE------------------------------------------------\n");
                printf(
                    "ID: %d\t No.SendStreamPerSec: %u\t No.DecFramePerSec: %u\t "
                    "HwDecTime: %llu us\n\n",
                    idx, pChnHandle->stFPS.u32InFPS, pChnHandle->stFPS.u32OutFPS,
                    pChnHandle->stFPS.u64HwTime);
            }
        }
    }

    // printf(
    // 	"\n----- CVITEK Debug Level STATE
    // ----------------------------------------\n"); printf( 	"VdecDebugMask:
    // 0x%X\t VdecStartFrmIdx: %u\t VdecEndFrmIdx: %u\t VdecDumpPath: %s\n",
    // 	tVdecDebugConfig.u32DbgMask, tVdecDebugConfig.u32StartFrmIdx,
    // 	tVdecDebugConfig.u32EndFrmIdx, tVdecDebugConfig.cDumpPath);
    return 0;
}

static void vcodec_vdec_proc_show(int32_t argc, char **argv)
{
    vdec_proc_show();
}
MSH_CMD_EXPORT_ALIAS(vcodec_vdec_proc_show, proc_vdec, vdec info);
