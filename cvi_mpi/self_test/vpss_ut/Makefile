SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

SDIR = $(PWD)
#SRCS = $(wildcard $(SDIR)/*.c)
SRCS := $(SDIR)/vpss_ut.c $(SDIR)/vpss_ut_comm.c $(SDIR)/vpss_cmodel.c
SRCS += $(wildcard $(SDIR)/../common/*.c)
INCS = -I$(MW_INC) -I$(SDIR)/../common/

SRCS_CLNT := $(SDIR)/vpss_ut_client.c $(SDIR)/vpss_ut_comm.c
SRCS_CLNT += $(wildcard $(SDIR)/../common/*.c)
OBJS_CLNT = $(SRCS_CLNT:.c=.o)
DEPS_CLNT = $(SRCS_CLNT:.c=.d)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)


TARGET = vpss_ut
LIBS = -lvpss -lgdc -lsys

EXTRA_CFLAGS += $(DEFS) $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

ldflags-y += $(LIBS) -lpthread -lm -lini -latomic

ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common
LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) cvi_common)
else
ELFFLAGS += -static
endif

all : vpss_ut vpss_ut_client

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

vpss_ut : $(OBJS) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $@ ../auto_test/

vpss_ut_client : $(OBJS_CLNT) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS_CLNT) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)
	@cp $@ ../auto_test/

.PHONY : clean
clean:
	@rm -f $(OBJS) $(OBJS_CLNT) $(DEPS) $(DEPS_CLNT) $(COMM_DEPS) vpss_ut vpss_ut_client

-include $(DEPS)
