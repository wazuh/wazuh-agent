add_library(version_op STATIC src/version_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(version_op PUBLIC
    include
    ${COMMON_FOLDER}/binaries_op/include
    ${COMMON_FOLDER}/file_op/include
    ${COMMON_FOLDER}/pal/include
    ${COMMON_FOLDER}/regex_op/include
    ${COMMON_FOLDER}/utils/include)

target_link_libraries(version_op
    utils
    time_op
    mem_op
    privsep_op
    pthreads_op
    binaries_op
    error_messages
    binaries_op
    regex_op
    randombytes
    bzip2_op
    Logger)
