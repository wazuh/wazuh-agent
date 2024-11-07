add_library(bzip2_op STATIC src/bzip2_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(bzip2_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)

target_link_libraries(bzip2_op
    utils
    Logger
    time_op)
