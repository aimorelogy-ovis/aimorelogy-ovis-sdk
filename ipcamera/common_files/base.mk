# *
# * Copyright (C) 2024 Cvitek Co., Ltd.  All rights reserved.
# *
# * File Name: common/base.mk
# * Description:
# * Author: wangliang.wang@sophgo.com
# *
#
CROSS_COMPILE ?=
#
CC        := $(CROSS_COMPILE)gcc
CXX       := $(CROSS_COMPILE)g++
AS        := $(CROSS_COMPILE)as
LD        := $(CROSS_COMPILE)ld
CPP       := $(CC) -E
AR        := $(CROSS_COMPILE)ar
NM        := $(CROSS_COMPILE)nm
STRIP     := $(CROSS_COMPILE)strip
OBJCOPY   := $(CROSS_COMPILE)objcopy
OBJDUMP   := $(CROSS_COMPILE)objdump
AWK       := awk
PERL      := perl
PYTHON    := python
CHECK     := sparse
MAKE      := make
CMAKE     := cmake
# riscv64-unknown-linux-musl, riscv64-unknown-linux-gnu, aarch64-none-linux-gnu, arm-linux-gnueabihf gcc
TARGET_MACHINE  := $(shell ${CROSS_COMPILE}gcc -dumpmachine)
#
ifeq ($(notdir $(CROSS_COMPILE)), riscv64-unknown-linux-musl-)
CFLAGS		+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
CXXFLAGS	+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
ELFFLAGS 	+= -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
else ifeq ($(notdir $(CROSS_COMPILE)), riscv64-unknown-linux-gnu-)
CFLAGS		+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
CXXFLAGS	+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
ELFFLAGS 	+= -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
endif
#
ifeq ("$(origin V)", "command line")
  BUILD_VERBOSE = $(V)
endif
ifeq ($(BUILD_VERBOSE),1)
  Q =
else
  Q = @
endif
## COLOR ##
BLACK   = "\e[30;1m"
RED     =  "\e[31;1m"
GREEN   = "\e[32;1m"
YELLOW  = "\e[33;1m"
BLUE    = "\e[34;1m"
PURPLE  = "\e[35;1m"
CYAN    = "\e[36;1m"
WHITE   = "\e[37;1m"
END     = "\e[0m"
# Compile flags
OPT_CFLAGS	:= -O3
OPT_CFLAGS	+= -MMD
# for code debug
#OPT_CFLAGS += -save-temps=obj
CFLAGS		+= -g -fPIC -Wall -Wextra -Werror -std=gnu11 -D_GNU_SOURCE -Wno-stringop-truncation \
				-Wno-format-truncation -Wno-type-limits -Wno-enum-conversion -Wno-implicit-fallthrough \
				-Wno-stringop-overflow -Wno-sizeof-pointer-memaccess
CXX_STD		?= -std=gnu++11
CXXFLAGS	+= -g -fPIC -Wall -Wextra -Werror $(CXX_STD) -D_GNU_SOURCE -Wno-stringop-truncation
CFLAGS		+= $(OPT_CFLAGS) -D__FILENAME__=\"$(notdir $<)\"
CXXFLAGS	+= $(OPT_CFLAGS) -D__FILENAME__=\"$(notdir $<)\"
#
