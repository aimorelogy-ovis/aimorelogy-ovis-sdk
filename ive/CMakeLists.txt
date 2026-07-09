cmake_minimum_required(VERSION 3.2.2)
project(ive)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create target directory for TPU kernel soft links first
set(TPU_KERNEL_LINK_TARGET_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/tpu_kernel_module")
if(NOT EXISTS "${TPU_KERNEL_LINK_TARGET_ROOT}")
    file(MAKE_DIRECTORY "${TPU_KERNEL_LINK_TARGET_ROOT}")
    message(STATUS "Created tpu_kernel_module directory under 3rdparty: ${TPU_KERNEL_LINK_TARGET_ROOT}")
else()
    message(STATUS "tpu_kernel_module directory under 3rdparty already exists: ${TPU_KERNEL_LINK_TARGET_ROOT}")
endif()

# Configure parent directory of TPU kernel source (contains subdirectories to be linked)
set(TPU_KERNEL_LIB_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libsophon/tpu-kernel/lib")

# Check if source parent directory exists; abort subsequent steps if not
if(NOT EXISTS "${TPU_KERNEL_LIB_PARENT_DIR}")
    message(WARNING "Source parent directory for soft links does not exist; cannot proceed: ${TPU_KERNEL_LIB_PARENT_DIR}")
else()
    # Get all subdirectories under the source parent directory (filter out files, only keep directories)
    file(GLOB SUBDIRS RELATIVE "${TPU_KERNEL_LIB_PARENT_DIR}" "${TPU_KERNEL_LIB_PARENT_DIR}/*")
    foreach(SUBDIR ${SUBDIRS})
        set(SRC_SUB_DIR "${TPU_KERNEL_LIB_PARENT_DIR}/${SUBDIR}")
        if(IS_DIRECTORY "${SRC_SUB_DIR}")
            # Define path for the target soft link (same name as the source subdirectory)
            set(CURRENT_LINK "${TPU_KERNEL_LINK_TARGET_ROOT}/${SUBDIR}")
            execute_process(
                COMMAND ln -sf "${SRC_SUB_DIR}" "${CURRENT_LINK}"
                RESULT_VARIABLE LINK_RESULT
                ERROR_VARIABLE LINK_ERROR
            )

            # Output result of soft link creation
            if(LINK_RESULT EQUAL 0)
                message(STATUS "Successfully created soft link: ${CURRENT_LINK} -> ${SRC_SUB_DIR}")
            else()
                message(WARNING "Failed to create soft link (subdirectory: ${SUBDIR}): ${LINK_ERROR}")
            endif()
        endif()
    endforeach()

    # Check if any subdirectories were processed; warn if none
    list(LENGTH SUBDIRS SUBDIR_COUNT)
    if(SUBDIR_COUNT EQUAL 0)
        message(WARNING "No subdirectories found under source parent directory; no soft links to create: ${TPU_KERNEL_LIB_PARENT_DIR}")
    endif()
endif()

# Options
option(ENABLE_SYSTRACE "Use systrace" OFF)
# Get current commit
execute_process(
  COMMAND git show --oneline -s
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE CURRENT_COMMIT
  ERROR_VARIABLE GIT_ERROR
  RESULT_VARIABLE GIT_RESULT)

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE LATEST_BRANCH
  ERROR_VARIABLE GIT_BRANCH_ERROR
  RESULT_VARIABLE GIT_BRANCH_RESULT)

if(GIT_RESULT EQUAL 0 AND GIT_BRANCH_RESULT EQUAL 0)
  string(STRIP ${LATEST_BRANCH} LATEST_BRANCH)
else()
  set(CURRENT_COMMIT "unknown")
  set(LATEST_BRANCH "unknown")
  message(WARNING "Not a git repository or git commands failed. Setting version info to 'unknown'")
endif()

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
message("CMAKE_C_COMPILER" ${CMAKE_C_COMPILER} )
message("tcpath" ${TC_PATH}${CROSS_COMPILE}gcc)
if ("${LATEST_BRANCH}" STREQUAL "master")
  if(NOT "${CURRENT_COMMIT}" STREQUAL "unknown")
    string(REPLACE " " ";" WORD_LIST ${CURRENT_COMMIT})
    list(GET WORD_LIST 0 COMMITID)
    set(LATEST_TAG "nightly-${COMMITID}")
  else()
    set(LATEST_TAG "nightly-unknown")
  endif()
elseif("${LATEST_BRANCH}" STREQUAL "HEAD")
  if(NOT "${CURRENT_COMMIT}" STREQUAL "unknown")
    string(REPLACE " " ";" WORD_LIST ${CURRENT_COMMIT})
    list(GET WORD_LIST 0 COMMITID)
    set(LATEST_TAG "${COMMITID}")
  else()
    set(LATEST_TAG "unknown")
  endif()
elseif("${LATEST_BRANCH}" STREQUAL "unknown")
  set(LATEST_TAG "unknown")
else()
  string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)*[^(\\\n).]+" LATEST_TAG ${LATEST_BRANCH})
  if ("${LATEST_TAG}" STREQUAL "")
    set(LATEST_TAG "unknown")
  endif()
  if(NOT "${CURRENT_COMMIT}" STREQUAL "unknown")
    string(REPLACE " " ";" WORD_LIST ${CURRENT_COMMIT})
    list(GET WORD_LIST 0 COMMITID)
    set(LATEST_TAG "${LATEST_TAG}-${COMMITID}")
  endif()
endif()

set(CMAKE_C_COMPILER ${TC_PATH}${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${TC_PATH}${CROSS_COMPILE}g++)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release")
endif()

if("${CVI_PLATFORM}" STREQUAL "")
  set(CVI_PLATFORM "cv1835")
  message(AUTHOR_WARNING "Platform not provided, set to ${CVI_PLATFORM}.")
endif()

# Reset flag string
set(WORKAROUND_FLAGS_STR "")

# platform specific settings
if("${CVI_PLATFORM}" STREQUAL "CV183X")
  add_definitions(-DWORKAROUND_SCALAR_4096_ALIGN_BUG)
  set(WORKAROUND_FLAGS_STR "${WORKAROUND_FLAGS_STR} WORKAROUND_SCALAR_4096_ALIGN_BUG")
elseif ("${CVI_PLATFORM}" STREQUAL "CV180X")
  add_definitions(-DCV180X)
  add_definitions(-D__CV180X__)
endif()

if("${CVI_TARGET}" STREQUAL "soc")
  add_definitions(-DCVI_SOC)
  if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install_soc")
  endif()
  # Find toolchain cmake file and toolchain folder
  if("${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is not set. Aborting.")
  endif()
  set(USE_TOOLCHAIN "true")
  set(SSE_FLAGS "")
  set(GCC_STRIP_FLAG "-s")
else()
  set(CVI_TARGET "cmodel")
  if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")
  endif()
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(USE_TOOLCHAIN "false")
  set(SSE_FLAGS "-mssse3 -msse4.1 -mavx2 -mfma")
  set(GCC_STRIP_FLAG "")
endif()

# # Check if tracer exists.
# if ("${TRACER_ROOT}" STREQUAL "")
#   set(TRACER_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/tracer)
#   message("use tracerroot:" ${TRACER_ROOT})
# endif()
# if(EXISTS "${TRACER_ROOT}")
#   execute_process(COMMAND git remote -v
#                   WORKING_DIRECTORY ${TRACER_ROOT}
#                   OUTPUT_VARIABLE TRACER_REMOTE_URL)
#   if (NOT "${TRACER_REMOTE_URL}" MATCHES "10.34.33.3:8480" OR
#           "${TRACER_REMOTE_URL}" MATCHES "10.34.33.3:8422")
#     message(AUTHOR_WARNING "Warning, this tracer is not cloned from GitLab. "
#                            "May cause error while compiling or running.")
#   endif()
#   message("-- Found Tracer (directory: ${CMAKE_SOURCE_DIR}/3rdparty/tracer)")
#   include(cmake/tracer.cmake)
# else()
#   message(FATAL_ERROR "Tracer folder not found. Git clone to 3rdparty at "
#                       "http://10.34.33.3:8480/sys_app/tracer.")
# endif()
# if (ENABLE_SYSTRACE)
#   add_definitions(-DENABLE_TRACE)
# endif()

# Check if MLIR SDK exists.
# include(cmake/mlir.cmake)

# Check if middleware exists.
include (cmake/middleware.cmake)

#include sysroot/include
include_directories(${CMAKE_SYSROOT}/include)

# include kernel headers for user space
include_directories(${KERNEL_HEADERS_ROOT}/include)

set(CMAKE_C_INIT "-fsigned-char -Werror=all -fdiagnostics-color=always ${SSE_FLAGS}")
set(CMAKE_CXX_INIT "-fsigned-char -fPIC -Werror=all -fdiagnostics-color=always -std=gnu++11 ${SSE_FLAGS}")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_INIT} ${CMAKE_C_FLAGS_RELEASE}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_INIT} ${CMAKE_CXX_FLAGS_RELEASE}" )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "SDKRelease")
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_INIT} ${CMAKE_C_FLAGS_RELEASE} ${GCC_STRIP_FLAG}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_INIT} ${CMAKE_CXX_FLAGS_RELEASE} ${GCC_STRIP_FLAG}" )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_INIT} ${CMAKE_C_FLAGS_DEBUG}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_INIT} ${CMAKE_CXX_FLAGS_DEBUG}" )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Asan")
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_INIT} ${CMAKE_C_FLAGS_DEBUG} -fsanitize=address" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_INIT} ${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Tsan")
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_INIT} ${CMAKE_C_FLAGS_DEBUG} -fsanitize=thread" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_INIT} ${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread" )
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "UBsan")
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_INIT} ${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_INIT} ${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined" )
    set( CMAKE_EXE_LINKER_FLAGS "-lubsan")
else()
    message(FATAL_ERROR "No build type!!!")
endif()

message("==================================================")
message("Latest tag       ${LATEST_TAG}")
message("Commit           ${CURRENT_COMMIT}")
message("[Summary]")
message("Platform         ${CVI_PLATFORM}")
message("Target           ${CVI_TARGET}")
message("Build type       ${CMAKE_BUILD_TYPE}")
message("C   compiler     ${CMAKE_C_COMPILER}")
message("CXX compiler     ${CMAKE_CXX_COMPILER}")
message("C Flags          ${CMAKE_C_FLAGS}")
message("CXX Flags        ${CMAKE_CXX_FLAGS}")
message("SYSROOT          ${CMAKE_SYSROOT}")
message("Install dir      ${CMAKE_INSTALL_PREFIX}")
message("Use Systrace     ${ENABLE_SYSTRACE}")
message("")
message("Workaround flags ${WORKAROUND_FLAGS_STR}")
message("==================================================")

# add_subdirectory(tpu_math)
add_subdirectory(src)
add_subdirectory(sample)
#if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "SDKRelease")
  add_subdirectory(test)
#endif()
# Install cmake file
install(FILES cmake/ive.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/)
