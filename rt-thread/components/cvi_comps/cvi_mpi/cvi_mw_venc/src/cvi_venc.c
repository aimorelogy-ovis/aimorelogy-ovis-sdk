#include "cvi_venc.h"

#include "cvi_comm_vb.h"
#include "cvi_comm_venc.h"
#include "cvi_defines.h"
#include "cvi_float_point.h"
#include "cvi_rc_debug.h"
#include "cvi_rc_kernel.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vc_ctrl.h"
#include "errno.h"
#include "inttypes.h"
#include "malloc.h"
#include "pthread.h"
#include "stdint.h"
#include "unistd.h"
#include "venc.h"

#define Q_TABLE_MAX                      99
#define Q_TABLE_CUSTOM                   50
#define Q_TABLE_MIN                      0
#define Q_TABLE_DEFAULT                  0 // 0 = backward compatible
#define SRC_FRAMERATE_DEF                30
#define SRC_FRAMERATE_MAX                240
#define SRC_FRAMERATE_MIN                1
#define DEST_FRAMERATE_DEF               30
#define DEST_FRAMERATE_MAX               60
#define DEST_FRAMERATE_MIN               1
#define CVI_VENC_NO_INPUT                -10
#define CVI_VENC_INPUT_ERR               -11
#define SEC_TO_MS                        1000
#define USERDATA_MAX_DEFAULT             1024
#define USERDATA_MAX_LIMIT               65536
#define DEFAULT_NO_INPUTDATA_TIMEOUT_SEC (5)
#define BYPASS_SB_MODE                   (0)

// below should align to cv183x_vcodec.h
#define VPU_MISCDEV_NAME           "/dev/cvi-vcodec"
#define VCODEC_VENC_SHARE_MEM_SIZE (0x30000) // 192k
#define VCODEC_VDEC_SHARE_MEM_SIZE (0x8000)  // 32k
#define VCODEC_SHARE_MEM_SIZE \
    (VCODEC_VENC_SHARE_MEM_SIZE + VCODEC_VDEC_SHARE_MEM_SIZE)

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CHECK_MAX(NAME, VAL, MAX, ERRC)                                            \
    do {                                                                           \
        CVI_S32 Ret;                                                               \
        Ret = (((VAL) <= (MAX)) ? CVI_SUCCESS : (ERRC));                           \
        if (Ret != CVI_SUCCESS)                                                    \
        {                                                                          \
            printf("%s = %d,error, %s = %d\n", __func__, __LINE__, (NAME), (VAL)); \
            return Ret;                                                            \
        }                                                                          \
    } while (0)
#define CHECK_MIN_MAX(NAME, VAL, MIN, MAX, ERRC)                                   \
    do {                                                                           \
        CVI_S32 Ret;                                                               \
        Ret = (((VAL) >= (MIN) && (VAL) <= (MAX)) ? CVI_SUCCESS : (ERRC));         \
        if (Ret != CVI_SUCCESS)                                                    \
        {                                                                          \
            printf("%s = %d,error, %s = %d\n", __func__, __LINE__, (NAME), (VAL)); \
            return Ret;                                                            \
        }                                                                          \
    } while (0)
#define CHECK_COMMON_RC_PARAM(RC)                                                   \
    do {                                                                            \
        CHECK_MAX("u32RowQpDelta", pstRcParam->u32RowQpDelta,                       \
                  CVI_H26X_ROW_QP_DELTA_MAX, CVI_ERR_VENC_RC_PARAM);                \
        if (pVencAttr->enType != PT_H265 || pstRcParam->s32FirstFrameStartQp != 63) \
        {                                                                           \
            CHECK_MIN_MAX("s32FirstFrameStartQp", pstRcParam->s32FirstFrameStartQp, \
                          0, 51, CVI_ERR_VENC_RC_PARAM);                            \
        }                                                                           \
        CHECK_MAX("u32ThrdLv", pstRcParam->u32ThrdLv, CVI_H26X_THRDLV_MAX,          \
                  CVI_ERR_VENC_RC_PARAM);                                           \
        CHECK_MIN_MAX("s32BgDeltaQp", pstRcParam->s32BgDeltaQp,                     \
                      CVI_H26X_BG_DELTA_QP_MIN, CVI_H26X_BG_DELTA_QP_MAX,           \
                      CVI_ERR_VENC_RC_PARAM);                                       \
        CHECK_MIN_MAX("u32MaxIprop", (RC)->u32MaxIprop, CVI_H26X_MAX_I_PROP_MIN,    \
                      CVI_H26X_MAX_I_PROP_MAX, CVI_ERR_VENC_RC_PARAM);              \
        CHECK_MIN_MAX("u32MinIprop", (RC)->u32MinIprop, CVI_H26X_MIN_I_PROP_MIN,    \
                      CVI_H26X_MAX_I_PROP_MAX, CVI_ERR_VENC_RC_PARAM);              \
        CHECK_MIN_MAX("u32MaxIQp", (int)(RC)->u32MaxIQp, 0, 51,                     \
                      CVI_ERR_VENC_RC_PARAM);                                       \
        CHECK_MIN_MAX("u32MinIQp", (int)(RC)->u32MinIQp, 0, 51,                     \
                      CVI_ERR_VENC_RC_PARAM);                                       \
        CHECK_MIN_MAX("u32MaxQp", (int)(RC)->u32MaxQp, 0, 51,                       \
                      CVI_ERR_VENC_RC_PARAM);                                       \
        CHECK_MIN_MAX("u32MinQp", (int)(RC)->u32MinQp, 0, 51,                       \
                      CVI_ERR_VENC_RC_PARAM);                                       \
    } while (0)

#define SET_DEFAULT_RC_PARAM(RC)                         \
    do {                                                 \
        (RC)->u32MaxIprop = CVI_H26X_MAX_I_PROP_DEFAULT; \
        (RC)->u32MinIprop = CVI_H26X_MIN_I_PROP_DEFAULT; \
        (RC)->u32MaxIQp   = DEF_264_MAXIQP;              \
        (RC)->u32MinIQp   = DEF_264_MINIQP;              \
        (RC)->u32MaxQp    = DEF_264_MAXQP;               \
        (RC)->u32MinQp    = DEF_264_MINQP;               \
    } while (0)

#define SET_COMMON_RC_PARAM(DEST, SRC)                            \
    do {                                                          \
        (DEST)->u32MaxIprop         = (SRC)->u32MaxIprop;         \
        (DEST)->u32MinIprop         = (SRC)->u32MinIprop;         \
        (DEST)->u32MaxIQp           = (SRC)->u32MaxIQp;           \
        (DEST)->u32MinIQp           = (SRC)->u32MinIQp;           \
        (DEST)->u32MaxQp            = (SRC)->u32MaxQp;            \
        (DEST)->u32MinQp            = (SRC)->u32MinQp;            \
        (DEST)->s32MaxReEncodeTimes = (SRC)->s32MaxReEncodeTimes; \
    } while (0)

#define IF_WANNA_DISABLE_BIND_MODE() \
    ((pVbCtx->currBindMode == CVI_TRUE) && (pVbCtx->enable_bind_mode == CVI_FALSE))
#define IF_WANNA_ENABLE_BIND_MODE() \
    ((pVbCtx->currBindMode == CVI_FALSE) && (pVbCtx->enable_bind_mode == CVI_TRUE))

venc_dbg      vencDbg;
venc_context *handle;
SEM_T         tSbmWaitQueue[VENC_MAX_CHN_NUM];
CVI_U32       frm_done_event[VENC_MAX_CHN_NUM];
CVI_U32       err_frm_skip[VENC_MAX_CHN_NUM];

static void *vpu_comm_shared_mem;
static void *venc_proc_shared_mem;
// static CVI_S32 vcodec_fd = -1;
static CVI_U32         venc_shared_mem_usr_cnt;
struct cvi_venc_vb_ctx venc_vb_ctx[VENC_MAX_CHN_NUM] = {0};

extern int32_t vb_done_handler(MMF_CHN_S chn, enum CHN_TYPE_E chn_type,
                               VB_BLK blk);

static CVI_S32 cviInitChnCtx(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr);
static CVI_S32 cviCheckRcModeAttr(venc_chn_context *pChnHandle);
static CVI_S32 cviCheckGopAttr(venc_chn_context *pChnHandle);
static CVI_S32 cviInitFrc(venc_chn_context *pChnHandle, CVI_U32 u32SrcFrameRate,
                          CVI_FR32 fr32DstFrameRate);
// static CVI_S32 cviInitFrcFloat(venc_chn_context *pChnHandle,
// 		CVI_U32 u32SrcFrameRate, CVI_FR32 fr32DstFrameRate);
static CVI_S32  cviSetChnDefault(venc_chn_context *pChnHandle);
static CVI_S32  cviSetDefaultRcParam(venc_chn_context *pChnHandle);
static CVI_VOID cviInitVfps(venc_chn_context *pChnHandle);
static CVI_S32  cviSetRcParamToDrv(venc_chn_context *pChnHandle);
static CVI_S32  cviGetEnv(CVI_CHAR *env, CVI_CHAR *fmt, CVI_VOID *param);
static CVI_S32  cviCheckFps(venc_chn_context         *pChnHandle,
                            const VIDEO_FRAME_INFO_S *pstFrame);
static CVI_VOID cviSetFps(venc_chn_context *pChnHandle, CVI_S32 currFps);
static CVI_S32  cviSetChnAttr(venc_chn_context *pChnHandle);
static CVI_S32  cviCheckFrc(venc_chn_context *pChnHandle);
static CVI_S32  cviUpdateFrcSrc(venc_chn_context *pChnHandle);
static CVI_S32  cviUpdateFrcDst(venc_chn_context *pChnHandle);
static CVI_VOID cviCheckFrcOverflow(venc_chn_context *pChnHandle);
static CVI_S32  cviCheckLeftStreamFrames(venc_chn_context *pChnHandle);
static CVI_S32  cviProcessResult(venc_chn_context *pChnHandle,
                                 VENC_STREAM_S    *pstStream);
static CVI_VOID cviUpdateChnStatus(venc_chn_context *pChnHandle);
// static CVI_S32 CVI_COMM_VENC_GetDataType(PAYLOAD_TYPE_E enType, VENC_PACK_S
// *ppack);
static CVI_VOID cviChangeMask(CVI_S32 frameIdx);
static CVI_S32  cviSetVencChnAttrToProc(VENC_CHN                  VeChn,
                                        const VIDEO_FRAME_INFO_S *pstFrame);
static CVI_S32  cviSetVencPerfAttrToProc(venc_chn_context *pChnHandle);
static CVI_VOID cviGetDebugConfigFromEncProc(void);

// static CVI_S32 cviVenc_sem_timedwait_Millsecs(sem_t *sem, long msecs);
static void    cviVenc_sem_init(ptread_sem_s *sem);
static void    cviVenc_sem_destroy(ptread_sem_s *sem);
static void    cviVenc_sem_post(ptread_sem_s *sem);
static CVI_S32 cviVenc_sem_wait(ptread_sem_s *sem, long msecs);

static CVI_S32 cviCheckRcParam(venc_chn_context      *pChnHandle,
                               const VENC_RC_PARAM_S *pstRcParam);
static CVI_S32 cviSetCuPredToDrv(venc_chn_context *pChnHandle);

static CVI_S32 cviSetSbSetting(VENC_SB_Setting *pstSbSetting);
static CVI_S32 cviStartSb(VENC_SB_Setting *pstSbSetting);
// static CVI_S32 cviUpdateSbWptr(VENC_SB_Setting *pstSbSetting);
static CVI_S32 cviSbSkipOneFrm(VENC_CHN VeChn, VENC_SB_Setting *pstSbSetting);
static CVI_S32 cviResetSb(VENC_SB_Setting *pstSbSetting);
static CVI_S32 cviSetSBMEnable(venc_chn_context *pChnHandle, CVI_BOOL bSBMEn);

static CVI_S32 _cviVencAllocVbBuf(VENC_CHN VeChn);
// static CVI_S32 _cviVencUpdateVbConf(venc_chn_context *pChnHandle,
// 				cviVbVencBufConfig *pVbVencCfg,
// 				int VbSetFrameBufSize,
// 				int VbSetFrameBufCnt);
static CVI_S32 _cviVencInitCtxHandle(void);
static CVI_S32 _cviCheckFrameRate(venc_chn_context *pChnHandle,
                                  CVI_U32          *pu32SrcFrameRate,
                                  CVI_FR32         *pfr32DstFrameRate,
                                  CVI_BOOL          bVariFpsEn);
static CVI_S32 _cviVencRegVbBuf(VENC_CHN VeChn);
static CVI_S32 _cviVencSetInPixelFormat(VENC_CHN VeChn,
                                        CVI_BOOL bCbCrInterleave,
                                        CVI_BOOL bNV21);
static CVI_S32 _cviCheckH264VuiParam(const VENC_H264_VUI_S *pstH264Vui);
static CVI_S32 _cviCheckH265VuiParam(const VENC_H265_VUI_S *pstH265Vui);
static CVI_S32 _cviSetH264Vui(venc_chn_context *pChnHandle,
                              VENC_H264_VUI_S  *pstH264Vui);
static CVI_S32 _cviSetH265Vui(venc_chn_context *pChnHandle,
                              VENC_H265_VUI_S  *pstH265Vui);

CVI_S32 cviInitFrcSoftFloat(venc_chn_context *pChnHandle,
                            CVI_U32 u32SrcFrameRate, CVI_FR32 fr32DstFrameRate);
CVI_S32 cvi_VENC_CB_SkipFrame(CVI_S32 VpssGrp, CVI_S32 VpssChn,
                              CVI_U32 srcImgHeight, CVI_S32 s32MilliSec);

static inline CVI_U32 _cviGetNumPacks(PAYLOAD_TYPE_E enType)
{
    return (enType == PT_JPEG || enType == PT_MJPEG) ? 1 : MAX_NUM_PACKS;
}

void *venc_get_share_mem(void)
{
    return venc_proc_shared_mem;
}

static CVI_U64 get_current_time(CVI_VOID)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000; // in ms
}

static void *vpu_get_shm(void)
{
    if (vpu_comm_shared_mem == NULL)
    {
        vpu_comm_shared_mem = calloc(1, VCODEC_SHARE_MEM_SIZE);
        if (vpu_comm_shared_mem == NULL)
        {
            CVI_VENC_ERR("vcodec mmap fail!\n");
            return NULL;
        }
    }
    return vpu_comm_shared_mem;
}

static void vpu_release_shm(void)
{
    venc_shared_mem_usr_cnt--;

    if (venc_shared_mem_usr_cnt == 0 && vpu_comm_shared_mem != NULL)
    {
#if 0
		pthread_cancel(handle->threadDataInput);
		pthread_join(handle->threadDataInput, NULL);
#endif
        free(vpu_comm_shared_mem);
        venc_proc_shared_mem = NULL;
        vpu_comm_shared_mem  = NULL;
    }
}

CVI_VOID *venc_monitor_data_input_task(CVI_VOID *data)
{
    CVI_U32           u32ChnId                       = 0;
    CVI_U64           u64ChnCurPTS[VENC_MAX_CHN_NUM] = {-1};
    venc_proc_info_t *pProcInfoShareMem =
        (venc_proc_info_t *)venc_proc_shared_mem;
    venc_proc_info_t *pProcSharedMem;
    CVI_U32           u32Timeout = DEFAULT_NO_INPUTDATA_TIMEOUT_SEC;

    (void)(data);

    while (venc_shared_mem_usr_cnt)
    {
        venc_dbg *pDbg = &vencDbg;

        u32Timeout = pDbg->noDataTimeout ? pDbg->noDataTimeout
                                         : DEFAULT_NO_INPUTDATA_TIMEOUT_SEC;
        for (u32ChnId = 0; u32ChnId < VENC_MAX_CHN_NUM; u32ChnId++)
        {
            pProcSharedMem = (pProcInfoShareMem + u32ChnId);

            if (pProcSharedMem->u8ChnUsed)
            {
                if (pProcInfoShareMem->stFrame.u64PTS == u64ChnCurPTS[u32ChnId])
                {
                    pProcSharedMem->stFPS.u32InFPS = pProcSharedMem->stFPS.u32OutFPS = 0;
                    CVI_VENC_WARN("reset procfs info for venc channel_%d\n", u32ChnId);
                }
                u64ChnCurPTS[u32ChnId] = pProcInfoShareMem->stFrame.u64PTS;
            }
        }
        sleep(u32Timeout);
    }
    return (CVI_VOID *)CVI_SUCCESS;
}

static CVI_S32 _VENC_MMAP(void)
{
    CVI_S32 ret = CVI_SUCCESS;

    venc_shared_mem_usr_cnt++;

    if (vpu_comm_shared_mem != NULL)
    {
        CVI_VENC_INFO("already done mmap\n");
        return CVI_SUCCESS;
    }

    vpu_comm_shared_mem = vpu_get_shm();
    if (vpu_comm_shared_mem == NULL)
    {
        CVI_VENC_ERR("base_get_shm failed!\n");
        return CVI_ERR_VENC_NULL_PTR;
    }
    venc_proc_shared_mem = vpu_comm_shared_mem + sizeof(proc_debug_config_t) + sizeof(CVI_VENC_PARAM_MOD_S);
#if 0
	if (vcodec_fd != -1 && venc_proc_shared_mem != NULL) {
		struct sched_param param;
		pthread_attr_t attr;

		param.sched_priority = 40;
		pthread_attr_init(&attr);
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		pthread_attr_setschedparam(&attr, &param);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		pthread_create(&handle->threadDataInput, &attr, venc_monitor_data_input_task, NULL);
	}
#endif
    return ret;
}

static CVI_S32 _VENC_UNMAP(void)
{
    if (vpu_comm_shared_mem == NULL)
    {
        CVI_VENC_INFO("No need to unmap\n");
        return CVI_SUCCESS;
    }

    vpu_release_shm();
    return CVI_SUCCESS;
}

static void init_eventfd(venc_chn_context *pChnHandle)
{
    venc_chn_vars *pChnVars = pChnHandle->pChnVars;

    pChnVars->bind_event_fd = -1;
}

// static int open_eventfd(venc_chn_context *pChnHandle)
// {
// 	venc_chn_vars *pChnVars = pChnHandle->pChnVars;
// 	return pChnVars->bind_event_fd;
// }

// static void close_eventfd(venc_chn_context *pChnHandle)
// {
// 	UNUSED(pChnHandle);
// }

static void op_eventfd(venc_chn_context *pChnHandle, CVI_BOOL bRead,
                       CVI_BOOL bWrite)
{
    UNUSED(pChnHandle);
    UNUSED(bRead);
    UNUSED(bWrite);
}

#define CVI_VENC_MAILBOX_MAX_PARAM_COUNT 8
typedef struct _st_cvi_venc_mailbox
{
    uint64_t total_size;
    uint64_t params_cnt;
    uint64_t params_offset[CVI_VENC_MAILBOX_MAX_PARAM_COUNT];
} ST_CVI_VENC_MAILBOX;

//#define USE_RTOS_CMD_QUEUE
#ifdef USE_RTOS_CMD_QUEUE
QueueHandle_t xQueueCmdqu = NULL;
CVI_U8        cmdquId     = 0;
void          venc_set_cmdQuInfo(QueueHandle_t cmdquHandle, CVI_U8 ipid)
{
    xQueueCmdqu = cmdquHandle;
    cmdquId     = ipid;
}

void venc_release_vb(VENC_CHN VeChn)
{
    cmdqu_t              rtos_cmdq;
    venc_chn_context    *pChnHandle;
    ST_CVI_VENC_MAILBOX *gpstVencMailBox;
    CVI_S32              sParam1Size   = sizeof(VENC_CHN);
    CVI_S32              sParam2Size   = sizeof(struct vb_s);
    CVI_U64              sTotalSize    = 0;
    CVI_S32              sOriginOffset = sizeof(ST_CVI_VENC_MAILBOX);

    if (CVI_SUCCESS != check_chn_handle(VeChn))
    {
        CVI_VENC_ERR("check_chn_handle\n");
        return;
    }

    if ((xQueueCmdqu == NULL) || (cmdquId == 0)) return;

    pChnHandle      = handle->chn_handle[VeChn];
    gpstVencMailBox = (ST_CVI_VENC_MAILBOX *)pChnHandle->releaseVbAddr;
    sTotalSize      = sOriginOffset + sParam1Size + sParam2Size;

    gpstVencMailBox->total_size       = sTotalSize;
    gpstVencMailBox->params_cnt       = 2;
    gpstVencMailBox->params_offset[0] = sOriginOffset;
    gpstVencMailBox->params_offset[1] = sOriginOffset + sParam1Size;

    memcpy((uint8_t *)gpstVencMailBox + gpstVencMailBox->params_offset[0], &VeChn,
           sParam1Size);
    memcpy((uint8_t *)gpstVencMailBox + gpstVencMailBox->params_offset[1],
           &pChnHandle->vb, sizeof(struct vb_s));

    rtos_cmdq.ip_id     = cmdquId;
    rtos_cmdq.cmd_id    = 0;
    rtos_cmdq.param_ptr = pChnHandle->releaseVbAddr;

    flush_dcache_range(pChnHandle->releaseVbAddr, sTotalSize);
    xQueueSend(xQueueCmdqu, &rtos_cmdq, 0U);
    pChnHandle->useVbFlag = 0;
}
#endif

CVI_VOID *venc_event_handler(CVI_VOID *data)
{
    venc_chn_context *pChnHandle = (venc_chn_context *)data;
    VENC_CHN          VencChn;
    venc_chn_vars    *pChnVars = NULL;
    VENC_CHN_STATUS_S stStat;
    venc_enc_ctx     *pEncCtx;

    CVI_S32            s32MilliSec = -1;
    MMF_CHN_S          chn         = {.enModId = CVI_ID_VENC, .s32DevId = 0, .s32ChnId = 0};
    VB_BLK             blk;
    struct vb_s       *vb;
    VIDEO_FRAME_INFO_S stVFrame;
    VIDEO_FRAME_S     *pstVFrame = &stVFrame.stVFrame;
    int                i;
    int                vi_cnt = 0;
#ifdef USE_POSIX
    // CVI_CHAR cThreadName[16]; to do?
#endif
    // struct timespec ts;
    // CVI_S32 ret;
    CVI_S32                 s32SetFrameMilliSec = 20000;
    CVI_S32                 s32Ret              = CVI_SUCCESS;
    struct cvi_venc_vb_ctx *pVbCtx;
    // CVI_BOOL isChnEnable = CVI_TRUE;

    memset(&stVFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

    pChnVars = pChnHandle->pChnVars;
    pEncCtx  = &pChnHandle->encCtx;
    pVbCtx   = pChnHandle->pVbCtx;
    VencChn  = pChnHandle->VeChn;

    chn.s32ChnId = VencChn;

    memset(&stVFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

    cviVenc_sem_wait(&pChnVars->sem_send, -1);
    pstVFrame->u32Width  = pChnHandle->pChnAttr->stVencAttr.u32PicWidth;
    pstVFrame->u32Height = pChnHandle->pChnAttr->stVencAttr.u32PicHeight;
    CVI_VENC_FLOW("[%d] VENC_CHN_STATE_START_ENC\n", VencChn);
#if 0
#ifdef USE_POSIX
	snprintf(cThreadName, sizeof(cThreadName), "venc_handler-%d", VencChn);
	prctl(PR_SET_NAME, cThreadName);
#endif
#endif

    while (pChnHandle->bChnEnable)
    {
        CVI_VENC_BIND("[%d]\n", VencChn);
        if (IF_WANNA_DISABLE_BIND_MODE() || (pChnVars->s32RecvPicNum > 0 && vi_cnt >= pChnVars->s32RecvPicNum))
        {
            pChnHandle->bChnEnable = CVI_FALSE;
            CVI_VENC_SYNC("end\n");
            break;
        }

        // wait job
        if (cviVenc_sem_wait(&pVbCtx->sem_vb, 1000) != CVI_SUCCESS)
        {
            continue;
        }

        if (pChnHandle->bChnEnable == CVI_FALSE)
        {
            CVI_VENC_SYNC("end2\n");
            break;
        }

        if (pVbCtx->pause)
        {
            CVI_VENC_TRACE("pause and skip update.\n");
            continue;
        }

        // get venc input buf.
        blk = base_mod_jobs_waitq_pop(chn, CHN_TYPE_IN);
        if (blk == VB_INVALID_HANDLE)
        {
            CVI_VENC_TRACE("No more vb for dequeue.\n");
            continue;
        }
        vb           = (struct vb_s *)blk;
        vb->mod_ids &= ~BIT(CVI_ID_VENC);

        for (i = 0; i < 3; i++)
        {
            pstVFrame->u64PhyAddr[i] = vb->buf.phy_addr[i];
            pstVFrame->u32Stride[i]  = vb->buf.stride[i];
            CVI_VENC_TRACE("phy: 0x%lx, stride: 0x%x\n", pstVFrame->u64PhyAddr[i],
                           pstVFrame->u32Stride[i]);
        }
        pstVFrame->u64PTS        = vb->buf.u64PTS;
        pstVFrame->enPixelFormat = vb->buf.enPixelFormat;
        pstVFrame->pPrivateData  = vb;
        // for crop
        pstVFrame->s16OffsetBottom = vb->buf.s16OffsetBottom;
        pstVFrame->s16OffsetLeft   = vb->buf.s16OffsetLeft;
        pstVFrame->s16OffsetRight  = vb->buf.s16OffsetRight;
        pstVFrame->s16OffsetTop    = vb->buf.s16OffsetTop;

        s32SetFrameMilliSec = -1;
        CVI_VENC_BIND("[%d]\n", VencChn);
        s32Ret = CVI_VENC_SendFrame(VencChn, &stVFrame, s32SetFrameMilliSec);
        if (s32Ret == CVI_ERR_VENC_INIT || s32Ret == CVI_ERR_VENC_FRC_NO_ENC || s32Ret == CVI_ERR_VENC_BUSY)
        {
            CVI_VENC_FRC("no encode,continue\n");
            vb_done_handler(chn, CHN_TYPE_IN, blk);
            continue;
        }
        else if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_VENC_SendFrame, VencChn = %d, s32Ret = %d\n", VencChn,
                         s32Ret);
            goto VENC_EVENT_HANDLER_ERR;
        }
        s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_VENC_QueryStatus, VencChn = %d, s32Ret = %d\n", VencChn,
                         s32Ret);
            goto VENC_EVENT_HANDLER_ERR;
        }

        if (!stStat.u32CurPacks)
        {
            CVI_VENC_ERR("u32CurPacks = 0\n");
            s32Ret = CVI_ERR_VENC_EMPTY_PACK;
            goto VENC_EVENT_HANDLER_ERR;
        }
        pChnVars->stStream.pstPack =
            (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
        if (pChnVars->stStream.pstPack == NULL)
        {
            CVI_VENC_ERR("malloc memory failed!\n");
            s32Ret = CVI_ERR_VENC_NOMEM;
            goto VENC_EVENT_HANDLER_ERR;
        }

        pChnVars->stStream.u32PackCount = stStat.u32CurPacks;

        s32Ret                            = pEncCtx->base.getStream(pEncCtx, &pChnVars->stStream, s32MilliSec);
        pChnVars->s32BindModeGetStreamRet = s32Ret;
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("getStream, VencChn = %d, s32Ret = %d\n", VencChn, s32Ret);
            cviVenc_sem_post(&pChnVars->sem_send);
            goto VENC_EVENT_HANDLER_ERR;
        }

        vb_done_handler(chn, CHN_TYPE_IN, blk);

        s32Ret = cviProcessResult(pChnHandle, &pChnVars->stStream);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("(chn %d) cviProcessResult fail\n", VencChn);
            cviVenc_sem_post(&pChnVars->sem_send);
            goto VENC_EVENT_HANDLER_ERR_2;
        }

        cviVenc_sem_post(&pChnVars->sem_send);
        op_eventfd(pChnHandle, 0, 1);
        CVI_VENC_TRACE("[%d]wait release\n", vi_cnt);
        cviVenc_sem_wait(&pChnVars->sem_release, -1);

        s32Ret = pEncCtx->base.releaseStream(pEncCtx, &pChnVars->stStream);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("[Vench %d]releaseStream ,s32Ret = %d\n", VencChn, s32Ret);
            goto VENC_EVENT_HANDLER_ERR_2;
        }

        free(pChnVars->stStream.pstPack);
        pChnVars->stStream.pstPack = NULL;

        vi_cnt++;

        sched_yield();
    }
    CVI_VENC_SYNC("end\n");

    return (void *)CVI_SUCCESS;

VENC_EVENT_HANDLER_ERR:
    vb_done_handler(chn, CHN_TYPE_IN, blk);

VENC_EVENT_HANDLER_ERR_2:
    if (pChnVars->stStream.pstPack)
    {
        free(pChnVars->stStream.pstPack);
        pChnVars->stStream.pstPack = NULL;
    }

    CVI_VENC_SYNC("end\n");

    return (void *)CVI_SUCCESS;
}

#if 0
static void venc_sbm_send_frame_thread(CVI_VOID *data)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	venc_chn_context *pChnHandle = (venc_chn_context *)data;

	while (1) {
		//wait_event(tSbmWaitQueue[pChnHandle->VeChn], (frm_done_event[pChnHandle->VeChn] != 0));
		SEM_WAIT(tSbmWaitQueue[pChnHandle->VeChn]);

		frm_done_event[pChnHandle->VeChn] = 0;

		if (err_frm_skip[pChnHandle->VeChn] == 1) {
			err_frm_skip[pChnHandle->VeChn] = 0;
			if (pChnHandle->sbm_state == VENC_SBM_STATE_FRM_RUN)
				pChnHandle->sbm_state = VENC_SBM_STATE_IDLE;
			continue;
		}

		if (pChnHandle->sbm_state == VENC_SBM_STATE_CHN_CLOSED)
			break;

		if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265) {
			pChnHandle->stSbSetting.codec = 0x1;
		} else if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H264) {
			pChnHandle->stSbSetting.codec = 0x2;
		} else if (pChnHandle->pChnAttr->stVencAttr.enType == PT_JPEG ||
					pChnHandle->pChnAttr->stVencAttr.enType == PT_MJPEG){
			pChnHandle->stSbSetting.codec = 0x4;
		} else {
			break;
		}

		s32Ret = cviSetSbSetting(&pChnHandle->stSbSetting);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("cviSetSbSetting fail, s32Ret=%d\n", s32Ret);
			break;
		}

		s32Ret = cviStartSb(&pChnHandle->stSbSetting);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("cviStartSb fail, s32Ret=%d\n", s32Ret);
			break;
		}

		if (pChnHandle->sbm_state == VENC_SBM_STATE_IDLE)
			pChnHandle->sbm_state = VENC_SBM_STATE_FRM_RUN;

		s32Ret = CVI_VENC_SendFrame(pChnHandle->VeChn, &pChnHandle->stVideoFrameInfo, 20000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_SendFrame(%d) fail, s32Ret=%d\n", pChnHandle->VeChn, s32Ret);
			break;
		}
	}

	// vTaskDelay(NULL);
	return ;
}
#endif

CVI_S32 check_chn_handle(VENC_CHN VeChn)
{
    if (handle == NULL)
    {
        CVI_VENC_ERR("Call VENC Destroy before Create, failed\n");
        return CVI_ERR_VENC_UNEXIST;
    }

    if (handle->chn_handle[VeChn] == NULL)
    {
        CVI_VENC_ERR("VENC Chn #%d haven't created !\n", VeChn);
        return CVI_ERR_VENC_INVALID_CHNID;
    }

    return CVI_SUCCESS;
}

static void _cviVencInitModParam(CVI_VENC_PARAM_MOD_S *pModParam)
{
    if (!pModParam) return;

    memset(pModParam, 0, sizeof(*pModParam));
    pModParam->stJpegeModParam.JpegMarkerOrder[0] = JPEGE_MARKER_SOI;
    pModParam->stJpegeModParam.JpegMarkerOrder[1] = JPEGE_MARKER_FRAME_INDEX;
    pModParam->stJpegeModParam.JpegMarkerOrder[2] = JPEGE_MARKER_USER_DATA;
    pModParam->stJpegeModParam.JpegMarkerOrder[3] = JPEGE_MARKER_DRI_OPT;
    pModParam->stJpegeModParam.JpegMarkerOrder[4] = JPEGE_MARKER_DQT;
    pModParam->stJpegeModParam.JpegMarkerOrder[5] = JPEGE_MARKER_DHT;
    pModParam->stJpegeModParam.JpegMarkerOrder[6] = JPEGE_MARKER_SOF0;
    pModParam->stJpegeModParam.JpegMarkerOrder[7] = JPEGE_MARKER_BUTT;
    pModParam->stH264eModParam.u32UserDataMaxLen  = USERDATA_MAX_DEFAULT;
    pModParam->stH265eModParam.u32UserDataMaxLen  = USERDATA_MAX_DEFAULT;
}

static CVI_S32 _cviVencInitCtxHandle(void)
{
    venc_context *pVencHandle;

    if (handle == NULL)
    {
        handle = calloc(1, sizeof(venc_context));
        if (handle == NULL)
        {
            CVI_VENC_ERR("venc_context create failure\n");
            return CVI_ERR_VENC_NOMEM;
        }

        pVencHandle = (venc_context *)handle;
        _cviVencInitModParam(&pVencHandle->ModParam);
    }

    return CVI_SUCCESS;
}

static CVI_S32 _cviCheckFrameRate(venc_chn_context *pChnHandle,
                                  CVI_U32          *pu32SrcFrameRate,
                                  CVI_FR32         *pfr32DstFrameRate,
                                  CVI_BOOL          bVariFpsEn)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    int   frameRateDiv, frameRateRes;
    float fSrcFrmrate, fDstFrmrate;

    CVI_VENC_CFG("dst-fps = %d\n", *pfr32DstFrameRate);
    CVI_VENC_CFG("src-fps = %d\n", *pu32SrcFrameRate);
    CVI_VENC_CFG("bVariFpsEn = %d\n", bVariFpsEn);
    CHECK_MIN_MAX("bVariFpsEn", (int)bVariFpsEn, CVI_VARI_FPS_EN_MIN,
                  CVI_VARI_FPS_EN_MAX, CVI_ERR_VENC_ILLEGAL_PARAM);

    if (*pfr32DstFrameRate == 0)
    {
        CVI_VENC_WARN("set fr32DstFrameRate to %d\n", DEST_FRAMERATE_DEF);
        *pfr32DstFrameRate = DEST_FRAMERATE_DEF;
    }

    if (*pu32SrcFrameRate == 0)
    {
        CVI_VENC_WARN("set u32SrcFrameRate to %d\n", *pfr32DstFrameRate);
        *pu32SrcFrameRate = *pfr32DstFrameRate;
    }

    frameRateDiv = (*pfr32DstFrameRate >> 16);
    frameRateRes = *pfr32DstFrameRate & 0xFFFF;

    if (frameRateDiv == 0)
        fDstFrmrate = frameRateRes;
    else
        fDstFrmrate = (float)frameRateRes / (float)frameRateDiv;

    frameRateDiv = (*pu32SrcFrameRate >> 16);
    frameRateRes = *pu32SrcFrameRate & 0xFFFF;

    if (frameRateDiv == 0)
        fSrcFrmrate = frameRateRes;
    else
        fSrcFrmrate = (float)frameRateRes / (float)frameRateDiv;

    if (fDstFrmrate > DEST_FRAMERATE_MAX || fDstFrmrate < DEST_FRAMERATE_MIN)
    {
        CVI_VENC_ERR("fr32DstFrameRate = 0x%X, not support\n", *pfr32DstFrameRate);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    if (fSrcFrmrate > SRC_FRAMERATE_MAX || fSrcFrmrate < SRC_FRAMERATE_MIN)
    {
        CVI_VENC_ERR("u32SrcFrameRate = %d, not support\n", *pu32SrcFrameRate);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    if (fDstFrmrate > fSrcFrmrate)
    {
        *pfr32DstFrameRate = *pu32SrcFrameRate;
        CVI_VENC_WARN("fDstFrmrate > fSrcFrmrate\n");
        CVI_VENC_WARN("=> fr32DstFrameRate = u32SrcFrameRate\n");
    }

    s32Ret = cviInitFrc(pChnHandle, *pu32SrcFrameRate, *pfr32DstFrameRate);

    return s32Ret;
}

static CVI_S32 _cviVencRegVbBuf(VENC_CHN VeChn)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    venc_context *pVencHandle = handle;
    venc_enc_ctx *pEncCtx     = NULL;

    venc_chn_context *pChnHandle = NULL;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    if (pVencHandle == NULL)
    {
        CVI_VENC_ERR("p_venctx_handle NULL (Channel not create yet..)\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;

    if (pEncCtx->base.ioctl)
    {
        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_REG_VB_BUFFER, NULL);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_H26X_OP_REG_VB_BUFFER, %d\n", s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}

static CVI_S32 _cviVencSetInPixelFormat(VENC_CHN VeChn,
                                        CVI_BOOL bCbCrInterleave,
                                        CVI_BOOL bNV21)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_enc_ctx     *pEncCtx    = NULL;
    venc_chn_context *pChnHandle = NULL;
    cviInPixelFormat  inPixelFormat;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;

    inPixelFormat.bCbCrInterleave = bCbCrInterleave;
    inPixelFormat.bNV21           = bNV21;

    s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_IN_PIXEL_FORMAT,
                                 (CVI_VOID *)&inPixelFormat);

    return s32Ret;
}

CVI_S32 CVI_VENC_GetFd(VENC_CHN VeChn)
{
    UNUSED(VeChn);

    return -1;
}

CVI_S32 CVI_VENC_CloseFd(VENC_CHN VeChn)
{
    UNUSED(VeChn);

    return 0;
}

/**
 * @brief Create One VENC Channel.
 * @param[in] VeChn VENC Channel Number.
 * @param[in] pstAttr pointer to VENC_CHN_ATTR_S.
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_createchn(VeChn, pstAttr);
    }
#endif
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    venc_enc_ctx     *pEncCtx    = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = cviInitChnCtx(VeChn, pstAttr);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("init\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;

    s32Ret = pEncCtx->base.open(handle, pChnHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("venc_init_encoder\n");
        return s32Ret;
    }

    pChnHandle->bSbSkipFrm    = false;
    pChnHandle->sbm_state     = VENC_SBM_STATE_IDLE;
    handle->chn_status[VeChn] = VENC_CHN_STATE_INIT;

    CVI_VENC_TRACE("\n");
    return s32Ret;
}

static CVI_S32 cviInitChnCtx(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
    CVI_S32           s32Ret = CVI_SUCCESS;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    pthread_mutexattr_t ma;

    s32Ret = _cviVencInitCtxHandle();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("venc_context\n");
        s32Ret = CVI_ERR_VENC_NOMEM;
        goto ERR_CVI_INIT_CHN_CTX;
    }

    s32Ret = _VENC_MMAP();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("_VENC_MMAP failed.\n");
        return s32Ret;
    }

    cviGetDebugConfigFromEncProc();

    handle->chn_handle[VeChn] = calloc(1, sizeof(venc_chn_context));
    if (handle->chn_handle[VeChn] == NULL)
    {
        CVI_VENC_ERR("Allocate chn_handle memory failed !\n");
        s32Ret = CVI_ERR_VENC_NOMEM;
        goto ERR_CVI_INIT_CHN_CTX;
    }

    pChnHandle        = handle->chn_handle[VeChn];
    pChnHandle->VeChn = VeChn;

    pthread_mutex_init(&pChnHandle->chnMutex, 0);

    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
    //	pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&pChnHandle->chnShmMutex, &ma);

    pChnHandle->pChnAttr = malloc(sizeof(VENC_CHN_ATTR_S));
    if (pChnHandle->pChnAttr == NULL)
    {
        CVI_VENC_ERR("Allocate pChnAttr memory failed !\n");
        s32Ret = CVI_ERR_VENC_NOMEM;
        goto ERR_CVI_INIT_CHN_CTX_1;
    }

    memcpy(pChnHandle->pChnAttr, pstAttr, sizeof(VENC_CHN_ATTR_S));

    pChnHandle->pChnVars = calloc(1, sizeof(venc_chn_vars));
    if (pChnHandle->pChnVars == NULL)
    {
        CVI_VENC_ERR("Allocate pChnVars memory failed !\n");
        s32Ret = CVI_ERR_VENC_NOMEM;
        goto ERR_CVI_INIT_CHN_CTX_2;
    }
    pChnHandle->pChnVars->chnState = VENC_CHN_STATE_INIT;

    pChnHandle->pVbCtx = &venc_vb_ctx[VeChn];

    pEncCtx = &pChnHandle->encCtx;
    if (venc_create_enc_ctx(pEncCtx, pChnHandle) < 0)
    {
        CVI_VENC_ERR("venc_create_enc_ctx\n");
        s32Ret = CVI_ERR_VENC_NOMEM;
        goto ERR_CVI_INIT_CHN_CTX_3;
    }

    s32Ret = pEncCtx->base.init();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("base init,s32Ret %#x\n", s32Ret);
        goto ERR_CVI_INIT_CHN_CTX_3;
    }

    s32Ret = cviCheckRcModeAttr(pChnHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviCheckRcModeAttr\n");
        goto ERR_CVI_INIT_CHN_CTX_3;
    }

    s32Ret = cviCheckGopAttr(pChnHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviCheckGopAttr\n");
        goto ERR_CVI_INIT_CHN_CTX_3;
    }

    s32Ret = cviSetChnDefault(pChnHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviSetChnDefault,s32Ret %#x\n", s32Ret);
        goto ERR_CVI_INIT_CHN_CTX_3;
    }

    return s32Ret;

ERR_CVI_INIT_CHN_CTX_3:
    if (pChnHandle->pChnVars)
    {
        free(pChnHandle->pChnVars);
        pChnHandle->pChnVars = NULL;
    }
ERR_CVI_INIT_CHN_CTX_2:
    if (pChnHandle->pChnAttr)
    {
        free(pChnHandle->pChnAttr);
        pChnHandle->pChnAttr = NULL;
    }
ERR_CVI_INIT_CHN_CTX_1:
    if (handle->chn_handle[VeChn])
    {
        free(handle->chn_handle[VeChn]);
        handle->chn_handle[VeChn] = NULL;
    }
ERR_CVI_INIT_CHN_CTX:
    return s32Ret;
}

static CVI_S32 cviCheckRcModeAttr(venc_chn_context *pChnHandle)
{
    VENC_CHN_ATTR_S *pChnAttr = pChnHandle->pChnAttr;
    CVI_S32          s32Ret   = CVI_SUCCESS;

    if (pChnAttr->stVencAttr.enType == PT_MJPEG)
    {
        if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGCBR)
        {
            VENC_MJPEG_CBR_S *pstMjpegeCbr = &pChnAttr->stRcAttr.stMjpegCbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstMjpegeCbr->u32SrcFrameRate,
                                        &pstMjpegeCbr->fr32DstFrameRate,
                                        pstMjpegeCbr->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGFIXQP)
        {
            VENC_MJPEG_FIXQP_S *pstMjpegeFixQp = &pChnAttr->stRcAttr.stMjpegFixQp;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstMjpegeFixQp->u32SrcFrameRate,
                                        &pstMjpegeFixQp->fr32DstFrameRate,
                                        pstMjpegeFixQp->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_MJPEGVBR)
        {
            VENC_MJPEG_VBR_S *pstMjpegeVbr = &pChnAttr->stRcAttr.stMjpegVbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstMjpegeVbr->u32SrcFrameRate,
                                        &pstMjpegeVbr->fr32DstFrameRate,
                                        pstMjpegeVbr->bVariFpsEn);
        }
        else
        {
            s32Ret = CVI_ERR_VENC_NOT_SUPPORT;
            CVI_VENC_ERR("enRcMode = %d, not support\n", pChnAttr->stRcAttr.enRcMode);
        }
    }
    else if (pChnAttr->stVencAttr.enType == PT_H264)
    {
        if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264CBR)
        {
            VENC_H264_CBR_S *pstH264Cbr = &pChnAttr->stRcAttr.stH264Cbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH264Cbr->u32SrcFrameRate,
                                        &pstH264Cbr->fr32DstFrameRate,
                                        pstH264Cbr->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264VBR)
        {
            VENC_H264_VBR_S *pstH264Vbr = &pChnAttr->stRcAttr.stH264Vbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH264Vbr->u32SrcFrameRate,
                                        &pstH264Vbr->fr32DstFrameRate,
                                        pstH264Vbr->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264AVBR)
        {
            VENC_H264_AVBR_S *pstH264AVbr = &pChnAttr->stRcAttr.stH264AVbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH264AVbr->u32SrcFrameRate,
                                        &pstH264AVbr->fr32DstFrameRate,
                                        pstH264AVbr->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264FIXQP)
        {
            VENC_H264_FIXQP_S *pstH264FixQp = &pChnAttr->stRcAttr.stH264FixQp;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH264FixQp->u32SrcFrameRate,
                                        &pstH264FixQp->fr32DstFrameRate,
                                        pstH264FixQp->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264UBR)
        {
            VENC_H264_UBR_S *pstH264Ubr = &pChnAttr->stRcAttr.stH264Ubr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH264Ubr->u32SrcFrameRate,
                                        &pstH264Ubr->fr32DstFrameRate,
                                        pstH264Ubr->bVariFpsEn);
        }
        else
        {
            s32Ret = CVI_ERR_VENC_NOT_SUPPORT;
            CVI_VENC_ERR("enRcMode = %d, not support\n", pChnAttr->stRcAttr.enRcMode);
        }
    }
    else if (pChnAttr->stVencAttr.enType == PT_H265)
    {
        if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265CBR)
        {
            VENC_H265_CBR_S *pstH265Cbr = &pChnAttr->stRcAttr.stH265Cbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH265Cbr->u32SrcFrameRate,
                                        &pstH265Cbr->fr32DstFrameRate,
                                        pstH265Cbr->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265VBR)
        {
            VENC_H265_VBR_S *pstH265Vbr = &pChnAttr->stRcAttr.stH265Vbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH265Vbr->u32SrcFrameRate,
                                        &pstH265Vbr->fr32DstFrameRate,
                                        pstH265Vbr->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265AVBR)
        {
            VENC_H265_AVBR_S *pstH265AVbr = &pChnAttr->stRcAttr.stH265AVbr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH265AVbr->u32SrcFrameRate,
                                        &pstH265AVbr->fr32DstFrameRate,
                                        pstH265AVbr->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265FIXQP)
        {
            VENC_H265_FIXQP_S *pstH265FixQp = &pChnAttr->stRcAttr.stH265FixQp;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH265FixQp->u32SrcFrameRate,
                                        &pstH265FixQp->fr32DstFrameRate,
                                        pstH265FixQp->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QPMAP)
        {
            VENC_H265_QPMAP_S *pstH265QpMap = &pChnAttr->stRcAttr.stH265QpMap;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH265QpMap->u32SrcFrameRate,
                                        &pstH265QpMap->fr32DstFrameRate,
                                        pstH265QpMap->bVariFpsEn);
        }
        else if (pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265UBR)
        {
            VENC_H265_UBR_S *pstH265Ubr = &pChnAttr->stRcAttr.stH265Ubr;

            s32Ret = _cviCheckFrameRate(pChnHandle, &pstH265Ubr->u32SrcFrameRate,
                                        &pstH265Ubr->fr32DstFrameRate,
                                        pstH265Ubr->bVariFpsEn);
        }
        else
        {
            s32Ret = CVI_ERR_VENC_NOT_SUPPORT;
            CVI_VENC_ERR("enRcMode = %d, not support\n", pChnAttr->stRcAttr.enRcMode);
        }
    }

    return s32Ret;
}

static CVI_U32 _cviGetGop(const VENC_RC_ATTR_S *pRcAttr)
{
    switch (pRcAttr->enRcMode)
    {
    case VENC_RC_MODE_H264CBR:
        return pRcAttr->stH264Cbr.u32Gop;
    case VENC_RC_MODE_H264VBR:
        return pRcAttr->stH264Vbr.u32Gop;
    case VENC_RC_MODE_H264AVBR:
        return pRcAttr->stH264AVbr.u32Gop;
    case VENC_RC_MODE_H264QVBR:
        return pRcAttr->stH264QVbr.u32Gop;
    case VENC_RC_MODE_H264FIXQP:
        return pRcAttr->stH264FixQp.u32Gop;
    case VENC_RC_MODE_H264QPMAP:
        return pRcAttr->stH264QpMap.u32Gop;
    case VENC_RC_MODE_H264UBR:
        return pRcAttr->stH264Ubr.u32Gop;
    case VENC_RC_MODE_H265CBR:
        return pRcAttr->stH265Cbr.u32Gop;
    case VENC_RC_MODE_H265VBR:
        return pRcAttr->stH265Vbr.u32Gop;
    case VENC_RC_MODE_H265AVBR:
        return pRcAttr->stH265AVbr.u32Gop;
    case VENC_RC_MODE_H265QVBR:
        return pRcAttr->stH265QVbr.u32Gop;
    case VENC_RC_MODE_H265FIXQP:
        return pRcAttr->stH265FixQp.u32Gop;
    case VENC_RC_MODE_H265QPMAP:
        return pRcAttr->stH265QpMap.u32Gop;
    case VENC_RC_MODE_H265UBR:
        return pRcAttr->stH265Ubr.u32Gop;
    default:
        return 0;
    }
}

static CVI_S32 cviCheckGopAttr(venc_chn_context *pChnHandle)
{
    VENC_CHN_ATTR_S *pChnAttr = pChnHandle->pChnAttr;
    VENC_RC_ATTR_S  *pRcAttr  = &pChnAttr->stRcAttr;
    VENC_GOP_ATTR_S *pGopAttr = &pChnAttr->stGopAttr;
    CVI_U32          u32Gop   = _cviGetGop(pRcAttr);

    if (pChnAttr->stVencAttr.enType == PT_JPEG || pChnAttr->stVencAttr.enType == PT_MJPEG)
    {
        return CVI_SUCCESS;
    }

    if (u32Gop == 0)
    {
        CVI_VENC_ERR("enRcMode = %d, not support\n", pRcAttr->enRcMode);
        return CVI_ERR_VENC_RC_PARAM;
    }

    switch (pGopAttr->enGopMode)
    {
    case VENC_GOPMODE_NORMALP:
        CHECK_MIN_MAX("s32IPQpDelta", pGopAttr->stNormalP.s32IPQpDelta,
                      CVI_H26X_NORMALP_IP_QP_DELTA_MIN,
                      CVI_H26X_NORMALP_IP_QP_DELTA_MAX, CVI_ERR_VENC_GOP_ATTR);
        break;

    case VENC_GOPMODE_SMARTP:
        CHECK_MIN_MAX("u32BgInterval", pGopAttr->stSmartP.u32BgInterval, u32Gop,
                      CVI_H26X_SMARTP_BG_INTERVAL_MAX, CVI_ERR_VENC_GOP_ATTR);
        CHECK_MIN_MAX("s32BgQpDelta", pGopAttr->stSmartP.s32BgQpDelta,
                      CVI_H26X_SMARTP_BG_QP_DELTA_MIN,
                      CVI_H26X_SMARTP_BG_QP_DELTA_MAX, CVI_ERR_VENC_GOP_ATTR);
        CHECK_MIN_MAX("s32ViQpDelta", pGopAttr->stSmartP.s32ViQpDelta,
                      CVI_H26X_SMARTP_VI_QP_DELTA_MIN,
                      CVI_H26X_SMARTP_VI_QP_DELTA_MAX, CVI_ERR_VENC_GOP_ATTR);
        if ((pGopAttr->stSmartP.u32BgInterval % u32Gop) != 0)
        {
            CVI_VENC_ERR("u32BgInterval %d, not a multiple of u32Gop %d\n",
                         pGopAttr->stAdvSmartP.u32BgInterval, u32Gop);
            return CVI_ERR_VENC_GOP_ATTR;
        }
        break;

    default:
        CVI_VENC_ERR("enGopMode = %d, not support\n", pGopAttr->enGopMode);
        return CVI_ERR_VENC_GOP_ATTR;
    }

    return CVI_SUCCESS;
}

#define FRC_TIME_SCALE 0xFFF0
#if SOFT_FLOAT
#define FLOAT_VAL_FRC_TIME_SCALE (0x477ff000)
#else
#define FLOAT_VAL_FRC_TIME_SCALE (FRC_TIME_SCALE)
#endif
#define FRC_TIME_OVERFLOW_OFFSET 0x40000000

static CVI_S32 cviInitFrc(venc_chn_context *pChnHandle, CVI_U32 u32SrcFrameRate,
                          CVI_FR32 fr32DstFrameRate)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
#if 0 // ndef USE_KERNEL_MODE
	CVI_S32 cviRcEn;

	cviRcEn = cviGetEnv("cviRcEn", "%d", NULL);
	cviRcEn = (cviRcEn >= 0) ? 1 : 0;

	if (cviRcEn == 0)
		s32Ret = cviInitFrcFloat(pChnHandle, u32SrcFrameRate, fr32DstFrameRate);
	else
#endif
    s32Ret = cviInitFrcSoftFloat(pChnHandle, u32SrcFrameRate, fr32DstFrameRate);

    return s32Ret;
}

#if 0 // ndef USE_KERNEL_MODE
static CVI_S32 cviInitFrcFloat(venc_chn_context *pChnHandle,
		CVI_U32 u32SrcFrameRate, CVI_FR32 fr32DstFrameRate)
{
	venc_chn_vars *pChnVars = pChnHandle->pChnVars;
	venc_frc *pvf = &pChnVars->frc;
	CVI_S32 dstFrDenom;
	CVI_S32 dstFrfract;
	CVI_DOUBLE srcFrameRate = u32SrcFrameRate;
	CVI_DOUBLE dstFrameRate = 0.0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int srcFrChecker, dstFrChecker;

	dstFrDenom = (fr32DstFrameRate >> 16) & 0xFFFF;
	dstFrfract = (fr32DstFrameRate & 0xFFFF);

	if (dstFrDenom == 0)
		dstFrameRate = dstFrfract;
	else
		dstFrameRate = (double)(dstFrfract) / dstFrDenom;

	dstFrDenom = (u32SrcFrameRate >> 16) & 0xFFFF;
	dstFrfract = (u32SrcFrameRate & 0xFFFF);

	if (dstFrDenom == 0)
		srcFrameRate = dstFrfract;
	else
		srcFrameRate = (double)(dstFrfract) / dstFrDenom;

	CVI_VENC_FRC("srcFrameRate = %.2lf, dstFrameRate = %.2lf\n",
			srcFrameRate, dstFrameRate);

	srcFrChecker = (int)(srcFrameRate * 10000);
	dstFrChecker = (int)(dstFrameRate * 10000);

	CVI_VENC_FRC("srcFrChecker = %d, dstFrChecker = %d\n", srcFrChecker, dstFrChecker);

	if (srcFrChecker > dstFrChecker) {
		if (srcFrameRate == 0 || dstFrameRate == 0) {
			CVI_VENC_ERR(
				"Dst frame rate(%.2lf), Src frame rate(%.2lf), not supported\n",
				dstFrameRate, srcFrameRate);
			pvf->bFrcEnable = CVI_FALSE;
			s32Ret = CVI_ERR_VENC_NOT_SUPPORT;
			return s32Ret;
		}
		pvf->bFrcEnable = CVI_TRUE;
		pvf->srcFrameDur = FRC_TIME_SCALE / srcFrameRate;
		pvf->dstFrameDur = FRC_TIME_SCALE / dstFrameRate;
		pvf->srcTs = pvf->srcFrameDur;
		pvf->dstTs = pvf->dstFrameDur;
		CVI_VENC_FRC("srcFrameDur = %d, dstFrameDur = %d\n",
			     pvf->srcFrameDur, pvf->dstFrameDur);
	} else if (srcFrChecker == dstFrChecker) {
		pvf->bFrcEnable = CVI_FALSE;
	} else {
		pvf->bFrcEnable = CVI_FALSE;
		CVI_VENC_ERR(
			"Dst frame rate(%.2lf) > Src frame rate(%.2lf), not supported\n",
			dstFrameRate, srcFrameRate);
		s32Ret = CVI_ERR_VENC_NOT_SUPPORT;
		return s32Ret;
	}

	CVI_VENC_FRC("bFrcEnable = %d\n", pvf->bFrcEnable);

	return s32Ret;
}
#endif

CVI_S32 cviInitFrcSoftFloat(venc_chn_context *pChnHandle,
                            CVI_U32           u32SrcFrameRate,
                            CVI_FR32          fr32DstFrameRate)
{
    venc_chn_vars *pChnVars = pChnHandle->pChnVars;
    venc_frc      *pvf      = &pChnVars->frc;
    CVI_S32        s32Ret   = CVI_SUCCESS;
    CVI_S32        dstFrDenom;
    CVI_S32        dstFrfract;
    float32        srcFrameRate;
    float32        dstFrameRate;
    int            srcFrChecker, dstFrChecker;

    dstFrDenom = (fr32DstFrameRate >> 16) & 0xFFFF;
    dstFrfract = (fr32DstFrameRate & 0xFFFF);

    dstFrameRate = INT_TO_CVI_FLOAT(dstFrfract);
    if (dstFrDenom != 0)
        dstFrameRate = CVI_FLOAT_DIV(dstFrameRate, INT_TO_CVI_FLOAT(dstFrDenom));

    dstFrDenom = (u32SrcFrameRate >> 16) & 0xFFFF;
    dstFrfract = (u32SrcFrameRate & 0xFFFF);

    srcFrameRate = INT_TO_CVI_FLOAT(dstFrfract);
    if (dstFrDenom != 0)
        srcFrameRate = CVI_FLOAT_DIV(srcFrameRate, INT_TO_CVI_FLOAT(dstFrDenom));

    if (vencDbg.currMask & CVI_VENC_MASK_FRC)
    {
        CVI_RC_FLOAT("srcFrameRate = %f, dstFrameRate = %f\n",
                     getFloat(srcFrameRate), getFloat(dstFrameRate));
    }

    srcFrChecker = CVI_FLOAT_TO_INT(CVI_FLOAT_MUL(srcFrameRate, FLOAT_VAL_10000));
    dstFrChecker = CVI_FLOAT_TO_INT(CVI_FLOAT_MUL(dstFrameRate, FLOAT_VAL_10000));

    CVI_VENC_FRC("srcFrChecker = %d, dstFrChecker = %d\n", srcFrChecker,
                 dstFrChecker);

    if (srcFrChecker > dstFrChecker)
    {
        if (CVI_FLOAT_EQ(srcFrameRate, FLOAT_VAL_0) || CVI_FLOAT_EQ(dstFrameRate, FLOAT_VAL_0))
        {
            CVI_VENC_ERR("Dst frame rate(%d), Src frame rate(%d), not supported\n",
                         dstFrameRate, srcFrameRate);
            pvf->bFrcEnable = CVI_FALSE;
            s32Ret          = CVI_ERR_VENC_NOT_SUPPORT;
            return s32Ret;
        }
        pvf->bFrcEnable = CVI_TRUE;
        pvf->srcFrameDur =
            CVI_FLOAT_TO_INT(CVI_FLOAT_DIV(FLOAT_VAL_FRC_TIME_SCALE, srcFrameRate));
        pvf->dstFrameDur =
            CVI_FLOAT_TO_INT(CVI_FLOAT_DIV(FLOAT_VAL_FRC_TIME_SCALE, dstFrameRate));
        pvf->srcTs = pvf->srcFrameDur;
        pvf->dstTs = pvf->dstFrameDur;
        CVI_VENC_FRC("srcFrameDur = %d, dstFrameDur = %d\n", pvf->srcFrameDur,
                     pvf->dstFrameDur);
    }
    else if (srcFrChecker == dstFrChecker)
    {
        pvf->bFrcEnable = CVI_FALSE;
    }
    else
    {
        pvf->bFrcEnable = CVI_FALSE;
        CVI_VENC_ERR("Dst frame rate(%d) > Src frame rate(%d), not supported\n",
                     dstFrameRate, srcFrameRate);
        s32Ret = CVI_ERR_VENC_NOT_SUPPORT;
        return s32Ret;
    }

    CVI_VENC_FRC("bFrcEnable = %d\n", pvf->bFrcEnable);

    return s32Ret;
}

static CVI_S32 cviSetChnDefault(venc_chn_context *pChnHandle)
{
    venc_chn_vars          *pChnVars  = pChnHandle->pChnVars;
    struct cvi_venc_vb_ctx *pVbCtx    = pChnHandle->pVbCtx;
    VENC_CHN_ATTR_S        *pChnAttr  = pChnHandle->pChnAttr;
    VENC_ATTR_S            *pVencAttr = &pChnAttr->stVencAttr;
    VENC_JPEG_PARAM_S      *pvjp      = &pChnVars->stJpegParam;
    VENC_CU_PREDICTION_S   *pcup      = &pChnVars->cuPrediction;
    VENC_SUPERFRAME_CFG_S  *pcsf      = &pChnVars->stSuperFrmParam;
    VENC_FRAME_PARAM_S     *pfp       = &pChnVars->frameParam;
    CVI_S32                 s32Ret    = CVI_SUCCESS;
    MMF_CHN_S               chn       = {
                            .enModId  = CVI_ID_VENC,
                            .s32DevId = 0,
                            .s32ChnId = pChnHandle->VeChn};

    if (pVencAttr->enType == PT_H264)
    {
        pChnHandle->h264Vui.stVuiAspectRatio.aspect_ratio_idc =
            CVI_H26X_ASPECT_RATIO_IDC_DEFAULT;
        pChnHandle->h264Vui.stVuiAspectRatio.sar_width = CVI_H26X_SAR_WIDTH_DEFAULT;
        pChnHandle->h264Vui.stVuiAspectRatio.sar_height =
            CVI_H26X_SAR_HEIGHT_DEFAULT;
        pChnHandle->h264Vui.stVuiTimeInfo.num_units_in_tick =
            CVI_H26X_NUM_UNITS_IN_TICK_DEFAULT;
        pChnHandle->h264Vui.stVuiTimeInfo.time_scale = CVI_H26X_TIME_SCALE_DEFAULT;
        pChnHandle->h264Vui.stVuiVideoSignal.video_format =
            CVI_H26X_VIDEO_FORMAT_DEFAULT;
        pChnHandle->h264Vui.stVuiVideoSignal.colour_primaries =
            CVI_H26X_COLOUR_PRIMARIES_DEFAULT;
        pChnHandle->h264Vui.stVuiVideoSignal.transfer_characteristics =
            CVI_H26X_TRANSFER_CHARACTERISTICS_DEFAULT;
        pChnHandle->h264Vui.stVuiVideoSignal.matrix_coefficients =
            CVI_H26X_MATRIX_COEFFICIENTS_DEFAULT;
    }
    else if (pVencAttr->enType == PT_H265)
    {
        pChnHandle->h265Vui.stVuiAspectRatio.aspect_ratio_idc =
            CVI_H26X_ASPECT_RATIO_IDC_DEFAULT;
        pChnHandle->h265Vui.stVuiAspectRatio.sar_width = CVI_H26X_SAR_WIDTH_DEFAULT;
        pChnHandle->h265Vui.stVuiAspectRatio.sar_height =
            CVI_H26X_SAR_HEIGHT_DEFAULT;
        pChnHandle->h265Vui.stVuiTimeInfo.num_units_in_tick =
            CVI_H26X_NUM_UNITS_IN_TICK_DEFAULT;
        pChnHandle->h265Vui.stVuiTimeInfo.time_scale = CVI_H26X_TIME_SCALE_DEFAULT;
        pChnHandle->h265Vui.stVuiTimeInfo.num_ticks_poc_diff_one_minus1 =
            CVI_H265_NUM_TICKS_POC_DIFF_ONE_MINUS1_DEFAULT;
        pChnHandle->h265Vui.stVuiVideoSignal.video_format =
            CVI_H26X_VIDEO_FORMAT_DEFAULT;
        pChnHandle->h265Vui.stVuiVideoSignal.colour_primaries =
            CVI_H26X_COLOUR_PRIMARIES_DEFAULT;
        pChnHandle->h265Vui.stVuiVideoSignal.transfer_characteristics =
            CVI_H26X_TRANSFER_CHARACTERISTICS_DEFAULT;
        pChnHandle->h265Vui.stVuiVideoSignal.matrix_coefficients =
            CVI_H26X_MATRIX_COEFFICIENTS_DEFAULT;
    }

    if (pVencAttr->enType == PT_H264)
    {
        if (pVencAttr->u32Profile == H264E_PROFILE_BASELINE)
        {
            pChnHandle->h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CAVLC;
            pChnHandle->h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CAVLC;
        }
        else
        {
            pChnHandle->h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CABAC;
            pChnHandle->h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CABAC;
        }
    }

    pvjp->u32Qfactor          = Q_TABLE_DEFAULT;
    pcup->u32IntraCost        = CVI_H26X_INTRACOST_DEFAULT;
    pcsf->enSuperFrmMode      = CVI_H26X_SUPER_FRM_MODE_DEFAULT;
    pcsf->u32SuperIFrmBitsThr = CVI_H26X_SUPER_I_BITS_THR_DEFAULT;
    pcsf->u32SuperPFrmBitsThr = CVI_H26X_SUPER_P_BITS_THR_DEFAULT;
    pfp->u32FrameQp           = CVI_H26X_FRAME_QP_DEFAULT;
    pfp->u32FrameBits         = CVI_H26X_FRAME_BITS_DEFAULT;

    s32Ret = cviSetDefaultRcParam(pChnHandle);
    if (s32Ret < 0)
    {
        CVI_VENC_ERR("cviSetDefaultRcParam\n");
        return s32Ret;
    }

    pChnVars->vbpool.hPicInfoVbPool = VB_INVALID_POOLID;

    base_mod_jobs_init(chn, CHN_TYPE_IN, 1, 1, 0);
    cviVenc_sem_init(&pVbCtx->sem_vb);
    cviVenc_sem_init(&pChnVars->sem_send);
    cviVenc_sem_init(&pChnVars->sem_release);

    init_eventfd(pChnHandle);

    cviInitVfps(pChnHandle);

    return s32Ret;
}

static CVI_S32 cviSetDefaultRcParam(venc_chn_context *pChnHandle)
{
    CVI_S32          s32Ret = CVI_SUCCESS;
    VENC_CHN_ATTR_S *pChnAttr;
    VENC_ATTR_S     *pVencAttr;
    VENC_RC_ATTR_S  *prcatt;
    VENC_RC_PARAM_S *prcparam;

    pChnAttr  = pChnHandle->pChnAttr;
    pVencAttr = &pChnAttr->stVencAttr;
    prcatt    = &pChnAttr->stRcAttr;
    prcparam  = &pChnHandle->rcParam;

    prcparam->u32RowQpDelta = CVI_H26X_ROW_QP_DELTA_DEFAULT;
    prcparam->s32FirstFrameStartQp =
        ((pVencAttr->enType == PT_H265) ? 63 : DEF_IQP);
    prcparam->s32InitialDelay = CVI_INITIAL_DELAY_DEFAULT;
    prcparam->u32ThrdLv       = CVI_H26X_THRDLV_DEFAULT;
    prcparam->bBgEnhanceEn    = CVI_H26X_BG_ENHANCE_EN_DEFAULT;
    prcparam->s32BgDeltaQp    = CVI_H26X_BG_DELTA_QP_DEFAULT;

    if (pVencAttr->enType == PT_H264)
    {
        if (prcatt->enRcMode == VENC_RC_MODE_H264CBR)
        {
            VENC_PARAM_H264_CBR_S *pstParamH264Cbr = &prcparam->stParamH264Cbr;

            SET_DEFAULT_RC_PARAM(pstParamH264Cbr);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264VBR)
        {
            VENC_PARAM_H264_VBR_S *pstParamH264Vbr = &prcparam->stParamH264Vbr;

            SET_DEFAULT_RC_PARAM(pstParamH264Vbr);
            pstParamH264Vbr->s32ChangePos = CVI_H26X_CHANGE_POS_DEFAULT;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264AVBR)
        {
            VENC_PARAM_H264_AVBR_S *pstParamH264AVbr = &prcparam->stParamH264AVbr;

            SET_DEFAULT_RC_PARAM(pstParamH264AVbr);
            pstParamH264AVbr->s32ChangePos       = CVI_H26X_CHANGE_POS_DEFAULT;
            pstParamH264AVbr->s32MinStillPercent = CVI_H26X_MIN_STILL_PERCENT_DEFAULT;
            pstParamH264AVbr->u32MaxStillQP      = CVI_H26X_MAX_STILL_QP_DEFAULT;
            pstParamH264AVbr->u32MotionSensitivity =
                CVI_H26X_MOTION_SENSITIVITY_DEFAULT;
            pstParamH264AVbr->s32AvbrFrmLostOpen =
                CVI_H26X_AVBR_FRM_LOST_OPEN_DEFAULT;
            pstParamH264AVbr->s32AvbrFrmGap = CVI_H26X_AVBR_FRM_GAP_DEFAULT;
            pstParamH264AVbr->s32AvbrPureStillThr =
                CVI_H26X_AVBR_PURE_STILL_THR_DEFAULT;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264UBR)
        {
            VENC_PARAM_H264_UBR_S *pstParamH264Ubr = &prcparam->stParamH264Ubr;

            SET_DEFAULT_RC_PARAM(pstParamH264Ubr);
        }
    }
    else if (pVencAttr->enType == PT_H265)
    {
        if (prcatt->enRcMode == VENC_RC_MODE_H265CBR)
        {
            VENC_PARAM_H265_CBR_S *pstParamH265Cbr = &prcparam->stParamH265Cbr;

            SET_DEFAULT_RC_PARAM(pstParamH265Cbr);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265VBR)
        {
            VENC_PARAM_H265_VBR_S *pstParamH265Vbr = &prcparam->stParamH265Vbr;

            SET_DEFAULT_RC_PARAM(pstParamH265Vbr);
            pstParamH265Vbr->s32ChangePos = CVI_H26X_CHANGE_POS_DEFAULT;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265AVBR)
        {
            VENC_PARAM_H265_AVBR_S *pstParamH265AVbr = &prcparam->stParamH265AVbr;

            SET_DEFAULT_RC_PARAM(pstParamH265AVbr);
            pstParamH265AVbr->s32ChangePos       = CVI_H26X_CHANGE_POS_DEFAULT;
            pstParamH265AVbr->s32MinStillPercent = CVI_H26X_MIN_STILL_PERCENT_DEFAULT;
            pstParamH265AVbr->u32MaxStillQP      = CVI_H26X_MAX_STILL_QP_DEFAULT;
            pstParamH265AVbr->u32MotionSensitivity =
                CVI_H26X_MOTION_SENSITIVITY_DEFAULT;
            pstParamH265AVbr->s32AvbrFrmLostOpen =
                CVI_H26X_AVBR_FRM_LOST_OPEN_DEFAULT;
            pstParamH265AVbr->s32AvbrFrmGap = CVI_H26X_AVBR_FRM_GAP_DEFAULT;
            pstParamH265AVbr->s32AvbrPureStillThr =
                CVI_H26X_AVBR_PURE_STILL_THR_DEFAULT;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265QPMAP)
        {
            VENC_PARAM_H265_CBR_S *pstParamH265Cbr = &prcparam->stParamH265Cbr;

            // When using QP Map, we use CBR as basic setting
            SET_DEFAULT_RC_PARAM(pstParamH265Cbr);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265UBR)
        {
            VENC_PARAM_H265_UBR_S *pstParamH265Ubr = &prcparam->stParamH265Ubr;

            SET_DEFAULT_RC_PARAM(pstParamH265Ubr);
        }
    } else if (pVencAttr->enType == PT_MJPEG) {
        if (prcatt->enRcMode == VENC_RC_MODE_MJPEGCBR) {
            VENC_PARAM_MJPEG_CBR_S *pstParamMjpegCbr = &prcparam->stParamMjpegCbr;

            pstParamMjpegCbr->u32MaxQfactor = Q_TABLE_MAX;
            pstParamMjpegCbr->u32MinQfactor = Q_TABLE_MIN;
        } else if (prcatt->enRcMode == VENC_RC_MODE_MJPEGVBR) {
            VENC_PARAM_MJPEG_VBR_S *pstParamMjpegVbr = &prcparam->stParamMjpegVbr;

            pstParamMjpegVbr->u32MaxQfactor = Q_TABLE_MAX;
            pstParamMjpegVbr->u32MinQfactor = Q_TABLE_MIN;
        }
    }

    return s32Ret;
}

static CVI_VOID cviInitVfps(venc_chn_context *pChnHandle)
{
    venc_chn_vars *pChnVars = pChnHandle->pChnVars;
    venc_vfps     *pVfps    = &pChnVars->vfps;

    memset(pVfps, 0, sizeof(venc_vfps));
    pVfps->u64StatTime = CVI_DEF_VFPFS_STAT_TIME * 1000 * 1000;
}

static CVI_S32 cviSetRcParamToDrv(venc_chn_context *pChnHandle)
{
    CVI_S32          s32Ret    = CVI_SUCCESS;
    VENC_CHN_ATTR_S *pChnAttr  = pChnHandle->pChnAttr;
    venc_enc_ctx    *pEncCtx   = &pChnHandle->encCtx;
    VENC_RC_PARAM_S *prcparam  = &pChnHandle->rcParam;
    VENC_ATTR_S     *pVencAttr = &pChnAttr->stVencAttr;
    VENC_RC_ATTR_S  *prcatt    = &pChnAttr->stRcAttr;

    if (pEncCtx->base.ioctl)
    {
        cviRcParam               rcp, *prcp   = &rcp;
        struct CVIJpegEncRcParam jrcp, *pjrcp = &jrcp;

        prcp->u32RowQpDelta   = prcparam->u32RowQpDelta;
        prcp->firstFrmstartQp = prcparam->s32FirstFrameStartQp;
        prcp->u32ThrdLv       = prcparam->u32ThrdLv;
        prcp->s32InitialDelay = prcparam->s32InitialDelay;

        CVI_VENC_CFG(
            "RowQpDelta = %d, firstFrmstartQp = %d, ThrdLv = %d, InitialDelay = "
            "%d\n",
            prcp->u32RowQpDelta, prcp->firstFrmstartQp, prcp->u32ThrdLv,
            prcp->s32InitialDelay);
        prcp->bBgEnhanceEn = prcparam->bBgEnhanceEn;
        prcp->s32BgDeltaQp = prcparam->s32BgDeltaQp;
        CVI_VENC_CFG("BgEnhanceEn = %d, BgDeltaQp = %d\n", prcp->bBgEnhanceEn,
                     prcp->s32BgDeltaQp);

        prcp->s32ChangePos = 0;

        if (pVencAttr->enType == PT_H264)
        {
            if (prcatt->enRcMode == VENC_RC_MODE_H264CBR)
            {
                VENC_PARAM_H264_CBR_S *pstParamH264Cbr = &prcparam->stParamH264Cbr;

                SET_COMMON_RC_PARAM(prcp, pstParamH264Cbr);
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_H264VBR)
            {
                VENC_PARAM_H264_VBR_S *pstParamH264Vbr = &prcparam->stParamH264Vbr;

                SET_COMMON_RC_PARAM(prcp, pstParamH264Vbr);
                prcp->s32ChangePos = pstParamH264Vbr->s32ChangePos;
                CVI_VENC_CFG("s32ChangePos = %d\n", prcp->s32ChangePos);
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_H264AVBR)
            {
                VENC_PARAM_H264_AVBR_S *pstParamH264AVbr = &prcparam->stParamH264AVbr;

                SET_COMMON_RC_PARAM(prcp, pstParamH264AVbr);
                prcp->s32ChangePos         = pstParamH264AVbr->s32ChangePos;
                prcp->s32MinStillPercent   = pstParamH264AVbr->s32MinStillPercent;
                prcp->u32MaxStillQP        = pstParamH264AVbr->u32MaxStillQP;
                prcp->u32MotionSensitivity = pstParamH264AVbr->u32MotionSensitivity;
                prcp->s32AvbrFrmLostOpen   = pstParamH264AVbr->s32AvbrFrmLostOpen;
                prcp->s32AvbrFrmGap        = pstParamH264AVbr->s32AvbrFrmGap;
                prcp->s32AvbrPureStillThr  = pstParamH264AVbr->s32AvbrPureStillThr;

                CVI_VENC_CFG("s32ChangePos = %d\n", prcp->s32ChangePos);
                CVI_VENC_CFG("Still Percent = %d, QP = %d, MotionSensitivity = %d\n",
                             prcp->s32MinStillPercent, prcp->u32MaxStillQP,
                             prcp->u32MotionSensitivity);
                CVI_VENC_CFG("FrmLostOpen = %d, FrmGap = %d, PureStillThr = %d\n",
                             prcp->s32AvbrFrmLostOpen, prcp->s32AvbrFrmGap,
                             prcp->s32AvbrPureStillThr);
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_H264UBR)
            {
                VENC_PARAM_H264_UBR_S *pstParamH264Ubr = &prcparam->stParamH264Ubr;

                SET_COMMON_RC_PARAM(prcp, pstParamH264Ubr);
            }
        }
        else if (pVencAttr->enType == PT_H265)
        {
            if (prcatt->enRcMode == VENC_RC_MODE_H265CBR)
            {
                VENC_PARAM_H265_CBR_S *pstParamH265Cbr = &prcparam->stParamH265Cbr;

                SET_COMMON_RC_PARAM(prcp, pstParamH265Cbr);
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_H265VBR)
            {
                VENC_PARAM_H265_VBR_S *pstParamH265Vbr = &prcparam->stParamH265Vbr;

                SET_COMMON_RC_PARAM(prcp, pstParamH265Vbr);
                prcp->s32ChangePos = pstParamH265Vbr->s32ChangePos;
                CVI_VENC_CFG("s32ChangePos = %d\n", prcp->s32ChangePos);
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_H265AVBR)
            {
                VENC_PARAM_H265_AVBR_S *pstParamH265AVbr = &prcparam->stParamH265AVbr;

                SET_COMMON_RC_PARAM(prcp, pstParamH265AVbr);
                prcp->s32ChangePos         = pstParamH265AVbr->s32ChangePos;
                prcp->s32MinStillPercent   = pstParamH265AVbr->s32MinStillPercent;
                prcp->u32MaxStillQP        = pstParamH265AVbr->u32MaxStillQP;
                prcp->u32MotionSensitivity = pstParamH265AVbr->u32MotionSensitivity;
                prcp->s32AvbrFrmLostOpen   = pstParamH265AVbr->s32AvbrFrmLostOpen;
                prcp->s32AvbrFrmGap        = pstParamH265AVbr->s32AvbrFrmGap;
                prcp->s32AvbrPureStillThr  = pstParamH265AVbr->s32AvbrPureStillThr;

                CVI_VENC_CFG("s32ChangePos = %d\n", prcp->s32ChangePos);
                CVI_VENC_CFG("Still Percent = %d, QP = %d, MotionSensitivity = %d\n",
                             prcp->s32MinStillPercent, prcp->u32MaxStillQP,
                             prcp->u32MotionSensitivity);
                CVI_VENC_CFG("FrmLostOpen = %d, FrmGap = %d, PureStillThr = %d\n",
                             prcp->s32AvbrFrmLostOpen, prcp->s32AvbrFrmGap,
                             prcp->s32AvbrPureStillThr);
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_H265QPMAP)
            {
                VENC_PARAM_H265_CBR_S *pstParamH265Cbr = &prcparam->stParamH265Cbr;

                // When using QP Map, we use CBR as basic setting
                SET_COMMON_RC_PARAM(prcp, pstParamH265Cbr);
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_H265UBR)
            {
                VENC_PARAM_H265_UBR_S *pstParamH265Ubr = &prcparam->stParamH265Ubr;

                SET_COMMON_RC_PARAM(prcp, pstParamH265Ubr);
            }
        }
        else if (pVencAttr->enType == PT_MJPEG)
        {
            if (prcatt->enRcMode == VENC_RC_MODE_MJPEGCBR)
            {
                VENC_PARAM_MJPEG_CBR_S *pstParamMjpegCbr = &prcparam->stParamMjpegCbr;
                pjrcp->chgPos                            = 100;
                pjrcp->maxQfactor                        = pstParamMjpegCbr->u32MaxQfactor;
                pjrcp->minQfactor                        = pstParamMjpegCbr->u32MinQfactor;
            }
            else if (prcatt->enRcMode == VENC_RC_MODE_MJPEGVBR)
            {
                VENC_PARAM_MJPEG_VBR_S *pstParamMjpegVbr = &prcparam->stParamMjpegVbr;
                pjrcp->chgPos                            = pstParamMjpegVbr->s32ChangePos;
                pjrcp->maxQfactor                        = pstParamMjpegVbr->u32MaxQfactor;
                pjrcp->minQfactor                        = pstParamMjpegVbr->u32MinQfactor;
            }
        }

        CVI_VENC_CFG(
            "u32MinIQp = %d, u32MaxIQp = %d, u32MinQp = %d, u32MaxQp = %d\n",
            prcp->u32MinIQp, prcp->u32MaxIQp, prcp->u32MinQp, prcp->u32MaxQp);
        if (pVencAttr->enType == PT_MJPEG)
            s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_SET_RC_PARAM,
                                         (CVI_VOID *)pjrcp);
        else
            s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_RC_PARAM,
                                         (CVI_VOID *)prcp);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_H26X_OP_SET_RC_PARAM, %d\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    return s32Ret;
}

CVI_VOID cviGetMask(void)
{
    venc_dbg *pDbg = &vencDbg;

    CVI_VENC_TRACE("\n");

    memset(pDbg, 0, sizeof(venc_dbg));

    pDbg->dbgMask = cviGetEnv("venc_mask", "0x%x", NULL);
    if (pDbg->dbgMask == CVI_VENC_NO_INPUT || pDbg->dbgMask == CVI_VENC_INPUT_ERR)
        pDbg->dbgMask = CVI_VENC_MASK_ERR;
    else
        pDbg->dbgMask |= CVI_VENC_MASK_ERR;

    pDbg->currMask = pDbg->dbgMask;
    pDbg->startFn  = cviGetEnv("venc_sfn", "%d", NULL);
    pDbg->endFn    = cviGetEnv("venc_efn", "%d", NULL);
    cviGetEnv("venc_dir", "%s", pDbg->dbgDir);

    cviChangeMask(0);
}

static CVI_S32 cviGetEnv(CVI_CHAR *env, CVI_CHAR *fmt, CVI_VOID *param)
{
    CVI_CHAR *debugEnv;
    CVI_S32   val = CVI_VENC_NO_INPUT;

    debugEnv = getenv(env);
    if (debugEnv)
    {
        if (strcmp(fmt, "%s") == 0)
        {
            strcpy(param, debugEnv);
        }
        else
        {
            if (sscanf(debugEnv, fmt, &val) != 1) return CVI_VENC_INPUT_ERR;
            CVI_VENC_TRACE("%s = 0x%X\n", env, val);
        }
    }

    return val;
}

CVI_S32 _cvi_h26x_trans_chn_attr(VENC_CHN_ATTR_S *pInChnAttr,
                                 cviVidChnAttr   *pOutAttr)
{
    CVI_S32         s32Ret    = CVI_FAILURE;
    VENC_RC_ATTR_S *pstRcAttr = NULL;

    if ((pInChnAttr == NULL) || (pOutAttr == NULL)) return s32Ret;

    pstRcAttr = &pInChnAttr->stRcAttr;

    if (pInChnAttr->stVencAttr.enType == PT_H264)
    {
        if (pstRcAttr->enRcMode == VENC_RC_MODE_H264CBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH264Cbr.u32BitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH264Cbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH264Cbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_H264VBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH264Vbr.u32MaxBitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH264Vbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH264Vbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_H264AVBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH264AVbr.u32MaxBitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH264AVbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH264AVbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_H264UBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH264Ubr.u32BitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH264Ubr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH264Ubr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
    }
    else if (pInChnAttr->stVencAttr.enType == PT_H265)
    {
        if (pstRcAttr->enRcMode == VENC_RC_MODE_H265CBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH265Cbr.u32BitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH265Cbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH265Cbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_H265VBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH265Vbr.u32MaxBitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH265Vbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH265Vbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_H265AVBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH265AVbr.u32MaxBitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH265AVbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH265AVbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_H265UBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stH265Ubr.u32BitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stH265Ubr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stH265Ubr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
    }

    return s32Ret;
}

CVI_S32 _cvi_jpg_trans_chn_attr(VENC_CHN_ATTR_S *pInChnAttr,
                                cviJpegChnAttr  *pOutAttr)
{
    CVI_S32         s32Ret      = CVI_FAILURE;
    VENC_RC_ATTR_S *pstRcAttr   = NULL;
    VENC_ATTR_S    *pstVencAttr = NULL;

    if ((pInChnAttr == NULL) || (pOutAttr == NULL)) return s32Ret;

    pstRcAttr   = &(pInChnAttr->stRcAttr);
    pstVencAttr = &(pInChnAttr->stVencAttr);

    if (pInChnAttr->stVencAttr.enType == PT_MJPEG)
    {
        pOutAttr->picWidth  = pstVencAttr->u32PicWidth;
        pOutAttr->picHeight = pstVencAttr->u32PicHeight;
        if (pstRcAttr->enRcMode == VENC_RC_MODE_MJPEGCBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stMjpegCbr.u32BitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stMjpegCbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stMjpegCbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_MJPEGVBR)
        {
            pOutAttr->u32BitRate       = pstRcAttr->stMjpegVbr.u32MaxBitRate;
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stMjpegVbr.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stMjpegVbr.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
        else if (pstRcAttr->enRcMode == VENC_RC_MODE_MJPEGFIXQP)
        {
            pOutAttr->u32SrcFrameRate  = pstRcAttr->stMjpegFixQp.u32SrcFrameRate;
            pOutAttr->fr32DstFrameRate = pstRcAttr->stMjpegFixQp.fr32DstFrameRate;
            s32Ret                     = CVI_SUCCESS;
        }
    }
    else if (pInChnAttr->stVencAttr.enType == PT_JPEG)
    {
        pOutAttr->picWidth  = pstVencAttr->u32PicWidth;
        pOutAttr->picHeight = pstVencAttr->u32PicHeight;
        s32Ret              = CVI_SUCCESS;
    }

    return s32Ret;
}

static CVI_S32 _cvi_venc_check_jpege_format(
    const VENC_MOD_JPEGE_S *pJpegeModParam)
{
    int                        i;
    unsigned int               marker_cnt[JPEGE_MARKER_BUTT];
    const JPEGE_MARKER_TYPE_E *p = pJpegeModParam->JpegMarkerOrder;

    switch (pJpegeModParam->enJpegeFormat)
    {
    case JPEGE_FORMAT_DEFAULT:
    case JPEGE_FORMAT_TYPE_1:
        return CVI_SUCCESS;
    case JPEGE_FORMAT_CUSTOM:
        // proceed to check marker order validity
        break;
    default:
        CVI_VENC_ERR("Unknown JPEG format %d\n", pJpegeModParam->enJpegeFormat);
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }

    memset(marker_cnt, 0, sizeof(marker_cnt));

    if (p[0] != JPEGE_MARKER_SOI)
    {
        CVI_VENC_ERR("The first jpeg marker must be SOI\n");
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }

    for (i = 0; i < JPEG_MARKER_ORDER_CNT; i++)
    {
        int type = p[i];
        if (JPEGE_MARKER_SOI <= type && type < JPEGE_MARKER_BUTT)
            marker_cnt[type] += 1;
        else
            break;
    }

    if (marker_cnt[JPEGE_MARKER_SOI] == 0)
    {
        CVI_VENC_ERR("There must be one SOI\n");
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }
    if (marker_cnt[JPEGE_MARKER_SOF0] == 0)
    {
        CVI_VENC_ERR("There must be one SOF0\n");
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }
    if (marker_cnt[JPEGE_MARKER_DQT] == 0 && marker_cnt[JPEGE_MARKER_DQT_MERGE] == 0)
    {
        CVI_VENC_ERR("There must be one DQT or DQT_MERGE\n");
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }
    if (marker_cnt[JPEGE_MARKER_DQT] > 0 && marker_cnt[JPEGE_MARKER_DQT_MERGE] > 0)
    {
        CVI_VENC_ERR("DQT and DQT_MERGE are mutually exclusive\n");
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }
    if (marker_cnt[JPEGE_MARKER_DHT] > 0 && marker_cnt[JPEGE_MARKER_DHT_MERGE] > 0)
    {
        CVI_VENC_ERR("DHT and DHT_MERGE are mutually exclusive\n");
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }
    if (marker_cnt[JPEGE_MARKER_DRI] > 0 && marker_cnt[JPEGE_MARKER_DRI_OPT] > 0)
    {
        CVI_VENC_ERR("DRI and DRI_OPT are mutually exclusive\n");
        return CVI_ERR_VENC_JPEG_MARKER_ORDER;
    }

    for (i = JPEGE_MARKER_SOI; i < JPEGE_MARKER_BUTT; i++)
    {
        if (marker_cnt[i] > 1)
        {
            CVI_VENC_ERR("Repeating marker type %d present\n", i);
            return CVI_ERR_VENC_JPEG_MARKER_ORDER;
        }
    }

    return CVI_SUCCESS;
}

static void _cvi_venc_config_jpege_format(VENC_MOD_JPEGE_S *pJpegeModParam)
{
    switch (pJpegeModParam->enJpegeFormat)
    {
    case JPEGE_FORMAT_DEFAULT:
        pJpegeModParam->JpegMarkerOrder[0] = JPEGE_MARKER_SOI;
        pJpegeModParam->JpegMarkerOrder[1] = JPEGE_MARKER_FRAME_INDEX;
        pJpegeModParam->JpegMarkerOrder[2] = JPEGE_MARKER_USER_DATA;
        pJpegeModParam->JpegMarkerOrder[3] = JPEGE_MARKER_DRI_OPT;
        pJpegeModParam->JpegMarkerOrder[4] = JPEGE_MARKER_DQT;
        pJpegeModParam->JpegMarkerOrder[5] = JPEGE_MARKER_DHT;
        pJpegeModParam->JpegMarkerOrder[6] = JPEGE_MARKER_SOF0;
        pJpegeModParam->JpegMarkerOrder[7] = JPEGE_MARKER_BUTT;
        return;
    case JPEGE_FORMAT_TYPE_1:
        pJpegeModParam->JpegMarkerOrder[0] = JPEGE_MARKER_SOI;
        pJpegeModParam->JpegMarkerOrder[1] = JPEGE_MARKER_JFIF;
        pJpegeModParam->JpegMarkerOrder[2] = JPEGE_MARKER_DQT_MERGE;
        pJpegeModParam->JpegMarkerOrder[3] = JPEGE_MARKER_SOF0;
        pJpegeModParam->JpegMarkerOrder[4] = JPEGE_MARKER_DHT_MERGE;
        pJpegeModParam->JpegMarkerOrder[5] = JPEGE_MARKER_DRI;
        pJpegeModParam->JpegMarkerOrder[6] = JPEGE_MARKER_BUTT;
        return;
    default:
        return;
    }
}

/**
 * @brief Send encoder module parameters
 * @param[in] VENC_PARAM_MOD_S.
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_SetModParam(const VENC_PARAM_MOD_S *pstModParam)
{
    CVI_S32       s32Ret = CVI_SUCCESS;
    venc_context *pVencHandle;
    CVI_U32      *pUserDataMaxLen;

    if (pstModParam == NULL)
    {
        CVI_VENC_ERR("pstModParam NULL !\n");
        return CVI_ERR_VENC_ILLEGAL_PARAM;
    }

    s32Ret = _cviVencInitCtxHandle();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("CVI_VENC_GetModParam  init failure\n");
        return s32Ret;
    }

    pVencHandle = (venc_context *)handle;
    if (pVencHandle == NULL)
    {
        CVI_VENC_ERR(
            "CVI_VENC_SetModParam venc_context global handle not create\n");
        return CVI_ERR_VENC_NULL_PTR;
    }
    else
    {
        if (pstModParam->enVencModType == MODTYPE_H264E)
        {
            memcpy(&pVencHandle->ModParam.stH264eModParam,
                   &pstModParam->stH264eModParam, sizeof(VENC_MOD_H264E_S));

            pUserDataMaxLen =
                &(pVencHandle->ModParam.stH264eModParam.u32UserDataMaxLen);
            *pUserDataMaxLen = MIN(*pUserDataMaxLen, USERDATA_MAX_LIMIT);
        }
        else if (pstModParam->enVencModType == MODTYPE_H265E)
        {
            memcpy(&pVencHandle->ModParam.stH265eModParam,
                   &pstModParam->stH265eModParam, sizeof(VENC_MOD_H265E_S));

            pUserDataMaxLen =
                &(pVencHandle->ModParam.stH265eModParam.u32UserDataMaxLen);
            *pUserDataMaxLen = MIN(*pUserDataMaxLen, USERDATA_MAX_LIMIT);
        }
        else if (pstModParam->enVencModType == MODTYPE_JPEGE)
        {
            s32Ret = _cvi_venc_check_jpege_format(&pstModParam->stJpegeModParam);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_VENC_SetModParam  JPEG marker order error\n");
                return s32Ret;
            }

            memcpy(&pVencHandle->ModParam.stJpegeModParam,
                   &pstModParam->stJpegeModParam, sizeof(VENC_MOD_JPEGE_S));
            _cvi_venc_config_jpege_format(&pVencHandle->ModParam.stJpegeModParam);
        }
    }

    return s32Ret;
}

/**
 * @brief Get encoder module parameters
 * @param[in] VENC_PARAM_MOD_S.
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_GetModParam(VENC_PARAM_MOD_S *pstModParam)
{
    CVI_S32       s32Ret;
    venc_context *pVencHandle;

    if (pstModParam == NULL)
    {
        CVI_VENC_ERR("pstModParam NULL !\n");
        return CVI_ERR_VENC_ILLEGAL_PARAM;
    }

    s32Ret = _cviVencInitCtxHandle();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("CVI_VENC_GetModParam  init failure\n");
        return s32Ret;
    }

    pVencHandle = (venc_context *)handle;
    if (pVencHandle == NULL)
    {
        CVI_VENC_ERR(
            "CVI_VENC_GetModParam venc_context global handle not create\n");
        return CVI_ERR_VENC_NULL_PTR;
    }
    else
    {
        if (pstModParam->enVencModType == MODTYPE_H264E)
        {
            memcpy(&pstModParam->stH264eModParam,
                   &pVencHandle->ModParam.stH264eModParam, sizeof(VENC_MOD_H264E_S));
        }
        else if (pstModParam->enVencModType == MODTYPE_H265E)
        {
            memcpy(&pstModParam->stH265eModParam,
                   &pVencHandle->ModParam.stH265eModParam, sizeof(VENC_MOD_H265E_S));
        }
        else if (pstModParam->enVencModType == MODTYPE_JPEGE)
        {
            memcpy(&pstModParam->stJpegeModParam,
                   &pVencHandle->ModParam.stJpegeModParam, sizeof(VENC_MOD_JPEGE_S));
        }
    }

    return CVI_SUCCESS;
}

/**
 * @brief encoder module attach vb buffer
 * @param[in] VeChn VENC Channel Number.
 * @param[in] pstPool VENC_CHN_POOL_S.
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_AttachVbPool(VENC_CHN VeChn, const VENC_CHN_POOL_S *pstPool)
{
    CVI_S32           s32Ret          = CVI_SUCCESS;
    venc_chn_context *pChnHandle      = NULL;
    venc_context     *p_venctx_handle = handle;
    VB_SOURCE_E       eVbSource       = VB_SOURCE_COMMON;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = p_venctx_handle->chn_handle[VeChn];

    if (pstPool == NULL)
    {
        CVI_VENC_ERR("pstPool = NULL\n");
        return CVI_ERR_VENC_ILLEGAL_PARAM;
    }

    if (pChnHandle->pChnAttr->stVencAttr.enType == PT_JPEG || pChnHandle->pChnAttr->stVencAttr.enType == PT_MJPEG)
    {
        CVI_VENC_ERR("Not support Picture type\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }
    else if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H264)
        eVbSource = p_venctx_handle->ModParam.stH264eModParam.enH264eVBSource;
    else if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265)
        eVbSource = p_venctx_handle->ModParam.stH265eModParam.enH265eVBSource;

    if (eVbSource != VB_SOURCE_USER)
    {
        CVI_VENC_ERR("Not support eVbSource:%d\n", eVbSource);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    pChnHandle->pChnVars->bHasVbPool = CVI_TRUE;
    pChnHandle->pChnVars->vbpool     = *pstPool;

    return CVI_SUCCESS;
}

/**
 * @brief encoder module detach vb buffer
 * @param[in] VeChn VENC Channel Number.
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_DetachVbPool(VENC_CHN VeChn)
{
    CVI_S32           s32Ret          = CVI_SUCCESS;
    venc_chn_context *pChnHandle      = NULL;
    venc_context     *p_venctx_handle = handle;
    venc_chn_vars    *pChnVars        = NULL;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = p_venctx_handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    if (pChnVars->bHasVbPool == CVI_FALSE)
    {
        CVI_VENC_ERR("VeChn= %d vbpool not been attached\n", VeChn);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }
    else
    {
        if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265 || (pChnHandle->pChnAttr->stVencAttr.enType == PT_H264))
        {
            int i = 0;

            for (i = 0; i < (int)pChnVars->FrmNum; i++)
            {
                VB_BLK blk;

                CVI_SYS_Munmap(pChnVars->FrmArray[i].virtAddr,
                               pChnVars->FrmArray[i].size);
                blk = CVI_VB_PhysAddr2Handle(pChnVars->FrmArray[i].phyAddr);
                if (blk != VB_INVALID_HANDLE) CVI_VB_ReleaseBlock(blk);
            }
        }
        else
        {
            CVI_VENC_ERR("Not Support Type with bHasVbPool on\n");
            return CVI_ERR_VENC_NOT_SUPPORT;
        }

        pChnVars->bHasVbPool = CVI_FALSE;
    }

    return CVI_SUCCESS;
}

/**
 * @brief Send one frame to encode
 * @param[in] VeChn VENC Channel Number.
 * @param[in] pstFrame pointer to VIDEO_FRAME_INFO_S.
 * @param[in] s32MilliSec TODO VENC
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */

CVI_S32 CVI_VENC_SendFrame(VENC_CHN VeChn, const VIDEO_FRAME_INFO_S *pstFrame,
                           CVI_S32 s32MilliSec)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_sendframe(VeChn, pstFrame, s32MilliSec);
    }
#endif
    CVI_S32              s32Ret     = CVI_SUCCESS;
    venc_chn_context    *pChnHandle = NULL;
    venc_enc_ctx        *pEncCtx    = NULL;
    venc_chn_vars       *pChnVars   = NULL;
    VENC_CHN_STATUS_S   *pChnStat   = NULL;
    venc_enc_ctx_base   *pvecb;
    RECT_S              *pstRect;
    const VIDEO_FRAME_S *pstVFrame = &pstFrame->stVFrame;
    int                  i;
    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    pChnVars   = pChnHandle->pChnVars;
    pChnStat   = &pChnVars->chnStatus;
    pvecb      = &pChnHandle->encCtx.base;
    pstRect    = &pChnVars->stChnParam.stCropCfg.stRect;

    if (pstRect->s32X + pstVFrame->s16OffsetLeft > pvecb->x)
    {
        pvecb->x = pstRect->s32X + pstVFrame->s16OffsetLeft;
        CVI_VENC_TRACE("VeChn:%d pstRect->s32X:%d OffsetLeft:%d pvecb->x:%d \n",
                       VeChn, pstRect->s32X, pstVFrame->s16OffsetLeft, pvecb->x);
    }

    if (pstRect->s32Y + pstVFrame->s16OffsetTop > pvecb->y)
    {
        pvecb->y = pstRect->s32Y + pstVFrame->s16OffsetTop;
        CVI_VENC_TRACE("VeChn:%d pstRect->s32Y:%d OffsetTop:%d pvecb->y:%d \n",
                       VeChn, pstRect->s32Y, pstVFrame->s16OffsetTop, pvecb->y);
    }

    if (pChnVars->chnState <= VENC_CHN_STATE_INIT)
    {
        CVI_VENC_SYNC("chnState = VENC_CHN_STATE_INIT\n");
        return CVI_ERR_VENC_INIT;
    }

    if (pChnVars->bSendFirstFrm == false)
    {
        CVI_BOOL bCbCrInterleave = 0;
        CVI_BOOL bNV21           = 0;

        switch (pstFrame->stVFrame.enPixelFormat)
        {
        case PIXEL_FORMAT_NV12:
            bCbCrInterleave = true;
            bNV21           = false;
            break;
        case PIXEL_FORMAT_NV21:
            bCbCrInterleave = true;
            bNV21           = true;
            break;
        case PIXEL_FORMAT_YUV_PLANAR_420:
        default:
            bCbCrInterleave = false;
            bNV21           = false;
            break;
        }

        _cviVencSetInPixelFormat(VeChn, bCbCrInterleave, bNV21);

        s32Ret = _cviVencAllocVbBuf(VeChn);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("_cviVencAllocVbBuf\n");
            return CVI_ERR_VENC_NOBUF;
        }

        if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H264 || pChnHandle->pChnAttr->stVencAttr.enType == PT_H265)
        {
            _cviVencRegVbBuf(VeChn);
        }

        pChnVars->u32FirstPixelFormat = pstFrame->stVFrame.enPixelFormat;
        pChnVars->bSendFirstFrm       = true;
    }
    else
    {
        if (pChnVars->u32FirstPixelFormat != pstFrame->stVFrame.enPixelFormat)
        {
            if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H264 || pChnHandle->pChnAttr->stVencAttr.enType == PT_H265)
            {
                CVI_VENC_ERR("Input enPixelFormat change\n");
                return CVI_ERR_VENC_ILLEGAL_PARAM;
            }
        }
    }

    cviGetDebugConfigFromEncProc();

    for (i = 0; i < 3; i++)
    {
        pChnVars->u32Stride[i] = pstFrame->stVFrame.u32Stride[i];
    }

    pChnVars->u32SendFrameCnt++;

    s32Ret = cviSetVencChnAttrToProc(VeChn, pstFrame);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("(chn %d) cviSetVencChnAttrToProc fail\n", VeChn);
        return s32Ret;
    }
    pChnVars->u64TimeOfSendFrame = get_current_time();

    if (pEncCtx->base.bVariFpsEn)
    {
        s32Ret = cviCheckFps(pChnHandle, pstFrame);
        if (s32Ret == CVI_ERR_VENC_STAT_VFPS_CHANGE)
        {
            pChnVars->bAttrChange = CVI_TRUE;
            s32Ret                = cviSetChnAttr(pChnHandle);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("cviSetChnAttr, %d\n", s32Ret);
                return s32Ret;
            }
        }
        else if (s32Ret < 0)
        {
            CVI_VENC_ERR("cviCheckFps, %d\n", s32Ret);
            return s32Ret;
        }

        s32Ret = pEncCtx->base.encOnePic(pEncCtx, pstFrame, s32MilliSec);
        if (s32Ret != CVI_SUCCESS)
        {
            if (s32Ret == CVI_ERR_VENC_BUSY)
                CVI_VENC_TRACE("encOnePic Error, s32Ret = %d\n", s32Ret);
            else
                CVI_VENC_ERR("encOnePic Error, s32Ret = %d\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        if (pChnVars->bAttrChange == CVI_TRUE)
        {
            s32Ret = cviSetChnAttr(pChnHandle);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("cviSetChnAttr, %d\n", s32Ret);
                return s32Ret;
            }
        }

        s32Ret = cviCheckFrc(pChnHandle);
        if (s32Ret <= 0)
        {
            cviUpdateFrcSrc(pChnHandle);
            CVI_VENC_FRC("no of encoding frames = %d\n", s32Ret);
            return CVI_ERR_VENC_FRC_NO_ENC;
        }

        CVI_VENC_FRC("cviUpdateFrc Curr Enable = %d\n", s32Ret);

        s32Ret = pEncCtx->base.encOnePic(pEncCtx, pstFrame, s32MilliSec);
        if (s32Ret != CVI_SUCCESS)
        {
            if (s32Ret == CVI_ERR_VENC_BUSY)
                CVI_VENC_TRACE("encOnePic Error, s32Ret = %d\n", s32Ret);
            else
                CVI_VENC_ERR("encOnePic Error, s32Ret = %d\n", s32Ret);

            return s32Ret;
        }

        cviUpdateFrcDst(pChnHandle);
        cviUpdateFrcSrc(pChnHandle);
        cviCheckFrcOverflow(pChnHandle);
    }

    MUTEX_LOCK(pChnHandle->chnMutex);
    pChnStat->u32LeftStreamFrames++;
    pChnVars->currPTS = pstFrame->stVFrame.u64PTS;
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    CVI_VENC_PERF("currPTS = %ld\n", pChnVars->currPTS);

    return s32Ret;
}

static CVI_S32 cviCheckFps(venc_chn_context         *pChnHandle,
                           const VIDEO_FRAME_INFO_S *pstFrame)
{
    venc_chn_vars       *pChnVars  = pChnHandle->pChnVars;
    venc_vfps           *pVfps     = &pChnVars->vfps;
    const VIDEO_FRAME_S *pstVFrame = &pstFrame->stVFrame;
    CVI_S32              s32Ret    = CVI_SUCCESS;
    CVI_S32              nextFps;

    if (pVfps->u64prevSec == 0)
    {
        pVfps->u64prevSec = pstVFrame->u64PTS;
    }
    else
    {
        if (pstVFrame->u64PTS - pVfps->u64prevSec >= pVfps->u64StatTime)
        {
            nextFps = pVfps->s32NumFrmsInOneSec / CVI_DEF_VFPFS_STAT_TIME;

            CVI_VENC_FRC("u64PTS = %ld, prevSec = %ld\n", pstVFrame->u64PTS,
                         pVfps->u64prevSec);
            CVI_VENC_FRC("nextFps = %d\n", nextFps);

            cviSetFps(pChnHandle, nextFps);

            pVfps->u64prevSec         = pstVFrame->u64PTS;
            pVfps->s32NumFrmsInOneSec = 0;

            s32Ret = CVI_ERR_VENC_STAT_VFPS_CHANGE;
        }
    }
    pVfps->s32NumFrmsInOneSec++;

    return s32Ret;
}

static CVI_VOID cviSetFps(venc_chn_context *pChnHandle, CVI_S32 currFps)
{
    CVI_FR32         fr32DstFrameRate = currFps & 0xFFFF;
    VENC_CHN_ATTR_S *pChnAttr;
    VENC_RC_ATTR_S  *prcatt;

    pChnAttr = pChnHandle->pChnAttr;
    prcatt   = &pChnAttr->stRcAttr;

    CVI_VENC_FRC("currFps = %d\n", currFps);

    if (pChnAttr->stVencAttr.enType == PT_H264)
    {
        if (prcatt->enRcMode == VENC_RC_MODE_H264CBR)
        {
            prcatt->stH264Cbr.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264VBR)
        {
            prcatt->stH264Vbr.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264AVBR)
        {
            prcatt->stH264AVbr.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264FIXQP)
        {
            prcatt->stH264FixQp.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264UBR)
        {
            prcatt->stH264Ubr.fr32DstFrameRate = fr32DstFrameRate;
        }
    }
    else if (pChnAttr->stVencAttr.enType == PT_H265)
    {
        if (prcatt->enRcMode == VENC_RC_MODE_H265CBR)
        {
            prcatt->stH265Cbr.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265VBR)
        {
            prcatt->stH265Vbr.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265AVBR)
        {
            prcatt->stH265AVbr.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265FIXQP)
        {
            prcatt->stH265FixQp.fr32DstFrameRate = fr32DstFrameRate;
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265UBR)
        {
            prcatt->stH265Ubr.fr32DstFrameRate = fr32DstFrameRate;
        }
    }
}

static CVI_S32 cviSetChnAttr(venc_chn_context *pChnHandle)
{
    cviVidChnAttr    vidChnAttr = {0};
    cviJpegChnAttr   jpgChnAttr = {0};
    CVI_S32          s32Ret     = CVI_SUCCESS;
    VENC_CHN_ATTR_S *pChnAttr;
    VENC_ATTR_S     *pVencAttr;
    venc_enc_ctx    *pEncCtx  = NULL;
    venc_chn_vars   *pChnVars = NULL;

    pChnAttr  = pChnHandle->pChnAttr;
    pChnVars  = pChnHandle->pChnVars;
    pVencAttr = &pChnAttr->stVencAttr;
    pEncCtx   = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);

    if (pVencAttr->enType == PT_H265 || pVencAttr->enType == PT_H264)
    {
        s32Ret = _cvi_h26x_trans_chn_attr(pChnHandle->pChnAttr, &vidChnAttr);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("h26x trans_chn_attr fail, %d\n", s32Ret);
            goto CHECK_RC_ATTR_RET;
        }

        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_CHN_ATTR,
                                     (CVI_VOID *)&vidChnAttr);
    }
    else if (pVencAttr->enType == PT_MJPEG || pVencAttr->enType == PT_JPEG)
    {
        s32Ret = _cvi_jpg_trans_chn_attr(pChnHandle->pChnAttr, &jpgChnAttr);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("mjpeg trans_chn_attr fail, %d\n", s32Ret);
            goto CHECK_RC_ATTR_RET;
        }

        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_SET_CHN_ATTR,
                                     (CVI_VOID *)&jpgChnAttr);
    }

    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("CVI_H26X_OP_SET_CHN_ATTR or CVI_JPEG_OP_SET_CHN_ATTR, %d\n",
                     s32Ret);
        goto CHECK_RC_ATTR_RET;
    }

    s32Ret = cviCheckRcModeAttr(pChnHandle); // RC side framerate control

CHECK_RC_ATTR_RET:
    pChnVars->bAttrChange = CVI_FALSE;
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviCheckRcModeAttr fail, %d\n", s32Ret);
        return s32Ret;
    }

    s32Ret = cviCheckGopAttr(pChnHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviCheckGopAttr fail, %d\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static CVI_S32 cviCheckFrc(venc_chn_context *pChnHandle)
{
    venc_chn_vars *pChnVars = pChnHandle->pChnVars;
    venc_frc      *pvf      = &pChnVars->frc;
    CVI_S32        ifEncode = 1;

    if (pvf->bFrcEnable)
    {
        if (pvf->srcTs < pvf->dstTs)
        {
            ifEncode = 0;
        }
    }

    CVI_VENC_FRC("srcTs = %d, dstTs = %d, ifEncode = %d\n", pvf->srcTs,
                 pvf->dstTs, ifEncode);
    return ifEncode;
}

static CVI_S32 cviUpdateFrcDst(venc_chn_context *pChnHandle)
{
    venc_chn_vars *pChnVars = pChnHandle->pChnVars;
    venc_frc      *pvf      = &pChnVars->frc;

    if (pvf->bFrcEnable)
    {
        if (pvf->srcTs >= pvf->dstTs)
        {
            pvf->dstTs += pvf->dstFrameDur;
        }
    }

    CVI_VENC_FRC("pvf->bFrcEnable = %d\n", pvf->bFrcEnable);
    return pvf->bFrcEnable;
}

static CVI_S32 cviUpdateFrcSrc(venc_chn_context *pChnHandle)
{
    venc_chn_vars *pChnVars = pChnHandle->pChnVars;
    venc_frc      *pvf      = &pChnVars->frc;

    if (pvf->bFrcEnable)
    {
        pvf->srcTs += pvf->srcFrameDur;
    }

    CVI_VENC_FRC("pvf->bFrcEnable = %d\n", pvf->bFrcEnable);
    return pvf->bFrcEnable;
}

static CVI_VOID cviCheckFrcOverflow(venc_chn_context *pChnHandle)
{
    venc_chn_vars *pChnVars = pChnHandle->pChnVars;
    venc_frc      *pvf      = &pChnVars->frc;

    if (pvf->srcTs >= FRC_TIME_OVERFLOW_OFFSET && pvf->dstTs >= FRC_TIME_OVERFLOW_OFFSET)
    {
        pvf->srcTs -= FRC_TIME_OVERFLOW_OFFSET;
        pvf->dstTs -= FRC_TIME_OVERFLOW_OFFSET;
    }
}

CVI_S32 CVI_VENC_SendFrameEx(VENC_CHN                 VeChn,
                             const USER_FRAME_INFO_S *pstUserFrame,
                             CVI_S32                  s32MilliSec)
{
    const VIDEO_FRAME_INFO_S *pstFrame = &pstUserFrame->stUserFrame;
    cviUserRcInfo             userRcInfo, *puri = &userRcInfo;
    venc_chn_context         *pChnHandle = NULL;
    venc_chn_vars            *pChnVars   = NULL;
    venc_enc_ctx             *pEncCtx    = NULL;
    CVI_S32                   s32Ret     = CVI_SUCCESS;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    pEncCtx    = &pChnHandle->encCtx;

    memcpy(&pChnVars->stUserRcInfo, &pstUserFrame->stUserRcInfo,
           sizeof(USER_RC_INFO_S));

    puri->bQpMapValid     = pChnVars->stUserRcInfo.bQpMapValid;
    puri->u64QpMapPhyAddr = pChnVars->stUserRcInfo.u64QpMapPhyAddr;

    CVI_VENC_CFG("bQpMapValid = %d\n", puri->bQpMapValid);
    CVI_VENC_CFG("u64QpMapPhyAddr = 0x%llX\n", puri->u64QpMapPhyAddr);

    MUTEX_LOCK(pChnHandle->chnMutex);
    s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_USER_RC_INFO,
                                 (CVI_VOID *)puri);
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("CVI_H26X_OP_SET_USER_RC_INFO, 0x%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VENC_SendFrame(VeChn, pstFrame, s32MilliSec);

    return s32Ret;
}

#if 0
static CVI_S32 _cviVencUpdateVbConf(venc_chn_context *pChnHandle,
				    cviVbVencBufConfig *pVbVencCfg,
				    int VbSetFrameBufSize, int VbSetFrameBufCnt)
{
	int j = 0;
	venc_chn_vars *pChnVars = pChnHandle->pChnVars;

	for (j = 0; j < VbSetFrameBufCnt; j++) {
		CVI_U64 u64PhyAddr;

		pChnVars->vbBLK[j] =
			CVI_VB_GetBlockwithID(pChnVars->vbpool.hPicVbPool, VbSetFrameBufSize, CVI_ID_VENC);
		if (pChnVars->vbBLK[j] == VB_INVALID_HANDLE) {
			//not enough size in VB  or VB  pool not create
			CVI_VENC_ERR("Not enough size in VB  or VB  pool Not create\n");
			return CVI_ERR_VENC_NOMEM;
		}

		u64PhyAddr = CVI_VB_Handle2PhysAddr(pChnVars->vbBLK[j]);
		pChnVars->FrmArray[j].phyAddr = u64PhyAddr;
		pChnVars->FrmArray[j].size = VbSetFrameBufSize;
		pChnVars->FrmArray[j].virtAddr = (void *)u64PhyAddr;
		pVbVencCfg->vbModeAddr[j] = u64PhyAddr;
	}

	pChnVars->FrmNum = VbSetFrameBufCnt;
	pVbVencCfg->VbSetFrameBufCnt = VbSetFrameBufCnt;
	pVbVencCfg->VbSetFrameBufSize = VbSetFrameBufSize;

	return CVI_SUCCESS;
}
#endif

static CVI_S32 _cviVencAllocVbBuf(VENC_CHN VeChn)
{
    CVI_S32           s32Ret      = CVI_SUCCESS;
    venc_context     *pVencHandle = handle;
    venc_chn_context *pChnHandle  = NULL;
    VENC_CHN_ATTR_S  *pChnAttr    = NULL;
    VENC_ATTR_S      *pVencAttr   = NULL;
    venc_enc_ctx     *pEncCtx     = NULL;
    // venc_chn_vars *pChnVars = NULL;
    VB_SOURCE_E        eVbSource;
    cviVbVencBufConfig cviVbVencCfg;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    if (pVencHandle == NULL)
    {
        CVI_VENC_ERR("p_venctx_handle NULL (Channel not create yet..)\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnAttr   = pChnHandle->pChnAttr;
    // pChnVars = pChnHandle->pChnVars;
    pEncCtx   = &pChnHandle->encCtx;
    pVencAttr = &pChnAttr->stVencAttr;

    if (pVencAttr->enType == PT_H264)
    {
        CVI_VENC_TRACE("pt_h264 stH264eModParam\n");
        eVbSource = pVencHandle->ModParam.stH264eModParam.enH264eVBSource;
    }
    else if (pVencAttr->enType == PT_H265)
    {
        CVI_VENC_TRACE("pt_h265 stH265eModParam\n");
        eVbSource = pVencHandle->ModParam.stH265eModParam.enH265eVBSource;
    }
    else
    {
        CVI_VENC_TRACE("none H26X\n");
        eVbSource = VB_SOURCE_COMMON;
    }

    CVI_VENC_TRACE("type[%d] eVbSource[%d]\n", pVencAttr->enType, eVbSource);

    memset(&cviVbVencCfg, 0, sizeof(cviVbVencBufConfig));
    cviVbVencCfg.VbType = VB_SOURCE_COMMON;
#if 0
	if (eVbSource == VB_SOURCE_PRIVATE) {
		CVI_VENC_TRACE("enter private vb mode\n");
		if (pChnVars->bHasVbPool == CVI_FALSE) {
			VB_POOL_CONFIG_S stVbPoolCfg;

			//step1 get vb info from vpu driver: calculate the buffersize and buffer count
			s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_GET_VB_INFO, (CVI_VOID *)&cviVbVencCfg);

			if (s32Ret != CVI_SUCCESS) {
				CVI_VENC_ERR("CVI_H26X_OP_GET_VB_INFO, %d\n", s32Ret);
				return s32Ret;
			}

			CVI_VENC_TRACE("private mode get vb info ---\n");
			CVI_VENC_TRACE("Chn[%d]get vb info FrameBufferSize[%d] FrameBufferCnt[%d]\n",
					VeChn, cviVbVencCfg.VbGetFrameBufSize, cviVbVencCfg.VbGetFrameBufCnt);

			stVbPoolCfg.u32BlkSize = cviVbVencCfg.VbGetFrameBufSize;
			stVbPoolCfg.u32BlkCnt = cviVbVencCfg.VbGetFrameBufCnt;
			stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;

			pChnVars->vbpool.hPicVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
			pChnVars->bHasVbPool = CVI_TRUE;
			s32Ret = _cviVencUpdateVbConf(pChnHandle, &cviVbVencCfg,
									cviVbVencCfg.VbGetFrameBufSize,
									cviVbVencCfg.VbGetFrameBufCnt);
			if (s32Ret != CVI_SUCCESS) {
				CVI_VENC_ERR("_cviVencUpdateVbConf, %d\n", s32Ret);
				return s32Ret;
			}

			cviVbVencCfg.VbType = VB_SOURCE_PRIVATE;
		}
	} else if (eVbSource == VB_SOURCE_USER) {
		VB_CONFIG_S pstVbConfig;
		CVI_S32 s32UserSetupFrmCnt;
		CVI_S32 s32UserSetupFrmSize;
		//step1 get vb info from vpu driver: calculate the buffersize and buffer count
		s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_GET_VB_INFO, (CVI_VOID *)&cviVbVencCfg);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_H26X_OP_GET_VB_INFO, %d\n", s32Ret);
			return s32Ret;
		}

		//totally setup by user vbpool id, if not been attachVB return failure
		if (pChnVars->bHasVbPool == CVI_FALSE) {
			CVI_VENC_ERR("[error][%s][%d]\n", __func__, __LINE__);
			CVI_VENC_ERR("Not attach vb pool for channel[%d]\n", VeChn);
			return CVI_ERR_VENC_NOT_SUPPORT;
		}
		//check the VB config and compare with VPU requirement
		s32Ret = CVI_VB_GetConfig(&pstVbConfig);
		if (s32Ret == CVI_SUCCESS) {
			CVI_VENC_TRACE("max pool cnt[%d]  currentPoolId[%d] blksize[%d] blkcnt[%d]\n",
				pstVbConfig.u32MaxPoolCnt, pChnVars->vbpool.hPicVbPool,
				pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkSize,
				pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkCnt);
			if ((int)pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkSize <
				cviVbVencCfg.VbGetFrameBufSize) {
				CVI_VENC_WARN("[Warning] create size is smaller than require size\n");
			}
			if ((int)pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkCnt <
				cviVbVencCfg.VbGetFrameBufCnt) {
				CVI_VENC_WARN("[Warning] create blk is smaller than require blk\n");
			}
		} else
			CVI_VENC_ERR("Error while CVI_VB_GetConfig\n");

		CVI_VENC_TRACE("[required] size[%d] cnt[%d]\n",
			cviVbVencCfg.VbGetFrameBufSize, cviVbVencCfg.VbGetFrameBufCnt);
		CVI_VENC_TRACE("[user set] size[%d] cnt[%d]\n",
			pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkSize,
			pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkCnt);

		s32UserSetupFrmSize = pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkSize;
		//s32UserSetupFrmCnt = pstVbConfig.astCommPool[pChnHandle->vbpool.hPicVbPool].u32BlkCnt;
		if (s32UserSetupFrmSize < cviVbVencCfg.VbGetFrameBufSize) {
			CVI_VENC_WARN("Buffer size too small for frame buffer : user mode VB pool[%d] < [%d]n",
				pstVbConfig.astCommPool[pChnVars->vbpool.hPicVbPool].u32BlkSize,
				cviVbVencCfg.VbGetFrameBufSize);
		}

		s32UserSetupFrmCnt = cviVbVencCfg.VbGetFrameBufCnt;
		s32UserSetupFrmSize = cviVbVencCfg.VbGetFrameBufSize;
		s32Ret = _cviVencUpdateVbConf(pChnHandle, &cviVbVencCfg,
					      s32UserSetupFrmSize,
					      s32UserSetupFrmCnt);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("_cviVencUpdateVbConf, %d\n", s32Ret);
			return s32Ret;
		}
		cviVbVencCfg.VbType = VB_SOURCE_USER;
	}
#endif

    if (pEncCtx->base.ioctl)
    {
        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_VB_BUFFER,
                                     (CVI_VOID *)&cviVbVencCfg);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_H26X_OP_SET_VB_BUFFER, %d\n", s32Ret);
            return s32Ret;
        }
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_StartRecvFrame(VENC_CHN                     VeChn,
                                const VENC_RECV_PIC_PARAM_S *pstRecvParam)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_startrecvframe(VeChn, pstRecvParam);
    }
#endif
    CVI_S32                 s32Ret     = CVI_SUCCESS;
    venc_chn_context       *pChnHandle = NULL;
    venc_enc_ctx           *pEncCtx    = NULL;
    venc_chn_vars          *pChnVars   = NULL;
    struct cvi_venc_vb_ctx *pVbCtx     = NULL;

    CVI_VENC_API("VeChn = %d, s32RecvPicNum = %d\n", VeChn,
                 pstRecvParam->s32RecvPicNum);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    pEncCtx    = &pChnHandle->encCtx;
    pVbCtx     = pChnHandle->pVbCtx;

    pChnVars->s32RecvPicNum = pstRecvParam->s32RecvPicNum;

    MUTEX_LOCK(pChnHandle->chnMutex);

    s32Ret = cviSetRcParamToDrv(pChnHandle);
    if (s32Ret < 0)
    {
        CVI_VENC_ERR("cviSetRcParamToDrv\n");
        MUTEX_UNLOCK(pChnHandle->chnMutex);
        return s32Ret;
    }

    s32Ret = cviSetCuPredToDrv(pChnHandle);
    if (s32Ret < 0)
    {
        CVI_VENC_ERR("cviSetCuPredToDrv\n");
        return s32Ret;
    }

    if (pEncCtx->base.ioctl)
    {
        if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265 || pChnHandle->pChnAttr->stVencAttr.enType == PT_H264)
        {
            s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_START, NULL);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_H26X_OP_START, %d\n", s32Ret);
                MUTEX_UNLOCK(pChnHandle->chnMutex);
                return -1;
            }
        }
        else
        {
            s32Ret = pEncCtx->base.ioctl(
                pEncCtx, CVI_JPEG_OP_START,
                (CVI_VOID *)&pChnHandle->pChnAttr->stVencAttr.bIsoSendFrmEn);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_JPEG_OP_START, %d\n", s32Ret);
                MUTEX_UNLOCK(pChnHandle->chnMutex);
                return -1;
            }
        }
    }
    MUTEX_UNLOCK(pChnHandle->chnMutex);
    if (IF_WANNA_ENABLE_BIND_MODE())
    {
        struct sched_param param;
        pthread_attr_t     attr;

        pVbCtx->currBindMode   = CVI_TRUE;
        pChnHandle->bChnEnable = CVI_TRUE;

        param.sched_priority = 31;
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setstacksize(&attr, 8192);
        pthread_create(&pVbCtx->thread, &attr, venc_event_handler,
                       (CVI_VOID *)pChnHandle);
        cviVenc_sem_post(&pChnVars->sem_send);
    }

    pChnVars->chnState = VENC_CHN_STATE_START_ENC;
    CVI_VENC_SYNC("VENC_CHN_STATE_START_ENC\n");
    handle->chn_status[VeChn] = VENC_CHN_STATE_START_ENC;

    return s32Ret;
}

CVI_S32 CVI_VENC_StopRecvFrame(VENC_CHN VeChn)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_stoprecvframe(VeChn);
    }
#endif
    CVI_S32                 s32Ret     = CVI_SUCCESS;
    venc_chn_context       *pChnHandle = NULL;
    struct cvi_venc_vb_ctx *pVbCtx     = NULL;
    venc_chn_vars          *pChnVars   = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    pVbCtx     = pChnHandle->pVbCtx;

    if (IF_WANNA_DISABLE_BIND_MODE())
    {
        cviVenc_sem_post(&pChnVars->sem_release);
        pChnHandle->bChnEnable = CVI_FALSE;
        cviVenc_sem_post(&pVbCtx->sem_vb);
        pthread_join(pVbCtx->thread, NULL);

        pVbCtx->currBindMode = CVI_FALSE;
        cviVenc_sem_post(&pChnVars->sem_send);
        CVI_VENC_SYNC("venc_event_handler end\n");
    }

    pChnVars->chnState        = VENC_CHN_STATE_STOP_ENC;
    handle->chn_status[VeChn] = VENC_CHN_STATE_STOP_ENC;

    return s32Ret;
}

/**
 * @brief Query the current channel status of encoder
 * @param[in] VeChn VENC Channel Number.
 * @param[out] pstStatus Current channel status of encoder
 * @retval 0 Success
 * @retval Non-0 Failure, Please see the error code table
 */
CVI_S32 CVI_VENC_QueryStatus(VENC_CHN VeChn, VENC_CHN_STATUS_S *pstStatus)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_querystatus(VeChn, pstStatus);
    }
#endif
    CVI_S32            s32Ret     = CVI_SUCCESS;
    venc_chn_context  *pChnHandle = NULL;
    VENC_ATTR_S       *pVencAttr  = NULL;
    VENC_CHN_STATUS_S *pChnStat   = NULL;
    venc_chn_vars     *pChnVars   = NULL;
    VENC_CHN_ATTR_S   *pChnAttr   = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    pChnAttr   = pChnHandle->pChnAttr;
    pChnStat   = &pChnVars->chnStatus;
    pVencAttr  = &pChnAttr->stVencAttr;

    memcpy(pstStatus, pChnStat, sizeof(VENC_CHN_STATUS_S));

    pstStatus->u32CurPacks = _cviGetNumPacks(pVencAttr->enType);

    return s32Ret;
}

static CVI_S32 updateStreamPackInBindMode(venc_chn_context *pChnHandle,
                                          venc_chn_vars    *pChnVars,
                                          VENC_STREAM_S    *pstStream)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U32 idx    = 0;

    if (pChnVars->s32BindModeGetStreamRet != CVI_SUCCESS)
    {
        CVI_VENC_ERR("bind mode get stream error: 0x%X\n",
                     pChnVars->s32BindModeGetStreamRet);
        return pChnVars->s32BindModeGetStreamRet;
    }

    if (!pChnVars->stStream.pstPack)
    {
        CVI_VENC_ERR("pChnVars->stStream.pstPack is NULL\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    s32Ret = cviCheckLeftStreamFrames(pChnHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_WARN("cviCheckLeftStreamFrames, s32Ret = 0x%X\n", s32Ret);
        return s32Ret;
    }

    pstStream->u32PackCount = pChnVars->stStream.u32PackCount;
    for (idx = 0; idx < pstStream->u32PackCount; idx++)
    {
        VENC_PACK_S *ppack = &pstStream->pstPack[idx];

        memcpy(ppack, &pChnVars->stStream.pstPack[idx], sizeof(VENC_PACK_S));
        ppack->u64PTS = pChnVars->currPTS;
    }
    CVI_VENC_PERF("currPTS = %ld\n", pChnVars->currPTS);

    return s32Ret;
}

static void cviVenc_sem_init(ptread_sem_s *sem)
{
    pthread_mutexattr_t ma;
    pthread_condattr_t  cattr;

    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);

    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
    // pthread_mutexattr_setrobust(&ma, PTHREAD_MUTEX_ROBUST);

    sem->cnt = 0;
    pthread_cond_init(&sem->cond, &cattr);
    pthread_mutex_init(&sem->mutex, &ma);

    return;
}

static void cviVenc_sem_post(ptread_sem_s *sem)
{
    pthread_mutex_lock(&sem->mutex);
    sem->cnt++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);

    return;
}

static CVI_S32 cviVenc_sem_wait(ptread_sem_s *sem, long msecs)
{
    struct timespec ts;
    unsigned long   secs = msecs / 1000;
    unsigned long   add  = 0;
    int             ret;
    CVI_S32         s32RetStatus;

    pthread_mutex_lock(&sem->mutex);
    if (sem->cnt > 0) {
        sem->cnt--;
        pthread_mutex_unlock(&sem->mutex);
        return CVI_SUCCESS;
    }

    if (msecs < 0) {
        ret = pthread_cond_wait(&sem->cond, &sem->mutex);
    } else if (msecs == 0) {
        ret = -1;
    } else {
        msecs      = msecs % 1000;
        msecs      = msecs * 1000 * 1000;
        add        = msecs / (1000 * 1000 * 1000);
        ts.tv_sec  = (add + secs);
        ts.tv_nsec = msecs % (1000 * 1000 * 1000);

        CVI_VENC_TRACE("pthread_cond_timedwait\n");
        ret = pthread_cond_timedwait(&sem->cond, &sem->mutex, &ts);
    }

    if (ret == 0) {
        CVI_VENC_TRACE("sem_wait success in time[%ld]\n", msecs);
        sem->cnt--;
        s32RetStatus = CVI_SUCCESS;
    } else if (ret == EAGAIN) {
        CVI_VENC_TRACE("sem EAGAIN[%ld]\n", msecs);
        s32RetStatus = CVI_ERR_VENC_BUSY;
    } else if (ret == ETIMEDOUT) {
        CVI_VENC_TRACE("sem ETIMEDOUT[%ld]\n", msecs);
        s32RetStatus = CVI_ERR_VENC_BUSY;
    } else {
        CVI_VENC_ERR("sem unexpected errno[%d]time[%ld]\n", ret, msecs);
        s32RetStatus = CVI_FAILURE;
    }

    pthread_mutex_unlock(&sem->mutex);

    return s32RetStatus;
}

static void cviVenc_sem_destroy(ptread_sem_s *sem)
{
    pthread_cond_destroy(&sem->cond);
    pthread_mutex_destroy(&sem->mutex);

    return;
}
#if 0
static CVI_S32 cviVenc_sem_timedwait_Millsecs(sem_t *sem, long msecs)
{
	struct timespec ts;
	unsigned long secs = msecs/1000;
	unsigned long add = 0;
	int ret;
	CVI_S32 s32RetStatus;

	if (msecs < 0)
		ret =  sem_wait(sem);
	else if (msecs == 0)
		ret = sem_trywait(sem);
	else {
		msecs = msecs%1000;
//		clock_gettime(CLOCK_REALTIME, &ts);
		msecs = msecs * 1000 * 1000 + ts.tv_nsec;
		add = msecs / (1000 * 1000 * 1000);
		ts.tv_sec += (add + secs);
		ts.tv_nsec = msecs % (1000 * 1000 * 1000);

		CVI_VENC_TRACE("sem_timedwait\n");
		ret = sem_timedwait(sem, &ts);
	}

	if (ret == 0) {
		CVI_VENC_TRACE("sem_wait success in time[%ld]\n", msecs);
		s32RetStatus = CVI_SUCCESS;
	} else {
		if (errno == EINTR) {
			CVI_VENC_ERR("sem_wait interrupt by SIG\n");
			s32RetStatus = CVI_FAILURE;
		} else if (errno == EINVAL) {
			CVI_VENC_ERR("sem_wait invalid sem\n");
			s32RetStatus = CVI_FAILURE;
		} else if (errno == EAGAIN) {
			CVI_VENC_TRACE("sem EAGAIN[%ld]\n", msecs);
			s32RetStatus = CVI_ERR_VENC_BUSY;
		} else if (errno == ETIMEDOUT) {
			CVI_VENC_TRACE("sem ETIMEDOUT[%ld]\n", msecs);
			s32RetStatus = CVI_ERR_VENC_BUSY;
		} else {
			CVI_VENC_ERR("sem unexpected errno[%d]time[%ld]\n", errno, msecs);
			s32RetStatus = CVI_FAILURE;
		}
	}
	return s32RetStatus;
}
#endif

/**
 * @brief Get encoded bitstream
 * @param[in] VeChn VENC Channel Number.
 * @param[out] pstStream pointer to VIDEO_FRAME_INFO_S.
 * @param[in] S32MilliSec TODO VENC
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream,
                           CVI_S32 S32MilliSec)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_getstream(VeChn, pstStream, S32MilliSec);
    }
#endif
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    venc_enc_ctx     *pEncCtx    = NULL;
    venc_chn_vars    *pChnVars   = NULL;
    CVI_U64           getStream_e;
    // VENC_CHN_ATTR_S *pChnAttr;
    CVI_U64                 getStream_s = get_current_time();
    struct cvi_venc_vb_ctx *pVbCtx      = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    pChnVars   = pChnHandle->pChnVars;
    // pChnAttr = pChnHandle->pChnAttr;
    pVbCtx = pChnHandle->pVbCtx;

    if (pChnHandle->bSbSkipFrm == true)
    {
        // CVI_VENC_ERR("SbSkipFrm\n");
        return CVI_ERR_VENC_BUSY;
    }

    if (pVbCtx->currBindMode == CVI_TRUE)
    {
        CVI_VENC_TRACE("wait encode done\n");
#if 1
        s32Ret = cviVenc_sem_wait(&pChnVars->sem_send, S32MilliSec);
        if (s32Ret == CVI_FAILURE)
        {
            CVI_VENC_ERR("sem wait error\n");
            return CVI_ERR_VENC_MUTEX_ERROR;
        }
        else if (s32Ret == CVI_ERR_VENC_BUSY)
        {
            CVI_VENC_TRACE("sem wait timeout\n");
            return s32Ret;
        }
#else
        SEM_WAIT(pChnVars->sem_send);
#endif
        s32Ret = updateStreamPackInBindMode(pChnHandle, pChnVars, pstStream);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("updateStreamPackInBindMode fail, s32Ret = 0x%X\n", s32Ret);
            return s32Ret;
        }

        cviUpdateChnStatus(pChnHandle);
        op_eventfd(pChnHandle, 1, 0);

        return CVI_SUCCESS;
    }
#if 0
	s32Ret = cviCheckLeftStreamFrames(pChnHandle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_WARN("cviCheckLeftStreamFrames, s32Ret = 0x%X\n", s32Ret);
		return s32Ret;
	}
#endif
    pEncCtx->base.u64PTS = pChnVars->currPTS;
    s32Ret               = pEncCtx->base.getStream(pEncCtx, pstStream, S32MilliSec);

    if (s32Ret != CVI_SUCCESS)
    {
        if ((S32MilliSec >= 0) && (s32Ret == CVI_ERR_VENC_BUSY))
        {
            // none block mode: timeout
            CVI_VENC_TRACE("Non-blcok mode timeout\n");
        }
        else
        {
            // block mode
            if (s32Ret != CVI_ERR_VENC_EMPTY_STREAM_FRAME)
                CVI_VENC_ERR("getStream, s32Ret = %d\n", s32Ret);
        }

        return s32Ret;
    }

    s32Ret = cviProcessResult(pChnHandle, pstStream);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("(chn %d) cviProcessResult fail\n", VeChn);
        return s32Ret;
    }

    getStream_e          = get_current_time();
    pChnVars->totalTime += (getStream_e - pChnVars->u64TimeOfSendFrame);
    CVI_VENC_PERF(
        "send frame timestamp = 0x%lx, enc time = 0x%lx (0x%lx + 0x%lx)ms, total "
        "time = 0x%lx\n",
        (pChnVars->u64TimeOfSendFrame),
        (getStream_e - pChnVars->u64TimeOfSendFrame),
        (getStream_s - pChnVars->u64TimeOfSendFrame), (getStream_e - getStream_s),
        pChnVars->totalTime);

    cviUpdateChnStatus(pChnHandle);

    if (pChnHandle->sbm_state == VENC_SBM_STATE_FRM_RUN)
        pChnHandle->sbm_state = VENC_SBM_STATE_IDLE;

    return s32Ret;
}

static CVI_S32 cviCheckLeftStreamFrames(venc_chn_context *pChnHandle)
{
    venc_chn_vars     *pChnVars = pChnHandle->pChnVars;
    VENC_CHN_STATUS_S *pChnStat = &pChnVars->chnStatus;
    CVI_S32            s32Ret   = CVI_SUCCESS;

    MUTEX_LOCK(pChnHandle->chnMutex);
    if (pChnStat->u32LeftStreamFrames <= 0)
    {
        CVI_VENC_ERR("u32LeftStreamFrames <= 0, no stream data to get\n");
        s32Ret = CVI_ERR_VENC_EMPTY_STREAM_FRAME;
    }
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    return s32Ret;
}

static CVI_S32 cviProcessResult(venc_chn_context *pChnHandle,
                                VENC_STREAM_S    *pstStream)
{
    venc_chn_vars   *pChnVars = NULL;
    VENC_CHN_ATTR_S *pChnAttr;
    VENC_ATTR_S     *pVencAttr;
    CVI_S32          s32Ret = CVI_SUCCESS;

    pChnVars  = pChnHandle->pChnVars;
    pChnAttr  = pChnHandle->pChnAttr;
    pVencAttr = &pChnAttr->stVencAttr;

    pChnVars->u32GetStreamCnt++;
    s32Ret = cviSetVencPerfAttrToProc(pChnHandle);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("(chn %d) cviSetVencPerfAttrToProc fail\n", pChnHandle->VeChn);
        return s32Ret;
    }

    if (pVencAttr->enType == PT_JPEG || pVencAttr->enType == PT_MJPEG)
    {
        CVI_U32 i = 0;

        for (i = 0; i < pstStream->u32PackCount; i++)
        {
            VENC_PACK_S *ppack = &pstStream->pstPack[i];
            int          j;

            for (j = (ppack->u32Len - ppack->u32Offset - 1); j > 0; j--)
            {
                unsigned char *tmp_ptr = ppack->pu8Addr + ppack->u32Offset + j;
                if (tmp_ptr[0] == 0xd9 && tmp_ptr[-1] == 0xff)
                {
                    break;
                }
            }
            ppack->u32Len = ppack->u32Offset + j + 1;
        }
    }

    return s32Ret;
}

static CVI_VOID cviUpdateChnStatus(venc_chn_context *pChnHandle)
{
    venc_chn_vars     *pChnVars = pChnHandle->pChnVars;
    VENC_CHN_STATUS_S *pChnStat = &pChnVars->chnStatus;
    venc_enc_ctx      *pEncCtx  = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);

    pChnStat->stVencStrmInfo.u32MeanQp = pEncCtx->ext.vid.streamInfo.u32MeanQp;
    pChnStat->u32LeftStreamFrames--;

    CVI_VENC_TRACE("LeftStreamFrames = %d, u32MeanQp = %d\n",
                   pChnStat->u32LeftStreamFrames,
                   pChnStat->stVencStrmInfo.u32MeanQp);

    MUTEX_UNLOCK(pChnHandle->chnMutex);
}

#if 0
static CVI_S32 CVI_COMM_VENC_GetDataType(PAYLOAD_TYPE_E enType, VENC_PACK_S *ppack)
{
	if (enType == PT_H264)
		return ppack->DataType.enH264EType;
	else if (enType == PT_H265)
		return ppack->DataType.enH265EType;
	else if (enType == PT_JPEG || enType == PT_MJPEG)
		return ppack->DataType.enJPEGEType;

	CVI_VENC_ERR("enType = %d\n", enType);
	return CVI_FAILURE;
}
#endif

CVI_S32 CVI_VENC_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_releasestream(VeChn, pstStream);
    }
#endif
    CVI_S32                 s32Ret     = CVI_SUCCESS;
    venc_chn_context       *pChnHandle = NULL;
    venc_enc_ctx           *pEncCtx    = NULL;
    venc_chn_vars          *pChnVars   = NULL;
    struct cvi_venc_vb_ctx *pVbCtx     = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    pChnVars   = pChnHandle->pChnVars;
    pVbCtx     = pChnHandle->pVbCtx;

    if (pVbCtx->currBindMode == CVI_TRUE)
    {
        CVI_VENC_TRACE("[%d]post release\n", pChnVars->frameIdx);
        cviVenc_sem_post(&pChnVars->sem_release);
    }
    else
    {
        s32Ret = pEncCtx->base.releaseStream(pEncCtx, pstStream);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("releaseStream, s32Ret = %d\n", s32Ret);
            return s32Ret;
        }
    }

    pChnVars->frameIdx++;
    CVI_VENC_API("frameIdx = %d\n", pChnVars->frameIdx);

    if (pChnHandle->jpgFrmSkipCnt)
    {
        pChnHandle->jpgFrmSkipCnt--;
        CVI_VENC_TRACE("chn%d: jpgFrmSkipCnt = %d\n", VeChn,
                       pChnHandle->jpgFrmSkipCnt);
    }

    cviChangeMask(pChnVars->frameIdx);

    return s32Ret;
}

static CVI_VOID cviChangeMask(CVI_S32 frameIdx)
{
    venc_dbg *pDbg = &vencDbg;

    pDbg->currMask = pDbg->dbgMask;
    if (pDbg->startFn >= 0)
    {
        if (frameIdx >= pDbg->startFn && frameIdx <= pDbg->endFn)
            pDbg->currMask = pDbg->dbgMask;
        else
            pDbg->currMask = CVI_VENC_MASK_ERR;
    }

    CVI_VENC_TRACE("currMask = 0x%X\n", pDbg->currMask);
}

static CVI_S32 cviSetVencChnAttrToProc(VENC_CHN                  VeChn,
                                       const VIDEO_FRAME_INFO_S *pstFrame)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_vars    *pChnVars   = NULL;
    venc_chn_context *pChnHandle = NULL;
    CVI_U64           u64CurTime = get_current_time();

    CVI_VENC_TRACE("\n");

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    if (pstFrame == NULL)
    {
        CVI_VENC_ERR("pstFrame is NULL\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    if (MUTEX_LOCK(pChnHandle->chnShmMutex) != 0)
    {
        CVI_VENC_ERR("can not lock chnShmMutex\n");
        return CVI_FAILURE;
    }
    memcpy(&pChnVars->stFrameInfo, pstFrame, sizeof(VIDEO_FRAME_INFO_S));
    if ((u64CurTime - pChnVars->u64LastSendFrameTimeStamp) > SEC_TO_MS)
    {
        pChnVars->stFPS.u32InFPS =
            (CVI_U32)((pChnVars->u32SendFrameCnt * SEC_TO_MS) / (CVI_U32)(u64CurTime - pChnVars->u64LastSendFrameTimeStamp));
        pChnVars->u64LastSendFrameTimeStamp = u64CurTime;
        pChnVars->u32SendFrameCnt           = 0;
    }
    MUTEX_UNLOCK(pChnHandle->chnShmMutex);

    return s32Ret;
}

static CVI_S32 cviSetVencPerfAttrToProc(venc_chn_context *pChnHandle)
{
    CVI_S32        s32Ret     = CVI_SUCCESS;
    venc_chn_vars *pChnVars   = NULL;
    venc_enc_ctx  *pEncCtx    = NULL;
    CVI_U64        u64CurTime = get_current_time();

    CVI_VENC_TRACE("\n");
    pChnVars = pChnHandle->pChnVars;
    pEncCtx  = &pChnHandle->encCtx;

    if (MUTEX_LOCK(pChnHandle->chnShmMutex) != 0)
    {
        CVI_VENC_ERR("can not lock chnShmMutex\n");
        return CVI_FAILURE;
    }
    if ((u64CurTime - pChnVars->u64LastGetStreamTimeStamp) > SEC_TO_MS)
    {
        pChnVars->stFPS.u32OutFPS           = (CVI_U32)((pChnVars->u32GetStreamCnt * SEC_TO_MS) / ((CVI_U32)(u64CurTime - pChnVars->u64LastGetStreamTimeStamp)));
        pChnVars->u64LastGetStreamTimeStamp = u64CurTime;
        pChnVars->u32GetStreamCnt           = 0;
    }
    pChnVars->stFPS.u64HwTime = pEncCtx->base.u64EncHwTime;
    MUTEX_UNLOCK(pChnHandle->chnShmMutex);

    return s32Ret;
}

static CVI_VOID cviGetDebugConfigFromEncProc(void)
{
    proc_debug_config_t *ptEncProcDebugConfig =
        (proc_debug_config_t *)vpu_comm_shared_mem;
    venc_dbg *pDbg = &vencDbg;

    CVI_VENC_TRACE("\n");

    if (ptEncProcDebugConfig == NULL)
    {
        CVI_VENC_ERR("ptEncProcDebugConfig is NULL\n");
        return;
    }

    memset(pDbg, 0, sizeof(venc_dbg));

    pDbg->dbgMask = ptEncProcDebugConfig->u32DbgMask;
    if (pDbg->dbgMask == CVI_VENC_NO_INPUT || pDbg->dbgMask == CVI_VENC_INPUT_ERR)
        pDbg->dbgMask = CVI_VENC_MASK_ERR;
    else
        pDbg->dbgMask |= CVI_VENC_MASK_ERR;

    pDbg->currMask = pDbg->dbgMask;
    pDbg->startFn  = ptEncProcDebugConfig->u32StartFrmIdx;
    pDbg->endFn    = ptEncProcDebugConfig->u32EndFrmIdx;
    // strcpy(pDbg->dbgDir, ptEncProcDebugConfig->cDumpPath);
    pDbg->noDataTimeout = ptEncProcDebugConfig->u32NoDataTimeout;

    cviChangeMask(0);
}

/**
 * @brief Destroy One VENC Channel.
 * @param[in] VeChn VENC Channel Number.
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_DestroyChn(VENC_CHN VeChn)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_destroychn(VeChn);
    }
#endif
    CVI_S32                 s32Ret     = CVI_SUCCESS;
    venc_chn_context       *pChnHandle = NULL;
    venc_enc_ctx           *pEncCtx    = NULL;
    venc_chn_vars          *pChnVars   = NULL;
    struct cvi_venc_vb_ctx *pVbCtx     = NULL;
    MMF_CHN_S               chn        = {.enModId = CVI_ID_VENC, .s32DevId = 0, .s32ChnId = VeChn};
    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    pChnVars   = pChnHandle->pChnVars;
    pVbCtx     = pChnHandle->pVbCtx;

    base_mod_jobs_exit(chn, CHN_TYPE_IN);

    s32Ret = _VENC_UNMAP();
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("_VENC_UNMAP failed.\n");
        return s32Ret;
    }

    if (pChnHandle->sbm_state == VENC_SBM_STATE_FRM_RUN)
    {
        VENC_CHN_STATUS_S stStat;
        VENC_STREAM_S     stStream;

        cvi_VENC_CB_SkipFrame(0, 0, pChnHandle->pChnAttr->stVencAttr.u32PicHeight,
                              1000);

        s32Ret = CVI_VENC_QueryStatus(VeChn, &stStat);

        stStream.pstPack =
            (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
        if (stStream.pstPack == NULL)
        {
            CVI_VENC_ERR("malloc memory failed!\n");
            return s32Ret;
        }

        s32Ret = CVI_VENC_GetStream(VeChn, &stStream, -1);
        s32Ret = CVI_VENC_ReleaseStream(VeChn, &stStream);
    }

    pChnHandle->sbm_state = VENC_SBM_STATE_CHN_CLOSED;

    //	cviSetSBMEnable(pChnHandle, CVI_FALSE);
    s32Ret = pEncCtx->base.close(pEncCtx);

    SEM_DESTROY(tSbmWaitQueue[VeChn]);

    cviVenc_sem_destroy(&pVbCtx->sem_vb);
    cviVenc_sem_destroy(&pChnVars->sem_send);
    cviVenc_sem_destroy(&pChnVars->sem_release);

    if (pChnHandle->pChnVars)
    {
        free(pChnHandle->pChnVars);
        pChnHandle->pChnVars = NULL;
    }

    if (pChnHandle->pChnAttr)
    {
        free(pChnHandle->pChnAttr);
        pChnHandle->pChnAttr = NULL;
    }

    if (pChnHandle->pSBMSendFrameThread)
    {
        frm_done_event[pChnHandle->VeChn] = 1;
        SEM_POST(tSbmWaitQueue[pChnHandle->VeChn]);
        pChnHandle->pSBMSendFrameThread = NULL;
    }
    MUTEX_DESTROY(pChnHandle->chnMutex);
    MUTEX_DESTROY(pChnHandle->chnShmMutex);

    if (handle->chn_handle[VeChn])
    {
        free(handle->chn_handle[VeChn]);
        handle->chn_handle[VeChn] = NULL;
    }

    handle->chn_status[VeChn] = VENC_CHN_STATE_NONE;
    {
        VENC_CHN i               = 0;
        CVI_BOOL bFreeVencHandle = CVI_TRUE;

        for (i = 0; i < VENC_MAX_CHN_NUM; i++)
        {
            if (handle->chn_handle[i] != NULL)
            {
                bFreeVencHandle = CVI_FALSE;
                break;
            }
        }
        if (bFreeVencHandle)
        {
            free(handle);
            handle = NULL;
        }
    }

    return s32Ret;
}

/**
 * @brief Resets a VENC channel.
 * @param[in] VeChn VENC Channel Number.
 * @retval 0 Success
 * @retval Non-0 Failure, please see the error code table
 */
CVI_S32 CVI_VENC_ResetChn(VENC_CHN VeChn)
{
    /* from Hisi,
   * You are advised to call HI_MPI_VENC_ResetChn
   * to reset the channel before switching the format of the input image
   * from non-single-component format to single-component format.
   */
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    PAYLOAD_TYPE_E    enType;
    venc_chn_vars    *pChnVars = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;

    if (pChnVars->chnState == VENC_CHN_STATE_START_ENC)
    {
        CVI_VENC_ERR("Chn %d is not stopped, failure\n", VeChn);
        return CVI_ERR_VENC_BUSY;
    }

    enType = pChnHandle->pChnAttr->stVencAttr.enType;
    if (enType == PT_JPEG)
    {
        venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;
        if (pEncCtx->base.ioctl)
        {
            s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_RESET_CHN, NULL);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_JPEG_OP_RESET_CHN fail, s32Ret = %d\n", s32Ret);
                return s32Ret;
            }
        }
    }

    // TODO: + Check resolution to decide if release memory / + re-init FW

    return s32Ret;
}

CVI_S32 CVI_VENC_SetJpegParam(VENC_CHN                 VeChn,
                              const VENC_JPEG_PARAM_S *pstJpegParam)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_setjpegparam(VeChn, pstJpegParam);
    }
#endif
    venc_chn_context   *pChnHandle = NULL;
    VENC_JPEG_PARAM_S  *pvjp       = NULL;
    CVI_S32             s32Ret     = CVI_SUCCESS;
    VENC_MJPEG_FIXQP_S *pstMJPEGFixQp;
    VENC_RC_ATTR_S     *prcatt;
    venc_chn_vars      *pChnVars;
    venc_enc_ctx       *pEncCtx;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    pChnVars   = pChnHandle->pChnVars;
    pvjp       = &pChnVars->stJpegParam;

    if (pEncCtx->base.ioctl)
    {
        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_SET_MCUPerECS,
                                     (CVI_VOID *)&pstJpegParam->u32MCUPerECS);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_JPEG_OP_SET_MCUPerECS, %d\n", s32Ret);
            return -1;
        }
    }

    if (pstJpegParam->u32Qfactor > Q_TABLE_MAX)
    {
        CVI_VENC_ERR("u32Qfactor <= 0 || >= 100, s32Ret = %d\n", s32Ret);
        return CVI_ERR_VENC_ILLEGAL_PARAM;
    }
    else if (pstJpegParam->u32Qfactor == Q_TABLE_CUSTOM)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    prcatt        = &pChnHandle->pChnAttr->stRcAttr;
    pstMJPEGFixQp = &prcatt->stMjpegFixQp;

    prcatt->enRcMode          = VENC_RC_MODE_MJPEGFIXQP;
    pstMJPEGFixQp->u32Qfactor = pstJpegParam->u32Qfactor;
    CVI_VENC_TRACE("enRcMode = %d, u32Qfactor = %d\n", prcatt->enRcMode,
                   pstMJPEGFixQp->u32Qfactor);

    if (pEncCtx->base.ioctl)
    {
        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_SET_QUALITY,
                                     (CVI_VOID *)&pstMJPEGFixQp->u32Qfactor);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_JPEG_OP_SET_QUALITY, %d\n", s32Ret);
            return -1;
        }
    }

    memcpy(pvjp, pstJpegParam, sizeof(VENC_JPEG_PARAM_S));

    return s32Ret;
}

CVI_S32 CVI_VENC_GetJpegParam(VENC_CHN VeChn, VENC_JPEG_PARAM_S *pstJpegParam)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_getjpegparam(VeChn, pstJpegParam);
    }
#endif
    venc_chn_context  *pChnHandle = NULL;
    venc_chn_vars     *pChnVars   = NULL;
    VENC_JPEG_PARAM_S *pvjp       = NULL;
    CVI_S32            s32Ret     = CVI_SUCCESS;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    pvjp       = &pChnVars->stJpegParam;

    memcpy(pstJpegParam, pvjp, sizeof(VENC_JPEG_PARAM_S));

    return s32Ret;
}

CVI_S32 CVI_VENC_RequestIDR(VENC_CHN VeChn, CVI_BOOL bInstant)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    PAYLOAD_TYPE_E    enType;
    venc_enc_ctx     *pEncCtx;

    CVI_VENC_API("\n");

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    pEncCtx = &pChnHandle->encCtx;

    if (enType == PT_H264 || enType == PT_H265)
    {
        if (pEncCtx->base.ioctl)
        {
            MUTEX_LOCK(pChnHandle->chnMutex);
            s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_REQUEST_IDR,
                                         (CVI_VOID *)&bInstant);
            MUTEX_UNLOCK(pChnHandle->chnMutex);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_H26X_OP_SET_REQUEST_IDR, %d\n", s32Ret);
                return s32Ret;
            }
        }
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_SetChnAttr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstChnAttr)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_setchnattr(VeChn, pstChnAttr);
    }
#endif
    CVI_S32           s32Ret      = CVI_SUCCESS;
    venc_chn_context *pChnHandle  = NULL;
    VENC_RC_ATTR_S   *pstRcAttr   = NULL;
    VENC_ATTR_S      *pstVencAttr = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    if (pstChnAttr == NULL)
    {
        CVI_VENC_ERR("pstChnAttr == NULL\n");
        return CVI_ERR_VENC_ILLEGAL_PARAM;
    }

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    if ((pChnHandle->pChnAttr == NULL) || (pChnHandle->pChnVars == NULL))
    {
        CVI_VENC_ERR("pChnAttr or pChnVars is NULL\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    if (pstChnAttr->stVencAttr.enType != pChnHandle->pChnAttr->stVencAttr.enType)
    {
        CVI_VENC_ERR("enType is incorrect\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    pstRcAttr   = &(pChnHandle->pChnAttr->stRcAttr);
    pstVencAttr = &(pChnHandle->pChnAttr->stVencAttr);

    MUTEX_LOCK(pChnHandle->chnMutex);
    memcpy(pstRcAttr, &pstChnAttr->stRcAttr, sizeof(VENC_RC_ATTR_S));
    memcpy(pstVencAttr, &pstChnAttr->stVencAttr, sizeof(VENC_ATTR_S));
    pChnHandle->pChnVars->bAttrChange = CVI_TRUE;
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    return s32Ret;
}

CVI_S32 CVI_VENC_GetChnAttr(VENC_CHN VeChn, VENC_CHN_ATTR_S *pstChnAttr)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_getchnattr(VeChn, pstChnAttr);
    }
#endif
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle Error, s32Ret = %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    MUTEX_LOCK(pChnHandle->chnMutex);
    memcpy(pstChnAttr, pChnHandle->pChnAttr, sizeof(VENC_CHN_ATTR_S));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    return s32Ret;
}

CVI_S32 CVI_VENC_GetRcParam(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcParam)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    memcpy(pstRcParam, &pChnHandle->rcParam, sizeof(VENC_RC_PARAM_S));

    return s32Ret;
}

CVI_S32 CVI_VENC_SetRcParam(VENC_CHN VeChn, const VENC_RC_PARAM_S *pstRcParam)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    VENC_CHN_ATTR_S  *pChnAttr;
    VENC_ATTR_S      *pVencAttr;
    VENC_RC_PARAM_S  *prcparam;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnAttr   = pChnHandle->pChnAttr;
    pVencAttr  = &pChnAttr->stVencAttr;
    prcparam   = &pChnHandle->rcParam;

    s32Ret = cviCheckRcParam(pChnHandle, pstRcParam);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviCheckRcParam\n");
        return s32Ret;
    }

    memcpy(prcparam, pstRcParam, sizeof(VENC_RC_PARAM_S));

    if (pVencAttr->enType == PT_H264 || pVencAttr->enType == PT_H265)
    {
        prcparam->s32FirstFrameStartQp =
            (prcparam->s32FirstFrameStartQp < 0 || prcparam->s32FirstFrameStartQp > 51)
                ? ((pVencAttr->enType == PT_H265) ? 63 : DEF_IQP)
                : prcparam->s32FirstFrameStartQp;

        if (pstRcParam->s32FirstFrameStartQp != prcparam->s32FirstFrameStartQp)
        {
            CVI_VENC_CFG("pstRcParam 1stQp = %d, prcparam 1stQp = %d\n",
                         pstRcParam->s32FirstFrameStartQp,
                         prcparam->s32FirstFrameStartQp);
        }
    }

    s32Ret = cviSetRcParamToDrv(pChnHandle);
    if (s32Ret < 0)
    {
        CVI_VENC_ERR("cviSetRcParamToDrv\n");
        return s32Ret;
    }

    return s32Ret;
}

static CVI_S32 cviCheckRcParam(venc_chn_context      *pChnHandle,
                               const VENC_RC_PARAM_S *pstRcParam)
{
    VENC_CHN_ATTR_S *pChnAttr  = pChnHandle->pChnAttr;
    VENC_ATTR_S     *pVencAttr = &pChnAttr->stVencAttr;
    VENC_RC_ATTR_S  *prcatt    = &pChnAttr->stRcAttr;
    CVI_S32          s32Ret    = CVI_SUCCESS;

    CHECK_MIN_MAX("s32InitialDelay", pstRcParam->s32InitialDelay,
                  CVI_INITIAL_DELAY_MIN, CVI_INITIAL_DELAY_MAX,
                  CVI_ERR_VENC_RC_PARAM);

    if (pVencAttr->enType == PT_H264)
    {
        if (prcatt->enRcMode == VENC_RC_MODE_H264CBR)
        {
            const VENC_PARAM_H264_CBR_S *pprc = &pstRcParam->stParamH264Cbr;

            CHECK_COMMON_RC_PARAM(pprc);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264VBR)
        {
            const VENC_PARAM_H264_VBR_S *pprc = &pstRcParam->stParamH264Vbr;

            CHECK_COMMON_RC_PARAM(pprc);
            CHECK_MIN_MAX("s32ChangePos", pprc->s32ChangePos, CVI_H26X_CHANGE_POS_MIN,
                          CVI_H26X_CHANGE_POS_MAX, CVI_ERR_VENC_RC_PARAM);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264AVBR)
        {
            const VENC_PARAM_H264_AVBR_S *pprc = &pstRcParam->stParamH264AVbr;
            CVI_VENC_CFG("enType = %d, enRcMode = %d, s32ChangePos = %d\n",
                         pVencAttr->enType, prcatt->enRcMode, pprc->s32ChangePos);

            CHECK_COMMON_RC_PARAM(pprc);
            CHECK_MIN_MAX("s32ChangePos", pprc->s32ChangePos, CVI_H26X_CHANGE_POS_MIN,
                          CVI_H26X_CHANGE_POS_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32MinStillPercent", pprc->s32MinStillPercent,
                          CVI_H26X_MIN_STILL_PERCENT_MIN,
                          CVI_H26X_MIN_STILL_PERCENT_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("u32MaxStillQP", pprc->u32MaxStillQP, pprc->u32MinQp,
                          pprc->u32MaxQp, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("u32MotionSensitivity", pprc->u32MotionSensitivity,
                          CVI_H26X_MOTION_SENSITIVITY_MIN,
                          CVI_H26X_MOTION_SENSITIVITY_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32AvbrFrmLostOpen", pprc->s32AvbrFrmLostOpen,
                          CVI_H26X_AVBR_FRM_LOST_OPEN_MIN,
                          CVI_H26X_AVBR_FRM_LOST_OPEN_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32AvbrFrmGap", pprc->s32AvbrFrmGap,
                          CVI_H26X_AVBR_FRM_GAP_MIN, CVI_H26X_AVBR_FRM_GAP_MAX,
                          CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32AvbrPureStillThr", pprc->s32AvbrPureStillThr,
                          CVI_H26X_AVBR_PURE_STILL_THR_MIN,
                          CVI_H26X_AVBR_PURE_STILL_THR_MAX, CVI_ERR_VENC_RC_PARAM);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H264UBR)
        {
            const VENC_PARAM_H264_UBR_S *pprc = &pstRcParam->stParamH264Ubr;

            CHECK_COMMON_RC_PARAM(pprc);
        }
    }
    else if (pVencAttr->enType == PT_H265)
    {
        if (prcatt->enRcMode == VENC_RC_MODE_H265CBR)
        {
            const VENC_PARAM_H265_CBR_S *pprc = &pstRcParam->stParamH265Cbr;

            CHECK_COMMON_RC_PARAM(pprc);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265VBR)
        {
            const VENC_PARAM_H265_VBR_S *pprc = &pstRcParam->stParamH265Vbr;

            CHECK_COMMON_RC_PARAM(pprc);
            CHECK_MIN_MAX("s32ChangePos", pprc->s32ChangePos, CVI_H26X_CHANGE_POS_MIN,
                          CVI_H26X_CHANGE_POS_MAX, CVI_ERR_VENC_RC_PARAM);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265AVBR)
        {
            const VENC_PARAM_H265_AVBR_S *pprc = &pstRcParam->stParamH265AVbr;

            CHECK_COMMON_RC_PARAM(pprc);
            CHECK_MIN_MAX("s32ChangePos", pprc->s32ChangePos, CVI_H26X_CHANGE_POS_MIN,
                          CVI_H26X_CHANGE_POS_MAX, CVI_ERR_VENC_RC_PARAM);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265AVBR)
        {
            const VENC_PARAM_H265_AVBR_S *pprc = &pstRcParam->stParamH265AVbr;

            CHECK_COMMON_RC_PARAM(pprc);
            CHECK_MIN_MAX("s32ChangePos", pprc->s32ChangePos, CVI_H26X_CHANGE_POS_MIN,
                          CVI_H26X_CHANGE_POS_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32MinStillPercent", pprc->s32MinStillPercent,
                          CVI_H26X_MIN_STILL_PERCENT_MIN,
                          CVI_H26X_MIN_STILL_PERCENT_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("u32MaxStillQP", pprc->u32MaxStillQP, pprc->u32MinQp,
                          pprc->u32MaxQp, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("u32MotionSensitivity", pprc->u32MotionSensitivity,
                          CVI_H26X_MOTION_SENSITIVITY_MIN,
                          CVI_H26X_MOTION_SENSITIVITY_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32AvbrFrmLostOpen", pprc->s32AvbrFrmLostOpen,
                          CVI_H26X_AVBR_FRM_LOST_OPEN_MIN,
                          CVI_H26X_AVBR_FRM_LOST_OPEN_MAX, CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32AvbrFrmGap", pprc->s32AvbrFrmGap,
                          CVI_H26X_AVBR_FRM_GAP_MIN, CVI_H26X_AVBR_FRM_GAP_MAX,
                          CVI_ERR_VENC_RC_PARAM);
            CHECK_MIN_MAX("s32AvbrPureStillThr", (CVI_S32)pprc->s32AvbrPureStillThr,
                          CVI_H26X_AVBR_PURE_STILL_THR_MIN,
                          CVI_H26X_AVBR_PURE_STILL_THR_MAX, CVI_ERR_VENC_RC_PARAM);
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_H265UBR)
        {
            const VENC_PARAM_H265_UBR_S *pprc = &pstRcParam->stParamH265Ubr;

            CHECK_COMMON_RC_PARAM(pprc);
        }
    }
    else if (pVencAttr->enType == PT_MJPEG)
    {
        if (prcatt->enRcMode == VENC_RC_MODE_MJPEGCBR)
        {
            const VENC_PARAM_MJPEG_CBR_S *pstParamJpgCbr =
                &pstRcParam->stParamMjpegCbr;
            if (pstParamJpgCbr->u32MinQfactor <= Q_TABLE_MIN || pstParamJpgCbr->u32MinQfactor > Q_TABLE_MAX)
            {
                CVI_VENC_ERR("error, %s = %d\n", "cbr u32MinQfactor",
                             pstParamJpgCbr->u32MinQfactor);
                return CVI_ERR_VENC_RC_PARAM;
            }
            if (pstParamJpgCbr->u32MaxQfactor < pstParamJpgCbr->u32MinQfactor || pstParamJpgCbr->u32MaxQfactor > Q_TABLE_MAX)
            {
                CVI_VENC_ERR("error, %s = %d\n", "cbr u32MaxQfactor",
                             pstParamJpgCbr->u32MaxQfactor);
                return CVI_ERR_VENC_RC_PARAM;
            }
        }
        else if (prcatt->enRcMode == VENC_RC_MODE_MJPEGVBR)
        {
            const VENC_PARAM_MJPEG_VBR_S *pstParamJpgVbr =
                &pstRcParam->stParamMjpegVbr;
            if (pstParamJpgVbr->u32MinQfactor <= Q_TABLE_MIN || pstParamJpgVbr->u32MinQfactor > Q_TABLE_MAX)
            {
                CVI_VENC_ERR("error, %s = %d\n", "vbr u32MinQfactor",
                             pstParamJpgVbr->u32MinQfactor);
                return CVI_ERR_VENC_RC_PARAM;
            }

            if (pstParamJpgVbr->u32MaxQfactor < pstParamJpgVbr->u32MinQfactor || pstParamJpgVbr->u32MaxQfactor > Q_TABLE_MAX)
            {
                CVI_VENC_ERR("error, %s = %d\n", "vbr u32MaxQfactor",
                             pstParamJpgVbr->u32MaxQfactor);
                return CVI_ERR_VENC_RC_PARAM;
            }

            if (pstParamJpgVbr->s32ChangePos < 50 || pstParamJpgVbr->s32ChangePos > 100)
            {
                CVI_VENC_ERR("error, %s = %d\n", "vbr s32ChangePos",
                             pstParamJpgVbr->s32ChangePos);
                return CVI_ERR_VENC_RC_PARAM;
            }
        }
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetRefParam(VENC_CHN VeChn, VENC_REF_PARAM_S *pstRefParam)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    memcpy(pstRefParam, &pChnHandle->refParam, sizeof(VENC_REF_PARAM_S));

    return s32Ret;
}

CVI_S32 CVI_VENC_SetRefParam(VENC_CHN                VeChn,
                             const VENC_REF_PARAM_S *pstRefParam)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    memcpy(&pChnHandle->refParam, pstRefParam, sizeof(VENC_REF_PARAM_S));

    pEncCtx = &pChnHandle->encCtx;

    enType = pChnHandle->pChnAttr->stVencAttr.enType;

    if (pEncCtx->base.ioctl && (enType == PT_H264 || enType == PT_H265))
    {
        unsigned int tempLayer = 1;

        if (pstRefParam->u32Base == 1 && pstRefParam->u32Enhance == 1 && pstRefParam->bEnablePred == CVI_TRUE)
            tempLayer = 2;
        else if (pstRefParam->u32Base == 2 && pstRefParam->u32Enhance == 1 && pstRefParam->bEnablePred == CVI_TRUE)
            tempLayer = 3;

        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_REF_PARAM,
                                     (CVI_VOID *)&tempLayer);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_H26X_OP_SET_REF_PARAM, %d\n", s32Ret);
            return -1;
        }
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_SetCuPrediction(VENC_CHN                    VeChn,
                                 const VENC_CU_PREDICTION_S *pstCuPrediction)
{
    CVI_S32               s32Ret     = CVI_SUCCESS;
    venc_chn_context     *pChnHandle = NULL;
    VENC_CU_PREDICTION_S *pcup;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pcup       = &pChnHandle->pChnVars->cuPrediction;

    CHECK_MAX("u32IntraCost", pstCuPrediction->u32IntraCost,
              CVI_H26X_INTRACOST_MAX, CVI_ERR_VENC_CU_PREDICTION);

    memcpy(pcup, pstCuPrediction, sizeof(VENC_CU_PREDICTION_S));

    s32Ret = cviSetCuPredToDrv(pChnHandle);
    if (s32Ret < 0)
    {
        CVI_VENC_ERR("cviSetCuPredToDrv\n");
        return s32Ret;
    }

    return s32Ret;
}

static CVI_S32 cviSetCuPredToDrv(venc_chn_context *pChnHandle)
{
    VENC_CU_PREDICTION_S *pcup    = &pChnHandle->pChnVars->cuPrediction;
    venc_enc_ctx         *pEncCtx = &pChnHandle->encCtx;
    cviPred               pred, *pPred = &pred;
    CVI_S32               s32Ret = CVI_SUCCESS;

    pPred->u32IntraCost = pcup->u32IntraCost;

    s32Ret =
        pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_PREDICT, (CVI_VOID *)pPred);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("CVI_H26X_OP_SET_PREDICT, %d\n", s32Ret);
        return CVI_ERR_VENC_CU_PREDICTION;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetCuPrediction(VENC_CHN              VeChn,
                                 VENC_CU_PREDICTION_S *pstCuPrediction)
{
    CVI_S32               s32Ret     = CVI_SUCCESS;
    venc_chn_context     *pChnHandle = NULL;
    VENC_CU_PREDICTION_S *pcup;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pcup       = &pChnHandle->pChnVars->cuPrediction;

    memcpy(pstCuPrediction, pcup, sizeof(VENC_CU_PREDICTION_S));

    return s32Ret;
}

CVI_S32 CVI_VENC_GetRoiAttr(VENC_CHN VeChn, CVI_U32 u32Index,
                            VENC_ROI_ATTR_S *pstRoiAttr)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    pEncCtx = &pChnHandle->encCtx;

    enType = pChnHandle->pChnAttr->stVencAttr.enType;

    if (pEncCtx->base.ioctl && (enType == PT_H264 || enType == PT_H265))
    {
        cviRoiParam RoiParam;

        memset(&RoiParam, 0x0, sizeof(cviRoiParam));
        RoiParam.roi_index = u32Index;
        s32Ret             = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_GET_ROI_PARAM,
                                                 (CVI_VOID *)&RoiParam);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_H26X_OP_SET_ROI_PARAM, %d\n", s32Ret);
            return -1;
        }
        pstRoiAttr->u32Index         = RoiParam.roi_index;
        pstRoiAttr->bEnable          = RoiParam.roi_enable;
        pstRoiAttr->bAbsQp           = RoiParam.roi_qp_mode;
        pstRoiAttr->s32Qp            = RoiParam.roi_qp;
        pstRoiAttr->stRect.s32X      = RoiParam.roi_rect_x;
        pstRoiAttr->stRect.s32Y      = RoiParam.roi_rect_y;
        pstRoiAttr->stRect.u32Width  = RoiParam.roi_rect_width;
        pstRoiAttr->stRect.u32Height = RoiParam.roi_rect_height;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_SetFrameLostStrategy(VENC_CHN                VeChn,
                                      const VENC_FRAMELOST_S *pstFrmLostParam)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    PAYLOAD_TYPE_E    enType;
    venc_enc_ctx     *pEncCtx;

    CVI_VENC_API("\n");

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    enType  = pChnHandle->pChnAttr->stVencAttr.enType;
    pEncCtx = &pChnHandle->encCtx;

    if (enType == PT_H264 || enType == PT_H265)
    {
        if (pEncCtx->base.ioctl)
        {
            cviFrameLost frameLostSetting;

            if (pstFrmLostParam->bFrmLostOpen)
            {
                // If frame lost is on, check parameters
                if (pstFrmLostParam->enFrmLostMode != FRMLOST_PSKIP)
                {
                    CVI_VENC_ERR("frame lost mode = %d, unsupport\n",
                                 (int)(pstFrmLostParam->enFrmLostMode));
                    return CVI_ERR_VENC_NOT_SUPPORT;
                }

                frameLostSetting.frameLostMode    = pstFrmLostParam->enFrmLostMode;
                frameLostSetting.u32EncFrmGaps    = pstFrmLostParam->u32EncFrmGaps;
                frameLostSetting.bFrameLostOpen   = pstFrmLostParam->bFrmLostOpen;
                frameLostSetting.u32FrmLostBpsThr = pstFrmLostParam->u32FrmLostBpsThr;
            }
            else
            {
                // set gap and threshold back to 0
                frameLostSetting.frameLostMode    = pstFrmLostParam->enFrmLostMode;
                frameLostSetting.u32EncFrmGaps    = 0;
                frameLostSetting.bFrameLostOpen   = 0;
                frameLostSetting.u32FrmLostBpsThr = 0;
            }
            CVI_VENC_CFG(
                "bFrameLostOpen = %d, frameLostMode = %d, u32EncFrmGaps = %d, "
                "u32FrmLostBpsThr = %d\n",
                frameLostSetting.bFrameLostOpen, frameLostSetting.frameLostMode,
                frameLostSetting.u32EncFrmGaps, frameLostSetting.u32FrmLostBpsThr);
            memcpy(&pChnHandle->pChnVars->frameLost, pstFrmLostParam,
                   sizeof(VENC_FRAMELOST_S));

            MUTEX_LOCK(pChnHandle->chnMutex);
            s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_FRAME_LOST_STRATEGY,
                                         (CVI_VOID *)(&frameLostSetting));
            MUTEX_UNLOCK(pChnHandle->chnMutex);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_H26X_OP_SET_FRAME_LOST_STRATEGY, %d\n", s32Ret);
                return s32Ret;
            }
        }
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_SetRoiAttr(VENC_CHN VeChn, const VENC_ROI_ATTR_S *pstRoiAttr)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    pEncCtx = &pChnHandle->encCtx;

    enType = pChnHandle->pChnAttr->stVencAttr.enType;

    if (pEncCtx->base.ioctl && (enType == PT_H264 || enType == PT_H265))
    {
        cviRoiParam RoiParam;

        if (enType == PT_H265)
        {
            if (!pstRoiAttr->bAbsQp && (pstRoiAttr->s32Qp > 0 || pstRoiAttr->s32Qp < -8))
            {
                CVI_VENC_ERR("[H265] if ROI MODE == 0, -8 <= QP < 0\n");
                return -1;
            }
            if (pChnHandle->pChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265FIXQP)
            {
                CVI_VENC_ERR("[H265]ROI does not support FIX QP mode.\n");
                return -1;
            }
        }
        memset(&RoiParam, 0x0, sizeof(cviRoiParam));
        RoiParam.roi_index       = pstRoiAttr->u32Index;
        RoiParam.roi_enable      = pstRoiAttr->bEnable;
        RoiParam.roi_qp_mode     = pstRoiAttr->bAbsQp;
        RoiParam.roi_qp          = pstRoiAttr->s32Qp;
        RoiParam.roi_rect_x      = pstRoiAttr->stRect.s32X;
        RoiParam.roi_rect_y      = pstRoiAttr->stRect.s32Y;
        RoiParam.roi_rect_width  = pstRoiAttr->stRect.u32Width;
        RoiParam.roi_rect_height = pstRoiAttr->stRect.u32Height;

        s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_ROI_PARAM,
                                     (CVI_VOID *)&RoiParam);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_H26X_OP_SET_ROI_PARAM, %d\n", s32Ret);
            return -1;
        }

        if (MUTEX_LOCK(pChnHandle->chnShmMutex) != 0)
        {
            CVI_VENC_ERR("can not lock chnShmMutex\n");
            return CVI_FAILURE;
        }
        memcpy(&pChnHandle->pChnVars->stRoiAttr[pstRoiAttr->u32Index], pstRoiAttr,
               sizeof(VENC_ROI_ATTR_S));
        MUTEX_UNLOCK(pChnHandle->chnShmMutex);
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetFrameLostStrategy(VENC_CHN          VeChn,
                                      VENC_FRAMELOST_S *pstFrmLostParam)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    if (pChnHandle->pChnVars)
    {
        memcpy(pstFrmLostParam, &pChnHandle->pChnVars->frameLost,
               sizeof(VENC_FRAMELOST_S));
    }
    else
    {
        s32Ret = CVI_ERR_VENC_NOT_CONFIG;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_SetChnParam(VENC_CHN                VeChn,
                             const VENC_CHN_PARAM_S *pstChnParam)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_setchnparam(VeChn, pstChnParam);
    }
#endif
    CVI_S32            s32Ret = CVI_SUCCESS;
    venc_chn_context  *pChnHandle;
    venc_chn_vars     *pChnVars;
    RECT_S            *prect;
    venc_enc_ctx_base *pvecb;

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    prect      = &pChnVars->stChnParam.stCropCfg.stRect;
    pvecb      = &pChnHandle->encCtx.base;

    memcpy(&pChnVars->stChnParam, pstChnParam, sizeof(VENC_CHN_PARAM_S));

    if ((prect->s32X & 0xF) || (prect->s32Y & 0xF))
    {
        prect->s32X &= (~0xF);
        prect->s32Y &= (~0xF);
        CVI_VENC_TRACE("s32X = %d, s32Y = %d\n", prect->s32X, prect->s32Y);
    }

    pvecb->x = prect->s32X;
    pvecb->y = prect->s32Y;

    return s32Ret;
}

CVI_S32 CVI_VENC_GetChnParam(VENC_CHN VeChn, VENC_CHN_PARAM_S *pstChnParam)
{
#ifdef RPC_MULTI_PROCESS
    if (!isMaster())
    {
        return rpc_client_venc_getchnparam(VeChn, pstChnParam);
    }
#endif
    CVI_S32           s32Ret = CVI_SUCCESS;
    VENC_CHN_PARAM_S *pvcp;
    venc_chn_context *pChnHandle;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pvcp       = &pChnHandle->pChnVars->stChnParam;

    memcpy(pstChnParam, pvcp, sizeof(VENC_CHN_PARAM_S));

    return s32Ret;
}

static CVI_S32 cviVencJpegInsertUserData(venc_chn_context *pChnHandle,
                                         CVI_U8 *pu8Data, CVI_U32 u32Len)
{
    CVI_S32         ret;
    venc_enc_ctx   *pEncCtx = &pChnHandle->encCtx;
    cviJpegUserData stUserData;

    stUserData.userData = pu8Data;
    stUserData.len      = u32Len;

    MUTEX_LOCK(pChnHandle->chnMutex);
    ret = pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_SET_USER_DATA,
                              (CVI_VOID *)(&stUserData));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS)
        CVI_VENC_ERR("jpeg failed to insert user data, %d", ret);
    return ret;
}

static CVI_S32 cviVencH26xInsertUserData(venc_chn_context *pChnHandle,
                                         CVI_U8 *pu8Data, CVI_U32 u32Len)
{
    CVI_S32       ret;
    venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;
    cviUserData   stUserData;

    stUserData.userData = pu8Data;
    stUserData.len      = u32Len;

    MUTEX_LOCK(pChnHandle->chnMutex);
    ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_USER_DATA,
                              (CVI_VOID *)(&stUserData));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS)
        CVI_VENC_ERR("h26x failed to insert user data, %d", ret);
    return ret;
}

CVI_S32 CVI_VENC_InsertUserData(VENC_CHN VeChn, CVI_U8 *pu8Data,
                                CVI_U32 u32Len)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pu8Data == NULL || u32Len == 0)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no user data\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    if (enType == PT_JPEG || enType == PT_MJPEG)
    {
        s32Ret = cviVencJpegInsertUserData(pChnHandle, pu8Data, u32Len);
    }
    else if (enType == PT_H264 || enType == PT_H265)
    {
        s32Ret = cviVencH26xInsertUserData(pChnHandle, pu8Data, u32Len);
    }

    if (s32Ret != CVI_SUCCESS)
        CVI_VENC_ERR("failed to insert user data %d", s32Ret);
    return s32Ret;
}

static CVI_S32 cviVencH264SetEntropy(venc_chn_context    *pChnHandle,
                                     VENC_H264_ENTROPY_S *pstH264Entropy)
{
    CVI_S32       ret;
    venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);
    ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_H264_ENTROPY,
                              (CVI_VOID *)(pstH264Entropy));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS) CVI_VENC_ERR("failed to set h264 entropy, %d", ret);

    return ret;
}

CVI_S32 CVI_VENC_SetH264Entropy(VENC_CHN                   VeChn,
                                const VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
    CVI_S32             s32Ret = CVI_FAILURE;
    venc_chn_context   *pChnHandle;
    VENC_ATTR_S        *pVencAttr;
    venc_enc_ctx       *pEncCtx;
    VENC_H264_ENTROPY_S h264Entropy;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH264EntropyEnc == NULL)
    {
        CVI_VENC_ERR("no user data\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pVencAttr  = &pChnHandle->pChnAttr->stVencAttr;
    pEncCtx    = &pChnHandle->encCtx;

    if (pVencAttr->enType != PT_H264)
    {
        CVI_VENC_ERR("not h264 encode channel\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    if (pVencAttr->u32Profile == H264E_PROFILE_BASELINE)
    {
        if (pstH264EntropyEnc->u32EntropyEncModeI != H264E_ENTROPY_CAVLC || pstH264EntropyEnc->u32EntropyEncModeP != H264E_ENTROPY_CAVLC)
        {
            CVI_VENC_ERR("h264 Baseline Profile only supports CAVLC\n");
            return CVI_ERR_VENC_NOT_SUPPORT;
        }
    }

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    memcpy(&h264Entropy, pstH264EntropyEnc, sizeof(VENC_H264_ENTROPY_S));
    s32Ret = cviVencH264SetEntropy(pChnHandle, &h264Entropy);

    if (s32Ret == CVI_SUCCESS)
    {
        memcpy(&pChnHandle->h264Entropy, &h264Entropy, sizeof(VENC_H264_ENTROPY_S));
    }
    else
    {
        CVI_VENC_ERR("failed to set h264 entropy %d", s32Ret);
        s32Ret = CVI_ERR_VENC_H264_ENTROPY;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetH264Entropy(VENC_CHN             VeChn,
                                VENC_H264_ENTROPY_S *pstH264EntropyEnc)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    PAYLOAD_TYPE_E    enType;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH264EntropyEnc == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no h264 entropy param\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H264)
    {
        CVI_VENC_ERR("not h264 encode channel\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    memcpy(pstH264EntropyEnc, &pChnHandle->h264Entropy,
           sizeof(VENC_H264_ENTROPY_S));
    return CVI_SUCCESS;
}

static CVI_S32 cviVencH264SetTrans(venc_chn_context  *pChnHandle,
                                   VENC_H264_TRANS_S *pstH264Trans)
{
    CVI_S32       ret;
    venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);
    ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_H264_TRANS,
                              (CVI_VOID *)(pstH264Trans));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS) CVI_VENC_ERR("failed to set h264 trans %d", ret);

    return ret;
}

CVI_S32 CVI_VENC_SetH264Trans(VENC_CHN                 VeChn,
                              const VENC_H264_TRANS_S *pstH264Trans)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;
    VENC_H264_TRANS_S h264Trans;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH264Trans == NULL)
    {
        CVI_VENC_ERR("no h264 trans param\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H264)
    {
        CVI_VENC_ERR("not h264 encode channel\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    memcpy(&h264Trans, pstH264Trans, sizeof(VENC_H264_TRANS_S));
    s32Ret = cviVencH264SetTrans(pChnHandle, &h264Trans);

    if (s32Ret == CVI_SUCCESS)
    {
        memcpy(&pChnHandle->h264Trans, &h264Trans, sizeof(VENC_H264_TRANS_S));
    }
    else
    {
        CVI_VENC_ERR("failed to set h264 trans %d", s32Ret);
        s32Ret = CVI_ERR_VENC_H264_TRANS;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetH264Trans(VENC_CHN VeChn, VENC_H264_TRANS_S *pstH264Trans)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    PAYLOAD_TYPE_E    enType;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH264Trans == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no h264 trans param\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H264)
    {
        CVI_VENC_ERR("not h264 encode channel\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    memcpy(pstH264Trans, &pChnHandle->h264Trans, sizeof(VENC_H264_TRANS_S));
    return CVI_SUCCESS;
}

static CVI_S32 cviVencH265SetTrans(venc_chn_context  *pChnHandle,
                                   VENC_H265_TRANS_S *pstH265Trans)
{
    CVI_S32       ret;
    venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);
    ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_H265_TRANS,
                              (CVI_VOID *)(pstH265Trans));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS) CVI_VENC_ERR("failed to set h265 trans %d", ret);

    return ret;
}

CVI_S32 CVI_VENC_SetH265Trans(VENC_CHN                 VeChn,
                              const VENC_H265_TRANS_S *pstH265Trans)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;
    VENC_H265_TRANS_S h265Trans;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH265Trans == NULL)
    {
        CVI_VENC_ERR("no h265 trans param\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H265)
    {
        CVI_VENC_ERR("not h265 encode channel\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_ERR_VENC_NULL_PTR;
    }

    memcpy(&h265Trans, pstH265Trans, sizeof(VENC_H265_TRANS_S));
    s32Ret = cviVencH265SetTrans(pChnHandle, &h265Trans);

    if (s32Ret == CVI_SUCCESS)
    {
        memcpy(&pChnHandle->h265Trans, &h265Trans, sizeof(VENC_H265_TRANS_S));
    }
    else
    {
        CVI_VENC_ERR("failed to set h265 trans %d", s32Ret);
        s32Ret = CVI_ERR_VENC_H265_TRANS;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetH265Trans(VENC_CHN VeChn, VENC_H265_TRANS_S *pstH265Trans)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    PAYLOAD_TYPE_E    enType;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH265Trans == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no h265 trans param\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H265)
    {
        CVI_VENC_ERR("not h265 encode channel\n");
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    memcpy(pstH265Trans, &pChnHandle->h265Trans, sizeof(VENC_H265_TRANS_S));
    return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_SetSuperFrameStrategy(
    VENC_CHN VeChn, const VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_chn_vars    *pChnVars = NULL;
    venc_enc_ctx     *pEncCtx;
    cviSuperFrame     super, *pSuper = &super;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstSuperFrmParam == NULL)
    {
        CVI_VENC_ERR("pstSuperFrmParam = NULL\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle                  = handle->chn_handle[VeChn];
    pChnVars                    = pChnHandle->pChnVars;
    pEncCtx                     = &pChnHandle->encCtx;
    pSuper->enSuperFrmMode      = (pstSuperFrmParam->enSuperFrmMode == SUPERFRM_NONE)
                                      ? CVI_SUPERFRM_NONE
                                      : CVI_SUPERFRM_REENCODE_IDR;
    pSuper->u32SuperIFrmBitsThr = pstSuperFrmParam->u32SuperIFrmBitsThr;
    pSuper->u32SuperPFrmBitsThr = pstSuperFrmParam->u32SuperPFrmBitsThr;

    MUTEX_LOCK(pChnHandle->chnMutex);
    s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_SUPER_FRAME,
                                 (CVI_VOID *)pSuper);
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (s32Ret == CVI_SUCCESS)
    {
        memcpy(&pChnVars->stSuperFrmParam, pstSuperFrmParam,
               sizeof(VENC_SUPERFRAME_CFG_S));
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetSuperFrameStrategy(
    VENC_CHN VeChn, VENC_SUPERFRAME_CFG_S *pstSuperFrmParam)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_chn_vars    *pChnVars = NULL;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstSuperFrmParam == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("pstSuperFrmParam = NULL\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;

    memcpy(pstSuperFrmParam, &pChnVars->stSuperFrmParam,
           sizeof(VENC_SUPERFRAME_CFG_S));
    return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_CalcFrameParam(VENC_CHN            VeChn,
                                VENC_FRAME_PARAM_S *pstFrameParam)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    cviFrameParam     frmp, *pfp = &frmp;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstFrameParam == NULL)
    {
        CVI_VENC_ERR("pstFrameParam is NULL\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_FAILURE;
    }

    MUTEX_LOCK(pChnHandle->chnMutex);
    s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_CALC_FRAME_PARAM,
                                 (CVI_VOID *)(pfp));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("failed to calculate FrameParam %d", s32Ret);
        s32Ret = CVI_ERR_VENC_FRAME_PARAM;
    }

    pstFrameParam->u32FrameQp   = pfp->u32FrameQp;
    pstFrameParam->u32FrameBits = pfp->u32FrameBits;
    return s32Ret;
}

CVI_S32 CVI_VENC_SetFrameParam(VENC_CHN                  VeChn,
                               const VENC_FRAME_PARAM_S *pstFrameParam)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    venc_chn_vars    *pChnVars;
    cviFrameParam     frmp, *pfp = &frmp;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstFrameParam == NULL)
    {
        CVI_VENC_ERR("pstFrameParam is NULL\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    pEncCtx    = &pChnHandle->encCtx;

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_FAILURE;
    }

    pfp->u32FrameQp   = pstFrameParam->u32FrameQp;
    pfp->u32FrameBits = pstFrameParam->u32FrameBits;
    CHECK_MAX("u32FrameQp", pfp->u32FrameQp, CVI_H26X_FRAME_QP_MAX,
              CVI_ERR_VENC_ILLEGAL_PARAM);
    CHECK_MIN_MAX("u32FrameBits", pfp->u32FrameBits, CVI_H26X_FRAME_BITS_MIN,
                  CVI_H26X_FRAME_BITS_MAX, CVI_ERR_VENC_ILLEGAL_PARAM);

    MUTEX_LOCK(pChnHandle->chnMutex);
    s32Ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_FRAME_PARAM,
                                 (CVI_VOID *)(pfp));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (s32Ret == CVI_SUCCESS)
    {
        memcpy(&pChnVars->frameParam, pstFrameParam, sizeof(VENC_FRAME_PARAM_S));
    }
    else
    {
        CVI_VENC_ERR("failed to set FrameParam %d", s32Ret);
        s32Ret = CVI_ERR_VENC_FRAME_PARAM;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetFrameParam(VENC_CHN            VeChn,
                               VENC_FRAME_PARAM_S *pstFrameParam)
{
    CVI_S32             s32Ret = CVI_FAILURE;
    venc_chn_context   *pChnHandle;
    venc_chn_vars      *pChnVars;
    VENC_FRAME_PARAM_S *pfp;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstFrameParam == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no param\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;
    pfp        = &pChnVars->frameParam;

    memcpy(pstFrameParam, pfp, sizeof(VENC_FRAME_PARAM_S));

    return s32Ret;
}

#define CHECK_VUI_ASPECT_RATIO(st, err)                                          \
    CVI_VENC_CFG(                                                                \
        "aspect_ratio_info_present_flag %u, aspect_ratio_idc %u, sar_width %u, " \
        "sar_height %u\n",                                                       \
        st.aspect_ratio_info_present_flag, st.aspect_ratio_idc, st.sar_width,    \
        st.sar_height);                                                          \
    CVI_VENC_CFG(                                                                \
        "overscan_info_present_flag %u, overscan_appropriate_flag %u\n",         \
        st.overscan_info_present_flag, st.overscan_appropriate_flag);            \
    CHECK_MAX("aspect_ratio_info_present_flag",                                  \
              st.aspect_ratio_info_present_flag, 1, err);                        \
    if (st.aspect_ratio_info_present_flag)                                       \
    {                                                                            \
        CHECK_MAX("aspect_ratio_idc", st.aspect_ratio_idc,                       \
                  CVI_H26X_ASPECT_RATIO_IDC_MAX, err);                           \
        CHECK_MIN_MAX("sar_width", st.sar_width, CVI_H26X_SAR_WIDTH_MIN,         \
                      CVI_H26X_SAR_WIDTH_MAX, err);                              \
        CHECK_MIN_MAX("sar_height", st.sar_height, CVI_H26X_SAR_HEIGHT_MIN,      \
                      CVI_H26X_SAR_HEIGHT_MAX, err);                             \
    }                                                                            \
    CHECK_MAX("overscan_info_present_flag", st.overscan_info_present_flag, 1,    \
              err);                                                              \
    if (st.overscan_info_present_flag)                                           \
    {                                                                            \
        CHECK_MAX("overscan_appropriate_flag", st.overscan_appropriate_flag, 1,  \
                  err);                                                          \
    }

#define CHECK_VUI_H264_TIMING_INFO(st, err)                                     \
    CVI_VENC_CFG(                                                               \
        "timing_info_present_flag %u, fixed_frame_rate_flag %u, "               \
        "num_units_in_tick %u, time_scale %u\n",                                \
        st.timing_info_present_flag, st.fixed_frame_rate_flag,                  \
        st.num_units_in_tick, st.time_scale);                                   \
    CHECK_MAX("timing_info_present_flag", st.timing_info_present_flag, 1, err); \
    if (st.timing_info_present_flag)                                            \
    {                                                                           \
        CHECK_MAX("fixed_frame_rate_flag", st.fixed_frame_rate_flag, 1, err);   \
        CHECK_MIN_MAX("num_units_in_tick", st.num_units_in_tick,                \
                      CVI_H26X_NUM_UNITS_IN_TICK_MIN,                           \
                      CVI_H26X_NUM_UNITS_IN_TICK_MAX, err);                     \
        CHECK_MIN_MAX("time_scale", st.time_scale, CVI_H26X_TIME_SCALE_MIN,     \
                      CVI_H26X_TIME_SCALE_MAX, err);                            \
    }

#define CHECK_VUI_H265_TIMING_INFO(st, err)                                     \
    CVI_VENC_CFG(                                                               \
        "timing_info_present_flag %u, num_units_in_tick %u, time_scale %u\n",   \
        st.timing_info_present_flag, st.num_units_in_tick, st.time_scale);      \
    CVI_VENC_CFG("num_ticks_poc_diff_one_minus1 %u\n",                          \
                 st.num_ticks_poc_diff_one_minus1);                             \
    CHECK_MAX("timing_info_present_flag", st.timing_info_present_flag, 1, err); \
    if (st.timing_info_present_flag)                                            \
    {                                                                           \
        CHECK_MIN_MAX("num_units_in_tick", st.num_units_in_tick,                \
                      CVI_H26X_NUM_UNITS_IN_TICK_MIN,                           \
                      CVI_H26X_NUM_UNITS_IN_TICK_MAX, err);                     \
        CHECK_MIN_MAX("time_scale", st.time_scale, CVI_H26X_TIME_SCALE_MIN,     \
                      CVI_H26X_TIME_SCALE_MAX, err);                            \
        CHECK_MAX("num_ticks_poc_diff_one_minus1",                              \
                  st.num_ticks_poc_diff_one_minus1,                             \
                  CVI_H265_NUM_TICKS_POC_DIFF_ONE_MINUS1_MAX, err);             \
    }

#define CHECK_VUI_VIDEO_SIGNAL(st, video_format_max, err)                      \
    CVI_VENC_CFG(                                                              \
        "video_signal_type_present_flag %u, video_format %d, "                 \
        "video_full_range_flag %u\n",                                          \
        st.video_signal_type_present_flag, st.video_format,                    \
        st.video_full_range_flag);                                             \
    CVI_VENC_CFG(                                                              \
        "colour_description_present_flag %u, colour_primaries %u, "            \
        "transfer_characteristics %u, matrix_coefficients %u\n",               \
        st.colour_description_present_flag, st.colour_primaries,               \
        st.transfer_characteristics, st.matrix_coefficients);                  \
    CHECK_MAX("video_signal_type_present_flag",                                \
              st.video_signal_type_present_flag, 1, err);                      \
    if (st.video_signal_type_present_flag)                                     \
    {                                                                          \
        CHECK_MAX("video_format", st.video_format, video_format_max, err);     \
        CHECK_MAX("video_full_range_flag", st.video_full_range_flag, 1, err);  \
        if (st.colour_description_present_flag)                                \
        {                                                                      \
            CHECK_MAX("colour_description_present_flag",                       \
                      st.colour_description_present_flag, 1, err);             \
            CHECK_MAX("colour_primaries", st.colour_primaries,                 \
                      CVI_H26X_COLOUR_PRIMARIES_MAX, err);                     \
            CHECK_MAX("transfer_characteristics", st.transfer_characteristics, \
                      CVI_H26X_TRANSFER_CHARACTERISTICS_MAX, err);             \
            CHECK_MAX("matrix_coefficients", st.matrix_coefficients,           \
                      CVI_H26X_MATRIX_COEFFICIENTS_MAX, err);                  \
        }                                                                      \
    }

#define CHECK_VUI_BITSTREAM_RESTRIC(st, err)                                  \
    CVI_VENC_CFG("bitstream_restriction_flag %u\n",                           \
                 st.bitstream_restriction_flag);                              \
    CHECK_MAX("bitstream_restriction_flag", st.bitstream_restriction_flag, 1, \
              err);

static CVI_S32 _cviCheckH264VuiParam(const VENC_H264_VUI_S *pstH264Vui)
{
    CHECK_VUI_ASPECT_RATIO(pstH264Vui->stVuiAspectRatio, CVI_ERR_VENC_H264_VUI);
    CHECK_VUI_H264_TIMING_INFO(pstH264Vui->stVuiTimeInfo, CVI_ERR_VENC_H264_VUI);
    CHECK_VUI_VIDEO_SIGNAL(pstH264Vui->stVuiVideoSignal,
                           CVI_H264_VIDEO_FORMAT_MAX, CVI_ERR_VENC_H264_VUI);
    CHECK_VUI_BITSTREAM_RESTRIC(pstH264Vui->stVuiBitstreamRestric,
                                CVI_ERR_VENC_H264_VUI);
    return CVI_SUCCESS;
}

static CVI_S32 _cviCheckH265VuiParam(const VENC_H265_VUI_S *pstH265Vui)
{
    CHECK_VUI_ASPECT_RATIO(pstH265Vui->stVuiAspectRatio, CVI_ERR_VENC_H265_VUI);
    CHECK_VUI_H265_TIMING_INFO(pstH265Vui->stVuiTimeInfo, CVI_ERR_VENC_H265_VUI);
    CHECK_VUI_VIDEO_SIGNAL(pstH265Vui->stVuiVideoSignal,
                           CVI_H265_VIDEO_FORMAT_MAX, CVI_ERR_VENC_H265_VUI);
    CHECK_VUI_BITSTREAM_RESTRIC(pstH265Vui->stVuiBitstreamRestric,
                                CVI_ERR_VENC_H265_VUI);
    return CVI_SUCCESS;
}

static CVI_S32 _cviSetH264Vui(venc_chn_context *pChnHandle,
                              VENC_H264_VUI_S  *pstH264Vui)
{
    CVI_S32       ret;
    venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);
    ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_H264_VUI,
                              (CVI_VOID *)(pstH264Vui));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS) CVI_VENC_ERR("failed to set h264 vui %d", ret);

    return ret;
}

static CVI_S32 _cviSetH265Vui(venc_chn_context *pChnHandle,
                              VENC_H265_VUI_S  *pstH265Vui)
{
    CVI_S32       ret;
    venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);
    ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_H265_VUI,
                              (CVI_VOID *)(pstH265Vui));
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS) CVI_VENC_ERR("failed to set h265 vui %d", ret);

    return ret;
}

CVI_S32 CVI_VENC_SetH264Vui(VENC_CHN VeChn, const VENC_H264_VUI_S *pstH264Vui)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;
    VENC_H264_VUI_S   h264Vui;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH264Vui == NULL)
    {
        CVI_VENC_ERR("no h264 vui param\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H264)
    {
        CVI_VENC_ERR("not h264 encode channel\n");
        return CVI_FAILURE;
    }

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_FAILURE;
    }

    s32Ret = _cviCheckH264VuiParam(pstH264Vui);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("invalid h264 vui param, %d\n", s32Ret);
        return s32Ret;
    }

    memcpy(&h264Vui, pstH264Vui, sizeof(VENC_H264_VUI_S));
    s32Ret = _cviSetH264Vui(pChnHandle, &h264Vui);

    if (s32Ret == CVI_SUCCESS)
    {
        memcpy(&pChnHandle->h264Vui, &h264Vui, sizeof(VENC_H264_VUI_S));
    }
    else
    {
        CVI_VENC_ERR("failed to set h264 vui %d", s32Ret);
        s32Ret = CVI_ERR_VENC_H264_VUI;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_SetH265Vui(VENC_CHN VeChn, const VENC_H265_VUI_S *pstH265Vui)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    venc_enc_ctx     *pEncCtx;
    PAYLOAD_TYPE_E    enType;
    VENC_H265_VUI_S   h265Vui;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH265Vui == NULL)
    {
        CVI_VENC_ERR("no h265 vui param\n");
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pEncCtx    = &pChnHandle->encCtx;
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H265)
    {
        CVI_VENC_ERR("not h265 encode channel\n");
        return CVI_FAILURE;
    }

    if (!pEncCtx->base.ioctl)
    {
        CVI_VENC_ERR("base.ioctl is NULL\n");
        return CVI_FAILURE;
    }

    s32Ret = _cviCheckH265VuiParam(pstH265Vui);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("invalid h265 vui param, %d\n", s32Ret);
        return s32Ret;
    }

    memcpy(&h265Vui, pstH265Vui, sizeof(VENC_H265_VUI_S));
    s32Ret = _cviSetH265Vui(pChnHandle, &h265Vui);

    if (s32Ret == CVI_SUCCESS)
    {
        memcpy(&pChnHandle->h265Vui, &h265Vui, sizeof(VENC_H265_VUI_S));
    }
    else
    {
        CVI_VENC_ERR("failed to set h265 vui %d", s32Ret);
        s32Ret = CVI_ERR_VENC_H265_VUI;
    }

    return s32Ret;
}

CVI_S32 CVI_VENC_GetH264Vui(VENC_CHN VeChn, VENC_H264_VUI_S *pstH264Vui)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    PAYLOAD_TYPE_E    enType;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH264Vui == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no h264 vui param\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H264)
    {
        CVI_VENC_ERR("not h264 encode channel\n");
        return CVI_FAILURE;
    }

    memcpy(pstH264Vui, &pChnHandle->h264Vui, sizeof(VENC_H264_VUI_S));
    return CVI_SUCCESS;
}

CVI_S32 CVI_VENC_GetH265Vui(VENC_CHN VeChn, VENC_H265_VUI_S *pstH265Vui)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    venc_chn_context *pChnHandle;
    PAYLOAD_TYPE_E    enType;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstH265Vui == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no h265 vui param\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    enType     = pChnHandle->pChnAttr->stVencAttr.enType;

    if (enType != PT_H265)
    {
        CVI_VENC_ERR("not h265 encode channel\n");
        return CVI_FAILURE;
    }

    memcpy(pstH265Vui, &pChnHandle->h265Vui, sizeof(VENC_H265_VUI_S));
    return CVI_SUCCESS;
}

CVI_S32 cviSetSbSetting(VENC_SB_Setting *pstSbSetting)
{
    CVI_S32 s32Ret         = CVI_SUCCESS;
    CVI_U32 srcHeightAlign = 0;
    int     reg            = 0;

    CVI_VENC_API("\n");

    // Set Register 0x0
    reg  = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);
    reg &= ~0xFF00FFF0;

    // sb_nb:3, sbc1_mode:2, //sbc1_frm_start:1
    if (pstSbSetting->codec & 0x1) reg |= (pstSbSetting->sb_mode << 4);

    if (pstSbSetting->codec & 0x2) reg |= (pstSbSetting->sb_mode << 8);

    if (pstSbSetting->codec & 0x4) reg |= (pstSbSetting->sb_mode << 12);

    reg |= (pstSbSetting->sb_size << 17);
    reg |= (pstSbSetting->sb_nb << 24);

    if ((pstSbSetting->sb_ybase1 != 0) && (pstSbSetting->sb_uvbase1 != 0))
        reg |= (1 << 19); // 1: sbc0/sbc1 sync with interface0 and  sbc2  sync with
                          // interface1

    cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x00 = 0x%x\n", reg);

    // pri interface
    if ((pstSbSetting->sb_ybase != 0) && (pstSbSetting->sb_uvbase != 0))
    {
        // pri address setting /////////////////////////////////////////////
        // Set Register 0x20
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x20);
        reg = ((pstSbSetting->src_height + 63) >> 6) << 22;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x20, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x20 = 0x%x\n", reg);

        // Set Register 0x24
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x24);
        reg = ((pstSbSetting->y_stride >> 4) << 4) + ((pstSbSetting->uv_stride >> 4) << 20);

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x24, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x24 = 0x%x\n", reg);

        // Set Register 0x28   src y base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x28);
        reg = 0x80000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x28, reg);
        pstSbSetting->src_ybase = reg;

        CVI_VENC_INFO("psbSetting->src_ybase  = 0x%x\n", pstSbSetting->src_ybase);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x28 = 0x%x\n", reg);

        // Set Register 0x2C   src y end addr
        srcHeightAlign = ((pstSbSetting->src_height + 15) >> 4) << 4;
        reg            = 0x80000000 + srcHeightAlign * pstSbSetting->y_stride;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x2C, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x2C = 0x%x\n", reg);

        // Set Register 0x30   sb y base addr
        reg = pstSbSetting->sb_ybase;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x30, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x30 = 0x%x\n", reg);

        // Set Register 0x34   src c base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x34);
        reg = 0x88000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x34, reg);
        pstSbSetting->src_uvbase = reg;
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x34 = 0x%x\n", reg);

        // Set Register 0x38   src c end addr
        reg = 0x88000000 + srcHeightAlign * pstSbSetting->y_stride / 2;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x38, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x38 = 0x%x\n", reg);

        // Set Register 0x3C   sb c base addr
        reg = pstSbSetting->sb_uvbase;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x3C, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x3C = 0x%x\n", reg);
    }

    // sec interface
    if ((pstSbSetting->sb_ybase1 != 0) && (pstSbSetting->sb_uvbase1 != 0))
    {
        // sec address setting /////////////////////////////////////////////
        // Set Register 0x40
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x40);
        reg = ((pstSbSetting->src_height + 63) >> 6) << 22;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x40, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x40 = 0x%x\n", reg);

        // Set Register 0x44
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x44);
        reg = ((pstSbSetting->y_stride >> 4) << 4) + ((pstSbSetting->uv_stride >> 4) << 20);

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x44, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x44 = 0x%x\n", reg);

        // Set Register 0x48   src y base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x48);
        reg = 0x90000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x48, reg);
        pstSbSetting->src_ybase1 = reg;

        CVI_VENC_INFO("psbSetting->src_ybase1  = 0x%x\n", pstSbSetting->src_ybase1);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x48 = 0x%x\n", reg);

        // Set Register 0x4C   src y end addr
        srcHeightAlign = ((pstSbSetting->src_height + 15) >> 4) << 4;
        reg            = 0x90000000 + srcHeightAlign * pstSbSetting->y_stride;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x4C, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x4C = 0x%x\n", reg);

        // Set Register 0x50   sb y base addr
        reg = pstSbSetting->sb_ybase1;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x50, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x50 = 0x%x\n", reg);

        // Set Register 0x54   src c base addr
        reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x54);
        reg = 0x98000000;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x54, reg);
        pstSbSetting->src_uvbase1 = reg;
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x54 = 0x%x\n", reg);

        // Set Register 0x58   src c end addr
        reg = 0x98000000 + srcHeightAlign * pstSbSetting->y_stride / 2;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x58, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x58 = 0x%x\n", reg);

        // Set Register 0x5C   sb c base addr
        reg = pstSbSetting->sb_uvbase1;

        cvi_vc_drv_write_vc_reg(REG_SBM, 0x5C, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x5C = 0x%x\n", reg);
    }

    return s32Ret;
}

CVI_S32 cviStartSb(VENC_SB_Setting *pstSbSetting)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    int     reg    = 0;

    CVI_VENC_API("\n");

    // Set Register 0x0
    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);

    // frm_start:1
    if (pstSbSetting->codec & 0x1) reg |= (1 << 7);

    if (pstSbSetting->codec & 0x2) reg |= (1 << 11);

    if (pstSbSetting->codec & 0x4) reg |= (1 << 15);

    cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);

    CVI_VENC_INFO("VC_REG_BANK_SBM 0x00 = 0x%x\n", reg);

    return s32Ret;
}

#if 0
CVI_S32 cviUpdateSbWptr(VENC_SB_Setting *pstSbSetting)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int sw_mode = 0;
	int reg = 0;

	CVI_VENC_API("\n");

	reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);

	if (pstSbSetting->codec & 0x1)
		sw_mode = (reg & 0x30) >> 4;
	else if (pstSbSetting->codec & 0x2)
		sw_mode = (reg & 0x300) >> 8;
	else if (pstSbSetting->codec & 0x4)
		sw_mode = (reg & 0x3000) >> 12;

	if (sw_mode == 3) { // SW mode
		// Set Register 0x0c
		int wptr = 0;

		reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x88);
		CVI_VENC_INFO("VC_REG_BANK_SBM 0x88 = 0x%x\n", reg);

		wptr = (reg >> 16) & 0x1F;

		reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x0C);
		reg = (reg & 0xFFFFFFE0) | wptr;

		cvi_vc_drv_write_vc_reg(REG_SBM, 0x0C, reg);
		CVI_VENC_INFO("VC_REG_BANK_SBM 0x0C = 0x%x\n", reg);
	}

	return s32Ret;
}
#endif

static int _cviVEncSbEnDummyPush(VENC_SB_Setting *pstSbSetting)
{
    int ret = 0;
    int reg = 0;

    // Enable sb dummy push
    reg  = cvi_vc_drv_read_vc_reg(REG_SBM, 0x14);
    reg |= 0x1; // reg_pri_push_ow_en bit0
    cvi_vc_drv_write_vc_reg(REG_SBM, 0x14, reg);
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x14 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x14));

    return ret;
}

static int _cviVEncSbTrigDummyPush(VENC_SB_Setting *pstSbSetting)
{
    int ret          = 0;
    int reg          = 0;
    int pop_cnt_pri  = 0;
    int push_cnt_pri = 0;

    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x94);
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n", reg);

    push_cnt_pri = reg & 0x3F;
    pop_cnt_pri  = (reg >> 16) & 0x3F;
    CVI_VENC_INFO("push_cnt_pri=%d, pop_cnt_pri=%d\n", push_cnt_pri, pop_cnt_pri);

    if (push_cnt_pri == pop_cnt_pri)
    {
        reg  = cvi_vc_drv_read_vc_reg(REG_SBM, 0x14);
        reg |= 0x4; // reg_pri_push_ow bit2
        cvi_vc_drv_write_vc_reg(REG_SBM, 0x14, reg);
        CVI_VENC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n",
                      cvi_vc_drv_read_vc_reg(REG_SBM, 0x94));
    }

    return ret;
}

static int _cviVEncSbDisDummyPush(VENC_SB_Setting *pstSbSetting)
{
    int ret = 0;
    int reg = 0;

    reg  = cvi_vc_drv_read_vc_reg(REG_SBM, 0x14);
    reg &= (~0x1); // reg_pri_push_ow_en bit0
    cvi_vc_drv_write_vc_reg(REG_SBM, 0x14, reg);
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x14 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x14));

    return ret;
}

static int _cviVEncSbGetSkipFrmStatus(VENC_SB_Setting *pstSbSetting)
{
    int ret          = 0;
    int reg          = 0;
    int pop_cnt_pri  = 0;
    int push_cnt_pri = 0;
    int target_slice_cnt;

    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x94);
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n", reg);

    push_cnt_pri = reg & 0x3F;
    pop_cnt_pri  = (reg >> 16) & 0x3F;

    if (pstSbSetting->sb_size == 0)
        target_slice_cnt = (pstSbSetting->src_height + 63) / 64;
    else
        target_slice_cnt = (pstSbSetting->src_height + 127) / 128;

    CVI_VENC_INFO(
        "push_cnt_pri=%d, pop_cnt_pri=%d, src_height=%d, target_slice_cnt=%d\n",
        push_cnt_pri, pop_cnt_pri, pstSbSetting->src_height, target_slice_cnt);

    if ((pop_cnt_pri == target_slice_cnt) || (push_cnt_pri > target_slice_cnt))
    {
        ret = 1;
        CVI_VENC_INFO(
            "psbSetting->src_height=%d, psbSetting->sb_size=%d, "
            "target_slice_cnt=%d\n",
            pstSbSetting->src_height, pstSbSetting->sb_size, target_slice_cnt);
    }

    return ret;
}

CVI_S32 cviSbSkipOneFrm(VENC_CHN VeChn, VENC_SB_Setting *pstSbSetting)
{
    CVI_S32           s32Ret     = CVI_SUCCESS;
    venc_chn_context *pChnHandle = NULL;
    // venc_enc_ctx *pEncCtx;
    // cviVencSbSetting sbSetting;

    // venc_chn_context *pChnHandleDummy = NULL;
    // venc_enc_ctx *pEncCtxDummy;

#if 0
	VIDEO_FRAME_INFO_S stFrame;
	VENC_STREAM_S stStream;
	VENC_CHN_STATUS_S stStat;
#endif

    CVI_VENC_API("VeChn = %d\n", VeChn);

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];

    // pEncCtx = &pChnHandle->encCtx;

    // pChnHandleDummy = handle->chn_handle[2];
    // pEncCtxDummy = &pChnHandleDummy->encCtx;

    // sbSetting.codec = pstSbSetting->codec;
    // sbSetting.sb_mode = pstSbSetting->sb_mode;
    // sbSetting.sb_nb = pstSbSetting->sb_nb;
    // sbSetting.sb_size = pstSbSetting->sb_size;
    // sbSetting.sb_uvbase = pstSbSetting->sb_uvbase;
    // sbSetting.sb_ybase = pstSbSetting->sb_ybase;
    // sbSetting.y_stride = pstSbSetting->y_stride;
    // sbSetting.uv_stride = pstSbSetting->uv_stride;
    // sbSetting.src_height = pstSbSetting->src_height;

    // sbSetting.src_ybase = pstSbSetting->src_ybase;
    // sbSetting.src_uvbase = pstSbSetting->src_uvbase;

#if 0
	memset(&stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
	memset(&stStream, 0, sizeof(VENC_STREAM_S));

	CVI_VENC_QueryStatus(VeChn, &stStat);

	stStream.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
#endif

    s32Ret                 = _cviVEncSbEnDummyPush(pstSbSetting);
    pChnHandle->bSbSkipFrm = true;

    while (1)
    {
        s32Ret = _cviVEncSbTrigDummyPush(pstSbSetting);
        // sbSetting.status = 0;
        s32Ret = _cviVEncSbGetSkipFrmStatus(pstSbSetting);
        if (s32Ret == 1)
        {
            CVI_VENC_INFO("[%s][%d] Sb Skip Frame done\n", __func__, __LINE__);
            break;
        }
    }

#if 0
	free(stStream.pstPack);
	stStream.pstPack = NULL;
#endif

    pChnHandle->bSbSkipFrm = false;
    s32Ret                 = _cviVEncSbDisDummyPush(pstSbSetting);

    return s32Ret;
}

CVI_S32 cviResetSb(VENC_SB_Setting *pstSbSetting)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    int     reg    = 0;

    CVI_VENC_API("\n");

    CVI_VENC_INFO("Before sw reset sb =================\n");
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x00 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x00));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x80 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x80));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x84 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x84));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x88 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x88));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x90 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x90));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x94));

    // Reset VC SB ctrl
    reg = cvi_vc_drv_read_vc_reg(REG_SBM, 0x00);
#if 1
    reg |= 0x8; // reset all
    cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);
#else
    if (pstSbSetting->codec & 0x1)
    { // h265
        reg |= 0x1;
        cvi_vc_drv_write_vc_reg(REG_SBM, 0x00, reg);
    }
    else if (pstSbSetting->codec & 0x2)
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

    CVI_VENC_INFO("After sw reset sb =================\n");
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x00 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x00));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x80 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x80));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x84 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x84));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x88 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x88));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x90 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x90));
    CVI_VENC_INFO("VC_REG_BANK_SBM 0x94 = 0x%x\n",
                  cvi_vc_drv_read_vc_reg(REG_SBM, 0x94));

    return s32Ret;
}

static CVI_S32 cviSetSBMEnable(venc_chn_context *pChnHandle, CVI_BOOL bSBMEn)
{
    CVI_S32       ret;
    venc_enc_ctx *pEncCtx = &pChnHandle->encCtx;

    MUTEX_LOCK(pChnHandle->chnMutex);

    if (pEncCtx->base.ioctl)
    {
        if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265 || pChnHandle->pChnAttr->stVencAttr.enType == PT_H264)
        {
            ret = pEncCtx->base.ioctl(pEncCtx, CVI_H26X_OP_SET_SBM_ENABLE,
                                      (CVI_VOID *)&bSBMEn);
            if (ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_H26X_OP_SET_SBM_ENABLE, %d\n", ret);
                MUTEX_UNLOCK(pChnHandle->chnMutex);
                return -1;
            }
        }
        else
        {
            ret = pEncCtx->base.ioctl(pEncCtx, CVI_JPEG_OP_SET_SBM_ENABLE,
                                      (CVI_VOID *)&bSBMEn);
            if (ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_JPEG_OP_SET_SBM_ENABLE, %d\n", ret);
                MUTEX_UNLOCK(pChnHandle->chnMutex);
                return -1;
            }
        }
    }
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    if (ret != CVI_SUCCESS) CVI_VENC_ERR("failed to set sbm enable %d", ret);

    return ret;
}

#define ONLY_SEC_JPG 0

CVI_S32 cvi_VENC_CB_SendFrame(CVI_S32 VpssChn1, MMF_BIND_DEST_S stBindDest,
                              const struct cvi_buffer *pstInFrmBuf,
                              const struct cvi_buffer *pstInFrmBuf1,
                              CVI_U32 sb_nb, CVI_S32 s32MilliSec)
{
    CVI_S32            s32Ret        = CVI_FAILURE;
    venc_chn_context  *pChnHandle    = NULL;
    venc_chn_context  *pSecChnHandle = NULL;
    VIDEO_FRAME_INFO_S stVirtVideoFrame;
    VIDEO_FRAME_S     *pstVFrame = &stVirtVideoFrame.stVFrame;

    VIDEO_FRAME_INFO_S stVirtVideoFrame1;
    VIDEO_FRAME_S     *pstVFrame1 = &stVirtVideoFrame1.stVFrame;

#if BYPASS_SB_MODE == 0
    VENC_SB_Setting stSbSetting;
#endif
    int        i          = 0;
    MMF_CHN_S *pstChn     = {0};
    CVI_S32    priChn     = -1;
    CVI_S32    secChn     = -1;
    CVI_BOOL   SnapEnable = false;

    if (handle == NULL)
    {
        CVI_VENC_ERR("Call VENC Destroy before Create, failed\n");
        return CVI_ERR_VENC_UNEXIST;
    }

    for (i = 0; i < stBindDest.u32Num; i++)
    {
        pstChn = &stBindDest.astMmfChn[i];
        CVI_VENC_INFO("s32DevId = %d, s32ChnId = %d\n", pstChn->s32DevId,
                      pstChn->s32ChnId);

        if (pstChn->enModId != CVI_ID_VENC) continue;

        if (handle->chn_status[pstChn->s32ChnId] != VENC_CHN_STATE_START_ENC)
            continue;

        pChnHandle = handle->chn_handle[pstChn->s32ChnId];

        if ((pChnHandle->pChnAttr->stVencAttr.enType == PT_H264) || (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265))
        {
            priChn = pstChn->s32ChnId;
        }

        if ((pChnHandle->pChnAttr->stVencAttr.enType == PT_JPEG) || (pChnHandle->pChnAttr->stVencAttr.enType == PT_MJPEG))
        {
            if (stBindDest.u32Num == 1)
                priChn = pstChn->s32ChnId;
            else
                secChn = pstChn->s32ChnId;
        }
    }

    CVI_VENC_INFO("priChn = %d, secChn = %d\n", priChn, secChn);

    if ((priChn == -1) && (secChn == -1))
    {
        CVI_VENC_ERR("(priChn == -1) && (secChn == -1)\n");
        return CVI_FAILURE;
    }

    if (priChn != -1)
    {
        s32Ret = check_chn_handle(priChn);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("check_chn_handle(%d), ret:%d\n", priChn, s32Ret);
            return s32Ret;
        }

        pChnHandle = handle->chn_handle[priChn];
        CVI_VENC_INFO("u64PhyAddr[0]=0x%lx, u64PhyAddr[1]=0x%lx\n",
                      pstInFrmBuf->phy_addr[0], pstInFrmBuf->phy_addr[1]);
        CVI_VENC_INFO("u32Stride[0]=%d,u32Stride[1]=%d, u32Height=%d\n",
                      pstInFrmBuf->stride[0], pstInFrmBuf->stride[1],
                      pstInFrmBuf->size.u32Height);

        if (pChnHandle->sbm_state != VENC_SBM_STATE_IDLE)
        {
            CVI_VENC_ERR("pChnHandle->sbm_state(%d) != VENC_SBM_STATE_IDLE\n",
                         pChnHandle->sbm_state);
            return CVI_FAILURE;
        }

        pChnHandle->sbm_state = VENC_SBM_STATE_FRM_RUN;
        if (!pChnHandle->pSBMSendFrameThread)
        {
            SEM_INIT(tSbmWaitQueue[priChn]);
// use posix thread
#if 1       // sbm                          \
            // struct sched_param param = { \
            // 	.sched_priority = 80,       \
            // };

            // pChnHandle->pSBMSendFrameThread =
            // kthread_run(venc_sbm_send_frame_thread,
            // (CVI_VOID *)pChnHandle,
            // "venc-sbm-send_frame_thread%d", priChn); if
            // (IS_ERR(pChnHandle->pSBMSendFrameThread)) { 	CVI_VENC_ERR(
            // 		"venc-sbm-send_frame_threadfor chn %d\n",
            // 		priChn);
            // 	return CVI_FAILURE;
            // }
            // sched_setscheduler(pChnHandle->pSBMSendFrameThread, SCHED_RR, &param);

            // pthread_create(&handle->threadDataInput, &attr,
            // venc_monitor_data_input_task, NULL);
#else
            xTaskCreate(venc_sbm_send_frame_thread, "venc_sbm_send_frame_thread",
                        configMINIMAL_STACK_SIZE, (void *)pChnHandle, 3,
                        &pChnHandle->pSBMSendFrameThread);
#endif
        }
        cviSetSBMEnable(pChnHandle, CVI_TRUE);
    }

#if BYPASS_SB_MODE == 0
    if (priChn != -1)
    {
        if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265)
        {
            stSbSetting.codec = 0x1;
        }
        else if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H264)
        {
            stSbSetting.codec = 0x2;
        }
        else
        {
            stSbSetting.codec = 0x4;
        }
    }

    SnapEnable = false;

    if (secChn != -1)
    {
        s32Ret = check_chn_handle(secChn);
        if (s32Ret == CVI_SUCCESS)
        {
            pSecChnHandle = handle->chn_handle[secChn];
            CVI_VENC_INFO("pSecChnHandle->jpgFrmSkipCnt = %d\n",
                          pSecChnHandle->jpgFrmSkipCnt);

            if (pSecChnHandle->jpgFrmSkipCnt > 0)
            {
                stSbSetting.codec |= 0x4;
                SnapEnable         = true;
            }
        }
    }

    stSbSetting.sb_mode = 2;
    stSbSetting.sb_nb   = sb_nb;
    stSbSetting.sb_size = 0; // 64 line

    stSbSetting.y_stride   = pstInFrmBuf->stride[0];
    stSbSetting.uv_stride  = pstInFrmBuf->stride[1];
    stSbSetting.src_height = pstInFrmBuf->size.u32Height;
    stSbSetting.sb_ybase   = pstInFrmBuf->phy_addr[0];
    stSbSetting.sb_uvbase  = pstInFrmBuf->phy_addr[1];
    stSbSetting.sb_ybase1  = 0;
    stSbSetting.sb_uvbase1 = 0;

#if ONLY_SEC_JPG
    stSbSetting.sb_ybase   = 0;
    stSbSetting.sb_uvbase  = 0;
    stSbSetting.sb_ybase1  = pstInFrmBuf->phy_addr[0];
    stSbSetting.sb_uvbase1 = pstInFrmBuf->phy_addr[1];
#endif

    if (VpssChn1 != 0xFFFFFFFF)
    {
        stSbSetting.sb_ybase1   = pstInFrmBuf1->phy_addr[0];
        stSbSetting.sb_uvbase1  = pstInFrmBuf1->phy_addr[1];
        stSbSetting.codec      |= 0x4;
    }

    s32Ret = cviSetSbSetting(&stSbSetting);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviSetSbSetting fail, s32Ret=%d\n", s32Ret);
    }

    s32Ret = cviStartSb(&stSbSetting);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("cviStartSb fail, s32Ret=%d\n", s32Ret);
    }
    memcpy(&pChnHandle->stSbSetting, &stSbSetting, sizeof(VENC_SB_Setting));
#endif // #if BYPASS_SB_MODE == 0

    memset(pstVFrame, 0, sizeof(*pstVFrame));
    pstVFrame->enPixelFormat = pstInFrmBuf->enPixelFormat;
    pstVFrame->u32Width      = pstInFrmBuf->size.u32Width;
    pstVFrame->u32Height     = pstInFrmBuf->size.u32Height;
    pstVFrame->u32Stride[0]  = pstInFrmBuf->stride[0];
    pstVFrame->u32Stride[1]  = pstInFrmBuf->stride[1];
    pstVFrame->u32Stride[2]  = pstInFrmBuf->stride[2];

    pstVFrame->u32TimeRef     = 0;
    pstVFrame->u64PTS         = 0;
    pstVFrame->enDynamicRange = DYNAMIC_RANGE_SDR8;

    pstVFrame->u32Length[0] = pstInFrmBuf->length[0];
    pstVFrame->u32Length[1] = pstInFrmBuf->length[1];

#if BYPASS_SB_MODE
    pstVFrame->u64PhyAddr[0] = pstInFrmBuf->phy_addr[0];
    pstVFrame->u64PhyAddr[1] = pstInFrmBuf->phy_addr[1];
#else

#if ONLY_SEC_JPG
    pstVFrame->u64PhyAddr[0] = stSbSetting.src_ybase1;
    pstVFrame->u64PhyAddr[1] = stSbSetting.src_uvbase1;
    CVI_VENC_INFO(
        "stSbSetting.src_ybase1 = 0x%x, stSbSetting.src_uvbase1 = 0x%x\n",
        stSbSetting.src_ybase1, stSbSetting.src_uvbase1);
#else
    pstVFrame->u64PhyAddr[0] = stSbSetting.src_ybase;
    pstVFrame->u64PhyAddr[1] = stSbSetting.src_uvbase;
    CVI_VENC_INFO("stSbSetting.src_ybase = 0x%x, stSbSetting.src_uvbase = 0x%x\n",
                  stSbSetting.src_ybase, stSbSetting.src_uvbase);
#endif
#endif

    CVI_VENC_INFO(" (P0:0x%lx, P1:0x%lx) (S0:%d, S1:%d)\n",
                  pstVFrame->u64PhyAddr[0], pstVFrame->u64PhyAddr[1],
                  pstVFrame->u32Stride[0], pstVFrame->u32Stride[1]);
    CVI_VENC_INFO(" (W:%d, H:%d)\n", pstVFrame->u32Width, pstVFrame->u32Height);

    if (priChn != -1)
    {
        s32Ret = CVI_VENC_SendFrame(priChn, &stVirtVideoFrame, s32MilliSec);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_VENC_SendFrame(%d) fail, s32Ret=%d\n", priChn, s32Ret);
        }
        memcpy(&pChnHandle->stVideoFrameInfo, &stVirtVideoFrame,
               sizeof(VIDEO_FRAME_INFO_S));
    }

    if (VpssChn1 != 0xFFFFFFFF)
    {
        secChn = 1;

        memset(pstVFrame1, 0, sizeof(*pstVFrame1));
        pstVFrame1->enPixelFormat = pstInFrmBuf1->enPixelFormat;
        pstVFrame1->u32Width      = pstInFrmBuf1->size.u32Width;
        pstVFrame1->u32Height     = pstInFrmBuf1->size.u32Height;
        pstVFrame1->u32Stride[0]  = pstInFrmBuf1->stride[0];
        pstVFrame1->u32Stride[1]  = pstInFrmBuf1->stride[1];
        pstVFrame1->u32Stride[2]  = pstInFrmBuf1->stride[2];

        pstVFrame1->u32TimeRef     = 0;
        pstVFrame1->u64PTS         = 0;
        pstVFrame1->enDynamicRange = DYNAMIC_RANGE_SDR8;

        pstVFrame1->u32Length[0] = pstInFrmBuf1->length[0];
        pstVFrame1->u32Length[1] = pstInFrmBuf1->length[1];

        pstVFrame1->u64PhyAddr[0] = stSbSetting.src_ybase1;
        pstVFrame1->u64PhyAddr[1] = stSbSetting.src_uvbase1;
        CVI_VENC_INFO(
            "stSbSetting.src_ybase1 = 0x%x, stSbSetting.src_uvbase1 = 0x%x\n",
            stSbSetting.src_ybase1, stSbSetting.src_uvbase1);

        CVI_VENC_INFO(" frm1 (%dx%d), %d, %d, %d\n",
                      stVirtVideoFrame1.stVFrame.u32Height,
                      stVirtVideoFrame1.stVFrame.u32Width,
                      stVirtVideoFrame1.stVFrame.enPixelFormat,
                      stVirtVideoFrame1.stVFrame.u32Stride[0],
                      stVirtVideoFrame1.stVFrame.u32Stride[1]);

        s32Ret = CVI_VENC_SendFrame(secChn, &stVirtVideoFrame1, -1);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("CVI_VENC_SendFrame(%d) fail, s32Ret=%d\n", secChn, s32Ret);
        }
    }
    else
    {
        if (SnapEnable)
        {
            s32Ret = CVI_VENC_SendFrame(secChn, &stVirtVideoFrame, -1);
            if (s32Ret != CVI_SUCCESS)
            {
                CVI_VENC_ERR("CVI_VENC_SendFrame(%d) fail, s32Ret=%d\n", secChn,
                             s32Ret);
            }
        }
    }

    return s32Ret;
}

CVI_S32 cvi_VENC_CB_SkipFrame(CVI_S32 VpssGrp, CVI_S32 VpssChn,
                              CVI_U32 srcImgHeight, CVI_S32 s32MilliSec)
{
    CVI_S32           s32Ret = CVI_FAILURE;
    VENC_SB_Setting   stSbSetting;
    venc_chn_context *pChnHandle;
    CVI_S32           priChn = -1;
    CVI_S32           secChn = -1;
#if 0
	int i = 0;
	MMF_CHN_S encChn = {0};
	MMF_CHN_S chn = {.enModId = CVI_ID_VPSS, .s32DevId = VpssGrp, .s32ChnId = VpssChn};
	MMF_BIND_DEST_S stBindDest;
#endif
    CVI_VENC_API("\n");

    if (VpssChn < 0)
    {
        CVI_VENC_ERR("VpssChn(%d)\n", VpssChn);
        return s32Ret;
    }
#if 0
	if (sys_get_bindbysrc(&chn, &stBindDest) != CVI_SUCCESS) {
		CVI_VENC_ERR("sys_get_bindbysrc fail\n");
		return CVI_FAILURE;
	}

	if (stBindDest.astMmfChn[0].enModId != CVI_ID_VENC) {
		CVI_VENC_ERR("next Mod(%d) is not vcodec\n", stBindDest.astMmfChn[0].enModId);
		return CVI_FAILURE;
	}

	CVI_VENC_INFO("stBindDest.u32Num = %d\n", stBindDest.u32Num);

	if (stBindDest.u32Num > 2) {
		CVI_VENC_ERR("vpss chn(%d) sbm bind num %d > 2\n", VpssChn, stBindDest.u32Num);
		return CVI_FAILURE;
	}

	for (i = 0; i < stBindDest.u32Num; i++) {
		encChn = stBindDest.astMmfChn[i];
		CVI_VENC_INFO("s32DevId = %d, s32ChnId = %d\n", encChn.s32DevId, encChn.s32ChnId);

		if (handle->chn_status[encChn.s32ChnId] != VENC_CHN_STATE_START_ENC)
			continue;

		pChnHandle = handle->chn_handle[encChn.s32ChnId];

		if ((pChnHandle->pChnAttr->stVencAttr.enType == PT_H264) ||
			(pChnHandle->pChnAttr->stVencAttr.enType == PT_H265)) {
			priChn = encChn.s32ChnId;
		}

		if ((pChnHandle->pChnAttr->stVencAttr.enType == PT_JPEG) ||
			(pChnHandle->pChnAttr->stVencAttr.enType == PT_MJPEG)) {
			if (stBindDest.u32Num == 1)
				priChn = encChn.s32ChnId;
			else
				secChn = encChn.s32ChnId;
		}
	}
#else
    // can't use sys_get_bindbysrc, unbind before stop venc
    priChn = 0;
    secChn = -1;
#endif

    CVI_VENC_INFO("priChn = %d, secChn = %d\n", priChn, secChn);

    if ((priChn == -1) && (secChn == -1))
    {
        CVI_VENC_ERR("(priChn == -1) && (secChn == -1)\n");
        return CVI_FAILURE;
    }

    if (priChn != -1)
    {
        s32Ret = check_chn_handle(priChn);
        if (s32Ret != CVI_SUCCESS)
        {
            CVI_VENC_ERR("check_chn_handle(%d), ret:%d\n", priChn, s32Ret);
            return s32Ret;
        }

        pChnHandle = handle->chn_handle[priChn];
    }

    if (pChnHandle->pChnAttr->stVencAttr.enType == PT_H265)
    {
        stSbSetting.codec = 0x1; // h265
    }
    else
    {
        stSbSetting.codec = 0x2; // h264
    }

    stSbSetting.sb_size    = 0;
    stSbSetting.src_height = srcImgHeight;

    err_frm_skip[priChn] = 1;
    cviSbSkipOneFrm(priChn, &stSbSetting);
    cviResetSb(&stSbSetting);
    if (pChnHandle->sbm_state == VENC_SBM_STATE_FRM_RUN)
        pChnHandle->sbm_state = VENC_SBM_STATE_IDLE;

    return s32Ret;
}

CVI_S32 cvi_VENC_CB_SnapJpgFrame(MMF_BIND_DEST_S stBindDest, CVI_U32 FrmCnt)
{
    CVI_S32           s32Ret     = CVI_FAILURE;
    venc_chn_context *pChnHandle = NULL;
    int               i          = 0;
    MMF_CHN_S        *pJpgChn;

    for (i = 0; i < stBindDest.u32Num; i++)
    {
        pJpgChn    = &stBindDest.astMmfChn[i];
        pChnHandle = handle->chn_handle[pJpgChn->s32ChnId];

        if ((pChnHandle->pChnAttr->stVencAttr.enType == PT_JPEG) || (pChnHandle->pChnAttr->stVencAttr.enType == PT_MJPEG))
        {
            break;
        }
    }

    if (i == stBindDest.u32Num)
    {
        CVI_VENC_ERR("Can't find jpeg encoder\n");
        return CVI_FAILURE;
    }

    if (pChnHandle)
    {
        pChnHandle->jpgFrmSkipCnt = FrmCnt;
        CVI_VENC_INFO("chn(%d), jpgFrmSkipCnt = %d\n", pJpgChn->s32ChnId,
                      pChnHandle->jpgFrmSkipCnt);
    }

    s32Ret = CVI_SUCCESS;

    return s32Ret;
}

CVI_S32 cvi_VENC_Bind(VENC_CHN VeChn)
{
    // CVI_U32 i;
    venc_chn_context       *pChnHandle;
    struct cvi_venc_vb_ctx *pVbCtx;
    CVI_S32                 s32Ret = CVI_SUCCESS;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pVbCtx     = pChnHandle->pVbCtx;

    pVbCtx->enable_bind_mode = CVI_TRUE;

    return s32Ret;
}

CVI_S32 cvi_VENC_UnBind(VENC_CHN VeChn)
{
    // CVI_U32 i;
    venc_chn_context       *pChnHandle;
    struct cvi_venc_vb_ctx *pVbCtx;
    CVI_S32                 s32Ret = CVI_SUCCESS;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pVbCtx     = pChnHandle->pVbCtx;

    pVbCtx->enable_bind_mode = CVI_FALSE;

    return s32Ret;
}

CVI_S32 cvi_VENC_CB_SendVb(VENC_CHN VeChn, struct vb_s *pstVb, CVI_U64 addr)
{
    // CVI_U32 i;
    venc_chn_context *pChnHandle;
    venc_chn_vars    *pChnVars;
    CVI_S32           s32Ret = CVI_SUCCESS;

    s32Ret = check_chn_handle(VeChn);
    if (s32Ret != CVI_SUCCESS)
    {
        CVI_VENC_ERR("check_chn_handle, %d\n", s32Ret);
        return s32Ret;
    }

    if (pstVb == NULL)
    {
        s32Ret = CVI_FAILURE_ILLEGAL_PARAM;
        CVI_VENC_ERR("no h265 vui param\n");
        return s32Ret;
    }

    pChnHandle = handle->chn_handle[VeChn];
    pChnVars   = pChnHandle->pChnVars;

    MUTEX_LOCK(pChnHandle->chnMutex);
    // fifo push vb
    if (!pChnHandle->useVbFlag)
    {
        memcpy(&pChnHandle->vb, pstVb, sizeof(struct vb_s));
        pChnHandle->useVbFlag     = 1;
        pChnHandle->releaseVbAddr = addr;
        cviVenc_sem_post(&pChnVars->sem_vb);
    }
    MUTEX_UNLOCK(pChnHandle->chnMutex);

    return s32Ret;
}
