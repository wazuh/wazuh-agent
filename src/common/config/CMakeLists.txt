cmake_minimum_required(VERSION 3.22)

project(Config)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(Config INTERFACE)

include(../../cmake/config.cmake)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/config.h")

target_include_directories(Config INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include")
