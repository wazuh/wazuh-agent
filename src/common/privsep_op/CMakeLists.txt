add_library(privsep_op STATIC src/privsep_op.c)

target_include_directories(privsep_op PUBLIC include)

target_link_libraries(privsep_op utils time_op debug_op)