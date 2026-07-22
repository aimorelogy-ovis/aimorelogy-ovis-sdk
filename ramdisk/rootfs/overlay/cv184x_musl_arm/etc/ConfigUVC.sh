#!/bin/sh

set -e
UVC_MIN_BIT_RATE=20000000
UVC_MAX_BIT_RATE=50000000
MAX_FRAME_SIZE=2097152
PARAM_CONFIG=${PARAM_CONFIG:-/mnt/cfg/ipcamera/param_config.ini}
UVC_INTERVAL_FILE=/var/run/ovis-uvc-frame-interval

CVI_GADGET=${CVI_GADGET:-/tmp/usb/usb_gadget/cvitek}

load_uvc_frame_interval() {
	uvc_fps=$(sed -n '/^\[vencchn3\]/,/^\[/ {
		s/^[[:space:]]*dst_framerate[[:space:]]*=[[:space:]]*\([0-9][0-9]*\).*$/\1/p
	}' "$PARAM_CONFIG" 2>/dev/null | sed -n '1p')

	case "$uvc_fps" in
		60)
			UVC_FRAME_INTERVAL=166666
			;;
		*)
			uvc_fps=30
			UVC_FRAME_INTERVAL=333333
			;;
	esac

	mkdir -p /var/run
	printf '%s\n' "$UVC_FRAME_INTERVAL" > "$UVC_INTERVAL_FILE"
}

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
	load_uvc_frame_interval

	control=$UVC_FUNCTION/control
	streaming=$UVC_FUNCTION/streaming
	format=$streaming/mjpeg/m
	frame=$format/1080p
	header=$streaming/header/h

	mkdir -p "$control/header/h"
	# This 5.10 UVC gadget emits the UVC 1.0 Processing Unit layout.
	# Advertising 1.1 makes Windows expect an extra bmVideoStandards byte.
	echo 0x0100 > "$control/header/h/bcdUVC"
	echo 48000000 > "$control/header/h/dwClockFrequency"
	mkdir -p "$frame"

	# MJPEG format index 1, frame index 1 follows the active sensor mode.
	echo 1920 > "$frame/wWidth"
	echo 1080 > "$frame/wHeight"
	echo "$UVC_MIN_BIT_RATE" > "$frame/dwMinBitRate"
	echo "$UVC_MAX_BIT_RATE" > "$frame/dwMaxBitRate"
	echo "$MAX_FRAME_SIZE" > "$frame/dwMaxVideoFrameBufferSize"
	echo "$UVC_FRAME_INTERVAL" > "$frame/dwDefaultFrameInterval"
	echo "$UVC_FRAME_INTERVAL" > "$frame/dwFrameInterval"
	echo "UVC descriptor: MJPEG 1920x1080 at ${uvc_fps} fps"

	mkdir -p "$header"
	add_link "$format" "$header/m"
	add_link "$control/header/h" "$control/class/fs/h"
	add_link "$control/header/h" "$control/class/ss/h"
	add_link "$header" "$streaming/class/fs/h"
	add_link "$header" "$streaming/class/hs/h"
	add_link "$header" "$streaming/class/ss/h"

	# Use all three high-speed isochronous transactions per microframe. This
	# reduces request completion pressure and leaves headroom for 1080p60 MJPEG
	# bursts while remaining within the USB 2.0 high-bandwidth endpoint limit.
	echo 0 > "$UVC_FUNCTION/streaming_bulk"
	echo 1 > "$UVC_FUNCTION/streaming_interval"
	echo 3072 > "$UVC_FUNCTION/streaming_maxpacket"
	echo 0 > "$UVC_FUNCTION/streaming_maxburst"
	echo 1 > "$UVC_FUNCTION/defer_connect"
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
	rm -f "$UVC_INTERVAL_FILE"
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
