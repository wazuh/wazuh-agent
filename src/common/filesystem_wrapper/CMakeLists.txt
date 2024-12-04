cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(FilesystemWrapper)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(FilesystemWrapper STATIC
    src/filesystem_wrapper.cpp
)

target_include_directories(FilesystemWrapper PUBLIC include)
