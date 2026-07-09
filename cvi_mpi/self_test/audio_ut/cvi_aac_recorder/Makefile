SHELL = /bin/bash
#
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../../Makefile.param
include $(PARAM_FILE)
endif
#
# include ../../self_test.mk
include ../audio_ut.mk
#
EXENAME = cvi_aac_recorder
SDIR := $(CURDIR)
MW_PATCH = $(CURDIR)/../../..
C_SRCS = $(wildcard $(SDIR)/*.c)
C_OBJS = $(C_SRCS:.c=.o)
#
ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
CC = aarch64-linux-gnu-gcc
LIBS = -L$(CURDIR)/aac_lib/aarch64-linux-gnu -lfdkaac_enc
LIBS += -L$(MW_PATCH)/3rdparty/libaac/aarch64-linux-gnu -lcvi_aac_enc
endif
#
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
CC = arm-linux-gnueabihf-gcc
LIBS = -L$(CURDIR)/aac_lib/arm-linux-gnueabihf -lfdkaac_enc
LIBS += -L$(MW_PATCH)/3rdparty/libaac/arm-linux-gnueabihf -lcvi_aac_enc
endif

DEFINES =
#INC = -I$(MW_INC) -I$(CURDIR)/
INCS = -I$(MW_INC) -I../../sample/common -I$(KERNEL_INC) -I ./include -I ./
LIBS += -L$(MW_PATCH)/lib -lcvi_audio -ldl -lcvi_VoiceEngine -lcvi_vqe -ltinyalsa -lcvi_RES1 -lm -lpthread  -ldl

##CFLAGS = -g -Wall -O0 -fPIC -fgnu89-inline $(DEFINES) $(INC)
##LDFLAGS = -lpthread -pthread  -lm

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

ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/arm-linux-gnueabihf/
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/aac_enc/arm-linux-gnueabihf/
endif
ifeq ($(TARGET_MACHINE), aarch64-linux-gnu)
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/aarch64-linux-gnu/
EXTRA_LDFLAGS += -L $(MW_PATH)/modules/audio/src/cvifdk_aac/aac_enc/aarch64-linux-gnu/
endif

EXTRA_LDFLAGS += -L $(LIBS)
EXTRA_CFLAGS += $(DEFINES) $(INCS)  -Wno-unused-parameter

DEPS = $(C_SRCS:.c=.d)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration


all: $(EXENAME)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CFLAGS2) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

#$(EXENAME): $(C_OBJS)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^
$(EXENAME) : $(C_OBJS)
	@$(CXX) -o $@ $(C_OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)


.PHONY : clean
clean:
	@rm -f $(C_OBJS) $(DEPS) $(COMM_DEPS) $(EXENAME)

-include $(DEPS)


