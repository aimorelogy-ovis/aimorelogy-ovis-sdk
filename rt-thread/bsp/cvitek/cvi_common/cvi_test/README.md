# Cvitek 通用测试及工具集

本目录包含用于 Cvitek SoC 的通用测试程序和命令行工具，已集成到 RT-Thread 操作系统中。这些工具的编译受 Kconfig 选项控制。

## 目录结构

```
cvi_test/
├── bsp/                  # BSP自测试程序的源文件
|   ├── gpio_test.c       # GPIO 测试
|   ├── spi_loopback_test.c # SPI 回环测试
|   └── tpu_tdma_test.c     # TPU TDMA 测试
|
└── SConscript             # 本模块的构建脚本
```

## 功能特性

### 自测试程序

这些测试程序在使能 `BSP_USING_SELFTEST` 以及特定的测试用例选项后会被编译。

1.  **GPIO 测试 (`bsp/gpio_test.c`)**
    *   执行 GPIO 功能测试。
    *   **Kconfig 选项:** `BSP_USING_SELFTEST` 和 `BSP_TEST_GPIO`。

2.  **SPI 回环测试 (`bsp/spi_loopback_test.c`)**
    *   在 SPI 接口上执行回环测试。
    *   **Kconfig 选项:** `BSP_USING_SELFTEST` 和 `BSP_TEST_SPI`。

3.  **TPU TDMA 测试 (`bsp/tpu_tdma_test.c`)**
    *   使用 TDMA 测试 TPU 功能。
    *   **Kconfig 选项:** `BSP_USING_SELFTEST` 和 `BSP_TEST_TPU`。

## 构建说明

本目录中的组件作为 RT-Thread 主构建过程的一部分进行构建。请通过 Kconfig (例如，使用 `menuconfig`) 来使能所需的功能。

1.  根据需要在 `ENABLE SELFTEST`(`BSP_USING_SELFTEST`) 部分下使能特定的测试case。例如：
    *   要使能 GPIO 测试：使能 `Enable GPIO_SELFTEST` (`BSP_TEST_GPIO`)。
    *   要使能 SPI 测试：使能 `Enable SPI_SELFTEST` (`BSP_TEST_SPI`)。

## 使用方法

编译并烧录到目标设备后，可以通过 MSH shell 调用这些工具。例如：

```msh
msh />gpio_test
msh />spi_loopback_test
```