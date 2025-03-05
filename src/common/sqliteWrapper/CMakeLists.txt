cmake_minimum_required(VERSION 3.22)

project(sqlite_wrapper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(sqlite_wrapper INTERFACE)

target_include_directories(sqlite_wrapper INTERFACE include)

target_link_libraries(sqlite_wrapper INTERFACE deleter_helper pal)
