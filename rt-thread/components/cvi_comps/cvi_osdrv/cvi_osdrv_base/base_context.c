#include "base.h"
#include "base_cb.h"
#include "base_ctx.h"
#include "cvi_debug.h"
#include "sys.h"
#include "vb.h"
#include "vo_cb.h"
#include "vpss_cb.h"
#include <cvi_base.h>
#include <cvi_base_ctx.h>
#include <cvi_buffer.h>
#include <cvi_vi_ctx.h>
#include <cvi_vo_ctx.h>
#include <cvi_vpss_ctx.h>
#include <inttypes.h>
#include <stdlib.h>
#include <vip_atomic.h>
#include <semaphore.h>
#include <rtthread.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct vi_jobs_ctx gViJobs;
struct vpss_jobs_ctx gVpssJobs;
struct vo_jobs_ctx gVoJobs;

struct cvi_vi_ctx *vi_ctx;
struct cvi_vo_ctx *vo_ctx;
// struct cvi_gdc_ctx gdc_ctx;

CVI_S32 (*base_qbuf_cb[CVI_ID_BUTT])(struct cvi_buffer *buf, CVI_U32 param) = {0};
CVI_S32 (*base_dqbuf_cb[CVI_ID_BUTT])(struct cvi_buffer *buf, CVI_U32 param) = {0};

static void _vpss_post_job(CVI_S32 dev_id)
{
    struct base_exe_m_cb exe_cb;

    exe_cb.caller = E_MODULE_BASE;
    exe_cb.callee = E_MODULE_VPSS;
    exe_cb.cmd_id = VPSS_CB_QBUF_TRIGGER;
    exe_cb.data = (void *)&dev_id;

    base_exe_module_cb(&exe_cb);
}

int base_set_mod_ctx(struct mod_ctx_s *ctx_s)
{
    if (!ctx_s || !ctx_s->ctx_info) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "set mod ctx error: data is NULL\n");
        return -1;
    }

    switch (ctx_s->modID) {
    case CVI_ID_VI:
        vi_ctx = (struct cvi_vi_ctx *)ctx_s->ctx_info;
        break;

    case CVI_ID_VPSS:
        break;

    case CVI_ID_VO:
        vo_ctx = (struct cvi_vo_ctx *)ctx_s->ctx_info;
        break;

    default:
        break;
    }

    return 0;
}

struct vb_jobs_t *base_get_jobs_by_chn(MMF_CHN_S chn, enum CHN_TYPE_E chn_type)
{
    if (chn.enModId == CVI_ID_VI) {
        if (chn.s32ChnId < VI_EXT_CHN_START) {
            return &gViJobs.vb_jobs[chn.s32ChnId];
        }
        return &gViJobs.vb_jobs[vi_ctx->stExtChnAttr[chn.s32ChnId - VI_EXT_CHN_START].s32BindChn];
    } else if (chn.enModId == CVI_ID_VO) {
        return &gVoJobs.vb_jobs;
    } else if (chn.enModId == CVI_ID_VPSS) {
        return (chn_type == CHN_TYPE_OUT) ? &gVpssJobs.outs[chn.s32DevId][chn.s32ChnId]
                                          : &gVpssJobs.ins[chn.s32DevId];
    }
    else if (chn.enModId == CVI_ID_VENC) {
        return &venc_vb_ctx[chn.s32ChnId].vb_jobs;
    } else if (chn.enModId == CVI_ID_VDEC) {
        return &vdec_vb_ctx[chn.s32ChnId].vb_jobs;
    }

    return NULL;
}

static struct vbq *_get_doneq(MMF_CHN_S chn)
{
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, CVI_TRUE);

    if (chn.enModId == CVI_ID_VI) {
        if (chn.s32ChnId < VI_EXT_CHN_START)
            return &jobs->doneq;
        else
            return &gViJobs.extchn_doneq[chn.s32ChnId - VI_EXT_CHN_START];
    }

    return &jobs->doneq;
}

CVI_S32 base_fill_videoframe2buffer(MMF_CHN_S chn, const VIDEO_FRAME_INFO_S *pstVideoFrame,
                                    struct cvi_buffer *buf)
{
    CVI_U32 plane_size;
    VB_CAL_CONFIG_S stVbCalConfig;
    CVI_U8 i = 0;

    COMMON_GetPicBufferConfig(pstVideoFrame->stVFrame.u32Width, pstVideoFrame->stVFrame.u32Height,
                              pstVideoFrame->stVFrame.enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,
                              DEFAULT_ALIGN, &stVbCalConfig);

    buf->size.u32Width = pstVideoFrame->stVFrame.u32Width;
    buf->size.u32Height = pstVideoFrame->stVFrame.u32Height;
    buf->enPixelFormat = pstVideoFrame->stVFrame.enPixelFormat;
    buf->s16OffsetLeft = pstVideoFrame->stVFrame.s16OffsetLeft;
    buf->s16OffsetTop = pstVideoFrame->stVFrame.s16OffsetTop;
    buf->s16OffsetRight = pstVideoFrame->stVFrame.s16OffsetRight;
    buf->s16OffsetBottom = pstVideoFrame->stVFrame.s16OffsetBottom;
    buf->frm_num = pstVideoFrame->stVFrame.u32TimeRef;
    buf->u64PTS = pstVideoFrame->stVFrame.u64PTS;
    memset(&buf->frame_crop, 0, sizeof(buf->frame_crop));

    for (i = 0; i < NUM_OF_PLANES; ++i) {
        if (i >= stVbCalConfig.plane_num) {
            buf->phy_addr[i] = 0;
            buf->length[i] = 0;
            buf->stride[i] = 0;
            continue;
        }

        plane_size = (i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize;
        buf->phy_addr[i] = pstVideoFrame->stVFrame.u64PhyAddr[i];
        buf->length[i] = pstVideoFrame->stVFrame.u32Length[i];
        buf->stride[i] = pstVideoFrame->stVFrame.u32Stride[i];
        if (buf->length[i] < plane_size) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "Mod(%s) Dev(%d) Chn(%d) Plane[%d]\n", base_get_modname(chn.enModId), chn.s32DevId, chn.s32ChnId, i);
            CVI_TRACE_SYS(CVI_DBG_ERR, " length(%zu) less than expected(%d).\n", buf->length[i], plane_size);
            return CVI_FAILURE;
        }
        if (buf->stride[i] % DEFAULT_ALIGN) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "Mod(%s) Dev(%d) Chn(%d) Plane[%d]\n", base_get_modname(chn.enModId), chn.s32DevId, chn.s32ChnId, i);
            CVI_TRACE_SYS(CVI_DBG_ERR, " stride(%d) not aligned(%d).\n", buf->stride[i], DEFAULT_ALIGN);
            return CVI_FAILURE;
        }
        if (buf->phy_addr[i] % DEFAULT_ALIGN) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "Mod(%s) Dev(%d) Chn(%d) Plane[%d]\n", base_get_modname(chn.enModId), chn.s32DevId, chn.s32ChnId, i);
            CVI_TRACE_SYS(CVI_DBG_ERR, " address(%lx) not aligned(%d).\n", buf->phy_addr[i], DEFAULT_ALIGN);
            return CVI_FAILURE;
        }
    }
    // [WA-01]
    if (stVbCalConfig.plane_num > 1) {
        if (((buf->phy_addr[0] & (stVbCalConfig.u16AddrAlign - 1)) != (buf->phy_addr[1] & (stVbCalConfig.u16AddrAlign - 1))) || ((buf->phy_addr[0] & (stVbCalConfig.u16AddrAlign - 1)) != (buf->phy_addr[2] & (stVbCalConfig.u16AddrAlign - 1)))) {
            CVI_TRACE_SYS(CVI_DBG_ERR, "Mod(%s) Dev(%d) Chn(%d)\n", base_get_modname(chn.enModId), chn.s32DevId, chn.s32ChnId);
            CVI_TRACE_SYS(CVI_DBG_ERR, "plane address offset (%lx-%lx-%lx)", buf->phy_addr[0], buf->phy_addr[1], buf->phy_addr[2]);
            CVI_TRACE_SYS(CVI_DBG_ERR, "not aligned to %#x.\n", stVbCalConfig.u16AddrAlign);
            return CVI_FAILURE;
        }
    }
    return CVI_SUCCESS;
}

CVI_S32 base_get_chn_buffer(MMF_CHN_S chn, VB_BLK *blk, CVI_S32 timeout_ms)
{
    CVI_S32 ret = CVI_FAILURE;
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, CHN_TYPE_OUT);
    struct vb_s *vb;
    struct vbq *doneq = _get_doneq(chn);
    struct snap_s *s;
    struct timespec abstime;
    pthread_condattr_t cattr;

    if (!jobs) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s), get chn buf fail, jobs NULL\n", base_get_modname(chn.enModId));
        return CVI_FAILURE;
    }

    if (!jobs->inited) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s) get chn buf fail, not inited yet\n", base_get_modname(chn.enModId));
        return CVI_FAILURE;
    }

    pthread_mutex_lock(&jobs->dlock);
    if (!FIFO_EMPTY(doneq)) {
        FIFO_POP(doneq, &vb);
        vb->mod_ids &= ~BIT(chn.enModId);
        vb->mod_ids |= BIT(CVI_ID_USER);
        pthread_mutex_unlock(&jobs->dlock);
        *blk = (VB_BLK)vb;
        return CVI_SUCCESS;
    }

    s = malloc(sizeof(*s));
    if (!s) {
        pthread_mutex_unlock(&jobs->dlock);
        return CVI_FAILURE;
    }

    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    if (pthread_cond_init(&s->cond_queue, &cattr) != 0) {
        pthread_mutex_unlock(&jobs->dlock);
        CVI_TRACE_SYS(CVI_DBG_ERR, "get_chn_buffer cond init failed.");
        goto err_cond;
    }

    s->chn = chn;
    s->blk = VB_INVALID_HANDLE;

    if (timeout_ms < 0) {
        TAILQ_INSERT_TAIL(&jobs->snap_jobs, s, tailq);
        ret = pthread_cond_wait(&s->cond_queue, &jobs->dlock);
        // ret < 0, interrupt by a signal
        // ret = 0, condition true
    } else {
        if (clock_gettime(CLOCK_MONOTONIC, &abstime) != 0) {
            pthread_mutex_unlock(&jobs->dlock);
            CVI_TRACE_SYS(CVI_DBG_ERR, "clock_gettime:");
            goto err_clk;
        }
        CVI_U32 timeout_s = timeout_ms / 1000;

        timeout_ms %= 1000;
        abstime.tv_nsec += timeout_ms * 1000000;
        abstime.tv_sec += (timeout_s + abstime.tv_nsec / 1000000000L);
        abstime.tv_nsec = abstime.tv_nsec % 1000000000L;
        TAILQ_INSERT_TAIL(&jobs->snap_jobs, s, tailq);
        ret = pthread_cond_timedwait(&s->cond_queue, &jobs->dlock, &abstime);
        // ret < 0, interrupted by a signal
        // ret = 0, timeout
        // ret = 1, condition true
    }
    pthread_mutex_unlock(&jobs->dlock);

    if (ret == 0) {
        *blk = s->blk;
    } else {
        pthread_mutex_lock(&jobs->dlock);
        if (s->blk != VB_INVALID_HANDLE)
            vb_release_block(s->blk);
        TAILQ_REMOVE(&jobs->snap_jobs, s, tailq);
        pthread_mutex_unlock(&jobs->dlock);
        CVI_TRACE_SYS(CVI_DBG_ERR, "Mod(%s) Grp(%d) Chn(%d), jobs wait(%d) work(%d) done(%d)\n", base_get_modname(chn.enModId), chn.s32DevId, chn.s32ChnId, FIFO_SIZE(&jobs->waitq), FIFO_SIZE(&jobs->workq), FIFO_SIZE(&jobs->doneq));
    }

err_clk:
    pthread_cond_destroy(&s->cond_queue);
err_cond:
    free(s);
    return ret;
}

/* base_mod_jobs_init: initialize the jobs.
 *
 * @param chn: the channel to be inited.
 * @param chn_type: the chn is input(read) or output(write)
 * @param waitq_depth: the depth for waitq.
 * @param workq_depth: the depth for workq.
 * @param doneq_depth: the depth for doneq.
 */
void base_mod_jobs_init(MMF_CHN_S chn, enum CHN_TYPE_E chn_type,
                        CVI_U8 waitq_depth, CVI_U8 workq_depth, CVI_U8 doneq_depth)
{
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s) job init fail, Null parameter\n", base_get_modname(chn.enModId));
        return;
    }

    if (jobs->inited) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s) job init fail, already inited\n", base_get_modname(chn.enModId));
        return;
    }

    pthread_mutex_init(&jobs->lock, NULL);
    pthread_mutex_init(&jobs->dlock, NULL);
    sem_init(&jobs->sem, 0, 0);
    FIFO_INIT(&jobs->waitq, waitq_depth);
    FIFO_INIT(&jobs->workq, workq_depth);
    FIFO_INIT(&jobs->doneq, doneq_depth);
    TAILQ_INIT(&jobs->snap_jobs);
    jobs->inited = true;
}

/* mod_jobs_exit: end the jobs and release all resources.
 *
 * @param chn: the channel to be exited.
 * @param chn_type: the chn is input(read) or output(write)
 */
void base_mod_jobs_exit(MMF_CHN_S chn, enum CHN_TYPE_E chn_type)
{
    struct vb_s *vb;
    struct snap_s *s, *s_tmp;
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s) job exit fail, Null parameter\n", base_get_modname(chn.enModId));
        return;
    }

    if (!jobs->inited) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s) job exit fail, not inited yet\n", base_get_modname(chn.enModId));
        return;
    }

    pthread_mutex_lock(&jobs->lock);
    while (!FIFO_EMPTY(&jobs->waitq)) {
        FIFO_POP(&jobs->waitq, &vb);
        vb_release_block((VB_BLK)vb);
    }
    FIFO_EXIT(&jobs->waitq);
    while (!FIFO_EMPTY(&jobs->workq)) {
        FIFO_POP(&jobs->workq, &vb);
        vb_release_block((VB_BLK)vb);
    }
    FIFO_EXIT(&jobs->workq);
    pthread_mutex_unlock(&jobs->lock);
    pthread_mutex_destroy(&jobs->lock);

    pthread_mutex_lock(&jobs->dlock);
    while (!FIFO_EMPTY(&jobs->doneq)) {
        FIFO_POP(&jobs->doneq, &vb);
        vb_release_block((VB_BLK)vb);
    }
    FIFO_EXIT(&jobs->doneq);

    TAILQ_FOREACH_SAFE(s, &jobs->snap_jobs, tailq, s_tmp)
    TAILQ_REMOVE(&jobs->snap_jobs, s, tailq);
    pthread_mutex_unlock(&jobs->dlock);
    pthread_mutex_destroy(&jobs->dlock);
    sem_destroy(&jobs->sem);
    jobs->inited = false;
}

/* mod_jobs_enque_work: Put job into work.
 *     Move vb from waitq into workq and put into driver.
 *
 * @param d: vdev which provide info of driver.
 * @param chn: the target channel.
 * @param chn_type: the chn is input(read) or output(write)
 * @return: CVI_SUCCESS if OK.
 */
struct cvi_buffer *base_mod_jobs_enque_work(MMF_CHN_S chn,
                                            enum CHN_TYPE_E chn_type)
{
    struct vb_s *vb;
    CVI_S32 ret = 0;
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "Null parameter\n");
        return NULL;
    }

    pthread_mutex_lock(&jobs->lock);
    if (FIFO_EMPTY(&jobs->waitq)) {
        pthread_mutex_unlock(&jobs->lock);
        CVI_TRACE_SYS(CVI_DBG_ERR, "waitq is empty.\n");
        return NULL;
    }
    if (FIFO_FULL(&jobs->workq)) {
        pthread_mutex_unlock(&jobs->lock);
        CVI_TRACE_SYS(CVI_DBG_ERR, "workq is full.\n");
        return NULL;
    }

    FIFO_POP(&jobs->waitq, &vb);
    FIFO_PUSH(&jobs->workq, vb);
    pthread_mutex_unlock(&jobs->lock);

    CVI_TRACE_SYS(CVI_DBG_DEBUG, "phy-addr(%lx).\n", vb->phy_addr);

    if (ret != 0) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "qbuf error\n");
        return NULL;
    }
    return &vb->buf;
}

/* mod_jobs_waitq_empty: if waitq is empty
 *
 * @param chn: the target channel.
 * @param chn_type: the chn is input(read) or output(write)
 * @return: TRUE if empty.
 */
bool base_mod_jobs_waitq_empty(MMF_CHN_S chn, enum CHN_TYPE_E chn_type)
{
    bool is_empty;
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "Null parameter\n");
        return false;
    }

    pthread_mutex_lock(&jobs->lock);
    is_empty = FIFO_EMPTY(&jobs->waitq);
    pthread_mutex_unlock(&jobs->lock);

    return is_empty;
}

/* mod_jobs_workq_empty: if workq is empty
 *
 * @param chn: the target channel.
 * @param chn_type: the chn is input(read) or output(write)
 * @return: TRUE if empty.
 */
bool base_mod_jobs_workq_empty(MMF_CHN_S chn, enum CHN_TYPE_E chn_type)
{
    bool is_empty;
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "Null parameter\n");
        return false;
    }

    pthread_mutex_lock(&jobs->lock);
    is_empty = FIFO_EMPTY(&jobs->workq);
    pthread_mutex_unlock(&jobs->lock);

    return is_empty;
}

/* mod_jobs_waitq_pop: pop out from waitq.
 *
 * @param chn: the target channel.
 * @param chn_type: the chn is input(read) or output(write)
 * @return: VB_INVALID_HANDLE is not available; o/w, the VB_BLK.
 */
VB_BLK base_mod_jobs_waitq_pop(MMF_CHN_S chn, enum CHN_TYPE_E chn_type)
{
    struct vb_s *p;
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "Null parameter\n");
        return -1;
    }

    pthread_mutex_lock(&jobs->lock);
    if (FIFO_EMPTY(&jobs->waitq)) {
        pthread_mutex_unlock(&jobs->lock);
        CVI_TRACE_SYS(CVI_DBG_ERR, "No more vb in waitq for dequeue.\n");
        return -1;
    }
    FIFO_POP(&jobs->waitq, &p);
    pthread_mutex_unlock(&jobs->lock);
    return (VB_BLK)p;
}

/* mod_jobs_workq_pop: pop out from workq.
 *
 * @param chn: the target channel.
 * @param chn_type: the chn is input(read) or output(write)
 * @return: VB_INVALID_HANDLE is not available; o/w, the VB_BLK.
 */
VB_BLK base_mod_jobs_workq_pop(MMF_CHN_S chn, enum CHN_TYPE_E chn_type)
{
    struct vb_s *p;
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "Null parameter\n");
        return -1;
    }

    pthread_mutex_lock(&jobs->lock);
    if (FIFO_EMPTY(&jobs->workq)) {
        pthread_mutex_unlock(&jobs->lock);
        CVI_TRACE_SYS(CVI_DBG_ERR, "No more vb in workq for dequeue.\n");
        return -1;
    }
    FIFO_POP(&jobs->workq, &p);
    pthread_mutex_unlock(&jobs->lock);
    return (VB_BLK)p;
}

CVI_S32 base_get_frame_info(PIXEL_FORMAT_E fmt, SIZE_S size, struct cvi_buffer *buf, u64 mem_base, u8 align)
{
    VB_CAL_CONFIG_S stVbCalConfig;
    u8 i = 0;

    COMMON_GetPicBufferConfig(size.u32Width, size.u32Height, fmt, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, align, &stVbCalConfig);

    memset(buf, 0, sizeof(*buf));
    buf->size = size;
    buf->enPixelFormat = fmt;
    for (i = 0; i < stVbCalConfig.plane_num; ++i) {
        buf->phy_addr[i] = mem_base;
        buf->length[i] = ALIGN((i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize,
                               stVbCalConfig.u16AddrAlign);
        buf->stride[i] = (i == 0) ? stVbCalConfig.u32MainStride : stVbCalConfig.u32CStride;
        mem_base += buf->length[i];

        CVI_TRACE_SYS(CVI_DBG_DEBUG, "(%lx-%zu-%d)\n", buf->phy_addr[i], buf->length[i], buf->stride[i]);
    }

    return CVI_SUCCESS;
}

/* _handle_snap: if there is get-frame request, hanlde it.
 *
 * @param chn: the channel where the blk is dequeued.
 * @param chn_type: the chn is input(read) or output(write)
 * @param blk: the VB_BLK to handle.
 */
static void _handle_snap(MMF_CHN_S chn, enum CHN_TYPE_E chn_type, VB_BLK blk)
{
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);
    struct vb_s *p = (struct vb_s *)blk;
    struct vbq *doneq;
    struct snap_s *s, *s_tmp;

    if (chn_type != CHN_TYPE_OUT)
        return;

    if (chn.enModId == CVI_ID_VDEC)
        return;

    if (jobs == NULL) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "handle snap fail, Null parameter\n");
        return;
    }

    if (!jobs->inited) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "handle snap fail, job not inited yet\n");
        return;
    }
    pthread_mutex_lock(&jobs->dlock);
    TAILQ_FOREACH_SAFE(s, &jobs->snap_jobs, tailq, s_tmp)
    {
        if (CHN_MATCH(&s->chn, &chn)) {
            TAILQ_REMOVE(&jobs->snap_jobs, s, tailq);
            s->blk = blk;
            atomic_inc(&p->usr_cnt);
            p->mod_ids |= BIT(CVI_ID_USER);
            pthread_cond_signal(&s->cond_queue);
            pthread_mutex_unlock(&jobs->dlock);
            return;
        }
    }
    doneq = _get_doneq(chn);
    // check if there is a snap-queue
    if (FIFO_CAPACITY(doneq)) {
        if (FIFO_FULL(doneq)) {
            struct vb_s *vb = NULL;
            FIFO_POP(doneq, &vb);
            vb->mod_ids &= ~BIT(chn.enModId);
            vb_release_block((VB_BLK)vb);
        }
        atomic_inc(&p->usr_cnt);
        p->mod_ids |= BIT(chn.enModId);
        FIFO_PUSH(doneq, p);
    }
    pthread_mutex_unlock(&jobs->dlock);
}

/* vb_qbuf: queue vb into the specified channel.
 *     (src) Put into workq and driver.
 *     (dst) Put into waitq and sem_post
 *
 * @param chn: the channel to be queued.
 * @param chn_type: the chn is input(read) or output(write)
 * @param blk: VB_BLK to be queued.
 */
CVI_S32 vb_qbuf(MMF_CHN_S chn, enum CHN_TYPE_E chn_type, VB_BLK blk)
{
    struct vb_jobs_t *jobs = base_get_jobs_by_chn(chn, chn_type);
    struct vb_s *vb = (struct vb_s *)blk;
    CVI_S32 ret = CVI_SUCCESS;
    CVI_U32 cb_param = 0;

    CVI_TRACE_SYS(CVI_DBG_DEBUG, "%s dev(%d) chn(%d) chnType(%d): phy-addr(0x%lx) cnt(%d)\n",
                  base_get_modname(chn.enModId), chn.s32DevId, chn.s32ChnId, chn_type,
                  vb->phy_addr, atomic_read(&(vb->usr_cnt)));

    if (!jobs) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s), vb_qbuf fail, error, empty jobs\n", base_get_modname(chn.enModId));
        return -1;
    }
    if (!jobs->inited) {
        CVI_TRACE_SYS(CVI_DBG_ERR, "mod(%s), vb_qbuf fail, jobs not initialized yet\n", base_get_modname(chn.enModId));
        return -1;
    }

    pthread_mutex_lock(&jobs->lock);
    atomic_inc(&vb->usr_cnt);
    pthread_mutex_unlock(&jobs->lock);

    if (chn_type == CHN_TYPE_OUT) {
        pthread_mutex_lock(&jobs->lock);
        if (FIFO_FULL(&jobs->workq)) {
            atomic_dec(&vb->usr_cnt);
            pthread_mutex_unlock(&jobs->lock);
            CVI_TRACE_SYS(CVI_DBG_ERR, "%s workq is full. drop new one.\n", base_get_modname(chn.enModId));
            return -1;
        }
        vb->buf.dev_num = chn.s32ChnId;
        pthread_mutex_unlock(&jobs->lock);

        ///< TODO:
        cb_param = 0xFF;
        if (base_qbuf_cb[chn.enModId]) {
            pthread_mutex_lock(&jobs->lock);
            ret = (*base_qbuf_cb[chn.enModId])(&vb->buf, cb_param);
            if (ret != CVI_SUCCESS) {
                atomic_dec(&vb->usr_cnt);
                pthread_mutex_unlock(&jobs->lock);
                CVI_TRACE_SYS(CVI_DBG_ERR, "%s qbuf_cb fail.", base_get_modname(chn.enModId));
                return ret;
            }
            pthread_mutex_unlock(&jobs->lock);
        }
        pthread_mutex_lock(&jobs->lock);
        FIFO_PUSH(&jobs->workq, vb);
        pthread_mutex_unlock(&jobs->lock);
    } else {
        pthread_mutex_lock(&jobs->lock);
        if (FIFO_FULL(&jobs->waitq)) {
            atomic_dec(&vb->usr_cnt);
            pthread_mutex_unlock(&jobs->lock);
            CVI_TRACE_SYS(CVI_DBG_DEBUG, "grp(%d) %s waitq is full. drop new one.\n", chn.s32DevId, base_get_modname(chn.enModId));
            return -1;
        }
        FIFO_PUSH(&jobs->waitq, vb);
        pthread_mutex_unlock(&jobs->lock);
        // sem_post(&jobs->sem);
        sem_post(&jobs->sem);
        if (chn.enModId == CVI_ID_VPSS) {
            _vpss_post_job(chn.s32DevId);
        }
        else if (chn.enModId == CVI_ID_VENC) {
            pthread_mutex_lock(&venc_vb_ctx[chn.s32ChnId].sem_vb.mutex);
            venc_vb_ctx[chn.s32ChnId].sem_vb.cnt++;
            pthread_cond_signal(&venc_vb_ctx[chn.s32ChnId].sem_vb.cond);
            pthread_mutex_unlock(&venc_vb_ctx[chn.s32ChnId].sem_vb.mutex);
        }
    }

    vb->mod_ids |= BIT(chn.enModId);
    return ret;
}

/* vb_dqbuf: dequeue vb from the specified channel(driver).
 *
 * @param chn: the channel to be dequeued.
 * @param chn_type: the chn is input(read) or output(write)
 * @param blk: the VB_BLK dequeued.
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 vb_dqbuf(MMF_CHN_S chn, enum CHN_TYPE_E chn_type, VB_BLK *blk)
{
    struct vb_jobs_t *jobs;
    struct vb_s *p;
    struct cvi_buffer buf;
    CVI_U32 cb_param = 0;

    *blk = VB_INVALID_HANDLE;

    ///< TODO:
    cb_param = 0xFF;
    if (base_dqbuf_cb[chn.enModId])
        (*base_dqbuf_cb[chn.enModId])(&buf, cb_param);
    jobs = base_get_jobs_by_chn(chn, chn_type);

    pthread_mutex_lock(&jobs->lock);
    // get vb from workq which is done.
    if (FIFO_EMPTY(&jobs->workq)) {
        pthread_mutex_unlock(&jobs->lock);
        CVI_TRACE_SYS(CVI_DBG_ERR, "%s ChnId(%d) No more vb for dequeue.\n",
                      base_get_modname(chn.enModId), chn.s32ChnId);
        return CVI_FAILURE;
    }
    FIFO_POP(&jobs->workq, &p);
    pthread_mutex_unlock(&jobs->lock);
    *blk = (VB_BLK)p;
    p->mod_ids &= ~BIT(chn.enModId);

    return CVI_SUCCESS;
}

/* vb_done_handler: called when vb on specified chn is ready for delivery.
 *    Get vb from chn and deliver to its binding dsts if available;
 *    O/W, release back to vb_pool.
 *
 * @param chn: the chn which has vb to be released
 * @param chn_type: for modules which has both in/out.
 *                True: module generates(output) vb.
 *                False: module take(input) vb.
 * @param blk: VB_BLK.
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 vb_done_handler(MMF_CHN_S chn, enum CHN_TYPE_E chn_type, VB_BLK blk)
{
    MMF_BIND_DEST_S stBindDest;
    CVI_S32 ret;
    CVI_U8 i;

    _handle_snap(chn, chn_type, blk);

    if (chn_type == CHN_TYPE_OUT) {
        if (base_get_bindbysrc(&chn, &stBindDest) == CVI_SUCCESS) {
            for (i = 0; i < stBindDest.u32Num; ++i) {
                vb_qbuf(stBindDest.astMmfChn[i], false, blk);
                CVI_TRACE_SYS(CVI_DBG_DEBUG, " Mod(%s) chn(%d) dev(%d) -> Mod(%s) chn(%d) dev(%d)\n", base_get_modname(chn.enModId), chn.s32ChnId, chn.s32DevId, base_get_modname(stBindDest.astMmfChn[i].enModId), stBindDest.astMmfChn[i].s32ChnId, stBindDest.astMmfChn[i].s32DevId);
            }
        } else {
            // release if not found
            CVI_TRACE_SYS(CVI_DBG_DEBUG, "Mod(%s) chn(%d) dev(%d) src no dst release\n", base_get_modname(chn.enModId), chn.s32ChnId, chn.s32DevId);
        }
    } else {
        CVI_TRACE_SYS(CVI_DBG_DEBUG, "Mod(%s) chn(%d) dev(%d) dst out release\n", base_get_modname(chn.enModId), chn.s32ChnId, chn.s32DevId);
    }
    ret = vb_release_block(blk);

    return ret;
}

CVI_U32 get_diff_in_us(struct timespec t1, struct timespec t2)
{
    struct timespec diff;

    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}
