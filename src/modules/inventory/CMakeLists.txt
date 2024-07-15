cmake_minimum_required(VERSION 3.24)

project(Inventory)

add_library(inventory src/inventory.cpp)

target_include_directories(inventory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)