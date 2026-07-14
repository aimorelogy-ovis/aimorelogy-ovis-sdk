-include $(TOP_DIR)/build/.config
-include $(SRCTREE)/.config
-include $(SRCTREE)/build/peripheral/panel.mk
#
CROSS_COMPILE ?=
#
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
CXX             = $(CROSS_COMPILE)g++
CPP             = $(CC) -E
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
ARFLAGS         = rcs
LDFLAGS_SO      = -shared -fPIC
# riscv64-unknown-linux-musl, riscv64-unknown-linux-gnu, aarch64-none-linux-gnu, arm-linux-gnueabihf
export TARGET_MACHINE	:= $(shell ${CC} -dumpmachine)
#
APP_MODULES_PATH  := $(SRCTREE)/modules
APP_PREBUILT_DIR := $(SRCTREE)/prebuilt
APP_RESOURCE_DIR := $(SRCTREE)/resource
APP_INSTALL_DIR  := $(SRCTREE)/install
# COMPONENTS
ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
  APP_COMPONENTS_INSTALL_DIR := $(SRCTREE)/components/comps_install/riscv64-unknown-linux-musl
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  APP_COMPONENTS_INSTALL_DIR := $(SRCTREE)/components/comps_install/riscv64-unknown-linux-gnu
else ifeq ($(TARGET_MACHINE), aarch64-none-linux-gnu)
  APP_COMPONENTS_INSTALL_DIR := $(SRCTREE)/components/comps_install/aarch64-none-linux-gnu
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
  APP_COMPONENTS_INSTALL_DIR := $(SRCTREE)/components/comps_install/arm-linux-gnueabihf
else ifeq ($(TARGET_MACHINE), arm-none-linux-gnueabihf)
  APP_COMPONENTS_INSTALL_DIR := $(SRCTREE)/components/comps_install/arm-none-linux-gnueabihf
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
  APP_COMPONENTS_INSTALL_DIR := $(SRCTREE)/components/comps_install/arm-none-linux-musleabihf
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif

#default set CONFIG_SDK_DIR = TOP_DIR
ifeq ($(CONFIG_SDK_DIR), )
	CONFIG_SDK_DIR := $(TOP_DIR)
endif

# tdl_sdk path
TDL_PATH := $(TOP_DIR)/tdl_sdk

# cvi_mpi path
MW_PATH := $(TOP_DIR)/cvi_mpi

# isp path
ISP_INC := $(MW_PATH)/modules/isp/include/$(SOC_NICK_NAME_LOWER)

# SensorSupportList path
ifeq ($(SOC_SEGMENT), CV184X)
SENSOR_LIST_INC := $(MW_PATH)/../build/media/SensorSupportList/sensor_cfg
endif

# kernel path
KERNEL_PATH ?= $(TOP_DIR)/linux_5.10

# FFMPEG 6.0
FFMPEG_6_0_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg6.0/lib

#NET
ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/musl_riscv64_lib
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/musl_riscv64_lib
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/glibc_riscv64_lib
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/glibc_riscv64_lib
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/glibc_arm32_lib
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/glibc_arm32_lib
else ifeq ($(TARGET_MACHINE), arm-none-linux-gnueabihf)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/glibc_arm32_lib
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/glibc_arm32_lib
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/musl_arm32_lib
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/musl_arm32_lib
else ifeq ($(TARGET_MACHINE), aarch64-none-linux-gnu)
  THTTPD_LIB_DIR = $(APP_PREBUILT_DIR)/thttpd/glibc_arm64_lib
  WEB_SOCKET_LIB_DIR = $(APP_PREBUILT_DIR)/libwebsockets/glibc_arm64_lib
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif
# FFMPEG
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/glibc_arm32_lib
else ifeq ($(TARGET_MACHINE), arm-none-linux-gnueabihf)
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/glibc_arm32_lib
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/musl_arm32_lib
else ifeq ($(TARGET_MACHINE), aarch64-none-linux-gnu)
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/glibc_arm64_lib
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
	FFMPEG_LIB_DIR = $(APP_PREBUILT_DIR)/ffmpeg/musl_riscv64_lib
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif

#OSAL
OSAL_DIR = $(APP_COMPONENTS_INSTALL_DIR)/osal

#RINGBUFFER
RINGBUFFER_DIR = $(APP_COMPONENTS_INSTALL_DIR)/ringbuffer

#RTSP
RTSP_DIR = $(APP_COMPONENTS_INSTALL_DIR)/rtsp

#OPENSSL
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/glibc_arm32_lib
else ifeq ($(TARGET_MACHINE), arm-none-linux-gnueabihf)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/glibc_arm32_lib
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/musl_arm32_lib
else ifeq ($(TARGET_MACHINE), aarch64-none-linux-gnu)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/glibc_arm64_lib
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/glibc_riscv64_lib
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
  OPENSSL_LIB_DIR = $(APP_PREBUILT_DIR)/openssl/musl_riscv64_lib
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif

#LVGL
LVGL_LIB_DIR = $(APP_PREBUILT_DIR)/lvgl/lib

ifeq ($(TARGET_MACHINE), aarch64-none-linux-gnu)
SDK_VER := 64bit
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
SDK_VER := 32bit
else ifeq ($(TARGET_MACHINE), arm-none-linux-gnueabihf)
SDK_VER := glibc_arm
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
SDK_VER := musl
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
SDK_VER := glibc_riscv64
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
SDK_VER := musl_riscv64
endif

include $(SRCTREE)/build/dep.mk
include $(SRCTREE)/build/link.mk
