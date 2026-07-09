SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=$(TOP_DIR)/cvi_mpi/Makefile.param
	include $(PARAM_FILE)
endif

API_TEST_SDIR = $(PWD)/api_test
UNIT_TEST_SDIR = $(PWD)/unit_test

SRCS = $(wildcard $(API_TEST_SDIR)/*.c)
SRCS += $(wildcard $(UNIT_TEST_SDIR)/*.c)

INCS += -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC) -I$(SYS_INC)
INCS += -I$(API_TEST_SDIR) -I$(UNIT_TEST_SDIR)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
TARGET_A = libisp_test.a
TARGET_SO = libisp_test.so

LOCAL_CFLAGS = $(INCS)

.PHONY: clean all
all: $(TARGET_A) $(TARGET_SO)

$(API_TEST_SDIR)/%.o: $(API_TEST_SDIR)/%.c
	@$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(UNIT_TEST_SDIR)/%.o: $(UNIT_TEST_SDIR)/%.c
	@$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)
