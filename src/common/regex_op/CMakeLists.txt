add_library(regex_op STATIC src/regex_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(regex_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)

target_link_libraries(regex_op utils Logger time_op)
