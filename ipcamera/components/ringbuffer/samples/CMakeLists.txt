#Usage:
# step1: mkdir build && cd build
# step2:
# local gcc: cmake ../
# riscv64 musl: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-
# riscv64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/riscv64-linux-x86_64/bin/riscv64-unknown-linux-gnu-
# arm64 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
# arm32 glibc: cmake ../ -DGCC_CROSS_COMPILE=${HOME}/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
# step3: make
# step4: make install
# run: sample_ringbuffer.bin.static

cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本号
project(sample_ringbuffer VERSION 0.0.1 LANGUAGES C)
#
set(COMPS_TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "components directory." FORCE)
include("${COMPS_TOP_DIR}/../common_files/components_common.cmake")
# 设置安装前缀（如果是默认路径）
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
  set(CMAKE_INSTALL_PREFIX "${COMPS_INSTALL_DIR}/ini" CACHE PATH "Installation directory prefix." FORCE)
  message(STATUS "Setting default CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endif()

# 指定源文件
set(SAMPLE_RINGBUFFER_SOURCE_FILE sample_ringbuffer.c)

# 指定头文件目录
include_directories("${COMPS_TOP_DIR}/../common_files")
include_directories("${COMPS_INSTALL_DIR}/ringbuffer/include")

# 设置库路径和库文件
link_directories("${COMPS_INSTALL_DIR}/ringbuffer/lib")

# 生成可执行文件（动态链接）
add_executable(sample_ringbuffer ${SAMPLE_RINGBUFFER_SOURCE_FILE})
target_link_libraries(sample_ringbuffer PRIVATE -lcomp_ringbuffer)
set_target_properties(sample_ringbuffer PROPERTIES OUTPUT_NAME sample_ringbuffer.bin)

# 生成可执行文件（静态链接）
add_executable(sample_ringbuffer_static ${SAMPLE_RINGBUFFER_SOURCE_FILE})
target_link_libraries(sample_ringbuffer_static PRIVATE -Wl,-gc-sections -Wl,-Bstatic -lcomp_ringbuffer)
target_link_libraries(sample_ringbuffer_static PRIVATE -Wl,-Bdynamic -lm)
set_target_properties(sample_ringbuffer_static PROPERTIES OUTPUT_NAME sample_ringbuffer.bin.static)

# 设置安装目标
install(TARGETS sample_ringbuffer DESTINATION bin RENAME sample_ringbuffer.bin)
install(TARGETS sample_ringbuffer_static DESTINATION bin RENAME sample_ringbuffer.bin.static)

# 可选：添加编译选项或链接库（如果需要）
# target_compile_options(... PRIVATE -Wall -Wextra)
# target_link_libraries(... PRIVATE some_other_library)
