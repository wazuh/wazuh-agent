add_library(pthreads_op STATIC src/pthreads_op.c)

target_include_directories(pthreads_op PUBLIC include)

target_link_libraries(pthreads_op utils time_op debug_op)