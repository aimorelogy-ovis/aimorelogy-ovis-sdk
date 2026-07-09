SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

SDIR = $(CURDIR)

COMMON_DIR ?= $(PWD)/../common
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := $(COMMON_DIR)
COMM_DEPS = $(COMM_SRC:.c=.d)

SRCS = $(wildcard $(SDIR)/src/*.c)
VDEC_SRCS = $(SDIR)/sample_vdec.c
VDEC_INC=../../modules/vdec
SAMPLE_VENC_INC=$(SDIR)/../venc/include
SAMPLE_VDEC_INC=$(SDIR)/include
ISP_INC=../../include/isp

INCS = -I$(MW_INC) -I$(COMM_INC) -I$(SAMPLE_VDEC_INC) -I$(SAMPLE_VENC_INC) -I$(VDEC_INC) -I$(ISP_INC)

OBJS = $(SRCS:.c=.o)
VDEC_OBJS = $(VDEC_SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
VDEC_DEPS = $(VDEC_SRCS:.c=.d)

TARGET = sample_vdec

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common)
else
ELFFLAGS += -static
LIBS +=  -lvi -lvpss -lvenc -lrgn -lvo -lmipi -lsys -lsensor -lvdec -lisp
endif


EXTRA_CFLAGS = $(DEFS) $(INCS)
EXTRA_LDFLAGS = $(LIBS) -lini -lm -lpthread


ifeq ($(DEBUG), 1)
EXTRA_LDFLAGS += -g -O0
endif


.PHONY : clean all
all: $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(VDEC_OBJS) $(OBJS)
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(VDEC_OBJS) $(COMM_OBJ) -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)



clean:
	@rm -f $(VDEC_OBJS) $(OBJS) $(VDEC_DEPS) $(DEPS) $(TARGET) $(COMM_OBJ)

-include $(VDEC_DEPS) $(DEPS)
