SHELL = /bin/bash

.PHONY : all target prepare clean

CC := $(CROSS_COMPILE)gcc
chip_arch := $(shell echo $(CHIP_ARCH) | tr A-Z a-z)
TMP_FOLDER = tmp

CODE_TEST_DIR=code_test

SDIR=$(PWD)/$(CODE_TEST_DIR)
SRCS := $(wildcard $(SDIR)/*.c)
OBJS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.o, $(wildcard $(SDIR)/*.c))
DEPS = $(patsubst $(SDIR)/%.c, $(TMP_FOLDER)/%.d, $(wildcard $(SDIR)/*.c))

LOCAL_LDFLAGS = -L$(TOP_DIR)/cvi_mpi/lib -lisp -lisp_algo -lae -lawb -laf
LOCAL_LDFLAGS += -lvi -lsys -lm -static

LOCAL_CFLAGS = -I$(TOP_DIR)/cvi_mpi/include -I$(TOP_DIR)/cvi_mpi/include/isp

ISP_TUNING_TARGET = pqjson2ccode_test

all:
	@echo "generate c code!"
	@./script/gen.sh
	@$(MAKE) target

target: $(ISP_TUNING_TARGET)

prepare:
	-@mkdir -p $(TMP_FOLDER)
	-@mkdir -p $(TMP_FOLDER)/module

$(TMP_FOLDER)/%.o: $(SDIR)/%.c | prepare
	@$(CC) $(LOCAL_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(ISP_TUNING_TARGET): $(OBJS)
	@$(CC) -o $@ $(OBJS) $(LOCAL_LDFLAGS)
	@echo -e [LINK][$(notdir $(CC))] $(notdir $@)

clean:
	@rm -rf tmp/
	@rm -f $(ISP_TUNING_TARGET)
	@rm -rf $(CODE_TEST_DIR)
	@find $(chip_arch) \( -name "*.c" -o -name "*.h" \) -exec rm -f {} +
