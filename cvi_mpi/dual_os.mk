SHELL = /bin/bash

.PHONY: all prepare unprepare

all: prepare 3rdparty module sample_app

prepare:
	@mkdir -p modules/uapi
	@mkdir -p modules/msg
	@cp -f $(MEDIA_INCLUDE_DIR)/internal/mpi_uapi/base_uapi.h modules/uapi
	@cp -f $(MEDIA_INCLUDE_DIR)/internal/mpi_uapi/tde_uapi.h modules/uapi
	@cp -f $(MEDIA_INCLUDE_DIR)/internal/msg/*.h modules/msg/
	@cp -f $(MEDIA_INCLUDE_DIR)/release/*.h include/
	@cp -f $(MW_SNS_INC)/sensor_cfg/*.h include
	@cp -f modules/cipher/include/*.h include
	@cp -f modules/efuse/include/*.h include

unprepare:
	@rm -rf modules/uapi
	@rm -rf modules/msg
	@rm -rf include/*.h
	@rm -rf lib/3rd/libcvi_json-c.a
	@rm -rf lib/tiny