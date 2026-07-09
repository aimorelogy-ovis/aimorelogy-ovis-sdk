SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../../Makefile.param
include $(PARAM_FILE)
endif

# include ../../self_test.mk
include ../audio_ut.mk

SDIR := $(CURDIR)
SRCS := $(wildcard $(SDIR)/*.c)
# SRCS += ../audio_utility/*.c
INCS := -I$(MW_INC) -I../../sample/common -I$(KERNEL_INC) -I$(MW_PATH)/modules/audio/common -I../audio_utility/
OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.d)


TARGET = sample_audio_aec

AUD_LIB = -lcvi_audio -lcvi_vqe -lcvi_VoiceEngine -lcvi_RES1  -ltinyalsa -lm -lpthread  -ldl -lcvi_dnvqe -lsys
AUD_LIB += -lpthread -pthread
LIBS = $(AUD_LIB)

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif

ifeq ($(USE_NEXT_SSP_DUAL), yes)

ifeq ($(USE_CVIAUDIO_STATIC), yes)
AUD_LIB += -lcvi_ssp -lcvi_audio
endif
else
ifeq ($(USE_CVIAUDIO_STATIC), yes)
AUD_LIB += -lcvi_ssp
AUD_LIB += -lcvi_ssp2
endif
AUD_LIB += -ldnvqe
endif

ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV181X CV180X SG200X))
AUD_LIB += -lsys
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
ifeq ($(USE_ALIOS_AUD_DRV), yes)
DEFS += -DUSE_ALIOS_AUD_DRV
endif

EXTRA_CFLAGS += $(DEFS) $(INCS)  -Wno-unused-parameter
EXTRA_LDFLAGS += $(LIBS)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration
AEC_TEST_LDLIBS =

all : $(TARGET) post_remove

post_remove:
	@rm -f ../audio_utility/*.o
	@rm -f ../audio_utility/*.d

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(CFLAGS2) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS) $(AEC_TEST_LDLIBS) $(STATIC_FLAG)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)


.PHONY : clean
clean:
	echo "clean...audio_aec"
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
