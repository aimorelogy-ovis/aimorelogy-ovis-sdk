/*
 * Copyright (c) 2025-2025 CVITEK
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "cache.h"
#include "drv_i2s.h"
#include "drv_phy_adc_dac.h"
#include "drv_pinmux.h"
#include <rtdbg.h>
#define DBG_TAG "cvi_i2s"
#define DBG_LVL DBG_LOG
#define GET_PIN(port, no) (((((port)&0xFu) << 8) | ((no)&0xFFu)))

__aligned(64) uint8_t i2s0_rx_buff[I2S_BUFFER_SIZE];
__aligned(64) uint8_t i2s3_tx_buff[I2S_BUFFER_SIZE];

static struct cvi_i2s cvi_i2s_set[CVI_MAX_I2S_DEV_NUM] = {
	{
		.dev_name = "i2s0",
		.i2s_id = I2S0,
		.i2s_ch_slot_mask = I2S_LEFT_CHANNEL,
		.i2s_direction = RX_MODE,
		.rx_buff = i2s0_rx_buff,
	},
	{
		.dev_name = "i2s1",
		.i2s_id = I2S1,
		.i2s_ch_slot_mask = I2S_LEFT_RIGHT_CHANNEL,
		.i2s_direction = RX_MODE,
	},
	{
		.dev_name = "i2s2",
		.i2s_id = I2S2,
		.i2s_ch_slot_mask = I2S_LEFT_RIGHT_CHANNEL,
		.i2s_direction = TX_MODE,
	},
	{
		.dev_name = "i2s3",
		.i2s_id = I2S3,
		.i2s_ch_slot_mask = I2S_RIGHT_CHANNEL,
		.i2s_direction = TX_MODE,
		.tx_buff = i2s3_tx_buff,
	},
};

void gpio_control(void)
{
	rt_base_t pin = GET_PIN(0, 15); // GPIOA15 SPK_EN

	rt_pin_mode(pin, PIN_MODE_OUTPUT);
	rt_pin_write(pin, PIN_HIGH);
	audio_dbg("GPIOA_15 Level: %s\n", rt_pin_read(pin) ? "HIGH" : "LOW");
}

static void i2s_tx_debug(void)
{
	for (int i = 0; i < 5; i++) {
		audio_dbg(
			"[i2s_debug] d0:%x tx_s:0x%x, fifo_s:0x%x, req_cnt:0x%x, ack_cnt:0x%x\n",
			*(uint32_t *)(0x41300d0), *(uint32_t *)(0x4130048),
			*(uint32_t *)(0x413004C), *(uint32_t *)(0x4130050),
			*(uint32_t *)(0x4130054));
		usleep(1000 * 50);
	}
}

static void i2s_rx_debug(void)
{
	for (int i = 0; i < 5; i++) {
		audio_dbg("[i2s_debug] d0:%x\n", *(uint32_t *)(0x41000d0));
		usleep(1000 * 50);
	}
	for (int i = 0; i < 3; i++) {
		audio_dbg(
			"[i2s_debug] rx_s:0x%x, fifo_s:0x%x, req_cnt:0x%x, ack_cnt:0x%x\n",
			*(uint32_t *)(0x4100040), *(uint32_t *)(0x410004C),
			*(uint32_t *)(0x4100050), *(uint32_t *)(0x4100054));
		usleep(1000 * 50);
	}
}
void _set_default_dma_config(struct cvi_i2s *cvi_audio)
{
	if (cvi_audio->i2s_direction == TX_MODE) {
		cvi_audio->tx_dma_conf.src_inc = DMA_ADDR_INC;
		cvi_audio->tx_dma_conf.dst_inc = DMA_ADDR_CONSTANT;
		cvi_audio->tx_dma_conf.src_tw = DMA_DATA_WIDTH_16_BITS;
		cvi_audio->tx_dma_conf.dst_tw = DMA_DATA_WIDTH_32_BITS;
		cvi_audio->tx_dma_conf.trans_dir = DMA_MEM2DEV;
		cvi_audio->tx_dma_conf.group_len =
			(INIT_FIFO_THRESHOLD + 1) * 4; // 32U,//16U,
		cvi_audio->tx_dma_conf.handshake = 2;
	}

	if (cvi_audio->i2s_direction == RX_MODE) {
		cvi_audio->rx_dma_conf.src_inc = DMA_ADDR_CONSTANT;
		cvi_audio->rx_dma_conf.dst_inc = DMA_ADDR_INC;
		cvi_audio->rx_dma_conf.src_tw = DMA_DATA_WIDTH_32_BITS;
		cvi_audio->rx_dma_conf.dst_tw = DMA_DATA_WIDTH_16_BITS;
		cvi_audio->rx_dma_conf.trans_dir = DMA_DEV2MEM;
		cvi_audio->rx_dma_conf.group_len = (INIT_FIFO_THRESHOLD + 1) * 4;
		cvi_audio->rx_dma_conf.handshake = 3;
	}
}

int _set_default_i2s_config(struct i2s_config_info *pi2s_config,
							rt_uint32_t i2s_id, rt_uint8_t i2s_direction)
{
	pi2s_config->clk_src = AUD_CLK_FROM_PLL;
	pi2s_config->id = i2s_id;
	pi2s_config->base_address = i2s_get_base(i2s_id);
	pi2s_config->sys_base_address = i2s_get_sys_base();

	// adc only support 2ch.
	pi2s_config->channels = 2;
	pi2s_config->slot_no = 2;
	pi2s_config->inv = FMT_IB_NF; // FMT_IB_NF;
	pi2s_config->aud_mode = I2S_MODE;
	pi2s_config->mclk_div = 1; /* 1 means bypass*/

	pi2s_config->samplingrate = 8000;
	pi2s_config->bitspersample = 16;         /* bit resolution */
	pi2s_config->sync_div = WSS_16_CLKCYCLE;
	pi2s_config->bclk_div = 0x01;            /* 1 means bypass*/
	pi2s_config->fifo_threshold = INIT_FIFO_THRESHOLD;
	pi2s_config->fifo_high_threshold = INIT_FIFO_HIGH_THRESHOLD;
	pi2s_config->direction = i2s_direction;

	if (i2s_direction == RX_MODE) {
		pi2s_config->mclk_out_en = 1;
		pi2s_config->role = SLAVE_MODE;
	} else {
		pi2s_config->mclk_out_en = 0;
		pi2s_config->role = MASTER_MODE;
	}

	return 0;
}

static rt_err_t cvi_i2s_getcaps(struct rt_audio_device *audio,
								struct rt_audio_caps *caps)
{
	audio_dbg("%s\n", __func__);
	rt_err_t result = RT_EOK;

	RT_ASSERT(audio != RT_NULL);
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)audio->parent.user_data;
	struct cvi_vol_ctrl vol;

	switch (caps->main_type) {
	case AUDIO_TYPE_INPUT: {
		switch (caps->sub_type) {
		case AUDIO_DSP_PARAM: {
			caps->udata.config.channels = cvi_audio->audio_config.channels;
			caps->udata.config.samplebits = cvi_audio->audio_config.samplebits;
			caps->udata.config.samplerate = cvi_audio->audio_config.samplerate;
			break;
		}
		// case AUDIO_PARM_I2S_CH_SLOT_MASK:
		// {
		//     caps->udata.value               = cvi_audio->i2s_ch_slot_mask;
		//     break;
		// }
		case AUDIO_TYPE_MIXER:
			switch (caps->sub_type) {
			case AUDIO_MIXER_MIC:

				cvi_adc_ioctl(ACODEC_GET_INPUT_VOL, &vol);
				caps->udata.value = ADC_DAC_VOL(vol.vol_l, vol.vol_r);
				audio_dbg("%s [adc] L:%d R:%d vol:0x%x\n", __func__, vol.vol_l,
							vol.vol_r, caps->udata.value);
			return RT_EOK;
			default:
				return -RT_ERROR;
			}
		break;
		default: {
			result = -RT_ERROR;
			break;
		}
		}
	break;
	}
	case AUDIO_TYPE_OUTPUT: {
		switch (caps->sub_type) {
		case AUDIO_DSP_PARAM: {
			caps->udata.config.samplerate = cvi_audio->audio_config.samplerate;
			caps->udata.config.channels = cvi_audio->audio_config.channels;
			caps->udata.config.samplebits = cvi_audio->audio_config.samplebits;
			break;
		}

		// case AUDIO_PARM_I2S_CH_SLOT_MASK:
		// {
		//     caps->udata.value               = cvi_audio->i2s_ch_slot_mask;
		//     break;
		// }
		case AUDIO_TYPE_MIXER:
			switch (caps->sub_type) {
			case AUDIO_MIXER_VOLUME:

				cvi_dac_ioctl(ACODEC_GET_OUTPUT_VOL, &vol);
				caps->udata.value = ADC_DAC_VOL(vol.vol_l, vol.vol_r);
				audio_dbg("%s [dac] L:%d, R:%d vol:0x%x\n", __func__, vol.vol_l,
							vol.vol_r, caps->udata.value);

			return RT_EOK;
			default:
				return -RT_ERROR;
			}
			break;
		default: {
			result = -RT_ERROR;
			break;
		}
		}

	break;
	}

	default:
		result = -RT_ERROR;
	break;
	}

	return result;
}

static rt_err_t cvi_i2s_configure(struct rt_audio_device *audio,
								  struct rt_audio_caps *caps)
{
	audio_dbg("%s\n", __func__);
	rt_err_t result = RT_EOK;

	RT_ASSERT(audio != RT_NULL);
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)audio->parent.user_data;
	struct cvi_vol_ctrl vol;

	switch (caps->main_type) {
	case AUDIO_TYPE_OUTPUT: {
		switch (caps->sub_type) {
		case AUDIO_DSP_PARAM: {
			cvi_audio->audio_config.samplerate = caps->udata.config.samplerate;
			cvi_audio->audio_config.samplebits = caps->udata.config.samplebits;
			cvi_audio->audio_config.channels = caps->udata.config.channels;
			break;
		}
		// case AUDIO_PARM_I2S_PERIOD_SIZE:
		// {
		//     //cvi_audio->transfer.data_line      = caps->udata.value;
		//     break;
		// }
		// case AUDIO_PARM_I2S_CH_SLOT_MASK:
		// {
		//     cvi_audio->i2s_ch_slot_mask      = caps->udata.value;
		//     break;
		// }
		case AUDIO_TYPE_MIXER:
			switch (caps->sub_type) {
			case AUDIO_MIXER_VOLUME:

				vol.vol_l = DAC_L(caps->udata.value);
				vol.vol_r = DAC_R(caps->udata.value);
				audio_dbg("%s [dac] L:%d R:%d vol:0x%x\n", __func__, vol.vol_l,
							vol.vol_r, caps->udata.value);
				cvi_dac_ioctl(ACODEC_SET_OUTPUT_VOL, &vol);
				return RT_EOK;
			default:
				return -RT_ERROR;
			}
			break;
		default:
			result = -RT_ERROR;
			break;
		}
	break;
	}
	case AUDIO_TYPE_INPUT: {
	switch (caps->sub_type) {

	case AUDIO_DSP_PARAM: {
		cvi_audio->audio_config.samplerate = caps->udata.config.samplerate;
		cvi_audio->audio_config.channels = caps->udata.config.channels;
		cvi_audio->audio_config.samplebits = caps->udata.config.samplebits;
		break;
	}

		// case AUDIO_PARM_I2S_DATA_LINE:
		// {
		//     //cvi_audio->transfer.data_line      = caps->udata.value;
		//     break;
		// }
		// case AUDIO_PARM_I2S_CH_SLOT_MASK:
		// {
		//     cvi_audio->i2s_ch_slot_mask      = caps->udata.value;
		//     break;
		// }
	default:
			result = -RT_ERROR;
			break;
	}
	break;
	}
	case AUDIO_TYPE_MIXER:
		switch (caps->sub_type) {
		case AUDIO_MIXER_MIC:

			vol.vol_l = ADC_L(caps->udata.value);
			vol.vol_r = ADC_R(caps->udata.value);
			audio_dbg("%s [adc] L:%d R:%d vol:0x%x\n", __func__, vol.vol_l, vol.vol_r,
					caps->udata.value);
			cvi_adc_ioctl(ACODEC_SET_INPUT_VOL, &vol);
			return RT_EOK;
		default:
			return -RT_ERROR;
		}
	break;
	default:
	break;
	}

	cvi_audio->i2s_config.samplingrate = cvi_audio->audio_config.samplerate;
	cvi_audio->i2s_config.channels = cvi_audio->audio_config.channels;
	cvi_audio->i2s_config.slot_no = cvi_audio->audio_config.channels;
	cvi_audio->i2s_config.bitspersample = cvi_audio->audio_config.samplebits;
	cvi_audio->i2s_config.sync_div =
		I2S_WIDTH(cvi_audio->i2s_config.bitspersample);

	i2s_init(&cvi_audio->i2s_config);
	if (cvi_audio->i2s_id == I2S0 || cvi_audio->i2s_id == I2S3) {
		i2s_set_clk_source(cvi_audio->i2s_config.base_address,
							cvi_audio->i2s_config.clk_src);
		i2s_set_clk_sample_rate(cvi_audio->i2s_config.base_address,
								cvi_audio->audio_config.samplerate);
	}

	/* enable adc /dac */
	if (cvi_audio->i2s_id == I2S3) {
		vol.vol_r = 20;
		vol.vol_l = 20;
		cvi_dac_init(cvi_audio->i2s_config.samplingrate, 2);
		cvi_dac_ioctl(ACODEC_SET_OUTPUT_VOL, &vol);
		cvi_audio->i2s_ch_slot_mask =
			(cvi_audio->i2s_config.channels == 1 ? I2S_RIGHT_CHANNEL
									: I2S_LEFT_RIGHT_CHANNEL);
	} else if (cvi_audio->i2s_id == I2S0) {
		vol.vol_r = 10;
		vol.vol_l = 10;
		cvi_adc_init(cvi_audio->i2s_config.samplingrate);
		cvi_adc_ioctl(ACODEC_SET_ADCL_VOL, &vol);
		cvi_audio->i2s_ch_slot_mask =
			(cvi_audio->i2s_config.channels == 1 ? I2S_LEFT_CHANNEL
									: I2S_LEFT_RIGHT_CHANNEL);
	}

	/* configure I2S transfer */
	i2s_select_sound_channel(cvi_audio->i2s_config.base_address,
							cvi_audio->i2s_ch_slot_mask,
							cvi_audio->i2s_direction); // rx or tx

	/* i2s dma only support sample bit: 16 and 32 bits */

	/* Stop I2S transfer if the I2S needs to be re-configured */

	/* Restore I2S to previous state */
	return result;
}

static rt_err_t cvi_i2s_init(struct rt_audio_device *audio)
{
	audio_dbg("%s\n", __func__);
	RT_ASSERT(audio != RT_NULL);
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)audio->parent.user_data;

	_set_default_i2s_config(&cvi_audio->i2s_config, cvi_audio->i2s_id,
							cvi_audio->i2s_direction);
	_set_default_dma_config(cvi_audio);

	if (cvi_audio->i2s_id == I2S3)
		gpio_control();

	return RT_EOK;
}

static void cvi_i2s_buffer_info(struct rt_audio_device *audio,
								struct rt_audio_buf_info *info)
{
	audio_dbg("%s\n", __func__);
	RT_ASSERT(audio != RT_NULL);
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)audio->parent.user_data;
	/**
	 *               AUD_FIFO
	 * +----------------+----------------+
	 * |     block1     |     block2     |
	 * +----------------+----------------+
	 *  \  block_size  /
	 */
	info->buffer = cvi_audio->tx_buff;
	info->total_size = I2S_BUFFER_SIZE;
	info->block_size = I2S_PERIOD_SIZE;
	info->block_count = I2S_PERIOD_COUNT;
}
static rt_ssize_t cvi_i2s_transmit(struct rt_audio_device *audio,
								   const void *writeBuf, void *readBuf,
								   rt_size_t size)
{
	RT_ASSERT(audio != RT_NULL);
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)audio->parent.user_data;
	struct i2s_config_info *pi2s_conf = &cvi_audio->i2s_config;
	// rt_uint8_t *write = (rt_uint8_t *)writeBuf;

	if (writeBuf != RT_NULL) {
		// audio_dbg("%s size:%d write:%p [0x%x, 0x%x, 0x%x, 0x%x]\n", __func__,
		// size, writeBuf, write[0], write[1],write[2], write[3]);

		hal_dma_ch_start(&cvi_audio->tx_dma, (void *)writeBuf,
							(void *)&(pi2s_conf->base_address->tx_wr_port_ch0),
							I2S_PERIOD_SIZE);
		cvi_audio->i2s_state = cvi_i2s_state_write;
	} else if (readBuf) {
		hal_dma_ch_start(&cvi_audio->rx_dma,
							(void *)&(pi2s_conf->base_address->rx_rd_port_ch0),
							readBuf, I2S_PERIOD_SIZE);
		cvi_audio->i2s_state = cvi_i2s_state_read;
	}
}
void i2s_dma_event_cb(hal_dma_ch_t *dma, dma_event_t event, void *arg)
{
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)arg;

	if (event == DMA_EVENT_TRANSFER_DONE) {
		if (cvi_audio->tx_dma.ch_id == dma->ch_id) { // tx
			cvi_audio->isr_event_flags |= I2S_EVT_WRITE;

		} else { // rx
			cvi_audio->isr_event_flags |= I2S_EVT_READ;
		}
	} else if (event == DMA_EVENT_TRANSFER_ERROR) {
	}
	rt_sem_release(&cvi_audio->event_sem);
}

static void i2s_worker_thread_entry(void *parameter)
{
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)parameter;

	while (1) {

		rt_sem_take(&cvi_audio->event_sem, RT_WAITING_FOREVER);
		if (cvi_audio->isr_event_flags & I2S_EVT_WRITE) {
			cvi_audio->isr_event_flags &= ~I2S_EVT_WRITE;
			rt_audio_tx_complete(&cvi_audio->audio);
		}

		if (cvi_audio->isr_event_flags & I2S_EVT_READ) {
			cvi_audio->isr_event_flags &= ~I2S_EVT_READ;
			rt_audio_rx_done(&cvi_audio->audio, cvi_audio->rx_buff, I2S_PERIOD_SIZE);
			cvi_i2s_transmit(&cvi_audio->audio, NULL, cvi_audio->rx_buff,
							I2S_PERIOD_SIZE);
		}
	}
}

static rt_err_t cvi_i2s_start(struct rt_audio_device *audio, int stream)
{
	audio_dbg("%s\n", __func__);
	RT_ASSERT(audio != RT_NULL);
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)audio->parent.user_data;

	rt_sem_init(&cvi_audio->event_sem, "i2s_sem", 0, RT_IPC_FLAG_FIFO);
	cvi_audio->work_tid = rt_thread_create(
		"i2s_cb", i2s_worker_thread_entry, cvi_audio, AUDIO_THREAD_STACK_SIZE,
		AUDIO_THREAD_PRIORITY, AUDIO_THREAD_TIMESLICE);
	RT_ASSERT(cvi_audio->work_tid != RT_NULL);

	rt_thread_startup(cvi_audio->work_tid);
	i2s_enable(&cvi_audio->i2s_config);

	if (stream == AUDIO_STREAM_REPLAY) {

		hal_dma_channel_request(&cvi_audio->tx_dma, 2, 0);
		hal_dma_ch_config(&cvi_audio->tx_dma, &cvi_audio->tx_dma_conf);
		hal_dma_ch_attach_callback(&cvi_audio->tx_dma, i2s_dma_event_cb, cvi_audio);
		hal_dma_ch_start(
			&cvi_audio->tx_dma, cvi_audio->tx_buff,
			(void *)&(cvi_audio->i2s_config.base_address->tx_wr_port_ch0),
			I2S_PERIOD_SIZE);

	} else if (stream == AUDIO_STREAM_RECORD) {

		hal_dma_channel_request(&cvi_audio->rx_dma, 3,
								0); // drv_dma.c->remap_table[] CVI_I2S0_RX:3
		hal_dma_ch_config(&cvi_audio->rx_dma, &cvi_audio->rx_dma_conf);
		hal_dma_ch_attach_callback(&cvi_audio->rx_dma, i2s_dma_event_cb, cvi_audio);
		hal_dma_ch_start(
			&cvi_audio->rx_dma,
			(void *)&(cvi_audio->i2s_config.base_address->rx_rd_port_ch0),
			cvi_audio->rx_buff, I2S_PERIOD_SIZE);
	} else {
		return -RT_ERROR;
	}

	cvi_audio->i2s_state = cvi_i2s_state_start;

	return RT_EOK;
}

static rt_err_t cvi_i2s_stop(struct rt_audio_device *audio, int stream)
{
	audio_dbg("%s\n", __func__);
	RT_ASSERT(audio != RT_NULL);
	struct cvi_i2s *cvi_audio = (struct cvi_i2s *)audio->parent.user_data;

	if (stream == AUDIO_STREAM_REPLAY) {
		hal_dma_ch_detach_callback(&cvi_audio->tx_dma);
		hal_dma_ch_stop(&cvi_audio->tx_dma);
		hal_dma_ch_free(&cvi_audio->tx_dma);
	} else if (stream == AUDIO_STREAM_RECORD) {
		hal_dma_ch_detach_callback(&cvi_audio->rx_dma);
		hal_dma_ch_stop(&cvi_audio->rx_dma);
		hal_dma_ch_free(&cvi_audio->rx_dma);

	} else {
		return -RT_ERROR;
	}

	i2s_disable(&cvi_audio->i2s_config);
	rt_sem_detach(&cvi_audio->event_sem);
	rt_thread_delete(cvi_audio->work_tid);
	cvi_audio->i2s_state = cvi_i2s_state_stop;

	return RT_EOK;
}

static struct rt_audio_ops cvi_i2s_ops = {
	.getcaps = cvi_i2s_getcaps,
	.configure = cvi_i2s_configure,
	.init = cvi_i2s_init,
	.start = cvi_i2s_start,
	.stop = cvi_i2s_stop,
	.transmit = cvi_i2s_transmit,
	.buffer_info = cvi_i2s_buffer_info,
};

static char *pinname_whitelist_spk_en[] = {
	"SPK_EN",
	NULL,
};

static void rt_hw_i2s_pinmux_config(void)
{
	pinmux_config(BSP_I2S_SPK_EN_PINNAME, XGPIOA_15, pinname_whitelist_spk_en);
}

int rt_hw_i2s_init(void)
{
	rt_err_t ret = RT_EOK;

	rt_hw_i2s_pinmux_config();
	audio_dbg("%s\n", __func__);
	for (uint32_t i = 0; i < ARRAY_SIZE(cvi_i2s_set); i++) {
		cvi_i2s_set[i].audio.ops = &cvi_i2s_ops;

		ret = rt_audio_register(&cvi_i2s_set[i].audio, cvi_i2s_set[i].dev_name,
								RT_DEVICE_FLAG_RDWR, &cvi_i2s_set[i]);

		if (ret != RT_EOK) {
			LOG_E("rt audio %s register failed, status=%d\n", cvi_i2s_set[i].dev_name,
				ret);
		}
	}

	return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_i2s_init);
