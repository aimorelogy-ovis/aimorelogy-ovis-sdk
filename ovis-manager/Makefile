CC ?= gcc
CFLAGS ?= -O2
-include configs/cv1842hp_ovis_spinand_defconfig
CFLAGS += -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Ibackend/include
CFLAGS += -DOVIS_DEFAULT_PORT=$(CONFIG_OVIS_MANAGER_PORT)
CFLAGS += -DOVIS_BIND_ADDRESS=\"$(CONFIG_OVIS_MANAGER_BIND_ADDRESS)\"
LDFLAGS += -pthread

TARGET := build/ovis-managerd
SOURCES := $(wildcard backend/src/*.c)
OBJECTS := $(patsubst backend/src/%.c,build/%.o,$(SOURCES))

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

build/%.o: backend/src/%.c backend/include/ovis_manager.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	rm -rf install
	install -D -m 0755 $(TARGET) install/usr/sbin/ovis-managerd
	install -D -m 0755 scripts/S98ovis-manager install/etc/init.d/S98ovis-manager
	install -D -m 0644 resource/www/index.html install/usr/share/ovis-manager/www/index.html
	install -D -m 0644 resource/www/app.js install/usr/share/ovis-manager/www/app.js
	install -D -m 0644 resource/www/style.css install/usr/share/ovis-manager/www/style.css

clean:
	rm -rf build install
