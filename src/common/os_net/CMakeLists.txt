add_library(os_net STATIC src/os_net.c)

target_include_directories(os_net PUBLIC include)

target_link_libraries(os_net utils debug_op)