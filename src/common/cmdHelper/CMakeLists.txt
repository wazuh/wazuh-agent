cmake_minimum_required(VERSION 3.22)

project(cmd_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(cmd_helper INTERFACE)

target_include_directories(cmd_helper INTERFACE include)

target_link_libraries(cmd_helper INTERFACE pal)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
