# UVC / RTSP 拉流后切换排查与回归

## 现象与定位范围

用户反馈：开机后未拉流时可以切换 UVC / RTSP；先拉过流再切换，另一种输出无法拉流，两个方向均可能发生。

当前配置应用链路是：`POST /api/v1/config/apply` 保存配置，发现 `uvc_enable` 改变后调用 `S77ncm output-reboot`，同步文件系统并延迟请求正常系统重启。切换不是进程内热切换 USB gadget。

因此需要首先确认设备是否真的完成重启，再区分旧进程退出卡住、USB 关机卡住，以及重启后的新链路启动失败。下述问题由源码及仓库内预编译库检查发现，尚未通过板上复现确认它们与用户现象一一对应。

## 本次修正

### RTSP 进程退出

旧流程在 `app_ipcam_Exit()` 中调用 `RTSP_Destroy()`。检查仓库内 ARM musl 版本 `libcomp_rtsp.so.0.0.1` 可见：内部 `rtsp_server_destroy()` 持有服务对象的 mutex 时设置监听线程退出标志并执行 Join；`rtsp_server_listen_task()` 每轮也获取同一把 mutex。如果监听线程已经检查过退出标志，随后等待这把锁，销毁线程与监听线程会互相等待。

该预编译库没有随附对应实现源码。本次增加仅用于进程退出的 `app_ipcam_rtsp_Server_Quiesce()`：

- 屏蔽新的应用侧连接、断开处理，防止退出时重新启动送帧线程。
- 通知所有应用侧送帧线程退出，并等待它们释放 MBUF reader。
- 保留 RTSP 库对象，继续正常释放 VENC、VPSS、VI 等硬件资源。
- `ipcamera` 正常退出时，由操作系统回收剩余库线程、socket 和进程内存。

该接口不能用于进程内停止后重新创建 RTSP。原有 `Server_Destroy()` 仍保留给其他调用路径；预编译库内部销毁问题没有被直接修复。没有修改库二进制，也没有引入强杀或强制重启。

### UVC 应用侧缓存

旧编码送帧函数仅在开始、结束时短暂持锁，中间取缓存节点和复制图像时不持锁。`UVC_Stop()` 关闭推送并退出事件线程后，可能释放仍被编码线程使用的缓存。事件线程处理超时或 STREAMON 时也可能重新开启推送。

本次让送帧调用在缓存节点使用期间持有流锁，使停流等待已开始的复制完成；停止时先清除运行标志，后续事件不能重新启用推送。

### UVC 内核发送任务

旧停流流程先执行 `cancel_work_sync()`，随后取消 USB request 并释放 request 缓冲。但完成回调仍无条件重新排队发送任务；DWC2 的 dequeue 路径也会调用该完成回调。这会产生取消任务后重新调度、任务与 request 释放并发的窗口。

本次增加由 `req_lock` 保护的流状态。停流先禁止调度，再同步取消任务及回收 request；发送任务和完成回调的调度入口均受该状态约束。

## 板上回归

本次涉及 `ipcamera` 和内核 UVC gadget 驱动。验证固件需要同时包含这两部分改动；`uvc.h` 中的共享结构有变化，驱动中包含该头文件的对象需要一致更新。

没有执行编译或板上测试。

每次测试前记录以下信息，切换并恢复连接后再次读取：

```sh
cat /proc/sys/kernel/random/boot_id
cat /proc/uptime
/etc/init.d/S99z_ipcamera status
```

`boot_id` 应变化，uptime 应重新计时。仅网页恢复访问或配置显示已切换，不足以证明重启完成。

| 起始模式 | 切换前操作 | 切换后检查 |
| --- | --- | --- |
| UVC | 未打开视频 | 重启后 RTSP 可拉流 |
| UVC | 保持 UVC 播放时切换 | 重启完成，RTSP 可拉流 |
| UVC | 播放后关闭播放器再切换 | 重启完成，RTSP 可拉流 |
| RTSP | 未拉流 | 重启后主机可打开 UVC |
| RTSP | 保持 RTSP 播放时切换 | 重启完成，主机可打开 UVC |
| RTSP | 拉流后关闭播放器再切换 | 重启完成，主机可打开 UVC |

以上场景连续重复，并分别覆盖实际使用的 30/60 fps、RTSP TCP/UDP，以及多客户端连接。还需检查同一模式内关闭再打开播放器，确认停流后的再次启动正常。USB 重新枚举后，应重新打开主机端视频设备。

串口重点观察：

- `OVIS USB output configuration reboot now`：已发出重启请求。
- `RTSP media quiesce completed: ret=0`：应用侧 RTSP 送帧线程已停止。
- `UVC: event thread stopped.` 和 `UVC_GADGET_DeviceClose` 的完成日志。
- `ipcamera` 正常退出、内核重启和新的开机日志。
- `timeout waiting for PID`、kernel oops、USB request / VENC 错误。

如果仍卡住，在串口保留完整现场，并在设备仍响应命令时采集线程等待位置：

```sh
cat /proc/uptime
cat /var/run/ipcamera.pid
for task in /proc/$(cat /var/run/ipcamera.pid)/task/*; do
    printf '\n%s\n' "$task"
    cat "$task/comm" "$task/wchan"
done
dmesg | tail -100
```

若 `boot_id` 已变化但新流仍不可用，后续应转查新启动日志、实际输出配置、NCM 网卡恢复及主机端设备重新枚举，不再归因于旧进程资源残留。
