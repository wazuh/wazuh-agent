add_library(logging_helper STATIC src/logging_helper.c)

target_include_directories(logging_helper PUBLIC include)

target_link_libraries(logging_helper debug_op)