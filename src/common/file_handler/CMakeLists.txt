cmake_minimum_required(VERSION 3.22)

project(FileHandler)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

# Filesystem target

add_library(Filesystem STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem/src/filesystem.cpp
)

target_include_directories(Filesystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/filesystem/include)
target_link_libraries(Filesystem PUBLIC utils)

# File io target

add_library(FileIO STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/file_io/src/file_io.cpp
)

target_include_directories(FileIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/file_io/include)

