include(CMakeForceCompiler)

# 设置C, CXX标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 如果指定了CMAKE_SYSROOT，则设置它
if(DEFINED CMAKE_SYSROOT)
  set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
endif()
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 设置编译器
if(NOT DEFINED GCC_CROSS_COMPILE)
    message(STATUS "Default use gcc compiler!")
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
else()
    message(STATUS "Use cross compiler: ${GCC_CROSS_COMPILE}")
    find_program(CMAKE_C_COMPILER_FOUND ${GCC_CROSS_COMPILE}gcc)
    if( NOT CMAKE_C_COMPILER_FOUND)
        message(FATAL_ERROR "${GCC_CROSS_COMPILE}gcc not found!")
    endif()
    set(CMAKE_C_COMPILER ${GCC_CROSS_COMPILE}gcc)
    set(CMAKE_CXX_COMPILER ${GCC_CROSS_COMPILE}g++)
endif()

# We must set the OBJCOPY setting into cache so that it's available to the
# whole project. Otherwise, this does not get set into the CACHE and therefore
# the build doesn't know what the OBJCOPY filepath is
set( CMAKE_OBJCOPY ${GCC_CROSS_COMPILE}objcopy CACHE FILEPATH "The toolchain objcopy command " FORCE )

# 设置目标架构
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE GENERATE_RESULT
    OUTPUT_VARIABLE GCC_TARGET_MACHINE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 设置OSS_INSTALL_DIR的默认值（如果未定义）
if(NOT DEFINED OSS_INSTALL_DIR)
  set(OSS_INSTALL_DIR "${COMPS_TOP_DIR}/../oss/oss_install/${GCC_TARGET_MACHINE}" CACHE PATH "Path to the OSS installation directory")
  message(STATUS "Setting default OSS_INSTALL_DIR: ${OSS_INSTALL_DIR}")
endif()

# 设置COMPS_INSTALL_DIR的默认值（如果未定义）
if(NOT DEFINED COMPS_INSTALL_DIR)
  set(COMPS_INSTALL_DIR "${COMPS_TOP_DIR}/comps_install/${GCC_TARGET_MACHINE}" CACHE PATH "Path to the components library installation directory")
  message(STATUS "Setting default BE_INSTALL_DIR: ${COMPS_INSTALL_DIR}")
endif()

# Set the CMAKE C flags (which should also be used by the assembler!
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC" )
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall -Wextra -Werror -O3")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -save-temps=obj")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -fsigned-char -ffunction-sections -fdata-sections" )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC" )
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps=obj")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE -fsigned-char -ffunction-sections -fdata-sections")

if(GCC_TARGET_MACHINE MATCHES "riscv64-unknown-linux-musl" OR GCC_TARGET_MACHINE MATCHES "riscv64-unknown-linux-gnu")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d")
endif()

if(GCC_TARGET_MACHINE MATCHES "aarch64-linux-gnu" OR GCC_TARGET_MACHINE MATCHES "aarch64-none-linux-gnu")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a")
endif()

if(GCC_TARGET_MACHINE MATCHES "arm-linux-gnueabihf")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard -mfpu=neon-vfpv4")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=hard -mfpu=neon-vfpv4")
endif()

if(GCC_TARGET_MACHINE MATCHES "x86_64-linux-gnu")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=x86-64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")