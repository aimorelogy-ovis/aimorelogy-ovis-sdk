SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=$(TOP_DIR)/cvi_mpi/Makefile.param
	include $(PARAM_FILE)
endif

CHIP_ARCH_L ?= $(shell echo $(CHIP_ARCH) | tr A-Z a-z)

SDIR = $(PWD)
TMP_FOLDER = tmp
ISP_DIR = $(TOP_DIR)/cvi_mpi/modules/isp
ISP_COMMON_DIR = $(ISP_DIR)/common

INCS = -I$(KERNEL_INC) -I$(MW_INC) -I$(ISP_INC)
INCS += -I$(ISP_COMMON_DIR)/clog
INCS += -I$(ISP_COMMON_DIR)/raw_dump/inc
INCS += -I$(ISP_COMMON_DIR)/raw_dump/src
INCS += -I$(ISP_DIR)/$(CHIP_ARCH_L)/isp-daemon2/inc

COBJS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.o, $(wildcard $(SDIR)/*.c))
CDEPS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.d, $(wildcard $(SDIR)/*.c))

TARGET = isp_tool_daemon_ctrl

LIBS += -lcvi_ispd2 -lisp_ipc_client -lsys -lvo -lvpss -lvi -lcvi_bin
STATIC_LIBS = -static

LOCAL_CFLAGS = $(DEFS) $(INCS)
LOCAL_CFLAGS += -MMD -MP
LOCAL_LDFLAGS = -L$(MW_LIB) $(STATIC_LIBS) -Wl,--start-group $(LIBS) -lpthread -lm -Wl,--end-group

.PHONY: clean all
all: prepare $(TARGET)

prepare:
	@mkdir -p $(TMP_FOLDER)

$(TMP_FOLDER)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COBJS)
	$(CC) -o $@ $(COBJS) $(LOCAL_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(COBJS) $(CDEPS) $(TARGET)

-include $(CDEPS)
