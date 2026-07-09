# Sample_vio User Guide

## 示例列表

| Sample                                                 | Usage                | Purpose                          | Res                 |
|--------------------------------------------------------|----------------------|----------------------------------|---------------------|
| VI(Offline) + VPSS + VO (Offline, Rotation)            | `./sample_vio 0`     | Video capture with rotation      | NULL                |
| VI(Offline) + VPSS + VO (Offline, Keep AR)             | `./sample_vio 1`     | Capture video, keep aspect ratio | NULL                |
| VI(Offline, Rotation) + VPSS (Offline, Keep AR) + VO   | `./sample_vio 2`     | Rotate video, keep aspect ratio  | NULL                |
| VI(Offline) + VPSS (Offline, Rotation) + VO            | `./sample_vio 3`     | Process video with rotation      | NULL                |
| VI(Offline, Two devices) + VPSS(Offline) + VO          | `./sample_vio 4`     | Capture from two sensors         | NULL                |
| VI(Offline, Rotation, LDC) +                           | `./sample_vio 5`     | Capture and process video with   | NULL                |

## 注意事项

- 在运行这些示例前，请确保必要的硬件（例如摄像头传感器、显示设备）已正确连接和配置。
- 请参考 CVI SDK 文档以获取 VIO、VI、VPSS 和 VO 模块的详细信息。
- 如需了解示例代码的具体实现，建议查阅 `sample_vio_main.c` 和 `sample_vio.c` 文件。
