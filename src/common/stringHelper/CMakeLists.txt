cmake_minimum_required(VERSION 3.22)

project(string_helper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(fmt REQUIRED)

add_library(string_helper src/stringHelper.cpp)

target_include_directories(string_helper PUBLIC include)

include(../../cmake/ConfigureTarget.cmake)

target_link_libraries(string_helper fmt::fmt)

configure_target(string_helper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
