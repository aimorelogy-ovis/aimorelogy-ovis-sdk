#include <board.h>
#include <rtdevice.h>
#include <stdint.h>
#include <stdlib.h>

#include "mmio.h"

void devmem(int argc, char **argv)
{
    uint32_t addr, value;
    char    *endptr;

    if (argc != 2 && argc != 4) {
        rt_kprintf("usage: %s <addr>\n    %s <addr> 32 <value>\n", __func__, __func__);
        return;
    }

    // Parse address (argv[1]) - expected hexadecimal
    addr = strtoul(argv[1], &endptr, 16);
    if (endptr == argv[1] ||
        *endptr != '\0') {  // Check if the entire string was successfully converted
        rt_kprintf("dev_addr data type error (must be hex)\n");
        return;
    }

    if (argc == 2) {  // read
        rt_kprintf("0x%x\n", mmio_read_32(addr));
    } else if (argc == 4) {  // write
        // Width check (argv[2]) - atoi is suitable for simple decimal number "32"
        if (atoi(argv[2]) != 32) {
            rt_kprintf("32 bit width only\n");
            return;
        }

        // Parse value (argv[3]) - expected hexadecimal
        value = strtoul(argv[3], &endptr, 16);
        if (endptr == argv[3] ||
            *endptr != '\0') {  // Check if the entire string was successfully converted
            rt_kprintf("value data type error (must be hex)\n");
            return;
        }

        mmio_write_32(addr, value);
        rt_kprintf("write 0x%x to 0x%x\n", value, addr);
    }
}
MSH_CMD_EXPORT(devmem, "devmem < addr >");

/**
 * @brief dump memory
 */
void dumpmem(int argc, char **argv)
{
    uint32_t addr, size;
    char    *endptr;

    if (argc != 3) {
        rt_kprintf("usage: %s <addr> <size>\n", __func__);
        return;
    }

    // Parse address (argv[1]) - expected hexadecimal
    addr = strtoul(argv[1], &endptr, 16);
    if (endptr == argv[1] ||
        *endptr != '\0') {  // Check if the entire string was successfully converted
        rt_kprintf("dev_addr data type error (must be hex)\n");
        return;
    }

    size = strtoul(argv[2], &endptr, 16);
    if (endptr == argv[2] ||
        *endptr != '\0') {  // Check if the entire string was successfully converted
        rt_kprintf("dev_size size type error (must be hex)\n");
        return;
    }

    addr = addr & 0xFFFFFFF0;
    for (int i = 0; i < size+0x10; i += 0x10) {
        rt_kprintf("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n", addr + i, 
            mmio_read_32(addr + i), mmio_read_32(addr + i + 0x4), mmio_read_32(addr + i + 0x8), mmio_read_32(addr + i + 0xc));
    }
}
MSH_CMD_EXPORT(dumpmem, "dumpmem < addr > < size >");