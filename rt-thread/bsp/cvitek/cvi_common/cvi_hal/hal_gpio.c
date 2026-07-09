/*
 * Demo program on CviTek cv18xx
 * Copyright CviTek Technologies. All Rights Reserved.
 *
 * Change Logs:
 * Date           Author        Notes           Contact
 * 2025-1-10     songtao.lian  first version   songtao.lian@sophgo.com
 */
#include <rtdbg.h>
#include "hal_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


void cvi_pin_mode(rt_base_t pin, rt_uint8_t mode)
{
    rt_pin_mode(pin, mode);
}

void cvi_pin_write(rt_base_t pin, rt_ssize_t value)
{
    rt_pin_write(pin, value);
}

rt_ssize_t cvi_pin_read(rt_base_t pin)
{
    return rt_pin_read(pin);
}

rt_base_t cvi_pin_get(const char *name)
{
    return rt_pin_get(name);
}

rt_err_t cvi_pin_attach_irq(rt_base_t pin, rt_uint8_t      mode,
                            void (*hdr)(void *args), void *args)
{
    return rt_pin_attach_irq(pin, mode, hdr, args);
}

rt_err_t cvi_pin_detach_irq(rt_base_t pin)
{
    return rt_pin_detach_irq(pin);
}

rt_err_t cvi_pin_irq_enable(rt_base_t pin, rt_uint8_t enabled)
{
    return rt_pin_irq_enable(pin, enabled);
}

rt_ssize_t cvi_pin_get_named_pin(struct rt_device *dev, const char *propname, int index,
                                 rt_uint8_t *out_mode, rt_uint8_t *out_value)
{
    return rt_pin_get_named_pin(dev, propname, index, out_mode, out_value);
}

rt_ssize_t cvi_pin_get_named_pin_count(struct rt_device *dev, const char *propname)
{
    return rt_pin_get_named_pin_count(dev, propname);
}

rt_ssize_t cvi_ofw_get_named_pin(struct rt_ofw_node *np, const char *propname, int index,
                                 rt_uint8_t *out_mode, rt_uint8_t *out_value)
{
    return rt_ofw_get_named_pin(np, propname, index, out_mode, out_value);
}

rt_ssize_t cvi_ofw_get_named_pin_count(struct rt_ofw_node *np, const char *propname)
{
    return rt_ofw_get_named_pin_count(np, propname);
}

rt_ssize_t cvi_pin_ctrl_confs_lookup(struct rt_device *device, const char *name)
{
    return rt_pin_ctrl_confs_lookup(device, name);
}

rt_err_t cvi_pin_ctrl_confs_apply(struct rt_device *device, int index)
{
    return rt_pin_ctrl_confs_apply(device, index);
}

rt_err_t cvi_pin_ctrl_confs_apply_by_name(struct rt_device *device, const char *name)
{
    return rt_pin_ctrl_confs_apply_by_name(device, name);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
