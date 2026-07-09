CC = arm-linux-gnueabihf-gcc

source  = cvi_pinmux.c devmem.c
object  = $(source:%.cc=%.o)

all: $(object)
	$(CC) $(object) -o cvi_pinmux

%.o: %.c
	$(CC) $(CCCFLAGS) -c $<

clean:
	$(RM) *.o cvi_pinmux
