add_library(time_op STATIC src/time_op.c)

target_include_directories(time_op PUBLIC include)

target_link_libraries(time_op utils Logger file_op)
