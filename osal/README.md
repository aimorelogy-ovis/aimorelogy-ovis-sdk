# OSAL编译与测试指南

OSAL（操作系统抽象层）是一个跨平台的抽象层，用于屏蔽不同操作系统之间的差异，使上层应用可以无需关注底层操作系统的特性。本文档将详细介绍如何编译不同版本的 OSAL 以及如何进行测试。

---

## 目录结构

- [背景](#背景)
- [编译不同版本的 OSAL](#编译不同版本的-osal)
  - [编译 AliOS版本](#编译-alios)
  - [编译 Linux内核版本](#编译-linux)
  - [编译 RT-Thread版本](#编译-rt-thread)
- [测试 OSAL](#测试-osal)
  - [测试 AliOS](#测试-alios)
  - [测试 Linux](#测试-linux)
  - [测试 RT-Thread](#测试-rt-thread)


## 背景


OSAL 是一个操作系统抽象层，旨在为不同的操作系统提供一致的接口。该项目支持 AliOS、Linux 和 RT-Thread 三个版本，用户可以根据需要选择合适的版本进行编译和测试。




## 编译不同平台版本的OSAL



### 编译Alios版本



### 编译Linux内核版本

```
// 环境搭建;
git clone https://gerrit-ai.sophgo.vip:8443/plugins/gitblit/summary/?r=cvi_manifest.git
```

```
// 拉单系统（v4.1.0）分支源码;
./cvi_manifest/cvitek_repo_clone.sh --gitclone ./cvi_manifest/golden/cv181x_cv180x_v4.1.0.xml
```

```
// 编译SDK;
source build/envsetup_soc.sh
defconfig cv181x
defconfig  cv1811c_wevb_0006a_spinor
clean_all && build_all
```
将固件fip.bin、boot.spinor、rootfs.spinor拷贝至sd卡中。
将sd卡插入对应evb（cv1811c\_wevb\_0006a）中，重新上电升级，待升级完成，拔出sd卡，重新上电。

```
// 拉取代码;
git clone https://gerrit-ai.sophgo.vip:8443/plugins/gitblit/summary/?r=cvitek/osal.git
```

将osal放在osdrv/interdrv下，在v4.1.0目录下使用build_osrdv编译出cv181x_osal.ko和cv181x_osal_test.ko。这两个.ko文件位于osdrv/interdrv/osal目录下。



### 编译rt-thread版本

```
// 环境搭建;
git clone https://gerrit-ai.sophgo.vip:8443/plugins/gitblit/summary/?r=cvi_manifest.git
```

```
// 拉取RT-Thread SDK;
./cvi_manifest/cvitek_repo_clone.sh --gitclone ./cvi_manifest/rtos/cvi_rtt.xml`
```

拉取sdk完整sdk后，SDK根目录下会有cvi_rtt这个目录

```
// 获取README，了解SDK的编译和烧录;
cd rt-thread/cvi_rtt
vim REAEME.md
```

在当前路径下 make smart_doorbell menuconfig即可选择板卡和配置，选中Board Type：cv180zb_wevb_0008a_spinor以及Project Configuration: turnkey_180xb，选中之后输入指令make smart_doorbell 编译。

```
// 编译后的固件在下面的路径中;
rt-thread/cvi_rtt/generated/image
```

烧录整个固件包即可



## 测试不同平台版本的OSAL

### 测试Alios版本

### 测试linux版本

需先编译linux内核版本osal，将生成的两个.ko拷贝至SD卡。

将sd卡插入对应evb（cv1811c_wevb_0006a）并挂载（mount  /dev/mmcblk0p1  /mnt/sd/ ）

```
// 加载cv181x_osal.ko和cv181x_osal_test.ko内核模块;
cd /mnt/sd
insmod cv181x_osal.ko
insmod cv181x_osal_test.ko
```

```
// 调用全部测试
echo "test_all"  > /proc/osal_test
```

```
// 查看测试结果报告
cd /tmp
vi osal_test_report.txt
```

如要进行单一测试，使用以下：

```
// 举例：单一测试osal_addr.c
echo "addr"  > /proc/osal_test
//查看测试情况
dmesg | tail -n 20
```




### 测试rt-thread版本

编译rt-thread版本后，烧录整个固件包（使用usb烧录），输入help指令，即可执行osal的测试命令。
（osal_fileops_test测试前需要插入SD卡，因此，进行test_all测试前也需要插入SD卡）

```
// 调用全部测试
osal_test_all
```

如要进行单一测试，使用以下：

```
// 举例：单一测试osal_addr.c
osal_addr_test
//测试osal_fileops.c，记得插入sd卡
```


## 许可证

本项目基于 [GPL-2.0](LICENSE) 许可证开源.
