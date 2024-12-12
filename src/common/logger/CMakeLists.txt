cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Logger)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(spdlog REQUIRED)

set(LIBRARIES
    spdlog::spdlog
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    list(APPEND LIBRARIES)
endif()

if(WIN32)
    set(SOURCES src/logger_win.cpp)
else()
    set(SOURCES src/logger_unix.cpp)
endif()

add_library(Logger ${SOURCES})
target_include_directories(Logger PUBLIC include)
target_link_libraries(Logger PUBLIC ${LIBRARIES} utils)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Logger)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
