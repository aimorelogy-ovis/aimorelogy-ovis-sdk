# OVIS Manager

`ovis-managerd` 是独立于 `ipcamera` 的设备管理进程，提供静态管理页面、服务控制、运行配置持久化和版本查询接口。

## 目标文件

- `/usr/sbin/ovis-managerd`
- `/usr/share/ovis-manager/www/`
- `/etc/init.d/S98ovis-manager`
- `/mnt/cfg/ovis-manager/ovis.account`
- `/mnt/cfg/ipcamera/param_config.ini`

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
ovis-manager/install/usr/share/ovis-manager/www/
ovis-manager/install/etc/init.d/S98ovis-manager
```

`pack_rootfs` 不会重新编译管理应用，只会检查上述产物并将 `ovis-manager/install/` 复制到目标 rootfs。`build_all` 已在 `pack_rootfs` 前调用 `build_ovis_manager`，完整构建不需要额外手动调用。

默认监听 TCP 8080 端口。生产部署应在设备网络或防火墙层限制管理端口的访问范围。

## 配置接口

配置接口只开放主码流、子码流、OSD 和人员检测的明确白名单字段。写入采用临时文件、完整校验、`fsync`、备份和原子替换，保存后需要重启 `ipcamera` 生效。
