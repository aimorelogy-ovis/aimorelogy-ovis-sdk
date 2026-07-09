SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
INCS = -I$(MW_INC) $(COMM_INC) -I$(ISP_INC) -I$(MW_INC)/sensor

TARGET = sample_sbm

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common)
else
ELFFLAGS += -static
LIBS +=  -lsensor -lvi -lvpss -lsys -lvenc -lrgn -lvo -lcvi_bin -lisp -lisp_algo -lawb -lae -laf
endif

ifneq ($(OS_TYPE), DUAL_OS)
LIBS += -lsns_full
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm -lini -lsys -lmipi

.PHONY : clean all
all: $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(COMM_OBJ) $(OBJS)
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(COMM_OBJS) $(EXTRA_LDFLAGS) -Wl,--end-group $(ELFFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
