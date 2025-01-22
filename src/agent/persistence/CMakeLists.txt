cmake_minimum_required(VERSION 3.22)

project(Persistence)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(fmt REQUIRED)
find_package(SQLiteCpp REQUIRED)

add_library(Persistence src/persistence_factory.cpp src/sqlite_manager.cpp)

include(../../cmake/ConfigureTarget.cmake)
configure_target(Persistence)

target_include_directories(Persistence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Persistence PRIVATE SQLiteCpp fmt::fmt Logger)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
