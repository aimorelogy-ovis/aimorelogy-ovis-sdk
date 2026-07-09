SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include $(BUILD_PATH)/.config

SUPPORT_ISP_PQTOOL = 0

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c) $(wildcard $(SDIR)/src/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(KERNEL_INC) -I./inc $(COMM_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
COMM_INC = $(PWD)/../common
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(MW_INC)/sensor -I./inc -I$(COMM_INC)

TARGET = sample_sensor

ifeq ($(OS_TYPE), DUAL_OS)
LIBS += -lvi -lvpss -lsys -lini -lsensor -lisp -lisp_algo -lawb -lae -laf
else
LIBS = -lvi -lini -lsns_full -lsensor -lgdc -lsys -lisp -lcvi_bin -lawb -lae -laf -lisp_algo -lvo -lvpss
endif

ifeq ($(SUPPORT_ISP_PQTOOL), 1)
	EXTRA_CFLAGS += -DSUPPORT_ISP_PQTOOL
	LIBS_ISPD2 = -lcvi_ispd2 -lcvi_bin -lvo -lvpss
endif


EXTRA_CFLAGS += $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

EXTRA_LDFLAGS += $(LIBS) -lpthread -lm -latomic

ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ELFFLAGS += -static
endif

.PHONY : clean all

all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS) $(MW_LIB)/libsys.a $(MW_LIB)/libmipi.a $(MW_LIB)/libsensor.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libsys.a $(LIBS_ISPD2) \
	$(MW_LIB)/libmipi.a $(MW_LIB)/libsensor.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(TARGET)

