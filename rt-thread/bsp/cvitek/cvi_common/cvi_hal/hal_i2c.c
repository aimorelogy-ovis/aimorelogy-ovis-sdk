/*
 * Demo program on CviTek cv18xx
 * Copyright CviTek Technologies. All Rights Reserved.
 *
 * Change Logs:
 * Date           Author        Notes           Contact
 * 2024-12-30     shuhan.zhang  first version   shuhan.zhang@sophgo.com
 */
#include <rtdbg.h>
#include "hal_i2c.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int cvi_i2c_init(void)
{
    return rt_hw_i2c_init();
}

rt_ssize_t cvi_i2c_transfer(const char   *name,
                            cvi_i2c_msg_t msgs[],
                            uint32_t      num)
{
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(name);
    if (!bus)
    {
        LOG_E("I2C bus %s not exist", name);
        return RT_NULL;
    }

    return rt_i2c_transfer(bus, msgs, num);
}

rt_err_t cvi_i2c_control(const char *name,
                         int         cmd,
                         void       *args)
{
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(name);
    if (!bus)
    {
        LOG_E("I2C bus %s not exist", name);
        return RT_NULL;
    }

    return rt_i2c_control(bus, cmd, args);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* exapmles1:
 * flages write 0 ,defult 7 bit address
 * i2c0 send , slave addr is 0x29, slave reg addr is 0x45,
 * the data you want to write is 0x78 ,len is 2 byte
 *
 *	cvi_i2c_msg_t msg[1];
 *	uint8_t buf[2];
 *
 *	buf[0] = 0x45;
 *	buf[1] = 0x78;
 *	msgs->addr = 0x10;
 *	msgs->flags = CVI_I2C_WR;
 *	msgs->buf = buf;
 *	msgs->len = 2;
 *
 *	cvi_i2c_transfer(CVI_I2C_I2C0, msgs, 1);
 */

/* exapmles2:
 * i2c1 write with 10bit mode, slave addr is 0x329, slave reg addr is 0x4567
 *
 *	cvi_i2c_msg_t msgs[2];
 *	uint8_t reg[2];
 *	uint8_t data[4];
 *
 *	reg[0] = 0x45;
 *	reg[1] = 0x67;
 *	msgs[0].addr = 0x329;
 *	msgs[0].flags =CVI_I2C_ADDR_10BIT | CVI_I2C_WR;
 *	msgs[0].buf = reg;
 *	msgs[0].len = 2;
 *
 *	msgs[1].addr = 0x329;
 *	msgs[1].flags =CVI_I2C_ADDR_10BIT | CVI_I2C_RD;
 *	msgs[1].buf = data;
 *	msgs[1].len = 4;
 *
 *	cvi_i2c_transfer(CVI_I2C_I2C1, msgs, 2);
 */

/* exapmles3:
* set  i2c0  speed to  400KHz
* cvi_i2c_control("i2c0",CVI_I2C_DEV_CTRL_CLK, CVI_I2C_SPEED_FAST);
*/

