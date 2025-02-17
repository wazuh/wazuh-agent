cmake_minimum_required(VERSION 3.22)

project(time_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(time_helper INTERFACE)

target_include_directories(time_helper INTERFACE include)

target_link_libraries(time_helper INTERFACE string_helper pal)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
