SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../../Makefile.param
include $(PARAM_FILE)
endif

# include ../../self_test.mk
include ../audio_ut.mk

# COMMON_DIR := $(MW_PATH)/sample/common
# COMM_INC := $(COMMON_DIR)
SDIR := $(CURDIR)
#SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(CURDIR)/aac/ -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC) -I ./include -I$(COMM_INC) -I$(SENSOR_LIST_INC)
#OBJS = $(SRCS:.c=.o)
#DEPS = $(SRCS:.c=.d)
DEFS += -DSUPPORT_EXTERNAL_AAC


ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
APLAY_TARGET = cvi_auto_play_asan
ACAP_TARGET = cvi_auto_record_asan
SPECIAL_TEST = cvi_auto_special_asan
else
APLAY_TARGET = cvi_auto_play
ACAP_TARGET = cvi_auto_record
SPECIAL_TEST = cvi_auto_special
endif

APLAY_OBJ = cvi_auto_player.o cvi_audio_aac_adp.o ../audio_resample/cvi_audio_dl_adp.o
ACAP_OBJ = cvi_auto_recorder.o cvi_audio_aac_adp.o ../audio_resample/cvi_audio_dl_adp.o
SPECIAL_OBJ = cvi_auto_special_test.o cvi_audio_aac_adp.o ../audio_resample/cvi_audio_dl_adp.o

AUD_LIB = -lcvi_audio -lcvi_vqe -lcvi_ssp -lcvi_VoiceEngine -lcvi_RES1 -ltinyalsa -lm -lpthread -pthread -ldl
AUD_LIB += -lcvi_dnvqe -lcvi_ssp2
LIBS = $(AUD_LIB)
LIBS += -laacdec2
LIBS += -laacsbrdec2
LIBS += -laacenc2
LIBS += -laaccomm2
LIBS += -laacsbrenc2

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
USE_CVIAUDIO_STATIC = no
endif

LIBS += -lsys

ifeq ($(USE_CVIAUDIO_STATIC), yes)
DEFS += -DCVIAUDIO_STATIC
STATIC_FLAG = -static
else
STATIC_FALG =
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

EXTRA_LDFLAGS += -lrt
EXTRA_CFLAGS += $(DEFS) $(INCS)  -Wno-unused-parameter
EXTRA_LDFLAGS += $(LIBS)
CFLAGS2  = -fPIC $(OPT_LEVEL) -Wno-implicit-function-declaration

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
EXTRA_LDFLAGS_WITH_ASAN = -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address -lasan
EXTRA_LDFLAGS +=$(EXTRA_LDFLAGS_WITH_ASAN)
endif
ifeq ($(USE_ALIOS_AUD_DRV), yes)
DEFS += -DUSE_ALIOS_AUD_DRV
endif

all : $(APLAY_TARGET) $(ACAP_TARGET) $(SPECIAL_TEST)

%.o: %.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(APLAY_TARGET): $(APLAY_OBJ)
	@$(CXX) -o $@ $(APLAY_OBJ) $(ELFFLAGS) $(STATIC_FLAG) $(EXTRA_LDFLAGS) $(STATIC_FLAG)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(ACAP_TARGET): $(ACAP_OBJ)
	@$(CXX) -o $@ $(ACAP_OBJ) $(ELFFLAGS) $(STATIC_FLAG) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(SPECIAL_TEST): $(SPECIAL_OBJ)
	@$(CXX) -o $@ $(SPECIAL_OBJ) $(ELFFLAGS) $(STATIC_FLAG) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

.PHONY : clean
clean:
	@rm -f *.o *.d $(DEPS) $(COMM_DEPS) $(APLAY_TARGET) $(ACAP_TARGET) $(SPECIAL_TEST)

-include $(DEPS)

