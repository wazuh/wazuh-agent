cmake_minimum_required(VERSION 3.24)

project(Inventory)

get_filename_component(SRC_FOLDER       ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
get_filename_component(COMMON_FOLDER    ${SRC_FOLDER}/common/ ABSOLUTE)

add_library(inventory src/inventory.cpp)

target_include_directories(inventory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${COMMON_FOLDER}
    ${COMMON_FOLDER}/dbsync/include
    ${COMMON_FOLDER}/debug_op/include
    ${COMMON_FOLDER}/string_op/include
    ${COMMON_FOLDER}/pthreads_op/include
    ${COMMON_FOLDER}/logging_helper/include
    ${COMMON_FOLDER}/data_provider/include
    ${COMMON_FOLDER}/utils

)

target_link_libraries(inventory dbsync sysinfo cjson nlohmann_json::nlohmann_json)