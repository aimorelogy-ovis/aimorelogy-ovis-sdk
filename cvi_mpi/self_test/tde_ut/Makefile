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

TARGET = tde_ut
LIBS = -ltde -lsys

EXTRA_CFLAGS += $(DEFS) $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

ldflags-y += $(LIBS) -lpthread -lm -lini -latomic

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common)
else
ELFFLAGS += -static
endif

all : tde_ut

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

	#/mnt/workspace/mars_daily_32_2/host-tools/gcc/gcc-linaro-12.0.0-2021.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -fanalyzer -fdump-analyzer-callgraph $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

tde_ut : $(OBJS) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $@ ../auto_test/

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) tde_ut

-include $(DEPS)
