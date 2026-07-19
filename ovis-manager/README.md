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

后续启动直接进入 `NCM + UVC` 运行模式并恢复地址、DHCP 和 Manager，不再加载
FunctionFS WebUSB 接口，也不再要求配置。配置态和运行态分离，避免 FunctionFS
影响 Windows 对视频复合设备的描述符枚举。

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

配置白名单包括主码流帧率和码率、子码流开关/帧率/码率、OSD、人员检测、人脸检测、人体姿态、目标检测与跟踪和移动检测。分辨率使用板端公布的固定 profile。主码流选择 60 fps 时，接口会同步切换 SC235HAI 到 1080p60 sensor 模式；选择 15、25 或 30 fps 时使用 1080p30 sensor 模式，由编码通道按目标帧率输出。人员、人脸和移动检测的独立 VPSS 处理组跟随对应功能开关；目标跟踪复用 grp0 的全高清源通道，遗留 grp5 始终关闭。运行时配置迁移会自动校正这套固定 VPSS/VB 拓扑，普通配置提交不会改动 UVC 或 MIPI 参数。

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

能力接口使用 schema version 2，并公布板端实际安装的 AI 工作负载。人员检测、人脸检测、人体姿态和目标跟踪最多启用一项；移动检测不占用 TPU，可独立同时开启。目标跟踪支持 `color` 和 `fastsam` 两种搜索方式。

固件升级时，Manager 会补齐旧运行配置缺少的 AI 参数段，将旧 `/mnt/sd` 模型路径迁移到 `/usr/share/ipcamera/cv184x`，消除旧配置中同时开启多个 TPU 功能的冲突，并同步各 AI 功能对应的 VPSS 处理组开关，不覆盖已有视频参数。

`PUT /api/v1/config` 只生成待应用配置，不会直接影响当前视频服务。`POST /api/v1/config/apply` 先返回处于 `queued` 状态的任务，并保留 1 秒响应窗口，然后校验对应 revision、备份当前配置、原子切换并异步重启 `ipcamera`；新配置启动失败时自动恢复备份并再次启动旧配置。网页通过任务接口读取进度及 `rolled_back` 结果，USB 网络短暂断开后可按设备 ID 重连并继续确认任务。

可单独运行配置事务测试：

```bash
make -C ovis-manager test
```
