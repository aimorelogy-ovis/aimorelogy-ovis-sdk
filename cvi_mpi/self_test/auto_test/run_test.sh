#!/bin/sh

UT_BIN_DIR=$(cd "$(dirname "$0")"; pwd)

BASEDIR=$(dirname "$0")
. $BASEDIR/env

function usage()
{
    echo "$0 [TEST CASE]"
    echo "              all"
    for f in $BASEDIR/test/*.sh;
    do
        echo "              ${f}"
    done
    echo ""
    echo "Mandatory env"
    echo "    UT_BIN_DIR                   - middleware unit test binary directory"
    echo ""
    echo "Optional env"
    echo "    TEST_TIMES                   - repeat times for each test case, default 1"
    echo "    SLEEP_SECONDS                - sleep seconds between each test case, default 3"
    echo ""

    exit 1
}

function run_all()
{
    for base_test in "run_sys_ut.sh" "run_vb_ut.sh"; do
        test_file="$BASEDIR/test/$base_test"
        if [ -f "$test_file" ]; then
            echo "=================== run $test_file ==================="
            i=1
            while [ $i -le $TEST_TIMES ]; do
                if [ $TEST_TIMES -gt 1 ]; then
                    echo "--- $test_file run times: $i ---"
                fi
                $test_file
                if [ $i -lt $TEST_TIMES ]; then
                    sleep $SLEEP_SECONDS
                fi
                i=$((i + 1))
            done
            if [ $TEST_TIMES -gt 1 ]; then
                echo "--- $test_file done, run $TEST_TIMES times ---"
            fi
            sleep $SLEEP_SECONDS
        fi
    done

    for f in $(ls $BASEDIR/test/*.sh | grep -v "run_sys_ut.sh" | grep -v "run_vb_ut.sh" | awk 'BEGIN{srand()} {print rand() "\t" $0}' | sort -n | cut -f2-);
    do
        echo "=================== run $f ==================="
        i=1
        while [ $i -le $TEST_TIMES ]; do
            if [ $TEST_TIMES -gt 1 ]; then
                echo "--- $f run times: $i ---"
            fi
            $f
            if [ $i -lt $TEST_TIMES ]; then
                sleep $SLEEP_SECONDS
            fi
            i=$((i + 1))
        done
        if [ $TEST_TIMES -gt 1 ]; then
            echo "--- $f done, run $TEST_TIMES times ---"
        fi
        sleep $SLEEP_SECONDS
    done
}

function run_single()
{
    $1
}

if [ -z $UT_BIN_DIR ]; then
    echo "UT_BIN_DIR is not set"
    echo ""
    usage
fi

export UT_BIN_DIR TEST_TIMES SLEEP_SECONDS

if [ "$1" == "all" ]; then
    run_all
elif [ -n $1 -a -x "$1" ]; then
    run_single "$1"
else
    usage
fi
