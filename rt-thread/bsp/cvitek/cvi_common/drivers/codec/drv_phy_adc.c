/* SPDX-License-Identifier: GPL-2.0-or-later
 * SOPHGO ADC driver on  SOPHGO
 *
 * Copyright 2025
 *
 * Author: rachel.jiang
 *
 */

#include "drv_phy_adc_dac.h"

static struct cvi_adc g_adc;
static struct cvi_adc *adc;

static int adc_vol_list[25] = {
	ADC_VOL_GAIN_0,  ADC_VOL_GAIN_1,  ADC_VOL_GAIN_2,  ADC_VOL_GAIN_3,
	ADC_VOL_GAIN_4,  ADC_VOL_GAIN_5,  ADC_VOL_GAIN_6,  ADC_VOL_GAIN_7,
	ADC_VOL_GAIN_8,  ADC_VOL_GAIN_9,  ADC_VOL_GAIN_10, ADC_VOL_GAIN_11,
	ADC_VOL_GAIN_12, ADC_VOL_GAIN_13, ADC_VOL_GAIN_14, ADC_VOL_GAIN_15,
	ADC_VOL_GAIN_16, ADC_VOL_GAIN_17, ADC_VOL_GAIN_18, ADC_VOL_GAIN_19,
	ADC_VOL_GAIN_20, ADC_VOL_GAIN_21, ADC_VOL_GAIN_22, ADC_VOL_GAIN_23,
	ADC_VOL_GAIN_24};

uint32_t old_adc_voll;
uint32_t old_adc_volr;

static void writel(uint32_t val, uint32_t *addr) { *addr = val; }

static uint32_t readl(uint32_t *addr) { return *addr; }

static int cvi_adc_hw_params(struct cvi_adc *adc, uint32_t rate)
{
	LOG_D("%s start %d\n", __func__, __LINE__);
	uint32_t ctrl1 = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1) &
					~AUDIO_PHY_REG_RXADC_CIC_OPT_MASK;
	uint32_t ana3 = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA3) &
					~AUDIO_PHY_REG_CTUNE_RXADC_MASK;
	uint32_t ana0;
	uint32_t clk = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CLK) &
					~(AUDIO_RXADC_SCK_DIV_MASK | AUDIO_RXADC_DLYEN_MASK);
	// uint32_t spare0 = adc_read_reg(adc->adc_base, AUDIO_PHY_SPARE_0) &
	// ~AUDIO_ADC_SCK_DIV_MASK;
	uint32_t *dac;

/* ECO function, register naming is not corrected, use ioremap to access
 * register of DAC
 */
	dac = (uint32_t *)(0x0300A000);
	ana0 = readl(dac + AUDIO_PHY_TXDAC_ANA0) & ~AUDIO_PHY_REG_ADDI_TXDAC_MASK;

	if (rate >= 8000 && rate <= 48000) {
		LOG_D("%s, set rate to %d\n", __func__, rate);

		switch (rate) {
		case 8000:
			ctrl1 |= RXADC_CIC_DS_512;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			// spare0 |= SPARE_SCK_DIV(8);  4096 / 8 / 32 / 2 (if CIC is 128 need
			// divide 2 in addition)
			clk |= RXADC_SCK_DIV(32) | RXADC_DLYEN(0x21); /* 16384 / 8 / 32 / 2 */
			break;
		case 11025:
			ctrl1 |= RXADC_CIC_DS_256;
			ana3 |= RXADC_CTUNE_MCLK_11298;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			clk |=
				RXADC_SCK_DIV(16) | RXADC_DLYEN(0x17); /* 112896 / 11.025 / 32 / 2 */
			// spare0 |= SPARE_SCK_DIV(8);  5644 / 11.025 / 32 / 2
			break;
		case 16000:
			ctrl1 |= RXADC_CIC_DS_256;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			// spare0 |= SPARE_SCK_DIV(8);  8192 / 16 / 32 / 2
			clk |= RXADC_SCK_DIV(16) | RXADC_DLYEN(0x21); /* 16384 / 16 / 32 / 2 */
			break;
		case 22050:
			ctrl1 |= RXADC_CIC_DS_128;
			ana3 |= RXADC_CTUNE_MCLK_11298;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			// spare0 |= SPARE_SCK_DIV(8);  11298 / 22.05 / 32 / 2
			clk |= RXADC_SCK_DIV(8) | RXADC_DLYEN(0x17); /* 112896 / 22.05 / 32 / 2 */
			break;
		case 32000:
			ctrl1 &= RXADC_CIC_DS_128;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			// spare0 |= SPARE_SCK_DIV(4);  8192 / 32 / 32 / 2
			clk |= RXADC_SCK_DIV(8) | RXADC_DLYEN(0x21); /* 16384 / 32 / 32 / 2 */
			break;
		case 44100:
			ctrl1 &= RXADC_CIC_DS_64;
			ana3 |= RXADC_CTUNE_MCLK_11298;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			// spare0 |= SPARE_SCK_DIV(4);  11298 / 44.1 / 32 / 2
			clk |= RXADC_SCK_DIV(4) | RXADC_DLYEN(0x17); /* 112896 / 44.1 / 32 / 2 */
			break;
		case 48000:
			ctrl1 &= RXADC_CIC_DS_64;
			ana3 |= RXADC_CTUNE_MCLK_12288;
			ana0 &= ADDI_TXDAC_GAIN_RATIO_1;
			// spare0 |= SPARE_SCK_DIV(4);  12288 / 48 / 32 / 2
			clk |= RXADC_SCK_DIV(4) | RXADC_DLYEN(0x19); /* 16384 / 16 / 32 / 2 */
			break;
		default:
			ctrl1 |= RXADC_CIC_DS_256;
			ana3 |= RXADC_CTUNE_MCLK_16384;
			ana0 |= ADDI_TXDAC_GAIN_RATIO_1;
			clk |= RXADC_SCK_DIV(16) | RXADC_DLYEN(0x21); /* 16384 / 16 / 32 / 2 */
			// spare0 |= SPARE_SCK_DIV(8);  8192 / 16 / 32 / 2
			LOG_D("%s, unsupported sample rate. Set with default 16KHz\n", __func__);
			break;
		}

		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1, ctrl1);
		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA3, ana3);
		adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CLK, clk);

		LOG_D("adc_clk:%x\n", adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CLK));

		// adc_write_reg(adc->adc_base, AUDIO_PHY_SPARE_0, spare0);
		writel(ana0, dac + AUDIO_PHY_TXDAC_ANA0);
	} else {
		LOG_D("%s, unsupported sample rate\n", __func__);
	}

	return 0;
}

static void cvi_adc_on(struct cvi_adc *adc)
{

	uint32_t val = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);

	LOG_D("%s, before rxadc reg val=0x%08x\n", __func__,
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));

	if ((val & AUDIO_PHY_REG_RXADC_EN_ON) | (val & AUDIO_PHY_REG_I2S_TX_EN_ON))
		LOG_D("ADC or I2S TX already switched ON!!, val=0x%08x\n", val);

	val |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
	adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, val);

	LOG_D("%s, after rxadc reg val=0x%08x\n", __func__,
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));
}

static void cvi_adc_off(struct cvi_adc *adc)
{

	uint32_t val = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);

	val &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
	adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, val);

	LOG_D("%s, after rxadc reg val=0x%08x\n", __func__,
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));
}

static void cvi_adc_shutdown(struct cvi_adc *adc)
{
	_adc_reset();
	cvi_adc_off(adc);
}

static void _adc_set_gain(uint32_t vol, uint64_t vol_mask)
{
	uint32_t val2;
	uint32_t temp;

	if ((vol < 0) | vol > 24) {
		LOG_E("Only support range 0 [0dB] ~ 24 [48dB]\n");
		vol = 24;
	}

	if (vol_mask == AUDIO_PHY_REG_ADC_VOLL_MASK) { // set vol_l
		if (vol == 0) {
			/* set mute */
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) |
					AUDIO_PHY_REG_MUTEL_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
			temp |= adc_vol_list[vol];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		} else {
			temp = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) &
					AUDIO_PHY_REG_ADC_VOLL_MASK);
			for (val2 = 0; val2 < 25; val2++) {
				if (temp == adc_vol_list[val2])
					break;
			}
			if (val2 == 0) {
				/* unmute */
				temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) &
						AUDIO_PHY_REG_MUTEL_OFF;
				adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
			}
			temp |= adc_vol_list[vol];
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		}
	} else {
		if (vol == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) |
				AUDIO_PHY_REG_MUTER_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
			temp |= (adc_vol_list[vol] << 16);
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		} else {
			temp = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) &
				AUDIO_PHY_REG_ADC_VOLR_MASK) >>
				16;
			for (val2 = 0; val2 < 25; val2++) {
				if (temp == adc_vol_list[val2])
					break;
			}
			if (val2 == 0) {
				temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2) &
					AUDIO_PHY_REG_MUTER_OFF;
				adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2, temp);
			}
			temp = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) &
				AUDIO_PHY_REG_ADC_VOLR_MASK);
			temp |= (adc_vol_list[vol] << 16);
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0, temp);
		}
	}
}

static void _adc_get_gain(uint32_t *pvol, uint64_t vol_mask)
{
	uint32_t val2;
	uint32_t temp;

	if (vol_mask == AUDIO_PHY_REG_ADC_VOLL_MASK) {
		temp = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) &
				AUDIO_PHY_REG_ADC_VOLL_MASK);
		for (val2 = 0; val2 < 25; val2++) {
			if (temp == adc_vol_list[val2])
				break;
		}
	} else {
		temp = (adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0) &
				AUDIO_PHY_REG_ADC_VOLR_MASK) >>
				16;
		for (val2 = 0; val2 < 25; val2++) {
			if (temp == adc_vol_list[val2])
				break;
		}
	}
	*pvol = val2;
}

int cvi_adc_ioctl(uint32_t cmd, struct cvi_vol_ctrl *pvol)
{
	uint32_t val2;
	uint32_t temp;

	LOG_D("%s, received cmd=%u, vol[l:%d, r:%d, mute:%d]\n", __func__, cmd,
		pvol->vol_l, pvol->vol_r, pvol->vol_ctrl_mute);

	switch (cmd) {
	case ACODEC_SOFT_RESET_CTRL:
		_adc_reset();
	break;
	case ACODEC_SET_INPUT_VOL:
		LOG_D("adc: ACODEC_SET_INPUT_VOL\n");
		_adc_set_gain(pvol->vol_l, AUDIO_PHY_REG_ADC_VOLL_MASK);
		_adc_set_gain(pvol->vol_r, AUDIO_PHY_REG_ADC_VOLR_MASK);
	break;
	case ACODEC_GET_INPUT_VOL:
		_adc_get_gain(&pvol->vol_l, AUDIO_PHY_REG_ADC_VOLL_MASK);
		_adc_get_gain(&pvol->vol_r, AUDIO_PHY_REG_ADC_VOLR_MASK);
	break;
	case ACODEC_SET_I2S1_FS:
		LOG_D("adc: ACODEC_SET_I2S1_FS is not support\n");
	break;
	case ACODEC_SET_ADCL_VOL:
		_adc_set_gain(pvol->vol_l, AUDIO_PHY_REG_ADC_VOLL_MASK);
	break;

	case ACODEC_SET_ADCR_VOL:
		_adc_set_gain(pvol->vol_r, AUDIO_PHY_REG_ADC_VOLR_MASK);
	break;

	case ACODEC_GET_ADCL_VOL:
		_adc_get_gain(&pvol->vol_l, AUDIO_PHY_REG_ADC_VOLL_MASK);
	break;
	case ACODEC_GET_ADCR_VOL:
		_adc_get_gain(&pvol->vol_r, AUDIO_PHY_REG_ADC_VOLR_MASK);
	break;

	case ACODEC_SET_PD_ADCL:
		LOG_D("adc: ACODEC_SET_PD_ADCL, *pval=%d\n", *pval);
		if (pvol->vol_l == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
	break;
	case ACODEC_SET_PD_ADCR:
		LOG_D("adc: ACODEC_SET_PD_ADCR, *pval=%d\n", *pval);
		if (pvol->vol_r == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
	break;

	case ACODEC_SET_PD_LINEINL:
		LOG_D("adc: ACODEC_SET_PD_LINEINL, *pval=%d\n", *pval);
		if (pvol->vol_l == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
	break;
	case ACODEC_SET_PD_LINEINR:
		LOG_D("adc: ACODEC_SET_PD_LINEINR, *pval=%d\n", *pval);
		if (pvol->vol_r == 0) {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp |= AUDIO_PHY_REG_RXADC_EN_ON | AUDIO_PHY_REG_I2S_TX_EN_ON;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		} else {
			temp = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0);
			temp &= AUDIO_PHY_REG_RXADC_EN_OFF & AUDIO_PHY_REG_I2S_TX_EN_OFF;
			adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0, temp);
		}
	break;
	case ACODEC_SET_ADC_HP_FILTER:
		LOG_D("adc: ACODEC_SET_ADC_HP_FILTER is not support\n");
	break;
	default:
		LOG_D("%s, received unsupport cmd=%u\n", __func__, cmd);
	break;
	}

	return 0;
}

static void dump_adc_reg(struct cvi_adc *adc)
{
	LOG_D("AUDIO_PHY_RXADC_CTRL0 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL0));
	LOG_D("AUDIO_PHY_RXADC_CTRL1 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1));
	LOG_D("AUDIO_PHY_RXADC_STATUS = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_STATUS));
	LOG_D("AUDIO_PHY_RXADC_ANA0 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA0));
	LOG_D("AUDIO_PHY_RXADC_ANA1 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA1));
	LOG_D("AUDIO_PHY_RXADC_ANA2 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA2));
	LOG_D("AUDIO_PHY_RXADC_ANA3 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA3));
	LOG_D("AUDIO_PHY_RXADC_ANA4 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA4));
	LOG_D("AUDIO_PHY_RXADC_ANA5 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA5));
	LOG_D("AUDIO_PHY_RXADC_ANA6 = 0x%x\n",
		adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_ANA6));
}

int cvi_adc_init(uint32_t rate)
{
	uint32_t ctrl1;

	LOG_D("%s,%d start\n", __func__, __LINE__);
	adc = &g_adc;
	adc->adc_base = (uint32_t *)(0x0300A100);
	adc->mclk_source = (uint32_t *)(0x04130000);

	cvi_adc_shutdown(adc);
	/* set default input vol gain to maxmum 48dB, vol range is 0~24 */
	ctrl1 = adc_read_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1);
	adc_write_reg(adc->adc_base, AUDIO_PHY_RXADC_CTRL1,
		ctrl1 | AUDIO_ADC_IGR_INIT_EN);
	cvi_adc_on(adc);
	cvi_adc_hw_params(adc, rate);
	dump_adc_reg(adc);

	return 0;
}
