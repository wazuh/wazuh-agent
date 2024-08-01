add_library(string_op STATIC src/string_op.c)

target_include_directories(string_op PUBLIC include)

target_link_libraries(string_op utils debug_op)