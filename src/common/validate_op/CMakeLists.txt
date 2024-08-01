add_library(validate_op STATIC src/validate_op.c)

target_include_directories(validate_op PUBLIC include)

target_link_libraries(validate_op utils debug_op)