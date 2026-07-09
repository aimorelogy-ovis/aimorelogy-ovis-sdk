
#include <drivers/mtd_nor.h>
#include <fal.h>

#ifdef RT_USING_SFUD
#include <dev_spi_flash_sfud.h>
#endif

#ifndef FAL_USING_NOR_FLASH_DEV_NAME
#define FAL_USING_NOR_FLASH_DEV_NAME "norflash0"
#endif

static int init(void);
static int read(long offset, uint8_t *buf, size_t size);
static int write(long offset, const uint8_t *buf, size_t size);
static int erase(long offset, size_t size);

rt_device_t mtd = NULL;

static struct rt_mtd_nor_device *mtd_device;

struct fal_flash_dev nor_flash0 = {
    .name       = FAL_USING_NOR_FLASH_DEV_NAME,
    .addr       = 0,
    .ops        = {init, read, write, erase},
    .write_gran = 1,
};

static int init(void)
{
    mtd = rt_device_find("norflash0");
    if (NULL == mtd) {
        return -1;
    }

    mtd_device = (struct rt_mtd_nor_device *)mtd;

    /* update the flash chip information */
    nor_flash0.blk_size = mtd_device->block_size;
    nor_flash0.len      = mtd_device->block_end;

    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    assert(mtd_device);
    mtd_device->ops->read(mtd_device, nor_flash0.addr + offset, buf, size);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    assert(mtd_device);
    if (mtd_device->ops->write(mtd_device, nor_flash0.addr + offset, buf, size) < 0) return -1;

    return size;
}

static int erase(long offset, size_t size)
{
    assert(mtd_device);
    if (mtd_device->ops->erase_block(mtd_device, nor_flash0.addr + offset, size) < 0) return size;

    return size;
}
