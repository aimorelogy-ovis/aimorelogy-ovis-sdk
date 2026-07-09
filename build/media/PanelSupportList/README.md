# PanelSupportList

`PanelSupportList` 是 U-Boot、cvi_mpi 和 AliOS 共用的屏参头文件源仓。这里只保留每个屏参头文件的一份规范副本。

屏参头文件和可复用的公共聚合选择头放在本仓维护。各仓如果还有自己的板级选择逻辑，可以继续保留在各自仓库里。

## 适用范围

- 本仓只维护规范屏参头文件，不维护旧文件名兼容头文件或软链接。
- `cvi_panels.h` 作为 cvi_mpi、U-Boot、AliOS 共用的兼容汇总入口。

## 命名规范

所有新增屏参头文件统一使用：

```text
{if}_{driveric}_{wxh}_{moduleid}[_{lanenum}lane]_{fps}fps[_vN].h
```

- `if`：接口类型，统一小写。例如 `dsi`、`i80`、`lvds`、`bt`、`rgb`。
- `driveric`：驱动 IC，统一小写。例如 `hx8394`、`st7789v`。
- `wxh`：有效分辨率，例如 `720x1280`。
- `moduleid`：屏幕模组 ID。模组名不确定时用 `NULL` 占位。
- `lanenum`：物理 lane 数，例如 `4lane`。只有需要 lane 的接口才保留这段
  （例如 DSI）；I80、LVDS、BT、RGB 等接口可以不带 lane。
- `fps`：目标刷新率，例如 `60fps`。
- `_vN`：可选的小写版本后缀。只有当同一个 `driveric + wxh + moduleid + lanenum + fps`
  仍然对应不同硬件、lane 序列、timing 或初始化参数时才追加。

示例：

```text
dsi_hx8394_720x1280_NULL_4lane_60fps.h
dsi_hx8394_720x1280_NULL_4lane_60fps_v2.h
i80_st7789v_240x320_NULL_60fps_hw.h
lvds_ek79202_1280x800_NULL_60fps.h
bt1120_pt1000k_NULL_NULL_NULL.h
```

### DSI 屏参命名补充

DSI 头文件必须包含完整的 lane 数和 fps 信息，**禁止**使用只包含 IC 和分辨率的模糊命名（例如 `dsi_hx8394_720x1280.h`）。
同 IC、同分辨率但 lane 数、lane 序列、timing 或初始化命令不同的屏，必须能通过文件名区分。

### 非 DSI 屏参命名

I80、LVDS、BT、RGB 等接口也遵循上述通用格式。对缺少 `moduleid`、`lane` 或 `fps`信息的场景，统一用 `NULL` 占位。

示例：

- `bt1120_pt1000k_NULL_NULL_NULL.h`
- `lvds_lcm185x56_NULL_NULL_NULL.h`
- `i80_st7789v_240x320_NULL_60fps_hw.h`
- `i80_st7789v3_240x320_NULL_60fps_hw_mcu.h`

## 符号命名规范

### DSI 符号命名

新增 DSI 头文件时，导出的 DSI 符号后缀必须完全跟头文件 stem 一致（去掉 `.h`）：

```text
stem = dsi_hx8394_720x1280_NULL_4lane_60fps
dev_cfg_{stem}
hs_timing_cfg_{stem}
dsi_init_cmds_{stem}
```

`cvi_panels.h` 也必须引用同一组 full-stem 符号：

```c
#include "panels/dsi_hx8394_720x1280_NULL_4lane_60fps.h"
static struct panel_desc_s panel_desc = {
    .panel_name = "HX8394-720x1280-NULL-4lane-60fps",
    .dev_cfg = &dev_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .hs_timing_cfg = &hs_timing_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds = dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps,
    .dsi_init_cmds_size = ARRAY_SIZE(dsi_init_cmds_dsi_hx8394_720x1280_NULL_4lane_60fps),
};
```

### 非 DSI 符号命名

非 DSI 接口的导出符号沿用各自现有的命名习惯，保持与头文件 stem 的对应关系即可。

## 头文件内容规则

### 平台适配

所有屏参头文件统一包含 `panel_platform.h`，不要直接包含平台头文件。
这个适配层在 U-Boot 下映射到 `<cvi_mipi.h>`，在 cvi_mpi / AliOS 下映射到
`<cvi_type.h>` 和 `<cvi_comm_mipi_tx.h>`。

### DSI 头文件内容规则

1. DSI lane 数、lane 序列、`lane_pn_swap` 要在头文件里明确表达。如果单独的
   2-lane 或 4-lane 规范头文件更清晰，不要再用通用的 `MIPI_PANEL_2_LANES`
   分支隐藏一个新屏变体。
2. `sync_info`、`pixel_clk`、`hs_timing_cfg` 和初始化命令数组必须和文件名代表
   的实际屏变体一致。
3. timing 参数统一先定义为宏，再在 `sync_info` 中引用；`pixel_clk` 使用
   full-stem 派生的函数式宏计算，不直接写裸常量。
4. 现有消费端头文件能接受 `const` 时，新导出的配置数组/结构体优先使用 `const`；
   否则沿用当前本地兼容写法。
5. 迁移已有头文件时，除命名、`panel_platform.h`、必要的 full-stem 符号规范化
   外，不改变原始行为。

#### DSI Timing 宏

新增 DSI 头文件内部的 timing 宏名从头文件 stem 派生。宏使用大写 full-stem，
把 stem 中分辨率的 `x` 转成 `X`，再追加具体后缀：

```text
stem = dsi_hx8394_720x1280_NULL_4lane_60fps
macro_prefix = DSI_HX8394_720X1280_NULL_4LANE_60FPS
```

标准 timing 宏至少包含：

```c
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VACT   1280
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VSA    16
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VBP    4
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VFP    6
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HACT   720
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HSA    64
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HBP    36
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HFP    128
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_FPS    60
```

`pixel_clk` 用 full-stem 派生的函数式宏计算：

```c
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_PIXEL_CLK(x) \
    ((x##_VACT + x##_VSA + x##_VBP + x##_VFP) \
     * (x##_HACT + x##_HSA + x##_HBP + x##_HFP) * x##_FPS / 1000)
```

`struct combo_dev_cfg_s` 里引用这些宏：

```c
struct combo_dev_cfg_s dev_cfg_dsi_hx8394_720x1280_NULL_4lane_60fps = {
    /* MIPI TX 设备号，通常主屏使用 0。 */
    .devno = 0,
    /* DSI 物理 lane 映射顺序，依次填写 data lane 和 clock lane。 */
    .lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK,
        MIPI_TX_LANE_2, MIPI_TX_LANE_3},
    /* 每条 lane 的 P/N 是否交换，顺序必须和 lane_id 对齐。 */
    .lane_pn_swap = {true, true, true, true, true},
    /* 输出模式，DSI 视频屏固定使用 DSI video mode。 */
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    /* DSI video mode 子模式，按屏参要求选择 burst 或 sync pulse/event。 */
    .video_mode = BURST_MODE,
    /* DSI 输出像素格式，必须和屏端初始化及 VO 输出格式匹配。 */
    .output_format = OUT_FORMAT_RGB_24_BIT,
    /* DSI video timing，所有数值来自同一组 full-stem timing 宏。 */
    .sync_info = {
        /* HSYNC 有效宽度，单位 pixel clock。 */
        .vid_hsa_pixels = DSI_HX8394_720X1280_NULL_4LANE_60FPS_HSA,
        /* 水平后肩，单位 pixel clock。 */
        .vid_hbp_pixels = DSI_HX8394_720X1280_NULL_4LANE_60FPS_HBP,
        /* 水平前肩，单位 pixel clock。 */
        .vid_hfp_pixels = DSI_HX8394_720X1280_NULL_4LANE_60FPS_HFP,
        /* 水平有效显示宽度，单位 pixel。 */
        .vid_hline_pixels = DSI_HX8394_720X1280_NULL_4LANE_60FPS_HACT,
        /* VSYNC 有效宽度，单位 line。 */
        .vid_vsa_lines = DSI_HX8394_720X1280_NULL_4LANE_60FPS_VSA,
        /* 垂直后肩，单位 line。 */
        .vid_vbp_lines = DSI_HX8394_720X1280_NULL_4LANE_60FPS_VBP,
        /* 垂直前肩，单位 line。 */
        .vid_vfp_lines = DSI_HX8394_720X1280_NULL_4LANE_60FPS_VFP,
        /* 垂直有效显示高度，单位 line。 */
        .vid_active_lines = DSI_HX8394_720X1280_NULL_4LANE_60FPS_VACT,
        /* VSYNC 极性，按屏参手册或原始屏参保持。 */
        .vid_vsa_pos_polarity = false,
        /* HSYNC 极性，按屏参手册或原始屏参保持。 */
        .vid_hsa_pos_polarity = true,
    },
    /* pixel clock，使用同一组 timing 宏和 FPS 计算得到，单位 kHz。 */
    .pixel_clk = DSI_HX8394_720X1280_NULL_4LANE_60FPS_PIXEL_CLK(
        DSI_HX8394_720X1280_NULL_4LANE_60FPS),
};
```

结构体初始化时，关键字段或不直观取值需要说明；嵌套成员也按需说明单位、含义或取值依据。
说明优先写在成员上一行，不在成员之间额外加空行，避免行尾注释和长宏名互相挤压。

#### DSI 初始化命令规则

常规 DCS 初始化命令用轻量宏生成 `struct dsc_instr`，避免同时手写
`.data_type` 和 `.size`。默认规则按数据字节数选择 data type：1 字节用 `0x05`，
2 字节用 `0x15`，3 字节及以上用 `0x29`。`DSI_CMD()` 和 `DSI_CMD_TYPE()` 的
第一个参数固定是 delay，单位 ms；第一行注释必须写清楚这个约定。如果屏厂明确
要求 `0x23`、`0x39` 或其他类型，使用显式宏覆盖：

```c
/* First argument of DSI_CMD/DSI_CMD_TYPE is delay in ms after sending. */
#define DSI_DCS_DATA_TYPE_BY_SIZE(size) \
    ((size) == 1 ? 0x05 : ((size) == 2 ? 0x15 : 0x29))
#define DSI_CMD(delay_ms, ...) \
    {.delay = (delay_ms), \
     .data_type = DSI_DCS_DATA_TYPE_BY_SIZE(sizeof((CVI_U8[]){__VA_ARGS__})), \
     .size = sizeof((CVI_U8[]){__VA_ARGS__}), \
     .data = (CVI_U8[]){__VA_ARGS__} }
#define DSI_CMD_TYPE(delay_ms, type, ...) \
    {.delay = (delay_ms), .data_type = (type), \
     .size = sizeof((CVI_U8[]){__VA_ARGS__}), \
     .data = (CVI_U8[]){__VA_ARGS__} }
```

`struct dsc_instr` 数组同样使用 full-stem 符号名。常规项优先使用 `DSI_CMD()`；
只有屏厂指定 data type 和默认规则不一致时才使用 `DSI_CMD_TYPE()`：

```c
struct dsc_instr dsi_init_cmds_dsi_st7701_480x640_NULL_2lane_60fps[] = {
    DSI_CMD(120, 0x11),
    DSI_CMD(20, 0x29),
    DSI_CMD(0, 0x35, 0x00),
    DSI_CMD_TYPE(0, 0x39, 0xff, 0x77, 0x01, 0x00),
};
```

`struct dsc_instr` 字段含义：

- `.delay`：发送完该命令后的延时，单位 ms。
- `.data_type`：DSI DCS/Generic 写命令 data type，默认规则见上方宏定义，
  最终以屏厂资料为准。
- `.size`：命令数据总字节数，包含第一个寄存器地址和后续数据。只有寄存器地址
  时为 1；寄存器地址加 1 个数据时为 2；依此类推。
- `.data`：命令数据指针。数组第一个字节必须是寄存器地址，后面才是寄存器数据。

`DSI_CMD()` 只用于没有屏厂特殊说明时的默认 DCS 写命令。这些宏使用 compound
literal，要求初始化命令数组定义在文件作用域，确保 `.data` 指向的数据具备静态
生命周期。

### 非 DSI 头文件内容规则

I80、LVDS、BT、RGB 等接口的屏参头文件沿用各自现有格式，保持以下原则：

1. 平台适配只包含 `panel_platform.h`。
2. 导出的配置数据尽量使用 `const`（只要现有消费端能接受）。
3. 迁移时只改命名和必要的规范化，不改变原始行为。

## `cvi_panels.h` 入口规则

`cvi_panels.h` 是公共聚合选择头，内部使用 `panels/` 下规范命名的屏参头文件。

### 新增 DSI 屏

在 `cvi_panels.h` 里增加一个 `#elif` block：

- include 的文件名必须指向 `panels/` 下的规范 DSI 头文件。
- `panel_desc` 字段必须引用该头文件里的 full-stem DSI 符号。
- 选择条件只使用按头文件名派生的 `CONFIG_` 宏；
  `CONFIG_` 后缀使用规范头文件 stem 的大写形式，不带 `.h`：

```c
#elif CONFIG_DSI_HX8394_720X1280_NULL_4LANE_60FPS
```

- 不再维护 `MIPI_PANEL_xxx` 或 `CONFIG_PANEL_xxx` 旧别名。

### 新增非 DSI 屏

同样使用 `#elif` block，选择条件只使用对应规范头文件 stem 派生出的
`CONFIG_` 宏。

## build config 生成

生成脚本直接扫描 `panels/*.h`，使用屏参头文件名去掉 `.h` 后的 stem 作为
Kconfig 项。

- 例如 `panels/dsi_st7703_640x480_NULL_2lane_60fps.h` 会生成
  `CONFIG_DSI_ST7703_640X480_NULL_2LANE_60FPS`。
- `panel_platform.h` 是平台适配头，不参与 build config 生成。
- 新增屏参如果要通过公共选择头使用，需要在 `cvi_panels.h` 中补充同名
  `CONFIG_` 条件。

## 消费规则

消费仓保持原有的 include 目录结构，但不在 `PanelSupportList` 内维护旧名
兼容软链接。新增或迁移屏参时，消费仓应直接引用 `panels/` 下的规范命名
头文件。

新增屏参后，如果需要被公共选择头使用，也要同步更新 `cvi_panels.h`。

## 消费端检查

新增或迁移屏参头文件后：

1. 在 `cvi_mpi`、`u-boot-2021.10`、`cvi_alios` 里搜索旧的直接 include。
2. 确认消费端要么直接 include 规范头文件，要么走共用的 `cvi_panels.h` 汇总入口。
3. 如果修改了 `panels/` 目录或 `cvi_panels.h`，重新生成 panel config：

   ```bash
   python3 build/scripts/gen_panel_config.py
   ```

4. 条件允许时，对被影响的消费端做构建或编译检查。

## 需要确认的点

下面信息不明确时，先确认再写代码：

- 实际 DSI lane 数和 lane 序列。
- `moduleid` 是继续用 `NULL`，还是使用真实模组/厂商 ID。
- 是否要重命名已有头文件的导出符号，因为这会影响每个直接 include 的消费端。
- 可选后缀应该用 `_v1`、`_v2`，还是硬件 revision 名称。
