cmake_minimum_required(VERSION 3.22)

project(json_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(json_helper INTERFACE)

target_include_directories(json_helper INTERFACE include)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
