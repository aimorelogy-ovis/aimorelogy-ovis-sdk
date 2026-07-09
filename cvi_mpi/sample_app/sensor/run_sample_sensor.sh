#!/bin/sh

TEST_PASS="SENSOR-TEST-PASS"
TEST_FAIL="SENSOR-TEST-FAIL"

SAMPLE_BIN_NAME=sample_sensor
OUT_FILE="tmp_output"
result=$TEST_PASS
check_ret=0

function verify() {
    if grep -q -E "ERROR|failed" "$OUT_FILE"; then
        check_ret=-1
    else
        check_ret=0
    fi
}

MODE=${1:-0}

function run_all_sensor_test() {
    cmds_list=""

    case $MODE in
        0)
            cmds_list="'sample_sensor 0 2 0 1 255'"
            ;;
        1)
            cmds_list="'sample_sensor 0 1 0 1 2 0 1 255'"
            ;;
        2)
            cmds_list="'sample_sensor 0 0 1 0 1 1 1 1 2 0 1 2 1 1 255'"
            ;;
        *)
            echo "Invalid mode $MODE. Please use one of the following:"
            echo "  0 - no sensor"
            echo "  1 - single sensor"
            echo "  2 - dual sensor"
            exit 1
            ;;
    esac

    while IFS= read -r cmd_str; do
        cmd_str_tmp="$cmd_str"
        cmd=$(echo "$cmd_str_tmp" | tr -d "'")
        cmd=$(echo "$cmd" | xargs)
        sample_bin_name=$(echo $cmd | cut -d' ' -f1)
        sample_bin_params=$(echo $cmd | cut -d' ' -f2-)

        printf "%s\n" $(echo $sample_bin_params) | ./$sample_bin_name | tee $OUT_FILE
        verify $?

        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            break
        fi

        sleep 0.5

    done <<EOF
$cmds_list
EOF
}

TEST_TIMES=${TEST_TIMES:-1}

for f in $(seq 1 $TEST_TIMES)
do
    run_all_sensor_test
done

rm -f /mnt/data/*.yuv /mnt/data/*.raw

echo "========================================="
echo "$result"
echo "========================================="
