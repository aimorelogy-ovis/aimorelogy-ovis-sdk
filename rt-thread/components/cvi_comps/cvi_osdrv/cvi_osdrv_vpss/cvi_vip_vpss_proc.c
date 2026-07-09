#include <string.h>
#include <stdio.h>
#include <cvi_vip.h>
#include "vpss_debug.h"
#include "vpss_common.h"
#include "scaler.h"
#include "vpss_core.h"
#include "cvi_vpss_ctx.h"


/*************************************************************************
 *	VPSS proc functions
 *************************************************************************/
static void _pixFmt_to_String(enum _PIXEL_FORMAT_E PixFmt, char *str, int len)
{
    switch (PixFmt)
    {
    case PIXEL_FORMAT_RGB_888:
        strncpy(str, "RGB_888", len);
        break;
    case PIXEL_FORMAT_BGR_888:
        strncpy(str, "BGR_888", len);
        break;
    case PIXEL_FORMAT_RGB_888_PLANAR:
        strncpy(str, "RGB_888_PLANAR", len);
        break;
    case PIXEL_FORMAT_BGR_888_PLANAR:
        strncpy(str, "BGR_888_PLANAR", len);
        break;
    case PIXEL_FORMAT_ARGB_1555:
        strncpy(str, "ARGB_1555", len);
        break;
    case PIXEL_FORMAT_ARGB_4444:
        strncpy(str, "ARGB_4444", len);
        break;
    case PIXEL_FORMAT_ARGB_8888:
        strncpy(str, "ARGB_8888", len);
        break;
    case PIXEL_FORMAT_RGB_BAYER_8BPP:
        strncpy(str, "RGB_BAYER_8BPP", len);
        break;
    case PIXEL_FORMAT_RGB_BAYER_10BPP:
        strncpy(str, "RGB_BAYER_10BPP", len);
        break;
    case PIXEL_FORMAT_RGB_BAYER_12BPP:
        strncpy(str, "RGB_BAYER_12BPP", len);
        break;
    case PIXEL_FORMAT_RGB_BAYER_14BPP:
        strncpy(str, "RGB_BAYER_14BPP", len);
        break;
    case PIXEL_FORMAT_RGB_BAYER_16BPP:
        strncpy(str, "RGB_BAYER_16BPP", len);
        break;
    case PIXEL_FORMAT_YUV_PLANAR_422:
        strncpy(str, "YUV_PLANAR_422", len);
        break;
    case PIXEL_FORMAT_YUV_PLANAR_420:
        strncpy(str, "YUV_PLANAR_420", len);
        break;
    case PIXEL_FORMAT_YUV_PLANAR_444:
        strncpy(str, "YUV_PLANAR_444", len);
        break;
    case PIXEL_FORMAT_YUV_400:
        strncpy(str, "YUV_400", len);
        break;
    case PIXEL_FORMAT_HSV_888:
        strncpy(str, "HSV_888", len);
        break;
    case PIXEL_FORMAT_HSV_888_PLANAR:
        strncpy(str, "HSV_888_PLANAR", len);
        break;
    case PIXEL_FORMAT_NV12:
        strncpy(str, "NV12", len);
        break;
    case PIXEL_FORMAT_NV21:
        strncpy(str, "NV21", len);
        break;
    case PIXEL_FORMAT_NV16:
        strncpy(str, "NV16", len);
        break;
    case PIXEL_FORMAT_NV61:
        strncpy(str, "NV61", len);
        break;
    case PIXEL_FORMAT_YUYV:
        strncpy(str, "YUYV", len);
        break;
    case PIXEL_FORMAT_UYVY:
        strncpy(str, "UYVY", len);
        break;
    case PIXEL_FORMAT_YVYU:
        strncpy(str, "YVYU", len);
        break;
    case PIXEL_FORMAT_VYUY:
        strncpy(str, "VYUY", len);
        break;
    case PIXEL_FORMAT_FP32_C1:
        strncpy(str, "FP32_C1", len);
        break;
    case PIXEL_FORMAT_FP32_C3_PLANAR:
        strncpy(str, "FP32_C3_PLANAR", len);
        break;
    case PIXEL_FORMAT_INT32_C1:
        strncpy(str, "INT32_C1", len);
        break;
    case PIXEL_FORMAT_INT32_C3_PLANAR:
        strncpy(str, "INT32_C3_PLANAR", len);
        break;
    case PIXEL_FORMAT_UINT32_C1:
        strncpy(str, "UINT32_C1", len);
        break;
    case PIXEL_FORMAT_UINT32_C3_PLANAR:
        strncpy(str, "UINT32_C3_PLANAR", len);
        break;
    case PIXEL_FORMAT_BF16_C1:
        strncpy(str, "BF16_C1", len);
        break;
    case PIXEL_FORMAT_BF16_C3_PLANAR:
        strncpy(str, "BF16_C3_PLANAR", len);
        break;
    case PIXEL_FORMAT_INT16_C1:
        strncpy(str, "INT16_C1", len);
        break;
    case PIXEL_FORMAT_INT16_C3_PLANAR:
        strncpy(str, "INT16_C3_PLANAR", len);
        break;
    case PIXEL_FORMAT_UINT16_C1:
        strncpy(str, "UINT16_C1", len);
        break;
    case PIXEL_FORMAT_UINT16_C3_PLANAR:
        strncpy(str, "UINT16_C3_PLANAR", len);
        break;
    case PIXEL_FORMAT_INT8_C1:
        strncpy(str, "INT8_C1", len);
        break;
    case PIXEL_FORMAT_INT8_C3_PLANAR:
        strncpy(str, "INT8_C3_PLANAR", len);
        break;
    case PIXEL_FORMAT_UINT8_C1:
        strncpy(str, "UINT8_C1", len);
        break;
    case PIXEL_FORMAT_UINT8_C3_PLANAR:
        strncpy(str, "UINT8_C3_PLANAR", len);
        break;
    default:
        strncpy(str, "Unknown Fmt", len);
        break;
    }
}

static int vpss_ctx_proc_show(void)
{
    int                   i, j;
    char                  c[32];
    struct cvi_vip_dev   *bdev         = vpss_get_dev();
    bool                  isSingleMode = bdev->img_vdev[CVI_VIP_IMG_D].sc_bounding == CVI_VIP_IMG_2_SC_NONE ? true : false;
    struct cvi_vpss_ctx **pVpssCtx     = vpss_get_shdw_ctx();

    // Module Param
    printf("\n-------------------------------MODULE PARAM-------------------------------\n");
    printf("%25s%25s\n", "vpss_vb_source", "vpss_split_node_num");
    printf("%18d%25d\n", 0, 1);
    printf("\n-------------------------------VPSS MODE----------------------------------\n");
    printf("%25s%15s%15s\n", "vpss_mode", "dev0", "dev1");
    printf("%25s%15s%15s\n", isSingleMode ? "single" : "dual", isSingleMode ? "N" : (bdev->img_vdev[CVI_VIP_IMG_D].is_online_from_isp ? "input_isp" : "input_mem"),
           bdev->img_vdev[CVI_VIP_IMG_V].is_online_from_isp ? "input_isp" : "input_mem");

    // VPSS GRP ATTR
    printf("\n-------------------------------VPSS GRP ATTR------------------------------\n");
    printf("%10s%10s%10s%20s%10s%10s%5s\n", "GrpID", "MaxW", "MaxH", "PixFmt",
           "SrcFRate", "DstFRate", "dev");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            memset(c, 0, sizeof(c));
            _pixFmt_to_String(pVpssCtx[i]->stGrpAttr.enPixelFormat, c, sizeof(c));

            printf("%8s%2d%10d%10d%20s%10d%10d%5d\n",
                   "#",
                   i,
                   pVpssCtx[i]->stGrpAttr.u32MaxW,
                   pVpssCtx[i]->stGrpAttr.u32MaxH,
                   c,
                   pVpssCtx[i]->stGrpAttr.stFrameRate.s32SrcFrameRate,
                   pVpssCtx[i]->stGrpAttr.stFrameRate.s32DstFrameRate,
                   pVpssCtx[i]->stGrpAttr.u8VpssDev);
        }
    }

    //VPSS CHN ATTR
    printf("\n-------------------------------VPSS CHN ATTR------------------------------\n");
    printf("%10s%10s%10s%10s%10s%10s%10s\n",
           "GrpID", "PhyChnID", "Enable", "MirrorEn", "FlipEn", "SrcFRate", "DstFRate");
    printf("%10s%10s%10s%10s%10s%10s%10s\n",
           "Depth", "Aspect", "videoX", "videoY", "videoW", "videoH", "BgColor");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            for (j = 0; j < pVpssCtx[i]->chnNum; ++j)
            {
                if (!pVpssCtx[i]->stChnCfgs[j].isEnabled)
                    continue;
                CVI_S32 X, Y;
                CVI_U32 W, H;

                memset(c, 0, sizeof(c));
                if (pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.enMode == ASPECT_RATIO_NONE)
                    strncpy(c, "NONE", sizeof(c));
                else if (pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.enMode == ASPECT_RATIO_AUTO)
                    strncpy(c, "AUTO", sizeof(c));
                else if (pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.enMode == ASPECT_RATIO_MANUAL)
                    strncpy(c, "MANUAL", sizeof(c));
                else
                    strncpy(c, "Invalid", sizeof(c));

                if (pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.enMode == ASPECT_RATIO_MANUAL)
                {
                    X = pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.stVideoRect.s32X;
                    Y = pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.stVideoRect.s32Y;
                    W = pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.stVideoRect.u32Width;
                    H = pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.stVideoRect.u32Height;
                }
                else
                {
                    X = Y = 0;
                    W = H = 0;
                }

                printf("%8s%2d%8s%2d%10s%10s%10s%10d%10d\n%10d%10s%10d%10d%10d%10d%#10x\n",
                       "#",
                       i,
                       "#",
                       j,
                       (pVpssCtx[i]->stChnCfgs[j].isEnabled) ? "Y" : "N",
                       (pVpssCtx[i]->stChnCfgs[j].stChnAttr.bMirror) ? "Y" : "N",
                       (pVpssCtx[i]->stChnCfgs[j].stChnAttr.bFlip) ? "Y" : "N",
                       pVpssCtx[i]->stChnCfgs[j].stChnAttr.stFrameRate.s32SrcFrameRate,
                       pVpssCtx[i]->stChnCfgs[j].stChnAttr.stFrameRate.s32DstFrameRate,
                       pVpssCtx[i]->stChnCfgs[j].stChnAttr.u32Depth,
                       c,
                       X,
                       Y,
                       W,
                       H,
                       pVpssCtx[i]->stChnCfgs[j].stChnAttr.stAspectRatio.u32BgColor);
            }
        }
    }

    // VPSS GRP CROP INFO
    printf("\n-------------------------------VPSS GRP CROP INFO-------------------------\n");
    printf("%10s%10s%10s%10s%10s%10s%10s\n",
           "GrpID", "CropEn", "CoorType", "CoorX", "CoorY", "Width", "Height");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            if (!pVpssCtx[i]->stGrpCropInfo.bEnable)
                continue;
            printf("%8s%2d%10s%10s%10d%10d%10d%10d\n",
                   "#",
                   i,
                   (pVpssCtx[i]->stGrpCropInfo.bEnable) ? "Y" : "N",
                   (pVpssCtx[i]->stGrpCropInfo.enCropCoordinate == VPSS_CROP_RATIO_COOR) ? "RAT" : "ABS",
                   pVpssCtx[i]->stGrpCropInfo.stCropRect.s32X,
                   pVpssCtx[i]->stGrpCropInfo.stCropRect.s32Y,
                   pVpssCtx[i]->stGrpCropInfo.stCropRect.u32Width,
                   pVpssCtx[i]->stGrpCropInfo.stCropRect.u32Height);
        }
    }

    // VPSS CHN CROP INFO
    printf("\n-------------------------------VPSS CHN CROP INFO-------------------------\n");
    printf("%10s%10s%10s%10s%10s%10s%10s%10s\n",
           "GrpID", "ChnID", "CropEn", "CoorType", "CoorX", "CoorY", "Width", "Height");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            for (j = 0; j < pVpssCtx[i]->chnNum; ++j)
            {
                if (!pVpssCtx[i]->stChnCfgs[j].isEnabled || !pVpssCtx[i]->stChnCfgs[j].stCropInfo.bEnable)
                    continue;
                printf("%8s%2d%8s%2d%10s%10s%10d%10d%10d%10d\n",
                       "#",
                       i,
                       "#",
                       j,
                       (pVpssCtx[i]->stChnCfgs[j].stCropInfo.bEnable) ? "Y" : "N",
                       (pVpssCtx[i]->stChnCfgs[j].stCropInfo.enCropCoordinate
                        == VPSS_CROP_RATIO_COOR)
                           ? "RAT"
                           : "ABS",
                       pVpssCtx[i]->stChnCfgs[j].stCropInfo.stCropRect.s32X,
                       pVpssCtx[i]->stChnCfgs[j].stCropInfo.stCropRect.s32Y,
                       pVpssCtx[i]->stChnCfgs[j].stCropInfo.stCropRect.u32Width,
                       pVpssCtx[i]->stChnCfgs[j].stCropInfo.stCropRect.u32Height);
            }
        }
    }

    // VPSS GRP WORK STATUS
    printf("\n-------------------------------VPSS GRP WORK STATUS-----------------------\n");
    printf("%10s%10s%10s%20s%10s%20s%20s%20s%20s\n",
           "GrpID", "RecvCnt", "LostCnt", "StartFailCnt", "bStart",
           "CostTime(us)", "MaxCostTime(us)",
           "HwCostTime(us)", "HwMaxCostTime(us)");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            printf("%8s%2d%10d%10d%20d%10s%20d%20d%20d%20d\n",
                   "#",
                   i,
                   pVpssCtx[i]->stGrpWorkStatus.u32RecvCnt,
                   pVpssCtx[i]->stGrpWorkStatus.u32LostCnt,
                   pVpssCtx[i]->stGrpWorkStatus.u32StartFailCnt,
                   (pVpssCtx[i]->isStarted) ? "Y" : "N",
                   pVpssCtx[i]->stGrpWorkStatus.u32CostTime,
                   pVpssCtx[i]->stGrpWorkStatus.u32MaxCostTime,
                   pVpssCtx[i]->stGrpWorkStatus.u32HwCostTime,
                   pVpssCtx[i]->stGrpWorkStatus.u32HwMaxCostTime);
        }
    }

    // VPSS CHN OUTPUT RESOLUTION
    printf("\n-------------------------------VPSS CHN OUTPUT RESOLUTION-----------------\n");
    printf("%10s%10s%10s%10s%10s%20s%10s%10s%10s\n",
           "GrpID", "ChnID", "Enable", "Width", "Height", "Pixfmt", "Videofmt", "SendOK", "FrameRate");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            for (j = 0; j < pVpssCtx[i]->chnNum; ++j)
            {
                if (!pVpssCtx[i]->stChnCfgs[j].isEnabled)
                    continue;
                memset(c, 0, sizeof(c));
                _pixFmt_to_String(pVpssCtx[i]->stChnCfgs[j].stChnAttr.enPixelFormat, c, sizeof(c));

                printf("%8s%2d%8s%2d%10s%10d%10d%20s%10s%10d%10d\n",
                       "#",
                       i,
                       "#",
                       j,
                       (pVpssCtx[i]->stChnCfgs[j].isEnabled) ? "Y" : "N",
                       pVpssCtx[i]->stChnCfgs[j].stChnAttr.u32Width,
                       pVpssCtx[i]->stChnCfgs[j].stChnAttr.u32Height,
                       c,
                       (pVpssCtx[i]->stChnCfgs[j].stChnAttr.enVideoFormat
                        == VIDEO_FORMAT_LINEAR)
                           ? "LINEAR"
                           : "UNKNOWN",
                       pVpssCtx[i]->stChnCfgs[j].stChnWorkStatus.u32SendOk,
                       pVpssCtx[i]->stChnCfgs[j].stChnWorkStatus.u32RealFrameRate);
            }
        }
    }

    // VPSS CHN ROTATE INFO
    printf("\n-------------------------------VPSS CHN ROTATE INFO-----------------------\n");
    printf("%10s%10s%10s\n", "GrpID", "ChnID", "Rotate");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            for (j = 0; j < pVpssCtx[i]->chnNum; ++j)
            {
                if (!pVpssCtx[i]->stChnCfgs[j].isEnabled)
                    continue;
                memset(c, 0, sizeof(c));
                if (pVpssCtx[i]->stChnCfgs[j].enRotation == ROTATION_0)
                    strncpy(c, "0", sizeof(c));
                else if (pVpssCtx[i]->stChnCfgs[j].enRotation == ROTATION_90)
                    strncpy(c, "90", sizeof(c));
                else if (pVpssCtx[i]->stChnCfgs[j].enRotation == ROTATION_180)
                    strncpy(c, "180", sizeof(c));
                else if (pVpssCtx[i]->stChnCfgs[j].enRotation == ROTATION_270)
                    strncpy(c, "270", sizeof(c));
                else
                    strncpy(c, "Invalid", sizeof(c));

                printf("%8s%2d%8s%2d%10s\n", "#", i, "#", j, c);
            }
        }
    }

    // VPSS CHN LDC INFO
    printf("\n-------------------------------VPSS CHN LDC INFO-----------------------\n");
    printf("%10s%10s%10s%10s%10s%10s\n", "GrpID", "ChnID", "Enable", "Aspect", "XRatio", "YRatio");
    printf("%10s%10s%10s%20s\n", "XYRatio", "XOffset", "YOffset", "DistortionRatio");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            for (j = 0; j < pVpssCtx[i]->chnNum; ++j)
            {
                if (!pVpssCtx[i]->stChnCfgs[j].isEnabled)
                    continue;
                printf("%8s%2d%8s%2d%10s%10s%10d%10d\n%10d%10d%10d%20d\n",
                       "#",
                       i,
                       "#",
                       j,
                       (pVpssCtx[i]->stChnCfgs[j].stLDCAttr.bEnable) ? "Y" : "N",
                       (pVpssCtx[i]->stChnCfgs[j].stLDCAttr.stAttr.bAspect) ? "Y" : "N",
                       pVpssCtx[i]->stChnCfgs[j].stLDCAttr.stAttr.s32XRatio,
                       pVpssCtx[i]->stChnCfgs[j].stLDCAttr.stAttr.s32YRatio,
                       pVpssCtx[i]->stChnCfgs[j].stLDCAttr.stAttr.s32XYRatio,
                       pVpssCtx[i]->stChnCfgs[j].stLDCAttr.stAttr.s32CenterXOffset,
                       pVpssCtx[i]->stChnCfgs[j].stLDCAttr.stAttr.s32CenterYOffset,
                       pVpssCtx[i]->stChnCfgs[j].stLDCAttr.stAttr.s32DistortionRatio);
            }
        }
    }

    //VPSS driver status
    printf("\n------------------------------DRV WORK STATUS------------------------------\n");
    printf("%14s%20s%20s%20s%20s%12s\n", "dev", "UserTrigCnt", "UserTrigFailCnt",
           "IspTrigCnt0", "IspTrigFailCnt0", "IrqCnt0");
    printf("%14s%20s%20s%20s%20s%12s\n", "IspTrigCnt1", "IspTrigFailCnt1", "IrqCnt1",
           "IspTrigCnt2", "IspTrigFailCnt2", "IrqCnt2");
    for (i = 0; i < CVI_VIP_IMG_MAX; ++i)
    {
        printf("%12s%2d%20d%20d%20d%20d%12d\n%14d%20d%20d%20d%20d%12d\n",
               "#",
               i,
               bdev->img_vdev[i].user_trig_cnt,
               bdev->img_vdev[i].user_trig_fail_cnt,
               bdev->img_vdev[i].isp_trig_cnt[0],
               bdev->img_vdev[i].isp_trig_fail_cnt[0],
               bdev->img_vdev[i].irq_cnt[0],
               bdev->img_vdev[i].isp_trig_cnt[1],
               bdev->img_vdev[i].isp_trig_fail_cnt[1],
               bdev->img_vdev[i].irq_cnt[1],
               bdev->img_vdev[i].isp_trig_cnt[2],
               bdev->img_vdev[i].isp_trig_fail_cnt[2],
               bdev->img_vdev[i].irq_cnt[2]);
    }

    // VPSS Slice buffer status
    printf("\n-------------------------------VPSS CHN BUF WRAP ATTR---------------------\n");
    printf("%10s%10s%10s%10s%10s\n", "GrpID", "ChnID", "Enable", "BufLine", "WrapBufSize");
    for (i = 0; i < VPSS_MAX_GRP_NUM; ++i)
    {
        if (pVpssCtx[i] && pVpssCtx[i]->isCreated)
        {
            for (j = 0; j < pVpssCtx[i]->chnNum; ++j)
            {
                if (!pVpssCtx[i]->stChnCfgs[j].isEnabled || !pVpssCtx[i]->stChnCfgs[j].stBufWrap.bEnable)
                    continue;
                printf("%8s%2d%8s%2d%10s%10d%10d\n",
                       "#",
                       i,
                       "#",
                       j,
                       pVpssCtx[i]->stChnCfgs[j].stBufWrap.bEnable ? "Y" : "N",
                       pVpssCtx[i]->stChnCfgs[j].stBufWrap.u32BufLine,
                       pVpssCtx[i]->stChnCfgs[j].stBufWrap.u32WrapBufferSize);
            }
        }
    }

    return 0;
}

static void proc_vpss(CVI_S32 argc, char **argv)
{
    vpss_ctx_proc_show();
}

MSH_CMD_EXPORT(proc_vpss, vpss info);

