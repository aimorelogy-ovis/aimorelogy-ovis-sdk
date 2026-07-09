#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#include <drivers/mtd_nor.h>

typedef struct {
    char    *flash_name;   // Name string of spiflash
    uint32_t flash_id;     // JEDEC ID  = manufature ID <<16 | device ID (ID15~ID0)
    uint32_t flash_size;   // Flash chip size
    uint32_t xip_addr;     // If use qspi controler to access flash ,code can be ececuted on flash
                           // ,the addr is xip addr
    uint32_t sector_size;  // Sector size
    uint32_t page_size;    // Page size for read or program
} hal_spiflash_info_t;

struct cvi_spif *hal_spiflash_init(void);

int32_t hal_spiflash_read(struct rt_mtd_nor_device *device, uint32_t offset, void *data,
                          uint32_t size);
int32_t hal_spiflash_write(struct rt_mtd_nor_device *device, uint32_t offset, const void *data,
                           uint32_t size);
int32_t hal_spiflash_erase(struct rt_mtd_nor_device *device, uint32_t offset, uint32_t size);
int32_t hal_spiflash_get_flash_info(hal_spiflash_info_t *flash_info);
int32_t hal_spiflash_read_reg(uint8_t cmd_code, uint8_t *data, uint32_t size);
int32_t hal_spiflash_write_reg(uint8_t cmd_code, uint8_t *data, uint32_t size);
#endif