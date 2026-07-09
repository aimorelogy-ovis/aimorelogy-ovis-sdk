#!/bin/sh

echo 16777216 > /proc/sys/net/core/wmem_max
echo "4096 873800 16777216" > /proc/sys/net/ipv4/tcp_wmem
echo "3073344 4097792 16777216" > /proc/sys/net/ipv4/tcp_mem

CFG_JSON_FILE="./cfg.json"

if [ -z "$CVI_RTSP_JSON" ]; then
    export CVI_RTSP_JSON=$CFG_JSON_FILE
fi

sed -i 's/"replay-mode": true/"replay-mode": false/g' $CFG_JSON_FILE

while getopts "hrt:" OPTION; do
    case $OPTION in
        r)
            echo "start replay mode"
            sed -i 's/"replay-mode": false/"replay-mode": true/g' $CFG_JSON_FILE
            export CVI_REPLAY_MODE=1
            ;;
        t)
            echo "start isp auto test case: $OPTARG"
            export CVI_ISP_AUTO_TEST_CASE="$OPTARG"
            export PQBIN_DIR="/mnt/sd/res_mars3/pqbin"
            ;;
        h)
            echo "Usage:"
            echo "   -r     enter replay mode"
            echo "   -t N   start isp auto test case number N"
            echo "   -h     help (this output)"
            exit 0
            ;;
    esac
done

# for eaier debugging, add $PWD to LD_LIBRARY_PATH and PATH
SCRIPT_SELF=$(cd "$(dirname "$0")"; pwd)
export LD_LIBRARY_PATH=${SCRIPT_SELF}/lib:${SCRIPT_SELF}/lib/ai:${LD_LIBRARY_PATH}:/mnt/system/lib:/mnt/system/usr/lib:/mnt/system/usr/lib/3rd:/lib/3rd

PATH=${SCRIPT_SELF}:/mnt/system/usr/bin:$PATH
cd ${SCRIPT_SELF}
isp_tool_daemon
