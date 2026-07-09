#include <vi_dbg_proc.h>
// #include <aos/cli.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// one pipe use 832 byte
#define VI_DBG_BUF_SIZE 4096

/* Switch the output of proc.
 *
 * 0: VI debug info
 * 1: preraw0 reg-dump
 * 2: preraw1 reg-dump
 * 3: postraw reg-dump
 */
int proc_isp_mode;

static struct cvi_vi_dev *m_vdev;
/*************************************************************************
 *	Proc functions
 *************************************************************************/
static inline void _vi_dbg_proc_show(void)
{
    struct cvi_vi_dev *vdev    = m_vdev;
    struct isp_ctx    *ctx     = &vdev->ctx;
    enum cvi_isp_raw   raw_num = ISP_PRERAW_A;
    struct timespec    ts1, ts2;
    u32                frmCnt_start[ISP_PRERAW_VIRT_MAX], sofCnt_start[ISP_PRERAW_VIRT_MAX];
    u32                frmCnt_end[ISP_PRERAW_VIRT_MAX], sofCnt_end[ISP_PRERAW_VIRT_MAX];
    u64                t2 = 0, t1 = 0;
    char              *buf = NULL;
    int                pos = 0;

    if (vdev == NULL) {
        return;
    }
    buf = calloc(1, VI_DBG_BUF_SIZE);
    if (!buf) {
        printf("fail to malloc\n");
        return;
    }

    for (raw_num = ISP_PRERAW_A; raw_num < ISP_PRERAW_VIRT_MAX; raw_num++) {
        if (!ctx->isp_pipe_enable[raw_num]) continue;
        sofCnt_start[raw_num] = vdev->pre_fe_sof_cnt[raw_num][ISP_FE_CH0];
        frmCnt_start[raw_num] = vdev->ctx.isp_pipe_cfg[raw_num].is_yuv_bypass_path
                                    ? vdev->pre_fe_frm_num[raw_num][ISP_FE_CH0]
                                    : vdev->postraw_frame_number[raw_num];
    }

    clock_gettime(CLOCK_MONOTONIC, &ts1);
    t1 = ts1.tv_sec * 1000000 + ts1.tv_nsec / 1000;

    usleep(940 * 1000);
    do {
        for (raw_num = ISP_PRERAW_A; raw_num < ISP_PRERAW_VIRT_MAX; raw_num++) {
            if (!ctx->isp_pipe_enable[raw_num]) continue;
            sofCnt_end[raw_num] = vdev->pre_fe_sof_cnt[raw_num][ISP_FE_CH0];
            frmCnt_end[raw_num] = vdev->ctx.isp_pipe_cfg[raw_num].is_yuv_bypass_path
                                      ? vdev->pre_fe_frm_num[raw_num][ISP_FE_CH0]
                                      : vdev->postraw_frame_number[raw_num];
        }
        clock_gettime(CLOCK_MONOTONIC, &ts2);
        t2 = ts2.tv_sec * 1000000 + ts2.tv_nsec / 1000;
    } while ((t2 - t1) < 1000000);

    for (raw_num = ISP_PRERAW_A; raw_num < ISP_PRERAW_VIRT_MAX; raw_num++) {
        if (!ctx->isp_pipe_enable[raw_num]) continue;
        if (raw_num == ISP_PRERAW_A) {
            pos += sprintf(buf + pos, "[VI BE_Dbg_Info]\n");
            pos += sprintf(buf + pos, "VIPreBEDoneSts\t\t:0x%x\t\tVIPreBEDmaIdleStatus\t:0x%x\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.be_sts.be_done_sts,
                           ctx->isp_pipe_cfg[raw_num].dg_info.be_sts.be_dma_idle_sts);
            pos += sprintf(buf + pos, "[VI Post_Dbg_Info]\n");
            pos += sprintf(buf + pos, "VIIspTopStatus\t\t:0x%x\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.post_sts.top_sts);
            pos += sprintf(buf + pos, "[VI DMA_Dbg_Info]\n");
            pos += sprintf(buf + pos, "VIWdma0ErrStatus\t:0x%x\tVIWdma0IdleStatus\t:0x%x\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_0_err_sts,
                           ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_0_idle);
            pos += sprintf(buf + pos, "VIWdma1ErrStatus\t:0x%x\tVIWdma1IdleStatus\t:0x%x\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_1_err_sts,
                           ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_1_idle);
            pos += sprintf(buf + pos, "VIRdmaErrStatus\t\t:0x%x\tVIRdmaIdleStatus\t:0x%x\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.rdma_err_sts,
                           ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.rdma_idle);
        }

        pos += sprintf(buf + pos, "[VI ISP_PIPE_%c FE_Dbg_Info]\n", raw_num + 'A');
        pos += sprintf(buf + pos, "VIPreFERawDbgSts\t:0x%x\t\tVIPreFEDbgInfo\t\t:0x%x\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.fe_sts.fe_idle_sts,
                       ctx->isp_pipe_cfg[raw_num].dg_info.fe_sts.fe_done_sts);

        pos += sprintf(buf + pos, "[VI ISP_PIPE_%c]\n", raw_num + 'A');
        pos += sprintf(buf + pos, "VIOutImgWidth\t\t:%4d\n", ctx->isp_pipe_cfg[raw_num].post_img_w);
        pos +=
            sprintf(buf + pos, "VIOutImgHeight\t\t:%4d\n", ctx->isp_pipe_cfg[raw_num].post_img_h);
        pos +=
            sprintf(buf + pos, "VIInImgWidth\t\t:%4d\n", ctx->isp_pipe_cfg[raw_num].csibdg_width);
        pos +=
            sprintf(buf + pos, "VIInImgHeight\t\t:%4d\n", ctx->isp_pipe_cfg[raw_num].csibdg_height);

        pos +=
            sprintf(buf + pos, "VIDevFPS\t\t:%4d\n", sofCnt_end[raw_num] - sofCnt_start[raw_num]);
        pos += sprintf(buf + pos, "VIFPS\t\t\t:%4d\n", frmCnt_end[raw_num] - frmCnt_start[raw_num]);

        pos +=
            sprintf(buf + pos, "VISofCh0Cnt\t\t:%4d\n", vdev->pre_fe_sof_cnt[raw_num][ISP_FE_CH0]);
        if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
            pos += sprintf(buf + pos, "VISofCh1Cnt\t\t:%4d\n",
                           vdev->pre_fe_sof_cnt[raw_num][ISP_FE_CH1]);

        pos += sprintf(buf + pos, "VIPreFECh0Cnt\t\t:%4d\n",
                       vdev->pre_fe_frm_num[raw_num][ISP_FE_CH0]);
        if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
            pos += sprintf(buf + pos, "VIPreFECh1Cnt\t\t:%4d\n",
                           vdev->pre_fe_frm_num[raw_num][ISP_FE_CH1]);

        pos += sprintf(buf + pos, "VIPreBECh0Cnt\t\t:%4d\n",
                       vdev->pre_be_frm_num[raw_num][ISP_BE_CH0]);
        if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
            pos += sprintf(buf + pos, "VIPreBECh1Cnt\t\t:%4d\n",
                           vdev->pre_be_frm_num[raw_num][ISP_BE_CH1]);
        pos += sprintf(buf + pos, "VIPostCnt\t\t:%4d\n", vdev->postraw_frame_number[raw_num]);
        pos += sprintf(buf + pos, "VIDropCnt\t\t:%4d\n", vdev->drop_frame_number[raw_num]);
        pos += sprintf(buf + pos, "VIDumpCnt\t\t:%4d\n", vdev->dump_frame_number[raw_num]);

        pos += sprintf(buf + pos, "[VI ISP_PIPE_%c Csi_Dbg_Info]\n", raw_num + 'A');
        pos += sprintf(buf + pos, "VICsiIntStatus0\t\t:0x%x\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_int_sts_0);
        pos += sprintf(buf + pos, "VICsiIntStatus1\t\t:0x%x\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_int_sts_1);
        pos += sprintf(buf + pos, "VICsiCh0Dbg\t\t:0x%x\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_chn_debug[ISP_FE_CH0]);
        pos += sprintf(buf + pos, "VICsiCh1Dbg\t\t:0x%x\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_chn_debug[ISP_FE_CH1]);
        pos += sprintf(buf + pos, "VICsiOverFlowCnt\t:%4d\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_fifo_of_cnt);

        pos += sprintf(buf + pos, "VICsiCh0WidthGTCnt\t:%4d\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_gt_cnt[ISP_FE_CH0]);
        if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
            pos += sprintf(buf + pos, "VICsiCh1WidthGTCnt\t:%4d\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_gt_cnt[ISP_FE_CH1]);
        }

        pos += sprintf(buf + pos, "VICsiCh0WidthLSCnt\t:%4d\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_ls_cnt[ISP_FE_CH0]);
        if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
            pos += sprintf(buf + pos, "VICsiCh1WidthLSCnt\t:%4d\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_ls_cnt[ISP_FE_CH1]);
        }

        pos += sprintf(buf + pos, "VICsiCh0HeightGTCnt\t:%4d\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_gt_cnt[ISP_FE_CH0]);
        if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
            pos += sprintf(buf + pos, "VICsiCh1HeightGTCnt\t:%4d\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_gt_cnt[ISP_FE_CH1]);
        }

        pos += sprintf(buf + pos, "VICsiCh0HeightLSCnt\t:%4d\n",
                       ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_ls_cnt[ISP_FE_CH0]);
        if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
            pos += sprintf(buf + pos, "VICsiCh1HeightLSCnt\t:%4d\n",
                           ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_ls_cnt[ISP_FE_CH1]);
        }
    }
    printf(buf);
    free(buf);
}

static void vi_dbg_proc_show(int32_t argc, char **argv)
{
    if (proc_isp_mode == 0) _vi_dbg_proc_show();
#if 0
	else
		isp_register_dump(&isp_vdev->ctx, m, proc_isp_mode);
#endif
}
MSH_CMD_EXPORT_ALIAS(vi_dbg_proc_show, proc_vi_dbg, vi debug info);

int vi_dbg_proc_init(struct cvi_vi_dev *_vdev)
{
    int rc = 0;

    m_vdev = _vdev;
    return rc;
}

int vi_dbg_proc_remove(void)
{
    int rc = 0;

    m_vdev = NULL;
    return rc;
}
