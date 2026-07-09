SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

COMMON_DIR ?= $(PWD)/../common
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := $(COMMON_DIR)
COMM_DEPS = $(COMM_SRC:.c=.d)

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(COMM_INC) -I$(ISP_INC) -I$(MW_INC)/sensor -I./inc
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

TARGET = sample_region
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_region_asan
else
ELFFLAGS += -static
endif

ifeq ($(OS_TYPE), DUAL_OS)
LIBS = $(MW_LIBS) -lsensor -lvpss -lrgn -lsys -lcvi_bin -lisp -lawb -lae -laf
else
LIBS = $(MW_LIBS) -lsns_full -lsensor -lvpss -lrgn -lsys -lcvi_bin -lisp -lawb -lae -laf -lisp_algo
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm -lini
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
EXTRA_LDFLAGS += $(MW_LIB)/libvdec.a
endif

.PHONY : clean all
all: $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(OBJS) $(MW_LIB)/libsys.a $(MW_LIB)/libmipi.a $(MW_LIB)/libsensor.a
	@$(CXX) -o $@ $(OBJS) $(COMM_OBJ) $(MW_LIB)/libsys.a $(MW_LIB)/libmipi.a  \
		$(MW_LIB)/libsensor.a $(MW_LIB)/libvo.a $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
