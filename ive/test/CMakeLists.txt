set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

function(build_test FNAME)
  add_executable(${FNAME} ${FNAME}.c)
  set_source_files_properties(${FNAME}.c PROPERTIES LANGUAGE CXX)
  target_compile_features(${FNAME} PRIVATE cxx_std_11)
  target_link_libraries(${FNAME} cvi_ive_tpu -lm bmlib)
  install(TARGETS ${FNAME} DESTINATION bin)
endfunction()

project(test-c)
include_directories(
    ${MLIR_INCLUDES}
    ${MIDDLEWARE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/3rdparty
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libsophon/bmlib/include
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libsophon/build/bmlib
)

# build_test(test_add_c)
# build_test(test_and_c)
# build_test(test_block_c)
# build_test(test_copy_c)
# build_test(test_copy2_c)
# build_test(test_cmp_c)
# build_test(test_cviimg_c)
# build_test(test_hog_c)
# build_test(test_island_c)
# build_test(test_morph_c)
# build_test(test_mulsum_c)
# build_test(test_norm_grad_c)
# build_test(test_filter_c)
# build_test(test_or_c)
# build_test(test_read_c)
# build_test(test_sad_c)
# build_test(test_sad_stereo_c)
# build_test(test_sigmoid_c)
# build_test(test_sobel_grad_c)
# build_test(test_sub_c)
# build_test(test_table_c)
# build_test(test_threshold_c)
# build_test(test_thresholds16_c)
# build_test(test_thresholdu16_c)
# build_test(test_xor_c)
# # FIXME: Rearrange in alphabet order.
# build_test(test_hog_batch_c)
# build_test(test_16bitTo8bit_c)
# build_test(test_histogram_c)
# build_test(test_histEq_c)
# build_test(test_integral_image_c)
# build_test(test_ncc_c)
# build_test(test_lbp_c)
# build_test(test_resize_c)
# build_test(test_csc_c)
# build_test(test_filter_csc_c)
# build_test(test_sub_clip_c)
# build_test(test_blend_u8_ab_c)
build_test(test_tpu_blend_2way_c)
build_test(test_tpu_subads_c)
build_test(test_tpu_threshold_c)
build_test(test_tpu_morph_c)
build_test(test_tpu_raw12_to_uint16_c)
build_test(test_tpu_remap_c)