#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TRACER_ROOT=$(readlink -f $SCRIPT_DIR/../)
TMP_WORKING_DIR=$TRACER_ROOT/tmp

echo "Creating tmp working directory."
if [[ "$1" == "cmodel" ]]; then
    mkdir -p $TMP_WORKING_DIR/build_cmodel
    pushd $TMP_WORKING_DIR/build_cmodel
    CC=clang CXX=clang++ \
    cmake -G Ninja $TRACER_ROOT -DCMAKE_BUILD_TYPE=Release \
                                -DCMAKE_INSTALL_PREFIX=$TRACER_CMODEL_INSTALL_PATH \
    ninja -j8 && ninja install
    popd
elif [[ "$1" == "soc" ]]; then
    mkdir -p $TMP_WORKING_DIR/build_sdk
    pushd $TMP_WORKING_DIR/build_sdk
    cmake -G Ninja $TRACER_ROOT -DCMAKE_BUILD_TYPE=Release \
                                -DCMAKE_INSTALL_PREFIX=$TRACER_SDK_INSTALL_PATH \
                                -DTOOLCHAIN_ROOT_DIR=$HOST_TOOL_PATH \
                                -DCMAKE_TOOLCHAIN_FILE=$TRACER_ROOT/toolchain/toolchain-aarch64-linux.cmake
    ninja -j8 && ninja install
    popd
elif [[ "$1" == "soc32" ]]; then
    mkdir -p $TMP_WORKING_DIR/build_sdk
    pushd $TMP_WORKING_DIR/build_sdk
    cmake -G Ninja $TRACER_ROOT -DCMAKE_BUILD_TYPE=Release \
                                -DCMAKE_INSTALL_PREFIX=$TRACER_SDK_INSTALL_PATH \
                                -DTOOLCHAIN_ROOT_DIR=$HOST_TOOL_PATH \
                                -DCMAKE_TOOLCHAIN_FILE=$TRACER_ROOT/toolchain/toolchain-gnueabihf-linux.cmake
    ninja -j8 && ninja install
    popd
else
  echo "Unsupported build type."
  exit 1
fi
echo "Cleanup tmp folder."
rm -r $TMP_WORKING_DIR
