#!/bin/sh

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh

UT_BIN_NAME=vi_ut
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

TEST_CASES_linear="1 5 6 7 10 12 13 14"
TEST_CASES_wdr="1 6 7 10 12 13 14"
TEST_CASES_wdr_linear="6 10 12"
TEST_CASES_wdr_wdr="6 10 12"
TEST_CASES_yuv="6 10 13 14"
TEST_CASES_mipi_switch="6 10 11"
TEST_CASES_multi_init="22"

INI_DST_PATH=/mnt/data/sensor_cfg.ini

INI_PATH_linear=$BASEDIR/../res/sensor_cfg.ini.327_linear
INI_PATH_wdr=$BASEDIR/../res/sensor_cfg.ini.327_wdr
INI_PATH_wdr_linear=$BASEDIR/../res/sensor_cfg.ini.327_wdr_4653
INI_PATH_wdr_wdr=$BASEDIR/../res/sensor_cfg.ini.327_wdr_327_wdr
INI_PATH_yuv=$BASEDIR/../res/sensor_cfg.ini.pr2100

MIPI_SWITCH_INIS="$BASEDIR/../res/sensor_cfg.ini.327_triple_mipi_switch \
                  $BASEDIR/../res/sensor_cfg.ini.327_two_mipi_switch \
                  $BASEDIR/../res/sensor_cfg.ini.327_two_mipi_switch_normal"

function verify() {
    grep_result=$(grep "] pass" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $OUT_FILE $INPUT_FILE
}

function run_ut_test() {
    mode=$1
    ini_path=$2
    tests=$3

    cp $ini_path $INI_DST_PATH
    for i in $tests; do
        echo "========== test $i =========="
        cat /sys/kernel/debug/ion/cvi_carveout_heap_dump/summary
        $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
        verify
        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            return 1
        fi
        sleep $SLEEP_SECONDS
    done
}

function run_ut_mipi_switch_test() {
    for ini_path in $MIPI_SWITCH_INIS; do
        run_ut_test "mipi_switch" "$ini_path" "$TEST_CASES_mipi_switch" || return 1
    done
}

env_check
sample_check $UT_BIN_NAME
touch $INPUT_FILE

for t in $(seq 1 $TEST_TIMES)
do
    run_ut_test "linear"      "$INI_PATH_linear"      "$TEST_CASES_linear"      || break
    run_ut_test "wdr"         "$INI_PATH_wdr"         "$TEST_CASES_wdr"         || break
    run_ut_test "wdr_linear"  "$INI_PATH_wdr_linear"  "$TEST_CASES_wdr_linear"  || break
    run_ut_test "wdr_wdr"     "$INI_PATH_wdr_wdr"     "$TEST_CASES_wdr_wdr"     || break
    run_ut_test "yuv"         "$INI_PATH_yuv"         "$TEST_CASES_yuv"         || break
    run_ut_test "multi_init"  "$INI_PATH_wdr_linear"  "$TEST_CASES_multi_init"  || break
    run_ut_mipi_switch_test || break
done

clean_tmp_files

echo "========================================="
echo "middleware $UT_BIN_NAME $result"
echo "========================================="
