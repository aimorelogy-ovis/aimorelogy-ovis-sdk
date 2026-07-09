#include <cvi_base_ctx.h>
#include <cvi_buffer.h>
#include <cvi_defines.h>
#include <cvi_vi_ctx.h>
#include <inttypes.h>
#include <ldc_cb.h>
#include <stdlib.h>
#include <vi_ext.h>
#include <vi_raw_dump.h>
#include <vi_sdk_layer.h>

/****************************************************************************
 * Global parameters
 ****************************************************************************/
extern struct cvi_vi_ctx  *gViCtx;
extern struct cvi_gdc_mesh g_vi_mesh[VI_MAX_CHN_NUM];
static struct cvi_vi_dev  *gvdev;
static struct mlv_i_s      gmLVi[VI_MAX_DEV_NUM];

static struct crop_size_s dis_i_data[VI_MAX_DEV_NUM]    = {0};
static CVI_U32            dis_i_frm_num[VI_MAX_DEV_NUM] = {0};
static CVI_U32            dis_flag[VI_MAX_DEV_NUM]      = {0};

extern size_t    vi_block_size;
extern rt_smem_t vi_heap;

/****************************************************************************
 * SDK layer APIs
 ****************************************************************************/
static int _vi_sdk_drv_qbuf(struct cvi_buffer *buf, CVI_S32 chnId)
{
    struct cvi_isp_buf *qbuf;
    u8                  i = 0;

    qbuf = rt_smem_alloc(vi_heap, vi_block_size);
    if (qbuf == NULL) {
        vi_pr(VI_ERR, "prt_smem_alloc size(%lu) fail\n", sizeof(*qbuf));
        return -ENOMEM;
    }

    qbuf->buf.index = chnId;
    switch (buf->enPixelFormat) {
    default:
    case PIXEL_FORMAT_YUV_PLANAR_420:
    case PIXEL_FORMAT_YUV_PLANAR_422:
    case PIXEL_FORMAT_YUV_PLANAR_444:
        qbuf->buf.length = 3;
        break;
    case PIXEL_FORMAT_NV21:
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV61:
    case PIXEL_FORMAT_NV16:
        qbuf->buf.length = 2;
        break;
    case PIXEL_FORMAT_YUV_400:
    case PIXEL_FORMAT_YUYV:
    case PIXEL_FORMAT_YVYU:
    case PIXEL_FORMAT_UYVY:
    case PIXEL_FORMAT_VYUY:
        qbuf->buf.length = 1;
        break;
    }

    for (i = 0; i < qbuf->buf.length; i++) {
        qbuf->buf.planes[i].addr = buf->phy_addr[i];
    }

    cvi_isp_buf_queue(gvdev, qbuf);

    return CVI_SUCCESS;
}

int vi_sdk_qbuf(MMF_CHN_S chn)
{
    VB_BLK blk  = vb_get_block_with_id(gViCtx->chnAttr[chn.s32ChnId].u32BindVbPool,
                                       gViCtx->blk_size[chn.s32ChnId], CVI_ID_VI);
    SIZE_S size = gViCtx->chnAttr[chn.s32ChnId].stSize;
    int    rc   = CVI_SUCCESS;

    if (blk == VB_INVALID_HANDLE) {
        gViCtx->chnStatus[chn.s32ChnId].u32VbFail++;
        vi_pr(VI_DBG, "Can't acquire VB BLK for VI, size(%d)\n", gViCtx->blk_size[chn.s32ChnId]);
        return -ENOMEM;
    }

    // workaround for ldc 64-align for width/height.
    if (gViCtx->enRotation[chn.s32ChnId] != ROTATION_0 || gViCtx->stLDCAttr[chn.s32ChnId].bEnable) {
        size.u32Width  = ALIGN(size.u32Width, LDC_ALIGN);
        size.u32Height = ALIGN(size.u32Height, LDC_ALIGN);
    }

    base_get_frame_info(gViCtx->chnAttr[chn.s32ChnId].enPixelFormat, size,
                        &((struct vb_s *)blk)->buf, vb_handle2PhysAddr(blk), DEFAULT_ALIGN);

    ((struct vb_s *)blk)->buf.s16OffsetTop = 0;
    ((struct vb_s *)blk)->buf.s16OffsetRight =
        size.u32Width - gViCtx->chnAttr[chn.s32ChnId].stSize.u32Width;
    ((struct vb_s *)blk)->buf.s16OffsetLeft = 0;
    ((struct vb_s *)blk)->buf.s16OffsetBottom =
        size.u32Height - gViCtx->chnAttr[chn.s32ChnId].stSize.u32Height;

    rc = vb_qbuf(chn, CHN_TYPE_OUT, blk);
    if (rc != 0) {
        vi_pr(VI_ERR, "vb_qbuf failed\n");
        return rc;
    }

    rc = _vi_sdk_drv_qbuf(&((struct vb_s *)blk)->buf, chn.s32ChnId);
    if (rc != 0) {
        vi_pr(VI_ERR, "_vi_sdk_drv_qbuf failed\n");
        return rc;
    }

    rc = vb_release_block(blk);
    if (rc != 0) {
        vi_pr(VI_ERR, "vb_release_block failed\n");
        return rc;
    }

    return rc;
}

void vi_fill_mlv_info(struct vb_s *blk, u8 dev, struct mlv_i_s *m_lv_i, u8 is_vpss_offline)
{
    if (is_vpss_offline) {
        CVI_U8 snr_num = blk->buf.dev_num;

        blk->buf.motion_lv = gmLVi[snr_num].mlv_i_level;
        memcpy(blk->buf.motion_table, gmLVi[snr_num].mlv_i_table, MO_TBL_SIZE);
    } else {
        CVI_U8 snr_num = dev;

        m_lv_i->mlv_i_level = gmLVi[snr_num].mlv_i_level;
        memcpy(m_lv_i->mlv_i_table, gmLVi[snr_num].mlv_i_table, MO_TBL_SIZE);
    }
}

CVI_S32 vi_set_motion_lv(struct mlv_info_s mlv_i)
{
    if (mlv_i.sensor_num < 0 || mlv_i.sensor_num >= VI_MAX_DEV_NUM) return CVI_FAILURE;

    gmLVi[mlv_i.sensor_num].mlv_i_level = mlv_i.mlv;
    memcpy(gmLVi[mlv_i.sensor_num].mlv_i_table, mlv_i.mtable, MO_TBL_SIZE);

    return CVI_SUCCESS;
}

void vi_fill_dis_info(struct cvi_vi_dev *vdev, struct vb_s *blk)
{
    int                       ret                   = 0;
    CVI_U8                    snr_num               = blk->buf.dev_num;
    CVI_U32                   frm_num               = blk->buf.frm_num;
    CVI_BOOL                  isSetDisInfo          = false;
    unsigned int              actl_flags            = 0;
    static struct crop_size_s dis_i[VI_MAX_DEV_NUM] = {0};

    if (gViCtx->isDisEnable[snr_num]) {
        ret = rt_event_recv(&vdev->dis_wait_q[snr_num].event, 0x01,
                            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 10, &actl_flags);
        if (ret || dis_flag[snr_num] == 0) {
            vi_pr(VI_ERR, "snr(%d) frm(%d) timeout\n", snr_num, frm_num);
        }

        if (dis_flag[snr_num] == 1) {
            if (dis_i_frm_num[snr_num] == frm_num) {
                dis_i[snr_num] = dis_i_data[snr_num];
                isSetDisInfo   = true;
            }
            if (isSetDisInfo) {
                vi_pr(VI_DBG, "fill dis snr(%d), frm(%d), crop(%d, %d, %d, %d)\n", snr_num, frm_num,
                      dis_i[snr_num].start_x, dis_i[snr_num].start_y, dis_i[snr_num].end_x,
                      dis_i[snr_num].end_y);
            }
        }

        dis_flag[snr_num] = 0;

        blk->buf.frame_crop.start_x = dis_i[snr_num].start_x;
        blk->buf.frame_crop.start_y = dis_i[snr_num].start_y;
        blk->buf.frame_crop.end_x   = dis_i[snr_num].end_x;
        blk->buf.frame_crop.end_y   = dis_i[snr_num].end_y;
    }
}

CVI_S32 vi_set_dis_info(struct cvi_vi_dev *vdev, struct dis_info_s dis_i)
{
    if (dis_i.sensor_num < 0 || dis_i.sensor_num >= VI_MAX_DEV_NUM) {
        vi_pr(VI_ERR, "invalid sensor_num(%d)\n", dis_i.sensor_num);
        return CVI_FAILURE;
    }

    CVI_U32            frm_num = dis_i.frm_num;
    CVI_U8             snr_num = dis_i.sensor_num;
    struct crop_size_s crop    = dis_i.dis_i;

    vi_pr(VI_DBG, "set dis snr(%d), frm(%d), crop(%d, %d, %d, %d)\n", snr_num, frm_num,
          crop.start_x, crop.start_y, crop.end_x, crop.end_y);

    dis_i_frm_num[snr_num] = frm_num;
    dis_i_data[snr_num]    = crop;

    dis_flag[snr_num] = 1;
    rt_event_send(&vdev->dis_wait_q[snr_num].event, 0x01);
    return CVI_SUCCESS;
}

CVI_S32 vi_set_dev_attr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr)
{
    CVI_U32 chn_num = 1;

    memset(&gViCtx->devAttr[ViDev], 0, sizeof(VI_DEV_ATTR_S));
    gViCtx->devAttr[ViDev] = *pstDevAttr;

    if (pstDevAttr->enInputDataType == VI_DATA_TYPE_YUV) {
        switch (pstDevAttr->enWorkMode) {
        case VI_WORK_MODE_1Multiplex:
            chn_num = 1;
            break;
        case VI_WORK_MODE_2Multiplex:
            chn_num = 2;
            break;
        case VI_WORK_MODE_3Multiplex:
            chn_num = 3;
            break;
        case VI_WORK_MODE_4Multiplex:
            chn_num = 4;
            break;
        default:
            vi_pr(VI_ERR, "SNR work mode(%d) is wrong\n", pstDevAttr->enWorkMode);
            return CVI_FAILURE;
        }
    }

    if ((pstDevAttr->stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ||
        (pstDevAttr->stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME) ||
        (pstDevAttr->stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME_FULL_RATE)) {
        gvdev->ctx.is_synthetic_hdr_on = pstDevAttr->stWDRAttr.bSyntheticWDR;
    } else {
        gvdev->ctx.is_synthetic_hdr_on = 0;
    }

    gViCtx->devAttr[ViDev].chn_num = chn_num;

    if (pstDevAttr->isMux) {
        gvdev->ctx.isp_pipe_cfg[ViDev].is_mux               = true;
        gvdev->ctx.isp_pipe_cfg[ViDev].switch_info.gpio_idx = pstDevAttr->switchGpioIdx;
        gvdev->ctx.isp_pipe_cfg[ViDev].switch_info.gpio_pin = pstDevAttr->switchGpioPin;
        gvdev->ctx.isp_pipe_cfg[ViDev].switch_info.gpio_pol =
            pstDevAttr->switchGPioPol != 255 ? pstDevAttr->switchGPioPol : 0;
        gvdev->ctx.isp_pipe_cfg[ViDev].switch_info.is_frm_ctrl = pstDevAttr->isFrmCtrl;
        gvdev->ctx.isp_pipe_cfg[ViDev].switch_info.dst_frm =
            pstDevAttr->isFrmCtrl ? pstDevAttr->dstFrm : 1;
        /*streamon don't modify is_mux_switch*/
        if (rt_atomic_load(&gvdev->isp_streamon) == 0)
            gvdev->ctx.isp_pipe_cfg[ViDev].is_mux_switch = pstDevAttr->switchGPioPol;

        vi_pr(VI_DBG, "mux dev=%d frm ctrl=%d dst_frm_%d\n", ViDev, pstDevAttr->isFrmCtrl,
              pstDevAttr->dstFrm);
    }

    vi_pr(VI_DBG, "dev=%d chn_num=%d %d\n", ViDev, chn_num, pstDevAttr->enPathMode);

    return CVI_SUCCESS;
}

CVI_S32 vi_enable_dev(VI_DEV ViDev)
{
    struct isp_ctx *ctx = &gvdev->ctx;

    if ((ViDev < (VI_DEV)ISP_PRERAW_A) || (ViDev >= (VI_DEV)ISP_PRERAW_VIRT_MAX)) {
        vi_pr(VI_ERR, "vi_enable_dev invalid Dev(%d)\n", ViDev);
        return CVI_FAILURE;
    }

    if (gViCtx->devAttr[ViDev].enInputDataType == VI_DATA_TYPE_YUV ||
        gViCtx->devAttr[ViDev].enInputDataType == VI_DATA_TYPE_YUV_EARLY) {
        ctx->isp_pipe_cfg[ViDev].is_yuv_bypass_path = true;
        ctx->isp_pipe_cfg[ViDev].infMode            = gViCtx->devAttr[ViDev].enIntfMode;
        ctx->isp_pipe_cfg[ViDev].muxMode            = gViCtx->devAttr[ViDev].enWorkMode;
        ctx->isp_pipe_cfg[ViDev].enDataSeq          = gViCtx->devAttr[ViDev].enDataSeq;
    }

    ctx->isp_pipe_cfg[ViDev].enPathMode = gViCtx->devAttr[ViDev].enPathMode;

    if (gViCtx->devAttr[ViDev].enIntfMode == VI_MODE_LVDS) {
        ctx->is_sublvds_path = true;
        vi_pr(VI_WARN, "SUBLVDS_PATH_ON(%d)\n", ctx->is_sublvds_path);
    }

    dis_flag[ViDev] = 0;

    gViCtx->isDevEnable[ViDev]  = true;
    ctx->isp_pipe_enable[ViDev] = true;

    gViCtx->total_dev_num++;
// TODO make sure mac clk
#if 0
	vi_mac_clk_ctrl(gvdev, (u8)ViDev, true);
#endif
    vi_pr(VI_DBG, "dev_%d enable=%d, total_dev_num=%d\n", ViDev, gViCtx->isDevEnable[ViDev],
          gViCtx->total_dev_num);

    return CVI_SUCCESS;
}

CVI_S32 vi_disable_dev(VI_DEV ViDev)
{
    return CVI_SUCCESS;
}

CVI_S32 vi_create_pipe(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
    struct isp_ctx *ctx = &gvdev->ctx;

    // Clear pipeAttr first.
    memset(&gViCtx->pipeAttr[ViPipe], 0, sizeof(gViCtx->pipeAttr[ViPipe]));

    gViCtx->isPipeCreated[ViPipe] = true;
    if (!(gViCtx->enSource[ViPipe] == VI_PIPE_FRAME_SOURCE_USER_FE ||
          gViCtx->enSource[ViPipe] == VI_PIPE_FRAME_SOURCE_USER_BE))
        gViCtx->enSource[ViPipe] = VI_PIPE_FRAME_SOURCE_DEV;
    gViCtx->pipeAttr[ViPipe] = *pstPipeAttr;

    ctx->isp_pipe_cfg[ViPipe].is_3dnr_enable = !pstPipeAttr->b3dnrBypass;

    vi_pr(VI_INFO, "pipe[%d] pipeCreated=%d is_3dnr_on=%d\n", ViPipe, gViCtx->isPipeCreated[ViPipe],
          !pstPipeAttr->b3dnrBypass);

    return CVI_SUCCESS;
}

CVI_S32 vi_start_pipe(VI_PIPE ViPipe)
{
    struct isp_ctx *ctx = &gvdev->ctx;

    if (gViCtx->pipeAttr[0].enCompressMode == COMPRESS_MODE_TILE) {
        ctx->is_dpcm_on = true;
        vi_pr(VI_INFO, "ISP_COMPRESS_ON(%d)\n", ctx->is_dpcm_on);
    }

    ctx->isp_pipe_cfg[ViPipe].is_pipe_on = true;

    vi_pr(VI_DBG, "pipe[%d] start_pipe\n", ViPipe);

    return CVI_SUCCESS;
}

CVI_S32 vi_stop_pipe(VI_PIPE ViPipe)
{
    struct isp_ctx *ctx = &gvdev->ctx;

    ctx->isp_pipe_cfg[ViPipe].is_pipe_on = false;

    vi_pr(VI_DBG, "pipe[%d] stop_pipe\n", ViPipe);

    return CVI_SUCCESS;
}

CVI_S32 vi_set_chn_attr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
    VB_CAL_CONFIG_S stVbCalConfig = {0};
    CVI_U32         chn_num       = 1;
    CVI_U8          chn_str       = gViCtx->total_chn_num;

    pstChnAttr->u32BindVbPool = VB_INVALID_POOLID;
    chn_num                   = gViCtx->total_chn_num + gViCtx->devAttr[ViPipe].chn_num;

    for (; chn_str < chn_num; chn_str++) {
        gViCtx->chnAttr[chn_str] = *pstChnAttr;

        COMMON_GetPicBufferConfig(pstChnAttr->stSize.u32Width, pstChnAttr->stSize.u32Height,
                                  pstChnAttr->enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,
                                  DEFAULT_ALIGN, &stVbCalConfig);

        gViCtx->blk_size[chn_str] = stVbCalConfig.u32VBSize;
    }

    gViCtx->total_chn_num += gViCtx->devAttr[ViPipe].chn_num;

    vi_pr(VI_DBG, "pipe_%d chn_%d total_chn_num=%d, blk_size=%d\n", ViPipe, ViChn,
          gViCtx->total_chn_num, stVbCalConfig.u32VBSize);

    return CVI_SUCCESS;
}

CVI_S32 vi_set_dev_timing_attr(VI_DEV ViDev, const VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
    gViCtx->stTimingAttr[ViDev] = *pstTimingAttr;
#if 0
	if (pstTimingAttr->s32FrmRate > 30)
		gvdev->usr_pic_delay = msecs_to_jiffies(33);
	else if (pstTimingAttr->s32FrmRate > 0)
		gvdev->usr_pic_delay = msecs_to_jiffies(1000 / pstTimingAttr->s32FrmRate);
	else
		gvdev->usr_pic_delay = 0;
#endif
    if (!gvdev->usr_pic_delay) usr_pic_time_remove();

    return CVI_SUCCESS;
}

CVI_S32 vi_set_pipe_frame_source(VI_PIPE ViPipe, const VI_PIPE_FRAME_SOURCE_E enSource)
{
    gViCtx->enSource[ViPipe] = enSource;
    gvdev->isp_source        = (u32)enSource;
    gvdev->ctx.isp_pipe_cfg[ISP_PRERAW_A].is_offline_preraw =
        (gvdev->isp_source == CVI_ISP_SOURCE_FE);

    vi_pr(VI_INFO, "isp_source=%d\n", gvdev->isp_source);
    vi_pr(VI_INFO, "ISP_PRERAW_A.is_offline_preraw=%d\n",
          gvdev->ctx.isp_pipe_cfg[ISP_PRERAW_A].is_offline_preraw);

    return CVI_SUCCESS;
}

CVI_S32 vi_send_pipe_raw(VI_PIPE PipeId, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    struct isp_ctx *ctx = &gvdev->ctx;

    if (gViCtx->enSource[PipeId] == VI_PIPE_FRAME_SOURCE_DEV) {
        vi_pr(VI_ERR, "Pipe(%d) source(%d) incorrect.\n", PipeId, gViCtx->enSource[PipeId]);
        return CVI_FAILURE;
    }

    if (gViCtx->enSource[PipeId] == VI_PIPE_FRAME_SOURCE_USER_FE) {
        if (pstVideoFrame->stVFrame.enDynamicRange == DYNAMIC_RANGE_HDR10) {
            ctx->is_hdr_on                            = true;
            ctx->isp_pipe_cfg[ISP_PRERAW_A].is_hdr_on = true;
            vi_pr(VI_INFO, "HDR_ON(%d) for raw replay\n", ctx->is_hdr_on);
        }

        gvdev->usr_fmt.width   = pstVideoFrame->stVFrame.u32Width;
        gvdev->usr_fmt.height  = pstVideoFrame->stVFrame.u32Height;
        gvdev->usr_fmt.code    = pstVideoFrame->stVFrame.enBayerFormat;
        gvdev->usr_crop.left   = pstVideoFrame->stVFrame.s16OffsetLeft;
        gvdev->usr_crop.top    = pstVideoFrame->stVFrame.s16OffsetTop;
        gvdev->usr_crop.width  = pstVideoFrame->stVFrame.u32Width;
        gvdev->usr_crop.height = pstVideoFrame->stVFrame.u32Height;

        if (ctx->isp_pipe_cfg[ISP_PRERAW_A].is_offline_preraw) {
            u64 phy_addr = pstVideoFrame->stVFrame.u64PhyAddr[0];

            ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL4, phy_addr);
            gvdev->usr_pic_phy_addr[0] = phy_addr;
            vi_pr(VI_INFO, "raw_replay le(0x%llx)\n", gvdev->usr_pic_phy_addr[0]);

            if (ctx->is_hdr_on) {
                phy_addr = pstVideoFrame->stVFrame.u64PhyAddr[1];
                ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL5, phy_addr);
                gvdev->usr_pic_phy_addr[1] = phy_addr;
                vi_pr(VI_INFO, "raw_replay se(0x%llx)\n", gvdev->usr_pic_phy_addr[1]);
            }

            if (gvdev->usr_pic_delay) {
                usr_pic_timer_init(gvdev);
            }
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 vi_enable_chn(VI_CHN ViChn)
{
    int              rc = 0, i = 0, j = 0;
    struct isp_ctx  *ctx           = &gvdev->ctx;
    enum cvi_isp_raw raw_num       = ISP_PRERAW_A;
    u8               create_thread = false;

    for (raw_num = ISP_PRERAW_A; raw_num < ISP_PRERAW_VIRT_MAX; raw_num++) {
        if (!ctx->isp_pipe_enable[raw_num]) continue;

        if (raw_num < ISP_PRERAW_MAX && ctx->isp_pipe_cfg[raw_num].is_mux) {
            u32 cur_size = gViCtx->blk_size[ctx->chnstr_num[raw_num]];
            u32 vir_size = gViCtx->blk_size[ctx->chnstr_num[raw_num + ISP_PRERAW_MAX]];

            ctx->isp_pipe_cfg[raw_num].switch_info.is_comm_chn = (cur_size == vir_size);
            ctx->isp_pipe_cfg[raw_num + ISP_PRERAW_MAX].switch_info.is_comm_chn =
                (cur_size == vir_size);
            vi_pr(VI_INFO, "raw_%d is_comm_chn_%d\n",
                  ctx->isp_pipe_cfg[raw_num].switch_info.is_comm_chn);
        }

        if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) {
            create_thread = true;
            break;
        }
    }

    for (i = 0; i < gViCtx->total_chn_num; i++) {
        CVI_U8    num_buffers = 0;
        MMF_CHN_S chn         = {.enModId = CVI_ID_VI, .s32DevId = 0, .s32ChnId = i};

        raw_num = find_raw_num_by_chn(ctx, i);

        if (i == 0)
            num_buffers = CVI_VI_CHN_0_BUF;
        else if (i == 1)
            num_buffers = CVI_VI_CHN_1_BUF;
        else if (i == 2)
            num_buffers = CVI_VI_CHN_2_BUF;
        else
            num_buffers = CVI_VI_CHN_3_BUF;

        if ((ctx->isp_pipe_cfg[raw_num].is_mux &&
             !ctx->isp_pipe_cfg[raw_num].switch_info.is_comm_chn) ||
            gViCtx->chnAttr[ViChn].bSingleBuffer)
            num_buffers = 1;

        ViChn                                     = i;
        gViCtx->is_enable[ViChn]                  = CVI_TRUE;
        gViCtx->chnStatus[ViChn].bEnable          = CVI_TRUE;
        gViCtx->chnStatus[ViChn].stSize.u32Width  = gViCtx->chnAttr[ViChn].stSize.u32Width;
        gViCtx->chnStatus[ViChn].stSize.u32Height = gViCtx->chnAttr[ViChn].stSize.u32Height;
        gViCtx->chnStatus[ViChn].u32IntCnt        = 0;
        gViCtx->chnStatus[ViChn].u32FrameNum      = 0;
        gViCtx->chnStatus[ViChn].u64PrevTime      = 0;
        gViCtx->chnStatus[ViChn].u32FrameRate     = 0;

        base_mod_jobs_init(chn, CHN_TYPE_OUT, 0, num_buffers, gViCtx->chnAttr[i].u32Depth);

        /*
         * the same chn size of mipi switch, use the same chn pool.
         * otherwise use different chn pool.
         */
        if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler &&
            (!ctx->isp_pipe_cfg[raw_num].switch_info.is_comm_chn || raw_num < ISP_PRERAW_MAX)) {
            for (j = 0; j < num_buffers; j++) {
                rc = vi_sdk_qbuf(chn);
                if (rc) {
                    vi_pr(VI_ERR, "vi_qbuf error (%d)", rc);
                    goto ERR_QBUF;
                }
            }
        }
    }

    if (create_thread) {
        rc = vi_create_thread(gvdev, E_VI_TH_EVENT_HANDLER);
        if (rc) {
            vi_pr(VI_ERR, "Failed to create VI_EVENT_HANDLER thread\n");
            goto ERR_CREATE_THREAD;
        }
    }

    if (vi_start_streaming(gvdev)) {
        vi_pr(VI_ERR, "Failed to vi start streaming\n");
        rc = -EAGAIN;
        goto ERR_START_STREAMING;
    }

    rt_atomic_store(&gvdev->isp_streamon, 1);

    return rc;

ERR_START_STREAMING:
    vi_destory_thread(gvdev, E_VI_TH_EVENT_HANDLER);
ERR_CREATE_THREAD:
ERR_QBUF:
    for (i = 0; i < gViCtx->total_chn_num; i++) {
        MMF_CHN_S chn = {.enModId = CVI_ID_VI, .s32DevId = 0, .s32ChnId = i};

        base_mod_jobs_exit(chn, CHN_TYPE_OUT);
    }

    return rc;
}

CVI_S32 vi_disable_chn(VI_CHN ViChn)
{
    int rc = CVI_SUCCESS, i = 0, j = 0;

    for (i = 0; i < gViCtx->total_chn_num; i++) {
        ViChn = i;

        gViCtx->is_enable[ViChn]              = CVI_FALSE;
        gViCtx->chnStatus[ViChn].bEnable      = CVI_FALSE;
        gViCtx->chnStatus[ViChn].u32FrameNum  = 0;
        gViCtx->chnStatus[ViChn].u64PrevTime  = 0;
        gViCtx->chnStatus[ViChn].u32FrameRate = 0;

        if (ViChn == (gViCtx->total_chn_num - 1)) {
            vi_destory_thread(gvdev, E_VI_TH_EVENT_HANDLER);
            // TODO dbg_thread
            // vi_destory_dbg_thread(gvdev);

            if (vi_stop_streaming(gvdev)) {
                vi_pr(VI_ERR, "Failed to vi stop streaming\n");
                return -EAGAIN;
            }

            rt_atomic_store(&gvdev->isp_streamon, 0);

            for (j = 0; j < gViCtx->total_chn_num; j++) {
                MMF_CHN_S chn = {.enModId = CVI_ID_VI, .s32DevId = 0, .s32ChnId = j};

                base_mod_jobs_exit(chn, CHN_TYPE_OUT);
            }

            gViCtx->total_chn_num = 0;
            gViCtx->total_dev_num = 0;
        }
    }

    return rc;
}

CVI_S32 vi_get_chn_frame(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
                         CVI_S32 s32MilliSec)
{
    VB_BLK       blk;
    struct vb_s *vb;
    CVI_S32      ret;
    MMF_CHN_S    chn = {.enModId = CVI_ID_VI, .s32DevId = ViPipe, .s32ChnId = ViChn};
    CVI_S32      i   = 0;

    memset(pstFrameInfo, 0, sizeof(*pstFrameInfo));
    ret = base_get_chn_buffer(chn, &blk, s32MilliSec);
    if (ret != CVI_SUCCESS) {
        vi_pr(VI_DBG, "vi get buf fail\n");
        return CVI_FAILURE;
    }

    vb = (struct vb_s *)blk;
    if (ViChn >= VI_EXT_CHN_START)
        pstFrameInfo->stVFrame.enPixelFormat =
            gViCtx->stExtChnAttr[ViChn - VI_EXT_CHN_START].enPixelFormat;
    else
        pstFrameInfo->stVFrame.enPixelFormat = gViCtx->chnAttr[ViChn].enPixelFormat;
    pstFrameInfo->stVFrame.u32Width   = vb->buf.size.u32Width;
    pstFrameInfo->stVFrame.u32Height  = vb->buf.size.u32Height;
    pstFrameInfo->stVFrame.u32TimeRef = vb->buf.frm_num;
    pstFrameInfo->stVFrame.u64PTS     = vb->buf.u64PTS;
    for (i = 0; i < 3; ++i) {
        pstFrameInfo->stVFrame.u64PhyAddr[i] = vb->buf.phy_addr[i];
        pstFrameInfo->stVFrame.u32Length[i]  = vb->buf.length[i];
        pstFrameInfo->stVFrame.u32Stride[i]  = vb->buf.stride[i];
    }

    pstFrameInfo->stVFrame.s16OffsetTop    = vb->buf.s16OffsetTop;
    pstFrameInfo->stVFrame.s16OffsetBottom = vb->buf.s16OffsetBottom;
    pstFrameInfo->stVFrame.s16OffsetLeft   = vb->buf.s16OffsetLeft;
    pstFrameInfo->stVFrame.s16OffsetRight  = vb->buf.s16OffsetRight;
    pstFrameInfo->stVFrame.pPrivateData    = vb;

    vi_pr(VI_DBG, "pixfmt(%d), w(%d), h(%d), pts(%lx), addr(0x%lx, 0x%lx, 0x%lx)\n",
          pstFrameInfo->stVFrame.enPixelFormat, pstFrameInfo->stVFrame.u32Width,
          pstFrameInfo->stVFrame.u32Height, pstFrameInfo->stVFrame.u64PTS,
          pstFrameInfo->stVFrame.u64PhyAddr[0], pstFrameInfo->stVFrame.u64PhyAddr[1],
          pstFrameInfo->stVFrame.u64PhyAddr[2]);
    vi_pr(VI_DBG, "length(%d, %d, %d), stride(%d, %d, %d)\n", pstFrameInfo->stVFrame.u32Length[0],
          pstFrameInfo->stVFrame.u32Length[1], pstFrameInfo->stVFrame.u32Length[2],
          pstFrameInfo->stVFrame.u32Stride[0], pstFrameInfo->stVFrame.u32Stride[1],
          pstFrameInfo->stVFrame.u32Stride[2]);

    return CVI_SUCCESS;
}

CVI_S32 vi_release_chn_frame(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    VB_BLK blk;

    blk = vb_physAddr2Handle(pstFrameInfo->stVFrame.u64PhyAddr[0]);
    if (blk == VB_INVALID_HANDLE) {
        vi_pr(VI_ERR, "Invalid phy-address(%lx) in pstVideoFrame. Can't find VB_BLK.\n",
              pstFrameInfo->stVFrame.u64PhyAddr[0]);
        return CVI_FAILURE;
    }

    if (vb_release_block(blk) != CVI_SUCCESS) return CVI_FAILURE;

    vi_pr(VI_DBG, "release chn frame, addr(0x%lx)\n", pstFrameInfo->stVFrame.u64PhyAddr[0]);

    return CVI_SUCCESS;
}

CVI_S32 vi_set_chn_crop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S *pstChnCrop)
{
    struct isp_ctx *ctx = &gvdev->ctx;
    struct vi_rect  crop;

    crop.x = pstChnCrop->stCropRect.s32X;
    crop.y = pstChnCrop->stCropRect.s32Y;
    crop.w = pstChnCrop->stCropRect.u32Width;
    crop.h = pstChnCrop->stCropRect.u32Height;

    vi_pr(VI_INFO, "set chn crop x(%d), y(%d), w(%d), h(%d)\n", crop.x, crop.y, crop.w, crop.h);

    ispblk_crop_config(ctx, ISP_BLK_ID_CROP4, crop);
    crop.w >>= 1;
    crop.h >>= 1;
    ispblk_crop_config(ctx, ISP_BLK_ID_CROP5, crop);

    gViCtx->chnAttr[ViChn].stSize.u32Width  = pstChnCrop->stCropRect.u32Width;
    gViCtx->chnAttr[ViChn].stSize.u32Height = pstChnCrop->stCropRect.u32Height;
    gViCtx->chnCrop[ViChn]                  = *pstChnCrop;

    return CVI_SUCCESS;
}

CVI_S32 vi_get_chn_crop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S *pstChnCrop)
{
    *pstChnCrop = gViCtx->chnCrop[ViChn];

    return CVI_SUCCESS;
}

CVI_S32 vi_get_pipe_frame(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
    CVI_S32                    ret;
    struct cvi_vip_isp_raw_blk dump[2];
    CVI_U32                    u32BlkSize, dev_frm_w, dev_frm_h, frm_w, frm_h;
    CVI_U64                    u64PhyAddr = 0, addr = 0x00;
    VB_BLK                     tempVB = VB_INVALID_HANDLE;
    int                        dev_id = 0, frm_num = 1, i = 0;
    struct vi_rect             rawdump_crop;
    struct isp_ctx            *ctx = &gvdev->ctx;
    COMPRESS_MODE_E            enCompressMode;

    memset(dump, 0, sizeof(dump));
    dump[0].raw_dump.raw_num = ViPipe;

    dev_frm_w = gViCtx->devAttr[ViPipe].stSize.u32Width;
    dev_frm_h = gViCtx->devAttr[ViPipe].stSize.u32Height;

    memset(&rawdump_crop, 0, sizeof(rawdump_crop));
    if ((pstFrameInfo[0].stVFrame.s16OffsetTop != 0) ||
        (pstFrameInfo[0].stVFrame.s16OffsetBottom != 0) ||
        (pstFrameInfo[0].stVFrame.s16OffsetLeft != 0) ||
        (pstFrameInfo[0].stVFrame.s16OffsetRight != 0)) {
        rawdump_crop.x = pstFrameInfo[0].stVFrame.s16OffsetLeft;
        rawdump_crop.y = pstFrameInfo[0].stVFrame.s16OffsetTop;
        rawdump_crop.w = dev_frm_w - rawdump_crop.x - pstFrameInfo[0].stVFrame.s16OffsetRight;
        rawdump_crop.h = dev_frm_h - rawdump_crop.y - pstFrameInfo[0].stVFrame.s16OffsetBottom;

        vi_pr(VI_INFO, "set rawdump crop x(%d), y(%d), w(%d), h(%d)\n", rawdump_crop.x,
              rawdump_crop.y, rawdump_crop.w, rawdump_crop.h);

        frm_w = rawdump_crop.w;
        frm_h = rawdump_crop.h;
    } else {
        frm_w = dev_frm_w;
        frm_h = dev_frm_h;
    }
    ctx->isp_pipe_cfg[ViPipe].rawdump_crop    = rawdump_crop;
    ctx->isp_pipe_cfg[ViPipe].rawdump_crop_se = rawdump_crop;

    gViCtx->vi_raw_blk[0] = VB_INVALID_HANDLE;
    gViCtx->vi_raw_blk[1] = VB_INVALID_HANDLE;

    ctx->isp_pipe_cfg[ViPipe].enDumpRaw = pstFrameInfo[0].stVFrame.enCompressMode;

    enCompressMode = (pstFrameInfo[0].stVFrame.enCompressMode == COMPRESS_MODE_DEFAULT)
                         ? gViCtx->pipeAttr[0].enCompressMode
                         : pstFrameInfo[0].stVFrame.enCompressMode;

    u32BlkSize = VI_GetRawBufferSize(frm_w, frm_h, PIXEL_FORMAT_RGB_BAYER_12BPP, enCompressMode, 16,
                                     gViCtx->isTile);

    vi_pr(VI_INFO, "frm_w(%d), frm_h(%d), u32BlkSize: %d enDumpRaw %d\n", frm_w, frm_h, u32BlkSize,
          ctx->isp_pipe_cfg[ViPipe].enDumpRaw);

    /* Check if it is valid, such as the frame from VPSS. */
    if (pstFrameInfo[0].stVFrame.u64PhyAddr[0] != 0) {
        /* If it is valid, we can use its VB to receive raw frames. */
        tempVB = vb_physAddr2Handle(pstFrameInfo[0].stVFrame.u64PhyAddr[0]);
    }

    if (tempVB == VB_INVALID_HANDLE) {
        gViCtx->vi_raw_blk[0] = vb_get_block_with_id(VB_INVALID_POOLID, u32BlkSize, CVI_ID_VI);
        if (gViCtx->vi_raw_blk[0] == VB_INVALID_HANDLE) {
            vi_pr(VI_ERR, "Alloc VB blk for RAW_LE dump failed\n");
            return CVI_FAILURE;
        }

        u64PhyAddr = vb_handle2PhysAddr(gViCtx->vi_raw_blk[0]);

    } else {
        if (u32BlkSize > pstFrameInfo[0].stVFrame.u32Length[0]) {
            vi_pr(VI_ERR, "input vb blk too small, in: %d, rq: %d\n", u32BlkSize,
                  pstFrameInfo[0].stVFrame.u32Length[0]);
            return CVI_FAILURE;
        }

        u64PhyAddr            = pstFrameInfo[0].stVFrame.u64PhyAddr[0];
        gViCtx->vi_raw_blk[0] = VB_INVALID_HANDLE;
    }

    dump[0].raw_dump.phy_addr = u64PhyAddr;

    if ((gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ||
        (gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME) ||
        (gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME_FULL_RATE)) {
        tempVB = VB_INVALID_HANDLE;

        if (pstFrameInfo[1].stVFrame.u64PhyAddr[0] != 0) {
            tempVB = vb_physAddr2Handle(pstFrameInfo[1].stVFrame.u64PhyAddr[0]);

            if (tempVB == VB_INVALID_HANDLE) {
                addr =
                    pstFrameInfo[0].stVFrame.u64PhyAddr[0] + pstFrameInfo[0].stVFrame.u32Length[0];
            }

            /* You can use the second half of the same VB to receive SE raw frame. */
            if (addr == pstFrameInfo[1].stVFrame.u64PhyAddr[0]) {
                tempVB = 0;
            }
        }

        if (tempVB == VB_INVALID_HANDLE) {
            gViCtx->vi_raw_blk[1] = vb_get_block_with_id(VB_INVALID_POOLID, u32BlkSize, CVI_ID_VI);
            if (gViCtx->vi_raw_blk[1] == VB_INVALID_HANDLE) {
                vb_release_block(gViCtx->vi_raw_blk[0]);
                gViCtx->vi_raw_blk[0] = VB_INVALID_HANDLE;
                vi_pr(VI_ERR, "Alloc VB blk for RAW_SE dump failed\n");
                return CVI_FAILURE;
            }

            u64PhyAddr = vb_handle2PhysAddr(gViCtx->vi_raw_blk[1]);

        } else {
            if (u32BlkSize > pstFrameInfo[1].stVFrame.u32Length[0]) {
                vi_pr(VI_ERR, "input vb blk too small, in: %d, rq: %d\n", u32BlkSize,
                      pstFrameInfo[1].stVFrame.u32Length[0]);
                return CVI_FAILURE;
            }

            u64PhyAddr            = pstFrameInfo[1].stVFrame.u64PhyAddr[0];
            gViCtx->vi_raw_blk[1] = VB_INVALID_HANDLE;
        }

        dump[1].raw_dump.phy_addr = u64PhyAddr;
        frm_num                   = 2;
    }

    if (s32MilliSec >= 0) dump[0].time_out = dump[1].time_out = s32MilliSec;

    ret = isp_raw_dump(gvdev, &dump[0]);
    if (ret != CVI_SUCCESS) {
        vi_pr(VI_ERR, "_isp_raw_dump fail\n");
        return CVI_FAILURE;
    }

    if (dump[0].is_b_not_rls) {
        if (gViCtx->vi_raw_blk[0] != VB_INVALID_HANDLE) {
            vb_release_block(gViCtx->vi_raw_blk[0]);
            gViCtx->vi_raw_blk[0] = VB_INVALID_HANDLE;
        }
        if (gViCtx->vi_raw_blk[1] != VB_INVALID_HANDLE) {
            vb_release_block(gViCtx->vi_raw_blk[1]);
            gViCtx->vi_raw_blk[1] = VB_INVALID_HANDLE;
        }

        vi_pr(VI_ERR, "Release pipe frame first, buffer not release.\n");
        return CVI_FAILURE;
    }

    if (dump[0].is_timeout) {
        if (gViCtx->vi_raw_blk[0] != VB_INVALID_HANDLE) {
            vb_release_block(gViCtx->vi_raw_blk[0]);
            gViCtx->vi_raw_blk[0] = VB_INVALID_HANDLE;
        }

        if (gViCtx->vi_raw_blk[1] != VB_INVALID_HANDLE) {
            vb_release_block(gViCtx->vi_raw_blk[1]);
            gViCtx->vi_raw_blk[1] = VB_INVALID_HANDLE;
        }

        vi_pr(VI_ERR, "Get pipe frame time out(%d)\n", s32MilliSec);
        return CVI_FAILURE;
    }

    if (dump[0].is_sig_int) {
        if (gViCtx->vi_raw_blk[0] != VB_INVALID_HANDLE) {
            vb_release_block(gViCtx->vi_raw_blk[0]);
            gViCtx->vi_raw_blk[0] = VB_INVALID_HANDLE;
        }

        if (gViCtx->vi_raw_blk[1] != VB_INVALID_HANDLE) {
            vb_release_block(gViCtx->vi_raw_blk[1]);
            gViCtx->vi_raw_blk[1] = VB_INVALID_HANDLE;
        }

        vi_pr(VI_ERR, "Get pipe frame signal interrupt\n");
        return CVI_FAILURE;
    }

    for (; i < frm_num; i++) {
        pstFrameInfo[i].stVFrame.u64PhyAddr[0]  = dump[i].raw_dump.phy_addr;
        pstFrameInfo[i].stVFrame.u32Length[0]   = u32BlkSize;
        pstFrameInfo[i].stVFrame.enBayerFormat  = gViCtx->devAttr[dev_id].enBayerFormat;
        pstFrameInfo[i].stVFrame.enCompressMode = dump[i].compress_mode;
        pstFrameInfo[i].stVFrame.u32Width       = dump[i].src_w;
        pstFrameInfo[i].stVFrame.u32Height      = dump[i].src_h;
        pstFrameInfo[i].stVFrame.s16OffsetLeft  = dump[i].crop_x;
        pstFrameInfo[i].stVFrame.s16OffsetTop   = dump[i].crop_y;
        pstFrameInfo[i].stVFrame.u32FrameFlag   = dump[i].frm_num;
    }

    return CVI_SUCCESS;
}

CVI_S32 vi_release_pipe_frame(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    CVI_S8 i;
    for (i = 0; i < ISP_PRERAW_VIRT_MAX; i++) free_isp_byr(i);

    if (gViCtx->vi_raw_blk[0] != VB_INVALID_HANDLE) {
        vb_release_block(gViCtx->vi_raw_blk[0]);
        gViCtx->vi_raw_blk[0] = VB_INVALID_HANDLE;
    }

    if (gViCtx->vi_raw_blk[1] != VB_INVALID_HANDLE) {
        vb_release_block(gViCtx->vi_raw_blk[1]);
        gViCtx->vi_raw_blk[1] = VB_INVALID_HANDLE;
    }

    return CVI_SUCCESS;
}

CVI_S32 vi_get_smooth_rawdump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
    CVI_S32                    ret;
    struct cvi_vip_isp_raw_blk dump[2];
    int                        dev_id = 0, frm_num = 1, i = 0;

    memset(dump, 0, sizeof(dump));
    dump[0].raw_dump.raw_num = dump[1].raw_dump.raw_num = ViPipe;
    dump[0].time_out = dump[1].time_out = s32MilliSec;

    ret = isp_get_smooth_raw_dump(gvdev, dump);
    if (ret != CVI_SUCCESS) {
        vi_pr(VI_ERR, "isp_smooth_raw_dump failed\n");
        return CVI_FAILURE;
    }

    if (dump[0].is_b_not_rls) {
        vi_pr(VI_ERR, "Release pipe frame first, buffer not release.\n");
        return CVI_FAILURE;
    }

    if (dump[0].is_timeout) {
        vi_pr(VI_ERR, "Get pipe frame time out(%d)\n", s32MilliSec);
        return CVI_FAILURE;
    }

    if (dump[0].is_sig_int) {
        vi_pr(VI_ERR, "Get pipe frame signal interrupt\n");
        return CVI_FAILURE;
    }

    if ((gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ||
        (gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME) ||
        (gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME_FULL_RATE)) {
        frm_num = 2;
    } else {
        frm_num = 1;
    }

    for (; i < frm_num; i++) {
        pstFrameInfo[i].stVFrame.u64PhyAddr[0]  = dump[i].raw_dump.phy_addr;
        pstFrameInfo[i].stVFrame.u32Length[0]   = dump[i].raw_dump.size;
        pstFrameInfo[i].stVFrame.enBayerFormat  = gViCtx->devAttr[dev_id].enBayerFormat;
        pstFrameInfo[i].stVFrame.enCompressMode = dump[i].compress_mode;
        pstFrameInfo[i].stVFrame.u32Width       = dump[i].src_w;
        pstFrameInfo[i].stVFrame.u32Height      = dump[i].src_h;
        pstFrameInfo[i].stVFrame.s16OffsetLeft  = dump[i].crop_x;
        pstFrameInfo[i].stVFrame.s16OffsetTop   = dump[i].crop_y;
        pstFrameInfo[i].stVFrame.u32TimeRef     = dump[i].frm_num;
        vi_pr(VI_DBG, "Get paddr(0x%lx) size(%d) frm_num(%d)\n",
              pstFrameInfo[i].stVFrame.u64PhyAddr[0], pstFrameInfo[i].stVFrame.u32Length[0],
              pstFrameInfo[i].stVFrame.u32TimeRef);
    }

    if (dump[0].is_unuse) {
        vi_pr(VI_DBG, "vi frame[%d] raw is_unuse(%d)\n", dump[0].frm_num, dump[0].is_unuse);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 vi_put_smooth_rawdump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    CVI_S32                    ret;
    struct cvi_vip_isp_raw_blk dump[2];
    VB_BLK                     vb;
    int                        frm_num, i;

    memset(dump, 0, sizeof(dump));

    if ((gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ||
        (gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME) ||
        (gViCtx->devAttr[ViPipe].stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME_FULL_RATE)) {
        frm_num = 2;
    } else {
        frm_num = 1;
    }

    for (i = 0; i < frm_num; i++) {
        vb = vb_physAddr2Handle(pstFrameInfo[i].stVFrame.u64PhyAddr[0]);
        if (vb == VB_INVALID_HANDLE) {
            vi_pr(VI_ERR, "Can't get valid vb_blk.\n");
            return CVI_FAILURE;
        }

        dump[i].raw_dump.phy_addr = pstFrameInfo[i].stVFrame.u64PhyAddr[0];
        vi_pr(VI_DBG, "Put paddr(0x%llx)\n", dump[i].raw_dump.phy_addr);
    }

    ret = isp_put_smooth_raw_dump(gvdev, dump);
    if (ret != CVI_SUCCESS) {
        vi_pr(VI_ERR, "isp_put_smooth_raw_dump failed\n");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}
// TODO mesh
static CVI_S32 _vi_update_rotation_mesh(VI_CHN ViChn, ROTATION_E enRotation)
{
    struct cvi_gdc_mesh *pmesh = &g_vi_mesh[ViChn];

    pthread_mutex_lock(&pmesh->lock);
    pmesh->paddr              = DEFAULT_MESH_PADDR;
    gViCtx->enRotation[ViChn] = enRotation;
    pthread_mutex_unlock(&pmesh->lock);

    return CVI_SUCCESS;
}

// TODO mesh
static CVI_S32 _vi_update_ldc_mesh(VPSS_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr,
                                   ROTATION_E enRotation, CVI_U64 paddr)
{
    struct cvi_gdc_mesh *pmesh = &g_vi_mesh[ViChn];

    pthread_mutex_lock(&pmesh->lock);
    pmesh->paddr = paddr;
    pmesh->vaddr = NULL;

    gViCtx->stLDCAttr[ViChn]  = *pstLDCAttr;
    gViCtx->enRotation[ViChn] = enRotation;
    pthread_mutex_unlock(&pmesh->lock);

    vi_pr(VI_DBG, "Chn(%d) mesh base(0x%llx)\n", ViChn, (unsigned long long)paddr);
    vi_pr(VI_DBG, "bEnable=%d, apect=%d, xyratio=%d, xoffset=%d, yoffset=%d, ratio=%d\n",
          pstLDCAttr->bEnable, pstLDCAttr->stAttr.bAspect, pstLDCAttr->stAttr.s32XYRatio,
          pstLDCAttr->stAttr.s32CenterXOffset, pstLDCAttr->stAttr.s32CenterYOffset,
          pstLDCAttr->stAttr.s32DistortionRatio);

    return CVI_SUCCESS;
}

CVI_S32 vi_set_chn_rotation(VI_CHN ViChn, ROTATION_E enRotation)
{
    vi_pr(VI_DBG, "Chn(%d) rotation(%d).\n", ViChn, enRotation);

    return _vi_update_rotation_mesh(ViChn, enRotation);
}

CVI_S32 vi_set_chn_ldc_attr(VI_CHN ViChn, ROTATION_E enRotation, const VI_LDC_ATTR_S *pstLDCAttr,
                            CVI_U64 mesh_addr)
{
    vi_pr(VI_DBG, "Chn(%d) mesh base(0x%llx)\n", ViChn, (unsigned long long)mesh_addr);

    return _vi_update_ldc_mesh(ViChn, pstLDCAttr, enRotation, mesh_addr);
}

CVI_S32 vi_attach_vb_pool(VI_PIPE ViPipe, VI_CHN ViChn, VB_POOL VbPool)
{
    gViCtx->chnAttr[ViChn].u32BindVbPool = VbPool;

    vi_pr(VI_DBG, "Chn(%d) attach VbPool(%d)\n", ViChn, gViCtx->chnAttr[ViChn].u32BindVbPool);

    return CVI_SUCCESS;
}

CVI_S32 vi_detach_vb_pool(VI_PIPE ViPipe, VI_CHN ViChn)
{
    gViCtx->chnAttr[ViChn].u32BindVbPool = VB_INVALID_POOLID;

    vi_pr(VI_DBG, "Chn(%d) detach VbPool\n", ViChn);

    return CVI_SUCCESS;
}

CVI_S32 vi_disable_frame_valid(VI_DEV ViDev)
{
    struct isp_ctx *ctx = &gvdev->ctx;

    ctx->isp_pipe_cfg[ViDev].is_frame_valid = false;

    vi_pr(VI_DBG, "ViDev[%d] disable_frame_valid\n", ViDev);

    return CVI_SUCCESS;
}

CVI_S32 vi_enable_frame_valid(VI_DEV ViDev)
{
    struct isp_ctx *ctx = &gvdev->ctx;

    ctx->isp_pipe_cfg[ViDev].is_frame_valid = true;

    vi_pr(VI_DBG, "ViDev[%d] enable_frame_valid\n", ViDev);

    return CVI_SUCCESS;
}

/*****************************************************************************
 *  SDK layer ioctl operations for vi.c
 ****************************************************************************/
long vi_sdk_ctrl(struct cvi_vi_dev *vdev, struct vi_ext_control *p)
{
    u32  id = p->sdk_id;
    long rc = -EINVAL;

    gvdev = vdev;

    switch (id) {
    case VI_SDK_SET_DEVATTR: {
        VI_DEV_ATTR_S dev_attr;

        memcpy(&dev_attr, p->sdk_cfg.ptr, sizeof(VI_DEV_ATTR_S));

        rc = vi_set_dev_attr(p->sdk_cfg.dev, &dev_attr);
        break;
    }
    case VI_SDK_GET_DEVATTR: {
        VI_DEV_ATTR_S dev_attr;

        dev_attr = gViCtx->devAttr[p->sdk_cfg.dev];
        memcpy(p->sdk_cfg.ptr, &dev_attr, sizeof(VI_DEV_ATTR_S));

        rc = 0;
        break;
    }
    case VI_SDK_ENABLE_DEV: {
        rc = vi_enable_dev(p->sdk_cfg.dev);
        break;
    }
    case VI_SDK_DISABLE_DEV: {
        rc = vi_disable_dev(p->sdk_cfg.dev);
        break;
    }
    case VI_SDK_CREATE_PIPE: {
        VI_PIPE_ATTR_S pipe_attr;

        memcpy(&pipe_attr, p->sdk_cfg.ptr, sizeof(VI_PIPE_ATTR_S));

        rc = vi_create_pipe(p->sdk_cfg.pipe, &pipe_attr);
        break;
    }
    case VI_SDK_DESTROY_PIPE: {
        rc = 0;
        break;
    }
    case VI_SDK_SET_PIPEATTR: {
        rc = 0;
        break;
    }
    case VI_SDK_GET_PIPEATTR: {
        rc = 0;
        break;
    }
    case VI_SDK_START_PIPE: {
        rc = vi_start_pipe(p->sdk_cfg.pipe);
        break;
    }
    case VI_SDK_STOP_PIPE: {
        rc = vi_stop_pipe(p->sdk_cfg.pipe);
        break;
    }
    case VI_SDK_SET_CHNATTR: {
        VI_CHN_ATTR_S chn_attr;

        memcpy(&chn_attr, p->sdk_cfg.ptr, sizeof(VI_CHN_ATTR_S));

        rc = vi_set_chn_attr(p->sdk_cfg.pipe, p->sdk_cfg.chn, &chn_attr);
        break;
    }
    case VI_SDK_GET_CHNATTR: {
        rc = 0;
        break;
    }
    case VI_SDK_ENABLE_CHN: {
        rc = vi_enable_chn(0);
        break;
    }
    case VI_SDK_DISABLE_CHN: {
        rc = vi_disable_chn(0);
        break;
    }
    case VI_SDK_SET_MOTION_LV: {
        struct mlv_info_s mlv_i;

        memcpy(&mlv_i, p->sdk_cfg.ptr, sizeof(struct mlv_info_s));
        rc = vi_set_motion_lv(mlv_i);
        break;
    }
    case VI_SDK_ENABLE_DIS: {
        gViCtx->isDisEnable[p->sdk_cfg.pipe] = 1;

        rc = 0;
        break;
    }
    case VI_SDK_DISABLE_DIS: {
        gViCtx->isDisEnable[p->sdk_cfg.pipe] = 0;

        rc = 0;
        break;
    }
    case VI_SDK_SET_DIS_INFO: {
        struct dis_info_s dis_i;

        memcpy(&dis_i, p->sdk_cfg.ptr, sizeof(struct dis_info_s));

        rc = vi_set_dis_info(vdev, dis_i);
        break;
    }
    case VI_SDK_SET_PIPE_FRM_SRC: {
        VI_PIPE_FRAME_SOURCE_E src;

        memcpy(&src, p->sdk_cfg.ptr, sizeof(VI_PIPE_FRAME_SOURCE_E));

        rc = vi_set_pipe_frame_source(p->sdk_cfg.pipe, src);
        break;
    }
    case VI_SDK_SEND_PIPE_RAW: {
        VIDEO_FRAME_INFO_S v_frm_info;

        memcpy(&v_frm_info, p->sdk_cfg.ptr, sizeof(VIDEO_FRAME_INFO_S));

        rc = vi_send_pipe_raw(p->sdk_cfg.pipe, &v_frm_info);
        break;
    }
    case VI_SDK_SET_DEV_TIMING_ATTR: {
        VI_DEV_TIMING_ATTR_S dev_timing_attr;

        memcpy(&dev_timing_attr, p->sdk_cfg.ptr, sizeof(VI_DEV_TIMING_ATTR_S));

        rc = vi_set_dev_timing_attr(p->sdk_cfg.dev, &dev_timing_attr);
        break;
    }
    case VI_SDK_GET_CHN_FRAME: {
        VIDEO_FRAME_INFO_S v_frm_info;

        memcpy(&v_frm_info, p->sdk_cfg.ptr, sizeof(VIDEO_FRAME_INFO_S));

        rc = vi_get_chn_frame(p->sdk_cfg.pipe, p->sdk_cfg.chn, &v_frm_info, p->sdk_cfg.val);

        memcpy(p->sdk_cfg.ptr, &v_frm_info, sizeof(VIDEO_FRAME_INFO_S));
        break;
    }
    case VI_SDK_RELEASE_CHN_FRAME: {
        VIDEO_FRAME_INFO_S v_frm_info;

        memcpy(&v_frm_info, p->sdk_cfg.ptr, sizeof(VIDEO_FRAME_INFO_S));

        rc = vi_release_chn_frame(p->sdk_cfg.pipe, p->sdk_cfg.chn, &v_frm_info);
        break;
    }
    case VI_SDK_SET_CHN_CROP: {
        VI_CROP_INFO_S chn_crop;

        memcpy(&chn_crop, p->sdk_cfg.ptr, sizeof(VI_CROP_INFO_S));

        rc = vi_set_chn_crop(p->sdk_cfg.pipe, p->sdk_cfg.chn, &chn_crop);
        break;
    }
    case VI_SDK_GET_CHN_CROP: {
        VI_CROP_INFO_S chn_crop;

        memset(&chn_crop, 0, sizeof(chn_crop));

        rc = vi_get_chn_crop(p->sdk_cfg.pipe, p->sdk_cfg.chn, &chn_crop);

        memcpy(p->sdk_cfg.ptr, &chn_crop, sizeof(VI_CROP_INFO_S));
        break;
    }
    case VI_SDK_GET_PIPE_FRAME: {
        VIDEO_FRAME_INFO_S v_frm_info[2];

        memcpy(v_frm_info, p->sdk_cfg.ptr, sizeof(VIDEO_FRAME_INFO_S) * 2);

        rc = vi_get_pipe_frame(p->sdk_cfg.pipe, v_frm_info, p->sdk_cfg.val);

        memcpy(p->sdk_cfg.ptr, v_frm_info, sizeof(VIDEO_FRAME_INFO_S) * 2);
        break;
    }
    case VI_SDK_RELEASE_PIPE_FRAME: {
        VIDEO_FRAME_INFO_S v_frm_info[2];

        memcpy(v_frm_info, p->sdk_cfg.ptr, sizeof(VIDEO_FRAME_INFO_S) * 2);

        rc = vi_release_pipe_frame(p->sdk_cfg.pipe, v_frm_info);
        break;
    }
    case VI_SDK_START_SMOOTH_RAWDUMP: {
        struct cvi_vip_isp_smooth_raw_param param;

        memcpy(&param, p->sdk_cfg.ptr, sizeof(struct cvi_vip_isp_smooth_raw_param));
        rc = isp_start_smooth_raw_dump(vdev, &param);
        break;
    }
    case VI_SDK_STOP_SMOOTH_RAWDUMP: {
        struct cvi_vip_isp_smooth_raw_param param;

        memcpy(&param, p->sdk_cfg.ptr, sizeof(struct cvi_vip_isp_smooth_raw_param));

        rc = isp_stop_smooth_raw_dump(vdev, &param);
        break;
    }
    case VI_SDK_GET_SMOOTH_RAWDUMP: {
        VIDEO_FRAME_INFO_S v_frm_info[2];

        memcpy(v_frm_info, p->sdk_cfg.ptr, sizeof(VIDEO_FRAME_INFO_S) * 2);

        rc = vi_get_smooth_rawdump(p->sdk_cfg.pipe, v_frm_info, p->sdk_cfg.val);

        memcpy(p->sdk_cfg.ptr, v_frm_info, sizeof(VIDEO_FRAME_INFO_S) * 2);
        break;
    }
    case VI_SDK_PUT_SMOOTH_RAWDUMP: {
        VIDEO_FRAME_INFO_S v_frm_info[2];

        memcpy(v_frm_info, p->sdk_cfg.ptr, sizeof(VIDEO_FRAME_INFO_S) * 2);

        rc = vi_put_smooth_rawdump(p->sdk_cfg.pipe, v_frm_info);
        break;
    }
    case VI_SDK_SET_CHN_ROTATION: {
        struct vi_chn_rot_cfg cfg;

        memcpy(&cfg, p->sdk_cfg.ptr, sizeof(cfg));

        VI_CHN     ViChn      = cfg.ViChn;
        ROTATION_E enRotation = cfg.enRotation;

        rc = vi_set_chn_rotation(ViChn, enRotation);
        break;
    }
    case VI_SDK_SET_CHN_LDC: {
        struct vi_chn_ldc_cfg cfg;

        memcpy(&cfg, p->sdk_cfg.ptr, sizeof(cfg));

        VI_CHN               ViChn      = cfg.ViChn;
        ROTATION_E           enRotation = cfg.enRotation;
        CVI_U64              mesh_addr  = cfg.meshHandle;
        const VI_LDC_ATTR_S *pstLDCAttr = &cfg.stLDCAttr;

        rc = vi_set_chn_ldc_attr(ViChn, enRotation, pstLDCAttr, mesh_addr);
        break;
    }
    case VI_SDK_REG_SYNC_TASK: {
        struct isp_sync_task_node *node;

        node = (struct isp_sync_task_node *)p->sdk_cfg.ptr;

        rc = isp_sync_task_register(p->sdk_cfg.pipe, node);

        break;
    }
    case VI_SDK_UNREG_SYNC_TASK: {
        struct isp_sync_task_node *node;

        node = (struct isp_sync_task_node *)p->sdk_cfg.ptr;

        rc = isp_sync_task_unregister(p->sdk_cfg.pipe, node);
        break;
    }

    case VI_SDK_DISABLE_FRAME_VALID: {
        rc = vi_disable_frame_valid(p->sdk_cfg.dev);
        break;
    }

    case VI_SDK_ENABLE_FRAME_VALID: {
        rc = vi_enable_frame_valid(p->sdk_cfg.dev);
        break;
    }

    case VI_SDK_ATTACH_VB_POOL: {
        struct vi_vb_pool_cfg cfg;

        memcpy(&cfg, p->sdk_cfg.ptr, sizeof(cfg));

        VI_PIPE ViPipe = cfg.ViPipe;
        VI_CHN  ViChn  = cfg.ViChn;
        VB_POOL VbPool = (VB_POOL)cfg.VbPool;

        rc = vi_attach_vb_pool(ViPipe, ViChn, VbPool);
        break;
    }
    case VI_SDK_DETACH_VB_POOL: {
        struct vi_vb_pool_cfg cfg;

        memcpy(&cfg, p->sdk_cfg.ptr, sizeof(cfg));

        VI_PIPE ViPipe = cfg.ViPipe;
        VI_CHN  ViChn  = cfg.ViChn;

        rc = vi_detach_vb_pool(ViPipe, ViChn);
        break;
    }

    default:
        break;
    }

    return rc;
}
