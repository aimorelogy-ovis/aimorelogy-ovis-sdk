/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-28     qiujingbao   first version
 * 2024/06/08     flyingcys    fix transmission failure
 */

#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>

#include "board.h"
#include "drv_spi.h"

#include "drv_pinmux.h"
#include "hal_def.h"
#include "hal_dma.h"
#include "hal_debug.h"
#include "hal_spi.h"
#include "cache.h"

#define DBG_LEVEL   DBG_LOG
#include <rtdbg.h>
#define LOG_TAG "drv.spi"

struct dw_spi g_dws[MAX_SPI_NUM]= {0};

static void interrupt_transfer(hal_spi_t *spi)
{
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	uint16_t irq_status = dw_readl(dws, CVI_DW_SPI_ISR);

	/* Error handling */
	if (dw_spi_check_status(dws, false))
		return;

	if (dws->rx) {
		dw_reader(dws);
		if (!dws->rx_len) {
			spi_mask_intr(dws, 0xff);

			if (spi->callback)
				spi->callback(spi, SPI_EVENT_RECEIVE_COMPLETE, spi->arg);

		} else if (dws->rx_len <= dw_readl(dws, CVI_DW_SPI_RXFTLR)) {
			dw_writel(dws, CVI_DW_SPI_RXFTLR, dws->rx_len - 1);
		}
	}

	if (irq_status & CVI_SPI_INT_TXEI) {
		spi_mask_intr(dws, CVI_SPI_INT_TXEI);
		dw_writer(dws);
		spi_umask_intr(dws, CVI_SPI_INT_TXEI);
		if (!dws->tx_len) {
			spi_mask_intr(dws, CVI_SPI_INT_TXEI);
			if (spi->callback)
				spi->callback(spi, SPI_EVENT_SEND_COMPLETE, spi->arg);
		}
	}
	return;
}

static void dw_spi_irq(int irq, void *args)
{
	hal_spi_t *spi = (hal_spi_t *)args;
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	uint32_t irq_status = dw_readl(dws, CVI_DW_SPI_ISR) & 0x3f;

	if (!irq_status)
		return;

	dws->transfer_handler(spi);
}

static int dma_transfer(hal_spi_t *spi)
{
	hal_dma_ch_config_t tx_config, rx_config;
	uint8_t             dma_data_width;

	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	if (dws->n_bytes == 2)
		dma_data_width = DMA_DATA_WIDTH_16_BITS;
	else
		dma_data_width = DMA_DATA_WIDTH_8_BITS;

	if (dws->tx) {
        memset(&tx_config, 0, sizeof(hal_dma_ch_config_t));
		/* configure tx dma channel */
		tx_config.src_tw = DMA_DATA_WIDTH_32_BITS;
		tx_config.dst_tw = dma_data_width;
		tx_config.src_inc = DMA_ADDR_INC;
		tx_config.dst_inc = DMA_ADDR_CONSTANT;
		tx_config.group_len = 8;
		tx_config.trans_dir = DMA_MEM2DEV;
		tx_config.handshake = 5; /* dma channel 5 */
		hal_dma_ch_config(spi->tx_dma, &tx_config);
		dw_writel(dws, CVI_DW_SPI_DMATDLR, 8);
		spi_enable_dma(dws, 1, 1);
		rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH | RT_HW_CACHE_INVALIDATE, (void *)dws->tx, dws->tx_len);
	}

	if (dws->rx) {
		memset(&rx_config, 0, sizeof(hal_dma_ch_config_t));
        /* configure rx dma channel */
		rx_config.src_tw = dma_data_width;
		rx_config.dst_tw = DMA_DATA_WIDTH_32_BITS;
		rx_config.src_inc = DMA_ADDR_CONSTANT;
		rx_config.dst_inc = DMA_ADDR_INC;
		rx_config.group_len = 8;
		rx_config.trans_dir = DMA_DEV2MEM;
		rx_config.handshake = 4;
		hal_dma_ch_config(spi->rx_dma, &rx_config);
		dw_writel(dws, CVI_DW_SPI_DMARDLR, 7);
		spi_enable_dma(dws, 0, 1);
		rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH | RT_HW_CACHE_INVALIDATE, (void *)dws->rx, dws->rx_len);
	}

	/* rx must be started before tx due to spi instinct */
    if (dws->rx) {
		hal_dma_ch_start(spi->rx_dma, (void *)(dws->regs + CVI_DW_SPI_DR), (void *)dws->rx, dws->rx_len);
	}

    if (dws->tx) {
		hal_dma_ch_start(spi->tx_dma, (void *)dws->tx, (void *)(dws->regs + CVI_DW_SPI_DR), dws->tx_len);
	}

	return 0;
}

static int dw_spi_allocate_txrx_td(hal_spi_t *spi)
{
	HAL_PARAM_CHK(spi, HAL_ERROR);
    
    int ret = HAL_OK;

	spi->tx_dma = (hal_dma_ch_t *)rt_malloc(sizeof(hal_dma_ch_t));
	if (spi->tx_dma == NULL) {
		printf("rt_malloc tx_dma failed!\n");
		return -1;
	}
	spi->tx_dma->parents = spi;
	ret = hal_dma_channel_request(spi->tx_dma, 5, 0);
	if (ret != HAL_OK) {
		printf("dma allocate tx channel failed!\n");
		return ret;
	}

	spi->rx_dma = (hal_dma_ch_t *)rt_malloc(sizeof(hal_dma_ch_t));
	if (spi->rx_dma == NULL) {
		printf("rt_malloc tx_dma failed!\n");
		return -1;
	}
	spi->rx_dma->parents = spi;
	ret = hal_dma_channel_request(spi->rx_dma, 4, 0);
	if (ret != HAL_OK) {
		printf("dma allocate rx channel failed!\n");
		return ret;
	}
	return ret;
}

static int dw_spi_transfer_one(hal_spi_t *spi, const void *tx_buf,
		void *rx_buf, uint32_t len, enum transfer_type  tran_type)
{
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	uint8_t imask = 0;
	uint16_t txlevel = 0;

	dws->tx = NULL;
	dws->tx_end = NULL;
	dws->rx = NULL;
	dws->rx_end = NULL;

	if (tx_buf != NULL) {
		dws->tx = tx_buf;
		dws->tx_end = dws->tx + len;
	}

	if (rx_buf != NULL) {
		dws->rx = rx_buf;
		dws->rx_end = dws->rx + len;
	}

	dws->rx_len = len / dws->n_bytes;
	dws->tx_len = len / dws->n_bytes;

	spi_enable_chip(dws, 0);

	/* For poll mode just disable all interrupts */
	spi_mask_intr(dws, 0xff);

	/* set tran mode */
	set_tran_mode(dws);
	/* cs0 */
	dw_spi_set_cs(dws, true, 0);
	/* enable spi */
	spi_enable_chip(dws, 1);
	rt_hw_us_delay(10);

	if (tran_type == DMA_TRAN) {
		dma_transfer(spi);
	}

	if (tran_type == IRQ_TRAN) {
		/*
		 * Interrupt mode
		 * we only need set the TXEI IRQ, as TX/RX always happen syncronizely
		 */
		if ((dws->fifo_len / 2) < dws->tx_len)
			txlevel = dws->fifo_len / 2;
		else
			txlevel = dws->tx_len;

		dw_writel(dws, CVI_DW_SPI_TXFTLR, txlevel);
		dw_writel(dws, CVI_DW_SPI_RXFTLR, txlevel - 1);
		dws->transfer_handler = interrupt_transfer;
		rt_hw_interrupt_install((uint32_t)dws->irq, dw_spi_irq, (void *)spi, "SPI_IRQ");
        rt_hw_interrupt_umask((uint32_t)dws->irq);
		/* Set the interrupt umask */
		imask |= CVI_SPI_INT_TXEI | CVI_SPI_INT_TXOI | CVI_SPI_INT_RXUI | CVI_SPI_INT_RXOI | CVI_SPI_INT_RXFI;
		spi_umask_intr(dws, imask);
	}
	dw_spi_show_regs(dws);

	if (tran_type == POLL_TRAN)
		return poll_transfer(dws);

	return 0;
}

static int wait_ready_until_timeout(hal_spi_t *spi, uint32_t timeout)
{
	uint32_t timestart = 0U;
	int ret = 0;
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	timestart = rt_tick_get_millisecond();

	while (dw_readl(dws, CVI_DW_SPI_SR) & 0x1) {
		if ((rt_tick_get_millisecond() - timestart) > timeout) {
			ret = HAL_TIMEOUT;
			break;
		}
	}
	return ret;
}

/*--------------------------------HAL--------------------------------*/
void hal_spi_init(hal_spi_t *spi, uint32_t idx)
{
    HAL_PARAM_CHK_NORETVAL(spi);
    struct dw_spi *dws = &g_dws[idx];
    
    memset(dws, 0x0, sizeof(struct dw_spi));
	spi_dbg("dws[%d] info: reg_base=0x%x, irq=%d, index=%d\n", idx, dws->regs, dws->irq, dws->index);
    spi_dbg("spi_configure input\n");

    /* set cs low when spi idle */
    writel(0, (void *)0x030001d0);

    spi->state.writeable = 1U;
	spi->state.readable  = 1U;
	spi->state.error     = 0U;
	spi->send            = NULL;
	spi->receive         = NULL;
	spi->send_receive    = NULL;
	spi->rx_dma          = NULL;
	spi->tx_dma          = NULL;
	spi->rx_data         = NULL;
	spi->tx_data         = NULL;
	spi->callback        = NULL;
	spi->arg             = NULL;
	spi->priv            = (void *)dws;

	dws->regs = (void *)(DW_SPI0_BASE + idx * DW_SPI_REG_SIZE);
	dws->irq = DW_SPI0_IRQn + idx;
	dws->index = 0;

	dws->tx              = NULL;
	dws->tx_end          = NULL;
	dws->rx              = NULL;
	dws->rx_end          = NULL;

    spi_reset_chip(dws);
    spi_hw_init(dws);
    spi_enable_chip(dws, 0);
    rt_hw_us_delay(1000);

}

void hal_spi_uninit(hal_spi_t *spi)
{
	HAL_PARAM_CHK_NORETVAL(spi);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	spi_shutdown_chip(dws);
	spi->send            = NULL;
	spi->receive         = NULL;
	spi->send_receive    = NULL;
	spi->rx_data         = NULL;
	spi->tx_data         = NULL;
	spi->callback        = NULL;
	spi->arg             = NULL;

	rt_free(spi->rx_dma);
	rt_free(spi->tx_dma);
	spi->rx_dma = NULL;
	spi->tx_dma = NULL;
}

int hal_spi_attach_callback(hal_spi_t *spi, void *callback, void *arg)
{
	HAL_PARAM_CHK(spi, HAL_ERROR);
	HAL_PARAM_CHK(callback, HAL_ERROR);

	spi->callback     = callback;
	spi->arg          = arg;
	spi->send         = NULL;
	spi->receive      = NULL;
	spi->send_receive = NULL;


	return HAL_OK;
}

void hal_spi_detach_callback(hal_spi_t *spi)
{
	HAL_PARAM_CHK_NORETVAL(spi);

	spi->callback     = NULL;
	spi->arg          = NULL;
	spi->send         = NULL;
	spi->receive      = NULL;
	spi->send_receive = NULL;
}

int hal_spi_mode(hal_spi_t *spi, hal_spi_mode_t mode)
{
	HAL_PARAM_CHK(spi, HAL_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	int   ret = HAL_OK;

	dw_spi_set_controller_mode(dws, mode);

	return ret;
}

int hal_spi_cp_format(hal_spi_t *spi, hal_spi_cp_format_t format)
{
	HAL_PARAM_CHK(spi, HAL_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	dw_spi_set_polarity_and_phase(dws, format);

	return HAL_OK;
}

uint32_t hal_spi_baud(hal_spi_t *spi, uint32_t baud)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(baud, HAL_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	/*need to ensure the clk */
	return dw_spi_set_clock(dws, SPI_REF_CLK, baud);
}

int hal_spi_frame_len(hal_spi_t *spi, hal_spi_frame_len_t length)
{
	HAL_PARAM_CHK(spi, HAL_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	int   ret = HAL_OK;

	ret = dw_spi_set_data_frame_len(dws, (uint32_t)length);

	return ret;
}

void hal_spi_select_slave(hal_spi_t *spi, uint32_t slave_num)
{
	HAL_PARAM_CHK_NORETVAL(spi);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	dw_spi_set_cs(dws, 1, slave_num);
}

int32_t hal_spi_send(hal_spi_t *spi, const void *data, uint32_t size, uint32_t timeout)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	ret = dw_spi_transfer_one(spi, data, NULL, size, POLL_TRAN);

	return ret;
}

int32_t hal_spi_send_async(hal_spi_t *spi, const void *data, uint32_t size)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	ret = dw_spi_transfer_one(spi, data, NULL, size, IRQ_TRAN);

	return ret;
}

int32_t hal_spi_receive(hal_spi_t *spi, void *data, uint32_t size, uint32_t timeout)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	ret = dw_spi_transfer_one(spi, NULL, data, size, POLL_TRAN);

	return ret;
}

int32_t hal_spi_receive_async(hal_spi_t *spi, void *data, uint32_t size)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	ret = dw_spi_transfer_one(spi, NULL, data, size, IRQ_TRAN);

	return ret;
}

int32_t hal_spi_send_dma(hal_spi_t *spi, const void *data, uint32_t size)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	if (size > 16)
		ret = dw_spi_transfer_one(spi, data, NULL, size, DMA_TRAN);
	else
		ret = dw_spi_transfer_one(spi, data, NULL, size, POLL_TRAN);

	return ret;
}

int32_t hal_spi_receive_dma(hal_spi_t *spi, void *data, uint32_t size)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	if (size > 16)
		ret = dw_spi_transfer_one(spi, NULL, data, size, DMA_TRAN);
	else
		ret = dw_spi_transfer_one(spi, NULL, data, size, POLL_TRAN);

	return ret;
}

int32_t hal_spi_send_receive(hal_spi_t *spi, const void *data_out,
		void *data_in, uint32_t size, uint32_t timeout)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data_out, HAL_ERROR);
	HAL_PARAM_CHK(data_in, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	ret = dw_spi_transfer_one(spi, data_out, data_in, size, POLL_TRAN);

	return ret;
}

int32_t hal_spi_send_receive_async(hal_spi_t *spi, const void *data_out,
								   void *data_in, uint32_t size)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data_out, HAL_ERROR);
	HAL_PARAM_CHK(data_in, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;

	ret = dw_spi_transfer_one(spi, data_out, data_in, size, IRQ_TRAN);

	return ret;
}

int32_t hal_spi_send_receive_dma(hal_spi_t *spi, const void *data_out,
				                 void *data_in, uint32_t size)
{
	HAL_PARAM_CHK(spi,  HAL_ERROR);
	HAL_PARAM_CHK(data_out, HAL_ERROR);
	HAL_PARAM_CHK(data_in, HAL_ERROR);
	HAL_PARAM_CHK(size, HAL_ERROR);
	int32_t  ret   = HAL_OK;
	if (size > 16)
		ret = dw_spi_transfer_one(spi, data_out, data_in, size, DMA_TRAN);
	else
		ret = dw_spi_transfer_one(spi, data_out, data_in, size, POLL_TRAN);
	return ret;
}

static void dw_spi_dma_event_cb(hal_dma_ch_t *hal_dma_ch, dma_event_t event, void *arg)
{

	hal_spi_t *spi = (hal_spi_t *)hal_dma_ch->parents;
	uint32_t mode, val;

	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	/*  00 -- send and receive ==> SPI_EVENT_SEND_RECEIVE_COMPLETE
	 *  01 -- send             ==> SPI_EVENT_SEND_COMPLETE
	 *  02 -- receive          ==> SPI_EVENT_RECEIVE_COMPLETE
	 */
	mode = (dw_readl(dws, CVI_DW_SPI_CTRLR0) >> 8 & 0x3);
	val = dw_readl(dws, CVI_DW_SPI_DMACR);

	if (event == DMA_EVENT_TRANSFER_DONE && !strcmp(arg, "TX")) {
		hal_dma_ch_stop(hal_dma_ch);
		mode = SPI_EVENT_SEND_COMPLETE;
		/* process end of transmit */
		if ((spi->tx_dma != NULL) && (spi->tx_dma->ch_id == hal_dma_ch->ch_id)) {
			if (wait_ready_until_timeout(spi, 20000) != HAL_OK)
				mode = SPI_EVENT_ERROR;
		}

		/* disable the dma op for tx 01b */
		dw_writel(dws, CVI_DW_SPI_DMACR, val &~(1 << 1));
		hal_dma_ch_free(spi->tx_dma);
		if (spi->callback)
			spi->callback(spi, mode, spi->arg);
	}

	if (event == DMA_EVENT_TRANSFER_DONE && !strcmp(arg, "RX")) {
		hal_dma_ch_stop(hal_dma_ch);

		/* disable the dma op for rx (10b) */
		dw_writel(dws, CVI_DW_SPI_DMACR, val &~(1 << 0));
		hal_dma_ch_free(spi->rx_dma);
		if (spi->callback)
            spi->callback(spi, SPI_EVENT_RECEIVE_COMPLETE, spi->arg);
	}

}

int hal_spi_link_dma(hal_spi_t *spi, hal_dma_ch_t *tx_dma, hal_dma_ch_t *rx_dma)
{
	HAL_PARAM_CHK(spi, HAL_ERROR);
	int ret = HAL_OK;

	ret = dw_spi_allocate_txrx_td(spi);
	if (ret != HAL_OK) {
		spi_err("allocate tx or rx td failed!\n");
		return ret;
	}

	if (spi->tx_dma != NULL) {
		spi->send = hal_spi_send_dma;
		hal_dma_ch_attach_callback(spi->tx_dma, dw_spi_dma_event_cb, "TX");
	}

	if (spi->rx_dma != NULL) {
		spi->receive = hal_spi_receive_dma;
		hal_dma_ch_attach_callback(spi->rx_dma, dw_spi_dma_event_cb, "RX");
	}

	if ((spi->tx_dma != NULL) && (spi->rx_dma != NULL))
		spi->send_receive = hal_spi_send_receive_dma;

	return ret;
}



