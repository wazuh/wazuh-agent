add_library(binaries_op STATIC src/binaries_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(binaries_op PUBLIC
    include
    ${COMMON_FOLDER}/error_messages/include
    ${COMMON_FOLDER}/file_op/include
    ${COMMON_FOLDER}/pal/include
    ${COMMON_FOLDER}/utils/include)

target_link_libraries(binaries_op
    utils
    Logger
    pal
    time_op)
