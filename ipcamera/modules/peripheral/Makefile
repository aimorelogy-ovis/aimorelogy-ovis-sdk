include $(SRCTREE)/build/rule.mk

TOPTARGETS := all clean install clean_all

module-$(CONFIG_MODULE_GPIO)				+= gpio
module-$(CONFIG_MODULE_ADC)					+= adc
module-$(CONFIG_MODULE_IRCUT)				+= ircut
module-$(CONFIG_MODULE_PWM)					+= pwm
module-$(CONFIG_MODULE_DISPLAY)				+= panel

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean:
	@$(foreach dir,$(SUBDIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) clean || exit 1;cd -; \
		else \
			exit 1; \
		fi;)

clean_all : clean
	@$(foreach dir,$(SUBDIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) clean_all || exit 1;cd -; \
		else \
			exit 1; \
		fi;)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
