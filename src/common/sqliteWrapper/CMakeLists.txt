cmake_minimum_required(VERSION 3.22)

project(sqlite_wrapper)

find_package(unofficial-sqlite3 CONFIG REQUIRED)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(sqlite_wrapper src/sqliteWrapper.cpp)

target_include_directories(sqlite_wrapper PUBLIC include)

target_link_libraries(sqlite_wrapper PUBLIC pal PRIVATE deleter_helper unofficial::sqlite3::sqlite3)

include(../../cmake/ConfigureTarget.cmake)
configure_target(sqlite_wrapper)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
