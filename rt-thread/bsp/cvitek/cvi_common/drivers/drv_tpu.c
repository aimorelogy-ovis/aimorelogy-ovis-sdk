#include "drv_tpu.h"

#include <hal_debug.h>
#include <list.h>
#include <rtos_types.h>
#include <semaphore.h>
#include <stdbool.h>

#include "cache.h"
#include "hal_barrier.h"
#include "hal_tpu.h"
#include "semaphore.h"

static struct cvi_tpu_device *g_TpuDev = NULL;

enum tpu_submit_path {
    TPU_PATH_DESNORMAL = 0,
    TPU_PATH_DESSEC    = 1,
    TPU_PATH_PIOTDMA   = 2,
    TPU_PATH_MAX       = 3,
};

struct cvi_list_node {
    struct device             *dev;
    dlist_t                    list;
    uint32_t                   pid;
    uint32_t                   seq_no;
    uint32_t                   pio_seq_no;
    int                        dmabuf_fd;
    void                      *dmabuf_vaddr;
    uint64_t                   dmabuf_paddr;
    struct tpu_tee_submit_info tee_info;
    enum tpu_submit_path       tpu_path;
    int                        ret;
    struct tpu_tdma_pio_info   pio_info;
};

struct tpu_profiling_info {
    uint32_t run_current_us;
    uint64_t run_sum_us;

    struct timespec timer_start;
    uint32_t        timer_current_us;
    uint32_t        usage;
};

struct tpu_suspend_info {
    pthread_mutex_t mutex_lock;
    uint8_t         running_cnt;
};

static struct tpu_suspend_info tpu_suspend;

#define STORE_NPU_USAGE 1
#define STORE_INTERVAL  2
#define STORE_ENABLE    3

#define TASK_LIST_MAX 100
#define DONE_LIST_MAX 1000

struct dma_hdr_t {
    uint16_t dmabuf_magic_m;
    uint16_t dmabuf_magic_s;
    uint32_t dmabuf_size;
    uint32_t cpu_desc_count;
    uint32_t bd_desc_count;  // 16bytes
    uint32_t tdma_desc_count;
    uint32_t tpu_clk_rate;
    uint32_t pmubuf_size;
    uint32_t pmubuf_offset;  // 32bytes
    uint32_t arraybase_0_L;
    uint32_t arraybase_0_H;
    uint32_t arraybase_1_L;
    uint32_t arraybase_1_H;  // 48bytes
    uint32_t arraybase_2_L;
    uint32_t arraybase_2_H;
    uint32_t arraybase_3_L;
    uint32_t arraybase_3_H;  // 64bytes

    uint32_t arraybase_4_L;
    uint32_t arraybase_4_H;
    uint32_t arraybase_5_L;
    uint32_t arraybase_5_H;
    uint32_t arraybase_6_L;
    uint32_t arraybase_6_H;
    uint32_t arraybase_7_L;
    uint32_t arraybase_7_H;
    uint32_t reserve[8];  // 128bytes, 128bytes align
};
#define TPU_DMABUF_HEADER_M 0xB5B5

struct tpu_reg_backup_info {
    uint32_t tdma_int_mask;
    uint32_t tdma_sync_status;
    uint32_t tiu_ctrl_base_address;

    uint32_t tdma_arraybase0_l;
    uint32_t tdma_arraybase1_l;
    uint32_t tdma_arraybase2_l;
    uint32_t tdma_arraybase3_l;
    uint32_t tdma_arraybase4_l;
    uint32_t tdma_arraybase5_l;
    uint32_t tdma_arraybase6_l;
    uint32_t tdma_arraybase7_l;
    uint32_t tdma_arraybase0_h;
    uint32_t tdma_arraybase1_h;

    uint32_t tdma_des_base;
    uint32_t tdma_dbg_mode;
    uint32_t tdma_dcm_disable;
    uint32_t tdma_ctrl;
};

typedef struct {
    uint32_t vld;
    uint32_t compress_en;
    uint32_t eod;
    uint32_t intp_en;
    uint32_t bar_en;
    uint32_t check_bf16_value;
    uint32_t trans_dir;
    uint32_t rsv00;
    uint32_t trans_fmt;
    uint32_t transpose_md;
    uint32_t rsv01;
    uint32_t intra_cmd_paral;
    uint32_t outstanding_en;
    uint32_t cmd_id;
    uint32_t spec_func;
    uint32_t dst_fmt;
    uint32_t src_fmt;
    uint32_t cmprs_fmt;
    uint32_t sys_dtype;
    uint32_t rsv2_1;
    uint32_t int8_sign;
    uint32_t compress_zero_guard;
    uint32_t int8_rnd_mode;
    uint32_t wait_id_tpu;
    uint32_t wait_id_other_tdma;
    uint32_t wait_id_sdma;
    uint32_t const_val;
    uint32_t src_base_reg_sel;
    uint32_t mv_lut_idx;
    uint32_t dst_base_reg_sel;
    uint32_t mv_lut_base;
    uint32_t rsv4_5;
    uint32_t dst_h_stride;
    uint32_t dst_c_stride_low;
    uint32_t dst_n_stride;
    uint32_t src_h_stride;
    uint32_t src_c_stride_low;
    uint32_t src_n_stride;
    uint32_t dst_c;
    uint32_t src_c;
    uint32_t dst_w;
    uint32_t dst_h;
    uint32_t src_w;
    uint32_t src_h;
    uint32_t dst_base_addr_low;
    uint32_t src_base_addr_low;
    uint32_t src_n;
    uint32_t dst_base_addr_high;
    uint32_t src_base_addr_high;
    uint32_t src_c_stride_high;
    uint32_t dst_c_stride_high;
    uint32_t compress_bias0;
    uint32_t compress_bias1;
    uint32_t layer_ID;
} tdma_reg_t;

#define PLATTAG_CHANNEL_COLOR(...)
#define PLATTAG_CHANNEL_END(...)
#define PLATTAG_NAME_CHANNEL(...)

#define TIMEOUT_MS (60 * 1000)

#define TDMA_MASK_INIT 0x20  // ignore descriptor nchw/stride=0 error
#define TDMA_INT_EOD   0x1
#define TDMA_INT_EOPMU 0x8000
#define TDMA_ALL_IDLE  0X1F

static pthread_mutex_t            tpu_int_got_mutexlock;
static uint8_t                    tpu_sync_backup;
static uint8_t                    tpu_suspend_handle_int;
static struct tpu_reg_backup_info tpu_reg_backup;

void disable_tdma_enable_bit(void)
{
    uintptr_t ctrl_reg = TDMA_ENGINE_BASE_ADDR + TDMA_CTRL;
    u32       val      = RAW_READ32(ctrl_reg);
    val &= ~1;  // clear tdma_enable bit
    RAW_WRITE32(ctrl_reg, val);
    tpu_printf(TPU_DBG, "disable_tdma_enable_bit done.\r\n");
}

void platform_clear_int(struct cvi_tpu_device *ndev)
{
    u32 reg_value, int_status;

    // get interrupt status
    reg_value  = RAW_READ32(ndev->tdma_paddr + TDMA_INT_MASK);
    int_status = (reg_value >> 16) & ~(TDMA_MASK_INIT);
    tpu_printf(TPU_DBG, "platform_tdma_irq()=0x%x int_status=0x%x\r\n", reg_value, int_status);

    if (int_status != TDMA_INT_EOD && int_status != TDMA_INT_EOPMU) {
        tpu_printf(TPU_ERR, "platform_tdma_irq() got error = 0x%x\r\n", reg_value);
        tpu_printf(TPU_ERR, "TDMA_SYNC_STATUS reg=0x%x\r\n",
                   RAW_READ32(ndev->tdma_paddr + TDMA_SYNC_STATUS));
    }

    RAW_WRITE32(ndev->tdma_paddr + TDMA_INT_MASK, 0xFFFF0000);

    tpu_reg_backup.tdma_int_mask         = RAW_READ32(ndev->tdma_paddr + TDMA_INT_MASK);
    tpu_reg_backup.tdma_sync_status      = RAW_READ32(ndev->tdma_paddr + TDMA_SYNC_STATUS);
    tpu_reg_backup.tiu_ctrl_base_address = RAW_READ32(ndev->tiu_paddr + BD_CTRL_BASE_ADDR);

    tpu_printf(TPU_DBG, "platform_clear_int() done\r\n");
}

void platform_tdma_irq(struct cvi_tpu_device *ndev)
{
    tpu_printf(TPU_DBG, "got platform_tdma_irq() callback\r\n");
    sem_post(&ndev->tdma_done);
}

static void set_tdma_descriptor_Fire(struct TPU_PLATFORM_CFG *pCfg, u64 desc_offset, u32 num_tdma)
{
    uintptr_t iomem_tdmaBase = pCfg->iomem_tdmaBase;

    // set TDMA descriptor address
    RAW_WRITE32(iomem_tdmaBase + TDMA_DES_BASE, desc_offset);

    // make sure that debug mode is disable
    RAW_WRITE32(iomem_tdmaBase + TDMA_DEBUG_MODE, 0x0);

    // tdma dcm enable
    RAW_WRITE32(iomem_tdmaBase + TDMA_DCM_DISABLE, 0x0);

    // init interrupt mask
    RAW_WRITE32(iomem_tdmaBase + TDMA_INT_MASK, TDMA_MASK_INIT);

    pthread_mutex_lock(&tpu_int_got_mutexlock);
    tpu_sync_backup = 0;
    pthread_mutex_unlock(&tpu_int_got_mutexlock);

    RAW_WRITE32(iomem_tdmaBase + TDMA_CTRL,
                (0x1 << TDMA_CTRL_ENABLE_BIT) | (0x1 << TDMA_CTRL_MODESEL_BIT) |
                    (num_tdma << TDMA_CTRL_DESNUM_BIT) | (0x3 << TDMA_CTRL_BURSTLEN_BIT) |
                    (0x1 << TDMA_CTRL_FORCE_1ARRAY) |   // set 1 array
                    (0x1 << TDMA_CTRL_INTRA_CMD_OFF) |  // force off intra_cmd feature
                    (0x1 << TDMA_CTRL_64BYTE_ALIGN_EN));
}

static void set_tiu_descriptor(struct TPU_PLATFORM_CFG *pCfg, u64 desc_offset, u32 num_bd)
{
    u32       regVal        = 0;
    u64       desc_addr     = 0;
    uintptr_t iomem_tiuBase = pCfg->iomem_tiuBase;

    desc_addr = (u64)(desc_offset << BDC_ENGINE_CMD_ALIGNED_BIT);

    // set TIU descriptor address
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0x4, desc_addr & 0xFFFFFFFF);
    regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0x8);
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0x8,
                (regVal & 0xFFFFFF00) | (desc_addr >> 32 & 0xFF));

    // disable tiu pre_exe
    regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0xC);
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0xC, (regVal | (0x1 << 11)));

    regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR);
    regVal &= ~0x3FC00000;

// set 1 array, lane=8
#ifdef __CV180X__
    // set 1 array, lane=2
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR, regVal | (1 << 22));
#else
    // set 1 array, lane=8
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR, regVal | (3 << 22));
#endif

    // fire TIU
    regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR);
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR,
                regVal | (0x1 << BD_DES_ADDR_VLD) | (0x1 << BD_INTR_ENABLE) | (0x1 << BD_TPU_EN));
}

static void resync_cmd_id(struct TPU_PLATFORM_CFG *pCfg)
{
    u32       regVal         = 0;
    uintptr_t iomem_tiuBase  = pCfg->iomem_tiuBase;
    uintptr_t iomem_tdmaBase = pCfg->iomem_tdmaBase;
    // reset TIU ID
    regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0xC);
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0xC, regVal | 0x1);
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR + 0xC, regVal & ~0x1);

    regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR);
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR,
                regVal & ~((0x1 << BD_TPU_EN) | (0x1 << BD_DES_ADDR_VLD)));

    // reset TIU interrupt status
    regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR);
    RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR, regVal | (0x1 << 1));

    // reset DMA ID
    RAW_WRITE32(iomem_tdmaBase + TDMA_CTRL, 0x1 << TDMA_CTRL_RESET_SYNCID_BIT);
    RAW_WRITE32(iomem_tdmaBase + TDMA_CTRL, 0x0);

    // reset DMA interrupt status
    RAW_WRITE32(iomem_tdmaBase + TDMA_INT_MASK, 0xFFFF0000);
}

static int poll_cmdbuf_done(struct TPU_PLATFORM_CFG *pCfg, struct CMD_ID_NODE *id_node)
{
    u32       regVal = 0, regVal2 = 0;
    uintptr_t iomem_tiuBase = pCfg->iomem_tiuBase;

    tpu_printf(TPU_DBG, "poll_cmdbuf_done() tdma_int_mask=0x%x tdma_sync_status=0x%x\r\n",
               tpu_reg_backup.tdma_int_mask, tpu_reg_backup.tdma_sync_status);

    if (id_node->tdma_cmd_id > 0) {
        regVal  = tpu_reg_backup.tdma_int_mask;
        regVal2 = tpu_reg_backup.tdma_sync_status;

        tpu_printf(TPU_DBG, "regVal=0x%x, regVal2=0x%x, tdma_cmd_id=0x%x\r\n", regVal, regVal2,
                   id_node->tdma_cmd_id);

        if ((regVal2 >> 16) < id_node->tdma_cmd_id) {
            tpu_printf(TPU_DBG,
                       "got tdma int but tdma id not the last one, maybe not finished\r\n");
            return -1;
        }
    }

    if (id_node->bd_cmd_id > 0) {
        // pool until bd done
        while (1) {
            regVal = RAW_READ32(iomem_tiuBase + BD_CTRL_BASE_ADDR);
            if ((((regVal >> 6) & 0xFFFF) >= id_node->bd_cmd_id) && ((regVal & (0x1 << 1)) != 0)) {
                RAW_WRITE32(iomem_tiuBase + BD_CTRL_BASE_ADDR, regVal | (0x1 << 1));
                break;
            }
        }
    }

    return 0;
}

static void platform_run_dmabuf_setArrayBase(struct TPU_PLATFORM_CFG *pCfg,
                                             struct dma_hdr_t        *header)
{
    uintptr_t iomem_tdmaBase = pCfg->iomem_tdmaBase;

    tpu_printf(TPU_DBG,
               "base0L=0x%x, base1L=0x%x, base2L=0x%x, base3L=0x%x,\r\n base0H=0x%x, base1H=0x%x, "
               "base2H=0x%x, base3H=0x%x\r\n",
               header->arraybase_0_L, header->arraybase_1_L, header->arraybase_2_L,
               header->arraybase_3_L, header->arraybase_0_H, header->arraybase_1_H,
               header->arraybase_2_H, header->arraybase_3_H);

    tpu_printf(TPU_DBG,
               "base4L=0x%x, base5L=0x%x, base6L=0x%x, base7L=0x%x,\r\n base4H=0x%x, base5H=0x%x, "
               "base6H=0x%x, base7H=0x%x\r\n",
               header->arraybase_4_L, header->arraybase_5_L, header->arraybase_6_L,
               header->arraybase_7_L, header->arraybase_4_H, header->arraybase_5_H,
               header->arraybase_6_H, header->arraybase_7_H);

    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE0_L, header->arraybase_0_L);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE1_L, header->arraybase_1_L);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE2_L, header->arraybase_2_L);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE3_L, header->arraybase_3_L);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE4_L, header->arraybase_4_L);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE5_L, header->arraybase_5_L);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE6_L, header->arraybase_6_L);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE7_L, header->arraybase_7_L);

    // assume high bit always 0
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE0_H, 0);
    RAW_WRITE32(iomem_tdmaBase + TDMA_ARRAYBASE1_H, 0);
}

static int reinit_sem(sem_t *tdma_done)
{
    int value = 0;
    int ret   = -1;

    // Get the current value of the semaphore
    if (sem_getvalue(tdma_done, &ret) == -1) {
        tpu_printf(TPU_ERR, "Failed to get semaphore value.\r\n");
        return -1;
    }

    if (ret == 0) {
        for (int tmp = 0; tmp < value; tmp++) {
            sem_wait(tdma_done);
        }
    }
    tpu_printf(TPU_DBG, "reinit_sem done.\r\n");
    return ret;
}

int platform_run_dmabuf(struct cvi_tpu_device *ndev, void *dmabuf_v, uint64_t dmabuf_p)
{
    struct timespec ts = {0, 0};
    int             i = 0, ret = -1;
    u8              u8pmu_enable = 0;

    struct dma_hdr_t           *header = (struct dma_hdr_t *)dmabuf_v;
    struct cvi_cpu_sync_desc_t *desc =
        (struct cvi_cpu_sync_desc_t *)(dmabuf_v + sizeof(struct dma_hdr_t));

    struct TPU_PLATFORM_CFG cfg;
    struct CMD_ID_NODE      id_node = {0};

    tpu_printf(TPU_DBG, "size_dma_hdr_t=%d, dmabuf_v=0x%p, dmabuf_p=0x%llx\r\n",
               sizeof(struct dma_hdr_t), dmabuf_v, dmabuf_p);

    if (header->dmabuf_magic_m != TPU_DMABUF_HEADER_M) {
        tpu_printf(TPU_ERR, "err dmabuf\r\n");
        return -1;
    }

    pthread_mutex_lock(&tpu_int_got_mutexlock);
    tpu_sync_backup        = 0;
    tpu_suspend_handle_int = 0;
    pthread_mutex_unlock(&tpu_int_got_mutexlock);

    // the first part tag point
    PLATTAG_CHANNEL_COLOR(1, ANNOTATE_GREEN, "tpu_SW_pre");

    // assign iomem base related
    cfg.iomem_tdmaBase = ndev->tdma_paddr;
    cfg.iomem_tiuBase  = ndev->tiu_paddr;
    cfg.pmubuf_addr_p  = dmabuf_p + (uint64_t)(header->pmubuf_offset);

    tpu_printf(TPU_DBG, "iomem_tdmaBase=%p, iomem_tiuBase=%p, pmubuf_addr_p=%llx\r\n",
               cfg.iomem_tdmaBase, cfg.iomem_tiuBase, cfg.pmubuf_addr_p);

    platform_run_dmabuf_setArrayBase(&cfg, header);

    // check if enable pmu
    if ((header->pmubuf_offset) && (header->pmubuf_size)) u8pmu_enable = 1;

    if (u8pmu_enable) TPUPMU_Enable(&cfg, 1, TPU_PMUEVENT_TDMABW);

    for (i = 0; i < header->cpu_desc_count; i++, desc++) {
        int bd_num      = desc->num_bd & 0xFFFF;
        int tdma_num    = desc->num_gdma & 0xFFFF;
        u32 bd_offset   = desc->offset_bd;
        u32 tdma_offset = desc->offset_gdma;

        reinit_sem(&(ndev->tdma_done));
        resync_cmd_id(&cfg);

        id_node.bd_cmd_id   = bd_num;
        id_node.tdma_cmd_id = tdma_num;

        tpu_printf(TPU_DBG, "num <bd: %d, gdma: %d>, offset <0x%08x, 0x%08x>\r\n", bd_num, tdma_num,
                   bd_offset, tdma_offset);

        if (bd_num > 0) set_tiu_descriptor(&cfg, bd_offset, bd_num);

        if (tdma_num > 0) set_tdma_descriptor_Fire(&cfg, tdma_offset, tdma_num);

        // the second part tag point
        PLATTAG_CHANNEL_END(1);
        PLATTAG_CHANNEL_COLOR(2, ANNOTATE_GREEN, "tpu_HW");

        if (tdma_num > 0) {
            while (clock_gettime(CLOCK_REALTIME, &ts) == -1) continue;
            ts.tv_sec += TIMEOUT_MS / 1000;
            ret = sem_timedwait(&ndev->tdma_done, &ts);

            if (ret != 0) {
                tpu_printf(TPU_ERR, "run dmabuf timeout\r\n");
                return -1;
            } else {
                tpu_printf(TPU_DBG, "run dmabuf interrupted\r\n");
            }
        }

        // the third part tag point
        PLATTAG_CHANNEL_END(2);
        PLATTAG_CHANNEL_COLOR(1, ANNOTATE_GREEN, "tpu_SW_post");

        // check tdma/tiu current descriptor
        if (!tpu_suspend_handle_int) {
            ret = poll_cmdbuf_done(&cfg, &id_node);
            if (ret < 0) {
                tpu_printf(TPU_DBG, "pool dmabuf timeout\r\n");
                return -1;
            }
        }
    }

    // disable PMU
    if (u8pmu_enable) {
        reinit_sem(&ndev->tdma_done);

        TPUPMU_Enable(&cfg, 0, TPU_PMUEVENT_TDMABW);

        if (!tpu_suspend_handle_int) {
            while (clock_gettime(CLOCK_REALTIME, &ts) == -1) continue;
            ts.tv_sec += TIMEOUT_MS / 1000;
            ret = sem_timedwait(&ndev->tdma_done, &ts);

            if (ret != 0) {
                tpu_printf(TPU_ERR, "stop pmu timeout, ret = %d\r\n", ret);
                return -1;
            }
        }
    }

    PLATTAG_CHANNEL_END(1);
    PLATTAG_NAME_CHANNEL(1, 1, "tpu_SW");
    PLATTAG_NAME_CHANNEL(2, 1, "tpu_HW");
    return 0;
}

void enable_tpu_clock(void)
{
    uint32_t val;

    val = RAW_READ32(0x03002000);
    val |= 0x30;
    RAW_WRITE32(0x03002000, val);
}
void disable_tpu_clock(void)
{
    uint32_t val;

    val = RAW_READ32(0x03002000);
    val &= (~0x30);
    RAW_WRITE32(0x03002000, val);
}
int platform_tpu_suspend(struct cvi_tpu_device *ndev)
{
    uintptr_t iomem_tdma_base = ndev->tdma_paddr;
    uintptr_t iomem_tiu_base  = ndev->tiu_paddr;

    tpu_reg_backup.tdma_int_mask         = RAW_READ32(iomem_tdma_base + TDMA_INT_MASK);
    tpu_reg_backup.tdma_sync_status      = RAW_READ32(iomem_tdma_base + TDMA_SYNC_STATUS);
    tpu_reg_backup.tiu_ctrl_base_address = RAW_READ32(iomem_tiu_base + BD_CTRL_BASE_ADDR);

    tpu_reg_backup.tdma_arraybase0_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE0_L);
    tpu_reg_backup.tdma_arraybase1_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE1_L);
    tpu_reg_backup.tdma_arraybase2_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE2_L);
    tpu_reg_backup.tdma_arraybase3_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE3_L);
    tpu_reg_backup.tdma_arraybase4_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE4_L);
    tpu_reg_backup.tdma_arraybase5_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE5_L);
    tpu_reg_backup.tdma_arraybase6_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE6_L);
    tpu_reg_backup.tdma_arraybase7_l = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE7_L);
    tpu_reg_backup.tdma_arraybase0_h = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE0_H);
    tpu_reg_backup.tdma_arraybase1_h = RAW_READ32(iomem_tdma_base + TDMA_ARRAYBASE1_H);

    tpu_reg_backup.tdma_des_base    = RAW_READ32(iomem_tdma_base + TDMA_DES_BASE);
    tpu_reg_backup.tdma_dbg_mode    = RAW_READ32(iomem_tdma_base + TDMA_DEBUG_MODE);
    tpu_reg_backup.tdma_dcm_disable = RAW_READ32(iomem_tdma_base + TDMA_DCM_DISABLE);
    tpu_reg_backup.tdma_ctrl        = RAW_READ32(iomem_tdma_base + TDMA_CTRL);

    if (tpu_reg_backup.tdma_ctrl & (0x1 << TDMA_CTRL_ENABLE_BIT)) {
        // if we need polling INT
        if (!tpu_sync_backup) {
            uint32_t reg_value, int_status;
            // polling for waiting INT
            while (1) {
                reg_value  = RAW_READ32(iomem_tdma_base + TDMA_INT_MASK);
                int_status = (reg_value >> 16) & ~(TDMA_MASK_INIT);
                tpu_printf(TPU_DBG, "platform_tdma_irq()=0x%x int_status=0x%x\r\n", reg_value,
                           int_status);

                if (int_status != TDMA_INT_EOD && int_status != TDMA_INT_EOPMU) {
                    tpu_printf(TPU_ERR, "platform_tdma_irq() got error = 0x%x\r\n", reg_value);
                    tpu_printf(TPU_ERR, "TDMA_SYNC_STATUS=0x%x\r\n",
                               RAW_READ32(iomem_tdma_base + TDMA_SYNC_STATUS));
                }
            }

            tpu_reg_backup.tdma_int_mask         = RAW_READ32(iomem_tdma_base + TDMA_INT_MASK);
            tpu_reg_backup.tdma_sync_status      = RAW_READ32(iomem_tdma_base + TDMA_SYNC_STATUS);
            tpu_reg_backup.tiu_ctrl_base_address = RAW_READ32(iomem_tiu_base + BD_CTRL_BASE_ADDR);

            pthread_mutex_lock(&tpu_int_got_mutexlock);
            tpu_sync_backup        = 1;
            tpu_suspend_handle_int = 1;
            pthread_mutex_unlock(&tpu_int_got_mutexlock);
        }
    }

    // disable clock
    tpu_printf(TPU_DBG, "tpu disable clock()\r\n");
    disable_tpu_clock();

    return 0;
}

int platform_tpu_resume(struct cvi_tpu_device *ndev)
{
    uintptr_t iomem_tdma_base = ndev->tdma_paddr;
    uintptr_t iomem_tiu_base  = ndev->tiu_paddr;

    // enable clock
    tpu_printf(TPU_DBG, "tpu enable clock()\r\n");
    enable_tpu_clock();

    RAW_WRITE32(iomem_tdma_base + TDMA_INT_MASK, tpu_reg_backup.tdma_int_mask);
    RAW_WRITE32(iomem_tdma_base + TDMA_SYNC_STATUS, tpu_reg_backup.tdma_sync_status);
    RAW_WRITE32(iomem_tiu_base + BD_CTRL_BASE_ADDR, tpu_reg_backup.tiu_ctrl_base_address);

    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE0_L, tpu_reg_backup.tdma_arraybase0_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE1_L, tpu_reg_backup.tdma_arraybase1_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE2_L, tpu_reg_backup.tdma_arraybase2_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE3_L, tpu_reg_backup.tdma_arraybase3_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE4_L, tpu_reg_backup.tdma_arraybase4_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE5_L, tpu_reg_backup.tdma_arraybase5_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE6_L, tpu_reg_backup.tdma_arraybase6_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE7_L, tpu_reg_backup.tdma_arraybase7_l);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE0_H, tpu_reg_backup.tdma_arraybase0_h);
    RAW_WRITE32(iomem_tdma_base + TDMA_ARRAYBASE1_H, tpu_reg_backup.tdma_arraybase1_h);

    RAW_WRITE32(iomem_tdma_base + TDMA_DES_BASE, tpu_reg_backup.tdma_des_base);
    RAW_WRITE32(iomem_tdma_base + TDMA_DEBUG_MODE, tpu_reg_backup.tdma_dbg_mode);
    RAW_WRITE32(iomem_tdma_base + TDMA_DCM_DISABLE, tpu_reg_backup.tdma_dcm_disable);

    return 0;
}

int platform_tpu_reset(struct cvi_tpu_device *ndev)
{
    tpu_printf(TPU_DBG, "tpu reset()\r\n");
    uintptr_t resetAddr = 0x03003000;
    uint32_t  regVal = 0x0, resetVal = 0x0;
    regVal = RAW_READ32(resetAddr);

    resetVal = ~((0x1 << 7) | (0x1 << 8) | (0x1 << 9));
    RAW_WRITE32(resetAddr, (regVal & resetVal));

    regVal = RAW_READ32(resetAddr);

    resetVal = ((0x1 << 7) | (0x1 << 8) | (0x1 << 9));
    RAW_WRITE32(resetAddr, regVal | resetVal);

    regVal = RAW_READ32(resetAddr);
    return 0;
}

int platform_tpu_init(struct cvi_tpu_device *ndev)
{
    // enable clock
    tpu_printf(TPU_DBG, "tpu enable clock()\r\n");
    enable_tpu_clock();

    // reset
    platform_tpu_reset(ndev);

    return 0;
}

void platform_tpu_deinit(struct cvi_tpu_device *ndev)
{
    // disable clock
    tpu_printf(TPU_DBG, "tpu disable clock()\r\n");
    disable_tpu_clock();
}

static void emit_tdma_reg(const tdma_reg_t *r, uint32_t *_p)
{
    uint32_t *p = _p;

    p[15] = (r->compress_bias0 & ((1u << 8) - 1)) | ((r->compress_bias1 & ((1u << 8) - 1)) << 8) |
            ((r->layer_ID & ((1u << 16) - 1)) << 16);
    p[14] = (r->src_c_stride_high & ((1u << 16) - 1)) |
            ((r->dst_c_stride_high & ((1u << 16) - 1)) << 16);
    p[13] = (r->src_n & ((1u << 16) - 1)) | ((r->dst_base_addr_high & ((1u << 8) - 1)) << 16) |
            ((r->src_base_addr_high & ((1u << 8) - 1)) << 24);
    p[12] = (r->src_base_addr_low & (((uint64_t)1 << 32) - 1));
    p[11] = (r->dst_base_addr_low & (((uint64_t)1 << 32) - 1));
    p[10] = (r->src_w & ((1u << 16) - 1)) | ((r->src_h & ((1u << 16) - 1)) << 16);
    p[9]  = (r->dst_w & ((1u << 16) - 1)) | ((r->dst_h & ((1u << 16) - 1)) << 16);
    p[8]  = (r->dst_c & ((1u << 16) - 1)) | ((r->src_c & ((1u << 16) - 1)) << 16);
    p[7]  = (r->src_n_stride & (((uint64_t)1 << 32) - 1));
    p[6]  = (r->src_h_stride & ((1u << 16) - 1)) | ((r->src_c_stride_low & ((1u << 16) - 1)) << 16);
    p[5]  = (r->dst_n_stride & (((uint64_t)1 << 32) - 1));
    p[4]  = (r->dst_h_stride & ((1u << 16) - 1)) | ((r->dst_c_stride_low & ((1u << 16) - 1)) << 16);
    p[3]  = (r->const_val & ((1u << 16) - 1)) | ((r->src_base_reg_sel & ((1u << 3) - 1)) << 16) |
           ((r->mv_lut_idx & 1) << 19) | ((r->dst_base_reg_sel & ((1u << 3) - 1)) << 20) |
           ((r->mv_lut_base & 1) << 23) | ((r->rsv4_5 & ((1u << 8) - 1)) << 24);
    p[2] =
        (r->wait_id_other_tdma & ((1u << 16) - 1)) | ((r->wait_id_sdma & ((1u << 16) - 1)) << 16);
    p[1] = (r->spec_func & ((1u << 3) - 1)) | ((r->dst_fmt & ((1u << 2) - 1)) << 3) |
           ((r->src_fmt & ((1u << 2) - 1)) << 5) | ((r->cmprs_fmt & 1) << 7) |
           ((r->sys_dtype & 1) << 8) | ((r->rsv2_1 & ((1u << 4) - 1)) << 9) |
           ((r->int8_sign & 1) << 13) | ((r->compress_zero_guard & 1) << 14) |
           ((r->int8_rnd_mode & 1) << 15) | ((r->wait_id_tpu & ((1u << 16) - 1)) << 16);
    p[0] = (r->vld & 1) | ((r->compress_en & 1) << 1) | ((r->eod & 1) << 2) |
           ((r->intp_en & 1) << 3) | ((r->bar_en & 1) << 4) | ((r->check_bf16_value & 1) << 5) |
           ((r->trans_dir & ((1u << 2) - 1)) << 6) | ((r->rsv00 & ((1u << 2) - 1)) << 8) |
           ((r->trans_fmt & 1) << 10) | ((r->transpose_md & ((1u << 2) - 1)) << 11) |
           ((r->rsv01 & 1) << 13) | ((r->intra_cmd_paral & 1) << 14) |
           ((r->outstanding_en & 1) << 15) | ((r->cmd_id & ((1u << 16) - 1)) << 16);
}

static void reset_tdma_reg(tdma_reg_t *r)
{
    r->vld                 = 0x0;
    r->compress_en         = 0x0;
    r->eod                 = 0x0;
    r->intp_en             = 0x0;
    r->bar_en              = 0x0;
    r->check_bf16_value    = 0x0;
    r->trans_dir           = 0x0;
    r->rsv00               = 0x0;
    r->trans_fmt           = 0x0;
    r->transpose_md        = 0x0;
    r->rsv01               = 0x0;
    r->intra_cmd_paral     = 0x0;
    r->outstanding_en      = 0x0;
    r->cmd_id              = 0x0;
    r->spec_func           = 0x0;
    r->dst_fmt             = 0x1;
    r->src_fmt             = 0x1;
    r->cmprs_fmt           = 0x0;
    r->sys_dtype           = 0x0;
    r->rsv2_1              = 0x0;
    r->int8_sign           = 0x0;
    r->compress_zero_guard = 0x0;
    r->int8_rnd_mode       = 0x0;
    r->wait_id_tpu         = 0x0;
    r->wait_id_other_tdma  = 0x0;
    r->wait_id_sdma        = 0x0;
    r->const_val           = 0x0;
    r->src_base_reg_sel    = 0x0;
    r->mv_lut_idx          = 0x0;
    r->dst_base_reg_sel    = 0x0;
    r->mv_lut_base         = 0x0;
    r->rsv4_5              = 0x0;
    r->dst_h_stride        = 0x1;
    r->dst_c_stride_low    = 0x1;
    r->dst_n_stride        = 0x1;
    r->src_h_stride        = 0x1;
    r->src_c_stride_low    = 0x1;
    r->src_n_stride        = 0x1;
    r->dst_c               = 0x1;
    r->src_c               = 0x1;
    r->dst_w               = 0x1;
    r->dst_h               = 0x1;
    r->src_w               = 0x1;
    r->src_h               = 0x1;
    r->dst_base_addr_low   = 0x0;
    r->src_base_addr_low   = 0x0;
    r->src_n               = 0x1;
    r->dst_base_addr_high  = 0x0;
    r->src_base_addr_high  = 0x0;
    r->src_c_stride_high   = 0x0;
    r->dst_c_stride_high   = 0x0;
    r->compress_bias0      = 0x0;
    r->compress_bias1      = 0x0;
    r->layer_ID            = 0x0;
}

static void set_tdma_pio(struct cvi_tpu_device *ndev, uint32_t *pio_array)
{
    uint32_t                i              = 0;
    uintptr_t               iomem_tdmaBase = ndev->tdma_paddr;
    struct TPU_PLATFORM_CFG cfg            = {0};

    cfg.iomem_tdmaBase = ndev->tdma_paddr;
    cfg.iomem_tiuBase  = ndev->tiu_paddr;
    resync_cmd_id(&cfg);

    for (i = 0; i < 16; i++) {
        RAW_WRITE32(iomem_tdmaBase + TDMA_CMD_ACCP0 + (i << 2), pio_array[i]);
    }
    // make sure that debug mode is disable
    RAW_WRITE32(iomem_tdmaBase + TDMA_DEBUG_MODE, 0x0);

    // tdma dcm enable
    RAW_WRITE32(iomem_tdmaBase + TDMA_DCM_DISABLE, 0x0);

    // init interrupt mask
    RAW_WRITE32(iomem_tdmaBase + TDMA_INT_MASK, TDMA_MASK_INIT);

    RAW_WRITE32(iomem_tdmaBase + TDMA_CTRL,
                (0x1 << TDMA_CTRL_ENABLE_BIT) | (0x1 << TDMA_CTRL_DESNUM_BIT) |
                    (0x3 << TDMA_CTRL_BURSTLEN_BIT) |
                    (0x1 << TDMA_CTRL_FORCE_1ARRAY) |   // set 1 array
                    (0x1 << TDMA_CTRL_INTRA_CMD_OFF) |  // force off intra_cmd feature
                    (0x1 << TDMA_CTRL_64BYTE_ALIGN_EN));
}

int platform_run_pio(struct cvi_tpu_device *ndev, struct tpu_tdma_pio_info *info)
{
    tdma_reg_t reg;
    uint32_t   pio_array[16] = {0};

    reset_tdma_reg(&reg);
    reinit_sem(&ndev->tdma_done);

    reg.vld                = 1;
    reg.trans_dir          = 2;  // 0:tg2l, 1:l2tg, 2:g2g, 3:l2l
    reg.src_base_addr_low  = (uint32_t)(info->paddr_src);
    reg.src_base_addr_high = (info->paddr_src) >> 32;
    reg.dst_base_addr_low  = (uint32_t)(info->paddr_dst);
    reg.dst_base_addr_high = (info->paddr_dst) >> 32;
    reg.eod                = 1;
    reg.intp_en            = 1;

    if (info->enable_2d) {
        reg.trans_fmt = 0;  // 0:tensor, 1:common
        reg.src_n     = 1;
        reg.src_c     = 1;
        reg.src_h     = info->h;
        reg.src_w     = info->w_bytes;

        reg.dst_c = 1;
        reg.dst_h = reg.src_h;
        reg.dst_w = reg.src_w;

        reg.src_n_stride = info->stride_bytes_src * info->h;
        reg.src_h_stride = info->stride_bytes_src;

        reg.dst_n_stride = info->stride_bytes_dst * info->h;
        reg.dst_h_stride = info->stride_bytes_dst;
    } else {
        reg.trans_fmt    = 1;  // 0:tensor, 1:common
        reg.src_n_stride = info->leng_bytes;
    }
    emit_tdma_reg(&reg, pio_array);
    set_tdma_pio(ndev, pio_array);

    tpu_printf(TPU_DBG, "wait tdma_done\r\n");
    sem_wait(&ndev->tdma_done);
    tpu_printf(TPU_DBG, "irq end\r\n");

    return 0;
}

int platform_tpu_probe_setting(struct cvi_tpu_device *ndev)
{
    // init waiting interrupt spinlock
    pthread_mutex_init(&tpu_int_got_mutexlock, NULL);
    tpu_sync_backup        = 0;
    tpu_suspend_handle_int = 0;

    return 0;
}

void cvi_tpu_tdma_irq(int irq, void *data)
{
    struct cvi_tpu_device *ndev = data;

    platform_clear_int(ndev);
    disable_tdma_enable_bit();

    rt_spin_lock(&ndev->close_lock);
    if (ndev->use_count != 0) {
        platform_tdma_irq(ndev);
    }
    rt_spin_unlock(&ndev->close_lock);
}

static struct cvi_list_node *get_from_done_list(struct cvi_kernel_work *kernel_work, u32 seq_no,
                                                enum tpu_submit_path path)
{
    struct cvi_list_node *node = NULL;
    struct cvi_list_node *pos;
    uint32_t              current_pid = (uint32_t)(uintptr_t)rt_thread_self();

    pthread_mutex_lock(&kernel_work->done_list_lock);

    if (path == TPU_PATH_PIOTDMA) {
        dlist_for_each_entry(&kernel_work->done_list, pos, struct cvi_list_node, list)
        {
            if ((pos->pid == current_pid) && (pos->pio_seq_no == seq_no)) {
                node = pos;
                break;
            }
        }
    } else {
        dlist_for_each_entry(&kernel_work->done_list, pos, struct cvi_list_node, list)
        {
            if ((pos->pid == current_pid) && (pos->seq_no == seq_no)) {
                node = pos;
                break;
            }
        }
    }
    pthread_mutex_unlock(&kernel_work->done_list_lock);

    return node;
}

static void remove_from_done_list(struct cvi_kernel_work *kernel_work, struct cvi_list_node *node)
{
    pthread_mutex_lock(&kernel_work->done_list_lock);
    dlist_del(&node->list);
    pthread_mutex_unlock(&kernel_work->done_list_lock);
    if (NULL != node) {
        rt_free(node);
    }
}

static int cvi_tpu_submit(struct cvi_tpu_device *ndev, unsigned long arg)
{
    u32                       task_list_count = 0;
    struct cvi_submit_dma_arg run_dmabuf_arg;
    struct cvi_list_node     *node;
    struct cvi_list_node     *pos;
    struct cvi_kernel_work   *kernel_work;

    memcpy(&run_dmabuf_arg, (struct cvi_submit_dma_arg *)arg, sizeof(struct cvi_submit_dma_arg));

    kernel_work = &ndev->kernel_work;
    while (1) {
        pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
        dlist_for_each_entry(&kernel_work->task_list, pos, struct cvi_list_node, list)
        {
            task_list_count++;
        }
        pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);
        if (task_list_count > TASK_LIST_MAX) {
            tpu_printf(TPU_DBG, "too much task in task list\r\n");
            rt_thread_mdelay(20);
        } else {
            break;
        }
    }

    tpu_printf(TPU_DBG, "cvi_tpu_submit path()\r\n");
    node = rt_malloc(sizeof(struct cvi_list_node));
    if (!node) return -ENOMEM;
    memset(node, 0, sizeof(struct cvi_list_node));

    node->pid          = (uint32_t)(uintptr_t)rt_thread_self();
    node->seq_no       = run_dmabuf_arg.seq_no;
    node->dmabuf_vaddr = run_dmabuf_arg.addr;
    node->dmabuf_paddr = (uintptr_t)run_dmabuf_arg.addr;
    node->tpu_path     = TPU_PATH_DESNORMAL;

    pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
    dlist_add_tail(&node->list, &ndev->kernel_work.task_list);
    sem_post(&(ndev->kernel_work.task_wait_sem));
    pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);

    return 0;
}

static int cvi_tpu_run_dmabuf(struct cvi_tpu_device *ndev, struct cvi_list_node *node)
{
    int ret = 0;

    tpu_printf(TPU_DBG, "enter cvi_tpu_run_dmabuf()\r\n");

    pthread_mutex_lock(&ndev->dev_lock);
    platform_tpu_init(ndev);

    tpu_printf(TPU_DBG, "node->tpu_path = %d\r\n", node->tpu_path);
    switch (node->tpu_path) {
    case TPU_PATH_DESNORMAL:
        ret = platform_run_dmabuf(ndev, node->dmabuf_vaddr, node->dmabuf_paddr);
        break;
    case TPU_PATH_PIOTDMA:
        ret = platform_run_pio(ndev, &node->pio_info);
        break;
    default:
        break;
    }

    if (ret == -ETIMEDOUT) {
        platform_tpu_reset(ndev);
    } else {
        platform_tpu_deinit(ndev);
    }
    pthread_mutex_unlock(&ndev->dev_lock);
    tpu_printf(TPU_DBG, "exit cvi_tpu_run_dmabuf()\r\n");
    return ret;
}

static int cvi_tpu_wait_pio(struct cvi_tpu_device *ndev, unsigned long arg)
{
    int                      ret         = 0;
    struct cvi_kernel_work  *kernel_work = &ndev->kernel_work;
    struct cvi_tdma_wait_arg wait_pio_arg;
    struct cvi_list_node    *node;

    memcpy(&wait_pio_arg, (struct cvi_tdma_wait_arg *)arg, sizeof(struct cvi_tdma_wait_arg));

    while (1) {
        sem_wait(&(kernel_work->done_wait_sem));
        node = get_from_done_list(kernel_work, wait_pio_arg.seq_no, TPU_PATH_PIOTDMA);
        if (node) {
            break;
        }
    }
    if (ret) {
        return -EINTR;
    }

    wait_pio_arg.ret = node->ret;
    memcpy((unsigned long *)arg, (const void *)&wait_pio_arg, sizeof(struct cvi_tdma_wait_arg));

    remove_from_done_list(kernel_work, node);
    return ret;
}

int cvi_tpu_submit_pio(struct cvi_tpu_device *ndev, unsigned long arg)
{
    struct cvi_tdma_copy_arg ioctl_arg;
    u32                      task_list_count = 0;
    struct cvi_list_node    *node            = NULL;
    struct cvi_list_node    *pos             = NULL;
    struct cvi_kernel_work  *kernel_work     = &ndev->kernel_work;

    tpu_printf(TPU_DBG, "enter cvi_tpu_submit_pio\r\n");

    memcpy(&ioctl_arg, (struct cvi_tdma_copy_arg *)arg, sizeof(struct cvi_tdma_copy_arg));
    while (1) {
        pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
        dlist_for_each_entry(&kernel_work->task_list, pos, struct cvi_list_node, list)
        {
            task_list_count++;
        }
        pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);
        if (task_list_count > TASK_LIST_MAX) {
            tpu_printf(TPU_DBG, "too much task in task list\r\n");
            rt_thread_mdelay(20);
        } else {
            break;
        }
    }

    node = rt_malloc(sizeof(struct cvi_list_node));
    if (NULL == node) {
        tpu_printf(TPU_ERR, "vmalloc error\r\n");
        return -ENOMEM;
    }
    memset(node, 0, sizeof(struct cvi_list_node));

    node->pid        = (uint32_t)(uintptr_t)rt_thread_self();
    node->pio_seq_no = ioctl_arg.seq_no;
    node->tpu_path   = TPU_PATH_PIOTDMA;
    if (ioctl_arg.enable_2d) {
        node->pio_info.enable_2d        = 1;
        node->pio_info.paddr_src        = ioctl_arg.paddr_src;
        node->pio_info.paddr_dst        = ioctl_arg.paddr_dst;
        node->pio_info.h                = ioctl_arg.h;
        node->pio_info.w_bytes          = ioctl_arg.w_bytes;
        node->pio_info.stride_bytes_src = ioctl_arg.stride_bytes_src;
        node->pio_info.stride_bytes_dst = ioctl_arg.stride_bytes_dst;
    } else {
        node->pio_info.paddr_src  = ioctl_arg.paddr_src;
        node->pio_info.paddr_dst  = ioctl_arg.paddr_dst;
        node->pio_info.leng_bytes = ioctl_arg.leng_bytes;
    }

    pthread_mutex_lock(&ndev->kernel_work.task_list_lock);
    dlist_add_tail(&node->list, &ndev->kernel_work.task_list);
    sem_post(&(kernel_work->task_wait_sem));
    pthread_mutex_unlock(&ndev->kernel_work.task_list_lock);
    return 0;
}

static void cvi_tpu_cleanup_done_list(struct cvi_tpu_device  *ndev,
                                      struct cvi_kernel_work *kernel_work)
{
    u32                   done_list_count = 0;
    struct cvi_list_node *pos;
    dlist_t              *tmp;
    char                  alive;
    rt_thread_t           thread;
    rt_list_t            *node;
    extern rt_list_t      rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];

    pthread_mutex_lock(&kernel_work->done_list_lock);

    dlist_for_each_entry(&kernel_work->done_list, pos, struct cvi_list_node, list)
    {
        done_list_count++;
    }

    if (done_list_count < DONE_LIST_MAX) {
        pthread_mutex_unlock(&kernel_work->done_list_lock);
        return;
    }

    tpu_printf(TPU_DBG, "done list too much node, clean up\r\n");

    // Delete the node of the task that has ended
    dlist_for_each_entry_safe(&kernel_work->done_list, tmp, pos, struct cvi_list_node, list)
    {
        alive = false;
        for (int prio = 0; prio < RT_THREAD_PRIORITY_MAX; prio++) {
            rt_list_for_each(node, &rt_thread_priority_table[prio])
            {
                thread = rt_list_entry(node, struct rt_thread, taken_object_list);
                if (pos->pid == (uint32_t)(uintptr_t)thread) {
                    alive = true;
                    break;
                }
            }
        }
        if (!alive) {
            dlist_del(&pos->list);
            tpu_printf(TPU_DBG, "free buf\r\n");
            if (NULL != pos) {
                rt_free(pos);
            }
        }
    }

    pthread_mutex_unlock(&kernel_work->done_list_lock);
}

static void work_thread_run(struct cvi_tpu_device *ndev)
{
    struct cvi_kernel_work *kernel_work = &ndev->kernel_work;
    struct cvi_list_node   *first_node;
    int                     ret = 0;

    pthread_mutex_lock(&kernel_work->task_list_lock);
    first_node = dlist_first_entry(&kernel_work->task_list, struct cvi_list_node, list);
    dlist_del(&first_node->list);
    pthread_mutex_unlock(&kernel_work->task_list_lock);

    // before tpu inference
    tpu_suspend.running_cnt = 1;

    // tpu inference HW running process
    ret             = cvi_tpu_run_dmabuf(ndev, first_node);
    first_node->ret = ret;

    // after tpu inference
    tpu_suspend.running_cnt = 0;

    pthread_mutex_lock(&kernel_work->done_list_lock);
    dlist_add_tail(&first_node->list, &kernel_work->done_list);
    pthread_mutex_unlock(&kernel_work->done_list_lock);

    sem_post(&kernel_work->done_wait_sem);

    cvi_tpu_cleanup_done_list(ndev, kernel_work);
}

static int task_list_empty(struct cvi_kernel_work *kernel_work)
{
    int ret;

    pthread_mutex_lock(&kernel_work->task_list_lock);
    ret = dlist_empty(&kernel_work->task_list);
    pthread_mutex_unlock(&kernel_work->task_list_lock);

    return ret;
}

static void work_thread_exit(struct cvi_tpu_device *ndev)
{
    struct cvi_kernel_work *kernel_work = &ndev->kernel_work;
    struct cvi_list_node   *pos;
    dlist_t                *tmp;

    pthread_mutex_lock(&kernel_work->task_list_lock);
    dlist_for_each_entry_safe(&kernel_work->task_list, tmp, pos, struct cvi_list_node, list)
    {
        dlist_del(&pos->list);
        if (NULL != pos) {
            rt_free(pos);
        }
    }
    pthread_mutex_unlock(&kernel_work->task_list_lock);

    pthread_mutex_lock(&kernel_work->done_list_lock);
    dlist_for_each_entry_safe(&kernel_work->done_list, tmp, pos, struct cvi_list_node, list)
    {
        dlist_del(&pos->list);
        if (NULL != pos) {
            rt_free(pos);
        }
    }
    pthread_mutex_unlock(&kernel_work->done_list_lock);
}

void *work_thread_main(void *data)
{
    struct cvi_tpu_device  *ndev     = (struct cvi_tpu_device *)data;
    struct cvi_kernel_work *work     = &ndev->kernel_work;
    struct timespec         cur_time = {0};

    tpu_printf(TPU_DBG, "enter work_thread_main()\r\n");
    rt_thread_t rt_tid = rt_thread_self();
    rt_strncpy(rt_tid->parent.name, "new_name", RT_NAME_MAX);

    work->work_run = 1;

    while (work->work_run) {
        while (clock_gettime(CLOCK_REALTIME, &cur_time) == -1) continue;
        cur_time.tv_nsec += 500 * 1000000;
        cur_time.tv_sec += cur_time.tv_nsec / 1000000000;
        cur_time.tv_nsec %= 1000000000;

        int ret = sem_timedwait(&(work->task_wait_sem), &cur_time);
        if (ret != 0) {
            continue;
        }
        if (!task_list_empty(work)) work_thread_run(ndev);
    }

    work_thread_exit(ndev);
    tpu_printf(TPU_DBG, "exit work_thread_main()\r\n");
    return NULL;
}

static int work_thread_init(struct cvi_tpu_device *ndev)
{
    struct cvi_kernel_work *kernel_work = &(ndev->kernel_work);

    // rt_err_t rets;
    sem_init(&(kernel_work->task_wait_sem), 0, 0);
    // RT_ASSERT(rets == RT_EOK);
    sem_init(&(kernel_work->done_wait_sem), 0, 0);
    // RT_ASSERT(rets == RT_EOK);
    INIT_DLIST_HEAD(&kernel_work->task_list);
    rt_kprintf("head->next = %p, head->prev = %p\n", kernel_work->task_list.next,
               kernel_work->task_list.prev);
    pthread_mutex_init(&kernel_work->task_list_lock, NULL);
    INIT_DLIST_HEAD(&kernel_work->done_list);
    pthread_mutex_init(&kernel_work->done_list_lock, NULL);

    int ret = pthread_create(&(kernel_work->work_thread), NULL, work_thread_main, ndev);
    if (ret != 0) {
        tpu_printf(TPU_ERR, "kthread run fail , ret = %d , errno = %d\r\n", ret, errno);
        return ret;
    }
    tpu_printf(TPU_DBG, "pthread_create success. pthreadID = 0x%x\r\n", kernel_work->work_thread);
    return 0;
}

static int cvi_tpu_wait_dmabuf(struct cvi_tpu_device *ndev, unsigned long arg)
{
    int                     ret         = 0;
    struct cvi_kernel_work *kernel_work = &ndev->kernel_work;
    struct cvi_wait_dma_arg wait_dmabuf_arg;
    struct cvi_list_node   *node;

    memcpy(&wait_dmabuf_arg, (struct cvi_wait_dma_arg *)arg, sizeof(struct cvi_wait_dma_arg));

    while (1) {
        sem_wait(&(kernel_work->done_wait_sem));
        node = get_from_done_list(kernel_work, wait_dmabuf_arg.seq_no, TPU_PATH_DESNORMAL);
        if (node) break;
    }

    wait_dmabuf_arg.ret = node->ret;
    memcpy((unsigned long *)arg, (const void *)&wait_dmabuf_arg, sizeof(struct cvi_wait_dma_arg));

    remove_from_done_list(kernel_work, node);

    return ret;
}

static int cvi_tpu_cache_flush(struct cvi_tpu_device *ndev, struct cvi_cache_op_arg *flush_arg)
{
    tpu_printf(TPU_DBG, "flush_arg->paddr=0x%x, flush_arg->size=0x%x\r\n", flush_arg->paddr,
               flush_arg->size);
    rt_hw_cpu_dcache_clean((uintptr_t *)flush_arg->paddr, flush_arg->size);
    /* sync */
    __smp_mb();

    return 0;
}

static int cvi_tpu_cache_invalidate(struct cvi_tpu_device   *ndev,
                                    struct cvi_cache_op_arg *invalidate_arg)
{
    tpu_printf(TPU_DBG, "invalidate_arg->paddr=0x%x, invalidate_arg->size=0x%x\r\n",
               invalidate_arg->paddr, invalidate_arg->size);
    rt_hw_cpu_dcache_invalidate((uintptr_t *)invalidate_arg->paddr, invalidate_arg->size);
    tpu_printf(TPU_DBG, "invalidate_arg->paddr=0x%x, invalidate_arg->size=0x%x\r\n",
               invalidate_arg->paddr, invalidate_arg->size);
    /*sync	*/
    __smp_mb();

    return 0;
}

int cvi_tpu_ioctl(void *dev, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    tpu_printf(TPU_DBG, "arg=0x%x \r\n", arg);

    struct cvi_tpu_device *ndev = (struct cvi_tpu_device *)dev;
    switch (cmd) {
    case CVITPU_SUBMIT_DMABUF:
        ret = cvi_tpu_submit(ndev, arg);
        break;
    case CVITPU_DMABUF_FLUSH:
        ret = cvi_tpu_cache_flush(ndev, (struct cvi_cache_op_arg *)arg);
        break;
    case CVITPU_DMABUF_INVLD:
        ret = cvi_tpu_cache_invalidate(ndev, (struct cvi_cache_op_arg *)arg);
        break;
    case CVITPU_WAIT_DMABUF:
        ret = cvi_tpu_wait_dmabuf(ndev, arg);
        break;
    case CVITPU_SUBMIT_PIO:
        ret = cvi_tpu_submit_pio(ndev, arg);
        break;
    case CVITPU_WAIT_PIO:
        ret = cvi_tpu_wait_pio(ndev, arg);
        break;

    default:
        return -ENOTTY;
    }

    return ret;
}

int platform_tpu_open(struct cvi_tpu_device *ndev)
{
    return 0;
}

void *cvi_tpu_open(void)
{
    int ret;
    if (g_TpuDev == NULL) {
        tpu_printf(TPU_ERR, "cvi_tpu_device is NULL\r\n");
        return NULL;
    }
    pthread_mutex_lock(&(g_TpuDev->close_lock));
    if (g_TpuDev->use_count == 0) {
        ret = platform_tpu_open(g_TpuDev);
        if (ret < 0) {
            tpu_printf(TPU_DBG, "npu open failed ret=%d\r\n", ret);
            return NULL;
        }
    }
    g_TpuDev->use_count++;
    pthread_mutex_unlock(&(g_TpuDev->close_lock));

    return (void *)g_TpuDev;
}

int cvi_tpu_close(void)
{
    pthread_mutex_lock(&(g_TpuDev->close_lock));
    g_TpuDev->use_count--;
    pthread_mutex_unlock(&(g_TpuDev->close_lock));

    return 0;
}

int cvi_tpu_probe(void)
{
    int ret = 0;

    tpu_printf(TPU_DBG, "===cvi_tpu_probe start\r\n");

    g_TpuDev = rt_malloc(sizeof(struct cvi_tpu_device));
    if (g_TpuDev == NULL) {
        tpu_printf(TPU_ERR, "cvi_tpu_device kmalloc failed!\r\n");
        return -1;
    }
    tpu_printf(TPU_DBG, "kmalloc g_TpuDev(0x%lx) success!\r\n", (uintptr_t)g_TpuDev);

    g_TpuDev->tdma_paddr   = TDMA_ENGINE_BASE_ADDR;
    g_TpuDev->tiu_paddr    = TIU_ENGINE_BASE_ADDR;
    g_TpuDev->tdma_irq_num = TPU_IRQ_2;
    if (g_TpuDev->tdma_irq_num < 0) {
        tpu_printf(TPU_ERR, "failed to retrieve tdma irq");
        return -ENXIO;
    }
    sem_init(&g_TpuDev->tdma_done, 0, 0);
    pthread_mutex_init(&tpu_suspend.mutex_lock, NULL);
    tpu_suspend.running_cnt = 0;
    pthread_mutex_init(&g_TpuDev->dev_lock, NULL);
    pthread_mutex_init(&g_TpuDev->close_lock, NULL);
    g_TpuDev->use_count = 0;
    // probe tpu setting
    platform_tpu_probe_setting(g_TpuDev);

    rt_hw_interrupt_install(g_TpuDev->tdma_irq_num, cvi_tpu_tdma_irq, g_TpuDev, "cvi-tpu-tdma");
    rt_hw_interrupt_umask(g_TpuDev->tdma_irq_num);

    ret = work_thread_init(g_TpuDev);
    if (ret < 0) {
        tpu_printf(TPU_ERR, "work thread init error\r\n");
        return ret;
    }
    tpu_printf(TPU_DBG, "exit cvi_tpu_probe\r\n");

    return 0;
}

// unused function
int cvi_tpu_remove(void)
{
    if (g_TpuDev->kernel_work.work_run == 1) {
        g_TpuDev->kernel_work.work_run = 0;
        pthread_join(g_TpuDev->kernel_work.work_thread, NULL);
    }

    pthread_cancel(g_TpuDev->kernel_work.work_thread);

    pthread_mutex_destroy(&tpu_suspend.mutex_lock);
    pthread_mutex_destroy(&g_TpuDev->dev_lock);
    pthread_mutex_destroy(&g_TpuDev->close_lock);
    pthread_mutex_destroy(&g_TpuDev->kernel_work.task_list_lock);
    pthread_mutex_destroy(&g_TpuDev->kernel_work.done_list_lock);

    rt_free(g_TpuDev);
    g_TpuDev = NULL;
    tpu_printf(TPU_DBG, "===cvi_tpu_remove\r\n");
    return 0;
}

void cvi_tpu_init(void)
{
    if (g_TpuDev) {
        tpu_printf(TPU_WARN, "tpu driver already initialized\r\n");
    } else {
        platform_tpu_init(NULL);
        cvi_tpu_probe();
    }
}

void cvi_tpu_deinit(void)
{
    if (g_TpuDev) {
        platform_tpu_deinit(NULL);
        cvi_tpu_remove();
    } else {
        tpu_printf(TPU_WARN, "tpu driver already deinitialization\r\n");
    }
}

struct TPU_PMUCONFIG {
    u8                enable;
    u8                enable_tpu;
    u8                enable_tdma;
    enum TPU_PMUEVENT event;
    u16               tpu_syncID_start;
    u16               tpu_syncID_end;
    u16               tdma_syncID_start;
    u16               tdma_syncID_end;
    u32               bufBaseAddr;  // register setting must right shirt 4bits, value >> 4
    u32               bufSize;      // register setting must right shirt 4bits, value >> 4
};

#define TPUPMU_CTRL    (0x200)
#define TPUPMU_BUFBASE (0x20C)
#define TPUPMU_BUFSIZE (0x210)

#define TPUPMU_BUFGUARD 0x12345678

static void TPUPMU_Config(struct TPU_PLATFORM_CFG *pCfg, struct TPU_PMUCONFIG *pconfig)
{
    u32 regValue = 0;

    if (pconfig->enable) {
        // set buffer starting and size
        RAW_WRITE32(pCfg->iomem_tdmaBase + TPUPMU_BUFBASE, pconfig->bufBaseAddr);
        RAW_WRITE32(pCfg->iomem_tdmaBase + TPUPMU_BUFSIZE, pconfig->bufSize);

        // set enable related
        regValue |= 0x1;
        if (pconfig->enable_tpu) regValue |= 0x8;

        if (pconfig->enable_tdma) regValue |= 0x10;

        // set event type
        regValue |= (pconfig->event << 5);

        // set burst length = 16
        regValue |= (0x3 << 8);

        // enable pmu ring buffer mode
        regValue |= (0x1 << 10);

        // enable pmu dcm
        regValue &= ~0xFFFF0000;

        // set control register
        RAW_WRITE32(pCfg->iomem_tdmaBase + TPUPMU_CTRL, regValue);
    } else {
        // disable register
        regValue = RAW_READ32(pCfg->iomem_tdmaBase + TPUPMU_CTRL);
        RAW_WRITE32(pCfg->iomem_tdmaBase + TPUPMU_CTRL, regValue & ~(0x1));
    }
}

int TPUPMU_Enable(struct TPU_PLATFORM_CFG *pCfg, u8 enable, enum TPU_PMUEVENT event)
{
    struct TPU_PMUCONFIG config;

    if (enable) {
        u64 bufAddr = pCfg->pmubuf_addr_p;
        u64 bufSize = pCfg->pmubuf_size;

        // right shift 4 bits
        bufAddr = bufAddr >> 4;
        bufSize = bufSize >> 4;

        config.enable      = 1;
        config.event       = event;
        config.enable_tdma = 1;
        config.enable_tpu  = 1;
        config.bufBaseAddr = bufAddr;
        config.bufSize     = bufSize;
    } else {
        config.enable = 0;
    }

    TPUPMU_Config(pCfg, &config);
    return 0;
}
