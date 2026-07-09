# sample_lsc — LSC (镜头阴影校正) 标定工具

## 1. 概述

`sample_lsc` 是 Sophgo CV184X 平台的 LSC (Lens Shading Correction) 标定示例程序。它通过采集 Bayer Raw 图像帧，计算镜头阴影校正增益表，支持 **在线模式**（从 VI 管道实时采集）和 **离线模式**（读取预存的 Raw 文件）两种工作方式。

标定结果包括：
- **MLSC** (Mesh LSC) 网格校正增益表 — `lsc_gain_*.txt`
- **RLSC** (Radial LSC) 径向校正参数 — `lsc_radius_*.txt`
- **Verify 验证 Raw** — 校正前后的 Raw 图像（可选）

## 2. 编译

```bash
# 在 cvi_mpi 根目录下
cd sample_app/sample_lsc && make clean && make -j

# 产出二进制: sample_lsc_cali
```

依赖：需要完整配置好的交叉编译工具链（`arm-none-linux-musleabihf-`）和 SDK 环境变量。

## 3. 运行模式

### 3.1 在线模式 (Online)

从摄像头 VI 管道实时采集 Raw 帧进行标定。

```bash
# 默认 online 模式
./sample_lsc_cali

# 指定色温
./sample_lsc_cali online [color_temp] [verify]
```

**参数说明：**

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `color_temp` | 色温值（单位: Kelvin） | 5000 |
| `verify` | 是否运行验证: 1=运行, 0=跳过 | 0 |

**示例：**
```bash
# 在线标定，色温 5000K，不运行验证
./sample_lsc_cali online 5000

# 在线标定，色温 3000K，运行验证
./sample_lsc_cali online 3000 1
```

**运行方式：** 程序启动后按任意键执行一次标定，按 `Ctrl+D` 退出。

### 3.2 离线模式 — 单文件 (Offline)

读取单个 Raw 文件进行标定。

```bash
./sample_lsc_cali offline <raw_file> <width> <height> <bayer_id> [color_temp] [verify]
```

**参数说明：**

| 参数 | 说明 |
|------|------|
| `raw_file` | Raw 文件路径（16-bit unpacked Bayer 格式） |
| `width` | 图像宽度 |
| `height` | 图像高度 |
| `bayer_id` | Bayer 排列: 0=RG, 1=GR, 2=GB, 3=BG |
| `color_temp` | 色温值（K），默认 5000 |
| `verify` | 是否运行验证: 1=运行, 0=跳过（默认） |

**示例：**
```bash
./sample_lsc_cali offline frame.raw 2560 1920 3 2800 1
```

### 3.3 离线模式 — 目录遍历 (Offline Dir)

遍历指定目录下所有色温子目录中的 Raw 文件，批量标定。

```bash
./sample_lsc_cali offline_dir <base_dir> [mode] [verify]
```

**参数说明：**

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `base_dir` | 根目录路径，包含多个色温子目录 | — |
| `mode` | 标定模式: 0=MESH_CHROMA_MODE (两遍), 1=MESH_CHROMA_LUMA_MODE (单遍) | 1 |
| `verify` | 是否运行验证: 1=运行, 0=跳过 | 0 |

**目录结构要求：**

```
base_dir/
├── A(2800K)/
│   ├── 2560X1920_BGGR_..._ISO=100_20251210135147.raw
│   └── 2560X1920_BGGR_..._ISO=100_20251210135147.txt
├── D75(7500K)/
│   ├── 2560X1920_BGGR_..._ISO=100_20251210135148.raw
│   └── 2560X1920_BGGR_..._ISO=100_20251210135148.txt
└── ...
```

**命名规范：**
- Raw 文件：`WxH_BayerFormat_..._timestamp.raw`，如 `2560X1920_BGGR_...raw`
- 配套 txt 文件：同名但 `.txt` 后缀，包含 AE/AWB 信息和 BLC 参数
- 色温从子目录名中提取，如 `A(2800K)` → 2800K

**示例：**
```bash
# 批量标定 res/ 目录下所有色温的 Raw 文件，单遍模式，运行验证
./sample_lsc_cali offline_dir res 1 1

# 两遍模式（MLSC 处理色度 + RLSC 处理亮度）
./sample_lsc_cali offline_dir res 0 0
```

## 4. 标定模式详解

### MESH_CHROMA_LUMA_MODE (mode=1, 默认)

单遍标定，MLSC 同时处理色度和亮度分量。
- `calib_flag = 0`
- 验证使用 `isp_algo_lsc_verify_v2`

### MESH_CHROMA_MODE (mode=0)

两遍标定：
1. **色度遍**：`calib_flag = 1`，MLSC 处理色度
2. **亮度遍**：`calib_flag = 2`，RLSC 处理亮度
- 验证使用 `isp_algo_lsc_verify_v1`

## 5. Verify 验证

当 `verify=1` 时，程序在校标完成后执行验证流程：

1. **保存输入 Raw** — 校正前的 `raw_data_unpack` 数据
2. **执行校正** — 按照标定模式调用 v1 或 v2 验证函数
3. **保存输出 Raw** — 校正后的 `lsc_raw_image` 和 `rlsc_raw_image`

**验证函数对照表：**

| 标定模式 | 验证函数 | 说明 |
|----------|----------|------|
| MESH_CHROMA_MODE (0) | `isp_algo_lsc_verify_v1` | cpp 移植版，简单 BLC→MLSC→RLSC 顺序处理 |
| MESH_CHROMA_LUMA_MODE (1) | `isp_algo_lsc_verify_v2` | C 原生版，支持 calib_flag 分支处理 |

**输出文件：**
- `verify_input_raw_*.raw` — 校正前 Raw（16-bit unpacked）
- `lsc_raw_image_*.raw` — MLSC 校正后 Raw
- `rlsc_raw_image_*.raw` — RLSC 校正后 Raw

**在线模式输出路径：** `/mnt/sd/sample_lsc/`
**离线模式输出路径：** 与 Raw 文件同目录

## 6. 输出文件格式

### 增益表文件 (`lsc_gain_*.txt`)

```
# LSC Gain Table (num_knot_x=33, num_knot_y=25)

lsc_r_gain:
1024, 1025, 1026, ..., 1030
1024, 1025, 1026, ..., 1030
...

lsc_g_gain:
...

lsc_b_gain:
...
```

### 半径参数文件 (`lsc_radius_*.txt`)

```
# LSC Radius Parameters
center_x  = 1280
center_y  = 960
radius    = 1500
norm      = 1024

# lsc_radius_gain (129 elements, x4 channels)
1024, 1025, ...,
...
```

## 7. 内存注意事项

Verify 功能需要分配额外的 Raw 图像缓冲区：
- `raw_data_unpack`: width × height × 4 字节 (int)
- `lsc_raw_image`: width × height × 4 字节
- `rlsc_raw_image`: width × height × 4 字节

对于 2560×1920 分辨率，仅 `raw_data_unpack` 就需要约 **20MB**。在内存受限的设备上（< 80MB 可用内存），建议关闭 verify 功能（`verify=0`）。

## 8. 传感器配置

在线模式需要设备上有传感器配置文件：
```
/mnt/data/sensor_cfg.ini
```

该文件定义了 sensor 型号、MIPI 配置、ISP 参数等。
