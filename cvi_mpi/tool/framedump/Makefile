#CC=riscv64-unknown-linux-gnu-gcc
CC=$(CROSS_COMPILE)gcc
SDIR = $(PWD)
DEBUG = 1

#include ../../Makefile.param
#$(info $(ELFFLAGS))

SRC := $(wildcard ./*.c)
OBJ := $(SRC:%.c=%.o)
TARGET := $(OBJ:%.o=%)
INC := -I./
INC += -I../../include/


#$(info $(INC))

ELFFLAGS := -Wl,--gc-sections
ELFFLAGS += -static

EXTRA_LDFLAGS = -L../../lib
EXTRA_LDFLAGS += -lvi -lvpss -lsys
EXTRA_LDFLAGS += -lm -lpthread -latomic

CFLAGS := $(INC)
CFLAGS += -DSUPPORT_MULTI_PROCESS

ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
endif

all : $(TARGET)

$(SDIR)/%.o: %.c
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET):%:%.o
	$(CC) $(CFLAGS) -o $@ $^ $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@rm *.o
ifneq ($(DEBUG), 1)
	@$(CROSS_COMPILE)strip $@
endif

clean:
	@rm -f $(OBJ) $(TARGET)
