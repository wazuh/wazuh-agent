cmake_minimum_required(VERSION 3.22)

project(map_wrapper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(map_wrapper INTERFACE)

target_include_directories(map_wrapper INTERFACE include)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
