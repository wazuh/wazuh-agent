cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Logger)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

set(LIBRARIES
    spdlog::spdlog
    fmt::fmt
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEMD REQUIRED libsystemd)
    list(APPEND LIBRARIES ${SYSTEMD_LIBRARIES})
endif()

if(WIN32)
    set(SOURCES src/logger_win.cpp)
elseif(APPLE)
    set(SOURCES src/logger_macos.cpp)
elseif(UNIX)
    set(SOURCES src/logger_linux.cpp)
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
