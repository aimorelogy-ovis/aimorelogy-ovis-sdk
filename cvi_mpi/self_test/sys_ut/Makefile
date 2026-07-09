SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include $(BUILD_PATH)/.config

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
SRCS += $(wildcard $(SDIR)/../common/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

INCS = -I$(MW_INC) -I$(SDIR)/../common/

TARGET = sys_ut
LIBS = -lsys

EXTRA_CFLAGS += $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

EXTRA_LDFLAGS += $(LIBS) -lpthread -lm -latomic

ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ELFFLAGS += -static
endif

.PHONY : clean all

all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS)  $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $(TARGET) ../auto_test/


clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

