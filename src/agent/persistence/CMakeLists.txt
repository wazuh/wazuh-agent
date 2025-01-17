cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../../)

project(Persistence)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(fmt REQUIRED)
find_package(SQLiteCpp REQUIRED)

add_library(Persistence src/persistence_factory.cpp src/sqlite_manager.cpp)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Persistence)

target_include_directories(Persistence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Persistence PRIVATE SQLiteCpp fmt::fmt Logger)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
