cmake_minimum_required(VERSION 3.12.4)
project(pal)

if(WIN32)
    set(OS_NAME "windows")
elseif(APPLE)
    set(OS_NAME "macos")
elseif(UNIX)
    set(OS_NAME "linux")
else()
    message(FATAL_ERROR "Unsupported OS")
endif()

file(GLOB_RECURSE SOURCE_FILES src/${OS_NAME}/*.c)

message(STATUS "Source files: ${SOURCE_FILES}")

if(NOT SOURCE_FILES)
    message(FATAL_ERROR "No source files found for OS: ${OS_NAME}")
endif()

add_library(pal STATIC ${SOURCE_FILES})
target_include_directories(pal PUBLIC include)
target_include_directories(pal PUBLIC include/${OS_NAME})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/${OS_NAME})
endif()
