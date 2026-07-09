#include "sensor_i2c.h"

#include <stdio.h>

int sensor_i2c_init(CVI_U8 i2c_id)
{
    // return rt_hw_i2c_init();
    return CVI_SUCCESS;
}

int sensor_i2c_read(CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr, CVI_U32 addr, CVI_U32 snsr_addr_byte,
                    CVI_U32 snsr_data_byte)
{
    int                       ret = 0;
    char                      i2c_name[64];
    struct rt_i2c_bus_device *i2c_bus;
    sprintf(i2c_name, "i2c%d", i2c_id);
    struct rt_i2c_msg msgs[2];
    uint8_t           reg[snsr_addr_byte];
    uint8_t           data[snsr_data_byte];
    uint16_t          value;

    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(i2c_name);

    if (i2c_id >= IIC_MAX) {
        return CVI_FAILURE;
    }

    if (snsr_addr_byte == 2) {
        reg[0] = addr >> 8;
        reg[1] = addr & 0xff;
    } else if (snsr_addr_byte == 1) {
        reg[0] = addr;
    }

    msgs[0].addr  = snsr_i2c_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = reg;
    msgs[0].len   = snsr_addr_byte;

    msgs[1].addr  = snsr_i2c_addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = data;
    msgs[1].len   = snsr_addr_byte;

    ret = rt_i2c_transfer(i2c_bus, msgs, 2);

    if (snsr_addr_byte == 2) {
        value = (data[0] << 8) | data[1];
    } else if (snsr_addr_byte == 1) {
        value = data[0];
    }
    // printf("i2c r 0x%x = 0x%02x\n", addr, value);
    return value;
}

int sensor_i2c_write(CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr, CVI_U32 addr, CVI_U32 snsr_addr_byte,
                     CVI_U32 data, CVI_U32 snsr_data_byte)
{
    int                       ret = 0;
    char                      i2c_name[64];
    struct rt_i2c_bus_device *i2c_bus;
    struct rt_i2c_msg         msgs[1];
    uint8_t                   buf[snsr_addr_byte + snsr_data_byte];

    sprintf(i2c_name, "i2c%d", i2c_id);

    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(i2c_name);

    if (snsr_addr_byte == 2) {
        buf[0] = addr >> 8;
        buf[1] = addr & 0xff;
        if (snsr_data_byte == 2) {
            buf[2] = data >> 8;
            buf[3] = data & 0xff;
        } else if (snsr_data_byte == 1) {
            buf[2] = data;
        }
    } else if (snsr_addr_byte == 1) {
        buf[0] = addr;
        if (snsr_data_byte == 2) {
            buf[1] = data >> 8;
            buf[2] = data & 0xff;
        } else if (snsr_data_byte == 1) {
            buf[1] = data;
        }
    }

    msgs[0].addr  = snsr_i2c_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = buf;
    msgs[0].len   = snsr_addr_byte + snsr_data_byte;

    ret = rt_i2c_transfer(i2c_bus, msgs, 1);

    // printf("i2c w 0x%x = 0x%2x\n", addr, data);
    return ret;
}
