SHELL = /bin/bash

LIB_RLS_DIR :=
ifeq ($(DUAL_OS),y)
LIB_RLS_DIR = lib/$(SDK_VER)_dual
else
LIB_RLS_DIR = lib/$(SDK_VER)_single
endif
ifeq ($(wildcard $(LIB_RLS_DIR)),)
$(error $(LIB_RLS_DIR) not exist!)
else
$(shell cp -rlf $(LIB_RLS_DIR)/* lib/)
endif

chip_arch = $(shell echo $(CHIP_ARCH) | tr A-Z a-z)
MEDIA_INCLUDE_DIR = $(BUILD_PATH)/media/include

ifeq ($(PARAM_FILE), )
     PARAM_FILE:=Makefile.param
     include $(PARAM_FILE)
endif

$(info ** [ CHIP_ARCH ] ** = $(CHIP_ARCH))
$(info ** [ SDK_VER ] ** = $(SDK_VER))
$(info ** [ CROSS_COMPILE ] ** = $(CROSS_COMPILE))
$(info ** [ OS_TYPE ] ** = $(OS_TYPE))

ifeq ($(OS_TYPE), DUAL_OS)
OS_MAKE_FILE:=dual_os.mk
else
OS_MAKE_FILE:=linux.mk
endif
include $(OS_MAKE_FILE)

ifeq ($(DESTDIR),)
    DESTDIR := $(shell pwd)/install
endif


.PHONY: 3rdparty SensorSupportList module sample_app install uninstall clean


3rdparty:
	@make -C 3rdparty/

ifeq ($(OS_TYPE), DUAL_OS)
SensorSupportList: prepare
	@cd ../build/media/SensorSupportList/sensor_cfg && make
	@echo "SensorSupportList sensor is no need build in dual os"
else
SensorSupportList: prepare
	@make -C ../build/media/SensorSupportList/ all
endif

module: prepare 3rdparty SensorSupportList
	@make -C modules/

ifeq ($(OS_TYPE), DUAL_OS)
sample_app: module SensorSupportList
	@make -C sample_app/
else
sample_app: module SensorSupportList
	@make -C sample_app/
endif

install:
	@mkdir -p $(DESTDIR)/usr/bin
	@mkdir -p $(DESTDIR)/usr/lib/3rd
	# copy mw lib
	@cp -a lib/*.so*  $(DESTDIR)/usr/lib
	@cp -a lib/3rd/*.so*  $(DESTDIR)/usr/lib/3rd

uninstall:
	@rm $(DESTDIR) -rf

clean:
	@make -C 3rdparty/ clean
	@make -C modules/ clean
	@make -C sample_app/ clean
	@make -f $(OS_MAKE_FILE) unprepare;
	@make -C ../build/media/SensorSupportList/ clean
