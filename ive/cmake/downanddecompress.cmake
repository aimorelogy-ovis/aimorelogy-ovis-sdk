function (download_and_decompress URL FILE_NAME WORK_DIR HASH_TYPE HASH)
    if (NOT EXISTS ${WORK_DIR})
    file(MAKE_DIRECTORY ${WORK_DIR})
    endif()
    file(DOWNLOAD ${URL} ${WORK_DIR}/${FILE_NAME}
         SHOW_PROGRESS
         TIMEOUT 60
         EXPECTED_HASH ${HASH_TYPE}=${HASH})
         #file(DOWNLOAD ${url} ${filename}
         #     TIMEOUT 60)  # seconds
         # EXPECTED_HASH SHA1=${hash}
         # TLS_VERIFY ON)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${FILE_NAME}
                    WORKING_DIRECTORY ${WORK_DIR})
endfunction(download_and_decompress)