include $(SRCTREE)/build/rule.mk

TOPTARGETS := all clean install clean_all

module-y									    += paramparse_common
module-y                                        += module
module-$(CONFIG_MODULE_MEDIA_SYS)		      	+= sys
module-$(CONFIG_MODULE_MEDIA_VI)		      	+= vi
module-$(CONFIG_MODULE_MEDIA_VPSS)		      	+= vpss
module-$(CONFIG_MODULE_MEDIA_VENC)		      	+= venc
module-$(CONFIG_MODULE_MEDIA_DEC)		      	+= vdec
module-$(CONFIG_MODULE_DISPLAY)			      	+= display
module-$(CONFIG_MODULE_RTSP)			      	+= rtsp
module-$(CONFIG_MODULE_MEDIA_OSD)		      	+= osd
module-$(CONFIG_MODULE_GPIO)			      	+= gpio
module-$(CONFIG_MODULE_PWM)				      	+= pwm
module-$(CONFIG_MODULE_MEDIA_AUDIO)		      	+= audio
module-$(CONFIG_MODULE_AI)				      	+= ai
module-$(CONFIG_MODULE_AI_MD)			      	+= ai_md
module-$(CONFIG_MODULE_AI_PD)			      	+= ai_pd
module-$(CONFIG_MODULE_AI_FD_FACE)              += ai_face
module-$(CONFIG_MODULE_AI_IRFAECE)		      	+= ai_ir_face
module-$(CONFIG_MODULE_AI_BABYCRY)			    += ai_babycry
module-$(CONFIG_MODULE_AI_HUMAN_KEYPOINT)	    += ai_human_keypoint_detect
module-$(CONFIG_MODULE_AI_OBJECT_TRACK)	       	+= ai_object_track
module-$(CONFIG_MODULE_AI_IMG_TXT_CLIP)	        += ai_img_txt_clip
module-$(CONFIG_MODULE_AI_KEYPOINT_HAND_GESTURE)+= ai_keypoint_hand_gesture
module-$(CONFIG_MODULE_AI_LPR)			+= ai_lpr
module-$(CONFIG_MODULE_RECORD)				    += record
module-$(CONFIG_MODULE_CVIUVC_HOST)				+= uvc_host

SUBDIRS += $(module-y)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
