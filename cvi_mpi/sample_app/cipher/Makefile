SHELL = /bin/bash

STATIC ?= 0

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

SDIR := $(CURDIR)
COMMON_DIR ?= $(CURDIR)/../common
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := $(COMMON_DIR)
COMM_DEPS = $(COMM_SRC:.c=.d)

SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(COMM_INC)  -I$(KERNEL_INC) -I$(CURDIR)
OBJS = $(SRCS:.c=.o) $(COMM_OBJ)
DEPS = $(SRCS:.c=.d) $(COMM_DEPS)

TARGET = sample_cipher
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_cipher_asan
endif

EXTRA_CFLAGS = $(INCS)

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common)
EXTRA_LDFLAGS = -Wl,-Bdynamic $(LIBS) -lcipher -lefuse -lsys -lini -lpthread -ldl
else
ELFFLAGS += -static
LIBS += -lsys -lcipher -lefuse
EXTRA_LDFLAGS = $(LIBS) -lini -lpthread -ldl
endif

ifeq ($(SAMPLE_STATIC),1)
LIB_DEP := $(MW_LIB)/libsys.a $(MW_LIB)/libcipher.a $(MW_LIB)/libefuse.a
else
LIB_DEP := $(MW_LIB)/libsys.so $(MW_LIB)/libcipher.so $(MW_LIB)/libefuse.so
endif

.PHONY : clean all
all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	echo "NW_INCLUDE"=$(MW_INC)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS) $(LIB_DEP)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
