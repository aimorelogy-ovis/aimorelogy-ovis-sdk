ifneq ($(BUILD_PATH),)
include $(BUILD_PATH)/.config
endif

ifeq ($(CONFIG_ENABLE_SDK_ASAN),y)
    CFLAGS += -g -fsanitize=address -fno-omit-frame-pointer
    LDFLAGS += -lasan
endif

ifeq ($(SDK_VER), glibc_riscv64)
  CFLAGS += -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
endif
ifeq ($(SDK_VER), musl_riscv64)
  CFLAGS += -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
endif

CFLAGS += -O3 -Wall  -Werror=all -DNO_OPENSSL=1
CPPFLAGS = -fPIC
CXXFLAGS = $(CPPFLAGS)

LIVE555_DIR ?= /system
LIVE555_INC ?= -I$(LIVE555_DIR)/include/BasicUsageEnvironment -I$(LIVE555_DIR)/include/groupsock -I$(LIVE555_DIR)/include/liveMedia -I$(LIVE555_DIR)/include/UsageEnvironment
LIVE555_LIB ?= $(LIVE555_DIR)/lib/libliveMedia.a $(LIVE555_DIR)/lib/libBasicUsageEnvironment.a $(LIVE555_DIR)/lib/libUsageEnvironment.a $(LIVE555_DIR)/lib/libgroupsock.a
