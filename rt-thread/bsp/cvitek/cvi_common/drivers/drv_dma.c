#include "drv_dma.h"

#include <rthw.h>
#include <stdlib.h>

#include "cache.h"
#include "hal_debug.h"
#include "hal_dma.h"

dw_dma_t       *dma_array[1]      = {0};
static uint32_t g_inited_ctrl_num = 0U;

static void dma_ch_resume(dw_dma_t *dma, uint8_t ch_mask);
static void dma_dwc_clk_set(int enable)
{
    uint32_t clk_state;

    clk_state = reg_read_32((long unsigned int)DMA_CLK_EN_REG);

    if (enable)
        clk_state |= 1 << CLK_SDMA_AXI_BIT;
    else
        clk_state &= ~(1 << CLK_SDMA_AXI_BIT);

    reg_write_32((long unsigned int)DMA_CLK_EN_REG, clk_state);
}

static void dma_off(dw_dma_t *dw_dma)
{
    dma_writeq(dw_dma, CFG, 0); /* disable dmac and interrupt */
    while (dma_readq(dw_dma, CFG) & 3) barrier();
}

void dma_on(dw_dma_t *dw_dma)
{
    dma_writeq(dw_dma, CFG, DW_CFG_DMA_EN | DW_CFG_DMA_INT_EN);
}

void dma_write_cfg(dw_dma_channel_t *dwc, uint64_t cfg)
{
    channel_writeq(dwc, CFG, cfg);
}

void dma_enable_irq(dw_dma_channel_t *dwc, uint64_t int_status_reg)
{
    channel_writeq(dwc, INTSTATUS_ENABLEREG, int_status_reg);
    channel_writeq(dwc, INTSIGNAL_ENABLEREG, int_status_reg);
}

void dma_write_llp(dw_dma_channel_t *dwc, uint64_t llp)
{
    channel_writeq(dwc, LLP, llp);
}

uint64_t dma_get_ch_en_status(dw_dma_t *dw_dma)
{
    return dma_readq(dw_dma, CH_EN);
}

void dma_ch_on(dw_dma_t *dw_dma, uint8_t ch_mask)
{
    uint64_t ch_en;

    ch_en = dma_readq(dw_dma, CH_EN);
    ch_en |= (ch_mask << DW_DMAC_CH_EN_WE_OFFSET) | ch_mask;
    dma_writeq(dw_dma, CH_EN, ch_en);
}

void dma_ch_off(dw_dma_t *dw_dma, uint8_t ch_mask)
{
    uint64_t dma_ch_en;

    dma_ch_en = dma_readq(dw_dma, CH_EN);
    if (dma_ch_en & (1 << (__ffs(ch_mask) + DW_DMAC_CH_PAUSE_OFFSET)))
        dma_ch_resume(dw_dma, ch_mask);
    dma_ch_en |= (ch_mask << DW_DMAC_CH_EN_WE_OFFSET);
    dma_ch_en &= ~ch_mask;
    // dma_writeq(dw_dma, CH_EN, dma_ch_en);
    while (dma_readq(dw_dma, CH_EN) & ch_mask) barrier();
}

void dma_ch_pause(dw_dma_t *dw_dma, uint8_t ch_mask)
{
    unsigned int count = 20; /* timeout iterations */
    uint64_t     mask  = ch_mask;

    dma_set_bit(dw_dma, CH_EN,
                (1 << (__ffs(mask) + DW_DMAC_CH_PAUSE_OFFSET)) |
                    (1 << (__ffs(mask) + DW_DMAC_CH_PAUSE_EN_OFFSET)));

    while (!(dma_readq(dw_dma, CH_EN) & (1 << (__ffs(ch_mask) + DW_DMAC_CH_PAUSE_OFFSET))) &&
           count--)
        rt_hw_us_delay(2);
}

void dma_ch_resume(dw_dma_t *dw_dma, uint8_t ch_mask)
{
    uint64_t mask = ch_mask;
    uint64_t dma_ch_en;

    dma_ch_en = dma_readq(dw_dma, CH_EN);
    dma_ch_en |= (mask << DW_DMAC_CH_EN_WE_OFFSET);
    dma_ch_en |= (1 << (__ffs(mask) + DW_DMAC_CH_PAUSE_EN_OFFSET));
    dma_ch_en &= ~(1 << (__ffs(mask) + DW_DMAC_CH_PAUSE_OFFSET));
    dma_writeq(dw_dma, CH_EN, dma_ch_en);
}

uint64_t dma_get_intstatus(dw_dma_t *dw_dma)
{
    return dma_readq(dw_dma, INTSTATUS);
}

void dma_clear_comm_intstatus(dw_dma_t *dw_dma)
{
    dma_writeq(dw_dma, COMM_INTCLEAR, 0x10f);
}

/* return dwc status */
uint64_t dma_dwc_read_clear_intstatus(dw_dma_channel_t *dwc)
{
    uint64_t dwc_status;

    dwc_status = channel_readq(dwc, INTSTATUS);
    channel_writeq(dwc, INTCLEARREG, dwc_status);

    return dwc_status;
}

static void dma_clear_channle_intstatus(dw_dma_channel_t *dwc)
{
    channel_writeq(dwc, INTCLEARREG, 0xffffffff);
}

static void dma_turn_off_chans(dw_dma_t *dw_dma)
{
    dma_set_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_EN_WE_OFFSET);
    dma_set_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_PAUSE_EN_OFFSET);
    dma_set_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_ABORT_EN_OFFSET);
    dma_clear_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK);
    dma_clear_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_PAUSE_OFFSET);
    dma_clear_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_ABORT_OFFSET);
    dma_clear_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_EN_WE_OFFSET);
    dma_clear_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_PAUSE_EN_OFFSET);
    dma_clear_bit(dw_dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_ABORT_EN_OFFSET);
}

static void inline dma_reset(dw_dma_t *dw_dma)
{
    dma_writeq(dw_dma, RESET, 1);
}

static inline void dma_int_mux_set_c906b(void)
{
    reg_write_32(SDMA_DMA_INT_MUX, SDMA_DMA_INT_MUX_C906B);
}

static inline void dma_int_mux_set_c906l(void)
{
    reg_write_32(SDMA_DMA_INT_MUX, SDMA_DMA_INT_MUX_C906L);
}

static inline uint32_t dma_int_mux_get(void)
{
    return reg_read_32(SDMA_DMA_INT_MUX);
}

static uint32_t remap0_val;
static uint32_t remap1_val;

static struct dma_remap_item remap_table[] = {
    {CVI_UART2_TX, 1}, {CVI_UART2_RX, 0}, {CVI_I2S3_TX, 2}, {CVI_I2S0_RX, 3},
    {CVI_SPI1_RX, 4},  {CVI_SPI1_TX, 5},  {CVI_I2S1_RX, 6}, {CVI_SPI_NAND, 7},
};

static void dma_remap(request_index_t hs_id, uint8_t channel_id)
{
    if (channel_id < 4) {
        remap0_val |= hs_id << (channel_id << 3);
    } else {
        channel_id -= 4;
        remap1_val |= hs_id << (channel_id << 3);
    }
}

void dma_remap_init(void)
{
    int table_size = sizeof(remap_table) / sizeof(struct dma_remap_item);

    for (int i = 0; i < table_size; i++) {
        dma_remap(remap_table[i].hs_id, remap_table[i].channel_id);
    }

    remap0_val |= 1 << 31;
    remap1_val |= 1 << 31;

    reg_write_32(REG_SDMA_DMA_CH_REMAP0, remap0_val);
    reg_write_32(REG_SDMA_DMA_CH_REMAP1, remap1_val);

    dma_dbg("remap0=0x%x\r\n", reg_read_32(REG_SDMA_DMA_CH_REMAP0));
    dma_dbg("remap1=0x%x\r\n", reg_read_32(REG_SDMA_DMA_CH_REMAP1));
}

static uint8_t convert_msize(int items)
{
    uint8_t ret;

    if (items > 1)
        ret = __fls(items) - 2;
    else
        ret = 0;

    return ret;
}

static uint32_t get_power(uint32_t base, uint32_t power)
{
    uint32_t ret, i;
    ret = 1U;

    for (i = 0U; i < power; i++) {
        ret *= base;
    }

    return ret;
}

static uint64_t dwc_prepare_default_ctl(struct dw_dma_channel *dwc)
{
    uint64_t           ctl       = 0;
    struct dw_dma_cfg *cfg       = &dwc->cfg;
    int                group_len = cfg->group_len;

    ctl |= DWC_CTL_SMS(cfg->master);
    ctl |= DWC_CTL_DMS(cfg->master);
    switch (cfg->trans_dir) {
    case DMA_MEM2DEV:
        group_len /= get_power(2U, (uint32_t)(cfg->dst_tw));
        ctl |= DWC_CTL_SRC_MSIZE(convert_msize(group_len));
        ctl |= DWC_CTL_DST_MSIZE(convert_msize(group_len));
        break;
    case DMA_DEV2MEM:
        group_len /= get_power(2U, (uint32_t)cfg->src_tw);
        ctl |= DWC_CTL_SRC_MSIZE(convert_msize(group_len));
        ctl |= DWC_CTL_DST_MSIZE(convert_msize(group_len));
        break;
    case DMA_MEM2MEM:
        ctl |= DWC_CTL_SRC_MSIZE(DW_DMA_MSIZE_32);
        ctl |= DWC_CTL_DST_MSIZE(DW_DMA_MSIZE_32);
        break;
    }

    return ctl;
}

static void *dw_desc_alloc(void)
{
    struct dw_desc *desc;
    uint64_t        tmp;

    // 64bytes align
    tmp = (uint64_t)rt_malloc(sizeof(struct dw_desc) + (1 << 6) - 1);
    if (!tmp) {
        return NULL;
    }
    memset((void *)tmp, 0, sizeof(struct dw_desc) + (1 << 6) - 1);

    dma_dbg("desc_alloc tmp=%lx, size=%lu\r\n", tmp, sizeof(struct dw_desc) + (1 << 6) - 1);
    desc           = (void *)((tmp + (1 << 6) - 1) & ~((1 << 6) - 1));
    desc->raw_addr = (void *)tmp;
    dma_dbg("desc_alloc desc->raw_addr=%p\r\n", desc->raw_addr);

    return desc;
}

static inline struct dw_desc *dw_desc_get()
{
    return dw_desc_alloc();
}

static void dma_clk_enable(dw_dma_t *dw_dma)
{
    int irq_state;

    irq_state = rt_hw_interrupt_disable();
    dw_dma->clk_enable_count++;
    if (dw_dma->clk_enable_count == 1) {
        dma_dwc_clk_set(1);
        dma_on(dw_dma);
    }
    rt_hw_interrupt_enable(irq_state);
}

static void dma_clk_disable(dw_dma_t *dw_dma)
{
    dw_dma->clk_enable_count--;
    if (!dw_dma->clk_enable_count) {
        dma_off(dw_dma);
        dma_dwc_clk_set(0);
    } else if (dw_dma->clk_enable_count < 0)
        dw_dma->clk_enable_count = 0;
}

static void release_descriptor(dw_dma_t *dw_dma, dlist_t *list)
{
    dlist_t         tmp_list;
    dlist_t        *plist;
    struct dw_desc *tmp_desc;
    struct dw_desc *node;

    dlist_init(&tmp_list);

    if (dlist_empty(list)) {
        return;
    }
    hal_list_splice_init(list, &tmp_list);

    dlist_for_each_entry_safe(&tmp_list, plist, node, struct dw_desc, list)
    {
        dma_clk_disable(dw_dma);
        dlist_del(&node->list);
        while (node) {
            tmp_desc = node->next;
            free(node->raw_addr);
            node = tmp_desc;
        }
    }
}

static void dwc_fill_register(dw_dma_channel_t *dwc)
{
    uint64_t cfg;
    uint64_t int_status_reg;

    struct dw_desc *desc = get_first_desc(&dwc->active_list);

    cfg = DWC_CFG_SRC_OSR_LMT((uint64_t)DW_DMA_MAX_NR_REQUESTS - 1) |
          DWC_CFG_DST_OSR_LMT((uint64_t)DW_DMA_MAX_NR_REQUESTS - 1) |
          DWC_CFG_CH_PRIOR((uint64_t)(DW_DMA_MAX_NR_CHANNELS - dwc->ch_id - 1)) |
          DWC_CFG_DST_MULTBLK_TYPE(LINK_LIST) | DWC_CFG_SRC_MULTBLK_TYPE(LINK_LIST);

    switch (dwc->cfg.trans_dir) {
    case DMA_MEM2DEV:
        cfg |= DWC_CFG_DST_PER((uint64_t)dwc->cfg.handshake);
        cfg |= dwc->cfg.hs_polarity ? DWC_CFG_DST_HWHS_POL_L : DWC_CFG_DST_HWHS_POL_H;
        cfg |= DWC_CFG_TT_FC(DW_DMA_FC_D_M2P);
        cfg |= DWC_CFG_HS_SEL_DST_HW;
        break;

    case DMA_DEV2MEM:
        cfg |= DWC_CFG_SRC_PER((uint64_t)dwc->cfg.handshake);
        cfg |= dwc->cfg.hs_polarity ? DWC_CFG_SRC_HWHS_POL_L : DWC_CFG_SRC_HWHS_POL_H;
        cfg |= DWC_CFG_TT_FC((uint64_t)DW_DMA_FC_D_P2M);
        cfg |= DWC_CFG_HS_SEL_SRC_HW;
        break;

    case DMA_MEM2MEM:
        cfg |= DWC_CFG_TT_FC((uint64_t)DW_DMA_FC_D_M2M);
        break;
    }

    dma_dbg("cfg write, CFG=%lx\r\n", cfg);
    dma_write_cfg(dwc, cfg);

    int_status_reg = DWC_CH_INTSTA_DMA_TFR_DONE;

    dma_dbg("enable irq, int_status_reg=%lx\r\n", int_status_reg);
    dma_enable_irq(dwc, int_status_reg);
    dma_write_llp(dwc, (uint64_t)&desc->lli.llp);
}

static void dwc_do_first_queue(dw_dma_channel_t *dwc)
{
    struct dw_dma     *dw_dma = dma_array[dwc->ctrl_id];
    struct dw_dma_cfg *cfg    = &dwc->cfg;
    uint32_t           irq_state;

    irq_state = rt_hw_interrupt_disable();
    if (dlist_empty(&dwc->queue_list)) {
        rt_hw_interrupt_enable(irq_state);
        dma_dbg("queue_list empty\r\n");
        return;
    }

    dlist_move(&dwc->queue_list, &dwc->active_list);
    rt_hw_interrupt_enable(irq_state);

    dwc_fill_register(dwc);

    switch (cfg->trans_dir) {
    case DMA_MEM2DEV:
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)cfg->src_addr, cfg->length);
        break;
    case DMA_DEV2MEM:
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH | RT_HW_CACHE_INVALIDATE, (void *)cfg->dst_addr,
                             cfg->length);
        break;
    case DMA_MEM2MEM:
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)cfg->src_addr, cfg->length);
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)cfg->dst_addr, cfg->length);
        break;
    }

    dma_ch_on(dw_dma, dwc->ch_mask);

    dma_dbg("dwc_do_first_queue finish\r\n");
}

void dwc_complete_all(dw_dma_channel_t *dwc)
{
    dw_dma_t *dw_dma = dma_array[dwc->ctrl_id];
    dlist_t   dlist;

    dlist_init(&dlist);
    if (!dlist_empty(&dwc->active_list)) hal_list_splice_init(&dwc->active_list, &dlist);

    dwc_do_first_queue(dwc);

    if (dwc->callback) {
        dwc->callback(dwc->ch_id, dwc->args);
    }
    release_descriptor(dw_dma, &dlist);
}

static void dma_init(dw_dma_t *dw_dma)
{
    dw_dma->nr_masters  = DW_DMA_MAX_NR_MASTERS;
    dw_dma->nr_channels = DW_DMA_MAX_NR_CHANNELS;
    dw_dma->block_ts    = DW_DWC_MAX_BLOCK_TS;

    for (int i = 0; i < dw_dma->nr_masters; i++) {
        dw_dma->data_width[i] = DW_DMA_MAX_DATA_WIDTH;
    }
}

static uint32_t dw_dma_bytes2block(dw_dma_t *dw_dma, size_t bytes, unsigned int width, size_t *len)
{
    uint32_t block;

    if ((bytes >> width) > dw_dma->block_ts) {
        block = dw_dma->block_ts;
        *len  = dw_dma->block_ts << width;
    } else {
        block = bytes >> width;
        *len  = bytes;
    }

    return block;
}

static inline uint64_t addr_fix_or_inc(struct dw_dma_cfg *cfg)
{
    uint64_t ctl = 0;

    switch (cfg->src_inc) {
    case DMA_ADDR_INC:
        ctl |= DWC_CTL_SRC_INC;
        break;
    case DMA_ADDR_CONSTANT:
        ctl |= DWC_CTL_SRC_FIX;
        break;
    default:
        break;
    }

    switch (cfg->dst_inc) {
    case DMA_ADDR_INC:
        ctl |= DWC_CTL_DST_INC;
        break;
    case DMA_ADDR_CONSTANT:
        ctl |= DWC_CTL_DST_FIX;
        break;
    default:
        break;
    }

    return ctl;
}

void dma_prep_transfer(dw_dma_channel_t *dwc, void *srcaddr, void *dstaddr, uint32_t length)
{
    dw_dma_t          *dw_dma = dma_array[dwc->ctrl_id];
    struct dw_dma_cfg *cfg    = &dwc->cfg;
    unsigned int       data_width;
    size_t             dlen;
    struct dw_desc    *desc;
    uint64_t           ctl;
    uint64_t           block_ts_shift = 0;
    uint32_t           block;
    uint64_t           src_width = 0;
    uint64_t           dst_width = 0;
    uint64_t           sar       = (uint64_t)srcaddr;
    uint64_t           dar       = (uint64_t)dstaddr;
    size_t             irq_state;

    dma_dbg("enter dma_prep_transfer\r\n");

    cfg->src_addr = srcaddr;
    cfg->dst_addr = dstaddr;
    cfg->length   = length;

    ctl = (dwc_prepare_default_ctl(dwc) | addr_fix_or_inc(cfg) | DWC_CTL_DST_STA_EN |
           DWC_CTL_SRC_STA_EN | DWC_CTL_SHADOWREG_OR_LLI_VALID);

    data_width = dw_dma->data_width[dwc->cfg.master];

    dma_dbg("channel data width=%d\r\n", data_width);

    switch (cfg->trans_dir) {
    case DMA_MEM2DEV:
        dst_width = cfg->dst_tw;
        src_width = __ffs((uint64_t)data_width | length | sar);
        break;

    case DMA_DEV2MEM:
        src_width = cfg->src_tw;
        dst_width = __ffs((uint64_t)data_width | length | dar);
        break;

    case DMA_MEM2MEM:
        src_width = __ffs((uint64_t)data_width | length | sar);
        dst_width = __ffs((uint64_t)data_width | length | dar);
        ;
        break;
    }

    dma_dbg("src_width=%lx, dst_width=%lx\r\n", src_width, dst_width);
    dma_dbg("length=%d\r\n", length);

    ctl |= DWC_CTL_SRC_WIDTH(src_width);
    ctl |= DWC_CTL_DST_WIDTH(dst_width);

    block_ts_shift = src_width;

    struct dw_desc *first = NULL;
    struct dw_desc *prev  = NULL;

    while (length) {
        if (is_slave(cfg))
            block = dw_dma_bytes2block(dw_dma, length, block_ts_shift, &dlen);
        else {
            block = length >> block_ts_shift;
            dlen  = length;
        }
        dma_dbg("remain bytes=%d\r\n", length);
        dma_dbg("dlen=%ld\r\n", dlen);

        desc = dw_desc_get();
        if (!desc) {
            dma_err("dw_desc_get failed\r\n");
            goto err;
        }

        dma_dbg("desc_get addr=%p\r\n", desc);

        lli_write(desc, sar, sar);
        lli_write(desc, dar, dar);

        if (cfg->src_inc == DMA_ADDR_INC) sar += dlen;
        if (cfg->dst_inc == DMA_ADDR_INC) dar += dlen;

        length -= dlen;
        if (!length) ctl |= DWC_CTL_SHADOWREG_OR_LLI_LAST;

        lli_write(desc, ctl, ctl);
        dma_dbg("start write block_ts:block=%d\r\n", block);
        lli_write(desc, block_ts, block - 1);

        if (!first) {
            first = desc;
        } else {
            lli_write(prev, llp, (uint64_t)desc);
            rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)&prev->lli, sizeof(struct dw_lli));
            prev->next = desc;
        }
        prev = desc;
        dma_dbg("desc->lli.llp=%lx\r\n", desc->lli.llp);
        dma_dbg("desc->next=%p\r\n", desc->next);
    }

    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)&prev->lli, sizeof(struct dw_lli));

    irq_state = rt_hw_interrupt_disable();
    dlist_add(&first->list, &dwc->queue_list);
    rt_hw_interrupt_enable(irq_state);

    dma_dbg("dma_prep_transfer finish\r\n");

    return;

err:
    dma_err("dma_prep_transfer err\r\n");
    if (first) {
        struct dw_desc *tmp_desc;
        dlist_del(&first->list);
        while (first) {
            tmp_desc = first->next;
            rt_free(first->raw_addr);
            first = tmp_desc;
        }
    }

    return;
}

static void dw_dma_irq_handler(int irqno, void *param)
{
    rt_interrupt_enter();
    dw_dma_t         *dw_dma = param;
    uint64_t          status;
    uint64_t          dwc_status;
    dw_dma_channel_t *dwc;

    status = dma_get_intstatus(dw_dma);
    /* Check if we have any interrupt from the DMAC */
    if (!status) return;

    dma_clear_comm_intstatus(dw_dma); /* clear all common interrupts */
    for (int i = 0; i < dw_dma->nr_channels; i++) {
        dwc        = &dw_dma->chans[i];
        dwc_status = dma_dwc_read_clear_intstatus(dwc);
        if (dwc_status & DWC_CH_INTSTA_DMA_TFR_DONE) {
            dwc_complete_all(dwc);
        }
    }
    rt_interrupt_leave();
}

int cvi_set_ch_hw_param(dw_dma_channel_t *dwc, uint8_t master, uint8_t hs_polarity)
{
    HAL_PARAM_CHK(dwc, HAL_ERROR);

    dw_dma_t *dw_dma = dma_array[dwc->ctrl_id];

    if (master >= dw_dma->nr_masters || hs_polarity > 1) {
        dma_err("master or hs_polarity error\r\n");
        return HAL_UNSUPPORTED;
    }

    dwc->cfg.master      = master;
    dwc->cfg.hs_polarity = hs_polarity;

    return 0;
}

static void cvi_dma_init(dw_dma_t *dw_dma, uint32_t ctrl_id, unsigned long reg_base,
                         uint32_t irq_num)
{
    dma_array[ctrl_id] = dw_dma;
    dma_init(dw_dma);
    dma_array[ctrl_id]->nr_channels = DW_DMA_MAX_NR_CHANNELS;
    dw_dma->regs                    = (struct dw_dma_regs *)reg_base;
    dw_dma->irq_num                 = irq_num;
    dw_dma->idx                     = 0;
    dma_clk_enable(dw_dma);
    dma_reset(dw_dma);

#ifdef __riscv
    dma_int_mux_set_c906l();
    dma_dbg("sdma_dma_int_mux=0x%x\r\n", dma_int_mux_get());
#endif
    rt_hw_interrupt_install(irq_num, dw_dma_irq_handler, dw_dma, "dma");
    rt_hw_interrupt_umask(irq_num);

    // init channel
    dw_dma->alloc_status = 0U;

    /* init each channel */
    for (int i = 0; i < dw_dma->nr_channels; i++) {
        dw_dma_channel_t *dwc = &dw_dma->chans[i];
        dwc->regs             = CH_REG_BASE(dw_dma, i);
        dma_dbg("dwc->ch[%d]_base_reg=%p\r\n", i, dwc->regs);
        dlist_init(&dwc->queue_list);
        dlist_init(&dwc->active_list);
        // clear channel int status
        dma_clear_channle_intstatus(dwc);
        // turn off all channel
        dma_turn_off_chans(dw_dma);
    }

    dma_clk_disable(dw_dma);

    g_inited_ctrl_num++;

    dma_remap_init();
}

static void cvi_dma_uninit(dw_dma_t *dw_dma)
{
    dma_off(dw_dma);
    dma_dwc_clk_set(0);

    rt_hw_interrupt_mask(dw_dma->irq_num);

    dma_array[dw_dma->idx] = NULL;
    g_inited_ctrl_num--;

    for (int i = 0; i < dw_dma->nr_channels; i++) {
        struct dw_dma_channel *dwc = &dw_dma->chans[i];
        release_descriptor(dw_dma, &dwc->queue_list);
        release_descriptor(dw_dma, &dwc->active_list);
    }
}

static int cvi_dma_lock_ch(uint8_t ctrl_idx, uint32_t ch_id)
{
    dw_dma_t         *dw_dma;
    dw_dma_channel_t *dwc;
    uint32_t          irq_flags;
    int               ret          = HAL_OK;
    uint32_t         *alloc_status = &dma_array[ctrl_idx]->alloc_status;

    irq_flags = rt_hw_interrupt_disable();

    if (*alloc_status & ((uint32_t)1 << ch_id)) {
        ret = HAL_UNSUPPORTED;
    } else {
        *alloc_status |= ((uint32_t)1 << ch_id);
    }

    rt_hw_interrupt_enable(irq_flags);

    if (ret) {
        dma_err("DMA Error: ch:%d unavailable\n", ch_id);
        return ret;
    }

    dw_dma = dma_array[ctrl_idx];
    dwc    = &dw_dma->chans[ch_id];
    memset(&dwc->cfg, 0, sizeof(dwc->cfg));
    dwc->ch_mask  = 1 << ch_id;
    dwc->regs     = CH_REG_BASE(dw_dma, ch_id);
    dwc->callback = dwc->args = NULL;
    dwc->ctrl_id              = ctrl_idx;
    dwc->ch_id                = ch_id;

    return ret;
}

static void cvi_dma_ch_stop(uint32_t ctrl_idx, uint32_t ch_idx)
{
    dw_dma_t              *dw_dma = dma_array[ctrl_idx];
    struct dw_dma_channel *dwc    = &dw_dma->chans[ch_idx];

    dma_ch_off(dw_dma, dwc->ch_mask);

    release_descriptor(dw_dma, &dwc->queue_list);
    release_descriptor(dw_dma, &dwc->active_list);
}

static void cvi_dma_ch_pause(uint32_t ctrl_idx, uint32_t ch_idx)
{
    uint32_t               irq_flags;
    dw_dma_t              *dw_dma = dma_array[ctrl_idx];
    struct dw_dma_channel *dwc    = &dw_dma->chans[ch_idx];

    irq_flags = rt_hw_interrupt_disable();
    dma_ch_pause(dw_dma, dwc->ch_mask);

    rt_hw_interrupt_enable(irq_flags);
}

static void cvi_dma_ch_resume(uint32_t ctrl_idx, uint32_t ch_idx)
{
    uint32_t               irq_flags;
    dw_dma_t              *dw_dam = dma_array[ctrl_idx];
    struct dw_dma_channel *dwc    = &dw_dam->chans[ch_idx];

    irq_flags = rt_hw_interrupt_disable();
    dma_ch_resume(dw_dam, dwc->ch_mask);

    rt_hw_interrupt_enable(irq_flags);
}

static void cvi_dma_ch_free(uint32_t ctrl_idx, uint32_t ch_idx)
{
    dw_dma_t *dw_dma = dma_array[ctrl_idx];
    uint32_t  temp_u32;

    temp_u32 = 1U << (uint32_t)(ch_idx);
    if (!(dma_array[ctrl_idx]->alloc_status & temp_u32)) {
        return;
    }

    cvi_dma_ch_stop(ctrl_idx, ch_idx);

    if (dw_dma->alloc_status & temp_u32) dw_dma->alloc_status &= ~((uint32_t)1 << (uint32_t)ch_idx);
}

static void cvi_dma_ch_attach_callback(uint32_t ctrl_idx, uint32_t ch_idx, void *callback,
                                       void *args)
{
    dw_dma_t              *dw_dma = dma_array[ctrl_idx];
    struct dw_dma_channel *dwc    = &dw_dma->chans[ch_idx];

    if (callback) {
        dwc->callback = callback;
        dwc->args     = args;
    }
}

static void cvi_dma_ch_detach_callback(uint32_t ctrl_idx, uint32_t ch_idx)
{
    dw_dma_t              *dw_dma = dma_array[ctrl_idx];
    struct dw_dma_channel *dwc    = &dw_dma->chans[ch_idx];

    dwc->callback = NULL;
    dwc->callback = NULL;
}

int set_dw_config(struct dw_dma_cfg *dw_cfg, hal_dma_ch_config_t *config)
{
    if (config->src_reload_en || config->dst_reload_en) {
        dma_err("src/dst reload_en not supported\r\n");
        return HAL_UNSUPPORTED;
    }

    if (config->half_int_en) {
        dma_err("half_int_en not supported\r\n");
        return HAL_UNSUPPORTED;
    }

    if (config->src_inc == DMA_ADDR_DEC || config->dst_inc == DMA_ADDR_DEC) {
        dma_err("DMA_ADDR_DEC not supported\r\n");
        return HAL_UNSUPPORTED;
    }

    dw_cfg->dst_inc   = (dma_addr_inc_t)config->dst_inc;
    dw_cfg->dst_tw    = (dma_data_width_t)config->dst_tw;
    dw_cfg->group_len = config->group_len;
    dw_cfg->handshake = config->handshake;
    dw_cfg->src_inc   = (dma_addr_inc_t)config->src_inc;
    dw_cfg->src_tw    = (dma_data_width_t)config->src_tw;
    dw_cfg->trans_dir = (dma_trans_dir_t)config->trans_dir;

    return HAL_OK;
}

int cvi_dma_ch_config(int ctrl_idx, int ch_idx, struct dw_dma_cfg *dw_cfg)
{
    struct dw_dma *dma;
    dma = dma_array[ctrl_idx];

    if (!dw_cfg) return HAL_ERROR;

    dw_dma_channel_t *dwc = &dma->chans[ch_idx];

    memcpy(&dwc->cfg, dw_cfg, sizeof(struct dw_dma_cfg));

    return 0;
}

static void cvi_dma_ch_start(uint32_t ctrl_idx, uint32_t ch_idx, void *srcaddr, void *dstaddr,
                             uint32_t length)
{
    struct dw_dma    *dw_dma = dma_array[ctrl_idx];
    dw_dma_channel_t *dwc    = &dw_dma->chans[ch_idx];

    dma_clk_enable(dw_dma);
    dma_dbg("enter cvi_dma_ch_start\r\n");
    dma_dbg("srcaddr=%p, dstaddr=%p, length=%x\r\n", srcaddr, dstaddr, length);

    dma_prep_transfer(dwc, srcaddr, dstaddr, length);

    if (dlist_empty(&dwc->active_list)) dwc_do_first_queue(dwc);
}

/*--------------------------HAL--------------------------------*/
/**
 * @brief Initialize the DMA controller.
 *
 * @note
 * This function initializes the DMA controller for the specified control ID.
 * Currently, only DMA_0 is supported. It sets the base register and IRQ number,
 * allocates memory for the DMA private structure, and initializes the DMA
 * controller.
 *
 * @param dma Pointer to the HAL DMA structure.
 * @param ctrl_id Control ID of the DMA to initialize.
 * @return HAL_OK on success, HAL_UNSUPPORTED for unsupported control IDs.
 */
int hal_dma_init(hal_dma_t *dma, uint32_t ctrl_id)
{
    HAL_PARAM_CHK(dma, HAL_ERROR);
    int       ret = HAL_OK;
    dw_dma_t *dw_dma;

    // Check if the control ID is unsupported
    if (ctrl_id != 0) {
        dma_err("Cvitek DMA Only Support DMA_0");
        return HAL_UNSUPPORTED;
    }

    // Set base register and IRQ number for DMA
    dma->reg_base = DW_DMA_BASE;
    dma->irq_num  = DW_DMA_IRQn;
    dma_dbg("DW_DMA: Irq_num=%d, Base_Reg=0x%x\n", dma->irq_num, dma->reg_base);

    // Allocate and initialize the DMA private structure
    dw_dma = rt_malloc(sizeof(dw_dma_t));
    HAL_PARAM_CHK(dw_dma, HAL_ERROR);
    memset(dw_dma, 0, sizeof(dw_dma_t));

    dma->priv   = dw_dma;
    dma->ch_num = DW_DMA_MAX_NR_CHANNELS;

    // Initialize the DMA controller
    cvi_dma_init(dw_dma, ctrl_id, dma->reg_base, dma->irq_num);

    return ret;
}

/**
 * @brief Deinitialize the DMA controller.
 *
 * @note
 * This function deinitializes the DMA controller for the specified control ID.
 * Currently, only DMA_0 is supported. It releases resources and cleans up
 * the DMA private structure associated with the given control ID.
 *
 * @param dma Pointer to the HAL DMA structure to be deinitialized.
 * @param ctrl_id Control ID of the DMA to deinitialize.
 */
void hal_dma_deinit(hal_dma_t *dma, uint32_t ctrl_id)
{
    HAL_PARAM_CHK_NORETVAL(dma);

    if (ctrl_id != 0) {
        dma_err("Cvitek DMA Only Support DMA_0\n");
        return;
    }

    dw_dma_t *dw_dma = dma->priv;

    // Uninitialize the DMA controller
    cvi_dma_uninit(dw_dma);

    // Release resources
    rt_free(dw_dma);
}

/**
 * @brief Request a DMA channel from the DMA controller.
 *
 * @note
 * This function requests a DMA channel from the DMA controller for the
 * specified control ID. The channel ID is used to identify the requested
 * channel. If the request is successful, the channel ID and control ID are
 * stored in the HAL DMA channel structure. If the request fails, an error code
 * is returned.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 * @param ch_id Channel ID of the DMA channel to request.
 * @param ctrl_id Control ID of the DMA controller.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
int hal_dma_channel_request(hal_dma_ch_t *hal_dma_ch, uint32_t ch_id, uint32_t ctrl_id)
{
    HAL_PARAM_CHK(hal_dma_ch, HAL_ERROR);

    int ret = HAL_ERROR;

    // Request the DMA channel
    ret = cvi_dma_lock_ch(ctrl_id, ch_id);
    if (ret) {
        // If the request fails, print an error message
        dma_err("DMA request channel fail\n");
        return ret;
    }

    // Store the channel ID and control ID in the HAL DMA channel structure
    hal_dma_ch->ch_id     = ch_id;
    hal_dma_ch->ctrl_id   = ctrl_id;
    hal_dma_ch->etb_ch_id = -1;

    return ret;
}

/**
 * @brief Stop the DMA channel.
 *
 * @note
 * This function stops the DMA channel specified by the HAL DMA channel
 * structure.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 */
void hal_dma_ch_stop(hal_dma_ch_t *hal_dma_ch)
{
    HAL_PARAM_CHK_NORETVAL(hal_dma_ch);

    cvi_dma_ch_stop(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id);
}

/**
 * @brief Release the DMA channel.
 *
 * @note
 * This function releases the DMA channel specified by the HAL DMA channel
 * structure. The channel ID and control ID are used to identify the channel
 * to be released.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 */
void hal_dma_ch_free(hal_dma_ch_t *hal_dma_ch)
{
    HAL_PARAM_CHK_NORETVAL(hal_dma_ch);

    cvi_dma_ch_free(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id);
}

/**
 * @brief Pause the DMA channel.
 *
 * @note
 * This function pauses the DMA channel specified by the HAL DMA channel
 * structure. The channel ID and control ID are used to identify the channel
 * to be paused.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 */
void hal_dma_ch_pause(hal_dma_ch_t *hal_dma_ch)
{
    HAL_PARAM_CHK_NORETVAL(hal_dma_ch);

    cvi_dma_ch_pause(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id);
}

/**
 * @brief Resume the DMA channel.
 *
 * @note
 * This function resumes the DMA channel specified by the HAL DMA channel
 * structure. The channel ID and control ID are used to identify the channel to
 * be resumed.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 */
void hal_dma_ch_resume(hal_dma_ch_t *hal_dma_ch)
{
    HAL_PARAM_CHK_NORETVAL(hal_dma_ch);

    cvi_dma_ch_resume(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id);
}

/**
 * @brief Default callback function for DMA channel.
 *
 * @note
 * This function is called after a DMA transfer is completed. It will call the
 * callback function registered by the user with the DMA_EVENT_TRANSFER_DONE
 * event and the user-provided argument.
 *
 * @param ch_id Channel ID of the DMA channel.
 * @param args  The user-provided argument.
 */
static void cvi_dma_ch_callback_default(uint32_t ch_id, void *args)
{
    hal_dma_ch_t *hal_dma_ch = args;

    if (hal_dma_ch->callback) {
        hal_dma_ch->callback(hal_dma_ch, DMA_EVENT_TRANSFER_DONE, hal_dma_ch->arg);
    }
}

/**
 * @brief Attach a callback function to a DMA channel.
 *
 * @note
 * This function attaches a callback function to a DMA channel. The callback
 * function is called after a DMA transfer is completed. The callback function
 * receives two arguments: the HAL DMA channel structure and a user-provided
 * argument.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 * @param callback    The callback function to attach.
 * @param arg         The user-provided argument to pass to the callback
 * function.
 *
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
int hal_dma_ch_attach_callback(hal_dma_ch_t *hal_dma_ch, void *callback, void *arg)
{
    int ret = HAL_OK;
    HAL_PARAM_CHK(hal_dma_ch, HAL_ERROR);

    // Store the callback function and user-provided argument in the HAL DMA
    // channel structure.
    hal_dma_ch->callback = callback;
    hal_dma_ch->arg      = arg;

    // Attach the callback function to the DMA channel using the default
    // callback function.
    cvi_dma_ch_attach_callback(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id, cvi_dma_ch_callback_default,
                               hal_dma_ch);

    return ret;
}

/**
 * @brief Detach a callback function from a DMA channel.
 *
 * @note
 * This function detaches the callback function from a DMA channel. The callback
 * function is called after a DMA transfer is completed. The callback function
 * receives two arguments: the HAL DMA channel structure and a user-provided
 * argument.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 */
void hal_dma_ch_detach_callback(hal_dma_ch_t *hal_dma_ch)
{
    HAL_PARAM_CHK_NORETVAL(hal_dma_ch);
    hal_dma_ch->callback = NULL;
    hal_dma_ch->arg      = NULL;

    cvi_dma_ch_detach_callback(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id);
}

/**
 * @brief Configure a DMA channel.
 *
 * @note
 * This function configures a DMA channel using the specified configuration
 * parameters. It validates the input parameters, converts the HAL DMA channel
 * configuration to a DW DMA configuration, and applies the configuration to the
 * specified channel.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 * @param config Pointer to the configuration structure containing the desired
 * settings for the DMA channel.
 * @return HAL_OK on success, HAL_ERROR if any parameter is invalid or if the
 * configuration fails.
 */
int hal_dma_ch_config(hal_dma_ch_t *hal_dma_ch, hal_dma_ch_config_t *config)
{
    // Validate the input parameters
    HAL_PARAM_CHK(hal_dma_ch, HAL_ERROR);
    HAL_PARAM_CHK(config, HAL_ERROR);

    int               ret = HAL_OK;
    struct dw_dma_cfg dw_cfg;

    // Initialize the DW DMA configuration structure to zero
    memset(&dw_cfg, 0, sizeof(dw_cfg));

    // Set the DW DMA configuration based on the provided HAL DMA configuration
    ret = set_dw_config(&dw_cfg, config);
    if (ret) {
        // Return if setting configuration fails
        return ret;
    }

    // Apply the configuration to the DMA channel
    cvi_dma_ch_config(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id, &dw_cfg);

    return ret;
}

/**
 * @brief Start a DMA channel.
 *
 * @note
 * This function starts a DMA channel using the configuration and parameters
 * set by the hal_dma_ch_config() function. It begins the DMA transfer based
 * on the provided source and destination addresses and length.
 *
 * @param hal_dma_ch Pointer to the HAL DMA channel structure.
 * @param srcaddr     Source address of the DMA transfer.
 * @param dstaddr     Destination address of the DMA transfer.
 * @param length      Length of the DMA transfer in bytes.
 */
void hal_dma_ch_start(hal_dma_ch_t *hal_dma_ch, void *srcaddr, void *dstaddr, uint32_t length)
{
    HAL_PARAM_CHK_NORETVAL(hal_dma_ch);

    // Print debug information about the DMA transfer
    dma_dbg(
        "hal_dma_ch_start: ctrl_id=%d, ch_id=%d, srcaddr=%p, dstaddr=%p, "
        "length=%u\r\n",
        hal_dma_ch->ctrl_id, hal_dma_ch->ch_id, srcaddr, dstaddr, length);

    // Start the DMA channel
    cvi_dma_ch_start(hal_dma_ch->ctrl_id, hal_dma_ch->ch_id, srcaddr, dstaddr, length);
}
