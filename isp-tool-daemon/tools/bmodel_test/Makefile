SHELL = /bin/bash

# Get top directory
ifeq ($(TOP_DIR),)
	TOP_DIR := $(shell cd ../../.. && pwd)
endif

# Import parameter configuration
ifeq ($(PARAM_FILE), )
	PARAM_FILE := $(TOP_DIR)/cvi_mpi/Makefile.param
	include $(PARAM_FILE)
endif

# Source and object files
SDIR = .
TMP_FOLDER = tmp
TARGET = bmodel_test

# Collect all source files
SRCS = $(wildcard $(SDIR)/*.c)
OBJS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.o, $(SRCS))
DEPS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.d, $(SRCS))

# Include paths
INCS = -I$(TOP_DIR)/libsophon/bmlib/include
INCS += -I$(TOP_DIR)/libsophon/tpu-runtime/include

# Compile flags
LOCAL_CFLAGS = $(INCS) -MMD -MP

# Libraries
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

BM_LIB = $(OUTPUT_DIR)/rootfs/system/lib

LIBS = -L$(BM_LIB) -lbmrt -lbmlib
LOCAL_LDFLAGS = $(LIBS) -lm -lpthread -shared-libgcc

# Installation directory (under test_tool)
INSTALL_DIR = ../install

.PHONY: all clean install

all: prepare $(TARGET) install

prepare:
	@mkdir -p $(TMP_FOLDER)

$(TMP_FOLDER)/%.o: $(SDIR)/%.c | prepare
	@$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@
	@echo "[CC] $(notdir $@)"

$(TARGET): $(OBJS)
	@$(CC) -o $@ $^ $(ELFFLAGS) $(LOCAL_LDFLAGS)
	@echo "[LINK] $@"

install: $(TARGET)
	@mkdir -p $(INSTALL_DIR)
	@cp $(TARGET) $(INSTALL_DIR)/
	@echo "Installed $(TARGET) to $(INSTALL_DIR)/"

clean:
	@rm -rf $(TMP_FOLDER) $(TARGET)
	@echo "Cleaned $(TARGET)"

-include $(DEPS)
