cmake_minimum_required(VERSION 3.22)

project(integration_tests)

include_directories(${CMAKE_SOURCE_DIR}/include/)

find_package(GTest CONFIG REQUIRED)

add_subdirectory(fim)
