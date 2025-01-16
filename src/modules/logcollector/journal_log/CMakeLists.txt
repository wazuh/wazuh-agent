cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(JournalLog)

include(../../../cmake/CommonSettings.cmake)
set_common_settings()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WIN32 OR APPLE)
    message(FATAL_ERROR "JournalLog is only supported on Linux platforms")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

FILE(GLOB JOURNAL_LOG_SOURCES src/*.cpp)

add_library(JournalLog ${JOURNAL_LOG_SOURCES})

target_include_directories(JournalLog
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${SYSTEMD_INCLUDE_DIRS}
)

target_link_libraries(JournalLog
    PUBLIC
        Logger
    PRIVATE
        systemd
)

include(../../../cmake/ConfigureTarget.cmake)
configure_target(JournalLog)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()