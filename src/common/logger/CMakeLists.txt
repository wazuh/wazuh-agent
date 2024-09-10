cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Logger)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEMD REQUIRED libsystemd)
endif()

set(SOURCES
    $<$<BOOL:$<PLATFORM_ID:Windows>>:src/logger_win.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:src/logger_linux.cpp>
)

add_library(Logger ${SOURCES})
target_include_directories(Logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Logger PUBLIC spdlog::spdlog fmt::fmt $<$<NOT:$<PLATFORM_ID:Windows>>:${SYSTEMD_LIBRARIES}>)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Logger)

if(BUILD_TESTS)
    enable_testing()
endif()
