set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

function(buildninstall_test FNAME)
add_executable(${FNAME} ${FNAME}.c)
  set_source_files_properties(${FNAME}.c PROPERTIES LANGUAGE CXX)

  target_compile_features(${FNAME} PRIVATE cxx_std_11)

  find_library(ATOMIC_LIB atomic)
  if(ATOMIC_LIB)
    target_link_libraries(${FNAME} cvi_ive_tpu ${ATOMIC_LIB} -lm bmlib)
  else()
    target_link_libraries(${FNAME} cvi_ive_tpu -lm bmlib)
  endif()

  install(TARGETS ${FNAME} DESTINATION bin)
  install(FILES ${FNAME}.c DESTINATION sample)
endfunction()

project(sample)
include_directories(
    ${MLIR_INCLUDES}
    ${MIDDLEWARE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libsophon/bmlib/include
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libsophon/build/bmlib
)

# buildninstall_test(sample_add)
# buildninstall_test(sample_and)
# buildninstall_test(sample_block)
# buildninstall_test(sample_copy)
# buildninstall_test(sample_copy_yuv420p)
# buildninstall_test(sample_copy_yuv420sp)
# buildninstall_test(sample_dilate)
# buildninstall_test(sample_filter)
# buildninstall_test(sample_island)
# buildninstall_test(sample_hog)
# buildninstall_test(sample_magandang)
# buildninstall_test(sample_manual)
# buildninstall_test(sample_map)
# buildninstall_test(sample_mask)
# buildninstall_test(sample_morph)
# buildninstall_test(sample_normgrad)
# buildninstall_test(sample_or)
# buildninstall_test(sample_ordstat)
# buildninstall_test(sample_sad)
# buildninstall_test(sample_sub)
# buildninstall_test(sample_thresh)
# buildninstall_test(sample_threshold)
# buildninstall_test(sample_thresh16)
# buildninstall_test(sample_xor)
# buildninstall_test(sample_alpha_blend)
# buildninstall_test(sample_alpha_blend_pixel)
# buildninstall_test(sample_alpha_blend_pixel_yuv420)
# buildninstall_test(sample_convert_scale_abs)
# buildninstall_test(sample_average)

# buildninstall_test(sample_draw_rect)


# buildninstall_test(sample_16bitTo8bit)
# buildninstall_test(sample_histEq)
# buildninstall_test(sample_histogram)
# buildninstall_test(sample_integral_image)
# buildninstall_test(sample_ncc)
# buildninstall_test(sample_lbp)

buildninstall_test(sample_tpu_subads)
buildninstall_test(sample_tpu_threshold)
buildninstall_test(sample_tpu_blend_2way)
buildninstall_test(sample_tpu_dilate)
buildninstall_test(sample_tpu_erode)
buildninstall_test(sample_tpu_raw12_to_uint16)
buildninstall_test(sample_tpu_remap)
