---
name: dsi-panel-header-addition
description: 这个 skill 只用于新增 PanelSupportList 里的 MIPI DSI 屏参头文件。不适用于 I80、LVDS、BT、RGB 等。
---

# DSI 屏参头文件新增

## 头文件命名规范

新增 DSI 头文件统一使用下面的命名规范，有参数不确定必须让用户提供！！

```text
dsi_{driveric}_{wxh}_{moduleid}_{lanenum}lane_{fps}fps[_vN].h
```

- `driveric`：驱动 IC，小写，例如 `hx8394`。
- `wxh`：有效分辨率，例如 `720x1280`。
- `moduleid`：屏幕模组 ID；不确定时用 `NULL`。
- `lanenum`：物理 DSI lane 数，例如 `4lane`。
- `fps`：目标刷新率，例如 `60fps`。
- `_vN`：可选的小写版本后缀，例如 `_v1`、`_v2`。只有同一个 `driveric + wxh + moduleid + lanenum + fps` 仍然对应不同硬件、lane 序列、timing 或初始化命令时才追加。

示例：

```text
dsi_hx8394_720x1280_NULL_4lane_60fps.h
dsi_hx8394_720x1280_NULL_4lane_60fps_v2.h
```

## DSI 符号命名

新增头文件时，导出的 DSI 符号后缀必须完全跟头文件 stem 一致，去掉 `.h`：

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

这样可以避免 `dev_cfg_hx8394_720x1280` 这类只包含 IC 和分辨率的模糊命名；同 IC、同分辨率但 lane 数、lane 序列、timing 或初始化命令不同的屏不会互相混淆。

## 头文件内容规则

1. 平台适配只包含 `panel_platform.h`。
2. DSI lane 数、lane 序列、`lane_pn_swap` 要在头文件里明确表达。如果单独的 2-lane 或 4-lane 规范头文件更清晰，不要再用通用的 `MIPI_PANEL_2_LANES` 分支隐藏一个新屏变体。
3. `sync_info`、`pixel_clk`、`hs_timing_cfg` 和初始化命令数组必须和文件名代表的实际屏变体一致。
4. timing 参数统一先定义为宏，再在 `sync_info` 中引用；`pixel_clk` 使用 full-stem 派生的函数式宏计算，不直接写裸常量。
5. 现有消费端头文件能接受 `const` 时，新导出的配置数组/结构体优先使用 `const`；否则沿用当前本地兼容写法。
6. 迁移已有头文件时，除命名、`panel_platform.h`、必要的 full-stem 符号规范化外，不改变原始行为。

## 头文件内部格式

新增 DSI 头文件内部的 timing 宏名也从头文件 stem 派生。宏使用大写 full-stem，把 stem 中分辨率的 `x` 转成 `X`，再追加具体后缀：

```text
stem = dsi_hx8394_720x1280_NULL_4lane_60fps
macro_prefix = DSI_HX8394_720X1280_NULL_4LANE_60FPS
```

标准 timing 宏至少包含：

```c
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VACT	1280
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VSA	16
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VBP	4
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_VFP	6
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HACT	720
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HSA	64
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HBP	36
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_HFP	128
#define DSI_HX8394_720X1280_NULL_4LANE_60FPS_FPS	60
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
	.pixel_clk = PIXEL_CLK(DSI_HX8394_720X1280_NULL_4LANE_60FPS),
};
```

结构体初始化时，不需要带上这里列出的注释信息，仅用于帮助添加屏幕时理解参数含义。

## DSI 初始化命令规则

常规 DCS 初始化命令使用 `panels/panel_platform.h` 中定义的 `DSI_CMD()` 和 `DSI_CMD_TYPE()` 宏生成 `struct dsc_instr`，避免同时手写 `.data_type` 和 `.size`。

默认规则按数据字节数选择 data type：1 字节用 `0x05`，2 字节用 `0x15`，3 字节及以上用 `0x29`。第一个参数固定是 delay，单位 ms。如果屏厂明确要求 `0x23`、`0x39` 或其他类型，使用 `DSI_CMD_TYPE()` 显式指定。

`struct dsc_instr` 数组同样使用 full-stem 符号名。常规项优先使用 `DSI_CMD()`；只有屏厂指定 data type 和默认规则不一致时才使用 `DSI_CMD_TYPE()`：

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
- `.data_type`：DSI DCS/Generic 写命令 data type，默认规则见 `panel_platform.h`，最终以屏厂资料为准。
- `.size`：命令数据总字节数，包含第一个寄存器地址和后续数据。只有寄存器地址时为 1；寄存器地址加 1 个数据时为 2；寄存器地址加 2 个数据时为 3，依此类推。
- `.data`：命令数据指针。数组第一个字节必须是寄存器地址，后面才是寄存器数据。

`DSI_CMD()` 只用于没有屏厂特殊说明时的默认 DCS 写命令。这些宏使用 compound literal，要求初始化命令数组定义在文件作用域。

## cvi_panels.h 入口规则

新增 DSI 屏时，在 `cvi_panels.h` 里增加一个 `#elif` block。

- include 的文件名必须指向 `panels/` 下的规范 DSI 头文件。
- `panel_desc` 字段必须引用该头文件里的 full-stem DSI 符号。
- build 生成宏和按头文件名派生的 `CONFIG_` 宏兼容关系集中保留在这里；`CONFIG_` 后缀使用规范头文件 stem 的大写形式，不带 `.h`：

```c
#elif CONFIG_DSI_HX8394_720X1280_NULL_4LANE_60FPS
```

- 如果多个配置宏有意选择同一个规范屏参，把所有别名保留在这个汇总 block 里，不要复制屏参头文件。
- 除非现有消费端确实需要，否则不要把 DSI 兼容宏堆到屏参头文件内部。

## Kconfig.panels 同步

新增接口屏后，必须重新生成 `Kconfig.panels` 使新增屏参可被 Kconfig 选到。

直接在当前目录运行：

```bash
python3 gen_panel_config.py
```

这会更新 `Kconfig.panels`。脚本会自动扫描 `panels/` 下所有头文件，按命名规范提取 stem 并生成 Kconfig `choice` 条目。

如果新屏不在生成结果里，检查头文件命名是否符合 DSI / I80 / LVDS / BT 规范。

## 代码格式化

修改 `.c` 或 `.h` 代码后，必须使用仓库根目录的 `.clang-format` 规则进行格式化：

```bash
clang-format-12 -i path/to/changed.c path/to/changed.h
```

只格式化本次实际修改过的 `.c` / `.h` 文件；不要为了格式化扩大改动范围。注意 `panels/panel_platform.h` 中多行宏区域有 `// clang-format off` / `// clang-format on` 保护，格式化时不要删除或移动这些标记。

## 需要确认的点

下面信息不明确时，先问用户再写代码：

- 实际 DSI lane 数和 lane 序列。
- `moduleid` 是继续用 `NULL`，还是使用真实模组/厂商 ID。
- 这个屏是否需要保留额外的 `MIPI_PANEL_xxx` 别名；`CONFIG_` 宏默认按头文件 stem 大写派生。
- 是否要重命名已有头文件的导出符号，因为这会影响每个直接 include 的消费端。
- 可选后缀应该用 `_v1`、`_v2`，还是硬件 revision 名称。
