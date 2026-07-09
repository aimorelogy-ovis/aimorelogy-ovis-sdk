#!/bin/sh

# run example
#./isp_tool_daemon_test.sh --script-args 15 --vi-vpss-mode 0

CFG_PATH="./cfg.json"
SCRIPT_PATH="./isp_tool_daemon_test.sh"
SCRIPT_ARGS=""

SENSOR_CFG_OR="/mnt/data/sensor_cfg.ini"
SENSOR_CFG_CP=""

IS_REPLAY_MODE=""

MODIFICATIONS_TEMP=$(mktemp)
> "$MODIFICATIONS_TEMP"

parse_args() {
	while [ $# -gt 0 ]; do
		case "$1" in
			--dev-num)
				echo "dev-num=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--rtsp-port)
				echo "rtsp-port=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--vi-vpss-mode)
				echo "vi-vpss-mode=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--replay-mode)
				echo "replay-mode=$2" >> "$MODIFICATIONS_TEMP"
				local replay_json_path="/mnt/sd/res_mars3/raw/raw/replay.json"
				if [ -f $replay_json_path ]; then
					cat $replay_json_path > replay.json
					chmod 777 $replay_json_path
				fi
				IS_REPLAY_MODE="true"
				touch /mnt/data/raw_replay_start
				shift 2
				;;
			--cvi-bin-path)
				echo "cvi-bin-path=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--sensor-cfg-ini)
				if [ ! -f $SENSOR_CFG_OR ]; then
					return -1
				fi
				local sensor_ini_path=$(echo "$2" | tr -d '"')
				cp -f -p $SENSOR_CFG_OR $(dirname $sensor_ini_path) || return -1
				echo "sensor-cfg-ini=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--compress-mode)
				echo "compress-mode=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--enable-isp-info-osd)
				echo "enable-isp-info-osd=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--buf-blk-cnt)
				echo "buf-blk-cnt=$2" >> "$MODIFICATIONS_TEMP"
				shift 2
				;;
			--script-args)
				SCRIPT_ARGS="$2"
				shift 2
				;;
			*)
				echo "Unknown args: $1"
				exit 1
				;;
		esac
	done
	return 0
}

modify_cfg() {
	cp -p "$CFG_PATH" "$CFG_PATH.bak"
	while IFS='=' read -r key value; do
		if [ -n "$key" ] && [ -n "$value" ]; then
			echo "set key value: $key = $value"
			safe_key=$(echo "$key" | sed 's/[\/&]/\\&/g')
			safe_value=$(echo "$value" | sed 's/[\/&]/\\&/g')
			sed -i "s/^\([ \t]*\)\"$safe_key\"[ \t]*:.*/\1\"$safe_key\": $safe_value,/" "$CFG_PATH"
			sed -i "/\"video-src-info\"/,/]/ s/^\([ \t]*\)\"$safe_key\"[ \t]*:.*/\1\"$safe_key\": $safe_value,/" "$CFG_PATH"
		fi
	done < "$MODIFICATIONS_TEMP"
	echo "config update to: $CFG_PATH"
	echo "backup config: $CFG_PATH.bak"
}

run_script() {
	local SLEEP_SECS=$1
	if [ -z $SLEEP_SECS ]; then
		SLEEP_SECS=20
	fi

	if [ "$IS_REPLAY_MODE" == "true" ]; then
		sleep 10
		./CviIspTool.sh -r &
	else
		./CviIspTool.sh &
	fi

	echo "isp_tool_daemon running, waiting ${SLEEP_SECS}s ..."
	sleep $SLEEP_SECS

	PID=$(pidof "isp_tool_daemon")

	kill -SIGINT "$PID"
}

main() {
	parse_args "$@" || return -1


	cat "$MODIFICATIONS_TEMP"

	if [ -s "$MODIFICATIONS_TEMP" ]; then
		modify_cfg
	fi

	run_script $SCRIPT_ARGS
	rm -f "$MODIFICATIONS_TEMP"
	cp -p $CFG_PATH.bak $CFG_PATH
}

main "$@"
