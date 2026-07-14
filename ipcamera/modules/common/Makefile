include $(SRCTREE)/build/rule.mk

TOPTARGETS := all clean install clean_all

module-$(CONFIG_MODULE_CJSON)					+= cjson
module-$(CONFIG_MODULE_MININI)					+= minini
module-$(CONFIG_MODULE_MBUF)					+= mbuf
module-$(CONFIG_MODULE_COMMON)					+= comm
module-$(CONFIG_MODULE_PARAMPARSE)				+= paramparse

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
