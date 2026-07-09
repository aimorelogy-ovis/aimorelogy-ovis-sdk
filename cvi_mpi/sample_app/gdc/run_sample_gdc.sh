#!/bin/sh

TEST_PASS="GDC-TEST-PASS"
TEST_FAIL="GDC-TEST-FAIL"

SAMPLE_BIN_NAME=sample_gdc
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function verify() {
    grep_result=$(grep -E "] pass" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}


touch $INPUT_FILE

for i in 0 1 2 3 4 5 6 7
do
	echo "========== gdc test case$i =========="
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