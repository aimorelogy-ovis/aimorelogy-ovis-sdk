SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../../Makefile.param
include $(PARAM_FILE)
endif

# include ../../self_test.mk
include ../audio_ut.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I../common/ -I$(MW_INC)/../3rdparty/lame/include
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = cvi_mp3recorder

EXTRA_CFLAGS = $(INCS)
EXTRA_LDFLAGS += -lmp3lame -lcvi_audio
EXTRA_LDFLAGS +=  -lm -lpthread  -ldl -lcvi_mp3enc -lcvi_VoiceEngine
EXTRA_LDFLAGS +=  -lcvi_vqe -ltinyalsa -lcvi_RES1

ifeq ($(SELF_TEST_STATIC),1)
LIB_DEP := $(MW_LIB)/3rd/libmp3lame.a
else
LIB_DEP := $(MW_LIB)/3rd/libmp3lame.so
endif


ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
EXTRA_LDFLAGS += -lnanomsg
endif

ifeq ($(MULTI_PROCESS_SUPPORT_AUDIO), yes)
DEFS += -DRPC_MULTI_PROCESS_AUDIO
EXTRA_LDFLAGS += -lnanomsg
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
EXTRA_LDFLAGS += -L $(MW_PATH)/3rdparty/nanomsg/lib_32
endif
ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
EXTRA_LDFLAGS += -L $(MW_PATH)/3rdparty/nanomsg/lib_64
endif
endif


.PHONY : clean all
all : $(TARGET)

$(SDIR)/%.o : $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS) $(LIB_DEP)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
