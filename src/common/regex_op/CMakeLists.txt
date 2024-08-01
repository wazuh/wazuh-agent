add_library(regex_op STATIC src/regex_op.c)

target_include_directories(regex_op PUBLIC include)

target_link_libraries(regex_op utils debug_op)