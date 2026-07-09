# IVE gst-plugin

IVE based gst-plugin.

Note:

**1. bmtap2 docker environment is recomended.**

## How to build

### Requirements

1. Middleware headers
2. MLIR SDK
3. Tracer lib
4. IVE SDK

SOC mode

```
$ mkdir build
$ cd build
$ cmake -G Ninja .. -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
                    -DTOOLCHAIN_ROOT_DIR=${PWD}/../../../gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu \
                    -DCMAKE_TOOLCHAIN_FILE=${PWD}/../../toolchain/toolchain-aarch64-linux.cmake \
                    -DCMAKE_BUILD_TYPE=Release \
                    -DCMAKE_INSTALL_PREFIX= \
                    -DGST_ROOT= \
                    -DMIDDLEWARE_SDK_ROOT= \
                    -DTPU_SDK_ROOT= \
                    -DIVE_SDK_ROOT=
$ ninja -j8 && ninja install
```
