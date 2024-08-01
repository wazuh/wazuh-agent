add_library(binaries_op STATIC src/binaries_op.c)

target_include_directories(binaries_op PUBLIC include)

target_link_libraries(binaries_op utils debug_op)