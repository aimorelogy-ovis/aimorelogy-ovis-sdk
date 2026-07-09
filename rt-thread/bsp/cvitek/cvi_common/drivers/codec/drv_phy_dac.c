/* SPDX-License-Identifier: GPL-2.0-or-later
 * CV181x DAC driver on CVITEK CV181x
 *
 * Copyright 2022 CVITEK
 *
 * Author: rachel.jiang
 *
 */

#include "drv_phy_adc_dac.h"

static struct cvi_dac g_dac;
static struct cvi_dac *dac;

static int cvi_dac_hw_params(struct cvi_dac *dac, uint32_t chan_nr,
							 uint32_t rate)
{

	uint32_t ctrl1 = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL1) &
					~AUDIO_PHY_REG_TXDAC_CIC_OPT_MASK;
	uint32_t tick = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE0) &
					~AUDIO_PHY_REG_TXDAC_INIT_DLY_CNT_MASK;
	uint32_t ana2 = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);

	// avoid pop when just set/clear mute/unmute bit
	ana2 |= 0x0F0F;
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, ana2);

	switch (chan_nr) {
	case 1:
		ana2 &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF; /* turn R-channel on */
		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, ana2);
	break;
	default:
		ana2 &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF; /* turn R-channel on */
		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, ana2);
	break;
	}

	if (rate >= 8000 && rate <= 48000) {
		LOG_D("%s, set rate to %d\n", __func__, rate);

		switch (rate) {
		case 8000:
			ctrl1 |= TXDAC_CIC_DS_512;
			tick |= 0x21;
			break;
		case 11025:
			ctrl1 |= TXDAC_CIC_DS_256;
			tick |= 0x17;
			break;
		case 16000:
			ctrl1 |= TXDAC_CIC_DS_256;
			tick |= 0x21;
			break;
		case 22050:
			ctrl1 |= TXDAC_CIC_DS_128;
			tick |= 0x17;
			break;
		case 32000:
			ctrl1 |= TXDAC_CIC_DS_128;
			tick |= 0x21;
			break;
		case 44100:
			ctrl1 &= TXDAC_CIC_DS_64;
			tick |= 0x17;
			break;
		case 48000:
			ctrl1 &= TXDAC_CIC_DS_64;
			tick |= 0x19;
			break;
		default:
			ctrl1 |= TXDAC_CIC_DS_256;
			tick |= 0x21;
			LOG_D(dac->dev, "%s, set sample rate with default 16KHz\n", __func__);
			break;
		}
	} else {
		LOG_D("%s, unsupported sample rate\n", __func__);
		return 0;
	}
	LOG_D("%s, ctrl1=0x%x\n", __func__, ctrl1);
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL1, ctrl1);
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE0, tick);

	return 0;
}

static void cvi_dac_on(struct cvi_dac *dac)
{

	uint32_t val = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);

	LOG_D("%s, before ctrl0_reg val=0x%08x\n", __func__, val);

	if ((val & AUDIO_PHY_REG_TXDAC_EN_ON) | (val & AUDIO_PHY_REG_I2S_RX_EN_ON))
		LOG_D("DAC already switched ON!!, val=0x%08x\n", val);

	val |= AUDIO_PHY_REG_TXDAC_EN_ON | AUDIO_PHY_REG_I2S_RX_EN_ON;
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, val);

	LOG_D("%s, after ctrl0_reg val=0x%08x\n", __func__,
		dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));
}

static void cvi_dac_off(struct cvi_dac *dac)
{
	uint32_t val = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);

	LOG_D("%s, before ctrl_reg val=0x%08x\n", __func__,
		dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));

	val &= AUDIO_PHY_REG_TXDAC_EN_OFF & AUDIO_PHY_REG_I2S_RX_EN_OFF;
	dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, val);

	LOG_D("%s, after ctrl_reg val=0x%08x\n", __func__,
		dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));
}

static void cvi_dac_shutdown(void)
{
	_dac_reset();
	cvi_dac_off(dac);
}

static void dump_dac_reg(struct cvi_dac *dac)
{
	rt_kprintf("AUDIO_PHY_TXDAC_CTRL0 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0));
	rt_kprintf("AUDIO_PHY_TXDAC_CTRL1 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL1));
	rt_kprintf("AUDIO_PHY_TXDAC_AFE0 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE0));
	rt_kprintf("AUDIO_PHY_TXDAC_AFE1 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1));
	rt_kprintf("AUDIO_PHY_TXDAC_ANA0 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA0));
	rt_kprintf("AUDIO_PHY_TXDAC_ANA1 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA1));
	rt_kprintf("AUDIO_PHY_TXDAC_ANA2 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2));

	rt_kprintf("status :\n");
	rt_kprintf("AUDIO_PHY_TXDAC_ANA3 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA3));
	rt_kprintf("AUDIO_PHY_RXADC_STATUS = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_RXADC_STATUS));
	rt_kprintf("AUDIO_PHY_RXADC_ANA1 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_RXADC_ANA1));
	rt_kprintf("AUDIO_PHY_RXADC_ANA4 = 0x%x\n",
				dac_read_reg(dac->dac_base, AUDIO_PHY_RXADC_ANA4));
}

static void _dac_set_gain(uint32_t vol, uint64_t vol_mask)
{
	uint32_t temp;

	if ((vol < 0) | vol > 32) {
		LOG_E("Only support range 0 [0dB] ~ 32 [48dB]\n");
		vol = 32;
	}

	if (vol_mask == AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK) { // set vol L
		if (vol == 0) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp |= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp); // mute

		} else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			if (temp & AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_MASK) { // unmute
				temp &= AUDIO_PHY_REG_DA_DEML_TXDAC_OW_EN_OFF;
				dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
			}
		}

		temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1) &
				~AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK;
		temp |= DAC_VOL_L(vol);
		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1, temp);

		LOG_D("[%s][DAC_l] vol:%d, tx_dac_ana2:0x%x, tx_dac_afe1:%x\n", __func__,
				vol, dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2),
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1));
	} else {
		if (vol == 0) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			temp |= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp); // mute

		} else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2);
			if (temp & AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_ON) { // unmute
				temp &= AUDIO_PHY_REG_DA_DEMR_TXDAC_OW_EN_OFF;
				dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2, temp);
			}
		}

		temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1) &
				~AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK;
		temp |= DAC_VOL_R(vol);
		dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1, temp);

		LOG_D("[%s][DAC_r] vol:%d, tx_dac_ana2:0x%x, tx_dac_afe1:%x\n", __func__,
				vol, dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_ANA2),
				dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1));
	}
}

static void _dac_get_gain(uint32_t *pvol, uint64_t vol_mask)
{
	uint32_t temp;

	if (vol_mask == AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK) { // l
		temp = ((dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1) &
					AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK) +
				1) /
				DAC_VOL_STEP;
	} else {
		temp = (((dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_AFE1) &
					AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK) >>
					16) +
				1) /
				DAC_VOL_STEP;
	}
	*pvol = temp;
}

int cvi_dac_ioctl(uint32_t cmd, struct cvi_vol_ctrl *pvol)
{
	uint32_t temp;

	switch (cmd) {
	case ACODEC_SOFT_RESET_CTRL:
		_dac_reset();
	break;

	case ACODEC_SET_OUTPUT_VOL:
		LOG_D("dac: ACODEC_SET_OUTPUT_VOL with val=%d\n", val);
		_dac_set_gain(pvol->vol_l, AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK);
		_dac_set_gain(pvol->vol_r, AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK);

	break;
	case ACODEC_GET_OUTPUT_VOL:
		LOG_D("dac: ACODEC_GET_OUTPUT_VOL\n");
		_dac_get_gain(&pvol->vol_l, AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK);
		_dac_get_gain(&pvol->vol_r, AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK);
	break;
	case ACODEC_SET_I2S1_FS:
		LOG_D("dac: ACODEC_SET_I2S1_FS is not support\n");
	break;
	case ACODEC_SET_DACL_VOL:

		LOG_D("dac: ACODEC_SET_DACL_VOL\n");
		_dac_set_gain(pvol->vol_l, AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK);
	break;
	case ACODEC_SET_DACR_VOL:

		LOG_D("dac: ACODEC_SET_DACR_VOL\n");
		_dac_set_gain(pvol->vol_r, AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK);
	break;
	case ACODEC_GET_DACL_VOL:

		LOG_D("dac: ACODEC_GET_DACL_VOL\n");
		_dac_get_gain(&pvol->vol_l, AUDIO_PHY_REG_TXDAC_GAIN_UB_0_MASK);
	break;
	case ACODEC_GET_DACR_VOL:

		_dac_get_gain(&pvol->vol_r, AUDIO_PHY_REG_TXDAC_GAIN_UB_1_MASK);
	break;
	case ACODEC_SET_PD_DACL:
		LOG_D("dac: ACODEC_SET_PD_DACL, val=%d\n", val);
		if (pvol->vol_l == 0) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_ON | AUDIO_PHY_REG_I2S_RX_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		} else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_OFF & AUDIO_PHY_REG_I2S_RX_EN_OFF;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		}
	break;
	case ACODEC_SET_PD_DACR:
		LOG_D("dac: ACODEC_SET_PD_DACR, val=%d\n", val);
		if (pvol->vol_r == 0) {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_ON | AUDIO_PHY_REG_I2S_RX_EN_ON;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		} else {
			temp = dac_read_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0);
			temp &= AUDIO_PHY_REG_TXDAC_EN_OFF & AUDIO_PHY_REG_I2S_RX_EN_OFF;
			dac_write_reg(dac->dac_base, AUDIO_PHY_TXDAC_CTRL0, temp);
		}
	break;
	case ACODEC_SET_DAC_DE_EMPHASIS:
		LOG_D("dac: ACODEC_SET_DAC_DE_EMPHASIS is not support\n");
	break;
	default:
		LOG_D("%s, received unsupported cmd=%u\n", __func__, cmd);
	break;
	}
	dump_dac_reg(dac);
	return 0;
}

int cvi_dac_init(uint32_t rate, uint32_t chan_nr)
{
	LOG_D("%s start rate = %d, chan_nr = %d\n", __func__, rate, chan_nr);
	dac = &g_dac;
	dac->dac_base = (uint32_t *)(0x0300A000);
	cvi_dac_shutdown();
	cvi_dac_hw_params(dac, chan_nr, rate);
	cvi_dac_on(dac);
	// dump_dac_reg(dac);

	return 0;
}
