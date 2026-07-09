ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

.PHONY: all clean


SAMPLE_APP = sbm vio vpss venc region gfbg tde sensor panel vdec audio cipher gdc sample_ir_auto

ifeq ($(OS_TYPE), DUAL_OS)
SAMPLE_APP += fastboot
endif

all:
	@echo "#################################"
	@echo "#                               #"
	@echo "#   Compiling 'sample_app'...   #"
	@echo "#                               #"
	@echo "#################################"
	@for x in $(SAMPLE_APP); \
	    do cd $$x; if [ $$? ]; then $(MAKE) all || exit 1; cd -; fi; done
	@echo "#"

clean:
	@for x in $(SAMPLE_APP); \
	    do cd $$x; if [ $$? ]; then $(MAKE) clean; cd -; fi; done
	@echo "#"
