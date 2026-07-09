#!/bin/bash

CHIP_ID=$1
CUR_PATH=$(cd "$(dirname "$0")"; pwd)
ISP_BASE_PATH=${CUR_PATH}/../../cvi_mw_isp_bin
MW_INCLUDE_PATH=${CUR_PATH}/../../include
ISP_INCLUDE_PATH=${MW_INCLUDE_PATH}/isp/$CHIP_ID
OSDRV_INCLUDE_PATH=${CUR_PATH}/../../../cvi_osdrv/include/common/uapi

HEADERLIST="${ISP_INCLUDE_PATH}/cvi_comm_isp.h"
HEADERLIST+=" ${ISP_INCLUDE_PATH}/cvi_comm_3a.h"
HEADERLIST+=" ${OSDRV_INCLUDE_PATH}/cvi_comm_video.h"
HEADERLIST+=" ${OSDRV_INCLUDE_PATH}/cvi_comm_vo.h"
HEADERLIST+=" ${MW_INCLUDE_PATH}/cvi_comm_sns.h"
ISP_JSON_STRUCTFILE="${ISP_BASE_PATH}/src/isp_json_struct.c"
APP=checkPqBinJsonIntegrity.py
cd ${CUR_PATH}
python ${APP} ${ISP_JSON_STRUCTFILE} ${HEADERLIST}

