
#include <rtthread.h>
#include <stdio.h>
#include "cvitek_spi_nor.h"
#include "spi_nor.h"

static struct cvi_spif g_cvi_spif = {0};

int hal_spiflash_init(void)
{
	struct cvi_spif *spif = NULL;
	uint32_t reg;

	spif = &g_cvi_spif;

	memset(spif, 0x0, sizeof(struct cvi_spif));

	if (spif->io_base != NULL)
		return 0;

	if (spif->io_base == NULL)
		spif->io_base = (void *)SPI_NOR_REGBASE;

	/* open gate clk for spi nor flash */
	reg = readl((void *)0x03002004);
	writel(reg | 0x1, (void *)0x03002004);
	/* open gate clk for spi nor flash */
	reg = readl((void *)0x03002004);
	writel(reg | 0x1, (void *)0x03002004);

	/* set clk to 75M */
	cvi_spif_clk_setup(spif, 1);
	cvi_spif_setup_flash(spif);

	if (spi_nor_rescan(&spif->nor)) {
		printf("scan flash failed!\n");
		return -1;
	}

	return 0;

}

int hal_spiflash_read(uint32_t offset, void *data, uint32_t size)
{
#define _32K      (0x8000UL)
	struct spi_nor *nor = &g_cvi_spif.nor;
	int32_t len = 0;
	uint32_t read_size;
	int ret;

	if (nor->read == NULL) {
		printf("there is no read operation!\n");
		return -1;
	}

	while (size) {
		if (size >= _32K)
			read_size = _32K;
		else
			read_size = size & (_32K - 1);

		ret = nor->read(nor, offset, read_size, data);
		if (ret < 0) {
			printf("read data failed, ret:%d\n", ret);
			return -1;
		}
		offset += read_size;
		data += read_size;
		len += read_size;
		size -= read_size;
	}
	return len;
}