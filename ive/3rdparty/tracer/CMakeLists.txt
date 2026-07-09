cmake_minimum_required(VERSION 3.2.2)

if (CMAKE_C_COMPILER STREQUAL "/usr/bin/cc")
  message(FATAL_ERROR "Please use clang compiler. Please export C and CXX compiler to clang.")
endif()

add_definitions(-DENABLE_TRACE)

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

set(CMAKE_C_INIT "-fsigned-char -Werror=all -fdiagnostics-color=always")
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


project(cvitracer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/tracer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cvi_tracer.cpp)

add_library(cvitracer SHARED ${SRC})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cvi_tracer.h DESTINATION include/tracer)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tracer.h DESTINATION include/tracer)
install(TARGETS cvitracer DESTINATION lib)