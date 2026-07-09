#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
IVE_ROOT=$(readlink -f $SCRIPT_DIR/../)
TMP_WORKING_DIR=$IVE_ROOT/tmp

echo "Creating tmp working directory."

mkdir -p $TMP_WORKING_DIR/build_sdk
pushd $TMP_WORKING_DIR/build_sdk

if [[ "$SDK_VER" == "uclibc" ]]; then
    TOOLCHAIN_FILE=$IVE_ROOT/toolchain/toolchain-uclibc-linux.cmake
elif [[ "$SDK_VER" == "glibc_arm64" ]]; then
    TOOLCHAIN_FILE=$IVE_ROOT/toolchain/toolchain-aarch64-linux.cmake
elif [[ "$SDK_VER" == "glibc_arm" ]]; then
    TOOLCHAIN_FILE=$IVE_ROOT/toolchain/toolchain-gnueabihf-linux.cmake
elif [[ "$SDK_VER" == "musl_riscv64" ]]; then
    KERNEL_HEADER_PATH="${KERNEL_PATH}"/build/"${PROJECT_FULLNAME}"/riscv/usr/
    TOOLCHAIN_FILE=$IVE_ROOT/toolchain/toolchain-riscv64-musl.cmake
elif [[ "$SDK_VER" == "glibc_riscv64" ]]; then
    TOOLCHAIN_FILE=$IVE_ROOT/toolchain/toolchain-riscv64-linux.cmake
    KERNEL_HEADER_PATH="${KERNEL_PATH}"/build/"${PROJECT_FULLNAME}"/riscv/usr/
elif [[ "$SDK_VER" == "musl_arm" ]]; then
    TOOLCHAIN_FILE=$IVE_ROOT/toolchain/arm-none-linux-musleabihf.cmake
elif [[ "$SDK_VER" == "musl_arm64" ]]; then
    TOOLCHAIN_FILE=$IVE_ROOT/toolchain/aarch64-none-linux-musl.cmake
else
    echo "Wrong SDK_VER=$SDK_VER"
    exit 1
fi

echo "CHIP_ARCH:" $CHIP_ARCH
echo "TOOLCHAIN:" $TOOLCHAIN_FILE
echo "HOST_TOOL_PATH:" $HOST_TOOL_PATH
cmake -G Ninja $IVE_ROOT -DCVI_TARGET=soc \
                            -DCVI_PLATFORM=$CHIP_ARCH \
                            -DENABLE_SYSTRACE=OFF \
                            -DCMAKE_BUILD_TYPE=SDKRelease \
                            -DKERNEL_HEADERS_ROOT=$KERNEL_HEADER_PATH \
                            -DMIDDLEWARE_SDK_ROOT=$MW_PATH \
                            -DCMAKE_INSTALL_PREFIX=$IVE_SDK_INSTALL_PATH \
                            -DTOOLCHAIN_ROOT_DIR=$HOST_TOOL_PATH \
                            -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
                            -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
# cmake -G Ninja $IVE_ROOT -DCVI_TARGET=soc \
#                             -DCVI_PLATFORM=$CHIP_ARCH \
#                             -DENABLE_SYSTRACE=OFF \
#                             -DCMAKE_BUILD_TYPE=SDKRelease \
#                             -DKERNEL_HEADERS_ROOT=$KERNEL_HEADER_PATH \
#                             -DMLIR_SDK_ROOT=$TPU_SDK_INSTALL_PATH \
#                             -DMIDDLEWARE_SDK_ROOT=$MW_PATH \
#                             -DCMAKE_INSTALL_PREFIX=$IVE_SDK_INSTALL_PATH \
#                             -DTOOLCHAIN_ROOT_DIR=$HOST_TOOL_PATH \
#                             -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
#                             -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON

ninja -j8 || exit 1
ninja install || exit 1
popd

# cd $TMP_WORKING_DIR
# echo "Compressing SDK release..."
# tar cf - ivesdk -P | pv -s $(du -sb ivesdk | awk '{print $1}') | gzip > $IVE_ROOT/$IVE_SDK_NAME.tar.gz
# echo "Output md5 sum."
# md5sum $IVE_ROOT/$IVE_SDK_NAME.tar.gz
echo "Cleanup tmp folder."
rm -r $TMP_WORKING_DIR
