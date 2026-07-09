# MEDIA
INCS-y += -I$(MW_PATH)/include -I$(MW_PATH)/include/isp/ -I$(ISP_INC) -I$(MW_PATH)/include/isp/$(SOC_NICK_NAME_LOWER) -I$(MW_PATH)/modules/isp/$(SOC_NICK_NAME_LOWER)/isp-daemon2/inc/

# DISPLAY
INCS-$(CONFIG_MODULE_DISPLAY) += -I$(MW_PATH)/component/panel
DEFS-$(CONFIG_MODULE_DISPLAY) += -DDISPLAY
DEFS-$(CONFIG_MODULE_MEDIA_VO) +=-DVO_SUPPORT

# EFUSE FASTBOOT
DEFS-$(CONFIG_MODULE_MEDIA_EFUSE) += -DEFUSE

# ANONMSG
DEFS-$(CONFIG_MODULE_MEDIA_ANONMSG) += -DANONMSG_ENABLE

# VDEC
DEFS-$(CONFIG_MODULE_MEDIA_DEC) += -DVDEC_SUPPORT

# OSDC
DEFS-$(CONFIG_MODULE_MEDIA_OSD) += -DOSDC_SUPPORT

# AUDIO
DEFS-$(CONFIG_MODULE_MEDIA_AUDIO) += -DAUDIO_SUPPORT
ifeq ($(CONFIG_STATIC_COMPILER_SUPPORT), y)
  DEFS-$(CONFIG_MODULE_MEDIA_AUDIO) += -DCVIAUDIO_STATIC
endif
DEFS-$(CONFIG_MODULE_AUDIO_MP3) += -DMP3_SUPPORT

# UAC
DEFS-$(CONFIG_MODULE_CVIUAC) += -DCVI_UAC_SUPPORT

# UVC
DEFS-$(CONFIG_MODULE_CVIUVC) += -DCVI_UVC_SUPPORT
DEFS-$(CONFIG_MODULE_CVIUVC_HOST) += -DCVI_UVC_HOST_SUPPORT

# MD
DEFS-$(CONFIG_MODULE_AI_MD) += -DMD_SUPPORT
INCS-$(CONFIG_MODULE_AI_MD) += -I$(MW_PATH)/include/md
INCS-$(CONFIG_MODULE_AI_MD) += -I$(TDL_PATH)/install/include/md
INCS-$(CONFIG_MODULE_AI_MD) += -I$(TDL_PATH)/install/include/cvi_md

INCS-$(CONFIG_MODULE_AI) += -I$(APP_PREBUILT_DIR)/jpegturbo/include
DEFS-$(CONFIG_MODULE_AI) += -DAI_SUPPORT
INCS-$(CONFIG_MODULE_AI) += -I$(TDL_PATH)/install/CV184X/include
INCS-$(CONFIG_MODULE_AI) += -I$(TDL_PATH)/install/CV184X/include/c_apis
INCS-$(CONFIG_MODULE_AI) += -I$(TDL_PATH)/include/framework
INCS-$(CONFIG_MODULE_AI) += -I$(TDL_PATH)/include/components
INCS-$(CONFIG_MODULE_AI) += -I$(TDL_PATH)/include/nn
INCS-$(CONFIG_MODULE_AI) += -I$(TOP_DIR)/libsophon/install/libsophon-0.4.9/include

# AI_PD
DEFS-$(CONFIG_MODULE_AI_PD) += -DPD_SUPPORT

# AI_FACE
DEFS-$(CONFIG_MODULE_AI_FD_FACE) += -DFACE_SUPPORT

# AI_HAND
DEFS-$(CONFIG_MODULE_AI_HAND_DETECT) += -DHAND_DETECT_SUPPORT

# AI_CONSUMER
DEFS-$(CONFIG_MODULE_AI_CONSUMER_COUNTING) += -DCONSUMER_COUNTING_SUPPORT

# AI_IR_FACE
DEFS-$(CONFIG_MODULE_AI_IRFAECE) += -DIR_FACE_SUPPORT

# AI_BABY_CRY
DEFS-$(CONFIG_MODULE_AI_BABYCRY) += -DAI_BABYCRY_SUPPORT

# AI_HUMAN_KEYPOINT
DEFS-$(CONFIG_MODULE_AI_HUMAN_KEYPOINT) += -DHUMAN_KEYPOINT_SUPPORT

# AI_OBJECT_TRACK
DEFS-$(CONFIG_MODULE_AI_OBJECT_TRACK) += -DOBJECT_TRACK_SUPPORT

# AI_KEYPOINT_HAND_GESTURE
DEFS-$(CONFIG_MODULE_AI_KEYPOINT_HAND_GESTURE) += -DKEYPOINT_HAND_GESTURE_SUPPORT

# AI_IMAGE_TXT_CLIP
DEFS-$(CONFIG_MODULE_AI_IMG_TXT_CLIP) += -DIMG_TXT_CLIP_SUPPORT
# AI_LPR
DEFS-$(CONFIG_MODULE_AI_LPR) += -DLPR_SUPPORT

# RTSP
INCS-$(CONFIG_MODULE_RTSP)    += -I$(APP_COMPONENTS_INSTALL_DIR)/rtsp/include
INCS-$(CONFIG_MODULE_RTSP)    += -I$(APP_COMPONENTS_INSTALL_DIR)/osal/include
INCS-$(CONFIG_MODULE_RTSP)    += -I$(APP_COMPONENTS_INSTALL_DIR)/ringbuffer/include
DEFS-$(CONFIG_MODULE_RTSP)    += -DRTSP_SUPPORT
DEFS-$(CONFIG_RTSP_AUDIO_ENABLE) += -DRTSP_AUDIO_ENABLE

# PQTOOL
DEFS-$(CONFIG_MODULE_PQTOOL) += -DSUPPORT_ISP_PQTOOL
INCS-$(CONFIG_MODULE_PQTOOL) += -I$(MW_PATH)/include/isp

# RECORD
DEFS-$(CONFIG_MODULE_RECORD) += -DRECORD_SUPPORT
INCS-$(CONFIG_MODULE_RECORD) += -I$(APP_COMPONENTS_INSTALL_DIR)/ringbuffer/include
INCS-$(CONFIG_MODULE_RECORD) += -I$(APP_PREBUILT_DIR)/ffmpeg/include

# NETWORK
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/thttpd/include
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include/libwebsockets
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include/libwebsockets/protocols
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/libwebsockets/include/libwebsockets/transports
DEFS-$(CONFIG_MODULE_NETWORK) += -DWEB_SOCKET

# OPENSSL
INCS-$(CONFIG_MODULE_NETWORK) += -I$(APP_PREBUILT_DIR)/openssl/include

# SENSOR
INCS-y += -I$(SENSOR_LIST_INC)

GDB_DEBUG = 0
ifeq ($(GDB_DEBUG), 1)
  CFLAGS += -g -O0
endif

# module include
INCS-y                      			+= -I$(SRCTREE)/modules/media/include/module
INCS-$(CONFIG_MODULE_COMMON)			+= -I$(SRCTREE)/modules/common/include
INCS-$(CONFIG_MODULE_MEDIA_SYS)		+= -I$(SRCTREE)/modules/media/include/sys
INCS-$(CONFIG_MODULE_MEDIA_VI)		+= -I$(SRCTREE)/modules/media/include/vi
INCS-$(CONFIG_MODULE_MEDIA_VPSS)	+= -I$(SRCTREE)/modules/media/include/vpss
INCS-$(CONFIG_MODULE_MEDIA_VENC)	+= -I$(SRCTREE)/modules/media/include/venc
INCS-$(CONFIG_MODULE_MEDIA_OSD)		+= -I$(SRCTREE)/modules/media/include/osd
INCS-$(CONFIG_MODULE_MEDIA_VO)		+= -I$(SRCTREE)/modules/media/include/vo
INCS-$(CONFIG_MODULE_MEDIA_AUDIO)	+= -I$(SRCTREE)/modules/media/include/audio
INCS-$(CONFIG_MODULE_MEDIA_DEC)		+= -I$(SRCTREE)/modules/media/include/vdec
INCS-$(CONFIG_MODULE_MEDIA_MSG)		+= -I$(SRCTREE)/modules/media/include/msg
INCS-$(CONFIG_MODULE_PARAMPARSE)	+= -I$(SRCTREE)/modules/common/paramparse/include
INCS-$(CONFIG_MODULE_AI)				  += -I$(SRCTREE)/modules/ai/include
INCS-$(CONFIG_MODULE_CVIUAC)			+= -I$(SRCTREE)/modules/protocol/cvi_uac/include
INCS-$(CONFIG_MODULE_CVIUVC)			+= -I$(SRCTREE)/modules/protocol/cvi_uvc/include
INCS-$(CONFIG_MODULE_CVIUVC_HOST)		+= -I$(SRCTREE)/modules/protocol/cvi_uvc_host/include
INCS-$(CONFIG_MODULE_NETWORK)			+= -I$(SRCTREE)/modules/protocol/network/include
INCS-$(CONFIG_MODULE_RTSP)				+= -I$(SRCTREE)/modules/protocol/rtsp/include
INCS-$(CONFIG_MODULE_OTA)				  += -I$(SRCTREE)/modules/protocol/ota/include
INCS-$(CONFIG_MODULE_GPIO)				+= -I$(SRCTREE)/modules/peripheral/gpio/include
INCS-$(CONFIG_MODULE_PWM)				  += -I$(SRCTREE)/modules/peripheral/pwm/include
INCS-$(CONFIG_MODULE_IRCUT)				+= -I$(SRCTREE)/modules/peripheral/ircut/include
INCS-$(CONFIG_MODULE_RECORD)			+= -I$(SRCTREE)/modules/record/include/record
INCS-$(CONFIG_MODULE_RECORD)			+= -I$(SRCTREE)/modules/record/include/file_recover
INCS-$(CONFIG_MODULE_DISPLAY)			+= -I$(SRCTREE)/modules/display/include
INCS-$(CONFIG_MODULE_DISPLAY)			+= -I$(SRCTREE)/modules/peripheral/panel/include
INCS-$(CONFIG_MODULE_AI_MD)				+= -I$(SRCTREE)/modules/ai/md/include
INCS-y                            += -I$(APP_COMPONENTS_DIR)/cvi_osal/include

INCS += $(INCS-y)
TARGETFLAGS += $(INCS)
TARGETFLAGS += -Os -Wl,--gc-sections -rdynamic

ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
  CFLAGS		  += -MMD -Os -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
  TARGETFLAGS += -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
  CFLAGS		  += -MMD -Os -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
  TARGETFLAGS += -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
else ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
else ifeq ($(TARGET_MACHINE), aarch64-none-linux-gnu)
else ifeq ($(TARGET_MACHINE), arm-none-linux-musleabihf)
else ifeq ($(TARGET_MACHINE), arm-none-linux-gnueabihf)
else
  $(error "TARGET_MACHINE = $(TARGET_MACHINE) not match??")
endif

CFLAGS += -D__CV184X__

CFLAGS += -std=gnu11 -g -Wall -Wextra -Werror -fPIC -ffunction-sections -fdata-sections -Wl,--gc-sections
ifeq ($(findstring $(TARGET_MACHINE), arm-linux-gnueabihf aarch64-none-linux-gnu arm-none-linux-musleabihf arm-none-linux-gnueabihf),)
  CFLAGS += -mno-ldd
endif
CFLAGS += $(DEFS-y)
CFLAGS += -Wno-unused-parameter
