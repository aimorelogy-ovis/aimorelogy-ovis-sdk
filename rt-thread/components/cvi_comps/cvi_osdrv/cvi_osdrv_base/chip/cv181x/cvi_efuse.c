// SPDX-License-Identifier: GPL-2.0+
//
// EFUSE implementation
//

#include "cvi_efuse.h"
#include "cvi_base.h"
#include "errno.h"
#include "mmio.h"
#include <stdio.h>
#include <string.h>

#define ERROR(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define VERBOSE(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define EFUSE_BASE (0x03050000)

#define EFUSE_SHADOW_REG (EFUSE_BASE + 0x100)
#define EFUSE_SIZE 0x100

#define EFUSE_MODE (EFUSE_BASE + 0x0)
#define EFUSE_ADR (EFUSE_BASE + 0x4)
#define EFUSE_DIR_CMD (EFUSE_BASE + 0x8)
#define EFUSE_RD_DATA (EFUSE_BASE + 0xC)
#define EFUSE_STATUS (EFUSE_BASE + 0x10)
#define EFUSE_ONE_WAY (EFUSE_BASE + 0x14)

#define EFUSE_BIT_AREAD BIT(0)
#define EFUSE_BIT_MREAD BIT(1)
#define EFUSE_BIT_PRG BIT(2)
#define EFUSE_BIT_PWR_DN BIT(3)
#define EFUSE_BIT_CMD BIT(4)
#define EFUSE_BIT_BUSY BIT(0)
#define EFUSE_CMD_REFRESH (0x30)

enum EFUSE_READ_TYPE { EFUSE_AREAD,
                       EFUSE_MREAD };

void cvi_efuse_wait_for_ready(void)
{
    while (mmio_read_32(EFUSE_STATUS) & EFUSE_BIT_BUSY)
        ;
}

static void cvi_efuse_power_on(CVI_U32 on)
{
    if (on)
        mmio_setbits_32(EFUSE_MODE, EFUSE_BIT_CMD);
    else
        mmio_setbits_32(EFUSE_MODE, EFUSE_BIT_PWR_DN | EFUSE_BIT_CMD);
}

static void cvi_efuse_refresh(void)
{
    mmio_write_32(EFUSE_MODE, EFUSE_CMD_REFRESH);
}

static void cvi_efuse_prog_bit(CVI_U32 word_addr, CVI_U32 bit_addr,
                               CVI_U32 high_row)
{
    CVI_U32 phy_addr;

    // word_addr: virtual addr, take "lower 6-bits" from 7-bits (0-127)
    // bit_addr: virtual addr, 5-bits (0-31)

    // composite physical addr[11:0] = [11:7]bit_addr + [6:0]word_addr
    phy_addr =
        ((bit_addr & 0x1F) << 7) | ((word_addr & 0x3F) << 1) | high_row;

    cvi_efuse_wait_for_ready();

    // send efuse program cmd
    mmio_write_32(EFUSE_ADR, phy_addr);
    mmio_write_32(EFUSE_MODE, EFUSE_BIT_PRG | EFUSE_BIT_CMD);
}

static CVI_U32 cvi_efuse_read_from_phy(CVI_U32 phy_word_addr,
                                       enum EFUSE_READ_TYPE type)
{
    // power on efuse macro
    cvi_efuse_power_on(1);

    cvi_efuse_wait_for_ready();

    mmio_write_32(EFUSE_ADR, phy_word_addr);

    if (type == EFUSE_AREAD) // array read
        mmio_write_32(EFUSE_MODE, EFUSE_BIT_AREAD | EFUSE_BIT_CMD);
    else if (type == EFUSE_MREAD) // margin read
        mmio_write_32(EFUSE_MODE, EFUSE_BIT_MREAD | EFUSE_BIT_CMD);
    else {
        ERROR("EFUSE: Unsupported read type!");
        return (CVI_U32)-1;
    }

    cvi_efuse_wait_for_ready();

    return mmio_read_32(EFUSE_RD_DATA);
}

static int cvi_efuse_write_word(CVI_U32 vir_word_addr, CVI_U32 val)
{
    CVI_U32 i, j, row_val, zero_bit;
    CVI_U32 new_value;
    int err_cnt = 0;

    for (j = 0; j < 2; j++) {
        VERBOSE("EFUSE: Program physical word addr #%d\n",
                (vir_word_addr << 1) | j);

        // array read by word address
        row_val = cvi_efuse_read_from_phy(
            (vir_word_addr << 1) | j,
            EFUSE_AREAD);            // read low word of word_addr
        zero_bit = val & (~row_val); // only program zero bit

        // program row which bit is zero
        for (i = 0; i < 32; i++) {
            if ((zero_bit >> i) & 1)
                cvi_efuse_prog_bit(vir_word_addr, i, j);
        }

        // check by margin read
        new_value = cvi_efuse_read_from_phy((vir_word_addr << 1) | j,
                                            EFUSE_MREAD);
        VERBOSE("%s(): val=0x%x new_value=0x%x\n", __func__, val,
                new_value);
        if ((val & new_value) != val) {
            err_cnt += 1;
            ERROR("EFUSE: Program bits check failed (%d)!\n",
                  err_cnt);
        }
    }

    cvi_efuse_refresh();

    return err_cnt >= 2 ? -EIO : 0;
}

CVI_S64 cvi_efuse_read_from_shadow(CVI_U32 addr)
{
    CVI_S64 ret = -1;

    if (addr >= EFUSE_SIZE)
        return -1;

    if (addr % 4 != 0)
        return -1;

    ret = mmio_read_32(EFUSE_SHADOW_REG + addr);

    return ret;
}

int cvi_efuse_write(CVI_U32 addr, CVI_U32 value)
{
    int ret;

    VERBOSE("%s(): 0x%x = 0x%x\n", __func__, addr, value);

    if (addr >= EFUSE_SIZE)
        return -1;

    if (addr % 4 != 0)
        return -1;

    ret = cvi_efuse_write_word(addr / 4, value);
    VERBOSE("%s(): ret=%d\n", __func__, ret);

    cvi_efuse_power_on(1);
    cvi_efuse_refresh();
    cvi_efuse_wait_for_ready();

    return ret;
}

int cvi_efuse_read_buf(u32 addr, void *buf, size_t buf_size)
{
    CVI_S64 ret = -1;
    int i;

    if (!buf)
        return -1;

    if (buf_size > EFUSE_SIZE)
        buf_size = EFUSE_SIZE;

    memset(buf, 0, buf_size);

    for (i = 0; i < buf_size; i += 4) {
        ret = cvi_efuse_read_from_shadow(addr + i);
        if (ret < 0)
            return ret;

        if (ret > 0) {
            u32 v = ret;

            memcpy(buf + i, &v, sizeof(v));
        }
    }

    return buf_size;
}
