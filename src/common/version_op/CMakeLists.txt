add_library(version_op STATIC src/version_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(version_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)

target_link_libraries(version_op
    utils
    time_op
    file_op
    mem_op
    privsep_op
    pthreads_op
    error_messages
    binaries_op
    regex_op
    randombytes
    bzip2_op
    Logger)
