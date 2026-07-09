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
INCS = -I$(MW_INC) -I$(COMM_INC) -I$(ISP_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

ELFFLAGS += -static
LIBS += -lsys -lvo -ltde

TARGET = sample_gfbg
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_gfbg_asan
endif

EXTRA_CFLAGS = $(INCS)
EXTRA_LDFLAGS = $(LIBS) -lpthread

.PHONY : clean all
all: $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(COMM_OBJ) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
