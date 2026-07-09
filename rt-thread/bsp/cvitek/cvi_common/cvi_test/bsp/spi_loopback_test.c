#include <drv_spi.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <string.h>

#include "drv_pinmux.h"
#include "hal_debug.h"
#include "hal_def.h"
#include "hal_dma.h"
#include "hal_spi.h"

#define DATA_LEN 128
static int send_finished    = 1;
static int receive_finished = 1;

static void hex_dump(char *buf, int len, int addr)
{
    int  i, j, k, extra_len;
    char binstr[128];

    extra_len = 0;
    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            extra_len = 0;
            extra_len += sprintf(binstr + extra_len, "%08x -", i + addr);
            extra_len += sprintf(binstr + extra_len, " %02x", (unsigned char)buf[i]);
        } else if ((i % 16) == 15) {
            extra_len += sprintf(binstr + extra_len, " %02x", (unsigned char)buf[i]);
            extra_len += sprintf(binstr + extra_len, "  ");
            for (j = i - 15; j <= i; j++)
                extra_len += sprintf(binstr + extra_len, "%c",
                                     ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
            printf("%s\n", binstr);
        } else {
            extra_len += sprintf(binstr + extra_len, " %02x", (unsigned char)buf[i]);
        }
    }

    if ((i % 16) != 0) {
        k = 16 - (i % 16);
        for (j = 0; j < k; j++) extra_len += sprintf(binstr + extra_len, "   ");
        extra_len += sprintf(binstr + extra_len, "  ");
        k = 16 - k;
        for (j = i - k; j < i; j++)
            extra_len +=
                sprintf(binstr + extra_len, "%c", ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
        printf("%s\n", binstr);
    }
}

#ifdef BSP_USING_SPI0
static const char *pinname_whitelist_spi0_sck[] = {
    NULL,
};
static const char *pinname_whitelist_spi0_sdo[] = {
    NULL,
};
static const char *pinname_whitelist_spi0_sdi[] = {
    NULL,
};
static const char *pinname_whitelist_spi0_cs[] = {
    NULL,
};
#endif

#ifdef BSP_USING_SPI1
static const char *pinname_whitelist_spi1_sck[] = {
    "PAD_MIPIRX4N",
    "MUX_SPI1_SCK",
    NULL,
};
static const char *pinname_whitelist_spi1_sdo[] = {
    "PAD_MIPIRX3P",
    "MUX_SPI1_MOSI",
    NULL,
};
static const char *pinname_whitelist_spi1_sdi[] = {
    "PAD_MIPIRX3N",
    "MUX_SPI1_MISO",
    NULL,
};
static const char *pinname_whitelist_spi1_cs[] = {
    "PAD_MIPIRX4P",
    "MUX_SPI1_CS",
    NULL,
};
#endif

#ifdef BSP_USING_SPI2
static const char *pinname_whitelist_spi2_sck[] = {
    "SD1_CLK",
    NULL,
};
static const char *pinname_whitelist_spi2_sdo[] = {
    "SD1_CMD",
    NULL,
};
static const char *pinname_whitelist_spi2_sdi[] = {
    "SD1_D0",
    NULL,
};
static const char *pinname_whitelist_spi2_cs[] = {
    "SD1_D3",
    NULL,
};
#endif

#ifdef BSP_USING_SPI3
static const char *pinname_whitelist_spi3_sck[] = {
    NULL,
};
static const char *pinname_whitelist_spi3_sdo[] = {
    NULL,
};
static const char *pinname_whitelist_spi3_sdi[] = {
    NULL,
};
static const char *pinname_whitelist_spi3_cs[] = {
    NULL,
};
#endif

static bool initialized = false;
static void cvi_spi_pinmux_config()
{
    if (initialized) return;

    initialized = true;

#ifdef BSP_USING_SPI0
    pinmux_config(BSP_SPI0_SCK_PINNAME, SPI0_SCK, pinname_whitelist_spi0_sck);
    pinmux_config(BSP_SPI0_SDO_PINNAME, SPI0_SDO, pinname_whitelist_spi0_sdo);
    pinmux_config(BSP_SPI0_SDI_PINNAME, SPI0_SDI, pinname_whitelist_spi0_sdi);
    pinmux_config(BSP_SPI0_CS_PINNAME, SPI0_CS_X, pinname_whitelist_spi0_cs);
#endif /* BSP_USING_SPI0 */

#ifdef BSP_USING_SPI1
    pinmux_config("PAD_MIPIRX4N", MUX_SPI1_SCK, pinname_whitelist_spi1_sck);
    pinmux_config("PAD_MIPIRX3P", MUX_SPI1_MOSI, pinname_whitelist_spi1_sdo);
    pinmux_config("PAD_MIPIRX3N", MUX_SPI1_MISO, pinname_whitelist_spi1_sdi);
    pinmux_config("PAD_MIPIRX4P", MUX_SPI1_CS, pinname_whitelist_spi1_cs);
    pinmux_config("MUX_SPI1_SCK", SPI1_SCK, pinname_whitelist_spi1_sck);
    pinmux_config("MUX_SPI1_MOSI", SPI1_SDO, pinname_whitelist_spi1_sdo);
    pinmux_config("MUX_SPI1_MISO", SPI1_SDI, pinname_whitelist_spi1_sdi);
    pinmux_config("MUX_SPI1_CS", SPI1_CS_X, pinname_whitelist_spi1_cs);
#endif /* BSP_USING_SPI1 */

#ifdef BSP_USING_SPI2
    pinmux_config(BSP_SPI2_SCK_PINNAME, SPI2_SCK, pinname_whitelist_spi2_sck);
    pinmux_config(BSP_SPI2_SDO_PINNAME, SPI2_SDO, pinname_whitelist_spi2_sdo);
    pinmux_config(BSP_SPI2_SDI_PINNAME, SPI2_SDI, pinname_whitelist_spi2_sdi);
    pinmux_config(BSP_SPI2_CS_PINNAME, SPI2_CS_X, pinname_whitelist_spi2_cs);
#endif /* BSP_USING_SPI2 */

#ifdef BSP_USING_SPI3
    pinmux_config(BSP_SPI3_SCK_PINNAME, SPI3_SCK, pinname_whitelist_spi3_sck);
    pinmux_config(BSP_SPI3_SDO_PINNAME, SPI3_SDO, pinname_whitelist_spi3_sdo);
    pinmux_config(BSP_SPI3_SDI_PINNAME, SPI3_SDI, pinname_whitelist_spi3_sdi);
    pinmux_config(BSP_SPI3_CS_PINNAME, SPI3_CS_X, pinname_whitelist_spi3_cs);
#endif /* BSP_USING_SPI3 */
}

void spi_callback_func(hal_spi_t *spi, hal_spi_event_t event, void *arg)
{
    if (event == SPI_EVENT_RECEIVE_COMPLETE) receive_finished = 0;

    if (event == SPI_EVENT_SEND_COMPLETE) send_finished = 0;
}

int config_spi_transfer(hal_spi_t *spi_handler)
{
    int spi_ret = 0;

    hal_spi_init(spi_handler, 1);

    // test hal_spi_mode
    spi_ret = hal_spi_mode(spi_handler, SPI_MASTER);
    if (spi_ret != HAL_OK) {
        printf("set spi mode failed, actual ret %d\n", spi_ret);
        return spi_ret;
    }

    // test hal_spi_cp_format
    spi_ret = hal_spi_cp_format(spi_handler, SPI_FORMAT_CPOL0_CPHA0);
    if (spi_ret != HAL_OK) {
        printf("set spi format failed returned, actual ret %d\n", spi_ret);
        return spi_ret;
    }

    // test hal_spi_frame_len
    spi_ret = hal_spi_frame_len(spi_handler, SPI_FRAME_LEN_8);
    if (spi_ret != HAL_OK) {
        printf("set spi frame len failed, actual ret %d\n", spi_ret);
        return spi_ret;
    }

    // test hal_spi_baud
    hal_spi_baud(spi_handler, 1000000);

    return spi_ret;
}

int32_t spi_send_receive_async(hal_spi_t *spi, const void *data_out, void *data_in, uint32_t size,
                               uint32_t timeout)
{
    return hal_spi_send_receive_async(spi, data_out, data_in, size);
}

int32_t spi_send_receive_dma(hal_spi_t *spi, const void *data_out, void *data_in, uint32_t size,
                             uint32_t timeout)
{
    return hal_spi_send_receive_dma(spi, data_out, data_in, size);
}

struct spi_test_function {
    char *desc;
    int32_t (*func)(hal_spi_t *spi, const void *data_out, void *data_in, uint32_t size,
                    uint32_t timeout);
};

const struct spi_test_function spi_test_case[6] = {
    {.desc = "poll mode", .func = hal_spi_send_receive},
    {.desc = "irq mode", .func = spi_send_receive_async},
    {.desc = "dma mode", .func = spi_send_receive_dma},
    {.desc = NULL, .func = NULL}};

int spi_loopback_test(int argc, char **argv)
{
    uint8_t   send_buffer[DATA_LEN]    = {0};
    uint8_t   receive_buffer[DATA_LEN] = {0};
    char      op[16]                   = {0};
    int       spi_ret                  = 0;
    hal_spi_t spi_handler;
    int       i;

    const struct spi_test_function *t_case = spi_test_case;

    rt_kprintf("\n--- Starting SPI Driver Test ---\n");
    cvi_spi_pinmux_config();
    for (i = 0; i < DATA_LEN; i++) send_buffer[i] = i;

    printf("--------------------- show src buffer data ------------------------\n");
    hex_dump((char *)send_buffer, DATA_LEN, 0);
    printf("\n");

    for (; t_case->desc; t_case++) {
        printf("######################## start %s test ##############################\n",
               t_case->desc);
        memset(receive_buffer, 0x0, DATA_LEN);
        spi_ret = config_spi_transfer(&spi_handler);
        if (spi_ret) {
            printf("[FAIL] set spi config failed, actual ret %d\n", spi_ret);
            goto un_init;
        }

        if (!strcmp(t_case->desc, "irq mode")) {
            spi_ret = hal_spi_attach_callback(&spi_handler, spi_callback_func, NULL);
            if (spi_ret) {
                printf("[FAIL] set spi config failed, actual ret %d\n", spi_ret);
                goto un_init;
            }
        }

        if (!strcmp(t_case->desc, "dma mode")) {
            spi_ret = hal_spi_attach_callback(&spi_handler, spi_callback_func, NULL);
            if (spi_ret) {
                printf("[FAIL] set spi config failed, actual ret %d\n", spi_ret);
                goto un_init;
            }

            spi_ret = hal_spi_link_dma(&spi_handler, NULL, NULL);
            if (spi_ret) {
                printf("[FAIL] link dma failed!\n");
                goto un_init;
            }
        }

        spi_ret = t_case->func(&spi_handler, send_buffer, receive_buffer, DATA_LEN, 1000);

        if (spi_ret) {
            printf("[FAIL] spi send and receive test failed, actual ret %d\n", spi_ret);
            goto un_init;
        }

        /* only for async such as irq and dma */
        if (!strcmp(t_case->desc, "irq mode") || !strcmp(t_case->desc, "dma mode")) {
            rt_thread_mdelay(100);
            while ((receive_finished || send_finished)) {};
            printf("receive_finished:%d, send_finished:%d\n", receive_finished, send_finished);
        }
        receive_finished = send_finished = 1;

        for (i = 0; i < DATA_LEN; i++) {
            if (send_buffer[i] != receive_buffer[i]) {
                strcpy(op, "failed");
                printf("[FAIL] data is different, at 0x%x pos\n", i);
                break;
            }
            strcpy(op, "success");
        }

        printf("--------------------- show recv buffer data ------------------------\n");
        hex_dump((char *)receive_buffer, DATA_LEN, 0);
        printf("\n######################## %s test %s! ##############################\n",
               t_case->desc, op);
        printf("\n");
        memset(receive_buffer, 0x0, DATA_LEN);
    un_init:
        hal_spi_uninit(&spi_handler);
    }
    rt_kprintf("\n--- END SPI Driver Test ---\n");

    return spi_ret;
}
MSH_CMD_EXPORT(spi_loopback_test, cvi spi loopback test);