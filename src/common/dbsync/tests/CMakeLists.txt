cmake_minimum_required(VERSION 3.22)

project(unit_tests)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/sqlite/)

find_package(GTest CONFIG REQUIRED)

add_subdirectory(sqlite)
add_subdirectory(interface)
add_subdirectory(pipelineFactory)
add_subdirectory(dbengine)
