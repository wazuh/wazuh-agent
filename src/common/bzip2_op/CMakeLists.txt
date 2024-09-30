add_library(bzip2_op STATIC src/bzip2_op.c)

target_include_directories(bzip2_op PUBLIC include ../logger/include)

target_link_libraries(bzip2_op
    utils
    Logger
    time_op)
