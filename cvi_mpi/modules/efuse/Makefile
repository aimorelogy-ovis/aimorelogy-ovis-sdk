SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

#
SDIR	:= $(CURDIR)/src
SRCS_C	:= $(SDIR)/cvi_efuse.c
#
ODIR    := $(CURDIR)/obj
OBJS    := $(SRCS_C:$(SDIR)/%.c=$(ODIR)/%.o)
DEPS	:= $(SRCS_C:.c=.d)
#
INCS := $(CURDIR)/include/
#
TARGET_A = $(MW_LIB)/libefuse.a
TARGET_SO = $(MW_LIB)/libefuse.so

EXTRA_CFLAGS = -I$(MW_INC) -I$(INCS)
LIBS +=  -lm -lpthread -lc -L$(MW_LIB)
.PHONY : clean all clean_all
all : $(TARGET_A) $(TARGET_SO)

$(ODIR)/%.o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO): $(OBJS)
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ --start-group $(OBJS) $(LIBS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -f $(TARGET_A) $(TARGET_SO)
	@rm -rf $(ODIR)

clean_all: clean

-include $(DEPS)
