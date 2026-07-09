#!/bin/sh

TEST_PASS="GFBG-TEST-PASS"
TEST_FAIL="GFBG-TEST-FAIL"
MODULE_PATH="/system/ko/cv184x_gfbg.ko"

SAMPLE_BIN_NAME=sample_gfbg
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function load_module() {
    if ! lsmod | grep -q "cv184x_gfbg"; then
        echo "[INFO] Loading kernel module: $MODULE_PATH"
        insmod $MODULE_PATH || {
            echo "[ERROR] Failed to load module!"
            exit 1
        }
    else
        echo "[INFO] Module already loaded."
    fi
}

load_module

function verify() {
    grep_result=$(grep -E "program exit normally" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

touch $INPUT_FILE

for i in 0 1 2 3 4 5
do
	echo "========== gfbg test case$i =========="
	./$SAMPLE_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
	verify

	if [ $check_ret != 0 ]; then
		result=$TEST_FAIL
		break
	fi
	sleep 1
done

rm -rf $OUT_FILE $INPUT_FILE

echo "========================================="
echo "$result"
echo "========================================="