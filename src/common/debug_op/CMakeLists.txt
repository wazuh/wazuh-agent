add_library(debug_op src/debug_op.c)

target_include_directories(debug_op PUBLIC include)
target_include_directories(debug_op PUBLIC ${CMAKE_SOURCE_DIR}/common/utils/)