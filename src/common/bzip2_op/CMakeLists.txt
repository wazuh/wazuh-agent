add_library(bzip2_op STATIC src/bzip2_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(bzip2_op PUBLIC include)

target_link_libraries(bzip2_op
    error_messages
    file_op
    Logger
    pal
    time_op
    utils)
