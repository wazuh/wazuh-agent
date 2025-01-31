cmake_minimum_required(VERSION 3.22)

project(FilesystemWrapper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(FilesystemWrapper STATIC
    src/filesystem_wrapper.cpp
)

target_include_directories(FilesystemWrapper PUBLIC include)
target_link_libraries(FilesystemWrapper PUBLIC utils)

include(../../cmake/ConfigureTarget.cmake)
configure_target(FilesystemWrapper)
