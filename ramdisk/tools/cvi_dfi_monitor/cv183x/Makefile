CC = $(CROSS_COMPILE)gcc

SRCS = dfi_monitor.c
SRCS += devmem.c
SRCS += argvtoint.c

OBJS = $(SRCS:.c=.o)

CFLAGS = --static

all : dfi_monitor

dfi_monitor : $(OBJS)
	$(CC) -o dfi_monitor $(SRCS) $(CFLAGS)
	rm -f $(OBJS)

.PHONY : clean
clean:
	rm -f $(OBJS) dfi_monitor
