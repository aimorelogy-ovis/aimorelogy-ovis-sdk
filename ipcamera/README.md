# ipcamera

## 1 简介
ipcamera是一款网络摄像机管理软件，是网络视频服务器和摄像头的集成。本软件基于插件化的思想，可实现多路数据流并发处理，并通过网络控制和展示数据流。使用ini文件对软件环境进行配置的方式，可大大简化用户配置工程或添加插件的复杂度。

主要目录结构和模块说明：
| 目录| 模块 | 功能说明 |
| ----------- | ---------------------|-----------|
| build       | build                |编译相关文件|
| components  | cvi_osal             |提供操作系统抽象层API，包括锁、任务、计时器等|
|             | cvi_rtsp             |提供rtsp推流功能的API|
|             | ringbuffer           |提供ringbuffer功能的API|
| configs     | configs              |选择需要进行编译的模块|
| modules     | ai                   |算法相关模块，包括有移动检测、行人检测、人脸检测等|
|             | common               |通用模块，包括有ini解析和cjson|
|             | display              |显示模块，用于显示屏的输出|
|             | media                |多媒体模块，包括有vi，vpss和venc等模块|
|             | peripheral           |外设模块，包括有adc，dma和gpio等模块|
|             | protocol             |协议模块，包括有uac，uvc和network等模块|
| prebuilt    |                      |预编译的第三方库文件，包括有ffmpeg，libwebsockets和openssl等|
| resource    | ai_models            |算法模型的资源文件|
|             | bitmap               |rgn叠图功能的资源文件|
|             | file_recover         |文件修复（录像模块使用）功能的资源文件|
|             | parameter            |ini配置文件|
|             | www                  |网络功能的资源文件|
| solutions   | ipcamera             |ipcamera程序的主函数|

## 2 环境准备和编译

ipcamera软件基于端侧sdk，需要将ipcamera软件放置于sophpi同级目录下，并事先完成sophpi的拉取和编译工作。步骤如下所示：

1. 安装依赖的工具库<br>
ubuntu推荐安装20.04或者22.04，并需要安装依赖的工具。
```bash
sudo apt-get update
sudo apt-get install -y build-essential ninja-build automake autoconf libtool wget curl git gcc libssl-dev bc slib squashfs-tools android-sdk-libsparse-utils android-sdk-ext4-utils jq cmake python3-distutils tcl scons parallel openssh-client tree python3-dev python3-pip ssh libncurses5 pkg-config lzop bison flex rsync kmod cpio sudo fakeroot dpkg-dev device-tree-compiler u-boot-tools uuid-dev libxml2-dev debootstrap qemu qemu-user-static kpartx binfmt-support git-lfs libisl-dev texlive-xetex libgflags-dev python-is-python3
sudo pip3 install -U yoctools
sudo python3 -m pip install -U pip setuptools wheel
sudo python3 -m pip install -i https://pypi.tuna.tsinghua.edu.cn/simple jinja2
```

2. 配置git账号<br>
在github建立个人账号，并配置好ssh key,下载代码需要用到个人github账号。
```bash
git config --global user.name "your_name"
git config --global user.email "your_email@example.com"     #这个必须是github账号
ssh-keygen -t ed25519 -C "your_email@example.com"
cat ~/.ssh/id_ed25519.pub   #将公钥加入到github的setting的ssh key中
```

3. 下载端侧sdk代码<br>
```bash
git clone -b sg200x-evb git@github.com:sophgo/sophpi.git      #端侧最新代码
cd sophpi
./scripts/repo_clone.sh --gitclone scripts/subtree_cv184x-v6.x.xml
```

4. 编译sdk<br>
```bash
export TPU_REL=1                      # 如果不需要算法功能这一步可以不执行
source build/envsetup_soc.sh
defconfig cv1842cp_wevb_0015a_spinor  # 选择对应的卡板，这里以cv1842cp_wevb_0015a_spinor为例
clean_all && build_all                # 编译sdk
```
5. 烧录固件<br>
sdk编译完成后，将在install目录下生成固件，将固件放置在tf卡中，插入板端，重新上电后即可进入升级。关于烧录的具体流程可以参考《SDK 编译使用手册》第3节 烧录说明，地址为：https://developer.sophgo.com/thread/813.html

6. 编译ipcamera程序<br>
将ipcamera程序与sdk放置在同一目录下，按一下流程即可进行编译，编译完成后的可执行文件ipcamera位置为install目录下。
```bash
cd ipcamera                     # 进入ipcamera目录
make cv184x_ipcamera_defconfig  # 配置需要编译的组件，这里以cv184x_ipcamera_defconfig为例，文件在configs目录下
make ipcamera clean_all && make ipcamera && make ipcamera install
```

## 3 程序运行
将install目录下的所有文件拷贝至tf卡中，并插入板端；板端上电后需要将板端的IP地址改为和PC端相同的网段，再运行程序即可，具体操作如下，这里以PC的IP地址为192.168.3.10作为例子，将板端的IP地址设置为192.168.3.8。
```bash
ifconfig eth0 192.168.3.8 netmask 255.255.255.0
mount /dev/mmcblk0p1 /mnt/sd
cd /mnt/sd
./ipcamera -i param_config.ini &
```
程序运行起来后，即可通过RTSP流rtsp://192.168.3.8:8554/live0来观看视频流；也可以通过`http://192.168.3.8/index.html`的url进行访问（sd卡中需要有www的文件夹，该文件夹位于install或ipcamera/resource目录下）。

## 4 模块的编译控制
1. 打开模块功能<br>
模块的编译控制在configs文件夹下，客户可以通过宏CONFIG_XXX来控制模块的开启或关闭;也可以在ipcamera的目录下执行如下命令:
```bash
make menuconfig
```
勾选所需要编译的模块。
```bash
 ────────────────────────────────────────────────────────────────────────────────────────────────────────────────
  ┌────────────────────────────────────────── Sophapp Configuration ──────────────────────────────────────────┐
  │  Arrow keys navigate the menu.  <Enter> selects submenus ---> (or empty submenus ----).  Highlighted      │  
  │  letters are hotkeys.  Pressing <Y> includes, <N> excludes, <M> modularizes features.  Press <Esc><Esc>   │  
  │  to exit, <?> for Help, </> for Search.  Legend: [*] built-in  [ ] excluded  <M> module  < > module       │  
  │  capable                                                                                                  │  
  │ ┌───────────────────────────────────────────────────────────────────────────────────────────────────────┐ │  
  │ │                    cross_compile options  --->                                                        │ │  
  │ │                    platform options  --->                                                             │ │  
  │ │                    module options  --->                                                               │ │  
  │ │                    resource options  --->                                                             │ │  
  │ │                    peripheral support list  --->                                                      │ │  
  │ │                                                                                                       │ │  
  │ └───────────────────────────────────────────────────────────────────────────────────────────────────────┘ │  
  ├───────────────────────────────────────────────────────────────────────────────────────────────────────────┤  
  │                         <Select>    < Exit >    < Help >    < Save >    < Load >                          │  
  └───────────────────────────────────────────────────────────────────────────────────────────────────────────┘ 
```

2. 添加新模块功能<br>
如果需要添加一个新的模块，可以按如下步骤进行：<br>
2.1 module 目录下对应添加 Makefile 以及src,include等目录 具体参考module/ai；<br>
2.2 如果需要全局依赖的头文件目录 则放到dep.mk中，如果为模块内需要依赖的头文件则对应放置于模块Makefile中；<br>
2.3 在link.mk中添加对应生成的libxxxx.a 作为依赖；<br>
2.4 在build/module/Kconfig 中添加对应的模块配置；<br>
2.5 在configs/xxxx_defconfig 添加&打开对应的模块配置。<br>


## 5 ini配置文件
本软件可以通过ini文件来配置工程，ini的使用例子可以参考ipcamera/resource/parameter/sbm下的文件。以下为ini配置文件常用的说明：
```bash
[module_config] 配置多媒体是否在小核中运行，仅在双系统下生效
[slice_config] 配置slice buffer模式，该模式可以减少内存占用
[vb_config] 配置VB池的数量
[vb_pool_x] 为第x个VB池配置属性，包括分辨率、帧格式、色深、压缩模式等
[vi_config] 配置Sensor的数量
[sensor_configx] 配置第x个Sensor的属性，包括帧率、Sensor型号、I2C地址、MIPI线序等
[vpss_config] 配置VPSS Group的数量
[vpssgrpx] 配置第x个VPSS Group的属性，包括帧格式、帧率、分辨率、通道数、绑定模式等
[vpssgrpx.chny] 配置第x个VPSS Group的第y个通道属性，包括分辨率、帧格式、裁切大小等
[venc_config] 配置编码通道数量
[vencchnx] 配置第x个编码通道的属性，包括缓存帧数、编码方式、分辨率、帧率、数据源、编码属性等
[osdc_config] 配置OSDC的数量
[osdc_configx] 配置第x个OSDC的属性
[osdcx_obj_infoy] 配置第x个OSDC的第y个信息，包括字符串内容、字符串颜色、图片地址等。
[rtsp_config] 配置RTSP通道数和端口
[sessionx] 配置第x个RTSP通道的数据源和码率
[gpio_config] 配置红外切换和日光灯切换控制的GPIO
[ai_pd_config] 配置PD行人检测功能
[md_config] 配置MD运动识别功能
[ai_fd_config] 配置人脸检测功能
[rtsp_config] 配置rtsp的数量和port
[sessionx] rtsp对应的venc通道和参数
[audio_config] 配置音频功能
```
