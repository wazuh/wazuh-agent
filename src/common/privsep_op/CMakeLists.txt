add_library(privsep_op STATIC src/privsep_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(privsep_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)

target_link_libraries(privsep_op utils time_op Logger)
