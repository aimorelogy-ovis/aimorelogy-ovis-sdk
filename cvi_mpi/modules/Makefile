ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

.PHONY: all clean $(MODULES)

MODULES = sys cif sensor vi vpss venc vdec gdc rgn tde vo mipi_tx cvi_bin efuse cipher


ifeq ($(OS_TYPE), DUAL_OS)
MODULES += ipcm
endif


all:
	@echo "########################################################################################################"
	@echo "#"
	@echo "#  Compiling 'module libs' Configs as below ..."
	@echo "#    "CROSS_COMPILE=$(CROSS_COMPILE)
	@echo "#    "KERNEL_DIR=$(KERNEL_DIR)
	@echo "#"
	@echo "########################################################################################################"
	@for x in $(MODULES); \
	    do cd $$x; if [ $$? ]; then $(MAKE) all || exit 1; cd -; fi; done
	@echo "#"

clean:
	@for x in $(MODULES); \
	    do cd $$x; if [ $$? ]; then $(MAKE) clean; cd -; fi; done
	@echo "#"
