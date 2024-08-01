add_library(version_op STATIC src/version_op.c)

target_include_directories(version_op PUBLIC include)

target_link_libraries(version_op utils time_op debug_op)