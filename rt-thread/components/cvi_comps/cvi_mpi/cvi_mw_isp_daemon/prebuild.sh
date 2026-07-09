
ISP_EXECSH_DIR=./../cvi_mw_isp_common/toolJsonGenerator
ISP_DAEMON_SRC_DIR=./src
ISP_BIN_EXECSH_DIR=./../cvi_mw_isp_bin
ISP_PQTOOLJSON_FILE=${ISP_EXECSH_DIR}/pqtool_definition.json
OUTPUT=pqtool_definition.json
CVI_CHIP_ARCH=$1
if [ ${CVI_CHIP_ARCH} == "cv181xx" ] ; then
    CVI_CHIP_ARCH=cv181x
elif [ ${CVI_CHIP_ARCH} == "cv180xx" ] ; then
    CVI_CHIP_ARCH=cv180x
fi

pushd ${ISP_EXECSH_DIR}
source generate_toolJson.sh ${CVI_CHIP_ARCH}
popd
pushd ${ISP_BIN_EXECSH_DIR}
source CalPqBinMD5.sh
popd

cp -f ${ISP_PQTOOLJSON_FILE} ${ISP_DAEMON_SRC_DIR}
cd ${ISP_DAEMON_SRC_DIR}
xxd -i ${OUTPUT} > cvi_pqtool_json.h

if [ -f ${OUTPUT} ]
then
	rm ${OUTPUT}
fi
