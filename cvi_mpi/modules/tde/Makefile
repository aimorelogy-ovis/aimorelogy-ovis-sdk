SHELL = /bin/bash

ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif


SDIR = $(PWD)
PLATFORM_DIR = $(SDIR)/platform/linux
SRCS := $(wildcard $(SDIR)/api/*.c)
SRCS += $(wildcard $(PLATFORM_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

SRCS_ASM =
OBJS_ASM = $(SRCS_ASM:.S=.o)
DEPS_ASM = $(SRCS_ASM:.S=.d)

INCS = -I$(OSAL_INC) -I$(MW_INC) -I$(ISP_INC) -I./include -I$(PLATFORM_DIR)

## INCLUDE PATH ##
UAPI_INC = $(MW_PATH)/modules/uapi
SYS_INC = $(MW_PATH)/modules/sys/platform/linux
INCS += -I$(UAPI_INC) -I$(SYS_INC)


EXTRA_CFLAGS = $(INCS) -fpack-struct=8
EXTRA_CFLAGS += -DSDK_VER=\"$(SDK_VER)\"
ifeq ($(SDK_VER), 32bit)
EXTRA_CFLAGS += -D_FILE_OFFSET_BITS=64
else ifeq ($(SDK_VER), glibc_arm)
EXTRA_CFLAGS += -D_FILE_OFFSET_BITS=64
endif

LDFLAGS  = -shared -fPIC -Wl,--gc-sections -export-dynamic -L$(MW_LIB) -L$(MW_3RD_LIB) $(CVI_TARGET_PACKAGES_LIBDIR)


TARGET_A = $(MW_LIB)/libtde.a
TARGET_SO = $(MW_LIB)/libtde.so

include ../module.mk