find_package(ZLIB REQUIRED)

add_library(file_op STATIC src/file_op.c)

target_include_directories(file_op PUBLIC include)

target_link_libraries(file_op utils time_op version_op debug_op ZLIB::ZLIB)