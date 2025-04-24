cmake_minimum_required(VERSION 3.22)

project(OsUtils)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(OsUtils src/os_utils_unix.cpp)
target_include_directories(OsUtils PUBLIC include PRIVATE src)
target_link_libraries(OsUtils PUBLIC FilesystemWrapper PRIVATE Logger)

include(../../cmake/ConfigureTarget.cmake)
configure_target(OsUtils)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
