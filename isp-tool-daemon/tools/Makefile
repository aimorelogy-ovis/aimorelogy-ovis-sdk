SHELL = /bin/bash

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=$(TOP_DIR)/cvi_mpi/Makefile.param
	include $(PARAM_FILE)
endif

ifeq ($(OS_TYPE), DUAL_OS)
TOOL_DIRS = bmodel_test template
INSTALL_DIRS = bmodel_test
else
TOOL_DIRS = bmodel_test pqbin_export_tool template
INSTALL_DIRS = bmodel_test pqbin_export_tool
endif

# Installation directory
INSTALL_DIR = $(PWD)/install

.PHONY: all clean $(TOOL_DIRS)

all: $(TOOL_DIRS)
	@echo "All test tools built successfully"

# Build each tool
$(TOOL_DIRS):
	@echo "Building $@..."
	@$(MAKE) -C $@ all
	@echo "$@ build complete"

# Clean all subdirectories
clean:
	@echo "Cleaning tools..."
	@for dir in $(TOOL_DIRS); do \
		if [ -d $$dir ]; then \
			echo "Cleaning $$dir..."; \
			$(MAKE) -C $$dir clean; \
		fi; \
	done
	@rm -rf install
	@echo "tools clean complete"

# Individual tool can be built separately
.PHONY: help
help:
	@echo "Usage:"
	@echo "  make           - Build all tools (including template)"
	@echo "  make clean     - Clean all tools"
	@echo "  make <dir>     - Build specific tool"
	@echo ""
	@echo "Tools to build:"
	@for dir in $(TOOL_DIRS); do echo "  - $$dir"; done
	@echo ""
	@echo "Tools to install:"
	@for dir in $(INSTALL_DIRS); do echo "  - $$dir"; done
	@echo ""
	@echo "Note: template is built but not installed"
