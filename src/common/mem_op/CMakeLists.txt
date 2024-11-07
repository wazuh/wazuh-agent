add_library(mem_op STATIC src/mem_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(mem_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)

target_link_libraries(mem_op utils Logger time_op)
