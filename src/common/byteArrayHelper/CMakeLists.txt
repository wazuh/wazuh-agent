cmake_minimum_required(VERSION 3.22)

project(byte_array_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(byte_array_helper INTERFACE)

target_include_directories(byte_array_helper INTERFACE include)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
