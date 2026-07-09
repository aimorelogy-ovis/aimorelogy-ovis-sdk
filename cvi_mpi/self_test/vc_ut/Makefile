SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

include $(BUILD_PATH)/.config

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(VENC_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

CVI_API_TEST_SRCS = $(SDIR)/cvi_api_test.c

CVI_API_TEST_OBJS = $(CVI_API_TEST_SRCS:.c=.o)


CVI_API_TEST = cvi_api_test


COMM_OBJ :=

LIBS =

EXTRA_CFLAGS += $(DEFS) $(INCS) -Wno-error=unused-variable -Wno-error=sign-compare -Wno-error=unused-parameter
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

EXTRA_LDFLAGS += $(LIBS) -pthread -lm -latomic

# ELFFLAGS +=  -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64dv
LD_FLAGS = -lvdec -lvenc -lsys -pthread -static

all :  $(CVI_API_TEST)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)


$(CVI_API_TEST) : $(CVI_API_TEST_OBJS)
	@$(CXX) -o $@ $(CVI_API_TEST_OBJS) $(ELFFLAGS) $(LD_FLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS)
	@rm -f  $(CVI_API_TEST)
-include $(DEPS)
