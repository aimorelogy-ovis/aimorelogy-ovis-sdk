SHELL = /bin/bash

# Control whether to build ctrl_tool (1=enable, 0=disable)
ENABLE_CTRL_TOOL ?= 1
# Control whether to build tools (1=enable, 0=disable)
ENABLE_TOOLS ?= 1
# Strip control: set ENABLE_STRIP=0 to disable stripping for debugging
ENABLE_STRIP ?= 1

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=$(TOP_DIR)/cvi_mpi/Makefile.param
	include $(PARAM_FILE)
endif

ifeq ($(OS_TYPE), DUAL_OS)
else
include $(BUILD_PATH)/.config
include $(MW_PATH)/sample_app/Kbuild
endif

CHIP_ARCH_L ?= $(shell echo $(CHIP_ARCH) | tr A-Z a-z)

include Makefile.releaseList

MW_SAMPLE_COMMON_PATH=$(TOP_DIR)/cvi_mpi/sample_app/common
#include $(BUILD_PATH)/.config
#include $(MW_PATH)/component/isp/common/Kbuild

SDIR = $(PWD)/isp_daemon_tool/src
TMP_FOLDER = tmp
ISP_DIR = $(TOP_DIR)/cvi_mpi/modules/isp
ISP_COMMON_DIR = $(ISP_DIR)/common
CVI_RTSP_PATH = $(TOP_DIR)/cvi_rtsp

ifeq ($(DESTDIR),)
	DESTDIR := $(shell pwd)/install
endif

INCS = -I$(KERNEL_INC) -I$(MW_INC) -I$(ISP_INC) -I$(MW_SAMPLE_COMMON_PATH)
INCS += -I$(CVI_RTSP_PATH)/include/cvi_rtsp
INCS += -I$(PWD)/isp_daemon_tool/include
INCS += -I$(ISP_COMMON_DIR)/clog
INCS += -I$(ISP_COMMON_DIR)/raw_dump/inc
INCS += -I$(ISP_COMMON_DIR)/raw_dump/src
INCS += -I$(ISP_DIR)/$(CHIP_ARCH_L)/isp-daemon2/inc
INCS += -I$(ISP_DIR)/$(CHIP_ARCH_L)/isp-daemon2/prebuilt/include/cvi_json-c
INCS += -I$(TOP_DIR)/tdl_sdk/include

COBJS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.o, $(wildcard $(SDIR)/*.c))
CDEPS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.d, $(wildcard $(SDIR)/*.c))
COBJS += $(patsubst $(SDIR)/module/%.c, $(TMP_FOLDER)/module/%.o, $(wildcard $(SDIR)/module/*.c))
CDEPS += $(patsubst $(SDIR)/module/%.c, $(TMP_FOLDER)/module/%.d, $(wildcard $(SDIR)/module/*.c))

# mw sample common
SAMPLE_SRCS = $(wildcard $(MW_SAMPLE_COMMON_PATH)/*.c)
SAMPLE_OBJS = $(SAMPLE_SRCS:.c=.o)
SAMPLE_DEPS = $(SAMPLE_SRCS:.c=.d)
PANEL_INC = $(TOP_DIR)/cvi_mpi/component/panel/$(shell echo $(CVIARCH) | tr A-Z a-z)
MW_SAMPLE_CFLAGS = -I$(MW_SAMPLE_COMMON_PATH) -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC) -I$(PANEL_INC)
MW_SAMPLE_CFLAGS += -I$(MW_PATH)/component/isp/common -I$(MW_PATH)/3rdparty/inih
MW_SAMPLE_CFLAGS += -MMD -MP
MW_SAMPLE_CFLAGS += $(KBUILD_DEFINES)

TARGET_MACHINE := $(shell ${CC} -dumpmachine)

ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
SDK_VER := 64bit
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
SDK_VER := 32bit
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
SDK_VER := musl_riscv64
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
SDK_VER := glibc_riscv64
endif

TDL_SDK_LIB_NAME = libtdl_core.so
TDL_SDK_INSTALL_PATH = $(TOP_DIR)/tdl_sdk/install/$(CHIP_ARCH)

TDL_SDK_LIB_EXIST = $(shell if [ -f $(TDL_SDK_INSTALL_PATH)/lib/$(TDL_SDK_LIB_NAME) ];\
					then echo "exist"; else echo "noexist"; fi)

PREBUILT_OBJS :=

TARGET = isp_tool_daemon
CTRL_TARGET = isp_tool_daemon_ctrl
OUT_TARBALL = isp_tool_daemon.tar.gz

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

BM_LIB = $(OUTPUT_DIR)/rootfs/system/lib
TPU_KERNEL_LIB = $(TOP_DIR)/libsophon/tpu-kernel/lib/$(SDK_VER)

LIBS += $(MW_LIBS) -latomic -ldl
LIBS += -lcvi_bin -lini
LIBS += -Wl,-Bstatic -lcvi_ispd2 -Wl,-Bdynamic
LIBS += -lraw_dump
LIBS += -lteaisp
LIBS += -lsensor
LIBS += -L$(BM_LIB) -lbmrt -lbmlib

ifeq ($(OS_TYPE), DUAL_OS)
else
LIBS += -lsns_full
endif

LOCAL_CFLAGS = $(DEFS) $(INCS) -DSDK_VER=$(SDK_VER)
LOCAL_CFLAGS += -MMD -MP
LIBS += -lcvi_rtsp

ifeq "$(TDL_SDK_LIB_EXIST)" "exist"
	LOCAL_CFLAGS += -DTDL_SDK_LIB=\"$(TDL_SDK_LIB_NAME)\"
endif

LOCAL_LDFLAGS = $(LIBS) -lm -lpthread
LOCAL_LDFLAGS += -L$(CVI_RTSP_PATH)/install/lib
LOCAL_LDFLAGS += -shared-libgcc
CFLAGS += -DENABLE_TEAISP_PQ -DENABLE_FACE_AE

.PHONY: clean all package test tools
ifeq ($(ENABLE_CTRL_TOOL), 1)
ifeq ($(ENABLE_TOOLS), 1)
all: prepare ctrl_tool $(TARGET) tools
else
all: prepare ctrl_tool $(TARGET)
endif
else
ifeq ($(ENABLE_TOOLS), 1)
all: prepare $(TARGET) tools
else
all: prepare $(TARGET)
endif
endif

tools:
	@cd tools && $(MAKE) all

ctrl_tool:
	@cd isp_daemon_ctrl_tool;make;cd ..

prepare:
	-@mkdir -p $(TMP_FOLDER)
	-@mkdir -p $(TMP_FOLDER)/module

$(TMP_FOLDER)/%.o: $(SDIR)/%.c | prepare
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(MW_SAMPLE_COMMON_PATH)/%.o: $(MW_SAMPLE_COMMON_PATH)/%.c
	$(CC) $(CFLAGS) $(MW_SAMPLE_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COBJS) $(SAMPLE_OBJS) | prepare
	$(CC) -o $@ $(COBJS) $(SAMPLE_OBJS) $(PREBUILT_OBJS) $(ELFFLAGS) $(LOCAL_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(TMP_FOLDER)/%.o: $(CTRL_SRC_DIR)/%.c | prepare
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LOCAL_CPPFLAGS) -c $< -o $@
	@echo [$(notdir $(CXX))] $(notdir $@)

package: all
	@rm -rf install/*
	@mkdir -p install/lib
	@mkdir -p install/tools
	@cp $(TARGET) install/
ifeq ($(ENABLE_CTRL_TOOL), 1)
	@cp isp_daemon_ctrl_tool/$(CTRL_TARGET) install/
endif
	@cp isp_daemon_tool/CviIspTool.sh install/
	@cp isp_daemon_tool/daemon_cfg/* install/
	@cp res/* install/ -rf
	@cp isp_test/scripts/*.sh install/

	@mkdir install/lib/ai
ifeq "$(TDL_SDK_LIB_EXIST)" "noexist"
	$(info tdl_sdk not exist, please export TPU_REL=1 run build_all !!)
else
	@cp -Lrf $(TDL_SDK_INSTALL_PATH)/lib/$(TDL_SDK_LIB_NAME)* install/lib/ai
	@cp -Lrf $(TDL_SDK_INSTALL_PATH)/sample/3rd/opencv/lib/libopencv_core.so* install/lib
	@cp -Lrf $(TDL_SDK_INSTALL_PATH)/sample/3rd/opencv/lib/libopencv_imgcodecs.so* install/lib
	@cp -Lrf $(TDL_SDK_INSTALL_PATH)/sample/3rd/opencv/lib/libopencv_imgproc.so* install/lib
	@cp -Lrf $(TDL_SDK_INSTALL_PATH)/sample/3rd/curl/lib/libcurl.so* install/lib
	@cp -Lrf $(TDL_SDK_INSTALL_PATH)/sample/3rd/libwebsockets/lib/libwebsockets.so* install/lib
endif

	@for so in $(file); \
	do \
		if [ -f $$so ]; then \
			cp -Lrf $$so install/lib > /dev/null 2>&1; \
		else \
			echo "Packing warning: $$so not found!"; \
		fi; \
	done

	@cp -Lrf $(BM_LIB)/libbmlib.so* install/lib
	@cp -Lrf $(BM_LIB)/libbmrt.so* install/lib
	@cp -Lrf $(TPU_KERNEL_LIB)/libtpu_kernel_module.so install/lib
ifeq ($(ENABLE_TOOLS), 1)
	@if [ -d tools/install ]; then cp -rf tools/install/* install/tools/; fi
	@if [ -f tools/run_test_tool.sh ]; then cp tools/run_test_tool.sh install/ && chmod +x install/run_test_tool.sh; fi
endif

ifeq ($(ENABLE_STRIP), 1)
	@echo "Stripping all binaries and libraries..."
	@find install -type f \( -name "*.so*" -o -perm -111 \) -exec sh -c 'file "{}" | grep -q ELF && $(STRIP) "{}" 2>/dev/null || true' \;
else
	@echo "Skipping strip (ENABLE_STRIP=0, debug symbols preserved)"
endif
	@tar -zcf $(OUT_TARBALL) install
	@echo "package: tar $(OUT_TARBALL) successful!"

clean:
	@cd isp_test;make clean
ifeq ($(ENABLE_CTRL_TOOL), 1)
	@cd isp_daemon_ctrl_tool;make clean;cd ..
endif
ifeq ($(ENABLE_TOOLS), 1)
	@cd tools && $(MAKE) clean
endif
	@rm -f $(COBJS) $(SAMPLE_OBJS) $(CDEPS) $(TARGET) $(SAMPLE_DEPS)
	@rm -rf install $(TMP_FOLDER)
	@rm -rf $(OUT_TARBALL)

test:
	@echo "mw sample objectd: " $(SAMPLE_OBJS)
	@echo "mw sample cflags: "  $(MW_SAMPLE_CFLAGS)

-include $(CDEPS) $(SAMPLE_DEPS)
