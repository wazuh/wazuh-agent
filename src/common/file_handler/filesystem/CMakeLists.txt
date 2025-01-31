cmake_minimum_required(VERSION 3.22)

project(Filesystem)

include(../../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(Filesystem STATIC
    src/filesystem.cpp
)

target_include_directories(Filesystem PUBLIC include)
target_link_libraries(Filesystem PUBLIC utils)

include(../../../cmake/ConfigureTarget.cmake)
configure_target(Filesystem)
