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
get_filename_component(COMMON_FOLDER  ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)

message(STATUS "Source files: ${SOURCE_FILES}")

if(NOT SOURCE_FILES)
    message(FATAL_ERROR "No source files found for OS: ${OS_NAME}")
endif()

add_library(pal STATIC ${SOURCE_FILES})

include_directories(${COMMON_FOLDER}/error_messages/include/)

target_include_directories(pal PUBLIC include)
target_include_directories(pal PUBLIC include/${OS_NAME})

find_package(cJSON CONFIG REQUIRED)
target_link_libraries(pal file_op cjson)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/${OS_NAME})
endif()
