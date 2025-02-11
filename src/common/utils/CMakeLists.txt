cmake_minimum_required(VERSION 3.22)

project(utils)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(utils INTERFACE)

target_include_directories(utils INTERFACE include)

target_link_libraries(utils INTERFACE Logger)
