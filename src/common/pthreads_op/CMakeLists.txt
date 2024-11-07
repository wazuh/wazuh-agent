add_library(pthreads_op STATIC src/pthreads_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(pthreads_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)

target_link_libraries(pthreads_op utils time_op Logger)
