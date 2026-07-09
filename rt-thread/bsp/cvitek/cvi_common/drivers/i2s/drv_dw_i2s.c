/* SPDX-License-Identifier: GPL-2.0-or-later
 * I2s driver on CVITEK
 *
 * Copyright 2022 CVITEK
 *
 * Author: Ruilong.Chen
 *
 */

#include "drv_dw_i2s.h"
#include <rtdevice.h>
#include <stdlib.h>
#include <unistd.h>

bool concurrent_rx_enable;

static void i2s_dump_reg(struct i2s_tdm_regs *i2s_reg)
{
	rt_kprintf("%s start addr:%p\n", __func__, i2s_reg);
	rt_kprintf("BLK_MODE_SETTING = 0x%x\n", i2s_reg->blk_mode_setting);
	rt_kprintf("FRAME_SETTING = 0x%x\n", i2s_reg->frame_setting);
	rt_kprintf("SLOT_SETTING1 = 0x%x\n", i2s_reg->slot_setting1);
	rt_kprintf("SLOT_SETTING2 = 0x%x\n", i2s_reg->slot_setting2);
	rt_kprintf("DATA_FORMAT = 0x%x\n", i2s_reg->data_format);
	rt_kprintf("BLK_CFG = 0x%x\n", i2s_reg->blk_cfg);
	rt_kprintf("I2S_ENABLE = 0x%x\n", i2s_reg->i2s_enable);
	rt_kprintf("I2S_INT_EN = 0x%x\n", i2s_reg->i2s_int_en);
	rt_kprintf("FIFO_THRESHOLD = 0x%x\n", i2s_reg->fifo_threshold);
	rt_kprintf("I2S_LRCK_MASTER = 0x%x\n", i2s_reg->i2s_lrck_master);
	rt_kprintf("I2S_CLK_CTRL0 = 0x%x\n", i2s_reg->i2s_clk_ctrl0);
	rt_kprintf("I2S_CLK_CTRL1 = 0x%x\n", i2s_reg->i2s_clk_ctrl1);
	rt_kprintf("I2S_PCM_SYNTH = 0x%x\n", i2s_reg->i2s_pcm_synth);
}

void i2s_prepare_clk(void)
{
#define Clock_Gen_base_address 0x03002000
#define CV1835_CLK_SDMA_AUD0_DIV 0x098
#define CV1835_CLK_SDMA_AUD1_DIV 0x09C
#define CV1835_CLK_SDMA_AUD2_DIV 0x0A0
#define CV1835_CLK_SDMA_AUD3_DIV 0x0A4
#define CV1835_CLK_AUDSRC_DIV 0x118

#define PLL_G2_base_address 0x03002800
#define pll_g2_ctrl 0x00
#define CV1835_CLK_A0PLL_CSR 0x0C

#define CV1835_CLK_A0PLL_SSC_SYN_CTRL 0x50
#define CV1835_CLK_A0PLL_SSC_SYN_SET 0x54
#define CV1835_CLK_A0PLL_SSC_SYN_SPAN 0x58
#define CV1835_CLK_A0PLL_SSC_SYN_STEP 0x5C
	static bool initonce;

	u32 aud_div;
	u32 apll_div = mmio_read_32(PLL_G2_base_address + CV1835_CLK_A0PLL_CSR) >> 17;

	if (initonce)
		return;
	initonce = true;

	audio_dbg("%s\n", __func__);
	apll_div = (apll_div & 0x7f);
	aud_div = ((((apll_div << 1) + apll_div) << 16) | 0x9);
	*(u32 *)(Clock_Gen_base_address + CV1835_CLK_SDMA_AUD0_DIV) =
		aud_div;
	*(u32 *)(Clock_Gen_base_address + CV1835_CLK_SDMA_AUD1_DIV) =
		aud_div;
	*(u32 *)(Clock_Gen_base_address + CV1835_CLK_SDMA_AUD2_DIV) =
		aud_div;
	*(u32 *)(Clock_Gen_base_address + CV1835_CLK_SDMA_AUD3_DIV) =
		aud_div;
	aud_div = (((apll_div << 1) << 16) | 0x9);
	*(u32 *)(Clock_Gen_base_address + CV1835_CLK_AUDSRC_DIV) = aud_div;
}

struct i2s_tdm_regs *i2s_get_base(unsigned int i2s_id)
{
	switch (i2s_id) {
	case I2S0:
	return (struct i2s_tdm_regs *)CONFIG_SYS_I2S0_BASE;
	case I2S1:
	return (struct i2s_tdm_regs *)CONFIG_SYS_I2S1_BASE;
	case I2S2:
	return (struct i2s_tdm_regs *)CONFIG_SYS_I2S2_BASE;
	case I2S3:
	return (struct i2s_tdm_regs *)CONFIG_SYS_I2S3_BASE;
	default:
	rt_kprintf("no such I2S device\n");
	break;
	}
	return NULL;
}

int i2s_get_no(unsigned int base_reg)
{
	switch (base_reg) {
	case CONFIG_SYS_I2S0_BASE:
	return I2S0;
	case CONFIG_SYS_I2S1_BASE:
	return I2S1;
	case CONFIG_SYS_I2S2_BASE:
	return I2S2;
	case CONFIG_SYS_I2S3_BASE:
	return I2S3;
	default:
	rt_kprintf("no such I2S device\n");
	break;
	}
	return -1;
}

struct i2s_sys_regs *i2s_get_sys_base(void)
{
	return (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
}

void i2s_select_sound_channel(struct i2s_tdm_regs *i2s_reg, i2s_slot_select ch,
							  u8 direction)
{

	u32 slot_setting2 = 0;
	u32 data_format = readl(&i2s_reg->data_format);

	if (direction == TX_MODE)
		data_format |= (1 << 5); // tx
	else
		data_format |= (1 << 4); // rx

	switch (ch) {
	case I2S_LEFT_CHANNEL:

		slot_setting2 = 0x1;
		mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
	break;
	case I2S_RIGHT_CHANNEL:

		slot_setting2 = 0x2;
		mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
	break;
	case I2S_LEFT_RIGHT_CHANNEL:

		slot_setting2 = 0x3;
	break;
	default:
		rt_kprintf("%s select channel slot error %d\n", __func__, ch);
	break;
	}
	mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, slot_setting2);
}

static void i2s_config_dma(struct i2s_tdm_regs *i2s_reg, bool on,
						   unsigned char fifo_threshold,
						   unsigned char fifo_high_threshold)
{
	u32 blk_mode_setting = 0;
	u32 blk_cfg = 0;

	blk_mode_setting = readl(&i2s_reg->blk_mode_setting) & ~(DMA_MODE_MASK);
	if (on == true) {
		audio_dbg("%s dma mode\n", __func__);
		mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting,
						blk_mode_setting | HW_DMA_MODE); /*not to use FIFO */
		mmio_write_32((uintptr_t)&i2s_reg->fifo_threshold,
						RX_FIFO_THRESHOLD(fifo_threshold) |
							TX_FIFO_THRESHOLD(fifo_threshold) |
							TX_FIFO_HIGH_THRESHOLD(fifo_high_threshold));
		blk_cfg = readl(&i2s_reg->blk_cfg) | AUTO_DISABLE_W_CH_EN | (0x1 << 17);
	} else {
		mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting,
						blk_mode_setting | SW_MODE); /*not to use FIFO */
		mmio_write_32((uintptr_t)&i2s_reg->fifo_threshold,
						RX_FIFO_THRESHOLD(fifo_threshold) |
							TX_FIFO_THRESHOLD(fifo_threshold) |
							TX_FIFO_HIGH_THRESHOLD(fifo_high_threshold));
	}

	blk_cfg = readl(&i2s_reg->blk_cfg) | AUTO_DISABLE_W_CH_EN; //| (0x1 << 16);
	mmio_write_32((uintptr_t)&i2s_reg->blk_cfg, blk_cfg);
	// u32 fifo_depth = (readl(&i2s_reg->fifo_threshold) &
	// TX_FIFO_THRESHOLD_MASK) >> 16;
}

void i2s_set_clk_source(struct i2s_tdm_regs *i2s_reg, unsigned int src)
{
	u32 tmp = 0;

	tmp = readl(&i2s_reg->i2s_clk_ctrl0) & ~(AUD_CLK_SOURCE_MASK);
	tmp &= ~(BCLK_OUT_FORCE_EN);
	switch (src) {
	case AUD_CLK_FROM_MCLK_IN:
	tmp |= AUD_CLK_FROM_MCLK_IN | AUD_ENABLE;
	break;
	case AUD_CLK_FROM_PLL:
	tmp |= AUD_CLK_FROM_PLL | AUD_ENABLE;
	break;
	}
	audio_dbg("%s 0x%x\n", __func__, tmp);
	mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, tmp);
}

void i2s_set_clk_sample_rate(struct i2s_tdm_regs *i2s_reg,
							 unsigned int sample_rate)
{

	audio_dbg("%s sample_rate:%d\n", __func__, sample_rate);
	u32 clk_ctrl1 = 0;
	u32 aud_div = 0;
	u32 apll_div = mmio_read_32(PLL_G2_base_address + CV1835_CLK_A0PLL_CSR) >> 17;

	clk_ctrl1 = (readl(&i2s_reg->i2s_clk_ctrl1) & 0xffff0000);
	/* cv182x internal adc codec need dynamic MCLK frequency input */
	switch (sample_rate) {
	case 8000:
	clk_ctrl1 |= MCLK_DIV(1);
	break;
	case 11025:
	clk_ctrl1 |= MCLK_DIV(4);
	break;
	case 16000:
	case 32000:
	clk_ctrl1 |= MCLK_DIV(1);
	break;
	case 22050:
	case 44100:
	case 48000:
	clk_ctrl1 |= MCLK_DIV(2);
	break;
	default:
	rt_kprintf("%s doesn't support this sample rate\n", __func__);
	break;
	}
	mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl1, clk_ctrl1);
	apll_div = (apll_div & 0x7f);
	if (sample_rate == 48000) {
		aud_div = (((apll_div << 1) << 16) | 0x9);
	} else {
		aud_div = ((((apll_div << 1) + apll_div) << 16) | 0x9);
	}
	*(u32 *)(Clock_Gen_base_address + CV1835_CLK_SDMA_AUD0_DIV) =
		aud_div;
	*(u32 *)(Clock_Gen_base_address + CV1835_CLK_SDMA_AUD3_DIV) =
		aud_div;

}

/*
 * Sets the frame size for I2S sample rate
 *
 * @param i2s_reg	i2s regiter address
 */
void i2s_set_sample_rate(struct i2s_tdm_regs *i2s_reg, unsigned int sample_rate,
						 u32 chan_nr, char *shortname)
{
	u32 frame_setting = 0;
	u32 slot_setting = 0;
	u32 data_format = 0;
	u32 clk_ctrl = 0;
	u32 div_multiplier = 2; //if use audio PLL (25 or 24.576Mhz),
							//div_multiplier should be 2
	u32 clk_ctrl1 = 0;
	u32 audio_clk = 24576000;
	u32 mclk_div = 0;
	u32 bclk_div = 0;

	audio_dbg("Set sample rate to %d, div_multiplier = %d\n", sample_rate,
		div_multiplier);

	frame_setting = readl(&i2s_reg->frame_setting);
	slot_setting = readl(&i2s_reg->slot_setting1);
	data_format = readl(&i2s_reg->data_format);
	clk_ctrl = readl(&i2s_reg->i2s_clk_ctrl1);

	frame_setting &= ~(FRAME_LENGTH_MASK | FS_ACT_LENGTH_MASK); // 4
	slot_setting &= ~(SLOT_SIZE_MASK | DATA_SIZE_MASK);
	data_format &= ~(WORD_LENGTH_MASK | SKIP_TX_INACT_SLOT_MASK); // 4

#if defined(CONFIG_USE_AUDIO_PLL)
	clk_ctrl = MCLK_DIV(2); /* audio PLL is 25 or 24.576 Mhz, need to div with 2*/
	div_multiplier = 2;     //_/ 1;
#else
	clk_ctrl = MCLK_DIV(1); /* mclk_in is 12.288 Mhz, no need to div*/
#endif

	if ((sample_rate == 8000 || sample_rate == 16000 || sample_rate == 32000)) {
		audio_clk = 16384000;
	}
	switch (sample_rate) {
	case 8000:
	case 16000:
	case 32000:
		/* apll is 16.384Mhz, no need to divide */
		clk_ctrl1 |= MCLK_DIV(1);
		mclk_div = 1;
	break;
	case 48000:
		clk_ctrl1 |= MCLK_DIV(2);
		mclk_div = 2;
	break;
	default:
		rt_kprintf("%s doesn't support this sample rate\n", __func__);
	break;
	}

	switch (sample_rate) {
	case 8000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_16;

		clk_ctrl1 |= MCLK_DIV(6);
		mclk_div = 1;

		bclk_div = (audio_clk / 1000) /
					(WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		clk_ctrl1 |= BCLK_DIV(bclk_div);

	break;
	case 12000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_32;
		clk_ctrl |= BCLK_DIV(16 * div_multiplier);
	break;
	case 16000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_16;

		bclk_div = (audio_clk / 1000) /
					(WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		clk_ctrl1 |= BCLK_DIV(bclk_div);
	break;

	case 24000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_32;
		clk_ctrl |= BCLK_DIV(8 * div_multiplier);
	break;
	case 32000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_16;

		bclk_div = (audio_clk / 1000) /
					(WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		clk_ctrl1 |= BCLK_DIV(bclk_div);
	break;

	case 48000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_16;

		bclk_div = (audio_clk / 1000) /
					(WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		clk_ctrl1 |= BCLK_DIV(bclk_div);
	break;

	case 96000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_32;

		bclk_div = (audio_clk / 1000) /
					(WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		clk_ctrl1 |= BCLK_DIV(bclk_div);
	break;

	case 192000:
		frame_setting |= FRAME_LENGTH(64);
		slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
		data_format = WORD_LEN_32;

		bclk_div = (audio_clk / 1000) /
					(WSS_16_CLKCYCLE * (sample_rate / 1000) * mclk_div);
		clk_ctrl1 |= BCLK_DIV(bclk_div);
	break;
	}

	mmio_write_32((uintptr_t)&i2s_reg->frame_setting, frame_setting);
	mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, slot_setting);
	mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
	mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl1, clk_ctrl1);
}

void i2s_set_ws_clock_cycle(struct i2s_tdm_regs *i2s_reg, unsigned int ws_clk,
							u8 aud_mode)
{
	u32 frame_setting = 0;
	u32 slot_setting1 = 0;

	audio_dbg("start Set ws clkcycle to %d ....................\n", ws_clk);
	frame_setting = readl(&i2s_reg->frame_setting) &
					~(FRAME_LENGTH_MASK | FS_ACT_LENGTH_MASK);
	slot_setting1 =
		readl(&i2s_reg->slot_setting1) & ~(SLOT_SIZE_MASK | DATA_SIZE_MASK);

	if ((aud_mode == I2S_MODE) || (aud_mode == LJ_MODE) ||
		(aud_mode == RJ_MODE)) {
		switch (ws_clk) {
		case WSS_16_CLKCYCLE:
			audio_dbg("%s,case WSS_16_CLKCYCLE:\n", __func__);

			frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(16);
			frame_setting |= 0x4000;
			slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);

			break;
		case WSS_24_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(48) | FS_ACT_LENGTH(24);
			slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
			break;
		case WSS_32_CLKCYCLE:
			audio_dbg("%s,case WSS_32_CLKCYCLE:\n", __func__);
			frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(16);
			break;
		case WSS_256_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(256);
			slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
			break;
		}
	} else if ((aud_mode == PCM_A_MODE) || (aud_mode == PCM_B_MODE)) {
		switch (ws_clk) {
		case WSS_16_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
			break;
		case WSS_24_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(48) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
			break;
		case WSS_32_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(32);
			break;
		case WSS_256_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
			break;
		}
	} else if (aud_mode == TDM_MODE) {
		unsigned int slot_no = (readl(&i2s_reg->slot_setting1) & SLOT_NUM_MASK) + 1;

		audio_dbg("set TDM mode with slot number=%d\n", slot_no);
		switch (ws_clk) {
		case WSS_16_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(slot_no * 16) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
			break;
		case WSS_24_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(slot_no * 24) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
			break;
		case WSS_32_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(slot_no * 32) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(32);
			break;
		case WSS_256_CLKCYCLE:
			frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(1);
			slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
			break;
		}
	}

	audio_dbg("Set ws clk cycle to %d\n", ws_clk);
	mmio_write_32((uintptr_t)&i2s_reg->frame_setting, frame_setting);
	mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, slot_setting1);
}

void i2s_set_resolution(struct i2s_tdm_regs *i2s_reg, unsigned int data_size,
						unsigned int slot_size)
{
	u32 slot_setting1 = 0;

	audio_dbg("Set resolution to data_size=%d, slot_size = %d\n", data_size,
			slot_size);
	slot_setting1 =
		readl(&i2s_reg->slot_setting1) & ~(DATA_SIZE_MASK | SLOT_SIZE_MASK);
	slot_setting1 |= DATA_SIZE(data_size) | SLOT_SIZE(slot_size);

	mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, slot_setting1);
}

/*
 * Set the i2s enable
 * @param i2s_req i2s register address
 * @param enable 1 to enable i2s, 0 to disable i2s
 */

void i2s_switch(int on, struct i2s_tdm_regs *i2s_reg)
{
	u32 i2s_enable = readl(&i2s_reg->i2s_enable);
	u32 aud_enable = readl(&i2s_reg->i2s_clk_ctrl0);
	u32 role = (readl(&i2s_reg->blk_mode_setting) & ROLE_MASK);

	if (on) {
		if (i2s_enable == I2S_OFF) {
			mmio_write_32((uintptr_t)&i2s_reg->i2s_enable, I2S_ON);
		}
	} else {
		if (i2s_enable == I2S_ON) {
			mmio_write_32((uintptr_t)&i2s_reg->i2s_enable, I2S_OFF);
		}

		// do not disable AUD_ENABLE due to external codec still need MCLK to do
		// configuration
		if (((aud_enable & AUD_ENABLE) == AUD_ENABLE) && role == MASTER_MODE) {
			// mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, aud_enable &
			// ~(AUD_ENABLE));
		}
	}
}

/*
 * Sets the i2s transfer control
 *
 * @param i2s_reg	i2s regiter address
 * @param on		1 to enable tx , 0 to disable tx transfer
 */
static void i2s_txctrl(struct i2s_tdm_regs *i2s_reg, int on)
{
	u32 blk_mode_setting = 0;
	u32 clk_ctrl = 0;

	blk_mode_setting = (readl(&i2s_reg->blk_mode_setting) & ~(TXRX_MODE_MASK));
	clk_ctrl = (readl(&i2s_reg->i2s_clk_ctrl0) & ~(AUD_SWITCH));

	blk_mode_setting |= TX_MODE;
	mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, blk_mode_setting);

	if ((blk_mode_setting & ROLE_MASK) == MASTER_MODE) {
		if (on) {
			audio_dbg("Enable tx aud_en\n");
			mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_ENABLE);
		} else {
			audio_dbg("Disalbe tx aud_en\n");
			// mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl &
			// ~(AUD_ENABLE));
		}
	} else {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl & ~(AUD_ENABLE));
	}
}

/*
 * Sets the i2s receiver control
 *
 * @param i2s_reg	i2s regiter address
 * @param on		1 to enable rx , 0 to disable rx transfer
 */
static void i2s_rxctrl(struct i2s_tdm_regs *i2s_reg, int on)
{
	u32 blk_mode_setting = 0;
	u32 clk_ctrl = 0;

	blk_mode_setting = (readl(&i2s_reg->blk_mode_setting) & ~(TXRX_MODE_MASK));
	clk_ctrl = (readl(&i2s_reg->i2s_clk_ctrl0) & ~(BCLK_OUT_FORCE_EN));

	blk_mode_setting |= RX_MODE;
	mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, blk_mode_setting);

	if ((blk_mode_setting & ROLE_MASK) == MASTER_MODE) {
		if (on) {
			audio_dbg("Enable rx aud_en\n");
			mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_ENABLE);
		} else {
			audio_dbg("Disalbe rx aud_en\n");
			mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_DISABLE);
		}
	} else {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_DISABLE);
	}
	}

/*
 * flushes the i2stx fifo
 *
 * @param i2s_reg	i2s regiter address
 */
void i2s_sw_reset(struct i2s_tdm_regs *i2s_reg)
{
	int timeout_count = 0;

	if ((readl(&i2s_reg->blk_mode_setting) & TXRX_MODE_MASK) == TX_MODE) {
		mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_UP);
		usleep(10);
		mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_DOWN);

		mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_UP);

		usleep(10);
		while (1) {
			if ((readl(&i2s_reg->tx_status) & RESET_TX_SCLK) >> 23) {
				audio_dbg("TX Reset complete\n");
				break;
			} else if (timeout_count > I2S_TIMEOUT) {
				rt_kprintf("TX Reset Timeout\n");
				break;
			}
			usleep(1000);
			timeout_count++;
		}
		mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_DOWN);

	} else { /* reset RX*/
		u32 val = readl(&i2s_reg->i2s_clk_ctrl0);

		audio_dbg("Reset i2s RX, 0x%x\n", val);
		mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, val | AUD_ENABLE);

		mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_UP);
		usleep(10);
		mmio_write_32((uintptr_t)&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_DOWN);

		mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_UP);
		usleep(10);
		timeout_count = 0;
		while (1) {
			u32 tmp = readl(&i2s_reg->rx_status);

			if ((tmp & RESET_RX_SCLK) >> 23) {
				rt_kprintf("RX Reset complete\n");
				break;
			} else if (timeout_count > I2S_TIMEOUT) {
				rt_kprintf("RX Reset Timeout\n");
				break;
			}
			usleep(1000);
			timeout_count++;
		}
		mmio_write_32((uintptr_t)&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_DOWN);

		mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, val);
	}
}

void i2s_set_interrupt(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
	if (stream == CVI_PCM_STREAM_PLAYBACK)
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en,
						I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU);
	else
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en,
						I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU);
}

void i2s_disable_all_interrupt(struct i2s_tdm_regs *i2s_reg)
{
	mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en, 0x00);
}

void i2s_clear_irqs(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
	u32 irq = readl(&i2s_reg->i2s_int);

	/* I2S_INT is write 1 clear */
	if (stream == CVI_PCM_STREAM_PLAYBACK) {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int, irq &
			(I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU
				| I2S_INT_TXDA_RAW | I2S_INT_TXFO_RAW | I2S_INT_TXFU_RAW));
	} else {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int, irq &
			(I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU
				| I2S_INT_RXDA_RAW | I2S_INT_RXFO_RAW | I2S_INT_RXFU_RAW));
	}

}

void i2s_disable_irqs(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
	u32 irq = readl(&i2s_reg->i2s_int_en);

	if (stream == CVI_PCM_STREAM_PLAYBACK) {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en,
			irq & ~(I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU));
	} else {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en,
			irq & ~(I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU));
	}
}

void i2s_enable_irqs(struct i2s_tdm_regs *i2s_reg, u32 stream)
{
	u32 irq = readl(&i2s_reg->i2s_int_en);

	if (stream == CVI_PCM_STREAM_PLAYBACK) {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en,
						irq | I2S_INT_TXDA | I2S_INT_TXFO | I2S_INT_TXFU);
	} else {
		mmio_write_32((uintptr_t)&i2s_reg->i2s_int_en,
		irq | I2S_INT_RXDA | I2S_INT_RXFO | I2S_INT_RXFU);
	}
}

void i2s_set_audio_gpio(int role)
{
	switch (role) {
	case MASTER_MODE:
		mmio_clrsetbits_32(REG_AUDIO_GPIO_BASE, 0xf << 0, 0x1);
	break;
	case SLAVE_MODE:
		mmio_clrsetbits_32(REG_AUDIO_GPIO_BASE, 0xf << 0, 0xB);
	break;
	default:
	break;
	}
}

/*
 * Sets I2S Clcok format
 *
 * @param fmt		i2s clock properties
 * @param i2s_reg	i2s regiter address
 *
 * @return		int value 0 for success, -1 in case of error
 */

int i2s_set_fmt(struct i2s_tdm_regs *i2s_reg, unsigned char role,
				unsigned char aud_mode, unsigned int fmt,
				unsigned char slot_no)
{
	unsigned int tmp = 0;
	unsigned int tmp2 = 0;
	unsigned int codec_fmt = 0;

	tmp = readl(&i2s_reg->frame_setting) &
		~(FS_OFFSET_MASK | FS_IDEF_MASK | FS_ACT_LENGTH_MASK);
	tmp2 = readl(&i2s_reg->slot_setting1) & ~(SLOT_NUM_MASK);
	// rt_kprintf("[%s] slot_no %d slot_set1:%x\n", __func__, slot_no, tmp2);

/* For cv181x DAC codec, while playing with mono audio data,
 * need to assume there are 2 channels but skip 1. Thus, need
 * to set frame length as 32, slot_num as 2, slot_en as 1 and
 * skip tx inactivate slot. I2S will duplucate 16 bits into
 * another skiped channel
 */
	slot_no = slot_no == 1 ? 2 : slot_no;
	rt_kprintf("[%s] slot_no %d slot_set1:%x\n", __func__, slot_no, tmp2);
	switch (aud_mode) {
	case I2S_MODE:
		tmp |= FS_OFFSET_1_BIT | FS_IDEF_FRAME_SYNC |
				FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK) + 1) / 2);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_I2S;
	break;
	case LJ_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC |
				FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK) + 1) / 2);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_LEFT_J;
	break;
	case RJ_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC |
				FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK) + 1) / 2);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 &= ~(FB_OFFSET_MASK);
		tmp2 |= SLOT_NUM(slot_no) |
			FB_OFFSET((((tmp & FS_ACT_LENGTH_MASK) >> 16) -
			((tmp2 & DATA_SIZE_MASK) >> 16)));
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_RIGHT_J;
	break;
	case PCM_A_MODE:
		tmp |= FS_OFFSET_1_BIT | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_DSP_A;
	break;
	case PCM_B_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_DSP_B;
	break;
	case TDM_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp);
		tmp2 |= SLOT_NUM(slot_no);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting1, tmp2);
		mmio_write_32((uintptr_t)&i2s_reg->slot_setting2,
						0x0f); /* enable slot 0-3 for TDM */
		codec_fmt |= SND_SOC_DAIFMT_PDM;
	break;
	default:

		rt_kprintf("%s: Invalid format\n", __func__);
	return -1;
	}
	// rt_kprintf("[%s] slot_no %d slot_set1:%x\n", __func__, slot_no,
	// readl(&i2s_reg->slot_setting1));
	tmp = readl(&i2s_reg->blk_mode_setting) &
		~(SAMPLE_EDGE_MASK |
			FS_SAMPLE_RX_DELAY_MASK); /* clear bit 2~4 to set frame format */
	tmp2 = readl(&i2s_reg->frame_setting) &
			~(FS_POLARITY_MASK); /* clear bit 12 to set fs polarity */
	if ((aud_mode == I2S_MODE) || (aud_mode == LJ_MODE) ||
		(aud_mode == RJ_MODE)) {
		switch (fmt) {
		case FMT_IB_NF:
		#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
		#else

			tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
		#endif
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_LOW;
			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_NF;
			break;

		case FMT_IB_IF:
		#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
		#else
			tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
		#endif
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);

			tmp2 |= FS_ACT_HIGH;

			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_IF;
			break;

		case FMT_NB_NF:
			audio_dbg("Set format to NBNF\n");
		#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N; /* for crx  */
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P;
		#else

			tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
		#endif

			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_LOW;
			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_NB_NF;
			break;
		case FMT_NB_IF:
			audio_dbg("Set format to NBIF\n");
		#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P;
		#else
			tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
		#endif
			mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_HIGH;
			mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_NB_IF;
			break;
		default:
			rt_kprintf("%s: Invalid clock ploarity input\n", __func__);
			return -1;
		}
	} else {
		#ifdef CONFIG_SHIFT_HALF_T
		if (role == MASTER_MODE)
			tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
		else
			tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
		#else
		tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
		#endif
		mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
		tmp2 |= FS_ACT_HIGH;
		mmio_write_32((uintptr_t)&i2s_reg->frame_setting, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_IB_IF;
	}

	tmp = readl(&i2s_reg->blk_mode_setting) &
		~(ROLE_MASK); /* clear bit 2~4 to set frame format */
	switch (role) {
	case MASTER_MODE:
		tmp |= MASTER_MODE;
		mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
		codec_fmt |= SND_SOC_DAIFMT_CBS_CFS; /* Set codec to slave */
	break;
	case SLAVE_MODE:
		tmp |= SLAVE_MODE;
		mmio_write_32((uintptr_t)&i2s_reg->blk_mode_setting, tmp);
		codec_fmt |= SND_SOC_DAIFMT_CBM_CFM; /* Set codec to master*/
	break;
	default:

		rt_kprintf("%s: Invalid master selection\n", __func__);
		return -1;
	}

	return 0;
}

int i2s_enable(struct i2s_config_info *pi2s_info)
{
	struct i2s_tdm_regs *i2s_reg = pi2s_info->base_address;
	u32 fifo_depth =
		(readl(&i2s_reg->fifo_threshold) & RX_FIFO_THRESHOLD_MASK);
	audio_dbg("%s, i2s_reg base=%p, fifo_depth = %d,\n", __func__, i2s_reg,
			fifo_depth);

	if (pi2s_info->direction == TX_MODE) {
		i2s_txctrl(i2s_reg, I2S_TX_ON);
		i2s_sw_reset(i2s_reg);
		i2s_clear_irqs(i2s_reg, CVI_PCM_STREAM_PLAYBACK);
		i2s_enable_irqs(i2s_reg, CVI_PCM_STREAM_PLAYBACK);
	}
	if (pi2s_info->direction == RX_MODE) {
		i2s_rxctrl(i2s_reg, I2S_RX_ON);
		i2s_sw_reset(i2s_reg);
		i2s_clear_irqs(i2s_reg, CVI_PCM_STREAM_CAPTURE);
		i2s_enable_irqs(i2s_reg, CVI_PCM_STREAM_CAPTURE);
	}

	i2s_switch(I2S_ON, i2s_reg);
	i2s_dump_reg(i2s_reg);
	return 0;
}
int i2s_disable(struct i2s_config_info *pi2s_info)
{
	struct i2s_tdm_regs *i2s_reg = pi2s_info->base_address;

	if (pi2s_info->direction == TX_MODE) {

		i2s_sw_reset(i2s_reg);
		i2s_clear_irqs(i2s_reg, CVI_PCM_STREAM_PLAYBACK);
		i2s_disable_irqs(i2s_reg, CVI_PCM_STREAM_PLAYBACK);
		i2s_txctrl(i2s_reg, I2S_TX_OFF);
	}

	if (pi2s_info->direction == RX_MODE) {

		i2s_sw_reset(i2s_reg);
		i2s_clear_irqs(i2s_reg, CVI_PCM_STREAM_CAPTURE);
		i2s_disable_irqs(i2s_reg, CVI_PCM_STREAM_CAPTURE);
		i2s_rxctrl(i2s_reg, I2S_RX_OFF);
	}

	i2s_switch(I2S_OFF, i2s_reg);
	return 0;
}

void i2s_subsys_io_init(struct i2s_config_info *pi2s_info)
{
	struct i2s_sys_regs *i2s_sys_reg =
		(struct i2s_sys_regs *)pi2s_info->sys_base_address;
	static bool initonce;

	if (initonce)
		return;
	initonce = true;

	if (pi2s_info->id == I2S1 || pi2s_info->id == I2S2) {
		audio_dbg("%s #ifdef !CV183X I2S1/I2S2\n", __func__);
		i2s_sys_reg->i2s_tdm_sclk_in_sel =
			0x7554; /* BCLK from I2S1 as master, I2S2 BCLK form I2S1 */
		i2s_sys_reg->i2s_tdm_fs_in_sel =
			0x7554; /* LRCK from I2S1 as master, I2S2 LRCK from I2S1 */
		i2s_sys_reg->i2s_tdm_sdi_in_sel =
			0x7654; /*select i2s_tdm1 sdi to IO SDI_i2s1 */
		i2s_sys_reg->i2s_tdm_sdo_out_sel =
			0x7654; /* select i2s_tdm1 sdo to IO SDO_i2s1 */
		i2s_sys_reg->i2s_tdm_multi_sync = 0x00; /*0x600 I2S2 sync with I2S1 */

	} else if (pi2s_info->id == I2S3) {
		audio_dbg("%s #ifdef !CV183X I2S3\n", __func__);
		i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_fs_in_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_multi_sync = 0x00000000;

		i2s_sys_reg->i2s_bclk_oen_sel = 0x00000000;
		i2s_sys_reg->i2s_bclk_out_ctrl = 0x00000000;
		i2s_sys_reg->audio_pdm_ctrl = 0x00000000;
		i2s_sys_reg->audio_phy_bypass1 = 0x00000000;
		i2s_sys_reg->audio_phy_bypass2 = 0x00000000;
		i2s_sys_reg->i2s_sys_clk_ctrl = 0x00000000;
		i2s_sys_reg->i2s0_master_clk_ctrl0 = 0x00000040;
		i2s_sys_reg->i2s0_master_clk_ctrl1 = 0x00020002;
		i2s_sys_reg->i2s1_master_clk_ctrl0 = 0x00000040;
		i2s_sys_reg->i2s1_master_clk_ctrl1 = 0x00020002;
		i2s_sys_reg->i2s2_master_clk_ctrl0 = 0x00000040;
		i2s_sys_reg->i2s2_master_clk_ctrl1 = 0x00020002;
		i2s_sys_reg->i2s3_master_clk_ctrl0 = 0x00000040;
		i2s_sys_reg->i2s3_master_clk_ctrl1 = 0x00020002;
		i2s_sys_reg->i2s_sys_lrck_ctrl = 0x00000000;

	} else if (pi2s_info->id == I2S0) {
		i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_fs_in_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x00007654;
		i2s_sys_reg->i2s_tdm_multi_sync = 0x00000000;
	}
}

int i2s_init(struct i2s_config_info *pi2s_info)
{
	int ret;
	struct i2s_tdm_regs *i2s_reg = (struct i2s_tdm_regs *)pi2s_info->base_address;
	//   struct i2s_sys_regs *i2s_sys_reg =
	//       (struct i2s_sys_regs *)pi2s_info->sys_base_address;

	audio_dbg("%s, tdm_base_reg_addr=%p, sys_base_reg_addr=%p\n", __func__,
			pi2s_info->base_address, pi2s_info->sys_base_address);
	i2s_prepare_clk();
	i2s_subsys_io_init(pi2s_info);
	i2s_set_clk_source(i2s_reg, pi2s_info->clk_src);
	i2s_disable_all_interrupt(i2s_reg);

	if (pi2s_info->id == I2S3)
		i2s_set_sample_rate(i2s_reg, pi2s_info->samplingrate, pi2s_info->slot_no,
							"dac"); /* sample rate must first prior to fmt */
	if (pi2s_info->id == I2S0)
		i2s_set_sample_rate(i2s_reg, pi2s_info->samplingrate, pi2s_info->slot_no,
							"adc"); /* sample rate must first prior to fmt */

	ret = i2s_set_fmt(i2s_reg, pi2s_info->role, pi2s_info->aud_mode,
					pi2s_info->inv, pi2s_info->slot_no);
	if (ret != 0) {
		rt_kprintf("%s:set format failed\n", __func__);
		return -1;
	}

	i2s_set_resolution(i2s_reg, pi2s_info->bitspersample, pi2s_info->slot_no);

	i2s_set_ws_clock_cycle(i2s_reg, pi2s_info->sync_div, pi2s_info->aud_mode);
	#ifdef USE_DMA_MODE
	i2s_config_dma(i2s_reg, true, pi2s_info->fifo_threshold,
					pi2s_info->fifo_high_threshold);
	#else
	i2s_config_dma(i2s_reg, false, pi2s_info->fifo_threshold,
					pi2s_info->fifo_high_threshold);
	#endif

	// i2s_dump_reg(pi2s_info->base_address);
	return ret;
}

static void audio_devmem(int argc, char **argv)
{
	if (argc < 2) {
		rt_kprintf("Usage: devmem <address> [value]\n");
		rt_kprintf("Example:\n");
		rt_kprintf("  Read:  devmem 0x4130000\n");
		rt_kprintf("  Write: devmem 0x4130000 0x1234\n");
		return;
	}

	uintptr_t reg = (uintptr_t)strtoul(argv[1], NULL, 16);
	uint32_t reg_val;

	if (argc == 2) { // Read
		reg_val = readl(reg);
		audio_dbg("0x%lx read val: 0x%x\n", reg, reg_val);
	} else if (argc == 3) { // Write
		reg_val = (uint32_t)strtoul(argv[2], NULL, 16);
		mmio_write_32(reg, reg_val);
		audio_dbg("0x%lx write val: 0x%x\n", reg, reg_val);
	} else {
		audio_dbg("Invalid arguments!\n");
	}
}
MSH_CMD_EXPORT(audio_devmem, i2s reg);
