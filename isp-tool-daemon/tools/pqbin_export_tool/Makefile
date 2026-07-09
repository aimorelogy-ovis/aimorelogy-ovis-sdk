SHELL = /bin/bash

# Get top directory
ifeq ($(TOP_DIR),)
	TOP_DIR := $(shell cd ../../.. && pwd)
endif

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=$(TOP_DIR)/cvi_mpi/Makefile.param
	include $(PARAM_FILE)
endif

CHIP_ARCH_L ?= $(shell echo $(CHIP_ARCH) | tr A-Z a-z)

SDIR = .
TMP_FOLDER = tmp
ISP_DIR = $(TOP_DIR)/cvi_mpi/modules/isp
ISP_COMMON_DIR = $(ISP_DIR)/common

INCS = -I$(KERNEL_INC) -I$(MW_INC) -I$(ISP_INC)
INCS += -I$(ISP_COMMON_DIR)/clog
INCS += -I$(ISP_DIR)/$(CHIP_ARCH_L)/isp-daemon2/inc

COBJS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.o, $(wildcard $(SDIR)/*.c))
CDEPS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.d, $(wildcard $(SDIR)/*.c))

TARGET = pqbin_export_tool

LIBS += -lisp -lisp_algo -lae -lawb -laf -lcvi_bin -lcvi_ispd2
STATIC_LIBS = -static

LOCAL_CFLAGS = $(DEFS) $(INCS)
LOCAL_CFLAGS += -MMD -MP
LOCAL_LDFLAGS = -L$(MW_LIB) $(STATIC_LIBS) -Wl,--start-group $(LIBS) -lpthread -lm -Wl,--end-group

INSTALL_DIR = ../install

.PHONY: clean all install
all: prepare $(TARGET) install

prepare:
	@mkdir -p $(TMP_FOLDER)

$(TMP_FOLDER)/%.o: $(SDIR)/%.c
	@$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@
	@echo "[CC] $(notdir $@)"

$(TARGET): $(COBJS)
	@$(CC) -o $@ $(COBJS) $(LOCAL_LDFLAGS)
	@echo "[LINK] $(notdir $@)"

install: $(TARGET)
	@mkdir -p $(INSTALL_DIR)
	@cp $(TARGET) $(INSTALL_DIR)/
	@echo "Installed $(TARGET) to $(INSTALL_DIR)/"

clean:
	@rm -f $(COBJS) $(CDEPS) $(TARGET)

-include $(CDEPS)
