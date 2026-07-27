CC ?= gcc
CFLAGS ?= -O2
-include configs/cv1842hp_ovis_spinand_defconfig
CJSON_ROOT := ../ipcamera/modules/common
CFLAGS += -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Ibackend/include
CFLAGS += -I$(CJSON_ROOT)/include
CFLAGS += -DOVIS_DEFAULT_PORT=$(CONFIG_OVIS_MANAGER_PORT)
CFLAGS += -DOVIS_BIND_ADDRESS=\"$(CONFIG_OVIS_MANAGER_BIND_ADDRESS)\"
CFLAGS += -DOVIS_ALLOWED_ORIGIN=\"$(CONFIG_OVIS_MANAGER_ALLOWED_ORIGIN)\"
CFLAGS += -DOVIS_DEV_ORIGIN=\"$(CONFIG_OVIS_MANAGER_DEV_ORIGIN)\"
CFLAGS += -DOVIS_DEV_ORIGIN_ALT=\"$(CONFIG_OVIS_MANAGER_DEV_ORIGIN_ALT)\"
CFLAGS += -DOVIS_FIRMWARE_VERSION=\"$(CONFIG_OVIS_FIRMWARE_VERSION)\"
ifeq ($(CONFIG_OVIS_AI_BNR),y)
CFLAGS += -DOVIS_AI_BNR_SUPPORT
endif
ifeq ($(CONFIG_OVIS_AI_BNR_CERTIFIED),y)
CFLAGS += -DOVIS_AI_BNR_CERTIFIED
endif
LDFLAGS += -pthread -lm

TARGET := build/ovis-managerd
SOURCES := $(wildcard backend/src/*.c)
OBJECTS := $(patsubst backend/src/%.c,build/%.o,$(SOURCES))
OBJECTS += build/cJSON.o

.PHONY: all install test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

build/%.o: backend/src/%.c backend/include/ovis_manager.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/cJSON.o: $(CJSON_ROOT)/cjson/src/cJSON.c $(CJSON_ROOT)/include/cJSON.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	rm -rf install
	install -D -m 0755 $(TARGET) install/usr/sbin/ovis-managerd
	install -D -m 0755 scripts/ovis-device-identity install/usr/sbin/ovis-device-identity
	install -D -m 0755 scripts/S98ovis-manager install/etc/init.d/S98ovis-manager

test: build/config_transaction_test build/model_store_test
	./build/config_transaction_test
	./build/model_store_test

build/config_transaction_test: tests/config_transaction_test.c backend/src/config_store.c \
		backend/src/config_validate.c $(CJSON_ROOT)/cjson/src/cJSON.c
	@mkdir -p build
	$(CC) $(CFLAGS) \
		-DOVIS_CONFIG_DIR=\"/tmp/ovis-manager-config-test\" \
		-DOVIS_CONFIG_FILE=\"/tmp/ovis-manager-config-test/active.ini\" \
		-DOVIS_CONFIG_PENDING=\"/tmp/ovis-manager-config-test/pending.ini\" \
		-DOVIS_CONFIG_BACKUP=\"/tmp/ovis-manager-config-test/backup.ini\" \
		-DOVIS_DEFAULT_CONFIG=\"$(CURDIR)/../ipcamera/resource/parameter/sbm/cv1842hp_ovis_sc235hai.ini\" \
		$^ -pthread -lm -o $@

build/model_store_test: tests/model_store_test.c backend/src/model_store.c \
		$(CJSON_ROOT)/cjson/src/cJSON.c
	@mkdir -p build
	$(CC) $(CFLAGS) \
		-DOVIS_MODEL_DIR=\"/tmp/ovis-manager-model-test\" \
		-DOVIS_CONFIG_FILE=\"/tmp/ovis-manager-model-test/active.ini\" \
		$^ -pthread -lm -o $@

clean:
	rm -rf build install
