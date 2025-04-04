cmake_minimum_required(VERSION 3.22)

project(Logger)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(spdlog REQUIRED)

set(LIBRARIES spdlog::spdlog)

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
target_link_libraries(Logger PUBLIC ${LIBRARIES})

include(../../cmake/ConfigureTarget.cmake)
configure_target(Logger)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
