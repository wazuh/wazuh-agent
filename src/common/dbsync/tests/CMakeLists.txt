cmake_minimum_required(VERSION 3.22)

project(unit_tests)

find_package(GTest CONFIG REQUIRED)

add_subdirectory(interface)
add_subdirectory(pipelineFactory)
add_subdirectory(dbengine)
