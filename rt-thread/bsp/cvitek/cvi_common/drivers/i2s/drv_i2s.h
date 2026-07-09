/*
 * Copyright (c) 2025-2025 CVITEK
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __DRV_I2S_H__
#define __DRV_I2S_H__

#include "drivers/dev_audio.h"
#include "drv_dw_i2s.h"
#include "hal_dma.h"
#include <rtdef.h>
#include <rtthread.h>

#define I2S_PERIOD_SIZE (1024)
#define I2S_PERIOD_COUNT (4)
#define I2S_BUFFER_SIZE (I2S_PERIOD_SIZE * I2S_PERIOD_COUNT)
#define CVI_MAX_I2S_DEV_NUM (4)

#define AUDIO_THREAD_PRIORITY (0)
#define AUDIO_THREAD_STACK_SIZE (2048)
#define AUDIO_THREAD_TIMESLICE (10)

#define I2S_EVT_WRITE (1 << 0)
#define I2S_EVT_READ (1 << 1)
#define I2S_EVT_XRUN                                                           \
	(1 << 2) /** Stopped: underrun (playback) or overrun (capture) detected */

#define DAC_L(vol) ((int32_t)((vol >> 16) & 0xFFFF) % 33)
#define DAC_R(vol) ((int32_t)(vol & 0xFFFF) % 33)

#define ADC_DAC_VOL(vol_l, vol_r) (((vol_l & 0xFFFF) << 16) | vol_r & 0xFFFF)

#define ADC_L(vol) ((int32_t)((vol >> 16) & 0xFFFF) % 25)
#define ADC_R(vol) ((int32_t)(vol & 0xFFFF) % 25)

typedef enum {
	cvi_i2s_state_stop,
	cvi_i2s_state_start,
	cvi_i2s_state_read,
	cvi_i2s_state_write,
} cvi_i2s_state_t;

struct cvi_i2s {
	struct rt_audio_device audio;
	struct rt_audio_configure audio_config;
	struct i2s_config_info i2s_config;

	rt_thread_t work_tid;
	struct rt_semaphore event_sem;
	rt_uint32_t isr_event_flags;

	char *dev_name;
	rt_uint8_t i2s_id;
	rt_uint8_t i2s_ch_slot_mask;
	rt_uint8_t i2s_direction;
	rt_uint8_t period_size;

	hal_dma_ch_t tx_dma;
	hal_dma_ch_t rx_dma;
	rt_uint8_t *tx_buff;
	rt_uint8_t *rx_buff;
	hal_dma_ch_config_t rx_dma_conf;
	hal_dma_ch_config_t tx_dma_conf;
	cvi_i2s_state_t i2s_state;
};

#endif /*__DRV_I2S_H__ */
