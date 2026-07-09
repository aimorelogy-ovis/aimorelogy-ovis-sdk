#include <cvi_vi_ctx.h>
#include <vi_proc.h>
// #include <aos/cli.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VI_PRC_NAME "cvitek/vi"

// base size 2128
#define VI_BUF_SIZE 4096

static void              *vi_shared_mem;
static struct cvi_vi_dev *m_vdev;
/*************************************************************************
 *	VI proc functions
 *************************************************************************/

static int _vi_proc_show(void)
{
    struct cvi_vi_dev *vdev       = m_vdev;
    struct cvi_vi_ctx *pviProcCtx = NULL;
    u8                 i = 0, j = 0, chn = 0;
    char               o[8], p[8];
    u8                 isRGB = 0;
    int                pos   = 0;
    char              *buf   = NULL;

    if (vdev == NULL) {
        return -1;
    }
    pviProcCtx = (struct cvi_vi_ctx *)(vi_shared_mem);

    buf = calloc(1, VI_BUF_SIZE);
    if (!buf) {
        printf("fail to malloc\n");
        return -1;
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------MODULE PARAM-------------------------------------\n");
    pos += sprintf(buf + pos, "\tDetectErrFrame\tDropErrFrame\n");
    pos += sprintf(buf + pos, "\t\t%d\t\t%d\n", pviProcCtx->modParam.s32DetectErrFrame,
                   pviProcCtx->modParam.u32DropErrFrame);

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI MODE------------------------------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tPrerawFE\tPrerawBE\tPostraw\t\tScaler\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            pos += sprintf(buf + pos, "\t%3d\t%7s\t\t%7s\t\t%7s\t\t%7s\n", i,
                           vdev->ctx.isp_pipe_cfg[i].is_offline_preraw ? "offline" : "online",
                           vdev->ctx.is_offline_be ? "offline" : "online",
                           vdev->ctx.is_offline_postraw
                               ? (vdev->ctx.is_slice_buf_on ? "slice" : "offline")
                               : "online",
                           vdev->ctx.isp_pipe_cfg[i].is_offline_scaler ? "offline" : "online");
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI DEV ATTR1-------------------------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tDevEn\tBindPipe\tWidth\tHeight\tIntfM\tWkM\tScanM\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            pos += sprintf(buf + pos, "\t%3d\t%3s\t%4s\t\t%4d\t%4d", i,
                           (pviProcCtx->isDevEnable[i] ? "Y" : "N"), "Y",
                           pviProcCtx->devAttr[i].stSize.u32Width,
                           pviProcCtx->devAttr[i].stSize.u32Height);

            memset(o, 0, 8);
            if (pviProcCtx->devAttr[i].enIntfMode == VI_MODE_BT656 ||
                pviProcCtx->devAttr[i].enIntfMode == VI_MODE_BT601 ||
                pviProcCtx->devAttr[i].enIntfMode == VI_MODE_BT1120_STANDARD ||
                pviProcCtx->devAttr[i].enIntfMode == VI_MODE_BT1120_INTERLEAVED)
                memcpy(o, "BT", sizeof("BT"));
            else if (pviProcCtx->devAttr[i].enIntfMode == VI_MODE_MIPI ||
                     pviProcCtx->devAttr[i].enIntfMode == VI_MODE_MIPI_YUV420_NORMAL ||
                     pviProcCtx->devAttr[i].enIntfMode == VI_MODE_MIPI_YUV420_LEGACY ||
                     pviProcCtx->devAttr[i].enIntfMode == VI_MODE_MIPI_YUV422)
                memcpy(o, "MIPI", sizeof("MIPI"));
            else if (pviProcCtx->devAttr[i].enIntfMode == VI_MODE_LVDS)
                memcpy(o, "LVDS", sizeof("LVDS"));

            memset(p, 0, 8);
            if (pviProcCtx->devAttr[i].enWorkMode == VI_WORK_MODE_1Multiplex)
                memcpy(p, "1MUX", sizeof("1MUX"));
            else if (pviProcCtx->devAttr[i].enWorkMode == VI_WORK_MODE_2Multiplex)
                memcpy(p, "2MUX", sizeof("2MUX"));
            else if (pviProcCtx->devAttr[i].enWorkMode == VI_WORK_MODE_3Multiplex)
                memcpy(p, "3MUX", sizeof("3MUX"));
            else if (pviProcCtx->devAttr[i].enWorkMode == VI_WORK_MODE_4Multiplex)
                memcpy(p, "4MUX", sizeof("4MUX"));
            else
                memcpy(p, "Other", sizeof("Other"));

            pos += sprintf(buf + pos, "\t%4s\t%4s\t%3s\n", o, p,
                           (pviProcCtx->devAttr[i].enScanMode == VI_SCAN_INTERLACED) ? "I" : "P");
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI DEV ATTR2-------------------------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tAD0\tAD1\tAD2\tAD3\tSeq\tDataType\tWDRMode\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            memset(o, 0, 8);
            if (pviProcCtx->devAttr[i].enDataSeq == VI_DATA_SEQ_VUVU)
                memcpy(o, "VUVU", sizeof("VUVU"));
            else if (pviProcCtx->devAttr[i].enDataSeq == VI_DATA_SEQ_UVUV)
                memcpy(o, "UVUV", sizeof("UVUV"));
            else if (pviProcCtx->devAttr[i].enDataSeq == VI_DATA_SEQ_UYVY)
                memcpy(o, "UYVY", sizeof("UYVY"));
            else if (pviProcCtx->devAttr[i].enDataSeq == VI_DATA_SEQ_VYUY)
                memcpy(o, "VYUY", sizeof("VYUY"));
            else if (pviProcCtx->devAttr[i].enDataSeq == VI_DATA_SEQ_YUYV)
                memcpy(o, "YUYV", sizeof("YUYV"));
            else if (pviProcCtx->devAttr[i].enDataSeq == VI_DATA_SEQ_YVYU)
                memcpy(o, "YVYU", sizeof("YVYU"));

            isRGB = (pviProcCtx->devAttr[i].enInputDataType == VI_DATA_TYPE_RGB);

            pos += sprintf(
                buf + pos, "\t%3d\t%1d\t%1d\t%1d\t%1d\t%3s\t%4s\t\t%3s\n", i,
                pviProcCtx->devAttr[i].as32AdChnId[0], pviProcCtx->devAttr[i].as32AdChnId[1],
                pviProcCtx->devAttr[i].as32AdChnId[2], pviProcCtx->devAttr[i].as32AdChnId[3],
                (isRGB) ? "N/A" : o, (isRGB) ? "RGB" : "YUV",
                (vdev->ctx.isp_pipe_cfg[i].is_hdr_on) ? "WDR_2F1" : "None");
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI BIND ATTR-------------------------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tPipeNum\t\tPipeId\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            pos += sprintf(
                buf + pos, "\t%3d\t%3d\t\t%3d\n", i, pviProcCtx->devBindPipeAttr[i].u32Num,
                pviProcCtx->devBindPipeAttr[i].PipeId[pviProcCtx->devBindPipeAttr[i].u32Num]);
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI DEV TIMING ATTR-------------------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tDevTimingEn\tDevFrmRate\tDevWidth\tDevHeight\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            pos += sprintf(buf + pos, "\t%3d\t%5s\t\t%4d\t\t%5d\t\t%5d\n", i,
                           (pviProcCtx->stTimingAttr[i].bEnable) ? "Y" : "N",
                           pviProcCtx->stTimingAttr[i].s32FrmRate,
                           pviProcCtx->devAttr[i].stSize.u32Width,
                           pviProcCtx->devAttr[i].stSize.u32Height);
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI CHN ATTR1-------------------------------------\n");
    pos += sprintf(buf + pos,
                   "\tDevID\tChnID\tWidth\tHeight\tMirror\tFlip\tSrcFRate\tDstFRate\tPixFmt\tVideoF"
                   "mt\tBindPool\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            for (chn = 0, j = 0; j < i; j++) {
                chn += pviProcCtx->devAttr[j].chn_num;
            }

            for (j = 0; j < pviProcCtx->devAttr[i].chn_num; j++, chn++) {
                if (chn >= pviProcCtx->total_chn_num) break;

                memset(o, 0, 8);
                if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422)
                    memcpy(o, "422P", sizeof("422P"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420)
                    memcpy(o, "420P", sizeof("420P"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444)
                    memcpy(o, "444P", sizeof("444P"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_NV12)
                    memcpy(o, "NV12", sizeof("NV12"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_NV21)
                    memcpy(o, "NV21", sizeof("NV21"));

                pos += sprintf(buf + pos,
                               "\t%3d\t%3d\t%4d\t%4d\t%3s\t%2s\t%4d\t\t%4d\t\t%3s\t%6s\t\t%4d\n", i,
                               j, pviProcCtx->chnAttr[chn].stSize.u32Width,
                               pviProcCtx->chnAttr[chn].stSize.u32Height,
                               (pviProcCtx->chnAttr[chn].bMirror) ? "Y" : "N",
                               (pviProcCtx->chnAttr[chn].bFlip) ? "Y" : "N",
                               pviProcCtx->chnAttr[chn].stFrameRate.s32SrcFrameRate,
                               pviProcCtx->chnAttr[chn].stFrameRate.s32DstFrameRate, o, "SDR8",
                               pviProcCtx->chnAttr[chn].u32BindVbPool);
            }
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI CHN ATT2--------------------------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tChnID\tCompressMode\tDepth\tAlign\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            for (chn = 0, j = 0; j < i; j++) {
                chn += pviProcCtx->devAttr[j].chn_num;
            }

            for (j = 0; j < pviProcCtx->devAttr[i].chn_num; j++, chn++) {
                if (chn >= pviProcCtx->total_chn_num) break;
                memset(o, 0, 8);
                if (pviProcCtx->chnAttr[chn].enCompressMode == COMPRESS_MODE_NONE)
                    memcpy(o, "None", sizeof("None"));
                else
                    memcpy(o, "Y", sizeof("Y"));

                pos += sprintf(buf + pos, "\t%3d\t%3d\t%4s\t\t%3d\t%3d\n", i, j, o,
                               pviProcCtx->chnAttr[chn].u32Depth, 32);
            }
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI CHN OUTPUT RESOLUTION-------------------------\n");
    pos += sprintf(
        buf + pos,
        "\tDevID\tChnID\tMirror\tFlip\tWidth\tHeight\tPixFmt\tVideoFmt\tCompressMode\tFrameRate\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            for (chn = 0, j = 0; j < i; j++) {
                chn += pviProcCtx->devAttr[j].chn_num;
            }

            for (j = 0; j < pviProcCtx->devAttr[i].chn_num; j++, chn++) {
                if (chn >= pviProcCtx->total_chn_num) break;

                memset(o, 0, 8);
                if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422)
                    memcpy(o, "422P", sizeof("422P"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420)
                    memcpy(o, "420P", sizeof("420P"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444)
                    memcpy(o, "444P", sizeof("444P"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_NV12)
                    memcpy(o, "NV12", sizeof("NV12"));
                else if (pviProcCtx->chnAttr[chn].enPixelFormat == PIXEL_FORMAT_NV21)
                    memcpy(o, "NV21", sizeof("NV21"));

                memset(p, 0, 8);
                if (pviProcCtx->chnAttr[chn].enCompressMode == COMPRESS_MODE_NONE)
                    memcpy(p, "None", sizeof("None"));
                else
                    memcpy(p, "Y", sizeof("Y"));

                pos +=
                    sprintf(buf + pos, "\t%3d\t%3d\t%3s\t%2s\t%4d\t%4d\t%3s\t%6s\t\t%6s\t\t%5d\n",
                            i, j, (pviProcCtx->chnAttr[chn].bMirror) ? "Y" : "N",
                            (pviProcCtx->chnAttr[chn].bFlip) ? "Y" : "N",
                            pviProcCtx->chnAttr[chn].stSize.u32Width,
                            pviProcCtx->chnAttr[chn].stSize.u32Height, o, "SDR8", p,
                            pviProcCtx->chnAttr[chn].stFrameRate.s32DstFrameRate);
            }
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI CHN ROTATE INFO-------------------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tChnID\tRotate\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            for (chn = 0, j = 0; j < i; j++) {
                chn += pviProcCtx->devAttr[j].chn_num;
            }

            for (j = 0; j < pviProcCtx->devAttr[i].chn_num; j++, chn++) {
                if (chn >= pviProcCtx->total_chn_num) break;

                memset(o, 0, 8);
                if (pviProcCtx->enRotation[chn] == ROTATION_0)
                    memcpy(o, "0", sizeof("0"));
                else if (pviProcCtx->enRotation[chn] == ROTATION_90)
                    memcpy(o, "90", sizeof("90"));
                else if (pviProcCtx->enRotation[chn] == ROTATION_180)
                    memcpy(o, "180", sizeof("180"));
                else if (pviProcCtx->enRotation[chn] == ROTATION_270)
                    memcpy(o, "270", sizeof("270"));
                else
                    memcpy(o, "Invalid", sizeof("Invalid"));

                pos += sprintf(buf + pos, "\t%3d\t%3d\t%3s\n", i, j, o);
            }
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI CHN EARLY INTERRUPT INFO----------------------\n");
    pos += sprintf(buf + pos, "\tDevID\tChnID\tEnable\tLineCnt\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            for (chn = 0, j = 0; j < i; j++) {
                chn += pviProcCtx->devAttr[j].chn_num;
            }

            for (j = 0; j < pviProcCtx->devAttr[i].chn_num; j++, chn++) {
                if (chn >= pviProcCtx->total_chn_num) break;

                pos += sprintf(buf + pos, "\t%3d\t%3d\t%3s\t%4d\n", i, j,
                               pviProcCtx->enEalyInt[chn].bEnable ? "Y" : "N",
                               pviProcCtx->enEalyInt[chn].u32LineCnt);
            }
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI CHN CROP INFO---------------------------------\n");
    pos += sprintf(buf + pos,
                   "\tDevID\tChnID\tCropEn\tCoorType\tCoorX\tCoorY\tWidth\tHeight\tTrimX\tTrimY\tTr"
                   "imWid\tTrimHgt\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            for (chn = 0, j = 0; j < i; j++) {
                chn += pviProcCtx->devAttr[j].chn_num;
            }

            for (j = 0; j < pviProcCtx->devAttr[i].chn_num; j++, chn++) {
                if (chn >= pviProcCtx->total_chn_num) break;

                memset(o, 0, 8);
                if (pviProcCtx->chnCrop[chn].enCropCoordinate == VI_CROP_RATIO_COOR)
                    memcpy(o, "RAT", sizeof("RAT"));
                else
                    memcpy(o, "ABS", sizeof("ABS"));

                pos += sprintf(buf + pos,
                               "\t%3d\t%3d\t%3s\t%5s\t\t%4d\t%4d\t%4d\t%4d\t%4d\t%3d\t%3d\t%4d\n",
                               i, j, pviProcCtx->chnCrop[chn].bEnable ? "Y" : "N", o,
                               pviProcCtx->chnCrop[chn].stCropRect.s32X,
                               pviProcCtx->chnCrop[chn].stCropRect.s32Y,
                               pviProcCtx->chnCrop[chn].stCropRect.u32Width,
                               pviProcCtx->chnCrop[chn].stCropRect.u32Height,
                               pviProcCtx->chnCrop[chn].stCropRect.s32X,
                               pviProcCtx->chnCrop[chn].stCropRect.s32Y,
                               pviProcCtx->chnCrop[chn].stCropRect.u32Width,
                               pviProcCtx->chnCrop[chn].stCropRect.u32Height);
            }
        }
    }

    pos += sprintf(
        buf + pos,
        "\n-------------------------------VI CHN STATUS------------------------------------\n");
    pos += sprintf(
        buf + pos,
        "\tDevID\tChnID\tEnable\tFrameRate\tIntCnt\tRecvPic\tLostFrame\tVbFail\tWidth\tHeight\n");
    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (pviProcCtx->isDevEnable[i]) {
            for (chn = 0, j = 0; j < i; j++) {
                chn += pviProcCtx->devAttr[j].chn_num;
            }

            for (j = 0; j < pviProcCtx->devAttr[i].chn_num; j++, chn++) {
                if (chn >= pviProcCtx->total_chn_num) break;

                pos += sprintf(
                    buf + pos, "\t%3d\t%3d\t%3s\t%5d\t\t%5d\t%5d\t%5d\t\t%5d\t%4d\t%4d\n", i, j,
                    pviProcCtx->chnStatus[chn].bEnable ? "Y" : "N",
                    pviProcCtx->chnStatus[chn].u32FrameRate, pviProcCtx->chnStatus[chn].u32IntCnt,
                    pviProcCtx->chnStatus[chn].u32RecvPic, pviProcCtx->chnStatus[chn].u32LostFrame,
                    pviProcCtx->chnStatus[chn].u32VbFail,
                    pviProcCtx->chnStatus[chn].stSize.u32Width,
                    pviProcCtx->chnStatus[chn].stSize.u32Height);
            }
        }
    }

    printf(buf);
    free(buf);
    return 0;
}

static void vi_proc_show(int32_t argc, char **argv)
{
    _vi_proc_show();
}
MSH_CMD_EXPORT_ALIAS(vi_proc_show,proc_vi,vi info);

int vi_proc_init(struct cvi_vi_dev *_vdev, void *shm)
{
    int rc = 0;

    m_vdev        = _vdev;
    vi_shared_mem = shm;

    return rc;
}

int vi_proc_remove(void)
{
    int rc        = 0;
    m_vdev        = NULL;
    vi_shared_mem = NULL;

    return rc;
}
