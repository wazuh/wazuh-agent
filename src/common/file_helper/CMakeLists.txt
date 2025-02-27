cmake_minimum_required(VERSION 3.22)

project(FileHandler)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

include(../../cmake/ConfigureTarget.cmake)

# FilesystemWrapper target
add_library(FilesystemWrapper STATIC ${CMAKE_CURRENT_SOURCE_DIR}/filesystem/src/filesystem_wrapper.cpp)
target_include_directories(FilesystemWrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/filesystem/include)
target_link_libraries(FilesystemWrapper PUBLIC utils)
configure_target(FilesystemWrapper)

# File io target
add_library(FileIO STATIC ${CMAKE_CURRENT_SOURCE_DIR}/file_io/src/file_io_utils.cpp ${CMAKE_CURRENT_SOURCE_DIR}/file_io/src/file_io_wrapper.cpp)
target_include_directories(FileIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/file_io/include)
configure_target(FileIO)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(filesystem/tests)
    add_subdirectory(file_io/tests)
endif()
