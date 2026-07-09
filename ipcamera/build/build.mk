
ifeq ($(SRCS), )
SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
endif
ifeq ($(SEXT),cpp)
OBJS := $(SRCS:$(SDIR)/%.cpp=$(ODIR)/%.$(TARGET_MACHINE).$(SOC_NICK_NAME_LOWER).o)
else
OBJS := $(SRCS:$(SDIR)/%.c=$(ODIR)/%.$(TARGET_MACHINE).$(SOC_NICK_NAME_LOWER).o)
endif
DEPS := $(OBJS:%.o=%.d)

CFLAGS += $(INCS)
CXXFLAGS += $(INCS)

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

$(ODIR)/%.$(TARGET_MACHINE).$(SOC_NICK_NAME_LOWER).o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(ODIR)/%.$(TARGET_MACHINE).$(SOC_NICK_NAME_LOWER).o : $(SDIR)/%.cpp
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET_A) : $(OBJS)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(TARGET_SO) : $(OBJS)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(LDFLAGS_SO) -o $@ $^

$(TARGET) : $(OBJS)
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(TARGETFLAGS) $^ $(LIBS) -o $@
	$(Q)$(STRIP) $@

clean:
	$(Q)rm -rf $(TARGET_A)
	$(Q)rm -rf $(TARGET_SO)
	$(Q)rm -rf $(TARGET)

clean_all: clean
	$(Q)if [ -d "$(SDIR)" ]; then \
		echo "rm -rf $(ODIR)"; \
		rm -rf $(ODIR); \
	fi

-include $(DEPS)
