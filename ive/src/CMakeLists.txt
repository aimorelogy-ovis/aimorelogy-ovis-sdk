project(ive)
set(CVIIVE_TAG ${LATEST_TAG})
configure_file(version.hpp.in version.hpp @ONLY)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MLIR_INCLUDES}
    ${CVIMATH_INCLUDES}
    ${MIDDLEWARE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../prebuilt/middleware/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libsophon/bmlib/include
)

set(SRC
    # ${CMAKE_CURRENT_SOURCE_DIR}/core.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/ive.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/ive_tpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_internel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_subads_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_threshold_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_two_way_blend_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_morph_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_raw12_to_uint16_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_remap_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mars3_tpu/tpu_cmodel.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/kernel_generator.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/table_manager.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu_data.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_add.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_add_bf16.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_and.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_block_bf16.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_block.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_const_fill.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_copy_direct.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_copy_interval.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_filter.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_magandang.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_mask.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_max.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_min.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_morph.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_mulsum.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_normalize.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_or.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_sad.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_sigmoid.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_sobel_gradonly.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_sobel.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_sub_abs.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_sub.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_table.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_threshold_high_low.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_threshold_slope.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_threshold.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_xor.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_alpha_blend.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_alpha_blend_pixel.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_convert_scale_abs.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/tpu/tpu_blend_pixel_ab.cpp
)

# set(DRAWSRC ${CMAKE_CURRENT_SOURCE_DIR}/ive_draw.cpp
#             ${CMAKE_CURRENT_SOURCE_DIR}/2ddraw/tpu_draw_rect.cpp)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libsophon/build/bmlib
)

add_library(cvi_ive_tpu SHARED ${SRC})

if (ENABLE_SYSTRACE)
# add_dependencies(cvi_ive_tpu tracer)
target_link_libraries(cvi_ive_tpu  ${MLIR_LIBS} bmlib)
set_target_properties(cvi_ive_tpu PROPERTIES
                     IMPORTED_LOCATION )
else()
target_link_libraries(cvi_ive_tpu ${MLIR_LIBS} bmlib)
endif()


add_library(cvi_ive_tpu-static ${SRC})

if (ENABLE_SYSTRACE)
# add_dependencies(cvi_ive_tpu-static tracer)
target_link_libraries(cvi_ive_tpu-static ${MLIR_LIBS} bmlib)
endif()

install(TARGETS cvi_ive_tpu DESTINATION lib)
install(TARGETS cvi_ive_tpu-static DESTINATION lib)
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/cvi_comm_ive.h DESTINATION include/ive)
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/cvi_draw_ive.h DESTINATION include/ive)
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/ive.h DESTINATION include/ive)
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/ive_draw.h DESTINATION include/ive)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/ive_tpu.h DESTINATION include/ive)
