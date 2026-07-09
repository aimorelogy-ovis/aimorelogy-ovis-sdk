#!/bin/sh

TEST_PASS="SBM-TEST-PASS"
TEST_FAIL="SBM-TEST-FAIL"

SAMPLE_BIN_NAME=sample_sbm
OUT_FILE="tmp_output"
result=$TEST_PASS

mode=${1:-0}

case "$mode" in
	1) test_cases="0 1 2 4" ;;
	2) test_cases="3" ;;
	*)
		echo "Invalid mode $mode. Please use one of the following:"
		echo "  1 - single sensor"
		echo "  2 - dual sensor"
		exit 1
		;;
esac

get_expected_string() {
	case "${1}" in
		0)
			echo "sample_sbm exit success"
			;;
		4)
			echo "press 'Enter' to Snap frame."
			;;
		*)
			echo "press 'ctrl + c' to exit this sample."
			;;
	esac
}

verify() {
	local pid_value="${1}"
	local case_num="${2}"
	local check_ret=0

	if kill -0 "$pid_value" 2>/dev/null; then
		wait "$pid_value" 2>/dev/null
	fi

	sleep 0.5

	if [ ! -s "$OUT_FILE" ]; then
		check_ret=1
		return $check_ret
	fi

	expected_str=$(get_expected_string "$case_num")

	grep_result=$(grep -F "$expected_str" "$OUT_FILE")

	if [ -z "$grep_result" ]; then
		cat "$OUT_FILE"
		check_ret=1
	fi

	return $check_ret
}

for current_case in $test_cases
do
	echo "========== sbm test case$current_case =========="

	> "$OUT_FILE"

	./"$SAMPLE_BIN_NAME" "$current_case" > "$OUT_FILE" 2>&1 &
	pid_value=$!

	wait_time=0
	max_wait=5


	while [ $wait_time -lt $max_wait ]; do

		if [ -s "$OUT_FILE" ]; then
			break
		fi

		if ! kill -0 "$pid_value" 2>/dev/null; then
			break
		fi

		sleep 1
		wait_time=$((wait_time + 1))
	done

	if kill -0 "$pid_value" 2>/dev/null; then
		kill -INT "$pid_value"

		sleep 3
	fi

	verify "$pid_value" "$current_case"
	verify_ret=$?

	if [ $verify_ret -ne 0 ]; then
		result=$TEST_FAIL

		if kill -0 "$pid_value" 2>/dev/null; then
			kill -9 "$pid_value"
		fi
		break
	fi

	sleep 1
done

rm -f "$OUT_FILE"

echo "========================================="
echo "$result"
echo "========================================="
