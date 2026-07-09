/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024/02/22     flyingcys    first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_adc.h"
#include "drv_pinmux.h"

#define DBG_LEVEL   DBG_LOG
#include <rtdbg.h>
#define LOG_TAG "DRV.ADC"

rt_inline void cvi_set_saradc_ctrl(unsigned long reg_base, rt_uint32_t value)
{
    value |= mmio_read_32(reg_base + SARADC_CTRL_OFFSET);
    mmio_write_32(reg_base + SARADC_CTRL_OFFSET, value);
}

rt_inline void cvi_reset_saradc_ctrl(unsigned long reg_base, rt_uint32_t value)
{
    value = mmio_read_32(reg_base + SARADC_CTRL_OFFSET) & ~value;
    mmio_write_32(reg_base + SARADC_CTRL_OFFSET, value);
}

rt_inline rt_uint32_t cvi_get_saradc_status(unsigned long reg_base)
{
    return((rt_uint32_t)mmio_read_32(reg_base + SARADC_STATUS_OFFSET));
}

rt_inline void cvi_set_cyc(unsigned long reg_base)
{
    rt_uint32_t value;

    value = mmio_read_32(reg_base + SARADC_CYC_SET_OFFSET);

    value &= ~SARADC_CYC_CLKDIV_DIV_16;
    mmio_write_32(reg_base + SARADC_CYC_SET_OFFSET, value);

    value |= SARADC_CYC_CLKDIV_DIV_16;                                                               //set saradc clock cycle=840ns
    mmio_write_32(reg_base + SARADC_CYC_SET_OFFSET, value);
}

rt_inline void cvi_do_calibration(unsigned long reg_base)
{
    rt_uint32_t efuse_val;
    rt_uint32_t top_trim, rtc_trim;

    // get trim value from efuse
    efuse_val = mmio_read_32(SARADC_EFUSE_TRIM_BASE);

    top_trim = (efuse_val & TOP_SARADC_TRIM_MASK) >> TOP_SARADC_TRIM_OFFSET;
	rtc_trim = (efuse_val & RTC_SARADC_TRIM_MASK) >> RTC_SARADC_TRIM_OFFSET;
    LOG_D("Setting top_trim = 0x%x, rtc_trim = 0x%x", top_trim, rtc_trim);

    //set saradc trim
    if((reg_base & TOP_ADC_PREFIX) == TOP_ADC_PREFIX)
        mmio_write_32(reg_base + SARADC_TRIM_REG, top_trim);
    else if((reg_base & RTC_ADC_PREFIX) == RTC_ADC_PREFIX)
        mmio_write_32(reg_base + SARADC_TRIM_REG, rtc_trim);
    else
        LOG_E("Wrong saradc base address: 0x%x", reg_base);

}

struct cvi_adc_dev
{
    struct rt_adc_device device;
    const char *name;
    rt_ubase_t base;
};

static struct cvi_adc_dev adc_dev_config[] =
{
#ifdef BSP_USING_ADC_ACTIVE_1
    {
        .name = "adc1",
        .base = SARADC_BASE
    },
#endif /* BSP_USING_ADC_ACTIVE_1 */
#ifdef BSP_USING_ADC_ACTIVE_2
    {
        .name = "adc2",
        .base = SARADC_BASE + SARADC_SIZE
    },
#endif /* BSP_USING_ADC_ACTIVE_2 */
#ifdef BSP_USING_ADC_ACTIVE_3
    {
        .name = "adc3",
        .base = SARADC_BASE + SARADC_SIZE * 2
    },
#endif /* BSP_USING_ADC_ACTIVE_3 */    

#ifdef BSP_USING_ADC_NODIE_1
    {
        .name = "adc4",
        .base = RTC_ADC_BASE 
    },
#endif /* BSP_USING_ADC_NODIE_1 */
#ifdef BSP_USING_ADC_NODIE_2
    {
        .name = "adc5",
        .base = RTC_ADC_BASE + SARADC_SIZE
    },
#endif /* BSP_USING_ADC_NODIE_2 */
};

static rt_err_t _adc_enabled(struct rt_adc_device *device, rt_int8_t channel, rt_bool_t enabled)
{
    struct cvi_adc_dev *adc_dev = (struct cvi_adc_dev *)device->parent.user_data;
    uint32_t value;

    RT_ASSERT(adc_dev != RT_NULL);

    if (channel > SARADC_CH_MAX) {
        LOG_E("[%s] Invalid channel: %d; Please input 1~3", adc_dev->name, channel);
        return -RT_EINVAL;
    }

    /*
    * Due to a known bug in the Mars3 ADC IP:
    * When multiple channels on a single ADC chip are enabled simultaneously,
    * their data interferes with each other, resulting in inaccurate readings.
    *
    * To work around this issue in this driver:
    * Channel configuration and enabling of the ADC are deferred and performed
    * immediately before reading the channel data.
    */
    // if (enabled)
    // {
    //     //set channel
    //     cvi_set_saradc_ctrl(adc_dev->base, (rt_uint32_t)channel << (SARADC_CTRL_SEL_POS + 1));

    //     //set saradc clock cycle
    //     cvi_set_cyc(adc_dev->base);

    //     //start
    //     cvi_set_saradc_ctrl(adc_dev->base, SARADC_CTRL_START);
    //     LOG_D("enable saradc...");
    // }
    // else
    // {
    //     cvi_reset_saradc_ctrl(adc_dev->base, (rt_uint32_t)channel << (SARADC_CTRL_SEL_POS + 1));
    //     LOG_D("disable saradc...");
    // }
    return RT_EOK;
}

static rt_err_t _adc_convert(struct rt_adc_device *device, rt_int8_t channel, rt_uint32_t *value)
{
    struct cvi_adc_dev *adc_dev = (struct cvi_adc_dev *)device->parent.user_data;
    rt_uint32_t result;
    rt_uint32_t cnt = 0;

    RT_ASSERT(adc_dev != RT_NULL);

    if (channel > SARADC_CH_MAX) {
        LOG_E("[%s] Invalid channel: %d; Please input 1~3", adc_dev->name, channel);
        return -RT_EINVAL;
    }

    while (cvi_get_saradc_status(adc_dev->base) & SARADC_STATUS_BUSY)
    {
        rt_thread_delay(10);
        LOG_D("wait saradc ready");
        cnt ++;
        if (cnt > 100)
            return -RT_ETIMEOUT;
    }

    // Disable adc and clean old channel config
    cvi_reset_saradc_ctrl(adc_dev->base, SARADC_CTRL_START);
    cvi_reset_saradc_ctrl(adc_dev->base, SARADC_CTRL_CONTINUE_MODE);
    cvi_reset_saradc_ctrl(adc_dev->base, SARADC_CTRL_CH_MASK);

    //set saradc clock cycle
    cvi_set_cyc(adc_dev->base);

    // Update channel config and enable ADC
    cvi_set_saradc_ctrl(adc_dev->base, (rt_uint32_t)channel << (SARADC_CTRL_SEL_POS + 1));
    cvi_set_saradc_ctrl(adc_dev->base, SARADC_CTRL_CONTINUE_MODE);
    cvi_set_saradc_ctrl(adc_dev->base, SARADC_CTRL_START);
    rt_hw_us_delay(10);

    // Sampling
    result = mmio_read_32(adc_dev->base + SARADC_RESULT(channel - 1));
    if (result & SARADC_RESULT_VALID)
    {
        *value = result & SARADC_RESULT_MASK;
        LOG_D("saradc channel %d value: %04x", channel, *value);
    }
    else
    {
        LOG_E("saradc channel %d read failed. result:0x%04x", channel, result);
        return -RT_ERROR;
    }
    return RT_EOK;
}

static const struct rt_adc_ops _adc_ops =
{
    .enabled = _adc_enabled,
    .convert = _adc_convert,
};


#if defined (SOC_TYPE_CV180XB_QFN)

/*
 * cv180xb supports
 * - adc1 & adc2 for active domain
 * - adc3 for no-die domain
 */
#ifdef BSP_USING_ADC_ACTIVE_1
static const char *pinname_whitelist_adc1_active[] = {
    "ADC1",
    NULL,
};
static const char *pinname_whitelist_adc2_active[] = {
    NULL,
};
static const char *pinname_whitelist_adc3_active[] = {
    NULL,
};
#endif

#ifdef BSP_USING_ADC_NODIE_1
static const char *pinname_whitelist_adc1_nodie[] = {
    "PWR_GPIO2",
    NULL,
};
static const char *pinname_whitelist_adc2_nodie[] = {
    "PWR_GPIO1",
    NULL,
};
static const char *pinname_whitelist_adc3_nodie[] = {
    "PWR_VBAT_DET",
    NULL,
};
#endif

#elif defined (SOC_TYPE_CV181XC_QFN) || defined (SOC_TYPE_CV181XH_BGA)

#ifdef BSP_USING_ADC_ACTIVE_1
static const char *pinname_whitelist_adc1_active[] = {
    "ADC1",
    NULL,
};
static const char *pinname_whitelist_adc2_active[] = {
    NULL,
};
static const char *pinname_whitelist_adc3_active[] = {
    NULL,
};
#endif

#ifdef BSP_USING_ADC_NODIE_1
static const char *pinname_whitelist_adc1_nodie[] = {
    "PWR_GPIO2",
    NULL,
};
static const char *pinname_whitelist_adc2_nodie[] = {
    "PWR_GPIO1",
    NULL,
};
static const char *pinname_whitelist_adc3_nodie[] = {
    "PWR_VBAT_DET",
    NULL,
};
#endif

#elif defined (SOC_TYPE_CV184XH_BGA)

#ifdef BSP_USING_ADC_ACTIVE_1
static const char *pinname_whitelist_adc1_active[] = {
    "ADC1",
    NULL,
};
static const char *pinname_whitelist_adc2_active[] = {
    "ADC2",
    NULL,
};
static const char *pinname_whitelist_adc3_active[] = {
    "ADC3"
    NULL,
};
#endif

#ifdef BSP_USING_ADC_ACTIVE_2
static const char *pinname_whitelist_adc4_active[] = {
    "PWM0_BUCK",
    NULL,
};
static const char *pinname_whitelist_adc5_active[] = {
    "USB_VBUS_EN",
    NULL,
};
static const char *pinname_whitelist_adc6_active[] = {
    "USB_ID",
    NULL,
};
#endif

#ifdef BSP_USING_ADC_ACTIVE_3
static const char *pinname_whitelist_adc7_active[] = {
    "IIC3_SDA",
    NULL,
};
static const char *pinname_whitelist_adc8_active[] = {
    "IIC3_SCL",
    NULL,
};
static const char *pinname_whitelist_adc9_active[] = {
    "CAM_MCLK1",
    NULL,
};
#endif

#ifdef BSP_USING_ADC_NODIE_1
static const char *pinname_whitelist_adc1_nodie[] = {
    "PWR_SEQ3",
    NULL,
};
static const char *pinname_whitelist_adc2_nodie[] = {
    "PWR_SEQ1",
    NULL,
};
static const char *pinname_whitelist_adc3_nodie[] = {
    "PWR_VBAT_DET",
    NULL,
};
#endif

#ifdef BSP_USING_ADC_NODIE_2
static const char *pinname_whitelist_adc4_nodie[] = {
    "PWR_GPIO0",
    NULL,
};
static const char *pinname_whitelist_adc5_nodie[] = {
    "PWR_GPIO1",
    NULL,
};
static const char *pinname_whitelist_adc6_nodie[] = {
    "PWR_GPIO2",
    NULL,
};
#endif

#elif defined (SOC_TYPE_CV184XC_QFN)

#ifdef BSP_USING_ADC_ACTIVE_1
static const char *pinname_whitelist_adc1_active[] = {
    "ADC1",
    NULL,
};
static const char *pinname_whitelist_adc2_active[] = {
    NULL,
};
static const char *pinname_whitelist_adc3_active[] = {
    NULL,
};
#endif

#ifdef BSP_USING_ADC_ACTIVE_2
static const char *pinname_whitelist_adc4_active[] = {
    NULL,
};
static const char *pinname_whitelist_adc5_active[] = {
    NULL,
};
static const char *pinname_whitelist_adc6_active[] = {
    NULL,
};
#endif

#ifdef BSP_USING_ADC_ACTIVE_3
static const char *pinname_whitelist_adc7_active[] = {
    NULL,
};
static const char *pinname_whitelist_adc8_active[] = {
    NULL,
};
static const char *pinname_whitelist_adc9_active[] = {
    NULL,
};
#endif

#ifdef BSP_USING_ADC_NODIE_1
static const char *pinname_whitelist_adc1_nodie[] = {
    NULL,
};
static const char *pinname_whitelist_adc2_nodie[] = {
    NULL,
};
static const char *pinname_whitelist_adc3_nodie[] = {
    NULL,
};
#endif

#ifdef BSP_USING_ADC_NODIE_2
static const char *pinname_whitelist_adc4_nodie[] = {
    NULL,
};
static const char *pinname_whitelist_adc5_nodie[] = {
    NULL,
};
static const char *pinname_whitelist_adc6_nodie[] = {
    NULL,
};
#endif
#else
    #error "Unsupported board type!"
#endif

static void rt_hw_adc_pinmux_config()
{
#ifdef BSP_USING_ADC_ACTIVE_1
    pinmux_config(BSP_ACTIVE_ADC1_PINNAME, XGPIOB_3, pinname_whitelist_adc1_active);
    pinmux_config(BSP_ACTIVE_ADC2_PINNAME, XGPIOB_2, pinname_whitelist_adc2_active);
    pinmux_config(BSP_ACTIVE_ADC3_PINNAME, XGPIOB_1, pinname_whitelist_adc3_active);
#endif
#ifdef BSP_USING_ADC_ACTIVE_2
    pinmux_config(BSP_ACTIVE_ADC4_PINNAME, ADC4, pinname_whitelist_adc4_active);
    pinmux_config(BSP_ACTIVE_ADC5_PINNAME, ADC5, pinname_whitelist_adc5_active);
    pinmux_config(BSP_ACTIVE_ADC6_PINNAME, ADC6, pinname_whitelist_adc6_active);
#endif
#ifdef BSP_USING_ADC_ACTIVE_3
    pinmux_config(BSP_ACTIVE_ADC7_PINNAME, ADC7, pinname_whitelist_adc7_active);
    pinmux_config(BSP_ACTIVE_ADC8_PINNAME, ADC8, pinname_whitelist_adc8_active);
    pinmux_config(BSP_ACTIVE_ADC9_PINNAME, ADC9, pinname_whitelist_adc9_active);
#endif
    /* QFN chip DO NOT have adc channel 2~9 */

#ifdef BSP_USING_ADC_NODIE_1
    pinmux_config(BSP_NODIE_ADC1_PINNAME, PWR_SEQ3, pinname_whitelist_adc1_nodie);
    pinmux_config(BSP_NODIE_ADC2_PINNAME, PWR_SEQ1, pinname_whitelist_adc2_nodie);
    pinmux_config(BSP_NODIE_ADC3_PINNAME, PWR_VBAT_DET, pinname_whitelist_adc3_nodie);
#endif
#ifdef BSP_USING_ADC_NODIE_2
    pinmux_config(BSP_NODIE_ADC4_PINNAME, PWR_GPIO0, pinname_whitelist_adc1_nodie);
    pinmux_config(BSP_NODIE_ADC5_PINNAME, PWR_GPIO1, pinname_whitelist_adc2_nodie);
    pinmux_config(BSP_NODIE_ADC6_PINNAME, PWR_GPIO2, pinname_whitelist_adc3_nodie);
#endif
    /* QFN chip DO NOT have pwr adc channel*/
}

int rt_hw_adc_init(void)
{
    rt_uint8_t i;

    rt_hw_adc_pinmux_config();

    for (i = 0; i < sizeof(adc_dev_config) / sizeof(adc_dev_config[0]); i++)
    {
        cvi_do_calibration(adc_dev_config[i].base);
    }

    for (i = 0; i < sizeof(adc_dev_config) / sizeof(adc_dev_config[0]); i++)
    {
        if (rt_hw_adc_register(&adc_dev_config[i].device, adc_dev_config[i].name, &_adc_ops, &adc_dev_config[i]) != RT_EOK)
        {
            LOG_E("%s register failed!", adc_dev_config[i].name);
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_adc_init);
