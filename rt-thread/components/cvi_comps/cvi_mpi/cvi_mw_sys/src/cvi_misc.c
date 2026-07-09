#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <queue.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmio.h"
#include <unistd.h>

#include "common.h"
#include "cvi_efuse.h"
#include "cvi_errno.h"
#include "cvi_mw_base.h"
#include "cvi_sys.h"
#include "sys_ioctl.h"

#include <cvi_base.h>

#define CVI_EFUSE_CUSTOMER_ADDR 0x1

CVI_S32 CVI_EFUSE_EnableFastBoot(void)
{
    csi_error_t ret = CSI_OK;
    CVI_U32 value = 0;
    CVI_U32 chip = 0;

    CVI_SYS_GetChipId(&chip);
    if ((!IS_CHIP_CV181X(chip)) && (!IS_CHIP_CV180X(chip))) {
        printf("chip id: %d\n", chip);
        return CVI_FAILURE;
    }

    ret = cvi_efuse_pwr_on();
    if (ret != CSI_OK) {
        printf("efuse power on failed\n");
        return CVI_FAILURE;
    }

    ret = cvi_efuse_sd_dl_config(SD_USB_UART_DL_FASTBOOT);
    ret |= cvi_efuse_usb_dl_config(SD_USB_UART_DL_FASTBOOT);
    ret |= cvi_efuse_uart_dl_config(SD_USB_UART_DL_FASTBOOT);
    if (ret != CSI_OK) {
        printf("config sd/usb/uart dl to fastboot failed\n");
    }

    if (IS_CHIP_PKG_TYPE_QFN(chip))
        value = 0x1E1E64; // AUX0
    else
        value = 0x1; // USB_ID

    ret = cvi_efuse_program_word(CVI_EFUSE_CUSTOMER_ADDR, value);
    if (ret != CSI_OK) {
        printf("efuse program customer failed\n");
        cvi_efuse_pwr_off();
        return CVI_FAILURE;
    }

    cvi_efuse_pwr_off();
    return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_IsFastBootEnabled(void)
{
    csi_error_t ret = CSI_OK;
    CVI_U32 value = 0;
    CVI_U32 chip = 0;
    CVI_U32 sd_dl = 0, usb_dl = 0, uart_dl = 0;

    sd_dl = cvi_efuse_get_sd_dl_config();
    usb_dl = cvi_efuse_get_usb_dl_config();
    uart_dl = cvi_efuse_get_uart_dl_config();
    if ((sd_dl != SD_USB_UART_DL_FASTBOOT) && (usb_dl != SD_USB_UART_DL_FASTBOOT) && (uart_dl != SD_USB_UART_DL_FASTBOOT)) {
        return CVI_FAILURE;
    }

    ret = cvi_efuse_read_word_from_shadow(CVI_EFUSE_CUSTOMER_ADDR, &value);
    CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
    if (ret < 0)
        return ret;

    CVI_SYS_GetChipId(&chip);
    if (IS_CHIP_CV181X(chip) || IS_CHIP_CV180X(chip)) {
        if (IS_CHIP_PKG_TYPE_QFN(chip)) {
            if (value == 0x1E1E64) // AUX0
                return CVI_SUCCESS;
        } else {
            if (value == 0x1) // USB_ID
                return CVI_SUCCESS;
        }

        return CVI_FAILURE;
    } else
        return CVI_FAILURE;
}

CVI_S32 CVI_EFUSE_BootFreqHigher(void)
{
    CVI_U32 writeData = 0x800; // Bonding0[10] Bonding0[11] 10 = CPU 750M
    CVI_U32 addr = 0x28;
    CVI_U8 buf[4];
    CVI_U32 ret = -1;

    memcpy(buf, &writeData, 4);
    ret = cvi_efuse_program_word((addr) / 4, *(CVI_U32 *)(buf));
    if (ret == CVI_SUCCESS) {
        return CVI_SUCCESS;
    } else {
        return CVI_FAILURE;
    }
}

CVI_S32 CVI_EFUSE_IsBootFreqHigher(void)
{
    CVI_U8 read_buf[16];
    CVI_U32 addr = 0x20;
    CVI_S32 ret = 0;
    CVI_U32 data;
    int i;

    memset(read_buf, 0, 16);
    for (i = 0; i < 16; i += 4) {
        if (addr >= 0x100)
            return CVI_FAILURE_ILLEGAL_PARAM;

        if (addr % 4 != 0)
            return CVI_FAILURE_ILLEGAL_PARAM;

        data = mmio_read_32(0x020C0100 + addr + i);
        memcpy(read_buf + i, &data, sizeof(data));
        ret = CVI_SUCCESS;
    }
    if (ret >= 0) {
        if (read_buf[9] & (1 << 3)) {
            printf("CPU_Boot_Freq is 750M\n");
            return CVI_SUCCESS;
        } else {
            return CVI_FAILURE;
        }
    } else {
        return CVI_FAILURE;
    }
}

#if (CONFIG_EFUSE_TEST == 1)

void cvi_efuse_program(int32_t argc, char **argv)
{
    uint32_t addr, data;
    char *ptr;
    long value = -1;
    csi_error_t ret = CSI_OK;

    if (argc != 3) {
        printf("invailed param! \n usage: efusew addr data\n");
        return;
    }

    value = strtol(argv[1], &ptr, 16);
    if (value < 0) {
        printf("invailed addr! \n");
        return;
    }

    addr = (uint32_t)value;
    data = strtol(argv[2], &ptr, 16);

    ret = cvi_efuse_pwr_on();
    if (ret != CSI_OK) {
        printf("cvi_efuse_pwr_on failed\n");
        return;
    }

    ret = cvi_efuse_program_word(addr, data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_program_word failed\n");
    }

    cvi_efuse_pwr_off();
}

MSH_CMD_EXPORT_ALIAS(cvi_efuse_program, efusew, efuse write);

#endif
