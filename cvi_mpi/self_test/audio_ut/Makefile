SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../Makefile.param
include $(PARAM_FILE)
endif
# include ../self_test.mk

ts := $(shell /bin/date "+%Y-%m-%d---%H-%M-%S")
SDIR = $(CURDIR)

BUILDLIST = audio_aec_test
BUILDLIST += audio_anr_test
BUILDLIST += audio_resample_test
BUILDLIST += audio_pcm_test
BUILDLIST += audio_transcode_test
BUILDLIST += audio_auto_test_sample
#BUILDLIST += audio_aac_recorder
#BUILDLIST += audio_mp3_recorder

.PHONY: clean

all: pre_check $(BUILDLIST) post_check

pre_check:
	@echo [aduio self_test]start Timestamp is $(ts)

post_check:
	@echo [aduio self_test]end Timestamp is $(ts)

audio_aec_test:
	@echo "self_tet audio_aec"
	@make -C $(SDIR)/audio_aec

audio_anr_test:
	@make -C $(SDIR)/audio_anr

audio_pcm_test:
	@make -C $(SDIR)/audio_pcm

audio_resample_test:
	@make -C $(SDIR)/audio_resample

audio_transcode_test:
	@make -C $(SDIR)/audio_transcode

audio_aac_recorder:
	@make -C $(SDIR)/cvi_aac_recorder

audio_mp3_recorder:
	@make -C $(SDIR)/cvi_mp3_recorder

audio_auto_test_sample:
	@make -C $(SDIR)/aud_auto_test_sample

install:
	@echo "[audio step]COPY LIB!! "



clean:
	@make -C $(SDIR)/audio_aec clean
	@make -C $(SDIR)/audio_anr clean
	@make -C $(SDIR)/audio_pcm clean
	@make -C $(SDIR)/audio_resample clean
	@make -C $(SDIR)/audio_transcode clean
#	@make -C $(SDIR)/cvi_aac_recorder clean
	@make -C $(SDIR)/cvi_mp3_recorder clean
	@make -C $(SDIR)/aud_auto_test_sample clean
