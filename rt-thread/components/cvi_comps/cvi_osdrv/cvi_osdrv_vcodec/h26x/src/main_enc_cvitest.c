//------------------------------------------------------------------------------
// File: main.c
//
// Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
//------------------------------------------------------------------------------
#include "stdint.h"
#include "cvi_vcodec_version.h"
#include "main_helper.h"
#include "vpuapi.h"
#include "main_enc_cvitest.h"
#include "cvitest_internal.h"
#include "signal.h"
#include "unistd.h"
#include "vdi.h"
#include "product.h"
#include "cvi_enc_rc.h"
#include "cvi_vcodec_lib.h"
#include "getopt.h"
#include "malloc.h"
#include "cvi_vc_ctrl.h"
#include "cvi_enc_internal.h"
#include "pthread.h"

#define STREAM_READ_SIZE        (512 * 16)
#define STREAM_READ_ALL_SIZE    (0)
#define STREAM_BUF_MIN_SIZE     (0x18000)
#define STREAM_BUF_SIZE         0x400000 // max bitstream size (4MB)
#define STREAM_BUF_SIZE_RESERVE 1024
#define WAVE4_ENC_REPORT_SIZE   144
#define INIT_TEST_ENCODER_OK    10
#define YUV_NAME                "../img/3-1920x1080.yuv"
#define BS_NAME                 "CVISDK"

#define SECONDARY_AXI_H264 0xf
#define SECONDARY_AXI_H265 0x0
#define TIME_BLOCK_MODE    (-1)
#define RET_VCODEC_TIMEOUT (-2)

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef ALIGN
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#endif

typedef enum _E_VENC_VB_SOURCE
{
    VB_SOURCE_TYPE_COMMON  = 0,
    VB_SOURCE_TYPE_PRIVATE = 2,
    VB_SOURCE_TYPE_USER    = 3
} E_VENC_VB_SOURCE;


#define COMP(a, b)                                    \
    do {                                              \
        if ((a) != (b))                               \
        {                                             \
            CVI_VC_ERR("0x%08X != 0x%X\n", (a), (b)); \
        }                                             \
    } while (0)

#if CFG_MEM
DRAM_CFG dramCfg;
#endif

extern SEM_T  tSbmWaitQueue[VENC_MAX_CHN_NUM];
extern Uint32 frm_done_event[VENC_MAX_CHN_NUM];
extern Uint32 err_frm_skip[VENC_MAX_CHN_NUM];

extern int  cviSetCtuQpMap(int coreIdx, TestEncConfig *pEncConfig, EncOpenParam *pEncOP,
                           PhysicalAddress addrCtuQpMap, Uint8 *ctuQpMapBuf,
                           EncParam *encParam, int maxCtuNum);
extern BOOL BitstreamReader_Act(EncHandle       handle,
                                PhysicalAddress bitstreamBuffer,
                                Uint32 bitstreamBufferSize, Uint32 streamReadSize,
                                Comparator comparator);
extern void rcLibSetupRc(CodecInst *pCodecInst);
extern int  rcLibCalcPicQp(CodecInst *pCodecInst, int *pRowMaxDqpMinus,
                           int *pRowMaxDqpPlus, int *pOutQp, int *pTargetPicBit,
                           int *pHrdBufLevel, int *pHrdBufSize);
extern void DeinitVcodecLock(Uint32 core_idx);

#define REPEAT_SPS 1

int gNumInstance = 1;

#ifndef FIRMWARE_H
static Uint32  sizeInWord;
static Uint16 *pusBitCode;
#endif

stTestEncoder          gTestEncoder[MAX_NUM_INSTANCE];
TestEncConfig          encConfig[MAX_NUM_INSTANCE];
static SEM_T           tSwitchCoreIdxWaitQueue;
static MUTEX_T         sSwitchCoreIdxSem;
static venc_sbm_info_t tVencSbmInfo = {0x0, .targetChnNum = -1,
                                       .sbmChnNum = -1,
                                       .frmChnNum = {-1}};
static int             initMcuEnv(TestEncConfig *pEncConfig);
static int             initEncOneFrame(stTestEncoder *pTestEnc, TestEncConfig *pEncConfig);
static void            cviDumpEncConfig(TestEncConfig *pEncConfig);
static void            cviDumpRcAttr(TestEncConfig *pEncConfig);
static void            cviDumpRcParam(TestEncConfig *pEncConfig);
static void            cviDumpCodingParam(TestEncConfig *pEncConfig);
static void            cviDumpGopMode(TestEncConfig *pEncConfig);
#ifdef SUPPORT_DONT_READ_STREAM
static int updateBitstream(vpu_buffer_t *vb, Int32 size);
#endif
static int cviEncodeOneFrame(stTestEncoder *pTestEnc);
/*
static void RcChangeSetParameter(EncHandle handle, TestEncConfig encConfig,
				 EncOpenParam *encOP, EncParam *encParam,
				 Uint32 frameIdx, Uint32 fieldDone,
				 int field_flag, vpu_buffer_t vbRoi); */
static void cviPicParamChangeCtrl(EncHandle handle, TestEncConfig *pEncConfig,
                                  EncOpenParam *pEncOP, EncParam *encParam, Uint32 frameIdx);

static int   cviGetStream(stTestEncoder *pTestEnc);
static int   cviCheckOutputInfo(stTestEncoder *pTestEnc);
static int   cviCheckEncodeEnd(stTestEncoder *pTestEnc);
static int   cviCheckAndCompareBitstream(stTestEncoder *pTestEnc);
static int   cviGetEncodedInfo(stTestEncoder *pTestEnc, int s32MilliSec);
static void  cviCloseVpuEnc(stTestEncoder *pTestEnc);
static void  cviDeInitVpuEnc(stTestEncoder *pTestEnc);
static void *cviInitEncoder(stTestEncoder *pTestEnc, TestEncConfig *pEncConfig);
static int   cviGetOneStream(void *handle, cviVEncStreamInfo *pStreamInfo, int s32MilliSec);
static int   cviCheckSuperFrame(stTestEncoder *pTestEnc);
static int   cviProcessSuperFrame(stTestEncoder *pTestEnc, cviVEncStreamInfo *pStreamInfo, int s32MilliSec);
static int   cviReEncodeIDR(stTestEncoder *pTestEnc, cviVEncStreamInfo *pStreamInfo, int s32MilliSec);
static int   checkEncConfig(TestEncConfig *pEncConfig, Uint32 productId);
#if 1 //ndef USE_KERNEL_MODE
static int initEncConfigByArgv(int argc, char **argv, TestEncConfig *pEncConfig);
#endif
static void initEncConfigByCtx(TestEncConfig    *pEncConfig,
                               cviInitEncConfig *pInitEncCfg);
static int  cviVEncGetVbInfo(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSetVbBuffer(stTestEncoder *pTestEnc, void *arg);
static int  _cviVencCalculateBufInfo(stTestEncoder *pTestEnc, int *butcnt, int *bufsize);
static int  cviVEncRegReconBuf(stTestEncoder *pTestEnc, void *arg);
static int  _cviVEncRegFrameBuffer(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSetInPixelFormat(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSetFrameParam(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncCalcFrameParam(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSetSbMode(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncStartSbMode(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncUpdateSbWptr(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncResetSb(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSbEnDummyPush(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSbTrigDummyPush(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSbDisDummyPush(stTestEncoder *pTestEnc, void *arg);
static int  cviVEncSbGetSkipFrmStatus(stTestEncoder *pTestEnc, void *arg);
#if CFG_MEM
static int checkDramCfg(DRAM_CFG *pDramCfg);
#endif
#ifdef USE_POSIX
static void *pfnWaitEncodeDone(void *param);
#else
static void pfnWaitEncodeDone(void *param);
#endif
static int pickupNextEncChnNum(void);

void cviVcodecGetVersion(void)
{
    CVI_VC_INFO("VCODEC_VERSION = %s\n", VCODEC_VERSION);
}

#if 0
static void help(struct OptionExt *opt, const char *programName)
{
	int i;

	CVI_VC_INFO(
		"------------------------------------------------------------------------------\n");
	CVI_VC_INFO("%s(API v%d.%d.%d)\n", programName, API_VERSION_MAJOR,
		    API_VERSION_MINOR, API_VERSION_PATCH);
	CVI_VC_INFO("\tAll rights reserved by Chips&Media(C)\n");
	CVI_VC_INFO("\tSample program controlling the Chips&Media VPU\n");
	CVI_VC_INFO(
		"------------------------------------------------------------------------------\n");
	CVI_VC_INFO("%s [option]\n", programName);
	CVI_VC_INFO("-h                          help\n");
	CVI_VC_INFO("-n [num]                    output frame number\n");
	CVI_VC_INFO("-v                          print version information\n");
	CVI_VC_INFO(
		"-c                          compare with golden bitstream\n");

	for (i = 0; i < MAX_GETOPT_OPTIONS; i++) {
		if (opt[i].name == NULL)
			break;
		CVI_VC_INFO("%s", opt[i].help);
	}
}
#endif

static void cviDumpEncConfig(TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

    cviDumpRcAttr(pEncConfig);
    cviDumpRcParam(pEncConfig);
    cviDumpCodingParam(pEncConfig);
    cviDumpGopMode(pEncConfig);

    CVI_VC_CFG("enSuperFrmMode = %d\n", pCviEc->enSuperFrmMode);
    CVI_VC_CFG("cviRcEn = %d\n", pCviEc->cviRcEn);
    CVI_VC_CFG("rotAngle %d\n", pEncConfig->rotAngle);
    CVI_VC_CFG("mirDir %d\n", pEncConfig->mirDir);
    CVI_VC_CFG("useRot %d\n", pEncConfig->useRot);
    CVI_VC_CFG("qpReport %d\n", pEncConfig->qpReport);
    CVI_VC_CFG("ringBufferEnable %d\n", pEncConfig->ringBufferEnable);
    CVI_VC_CFG("rcIntraQp %d\n", pEncConfig->rcIntraQp);
    CVI_VC_CFG("outNum %d\n", pEncConfig->outNum);
    CVI_VC_CFG("instNum %d\n", pEncConfig->instNum);
    CVI_VC_CFG("coreIdx %d\n", pEncConfig->coreIdx);
    CVI_VC_CFG("mapType %d\n", pEncConfig->mapType);
    CVI_VC_CFG("lineBufIntEn %d\n", pEncConfig->lineBufIntEn);
    CVI_VC_CFG("bEsBufQueueEn %d\n", pEncConfig->bEsBufQueueEn);
    CVI_VC_CFG("en_container %d\n", pEncConfig->en_container);
    CVI_VC_CFG("container_frame_rate %d\n", pEncConfig->container_frame_rate);
    CVI_VC_CFG("picQpY %d\n", pEncConfig->picQpY);
    CVI_VC_CFG("cbcrInterleave %d\n", pEncConfig->cbcrInterleave);
    CVI_VC_CFG("nv21 %d\n", pEncConfig->nv21);
    CVI_VC_CFG("needSourceConvert %d\n", pEncConfig->needSourceConvert);
    CVI_VC_CFG("packedFormat %d\n", pEncConfig->packedFormat);
    CVI_VC_CFG("srcFormat %d\n", pEncConfig->srcFormat);
    CVI_VC_CFG("srcFormat3p4b %d\n", pEncConfig->srcFormat3p4b);
    CVI_VC_CFG("decodingRefreshType %d\n", pEncConfig->decodingRefreshType);
    CVI_VC_CFG("tempLayer %d\n", pEncConfig->tempLayer);
    CVI_VC_CFG("useDeriveLambdaWeight %d\n", pEncConfig->useDeriveLambdaWeight);
    CVI_VC_CFG("dynamicMergeEnable %d\n", pEncConfig->dynamicMergeEnable);
    CVI_VC_CFG("independSliceMode %d\n", pEncConfig->independSliceMode);
    CVI_VC_CFG("independSliceModeArg %d\n", pEncConfig->independSliceModeArg);
    CVI_VC_CFG("RcEnable %d\n", pEncConfig->RcEnable);
    CVI_VC_CFG("bitdepth %d\n", pEncConfig->bitdepth);
    CVI_VC_CFG("secondary_axi %d\n", pEncConfig->secondary_axi);
    CVI_VC_CFG("stream_endian %d\n", pEncConfig->stream_endian);
    CVI_VC_CFG("frame_endian %d\n", pEncConfig->frame_endian);
    CVI_VC_CFG("source_endian %d\n", pEncConfig->source_endian);
    CVI_VC_CFG("yuv_mode %d\n", pEncConfig->yuv_mode);
    CVI_VC_CFG("loopCount %d\n", pEncConfig->loopCount);
}

static void cviDumpRcAttr(TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

    CVI_VC_CFG("stdMode %d\n", pEncConfig->stdMode);
    CVI_VC_CFG("picWidth %d\n", pEncConfig->picWidth);
    CVI_VC_CFG("picHeight %d\n", pEncConfig->picHeight);
    CVI_VC_CFG("rcMode %d\n", pEncConfig->rcMode);
    if (pCviEc->virtualIPeriod)
    {
        CVI_VC_CFG("gop %d\n", pCviEc->virtualIPeriod);
    }
    else
    {
        CVI_VC_CFG("gop %d\n", pCviEc->gop);
    }
    CVI_VC_CFG("framerate %d\n", pCviEc->framerate);
    CVI_VC_CFG("statTime %d\n", pCviEc->statTime);
    CVI_VC_CFG("bitrate %d\n", pCviEc->bitrate);
    CVI_VC_CFG("maxbitrate %d\n", pCviEc->maxbitrate);
    CVI_VC_CFG("iqp %d\n", pCviEc->iqp);
    CVI_VC_CFG("pqp %d\n", pCviEc->pqp);
}

static void cviDumpRcParam(TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

    CVI_VC_CFG("RowQpDelta %d\n", pCviEc->u32RowQpDelta);
    CVI_VC_CFG("ThrdLv %d\n", pCviEc->u32ThrdLv);
    CVI_VC_CFG("firstFrmstartQp %d\n", pCviEc->firstFrmstartQp);
    CVI_VC_CFG("initialDelay %d\n", pCviEc->initialDelay);
    CVI_VC_CFG("MaxIprop %d\n", pCviEc->u32MaxIprop);
    CVI_VC_CFG("MaxQp %d, MinQp %d\n", pCviEc->u32MaxQp, pCviEc->u32MinQp);
    CVI_VC_CFG("MaxIQp %d, MinIQp %d\n", pCviEc->u32MaxIQp, pCviEc->u32MinIQp);
    CVI_VC_CFG("changePos %d\n", pEncConfig->changePos);
    CVI_VC_CFG("StillPercent %d\n", pCviEc->s32MinStillPercent);
    CVI_VC_CFG("StillQP %d\n", pCviEc->u32MaxStillQP);
    CVI_VC_CFG("MotionSensitivity %d\n", pCviEc->u32MotionSensitivity);
    CVI_VC_CFG("AvbrPureStillThr %d\n", pCviEc->s32AvbrPureStillThr);
    CVI_VC_CFG("AvbrFrmLostOpen %d\n", pCviEc->s32AvbrFrmLostOpen);
    CVI_VC_CFG("AvbrFrmGap %d\n", pCviEc->s32AvbrFrmGap);
}

static void cviDumpCodingParam(TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

    CVI_VC_CFG("frmLostOpen %d\n", pEncConfig->frmLostOpen);
    CVI_VC_CFG("frameSkipBufThr %d\n", pCviEc->frmLostBpsThr);
    CVI_VC_CFG("encFrmGaps %d\n", pCviEc->encFrmGaps);
    CVI_VC_CFG("IntraCost %d\n", pCviEc->u32IntraCost);
    CVI_VC_CFG("chroma_qp_index_offset %d\n", pCviEc->h264Trans.chroma_qp_index_offset);
    CVI_VC_CFG("cb_qp_offset %d\n", pCviEc->h265Trans.cb_qp_offset);
    CVI_VC_CFG("cr_qp_offset %d\n", pCviEc->h265Trans.cr_qp_offset);
}

static void cviDumpGopMode(TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

    CVI_VC_CFG("IPQpDelta %d\n", pCviEc->s32IPQpDelta);
    if (pCviEc->virtualIPeriod)
    {
        CVI_VC_CFG("BgInterval %d\n", pCviEc->gop);
    }
}

extern char *optarg; /* argument associated with option */
int          TestEncoder(TestEncConfig *pEncConfig)
{
    stTestEncoder *pTestEnc = &gTestEncoder[pEncConfig->instNum];
    int            suc      = 0;
    int            ret;

    VDI_POWER_ON_DOING_JOB(pEncConfig->coreIdx, ret, initEncOneFrame(pTestEnc, pEncConfig));

    if (ret == TE_ERR_ENC_INIT)
    {
        CVI_VC_ERR("ret %d\n", ret);
        goto ERR_ENC_INIT;
    }
    else if (ret == TE_ERR_ENC_OPEN)
    {
        CVI_VC_ERR("ret %d\n", ret);
        goto ERR_ENC_OPEN;
    }

    while (1)
    {
        vdi_set_clock_gate(pEncConfig->coreIdx, CLK_ENABLE);
        ret = cviEncodeOneFrame(pTestEnc);

        if (ret == TE_STA_ENC_BREAK)
        {
            CVI_VC_INFO("cviEncodeOneFrame, ret = %d\n", ret);
            vdi_set_clock_gate(pEncConfig->coreIdx, CLK_DISABLE);
            break;
        }
        else if (ret == TE_ERR_ENC_OPEN)
        {
            CVI_VC_INFO("cviEncodeOneFrame, ret = %d\n", ret);
            vdi_set_clock_gate(pEncConfig->coreIdx, CLK_DISABLE);
            goto ERR_ENC_OPEN;
        }

        ret = cviGetStream(pTestEnc);
        vdi_set_clock_gate(pEncConfig->coreIdx, CLK_DISABLE);

        if (ret == TE_STA_ENC_BREAK)
        {
            break;
        }
        else if (ret)
        {
            CVI_VC_ERR("cviGetStream, ret = %d\n", ret);
            goto ERR_ENC_OPEN;
        }
    }

    VDI_POWER_ON_DOING_JOB(pEncConfig->coreIdx, ret, cviCheckAndCompareBitstream(pTestEnc));

    if (ret == TE_ERR_ENC_OPEN)
    {
        CVI_VC_ERR("cviCheckAndCompareBitstream\n");
        goto ERR_ENC_OPEN;
    }

    suc = 1;

ERR_ENC_OPEN:
    // Now that we are done with encoding, close the open instance.
    cviCloseVpuEnc(pTestEnc);
ERR_ENC_INIT:
    cviDeInitVpuEnc(pTestEnc);

    return suc;
}

static int _updateEncFrameInfo(stTestEncoder *pTestEnc, FrameBufferFormat *psrcFrameFormat)
{
    EncOpenParam  *pEncOP  = &pTestEnc->encOP;
    TestEncConfig *pEncCfg = &pTestEnc->encConfig;
    // width = 8-aligned (CU unit)
    pTestEnc->srcFrameWidth = VPU_ALIGN8(pTestEnc->encOP.picWidth);
    CVI_VC_CFG("srcFrameWidth = %d\n", pTestEnc->srcFrameWidth);

    // height = 8-aligned (CU unit)
    pTestEnc->srcFrameHeight = VPU_ALIGN8(pTestEnc->encOP.picHeight);
    CVI_VC_CFG("srcFrameHeight = %d\n", pTestEnc->srcFrameHeight);

    // stride should be a 32-aligned.
    pTestEnc->srcFrameStride = VPU_ALIGN32(pTestEnc->encOP.picWidth);

    if (pTestEnc->encConfig.packedFormat >= PACKED_YUYV)
        pTestEnc->encConfig.srcFormat = FORMAT_422;

    CVI_VC_TRACE("packedFormat = %d\n", pTestEnc->encConfig.packedFormat);

    if (pTestEnc->encConfig.srcFormat == FORMAT_422 && pTestEnc->encConfig.packedFormat >= PACKED_YUYV)
    {
        int               p10bits = pTestEnc->encConfig.srcFormat3p4b == 0 ? 16 : 32;
        FrameBufferFormat srcFormat =
            GetPackedFormat(pTestEnc->encOP.srcBitDepth,
                            pTestEnc->encConfig.packedFormat,
                            p10bits, 1);

        if (srcFormat == (FrameBufferFormat)-1)
        {
            CVI_VC_ERR("fail to GetPackedFormat\n");
            return TE_ERR_ENC_INIT;
        }
        pTestEnc->encOP.srcFormat      = srcFormat;
        *psrcFrameFormat               = srcFormat;
        pTestEnc->encOP.nv21           = 0;
        pTestEnc->encOP.cbcrInterleave = 0;
    }
    else
    {
        pTestEnc->encOP.srcFormat = pTestEnc->encConfig.srcFormat;
        *psrcFrameFormat          = pTestEnc->encConfig.srcFormat;
        pTestEnc->encOP.nv21      = pTestEnc->encConfig.nv21;
    }
    pTestEnc->encOP.packedFormat = pTestEnc->encConfig.packedFormat;

    CVI_VC_TRACE("packedFormat = %d\n", pTestEnc->encConfig.packedFormat);

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        if (pEncCfg->rotAngle == 90 || pEncCfg->rotAngle == 270)
        {
            pTestEnc->framebufWidth  = pEncOP->picHeight;
            pTestEnc->framebufHeight = pEncOP->picWidth;
        }
        else
        {
            pTestEnc->framebufWidth  = pEncOP->picWidth;
            pTestEnc->framebufHeight = pEncOP->picHeight;
        }

        pTestEnc->framebufWidth = VPU_ALIGN16(pTestEnc->framebufWidth);

        // To cover interlaced picture
        pTestEnc->framebufHeight = VPU_ALIGN32(pTestEnc->framebufHeight);
    }
    else
    {
        pTestEnc->framebufWidth  = VPU_ALIGN8(pTestEnc->encOP.picWidth);
        pTestEnc->framebufHeight = VPU_ALIGN8(pTestEnc->encOP.picHeight);

        if ((pTestEnc->encConfig.rotAngle != 0 || pTestEnc->encConfig.mirDir != 0) && !(pTestEnc->encConfig.rotAngle == 180 && pTestEnc->encConfig.mirDir == MIRDIR_HOR_VER))
        {
            pTestEnc->framebufWidth =
                VPU_ALIGN32(pTestEnc->encOP.picWidth);
            pTestEnc->framebufHeight =
                VPU_ALIGN32(pTestEnc->encOP.picHeight);
        }

        if (pTestEnc->encConfig.rotAngle == 90 || pTestEnc->encConfig.rotAngle == 270)
        {
            pTestEnc->framebufWidth =
                VPU_ALIGN32(pTestEnc->encOP.picHeight);
            pTestEnc->framebufHeight =
                VPU_ALIGN32(pTestEnc->encOP.picWidth);
        }
    }

    return RETCODE_SUCCESS;
}

int initEncOneFrame(stTestEncoder *pTestEnc, TestEncConfig *pEncConfig)
{
    TestEncConfig    *pEncCfg = &pTestEnc->encConfig;
    EncOpenParam     *pEncOP  = &pTestEnc->encOP;
    cviEncCfg        *pcviEc  = &pEncCfg->cviEc;
    cviCapability     cap, *pCap = &cap;
    MirrorDirection   mirrorDirection;
    FrameBufferFormat srcFrameFormat;
    RetCode           ret = RETCODE_SUCCESS;
    int               i, mapType;
    int               coreIdx;
    int               instIdx;
    Uint64            start_time, end_time;

#if defined(CVI_H26X_USE_ION_MEM)
#if defined(BITSTREAM_ION_CACHED_MEM)
    // int bBsStreamCached = 1;
#else
    int bBsStreamCached = 0;
#endif
#endif

    memset(pTestEnc, 0, sizeof(stTestEncoder));

    pTestEnc->bsBufferCount    = 1;
    pTestEnc->bsQueueIndex     = 1;
    pTestEnc->srcFrameIdx      = 0;
    pTestEnc->frameIdx         = 0;
    pTestEnc->framebufWidth    = 0;
    pTestEnc->framebufHeight   = 0;
    pTestEnc->regFrameBufCount = 0;

    memcpy(pEncCfg, pEncConfig, sizeof(TestEncConfig));
    memset(&pTestEnc->fbSrc[0], 0x00, sizeof(pTestEnc->fbSrc));
    memset(&pTestEnc->fbRecon[0], 0x00, sizeof(pTestEnc->fbRecon));
    memset(pTestEnc->vbReconFrameBuf, 0x00, sizeof(pTestEnc->vbReconFrameBuf));
    memset(pTestEnc->vbSourceFrameBuf, 0x00, sizeof(pTestEnc->vbSourceFrameBuf));
    memset(pEncOP, 0x00, sizeof(EncOpenParam));
    memset(&pTestEnc->encParam, 0x00, sizeof(EncParam));
    memset(&pTestEnc->initialInfo, 0x00, sizeof(EncInitialInfo));
    memset(&pTestEnc->outputInfo, 0x00, sizeof(EncOutputInfo));
    memset(&pTestEnc->secAxiUse, 0x00, sizeof(SecAxiUse));
    memset(pTestEnc->vbStream, 0x00, sizeof(pTestEnc->vbStream));
    memset(&pTestEnc->vbReport, 0x00, sizeof(vpu_buffer_t));
    memset(&pTestEnc->encHeaderParam, 0x00, sizeof(EncHeaderParam));
    memset(pTestEnc->vbRoi, 0x00, sizeof(pTestEnc->vbRoi));
    memset(pTestEnc->vbCtuQp, 0x00, sizeof(pTestEnc->vbCtuQp));
    memset(pTestEnc->vbPrefixSeiNal, 0x00, sizeof(pTestEnc->vbPrefixSeiNal));
    memset(pTestEnc->vbSuffixSeiNal, 0x00, sizeof(pTestEnc->vbSuffixSeiNal));
    memset(&pTestEnc->vbVuiRbsp, 0x00, sizeof(vpu_buffer_t));
    memset(&pTestEnc->vbHrdRbsp, 0x00, sizeof(vpu_buffer_t));
#ifdef TEST_ENCODE_CUSTOM_HEADER
    memset(&pTestEnc->vbSeiNal, 0x00, sizeof(pTestEnc->vbSeiNal));
    memset(&pTestEnc->vbCustomHeader, 0x00, sizeof(vpu_buffer_t));
#endif
    memset(&pTestEnc->streamPack, 0x00, sizeof(pTestEnc->streamPack));

    instIdx = pEncCfg->instNum;
    coreIdx = pEncCfg->coreIdx;

    pTestEnc->coreIdx   = coreIdx;
    pTestEnc->productId = VPU_GetProductId(coreIdx);

    start_time = cviGetCurrentTime();

    ret = VPU_InitWithBitcode(coreIdx, pEncCfg->pusBitCode, pEncCfg->sizeInWord);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("Failed to boot up VPU(coreIdx: %d, productId: %d)\n", coreIdx, pTestEnc->productId);
#ifdef ENABLE_CNM_DEBUG_MSG
        PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
        return TE_ERR_ENC_INIT;
    }

    end_time = cviGetCurrentTime();
    CVI_VC_PERF("VPU_InitWithBitcode = " VCODEC_UINT64_FORMAT_DEC " us\n", end_time - start_time);

    PrintVpuVersionInfo(coreIdx);

    ret = (RetCode)vdi_set_single_es_buf(coreIdx, pcviEc->bSingleEsBuf, &pcviEc->bitstreamBufferSize);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("set_single_es_buf fail with ret %d\n", ret);
        return TE_ERR_ENC_INIT;
    }

    pEncOP->bitstreamFormat = pEncCfg->stdMode;
    mapType                 = (pEncCfg->mapType & 0x0f);


    ret = (RetCode)GetEncOpenParamDefault(pEncOP, pEncCfg);

    vdi_get_chip_capability(coreIdx, pCap);

    pCap->addrRemapEn = pcviEc->addrRemapEn;

    if ((pEncOP->bitstreamFormat == STD_AVC) && (pcviEc->singleLumaBuf > 0))
        pEncOP->addrRemapEn = 0;
    else
        pEncOP->addrRemapEn = pCap->addrRemapEn;

    CVI_VC_CFG("addrRemapEn = %d\n", pEncOP->addrRemapEn);

    cviDumpEncConfig(pEncCfg);

    cviPrintRc(pTestEnc);

    if (ret == RETCODE_SUCCESS)
    {
        CVI_VC_ERR("\n");
        return TE_ERR_ENC_INIT;
    }

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        pEncOP->linear2TiledEnable = pEncCfg->coda9.enableLinear2Tiled;
        if (pEncOP->linear2TiledEnable == TRUE)
        {
            pEncOP->linear2TiledMode = FF_FRAME;
        }

        // enable SVC for advanced termporal reference structure
        pEncCfg->coda9.enableSvc                  = (pEncOP->gopPreset != 1) || (pEncOP->VirtualIPeriod > 1);
        pEncOP->EncStdParam.avcParam.mvcExtension = pEncCfg->coda9.enableMvc;
        pEncOP->EncStdParam.avcParam.svcExtension = pEncCfg->coda9.enableSvc;

        if (pEncOP->EncStdParam.avcParam.fieldFlag == TRUE)
        {
            if (pEncCfg->rotAngle != 0 || pEncCfg->mirDir != 0)
            {
                VLOG(WARN, "%s:%d When field Flag is enabled. VPU doesn't support rotation or mirror in field encoding mode.\n", __FUNCTION__, __LINE__);
                pEncCfg->rotAngle = 0;
                pEncCfg->mirDir   = MIRDIR_NONE;
            }
        }
    }

    ret = (RetCode)_updateEncFrameInfo(pTestEnc, &srcFrameFormat);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("_updateEncFrameInfo failure:%d\n", ret);
        return TE_ERR_ENC_INIT;
    }

    pTestEnc->bsBufferCount = NUM_OF_BS_BUF;

    for (i = 0; i < (int)pTestEnc->bsBufferCount; i++)
    {
        if (pEncConfig->cviEc.bitstreamBufferSize != 0)
        {
            pTestEnc->vbStream[i].size = pEncConfig->cviEc.bitstreamBufferSize;

            /*
			 * when bitstreamBufferSize is too small and occur INT_BIT_BIT_BUF_FULL
			 * hw will write more than bitstreamBufferSize, it will corrupt memory
			 * so need add reserve buff
			 */
            if (pTestEnc->vbStream[i].size + STREAM_BUF_SIZE_RESERVE < STREAM_BUF_SIZE)
                pTestEnc->vbStream[i].size += STREAM_BUF_SIZE_RESERVE;
        }
        else
            pTestEnc->vbStream[i].size = STREAM_BUF_SIZE;
        CVI_VC_MEM("vbStream[%d].size = 0x%X\n", i, pTestEnc->vbStream[i].size);
        if (vdi_get_is_single_es_buf(coreIdx))
            ret = (RetCode)vdi_allocate_single_es_buf_memory(coreIdx, &pTestEnc->vbStream[i]);
        else
            ret = (RetCode)VDI_ALLOCATE_MEMORY(coreIdx, &pTestEnc->vbStream[i], 0, NULL);

        if (ret < RETCODE_SUCCESS)
        {
            CVI_VC_ERR("fail to allocate bitstream buffer\n");
            return TE_ERR_ENC_INIT;
        }

        if (pTestEnc->vbStream[i].size < STREAM_BUF_SIZE)
            pTestEnc->vbStream[i].size -= STREAM_BUF_SIZE_RESERVE;

        CVI_VC_TRACE("i = %d, STREAM_BUF = 0x" VCODEC_UINT64_FORMAT_HEX ", STREAM_BUF_SIZE = 0x%X\n",
                     i, pTestEnc->vbStream[i].phys_addr, pTestEnc->vbStream[i].size);
    }

    pEncOP->bitstreamBuffer     = pTestEnc->vbStream[0].phys_addr;
    pEncOP->bitstreamBufferSize = pTestEnc->vbStream[0].size; //* bsBufferCount;//

    CVI_VC_BS("bitstreamBuffer = 0x" VCODEC_UINT64_FORMAT_HEX ", bitstreamBufferSize = 0x%X\n",
              pEncOP->bitstreamBuffer, pEncOP->bitstreamBufferSize);

    // -- HW Constraint --
    // Required size = actual size of bitstream buffer + 32KBytes
    // Minimum size of bitstream : 96KBytes
    // Margin : 32KBytes
    // Please refer to 3.2.4.4 Encoder Stream Handling in WAVE420
    // programmer's guide.
    if (pEncOP->bitstreamFormat == STD_HEVC)
    {
        if (pEncCfg->ringBufferEnable)
            pEncOP->bitstreamBufferSize -= 0x8000;
    }

    CVI_VC_TRACE("ringBufferEnable = %d\n", pEncCfg->ringBufferEnable);

    pEncOP->ringBufferEnable = pEncCfg->ringBufferEnable;
    pEncOP->cbcrInterleave   = pEncCfg->cbcrInterleave;
    pEncOP->frameEndian      = pEncCfg->frame_endian;
    pEncOP->streamEndian     = pEncCfg->stream_endian;
    pEncOP->sourceEndian     = pEncCfg->source_endian;
    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        pEncOP->bwbEnable = VPU_ENABLE_BWB;
    }

    pEncOP->lineBufIntEn  = pEncCfg->lineBufIntEn;
    pEncOP->bEsBufQueueEn = pEncCfg->bEsBufQueueEn;
    CVI_VC_TRACE("pEncCfg->bEsBufQueueEn = %d\n", pEncCfg->bEsBufQueueEn);
    pEncOP->coreIdx   = coreIdx;
    pEncOP->cbcrOrder = CBCR_ORDER_NORMAL;
    // host can set useLongTerm to 1 or 0 directly.
    pEncOP->EncStdParam.hevcParam.useLongTerm =
        (pEncCfg->useAsLongtermPeriod > 0 && pEncCfg->refLongtermPeriod > 0) ? 1 : 0;
    pEncOP->s32ChnNum = pEncCfg->s32ChnNum;

#ifdef SUPPORT_MULTIPLE_PPS // if SUPPORT_MULTIPLE_PPS is enabled. encoder can include multiple pps in bitstream output.
    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        getAvcEncPPS(&encOP); //add PPS before OPEN
    }
#endif

    if (writeVuiRbsp(coreIdx, pEncCfg, pEncOP, &pTestEnc->vbVuiRbsp) == FALSE)
        return TE_ERR_ENC_INIT;
    if (writeHrdRbsp(coreIdx, pEncCfg, pEncOP, &pTestEnc->vbHrdRbsp) == FALSE)
        return TE_ERR_ENC_INIT;

#ifdef TEST_ENCODE_CUSTOM_HEADER
    if (writeCustomHeader(coreIdx, pEncOP, &pTestEnc->vbCustomHeader,
                          &pTestEnc->hrd)
        == FALSE)
        return TE_ERR_ENC_INIT;
#endif

    // Open an instance and get initial information for encoding.
    ret = VPU_EncOpen(&pTestEnc->handle, pEncOP);

    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("VPU_EncOpen failed Error code is 0x%x\n", ret);
#ifdef ENABLE_CNM_DEBUG_MSG
        PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
        return TE_ERR_ENC_INIT;
    }

    cviEncRc_Open(&pTestEnc->handle->rcInfo, pEncOP);

    cviSetApiMode(pTestEnc->handle, pEncCfg->cviApiMode);

#ifdef ENABLE_CNM_DEBUG_MSG
    // VPU_EncGiveCommand(pTestEnc->handle, ENABLE_LOGGING, 0);
#endif
    if (pEncCfg->useRot == TRUE)
    {
        VPU_EncGiveCommand(pTestEnc->handle, ENABLE_ROTATION, 0);
        VPU_EncGiveCommand(pTestEnc->handle, ENABLE_MIRRORING, 0);
        VPU_EncGiveCommand(pTestEnc->handle, SET_ROTATION_ANGLE, &pEncCfg->rotAngle);

        mirrorDirection = (MirrorDirection)pEncCfg->mirDir;
        VPU_EncGiveCommand(pTestEnc->handle, SET_MIRROR_DIRECTION, &mirrorDirection);
    }

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        pTestEnc->secAxiUse.u.coda9.useBitEnable  = (pEncCfg->secondary_axi >> 0) & 0x01;
        pTestEnc->secAxiUse.u.coda9.useIpEnable   = (pEncCfg->secondary_axi >> 1) & 0x01;
        pTestEnc->secAxiUse.u.coda9.useDbkYEnable = (pEncCfg->secondary_axi >> 2) & 0x01;
        pTestEnc->secAxiUse.u.coda9.useDbkCEnable = (pEncCfg->secondary_axi >> 3) & 0x01;
        pTestEnc->secAxiUse.u.coda9.useBtpEnable  = (pEncCfg->secondary_axi >> 4) & 0x01;
        pTestEnc->secAxiUse.u.coda9.useOvlEnable  = (pEncCfg->secondary_axi >> 5) & 0x01;
        VPU_EncGiveCommand(pTestEnc->handle, SET_SEC_AXI, &pTestEnc->secAxiUse);
    }

    /*******************************************
	* INIT_SEQ                                *
	*******************************************/
    ret = VPU_EncGetInitialInfo(pTestEnc->handle, &pTestEnc->initialInfo);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("VPU_EncGetInitialInfo failed Error code is 0x%x %p\n", ret, __builtin_return_address(0));
#ifdef ENABLE_CNM_DEBUG_MSG
        PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
        return TE_ERR_ENC_OPEN;
    }

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        // Note: The below values of MaverickCache configuration are best values.
        MaverickCacheConfig encCacheConfig;
        MaverickCache2Config(
            &encCacheConfig,
            0,                      //encoder
            pEncOP->cbcrInterleave, // cb cr interleave
            0,                      /* bypass */
            0,                      /* burst  */
            3,                      /* merge mode */
            mapType,
            15 /* shape */);
        VPU_EncGiveCommand(pTestEnc->handle, SET_CACHE_CONFIG, &encCacheConfig);
    }

    CVI_VC_INFO("* Enc InitialInfo =>\n instance #%d\n minframeBuffercount: %u\n minSrcBufferCount: %d\n",
                instIdx, pTestEnc->initialInfo.minFrameBufferCount, pTestEnc->initialInfo.minSrcFrameCount);
    CVI_VC_INFO(" picWidth: %u\n picHeight: %u\n ", pEncOP->picWidth, pEncOP->picHeight);

    if (pEncOP->bitstreamFormat == STD_HEVC)
    {
        pTestEnc->secAxiUse.u.wave4.useEncImdEnable =
            (pEncCfg->secondary_axi & 0x1) ? TRUE : FALSE; // USE_IMD_INTERNAL_BUF
        pTestEnc->secAxiUse.u.wave4.useEncRdoEnable =
            (pEncCfg->secondary_axi & 0x2) ? TRUE : FALSE; // USE_RDO_INTERNAL_BUF
        pTestEnc->secAxiUse.u.wave4.useEncLfEnable =
            (pEncCfg->secondary_axi & 0x4) ? TRUE : FALSE; // USE_LF_INTERNAL_BUF
        VPU_EncGiveCommand(pTestEnc->handle, SET_SEC_AXI, &pTestEnc->secAxiUse);

        CVI_VC_FLOW(
            "SET_SEC_AXI: useEncImdEnable = %d, useEncRdoEnable = %d, useEncLfEnable = %d\n",
            pTestEnc->secAxiUse.u.wave4.useEncImdEnable,
            pTestEnc->secAxiUse.u.wave4.useEncRdoEnable,
            pTestEnc->secAxiUse.u.wave4.useEncLfEnable);
    }

    /*************************
	* BUILD SEQUENCE HEADER *
	************************/

    pTestEnc->encParam.quantParam = pEncCfg->picQpY;
    CVI_VC_TRACE("picQpY = %d\n", pEncCfg->picQpY);
    pTestEnc->encParam.skipPicture = 0;
    pTestEnc->encParam.forcePicQpI = 0;

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        if (pEncOP->ringBufferEnable == TRUE)
        {
            VPU_EncSetWrPtr(pTestEnc->handle, pEncOP->bitstreamBuffer, 1);
        }
        else
        {
            pTestEnc->encHeaderParam.buf  = pEncOP->bitstreamBuffer;
            pTestEnc->encHeaderParam.size = pEncOP->bitstreamBufferSize;
        }

#if REPEAT_SPS
        ret = TRUE;
#else
        ret = cviEncode264Header(pTestEnc);
#endif
    }
    else
    {
        pTestEnc->encParam.forcePicQpEnable   = 0;
        pTestEnc->encParam.forcePicQpP        = 0;
        pTestEnc->encParam.forcePicQpB        = 0;
        pTestEnc->encParam.forcePicTypeEnable = 0;
        pTestEnc->encParam.forcePicType       = 0;
        pTestEnc->encParam.codeOption.implicitHeaderEncode =
            1; // FW will encode header data implicitly when changing the
        // header syntaxes
        pTestEnc->encParam.codeOption.encodeAUD = pEncCfg->encAUD;
        pTestEnc->encParam.codeOption.encodeEOS = 0;

        CVI_VC_TRACE("ringBufferEnable = %d\n", pEncOP->ringBufferEnable);

#if REPEAT_SPS
        ret = TRUE;
#else
        ret = cviEncode265Header(pTestEnc);
#endif
    }

    if (ret == FALSE)
    {
        CVI_VC_ERR("ret = %d\n", ret);
        return TE_ERR_ENC_OPEN;
    }

    CVI_VC_TRACE("ret = %d\n", ret);

    if (pEncOP->ringBufferEnable == TRUE)
    {
        // this function shows that HOST can set wrPtr to
        // start position of encoded output in ringbuffer enable mode
        VPU_EncSetWrPtr(pTestEnc->handle, pEncOP->bitstreamBuffer, 1);
    }
    DisplayEncodedInformation(pTestEnc->handle, pEncOP->bitstreamFormat, NULL, 0, 0);
    pTestEnc->bIsEncoderInited = TRUE;

    if (pEncCfg->bIsoSendFrmEn && !pTestEnc->tPthreadId)
    {
        SEM_INIT(pTestEnc->semSendEncCmd);
        SEM_INIT(pTestEnc->semGetStreamCmd);
        ret = pthread_create(&pTestEnc->tPthreadId, NULL, pfnWaitEncodeDone, (void *)pTestEnc);
        if (ret)
        {
            CVI_VC_ERR("WaitEncodeDone thread error!\n");
            return RETCODE_FAILURE;
        }
    }

    return INIT_TEST_ENCODER_OK;
}

#ifdef SUPPORT_DONT_READ_STREAM
static int updateBitstream(vpu_buffer_t *vb, Int32 size)
{
    int status = TRUE;
    if (size > vb->size)
    {
        CVI_VC_ERR(
            "bitstream buffer is not enough, size = 0x%X, bitstream size = 0x%X!\n",
            size, vb->size);
        return FALSE;
    }

#if 1
    unsigned char *ptr;
    int            len, idx;

    ptr  = (unsigned char *)(vb->phys_addr + size);
    len  = size;
    size = (size + 16) & 0xfffffff0;
    len  = size - len;

    for (idx = 0; idx < len; idx++, ptr++)
        *ptr = 0x0;
#else
    size = (size + 16) & 0xfffffff0;
#endif

    vb->phys_addr += size;
    vb->size      -= size;

#if ES_CYCLIC
    if (vb->phys_addr + STREAM_BUF_MIN_SIZE >= dramCfg.pucVpuDramAddr + STREAM_BUF_SIZE + vb->base)
    {
        vb->phys_addr = dramCfg.pucVpuDramAddr + vb->base;
        vb->size      = STREAM_BUF_SIZE;
        CVI_VC_BS("vb, pucVpuDramAddr = 0x%lX, base = 0x%lX, phys_addr = 0x%lX, size = 0x%X\n",
                  dramCfg.pucVpuDramAddr, vb->base, vb->phys_addr, vb->size);
    }
#endif

    CVI_VC_BS("vb, phys_addr = 0x%lX, size = 0x%X\n", vb->phys_addr, vb->size);

    return status;
}
#endif

static void cviUpdateOneRoi(EncParam *param, const cviEncRoiCfg *proi, int index)
{
    param->roi_request            = TRUE;
    param->roi_enable[index]      = proi->roi_enable;
    param->roi_qp_mode[index]     = proi->roi_qp_mode;
    param->roi_qp[index]          = proi->roi_qp;
    param->roi_rect_x[index]      = proi->roi_rect_x;
    param->roi_rect_y[index]      = proi->roi_rect_y;
    param->roi_rect_width[index]  = proi->roi_rect_width;
    param->roi_rect_height[index] = proi->roi_rect_height;
}

static void cviUpdateRoiBySdk(EncParam *param, cviEncRoiCfg *cviRoi)
{
    int i;

    param->roi_request = TRUE;
    for (i = 0; i <= 7; i++)
    {
        cviUpdateOneRoi(param, &cviRoi[i], i);
    }
}

static int cviH264SpsAddVui(stTestEncoder *pTestEnc)
{
    stStreamPack *psp  = &pTestEnc->streamPack;
    cviH264Vui   *pVui = &pTestEnc->encConfig.cviEc.h264Vui;
    BOOL bVUIEnbled = pTestEnc->encConfig.cviEc.bEnableVUI;
    int i;

    if (!bVUIEnbled)
    {
        CVI_VC_INFO("VUI is not enabled, skip adding VUI to SPS\n");
        return TRUE;
    }

    for (i = 0; i < psp->totalPacks; i++)
    {
        if (psp->pack[i].cviNalType == NAL_SPS)
        {
            H264SpsAddVui(pVui, &psp->pack[i].addr, &psp->pack[i].len);
            psp->pack[i].u64PhyAddr = (uint64_t)psp->pack[i].addr;
        }
    }

    return TRUE;
}

static int cviH265SpsAddVui(stTestEncoder *pTestEnc)
{
    stStreamPack *psp  = &pTestEnc->streamPack;
    cviH265Vui   *pVui = &pTestEnc->encConfig.cviEc.h265Vui;
    BOOL bVUIEnbled = pTestEnc->encConfig.cviEc.bEnableVUI;
    int i;

    if (!bVUIEnbled)
    {
        CVI_VC_INFO("VUI is not enabled, skip adding VUI to SPS\n");
        return TRUE;
    }

    for (i = 0; i < psp->totalPacks; i++)
    {
        if (psp->pack[i].cviNalType == NAL_SPS)
        {
            H265SpsAddVui(pVui, &psp->pack[i].addr, &psp->pack[i].len);
            psp->pack[i].u64PhyAddr = (uint64_t)(psp->pack[i].addr);
        }
    }

    return TRUE;
}

static int cviEncodeOneFrame(stTestEncoder *pTestEnc)
{
    EncParam             *pEncParam    = &pTestEnc->encParam;
    EncOpenParam         *pEncOP       = &pTestEnc->encOP;
    FrameBufferAllocInfo *pFbAllocInfo = &pTestEnc->fbAllocInfo;
    cviEncCfg            *pCviEc       = &pTestEnc->encConfig.cviEc;
    int                   coreIdx      = pTestEnc->coreIdx;
    int                   ret          = 0, i;
    feederYuvaddr        *pYuvAddr;
    CodecInst            *pCodecInst;

    CVI_VC_FLOW("frameIdx = %d\n", pTestEnc->frameIdx);

#if 0
RETRY:
	ret = cviVPU_LockAndCheckState(pTestEnc->handle, CODEC_STAT_ENC_PIC);
	if (ret != RETCODE_SUCCESS) {
		CVI_VC_ERR("cviVPU_LockAndCheckState, ret = %d\n", ret);
		return ret;
	}
#endif
    if (GetPendingInst(coreIdx))
    {
        //LeaveLock(coreIdx);
        CVI_VC_TRACE("GetPendingInst, RETCODE_FRAME_NOT_COMPLETE\n");
        return RETCODE_FAILURE;
    }

    pCodecInst                                              = pTestEnc->handle;
    pCodecInst->CodecInfo->encInfo.openParam.cbcrInterleave = pEncOP->cbcrInterleave;
    pCodecInst->CodecInfo->encInfo.openParam.nv21           = pEncOP->nv21;

    SetPendingInst(coreIdx, pCodecInst, __func__, __LINE__);
    ret = cviVPU_ChangeState(pTestEnc->handle);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("cviVPU_ChangeState, ret = %d\n", ret);
    }

    if (pTestEnc->encConfig.cviEc.roi_request)
        cviUpdateRoiBySdk(pEncParam, pTestEnc->encConfig.cviEc.cviRoi);
    pTestEnc->encConfig.cviEc.roi_request = FALSE;

    pEncParam->is_idr_frame = cviCheckIdrPeriod(pTestEnc);

    cviPicParamChangeCtrl(pTestEnc->handle, &pTestEnc->encConfig, pEncOP, pEncParam, pTestEnc->frameIdx);

    ret = cviSetupPicConfig(pTestEnc);
    if (ret != 0)
    {
        CVI_VC_ERR("cviSetupPicConfig, ret = %d\n", ret);
        return RETCODE_FAILURE;
    }

#if (defined CVI_H26X_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
    for (i = 0; i < (int)pTestEnc->bsBufferCount; i++)
    {
        if (pTestEnc->vbStream[i].size != 0)
            vdi_invalidate_ion_cache(pTestEnc->vbStream[i].phys_addr,
                                     pTestEnc->vbStream[i].virt_addr, pTestEnc->vbStream[i].size);
    }
#endif

#if REPEAT_SPS
    if (pEncParam->is_idr_frame == TRUE)
    {
        if (pEncOP->bitstreamFormat == STD_AVC)
        {
            ret = cviEncode264Header(pTestEnc);
            if (ret == FALSE)
            {
                CVI_VC_ERR("cviEncode264Header = %d\n", ret);
                return TE_ERR_ENC_OPEN;
            }
            ret = cviH264SpsAddVui(pTestEnc);
            if (ret == FALSE)
            {
                CVI_VC_ERR("cviH264AddTimingInfo = %d\n", ret);
                return TE_ERR_ENC_OPEN;
            }
        }
        else
        {
            ret = cviEncode265Header(pTestEnc);
            if (ret == FALSE)
            {
                CVI_VC_ERR("cviEncode265Header = %d\n", ret);
                return TE_ERR_ENC_OPEN;
            }
            ret = cviH265SpsAddVui(pTestEnc);
            if (ret == FALSE)
            {
                CVI_VC_ERR("cviH265AddTimingInfo = %d\n", ret);
                return TE_ERR_ENC_OPEN;
            }
        }
    }
#endif

    ret = cviInsertUserData(pTestEnc);
    if (ret == FALSE)
    {
        CVI_VC_ERR("cviInsertUserData %d\n", ret);
        return TE_ERR_ENC_USER_DATA;
    }

    if (pEncOP->bitstreamFormat == STD_AVC && pEncOP->EncStdParam.avcParam.svcExtension == TRUE)
        VPU_EncGiveCommand(pTestEnc->handle, ENC_GET_FRAMEBUF_IDX, &pTestEnc->srcFrameIdx);
    else
        pTestEnc->srcFrameIdx = (pTestEnc->frameIdx % pFbAllocInfo->num);

    pEncParam->srcIdx = pTestEnc->srcFrameIdx;

    CVI_VC_TRACE("Read YUV, srcIdx = %d\n", pEncParam->srcIdx);


    if (pTestEnc->encConfig.yuv_mode == SOURCE_YUV_ADDR)
        pYuvAddr = &pTestEnc->yuvAddr;
    else
        pYuvAddr = NULL;

    ret = TRUE;
    if (ret == 0)
    {
        pEncParam->srcEndFlag = 1; // when there is no more source image
                                   // to be encoded, srcEndFlag should
                                   // be set 1. because of encoding
                                   // delay for WAVE420
    }

    if (pEncParam->srcEndFlag != 1)
    {
        if (pTestEnc->encConfig.cviApiMode == API_MODE_SDK)
        {
            pTestEnc->fbSrc[pTestEnc->srcFrameIdx].srcBufState = SRC_BUFFER_USE_ENCODE;
            pEncParam->sourceFrame                             = &pTestEnc->fbSrc[pTestEnc->srcFrameIdx];
            pEncParam->sourceFrame->endian                     = 0x10;
            pEncParam->sourceFrame->sourceLBurstEn             = 0;
            pEncParam->sourceFrame->bufY                       = (PhysicalAddress)pYuvAddr->phyAddrY;
            pEncParam->sourceFrame->bufCb                      = (PhysicalAddress)pYuvAddr->phyAddrCb;
            pEncParam->sourceFrame->bufCr                      = (PhysicalAddress)pYuvAddr->phyAddrCr;
            pEncParam->sourceFrame->stride                     = pTestEnc->srcFrameStride;
            pEncParam->sourceFrame->cbcrInterleave             = pEncOP->cbcrInterleave;
            pEncParam->sourceFrame->nv21                       = pEncOP->nv21;

            CVI_VC_SRC(
                "srcFrameIdx = %d, bufY = 0x" VCODEC_UINT64_FORMAT_HEX ", bufCb = 0x" VCODEC_UINT64_FORMAT_HEX ", bufCr = 0x" VCODEC_UINT64_FORMAT_HEX ", stride = %d\n",
                pTestEnc->srcFrameIdx,
                pEncParam->sourceFrame->bufY,
                pEncParam->sourceFrame->bufCb,
                pEncParam->sourceFrame->bufCr,
                pEncParam->sourceFrame->stride);
        }
        else
        {
            pTestEnc->fbSrc[pTestEnc->srcFrameIdx].srcBufState =
                SRC_BUFFER_USE_ENCODE;
            pEncParam->sourceFrame =
                &pTestEnc->fbSrc[pTestEnc->srcFrameIdx];
            pEncParam->sourceFrame->sourceLBurstEn = 0;
        }
    }

    if (pTestEnc->encOP.ringBufferEnable == FALSE)
    {
        pTestEnc->bsQueueIndex =
            (pTestEnc->bsQueueIndex + 1) % pTestEnc->bsBufferCount;
        pEncParam->picStreamBufferAddr =
            pTestEnc->vbStream[pTestEnc->bsQueueIndex]
                .phys_addr; // can set the newly allocated
                            // buffer.
#ifdef SUPPORT_DONT_READ_STREAM
        pEncParam->picStreamBufferSize =
            pTestEnc->vbStream[pTestEnc->bsQueueIndex].size;
#else
        pEncParam->picStreamBufferSize =
            pTestEnc->encOP.bitstreamBufferSize;
#endif

        CVI_VC_BS(
            "picStreamBufferAddr = 0x" VCODEC_UINT64_FORMAT_HEX ", picStreamBufferSize = 0x%X\n",
            pEncParam->picStreamBufferAddr,
            pEncParam->picStreamBufferSize);
    }

    if ((pTestEnc->encConfig.seiDataEnc.prefixSeiNalEnable || pTestEnc->encConfig.seiDataEnc.suffixSeiNalEnable) && pEncParam->srcEndFlag != 1)
    {
        pTestEnc->encConfig.seiDataEnc.prefixSeiNalAddr =
            pTestEnc->vbPrefixSeiNal[pTestEnc->srcFrameIdx].phys_addr;
        pTestEnc->encConfig.seiDataEnc.suffixSeiNalAddr =
            pTestEnc->vbSuffixSeiNal[pTestEnc->srcFrameIdx].phys_addr;
        VPU_EncGiveCommand(pTestEnc->handle, ENC_SET_SEI_NAL_DATA,
                           &pTestEnc->encConfig.seiDataEnc);
    }

    // set ROI map in dram
    if (pEncOP->bitstreamFormat == STD_HEVC)
    {
        CVI_VC_TRACE("ctu_qp_enable = %d, roi_cfg_type = %d\n",
                     pTestEnc->encConfig.ctu_qp_enable, pTestEnc->encConfig.roi_cfg_type);

        // ctuqp currently can't work with roi
        if (pTestEnc->encConfig.ctu_qp_enable)
        {
#if 0
			setCtuQpMap(coreIdx, &pTestEnc->encConfig, pTestEnc->encOP,
					pTestEnc->vbCtuQp[pTestEnc->srcFrameIdx].phys_addr,
					pCviEc->pu8QpMap, pEncParam, MAX_CTU_NUM);
#else
            cviSetCtuQpMap(coreIdx, &pTestEnc->encConfig, &pTestEnc->encOP,
                           pTestEnc->vbCtuQp[pTestEnc->srcFrameIdx].phys_addr,
                           pCviEc->pu8QpMap, pEncParam, MAX_CTU_NUM);
#endif
        }
        else
        {
            memset(&pEncParam->ctuOptParam, 0, sizeof(HevcCtuOptParam));
            if ((pTestEnc->encConfig.roi_cfg_type == 1 && pTestEnc->encConfig.cviApiMode == API_MODE_DRIVER) || (pTestEnc->encOP.bgEnhanceEn == TRUE && pTestEnc->encConfig.cviApiMode == API_MODE_SDK))
            {
                setRoiMapFromMap(coreIdx, &pTestEnc->encConfig, &pTestEnc->encOP,
                                 pTestEnc->vbRoi[pTestEnc->srcFrameIdx].phys_addr,
                                 &pTestEnc->roiMapBuf[0], pEncParam, MAX_CTU_NUM);
            }
            else
            {
                if (pTestEnc->encConfig.cviApiMode != API_MODE_DRIVER)
                {
                    cviSetWaveRoiBySdk(&pTestEnc->encConfig, pEncParam, &pTestEnc->encOP,
                                       pTestEnc->handle->CodecInfo->encInfo.pic_ctu_avg_qp);
                }
                setRoiMapFromBoundingBox(coreIdx, &pTestEnc->encConfig, &pTestEnc->encOP,
                                         pTestEnc->vbRoi[pTestEnc->srcFrameIdx].phys_addr,
                                         &pTestEnc->roiMapBuf[0], pEncParam, MAX_CTU_NUM);
            }
        }
    }

#ifdef TEST_ENCODE_CUSTOM_HEADER
    if (writeSeiNalData(pTestEnc->handle, pTestEnc->encOP.streamEndian,
                        pTestEnc->vbSeiNal[pTestEnc->srcFrameIdx].phys_addr,
                        &pTestEnc->hrd)
        == FALSE)
    {
        return TE_ERR_ENC_OPEN;
    }
#endif

    if (pTestEnc->encConfig.useAsLongtermPeriod > 0 && pTestEnc->encConfig.refLongtermPeriod > 0)
    {
        pEncParam->useCurSrcAsLongtermPic =
            (pTestEnc->frameIdx % pTestEnc->encConfig.useAsLongtermPeriod) == 0 ? 1 : 0;
        pEncParam->useLongtermRef =
            (pTestEnc->frameIdx % pTestEnc->encConfig.refLongtermPeriod) == 0 ? 1 : 0;
    }

    // Start encoding a frame.
    pTestEnc->frameIdx++;

    ret = VPU_EncStartOneFrame(pTestEnc->handle, pEncParam);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("VPU_EncStartOneFrame failed Error code is 0x%x\n",
                   ret);
        return TE_ERR_ENC_OPEN;
    }

    return ret;
}

static void setInitRcQp(EncChangeParam *changeParam, EncHevcParam *param, EncOpenParam *pEncOP, int initQp)
{
    changeParam->enable_option    |= ENC_RC_PARAM_CHANGE;
    changeParam->initialRcQp       = initQp;
    changeParam->rcEnable          = 1;
    changeParam->cuLevelRCEnable   = param->cuLevelRCEnable;
    changeParam->hvsQPEnable       = param->hvsQPEnable;
    changeParam->hvsQpScaleEnable  = param->hvsQpScaleEnable;
    changeParam->hvsQpScale        = param->hvsQpScale;
    changeParam->bitAllocMode      = param->bitAllocMode;
    changeParam->initBufLevelx8    = param->initBufLevelx8;
    changeParam->initialDelay      = pEncOP->initialDelay;
}

static void setMinMaxQpByDelta(EncHandle handle, EncOpenParam *pEncOP, int deltaQp, int isIFrame, int intraQpOffsetForce0)
{
    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        MinMaxQpChangeParam param;
        param.maxQpIEnable = 1;
        param.maxQpI       = CLIP3(0, 51, pEncOP->userQpMaxI + deltaQp);
        param.minQpIEnable = 1;
        param.minQpI       = CLIP3(0, 51, pEncOP->userQpMinI);
        param.maxQpPEnable = 1;
        param.maxQpP       = CLIP3(0, 51, pEncOP->userQpMaxP + deltaQp);
        param.minQpPEnable = 1;
        param.minQpP       = CLIP3(0, 51, pEncOP->userQpMinP);
        VPU_EncGiveCommand(handle, ENC_SET_MIN_MAX_QP, &param);
    }
    else if (pEncOP->bitstreamFormat == STD_HEVC)
    {
        int            maxQp = (isIFrame) ? pEncOP->userQpMaxI : pEncOP->userQpMaxP;
        int            minQp = (isIFrame) ? pEncOP->userQpMinI : pEncOP->userQpMinP;
        EncChangeParam param;
        param.changeParaMode = OPT_COMMON;
        param.enable_option  = ENC_RC_MIN_MAX_QP_CHANGE;
        param.maxQp          = CLIP3(0, 51, maxQp + deltaQp);
        param.minQp          = CLIP3(0, 51, minQp);
        param.maxDeltaQp     = pEncOP->EncStdParam.hevcParam.maxDeltaQp;
        param.intraQpOffset  = (intraQpOffsetForce0 == 0) ? 0 : pEncOP->EncStdParam.hevcParam.intraQpOffset;

        CVI_VC_TRACE("userQpMaxI = %d, userQpMaxP = %d, userQpMinI = %d, userQpMinP = %d\n",
                     pEncOP->userQpMaxI, pEncOP->userQpMaxP, pEncOP->userQpMinI, pEncOP->userQpMinP);
        CVI_VC_TRACE("minQp = %d, maxQp = %d\n", minQp, maxQp);
        CVI_VC_TRACE("pminQp = %d, pmaxQp = %d, maxDeltaQp = %d, intraQpOffset = %d\n",
                     param.minQp, param.maxQp, param.maxDeltaQp, param.intraQpOffset);

        VPU_EncGiveCommand(handle, ENC_SET_PARA_CHANGE, &param);
    }
}

static BOOL cviCheckIdrValid(EncOpenParam *pEncOP, Uint32 frameIdx)
{
    int keyframe_period = (pEncOP->bitstreamFormat == STD_AVC) ? pEncOP->set_dqp_pic_num : pEncOP->EncStdParam.hevcParam.gopParam.tidPeriod0;
    return (keyframe_period == 0 || (frameIdx % keyframe_period == 0)) ? TRUE : FALSE;
}

static void cviForcePicTypeCtrl(EncOpenParam *pEncOP, EncParam *encParam,
                                EncInfo *pEncInfo, BOOL is_intra_period,
                                BOOL force_idr, BOOL force_skip_frame,
                                Uint32 frameIdx)
{
    encParam->skipPicture            = 0;
    encParam->forceIPicture          = 0;
    encParam->forcePicTypeEnable     = 0;
    pEncInfo->force_as_long_term_ref = 0;

    if (is_intra_period && (!encParam->force_i_for_gop_sync))
    {
        return;
    }

    if (force_idr)
    {
        CVI_VC_TRACE("force IDR request\n");
    }

    encParam->idr_registered |= (force_idr && !is_intra_period);

    if ((encParam->idr_registered && cviCheckIdrValid(pEncOP, frameIdx)) || (is_intra_period && encParam->force_i_for_gop_sync))
    {
        if (pEncOP->bitstreamFormat == STD_HEVC)
        {
            encParam->forcePicTypeEnable = 1;
            encParam->forcePicType       = 3;
        }
        else
        { // encOP->bitstreamFormat == STD_AVC
            encParam->forceIPicture          = 1;
            pEncInfo->force_as_long_term_ref = 1;
            encParam->force_i_for_gop_sync   = encParam->idr_registered;
        }
        encParam->idr_registered  = 0;
        encParam->is_idr_frame   |= 1;
        CVI_VC_TRACE("force IDR ack\n");
    }
    else if (force_skip_frame)
    {
        encParam->skipPicture = 1;
        CVI_VC_TRACE("force skip ack\n");
    }
}

static void cviPicParamChangeCtrl(EncHandle handle, TestEncConfig *pEncConfig,
                                  EncOpenParam *pEncOP, EncParam *encParam, Uint32 frameIdx)
{
    stRcInfo *pRcInfo       = &handle->rcInfo;
    BOOL      rateChangeCmd = FALSE;
    // dynamic bitrate change
    CVI_VC_TRACE("gopSize = %d, frameIdx = %d\n", pEncOP->gopSize,
                 frameIdx);
    if (pRcInfo->rcMode == RC_MODE_AVBR)
    {
        // avbr bitrate change period check
        if (cviEnc_Avbr_PicCtrl(pRcInfo, pEncOP, frameIdx))
        {
            cviEncRc_SetParam(&handle->rcInfo, pEncOP, E_BITRATE);
            rateChangeCmd = TRUE;
        }
    }
    else
    {
        // detect bitrate change
        if (encParam->is_i_period && pEncOP->bitRate != cviEncRc_GetParam(pRcInfo, E_BITRATE))
        {
            cviEncRc_SetParam(&handle->rcInfo, pEncOP, E_BITRATE);
            rateChangeCmd = TRUE;
        }
    }
    // detect framerate change
    if (encParam->is_i_period && (pEncOP->frameRateInfo != cviEncRc_GetParam(pRcInfo, E_FRAMERATE)))
    {
        cviEncRc_SetParam(pRcInfo, pEncOP, E_FRAMERATE);
        rateChangeCmd = TRUE;
    }

    // bitrate/framerate change handle
    if (rateChangeCmd)
    {
        if (pEncOP->bitstreamFormat == STD_AVC)
        {
            VPU_EncGiveCommand(handle, ENC_SET_BITRATE, &pRcInfo->targetBitrate);
            if (pEncOP->rcEnable == 4)
            {
#if 0  //ndef USE_KERNEL_MODE
				int gamma = (pEncOP->userGamma > 1) ? pEncOP->userGamma : 1;
#ifdef CLIP_PIC_DELTA_QP
				int max_delta_qp_minus = (pEncOP->userMinDeltaQp > 0) ? pEncOP->userMinDeltaQp : 51;
				int max_delta_qp_plus = (pEncOP->userMaxDeltaQp > 0) ? pEncOP->userMaxDeltaQp : 51;
#endif // CLIP_PIC_DELTA_QP
				EncInfo *pEncInfo = &handle->CodecInfo->encInfo;
				frm_rc_seq_init(
					&pEncInfo->frm_rc,
					handle->rcInfo.targetBitrate * 1000,
					pEncOP->rcInitDelay,
					pEncOP->frameRateInfo, pEncOP->picWidth,
					pEncOP->picHeight, pEncOP->gopSize, 4,
					pEncOP->set_dqp_pic_num,
					pEncOP->gopEntry,
					pEncOP->LongTermDeltaQp, -1, 0, gamma,
					pEncOP->rcWeightFactor
#ifdef CLIP_PIC_DELTA_QP
					,
					max_delta_qp_minus, max_delta_qp_plus
#endif // CLIP_PIC_DELTA_QP
				);
#endif
            }
        }
        else if (pEncOP->bitstreamFormat == STD_HEVC)
        {
            EncChangeParam changeParam;

            changeParam.changeParaMode = OPT_COMMON;
            changeParam.enable_option  = ENC_RC_TRANS_RATE_CHANGE | ENC_RC_TARGET_RATE_CHANGE | ENC_FRAME_RATE_CHANGE;
            changeParam.bitRate        = handle->rcInfo.targetBitrate;
            changeParam.transRate =
                ((pEncOP->rcMode != RC_MODE_CBR && pEncOP->rcMode != RC_MODE_QPMAP && pEncOP->rcMode != RC_MODE_UBR) || (pEncOP->statTime <= 0)) ? MAX_TRANSRATE : handle->rcInfo.targetBitrate * pEncOP->statTime * 1000;
            changeParam.frameRate = pEncOP->frameRateInfo;

            if (pRcInfo->rcMode == RC_MODE_AVBR)
            {
                EncHevcParam *param  = &pEncOP->EncStdParam.hevcParam;
                int           initQp = (encParam->is_i_period) ? pRcInfo->lastPicQp + (param->intraQpOffset / 2) : pRcInfo->lastPicQp;
                setInitRcQp(&changeParam, param, pEncOP, initQp);
            }

            CVI_VC_TRACE("bitRate = %d, transRate = %d\n",
                         changeParam.bitRate,
                         changeParam.transRate);
            CVI_VC_INFO("bitRate = %d, transRate = %d, frameRate = %d\n",
                        changeParam.bitRate, changeParam.transRate, changeParam.frameRate);
            VPU_EncGiveCommand(handle, ENC_SET_PARA_CHANGE, &changeParam);
        }
    }

    // auto frame-skipping
    CVI_VC_TRACE("gopSize = %d, frameIdx = %d\n", pEncOP->gopSize, frameIdx);

    cviEncRc_UpdateFrameSkipSetting(pRcInfo, pEncConfig->frmLostOpen,
                                    pEncConfig->cviEc.encFrmGaps, pEncConfig->cviEc.frmLostBpsThr);
    do {
        BOOL force_skip_frame = cviEncRc_ChkFrameSkipByBitrate(pRcInfo, encParam->is_i_period);
        if (!force_skip_frame)
        {
            force_skip_frame |= cviEncRc_Avbr_CheckFrameSkip(pRcInfo, pEncOP, encParam->is_i_period);
        }

        cviForcePicTypeCtrl(pEncOP, encParam, &handle->CodecInfo->encInfo,
                            encParam->is_i_period, encParam->idr_request,
                            force_skip_frame, frameIdx);
    } while (0);

    encParam->idr_request = 0;

    CVI_VC_TRACE("skipPicture = %d\n", encParam->skipPicture);

    // TBD: scene change detect signal generate
    /*if(cviEncRc_StateCheck(&handle->rcInfo, FALSE)) {
		int deltaQp = (handle->rcInfo.rcState==UNSTABLE)
			? 2
			: (handle->rcInfo.rcState==RECOVERY)
			? -1 : 0;
		setMinMaxQpByDelta(handle, encOP, deltaQp\);
	}*/

    // avbr adaptive max Qp
    if (pRcInfo->rcEnable && pRcInfo->rcMode == RC_MODE_AVBR)
    {
        if (pRcInfo->avbrChangeBrEn == TRUE)
        {
            int deltaQp             = cviEncRc_Avbr_GetQpDelta(pRcInfo, pEncOP);
            int intraQpOffsetForce0 = (pRcInfo->periodMotionLvRaw > 64) ? 0 : -1;
            CVI_VC_INFO("avbr qp delta: %d %d\n", deltaQp, encParam->is_i_period);
            setMinMaxQpByDelta(handle, pEncOP, deltaQp, encParam->is_i_period, intraQpOffsetForce0);
            pRcInfo->avbrChangeBrEn = FALSE;
        }
    }
}

static int cviGetStream(stTestEncoder *pTestEnc)
{
    int            ret         = 0;
    EncOutputInfo *pOutputInfo = &pTestEnc->outputInfo;
    EncOpenParam  *peop        = &pTestEnc->encOP;

    ret = cviGetEncodedInfo(pTestEnc, TIME_BLOCK_MODE);
    if (ret == TE_STA_ENC_TIMEOUT)
    {
        return RET_VCODEC_TIMEOUT;
    }
    else if (ret)
    {
        CVI_VC_ERR("cviGetEncodedInfo, ret = %d\n", ret);
        return ret;
    }

    if (peop->ringBufferEnable == 0)
    {
        ret = cviCheckOutputInfo(pTestEnc);
        if (ret == TE_ERR_ENC_OPEN)
        {
            CVI_VC_ERR("cviCheckOutputInfo, ret = %d\n", ret);
            return ret;
        }

#ifdef SUPPORT_DONT_READ_STREAM
        updateBitstream(&pTestEnc->vbStream[0],
                        pOutputInfo->bitstreamSize);
#else
        if (pOutputInfo->bitstreamSize)
        {
            ret = BitstreamReader_Act(
                pTestEnc->handle,
                pOutputInfo->bitstreamBuffer,
                peop->bitstreamBufferSize,
                pOutputInfo->bitstreamSize,
                NULL);

            if (ret == FALSE)
            {
                CVI_VC_ERR("ES, ret = %d\n", ret);
                return TE_ERR_ENC_OPEN;
            }
        }
#endif
    }

    ret = cviCheckEncodeEnd(pTestEnc);

    return ret;
}

static int cviHandlePartialBitstream(stTestEncoder *pTestEnc, BOOL bStreamEnd)
{
    int             ret        = RETCODE_SUCCESS;
    CodecInst      *pCodecInst = pTestEnc->handle;
    EncInfo        *pEncInfo   = &pCodecInst->CodecInfo->encInfo;
    PhysicalAddress paRdPtr, paWrPtr;
    Uint8          *vaRdPtr;
    int             size = 0;

    ret = VPU_EncGetBitstreamBuffer(pTestEnc->handle, &paRdPtr, &paWrPtr, &size);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("VPU_EncGetBitstreamBuffer fail with ret %d\n", ret);
        return ret;
    }

    vaRdPtr = vdi_get_vir_addr(pTestEnc->coreIdx, paRdPtr);

    CVI_VC_TRACE("paRdPtr 0x" VCODEC_UINT64_FORMAT_HEX ", paWrPtr 0x" VCODEC_UINT64_FORMAT_HEX ", size 0x%x\n", paRdPtr, paWrPtr, size);

    // using heap to save partial bitstream,
    // if partial encoded bitstream size is larger than free bitstream buffer size:
    //    => RdPtr is less than (streamBufStartAddr + partial bitstream size)
    // otherwise, rotate the partial bitstream to the start addr. of bitstream buffer
    if ((pEncInfo->streamBufStartAddr + size) > paRdPtr)
    {
        Uint8 *pu8PartialBitstreamBuf;
        pu8PartialBitstreamBuf = malloc(pTestEnc->u32PreStreamLen + size);
        if (!pu8PartialBitstreamBuf)
        {
            CVI_VC_ERR("Can not allocate memory for partial bitstream\n");
            return RETCODE_FAILURE;
        }

        CVI_VC_WARN("heap allocation (0x%X) for BIT_BUF_FULL interrupt!\n",
                    pTestEnc->u32PreStreamLen + size);

#if (defined CVI_H26X_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
        vdi_invalidate_ion_cache(paRdPtr, vaRdPtr, size);
#endif
        if (pTestEnc->pu8PreStream)
        {
            memcpy(pu8PartialBitstreamBuf, pTestEnc->pu8PreStream, pTestEnc->u32PreStreamLen);
            memcpy(pu8PartialBitstreamBuf + pTestEnc->u32PreStreamLen, vaRdPtr, size);
            free(pTestEnc->pu8PreStream);
            pTestEnc->pu8PreStream = NULL;
        }
        else
        {
            memcpy(pu8PartialBitstreamBuf, vaRdPtr, size);
        }

        pTestEnc->pu8PreStream     = pu8PartialBitstreamBuf;
        pTestEnc->u32PreStreamLen += size;

        CVI_VC_TRACE("current partial bitstream size 0x%x\n", pTestEnc->u32PreStreamLen);
    }
    else
    {
#if (defined CVI_H26X_USE_ION_MEM && defined BITSTREAM_ION_CACHED_MEM)
        vdi_invalidate_ion_cache(paRdPtr, vaRdPtr, size);
#endif
        memcpy((pTestEnc->vbStream[0].virt_addr + pTestEnc->u32BsRotateOffset), vaRdPtr, size);

        pTestEnc->u32BsRotateOffset += size;

        CVI_VC_TRACE("u32BsRotateOffset 0x%x, bStreamEnd %d\n", pTestEnc->u32BsRotateOffset, bStreamEnd);

        if (bStreamEnd)
        {
            // to update both RdPtr/WrPtr for later VPU_EncGetOutputInfo()

            // update WrPtr to (BsStartAddr + BsRotateOffset)
            VpuWriteReg(pCodecInst->coreIdx, pEncInfo->streamWrPtrRegAddr,
                        pEncInfo->streamBufStartAddr + pTestEnc->u32BsRotateOffset);
            pEncInfo->streamWrPtr = pEncInfo->streamBufStartAddr + pTestEnc->u32BsRotateOffset;
            // update RdPtr to (BsStartAddr)
            VpuWriteReg(pTestEnc->coreIdx, pEncInfo->streamRdPtrRegAddr, pEncInfo->streamBufStartAddr);
            pEncInfo->streamRdPtr = pEncInfo->streamBufStartAddr;

            pTestEnc->u32BsRotateOffset = 0;
            pTestEnc->bBufferFullFlag = TRUE;
            CVI_VC_TRACE("streamRdPtr 0x" VCODEC_UINT64_FORMAT_HEX ", streamWrPtr 0x" VCODEC_UINT64_FORMAT_HEX "\n", pEncInfo->streamRdPtr, pEncInfo->streamWrPtr);
        }
    }

    return RETCODE_SUCCESS;
}

static int cviGetEncodedInfo(stTestEncoder *pTestEnc, int s32MilliSec)
{
    EncParam *pEncParam    = &pTestEnc->encParam;
    int       coreIdx      = pTestEnc->coreIdx;
    int       int_reason   = 0;
    Uint32    timeoutCount = 0;
    int       ret          = RETCODE_SUCCESS;
    CodStd    curCodec;

    while (1)
    {
        int_reason = VPU_WaitInterrupt(coreIdx, s32MilliSec);

        CVI_VC_TRACE("int_reason = 0x%X, timeoutCount = %d\n", int_reason, timeoutCount);

        if (int_reason == -1)
        {
            if (s32MilliSec >= 0)
            {
                CVI_VC_INFO("Error : encoder timeout happened in non_block mode\n");
                return TE_STA_ENC_TIMEOUT;
            }

            if (pTestEnc->interruptTimeout > 0 && timeoutCount * s32MilliSec > pTestEnc->interruptTimeout)
            {
                CVI_VC_ERR("Error : encoder timeout happened\n");
#ifdef ENABLE_CNM_DEBUG_MSG
                PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
                VPU_SWReset(coreIdx, SW_RESET_SAFETY,
                            pTestEnc->handle);
                break;
            }
            CVI_VC_TRACE("VPU_WaitInterrupt ret -1, retry timeoutCount[%d]\n", timeoutCount);

            int_reason = 0;
            timeoutCount++;
        }

        if (pTestEnc->encOP.ringBufferEnable == TRUE)
        {
            if (!BitstreamReader_Act(
                    pTestEnc->handle,
                    pTestEnc->encOP.bitstreamBuffer,
                    pTestEnc->encOP.bitstreamBufferSize,
                    STREAM_READ_SIZE,
                    NULL))
            {
#ifdef ENABLE_CNM_DEBUG_MSG
                PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
                break;
            }
        }

        if (int_reason & (1 << INT_BIT_BIT_BUF_FULL))
        {
            CVI_VC_WARN("INT_BIT_BIT_BUF_FULL\n");
            if (pTestEnc->encConfig.cviApiMode == API_MODE_SDK)
            {
                if (cviHandlePartialBitstream(pTestEnc, FALSE) != RETCODE_SUCCESS)
                {
                    CVI_VC_ERR("cviHandlePartialBitstream fail\n");
                    return RETCODE_FAILURE;
                }
            }
            else
            {
                BitstreamReader_Act(
                    pTestEnc->handle,
                    pTestEnc->encOP.bitstreamBuffer,
                    pTestEnc->encOP.bitstreamBufferSize,
                    STREAM_READ_ALL_SIZE,
                    NULL);
            }
            if (pTestEnc->encConfig.stdMode == STD_HEVC)
            {
                // CnM workaround for BUF_FULL interrupt (BITMONET-68)
                VpuWriteReg(coreIdx, 0x1EC, 0);
            }
        }

        if (int_reason)
        {
            VPU_ClearInterrupt(coreIdx);
            if (int_reason & (1 << INT_WAVE_ENC_PIC))
            {
                break;
            }
        }
    }

    if (pTestEnc->encOP.bEsBufQueueEn == 1 && pTestEnc->u32BsRotateOffset != 0)
    {
        if (cviHandlePartialBitstream(pTestEnc, TRUE) != RETCODE_SUCCESS)
        {
            CVI_VC_ERR("cviHandlePartialBitstream fail\n");
            return RETCODE_FAILURE;
        }
    }

    ret = VPU_EncGetOutputInfo(pTestEnc->handle, &pTestEnc->outputInfo);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("VPU_EncGetOutputInfo failed Error code is 0x%x\n", ret);
        if (ret == RETCODE_STREAM_BUF_FULL)
        {
#ifdef ENABLE_CNM_DEBUG_MSG
            PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
        }
        else if (ret == RETCODE_MEMORY_ACCESS_VIOLATION || ret == RETCODE_CP0_EXCEPTION || ret == RETCODE_ACCESS_VIOLATION_HW)
        {
#ifdef ENABLE_CNM_DEBUG_MSG
            PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
            VPU_SWReset(coreIdx, SW_RESET_SAFETY, pTestEnc->handle);
        }
        else
        {
#ifdef ENABLE_CNM_DEBUG_MSG
            PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
            VPU_SWReset(coreIdx, SW_RESET_SAFETY, pTestEnc->handle);
        }

        CVI_VC_ERR("VPU_EncGetOutputInfo, ret = %d\n", ret);
        return TE_ERR_ENC_OPEN;
    }

    cviEncRc_UpdatePicInfo(&pTestEnc->handle->rcInfo, &pTestEnc->outputInfo);

    if (coreIdx == 0)
        curCodec = STD_HEVC;
    else
        curCodec = STD_AVC;

    DisplayEncodedInformation(pTestEnc->handle, curCodec,
                              &pTestEnc->outputInfo, pEncParam->srcEndFlag,
                              pTestEnc->srcFrameIdx);

    return ret;
}

static int cviCheckOutputInfo(stTestEncoder *pTestEnc)
{
    int ret = RETCODE_SUCCESS;

    if (pTestEnc->outputInfo.bitstreamWrapAround == 1)
    {
        CVI_VC_WARN("Warnning!! BitStream buffer wrap arounded. prepare more large buffer \n");
    }

    if (pTestEnc->outputInfo.bitstreamSize == 0 && pTestEnc->outputInfo.reconFrameIndex >= 0)
    {
        CVI_VC_ERR("ERROR!!! bitstreamsize = 0\n");
    }

    if (pTestEnc->encOP.lineBufIntEn == 0)
    {
        if (pTestEnc->outputInfo.wrPtr < pTestEnc->outputInfo.rdPtr)
        {
            CVI_VC_ERR("wrptr < rdptr\n");
            return TE_ERR_ENC_OPEN;
        }
    }

    return ret;
}

static int cviCheckEncodeEnd(stTestEncoder *pTestEnc)
{
    EncParam *pEncParam = &pTestEnc->encParam;
    int       ret       = 0;

    // end of encoding
    if (pTestEnc->outputInfo.reconFrameIndex == -1)
    {
        CVI_VC_INFO("reconFrameIndex = -1\n");
        return TE_STA_ENC_BREAK;
    }

#ifdef ENC_RECON_FRAME_DISPLAY
    SimpleRenderer_Act();
#endif

    if (pTestEnc->frameIdx > (Uint32)MAX(0, (pTestEnc->encConfig.outNum - 1)))
    {
        if (pTestEnc->encOP.bitstreamFormat == STD_AVC)
            ret = TE_STA_ENC_BREAK;
        else
            pEncParam->srcEndFlag = 1;
    }

    return ret;
}

static int cviCheckAndCompareBitstream(stTestEncoder *pTestEnc)
{
#ifdef ENABLE_CNM_DEBUG_MSG
    int coreIdx = pTestEnc->coreIdx;
#endif
    int ret = 0;

    if (pTestEnc->encOP.ringBufferEnable == 1)
    {
        ret = BitstreamReader_Act(pTestEnc->handle,
                                  pTestEnc->encOP.bitstreamBuffer,
                                  pTestEnc->encOP.bitstreamBufferSize,
                                  0, NULL);
        if (ret == FALSE)
        {
#ifdef ENABLE_CNM_DEBUG_MSG
            PrintVpuStatus(coreIdx, pTestEnc->productId);
#endif
            return TE_ERR_ENC_OPEN;
        }
    }

    return ret;
}

static void cviCloseVpuEnc(stTestEncoder *pTestEnc)
{
    EncOpenParam         *pEncOP       = &pTestEnc->encOP;
    int                   coreIdx      = pTestEnc->coreIdx;
    FrameBufferAllocInfo *pFbAllocInfo = &pTestEnc->fbAllocInfo;
    int                   i;
    int                   buf_cnt;
    cviEncCfg            *pcviEc = &pTestEnc->encConfig.cviEc;

    if (pTestEnc->encConfig.cviEc.singleLumaBuf > 0)
    {
        buf_cnt = 1;
        // smart-p with additional 1 framebuffer
        if (pTestEnc->encConfig.cviEc.virtualIPeriod > 0)
        {
            buf_cnt += 1;
        }
    }
    else
    {
        buf_cnt = pTestEnc->regFrameBufCount;
    }

    if (pcviEc->VbBufCfg.VbType == VB_SOURCE_TYPE_PRIVATE || pcviEc->VbBufCfg.VbType == VB_SOURCE_TYPE_USER)
    {
        //skip dma free under non common mode
    }
    else
    {
        for (i = 0; i < buf_cnt; i++)
        {
            if (pTestEnc->vbReconFrameBuf[i].size > 0)
            {
                VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbReconFrameBuf[i]);
            }
        }
    }
    for (i = 0; i < pFbAllocInfo->num; i++)
    {
        if (pTestEnc->vbSourceFrameBuf[i].size > 0)
        {
            VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbSourceFrameBuf[i]);
        }
        if (pTestEnc->vbRoi[i].size)
        {
            VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbRoi[i]);
        }
        if (pTestEnc->vbCtuQp[i].size)
        {
            VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbCtuQp[i]);
        }
        if (pTestEnc->vbPrefixSeiNal[i].size)
            VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbPrefixSeiNal[i]);
        if (pTestEnc->vbSuffixSeiNal[i].size)
            VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbSuffixSeiNal[i]);
#ifdef TEST_ENCODE_CUSTOM_HEADER
        if (pTestEnc->vbSeiNal[i].size)
            VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbSeiNal[i]);
#endif
    }

    if (pTestEnc->vbHrdRbsp.size)
        VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbHrdRbsp);

    if (pTestEnc->vbVuiRbsp.size)
        VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbVuiRbsp);

#ifdef TEST_ENCODE_CUSTOM_HEADER
    if (pTestEnc->vbCustomHeader.size)
        VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbCustomHeader);
#endif

    VPU_EncClose(pTestEnc->handle);
    CoreSleepWake(coreIdx, 1); //ensure vpu sleep

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        CVI_VC_INFO("\ninst %d Enc End. Tot Frame %d\n", pTestEnc->encConfig.instNum, pTestEnc->outputInfo.encPicCnt);
    }
    else
    {
        CVI_VC_INFO("\ninst %d Enc End. Tot Frame %d\n", pTestEnc->encConfig.instNum, pTestEnc->outputInfo.encPicCnt);
    }
}

static void cviDeInitVpuEnc(stTestEncoder *pTestEnc)
{
    int    coreIdx = pTestEnc->coreIdx;
    Uint32 i       = 0;

    for (i = 0; i < pTestEnc->bsBufferCount; i++)
    {
        if (pTestEnc->vbStream[i].size)
        {
            if (vdi_get_is_single_es_buf(coreIdx))
                vdi_free_single_es_buf_memory(coreIdx, &pTestEnc->vbStream[i]);
            else
                VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbStream[i]);
        }
    }

    if (pTestEnc->vbReport.size)
        VDI_FREE_MEMORY(coreIdx, &pTestEnc->vbReport);

    VPU_DeInit(coreIdx);
}

static void *cviInitEncoder(stTestEncoder *pTestEnc, TestEncConfig *pEncConfig)
{
    int ret = 0;

    ret = initEncoder(pTestEnc, pEncConfig);
    if (ret == TE_ERR_ENC_INIT)
        goto INIT_ERR_ENC_INIT;
    else if (ret == TE_ERR_ENC_OPEN)
        goto INIT_ERR_ENC_OPEN;

    return (void *)pTestEnc;

INIT_ERR_ENC_OPEN:
    cviCloseVpuEnc(pTestEnc);
INIT_ERR_ENC_INIT:
    cviDeInitVpuEnc(pTestEnc);
    return NULL;
}

int initEncoder(stTestEncoder *pTestEnc, TestEncConfig *pEncConfig)
{
    int ret = 0;

    VDI_POWER_ON_DOING_JOB(pEncConfig->coreIdx, ret, initEncOneFrame(pTestEnc, pEncConfig));

    if (ret == TE_ERR_ENC_INIT)
    {
        CVI_VC_ERR("TE_ERR_ENC_INIT\n");
        return ret;
    }
    else if (ret == TE_ERR_ENC_OPEN)
    {
        // TODO -- error handling
        CVI_VC_ERR("TE_ERR_ENC_OPEN\n");
        return ret;
    }

    return ret;
}

int cviVEncEncOnePic(void *handle, cviEncOnePicCfg *pPicCfg, int s32MilliSec)
{
    int            ret      = 0;
    stTestEncoder *pTestEnc = (stTestEncoder *)handle;
    EncOpenParam  *pEncOP   = &pTestEnc->encOP;
    // CodecInst *pCodecInst = pTestEnc->handle;

    CVI_VC_IF("\n");

    if (tVencSbmInfo.bFRModeEn)
    {
#if 0
		if (!wait_event_timeout(tSwitchCoreIdxWaitQueue,
								(pCodecInst->s32ChnNum == tVencSbmInfo.targetChnNum),
								msecs_to_jiffies(s32MilliSec))) {
			return RET_VCODEC_TIMEOUT;
		}
#else
        SEM_WAIT(tSwitchCoreIdxWaitQueue);
#endif
        MUTEX_LOCK(sSwitchCoreIdxSem);
        tVencSbmInfo.targetChnNum = pickupNextEncChnNum();
        MUTEX_UNLOCK(sSwitchCoreIdxSem);

        // mutex lock should be lock in blocking mode to ensure switching encoding channel
        s32MilliSec = -1;
    }
    ret = cviVPU_LockAndCheck(pTestEnc->handle, CODEC_STAT_ENC_PIC, s32MilliSec);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_TRACE("cviVPU_LockAndCheck, ret = %d\n", ret);
        if (ret == RETCODE_VPU_RESPONSE_TIMEOUT)
        {
            return RET_VCODEC_TIMEOUT;
            //timeout return
        }
        else
        {
            CVI_VC_ERR("cviVPU_LockAndCheck error[%d]s32MilliSec[%d]\n", ret, s32MilliSec);
            return ret;
        }
    }

    pTestEnc->yuvAddr.addrY  = pPicCfg->addrY;
    pTestEnc->yuvAddr.addrCb = pPicCfg->addrCb;
    pTestEnc->yuvAddr.addrCr = pPicCfg->addrCr;

    pTestEnc->yuvAddr.phyAddrY  = pPicCfg->phyAddrY;
    pTestEnc->yuvAddr.phyAddrCb = pPicCfg->phyAddrCb;
    pTestEnc->yuvAddr.phyAddrCr = pPicCfg->phyAddrCr;

    pTestEnc->srcFrameStride = pPicCfg->stride;
    pEncOP->cbcrInterleave   = pPicCfg->cbcrInterleave;
    pEncOP->nv21             = pPicCfg->nv21;
    pEncOP->picMotionLevel   = pPicCfg->picMotionLevel;
    pEncOP->picMotionMap     = pPicCfg->picMotionMap;
    pEncOP->picMotionMapSize = pPicCfg->picMotionMapSize;

    CVI_VC_TRACE("addrY = 0x%p, addrCb = 0x%p, addrCr = 0x%p\n",
                 pTestEnc->yuvAddr.addrY, pTestEnc->yuvAddr.addrCb,
                 pTestEnc->yuvAddr.addrCr);
    CVI_VC_TRACE(
        "phyAddrY = 0x" VCODEC_UINT64_FORMAT_HEX ", phyAddrCb = 0x" VCODEC_UINT64_FORMAT_HEX ", phyAddrCr = 0x" VCODEC_UINT64_FORMAT_HEX "\n",
        pTestEnc->yuvAddr.phyAddrY, pTestEnc->yuvAddr.phyAddrCb,
        pTestEnc->yuvAddr.phyAddrCr);
    CVI_VC_TRACE("motion lv = %d, mSize = %d\n", pPicCfg->picMotionLevel, pPicCfg->picMotionMapSize);

    ret = cviEncodeOneFrame(pTestEnc);

    if (ret == TE_ERR_ENC_OPEN || ret == TE_STA_ENC_BREAK)
    {
        CVI_VC_INFO("cviEncodeOneFrame, ret = %d\n", ret);
        cviVPU_ChangeState(pTestEnc->handle);
        LeaveVcodecLock(pTestEnc->coreIdx);
    }
    else if (ret == RETCODE_SUCCESS && pTestEnc->encConfig.bIsoSendFrmEn && !pTestEnc->handle->rcInfo.isReEncodeIdr)
    {
        SEM_POST(pTestEnc->semSendEncCmd);
    }

    return ret;
}

int cviVEncGetStream(void *handle, cviVEncStreamInfo *pStreamInfo, int s32MilliSec)
{
    stTestEncoder *pTestEnc = (stTestEncoder *)handle;
    int            ret      = RETCODE_SUCCESS;

    // 	CVI_VC_IF("\n");

    // 	if (pTestEnc->encConfig.bIsoSendFrmEn) {
    // #ifdef USE_POSIX
    // 		struct timespec tsCurrent;
    // 		// struct timespec tsTimeout;

    // 		// milliSec2TimeSpec(&tsTimeout, (unsigned long)s32MilliSec);

    // 		if (clock_gettime(CLOCK_REALTIME, &tsCurrent) == -1) {
    // 			return RETCODE_FAILURE;
    // 		}

    // 		tsCurrent.tv_sec += s32MilliSec / 1000;
    // 		tsCurrent.tv_nsec += (s32MilliSec % (1000)) * (1000 * 1000);

    // 		// if (tsCurrent.tv_nsec >= (1000 * 1000 * 1000)) {
    // 		// 	tsCurrent.tv_sec++;
    // 		// 	tsCurrent.tv_nsec -= (1000 * 1000 * 1000);
    // 		// }

    // 		// wait for encode done
    // 		if (sem_timedwait(&pTestEnc->semGetStreamCmd, &tsCurrent) != 0) {
    // 			CVI_VC_WARN("get stream timeout!\n");
    // 			return RET_VCODEC_TIMEOUT;
    // 		}
    // #else
    // 		SEM_WAIT(pTestEnc->semGetStreamCmd);
    // #endif
    // 		memcpy(pStreamInfo, &pTestEnc->tStreamInfo, sizeof(cviVEncStreamInfo));
    // 		return RETCODE_SUCCESS;
    // 	}

    pTestEnc->encConfig.cviEc.originPicType = PIC_TYPE_MAX;

    ret = cviGetOneStream(handle, pStreamInfo, s32MilliSec);

    if (ret == TE_ERR_ENC_IS_SUPER_FRAME)
        ret = cviProcessSuperFrame(pTestEnc, pStreamInfo, s32MilliSec);

    return ret;
}

static int cviGetOneStream(void *handle, cviVEncStreamInfo *pStreamInfo, int s32MilliSec)
{
    stTestEncoder *pTestEnc    = (stTestEncoder *)handle;
    cviEncCfg     *pCviEc      = &pTestEnc->encConfig.cviEc;
    EncOpenParam  *peop        = &pTestEnc->encOP;
    EncOutputInfo *pOutputInfo = &pTestEnc->outputInfo;
    int            ret         = RETCODE_SUCCESS;

    CVI_VC_IF("\n");

    ret = cviGetEncodedInfo(pTestEnc, s32MilliSec);
    if (ret == TE_STA_ENC_TIMEOUT)
    {
        //user should keep get frame until success
        CVI_VC_TRACE("Time out please retry\n");
        return RET_VCODEC_TIMEOUT;
    }
    else if (ret)
    {
        CVI_VC_ERR("cviGetEncodedInfo, ret = %d\n", ret);
        goto ERR_CVI_VENC_GET_STREAM;
    }

    pStreamInfo->encHwTime = pOutputInfo->encHwTime;
    pStreamInfo->u32MeanQp = pOutputInfo->u32MeanQp;

    if (pTestEnc->encOP.ringBufferEnable == 0)
    {
        ret = cviCheckOutputInfo(pTestEnc);
        if (ret == TE_ERR_ENC_OPEN)
        {
            CVI_VC_ERR("cviCheckOutputInfo, ret = %d\n", ret);
            goto ERR_CVI_VENC_GET_STREAM;
        }

        if (pOutputInfo->bitstreamSize)
        {
            ret = cviPutEsInPack(pTestEnc,
                                 pOutputInfo->bitstreamBuffer,
                                 pOutputInfo->bitstreamBuffer + peop->bitstreamBufferSize,
                                 pOutputInfo->bitstreamSize,
                                 NAL_I + pOutputInfo->picType);

            if (ret == FALSE)
            {
                CVI_VC_ERR("cviPutEsInPack, ret = %d\n", ret);
                ret = TE_ERR_ENC_OPEN;
                goto ERR_CVI_VENC_GET_STREAM;
            }
            else
            {
                ret = RETCODE_SUCCESS;
            }
        }

        if (pOutputInfo->bitstreamSize)
            pStreamInfo->psp = &pTestEnc->streamPack;

        CVI_VC_TRACE("frameIdx = %d, isLastPicI = %d, size = %d\n",
                     pTestEnc->frameIdx,
                     (pOutputInfo->picType == PIC_TYPE_I) || (pOutputInfo->picType == PIC_TYPE_IDR),
                     pOutputInfo->bitstreamSize << 3);

        if (pCviEc->enSuperFrmMode != CVI_SUPERFRM_NONE)
            ret = cviCheckSuperFrame(pTestEnc);
    }

ERR_CVI_VENC_GET_STREAM:
    cviVPU_ChangeState(pTestEnc->handle);
    // if NOT sharing es buffer, we can unlock to speed-up enc process
    if (!pCviEc->bSingleEsBuf)
    {
        LeaveVcodecLock(pTestEnc->coreIdx);
    }

    return ret;
}

static int cviCheckSuperFrame(stTestEncoder *pTestEnc)
{
    EncOutputInfo *pOutputInfo  = &pTestEnc->outputInfo;
    unsigned int   currFrmBits  = pOutputInfo->bitstreamSize << 3;
    cviEncCfg     *pCviEc       = &pTestEnc->encConfig.cviEc;
    stRcInfo      *pRcInfo      = &pTestEnc->handle->rcInfo;
    int            isSuperFrame = 0;

    if (pCviEc->originPicType == PIC_TYPE_MAX)
        pCviEc->originPicType = pOutputInfo->picType;

    if (pCviEc->originPicType == PIC_TYPE_I || pCviEc->originPicType == PIC_TYPE_IDR)
    {
        CVI_VC_FLOW("currFrmBits = %d, SuperIFrmBitsThr = %d\n", currFrmBits, pCviEc->u32SuperIFrmBitsThr);

        if (currFrmBits > pCviEc->u32SuperIFrmBitsThr)
        {
            isSuperFrame                = TE_ERR_ENC_IS_SUPER_FRAME;
            pRcInfo->s32SuperFrmBitsThr = (int)pCviEc->u32SuperIFrmBitsThr;
        }
    }
    else if (pCviEc->originPicType == PIC_TYPE_P)
    {
        CVI_VC_FLOW("currFrmBits = %d, SuperPFrmBitsThr = %d\n", currFrmBits, pCviEc->u32SuperPFrmBitsThr);

        if (currFrmBits > pCviEc->u32SuperPFrmBitsThr)
        {
            isSuperFrame                = TE_ERR_ENC_IS_SUPER_FRAME;
            pRcInfo->s32SuperFrmBitsThr = (int)pCviEc->u32SuperPFrmBitsThr;
        }
    }

    return isSuperFrame;
}

static int cviProcessSuperFrame(stTestEncoder *pTestEnc, cviVEncStreamInfo *pStreamInfo, int s32MilliSec)
{
    cviEncCfg *pCviEc            = &pTestEnc->encConfig.cviEc;
    stRcInfo  *pRcInfo           = &pTestEnc->handle->rcInfo;
    int        currReEncodeTimes = 0;
    int        ret               = 0;

    pRcInfo->isReEncodeIdr = 1;

    while (currReEncodeTimes++ < pCviEc->s32MaxReEncodeTimes)
    {
        ret = cviReEncodeIDR(pTestEnc, pStreamInfo, s32MilliSec);
        if (ret != TE_ERR_ENC_IS_SUPER_FRAME)
            break;
    }

    if (ret == TE_ERR_ENC_IS_SUPER_FRAME)
    {
        CVI_VC_FLOW("only re-Enccode IDR %d time\n", pCviEc->s32MaxReEncodeTimes);
        ret = 0;
    }

    pRcInfo->isReEncodeIdr = 0;
    CVI_VC_FLOW("re-Enccode IDR %d time\n", pCviEc->s32MaxReEncodeTimes);

    return ret;
}

static int cviReEncodeIDR(stTestEncoder *pTestEnc, cviVEncStreamInfo *pStreamInfo, int s32MilliSec)
{
    EncOpenParam   *pEncOP = &pTestEnc->encOP;
    cviEncOnePicCfg picCfg, *pPicCfg = &picCfg;
    int             ret = 0;

    CVI_VC_FLOW("\n");

    pPicCfg->addrY  = pTestEnc->yuvAddr.addrY;
    pPicCfg->addrCb = pTestEnc->yuvAddr.addrCb;
    pPicCfg->addrCr = pTestEnc->yuvAddr.addrCr;

    pPicCfg->phyAddrY  = pTestEnc->yuvAddr.phyAddrY;
    pPicCfg->phyAddrCb = pTestEnc->yuvAddr.phyAddrCb;
    pPicCfg->phyAddrCr = pTestEnc->yuvAddr.phyAddrCr;

    pPicCfg->stride           = pTestEnc->srcFrameStride;
    pPicCfg->cbcrInterleave   = pEncOP->cbcrInterleave;
    pPicCfg->nv21             = pEncOP->nv21;
    pPicCfg->picMotionLevel   = pEncOP->picMotionLevel;
    pPicCfg->picMotionMap     = pEncOP->picMotionMap;
    pPicCfg->picMotionMapSize = pEncOP->picMotionMapSize;

    pTestEnc->encParam.idr_request = TRUE;

    ret = cviVEncReleaseStream((void *)pTestEnc, pStreamInfo);
    if (ret)
    {
        CVI_VC_ERR("cviVEncReleaseStream, %d\n", ret);
        return ret;
    }

    ret = cviVEncEncOnePic((void *)pTestEnc, pPicCfg, -1);
    if (ret != 0)
    {
        CVI_VC_ERR("cviVEncEncOnePic, %d\n", ret);
        return ret;
    }

    ret = cviGetOneStream((void *)pTestEnc, pStreamInfo, s32MilliSec);
    if (ret != 0 && ret != TE_ERR_ENC_IS_SUPER_FRAME)
    {
        CVI_VC_ERR("cviGetOneStream, %d\n", ret);
        return ret;
    }

    return ret;
}

int cviVEncReleaseStream(void *handle, cviVEncStreamInfo *pStreamInfo)
{
    stTestEncoder *pTestEnc = (stTestEncoder *)handle;
    cviEncCfg     *pCviEc   = &pTestEnc->encConfig.cviEc;
    stStreamPack  *psp      = pStreamInfo->psp;
    stPack        *pPack;
    int            ret = 0;
    int            idx;

    CVI_VC_IF("\n");

    if (psp->totalPacks)
    {
        for (idx = 0; idx < psp->totalPacks; idx++)
        {
            pPack = &psp->pack[idx];
            if (pPack->addr && pPack->need_free)
            {
                free(pPack->addr);
                pPack->addr = NULL;
            }
        }

        psp->totalPacks = 0;
    }

    ret = cviCheckEncodeEnd(pTestEnc);
    if (ret == TE_STA_ENC_BREAK)
    {
        CVI_VC_INFO("TE_STA_ENC_BREAK\n");
    }

    // if sharing es buffer, to unlock when releasing frame
    if (pCviEc->bSingleEsBuf)
    {
        LeaveVcodecLock(pTestEnc->coreIdx);
    }

    return ret;
}

static int cviVEncSetRequestIDR(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;

    UNREFERENCED_PARAMETER(arg);

    CVI_VC_IF("\n");

    pTestEnc->encParam.idr_request = TRUE;

    return ret;
}

int _cviVencCalculateBufInfo(stTestEncoder *pTestEnc, int *butcnt, int *bufsize)
{
    int FrameBufSize;
    int framebufStride = 0;

    if (pTestEnc->bIsEncoderInited == FALSE)
    {
        CVI_VC_ERR("\n");
        return TE_ERR_ENC_INIT;
    }

    // reference frame
    // calculate stride / size / apply variable
    framebufStride = CalcStride(pTestEnc->framebufWidth,
                                pTestEnc->framebufHeight,
                                pTestEnc->encOP.srcFormat,
                                pTestEnc->encOP.cbcrInterleave,
                                (TiledMapType)(pTestEnc->encConfig.mapType & 0x0f),
                                FALSE, FALSE);
    FrameBufSize   = VPU_GetFrameBufSize(
        pTestEnc->encConfig.coreIdx,
        framebufStride,
        pTestEnc->framebufHeight,
        (TiledMapType)(pTestEnc->encConfig.mapType & 0x0f),
        pTestEnc->encOP.srcFormat,
        pTestEnc->encOP.cbcrInterleave,
        NULL);

    pTestEnc->regFrameBufCount = pTestEnc->initialInfo.minFrameBufferCount;
    *butcnt                    = pTestEnc->regFrameBufCount;
    *bufsize                   = FrameBufSize;

    return 0;
}

static int cviVEncGetVbInfo(stTestEncoder *pTestENC, void *arg)
{
    int                 ret           = 0;
    cviVbVencBufConfig *pcviVbVencCfg = (cviVbVencBufConfig *)arg;

    CVI_VC_IF("\n");

    ret = _cviVencCalculateBufInfo(pTestENC, &pcviVbVencCfg->VbGetFrameBufCnt, &pcviVbVencCfg->VbGetFrameBufSize);

    return ret;
}

static int cviVEncSetVbBuffer(stTestEncoder *pTestEnc, void *arg)
{
    int                 ret           = 0;
    cviVbVencBufConfig *pcviVbVencCfg = (cviVbVencBufConfig *)arg;

    CVI_VC_IF("\n");

    CVI_VC_TRACE("[Before]cviVEncSetVbBuffer type[%d][%d] [%d]\n",
                 pcviVbVencCfg->VbType,
                 pcviVbVencCfg->VbSetFrameBufCnt,
                 pcviVbVencCfg->VbSetFrameBufSize);

    memcpy(&pTestEnc->encConfig.cviEc.VbBufCfg, pcviVbVencCfg, sizeof(cviVbVencBufConfig));

    CVI_VC_TRACE("[After MW Venc]cviVEncSetVbBuffer again type[%d][%d] [%d]\n",
                 pTestEnc->encConfig.cviEc.VbBufCfg.VbType,
                 pTestEnc->encConfig.cviEc.VbBufCfg.VbSetFrameBufCnt,
                 pTestEnc->encConfig.cviEc.VbBufCfg.VbSetFrameBufSize);

    return ret;
}

static int cviVEncRegReconBuf(stTestEncoder *pTestEnc, void *arg)
{
    int ret      = 0;
    int core_idx = pTestEnc->encConfig.stdMode == STD_AVC ? CORE_H264 : CORE_H265;

    UNREFERENCED_PARAMETER(arg);

    CVI_VC_IF("\n");

    EnterVcodecLock(core_idx);

    ret = _cviVEncRegFrameBuffer(pTestEnc, arg);

    LeaveVcodecLock(core_idx);

    return ret;
}

static int _cviVEncRegFrameBuffer(stTestEncoder *pTestEnc, void *arg)
{
    int                   ret = 0;
    int                   coreIdx;
    int                   FrameBufSize;
    int                   framebufStride = 0;
    FrameBufferAllocInfo *pFbAllocInfo   = &pTestEnc->fbAllocInfo;
    cviEncCfg            *pcviEc         = &pTestEnc->encConfig.cviEc;
    EncOpenParam         *pEncOP         = &pTestEnc->encOP;
    int                   i, mapType;
    CodecInst            *pCodecInst = pTestEnc->handle;

    UNREFERENCED_PARAMETER(arg);

    coreIdx = pTestEnc->encConfig.coreIdx;

    /* Allocate framebuffers */
    framebufStride =
        CalcStride(pTestEnc->framebufWidth, pTestEnc->framebufHeight,
                   pTestEnc->encOP.srcFormat,
                   pTestEnc->encOP.cbcrInterleave,
                   (TiledMapType)(pTestEnc->encConfig.mapType & 0x0f),
                   FALSE, FALSE);
    FrameBufSize = VPU_GetFrameBufSize(
        pTestEnc->encConfig.coreIdx,
        framebufStride,
        pTestEnc->framebufHeight,
        (TiledMapType)(pTestEnc->encConfig.mapType & 0x0f),
        pTestEnc->encOP.srcFormat,
        pTestEnc->encOP.cbcrInterleave,
        NULL);
    pTestEnc->regFrameBufCount = pTestEnc->initialInfo.minFrameBufferCount;

    CVI_VC_TRACE("framebufWidth = %d, framebufHeight = %d, framebufStride = %d\n",
                 pTestEnc->framebufWidth, pTestEnc->framebufHeight, framebufStride);
    CVI_VC_MEM("FrameBufSize = 0x%X, regFrameBufCount = %d\n", FrameBufSize, pTestEnc->regFrameBufCount);

    pCodecInst->CodecInfo->encInfo.singleLumaBuf = pcviEc->singleLumaBuf;

    if (pcviEc->singleLumaBuf > 0)
    {
        int ber, aft;
        int luma_size, chr_size;
        int productId;

        productId = ProductVpuGetId(coreIdx);
        luma_size = CalcLumaSize(
            productId, framebufStride, pTestEnc->framebufHeight,
            pTestEnc->encOP.cbcrInterleave,
            (TiledMapType)(pTestEnc->encConfig.mapType & 0x0f),
            NULL);

        chr_size = CalcChromaSize(
            productId, framebufStride, pTestEnc->framebufHeight,
            pTestEnc->encOP.srcFormat,
            pTestEnc->encOP.cbcrInterleave,
            (TiledMapType)(pTestEnc->encConfig.mapType & 0x0f),
            NULL);

        i = 0;
        // smart-p case
        if (pcviEc->virtualIPeriod > 0 && pTestEnc->regFrameBufCount > 2)
        {
            if (pcviEc->VbBufCfg.VbType == VB_SOURCE_TYPE_COMMON)
            {
                for (i = 0; i < (pTestEnc->regFrameBufCount - 2); i++)
                {
                    pTestEnc->vbReconFrameBuf[i].size = FrameBufSize;
                    if (VDI_ALLOCATE_MEMORY(coreIdx, &pTestEnc->vbReconFrameBuf[i], 0, NULL) < 0)
                    {
                        CVI_VC_ERR("fail to allocate recon buffer\n");
                        return TE_ERR_ENC_OPEN;
                    }

                    pTestEnc->fbRecon[i].bufY =
                        pTestEnc->vbReconFrameBuf[i].phys_addr;
                    pTestEnc->fbRecon[i].bufCb        = (PhysicalAddress)-1;
                    pTestEnc->fbRecon[i].bufCr        = (PhysicalAddress)-1;
                    pTestEnc->fbRecon[i].size         = FrameBufSize;
                    pTestEnc->fbRecon[i].updateFbInfo = TRUE;
                }
            }
            else
            {
                for (i = 0; i < (pTestEnc->regFrameBufCount - 2); i++)
                {
                    int FrameBufSizeSet = pcviEc->VbBufCfg.VbSetFrameBufSize;
                    if (FrameBufSize > FrameBufSizeSet)
                    {
                        CVI_VC_ERR("FrameBufSize > FrameBufSizeSet\n");
                        return TE_ERR_ENC_OPEN;
                    }

                    pTestEnc->vbReconFrameBuf[i].size = FrameBufSizeSet;
                    pTestEnc->fbRecon[i].bufY         = pcviEc->VbBufCfg.vbModeAddr[i];
                    pTestEnc->fbRecon[i].size         = FrameBufSizeSet;
                }
            }
        }

        aft = i;
        ber = aft + 1;

        if (pcviEc->VbBufCfg.VbType != VB_SOURCE_TYPE_COMMON)
        {
            int FrameBufSizeSet = pcviEc->VbBufCfg.VbSetFrameBufSize;

            CVI_VC_TRACE(" LumbBuf FrameBufSize[%d] set in[%d]\n", FrameBufSize, FrameBufSizeSet);
            if (FrameBufSize > FrameBufSizeSet)
            {
                CVI_VC_ERR("FrameBufSize > FrameBufSizeSet\n");
                return TE_ERR_ENC_OPEN;
            }

            pTestEnc->vbReconFrameBuf[ber].size = FrameBufSizeSet + chr_size * 2;
            pTestEnc->fbRecon[ber].bufY         = pcviEc->VbBufCfg.vbModeAddr[ber];
            pTestEnc->fbRecon[ber].size         = FrameBufSizeSet;
        }
        else
        {
            pTestEnc->vbReconFrameBuf[ber].size = FrameBufSize + chr_size * 2;
            //pTestEnc->vbReconFrameBuf[1].size = FrameBufSize * 2;
            if (VDI_ALLOCATE_MEMORY(coreIdx, &pTestEnc->vbReconFrameBuf[ber], 0, NULL) < 0)
            {
                CVI_VC_ERR("fail to allocate recon buffer\n");
                return TE_ERR_ENC_OPEN;
            }

            pTestEnc->fbRecon[ber].bufY = pTestEnc->vbReconFrameBuf[ber].phys_addr;
            pTestEnc->fbRecon[ber].size = FrameBufSize;
        }
        pTestEnc->fbRecon[aft].bufY  = pTestEnc->fbRecon[ber].bufY;
        pTestEnc->fbRecon[ber].bufCb = pTestEnc->fbRecon[aft].bufY + luma_size;
        pTestEnc->fbRecon[ber].bufCr = pTestEnc->fbRecon[ber].bufCb + chr_size;
        pTestEnc->fbRecon[aft].bufCb = pTestEnc->fbRecon[ber].bufCr + chr_size;
        pTestEnc->fbRecon[aft].bufCr = pTestEnc->fbRecon[aft].bufCb + chr_size;

        pTestEnc->fbRecon[ber].updateFbInfo = TRUE;
        memcpy(&pTestEnc->vbReconFrameBuf[aft], &pTestEnc->vbReconFrameBuf[ber], sizeof(vpu_buffer_t));
        pTestEnc->fbRecon[aft].size         = pTestEnc->fbRecon[ber].size;
        pTestEnc->fbRecon[aft].updateFbInfo = TRUE;
    }
    else
    {
        if (pcviEc->VbBufCfg.VbType == VB_SOURCE_TYPE_COMMON)
        {
            if (pEncOP->addrRemapEn)
            {
                ret = cviInitAddrRemap(pTestEnc);
                if (ret != 0)
                {
                    CVI_VC_ERR("cviInitAddrRemap\n");
                    return ret;
                }
            }
            else
            {
                for (i = 0; i < pTestEnc->regFrameBufCount; i++)
                {
                    pTestEnc->vbReconFrameBuf[i].size = FrameBufSize;
                    if (VDI_ALLOCATE_MEMORY(coreIdx, &pTestEnc->vbReconFrameBuf[i], 0, NULL) < 0)
                    {
                        CVI_VC_ERR("fail to allocate recon buffer\n");
                        return TE_ERR_ENC_OPEN;
                    }

                    pTestEnc->fbRecon[i].bufY         = pTestEnc->vbReconFrameBuf[i].phys_addr;
                    pTestEnc->fbRecon[i].bufCb        = (PhysicalAddress)-1;
                    pTestEnc->fbRecon[i].bufCr        = (PhysicalAddress)-1;
                    pTestEnc->fbRecon[i].size         = FrameBufSize;
                    pTestEnc->fbRecon[i].updateFbInfo = TRUE;
                }
            }
        }
        else
        {
            CVI_VC_TRACE("pcviEc->VbBufCfg.VbType[%d]\n", pcviEc->VbBufCfg.VbType);
            CVI_VC_TRACE("bufcnt compare[%d][%d]\n", pTestEnc->regFrameBufCount, pcviEc->VbBufCfg.VbSetFrameBufCnt);
            CVI_VC_TRACE("bufsize compare[%d][%d]\n", FrameBufSize, pcviEc->VbBufCfg.VbSetFrameBufSize);
            for (i = 0; i < pcviEc->VbBufCfg.VbSetFrameBufCnt; i++)
            {
                pTestEnc->vbReconFrameBuf[i].size = pcviEc->VbBufCfg.VbSetFrameBufSize;
                pTestEnc->fbRecon[i].bufY         = pcviEc->VbBufCfg.vbModeAddr[i];
                pTestEnc->fbRecon[i].bufCb        = (PhysicalAddress)-1;
                pTestEnc->fbRecon[i].bufCr        = (PhysicalAddress)-1;
                pTestEnc->fbRecon[i].size         = pcviEc->VbBufCfg.VbSetFrameBufSize;
                pTestEnc->fbRecon[i].updateFbInfo = TRUE;
            }
        }
    }

    mapType = (TiledMapType)(pTestEnc->encConfig.mapType & 0x0f);

    ret = VPU_EncRegisterFrameBuffer(pTestEnc->handle, pTestEnc->fbRecon,
                                     pTestEnc->regFrameBufCount,
                                     framebufStride,
                                     pTestEnc->framebufHeight, mapType);

    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR(
            "VPU_EncRegisterFrameBuffer failed Error code is 0x%x\n", ret);
        return TE_ERR_ENC_OPEN;
    }

    for (i = 0; i < pTestEnc->regFrameBufCount; i++)
    {
        CVI_VC_INFO(
            "Rec, bufY = 0x" VCODEC_UINT64_FORMAT_HEX ", bufCb = 0x" VCODEC_UINT64_FORMAT_HEX ", bufCr = 0x" VCODEC_UINT64_FORMAT_HEX "\n",
            pTestEnc->fbRecon[i].bufY, pTestEnc->fbRecon[i].bufCb,
            pTestEnc->fbRecon[i].bufCr);
    }

    /*****************************
	* ALLOCATE SROUCE BUFFERS	*
	****************************/
    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        if (pEncOP->linear2TiledEnable == TRUE)
        {
            pFbAllocInfo->mapType = LINEAR_FRAME_MAP;
            pFbAllocInfo->stride  = pEncOP->picWidth;
        }
        else
        {
            pFbAllocInfo->mapType = mapType;
            pFbAllocInfo->stride =
                CalcStride(pEncOP->picWidth, pEncOP->picHeight,
                           pEncOP->srcFormat,
                           pEncOP->cbcrInterleave, mapType,
                           FALSE, FALSE);
        }

        pFbAllocInfo->height = VPU_ALIGN16(pTestEnc->srcFrameHeight);
        pFbAllocInfo->num    = ENC_SRC_BUF_NUM;

        FrameBufSize = VPU_GetFrameBufSize(
            coreIdx, pFbAllocInfo->stride, pFbAllocInfo->height,
            pFbAllocInfo->mapType, pEncOP->srcFormat,
            pEncOP->cbcrInterleave, NULL);
    }
    else
    {
        pFbAllocInfo->mapType = LINEAR_FRAME_MAP;

        pTestEnc->srcFrameStride = CalcStride(
            pTestEnc->srcFrameWidth, pTestEnc->srcFrameHeight,
            (FrameBufferFormat)pEncOP->srcFormat,
            pTestEnc->encOP.cbcrInterleave,
            (TiledMapType)pFbAllocInfo->mapType, FALSE, FALSE);

        FrameBufSize = VPU_GetFrameBufSize(
            coreIdx, pTestEnc->srcFrameStride,
            pTestEnc->srcFrameHeight,
            (TiledMapType)pFbAllocInfo->mapType, pEncOP->srcFormat,
            pTestEnc->encOP.cbcrInterleave, NULL);

        pFbAllocInfo->stride = pTestEnc->srcFrameStride;
        pFbAllocInfo->height = pTestEnc->srcFrameHeight;
        pFbAllocInfo->num    = pTestEnc->initialInfo.minSrcFrameCount + EXTRA_SRC_BUFFER_NUM;
        pFbAllocInfo->num    = (pFbAllocInfo->num > ENC_SRC_BUF_NUM) ? ENC_SRC_BUF_NUM : pFbAllocInfo->num;
    }

    CVI_VC_TRACE("minSrcFrameCount = %d, num = %d, stride = %d\n",
                 pTestEnc->initialInfo.minSrcFrameCount, pFbAllocInfo->num, pFbAllocInfo->stride);

    pFbAllocInfo->format         = (FrameBufferFormat)pEncOP->srcFormat;
    pFbAllocInfo->cbcrInterleave = pTestEnc->encOP.cbcrInterleave;
    pFbAllocInfo->endian         = pTestEnc->encOP.sourceEndian;
    pFbAllocInfo->type           = FB_TYPE_PPU;
    pFbAllocInfo->nv21           = pTestEnc->encOP.nv21;

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        for (i = 0; i < pFbAllocInfo->num; i++)
        {
            pTestEnc->fbSrc[i].size         = FrameBufSize;
            pTestEnc->fbSrc[i].updateFbInfo = TRUE;
        }
    }

    ret = VPU_EncAllocateFrameBuffer(pTestEnc->handle, *pFbAllocInfo,
                                     pTestEnc->fbSrc);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR(
            "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x\n", ret);
        return TE_ERR_ENC_OPEN;
    }

    for (i = 0; i < pFbAllocInfo->num; i++)
    {
        CVI_VC_TRACE(
            "fbSrc[%d], bufY = 0x" VCODEC_UINT64_FORMAT_HEX ", bufCb = 0x" VCODEC_UINT64_FORMAT_HEX ", bufCr = 0x" VCODEC_UINT64_FORMAT_HEX "\n",
            i, pTestEnc->fbSrc[i].bufY, pTestEnc->fbSrc[i].bufCb,
            pTestEnc->fbSrc[i].bufCr);
    }

    if (pEncOP->bitstreamFormat == STD_HEVC)
    {
        if (allocateRoiMapBuf(coreIdx, &pTestEnc->encConfig,
                              &pTestEnc->vbRoi[0], pFbAllocInfo->num,
                              MAX_CTU_NUM)
            == FALSE)
        {
            CVI_VC_ERR("allocateRoiMapBuf fail\n");
            return TE_ERR_ENC_OPEN;
        }

        if (allocateCtuQpMapBuf(coreIdx, &pTestEnc->encConfig,
                                &pTestEnc->vbCtuQp[0],
                                pFbAllocInfo->num,
                                MAX_CTU_NUM)
            == FALSE)
        {
            CVI_VC_ERR("allocateCtuQpMapBuf fail\n");
            return TE_ERR_ENC_OPEN;
        }
    }

    // allocate User data buffer amount of source buffer num
    if (pTestEnc->encConfig.seiDataEnc.prefixSeiNalEnable)
    {
        for (i = 0; i < pFbAllocInfo->num; i++)
        { // the number of roi
            // buffer should be
            // the same as source
            // buffer num.
            pTestEnc->vbPrefixSeiNal[i].size =
                SEI_NAL_DATA_BUF_SIZE;
            if (VDI_ALLOCATE_MEMORY(coreIdx, &pTestEnc->vbPrefixSeiNal[i], 0, NULL) < 0)
            {
                CVI_VC_ERR("fail to allocate ROI buffer\n");
                return TE_ERR_ENC_OPEN;
            }
        }
    }

    if (pTestEnc->encConfig.seiDataEnc.suffixSeiNalEnable)
    {
        for (i = 0; i < pFbAllocInfo->num; i++)
        { // the number of roi
            // buffer should be
            // the same as source
            // buffer num.
            pTestEnc->vbSuffixSeiNal[i].size =
                SEI_NAL_DATA_BUF_SIZE;
            if (VDI_ALLOCATE_MEMORY(coreIdx, &pTestEnc->vbSuffixSeiNal[i], 0, NULL) < 0)
            {
                CVI_VC_ERR("fail to allocate ROI buffer\n");
                return TE_ERR_ENC_OPEN;
            }
        }
    }

#ifdef TEST_ENCODE_CUSTOM_HEADER
    if (allocateSeiNalDataBuf(coreIdx, &pTestEnc->vbSeiNal[0],
                              pFbAllocInfo->num)
        == FALSE)
    {
        CVI_VC_ERR("\n");
        return TE_ERR_ENC_OPEN;
    }
#endif

    return ret;
}

static int cviVEncSetInPixelFormat(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;

    TestEncConfig    *pEncCfg   = &pTestEnc->encConfig;
    EncOpenParam     *pEncOP    = &pTestEnc->encOP;
    cviInPixelFormat *pInFormat = (cviInPixelFormat *)arg;

    CVI_VC_IF("\n");

    pEncOP->cbcrInterleave = pInFormat->bCbCrInterleave;
    pEncOP->nv21           = pInFormat->bNV21;

    if (pEncOP->bitstreamFormat == STD_AVC)
    {
        if (pEncOP->cbcrInterleave == TRUE)
        {
            pEncCfg->mapType                  = TILED_FRAME_MB_RASTER_MAP;
            pEncCfg->coda9.enableLinear2Tiled = TRUE;
            pEncCfg->coda9.linear2TiledMode   = FF_FRAME;
        }
    }

    return ret;
}

static int cviVEncSetFrameParam(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;

    cviFrameParam *pfp       = (cviFrameParam *)arg;
    EncParam      *pEncParam = &pTestEnc->encParam;

    pEncParam->u32FrameQp   = pfp->u32FrameQp;
    pEncParam->u32FrameBits = pfp->u32FrameBits;
    CVI_VC_UBR("u32FrameQp = %d, u32FrameBits = %d\n", pEncParam->u32FrameQp, pEncParam->u32FrameBits);

    return ret;
}

static int cviVEncCalcFrameParam(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    /*
	This API is only used for testing UBR. It will use rcLib to
	do CBR and get Qp & target bits.
	Then these parameters will be sent to encoder by using UBR API.
	*/

    cviFrameParam *pfp        = (cviFrameParam *)arg;
    CodecInst     *pCodecInst = (CodecInst *)pTestEnc->handle;
    EncInfo       *pEncInfo   = &pCodecInst->CodecInfo->encInfo;
    EncOpenParam  *pOpenParam = &pEncInfo->openParam;
    int            s32RowMaxDqpMinus, s32RowMaxDqpPlus;
    int            s32HrdBufLevel, s32HrdBufSize;

    pCodecInst->rcInfo.bTestUbrEn = 1;

    if (pOpenParam->bitstreamFormat == STD_HEVC && pTestEnc->frameIdx == 0)
    {
        rcLibSetupRc(pCodecInst);
    }

    ret = rcLibCalcPicQp(pCodecInst,
#ifdef CLIP_PIC_DELTA_QP
                         &s32RowMaxDqpMinus, &s32RowMaxDqpPlus,
#endif
                         (int *)&pfp->u32FrameQp, (int *)&pfp->u32FrameBits,
                         &s32HrdBufLevel, &s32HrdBufSize);
    if (ret != RETCODE_SUCCESS)
    {
        CVI_VC_ERR("ret = %d\n", ret);
        return ret;
    }
    pTestEnc->encParam.s32HrdBufLevel = s32HrdBufLevel;

    CVI_VC_UBR("fidx = %d, Qp = %d, Bits = %d\n", pTestEnc->frameIdx, pfp->u32FrameQp, pfp->u32FrameBits);

    return ret;
}

static int cviVEncSetSbMode(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    // unsigned long coreIdx = 0;
    unsigned int      srcHeightAlign = 0;
    int               reg            = 0;
    cviVencSbSetting *psbSetting     = (cviVencSbSetting *)arg;

    CVI_VC_IF("\n");
    UNREFERENCED_PARAMETER(pTestEnc);

    // if (psbSetting->codec & 0x1)
    // 	coreIdx = 0;
    // else
    // 	coreIdx = 1;

    // Set Register 0x0
    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);

    // sb_nb:3, sbc1_mode:2, //sbc1_frm_start:1
    if (psbSetting->codec & 0x1)
        reg |= (psbSetting->sb_mode << 4);

    if (psbSetting->codec & 0x2)
        reg |= (psbSetting->sb_mode << 8);

    if (psbSetting->codec & 0x4)
        reg |= (psbSetting->sb_mode << 12);

    reg |= (psbSetting->sb_size << 17);
    reg |= (psbSetting->sb_nb << 24);

    if ((psbSetting->sb_ybase1 != 0) && (psbSetting->sb_uvbase1 != 0))
        reg |= (1 << 19); // 1: sbc0/sbc1 sync with interface0 and  sbc2  sync with  interface1

    cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);

    CVI_VC_REG("VC_REG_BANK_SBM 0x00 = 0x%x\n", reg);

    if ((psbSetting->sb_ybase != 0) && (psbSetting->sb_uvbase != 0))
    {
        // pri address setting /////////////////////////////////////////////
        // Set Register 0x20
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x20);
        reg = ((psbSetting->src_height + 63) >> 6) << 22;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x20, reg);

        CVI_VC_REG("VC_REG_BANK_SBM 0x20 = 0x%x\n", reg);

        // Set Register 0x24
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x24);
        reg = ((psbSetting->y_stride >> 4) << 4) + ((psbSetting->uv_stride >> 4) << 20);

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x24, reg);

        CVI_VC_REG("VC_REG_BANK_SBM 0x24 = 0x%x\n", reg);

        // Set Register 0x28   src y base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x28);
        reg = 0x80000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x28, reg);
        psbSetting->src_ybase = reg;

        CVI_VC_INFO("VC_REG_BANK_SBM 0x28 = 0x%x\n", reg);

        // Set Register 0x2C   src y end addr
        srcHeightAlign = ((psbSetting->src_height + 15) >> 4) << 4;
        reg            = 0x80000000 + srcHeightAlign * psbSetting->y_stride;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x2C, reg);
        CVI_VC_REG("VC_REG_BANK_SBM 0x2C = 0x%x\n", reg);

        // Set Register 0x30   sb y base addr
        reg = psbSetting->sb_ybase;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x30, reg);

        CVI_VC_INFO("VC_REG_BANK_SBM 0x30 = 0x%x\n", reg);

        // Set Register 0x34   src c base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x34);
        reg = 0x88000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x34, reg);
        psbSetting->src_uvbase = reg;

        CVI_VC_REG("VC_REG_BANK_SBM 0x34 = 0x%x\n", reg);

        // Set Register 0x38   src c end addr
        reg = 0x88000000 + srcHeightAlign * psbSetting->y_stride / 2;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x38, reg);

        CVI_VC_REG("VC_REG_BANK_SBM 0x38 = 0x%x\n", reg);

        // Set Register 0x3C   sb c base addr
        reg = psbSetting->sb_uvbase;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x3C, reg);

        CVI_VC_REG("VC_REG_BANK_SBM 0x3C = 0x%x\n", reg);
    }

    if ((psbSetting->sb_ybase1 != 0) && (psbSetting->sb_uvbase1 != 0))
    {
        // sec address setting /////////////////////////////////////////////
        // Set Register 0x40
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x40);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x40, reg);

        reg = ((psbSetting->src_height + 63) >> 6) << 22;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x40, reg);

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x40);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x40, reg);

        // Set Register 0x44
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x44);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x44, reg);

        reg = ((psbSetting->y_stride >> 4) << 4) + ((psbSetting->uv_stride >> 4) << 20);

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x44, reg);

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x44);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x44, reg);

        // Set Register 0x48   src y base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x48);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x48, reg);

        reg = 0x90000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x48, reg);
        psbSetting->src_ybase1 = reg;

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x48);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x48, reg);

        // Set Register 0x4C   src y end addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x4C);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x4C, reg);

        srcHeightAlign = ((psbSetting->src_height + 15) >> 4) << 4;
        CVI_VC_REG("[%s][%d] src_height=%d, srcHeightAlign=%d\n",
                   __func__, __LINE__, psbSetting->src_height, srcHeightAlign);
        reg = 0x90000000 + srcHeightAlign * psbSetting->y_stride;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x4C, reg);

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x4C);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x4C, reg);

        // Set Register 0x50   sb y base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x50);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x50, reg);

        reg = psbSetting->sb_ybase1;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x50, reg);

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x50);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x50, reg);

        // Set Register 0x54   src c base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x54);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x54, reg);

        reg = 0x98000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x54, reg);
        psbSetting->src_uvbase1 = reg;

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x54);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x54, reg);

        // Set Register 0x58   src c end addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x58);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x58, reg);

        reg = 0x98000000 + srcHeightAlign * psbSetting->y_stride / 2;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x58, reg);

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x58);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x58, reg);

        // Set Register 0x5C   sb c base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x5C);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x5C, reg);

        reg = psbSetting->sb_uvbase1;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x5C, reg);

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x5C);
        CVI_VC_REG("[%s][%d] SBM Reg 0x%x = 0x%x\n", __func__, __LINE__, 0x5C, reg);
    }

    return ret;
}

static int cviVEncStartSbMode(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    // unsigned long coreIdx = 0;
    cviVencSbSetting *psbSetting = (cviVencSbSetting *)arg;
    int               reg        = 0;

    UNUSED(pTestEnc);
    UNUSED(arg);

    // if (psbSetting->codec & 0x1)
    // 	coreIdx = 0;
    // else
    // 	coreIdx = 1;

    // Set Register 0x0
    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);

    // frm_start:1
    if (psbSetting->codec & 0x1)
        reg |= (1 << 7);

    if (psbSetting->codec & 0x2)
        reg |= (1 << 11);

    if (psbSetting->codec & 0x4)
        reg |= (1 << 15);

    cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);

    CVI_VC_REG("VC_REG_BANK_SBM 0x00 = 0x%x\n", reg);

    return ret;
}

static int cviVEncUpdateSbWptr(stTestEncoder *pTestEnc, void *arg)
{
    int               ret        = 0;
    unsigned long     coreIdx    = 0;
    int               sw_mode    = 0;
    cviVencSbSetting *psbSetting = (cviVencSbSetting *)arg;
    int               reg        = 0;

    UNUSED(pTestEnc);

    if (psbSetting->codec & 0x1)
        coreIdx = 0;
    else
        coreIdx = 1;

    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);

    if (coreIdx == 0)
        sw_mode = (reg & 0x30) >> 4;
    else
        sw_mode = (reg & 0x300) >> 8;

    if (sw_mode == 3)
    { // SW mode
        // Set Register 0x0c
        int wptr = 0;

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x88);
        CVI_VC_INFO("VC_REG_BANK_SBM 0x88 = 0x%x\n", reg);

        wptr = (reg >> 16) & 0x1F;

        CVI_VC_INFO("wptr = 0x%x\n", reg);

        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x0C);
        reg = (reg & 0xFFFFFFE0) | wptr;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x0C, reg);

        CVI_VC_INFO("VC_REG_BANK_SBM 0x0C = 0x%x\n", reg);
    }

    return ret;
}

static int cviVEncResetSb(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    // unsigned long coreIdx = 0;
    cviVencSbSetting *psbSetting = (cviVencSbSetting *)arg;
    int               reg        = 0;
    UNUSED(pTestEnc);

    CVI_VC_FLOW("\n");

    // if (psbSetting->codec & 0x1)
    // 	coreIdx = 0;
    // else
    // 	coreIdx = 1;

    CVI_VC_INFO("Before sw reset sb =================\n");
    CVI_VC_INFO("VC_REG_BANK_SBM 0x80 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x80));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x84 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x84));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x88 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x88));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x90 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x90));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x94));

    // Reset VC SB ctrl
    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);
#if 0
	reg |= 0x8;  // reset all
	cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);
#else
    if (psbSetting->codec & 0x1)
    { // h265
        reg |= 0x1;
        cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);
    }
    else if (psbSetting->codec & 0x2)
    { // h264
        reg |= 0x2;
        cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);
    }
    else
    { // jpeg
        reg |= 0x4;
        cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);
    }
#endif

    CVI_VC_INFO("After sw reset sb =================\n");
    CVI_VC_INFO("VC_REG_BANK_SBM 0x80 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x80));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x84 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x84));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x88 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x88));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x90 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x90));
    CVI_VC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x94));

    return ret;
}

static int cviVEncSbEnDummyPush(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    // unsigned long coreIdx = 0;
    // cviVencSbSetting *psbSetting = (cviVencSbSetting *)arg;
    int reg = 0;
    UNUSED(pTestEnc);

    CVI_VC_FLOW("\n");

    // if (psbSetting->codec & 0x1)
    // 	coreIdx = 0;
    // else
    // 	coreIdx = 1;

    // Enable sb dummy push
    reg  = cvi_vc_drv_read_vc_reg(REG_SBM, 0x14);
    reg |= 0x1; // reg_pri_push_ow_en bit0
    cvi_vc_drv_write_vc_reg(REG_SBM, 0x14, reg);

    CVI_VC_INFO("VC_REG_BANK_SBM 0x14 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x14));

    return ret;
}

static int cviVEncSbTrigDummyPush(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    // unsigned long coreIdx = 0;
    // cviVencSbSetting *psbSetting = (cviVencSbSetting *)arg;
    int reg          = 0;
    int pop_cnt_pri  = 0;
    int push_cnt_pri = 0;
    UNUSED(pTestEnc);

    CVI_VC_FLOW("\n");

    // if (psbSetting->codec & 0x1)
    // 	coreIdx = 0;
    // else
    // 	coreIdx = 1;

    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x94);
    CVI_VC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n", reg);

    push_cnt_pri = reg & 0x3F;
    pop_cnt_pri  = (reg >> 16) & 0x3F;

    CVI_VC_INFO("push_cnt_pri=%d, pop_cnt_pri=%d\n", push_cnt_pri, pop_cnt_pri);

    if (push_cnt_pri == pop_cnt_pri)
    {
        reg  = cvi_vc_drv_read_vc_reg(REG_SBM, 0x14);
        reg |= 0x4; // reg_pri_push_ow bit2
        cvi_vc_drv_write_vc_reg(REG_SBM, 0x14, reg);

        CVI_VC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x94));
    }

    return ret;
}

static int cviVEncSbDisDummyPush(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    // unsigned long coreIdx = 0;
    // cviVencSbSetting *psbSetting = (cviVencSbSetting *)arg;
    int reg = 0;

    UNUSED(pTestEnc);

    CVI_VC_FLOW("\n");

    // if (psbSetting->codec & 0x1)
    // 	coreIdx = 0;
    // else
    // 	coreIdx = 1;

    reg  = cvi_vc_drv_read_vc_reg(REG_SBM, 0x14);
    reg &= (~0x1); // reg_pri_push_ow_en bit0
    cvi_vc_drv_write_vc_reg(REG_SBM, 0x14, reg);

    CVI_VC_INFO("VC_REG_BANK_SBM 0x14 = 0x%x\n", cvi_vc_drv_read_vc_reg(REG_SBM, 0x14));

    return ret;
}

static int cviVEncSbGetSkipFrmStatus(stTestEncoder *pTestEnc, void *arg)
{
    int ret = 0;
    // unsigned long coreIdx = 0;
    cviVencSbSetting *psbSetting       = (cviVencSbSetting *)arg;
    int               reg              = 0;
    int               pop_cnt_pri      = 0;
    int               push_cnt_pri     = 0;
    int               target_slice_cnt = 0;
    UNUSED(pTestEnc);

    CVI_VC_FLOW("\n");

    // if (psbSetting->codec & 0x1)
    // 	coreIdx = 0;
    // else
    // 	coreIdx = 1;

    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x94);
    CVI_VC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n", reg);

    push_cnt_pri = reg & 0x3F;
    pop_cnt_pri  = (reg >> 16) & 0x3F;

    CVI_VC_INFO("push_cnt_pri=%d, pop_cnt_pri=%d\n", push_cnt_pri, pop_cnt_pri);

    if (psbSetting->sb_size == 0)
        target_slice_cnt = (psbSetting->src_height + 63) / 64;
    else
        target_slice_cnt = (psbSetting->src_height + 127) / 128;

    CVI_VC_INFO("psbSetting->src_height=%d, psbSetting->sb_size=%d, target_slice_cnt=%d\n",
                psbSetting->src_height, psbSetting->sb_size, target_slice_cnt);

    if (pop_cnt_pri == target_slice_cnt)
    {
        psbSetting->status = 1;

        CVI_VC_INFO("psbSetting->src_height=%d, psbSetting->sb_size=%d, target_slice_cnt=%d\n",
                    psbSetting->src_height, psbSetting->sb_size, target_slice_cnt);
    }

    return ret;
}

static int cviVEncSetSbmEnable(stTestEncoder *pTestEnc, void *arg)
{
    CodecInst *pCodecInst = pTestEnc->handle;

    pCodecInst->CodecInfo->encInfo.bSbmEn = *(BOOL *)arg;

    if (pCodecInst->CodecInfo->encInfo.bSbmEn)
    {
        int chnIdx;
        MUTEX_LOCK(sSwitchCoreIdxSem);
        tVencSbmInfo.rrChnIdx     = 0;
        tVencSbmInfo.targetChnNum = pCodecInst->s32ChnNum;
        tVencSbmInfo.sbmChnNum    = pCodecInst->s32ChnNum;
        tVencSbmInfo.bSBModeEn    = TRUE;
        if (tVencSbmInfo.numEncChn > 1)
        {
            /*  only one H26x chn in SBM mode
			 *  if multiple H26x channel (ex. N channels)
			 *  means 1 SBM channel with N-1 FRM channels
			 */
            tVencSbmInfo.bFRModeEn = TRUE;
        }
        MUTEX_UNLOCK(sSwitchCoreIdxSem);

        for (chnIdx = 1; chnIdx < tVencSbmInfo.numEncChn; chnIdx++)
        {
            // re-order sbm channel number into 1st element in array
            if (tVencSbmInfo.frmChnNum[chnIdx] == tVencSbmInfo.sbmChnNum)
            {
                int tmpChnNum = tVencSbmInfo.frmChnNum[0];

                tVencSbmInfo.frmChnNum[0]      = tVencSbmInfo.sbmChnNum;
                tVencSbmInfo.frmChnNum[chnIdx] = tmpChnNum;
                break;
            }
        }

        CVI_VC_INFO("sbmChnNum %d\n", tVencSbmInfo.sbmChnNum);
    }

    return 0;
}

static int cviVEncEncodeUserData(stTestEncoder *pTestEnc, void *arg)
{
    int            i       = 0;
    TestEncConfig *pEncCfg = &pTestEnc->encConfig;
    cviUserData   *pSrc    = (cviUserData *)arg;

    CVI_VC_IF("\n");

    if (pSrc == NULL || pSrc->userData == NULL || pSrc->len == 0)
    {
        CVI_VC_ERR("no user data\n");
        return TE_ERR_ENC_USER_DATA;
    }

    for (i = 0; i < NUM_OF_USER_DATA_BUF; ++i)
    {
        if (pEncCfg->userDataBuf[i] != NULL && pEncCfg->userDataLen[i] == 0)
        {
            // seiEncode has buffer wrap-around handling.
            // It does not write outside of the specified buffer range even if
            // the encoded es length is greater than the buffer size.
            unsigned int len =
                seiEncode(pEncCfg->stdMode, pSrc->userData,
                          pSrc->len, pEncCfg->userDataBuf[i],
                          pEncCfg->userDataBufSize);

            if (len > pEncCfg->userDataBufSize)
            {
                CVI_VC_ERR("encoded user data len %d exceeds buffer size %d\n", len, pEncCfg->userDataBufSize);
                return TE_ERR_ENC_USER_DATA;
            }

            pEncCfg->userDataLen[i] = len;
            return 0;
        }
    }

    CVI_VC_ERR("no available user data buf\n");
    return TE_ERR_ENC_USER_DATA;
}

static int cviVEncSetH264Entropy(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig  *pEncCfg  = &pTestEnc->encConfig;
    cviH264Entropy *pEntropy = &pEncCfg->cviEc.h264Entropy;
    cviH264Entropy *pSrc     = (cviH264Entropy *)arg;

    CVI_VC_IF("\n");

    if (pSrc == NULL)
    {
        CVI_VC_ERR("no h264 entropy data\n");
        return TE_ERR_ENC_H264_ENTROPY;
    }

    memcpy(pEntropy, pSrc, sizeof(cviH264Entropy));
    return 0;
}

static int cviVEncSetH264Trans(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig *pEncCfg = &pTestEnc->encConfig;
    cviH264Trans  *pTrans  = &pEncCfg->cviEc.h264Trans;
    cviH264Trans  *pSrc    = (cviH264Trans *)arg;

    CVI_VC_IF("\n");

    if (pSrc == NULL)
    {
        CVI_VC_ERR("no h264 trans data\n");
        return TE_ERR_ENC_H264_TRANS;
    }

    memcpy(pTrans, pSrc, sizeof(cviH264Trans));
    return 0;
}

static int cviVEncSetH265Trans(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig *pEncCfg = &pTestEnc->encConfig;
    cviH265Trans  *pTrans  = &pEncCfg->cviEc.h265Trans;
    cviH265Trans  *pSrc    = (cviH265Trans *)arg;

    CVI_VC_IF("\n");

    if (pSrc == NULL)
    {
        CVI_VC_ERR("no h265 trans data\n");
        return TE_ERR_ENC_H265_TRANS;
    }

    memcpy(pTrans, pSrc, sizeof(cviH265Trans));
    return 0;
}

static int cviVEncSetH264Vui(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig *pEncCfg = &pTestEnc->encConfig;
    cviH264Vui    *pVui    = &pEncCfg->cviEc.h264Vui;
    cviH264Vui    *pSrc    = (cviH264Vui *)arg;

    CVI_VC_IF("\n");

    if (pSrc == NULL)
    {
        CVI_VC_ERR("no h264 vui data\n");
        return TE_ERR_ENC_H264_VUI;
    }

    memcpy(pVui, pSrc, sizeof(cviH264Vui));

    if (pVui->aspect_ratio_info.aspect_ratio_info_present_flag ||
	    pVui->aspect_ratio_info.overscan_info_present_flag ||
	    pVui->video_signal_type.video_signal_type_present_flag ||
	    pVui->timing_info.timing_info_present_flag)
    {
        pEncCfg->cviEc.bEnableVUI = CVI_TRUE;
    }

    return 0;
}

static int cviVEncSetH265Vui(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig *pEncCfg = &pTestEnc->encConfig;
    cviH265Vui    *pVui    = &pEncCfg->cviEc.h265Vui;
    cviH265Vui    *pSrc    = (cviH265Vui *)arg;

    CVI_VC_IF("\n");

    if (pSrc == NULL)
    {
        CVI_VC_ERR("no h265 vui data\n");
        return TE_ERR_ENC_H265_VUI;
    }

    memcpy(pVui, pSrc, sizeof(cviH265Vui));

	if (pVui->aspect_ratio_info.aspect_ratio_info_present_flag ||
	    pVui->aspect_ratio_info.overscan_info_present_flag ||
	    pVui->video_signal_type.video_signal_type_present_flag ||
	    pVui->timing_info.timing_info_present_flag)
    {
        pEncCfg->cviEc.bEnableVUI = CVI_TRUE;
    }

    return 0;
}

static int cviVEncSetRc(stTestEncoder *pTestEnc, void *arg)
{
    cviRcParam    *prcp       = (cviRcParam *)arg;
    TestEncConfig *pEncConfig = &pTestEnc->encConfig;
    cviEncCfg     *pCviEc     = &pEncConfig->cviEc;
    int            ret        = 0;

    CVI_VC_IF("\n");

    pCviEc->u32RowQpDelta   = prcp->u32RowQpDelta;
    pCviEc->firstFrmstartQp = prcp->firstFrmstartQp;
    pCviEc->initialDelay    = prcp->s32InitialDelay;
    pCviEc->u32ThrdLv       = prcp->u32ThrdLv;
    CVI_VC_CFG("firstFrmstartQp = %d, initialDelay = %d, u32ThrdLv = %d\n",
               pCviEc->firstFrmstartQp, pCviEc->initialDelay, pCviEc->u32ThrdLv);

    pEncConfig->changePos = prcp->s32ChangePos;
    pCviEc->bBgEnhanceEn  = prcp->bBgEnhanceEn;
    pCviEc->s32BgDeltaQp  = prcp->s32BgDeltaQp;
    CVI_VC_CFG("s32ChangePos = %d, bBgEnhanceEn = %d, s32BgDeltaQp = %d\n",
               prcp->s32ChangePos, pCviEc->bBgEnhanceEn, pCviEc->s32BgDeltaQp);

    pCviEc->u32MaxIprop         = prcp->u32MaxIprop;
    pCviEc->u32MinIprop         = prcp->u32MinIprop;
    pCviEc->s32MaxReEncodeTimes = prcp->s32MaxReEncodeTimes;
    CVI_VC_CFG("u32MaxIprop = %d, u32MinIprop = %d, s32MaxReEncodeTimes = %d\n",
               pCviEc->u32MaxIprop, pCviEc->u32MinIprop, pCviEc->s32MaxReEncodeTimes);

    pCviEc->u32MaxQp  = prcp->u32MaxQp;
    pCviEc->u32MaxIQp = prcp->u32MaxIQp;
    pCviEc->u32MinQp  = prcp->u32MinQp;
    pCviEc->u32MinIQp = prcp->u32MinIQp;
    CVI_VC_CFG("u32MaxQp = %d, u32MaxIQp = %d, u32MinQp = %d, u32MinIQp = %d\n",
               pCviEc->u32MaxQp, pCviEc->u32MaxIQp, pCviEc->u32MinQp, pCviEc->u32MinIQp);

    pCviEc->s32MinStillPercent   = prcp->s32MinStillPercent;
    pCviEc->u32MaxStillQP        = prcp->u32MaxStillQP;
    pCviEc->u32MotionSensitivity = prcp->u32MotionSensitivity;
    CVI_VC_CFG("StillPercent = %d, StillQP = %d, MotionSensitivity = %d\n",
               prcp->s32MinStillPercent, prcp->u32MaxStillQP, prcp->u32MotionSensitivity);

    pCviEc->s32AvbrFrmLostOpen  = prcp->s32AvbrFrmLostOpen;
    pCviEc->s32AvbrFrmGap       = prcp->s32AvbrFrmGap;
    pCviEc->s32AvbrPureStillThr = prcp->s32AvbrPureStillThr;
    CVI_VC_CFG("FrmLostOpen = %d, FrmGap = %d, PureStillThr = %d\n",
               prcp->s32AvbrFrmLostOpen, prcp->s32AvbrFrmGap, prcp->s32AvbrPureStillThr);

    return ret;
}

static int cviVEncSetRef(stTestEncoder *pTestEnc, void *arg)
{
    unsigned int *tempLayer = (unsigned int *)arg;
    int           ret       = 0;

    CVI_VC_IF("\n");
    pTestEnc->encConfig.tempLayer = *tempLayer;

    return ret;
}

static int cviVEncSetPred(stTestEncoder *pTestEnc, void *arg)
{
    cviEncCfg *pCviEc = &pTestEnc->encConfig.cviEc;
    cviPred   *pPred  = (cviPred *)arg;

    CVI_VC_IF("\n");

    pCviEc->u32IntraCost = pPred->u32IntraCost;

    return 0;
}

static int cviVEncSetRoi(stTestEncoder *pTestEnc, void *arg)
{
    cviRoiParam  *proi    = (cviRoiParam *)arg;
    int           ret     = 0;
    int           index   = proi->roi_index;
    cviEncRoiCfg *pcviRoi = NULL;

    if (index >= 8 || index < 0)
    {
        CVI_VC_ERR("Set ROI index = %d\n", index);
        return -1;
    }

    CVI_VC_IF("\n");

    pcviRoi                               = &pTestEnc->encConfig.cviEc.cviRoi[index];
    pTestEnc->encConfig.cviEc.roi_request = TRUE;
    pcviRoi->roi_enable                   = proi->roi_enable;
    pcviRoi->roi_qp_mode                  = proi->roi_qp_mode;
    pcviRoi->roi_qp                       = proi->roi_qp;
    pcviRoi->roi_rect_x                   = proi->roi_rect_x;
    pcviRoi->roi_rect_y                   = proi->roi_rect_y;
    pcviRoi->roi_rect_width               = proi->roi_rect_width;
    pcviRoi->roi_rect_height              = proi->roi_rect_height;
    cviUpdateOneRoi(&pTestEnc->encParam, pcviRoi, index);

    CVI_VC_TRACE("cviVEncSetRoi [%d]enable %d, qp mode %d, qp %d\n", index,
                 proi->roi_enable, proi->roi_qp_mode, proi->roi_qp);
    CVI_VC_TRACE("cviVEncSetRoi [%d]X:%d,Y:%d,W:%d,H:%d\n", index,
                 proi->roi_rect_x, proi->roi_rect_y, proi->roi_rect_width, proi->roi_rect_height);
    return ret;
}

static int cviVEncGetRoi(stTestEncoder *pTestEnc, void *arg)
{
    cviRoiParam        *proi    = (cviRoiParam *)arg;
    int                 ret     = 0;
    int                 index   = proi->roi_index;
    const cviEncRoiCfg *pcviRoi = NULL;

    if (index >= 8 || index < 0)
    {
        CVI_VC_ERR("Get ROI index = %d\n", index);
        return -1;
    }

    CVI_VC_IF("\n");

    pcviRoi = &pTestEnc->encConfig.cviEc.cviRoi[index];

    proi->roi_enable      = pcviRoi->roi_enable;
    proi->roi_qp_mode     = pcviRoi->roi_qp;
    proi->roi_qp          = pcviRoi->roi_qp;
    proi->roi_rect_x      = pcviRoi->roi_rect_x;
    proi->roi_rect_y      = pcviRoi->roi_rect_y;
    proi->roi_rect_width  = pcviRoi->roi_rect_width;
    proi->roi_rect_height = pcviRoi->roi_rect_height;

    return ret;
}

static int cviVEncStart(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig *pEncConfig = malloc(sizeof(TestEncConfig));
    int            ret        = 0;
    int            core_idx   = pTestEnc->encConfig.stdMode == STD_AVC ? CORE_H264 : CORE_H265;

    UNREFERENCED_PARAMETER(arg);

    CVI_VC_IF("\n");

    EnterVcodecLock(core_idx);
    memcpy(pEncConfig, &pTestEnc->encConfig, sizeof(TestEncConfig));

    if (cviInitEncoder(pTestEnc, pEncConfig) == NULL)
    {
        ret = -1;
    }

    LeaveVcodecLock(core_idx);
    free(pEncConfig);
    return ret;
}

static int cviVEncOpGetFd(stTestEncoder *pTestEnc, void *arg)
{
    int *fd      = (int *)arg;
    int  coreIdx = pTestEnc->coreIdx;
    int  ret     = 0;

    CVI_VC_IF("\n");

    *fd = cviVPU_GetFd(coreIdx);

    if (*fd < 0)
    {
        CVI_VC_ERR("get-fd failed\n");
        *fd = -1;
        ret = -1;
    }

    CVI_VC_TRACE("fd = %d\n", *fd);

    return ret;
}

static int cviVEncOpSetChnAttr(stTestEncoder *pTestEnc, void *arg)
{
    cviVidChnAttr *pChnAttr = (cviVidChnAttr *)arg;
    int            ret      = 0;
    unsigned int   u32Sec   = 0;
    unsigned int   u32Frm   = 0;

    CVI_VC_IF("\n");

    pTestEnc->encOP.bitRate = pChnAttr->u32BitRate;

    u32Sec = pChnAttr->fr32DstFrameRate >> 16;
    u32Frm = pChnAttr->fr32DstFrameRate & 0xFFFF;

    if (u32Sec == 0)
    {
        pTestEnc->encOP.frameRateInfo = u32Frm;
    }
    else
    {
        pTestEnc->encOP.frameRateInfo = ((u32Sec - 1) << 16) + u32Frm;
    }

    CVI_VC_IF("\n");

    return ret;
}

static int cviVEncSetFrameLost(stTestEncoder *pTestEnc, void *arg)
{
    cviFrameLost  *pfrLost    = (cviFrameLost *)arg;
    TestEncConfig *pEncConfig = &pTestEnc->encConfig;
    int            ret        = 0;

    CVI_VC_IF("\n");

    pEncConfig->frmLostOpen         = pfrLost->bFrameLostOpen;
    pEncConfig->cviEc.frmLostBpsThr = pfrLost->u32FrmLostBpsThr;
    pEncConfig->cviEc.encFrmGaps    = pfrLost->u32EncFrmGaps;

    CVI_VC_CFG("frmLostOpen = %d\n", pEncConfig->frmLostOpen);
    CVI_VC_CFG("frameSkipBufThr = %d\n", pEncConfig->cviEc.frmLostBpsThr);
    CVI_VC_CFG("encFrmGaps = %d\n", pEncConfig->cviEc.encFrmGaps);

    return ret;
}

#ifdef CLI_DEBUG_SUPPORT

extern void showCodecInstPoolInfo(CodecInst *pCodecInst);
static int  cviVEncShowChnInfo(stTestEncoder *pTestEnc, void *arg)
{
    if (pTestEnc == NULL)
    {
        tcli_print("error params.\n");
        return -1;
    }

    UNUSED(arg);
    EncOpenParam *pEncOP = &pTestEnc->encOP;

    tcli_print("bitstreamBufferSize:%d\n", pEncOP->bitstreamBufferSize);
    tcli_print("bitstreamFormat:%d\n", pEncOP->bitstreamFormat);
    tcli_print("picWidth:%d\n", pEncOP->picWidth);
    tcli_print("picHeight:%d\n", pEncOP->picHeight);
    FrameBufferAllocInfo *pfbAllocInfo = &pTestEnc->fbAllocInfo;

    if (pfbAllocInfo)
    {
        tcli_print("pfbAllocInfo_stride:%d\n", pfbAllocInfo->stride);
        tcli_print("pfbAllocInfo_height:%d\n", pfbAllocInfo->height);
        tcli_print("pfbAllocInfo_size:%d\n", pfbAllocInfo->size);
        tcli_print("pfbAllocInfo_numt:%d\n", pfbAllocInfo->num);
    }

    TestEncConfig *pEncConfig = &pTestEnc->encConfig;

    tcli_print("encConfig info:\n");
    tcli_print("stdMode:%d\n", pEncConfig->stdMode);
    tcli_print("picWidth:%d\n", pEncConfig->picWidth);
    tcli_print("picHeight:%d\n", pEncConfig->picHeight);
    tcli_print("kbps:%d\n", pEncConfig->kbps);
    tcli_print("rcMode:%d\n", pEncConfig->rcMode);
    tcli_print("changePos:%d\n", pEncConfig->changePos);
    tcli_print("frmLostOpen:%d\n", pEncConfig->frmLostOpen);
    tcli_print("rotAngle:%d\n", pEncConfig->rotAngle);
    tcli_print("mirDir:%d\n", pEncConfig->mirDir);
    tcli_print("useRot:%d\n", pEncConfig->useRot);
    tcli_print("qpReport:%d\n", pEncConfig->qpReport);
    tcli_print("ringBufferEnable:%d\n", pEncConfig->ringBufferEnable);
    tcli_print("rcIntraQp:%d\n", pEncConfig->rcIntraQp);
    tcli_print("instNum:%d\n", pEncConfig->instNum);
    tcli_print("coreIdx:%d\n", pEncConfig->coreIdx);

    tcli_print("mapType:%d\n", pEncConfig->mapType);
    tcli_print("lineBufIntEn:%d\n", pEncConfig->lineBufIntEn);
    tcli_print("bEsBufQueueEn:%d\n", pEncConfig->bEsBufQueueEn);
    tcli_print("en_container:%d\n", pEncConfig->en_container);
    tcli_print("container_frame_rate:%d\n", pEncConfig->container_frame_rate);
    tcli_print("picQpY:%d\n", pEncConfig->picQpY);
    tcli_print("cbcrInterleave:%d\n", pEncConfig->cbcrInterleave);
    tcli_print("nv21:%d\n", pEncConfig->nv21);
    tcli_print("needSourceConvert:%d\n", pEncConfig->needSourceConvert);
    tcli_print("packedFormat:%d\n", pEncConfig->packedFormat);
    tcli_print("srcFormat:%d\n", pEncConfig->srcFormat);
    tcli_print("srcFormat3p4b:%d\n", pEncConfig->srcFormat3p4b);
    tcli_print("decodingRefreshType:%d\n", pEncConfig->decodingRefreshType);
    tcli_print("gopSize:%d\n", pEncConfig->gopSize);

    tcli_print("tempLayer:%d\n", pEncConfig->tempLayer);
    tcli_print("useDeriveLambdaWeight:%d\n", pEncConfig->useDeriveLambdaWeight);
    tcli_print("dynamicMergeEnable:%d\n", pEncConfig->dynamicMergeEnable);
    tcli_print("independSliceMode:%d\n", pEncConfig->independSliceMode);
    tcli_print("independSliceModeArg:%d\n", pEncConfig->independSliceModeArg);
    tcli_print("RcEnable:%d\n", pEncConfig->RcEnable);
    tcli_print("bitdepth:%d\n", pEncConfig->bitdepth);
    tcli_print("secondary_axi:%d\n", pEncConfig->secondary_axi);
    tcli_print("stream_endian:%d\n", pEncConfig->stream_endian);
    tcli_print("frame_endian:%d\n", pEncConfig->frame_endian);
    tcli_print("source_endian:%d\n", pEncConfig->source_endian);
    tcli_print("yuv_mode:%d\n", pEncConfig->yuv_mode);
    tcli_print("loopCount:%d\n", pEncConfig->loopCount);

    tcli_print("roi_enable:%d\n", pEncConfig->roi_enable);
    tcli_print("roi_delta_qp:%d\n", pEncConfig->roi_delta_qp);
    tcli_print("ctu_qp_enable:%d\n", pEncConfig->ctu_qp_enable);
    tcli_print("encAUD:%d\n", pEncConfig->encAUD);
    tcli_print("encEOS:%d\n", pEncConfig->encEOS);
    tcli_print("encEOB:%d\n", pEncConfig->encEOB);
    tcli_print("actRegNum:%d\n", pEncConfig->actRegNum);
    tcli_print("useAsLongtermPeriod:%d\n", pEncConfig->useAsLongtermPeriod);
    tcli_print("refLongtermPeriod:%d\n", pEncConfig->refLongtermPeriod);
    tcli_print("testEnvOptions:%d\n", pEncConfig->testEnvOptions);
    tcli_print("cviApiMode:%d\n", pEncConfig->cviApiMode);
    tcli_print("sizeInWord:%d\n", pEncConfig->sizeInWord);
    tcli_print("userDataBufSize:%d\n", pEncConfig->userDataBufSize);
    tcli_print("bIsoSendFrmEn:%d\n", pEncConfig->bIsoSendFrmEn);

    tcli_print("cviEncCfg cviEc info:\n");
    cviEncCfg *pcviEc = &pEncConfig->cviEc;

    tcli_print("	rcMode:%d\n", pcviEc->rcMode);
    tcli_print("	s32IPQpDelta:%d\n", pcviEc->s32IPQpDelta);
    tcli_print("	iqp:%d\n", pcviEc->iqp);
    tcli_print("	pqp:%d\n", pcviEc->pqp);
    tcli_print("	gop:%d\n", pcviEc->gop);
    tcli_print("	bitrate:%d\n", pcviEc->bitrate);
    tcli_print("	firstFrmstartQp:%d\n", pcviEc->firstFrmstartQp);
    tcli_print("	framerate:%d\n", pcviEc->framerate);
    tcli_print("	u32MaxIprop:%d\n", pcviEc->u32MaxIprop);
    tcli_print("	u32MinIprop:%d\n", pcviEc->u32MinIprop);
    tcli_print("	u32MaxQp:%d\n", pcviEc->u32MaxQp);
    tcli_print("	u32MinQp:%d\n", pcviEc->u32MinQp);
    tcli_print("	u32MaxIQp:%d\n", pcviEc->u32MaxIQp);
    tcli_print("	u32MinIQp:%d\n", pcviEc->u32MinIQp);
    tcli_print("	maxbitrate:%d\n", pcviEc->maxbitrate);
    tcli_print("	initialDelay:%d\n", pcviEc->initialDelay);
    tcli_print("	statTime:%d\n", pcviEc->statTime);

    tcli_print("	bitstreamBufferSize:%d\n", pcviEc->bitstreamBufferSize);
    tcli_print("	singleLumaBuf:%d\n", pcviEc->singleLumaBuf);
    tcli_print("	bSingleEsBuf:%d\n", pcviEc->bSingleEsBuf);
    tcli_print("	roi_request:%d\n", pcviEc->roi_request);
    tcli_print("	roi_enable	mode	roi_qp		x");
    tcli_print("		y		width		height\n");
    for (int j = 0; j < 8; j++)
    {
        tcli_print("	:%d		%d		%d		%d",
                   pcviEc->cviRoi[j].roi_enable,
                   pcviEc->cviRoi[j].roi_qp_mode,
                   pcviEc->cviRoi[j].roi_qp,
                   pcviEc->cviRoi[j].roi_rect_x);
        tcli_print("		%d		%d		%d\n",
                   pcviEc->cviRoi[j].roi_rect_y,
                   pcviEc->cviRoi[j].roi_rect_width,
                   pcviEc->cviRoi[j].roi_rect_height);
    }
    tcli_print("	virtualIPeriod:%d\n", pcviEc->virtualIPeriod);
    tcli_print("	frmLostBpsThr:%d\n", pcviEc->frmLostBpsThr);
    tcli_print("	encFrmGaps:%d\n", pcviEc->encFrmGaps);
    tcli_print("	s32ChangePos:%d\n", pcviEc->s32ChangePos);

    tcli_print("interruptTimeout:%d\n", pTestEnc->interruptTimeout);
    tcli_print("bsBufferCount:%d\n", pTestEnc->bsBufferCount);
    tcli_print("srcFrameIdx:%d\n", pTestEnc->srcFrameIdx);
    tcli_print("frameIdx:%d\n", pTestEnc->frameIdx);
    tcli_print("coreIdx:%d\n", pTestEnc->coreIdx);
    CodecInst *pCodecInst = pTestEnc->handle;

    showCodecInstPoolInfo(pCodecInst);

    return 0;
}
#else
static int cviVEncShowChnInfo(stTestEncoder *pTestEnc, void *arg)
{
    UNUSED(pTestEnc);
    UNUSED(arg);

    return 0;
}

#endif

static int cviVEncSetUserRcInfo(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig *pEncConfig = &pTestEnc->encConfig;
    cviEncCfg     *pCviEc     = &pEncConfig->cviEc;
    cviUserRcInfo *puri       = (cviUserRcInfo *)arg;
    int            ret        = 0;

    CVI_VC_IF("\n");

    pCviEc->bQpMapValid = puri->bQpMapValid;
    pCviEc->pu8QpMap    = (Uint8 *)((uintptr_t)puri->u64QpMapPhyAddr);

    CVI_VC_CFG("bQpMapValid = %d\n", pCviEc->bQpMapValid);
    CVI_VC_CFG("pu8QpMap = %p\n", pCviEc->pu8QpMap);

    return ret;
}

static int cviVEncSetSuperFrame(stTestEncoder *pTestEnc, void *arg)
{
    TestEncConfig *pEncConfig = &pTestEnc->encConfig;
    cviEncCfg     *pCviEc     = &pEncConfig->cviEc;
    cviSuperFrame *pSuper     = (cviSuperFrame *)arg;
    int            ret        = 0;

    CVI_VC_IF("\n");

    pCviEc->enSuperFrmMode      = pSuper->enSuperFrmMode;
    pCviEc->u32SuperIFrmBitsThr = pSuper->u32SuperIFrmBitsThr;
    pCviEc->u32SuperPFrmBitsThr = pSuper->u32SuperPFrmBitsThr;

    CVI_VC_CFG("enSuperFrmMode = %d, IFrmBitsThr = %d, PFrmBitsThr = %d\n",
               pCviEc->enSuperFrmMode, pCviEc->u32SuperIFrmBitsThr, pCviEc->u32SuperPFrmBitsThr);

    return ret;
}

typedef struct _CVI_VENC_IOCTL_OP_
{
    int opNum;
    int (*ioctlFunc)(stTestEncoder *pTestEnc, void *arg);
} CVI_VENC_IOCTL_OP;

CVI_VENC_IOCTL_OP cviIoctlOp[] = {
    {                   CVI_H26X_OP_NONE,                      NULL},
    {           CVI_H26X_OP_SET_RC_PARAM,              cviVEncSetRc},
    {                  CVI_H26X_OP_START,              cviVEncStart},
    {                 CVI_H26X_OP_GET_FD,            cviVEncOpGetFd},
    {        CVI_H26X_OP_SET_REQUEST_IDR,      cviVEncSetRequestIDR},
    {           CVI_H26X_OP_SET_CHN_ATTR,       cviVEncOpSetChnAttr},
    {          CVI_H26X_OP_SET_REF_PARAM,             cviVEncSetRef},
    {          CVI_H26X_OP_SET_ROI_PARAM,             cviVEncSetRoi},
    {          CVI_H26X_OP_GET_ROI_PARAM,             cviVEncGetRoi},
    {CVI_H26X_OP_SET_FRAME_LOST_STRATEGY,       cviVEncSetFrameLost},
    {            CVI_H26X_OP_GET_VB_INFO,          cviVEncGetVbInfo},
    {          CVI_H26X_OP_SET_VB_BUFFER,        cviVEncSetVbBuffer},
    {          CVI_H26X_OP_SET_USER_DATA,     cviVEncEncodeUserData},
    {            CVI_H26X_OP_SET_PREDICT,            cviVEncSetPred},
    {       CVI_H26X_OP_SET_H264_ENTROPY,     cviVEncSetH264Entropy},
    {         CVI_H26X_OP_SET_H264_TRANS,       cviVEncSetH264Trans},
    {         CVI_H26X_OP_SET_H265_TRANS,       cviVEncSetH265Trans},
    {          CVI_H26X_OP_REG_VB_BUFFER,        cviVEncRegReconBuf},
    {    CVI_H26X_OP_SET_IN_PIXEL_FORMAT,   cviVEncSetInPixelFormat},
    {           CVI_H26X_OP_GET_CHN_INFO,        cviVEncShowChnInfo},
    {       CVI_H26X_OP_SET_USER_RC_INFO,      cviVEncSetUserRcInfo},
    {        CVI_H26X_OP_SET_SUPER_FRAME,      cviVEncSetSuperFrame},
    {           CVI_H26X_OP_SET_H264_VUI,         cviVEncSetH264Vui},
    {           CVI_H26X_OP_SET_H265_VUI,         cviVEncSetH265Vui},
    {        CVI_H26X_OP_SET_FRAME_PARAM,      cviVEncSetFrameParam},
    {       CVI_H26X_OP_CALC_FRAME_PARAM,     cviVEncCalcFrameParam},
    {            CVI_H26X_OP_SET_SB_MODE,          cviVEncSetSbMode},
    {          CVI_H26X_OP_START_SB_MODE,        cviVEncStartSbMode},
    {         CVI_H26X_OP_UPDATE_SB_WPTR,       cviVEncUpdateSbWptr},
    {               CVI_H26X_OP_RESET_SB,            cviVEncResetSb},
    {       CVI_H26X_OP_SB_EN_DUMMY_PUSH,      cviVEncSbEnDummyPush},
    {     CVI_H26X_OP_SB_TRIG_DUMMY_PUSH,    cviVEncSbTrigDummyPush},
    {      CVI_H26X_OP_SB_DIS_DUMMY_PUSH,     cviVEncSbDisDummyPush},
    { CVI_H26X_OP_SB_GET_SKIP_FRM_STATUS, cviVEncSbGetSkipFrmStatus},
    {         CVI_H26X_OP_SET_SBM_ENABLE,       cviVEncSetSbmEnable},
};

int cviVEncIoctl(void *handle, int op, void *arg)
{
    stTestEncoder *pTestEnc = (stTestEncoder *)handle;
    int            ret      = 0;
    int            currOp;

    CVI_VC_IF("\n");

    if (op <= 0 || op >= CVI_H26X_OP_MAX)
    {
        CVI_VC_ERR("op = %d\n", op);
        return -1;
    }

    currOp = (cviIoctlOp[op].opNum & CVI_H26X_OP_MASK) >> CVI_H26X_OP_SHIFT;
    if (op != currOp)
    {
        CVI_VC_ERR("op = %d\n", op);
        return -1;
    }

    ret = cviIoctlOp[op].ioctlFunc(pTestEnc, arg);

    return ret;
}

void *FnEncoder(void *param)
{
    int            ret;
    TestEncConfig *pEncConfig = (TestEncConfig *)param;

    CVI_VC_TRACE("start\n");

    ret = TestEncoder(pEncConfig);

    CVI_VC_TRACE("exit %d\n", ret);

    if (ret)
        return param;
    else
        return NULL;
}

int cvitest_venc_main(int argc, char **argv)
{
    BOOL           debugMode = FALSE;
    TestEncConfig *pEncConfig[MAX_NUM_INSTANCE];
    pthread_t      thread_id[MAX_NUM_INSTANCE];
    int            ret = 0, pthread_ret = 0;
    void          *thread_ret[MAX_NUM_INSTANCE];

    for (int i = 0; i < MAX_NUM_INSTANCE; i++)
    {
        pEncConfig[i] = &encConfig[i];
    }

    ret = cviVcodecInit();
    if (ret < 0)
    {
        CVI_VC_INFO("cviVcodecInit, %d\n", ret);
    }

    ret = initEncConfigByArgv(argc, argv, pEncConfig[0]);
    if (ret < 0)
    {
        CVI_VC_ERR("initEncConfigByArgv\n");
        return ret;
    }

    VDI_POWER_ON_DOING_JOB(pEncConfig[0]->coreIdx, ret, initMcuEnv(pEncConfig[0]));

    if (ret != INIT_TEST_ENCODER_OK)
    {
        CVI_VC_ERR("initMcuEnv, ret = %d\n", ret);
        return ret;
    }

    for (int i = 1; i < MAX_NUM_INSTANCE; i++)
    {
        memcpy(&(encConfig[i]), &(encConfig[0]), sizeof(encConfig[i]));
        encConfig[i].instNum = (i);
    }

    ret = 1;

    if (gNumInstance > MAX_NUM_INSTANCE)
    {
        CVI_VC_ERR("gNumInstance(%d) > MAX_NUM_INSTANCE(%d) error!\n", gNumInstance, MAX_NUM_INSTANCE);
        ret = 0;
        goto BAILOUT;
    }

    for (int i = 0; i < gNumInstance; i++)
    {
        pthread_ret = pthread_create(&thread_id[i], NULL, FnEncoder, pEncConfig[i]);
        if (pthread_ret)
        {
            CVI_VC_ERR("pthread_create error!\n");
            ret = 0;
            goto BAILOUT;
        }
    }

    for (int i = 0; i < gNumInstance; i++)
    {
        pthread_ret = pthread_join(thread_id[i], &thread_ret[i]);
        if (pthread_ret)
        {
            CVI_VC_ERR("pthread_join error!\n");
            ret = 0;
        }

        CVI_VC_INFO("pthred join [%d] thd ret %p\n", i, thread_ret[i]);

        if (thread_ret[i] == NULL)
        {
            // if any of thread return NULL, means failure at that thread
            CVI_VC_ERR("thread %d failed\n", i);
            ret = 0;
        }
    }

BAILOUT:
    if (debugMode == TRUE)
    {
        VPU_DeInit(pEncConfig[0]->coreIdx);
    }
    CVI_VC_INFO("vdi_release\n");
    vdi_release(pEncConfig[0]->coreIdx);

    if (pEncConfig[0]->pusBitCode)
    {
        free(pEncConfig[0]->pusBitCode);
        pEncConfig[0]->pusBitCode = NULL;
        pEncConfig[0]->sizeInWord = 0;
    }

    return ret == 1 ? 0 : 1;
}

int initMcuEnv(TestEncConfig *pEncConfig)
{
    Uint32 productId;
    int    ret = 0;
#ifndef FIRMWARE_H
    char *fwPath = NULL;
#endif

    ret = cviSetCoreIdx(&pEncConfig->coreIdx, pEncConfig->stdMode);
    if (ret)
    {
        ret = TE_ERR_ENC_INIT;
        CVI_VC_ERR("cviSetCoreIdx, ret = %d\n", ret);
        return ret;
    }

    productId = VPU_GetProductId(pEncConfig->coreIdx);

    if (checkEncConfig(pEncConfig, productId))
    {
        CVI_VC_ERR("checkEncConfig\n");
        return -1;
    }

#ifdef FIRMWARE_H
    if (productId != PRODUCT_ID_420L && productId != PRODUCT_ID_980)
    {
        CVI_VC_ERR("productId = %d\n", productId);
        return -1;
    }

    if (pEncConfig->sizeInWord == 0 && pEncConfig->pusBitCode == NULL)
    {
        if (LoadFirmwareH(productId, (Uint8 **)&pEncConfig->pusBitCode, &pEncConfig->sizeInWord) < 0)
        {
            CVI_VC_ERR("Failed to load firmware: productId = %d\n", productId);
            return 1;
        }
    }
#else
    switch (productId)
    {
    case PRODUCT_ID_980:
        fwPath = CORE_1_BIT_CODE_FILE_PATH;
        break;
    case PRODUCT_ID_420L:
        fwPath = CORE_5_BIT_CODE_FILE_PATH;
        break;
    default:
        CVI_VC_ERR("Unknown product id: %d\n", productId);
        return -1;
    }

    if (LoadFirmware(productId, (Uint8 **)&pusBitCode, &sizeInWord,
                     fwPath)
        < 0)
    {
        CVI_VC_ERR("Failed to load firmware: %s\n", fwPath);
        return 1;
    }
#endif

    return INIT_TEST_ENCODER_OK;
}

static int pickupNextEncChnNum(void)
{
    tVencSbmInfo.rrChnIdx =
        (tVencSbmInfo.rrChnIdx + 1) % tVencSbmInfo.numEncChn;
    return tVencSbmInfo.frmChnNum[tVencSbmInfo.rrChnIdx];
}

static void *pfnWaitEncodeDone(void *param)
{
    int            ret;
    stTestEncoder *pTestEnc   = (stTestEncoder *)param;
    CodecInst     *pCodecInst = pTestEnc->handle;

    while (1)
    {
        // wait for enc cmd trigger
        SEM_WAIT(pTestEnc->semSendEncCmd);
        pTestEnc->encConfig.cviEc.originPicType = PIC_TYPE_MAX;
        ret                                     = cviGetOneStream(pTestEnc, &pTestEnc->tStreamInfo, TIME_BLOCK_MODE);

        if (ret == TE_ERR_ENC_IS_SUPER_FRAME)
        {
            ret = cviProcessSuperFrame(pTestEnc, &pTestEnc->tStreamInfo, TIME_BLOCK_MODE);
            SEM_POST(pTestEnc->semGetStreamCmd);
        }
        else if (ret == RETCODE_SUCCESS)
        {
            SEM_POST(pTestEnc->semGetStreamCmd);

            if (tVencSbmInfo.bFRModeEn)
            {
                SEM_POST(tSwitchCoreIdxWaitQueue);
            }
            // notify send frame thread
            if (tVencSbmInfo.bFRModeEn)
            {
                /* for 1SBM + 1FRM case,
				 * SBM encode cmd will be triggered after FRM done
				 */
                if (!pCodecInst->CodecInfo->encInfo.bSbmEn)
                {
                    frm_done_event[tVencSbmInfo.sbmChnNum] = 1;
                    SEM_POST(tSbmWaitQueue[tVencSbmInfo.sbmChnNum]);
                }
                else
                {
                    if (err_frm_skip[pCodecInst->s32ChnNum] == 1)
                    {
                        frm_done_event[pCodecInst->s32ChnNum] = 1;
                        SEM_POST(tSbmWaitQueue[pCodecInst->s32ChnNum]);
                    }
                }
            }
            else if (tVencSbmInfo.bSBModeEn)
            {
                frm_done_event[pCodecInst->s32ChnNum] = 1;
                SEM_POST(tSbmWaitQueue[pCodecInst->s32ChnNum]);
            }
        }
        else
        {
            CVI_VC_ERR("cviGetOneStream, ret = %d\n", ret);

            return NULL;
        }
    }
    return NULL;
}

void *cviVEncOpen(cviInitEncConfig *pInitEncCfg)
{
    stTestEncoder *pTestEnc = NULL;
    int            ret      = 0;
    TestEncConfig *pEncConfig;
    Uint32         core_idx = pInitEncCfg->codec == CODEC_H265 ? CORE_H265 : CORE_H264;

    ret = cviVcodecInit();
    if (ret < 0)
    {
        CVI_VC_INFO("cviVcodecInit, %d\n", ret);
    }

    CVI_VC_IF("\n");
    MUTEX_INIT(sSwitchCoreIdxSem);
    InitVcodecLock(core_idx);

    EnterVcodecLock(core_idx);

#if defined(__CV180X__)
    /* disable cpu access vc sram for 180x  romcode use vc sram*/
    cvi_vc_drv_write_vc_reg(REG_CTRL, 0x10, cvi_vc_drv_read_vc_reg(REG_CTRL, 0x10) & (~0x1));
#endif

    pTestEnc = (stTestEncoder *)malloc(sizeof(stTestEncoder));
    if (!pTestEnc)
    {
        CVI_VC_ERR("VENC context malloc\n");
        goto ERR_CVI_VENC_OPEN;
    }

    pEncConfig = &pTestEnc->encConfig;
    initEncConfigByCtx(pEncConfig, pInitEncCfg);

    ret = initMcuEnv(pEncConfig);

    if (ret != INIT_TEST_ENCODER_OK)
    {
        CVI_VC_ERR("initMcuEnv, ret = %d\n", ret);
        free(pTestEnc);
        pTestEnc = NULL;
    }
    MUTEX_LOCK(sSwitchCoreIdxSem);
    tVencSbmInfo.frmChnNum[tVencSbmInfo.numEncChn] = pInitEncCfg->s32ChnNum;
    tVencSbmInfo.numEncChn++;
    MUTEX_UNLOCK(sSwitchCoreIdxSem);

ERR_CVI_VENC_OPEN:
    LeaveVcodecLock(core_idx);

    return (void *)pTestEnc;
}

int cviVEncClose(void *handle)
{
    int            i, ret = 0;
    stTestEncoder *pTestEnc = (stTestEncoder *)handle;
    TestEncConfig *pEncCfg  = &pTestEnc->encConfig;

    CVI_VC_IF("\n");

    EnterVcodecLock(pTestEnc->coreIdx);

    ret = cviCheckAndCompareBitstream(pTestEnc);
    if (ret == TE_ERR_ENC_OPEN)
    {
        CVI_VC_ERR("cviCheckAndCompareBitstream, ret = %d\n", ret);
        LeaveVcodecLock(pTestEnc->coreIdx);
        return ret;
    }

    cviCloseVpuEnc(pTestEnc);
    cviDeInitVpuEnc(pTestEnc);

    if (pTestEnc->encConfig.pusBitCode)
    {
        free(pTestEnc->encConfig.pusBitCode);
        pTestEnc->encConfig.pusBitCode = NULL;
        pTestEnc->encConfig.sizeInWord = 0;
    }

    // user data bufs
    for (i = 0; i < NUM_OF_USER_DATA_BUF; ++i)
    {
        if (pTestEnc->encConfig.userDataBuf[i])
        {
            free(pTestEnc->encConfig.userDataBuf[i]);
            pTestEnc->encConfig.userDataBuf[i] = NULL;
            pTestEnc->encConfig.userDataLen[i] = 0;
        }
    }
    pTestEnc->encConfig.userDataBufSize = 0;

    if (pTestEnc->tPthreadId)
    {
        pthread_cancel(pTestEnc->tPthreadId);
        pthread_join(pTestEnc->tPthreadId, NULL);

        pTestEnc->tPthreadId = NULL;
    }

    if (pEncCfg->bIsoSendFrmEn)
    {
        //SEM_DESTROY(pTestEnc->semSendEncCmd);
        //SEM_DESTROY(pTestEnc->semGetStreamCmd);
    }
    LeaveVcodecLock(pTestEnc->coreIdx);
    DeinitVcodecLock(pTestEnc->coreIdx);
    MUTEX_LOCK(sSwitchCoreIdxSem);
    if (pTestEnc->handle->s32ChnNum == tVencSbmInfo.sbmChnNum)
    {
        tVencSbmInfo.sbmChnNum = -1;
        tVencSbmInfo.bSBModeEn = FALSE;
        tVencSbmInfo.bFRModeEn = FALSE;
    }
    tVencSbmInfo.numEncChn--;
    MUTEX_UNLOCK(sSwitchCoreIdxSem);

    if (pTestEnc)
        free(pTestEnc);

    return ret;
}

int initEncConfigByArgv(int argc, char **argv, TestEncConfig *pEncConfig)
{
    int ret = 0;

    initDefaultEncConfig(pEncConfig);

    CVI_VC_TRACE("\n");

    pEncConfig->cviApiMode   = API_MODE_DRIVER;
    pEncConfig->lineBufIntEn = 1;

    // from BITMONET-68 in CnM Jira:
    // There are two bit-stream handling method such as 'ring-buffer' and 'line-bufer'.
    // Also these two method cannot be activated together.
    if (pEncConfig->lineBufIntEn == pEncConfig->ringBufferEnable)
        CVI_VC_ERR("pEncConfig->lineBufIntEn != !pEncConfig->ringBufferEnable\n");

    return ret;
}

void initEncConfigByCtx(TestEncConfig *pEncConfig, cviInitEncConfig *pInitEncCfg)
{
    int        i;
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

    initDefaultEncConfig(pEncConfig);

    pEncConfig->cviApiMode   = API_MODE_SDK;
    pEncConfig->lineBufIntEn = 1;
    if (pInitEncCfg->bSingleEsBuf == 1)
        pEncConfig->bEsBufQueueEn = 0;
    else
        pEncConfig->bEsBufQueueEn = pInitEncCfg->bEsBufQueueEn;
    if (pInitEncCfg->bSingleEsBuf == 1)
        pEncConfig->bIsoSendFrmEn = 0;
    else
        pEncConfig->bIsoSendFrmEn = pInitEncCfg->bIsoSendFrmEn;
    pEncConfig->s32ChnNum = pInitEncCfg->s32ChnNum;

    if (pInitEncCfg->codec == CODEC_H264)
    {
        pEncConfig->stdMode = STD_AVC;

        pEncConfig->mapType                  = TILED_FRAME_V_MAP;
        pEncConfig->coda9.enableLinear2Tiled = TRUE;
        pEncConfig->coda9.linear2TiledMode   = FF_FRAME;

        pEncConfig->secondary_axi                     = SECONDARY_AXI_H264;
        pEncConfig->cviEc.h264Entropy.entropyEncModeI = CVI_CABAC;
        pEncConfig->cviEc.h264Entropy.entropyEncModeP = CVI_CABAC;
    }
    else
    {
        pEncConfig->stdMode       = STD_HEVC;
        pEncConfig->secondary_axi = SECONDARY_AXI_H265;
    }

    CVI_VC_CFG("stdMode = %s, 2ndAXI = 0x%X\n",
               (pInitEncCfg->codec == CODEC_H264) ? "264" : "265", pEncConfig->secondary_axi);

    pEncConfig->yuv_mode = SOURCE_YUV_ADDR;
    pEncConfig->outNum   = 0x7fffffff;

    pEncConfig->picWidth  = pInitEncCfg->width;
    pEncConfig->picHeight = pInitEncCfg->height;
    CVI_VC_CFG("picWidth = %d, picHeight = %d\n", pEncConfig->picWidth, pEncConfig->picHeight);

    pCviEc->u32Profile              = pInitEncCfg->u32Profile;
    pEncConfig->rcMode              = pInitEncCfg->rcMode;
    pCviEc->rcMode                  = pInitEncCfg->rcMode;
    pEncConfig->decodingRefreshType = pInitEncCfg->decodingRefreshType;
    pCviEc->s32IPQpDelta            = pInitEncCfg->s32IPQpDelta;
    pCviEc->s32BgQpDelta            = pInitEncCfg->s32BgQpDelta;
    pCviEc->s32ViQpDelta            = pInitEncCfg->s32ViQpDelta;
    CVI_VC_CFG("Profile = %d, rcMode = %d, RefreshType = %d, s32IPQpDelta = %d, s32BgQpDelta = %d, s32ViQpDelta = %d\n",
               pCviEc->u32Profile,
               pCviEc->rcMode,
               pEncConfig->decodingRefreshType,
               pCviEc->s32IPQpDelta,
               pCviEc->s32BgQpDelta,
               pCviEc->s32ViQpDelta);

    pCviEc->iqp     = pInitEncCfg->iqp;
    pCviEc->pqp     = pInitEncCfg->pqp;
    pCviEc->gop     = pInitEncCfg->gop;
    pCviEc->bitrate = pInitEncCfg->bitrate;
    CVI_VC_CFG("iqp = %d, pqp = %d, gop = %d, bitrate = %d\n",
               pInitEncCfg->iqp, pInitEncCfg->pqp, pInitEncCfg->gop, pInitEncCfg->bitrate);

    pCviEc->framerate    = pInitEncCfg->framerate;
    pCviEc->maxbitrate   = pInitEncCfg->maxbitrate;
    pCviEc->statTime     = pInitEncCfg->statTime;
    pCviEc->initialDelay = pInitEncCfg->initialDelay;
    CVI_VC_CFG("framerate = %d, maxbitrate = %d, statTime = %d, initialDelay = %d\n",
               pInitEncCfg->framerate, pCviEc->maxbitrate, pCviEc->statTime, pCviEc->initialDelay);

    pCviEc->bitstreamBufferSize = pInitEncCfg->bitstreamBufferSize;
    pCviEc->singleLumaBuf       = pInitEncCfg->singleLumaBuf;
    pCviEc->addrRemapEn         = pInitEncCfg->addrRemapEn;
    pCviEc->bSingleEsBuf        = pInitEncCfg->bSingleEsBuf;
    CVI_VC_CFG("bs size = 0x%X, 1YBuf = %d, bSingleEsBuf = %d\n",
               pCviEc->bitstreamBufferSize, pCviEc->singleLumaBuf, pCviEc->bSingleEsBuf);

    if (pInitEncCfg->virtualIPeriod)
    {
        pCviEc->virtualIPeriod = pInitEncCfg->virtualIPeriod;
        CVI_VC_CFG("virtualIPeriod = %d\n", pCviEc->virtualIPeriod);
    }

    // user data bufs
    pEncConfig->userDataBufSize = pInitEncCfg->userDataMaxLength;
    for (i = 0; i < NUM_OF_USER_DATA_BUF; i++)
    {
        pEncConfig->userDataBuf[i] = (Uint8 *)malloc(pEncConfig->userDataBufSize);
        pEncConfig->userDataLen[i] = 0;
    }

    // from BITMONET-68 in CnM Jira:
    // There are two bit-stream handling method such as 'ring-buffer' and 'line-bufer'.
    // Also these two method cannot be activated together.
    if (pEncConfig->lineBufIntEn == pEncConfig->ringBufferEnable)
        CVI_VC_ERR("pEncConfig->lineBufIntEn == pEncConfig->ringBufferEnable\n");
}

void initDefaultEncConfig(TestEncConfig *pEncConfig)
{
    cviEncCfg *pCviEc = &pEncConfig->cviEc;

#ifndef PLATFORM_NON_OS
    cviVcodecMask();
#endif

    CVI_VC_TRACE("\n");

    cviVcodecGetVersion();

#if CFG_MEM
    memset(&dramCfg, 0x0, sizeof(DRAM_CFG));
#endif

#ifdef ENABLE_LOG
    InitLog();
#endif

    memset(pEncConfig, 0, sizeof(TestEncConfig));
#ifdef CODA980
    pEncConfig->stdMode = STD_AVC;
    pEncConfig->mapType = LINEAR_FRAME_MAP;
#else
    pEncConfig->stdMode = STD_HEVC;
    pEncConfig->mapType = COMPRESSED_FRAME_MAP;
#endif
    pEncConfig->frame_endian  = VPU_FRAME_ENDIAN;
    pEncConfig->stream_endian = VPU_STREAM_ENDIAN;
    pEncConfig->source_endian = VPU_SOURCE_ENDIAN;
#ifdef PLATFORM_NON_OS
    pEncConfig->yuv_mode = SOURCE_YUV_FPGA;
#endif
    pEncConfig->RcEnable    = -1;
    pEncConfig->picQpY      = -1;
    pCviEc->firstFrmstartQp = -1;
    pCviEc->framerate       = 30;

    COMP(pEncConfig->coreIdx, 0);
}

#if 0 //ndef USE_KERNEL_MODE
static int parseArgs(int argc, char **argv, TestEncConfig *pEncConfig)
{
	struct option options[MAX_GETOPT_OPTIONS];
	struct OptionExt options_help[] = {
		{ "output", 1, NULL, 0,
		  "--output                    bitstream path\n" },
		{ "input", 1, NULL, 0,
		  "--input                     YUV file path\n" },
		  /* The value of InputFile in a cfg is replaced to this value */
		{ "codec", 1, NULL, 0,
		  "--codec                     codec index, HEVC = 12, AVC = 0\n" },
		{ "cfgFileName", 1, NULL, 0,
		  "--cfgFileName               cfg file path\n" },
		{ "cfgFileName0", 1, NULL, 0,
		  "--cfgFileName0              cfg file 0 path\n" },
		{ "cfgFileName1", 1, NULL, 0,
		  "--cfgFileName1              cfg file 1 path\n" },
		{ "cfgFileName2", 1, NULL, 0,
		  "--cfgFileName2              cfg file 2 path\n" },
		{ "cfgFileName3", 1, NULL, 0,
		  "--cfgFileName3              cfg file 3 path\n" },
		{ "coreIdx", 1, NULL, 0,
		  "--coreIdx                   core index: default 0\n" },
		{ "picWidth", 1, NULL, 0,
		  "--picWidth                  source width\n" },
		{ "picHeight", 1, NULL, 0,
		  "--picHeight                 source height\n" },
		{ "EncBitrate", 1, NULL, 0,
		  "--EncBitrate                RC bitrate in kbps\n" },
		  /* In case of without cfg file, if this option has value then RC will be enabled */
		{ "RcMode", 1, NULL, 0,
		  "--RcMode                    RC mode. 0: CBR\n" },
		{ "changePos", 1, NULL, 0,
		  "--changePos                 VBR bitrate change poistion\n" },
		{ "frmLostOpen", 1, NULL, 0,
		  "--frmLostOpen               auto-skip frame enable\n" },
		{ "maxIprop", 1, NULL, 0,
		  "--maxIprop                  max I frame bitrate ratio to P frame\n" },
		{ "enable-ringBuffer", 0, NULL, 0,
		  "--enable-ringBuffer         enable stream ring buffer mode\n" },
		{ "enable-lineBufInt", 0, NULL, 0,
		  "--enable-lineBufInt         enable linebuffer interrupt\n" },
		{ "mapType", 1, NULL, 0,
		  "--mapType                   mapType\n" },
		{ "loop-count", 1, NULL, 0,
		  "--loop-count                integer number. loop test, default 0\n" },
		{ "enable-cbcrInterleave", 0, NULL, 0,
		  "--enable-cbcrInterleave     enable cbcr interleave\n" },
		{ "nv21", 1, NULL, 0,
		  "--nv21                      enable NV21(must set enable-cbcrInterleave)\n" },
		{ "packedFormat", 1, NULL, 0,
		  "--packedFormat              1:YUYV, 2:YVYU, 3:UYVY, 4:VYUY\n" },
		{ "rotAngle", 1, NULL, 0,
		  "--rotAngle                  rotation angle(0,90,180,270), Not supported on WAVE420L\n" },
		{ "mirDir", 1, NULL, 0,
		  "--mirDir                    1:Vertical, 2: Horizontal, 3:Vert-Horz, Not supported on WAVE420L\n" },
		  /* 20 */
		{ "secondary-axi", 1, NULL, 0,
		  "--secondary-axi             0~7: bit mask values, Please refer programmer's guide or datasheet\n" },
		  /* 1:IMD(not supported on WAVE420L), 2: RDO, 4: LF */
		{ "frame-endian", 1, NULL, 0,
		  "--frame-endian              16~31, default 31(LE) Please refer programmer's guide or datasheet\n" },
		{ "stream-endian", 1, NULL, 0,
		  "--stream-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n" },
		{ "source-endian", 1, NULL, 0,
		  "--source-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n" },
		{ "ref_stream_path", 1, NULL, 0,
		  "--ref_stream_path           golden data which is compared with encoded stream when -c option\n" },
		{ "srcFormat3p4b", 1, NULL, 0,
		  "--srcFormat3p4b             [WAVE420]MUST BE enabled when yuv src format 3pixel 4byte format\n" },
		{ "decodingRefreshType", 1, NULL, 0,
		  "--decRefreshType            decodingRefreshType, 0 = disable, 1 = enable\n" },
		{ "gopSize", 1, NULL, 0,
		  "--gopSize                   gopSize, The interval of 2 Intra frames\n" },
		{ "tempLayer", 1, NULL, 0,
		  "--tempLayer                 temporal layer coding, Range from 1 to 3\n" },
		{ "useDeriveLambdaWeight", 1, NULL, 0,
		  "--useDeriveLambdaWeight     useDeriveLambdaWeight, 1 = use derived Lambda weight, 0 = No\n" },
		{ "dynamicMergeEnable", 1, NULL, 0,
		  "--dynamicMergeEnable        dynamicMergeEnable, 1 = enable dynamic merge, 0 = disable\n" },
		{ "IndeSliceMode", 1, NULL, 0,
		  "--IndeSliceMode             IndeSliceMode, 1 = enable, 0 = disable\n" },
		{ "IndeSliceArg", 1, NULL, 0,
		  "--IndeSliceArg              IndeSliceArg\n" },
		{ "RateControl", 1, NULL, 0,
		  "--RateControl               RateControl\n" },
		{ "RcInitQp", 1, NULL, 0,
		  "--RcInitQp                  Initial QP of rate control\n" },
		{ "PIC_QP_Y", 1, NULL, 0,
		  "--PIC_QP_Y                  PIC_QP_Y\n" },
		{ "RoiCfgType", 1, NULL, 0,
		  "--RoiCfgType                Roi map cfg type\n" },
#if CFG_MEM
		{ "code-addr", 1, NULL, 0,
		  "--code-addr                 cvitest, fw address\n" },
		{ "code-size", 1, NULL, 0,
		  "--code-size                 cvitest, fw size\n" },
		{ "vpu-dram-addr", 1, NULL, 0,
		  "--vpu-dram-addr             cvitest, dram address for vcodec\n" },
		{ "vpu-dram-size", 1, NULL, 0,
		  "--vpu-dram-size             cvitest, dram size for vcodec\n" },
		{ "src-yuv-addr", 1, NULL, 0,
		  "--src-yuv-addr              cvitest, source yuv address\n" },
		{ "src-yuv-size", 1, NULL, 0,
		  "--src-yuv-size              cvitest, source yuv size\n" },
#endif
		{ NULL, 0, NULL, 0, NULL },
	};
	char *optString = "c:rbhvn:t:";
	int opt, index = 0, ret = 0, i;
	cviEncCfg *pCviEc = &pEncConfig->cviEc;

	for (i = 0; i < MAX_GETOPT_OPTIONS; i++) {
		if (options_help[i].name == NULL)
			break;
		memcpy(&options[i], &options_help[i], sizeof(struct option));
	}

	CVI_VC_TRACE("argc = %d, optString = %s\n", argc, optString);

	while ((opt = getopt_long(argc, (char *const *)argv, optString, options,
				  &index)) != -1) {
		CVI_VC_TRACE("opt = %c, index = %d, optarg = %s\n", opt, index, optarg);

		switch (opt) {
		case 'n':
			pEncConfig->outNum = atoi(optarg);
			CVI_VC_TRACE("optarg = %s, outNum = %d\n", optarg, pEncConfig->outNum);
			break;
		case 'c':
			break;
		case 'h':
			help(options_help, argv[0]);
			return 0;
		case 't':
			gNumInstance = atoi(optarg);
			CVI_VC_CFG("gNumInstance %d\n", gNumInstance);
			break;
		case 0:
			CVI_VC_TRACE("name = %s\n", options[index].name);
			if (!strcmp(options[index].name, "output")) {
			} else if (!strcmp(options[index].name, "input")) {
				strcpy(optYuvPath, optarg);
			} else if (!strcmp(options[index].name, "codec")) {
				pEncConfig->stdMode = (CodStd)atoi(optarg);
				if (pEncConfig->stdMode == STD_AVC)
					pEncConfig->mapType = LINEAR_FRAME_MAP;
				else
					pEncConfig->mapType = COMPRESSED_FRAME_MAP;
			} else if (!strcmp(options[index].name, "cfgFileName") ||
					!strcmp(options[index].name, "cfgFileName0")) {
				memcpy(gCfgFileName[0], optarg, strlen(optarg));
			} else if (!strcmp(options[index].name, "cfgFileName1")) {
				memcpy(gCfgFileName[1], optarg, strlen(optarg));
			} else if (!strcmp(options[index].name, "cfgFileName2")) {
				memcpy(gCfgFileName[2], optarg, strlen(optarg));
			} else if (!strcmp(options[index].name, "cfgFileName3")) {
				memcpy(gCfgFileName[3], optarg, strlen(optarg));
			} else if (!strcmp(options[index].name, "coreIdx")) {
				pEncConfig->coreIdx = atoi(optarg);
			} else if (!strcmp(options[index].name, "picWidth")) {
				pEncConfig->picWidth = atoi(optarg);
				CVI_VC_TRACE("picWidth = %d\n", pEncConfig->picWidth);
			} else if (!strcmp(options[index].name, "picHeight")) {
				pEncConfig->picHeight = atoi(optarg);
				CVI_VC_TRACE("picHeight = %d\n", pEncConfig->picHeight);
			} else if (!strcmp(options[index].name, "EncBitrate")) {
				pEncConfig->kbps = atoi(optarg);
			} else if (!strcmp(options[index].name, "RcMode")) {
				pEncConfig->rcMode = atoi(optarg);
			} else if (!strcmp(options[index].name, "changePos")) {
				pEncConfig->changePos = atoi(optarg);
			} else if (!strcmp(options[index].name, "frmLostOpen")) {
				pEncConfig->frmLostOpen = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "enable-ringBuffer")) {
				pEncConfig->ringBufferEnable = TRUE;
			} else if (!strcmp(options[index].name,
					   "enable-lineBufInt")) {
				pEncConfig->lineBufIntEn = TRUE;
			} else if (!strcmp(options[index].name, "loop-count")) {
				pEncConfig->loopCount = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "enable-cbcrInterleave")) {
				pEncConfig->cbcrInterleave = 1;
				if (pEncConfig->stdMode == STD_AVC) {
					pEncConfig->mapType = TILED_FRAME_MB_RASTER_MAP;
					pEncConfig->coda9.enableLinear2Tiled = TRUE;
					pEncConfig->coda9.linear2TiledMode = FF_FRAME;
				}
			} else if (!strcmp(options[index].name, "nv21")) {
				pEncConfig->nv21 = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "packedFormat")) {
				pEncConfig->packedFormat = atoi(optarg);
			} else if (!strcmp(options[index].name, "rotAngle")) {
				pEncConfig->rotAngle = atoi(optarg);
			} else if (!strcmp(options[index].name, "mirDir")) {
				pEncConfig->mirDir = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "secondary-axi")) {
				pEncConfig->secondary_axi = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "frame-endian")) {
				pEncConfig->frame_endian = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "stream-endian")) {
				pEncConfig->stream_endian = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "source-endian")) {
				pEncConfig->source_endian = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "ref_stream_path")) {
			} else if (!strcmp(options[index].name,
					   "srcFormat3p4b")) {
				pEncConfig->srcFormat3p4b = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "decodingRefreshType")) {
				pEncConfig->decodingRefreshType = atoi(optarg);
			} else if (!strcmp(options[index].name, "gopSize")) {
				pEncConfig->gopSize = atoi(optarg);
			} else if (!strcmp(options[index].name, "tempLayer")) {
				pEncConfig->tempLayer = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "useDeriveLambdaWeight")) {
				pEncConfig->useDeriveLambdaWeight =
					atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "dynamicMergeEnable")) {
				pEncConfig->dynamicMergeEnable = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "IndeSliceMode")) {
				pEncConfig->independSliceMode = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "IndeSliceArg")) {
				pEncConfig->independSliceModeArg = atoi(optarg);
			} else if (!strcmp(options[index].name,
					   "RateControl")) {
				pEncConfig->RcEnable = atoi(optarg);
			} else if (!strcmp(options[index].name, "maxIprop")) {
				pCviEc->u32MaxIprop = atoi(optarg);
			} else if (!strcmp(options[index].name, "RcInitQp")) {
				pCviEc->firstFrmstartQp = atoi(optarg);
			} else if (!strcmp(options[index].name, "RoiCfgType")) {
				pEncConfig->roi_cfg_type = atoi(optarg);
			} else if (!strcmp(options[index].name, "PIC_QP_Y")) {
				pEncConfig->picQpY = atoi(optarg);
#if CFG_MEM
			} else if (!strcmp(options[index].name, "code-addr")) {
				dramCfg.pucCodeAddr = strtol(optarg, NULL, 16);
				CVI_VC_TRACE("pucCodeAddr = 0x%lX\n", dramCfg.pucCodeAddr);
			} else if (!strcmp(options[index].name, "code-size")) {
				dramCfg.iCodeSize = atoi(optarg);
				CVI_VC_TRACE("pucCodeSize = 0x%X\n", dramCfg.iCodeSize);
			} else if (!strcmp(options[index].name, "vpu-dram-addr")) {
				dramCfg.pucVpuDramAddr = strtol(optarg, NULL, 16);
				CVI_VC_TRACE("pucVpuDramAddr = 0x%lX\n", dramCfg.pucVpuDramAddr);
			} else if (!strcmp(options[index].name, "vpu-dram-size")) {
				dramCfg.iVpuDramSize = atoi(optarg);
				CVI_VC_TRACE("pucVpuDramSize = 0x%X\n", dramCfg.iVpuDramSize);
			} else if (!strcmp(options[index].name, "src-yuv-addr")) {
				dramCfg.pucSrcYuvAddr = strtol(optarg, NULL, 16);
				CVI_VC_TRACE("pucSrcYuvAddr = 0x%lX\n", dramCfg.pucSrcYuvAddr);
			} else if (!strcmp(options[index].name, "src-yuv-size")) {
				dramCfg.iSrcYuvSize = atoi(optarg);
				CVI_VC_TRACE("pucSrcYuvSize = 0x%X\n", dramCfg.iSrcYuvSize);
#endif
			} else {
				CVI_VC_ERR("not exist pEncConfig = %s\n", options[index].name);
				help(options_help, argv[0]);
				return 1;
			}
			break;
		default:
			help(options_help, argv[0]);
			return 1;
		}
	}

	return ret;
}
#endif

static int checkEncConfig(TestEncConfig *pEncConfig, Uint32 productId)
{
    int ret = 0;

    if (pEncConfig->mapType == TILED_FRAME_MB_RASTER_MAP || pEncConfig->mapType == TILED_FIELD_MB_RASTER_MAP)
        pEncConfig->cbcrInterleave = TRUE;

    if (pEncConfig->rotAngle > 0 || pEncConfig->mirDir > 0)
        pEncConfig->useRot = TRUE;

#if CFG_MEM
    if (checkDramCfg(&dramCfg))
    {
        CVI_VC_ERR("checkDramCfg\n");
        return -1;
    }
#endif

    if (checkParamRestriction(productId, pEncConfig) == FALSE)
    {
        CVI_VC_ERR("checkParamRestriction\n");
        return 1;
    }

    return ret;
}

#if CFG_MEM
static int checkDramCfg(DRAM_CFG *pDramCfg)
{
    if (!pDramCfg->pucCodeAddr || !pDramCfg->iCodeSize || !pDramCfg->pucVpuDramAddr || !pDramCfg->iVpuDramSize || !pDramCfg->pucSrcYuvAddr || !pDramCfg->iSrcYuvSize)
    {
        CVI_VC_TRACE("dramCfg = 0\n");
        return -1;
    }
    return 0;
}
#endif
