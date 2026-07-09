SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

PANEL_INC =../../component/PanelSupportList/panels
SDIR = $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)
COMM_INC = $(CURDIR)/../common
INCS = -I$(MW_INC) -I$(PANEL_INC) -I$(COMM_INC) -I$(ISP_INC) -I$(MW_INC)/sensor
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_panel
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_panel_asan
endif

ELFFLAGS += -static
LIBS = -lvo -lsys -lmipi_tx

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

EXTRA_CFLAGS = $(INCS)
EXTRA_LDFLAGS = $(LIBS) -lpthread

.PHONY : clean all lt9611 clean_lt9611
all: $(TARGET) lt9611

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

lt9611:
	cd $(SDIR)/lt9611;make clean;make

clean_lt9611:
	cd $(SDIR)/lt9611;make clean;

clean: clean_lt9611
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)