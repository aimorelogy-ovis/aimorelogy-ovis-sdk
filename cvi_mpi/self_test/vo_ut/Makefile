SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
SRCS += $(wildcard $(SDIR)/../common/*.c)
INCS = -I$(MW_INC) -I$(SDIR)/../common/
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = vo_ut

LIBS = -lvo -lvpss -lsys

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ELFFLAGS += -static
endif

EXTRA_CFLAGS += $(DEFS) $(INCS)
ldflags-y += $(LIBS) -lpthread -lm

.PHONY : clean

all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
	@cp ../../sample_app/panel/sample_panel ../auto_test/
	@cp $(TARGET) ../auto_test/

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)
