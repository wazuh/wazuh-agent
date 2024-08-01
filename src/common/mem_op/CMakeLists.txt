add_library(mem_op STATIC src/mem_op.c)

target_include_directories(mem_op PUBLIC include)

target_link_libraries(mem_op utils debug_op)
