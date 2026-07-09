SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

SDIR = $(PWD)

COMMON_DIR ?= $(PWD)/../common
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := $(COMMON_DIR)
COMM_DEPS = $(COMM_SRC:.c=.d)

SRCS = $(wildcard $(SDIR)/src/*.c)
VENC_SRCS = $(SDIR)/sample_venc.c
SAMPLE_VENC_INC=$(SDIR)/include
INCS = -I$(MW_INC) -I$(COMM_INC) -I$(SAMPLE_VENC_INC) -I$(ISP_INC) -I$(MW_INC)/sensor

$(info INCS is: $(INCS))

OBJS = $(SRCS:.c=.o)
VENC_OBJS = $(VENC_SRCS:.c=.o)

DEPS = $(SRCS:.c=.d)
VENC_DEPS = $(VENC_SRCS:.c=.d)

TARGET_VENC = sample_venc
ifeq ($(SDK_VER),32bit)
TARGET_VENC_WITH_ASAN = sample_venc_asan
else ifeq ($(SDK_VER), 64bit)
TARGET_VENC_WITH_ASAN = sample_venc_asan
else ifeq ($(SDK_VER), glibc_arm64)
TARGET_VENC_WITH_ASAN = sample_venc_asan
else ifeq ($(SDK_VER), glibc_arm)
TARGET_VENC_WITH_ASAN = sample_venc_asan
else
TARGET_VENC_WITH_ASAN =
TARGET_VCODEC_WITH_ASAN =
endif

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common)
else
ELFFLAGS += -static
LIBS += -lvpss -lvenc -lsys
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lini -lpthread

EXTRA_LDFLAGS_WITH_ASAN = -Wl,-Bdynamic $(LIBS) -lini -lpthread

ifeq ($(DEBUG), 1)
EXTRA_LDFLAGS += -g -O0
EXTRA_LDFLAGS_WITH_ASAN += -g -O0
endif

EXTRA_LDFLAGS_WITH_ASAN += -fno-omit-frame-pointer -fsanitize-recover=address -lasan

.PHONY : clean all

all : $(TARGET_VENC)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_VENC) : $(COMM_OBJ) $(VENC_OBJS) $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(VENC_OBJS) $(COMM_OBJ) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

$(TARGET_VENC_WITH_ASAN) : $(COMM_OBJ) $(VENC_OBJS) $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(VENC_OBJS) $(COMM_OBJ) $(ELFFLAGS) $(EXTRA_LDFLAGS_WITH_ASAN)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)


clean:
	@rm -f $(OBJS) $(VENC_OBJS) $(DEPS) $(VENC_DEPS)  $(COMM_DEPS) $(COMM_OBJ) $(TARGET_VENC) $(TARGET_VENC_WITH_ASAN)

-include $(VENC_DEPS) $(DEPS) $(COMM_DEPS)
