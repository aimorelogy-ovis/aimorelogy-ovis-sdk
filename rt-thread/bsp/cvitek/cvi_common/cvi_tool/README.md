# Cvitek 通用测试及工具集

本目录包含用于 Cvitek SoC 的通用测试程序和命令行工具，已集成到 RT-Thread 操作系统中。这些工具的编译受 Kconfig 选项控制。

## 目录结构

```
cvi_tool/                 # 命令行工具的源文件
├── devmem_tool.c      # 内存读写工具 (devmem)
├── i2c_tool.c         # I2C 工具 (i2cdetect, i2cget, i2cset)
|── cvi_pinmux/        # Pinmux (引脚复用) 配置工具
|   ├── chip_cv180x/
|   │   └── cvi_pinmux.c # 适用于 CV180x 系列的 Pinmux 工具实现
|   └── chip_cv181x/
|       └── cvi_pinmux.c # 适用于 CV181x 系列的 Pinmux 工具实现
|
└── SConscript             # 本模块的构建脚本
```

## 功能特性

### 命令行工具

这些工具在 Kconfig 中使能 `BSP_USING_DRIVER_TOOL` 后会被编译。它们提供了用于硬件交互和调试的 MSH shell 命令。

1.  **设备内存访问 (`devmem_tool.c`)**
    *   提供 `devmem` 命令，用于读取或写入物理内存地址。
    *   **Kconfig 选项:** `BSP_USING_DRIVER_TOOL`

2.  **Pinmux 工具 (`tools/cvi_pinmux/`)**
    *   提供 `cvi_pinmux` 命令，用于配置和查询引脚功能。
    *   根据 SoC 型号选择特定芯片的实现：
        *   `SOC_TYPE_CV180XB_QFN`: 编译 `tools/cvi_pinmux/chip_cv180x/cvi_pinmux.c`
        *   `SOC_TYPE_CV181XC_QFN` 或 `SOC_TYPE_CV181XH_BGA`: 编译 `tools/cvi_pinmux/chip_cv181x/cvi_pinmux.c`
    *   **Kconfig 选项:** `BSP_USING_DRIVER_TOOL` 和 `BSP_USING_PINMUX`，以及相关的 `SOC_TYPE_*` 选项。

3.  **I2C 工具 (`tools/i2c_tool.c`)**
    *   提供 `i2cdetect`, `i2cget`, `i2cset` 命令，用于 I2C 总线操作。
    *   **Kconfig 选项:** `BSP_USING_DRIVER_TOOL` 和 `BSP_USING_I2C_TOOL`。

## 构建说明

本目录中的组件作为 RT-Thread 主构建过程的一部分进行构建。请通过 Kconfig (例如，使用 `menuconfig`) 来使能所需的功能。

1.  使能通用驱动工具：
    *   `Project Configuration` -> `Cvitek Drivers Configuration` -> `ENABLE DRIVER_TOOLS`
2.  根据需要在 `ENABLE DRIVER_TOOLS`(`BSP_USING_DRIVER_TOOL`) 下使能特定的工具。例如：
    *   要使能 cvi_pinmux 工具：使能 `Enable CVI_PINMUX` (`BSP_USING_PINMUX`)。
    *   要使能 I2C 工具：使能 `Enable I2C_TOOL` (`BSP_USING_I2C_TOOL`)。

## 使用方法

编译并烧录到目标设备后，可以通过 MSH shell 调用这些工具。例如：

```msh
msh />devmem 0x03001000
msh />cvi_pinmux -r SD0_CLK
msh />i2cdetect 0
```

有关详细的使用说明，请参考各个命令的帮助信息。
```msh
msh />devmem
msh />cvi_pinmux
msh />i2cdetect
```