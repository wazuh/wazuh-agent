cmake_minimum_required(VERSION 3.24)

project(Inventory)

add_library(inventory src/inventory.cpp)

target_include_directories(inventory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/debug_op/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/string_op/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/pthreads_op/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/data_provider/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/utils

)

target_link_libraries(inventory cjson)