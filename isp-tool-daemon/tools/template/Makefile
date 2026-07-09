SHELL = /bin/bash

# ========================================================================
# Makefile Template for New Test Tools
# ========================================================================
# Instructions:
# 1. Change TARGET variable to your test tool name
# 2. Add include paths to INCS as needed
# 3. Add libraries to LIBS as needed
# 4. Add new directory name to TEST_DIRS in test_tool/Makefile
# ========================================================================

# Get top directory
ifeq ($(TOP_DIR),)
	TOP_DIR := $(shell cd ../../.. && pwd)
endif

# Import parameter configuration
ifeq ($(PARAM_FILE), )
	PARAM_FILE := $(TOP_DIR)/cvi_mpi/Makefile.param
	include $(PARAM_FILE)
endif

# ======== MODIFY HERE ========
# Target name (change to your test tool name)
TARGET = template
# =============================

# Source and object files
SDIR = .
TMP_FOLDER = tmp

# Collect all source files (auto-include all .c files in current dir)
SRCS = $(wildcard $(SDIR)/*.c)
OBJS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.o, $(SRCS))
DEPS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.d, $(SRCS))

# ======== MODIFY INCLUDE PATHS AS NEEDED ========
# Common include path examples (add/remove as needed)
INCS =
# INCS += -I$(KERNEL_INC) -I$(MW_INC) -I$(ISP_INC)
# INCS += -I$(TOP_DIR)/cvi_mpi/sample_app/common
# INCS += -I$(TOP_DIR)/libsophon/bmlib/include
# INCS += -I$(TOP_DIR)/libsophon/tpu-runtime/include
# ================================================

# Compile flags
LOCAL_CFLAGS = $(INCS) -MMD -MP

# ======== MODIFY LIBRARIES AS NEEDED ========
# Common library examples (add/remove as needed)
LIBS = -lm -lpthread
# LIBS += -L$(OUTPUT_DIR)/rootfs/system/lib -lbmrt -lbmlib
# ============================================

LOCAL_LDFLAGS = $(LIBS) -shared-libgcc

# Installation directory (template not installed)
INSTALL_DIR = ../install

.PHONY: all clean install

all: prepare $(TARGET)
	@echo "$(TARGET) built (template is not installed)"

prepare:
	@mkdir -p $(TMP_FOLDER)

$(TMP_FOLDER)/%.o: $(SDIR)/%.c | prepare
	@$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@
	@echo "[CC] $(notdir $@)"

$(TARGET): $(OBJS)
	@$(CC) -o $@ $^ $(ELFFLAGS) $(LOCAL_LDFLAGS)
	@echo "[LINK] $@"

# Template is not installed to install directory
install:
	@echo "Template is not installed (reference only)"

clean:
	@rm -rf $(TMP_FOLDER) $(TARGET)
	@echo "Cleaned $(TARGET)"

-include $(DEPS)
