# OVIS Manager

`ovis-managerd` 是独立于 `ipcamera` 的无界面设备管理进程，提供设备识别、服务控制、运行配置持久化和版本查询接口。管理网页由 GitHub Pages 独立托管，板端不再安装静态网页资源。

## 目标文件

- `/usr/sbin/ovis-managerd`
- `/usr/sbin/ovis-device-identity`
- `/etc/init.d/S98ovis-manager`
- `/mnt/cfg/ovis-manager/device-id`
- `/mnt/cfg/ovis-manager/ncm-subnet`
- `/mnt/cfg/ovis-manager/ovis.account`
- `/mnt/cfg/ipcamera/param_config.ini`

首次启动时，`S98ovis-manager` 会生成持久化设备 ID：

```text
/mnt/cfg/ovis-manager/device-id
```

设备识别接口在重启后返回相同的 `device_id` 和 `serial`。

首次启动时，`S98ovis-manager` 会生成随机管理员密码，并写入 CFG 持久化分区：

```text
/mnt/cfg/ovis-manager/ovis.account
```

账号文件采用可直接阅读的格式，权限为 `0600`：

```ini
username=admin
password=<随机密码>
```

已通过 SSH 或 UART 登录设备的管理员可以直接查看：

```bash
cat /mnt/cfg/ovis-manager/ovis.account
```

如果升级前已存在旧版 `/mnt/cfg/ovis-manager/credentials`，启动脚本会保留原账号密码并自动迁移到 `ovis.account`。

## 构建

在 SDK 根目录执行：

```bash
source build/envsetup_soc.sh
olddefconfig
build_ovis_manager
pack_rootfs
```

`build_ovis_manager` 只负责交叉编译和安装组件，产物先进入独立暂存目录：

```text
ovis-manager/install/usr/sbin/ovis-managerd
ovis-manager/install/etc/init.d/S98ovis-manager
```

`pack_rootfs` 不会重新编译管理应用，只会检查上述产物并将 `ovis-manager/install/` 复制到目标 rootfs。`build_all` 已在 `pack_rootfs` 前调用 `build_ovis_manager`，完整构建不需要额外手动调用。

首次启动没有网络配置时，设备只枚举 `PID 0x100E` 的 WebUSB 配置接口，不启用
NCM、UVC、DHCP 和 Manager。生产网页让用户填写 `192.168.X.1` 中的 `X`，校验当前
连接设备没有重复网段后提交。第三段保存到 `/mnt/cfg/ovis-manager/ncm-subnet`，设备在
前端确认写入结果后自动重启，并枚举为 `PID 0x100D` 的 `NCM + UVC` 运行设备。
运行期间修改 UVC 输出状态后，Manager 保存配置并安排设备重启，不在当前 NCM 管理
连接上热拆建 ConfigFS gadget。冷启动时，关闭 UVC 的设备枚举为 `PID 0x100F` 的
`NCM only` 设备；独立 PID 用于避免 Windows 复用同一 USB 身份下缓存的复合接口布局。
重新开启 UVC 并重启后恢复为 `PID 0x100D`。
板端在 gadget 绑定 UDC 后，从 NCM ConfigFS Function 的 `ifname` 属性读取实际网卡名，
不依赖重载后仍为 `usb0`；DHCP 配置会随实际接口名同步生成。UDC 绑定前出现的
`(unnamed net_device)` 仅用于配置 MAC，不会作为网络接口名使用。

后续启动根据 UVC 输出配置进入 `NCM + UVC` 或 `NCM only` 运行模式，并恢复地址、
DHCP 和 Manager，不再加载 FunctionFS WebUSB 接口，也不再要求配置。配置态和运行态
分离，避免 FunctionFS 影响 Windows 对视频复合设备的描述符枚举。

USB gadget 的实际开机启动入口是 `S99v_ovis_usb`，位于 `S99user` 加载 MPP 模块之后、
`S99z_ipcamera` 之前。`S77ncm` 保留为兼容控制入口。未配置网络时 `ipcamera` 延迟
启动；WebUSB 提交配置并自动重启后，由开机 USB 脚本启动 Manager 和 `ipcamera`。
延后入口会等待系统负载稳定，并在 UDC 未保持绑定时清理后重试，最多三次；完整启动
trace 保存到 `/var/log/ovis-usb-startup.log`。

USB NCM 地址、DHCP 地址池和 Manager 监听地址使用同一网段。USB 序列号和 NCM MAC
由持久化 `device_id` 稳定派生。Manager 监听对应 NCM 地址的 TCP 8080 端口，并允许
生产网页 `https://ovis.aimorelogy.com` 以及本地 Vite 开发地址访问 API。

WebUSB 配置态使用带 Bulk IN/OUT 端点的 FunctionFS Vendor Interface，以兼容板端
DWC2 UDC 和 Windows 枚举。配置命令仍通过 EP0 提供以下协议；这些端点不会出现在
`NCM + UVC` 运行态，因此不占用运行态端点资源：

| 请求 | 代码 | 方向 | 作用 |
| --- | --- | --- | --- |
| `GET_INFO` | `0x01` | IN | 读取设备 ID、当前第三段和待提交第三段 |
| `QUIESCE_NCM` | `0x02` | OUT | 停止 DHCP、Manager 并关闭当前 NCM 地址 |
| `SET_SUBNET` | `0x03` | OUT | 写入待提交的 `X`，有效范围为 `0-255` |
| `COMMIT` | `0x04` | OUT | 原子保存第三段、同步 CFG 并延迟自动重启 |
| `ABORT` | `0x05` | OUT | 取消待提交配置并保持 NCM 关闭，等待重试 |

网页在 `COMMIT` 后、设备自动重启前再次调用 `GET_INFO`，只有确认最终第三段已经写入且
待提交值已清除后才显示成功。板端将待提交文件重命名后同步 CFG 目录，避免紧接着断电
或重启时丢失持久配置；提交过程写入 `/var/log/ovis-webusb.log`。COMMIT 后预留约三秒
供前端确认状态，然后执行正常系统重启。开机入口重新加载持久化设备 ID，并据此恢复
USB serialnumber 与 NCM 两端 MAC。

## 设备识别接口

网页通过匿名接口识别设备并执行心跳检测：

```text
GET /api/v1/device/info
```

响应包含协议名、API 版本、设备 ID、名称、型号、序列号、固件版本和 Manager 版本。接口支持 GitHub Pages 跨域请求及浏览器本地网络访问预检。

对于允许的网页来源，Manager 同时返回
`Access-Control-Allow-Private-Network`、`Private-Network-Access-Name` 和
`Private-Network-Access-ID`。其中 48 位网络身份由持久化 `device_id` 派生，
用于兼容不同版本 Chromium 的本地网络权限流程，不改变设备 API 身份。

已初始化设备通过以下接口清除 USB NCM 网段：

```text
POST /api/v1/device/network/reset
```

请求没有消息体；Manager 先返回空的 `202 Accepted`，随后延迟删除
`/mnt/cfg/ovis-manager/ncm-subnet` 和待提交文件，同步 CFG 并正常重启。设备重启后
重新枚举为 `PID 0x100E` 的 WebUSB 配置设备，设备 ID、管理账号和视频配置不受影响。

## 配置接口

当前配置页面不做登录，以下接口允许受支持的网页来源直接访问：

```text
GET  /api/v1/config/capabilities
GET  /api/v1/config
POST /api/v1/config/validate
PUT  /api/v1/config
POST /api/v1/config/apply
POST /api/v1/config/reset
GET  /api/v1/tasks/{task_id}
```

配置白名单包括 RTSP/UVC 输出开关、主码流帧率和码率、子码流开关/帧率/码率、OSD、目标检测、人脸检测、人体姿态、目标检测与跟踪和移动检测。各 AI 功能通过 `processing_size` 设置送入对应 AI 管线的图像帧尺寸；该字段不改变 BModel 编译时固定的 Tensor 尺寸。目标跟踪分别返回固定的 `detection_processing_size` 和 `tracking_processing_size`。主码流分辨率仍使用板端公布的固定 profile。主码流选择 60 fps 时，接口会同步切换 SC235HAI 到 1080p60 sensor 模式；选择 15、25 或 30 fps 时使用 1080p30 sensor 模式，由编码通道按目标帧率输出。CV184X 的离线 VPSS 组只使用物理通道 0：`grp1 ch0` 保留主码流实际帧率供 RTSP 使用，独立的 `grp6 ch0` 供 UVC 使用。该通道在 30 fps sensor 模式配置为 `30 -> 30`，在 60 fps sensor 模式配置为 `60 -> 30`，避免非法帧率组合，并避免 60 fps 模式向 USB 推送双倍 MJPEG 帧。多条下游链路共享 `grp0 ch0` 的独立公共源池。目标检测、人脸检测和移动检测从该 NV12 帧分别进入独立 VPSS 组；只有人体姿态或目标跟踪启用时才创建 `grp0 ch2` 的 RGB 通道和 pool1。目标跟踪动态复用 `grp0 ch2`：检测态输出 640x384 C3 并使用 pool1，跟踪态输出 1920x1080 NV12 并使用独立 pool7；遗留 grp5 始终关闭。各 AI 专用 VB 池按功能开关动态启停，TDL 预处理会从剩余编号中动态申请临时 VPSS 组。

能力接口使用 schema version 4，输出开关位于 `values.outputs.rtsp.enabled` 和 `values.outputs.uvc.enabled`；目标检测位于 `values.detection.object`，其中 `model` 明确返回 `builtin` 或 `custom` 来源及模型 ID。关闭 RTSP 时同步关闭 RTSP Server、VENC0/1/2、VPSS grp1、子码流通道和 pool6，但保留 `video.sub.enabled` 的用户设置；关闭 UVC 时同步关闭 VENC3、VPSS grp6/chn0 和 pool8，并在重启后从 USB 复合设备中移除 UVC Function。校验响应在 UVC 状态变化时额外返回 `usb_gadget_restart` 和 `management_reconnect`。板端保存 UVC 状态后会延迟重启，不在当前 NCM 管理连接上热拆重建 USB Gadget。

目标跟踪通过 `/tmp/track` 接收一次性选择命令，命令被消费后文件会自动删除。坐标以预览画布为基准，板端会换算到检测输入尺寸：

```sh
# 点击选择：x、y、预览宽、预览高
printf 'point 960 540 1920 1080\n' > /tmp/track.tmp
mv /tmp/track.tmp /tmp/track

# 框选：x1、y1、x2、y2、预览宽、预览高
printf 'box 760 340 1160 740 1920 1080\n' > /tmp/track.tmp
mv /tmp/track.tmp /tmp/track

# 停止跟踪
printf 'stop\n' > /tmp/track.tmp
mv /tmp/track.tmp /tmp/track
```

`default` 和 `point` 始终按 `search_method` 使用颜色分割或 FastSAM，`box` 使用显式框作为搜索提示；只有 `id <track_id>` 会选择对应的完整检测框。兼容旧命令 `echo 1 > /tmp/track`，其含义是对画面中心点执行搜索。生产调用应像示例一样先写临时文件再原子重命名，避免读取到未写完的命令。

关闭子码流时，Manager 会同时关闭 `vpssgrp0.chn1`、子码流编码通道、依赖该 VPSS 通道的 JPEG 抓图通道以及第二路 OSD；重新开启子码流时会原子恢复这些依赖项。前端只需继续提交 `video.sub.enabled`，不需要增加额外字段。

能力接口同时公布板端实际安装的 AI 工作负载。目标检测、人脸检测、人体姿态和目标跟踪最多启用一项；移动检测不占用 TPU，可独立同时开启。目标跟踪支持 `color` 和 `fastsam` 两种搜索方式。

固件升级时，Manager 会补齐旧运行配置缺少的 AI 参数段，将旧 `/mnt/sd` 模型路径迁移到 `/usr/share/ipcamera/cv184x`，消除旧配置中同时开启多个 TPU 功能的冲突，并同步各 AI 功能对应的 VPSS 处理组开关，不覆盖已有视频参数。

`PUT /api/v1/config` 只生成待应用配置，不会直接影响当前视频服务。`POST /api/v1/config/apply` 先返回处于 `queued` 状态的任务，并保留 1 秒响应窗口，然后校验对应 revision、备份当前配置、原子切换并异步重启 `ipcamera`；新配置启动失败时自动恢复备份并再次启动旧配置。网页通过任务接口读取进度及 `rolled_back` 结果，USB 网络短暂断开后可按设备 ID 重连并继续确认任务。

可单独运行配置事务测试：

```bash
make -C ovis-manager test
```

## 自定义模型接口

模型接口的正式契约位于
[openapi-models.yaml](./openapi-models.yaml)。前端应以该文件生成 TypeScript 类型，
不要从本节示例推断字段。设备运行时还会通过 `GET /api/v1/models/importers` 返回当前
固件实际支持的导入器、metadata JSON Schema、默认值、约束和上传能力。

自定义模型保存在 `/mnt/system/ovis-models`。每个模型拥有独立的 BModel、
`factory.json` 和元数据文件，不修改系统 `/usr/share/ipcamera/model_factory.json`。
上传临时文件与最终目录位于同一分区，提交时通过目录重命名原子切换。

模型管理区域不使用独立账号或 Basic Authentication。创建、上传、提交、保存部署参数、
启用、停用和删除等写操作必须携带 `X-OVIS-CSRF: 1`；读取接口无需认证。

### 导入器

```http
GET /api/v1/models/importers
```

前端只显示响应中存在且 `schemaVersion` 匹配的导入器。目标检测导入器当前可以部署；
图像分类、关键点、实例分割、特征提取和语音分类可以导入保存，但在对应 ipcamera
运行管线接入前返回 `deployable: false`。YOLOv5 和 YOLOv7 导入器的 `defaults`
包含与当前 TDL 后处理一致的三组默认 Anchors。导入器的 `constraints` 会公布后处理
不支持的类别数量，前端应在创建导入任务前完成校验。`runtimeConsumers` 是当前固件
能够实际使用该模型的运行管线；`feature.image` 当前返回空数组，不能由前端虚构消费者。

### 创建与上传

```http
POST /api/v1/models/imports
Content-Type: application/json
```

```json
{
  "importerId": "detection.yolov8",
  "schemaVersion": 1,
  "name": "安全帽检测",
  "fileSize": 3145728,
  "metadata": {
    "labels": ["person", "helmet"]
  }
}
```

`fileSize` 必须与随后上传请求的 `Content-Length` 完全一致。模型文件使用原始二进制
上传，不允许使用 Base64、multipart、chunked encoding 或 `Content-Range`：

```http
PUT /api/v1/models/imports/{id}/content
Content-Type: application/octet-stream
Content-Length: <BModel 字节数>
```

浏览器端直接将 `File`/`Blob` 作为请求体，不要尝试手动设置 Fetch 禁止修改的
`Content-Length`；浏览器会根据请求体自动生成该头。

Manager 只对该接口进行流式接收，普通 JSON 请求限制为 128 KiB。上传完成后提交：

当前协议不支持断点续传。前端可以持久化 import ID；恢复时通过导入任务详情读取状态。
`created` 或 `failed` 状态都必须从 offset 0 全量重传，`uploadedBytes` 只用于展示上一次
尝试的结果，不能作为续传偏移。实时进度来自浏览器上传事件。

```http
POST /api/v1/models/imports/{id}/commit
```

提交会校验文件大小、BModel 文件头和对应导入器元数据，并生成独立
`factory.json`。模型真正打开时仍由 TDL 校验网络结构；检测模型无法启动时沿用配置
事务自动回滚。commit 校验失败会在 HTTP 错误响应及导入任务的 `validationError`
中保留，刷新页面后仍可读取。

任务状态和取消接口：

```http
GET    /api/v1/models/imports/{id}
DELETE /api/v1/models/imports/{id}
```

### 各导入器元数据

- `detection.yolov5`、`detection.yolov7`：`labels`、`anchors`。`anchors` 为
  `3 x 3 x 2` 的正整数数组。
- `detection.yolov6`、`detection.yolov8`、`detection.yolov10`、
  `detection.yolo26`、`detection.ppyoloe`、`detection.yolox`：`labels`。
- `classification.image`：`labels`、`rgbOrder`、`mean`、`std`。
- `pose.yolov8`：`labels`、`keypoints`、`rgbOrder`，可选 `skeleton`。
  `skeleton` 的每一项是 `[起点索引, 终点索引]`。
- `segmentation.yolov8`：`labels`、`rgbOrder`，可选与类别等长的 `colors`。
- `feature.image`：`rgbOrder`、`mean`、`std`。
- `classification.sound_command`：`labels`、`sampleRate`、`channels`、
  `hopLength`、`preprocessProfile`。采样率支持 8000 或 16000，通道数固定为 1，
  预处理规格支持 `common` 或 `fixed`。导入器默认值为 16000 Hz、单声道、
  Hop Length 128、`fixed`。

类别和关键点数组均禁止空项及重名，数组顺序就是模型输出索引。`rgbOrder` 仅接受
`RGB` 或 `BGR`，Mean/Std 必须分别包含三个数值且 Std 不能为零。

### 模型管理与部署

```http
GET    /api/v1/models
GET    /api/v1/models/{id}
DELETE /api/v1/models/{id}
GET    /api/v1/models/{id}/deployment
PUT    /api/v1/models/{id}/deployment
POST   /api/v1/models/{id}/activate
POST   /api/v1/models/{id}/deactivate
```

检测模型部署参数包含 `threshold` 和 `processingSize`。阈值范围为 `0..1`、默认值为
`0.5`、建议步进 `0.01`。`processingSize` 表示送入检测管线的 VPSS 图像帧尺寸，不是
BModel 编译时固定的 Tensor 尺寸；默认值为 `448x256`，宽度范围为 `160..1920`、高度范围
为 `96..1080`，宽高都必须是偶数。目标检测分支和 VB 池会按该尺寸同步配置，并统一使用
NV12 帧；TDL 再根据 BModel 的输入 Tensor 规格执行颜色转换、等比例缩放和归一化。1080p
输入不会创建 1080p RGB 常驻池，避免挤占 CV184X carveout 内存。
`GET/PUT deployment` 用于读取和保存参数，保存不会重启服务。启用请求可以不携带消息体
并使用已保存参数，也可以携带一次性参数覆盖：

```json
{
  "threshold": 0.5,
  "processingSize": {"width": 448, "height": 256}
}
```

启用和停用返回现有配置任务的 `task_id`，前端继续通过
`GET /api/v1/tasks/{task_id}` 跟踪重启与回滚结果。启用自定义检测模型会关闭冲突的
人脸、人体姿态和目标跟踪 TPU 任务，但不会关闭不占用 TPU 的移动检测。正在运行的
模型必须先停用并等待任务成功后才能删除。模型列表和详情分别通过 `active` 表示模型
当前正在推理，通过 `referenced` 表示运行配置仍引用该模型；`referenced: true` 时即使
`active: false` 也必须先调用停用接口，切回内置模型配置后才能删除。

Manager 在内存中保留最近 16 个配置任务，但任务不跨 Manager 进程重启持久化。重连后
任务接口返回 404 时，前端应重新读取模型详情和 deployment 状态，以 `active`、
`referenced` 和 `appliedParameters` 确认最终结果。
