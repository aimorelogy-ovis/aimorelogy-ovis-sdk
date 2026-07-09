# *
# * Copyright (C) 2024 Cvitek Co., Ltd.  All rights reserved.
# *
# * File Name: common/common_lib.mk
# * Description:
# * Author: wangliang.wang@sophgo.com
# *
#
include $(COMMON_DIR)/base.mk

TOP_CONFIG_FILE ?=
#
include $(TOP_CONFIG_FILE)
#
# assumptions
# - SEXT is defined (c or cpp)
# - SDIR is defined
# - SRCS is optional
# - if INCS is defined, will be add to CFLAGS or CXXFLAGS
# - if LIBS is defined, will be add to LDFLAGS
ifeq ($(SRCS_C), )
ifeq ($(SRCS_CPP), )
$(error "no source code file, please check!!")
endif
endif

# Customer set
ODIR	:= $(CURDIR)/obj
OBJS	:= $(SRCS_C:$(SDIR)/%.c=$(ODIR)/%.$(TARGET_MACHINE).o)
OBJS	+= $(SRCS_CPP:$(SDIR)/%.cpp=$(ODIR)/%.$(TARGET_MACHINE).o)
#
ARFLAGS		:= rcs
LDFLAGS_SO	:= -shared
CFLAGS		+= $(INCS)
CXXFLAGS	+= $(INCS)
#

.PHONY : all clean clean_all install $(TARGET_DIR)

all : $(TARGET_A) $(TARGET_SO) $(TARGET)

$(ODIR)/%.$(TARGET_MACHINE).o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<
	$(Q)echo [$(notdir $(CC))] $(notdir $@)

$(ODIR)/%.$(TARGET_MACHINE).o : $(SDIR)/%.cpp
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) -o $@ -c $<
	$(Q)echo [$(notdir $(CXX))] $(notdir $@)

$(TARGET_DIR) :
	$(Q)mkdir -p $@
	$(Q)$(CC) -v 2> $@/Target:"$(TARGET_MACHINE)".txt


$(TARGET_A) : $(OBJS) | $(TARGET_DIR)
	$(Q)$(AR) $(ARFLAGS) $@ $^
	$(Q)echo $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO) : $(OBJS) | $(TARGET_DIR)
	$(Q)$(CC) $(LDFLAGS_SO) -o $@ $^
	$(Q)echo $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

$(TARGET) : $(OBJS) | $(TARGET_DIR)
ifeq ($(CONFIG_STATIC), y)
	$(Q)$(CC) $(ELFFLAGS) -o $@ $^ -Wl,-Bstatic $(LIBS) -Wl,-Bdynamic $(DYN_LIBS)
else
	$(Q)$(CC) $(ELFFLAGS) -o $@ $^ $(LIBS) $(DYN_LIBS)
endif
	$(Q)$(STRIP) $@

clean:
	rm -rf $(TARGET_DIR)

clean_all: clean
	$(Q)if [ -d "$(SDIR)" ]; then \
		echo "rm -rf $(ODIR)"; \
		rm -rf $(ODIR); \
	fi

$(INSTALL_PATH)/lib :
	$(Q)mkdir -p $@

install: $(TARGET_A) $(TARGET_SO) | $(INSTALL_PATH)/lib
	$(Q)if [ -n "$(strip $^)" ]; then \
        cp $^ $(INSTALL_PATH)/lib; \
    fi
