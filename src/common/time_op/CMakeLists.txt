add_library(time_op STATIC src/time_op.c)

get_filename_component(COMMON_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)
target_include_directories(time_op PUBLIC include)
include_directories(${COMMON_FOLDER}/pal/include)
include_directories(${COMMON_FOLDER}/regex_op/include)
include_directories(${COMMON_FOLDER}/error_messages/include)
include_directories(${COMMON_FOLDER}/utils/include)

target_link_libraries(time_op utils Logger)
