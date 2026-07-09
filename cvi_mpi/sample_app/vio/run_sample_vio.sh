#!/bin/sh

TEST_PASS="VIO-TEST-PASS"
TEST_FAIL="VIO-TEST-FAIL"

SAMPLE_BIN_NAME=sample_vio
OUT_FILE="tmp_output"
result=$TEST_PASS

if [ $# -ne 1 ] || ! echo "$1" | grep -E -q '^[012]$'; then
    echo "Usage: $0 [0|1|2]"
    echo "  0 - no sensor"
    echo "  1 - single sensor"
    echo "  2 - dual sensor"
    exit 1
fi

if [ "$1" = "2" ]; then
    CASES="0 1 2 3 4 5"
else
    CASES="0 1 2 3 5"
fi

for i in $CASES
do
    echo "========== vio test case $i =========="
    if [ "$i" -eq 5 ]; then
        SLEEP_TIME=50
    else
        SLEEP_TIME=20
    fi

    if [ "$i" -eq 4 ]; then
        (
            sleep 10
            echo "1"
            sleep 10
            echo "0"
            sleep 5
            echo "255"
        ) | ./$SAMPLE_BIN_NAME $i | tee $OUT_FILE
    else
        (
            sleep $SLEEP_TIME
            echo "exit"
        ) | ./$SAMPLE_BIN_NAME $i | tee $OUT_FILE
    fi

    grep "sample_vio exit success" $OUT_FILE > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        result=$TEST_FAIL
        break
    fi
    sleep 1
done

rm -rf $OUT_FILE

echo "========================================="
echo "$result"
echo "========================================="
