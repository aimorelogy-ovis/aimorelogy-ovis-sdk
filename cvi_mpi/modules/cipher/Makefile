SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

OPENSSL_PATH=../../3rdparty/openssl
#
SDIR := $(CURDIR)/src
# SDIR := $(CURDIR)
SRCS_C := $(SDIR)/cvi_unf_cipher.c
ODIR    := $(CURDIR)/obj
OBJS    := $(SRCS_C:$(SDIR)/%.c=$(ODIR)/%.o)
DEPS	:= $(SRCS_C:.c=.d)

# Override any existing INCS from parent Makefile
override INCS := -I$(MW_INC) \
                 -I$(KERNEL_INC) \
                 -I$(CURDIR)/../../3rdparty/openssl/include \
                 -I$(CURDIR)/include
ifeq ($(CROSS_COMPILE), arm-none-linux-musleabihf-)
LIBS = $(OPENSSL_PATH)/libcrypto-musl_arm.a
LOCAL_OPT_LEVEL = -O3
else ifeq ($(CROSS_COMPILE), aarch64-none-linux-musl-)
LIBS = $(OPENSSL_PATH)/libcrypto-musl_arm64.a
LOCAL_OPT_LEVEL = -O3
else ifeq ($(CROSS_COMPILE), riscv64-unknown-linux-gnu-)
LIBS = $(OPENSSL_PATH)/libcrypto-riscv64.a
LOCAL_OPT_LEVEL = -O3
else ifeq ($(CROSS_COMPILE), riscv64-unknown-linux-musl-)
LIBS = $(OPENSSL_PATH)/libcrypto-musl_riscv64.a
LOCAL_OPT_LEVEL = -O3
else ifeq ($(CROSS_COMPILE), arm-none-linux-gnueabihf-)
LIBS = $(OPENSSL_PATH)/libcrypto-arm.a
LOCAL_OPT_LEVEL = -O3
else
LIBS = $(OPENSSL_PATH)/libcrypto.a
LOCAL_OPT_LEVEL = -Os
endif
TARGET_A = $(abspath  $(MW_LIB)/libcipher.a)
TARGET_SO = $(abspath  $(MW_LIB)/libcipher.so)
TMP_A = cipher_tmp.a

EXTRA_CFLAGS = $(INCS)

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

CFLAGS += -ggdb $(LOCAL_OPT_LEVEL) -fno-omit-frame-pointer
CIPHER_LDFLAGS := $(filter-out --gc-sections,${LDFLAGS})
CIPHER_LDFLAGS +=  -ldl

$(ODIR)/%.o: $(SDIR)/%.c
	mkdir -p $(ODIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(LD) -relocatable $(EXTRA_LDFLAGS) $(OBJS) $(LIBS) -o $(ODIR)/cipher.o
	@SYMS=$$(${NM} $(ODIR)/cipher.o | awk '$$3 ~ /./ { print $$3 }' | sort -u | grep -v '^CVI_' | sed 's/^/-L /'); \
		$(OBJCOPY) $$SYMS $(ODIR)/cipher.o $(ODIR)/cipher.o
	@$(AR) $(ARFLAGS) $(TARGET_A) $(ODIR)/cipher.o
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO): $(OBJS)
	@$(CC) -Wl,--version-script=libcipher.version $(CIPHER_LDFLAGS) $(EXTRA_LDFLAGS) -o $(TARGET_SO) -Wl,--start-group $(OBJS) $(LIBS) -Wl,--end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -rf $(ODIR) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)
