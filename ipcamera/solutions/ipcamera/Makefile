include $(SRCTREE)/build/rule.mk
#
TARGET = $(TARGET_OUT_DIR)/$(PROJECT)/$(PROJECT)
#
SEXT	:= c
SDIR	+= $(CURDIR)/src
ifeq ($(SRCS), )
SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
endif
#
ODIR 	+= $(CURDIR)/obj
OBJS 	:= $(SRCS:$(SDIR)/%.c=$(ODIR)/%.$(TARGET_MACHINE).$(SOC_NICK_NAME_LOWER).o)
DEPS 	:= $(OBJS:%.o=%.d)
#
INCS	+= -I$(CURDIR)/include
#
CFLAGS += $(INCS)
CXXFLAGS += $(INCS)
#
ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif
#
SUBDIRS := \
	$(SRCTREE)/modules \
#
all : $(SUBDIRS) $(TARGET)

$(SUBDIRS) :
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(ODIR)/%.$(TARGET_MACHINE).$(SOC_NICK_NAME_LOWER).o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET) : $(OBJS)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(TARGETFLAGS) $^ $(LIBS) -o $@
	$(Q)$(STRIP) $@

clean:
	$(MAKE) -C $(SRCTREE)/modules clean
	$(Q)rm -rf $(TARGET)
	$(Q)rm -rf $(TARGET_OUT_DIR)
	$(Q)rm -rf $(SRCTREE)/install

clean_all: clean
	$(MAKE) -C $(SRCTREE)/modules clean_all
	$(Q)if [ -d "$(SDIR)" ]; then \
		echo "rm -rf $(ODIR)"; \
		rm -rf $(ODIR); \
	fi

-include $(DEPS)
-include $(SRCTREE)/build/install.mk
.PHONY: $(SUBDIRS)