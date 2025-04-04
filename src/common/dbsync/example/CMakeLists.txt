cmake_minimum_required(VERSION 3.22)

project(dbsync_example)

add_executable(dbsync_example ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_include_directories(dbsync_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../src/)

target_link_libraries(dbsync_example dbsync)

include(../../../cmake/ConfigureTarget.cmake)
configure_target(dbsync_example)
