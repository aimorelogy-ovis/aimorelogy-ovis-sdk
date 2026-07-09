#include "devmem.h"
#include "func.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))
#define PINMUX_BASE 0x03001000
#define INVALID_PIN 9999

struct pinlist {
	char name[16];
	uint32_t offset;
} pinlist_st;

struct pinlist cv183x_pin[] = {
	{"JTAG_CPU_TCK", 0x0},
	{"SDIO0_CD", 0x4},
	{"RSTN", 0x8},
	{"JTAG_CPU_TRST", 0xc},
	{"UART0_RX", 0x10},
	{"SDIO0_PWR_EN", 0x14},
	{"UART0_TX", 0x18},
	{"JTAG_CPU_TMS", 0x1c},
	{"EMMC_CLK", 0x20},
	{"EMMC_RSTN", 0x24},
	{"EMMC_CMD", 0x28},
	{"EMMC_DAT1", 0x2c},
	{"EMMC_DAT0", 0x30},
	{"EMMC_DAT2", 0x34},
	{"EMMC_DAT3", 0x38},
	{"SDIO0_CMD", 0x3c},
	{"SDIO0_CLK", 0x40},
	{"SDIO0_D0", 0x44},
	{"SDIO0_D1", 0x48},
	{"SDIO0_D2", 0x4c},
	{"SDIO0_D3", 0x50},
	{"XGPIO_A_20", 0x54},
	{"XGPIO_A_21", 0x58},
	{"XGPIO_A_22", 0x5c},
	{"XGPIO_A_23", 0x60},
	{"XGPIO_A_24", 0x64},
	{"XGPIO_A_25", 0x68},
	{"XGPIO_A_26", 0x6c},
	{"XGPIO_A_27", 0x70},
	{"XGPIO_A_28", 0x74},
	{"XGPIO_A_29", 0x78},
	{"RTC_MODE", 0x7c},
	{"PWR_WAKEUP0", 0x80},
	{"PWR_BUTTON1", 0x84},
	{"PWR_WAKEUP1", 0x88},
	{"PWR_BUTTON0", 0x8c},
	{"PWR_VBAT_DET", 0x90},
	{"PWR_ON", 0x94},
	{"PWM2", 0x98},
	{"PWM0_BUCK", 0x9c},
	{"PWM3", 0xa0},
	{"PWM1", 0xa4},
	{"SPI0_SDI", 0xa8},
	{"SPI0_SDO", 0xac},
	{"SPI0_SCK", 0xb0},
	{"SPI0_CS_X", 0xb4},
	{"IIC2_SCL", 0xb8},
	{"IIC1_SCL", 0xbc},
	{"IIC1_SDA", 0xc0},
	{"UART2_TX", 0xc4},
	{"IIC2_SDA", 0xc8},
	{"UART1_RTS", 0xcc},
	{"UART2_RTS", 0xd0},
	{"UART2_RX", 0xd4},
	{"UART1_TX", 0xd8},
	{"UART1_CTS", 0xdc},
	{"BOOT_MS", 0xe0},
	{"UART2_CTS", 0xe4},
	{"ADC1", 0xe8},
	{"UART1_RX", 0xec},
	{"USB_ID", 0xf0},
	{"USB_VBUS_DET", 0xf4},
	{"USB_VBUS_EN", 0xf8},
	{"CLK32K", 0xfc},
	{"CLK25M", 0x100},
	{"XTAL_XIN_XI", 0x104},
	{"VO_DATA1", 0x108},
	{"VO_DATA0", 0x10c},
	{"PAD_MIPI_TXM4", 0x110},
	{"PAD_MIPI_TXP4", 0x114},
	{"PAD_MIPI_TXM3", 0x118},
	{"PAD_MIPI_TXP3", 0x11c},
	{"PAD_MIPI_TXM2", 0x120},
	{"PAD_MIPI_TXP2", 0x124},
	{"PAD_MIPI_TXM1", 0x128},
	{"PAD_MIPI_TXP1", 0x12c},
	{"PAD_MIPI_TXM0", 0x130},
	{"PAD_MIPI_TXP0", 0x134},
	{"MIPIRX1_PAD0P", 0x138},
	{"MIPIRX1_PAD0N", 0x13c},
	{"MIPIRX1_PAD1P", 0x140},
	{"MIPIRX1_PAD1N", 0x144},
	{"MIPIRX1_PAD2P", 0x148},
	{"MIPIRX1_PAD2N", 0x14c},
	{"MIPIRX1_PAD3P", 0x150},
	{"MIPIRX1_PAD3N", 0x154},
	{"MIPIRX1_PAD4P", 0x158},
	{"MIPIRX1_PAD4N", 0x15c},
	{"MIPIRX0_PAD4N", 0x160},
	{"MIPIRX0_PAD4P", 0x164},
	{"MIPIRX0_PAD3N", 0x168},
	{"MIPIRX0_PAD3P", 0x16c},
	{"MIPIRX0_PAD2N", 0x170},
	{"MIPIRX0_PAD2P", 0x174},
	{"MIPIRX0_PAD1N", 0x178},
	{"MIPIRX0_PAD1P", 0x17c},
	{"MIPIRX0_PAD0N", 0x180},
	{"MIPIRX0_PAD0P", 0x184},
	{"VI_DATA22", 0x188},
	{"VI_DATA20", 0x18c},
	{"VI_DATA21", 0x190},
	{"CAM_PD0", 0x194},
	{"VI_DATA19", 0x198},
	{"IIC0_SDA", 0x19c},
	{"CAM_MCLK0", 0x1a0},
	{"IIC3_SCL", 0x1a4},
	{"VI_DATA24", 0x1a8},
	{"CAM_RST0", 0x1ac},
	{"VI_DATA23", 0x1b0},
	{"IIC3_SDA", 0x1b4},
	{"IIC0_SCL", 0x1b8},
};

uint32_t convert_func_to_value(char *pin, char *func)
{
	uint32_t i = 0;
	uint32_t max_fun_num = NELEMS(cv183x_pin_func);
	char v;

	for (i = 0; i < max_fun_num; i++) {
		if (strcmp(cv183x_pin_func[i].func, func) == 0) {
			if (strncmp(cv183x_pin_func[i].name, pin, strlen(pin)) == 0) {
				v = cv183x_pin_func[i].name[strlen(cv183x_pin_func[i].name) - 1];
				break;
			}
		}
	}

	if (i == max_fun_num) {
		printf("ERROR: invalid pin or func\n");
		return INVALID_PIN;
	}

	return (v - 0x30);
}

void print_fun(char *name, uint32_t value)
{
	uint32_t i = 0;
	uint32_t max_fun_num = NELEMS(cv183x_pin_func);
	char pinname[128];

	sprintf(pinname, "%s%d", name, value);

	printf("%s function:\n", name);
	for (i = 0; i < max_fun_num; i++) {
		if (strncmp(pinname, cv183x_pin_func[i].name, strlen(name)) == 0) {
			if (strcmp(pinname, cv183x_pin_func[i].name) == 0)
				printf("[v] %s\n", cv183x_pin_func[i].func);
			else
				printf("[ ] %s\n", cv183x_pin_func[i].func);
			// break;
		}
	}
	printf("\n");
}

void print_usage(void)
{
	printf("./cvi_pinmux -p          <== List all pins\n");
	printf("./cvi_pinmux -l          <== List all pins and its func\n");
	printf("./cvi_pinmux -r pin      <== Get func from pin\n");
	printf("./cvi_pinmux -w pin/func <== Set func to pin\n");
}

int main(int argc, char *argv[])
{
	int opt = 0;
	uint32_t i = 0;
	uint32_t value;

	if (argc == 1) {
		print_usage();
		return 1;
	}

	while ((opt = getopt(argc, argv, "hplr:w:")) != -1) {
		switch (opt) {
		case 'r':
			for (i = 0; i < NELEMS(cv183x_pin); i++) {
				if (strcmp(optarg, cv183x_pin[i].name) == 0)
					break;
			}
			if (i != NELEMS(cv183x_pin)) {
				value = devmem_readl(PINMUX_BASE + cv183x_pin[i].offset);
				// printf("value %d\n", value);
				print_fun(optarg, value);

				printf("register: 0x%x\n", PINMUX_BASE + cv183x_pin[i].offset);
				printf("value: %d\n", value);
			} else {
				printf("\nInvalid option: %s", optarg);
			}
			break;

		case 'w':
			char pin[32];
			char func[32];
			uint32_t f_val;

			// printf("optarg %s\n", optarg);
			if (sscanf(optarg, "%[^/]/%s", pin, func) != 2)
				print_usage();

			printf("pin %s\n", pin);
			printf("func %s\n", func);

			for (i = 0; i < NELEMS(cv183x_pin); i++) {
				if (strcmp(pin, cv183x_pin[i].name) == 0)
					break;
			}

			if (i != NELEMS(cv183x_pin)) {
				f_val = convert_func_to_value(pin, func);
				if (f_val == INVALID_PIN)
					return 1;
				devmem_writel(PINMUX_BASE + cv183x_pin[i].offset, f_val);

				printf("register: %x\n", PINMUX_BASE + cv183x_pin[i].offset);
				printf("value: %d\n", f_val);
				// printf("value %d\n", value);
			} else {
				printf("\nInvalid option: %s\n", optarg);
			}
			break;

		case 'p':
			printf("Pinlist:\n");
			for (i = 0; i < NELEMS(cv183x_pin); i++)
				printf("%s\n", cv183x_pin[i].name);
			break;

		case 'l':
			for (i = 0; i < NELEMS(cv183x_pin); i++) {
				value = devmem_readl(PINMUX_BASE + cv183x_pin[i].offset);
				// printf("value %d\n", value);
				print_fun(cv183x_pin[i].name, value);
			}
			break;

		case 'h':
			print_usage();
			break;

		case '?':
			print_usage();
			break;

		default:
			print_usage();
			break;
		}
	}

	return 0;
}
