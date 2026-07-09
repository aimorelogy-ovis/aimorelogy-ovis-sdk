#!/bin/sh

TEST_PASS="PANEL-TEST-PASS"
TEST_FAIL="PANEL-TEST-FAIL"

SAMPLE_PANEL_BIN=sample_panel
INPUT_FILE="tmp_input"
result=$TEST_PASS
TEST_TIMES=1

function clean_tmp_files() {
    rm -rf $INPUT_FILE
}

touch $INPUT_FILE

function run_all_sample_panel() {
    cmds_list="'sample_panel --panel=HX8394_EVB'
        'sample_panel -h'
        'sample_panel --show-pattern=0'
        'sample_panel --show-pattern=1'
        'sample_panel --show-pattern=2'
        'sample_panel --show-pattern=3'
        'sample_panel --show-pattern=4'
        'sample_panel --show-pattern=5'
        'sample_panel --show-pattern=6'
        'sample_panel --show-pattern=7'
        'sample_panel --show-pattern=8'
        'sample_panel --show-pattern=9'
        'sample_panel --panel=HX8394_EVB --show-pattern=6'
        'sample_panel --panel=HX8394_EVB --show-pattern=0'"

    while IFS= read -r cmd_str; do
        cmd_str_tmp="$cmd_str"
        cmd=$(echo "$cmd_str_tmp" | tr -d "'")
        cmd=$(echo "$cmd_str_tmp" | xargs)
        sample_bin_name=$(echo $cmd | cut -d' ' -f1)
        echo "========== test $cmd =========="
        ./$cmd < $INPUT_FILE
        ret_code=$?

        if [ $ret_code != 0 ]; then
            result=$TEST_FAIL
            break
        fi

        sleep 1
    done <<EOF
$cmds_list
EOF
}

for f in $(seq 1 $TEST_TIMES)
do
    run_all_sample_panel
done

clean_tmp_files

echo "========================================="
echo "$result"
echo "========================================="