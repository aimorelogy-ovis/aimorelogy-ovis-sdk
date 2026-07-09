#include <board.h>
#include <ctype.h>
#include <rtdevice.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmio.h"

int str_is_hex(const char *str)
{
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) str += 2;
    while (*str) {
        if (!isxdigit(*str)) return 0;
        str++;
    }
    return 1;
}

int str_is_digit(const char *str)
{
    if (str == NULL || *str == '\0') return 0;
    while (*str) {
        if (!isdigit((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}

#ifdef BSP_USING_I2C_TOOL
const char *i2c_idx_to_name(int idx)
{
    static const char *const i2c_names[] = {"i2c0", "i2c1", "i2c2", "i2c3", "i2c4"};

    if (idx < 0 || idx > 4) {
        printf("i2c idx error\n");
        return NULL;
    }

    return i2c_names[idx];
}

static void i2cdetect(int argc, char **argv)
{
    uint16_t                  i, j, idx;
    uint8_t                   data[2];
    struct rt_i2c_bus_device *bus;

    if (argc != 2) {
        rt_kprintf("usage: %s <idx>\n", __func__);
        return;
    }

    idx     = atoi(argv[1]);
    bus     = rt_i2c_bus_device_find(i2c_idx_to_name(idx));
    data[0] = 0x11;

    if (!bus) {
        rt_kprintf("----%d bus no found---\n", idx);
        return;
    }

    rt_kprintf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (j = 0; j < 16; j++) {
            fflush(stdout);
            if (rt_i2c_master_recv(bus, i + j, RT_I2C_IGNORE_NACK, data, 1) < 0)
                rt_kprintf("-- ");
            else
                rt_kprintf("%02x ", i + j);
        }
        rt_kprintf("\n");
    }
}
MSH_CMD_EXPORT(i2cdetect, "i2cdetect < idx >");

void i2cget(int argc, char **argv)
{
    uint32_t                  dev_addr, reg_addr, len;
    uint16_t                  idx, value;
    uint8_t                   data[2], reg[2], read_16bit = 0;
    struct rt_i2c_bus_device *bus;
    struct rt_i2c_msg         msgs[2];

    if (argc < 4) {
        rt_kprintf("usage: %s <idx> <dev_addr> <reg_addr> [-w]\n", __func__);
        return;
    }

    idx = atoi(argv[1]);
    if (!idx) idx = 0;
    bus = rt_i2c_bus_device_find(i2c_idx_to_name(idx));
    if (!bus) {
        rt_kprintf("----%d bus not found---\n", idx);
        return;
    }

    if (str_is_hex(argv[2]))
        sscanf(argv[2], "%x", &dev_addr);
    else {
        rt_kprintf("dev_addr data type error\n");
        return;
    }

    if (str_is_hex(argv[3]))
        sscanf(argv[3], "%x", &reg_addr);
    else {
        rt_kprintf("reg_addr data type error\n");
        return;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            read_16bit = 1;
            break;
        }
    }

    if (read_16bit) {
        reg[0] = (uint8_t)(reg_addr >> 8);
        reg[1] = (uint8_t)(reg_addr);
        len    = 2;
    } else {
        reg[0] = (uint8_t)(reg_addr);
        len    = 1;
    }

    msgs[0].addr  = dev_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = reg;
    msgs[0].len   = len;

    msgs[1].addr  = dev_addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = data;
    msgs[1].len   = len;
    rt_i2c_transfer(bus, msgs, 2);

    if (len == 2) {
        value = (data[0] << 8) | data[1];
        rt_kprintf("0x%x\n", value);
    } else if (len == 1) {
        rt_kprintf("0x%x\n", data[0]);
    } else
        rt_kprintf("i2c error\n");
}
MSH_CMD_EXPORT(i2cget, "i2cget < idx > < dev_addr > < reg_addr > [-w]");

void i2cset(int argc, char **argv)
{
    uint32_t                  dev_addr, reg_addr, value, len;
    uint16_t                  idx;
    uint8_t                   data[4], write_16bit = 0;
    struct rt_i2c_bus_device *bus;
    struct rt_i2c_msg         msgs[1];

    if (argc != 5 && argc != 6) {
        rt_kprintf("usage: %s <idx> <dev_addr> <reg_addr> <value> [-w]\n", __func__);
        return;
    }

    idx = atoi(argv[1]);
    if (!idx) idx = 0;
    bus = rt_i2c_bus_device_find(i2c_idx_to_name(idx));

    if (!bus) {
        rt_kprintf("----%d bus not found---\n", idx);
        return;
    }

    if (str_is_hex(argv[2]))
        sscanf(argv[2], "%x", &dev_addr);
    else {
        rt_kprintf("dev_addr data type error\n");
        return;
    }

    if (str_is_hex(argv[3]))
        sscanf(argv[3], "%x", &reg_addr);
    else {
        rt_kprintf("reg_addr data type error\n");
        return;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            write_16bit = 1;
            break;
        }
    }

    if (str_is_hex(argv[4]))
        sscanf(argv[4], "%x", &value);
    else if (str_is_digit(argv[4])) {
        value = atoi(argv[4]);
    } else {
        rt_kprintf("value data type error\n");
        return;
    }

    if (write_16bit) {
        if (argc < 6) {
            rt_kprintf("Missing value for 16-bit write\n");
            return;
        }
        data[0] = (uint8_t)(reg_addr >> 8);
        data[1] = (uint8_t)(reg_addr & 0xFF);
        data[2] = (uint8_t)(value >> 8);
        data[3] = (uint8_t)(value & 0xFF);
        len     = 4;
    } else {
        data[0] = (uint8_t)(reg_addr);
        data[1] = (uint8_t)(value & 0xFF);
        len     = 2;
    }

    msgs->addr  = dev_addr;
    msgs->flags = RT_I2C_WR;
    msgs->buf   = data;
    msgs->len   = len;

    rt_i2c_transfer(bus, msgs, 1);
}
MSH_CMD_EXPORT(i2cset, "i2cset < idx > < dev_addr > < reg_addr > < value > [-w]");
#endif
