.PHONY: CREATE_DIR
SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../../Makefile.param
include $(PARAM_FILE)
endif


FDK_AAC_DIR=./fdkaac
CUR = $(CURDIR)

#aac common  -------------------------------------------start
AACCOMM_SRC =
AACCOMM_SRC += $(wildcard $(CURDIR)/fdkaac/libSYS/src/*.cpp)
AACCOMM_SRC += $(wildcard $(CURDIR)/fdkaac/libPCMutils/src/*.cpp)
AACCOMM_SRC += $(wildcard $(CURDIR)/fdkaac/libFDK/src/*.cpp)

AACCOMM_INCS =
AACCOMM_INCS += -I ./fdkaac/libFDK/include/
AACCOMM_INCS += -I ./fdkaac/libSYS/include/
AACCOMM_INCS += -I ./fdkaac/libPCMutils/include/
AACCOMM_INCS += -I $(KERNEL_INC)
OBJS_COMM = $(AACCOMM_SRC:.cpp=.o)
#aac encoder -------------------------------------------end

#aac sbr encoder -------------------------------------------start
AACSBRENC_SRC =
AACSBRENC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libSBRenc/src/*.cpp)
AACSBRENC_SRC += $(AACSBRENC_SRC_EXTRA)

OBJS_SBRENC = $(AACSBRENC_SRC:.cpp=.o)
#aac sbr encoder -------------------------------------------end

#sbr decoder -------------------------------------------start
AACSBRDEC_SRC =
AACSBRDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libSBRdec/src/*.cpp)
AACSBRDEC_SRC += $(AACSBRDEC_SRC_EXTRA)
OBJS_SBRDEC = $(AACSBRDEC_SRC:.cpp=.o)
#sbr decoder -------------------------------------------end

#aac encoder -------------------------------------------start
SYS_SRC = \
    libSYS/src/genericStds.cpp \
    libSYS/src/syslib_channelMapDescr.cpp

AACENC_SRC =  $(wildcard $(CURDIR)/fdkaac/libAACenc/src/*.cpp)
AACENC_SRC_EXTRA = $(wildcard $(CURDIR)/fdkaac/libSACenc/src/*.cpp)
#AACENC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libSBRenc/src/*.cpp)
AACENC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libMpegTPEnc/src/*.cpp)
##AACENC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libSYS/src/*.cpp)
##AACENC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libPCMutils/src/*.cpp)
##AACENC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libFDK/src/*.cpp)

AACENC_SRC += $(AACENC_SRC_EXTRA)
AACENC_SRC += ./aacenc_interface.cpp

AACENC_INCS =
AACENC_INCS += -I ./fdkaac/libFDK/include/
AACENC_INCS += -I ./fdkaac/libSYS/include/
AACENC_INCS += -I ./fdkaac/libMpegTPEnc/include/
AACENC_INCS += -I ./fdkaac/libSBRenc/include/
AACENC_INCS += -I ./fdkaac/libAACenc/include/
AACENC_INCS += -I ./fdkaac/libSACenc/include/
AACENC_INCS += -I ./fdkaac/libPCMutils/include/
AACENC_INCS += -I $(KERNEL_INC) -I$(MW_INC)
OBJS_ENC = $(AACENC_SRC:.cpp=.o)
#aac encoder -------------------------------------------end

#aac decoder -------------------------------------------start
AACDEC_SRC = $(wildcard $(CURDIR)/fdkaac/libAACdec/src/*.cpp)
##AACDEC_SRC_EXTRA = $(wildcard $(CURDIR)/fdkaac/libFDK/src/*.cpp)
##AACDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libSYS/src/*.cpp)
AACDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libArithCoding/src/*.cpp)
AACDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libMpegTPDec/src/*.cpp)
#AACDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libSBRdec/src/*.cpp)
##AACDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libPCMutils/src/*.cpp)
AACDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libDRCdec/src/*.cpp)
AACDEC_SRC_EXTRA += $(wildcard $(CURDIR)/fdkaac/libSACdec/src/*.cpp)

AACDEC_SRC += $(AACDEC_SRC_EXTRA)
AACDEC_SRC += ./aacdec_interface.cpp

AACDEC_INCS =
AACDEC_INCS += -I ./fdkaac/libFDK/include/
AACDEC_INCS += -I ./fdkaac/libSYS/include/
AACDEC_INCS += -I ./fdkaac/libAACdec/include/
AACDEC_INCS += -I ./fdkaac/libArithCoding/include/
AACDEC_INCS += -I ./fdkaac/libAACdec/src/
AACDEC_INCS += -I ./fdkaac/libMpegTPDec/include/
AACDEC_INCS += -I ./fdkaac/libSBRdec/include/
AACDEC_INCS += -I ./fdkaac/libPCMutils/include/
AACDEC_INCS += -I ./fdkaac/libDRCdec/include/
AACDEC_INCS += -I ./fdkaac/libSACdec/include/
AACDEC_INCS += -I $(KERNEL_INC)	-I$(MW_INC)

OBJS_DEC = $(AACDEC_SRC:.cpp=.o)
#aac decoder -------------------------------------------end

#AAC  test sample code --------------------------start
AAC_TEST =  $(CURDIR)/sample_audio_aac
AAC_TEST_SRC += $(CURDIR)/cvi_audio_aac_adp.c
AAC_TEST_SRC += $(CURDIR)/../cvi_audio_dl_adp.c

MW_LIB ?= $(CURDIR)/../../../lib
AAC_LDFLAGS =
AAC_LDFLAGS += -L$(MW_LIB) -L$(MW_LIB)/3rd
AAC_LDFLAGS += -L $(CURDIR)/$(TARGET_MACHINE)

AAC_LDLIBS =
AAC_LDLIBS += -lpthread
AAC_LDLIBS += -lm -ldl
AAC_LDLIBS += -lstdc++
AAC_LDLIBS += -laacdec2
AAC_LDLIBS += -laacenc2
AAC_LDLIBS += -laaccomm2
AAC_LDLIBS += -laacsbrdec2
AAC_LDLIBS += -laacsbrenc2

AAC_INCLUDES =
AAC_INCLUDES += -I ../../../include
AAC_INCLUDES += -I$(CURDIR)
AAC_INCLUDES += -I$(KERNEL_INC)
AAC_INCLUDES += -I$(MW_INC)
AAC_CFLAGS += -I . -fPIC -Wno-unused-result  -Wno-format -Wl,--fatal-warning $(AAC_INCLUDES) $(DEFINES)


ifeq ($(wildcard $(CUR)/$(TARGET_MACHINE)),)
$(shell mkdir -p $(CUR)/$(TARGET_MACHINE))
endif

AACENC_TARGET := $(CUR)/$(TARGET_MACHINE)/libaacenc2.a
AACENC_TARGET_S := $(CUR)/$(TARGET_MACHINE)/libaacenc2.so
AACDEC_TARGET := $(CUR)/$(TARGET_MACHINE)/libaacdec2.a 
AACDEC_TARGET_S := $(CUR)/$(TARGET_MACHINE)/libaacdec2.so
AACCOMM_TARGET := $(CUR)/$(TARGET_MACHINE)/libaaccomm2.a
AACCOMM_TARGET_S := $(CUR)/$(TARGET_MACHINE)/libaaccomm2.so
AACSBRENC_TARGET := $(CUR)/$(TARGET_MACHINE)/libaacsbrenc2.a
AACSBRENC_TARGET_S := $(CUR)/$(TARGET_MACHINE)/libaacsbrenc2.so
AACSBRDEC_TARGET := $(CUR)/$(TARGET_MACHINE)/libaacsbrdec2.a
AACSBRDEC_TARGET_S := $(CUR)/$(TARGET_MACHINE)/libaacsbrdec2.so

ARFLAGS = rcs
ARFLAGS_S = -shared -fPIC -o
CFLAGS = -fPIC -c -Os  -ffunction-sections -fdata-sections

AAC_TEST_LIB = $(AAC_LDLIBS)

BUILDLIST=
BUILDLIST += $(AACENC_TARGET)
BUILDLIST += $(AACDEC_TARGET)
BUILDLIST += $(AACCOMM_TARGET)
BUILDLIST += $(AACSBRENC_TARGET)
BUILDLIST += $(AACSBRDEC_TARGET)
BUILDLIST += $(AAC_TEST)

OBJECTPATHS_ENCSRC=$(addprefix $(CURDIR)/,$(notdir $(OBJS_ENC)))
OBJECTPATHS_DECSRC=$(addprefix $(CURDIR)/,$(notdir $(OBJS_DEC)))
OBJECTPATHS_COMMSRC=$(addprefix $(CURDIR)/,$(notdir $(OBJS_COMM)))
OBJECTPATHS_SBRENCSRC=$(addprefix $(CURDIR)/,$(notdir $(OBJS_SBRENC)))
OBJECTPATHS_SBRDECSRC=$(addprefix $(CURDIR)/,$(notdir $(OBJS_SBRDEC)))
all:$(BUILDLIST)


$(OBJS_ENC):
	$(CC) $(AACENC_SRC) $(CFLAGS) $(AACENC_INCS)

$(OBJS_DEC):
	$(CC) $(AACDEC_SRC) $(CFLAGS) $(AACDEC_INCS)


$(OBJS_COMM):
	$(CC) $(AACCOMM_SRC) $(CFLAGS) $(AACCOMM_INCS)

$(OBJS_SBRENC):
	$(CC) $(AACSBRENC_SRC) $(CFLAGS) $(AACENC_INCS)

$(OBJS_SBRDEC):
	$(CC) $(AACSBRDEC_SRC) $(CFLAGS) $(AACDEC_INCS)


$(AACENC_TARGET) : $(OBJS_ENC)
	$(AR) $(ARFLAGS) $(AACENC_TARGET) $(OBJECTPATHS_ENCSRC)
	$(CC) $(ARFLAGS_S)  $(AACENC_TARGET_S) $(OBJECTPATHS_ENCSRC)
	$(STRIP) $(AACENC_TARGET_S)
	rm -rf  $(OBJECTPATHS_ENCSRC)

$(AACDEC_TARGET) : $(OBJS_DEC)
	$(AR) $(ARFLAGS) $(AACDEC_TARGET) $(OBJECTPATHS_DECSRC)
	$(CC) $(ARFLAGS_S)  $(AACDEC_TARGET_S) $(OBJECTPATHS_DECSRC)
	$(STRIP) $(AACDEC_TARGET_S)
	rm -rf  $(OBJECTPATHS_DECSRC)

$(AACCOMM_TARGET) : $(OBJS_COMM)
	$(AR) $(ARFLAGS) $(AACCOMM_TARGET) $(OBJECTPATHS_COMMSRC)
	$(CC) $(ARFLAGS_S)  $(AACCOMM_TARGET_S) $(OBJECTPATHS_COMMSRC)
	$(STRIP) $(AACCOMM_TARGET_S)
	rm -rf  $(OBJECTPATHS_COMMSRC)

$(AACSBRENC_TARGET) : $(OBJS_SBRENC)
	$(AR) $(ARFLAGS) $(AACSBRENC_TARGET) $(OBJECTPATHS_SBRENCSRC)
	$(CC) $(ARFLAGS_S)  $(AACSBRENC_TARGET_S) $(OBJECTPATHS_SBRENCSRC)
	$(STRIP) $(AACSBRENC_TARGET_S)
	rm -rf  $(OBJECTPATHS_SBRENCSRC)

$(AACSBRDEC_TARGET) : $(OBJS_SBRDEC)
	$(AR) $(ARFLAGS) $(AACSBRDEC_TARGET) $(OBJECTPATHS_SBRDECSRC)
	$(CC) $(ARFLAGS_S)  $(AACSBRDEC_TARGET_S) $(OBJECTPATHS_SBRDECSRC)
	$(STRIP) $(AACSBRDEC_TARGET_S)
	rm -rf  $(OBJECTPATHS_SBRDECSRC)

$(AAC_TEST): $(AACENC_TARGET) $(AACDEC_TARGET) $(AACCOMM_TARGET) $(AACSBRENC_TARGET) $(AACSBRDEC_TARGET)
	$(CC) -o   $@ $(AAC_LDFLAGS) $(AAC_TEST_SRC) $(AAC_TEST_LIB) $(AAC_CFLAGS)

.PHONY : clean
clean:
	rm -f    $(OBJECTPATHS_ENCSRC) $(OBJECTPATHS_DECSRC) $(OBJECTPATHS_COMMSRC) $(OBJECTPATHS_ENCSRC) $(OBJECTPATHS_SBRDECSRC) $(AAC_TEST)

