#include <mmio.h>
#include <rtthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "func.h"
#include "rtdevice.h"

#define NELEMS(x)   (sizeof(x) / sizeof((x)[0]))
#define PINMUX_BASE 0x03001000
#define INVALID_PIN 9999

uint32_t convert_func_to_value(char *pin, char *func)
{
    uint32_t i           = 0;
    uint32_t max_fun_num = NELEMS(mars3_pin_func);
    char     v;

    for (i = 0; i < max_fun_num; i++) {
        if (strcmp(mars3_pin_func[i].func, func) == 0) {
            if (strncmp(mars3_pin_func[i].name, pin, strlen(pin)) == 0) {
                v = mars3_pin_func[i].name[strlen(mars3_pin_func[i].name) - 1];
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
    uint32_t i           = 0;
    uint32_t max_fun_num = NELEMS(mars3_pin_func);
    char     pinname[128];

    sprintf(pinname, "%s%d", name, value);

    printf("%s function:\n", name);
    for (i = 0; i < max_fun_num; i++) {
        if (strncmp(pinname, mars3_pin_func[i].name, strlen(name)) == 0) {
            if (strcmp(pinname, mars3_pin_func[i].name) == 0)
                printf("[v] %s\n", mars3_pin_func[i].func);
            else
                printf("[ ] %s\n", mars3_pin_func[i].func);
            // break;
        }
    }
    printf("\n");
}

void print_usage(void)
{
    printf("cvi_pinmux for Mars\n");
    printf("cvi_pinmux -p          <== List all pins\n");
    printf("cvi_pinmux -l          <== List all pins and its func\n");
    printf("cvi_pinmux -r pin      <== Get func from pin\n");
    printf("cvi_pinmux -w pin/func <== Set func to pin\n");
}

int pin_ctrl(int argc, char *argv[])
{
    char     opt = 0;  // Will store the option character, e.g., 'p', 'l'
    uint32_t i   = 0;
    uint32_t value;
    uint32_t f_val;
    // Variables for command arguments (pin, func, etc.)
    char  pin[32];
    char  func[32];
    char *command_argument_str = NULL;  // Used to store argv[2] (if the option has an argument)

    // Expect at least "cvi_pinmux -X" format
    if (argc < 2) {
        print_usage();
        return -1;
    }

    // Validate and parse the option (e.g., "-l")
    // Option should be in "-X" format (e.g., argv[1] = "-l")
    if (argv[1] == NULL || argv[1][0] != '-' || argv[1][1] == '\0' || argv[1][2] != '\0') {
        printf("Error: Invalid option format. Expected -X (e.g., -l, -p).\n");
        print_usage();
        return -1;
    }
    opt = argv[1][1];  // Get the option character, e.g., 'l' from "-l"

    // Check options that require an argument
    switch (opt) {
    case 'r':  // Get function from pin: -r pin
    case 'w':  // Set function to pin: -w pin/func
        if (argc < 3) {
            printf("Error: Option -%c requires an argument.\n", opt);
            print_usage();
            return -1;
        }
        command_argument_str = argv[2];
        break;
    case 'p':  // List all pins
    case 'l':  // List all pins and their functions
        if (argc > 2) {
            printf("Error: Option -%c does not take an argument.\n", opt);
            print_usage();
            return -1;
        }
        break;
    default:
        printf("Error: Unknown option '-%c'.\n", opt);
        print_usage();
        return -1;
    }

    switch (opt) {
    case 'r':
        // command_argument_str (argv[2]) contains the pin name
        if (command_argument_str == NULL) {  // Should have been caught by the preceding switch
            print_usage();
            return -1;
        }

        for (i = 0; i < NELEMS(mars3_pin); i++) {
            if (strcmp(command_argument_str, mars3_pin[i].name) == 0) break;
        }
        if (i != NELEMS(mars3_pin)) {
            value = mmio_read_32(PINMUX_BASE + mars3_pin[i].offset);
            // printf("value %d\n", value);
            print_fun(command_argument_str, value);

            printf("register: 0x%x\n", PINMUX_BASE + mars3_pin[i].offset);
            printf("value: %d\n", value);
        } else {
            printf("\nInvalid option: %s", command_argument_str);
        }
        break;

    case 'w':
        // command_argument_str (argv[2]) contains "pin/func"
        if (command_argument_str == NULL) {
            print_usage();
            return -1;
        }
        if (sscanf(command_argument_str, "%[^/]/%s", pin, func) != 2) {
            printf("Error: Invalid format for -w. Expected pin/func.\n");
            print_usage();
            return -1;  // Return an error code
        }

        printf("pin %s\n", pin);
        printf("func %s\n", func);

        for (i = 0; i < NELEMS(mars3_pin); i++) {
            if (strcmp(pin, mars3_pin[i].name) == 0) break;
        }

        if (i != NELEMS(mars3_pin)) {
            f_val = convert_func_to_value(pin, func);
            if (f_val == INVALID_PIN) return 1;
            mmio_write_32(PINMUX_BASE + mars3_pin[i].offset, f_val);

            printf("register: %x\n", PINMUX_BASE + mars3_pin[i].offset);
            printf("value: %d\n", f_val);
            // printf("value %d\n", value);
        } else {
            printf("\nInvalid option: %s\n", command_argument_str);
        }
        break;

    case 'p':
        printf("Pinlist:\n");
        for (i = 0; i < NELEMS(mars3_pin); i++) printf("%s\n", mars3_pin[i].name);
        break;

    case 'l':
        for (i = 0; i < NELEMS(mars3_pin); i++) {
            value = mmio_read_32(PINMUX_BASE + mars3_pin[i].offset);
            // printf("value %d\n", value);
            print_fun(mars3_pin[i].name, value);
        }
        break;

    case 'h':
        print_usage();
        return 0;  // Return success after printing usage
    case '?':
        print_usage();
        return -1;  // Return an error code

    default:
        print_usage();
        return -1;  // Return an error code
    }

    return 0;
}
MSH_CMD_EXPORT_ALIAS(pin_ctrl, cvi_pinmux, "Example of pinmux ctrl");
