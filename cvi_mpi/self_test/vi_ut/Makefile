SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include $(BUILD_PATH)/.config

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
SRCS += $(wildcard $(SDIR)/../common/*.c)
ifneq ($(CV184X_FPGA_RAW_REPLAY), 1)
	SRCS := $(filter-out $(SDIR)/vi_ut_isp_comm.c, $(SRCS))
	SRCS := $(filter-out $(SDIR)/vi_ut_isp_utils.c, $(SRCS))
	SRCS := $(filter-out $(SDIR)/vi_ut_isp_rawreplayoffline.c, $(SRCS))
else
	CFLAGS += -DCV184X_FPGA_RAW_REPLAY
endif

ISP_LIBS = -lae -lawb -laf -lcvi_ispd2 -lisp -lisp_algo -ldl

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

INCS = -I$(MW_INC) -I$(MW_INC)/sensor -I$(MW_PATH)/modules/vi/include/
INCS += -I$(ISP_INC)
INCS += -I$(MW_PATH)/modules/isp/cv184x/isp-daemon2/inc
INCS += -I$(MW_PATH)/modules/isp/algo
INCS += -I$(MW_PATH)/modules/isp/cv184x/isp/src/isp_ctrl
INCS += -I$(MW_PATH)/modules/isp/cv184x/isp/inc
INCS += -I$(SDIR)/../common/
TARGET = vi_ut

ifeq ($(OS_TYPE), DUAL_OS)
LIBS += -lvi -lvpss -lsys -lini -lsensor -lgdc -lcvi_bin -lvo
else
LIBS = -lvi -lvpss -lvo -lcvi_bin -lini -lsns_full -lsensor -lgdc -lsys $(ISP_LIB)
endif

LIBS += $(ISP_LIBS)

EXTRA_CFLAGS += $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

EXTRA_LDFLAGS += -lpthread -lm -latomic -lmipi

ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ELFFLAGS += -static
endif

.PHONY : clean all

all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS) $(MW_LIB)/libsys.a $(MW_LIB)/libsensor.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libsys.a $(MW_LIB)/libsensor.a $(LIBS) -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $(TARGET) ../auto_test/

clean:
	@rm -f $(OBJS) $(TARGET) $(DEPS)

