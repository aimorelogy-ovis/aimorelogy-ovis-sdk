cmake_minimum_required(VERSION 3.2.2)

if (CMAKE_C_COMPILER STREQUAL "/usr/bin/cc")
  message(FATAL_ERROR "Please use clang compiler. Please export C and CXX compiler to clang.")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release")
endif()

if("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
  message("No toolchain file found. Using host compiler.")
  if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")
  endif()
else()
  if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install_soc")
  endif()
endif()

set(CMAKE_C_INIT "-fsigned-char -fPIC -Werror=all -fdiagnostics-color=always")
set(CMAKE_CXX_INIT "-fsigned-char -fPIC -Werror=all -fdiagnostics-color=always -std=gnu++11")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set( CMAKE_C_FLAGS "${CMAKE_C_INIT} -O3" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_INIT} -O3" )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set( CMAKE_C_FLAGS "${CMAKE_C_INIT} -g -O0" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_INIT} -g -O0" )
else()
  message(FATAL_ERROR "No build type!!!")
endif()

message("==================================================")
message("[Summary]")
message("C   compiler ${CMAKE_C_COMPILER}")
message("CXX compiler ${CMAKE_CXX_COMPILER}")
message("Build type   ${CMAKE_BUILD_TYPE}")
message("Install dir  ${CMAKE_INSTALL_PREFIX}")
message("==================================================")

include_directories(
  ${GST_ROOT}/include/glib-2.0
  ${GST_ROOT}/include/gstreamer-1.0
  ${IVE_SDK_ROOT}/include/ive
  ${IVE_SDK_ROOT}/include/tracer
  ${MIDDLEWARE_SDK_ROOT}/include)

set(GLIB_LIBS "-L${GST_ROOT}/lib -lgobject-2.0 -lgthread-2.0 -lgmodule-2.0 -lglib-2.0")
set(GST_LIBS "-L${GST_ROOT}/lib -lgstvideo-1.0 -lgstbase-1.0 -lgstreamer-1.0")
set(TPU_LIBS "-L${TPU_SDK_ROOT}/lib -lcvikernel -lcviruntime")
set(IVE_LIBS "-L${IVE_SDK_ROOT}/lib -ltracer -lcvi_ive_tpu")

set(PLUGIN_LIBS "${GLIB_LIBS} ${GST_LIBS} ${TPU_LIBS} ${IVE_LIBS}")

add_subdirectory(background)
