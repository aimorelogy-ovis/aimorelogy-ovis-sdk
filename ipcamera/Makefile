SHELL = /bin/bash
SRCTREE := $(CURDIR)
TARGET_OUT_DIR := $(CURDIR)/out
TOPTARGETS := all clean install clean_all
TOPSUBDIRS := ipcamera
DEFAULT_DEFCONFIG := cv184x_ovis_app_defconfig

export SOC_SEGMENT := $(shell echo $(CHIP_SEGMENT) | tr a-z A-Z)
export SOC_SEGMENT_LOWER := $(shell echo $(SOC_SEGMENT) | tr A-Z a-z)

ifeq ($(SOC_SEGMENT), CV184X)
export SOC_NICK_NAME := CV184X
export SOC_NICK_NAME_LOWER := $(shell echo $(SOC_NICK_NAME) | tr A-Z a-z)
endif

$(info SOC_SEGMENT=$(SOC_SEGMENT))
$(info SOC_NICK_NAME=$(SOC_NICK_NAME))
$(info SOC_NICK_NAME_LOWER=$(SOC_NICK_NAME_LOWER))

## setup path ##
ifeq ($(findstring $(SOC_SEGMENT), CV184X), )
	$(error UNKNOWN chip series - $(SOC_SEGMENT))
endif

TMP_STRING = $(foreach f,$(TOPSUBDIRS),$(findstring $f,$(MAKECMDGOALS)))
PROJECT = $(strip $(TMP_STRING))
export SRCTREE PROJECT TARGET_OUT_DIR

$(PROJECT): .config
	$(MAKE) -C solutions/$@ $(strip $(subst $@,,$(MAKECMDGOALS)))

$(TOPSUBDIRS): $(PROJECT)

$(TOPTARGETS): $(TOPSUBDIRS)

# This application only consumes the selected defconfig as a Makefile include.
# The imported Kbuild helper scripts are not part of this source tree.
.config: $(SRCTREE)/configs/$(DEFAULT_DEFCONFIG)
	$(Q)cp $< $@

all : $(SRCTREE)/.config $(SUBDIRS) $(TARGET)


menuconfig:
	$(MAKE) -f $(SRCTREE)/Kbuild $@
%_defconfig: $(SRCTREE)/configs/%_defconfig
	$(Q)cp $< $(SRCTREE)/.config

include $(SRCTREE)/build/build.mk
.PHONY: $(SUBDIRS)
