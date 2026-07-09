ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

.PHONY: all clean

TEST_UT = sys_ut vb_ut vi_ut vpss_ut vo_ut vc_ut ldc_ut rgn_ut tde_ut audio_ut

ifeq ($(OS_TYPE), DUAL_OS)
TEST_UT += ipcm_ut
endif

all:
	@echo "#################################"
	@echo "#                               #"
	@echo "#   Compiling 'self tests'...   #"
	@echo "#                               #"
	@echo "#################################"
	@for x in $(TEST_UT); \
	    do cd $$x; if [ $$? ]; then $(MAKE) all || exit 1; cd -; fi; done
	@echo "#"

clean:
	@for x in $(TEST_UT); \
	    do cd $$x; if [ $$? ]; then $(MAKE) clean; cd -; fi; done
	@echo "#"