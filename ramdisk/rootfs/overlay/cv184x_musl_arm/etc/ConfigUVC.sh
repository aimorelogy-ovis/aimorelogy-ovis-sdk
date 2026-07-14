#!/bin/sh

set -e
UVC_BIT_RATE=20000000
MAX_FRAME_SIZE=2097152

CVI_GADGET=${CVI_GADGET:-/tmp/usb/usb_gadget/cvitek}

find_uvc_function() {
	for path in "$CVI_GADGET"/functions/uvc.usb*; do
		[ -d "$path" ] || continue
		UVC_FUNCTION=$path
		return 0
	done

	echo "UVC function not found under $CVI_GADGET/functions" >&2
	return 1
}

add_link() {
	target=$1
	link=$2

	if [ -L "$link" ]; then
		return 0
	fi
	if [ -e "$link" ]; then
		echo "Cannot create UVC descriptor link: $link already exists" >&2
		return 1
	fi

	ln -s "$target" "$link"
}

remove_link() {
	if [ -L "$1" ]; then
		rm -f "$1"
	fi
}

remove_dir() {
	if [ -d "$1" ]; then
		rmdir "$1"
	fi
}

setup_uvc() {
	find_uvc_function

	control=$UVC_FUNCTION/control
	streaming=$UVC_FUNCTION/streaming
	format=$streaming/mjpeg/m
	frame=$format/1080p
	header=$streaming/header/h

	mkdir -p "$control/header/h"
	echo 0x0110 > "$control/header/h/bcdUVC"
	echo 48000000 > "$control/header/h/dwClockFrequency"
	mkdir -p "$frame"

	# MJPEG format index 1, frame index 1: 1920x1080 at 30 fps.
	echo 1920 > "$frame/wWidth"
	echo 1080 > "$frame/wHeight"
	echo "$UVC_BIT_RATE" > "$frame/dwMinBitRate"
	echo "$UVC_BIT_RATE" > "$frame/dwMaxBitRate"
	echo "$MAX_FRAME_SIZE" > "$frame/dwMaxVideoFrameBufferSize"
	echo 333333 > "$frame/dwDefaultFrameInterval"
	echo 333333 > "$frame/dwFrameInterval"

	mkdir -p "$header"
	add_link "$format" "$header/m"
	add_link "$control/header/h" "$control/class/fs/h"
	add_link "$control/header/h" "$control/class/ss/h"
	add_link "$header" "$streaming/class/fs/h"
	add_link "$header" "$streaming/class/hs/h"
	add_link "$header" "$streaming/class/ss/h"

	echo 1 > "$UVC_FUNCTION/streaming_interval"
	echo 3072 > "$UVC_FUNCTION/streaming_maxpacket"
	echo 0 > "$UVC_FUNCTION/streaming_maxburst"
}

cleanup_function() {
	function=$1
	control=$function/control
	streaming=$function/streaming

	remove_link "$streaming/class/ss/h"
	remove_link "$streaming/class/hs/h"
	remove_link "$streaming/class/fs/h"
	remove_link "$control/class/ss/h"
	remove_link "$control/class/fs/h"
	remove_link "$streaming/header/h/m"

	remove_dir "$streaming/header/h"
	remove_dir "$streaming/mjpeg/m/1080p"
	remove_dir "$streaming/mjpeg/m"
	remove_dir "$control/header/h"
}

cleanup_uvc() {
	for function in "$CVI_GADGET"/functions/uvc.usb*; do
		[ -d "$function" ] || continue
		cleanup_function "$function"
	done
}

case "$1" in
	setup)
		setup_uvc
		;;
	cleanup)
		cleanup_uvc
		;;
	*)
		echo "Usage: $0 {setup|cleanup}" >&2
		exit 1
		;;
esac
