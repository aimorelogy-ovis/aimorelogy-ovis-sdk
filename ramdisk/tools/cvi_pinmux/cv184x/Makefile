ifeq ($(SDK_VER),musl_arm)
	CC = $(TOP_DIR)/host-tools/gcc/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-musleabihf/bin/arm-linux-gcc
else ifeq ($(SDK_VER),musl_arm64)
	CC = $(TOP_DIR)/host-tools/gcc/arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-musl/bin/aarch64-none-linux-musl-gcc
else ifeq ($(SDK_VER),glibc_arm)
	CC = $(TOP_DIR)/host-tools/gcc/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc
else ifeq ($(SDK_VER),glibc_arm64)
	CC = $(TOP_DIR)/host-tools/gcc/arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
else
$(error Unknown SDK_VER)
endif

source  = cvi_pinmux.c devmem.c
object  = $(source:%.c=%.o)
TARGET = cvi_pinmux

all: $(TARGET)

$(TARGET): $(object)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(TARGET)
