/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024/02/22     flyingcys    first version
 */
#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

#include "pinctrl.h"
#include "mmio.h"

#define SARADC_BASE                         0x030F0000
#define SARADC_SIZE                         0x1000
#define RTC_ADC_BASE                        0x0502C000
#define TOP_ADC_PREFIX                      0x030F0000
#define RTC_ADC_PREFIX                      0x05020000
#define SARADC_CH_MAX                       3

#define SARADC_CTRL_OFFSET                  0x04
#define SARADC_CTRL_START                   (1 << 0)
#define SARADC_CTRL_CONTINUE_MODE           (1 << 1)
#define SARADC_CTRL_PERIOD_MODE             (1 << 2)
#define SARADC_CTRL_SEL_POS                 0x04
#define SARADC_CTRL_CH_MASK                 0xF0

#define SARADC_STATUS_OFFSET                0x08
#define SARADC_STATUS_BUSY                  (1 << 0)

#define SARADC_CYC_SET_OFFSET               0x0C
#define SARADC_CYC_CLKDIV_DIV_POS           (12U)
#define SARADC_CYC_CLKDIV_DIV_MASK          (0xF << SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_1             (0U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_2             (1U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_3             (2U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_4             (3U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_5             (4U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_6             (5U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_7             (6U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_8             (7U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_9             (8U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_10            (9U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_11            (10U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_12            (11U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_13            (12U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_14            (13U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_15            (14U<< SARADC_CYC_CLKDIV_DIV_POS)
#define SARADC_CYC_CLKDIV_DIV_16            (15U<< SARADC_CYC_CLKDIV_DIV_POS)

#define SARADC_RESULT_OFFSET                0x014
#define SARADC_RESULT(n)                    (SARADC_RESULT_OFFSET + (n) * 4)
#define SARADC_RESULT_MASK                  0x0FFF
#define SARADC_RESULT_VALID                 (1 << 15)

#define SARADC_INT_REG                      0x20
#define SARADC_INT_CLR                      0x24
#define SARADC_INT_STATUS                   0x28

#define SARADC_TEST_OFFSET                  0x30
#define SARADC_TEST_VREFSEL_BIT             2

#define SARADC_TRIM_REG                     0x34
#define SARADC_EFUSE_TRIM_BASE              0x03050118
#define TOP_SARADC_TRIM_MASK                0xf0000000
#define TOP_SARADC_TRIM_OFFSET              28
#define RTC_SARADC_TRIM_MASK                0x0f000000
#define RTC_SARADC_TRIM_OFFSET              24

int rt_hw_adc_init(void);

#endif /* __DRV_ADC_H__ */
