cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(SQLiteManager)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(fmt REQUIRED)
find_package(SQLiteCpp REQUIRED)

add_library(SQLiteManager src/sqlite_manager.cpp)

include(../../cmake/ConfigureTarget.cmake)
configure_target(SQLiteManager)

target_include_directories(SQLiteManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SQLiteManager PUBLIC SQLiteCpp PRIVATE fmt::fmt Logger)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
