project(tracer)
include(ExternalProject)
set_directory_properties(PROPERTIES EP_BASE ${CMAKE_CURRENT_BINARY_DIR})
# Only Ninja needs this => BUILD_BYPRODUCTS.
ExternalProject_Add(tracer
                    BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libcvitracer.so
                    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/tracer
                    INSTALL_DIR ${INSTALL_DIR}
                    CMAKE_ARGS
                    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                    -DTOOLCHAIN_ROOT_DIR=${TOOLCHAIN_ROOT_DIR}
                    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                    -DCMAKE_INSTALL_MESSAGE=LAZY
                    -DBUILD_SHARED_LIBS=ON
                    -DCMAKE_CXX_FLAGS="-fPIC")
ExternalProject_Get_Property(tracer INSTALL_DIR)
if (ENABLE_SYSTRACE)
install(DIRECTORY ${INSTALL_DIR}/ DESTINATION . USE_SOURCE_PERMISSIONS)
set(TRACER_LIB_DIR ${INSTALL_DIR}/lib)
endif()
