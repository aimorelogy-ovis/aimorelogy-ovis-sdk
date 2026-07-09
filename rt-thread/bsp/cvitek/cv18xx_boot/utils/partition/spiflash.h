#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

int hal_spiflash_init(void);
int hal_spiflash_read(uint32_t offset, void *data, uint32_t size);

#endif