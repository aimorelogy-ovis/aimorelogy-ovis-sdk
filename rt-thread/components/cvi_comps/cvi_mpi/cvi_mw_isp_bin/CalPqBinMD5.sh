#!/bin/sh

CUR_PATH=${PWD}
#FILE_LIST="../../include/cv181x/cvi_comm_3a.h ../../include/cv181x/cvi_comm_isp.h ../../../../../include/cvi_comm_sns.h"
FILE_LIST="${CUR_PATH}/../include/isp/cv181x/cvi_comm_3a.h ${CUR_PATH}/../include/isp/cv181x/cvi_comm_isp.h"
OUTFILE="pqbin.$"
SRC_FILE="src/isp_bin.c"

for file in $FILE_LIST
do
    cat "$file" >> "$OUTFILE" 2>&1
    if [ $? -ne 0 ]; then
        sed -i "s|\(#define ISP_BIN_MD5 \).*|\1\"NULL\"|" "$SRC_FILE"
        rm -f "$OUTFILE"
        echo "filecat error $?"
        exit 1
    fi
done

result=$(md5sum "$OUTFILE" | cut -d" " -f1)

if [ $? -eq 0 ]; then
    sed -i "s|\(#define ISP_BIN_MD5 \).*|\1\"${result}\"|" "$SRC_FILE"
    echo "pqbin md5sum success!"
else
    sed -i "s|\(#define ISP_BIN_MD5 \).*|\1\"NULL\"|" "$SRC_FILE"
    echo "md5sum error $?"
fi

rm -f "$OUTFILE"
