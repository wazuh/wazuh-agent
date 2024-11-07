find_package(ZLIB REQUIRED)

add_library(file_op STATIC src/file_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(file_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)

target_link_libraries(file_op utils time_op version_op ZLIB::ZLIB Logger)
