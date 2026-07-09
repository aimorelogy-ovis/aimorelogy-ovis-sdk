SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../../Makefile.param
include $(PARAM_FILE)
endif

# include ../../self_test.mk
include ../audio_ut.mk

SDIR := $(CURDIR)
ifeq ($(USE_CVIAUDIO_STATIC), yes)
SRCS = $(filter-out $(SDIR)/cvi_audio_dl_adp.c, $(wildcard $(SDIR)/*.c))
else
SRCS = $(wildcard $(SDIR)/*.c)
endif
INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(MW_PATH)/modules/audio/common
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)


TARGET = sample_audio_resample
TARGET_WITH_ASAN =

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET_WITH_ASAN = sample_audio_resample_asan
endif

RES_LIB = -lcvi_RES1 -lm -lpthread  -ldl
LIBS = $(RES_LIB)

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif

ifeq ($(MULTI_PROCESS_SUPPORT_AUDIO), yes)
DEFS += -DRPC_MULTI_PROCESS_AUDIO
LIBS += -lnanomsg
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
EXTRA_LDFLAGS += -L $(MW_PATH)/3rdparty/nanomsg/lib_32
endif
ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
EXTRA_LDFLAGS += -L $(MW_PATH)/3rdparty/nanomsg/lib_64
endif
endif
EXTRA_LDFLAGS +=  -L $(MW_LIB)

ifeq ($(USE_CVIAUDIO_STATIC), yes)
DEFS += -DCVIAUDIO_STATIC
STATIC_FLAG = -static
else
STATIC_FALG =
endif

EXTRA_CFLAGS += $(DEFS) $(INCS)  -Wno-unused-parameter
EXTRA_LDFLAGS += $(LIBS)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration

EXTRA_WITH_ASAN = -lasan

all : $(TARGET) $(TARGET_WITH_ASAN)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CFLAGS2) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS) $(STATIC_FLAG)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(TARGET_WITH_ASAN) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_WITH_ASAN) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)


.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET) $(TARGET_WITH_ASAN)
	@rm -rf *.d
	@rm -rf *.o

-include $(DEPS)
