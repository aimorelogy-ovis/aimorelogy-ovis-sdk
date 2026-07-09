#include "time.h"
#include "base_cb.h"
#include "cvi_errno.h"
#include "cvi_vip_ldc.h"
#include "gdc.h"
#include "ldc.h"
#include "ldc_debug.h"
#include "ldc_platform.h"
#include "mesh.h"
#include "sys.h"
#include "sys_context.h"
#include "vb.h"
#include "vpss_cb.h"
#include "list.h"

#define LDC_YUV_BLACK       0x808000
#define CVI_GDC_TASK_PRIO   (40)
#define SEM_WAIT_TIMEOUT_MS 500

static inline CVI_S32 MOD_CHECK_NULL_PTR(MOD_ID_E mod, const void *ptr)
{
    if (mod >= CVI_ID_BUTT)
        return CVI_FAILURE;
    if (!ptr)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "NULL pointer\n");
        return CVI_ERR_GDC_NULL_PTR;
    }
    return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_GDC_FORMAT(VIDEO_FRAME_INFO_S imgIn, VIDEO_FRAME_INFO_S imgOut)
{
    if (imgIn.stVFrame.enPixelFormat != imgOut.stVFrame.enPixelFormat)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "in/out pixelformat(%d-%d) mismatch\n",
                      imgIn.stVFrame.enPixelFormat,
                      imgOut.stVFrame.enPixelFormat);
        return CVI_ERR_GDC_ILLEGAL_PARAM;
    }
    if (!GDC_SUPPORT_FMT(imgIn.stVFrame.enPixelFormat))
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "pixelformat(%d) unsupported\n",
                      imgIn.stVFrame.enPixelFormat);
        return CVI_ERR_GDC_ILLEGAL_PARAM;
    }
    return CVI_SUCCESS;
}

struct cvi_gdc_proc_ctx *gdc_proc_ctx;

int gdc_handle_to_procIdx(struct cvi_ldc_job *job)
{
    int idx = 0;

    if (!gdc_proc_ctx)
        return -1;

    for (idx = 0; idx < GDC_PROC_JOB_INFO_NUM; ++idx)
    {
        if (gdc_proc_ctx->stJobInfo[idx].hHandle == (u64)(uintptr_t)job && gdc_proc_ctx->stJobInfo[idx].eState == GDC_JOB_WORKING)
            return idx;

        if (gdc_proc_ctx->stJobInfo[idx].eState == GDC_JOB_WORKING)
        {
            CVI_TRACE_LDC(CVI_DBG_INFO, "  [%d] hHandle=0x%lx, job=0x%lx\n", idx,
                          gdc_proc_ctx->stJobInfo[idx].hHandle,
                          (u64)(uintptr_t)job);
        }
    }

    return -1;
}

void gdc_proc_record_hw_start(struct cvi_ldc_job *job)
{
    struct timespec curTime;

    clock_gettime(CLOCK_MONOTONIC, &curTime);
    job->hw_start_time = curTime;
}

void gdc_proc_record_hw_end(struct cvi_ldc_job *job)
{
    int             idx;
    struct timespec start_time;
    struct timespec curTime;

    if (!job)
        return;

    idx = gdc_handle_to_procIdx(job);
    if (idx < 0)
        return;

    /* accumulate every task execution time of one job */
    start_time = job->hw_start_time;

    clock_gettime(CLOCK_MONOTONIC, &curTime);
    gdc_proc_ctx->stJobInfo[idx].u32HwTime = (CVI_U32)get_diff_in_us(start_time, curTime);
}

void gdc_proc_record_job_start(struct cvi_ldc_job *job)
{
    int             idx;
    struct timespec curTime;
    u64             curTimeUs;

    if (!job)
        return;

    idx = gdc_handle_to_procIdx(job);
    if (idx < 0)
        return;

    clock_gettime(CLOCK_MONOTONIC, &curTime);
    curTimeUs = curTime.tv_sec * 1000000 + curTime.tv_nsec / 1000;
    gdc_proc_ctx->stJobInfo[idx].u32BusyTime =
        (u32)(curTimeUs - gdc_proc_ctx->stJobInfo[idx].u64SubmitTime);
    gdc_proc_ctx->stJobStatus.u32ProcingNum = 1;
}

void gdc_proc_record_job_end(struct cvi_ldc_job *job)
{
    int             idx;
    struct timespec curTime;
    u64             curTimeUs;

    if (!job)
        return;

    idx = gdc_handle_to_procIdx(job);
    if (idx < 0)
        return;

    clock_gettime(CLOCK_MONOTONIC, &curTime);
    curTimeUs = curTime.tv_sec * 1000000 + curTime.tv_nsec / 1000;

    gdc_proc_ctx->stJobStatus.u32ProcingNum = 0;
    gdc_proc_ctx->stJobStatus.u32Success++;
    gdc_proc_ctx->stTaskStatus.u32Success +=
        gdc_proc_ctx->stJobInfo[idx].u32TaskNum;

    gdc_proc_ctx->stJobInfo[idx].eState = GDC_JOB_SUCCESS;
    gdc_proc_ctx->stJobInfo[idx].u32CostTime =
        (u32)(curTimeUs - gdc_proc_ctx->stJobInfo[idx].u64SubmitTime);
}

static s32 gdc_rotation_check_size(ROTATION_E                  enRotation,
                                   const struct gdc_task_attr *pstTask)
{
    if (enRotation >= ROTATION_MAX)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
        return CVI_ERR_GDC_ILLEGAL_PARAM;
    }

    if (enRotation == ROTATION_90 || enRotation == ROTATION_270 || enRotation == ROTATION_XY_FLIP)
    {
        if (ALIGN(pstTask->stImgOut.stVFrame.u32Width, DEFAULT_ALIGN << 1) < ALIGN(pstTask->stImgIn.stVFrame.u32Height, DEFAULT_ALIGN << 1))
        {
            CVI_TRACE_LDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input height(%d)'\n",
                          enRotation, pstTask->stImgOut.stVFrame.u32Width,
                          pstTask->stImgIn.stVFrame.u32Height);
            return CVI_ERR_GDC_ILLEGAL_PARAM;
        }
        if (ALIGN(pstTask->stImgOut.stVFrame.u32Height, DEFAULT_ALIGN << 1) < ALIGN(pstTask->stImgIn.stVFrame.u32Width, DEFAULT_ALIGN << 1))
        {
            CVI_TRACE_LDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input width(%d)'\n",
                          enRotation, pstTask->stImgOut.stVFrame.u32Height,
                          pstTask->stImgIn.stVFrame.u32Width);
            return CVI_ERR_GDC_ILLEGAL_PARAM;
        }
    }
    else
    {
        if (ALIGN(pstTask->stImgOut.stVFrame.u32Width, DEFAULT_ALIGN << 1) < ALIGN(pstTask->stImgIn.stVFrame.u32Width, DEFAULT_ALIGN << 1))
        {
            CVI_TRACE_LDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input width(%d)'\n",
                          enRotation, pstTask->stImgOut.stVFrame.u32Width,
                          pstTask->stImgIn.stVFrame.u32Width);
            return CVI_ERR_GDC_ILLEGAL_PARAM;
        }
        if (ALIGN(pstTask->stImgOut.stVFrame.u32Height, DEFAULT_ALIGN << 1) < ALIGN(pstTask->stImgIn.stVFrame.u32Height, DEFAULT_ALIGN << 1))
        {
            CVI_TRACE_LDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input height(%d)'\n",
                          enRotation, pstTask->stImgOut.stVFrame.u32Height,
                          pstTask->stImgIn.stVFrame.u32Height);
            return CVI_ERR_GDC_ILLEGAL_PARAM;
        }
    }

    return CVI_SUCCESS;
}

static void cvi_ldc_submit_hw(struct cvi_ldc_vdev *wdev,
                              struct cvi_ldc_job *job, struct gdc_task *tsk)
{
    struct ldc_cfg          cfg;
    u8                      num_of_plane;
    VIDEO_FRAME_S          *in_frame, *out_frame;
    PIXEL_FORMAT_E          enPixFormat;
    ROTATION_E              enRotation;
    u64                     mesh_addr      = ALIGN(tsk->stTask.au64privateData[0], DEFAULT_ALIGN);
    struct _LDC_BUF_WRAP_S *pstBufWrap     = &tsk->stTask.stBufWrap;
    CVI_U32                 bufWrapDepth   = tsk->stTask.bufWrapDepth;
    u32                     bufWrapPhyAddr = (u32)tsk->stTask.bufWrapPhyAddr;
    u8                      sb_mode, ldc_dir;
    struct ldc_sb_cfg       ldc_sb_cfg;

    in_frame    = &tsk->stTask.stImgIn.stVFrame;
    out_frame   = &tsk->stTask.stImgOut.stVFrame;
    enPixFormat = in_frame->enPixelFormat;
    enRotation  = tsk->enRotation;

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "        enPixFormat=%d, enRotation=%d, mesh_addr=0x%lx\n",
                  enPixFormat, enRotation, mesh_addr);

    mesh_addr = (mesh_addr != DEFAULT_MESH_PADDR) ? mesh_addr : 0;

    memset(&cfg, 0, sizeof(cfg));
    switch (enPixFormat)
    {
    default:
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
        cfg.pixel_fmt = 1;
        num_of_plane  = 2;
        break;
    case PIXEL_FORMAT_YUV_400:
        cfg.pixel_fmt = 0;
        num_of_plane  = 1;
        break;
    };

    /* User space(clock wise) -> LDC hw(counter-clock wise) */
    switch (enRotation)
    {
    case ROTATION_90:
        cfg.dst_mode = LDC_DST_ROT_270;
        break;
    case ROTATION_270:
        cfg.dst_mode = LDC_DST_ROT_90;
        break;
    case ROTATION_XY_FLIP:
        cfg.dst_mode = LDC_DST_XY_FLIP;
        break;
    default:
        cfg.dst_mode = LDC_DST_FLAT;
        break;
    }

    cfg.map_base   = mesh_addr;
    cfg.bgcolor    = (u16)LDC_YUV_BLACK /*ctx->bgcolor*/;
    cfg.src_width  = ALIGN(in_frame->u32Width, 64);
    cfg.src_height = ALIGN(in_frame->u32Height, 64);
    cfg.ras_width  = cfg.src_width;
    cfg.ras_height = cfg.src_height;

    if (cfg.map_base == 0)
        cfg.map_bypass = true;
    else
        cfg.map_bypass = false;

    /* slice buffer mode */
    // if((reg_sb_mode>0 && reg_dst_mode!=Flat) ldc_dir = 1, else ldc_dir = 0
    // reg_src_str = 0
    // reg_src_end = (reg_ldc_dir) ? (reg_src_ysize -1) : (reg_src_xsize - 1)
    sb_mode        = (pstBufWrap->bEnable && bufWrapPhyAddr) ? LDC_SB_FRAME_BASE : LDC_SB_DISABLE;
    ldc_dir        = (sb_mode && cfg.dst_mode != LDC_DST_FLAT) ? 1 : 0;
    cfg.src_xstart = 0;
    cfg.src_xend   = ldc_dir ? in_frame->u32Height - 1 : in_frame->u32Width - 1;

    // TODO: define start/end by crop

    cfg.src_y_base = in_frame->u64PhyAddr[0];
    cfg.dst_y_base = out_frame->u64PhyAddr[0];
    if (num_of_plane == 2)
    {
        cfg.src_c_base = in_frame->u64PhyAddr[1];
        cfg.dst_c_base = out_frame->u64PhyAddr[1];
    }

    if (sb_mode)
    {
        /* rotation only, src height -> dst width */
        cfg.dst_y_base = bufWrapPhyAddr;
        // cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
        cfg.dst_c_base = cfg.dst_y_base + cfg.src_height * pstBufWrap->u32BufLine * bufWrapDepth;
    }

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "        update size src(%d %d)\n",
                  cfg.src_width, cfg.src_height);
    CVI_TRACE_LDC(CVI_DBG_DEBUG, "        update src-buf: 0x%llx-0x%llx\n",
                  in_frame->u64PhyAddr[0], in_frame->u64PhyAddr[1]);
    CVI_TRACE_LDC(CVI_DBG_DEBUG, "        update dst-buf: 0x%x-0x%x\n",
                  cfg.dst_y_base, cfg.dst_c_base);
    CVI_TRACE_LDC(CVI_DBG_DEBUG, "        update mesh_addr(%#lx)\n", mesh_addr);

    tsk->state = GDC_TASK_STATE_RUNNING;

    gdc_proc_record_hw_start(job);

    // ldc_reset();
    ldc_get_sb_default(&ldc_sb_cfg);
    if (sb_mode)
    {
        ldc_sb_cfg.sb_mode    = sb_mode;
        ldc_sb_cfg.sb_size    = (pstBufWrap->u32BufLine / 64) - 1;
        ldc_sb_cfg.sb_nb      = tsk->stTask.bufWrapDepth - 1;
        ldc_sb_cfg.sb_set_str = 1;
        ldc_sb_cfg.ldc_dir    = ldc_dir;
    }
    ldc_set_sb(&ldc_sb_cfg);

    ldc_engine(&cfg);
}

static enum ENUM_MODULES_ID convert_cb_id(MOD_ID_E enModId)
{
    if (enModId == CVI_ID_VI)
        return E_MODULE_VI;
    else if (enModId == CVI_ID_VO)
        return E_MODULE_VO;
    else if (enModId == CVI_ID_VPSS)
        return E_MODULE_VPSS;

    return E_MODULE_BUTT;
}

static void _ldc_op_done_cb(MOD_ID_E enModId, CVI_VOID *pParam, VB_BLK blk)
{
    struct ldc_op_done_cfg cfg;
    struct base_exe_m_cb   exe_cb;
    enum ENUM_MODULES_ID   callee = convert_cb_id(enModId);

    cfg.pParam = pParam;
    cfg.blk    = blk;

    exe_cb.callee = callee;
    exe_cb.caller = E_MODULE_LDC;
    exe_cb.cmd_id = LDC_CB_GDC_OP_DONE;
    exe_cb.data   = &cfg;
    base_exe_module_cb(&exe_cb);
}

static void cvi_ldc_handle_hw_cb(struct cvi_ldc_vdev *wdev, struct cvi_ldc_job *job,
                                 struct gdc_task *tsk, CVI_BOOL is_timeout)
{
    bool     is_internal = (tsk->stTask.reserved == CVI_GDC_MAGIC);
    VB_BLK   blk_in = VB_INVALID_HANDLE, blk_out = VB_INVALID_HANDLE;
    MOD_ID_E enModId = job->enModId;
    CVI_BOOL isLastTask;

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "is_internal=%d\n", is_internal);

    if (is_internal)
    {
        // for internal module handshaking. such as vi/vpss rotation.
        isLastTask = (CVI_BOOL)tsk->stTask.au64privateData[1];

        CVI_TRACE_LDC(CVI_DBG_DEBUG, "isLastTask=%d, blk_in(pa=0x%llx), blk_out(pa=0x%llx)\n",
                      isLastTask,
                      (unsigned long long)tsk->stTask.stImgIn.stVFrame.u64PhyAddr[0],
                      (unsigned long long)tsk->stTask.stImgOut.stVFrame.u64PhyAddr[0]);

        // blk_in = vb_physAddr2Handle(tsk->stTask.stImgIn.stVFrame.u64PhyAddr[0]);
        // blk_out = vb_physAddr2Handle(tsk->stTask.stImgOut.stVFrame.u64PhyAddr[0]);
        blk_in  = (VB_BLK)tsk->stTask.stImgIn.stVFrame.pPrivateData;
        blk_out = (VB_BLK)tsk->stTask.stImgOut.stVFrame.pPrivateData;
        if (blk_out == VB_INVALID_HANDLE)
            CVI_TRACE_LDC(CVI_DBG_ERR, "Can't get valid vb_blk.\n");
        else
        {
            // TODO: not handle timeout yet
            // VB_BLK blk_out_ret = (ret == CVI_SUCCESS) ? blk_out : VB_INVALID_HANDLE;
            VB_BLK blk_out_ret = blk_out;

            ((struct vb_s *)blk_out)->mod_ids &= ~BIT(CVI_ID_GDC);
            if (isLastTask && !is_timeout)
                _ldc_op_done_cb(enModId, (void *)(uintptr_t)tsk->stTask.au64privateData[2],
                                blk_out_ret);

            // if (ret != CVI_SUCCESS)
            //	vb_release_block(blk_out);
        }

        // User space:
        //   Caller always assign callback.
        //   Null callback used for internal gdc sub job.
        // Kernel space:
        //  !isLastTask used for internal gdc sub job.
        if (isLastTask && blk_in != VB_INVALID_HANDLE)
        {
            ((struct vb_s *)blk_in)->mod_ids &= ~BIT(CVI_ID_GDC);
            vb_release_block(blk_in);
        }
    }
}

static int _notify_vpp_sb_mode(struct gdc_task *tsk)
{
    struct base_exe_m_cb    exe_cb;
    struct vpss_grp_sbm_cfg cfg;
    MMF_CHN_S               stSrcChn   = {.enModId = CVI_ID_GDC, .s32DevId = 0, .s32ChnId = 0};
    MMF_BIND_DEST_S         stBindDest = {.u32Num = 0};
    struct _LDC_BUF_WRAP_S *pstBufWrap = &tsk->stTask.stBufWrap;
    int32_t                 ret;

    ret = base_get_bindbysrc(&stSrcChn, &stBindDest);
    if (ret)
    {
        CVI_TRACE_LDC(CVI_DBG_WARN, "fail to find bind dst\n");
        return ret;
    }

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "stBindDest u32Num=%d\n", stBindDest.u32Num);
    if (!stBindDest.u32Num)
        return -1;

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "stBindDest [0]enModId=%d\n", stBindDest.astMmfChn[0].enModId);
    if (stBindDest.astMmfChn[0].enModId != CVI_ID_VPSS)
        return -1;

    memset(&cfg, 0, sizeof(cfg));
    cfg.grp      = stBindDest.astMmfChn[0].s32DevId;
    cfg.sb_mode  = pstBufWrap->bEnable ? LDC_SB_FRAME_BASE : LDC_SB_DISABLE;
    cfg.sb_size  = (pstBufWrap->u32BufLine / 64) - 1;
    cfg.sb_nb    = tsk->stTask.bufWrapDepth - 1;
    cfg.ion_size = pstBufWrap->u32WrapBufferSize;

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "u32BufLine=%d, u32WrapBufferSize=%d, bufWrapDepth=%d\n",
                  pstBufWrap->u32BufLine, pstBufWrap->u32WrapBufferSize, tsk->stTask.bufWrapDepth);

    exe_cb.callee = E_MODULE_VPSS;
    exe_cb.caller = E_MODULE_LDC;
    exe_cb.cmd_id = VPSS_CB_SET_LDC_2_VPSS_SBM;
    exe_cb.data   = &cfg;
    tsk->state    = GDC_TASK_STATE_WAIT_SBM_CFG_DONE;
    ret           = base_exe_module_cb(&exe_cb);

    if (!ret)
    {
        tsk->stTask.bufWrapPhyAddr = cfg.ion_paddr;
        // tsk->state = GDC_TASK_STATE_WAIT_SBM_CFG_DONE;
    }

    return ret;
}

int ldc_vpss_sdm_cb_done(struct cvi_ldc_vdev *wdev)
{
    int                 ret = -1;
    struct cvi_ldc_job *job;
    struct gdc_task    *tsk;
    dlist_t            *tmp, *tmp2;

    if (!wdev)
        return -1;

    dlist_for_each_entry_safe(&wdev->jobq, tmp, job, struct cvi_ldc_job, node)
    {
        dlist_for_each_entry_safe(&job->task_list, tmp2, tsk, struct gdc_task, node)
        {
            if (tsk->state == GDC_TASK_STATE_WAIT_SBM_CFG_DONE)
            {
                /* Update ldc state in vpss context */
                tsk->state = GDC_TASK_STATE_SBM_CFG_DONE;

                /* wakeup worker */
                rt_sem_release(wdev->sem_sbm);
                ret = 0;
            }
        }
    }

    if (ret)
        CVI_TRACE_LDC(CVI_DBG_WARN, "ldc not wait vpss sbm cb\n");

    return ret;
}

#if 0
static void cvi_ldc_job_handler(struct cvi_ldc_vdev *wdev,
				struct cvi_ldc_job *job)
{
	struct gdc_task *tsk;
	dlist_t *tmp;
	int32_t ret;

	gdc_proc_record_job_start(job);

	dlist_for_each_entry_safe(&job->task_list, tmp, tsk, struct gdc_task, node) {
		CVI_TRACE_LDC(CVI_DBG_DEBUG, "    task state=%d\n", tsk->state);
		if (tsk->state == GDC_TASK_STATE_IDLE) {
			/* wait h/w done */
			ret = -1;
			if (tsk->stTask.stBufWrap.bEnable)
				ret = _notify_vpp_sb_mode(tsk);

			if (ret)
				cvi_ldc_submit_hw(wdev, job, tsk);
			break;
		} else if (tsk->state == GDC_TASK_STATE_WAIT_SBM_CFG_DONE) {
			/* wait vpss config done */
			break;
		} else if (tsk->state == GDC_TASK_STATE_SBM_CFG_DONE) {
			cvi_ldc_submit_hw(wdev, job, tsk);
			break;
		} else if (tsk->state == GDC_TASK_STATE_RUNNING) {
			/* wait h/w done */
			break;
		} else if (tsk->state == GDC_TASK_STATE_DONE) {
			cvi_ldc_handle_hw_cb(wdev, job, tsk);
		} else if (tsk->state == GDC_TASK_STATE_ABORT) {
			CVI_TRACE_LDC(CVI_DBG_ERR, "    not handle abort yet\n");
		} else
			CVI_TRACE_LDC(CVI_DBG_ERR, "    unexpected task state=%d\n", tsk->state);

		pthread_mutex_lock(&wdev->mutex);
		if (tsk->state == GDC_TASK_STATE_DONE) {
			CVI_TRACE_LDC(CVI_DBG_DEBUG, "    del tsk\n");
			dlist_del(&tsk->node);
			pthread_mutex_unlock(&wdev->mutex);
			free(tsk);
		} else
			pthread_mutex_unlock(&wdev->mutex);
	}
}
#endif

/* == gdc_event_handler */
void *gdc_job_worker(void *data)
{
    struct cvi_ldc_vdev *wdev = (struct cvi_ldc_vdev *)data;
    struct cvi_ldc_job  *job;
    dlist_t             *tmp_job, *tmp_tsk;
    struct gdc_task     *tsk;
    CVI_BOOL             is_timeout;

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "+\n");
    while (wdev->thread_created)
    {
        if (rt_sem_take(wdev->job_sem, rt_tick_from_millisecond(SEM_WAIT_TIMEOUT_MS)) != RT_EOK)
            continue;
        dlist_for_each_entry_safe(&wdev->jobq, tmp_job, job, struct cvi_ldc_job, node)
        {
            gdc_proc_record_job_start(job);

            dlist_for_each_entry_safe(&job->task_list, tmp_tsk, tsk, struct gdc_task, node)
            {
                if (tsk->state == GDC_TASK_STATE_IDLE)
                {
                    if (tsk->stTask.stBufWrap.bEnable)
                    {
                        if (_notify_vpp_sb_mode(tsk))
                        {
                            CVI_TRACE_LDC(CVI_DBG_WARN, "_notify_vpp_sb_mode fail\n");
                            continue;
                        }
                        if (rt_sem_take(wdev->sem_sbm, rt_tick_from_millisecond(SEM_WAIT_TIMEOUT_MS)) != RT_EOK)
                        {
                            CVI_TRACE_LDC(CVI_DBG_WARN, "wait vpp_sb_done timeout\n");
                            continue;
                        }
                    }
                    cvi_ldc_submit_hw(wdev, job, tsk);
                }
                else
                {
                    CVI_TRACE_LDC(CVI_DBG_WARN, "dwa hw busy, not done tsk yet\n");
                    continue;
                }

                // wait hw done
                if (rt_sem_take(wdev->hw_done_sem, rt_tick_from_millisecond(SEM_WAIT_TIMEOUT_MS)) != RT_EOK)
                {
                    CVI_TRACE_LDC(CVI_DBG_WARN, "dwa hw irq not respond\n");
                    u8 intr_status = ldc_intr_status();
                    ldc_intr_clr(intr_status);
                    is_timeout = CVI_TRUE;
                    // continue;
                }
                else
                {
                    is_timeout = CVI_FALSE;
                }
                cvi_ldc_handle_hw_cb(wdev, job, tsk, is_timeout);
                pthread_mutex_lock(&wdev->mutex);
                dlist_del(&tsk->node);
                pthread_mutex_unlock(&wdev->mutex);
                rt_free(tsk);
                CVI_TRACE_LDC(CVI_DBG_DEBUG, "tsk done, del tsk\n");
            }
            pthread_mutex_lock(&wdev->mutex);
            dlist_del(&job->node);
            wdev->job_done = true;
            pthread_mutex_unlock(&wdev->mutex);

            gdc_proc_record_job_end(job);
            if (job->sync_io)
            {
                rt_sem_release(wdev->dev_sem);
            }
            else
            {
                rt_free(job);
                job = NULL;
            }

            CVI_TRACE_LDC(CVI_DBG_DEBUG, "jobq del a job\n");
        }
    }

    wdev->thread_created = false;
    pthread_exit(NULL);
    CVI_TRACE_LDC(CVI_DBG_DEBUG, "-\n");
    return 0;
}

int cvi_gdc_init(struct cvi_ldc_vdev *wdev)
{
    int ret;

    struct sched_param param;
    pthread_attr_t     attr;
    pthread_condattr_t cattr;

    INIT_DLIST_HEAD(&wdev->jobq);
    wdev->dev_sem     = rt_sem_create("gdc_dev_sem", 0, RT_IPC_FLAG_PRIO);
    wdev->job_sem     = rt_sem_create("gdc_job_sem", 0, RT_IPC_FLAG_PRIO);
    wdev->hw_done_sem = rt_sem_create("gdc_hw_done_sem", 0, RT_IPC_FLAG_PRIO);
    wdev->sem_sbm     = rt_sem_create("gdc_sem_sbm", 0, RT_IPC_FLAG_PRIO);
    // kthread_init_work(&wdev->work, gdc_job_worker);
    pthread_mutex_init(&wdev->mutex, NULL);
    // kthread_init_worker(&wdev->worker);
    wdev->thread_created = true;

    param.sched_priority = 16;
    pthread_attr_init(&attr);
    size_t stacksize = 8 * 1024;
    pthread_attr_setstacksize(&attr, stacksize);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_REALTIME);

    ret = pthread_create(&wdev->thread, &attr, gdc_job_worker, wdev);
    if (ret)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "failed to create gdc thread\n");
        return -1;
    }
    // pthread_setname_np(wdev->thread, "gdc_job_worker");

    gdc_proc_ctx = wdev->shared_mem;

    return 0;
}

int cvi_gdc_deinit(struct cvi_ldc_vdev *wdev)
{
    int ret = 0;

    wdev->thread_created  = false;
    ret                  |= pthread_join(wdev->thread, NULL);
    gdc_proc_ctx          = NULL;
    ret                  |= pthread_mutex_destroy(&wdev->mutex);
    ret                  |= rt_sem_delete(wdev->dev_sem);
    ret                  |= rt_sem_delete(wdev->job_sem);
    ret                  |= rt_sem_delete(wdev->hw_done_sem);
    ret                  |= rt_sem_delete(wdev->sem_sbm);
    if (ret)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "%s failed: %d\n", __func__, ret);
        return ret;
    }

    return 0;
}

static s32 gdc_update_proc(struct cvi_ldc_job *job)
{
    u16 u16Idx;
    // struct cvi_ldc_job *job;
    struct gdc_task *curelm;
    dlist_t         *tmp;
    struct timespec  curTime;

    if (!job)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "job NULL pointer\n");
        return CVI_ERR_GDC_NULL_PTR;
    }
    if (!gdc_proc_ctx)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "gdc_proc_ctx NULL pointer\n");
        return CVI_ERR_GDC_NULL_PTR;
    }

    gdc_proc_ctx->u16JobInfoIdx =
        (++gdc_proc_ctx->u16JobInfoIdx >= GDC_PROC_JOB_INFO_NUM) ? 0 : gdc_proc_ctx->u16JobInfoIdx;
    u16Idx = gdc_proc_ctx->u16JobInfoIdx;

    memset(&gdc_proc_ctx->stJobInfo[u16Idx], 0,
           sizeof(gdc_proc_ctx->stJobInfo[u16Idx]));
    gdc_proc_ctx->stJobInfo[u16Idx].hHandle = (u64)(uintptr_t)job;
    gdc_proc_ctx->stJobInfo[u16Idx].enModId = CVI_ID_BUTT;

    dlist_for_each_entry_safe(&job->task_list, tmp, curelm, struct gdc_task, node)
    {
        gdc_proc_ctx->stJobInfo[u16Idx].u32TaskNum++;
        gdc_proc_ctx->stJobInfo[u16Idx].u32InSize +=
            curelm->stTask.stImgIn.stVFrame.u32Width * curelm->stTask.stImgIn.stVFrame.u32Height;
        gdc_proc_ctx->stJobInfo[u16Idx].u32OutSize +=
            curelm->stTask.stImgOut.stVFrame.u32Width * curelm->stTask.stImgOut.stVFrame.u32Height;
    }

    gdc_proc_ctx->stJobInfo[u16Idx].eState = GDC_JOB_WORKING;
    clock_gettime(CLOCK_MONOTONIC, &curTime);
    gdc_proc_ctx->stJobInfo[u16Idx].u64SubmitTime =
        (u64)curTime.tv_sec * 1000000 + curTime.tv_nsec / 1000;

    return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
s32 gdc_begin_job(struct cvi_ldc_vdev *wdev, struct gdc_handle_data *data)
{
    struct cvi_ldc_job *job;
    CVI_S32             ret;

    ret = MOD_CHECK_NULL_PTR(CVI_ID_GDC, data);
    if (ret != CVI_SUCCESS)
        return ret;

    if (!data)
    {
        return -1;
    }

    job = rt_malloc(sizeof(struct cvi_ldc_job));

    if (job == NULL)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "malloc failed.\n");
        return CVI_ERR_GDC_NOBUF;
    }

    INIT_DLIST_HEAD(&job->task_list);

    job->sync_io = true;
    data->handle = (u64)(uintptr_t)job;

    if (gdc_proc_ctx)
        gdc_proc_ctx->stJobStatus.u32BeginNum++;

    return CVI_SUCCESS;
}

s32 gdc_end_job(struct cvi_ldc_vdev *wdev, u64 hHandle)
{
    s32                 ret     = CVI_SUCCESS;
    struct cvi_ldc_job *job     = (struct cvi_ldc_job *)(uintptr_t)hHandle;
    bool                sync_io = job->sync_io;

    if (gdc_proc_ctx)
        gdc_proc_ctx->stJobStatus.u32BeginNum--;

    if (dlist_empty(&job->task_list))
    {
        CVI_TRACE_LDC(CVI_DBG_WARN, "    no task in job.\n");
        return CVI_ERR_GDC_NOT_PERMITTED;
    }

    gdc_update_proc(job);

    pthread_mutex_lock(&wdev->mutex);
    dlist_add_tail(&job->node, &wdev->jobq);
    wdev->job_done = false;
    pthread_mutex_unlock(&wdev->mutex);
    ret = rt_sem_release(wdev->job_sem);
    if (sync_io)
    {
        /* request from user space, block until finished */
        ret = rt_sem_take(wdev->dev_sem, RT_WAITING_FOREVER);
        CVI_TRACE_LDC(CVI_DBG_DEBUG, "job sync_io=%d, ret=%d\n", job->sync_io, ret);
        rt_free(job);
        job = NULL;
    }
    else
    {
        CVI_TRACE_LDC(CVI_DBG_DEBUG, "job sync_io=%d, ret=%d\n", job->sync_io, ret);
    }

    return ret;
}

s32 gdc_add_rotation_task(struct cvi_ldc_vdev  *wdev,
                          struct gdc_task_attr *attr)
{
    struct cvi_ldc_job *job;
    struct gdc_task    *tsk;
    u64                 handle = attr->handle;
    CVI_S32             ret;

    ret = MOD_CHECK_NULL_PTR(CVI_ID_GDC, attr);
    if (ret != CVI_SUCCESS)
        return ret;

    if (!attr)
        return -1;

    ret = CHECK_GDC_FORMAT(attr->stImgIn, attr->stImgOut);
    if (ret != CVI_SUCCESS)
        return ret;

    if (gdc_rotation_check_size(attr->enRotation, attr) != CVI_SUCCESS)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
        return CVI_ERR_GDC_ILLEGAL_PARAM;
    }

    job = (struct cvi_ldc_job *)(uintptr_t)handle;
    tsk = rt_malloc(sizeof(*tsk));

    memcpy(&tsk->stTask, attr, sizeof(tsk->stTask));
    tsk->type       = GDC_TASK_TYPE_ROT;
    tsk->enRotation = attr->enRotation;
    tsk->state      = GDC_TASK_STATE_IDLE;

    pthread_mutex_lock(&wdev->mutex);
    dlist_add_tail(&tsk->node, &job->task_list);
    pthread_mutex_unlock(&wdev->mutex);

    return CVI_SUCCESS;
}

s32 gdc_add_ldc_task(struct cvi_ldc_vdev *wdev, struct gdc_task_attr *attr)
{
    struct cvi_ldc_job *job;
    struct gdc_task    *tsk;
    u64                 handle = attr->handle;
    CVI_S32             ret;

    ret = MOD_CHECK_NULL_PTR(CVI_ID_GDC, attr);
    if (ret != CVI_SUCCESS)
        return ret;

    if (!attr)
        return -1;

    ret = CHECK_GDC_FORMAT(attr->stImgIn, attr->stImgOut);
    if (ret != CVI_SUCCESS)
        return ret;

    if (gdc_rotation_check_size(attr->enRotation, attr) != CVI_SUCCESS)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
        return CVI_ERR_GDC_ILLEGAL_PARAM;
    }

    job = (struct cvi_ldc_job *)(uintptr_t)handle;
    tsk = rt_malloc(sizeof(*tsk));

    memcpy(&tsk->stTask, attr, sizeof(tsk->stTask));
    tsk->type       = GDC_TASK_TYPE_LDC;
    tsk->enRotation = attr->enRotation;
    tsk->state      = GDC_TASK_STATE_IDLE;

    pthread_mutex_lock(&wdev->mutex);
    dlist_add_tail(&tsk->node, &job->task_list);
    pthread_mutex_unlock(&wdev->mutex);

    return CVI_SUCCESS;
}

s32 gdc_is_same_task_attr(const struct gdc_task_attr *tsk1, const struct gdc_task_attr *tsk2)
{
    CVI_TRACE_LDC(CVI_DBG_DEBUG, "tsk1: IW=%d, IH=%d, OW=%d, OH=%d\n",
                  tsk1->stImgIn.stVFrame.u32Width, tsk1->stImgIn.stVFrame.u32Height,
                  tsk1->stImgOut.stVFrame.u32Width, tsk1->stImgOut.stVFrame.u32Height);
    CVI_TRACE_LDC(CVI_DBG_DEBUG, "tsk2: IW=%d, IH=%d, OW=%d, OH=%d\n",
                  tsk2->stImgIn.stVFrame.u32Width, tsk2->stImgIn.stVFrame.u32Height,
                  tsk2->stImgOut.stVFrame.u32Width, tsk2->stImgOut.stVFrame.u32Height);

    if (tsk1->stImgIn.stVFrame.u32Width != tsk2->stImgIn.stVFrame.u32Width || tsk1->stImgIn.stVFrame.u32Height != tsk2->stImgIn.stVFrame.u32Height || tsk1->stImgOut.stVFrame.u32Width != tsk2->stImgOut.stVFrame.u32Width || tsk1->stImgOut.stVFrame.u32Height != tsk2->stImgOut.stVFrame.u32Height)
        return 0;

    return 1;
}

static CVI_S32 _calc_buf_wrp_depth(struct gdc_task *tsk, const struct ldc_buf_wrap_cfg *cfg, CVI_U32 *buf_wrp_depth)
{
    CVI_U32 min_buf_wrp_depth = 2;
    CVI_U32 min_buf_size, depth;
    CVI_U32 u32Width          = tsk->stTask.stImgOut.stVFrame.u32Width;
    CVI_U32 u32BufLine        = cfg->stBufWrap.u32BufLine;
    CVI_U32 u32WrapBufferSize = cfg->stBufWrap.u32WrapBufferSize;

    CVI_TRACE_LDC(CVI_DBG_DEBUG, "u32BufLine=%d, u32WrapBufferSize=%d\n",
                  u32BufLine, u32WrapBufferSize);

    min_buf_size = u32BufLine * u32Width * min_buf_wrp_depth;
    if (u32WrapBufferSize < min_buf_size)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "invalid bufwrap size %d, min %d\n",
                      u32WrapBufferSize, min_buf_size);
        return CVI_ERR_GDC_ILLEGAL_PARAM;
    }

    /* Size always calculated from nv21/n12 x1.5 */
    depth = (u32WrapBufferSize * 2) / (u32Width * u32BufLine * 3);
    if (depth < min_buf_wrp_depth)
    {
        CVI_TRACE_LDC(CVI_DBG_ERR, "invalid bufwrap depth %d, min %d\n",
                      (int)depth, (int)min_buf_wrp_depth);
        return CVI_ERR_GDC_ILLEGAL_PARAM;
    }

    *buf_wrp_depth = depth;

    return CVI_SUCCESS;
}

CVI_S32 gdc_set_buf_wrap(struct cvi_ldc_vdev *wdev, const struct ldc_buf_wrap_cfg *cfg)
{
    CVI_S32                     s32Ret = CVI_FAILURE;
    struct cvi_ldc_job         *job;
    struct gdc_task            *tsk;
    dlist_t                    *tmp;
    const struct gdc_task_attr *wrp_tsk_attr;
    CVI_U32                     buf_wrp_depth;

    job = (struct cvi_ldc_job *)(uintptr_t)cfg->handle;

    if (cfg->stBufWrap.bEnable)
    {
        if (cfg->stBufWrap.u32BufLine != 64 && cfg->stBufWrap.u32BufLine != 128 && cfg->stBufWrap.u32BufLine != 192 && cfg->stBufWrap.u32BufLine != 256)
        {
            CVI_TRACE_LDC(CVI_DBG_ERR, "invalid bufwrap line %d\n", cfg->stBufWrap.u32BufLine);
            return CVI_ERR_GDC_ILLEGAL_PARAM;
        }
    }

    wrp_tsk_attr = &cfg->stTask;
    dlist_for_each_entry_safe(&job->task_list, tmp, tsk, struct gdc_task, node)
    {
        if (gdc_is_same_task_attr(wrp_tsk_attr, &tsk->stTask))
        {
            s32Ret = _calc_buf_wrp_depth(tsk, cfg, &buf_wrp_depth);
            if (s32Ret != CVI_SUCCESS)
                break;

            pthread_mutex_lock(&wdev->mutex);
            if (tsk->state == GDC_TASK_STATE_IDLE)
            {
                memcpy(&tsk->stTask.stBufWrap, &cfg->stBufWrap,
                       sizeof(tsk->stTask.stBufWrap));
                tsk->stTask.bufWrapDepth = buf_wrp_depth;
                s32Ret                   = CVI_SUCCESS;
            }
            pthread_mutex_unlock(&wdev->mutex);

            break;
        }
    }

    return s32Ret;
}

s32 gdc_get_buf_wrap(struct cvi_ldc_vdev *wdev, struct ldc_buf_wrap_cfg *cfg)
{
    CVI_S32                     s32Ret = CVI_FAILURE;
    struct cvi_ldc_job         *job, *wrp_job;
    struct gdc_task            *tsk;
    const struct gdc_task_attr *wrp_tsk_attr;
    dlist_t                    *tmp, *tmp2;

    // list_for_each_entry_safe(job, tmp, &wdev->jobq, node) {
    dlist_for_each_entry_safe(&wdev->jobq, tmp, job, struct cvi_ldc_job, node)
    {
        wrp_job = (struct cvi_ldc_job *)(uintptr_t)cfg->handle;
        if (job == wrp_job)
        {
            // list_for_each_entry_safe(tsk, tmp2, &job->task_list, node) {
            dlist_for_each_entry_safe(&job->task_list, tmp2, tsk, struct gdc_task, node)
            {
                wrp_tsk_attr = &cfg->stTask;
                if (gdc_is_same_task_attr(wrp_tsk_attr, &tsk->stTask))
                {
                    memcpy(&cfg->stBufWrap, &tsk->stTask.stBufWrap,
                           sizeof(cfg->stBufWrap));
                    s32Ret = CVI_SUCCESS;
                    break;
                }
            }
        }
    }

    return s32Ret;
}
