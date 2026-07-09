#ifndef __CVI_HAL_GPIO_H__
#define __CVI_HAL_GPIO_H__

#include <rtthread.h>
#include <drivers/dev_pin.h>

/*
 * <b>Example</b>
 * @code {.c}
 * #include <rtthread.h>
 * #include <rtdevice.h>
 *
 *
 * #ifndef BEEP_PIN_NUM
 *     #define BEEP_PIN_NUM            35  // PB0
 * #endif
 * #ifndef KEY0_PIN_NUM
 *     #define KEY0_PIN_NUM            55  // PD8
 * #endif
 * #ifndef KEY1_PIN_NUM
 *     #define KEY1_PIN_NUM            56  // PD9
 * #endif
 *
 * void beep_on(void *args)
 * {
 *     rt_kprintf("turn on beep!\n");
 *
 *     cvi_pin_write(BEEP_PIN_NUM, PIN_HIGH);
 * }
 *
 * void beep_off(void *args)
 * {
 *     rt_kprintf("turn off beep!\n");
 *
 *     cvi_pin_write(BEEP_PIN_NUM, PIN_LOW);
 * }
 *
 * static void pin_beep_sample(void)
 * {
 *     cvi_pin_mode(BEEP_PIN_NUM, PIN_MODE_OUTPUT);
 *     cvi_pin_write(BEEP_PIN_NUM, PIN_LOW);
 *
 *     cvi_pin_mode(KEY0_PIN_NUM, PIN_MODE_INPUT_PULLUP);
 *     cvi_pin_attach_irq(KEY0_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_on, RT_NULL);
 *     cvi_pin_irq_enable(KEY0_PIN_NUM, PIN_IRQ_ENABLE);
 *
 *
 *     cvi_pin_mode(KEY1_PIN_NUM, PIN_MODE_INPUT_PULLUP);
 *     cvi_pin_attach_irq(KEY1_PIN_NUM, PIN_IRQ_MODE_FALLING, beep_off, RT_NULL);
 *     cvi_pin_irq_enable(KEY1_PIN_NUM, PIN_IRQ_ENABLE);
 * }
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RT_USING_DM
#include <drivers/pic.h>
typedef struct rt_pin_irqchip cvi_pin_irqchip_t;
#endif /* RT_USING_DM */

typedef struct rt_device_pin cvi_device_pin_t;

typedef struct rt_device_pin_mode  cvi_device_pin_mode_t;
typedef struct rt_device_pin_value cvi_device_pin_value_t;
typedef struct rt_pin_irq_hdr      cvi_pin_irq_hdr_t;
#ifdef RT_USING_PINCTRL
typedef struct rt_pin_ctrl_conf_params cvi_pin_ctrl_conf_params_t;
#endif /* RT_USING_PINCTRL */

/**
 * @brief set pin mode
 * @param pin the pin number
 * @param mode the pin mode
 */
void cvi_pin_mode(rt_base_t pin, rt_uint8_t mode);

/**
 * @brief write pin value
 * @param pin the pin number
 * @param value the pin value
 */
void cvi_pin_write(rt_base_t pin, rt_ssize_t value);

/**
 * @brief read pin value
 * @param pin the pin number
 * @return rt_ssize_t the pin value
 */
rt_ssize_t cvi_pin_read(rt_base_t pin);

/**
 * @brief get pin number by name
 * @param name the pin name
 * @return rt_base_t the pin number
 */
rt_base_t cvi_pin_get(const char *name);

/**
 * @brief bind the pin interrupt callback function
 * @param pin the pin number
 * @param mode the irq mode
 * @param hdr the irq callback function
 * @param args the argument of the callback function
 * @return rt_err_t error code
 */
rt_err_t cvi_pin_attach_irq(rt_base_t pin, rt_uint8_t      mode,
                            void (*hdr)(void *args), void *args);

/**
 * @brief detach the pin interrupt callback function
 * @param pin the pin number
 * @return rt_err_t error code
 */
rt_err_t cvi_pin_detach_irq(rt_base_t pin);

/**
 * @brief enable or disable the pin interrupt
 * @param pin the pin number
 * @param enabled PIN_IRQ_ENABLE or PIN_IRQ_DISABLE
 * @return rt_err_t error code
 */
rt_err_t cvi_pin_irq_enable(rt_base_t pin, rt_uint8_t enabled);

#ifdef RT_USING_DM
rt_ssize_t cvi_pin_get_named_pin(struct rt_device *dev, const char *propname, int index,
                                 rt_uint8_t *out_mode, rt_uint8_t *out_value);
rt_ssize_t cvi_pin_get_named_pin_count(struct rt_device *dev, const char *propname);

#ifdef RT_USING_OFW
rt_ssize_t cvi_ofw_get_named_pin(struct rt_ofw_node *np, const char *propname, int index,
                                 rt_uint8_t *out_mode, rt_uint8_t *out_value);
rt_ssize_t cvi_ofw_get_named_pin_count(struct rt_ofw_node *np, const char *propname);
#endif
#endif /* RT_USING_DM */

#ifdef RT_USING_PINCTRL
rt_ssize_t cvi_pin_ctrl_confs_lookup(struct rt_device *device, const char *name);
rt_err_t   cvi_pin_ctrl_confs_apply(struct rt_device *device, int index);
rt_err_t   cvi_pin_ctrl_confs_apply_by_name(struct rt_device *device, const char *name);
#endif /* RT_USING_PINCTRL */

#ifdef __cplusplus
}
#endif

/*! @}*/

#endif
