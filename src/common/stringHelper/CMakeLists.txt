cmake_minimum_required(VERSION 3.22)

project(string_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(string_helper INTERFACE)

target_include_directories(string_helper INTERFACE include)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
